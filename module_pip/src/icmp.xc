// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xclib.h>
#include <print.h>
#include "icmp.h"
#include "ethernet.h"
#include "tx.h"
#include "ipv4.h"
#include "checksum.h"


// RFC 0792

void pipIncomingICMP(unsigned short packet[], int ipOffset, int icmpOffset, int srcIP, int dstIP, int length) {
    int type = (packet, unsigned char[])[icmpOffset * 2];
    if (myIP != 0 && type == 8) {  // Echo request (PING).
                      // bounce message back with type = 0 (PONG), everything else is identical.
                      // And swap srcIP and dstIP.
        txData(17, (packet, unsigned char[]), 34, length);       // Copy contents of old packet
        txByte(icmpOffset*2, 0);                                 // Set type to 'reply'.
        txShort(18, 0);                                          // This could be simplified by patching old checksum rather than computing a new one.
        txShort(18, onesChecksum(0, (txbuf, unsigned short[]), 17, length));
        pipOutgoingIPv4(PIP_IPTYPE_ICMP, srcIP, length);
    }
}

void pipOutgoingICMPPing(unsigned dstIP, int sequence) {
    int offset = 17;
    int length = 32 - offset;
    txShort(offset, 8);   // Type 8: echo request; code 0.
    txShort(offset+1, 0); // Initial checksum
    txInt(offset + 2, byterev(sequence));
    for(int i = offset+4; i < offset + length; i++) {
        txShort(i, i);
    }
    txShort(offset + 1, onesChecksum(0, (txbuf, unsigned short[]), 17, length<<1));
    pipOutgoingIPv4(PIP_IPTYPE_ICMP, dstIP, length<<1);
}
