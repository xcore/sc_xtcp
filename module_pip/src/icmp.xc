// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include "icmp.h"

// RFC 0792

void pipIncomingICMP(unsigned short packet[], int offset, int srcIP, int dstIP) {
    int type = (packet, unsigned char[])[offset * 2];
    int identifier = packet[offset + 2];
    int sequence   = packet[offset + 3];
    if (type == 8) {  // Echo request (PING).
                      // bounce message back with type = 0 (PONG), everything else is identical.
                      // And swap srcIP and dstIP.
    }
}
