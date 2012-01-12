// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/** Function that gets an integer from a byte location in a packet. Can
 * have any alignment.
 */
extern unsigned getIntUnaligned(unsigned short packet[], int offset);
