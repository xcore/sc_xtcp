// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xclib.h>
#include <print.h>
#include "icmp.h"
#include "ethernet.h"
#include "tx.h"

// RFC 0792

void pipIncomingICMP(unsigned short packet[], int ipOffset, int icmpOffset, int srcIP, int dstIP) {
    int type = (packet, unsigned char[])[icmpOffset * 2];
    int identifier = packet[icmpOffset + 2];
    int sequence   = packet[icmpOffset + 3];
    if (type == 8) {  // Echo request (PING).
                      // bounce message back with type = 0 (PONG), everything else is identical.
                      // And swap srcIP and dstIP.
        txData(6, (packet, unsigned char[]), 12, 64);       // Copy contents of old packet
        txByte(icmpOffset*2, 0);                            // Set type to 'reply'.

        printhexln(srcIP);
        // This should be building an IP header.`
        txInt(ipOffset + 6, byterev(dstIP));
        txInt(ipOffset + 8, byterev(srcIP));

        pipOutgoingEthernet((packet, unsigned char[]), 0, PIP_ETHTYPE_IPV4_REV);
    }
}
