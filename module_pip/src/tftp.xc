#include <xclib.h>
#include <print.h>
#include "tftp.h"
#include "rx.h"
#include "tx.h"
#include "udp.h"
#include "ethernet.h"
#include "timer.h"


// DHCP: RFC 1350

#define RRQ     1
#define WRQ     2
#define DATA    3
#define ACK     4
#define ERROR   5

unsigned pipPortTFTP;
unsigned pipIpTFTP;
static unsigned clientPort;
static int address, nextAck;

void pipAcknowledgeTFTP() {
    txShortRev(21, ACK);    
    txShortRev(22, nextAck);
    pipOutgoingUDP(pipIpTFTP, pipPortTFTP, clientPort, 4);
}

void pipReadTFTP() {
    txShortRev(21, RRQ);
    txData(22, "/x\000octet\000", 0, 9); 
    pipOutgoingUDP(pipIpTFTP, pipPortTFTP, clientPort, 10);
}

void pipInitTFTP() {
    timer t;
    t :> pipPortTFTP;
    pipPortTFTP = pipPortTFTP % 0xFFFF + 1;
    nextAck = 1;
    clientPort = TFTP_SERVER_PORT;
    pipReadTFTP();
    address = 0x10000;
}

//TODO: implement CRC over whole packet
//TODO: make sure that code is located high up
//TODO: implement Jump
//TODO: implement timeout.

void pipIncomingTFTP(unsigned short packet[], unsigned srcIP, unsigned dstIP, 
                     unsigned srcPort, int offset, int length) {
    int opcode = byterev(packet[offset])>>16;
    nextAck = byterev(packet[offset+1])>>16;

    if (srcPort != clientPort && clientPort == 69) {
        clientPort = srcPort;
    }
    switch(opcode) {
    case WRQ:
    case RRQ:
    case ERROR:
        break;
    case DATA:
        for(int i = 0; i < length - offset * 2 - 4; i++) {
            asm("st8 %0, %1[%2]" :: "r" ((packet, unsigned char[])[offset * 2+4 + i]), "r" (address+(nextAck-1) * 512 + i), "r"(0));
        }
        pipAcknowledgeTFTP();
        break;
    case ACK:
        break;
    }
}
