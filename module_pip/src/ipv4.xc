// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xclib.h>
#include <print.h>
#include "ipv4.h"
#include "tcp.h"
#include "udp.h"
#include "icmp.h"
#include "igmp.h"
#include "checksum.h"

// RFC 791

// TODO: Must be doing defragmentation somewhere.
// TODO: Must support datagrams of at least 576 octets.

int myIP = 0xA9FEFFFF;

unsigned int getReversedInt(unsigned short packet[], int offset) {
    unsigned int x = packet[offset+1] << 16 | packet[offset];
    return byterev(x);
}

void pipIncomingIPv4(unsigned short packet[], int offset) {
    int ipType = (packet, unsigned char[])[offset * 2 + 9];
    int headerLength = (packet, unsigned char[])[offset * 2] & 0xF; // # words
    int contentOffset = headerLength * 2 + offset;

    unsigned int srcIP = getReversedInt(packet, offset + 6);
    unsigned int dstIP = getReversedInt(packet, offset + 8);

    int chkSum = onesChecksum(0, packet, offset, contentOffset);
    
    if (chkSum != 0) {
        printstr("Bad chksum ");
        printhexln(chkSum);
        return;        // Bad checksum; drop.
    }

    if (dstIP != 0xFFFFFFFF &&
        (dstIP >> 24) != 224 && 
        dstIP != 0 &&
        dstIP != myIP) {
        printstr("Not us ");
        printhexln(dstIP);
        return;        // dest ip address is not us; drop.

    }
#if defined(PIP_TCP)
    if (ipType == 0x06) {
        pipIncomingTCP(packet, contentOffset, srcIP, dstIP);
        return;
    }
#endif

#if defined(PIP_UDP)
    if (ipType == 0x11) {
        pipIncomingUDP(packet, contentOffset, srcIP, dstIP);
        return;
    }
#endif

#if defined(PIP_ICMP)
    if (ipType == 0x01) {
        pipIncomingICMP(packet, offset, contentOffset, srcIP, dstIP);
        return;
    }
#endif

#if defined(PIP_IGMP)
    if (ipType == 0x02) {
        pipIncomingIGMP(packet, contentOffset, srcIP, dstIP);
        return;
    }
#endif

}
