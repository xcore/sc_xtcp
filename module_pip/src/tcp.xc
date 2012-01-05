// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include <xclib.h>
#include <print.h>
#include "tcp.h"
#include "tx.h"
#include "checksum.h"
#include "ipv4.h"

// RFC 793

#define TCPCONNECTIONS 10

struct tcpConnection {
    unsigned short srcPortRev, dstPortRev;
    int srcIP, dstIP;
    short maxSegmentSize;
    char opened;
    char state;
    int streamSequenceNumber;
    int streamAckNumber;
    int appWaiting;               // This contains a streaming chanend if the app is waiting.
};

struct tcpConnection tcpConnections[TCPCONNECTIONS] = {
    {0, 0x5000, 0, 0, 0, 0, 0},
};

#define FIN 0x01
#define SYN 0x02
#define RST 0x04
#define PSH 0x08
#define ACK 0x10
#define URG 0x20

#define CLOSED    0
#define LISTEN    2
#define SYNSENT   3
#define SYNRCVD   4
#define ESTAB     5
#define FINWAIT1  6
#define FINWAIT2  7
#define CLOSING   8
#define CLOSEWAIT 9
#define LASTACK  10
#define TIMEWAIT 11
#define TIMEWAIT0 12
#define TIMEWAIT1 13
#define TIMEWAIT2 14

void error(char msg[], int flags) {
    printstr(msg);
    printstr(" received flags ");
    printhexln(flags);
}


void pipOutgoingTCP(struct tcpConnection &conn, int length, int flags) {
    int totalLength = length + 20;
    int chkSum;
    int zero = 17;
    txShort(zero+0, conn.dstPortRev);             // Store source port, already reversed
    txShort(zero+1, conn.srcPortRev);             // Store dst port, already reversed
    txInt(zero+2, byterev(conn.streamSequenceNumber)); // Sequence number, reversed
    txInt(zero+4, byterev(conn.streamAckNumber)); // Ack number, reversed
    txShort(zero+6, flags<<8 | 0x50);             // Store dst port, already reversed
    txShort(zero+7, byterev(1500)>>16);           // Store dst port, already reversed
    txInt(zero+8, 0);                             // checksum, urgent pointer
    chkSum = onesChecksum(0x0006 + totalLength + onesAdd(myIP, conn.srcIP), (txbuf, unsigned short[]), 17, totalLength);
    txShort(zero+8, chkSum);                      // checksum.
    pipOutgoingIPv4(PIP_IPTYPE_TCP, conn.srcIP, totalLength);
//    printstr("Send ");
//    printhexln(conn.srcIP);
//    printhexln(totalLength);
}

static void acknowledgeApp(struct tcpConnection & conn) {
#if PIP_TCP_ACK_CT != 3
#error "PIP_TCP_ACK_CT must be 3"
#endif
    asm("outct res[%0], 3" :: "r" (conn.appWaiting));
}

static void goTimeWait(struct tcpConnection & conn) {
    conn.state = TIMEWAIT;
    if (conn.appWaiting) {
        acknowledgeApp(conn);
    }
}

void pipTimeoutTimewaitTCP() {
    for(int i = 0; i < TCPCONNECTIONS; i++) {
        switch(tcpConnections[i].state) {
        case TIMEWAIT:  tcpConnections[i].state = TIMEWAIT0; break;
        case TIMEWAIT0: tcpConnections[i].state = TIMEWAIT1; break;
        case TIMEWAIT1: tcpConnections[i].state = TIMEWAIT2; break;
        case TIMEWAIT2:
            tcpConnections[i].state = CLOSED;
            if (tcpConnections[i].appWaiting) {
                acknowledgeApp(tcpConnections[i]);
            }
            break;
        }
    }
}

void pipIncomingTCP(unsigned short packet[], int offset, int srcIP, int dstIP) {
    int srcPortRev        = packet[offset+0];
    int dstPortRev        = packet[offset+1];
    int sequenceNumberRev = packet[offset+3]<<16 | packet[offset+2];
    int ackNumberRev      = packet[offset+5]<<16 | packet[offset+4];
    int dataOffset        = packet[offset+6] & 0xF;
    int flags             = packet[offset+6] >> 8;
    int tcpOffset         = offset + dataOffset * 2;
    int window            = packet[offset+8];

    int opened = -1;
    int openable = -1;
    for(int i = 0; i < TCPCONNECTIONS; i++) {
        if (dstPortRev == tcpConnections[i].dstPortRev) {
            if (tcpConnections[i].opened &&
                tcpConnections[i].srcPortRev == srcPortRev &&
                tcpConnections[i].srcIP      == srcIP &&
                tcpConnections[i].dstIP      == dstIP) { // Found the open stream.
                opened = i;
                break;
            }
            if (openable == -1 && !tcpConnections[i].opened) {
                openable = i;
            }
        }
    }
    if (opened == -1) {
        if (openable == -1) {               // Reject this connection; nobody listening.
            if (!(flags & RST)) {
                struct tcpConnection pseudoConnection;
                pseudoConnection.dstPortRev = dstPortRev;
                pseudoConnection.srcPortRev = srcPortRev;
                pseudoConnection.srcIP = srcIP;
                if (flags & ACK) {
                    pseudoConnection.streamAckNumber = 0;
                    pseudoConnection.streamSequenceNumber = byterev(ackNumberRev);
                } else {
                    pseudoConnection.streamSequenceNumber = 0;
                    pseudoConnection.streamAckNumber = 0;
                }
                pipOutgoingTCP(pseudoConnection, 0, RST);
            }
            return;
        }
        opened = openable;
        tcpConnections[opened].opened = 1;
        tcpConnections[opened].srcPortRev = srcPortRev;
        tcpConnections[opened].srcIP = srcIP;
        tcpConnections[opened].dstIP = dstIP;
        tcpConnections[opened].state = LISTEN;
    }

    switch(tcpConnections[opened].state) {
    case CLOSED:
        break;
    case LISTEN:
        if (flags & SYN) {
            tcpConnections[opened].streamAckNumber = byterev(sequenceNumberRev) + 1;
            tcpConnections[opened].streamSequenceNumber = 0; // could be random
            pipOutgoingTCP(tcpConnections[opened], 0, SYN|ACK);
            tcpConnections[opened].streamSequenceNumber++;
            tcpConnections[opened].state = SYNRCVD;
            return;
        }
        error("LISTEN", flags);
        break;
    case SYNSENT:
        break;
    case SYNRCVD:
        if (flags & ACK) {
            // Note or check sequence numbers?
            return;
        }
        error("SYNRCVD", flags);
        break;
    case ESTAB:
        if (flags & FIN) {
            tcpConnections[opened].streamSequenceNumber++;
            tcpConnections[opened].streamAckNumber++;
            pipOutgoingTCP(tcpConnections[opened], 0, ACK);
            tcpConnections[opened].state = CLOSEWAIT;
            return;
        }
        error("ESTAB", flags);

        break;
    case FINWAIT1:
        if (flags & FIN) {
            tcpConnections[opened].streamSequenceNumber++; // TODO
            tcpConnections[opened].streamAckNumber++;      // TODO
            pipOutgoingTCP(tcpConnections[opened], 0, ACK);
            tcpConnections[opened].state = CLOSING;
            return;
        }
        if (flags & ACK) {
            tcpConnections[opened].state = FINWAIT2;
            return;
        }
        error("FINWAIT1", flags);

        break;
    case FINWAIT2:
        if (flags & FIN) {
            tcpConnections[opened].streamSequenceNumber++; // TODO
            tcpConnections[opened].streamAckNumber++;      // TODO
            pipOutgoingTCP(tcpConnections[opened], 0, ACK);
            goTimeWait(tcpConnections[opened]);
            return;
        }
        error("FINWAIT2", flags);
        break;
    case CLOSING:
        if (flags & ACK) {
            tcpConnections[opened].state = TIMEWAIT;
            if (tcpConnections[opened].appWaiting) {
                acknowledgeApp(tcpConnections[opened]);
            }
            return;
        }
        error("CLOSING", flags);
        break;
    case TIMEWAIT:
    case TIMEWAIT0:
    case TIMEWAIT1:
    case TIMEWAIT2:
        break;
    case CLOSEWAIT:
        break;
    case LASTACK:
        break;
    }
    // Check TCP header.

    // Compare source port, dest port, and source IP against table.

    // Set data ready for appropriate entry.
}

static void setAppWaiting(struct tcpConnection &conn, streaming chanend app) {
    int x;
    asm(" or %0, %1, %2" : "=r" (x): "r" (app), "r" (app));
    conn.appWaiting = x;
}

static void doClose(struct tcpConnection &conn, streaming chanend app) {
    switch(conn.state) {
    case CLOSED:
        soutct(app, PIP_TCP_ACK_CT); 
        return;
    case SYNSENT:
    case LISTEN:
        conn.opened = 0;
        conn.state = CLOSED;
        soutct(app, PIP_TCP_ACK_CT); 
        return;
    case SYNRCVD:
    case ESTAB:
        conn.state = FINWAIT1;
        pipOutgoingTCP(conn, 0, FIN);
        setAppWaiting(conn, app);
        return;
    case LASTACK:
    case TIMEWAIT:
    case TIMEWAIT0:
    case TIMEWAIT1:
    case TIMEWAIT2:
    case FINWAIT1:
    case FINWAIT2:
    case CLOSING:
        return;
    case CLOSEWAIT:
        conn.state = LASTACK;
        pipOutgoingTCP(conn, 0, FIN);
        setAppWaiting(conn, app);
        return;
    }
}

static void doRead(struct tcpConnection &conn, streaming chanend app) {
    switch(conn.state) {
    case CLOSED:
    case SYNSENT:
    case LISTEN:
    case SYNRCVD:
    case LASTACK:
    case TIMEWAIT:
    case TIMEWAIT0:
    case TIMEWAIT1:
    case TIMEWAIT2:
    case FINWAIT1:
    case FINWAIT2:
    case CLOSING:
        soutct(app, PIP_TCP_ERROR_CT);
        return;
    case ESTAB:
        conn.state = FINWAIT1;
        pipOutgoingTCP(conn, 0, FIN);
        setAppWaiting(conn, app);
        return;
    case CLOSEWAIT:
        soutct(app, PIP_TCP_CLOSED_CT);
        return;
    }
}

void pipApplicationTCP(streaming chanend app, int cmd) {
    int connectionNumber;
    switch(cmd) {
    case PIP_TCP_ACCEPT:
        app :> connectionNumber;
        setAppWaiting(tcpConnections[connectionNumber], app);
        break;
    case PIP_TCP_CLOSE:
        app :> connectionNumber;
        doClose(tcpConnections[connectionNumber], app);
        break;
    case PIP_TCP_READ:
        app :> connectionNumber;
        doRead(tcpConnections[connectionNumber], app);
        break;
    case PIP_TCP_WRITE:
        break;
    }
}

void pipApplicationAccept(streaming chanend stack, int connection) {
    stack <: PIP_TCP_ACCEPT;
    stack <: connection;
    stack :> int ack;
}

void pipApplicationClose(streaming chanend stack, int connection) {
    stack <: PIP_TCP_CLOSE;
    stack <: connection;
    stack :> int ack;
}
#if 0

#define HEADERS_LEN_TCP 54

void tcpString(char s[]) {
    int t;
    int len = strlen(s);
    t = packetBufferAlloc();

    for(int i = 0; i < len; i++) {
        (packetBuffer[t], unsigned char[])[HEADERS_LEN_TCP+i] = s[i];
    }
    (packetBuffer[t], unsigned char[])[HEADERS_LEN_TCP+len] = 0;

    patchTCPHeader(packetBuffer[t], len, ACK | PSH | FIN);
    
    qPut(toHost, t, HEADERS_LEN_TCP + len);
    streamSequenceNumber += len;
    return;
}

void processTCPPacket(unsigned int packet, int len) {
    int sourcePortRev = (packetBuffer[packet], unsigned short[])[17];
    int destPortRev = (packetBuffer[packet], unsigned short[])[18];
    int sequenceNumberRev = (packetBuffer[packet], unsigned short[])[20]<<16 |
                            (packetBuffer[packet], unsigned short[])[19];
    int ackNumberRev = (packetBuffer[packet], unsigned short[])[22]<<16 |
                       (packetBuffer[packet], unsigned short[])[21];
    int packetLength;
    int headerLength;

    streamSourcePortRev = sourcePortRev;
    streamDestPortRev = destPortRev;

    if (packetBuffer[packet][11] & 0x02000000) { // SYN
        int t;
        t = packetBufferAlloc();
        streamAckNumber = byterev(sequenceNumberRev) + 1;
        streamSequenceNumber = 0; // could be random

        patchTCPHeader(packetBuffer[t], 0, SYN | ACK);
        streamSequenceNumber++;
        qPut(toHost, t, HEADERS_LEN_TCP);
        return;
    }
    if (packetBuffer[packet][11] & 0x01000000) { // FIN, send an ACK.
        int t;
        t = packetBufferAlloc();
        streamSequenceNumber++;
        streamAckNumber++;

        patchTCPHeader(packetBuffer[t], 0, ACK);
        qPut(toHost, t, HEADERS_LEN_TCP);
        return;
    }
    if (packetBuffer[packet][11] & 0x10000000) { // ACK
        ; // required later to send long responses.
    }
    packetLength = byterev((packetBuffer[packet], unsigned short[])[8]) >> 16;
    headerLength = (packetBuffer[packet], unsigned char[])[46]>>2;

    packetLength -= headerLength + 20;

    streamAckNumber += packetLength;

    if (packetLength > 0) {
        if (destPortRev == 0x5000) { // HTTP
            httpProcess(packet, 34 + headerLength, packetLength);
        }
    }
    if (packetBuffer[packet][11] & 0x08000000) { // PSH
        ; // Can safely be ignored.
    }
}

#endif
