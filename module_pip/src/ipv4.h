// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#define PIP_IPTYPE_TCP  0x06
#define PIP_IPTYPE_UDP  0x11
#define PIP_IPTYPE_ICMP 0x01
#define PIP_IPTYPE_IGMP 0x02

/** Function that deals with an incoming IPv4 packet.
 *
 * \param packet contains the packet data.
 * 
 * \param offset points to the IPv4 header in the packet.
 */

void pipIncomingIPv4(unsigned short packet[], int offset);


void pipOutgoingIPv4(int ipType, unsigned ipDst, int length);

extern unsigned myIP, mySubnetIP;
