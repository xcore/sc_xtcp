// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/** Function that processes an incoming UDP packet. The UDP header starts
 * at the given offset in the packet. The source and destination IP
 * addresses are passed to enable the checksum computation and for
 * application level software.
 *
 * \param packet packetdata.
 *
 * \param offset Index of first element in the array that contains the UDP header.
 *
 * \param srcIP  Source IP address from IP header, in host order.
 *
 * \param dstIP  Destination IP address from IP header (the address of this
 *               host if all is well, or some sort of multicast address), in host order.
 */
void pipIncomingUDP(unsigned short packet[], unsigned offset, unsigned srcIP, unsigned dstIP);

/** Function that prepends a UDP header in front of the current packet. The
 * UDP header will transmit to the given IP address and port, from the
 * given source port. The UDP packet contains length data bytes (note this
 * excludes the header, so pass 0 for an empty datagram). Note that if no
 * ARP entry is present, the current packet id destroyed and an ARP packet
 * is sent instead.
 *
 * \param dstIP   Destination IP address, in host order.
 *
 * \param srcPort Port number where the data came from
 *
 * \param dstPort Port number where the data goes to
 *
 * \param length  number of bytes in the datagram
 */
void pipOutgoingUDP(unsigned dstIP, unsigned srcPort, unsigned dstPort, unsigned length);
