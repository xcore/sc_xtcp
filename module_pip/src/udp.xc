// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xclib.h>
#include <print.h>
#include "config.h"
#include "udp.h"
#include "checksum.h"
#include "ipv4.h"
#include "dhcp.h"
#include "tftp.h"
#include "tx.h"

// RFC 0768

void pipIncomingUDP(unsigned short packet[], unsigned offset, unsigned srcIP, unsigned dstIP) {
    int srcPort =     byterev(packet[offset+0]) >> 16;  // Not used at present
    int dstPort =     byterev(packet[offset+1]) >> 16;
    int totalLength = byterev(packet[offset+2]) >> 16;
    int chkSum;
    
    chkSum = onesChecksum(0x0011 + totalLength + onesAdd(srcIP, dstIP),
                          packet, offset, totalLength);
    if (chkSum != 0) {
        return; /* ignore packet with bad chksum */
    }
    
    // Check destination port, set packet ready for appropriate packet handler.

#if defined(PIP_DHCP)
    if (dstPort == 0x0044) {
        pipIncomingDHCP(packet, srcIP, dstIP, offset + 4, totalLength - 8);
    }
#endif

#if defined(PIP_TFTP)
    if (dstPort == pipPortTFTP) {
        pipIncomingTFTP(packet, srcIP, dstIP, srcPort, offset + 4, totalLength - 8);
    }
#endif

}

void pipOutgoingUDP(unsigned dstIP, unsigned srcPort, unsigned dstPort, unsigned length) {
    int totalLength = length + 8;
    int chkSum;
    txShortRev(17, srcPort);               // Store source port
    txShortRev(18, dstPort);               // Store source port
    txShortRev(19, totalLength);           // Total length, including header.
    txShort(20, 0);                        // Total length, including header.
    chkSum = onesChecksum(0x0011 + totalLength + onesAdd(myIP, dstIP), (txbuf, unsigned short[]), 17, totalLength);
    txShort(20, chkSum);                        // Total length, including header.
    pipOutgoingIPv4(PIP_IPTYPE_UDP, dstIP, totalLength);
}
