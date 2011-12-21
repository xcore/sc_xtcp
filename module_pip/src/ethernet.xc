// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include "ipv4.h"
#include "arp.h"

void pipIncomingEthernet(unsigned short packet[]) {
    int offset = 6;
    int ethType;

    do {
        ethType = packet[offset];      // Ethernet type in network order - swapped
        offset++;
    } while (ethType == 0x0081);

#if defined(PIP_TCP) || defined(PIP_UDP) || defined(PIP_ICMP) || defined(PIP_IGMP)
    if (ethType == 0x0008) {
        pipIncomingIPv4(packet, offset);
        return;
    }
#endif

#if defined(PIP_ARP)
    if (ethType == 0x0608) {
        pipIncomingARP(packet, offset);
        return;
    }
#endif

}
