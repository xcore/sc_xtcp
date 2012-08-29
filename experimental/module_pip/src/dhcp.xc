// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xclib.h>
#include <print.h>
#include "config.h"
#include "dhcp.h"
#include "tftp.h"
#include "rx.h"
#include "tx.h"
#include "udp.h"
#include "linklocal.h"
#include "ipv4.h"
#include "ethernet.h"
#include "timer.h"

#define DHCP_CLIENT_PORT     68
#define DHCP_SERVER_PORT     67
// DHCP: RFC 2131

#define STATE_BLANK 0
#define STATE_INITED 1
#define STATE_ALLOCATED 2

static int xid;
static int interval = 4;
static int state = STATE_BLANK;

unsigned myIP, serverIP, mySubnetIP, myRouterIP;

void pipCreateDHCP(int firstMessage,
                  unsigned proposedIP,
                  unsigned serverIP) {
    int length = 244;
    int seconds = 0;
    int zero = 21;

    if (!firstMessage) {
        length += 12;
    }

    txInt(zero, 0x00060102);             // Fill Hop, Hlen, HType, OP.
    txInt(zero + 2, xid);                // Fill XID
    txShortRev(zero + 4, seconds);       // Seconds since we started
    txShort(zero + 5, 0x0080);           // Flags: broadcase
    txShortZeroes(zero + 6, 112);        // Set all addr values to 0, chaddr, sname, file
    txData(zero+14, myMacAddress, 0, 6);   // FIll in mac address
    txInt(zero+118, 0x63538263);         // Fill Hop, Hlen, HType, OP.
    txInt(zero+120, 0xFF010135);         // Discover & end option.
    if (firstMessage) {
        txInt(zero+120, 0xFF010135);         // Discover & end option.
    } else {
        txInt(zero+120, 0x32030135);         // REQUEST option, proposed IP option
        txInt(zero+122, byterev(proposedIP) << 8 | 0x04); // Length, address
        txInt(zero+124, (proposedIP & 0xff) |
                         0x00043600 |
                         (serverIP & 0xff000000) ); // server IP option & length
        txInt(zero+126, byterev(serverIP) >> 8 | 0xff000000); // address & end option.
    }
    pipOutgoingUDP(0xFFFFFFFF, DHCP_CLIENT_PORT, DHCP_SERVER_PORT, length);
}

void pipIncomingDHCP(unsigned short packet[], unsigned srcIP, unsigned dstIP, int offset, int length) {
    unsigned leaseTime;
    unsigned renewalTime;
    unsigned rebindTime;
    unsigned proposedIP;
    unsigned subnet, messageType, router;

    if (packet[offset+118] != 0x8263 || packet[offset+119] != 0x6353) {
        return;                                  // incorrect magic cookie
    }
    if ((packet[offset+2] | packet[offset+3]<<16) != xid) {
        return;                                  // incorrect XID
    }
    proposedIP = getIntUnaligned(packet, offset*2 + 16);
    subnet = 0;
    router = 0;
    leaseTime = 0;
    renewalTime = 0;
    rebindTime = 0;
    serverIP = 0;
    messageType = 0;
    for(int i = offset * 2 + 240; i < offset * 2 + length;) {
        if ((packet, unsigned char[])[i] == 255) {
            break;
        }
        switch((packet, unsigned char[])[i]) {
        case 0x35: messageType = (packet, unsigned char[])[i+2]; break;
        case 0x36: serverIP    = getIntUnaligned(packet, i+2); break;
        case 0x33: leaseTime   = getIntUnaligned(packet, i+2); break;
        case 0x3A: renewalTime = getIntUnaligned(packet, i+2); break;
        case 0x3B: rebindTime  = getIntUnaligned(packet, i+2); break;
        case 0x01: subnet      = getIntUnaligned(packet, i+2); break;
#ifdef PIP_TFTP
        case 0x96: pipIpTFTP   = getIntUnaligned(packet, i+2); break;
#endif
#ifdef PIP_GATEWAY
        case 0x03: router      = getIntUnaligned(packet, i+2); break;
#endif
            // TODO: store router(s)
            // TODO: store DNS server(s)
        }
        i += (packet, unsigned char[])[i+1] + 2;
    }
    if (messageType == 2) {
        pipCreateDHCP(0, proposedIP, serverIP);
        interval = 1;
        pipSetTimeOut(PIP_DHCP_TIMER_T2, interval, 0, PIP_FUZZ_1S);
    } else if (messageType == 5) {
        if (renewalTime == 0) {
            renewalTime = (leaseTime >> 1);
        }
        if (rebindTime == 0) {
            rebindTime = leaseTime - (leaseTime >> 3);
        }
#ifdef PIP_LINK_LOCAL
        pipDisableLinkLocal();
#endif        
        pipSetTimeOut(PIP_DHCP_TIMER_T1, renewalTime, 0, PIP_FUZZ_1S);
        pipSetTimeOut(PIP_DHCP_TIMER_T2, rebindTime, 0, PIP_FUZZ_1S);
        state = STATE_ALLOCATED;
        pipAssignIPv4(proposedIP, subnet, router);
#ifdef PIP_TFTP
        pipInitTFTP();
#endif
    }
}

void pipInitDHCP() {
#ifdef PIP_DHCP_DONT_WAIT
    pipSetTimeOut(PIP_DHCP_TIMER_T2, 3, 0, 0);
#else
    pipSetTimeOut(PIP_DHCP_TIMER_T2, 1, 0, PIP_FUZZ_10S);
#endif
}

void pipTimeOutDHCPT2() {
    timer t;
    t :> xid;
    if (state == STATE_ALLOCATED) {
        state = STATE_INITED;
        pipUnassignIPv4();
    }
    pipCreateDHCP(1, 0, 0);
    pipSetTimeOut(PIP_DHCP_TIMER_T2, interval, 0, PIP_FUZZ_1S);
#ifdef PIP_LINK_LOCAL
    if (state != STATE_BLANK) {
        pipInitLinkLocal();
    }
#endif
    state = STATE_INITED;
    interval = interval * 2;
    if (interval > 60) {
        interval = 60;
    }
}

void pipTimeOutDHCPT1() {
    pipCreateDHCP(0, myIP, serverIP);
}

