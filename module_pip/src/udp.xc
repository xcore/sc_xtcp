// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xclib.h>
#include "udp.h"
#include "checksum.h"

// RFC 0768

void pipIncomingUDP(unsigned short packet[], int offset, int srcIP, int dstIP) {

    int srcPort =     byterev(packet[offset+0]) >> 16;
    int dstPort =     byterev(packet[offset+1]) >> 16;
    int totalLength = byterev(packet[offset+2]) >> 16;
    int chkSum;
    // ignore +3, it is the checksum.
    
    chkSum = onesChecksum(0x0011 + totalLength + onesAdd(srcIP, dstIP),
                          packet, offset, offset + ((totalLength+1) >> 1));
    if (chkSum != 0xffff) {
        return; /* bad chksum */
    }

    
    // Check destination port, set packet ready for appropriate packet handler.

#if defined(DHCP)
    if (dstPort == 0x0044) {
        dhcp(packet, srcIP, dstIP, offset + 4, totalLength - 8);
    }
#endif

}
