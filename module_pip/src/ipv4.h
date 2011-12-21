// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/** Function that deals with an incoming IPv4 packet.
 *
 * \param packet contains the packet data.
 * 
 * \param offset points to the IPv4 header in the packet.
 */

void pipIncomingIPv4(unsigned short packet[], int offset);
