// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include <xclib.h>
#include <print.h>
#include "config.h"
#include "tftp.h"
#include "rx.h"
#include "tx.h"
#include "udp.h"
#include "ethernet.h"
#include "timer.h"


// TFTP: RFC xxxx

#define RRQ     1
#define WRQ     2
#define DATA    3
#define ACK     4
#define ERROR   5

unsigned pipPortTFTP;
unsigned pipIpTFTP;
static unsigned clientPort;
static int address, nextAck;
static int lastCmd = 0;

extern int reboot;

static void doSend(int cmd, int length) {
    txShortRev(21, cmd);
    pipOutgoingUDP(pipIpTFTP, pipPortTFTP, clientPort, length);
    lastCmd = cmd;
    pipSetTimeOut(PIP_TFTP_TIMER, 0, 1000000, PIP_FUZZ_10MS);
}

void pipAcknowledgeTFTP() {
    txShortRev(22, nextAck);
    doSend(ACK, 4);
}

void pipReadTFTP() {
    int fileNameLength = 2;
    txData(22, "/x\000octet\000", 0, fileNameLength + 7); 
    doSend(RRQ, fileNameLength + 9);
}

void pipInitTFTP() {
    timer t;
    t :> pipPortTFTP;
    pipPortTFTP = pipPortTFTP % 0xFFFF + 1;
    nextAck = 1;
    address = 0x1B000;            // TODO: MAKE 0x10000
    clientPort = TFTP_SERVER_PORT;
    pipReadTFTP();
}

static unsigned checksum = 0xDEADBEEF;

#define polynomial 0xEDB88320
#define BLOCKSIZE 512

void pipTimeOutTFTP() {
    if (lastCmd == RRQ) {
        pipReadTFTP();            // Read has gone missing.
    } else if (lastCmd == ACK) {
        pipAcknowledgeTFTP();     // Ack has gone missing.
    }
}

void pipIncomingTFTP(unsigned short packet[], unsigned srcIP, unsigned dstIP, 
                     unsigned srcPort, int offset, int length) {
    int opcode = byterev(packet[offset])>>16;
    int start;
    int base;
    int ourBase = 0x1C000;
    if (srcPort != clientPort) {
        if (clientPort == TFTP_SERVER_PORT) {
            clientPort = srcPort;
        } else {
            return;
        }
    }
    nextAck = byterev(packet[offset+1])>>16;
    switch(opcode) {
    case WRQ:
    case RRQ:
    case ERROR:
        break;
    case DATA:
        start = offset * 2 + 4;
        length -= start;
        base = address + (nextAck - 1) * BLOCKSIZE;
        if (base < ourBase) {
            for(int i = 0; i < length; i++) {
                int data = (packet, unsigned char[])[start + i];
                asm("st8 %0, %1[%2]" :: "r" (data), "r" (base + i), "r"(0));
                crc8shr(checksum, data, polynomial);
            }
        }
        pipAcknowledgeTFTP();
        if (length != BLOCKSIZE) {
            pipResetTimeOut(PIP_TFTP_TIMER);
            if (1 || checksum == 0) {
                reboot = 1;
            } else {
                pipInitTFTP();
            }
        }
        break;
    case ACK:
        break;
    }
}




//TODO: make sure that code is located high up
//TODO: implement Jump
