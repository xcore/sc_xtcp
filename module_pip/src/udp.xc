// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xclib.h>
#include <print.h>
#include "udp.h"
#include "checksum.h"
#include "ipv4.h"
#include "dhcp.h"
#include "tx.h"

// RFC 0768

void pipIncomingUDP(unsigned short packet[], int offset, int srcIP, int dstIP) {

    int srcPort =     byterev(packet[offset+0]) >> 16;
    int dstPort =     byterev(packet[offset+1]) >> 16;
    int totalLength = byterev(packet[offset+2]) >> 16;
    int chkSum;
    // ignore +3, it is the checksum.
    
    chkSum = onesChecksum(0x0011 + totalLength + onesAdd(srcIP, dstIP),
                          packet, offset, totalLength);
    if (chkSum != 0) {
        printstr("Bad checksum ");
        printhexln(chkSum);
//        return; /* bad chksum */ TODO
    }

    
    // Check destination port, set packet ready for appropriate packet handler.

#if defined(PIP_DHCP)
    if (dstPort == 0x0044) {
        pipDhcpIncoming(packet, srcIP, dstIP, offset + 4, totalLength - 8);
    }
#endif

}

extern unsigned int myIP;

void pipOutgoingUDP(int dstIP, int srcPort, int dstPort, int length) {
    int totalLength = length + 8;
    int chkSum;
    txShort(17, shortrev(srcPort));             // Store source port
    txShort(18, shortrev(dstPort));             // Store source port
    txShort(19, shortrev(totalLength));         // Total length, including header.
    txShort(20, 0);                        // Total length, including header.
    chkSum = onesChecksum(0x0011 + totalLength + onesAdd(myIP, dstIP), (txbuf, unsigned short[]), 17, totalLength);
    txShort(20, chkSum);                        // Total length, including header.
    pipOutgoingIPv4(PIP_IPTYPE_UDP, dstIP, totalLength);
}
