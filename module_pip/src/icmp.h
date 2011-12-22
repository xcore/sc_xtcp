// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/** Function that processes an incoming ICMP packet. The ICMP header starts
 * at the given offset in the packet.
 *
 * \param packet packetdata.
 *
 * \param ipOffset Index of first element in the array that contains the IP header.
 *
 * \param icmpOffset Index of first element in the array that contains the ICMP header.
 *
 * \param srcIP  Source IP address from IP header, in host order.
 *
 * \param dstIP  Destination IP address from IP header (the address of this
 *               host if all is well, or some sort of multicast address), in host order.
 */
void pipIncomingICMP(unsigned short packet[], int ipOffset, int icmpOffset, int srcIP, int dstIP);
