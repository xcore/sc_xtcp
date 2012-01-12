// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#define PIP_IPTYPE_TCP  0x06
#define PIP_IPTYPE_UDP  0x11
#define PIP_IPTYPE_ICMP 0x01
#define PIP_IPTYPE_IGMP 0x02

/** Function that deals with an incoming IPv4 packet. Normally called from the Ethernet layer.
 *
 * \param packet contains the packet data.
 * 
 * \param offset points to the IPv4 header in the packet.
 */

void pipIncomingIPv4(unsigned short packet[], int offset);

/** Function that adds an IPv4 header to the outgoing packet. It needs the
 * ipType (one of the above defines), destination address and a length of
 * the datagram. Note that if no ARP entry is available for this packet,
 * then the packet shall be discarded and a ARP request shall be send
 * instead.
 *
 * \param ipType type of the IP packet to be transmitted. One of the
 *               defines above.
 *
 * \param ipDst  IP address of the destination, in normal order, i.e.
 *               192.168.1.1 is represented as 0xC0A80101
 *
 * \param length Number of bytes in the datagram.
 */
void pipOutgoingIPv4(int ipType, unsigned ipDst, int length);

/** Variable that holds the current IP address; 0 if this node does not
 * have an IP address at present.
 */
extern unsigned myIP;

/** Variable that holds the current IP subnet; 0 if this node does not have
 * an IP address at present.
 */
extern unsigned mySubnetIP;
