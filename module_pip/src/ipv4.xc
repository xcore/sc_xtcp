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
#include "tx.h"
#include "arp.h"
#include "ethernet.h"

// RFC 791

// TODO: Must be doing defragmentation somewhere.
// TODO: Must support datagrams of at least 576 octets.

unsigned int getReversedInt(unsigned short packet[], int offset) {
    unsigned int x = packet[offset+1] << 16 | packet[offset];
    return byterev(x);
}

void pipIncomingIPv4(unsigned short packet[], int offset) {
    int ipType = (packet, unsigned char[])[offset * 2 + 9];
    int headerLength = (packet, unsigned char[])[offset * 2] & 0xF; // # words
    int totalLength = byterev(packet[offset + 1]) >> 16;
    int contentOffset = headerLength * 2 + offset;

    unsigned int srcIP = getReversedInt(packet, offset + 6);
    unsigned int dstIP = getReversedInt(packet, offset + 8);

    int chkSum = onesChecksum(0, packet, offset, headerLength * 4);
    
    if (chkSum != 0) {
        printstr("Bad chksum ");
        printhexln(chkSum);
        return;        // Bad checksum; drop.
    }

    if ((dstIP | mySubnetIP) != 0xFFFFFFFF &&
        (dstIP >> 24) != 224 && 
        dstIP != 0 &&
        dstIP != myIP) {
        printstr("Not us ");
        printhexln(dstIP);
        return;        // dest ip address is not us; drop.
    }

    pipARPStoreEntry(srcIP, (packet, unsigned char[]), 6);


#if defined(PIP_TCP)
    if (ipType == PIP_IPTYPE_TCP) {
        pipIncomingTCP(packet, contentOffset, srcIP, dstIP);
        return;
    }
#endif

#if defined(PIP_UDP)
    if (ipType == PIP_IPTYPE_UDP) {
        pipIncomingUDP(packet, contentOffset, srcIP, dstIP);
        return;
    }
#endif

#if defined(PIP_ICMP)
    if (ipType == PIP_IPTYPE_ICMP) {
        pipIncomingICMP(packet, offset, contentOffset, srcIP, dstIP, totalLength - headerLength * 4);
        return;
    }
#endif

#if defined(PIP_IGMP)
    if (ipType == PIP_IPTYPE_IGMP) {
        pipIncomingIGMP(packet, contentOffset, srcIP, dstIP);
        return;
    }
#endif

}

void pipOutgoingIPv4(int ipType, unsigned ipDst, int length) {
    int totalLength = length + 20;
    txShort(7, 0x0045);                    // IPv4, header length 5, no flags.
    txShort(8, shortrev(totalLength));     // Length, including header.
    txInt(9, 0x0);                         // Zero id, no flags, no fragment offset.
    txShort(11, 0xff | ipType << 8);       // Set ETH mac dst
    txShort(12, 0);                        // Init checksum
    txInt(13, byterev(myIP));             // Set source IP address.
    txInt(15, byterev(ipDst));            // Set destination IP address.
    txShort(12, onesChecksum(0, (txbuf, unsigned short[]), 7, 20)); // Compute checksum
    for(int i = 0; i < ARPENTRIES; i++) {
        if (pipArpTable[i].ipAddress == ipDst) {
            pipOutgoingEthernet(pipArpTable[i].macAddr, 0, PIP_ETHTYPE_IPV4_REV);
            return;
        }
    }
    printstr("Missing arp\n");
    printhexln(ipDst);
    txClear();
    pipCreateARP(0, ipDst, pipArpTable[0].macAddr, 0);
}
