// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/** Function that processes an incoming TCP packet. The TCP header starts
 * at the given offset in the packet. The source and destination IP
 * addresses are passed to enable the checksum computation and for
 * application level software.
 *
 * \param packet packetdata.
 *
 * \param offset Index of first element in the array that contains the TCP header.
 *
 * \param srcIP  Source IP address from IP header, in host order.
 *
 * \param dstIP  Destination IP address from IP header (the address of this
 *               host if all is well, or some sort of multicast address), in host order.
 */
void pipIncomingTCP(unsigned short packet[], int offset, int srcIP, int dstIP);
