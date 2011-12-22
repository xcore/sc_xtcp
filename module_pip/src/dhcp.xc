#include "tx.h"
#include "udp.h"
#include "ethernet.h"

#define DHCP_CLIENT_PORT     68
#define DHCP_SERVER_PORT     67
// DHCP: RFC 2131

int pipDhcpCreate(int firstMessage,
                  int proposedIP, int serverIP, int myIP) {
    int length = 244;
    int seconds = 0;
    int zero = 21;

    if (!firstMessage) {
        length += 12;
    }

    txInt(zero, 0x00060102);             // Fill Hop, Hlen, HType, OP.
    txInt(zero + 2, 0xFFFFFF);           // Fill XID - could be random
    txShort(zero + 4, shortrev(seconds));// Seconds since we started
    txShort(zero + 5, 0x0080);           // Flags: broadcase
    txShortZeroes(zero + 6, 112);        // Set all addr values to 0, chaddr, sname, file
    txData(zero+14, ourMacAddress, 0, 6);   // FIll in mac address
    txInt(zero+118, 0x63538263);         // Fill Hop, Hlen, HType, OP.
    txInt(zero+120, 0xFF010135);         // Discover & end option.
    if (firstMessage) {
#if 0
        packet[off+242] = 3;
        packet[off+243] = 50;
        packet[off+244] = 4;
        packet[off+245] = proposedIP >> 24; // IP ADDRESS THAT WE SELECTED
        packet[off+246] = proposedIP >> 16;
        packet[off+247] = proposedIP >> 8;
        packet[off+248] = proposedIP ;
        packet[off+249] = 54;
        packet[off+250] = 4;
        packet[off+251] = serverIP >> 24; // IP ADDRESS THAT IS SERVING US
        packet[off+252] = serverIP >> 16;
        packet[off+253] = serverIP >> 8;
        packet[off+254] = serverIP;
        packet[off+243] = 255;
#endif
    }
    pipOutgoingUDP(0xFFFFFFFF, DHCP_CLIENT_PORT, DHCP_SERVER_PORT, length);
    
}

