// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

// RFC 826

#include <print.h>
#include <xclib.h>
#include "arp.h"
#include "ipv4.h"
#include "rx.h"
#include "tx.h"
#include "ethernet.h"
#include "timer.h"

struct arp pipArpTable[ARPENTRIES] = {
    {0xFFFFFFFF, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1, 0xff}},
};


static void macCopy(unsigned char to[], unsigned char from[], int offset) {
    for(int i = 0; i < 6; i++) {
        to[i] = from[i+offset];
    }
}

void pipInitARP() {
    pipSetTimeOut(PIP_ARP_TIMER, 1, 0, 0); // 1 s clock
}

void pipARPStoreEntry(unsigned ipAddress, unsigned char macAddress[], unsigned offset) {
    int min = pipArpTable[1].macAddr[7];
    int minEntry = 1;
    for(int i = 1; i < ARPENTRIES; i++) {
        if (pipArpTable[i].ipAddress == ipAddress) {
            macCopy(pipArpTable[i].macAddr, macAddress, offset);
            pipArpTable[i].macAddr[6] = 1;
            pipArpTable[i].macAddr[7] = 0xfe;
            return;
        }
        if (pipArpTable[i].macAddr[7] < min) {
            min = pipArpTable[i].macAddr[7];
            minEntry = i;
        }
    }
    pipArpTable[minEntry].ipAddress = ipAddress;
    macCopy(pipArpTable[minEntry].macAddr, macAddress, offset);
    pipArpTable[minEntry].macAddr[6] = 1;
    pipArpTable[minEntry].macAddr[7] = 0xfe;
}


void pipCreateARP(unsigned reply, unsigned tpa, unsigned char tha[], unsigned offset) {
    unsigned zero = 7;
    txInt(32, 0);
    txInt(zero, 0x00080100);  // Fill in hw type (1) and proto type (0x800)
    txInt(zero+2, 0x01000406 + reply * 0x01000000);  // Fill in hw size, proto size
    txData(zero+4, myMacAddress, 0, 6);          // Set ETH mac src
    txInt(zero+7, byterev(myIP));                // Set IP src
    txData(zero+9, tha, offset, 6);              // Set ETH mac dst
    txInt(zero+12, byterev(tpa));                // Set IP dst
    pipOutgoingEthernet(tha, offset, PIP_ETHTYPE_ARP_REV);
}

void pipIncomingARP(unsigned short packet[], unsigned offset) {
    int oper = packet[offset + 3];
    unsigned tpa = getIntUnaligned(packet, 2*offset + 24);
    unsigned spa = getIntUnaligned(packet, 2*offset + 14);

#ifdef PIP_LINK_LOCAL
    if (pipIncomingLinkLocalARP(oper, tpa, (packet, unsigned char[]), 2*offset + 8)) {
        return;
    }
#endif
    if (oper == 256) {             // REQUEST
        if (tpa == myIP) {         // for us.
            pipARPStoreEntry(spa, (packet, unsigned char[]), 2*offset + 8);
            pipCreateARP(1, spa, (packet, unsigned char[]), 2*offset + 8);
        }
    } else if (oper == 512) {      // REPLY
        pipARPStoreEntry(spa, (packet, unsigned char[]), 2*offset + 8);
    }
}

// TODO: initialise timeout, and repeat timeout of ARP entries.
void pipTimeOutARP() {
    for(int i = 1; i < ARPENTRIES; i++) {
        if (pipArpTable[i].macAddr[7] > 0) {
            pipArpTable[i].macAddr[7]--;
        } else {
            pipArpTable[i].macAddr[6] = 0;
        }
    }
    pipSetTimeOut(PIP_ARP_TIMER, 1, 0, 0); // 1 s clock
}
