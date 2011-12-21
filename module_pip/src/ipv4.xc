#include "ipv4.h"
#include "tcp.h"
#include "udp.h"
#include "icmp.h"
#include "igmp.h"

// RFC 791

// TODO: Must be doing defragmentation somewhere.
// TODO: Must support datagrams of at least 576 octets.

unsigned int getReversedInt(unsigned short packet, int offset) {
    unsigned int x = packet[offset] << 16 | packet[offset+1];
    return byterev(x);
}

void pipIncomingIPv4(unsigned short packet[], int offset) {
    int ipType = (packet, unsigned char[])[offset * 2 + 9];
    int headerLength = (packet, unsigned char[])[offset * 2] >> 4; // # words
    int ipOffset = headerLength * 2 + offset;

    unsigned int srcIP = getReversedInt(packet, offset + 6);
    unsigned int dstIP = getReversedInt(packet, offset + 8);

    int chkSum = onesChecksum(0, packet, offset, ipOffset);
    
    if (chkSum != 0xffff) {
        return;        // Bad checksum; drop.
    }

    if (dstIP != 0xFFFFFFFF &&
        (dstIP >> 24) != 224 && 
        dstIP != 0 &&
        dstIP != myIP) {
        return;        // dest ip address is not us; drop.

    }
#if defined(PIP_TCP)
    if (ipType == 0x06) {
        pipIncomingTCP(packet, ipOffset, srcIP, dstIP);
        return;
    }
#endif

#if defined(PIP_UDP)
    if (ipType == 0x11) {
        pipIncomingUDP(packet, ipOffset, srcIP, dstIP);
        return;
    }
#endif

#if defined(PIP_ICMP)
    if (ethType == 0x01) {
        pipIncomingICMP(packet, ipOffset, srcIP, dstIP);
        return;
    }
#endif

#if defined(PIP_IGMP)
    if (ethType == 0x02) {
        pipIncomingIGMP(packet, ipOffset);
        return;
    }
#endif

}
