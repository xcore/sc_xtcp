// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <print.h>
#include "config.h"
#include "ethernet.h"
#include "ipv4.h"
#include "arp.h"
#include "tx.h"

unsigned char myMacAddress[6] = {0x00, 0x7F, 0xD3, 0x11, 0x22, 0x33};

void pipIncomingEthernet(unsigned short packet[]) {
    int offset = 6;
    int ethType;

    do {
        ethType = packet[offset];      // Ethernet type in network order - swapped
        offset++;
    } while (ethType == 0x0081);

#if defined(PIP_IPV4)
    if (ethType == PIP_ETHTYPE_IPV4_REV) {
        pipIncomingIPv4(packet, offset);
        return;
    }
#endif

#if defined(PIP_ARP)
    if (ethType == PIP_ETHTYPE_ARP_REV) {
        pipIncomingARP(packet, offset);
        return;
    }
#endif

}

void pipOutgoingEthernet(unsigned char macDest[], int offset, int revType) {
    txData(0, macDest, offset, 6);         // Set ETH mac dst
    txData(3, myMacAddress, 0, 6);        // Set ETH mac src
    txShort(6, revType);                   // Fill in type.
}
