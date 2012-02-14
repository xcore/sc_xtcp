// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include <xclib.h>
#include <print.h>
#include <stdio.h>
#include "config.h"
#include "tcp.h"
#include "tcpApplication.h"
#include "tx.h"
#include "checksum.h"
#include "ipv4.h"
#include "timer.h"

// RFC 793


#define APP_NOT_WAITING  0
#define APP_CLOSING      1
#define APP_READING      2
#define APP_WRITING      3
#define APP_ACCEPTING    4
#define APP_CONNECTING      5

#define TCPCONNECTIONS 2
#define TCP_MAX_TIMEOUT  2048            // TODO: 128. Must be a power of 2.
#define TCP_MIN_TIMEOUT  256             // TODO: 4. Must be a power of 2.

#define BUFFERSIZERX   2048               // Must be a power of 2.
#define BUFFERSIZETX   512               // Must be a power of 2.

struct packetHistory {
    int ack, seq, length, flags, state, time;
};

struct tcpConnection {
    unsigned short remotePortRev, localPortRev;
    int remoteIP, localIP;
    int maxSegmentSize;
    int state;
    int sndNXT;                   // Sequence number that we are transmitting (ours)
    int sndUNA;                   // Last ack that we received.
    int rcvNXT;                   // Data that has been acknowledged
    int rcvWND;                   // Our receive window
    int sndWND;                   // Their receive window
    int appWaiting;               // This contains a streaming chanend if the app is waiting.
    int appStatus;                // This contains one of APP_CLOSING, APP_READING, etc
    int ackRequired;
    int timeouts;
    struct rxqueue {
        int rd, wr, free, length;
        char buffer[BUFFERSIZERX];
    } rx;
    struct txqueue {
        int rd, wr, free, length;
        char buffer[BUFFERSIZETX];
    } tx;
#ifdef PIP_DEBUG_TCP
    int wtx, wrx;
    struct packetHistory ltx[PIP_DEBUG_TCP];
    struct packetHistory lrx[PIP_DEBUG_TCP];
#endif
};

struct tcpConnection tcpConnections[TCPCONNECTIONS] = {
    {0, 0x1700, 0, 0, 0, 0, 0},
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


void pipDebugTCPStore(int ack, int seq, int flags, int length, int state, struct packetHistory ph[], int &w) {
    timer t;
    ph[w].ack = ack;
    ph[w].seq = seq;
    ph[w].length = length;
    ph[w].flags = flags;
    ph[w].state = state;
    t :> ph[w].time;
    w++;
    if (w == PIP_DEBUG_TCP) {
        w = 0;
    }
}

void error(char msg[], int flags) {
    printstr(msg);
    printstr(" received flags ");
    printhexln(flags);
}


void pipOutgoingTCP(struct tcpConnection &conn, int length, int flags) {
    int totalLength = length + 20;
    int chkSum;
    int zero = 17;
    int seq, ack;
    if (flags & SYN) {
        totalLength += 4;
    }
    if (conn.ackRequired) {
        flags |= ACK;
    }
    txShort(zero+0, conn.localPortRev);             // Store source port, already reversed
    txShort(zero+1, conn.remotePortRev);             // Store dst port, already reversed
    seq = conn.sndNXT;
    txInt(zero+2, byterev(seq)); // Sequence number, reversed
    if (flags & ACK) {
        ack = conn.rcvNXT;
    } else {
        ack = 0;
    }
    txInt(zero+4, byterev(ack)); // Ack number, reversed
    txShort(zero+6, flags<<8 | (0x50 + ((flags & SYN)<<3)));             // Flags and header size
    conn.rcvWND = conn.rx.free;
    txShort(zero+7, byterev(conn.rcvWND)>>16);    // Number of bytes free in window.
    txInt(zero+8, 0);                             // checksum, urgent pointer
    if (flags & SYN) {
        txInt(zero+10, 0x00000402 | ((BUFFERSIZERX/2)&0xff)<<24 | ((BUFFERSIZERX/2)&0xff00)<<8 );
    }
    chkSum = onesChecksum(0x0006 + totalLength + onesAdd(myIP, conn.remoteIP), (txbuf, unsigned short[]), 17, totalLength);
    txShort(zero+8, chkSum);                      // checksum.
    conn.ackRequired = 0;
    pipOutgoingIPv4(PIP_IPTYPE_TCP, conn.remoteIP, totalLength);
#ifdef PIP_DEBUG_TCP
    pipDebugTCPStore(ack, seq, flags, length, conn.state, conn.ltx, conn.wtx);
#endif
    conn.timeouts = TCP_MIN_TIMEOUT;

}

static void appSendAcknowledge(struct tcpConnection & conn) {
#if PIP_TCP_ACK_CT != 3
#error "PIP_TCP_ACK_CT must be 3"
#endif
    asm("outct res[%0], 3" :: "r" (conn.appWaiting));
    conn.appStatus = APP_NOT_WAITING;
}

static void appSendClose(struct tcpConnection & conn) {
#if PIP_TCP_CLOSED_CT != 6
#error "PIP_TCP_CLOSED_CT must be 6"
#endif
    asm("outct res[%0], 6" :: "r" (conn.appWaiting));
    conn.appStatus = APP_NOT_WAITING;
}

static void appSendError(struct tcpConnection & conn) {
#if PIP_TCP_ERROR_CT != 5
#error "PIP_TCP_CLOSED_CT must be 5"
#endif
    asm("outct res[%0], 5" :: "r" (conn.appWaiting));
    conn.appStatus = APP_NOT_WAITING;
}

void pipInitTCP() {
    pipSetTimeOut(PIP_TCP_TIMER_TIMEWAIT, 0, 10*1000*100, 0); // 10 ms clock
}

static void bounceRST(int localPortRev, int remotePortRev, int remoteIP, int segACKRev, int sequenceNumberRev, int incorporateACK, int reason) {
    struct tcpConnection pseudoConnection;
    int flags;
    pseudoConnection.localPortRev = localPortRev;
    pseudoConnection.remotePortRev = remotePortRev;
    pseudoConnection.remoteIP = remoteIP;
    pseudoConnection.wtx = 0;
    pseudoConnection.wrx = 0;
    if (incorporateACK) {
        pseudoConnection.rcvNXT = 0;
        pseudoConnection.sndNXT = byterev(segACKRev);
        flags = RST;
    } else {
        pseudoConnection.sndNXT = 0;
        pseudoConnection.rcvNXT = byterev(sequenceNumberRev)+0;//todo LEN
        flags = ACK | RST;
    }
    pipOutgoingTCP(pseudoConnection, 0, flags);
    printf("RST %d\n", reason);
}

static void copyDataForRead(struct tcpConnection &conn, streaming chanend app) {
    int bytesToSend, maxLength;
    soutct(app, PIP_TCP_ACK_CT);
    app :> maxLength;
    bytesToSend = BUFFERSIZERX - conn.rx.free;
    if (bytesToSend > maxLength) {
        bytesToSend = maxLength;
    }
    conn.rx.free += bytesToSend;
    app <: bytesToSend;
    for(int i = 0; i < bytesToSend; i++) {
        app <: conn.rx.buffer[conn.rx.rd];
        conn.rx.rd = (conn.rx.rd + 1) & (BUFFERSIZERX - 1);
    }
}

// TODO: reconcile with previous
static void copyDataForRead2(struct tcpConnection &conn, int app) {
    int bytesToSend, maxLength;
    asm("outct res[%0], 3" :: "r" (app));
    asm("in %0, res[%1]" : "=r" (maxLength) : "r" (app));
    bytesToSend = BUFFERSIZERX - conn.rx.free;
    if (bytesToSend > maxLength) {
        bytesToSend = maxLength;
    }
    conn.rx.free += bytesToSend;
    asm("out res[%0], %1" :: "r" (app), "r" (bytesToSend));
    for(int i = 0; i < bytesToSend; i++) {
        asm("outt res[%0], %1" :: "r" (app), "r" (conn.rx.buffer[conn.rx.rd]));
        conn.rx.rd = (conn.rx.rd + 1) & (BUFFERSIZERX - 1);
    }
}

static int storeIncomingData(struct tcpConnection &conn, unsigned short packet[],
                             int offset, int length) {
    int i;
    for(i = 0; i < length; i++) {
        if (conn.rx.free == 0) {
            break;
        }
        conn.rx.buffer[conn.rx.wr] = (packet, unsigned char[])[offset * 2 + i];
        conn.rx.wr = (conn.rx.wr + 1) & (BUFFERSIZERX - 1);
        conn.rx.free--;
    }
    if (conn.appStatus == APP_READING && conn.rx.free != BUFFERSIZERX) {
        copyDataForRead2(conn, conn.appWaiting);
        conn.appStatus = APP_NOT_WAITING;
    }
    conn.rcvNXT += i;
    return i == length;
}

static void copyDataFromWrite(struct tcpConnection &conn, streaming chanend app) {
    int bytesRequested;
    soutct(app, PIP_TCP_ACK_CT);
    app :> bytesRequested;
    if (bytesRequested > conn.tx.free) {
        bytesRequested = conn.tx.free;
    }
    conn.tx.free -= bytesRequested;
    conn.tx.length += bytesRequested;
    app <: bytesRequested;
    for(int i = 0; i < bytesRequested; i++) {
        app :> conn.tx.buffer[conn.tx.wr];
        conn.tx.wr = (conn.tx.wr + 1) & (BUFFERSIZETX - 1);
    }
}

// TODO: Reconcile with previous
static void copyDataFromWrite2(struct tcpConnection &conn, unsigned app) {
    int bytesRequested;
    asm("outct res[%0],3" :: "r" (app));
    asm("in %0, res[%1]" : "=r" (bytesRequested) : "r" (app));
    if (bytesRequested > conn.tx.free) {
        bytesRequested = conn.tx.free;
    }
    conn.tx.free -= bytesRequested;
    conn.tx.length += bytesRequested;
    asm("out res[%0],%1" :: "r" (app), "r" (bytesRequested));
    for(int i = 0; i < bytesRequested; i++) {
        asm("int %0, res[%1]" : "=r" (conn.tx.buffer[conn.tx.wr]) : "r" (app));
        conn.tx.wr = (conn.tx.wr + 1) & (BUFFERSIZETX - 1);
    }
}

static void loadOutgoingData(struct tcpConnection &conn) {
    int i;
    int length = conn.sndWND;
    int offset = 27;
    if (length > conn.tx.length) {      // Cannot send more data then we have
        length = conn.tx.length ;
    }
    conn.tx.length -= length;
    for(i = 0; i < length; i++) {
        txByte(offset * 2 + i, conn.tx.buffer[conn.tx.rd]);
        conn.tx.rd = (conn.tx.rd + 1) & (BUFFERSIZETX - 1);
    }
    if (conn.appStatus == APP_WRITING && conn.tx.free != BUFFERSIZETX) {
        copyDataForRead2(conn, conn.appWaiting);
        conn.appStatus = APP_NOT_WAITING;
    }
    pipOutgoingTCP(conn, i, PSH|ACK);
    conn.sndNXT += i;
}

// TODO: rename to remove Timewait - as it does all.

void pipTimeOutTCPTimewait() {
    int t, t2, notAcknowledged;
    for(int i = 0; i < TCPCONNECTIONS; i++) {
        if (tcpConnections[i].timeouts == 0) {
            continue;
        }
        t = tcpConnections[i].timeouts;
        t2 = t + 1;
        tcpConnections[i].timeouts = t2;
        if ((t & t2) != 0) {        // 8, 16, ...
            continue;
        }
        if (t2 >= TCP_MAX_TIMEOUT) {
            tcpConnections[i].timeouts = TCP_MAX_TIMEOUT;
        }
        notAcknowledged = BUFFERSIZETX - (tcpConnections[i].tx.length + tcpConnections[i].tx.free);
        if (notAcknowledged > 0) {
            tcpConnections[i].sndNXT -= notAcknowledged;
            tcpConnections[i].tx.length += notAcknowledged;
            tcpConnections[i].tx.rd = (tcpConnections[i].tx.rd - notAcknowledged) & (BUFFERSIZETX - 1);
            loadOutgoingData(tcpConnections[i]);
            printstr("+");
            continue;
        }
        switch(tcpConnections[i].state) {
        case TIMEWAIT:  tcpConnections[i].state = TIMEWAIT0; break;
        case TIMEWAIT0: tcpConnections[i].state = TIMEWAIT1; break;
        case TIMEWAIT1: tcpConnections[i].state = TIMEWAIT2; break;
        case TIMEWAIT2:
            tcpConnections[i].state = CLOSED;
            if (tcpConnections[i].appStatus != APP_NOT_WAITING) {
                appSendClose(tcpConnections[i]);
            }
            break;
        case SYNSENT:
        case SYNRCVD:
            break;
        case FINWAIT1:
        case FINWAIT2:
        case CLOSEWAIT:
        case CLOSING:
        case LASTACK:
            if (tcpConnections[i].sndNXT + 1 - tcpConnections[i].sndUNA > 0) {
                pipOutgoingTCP(tcpConnections[i], 0, FIN|ACK);
                printstr("*");
                continue;
            }
            break;
        case ESTAB:
            break;
        }
        tcpConnections[i].timeouts = 0;
    }
    pipSetTimeOut(PIP_TCP_TIMER_TIMEWAIT, 0, 10*1000*100, 0); // 10 ms clock
}

void pipIncomingTCP(unsigned short packet[], unsigned offset, unsigned srcIP, unsigned dstIP, unsigned totalLength) {
    int srcPortRev = packet[offset+0];
    int dstPortRev = packet[offset+1];
    int segSEQRev  = packet[offset+3]<<16 | packet[offset+2];
    int segSEQ     = byterev(segSEQRev);
    int segACKRev  = packet[offset+5]<<16 | packet[offset+4];
    int segACK     = byterev(segACKRev);
    int dataOffset = packet[offset+6] >> 4 & 0xF;
    int flags      = packet[offset+6] >> 8;
    int tcpOffset  = offset + dataOffset * 2;
    int window     = byterev(packet[offset+8]) >> 16;
    int length     = totalLength - dataOffset*4;

    int opened = -1;
    int extraBytes;

    for(int i = 0; i < TCPCONNECTIONS; i++) {
        if (dstPortRev == tcpConnections[i].localPortRev) {
            if (tcpConnections[i].state      != CLOSED && // TODO not LISTEN either
                tcpConnections[i].remotePortRev == srcPortRev &&
                tcpConnections[i].remoteIP      == srcIP &&
                tcpConnections[i].localIP      == dstIP) { // Found the open stream.
                opened = i;
                break;
            }
            if (tcpConnections[i].state      == LISTEN) {
                tcpConnections[i].remotePortRev = srcPortRev;
                tcpConnections[i].remoteIP = srcIP;
                tcpConnections[i].localIP = dstIP;
                opened = i;
                break;
            }
        }
    }
    if (opened == -1) {
        if (!(flags & RST)) {
            bounceRST(dstPortRev, srcPortRev, srcIP, segACKRev, segSEQRev, !(flags & ACK), 1);
            pipDebugTCPPrint();
        }
        return;
    }

#ifdef PIP_DEBUG_TCP
    pipDebugTCPStore(byterev(segACKRev), byterev(segSEQRev), flags, length, tcpConnections[opened].state, tcpConnections[opened].lrx, tcpConnections[opened].wrx);
#endif
    switch(tcpConnections[opened].state) {
    case CLOSED:
        break;
    case LISTEN:
        if (flags & RST) {
            return;
        }
        if (flags & ACK) {
            bounceRST(dstPortRev, srcPortRev, srcIP, segACKRev, segSEQRev, !(flags & ACK), 2);
            return;
        }
        if (flags & SYN) {
            timer t;
            int t0;
            t :> t0;
            tcpConnections[opened].tx.free--;
            tcpConnections[opened].rcvNXT = segSEQ + 1;
            tcpConnections[opened].sndNXT = t0;
            tcpConnections[opened].sndUNA = t0 - 1;
            pipOutgoingTCP(tcpConnections[opened], 0, SYN|ACK);
            tcpConnections[opened].sndNXT++;
            tcpConnections[opened].state = SYNRCVD;
            return;
        }
        break;
#ifdef PIP_TCP_CONNECT
    case SYNSENT:                    // Only relevant if we open a connection.
        tcpConnections[opened].sndWND = window;

        if (flags & ACK) { // first check ACK
            if(tcpConnections[opened].sndUNA - segACK > 0 ||
               segACK - tcpConnections[opened].sndNXT > 0) {
                bounceRST(dstPortRev, srcPortRev, srcIP, segACKRev, segSEQRev, 0, 3);
                if (tcpConnections[opened].appStatus != APP_NOT_WAITING) {
                    appSendError(tcpConnections[opened]);     // report problem.
                }
                tcpConnections[opened].state = CLOSED;
                return;
            }
            tcpConnections[opened].sndUNA = segACK;
        }
        if (flags & RST) { // second check RST
            if (flags & ACK) {
                appSendError(tcpConnections[opened]);
                tcpConnections[opened].state = CLOSED;
            }
        }
        if (flags & SYN) { // fourth check SYN
            tcpConnections[opened].rcvNXT = segSEQ + 1;
            // IRS IS ST TO SEQSEQ, can be ignored??
            if (flags & ACK) {
                tcpConnections[opened].sndUNA = segACK;
            }
            appSendAcknowledge(tcpConnections[opened]);
            if(tcpConnections[opened].sndUNA == segACK) { // TODO: ISS not segACK
                pipOutgoingTCP(tcpConnections[opened], 0, ACK);
                tcpConnections[opened].state = ESTAB;                
                return;
            }
            pipOutgoingTCP(tcpConnections[opened], 0, SYN|ACK);
            tcpConnections[opened].state = SYNRCVD;                
            
        }

        break;
#endif
//    case TIMEWAIT:
    case SYNRCVD:
    case ESTAB:
    case FINWAIT1:
    case FINWAIT2:
    case CLOSEWAIT:
    case CLOSING:
    case LASTACK:
//        printf("segSEQ: %08x, rcvNXT: %08x, state %d\n", segSEQ, tcpConnections[opened].rcvNXT, tcpConnections[opened].state);
        if (segSEQ != tcpConnections[opened].rcvNXT) {
//            printstr("Ignore, out of order packet\n");
            pipOutgoingTCP(tcpConnections[opened], 0, ACK);           // Out of order - reject.
            return;
        }

        // 2. Deal with RST.
        if (flags & RST) {
            switch(tcpConnections[opened].state) {
            case SYNRCVD:
                if (tcpConnections[opened].appStatus == APP_ACCEPTING) {
                    appSendClose(tcpConnections[opened]);
                }
                break;
            case ESTAB:
            case FINWAIT1:
            case FINWAIT2:
            case CLOSEWAIT:
                if (tcpConnections[opened].appWaiting) {
                    appSendError(tcpConnections[opened]);
                }
                break;
            case CLOSING:
            case LASTACK:
//            case TIMEWAIT:
                break;
            }
            tcpConnections[opened].state = CLOSED;
            return;
        }
        // 3 Check security and precedence - not required.

        // 4. Check the SYN bit.

        if (flags & SYN) {
            if (tcpConnections[opened].appWaiting) {
                appSendError(tcpConnections[opened]);
            }
            tcpConnections[opened].state = CLOSED;
            return;
        }

        // 5. Check the ACK field

        if (!(flags & ACK)) {
            return;
        }

        tcpConnections[opened].sndWND = window;

        if (tcpConnections[opened].state == SYNRCVD) {
            if(tcpConnections[opened].sndUNA - segACK > 0 ||
               segACK - tcpConnections[opened].sndNXT > 0) {
                bounceRST(dstPortRev, srcPortRev, srcIP, segACKRev, segSEQRev, 1, 4);
                // TODO: set to CLOSED?
                return;
            }
            tcpConnections[opened].sndWND = window;
            tcpConnections[opened].sndUNA++;
            tcpConnections[opened].state = ESTAB;
            if (tcpConnections[opened].appStatus == APP_ACCEPTING) {
                appSendAcknowledge(tcpConnections[opened]);
            }
        }

        if (tcpConnections[opened].state == LASTACK) {
//            printintln(tcpConnections[opened].tx.free);
//            if (tcpConnections[opened].tx.free == BUFFERSIZETX + 1) {
            
            if (tcpConnections[opened].sndNXT+1 == segACK) {
                tcpConnections[opened].state  = CLOSED;
                if (tcpConnections[opened].appStatus == APP_CLOSING) {
                    appSendAcknowledge(tcpConnections[opened]);
                }
            } else {
//                printf("DUP ACK ON LASTACK sndNXT: %08x, segACK: %08x\n", tcpConnections[opened].sndNXT, segACK);
            }
            return;
        }

//        printf("sndUNA: %08x, segACK: %08x\n", tcpConnections[opened].sndUNA, segACK);
        if(tcpConnections[opened].sndUNA - segACK > 0) {
            printstr("Ignoring duplicate ACK\n");
            return; // Duplicate ACK can be ignored.
        }
        if (segACK - tcpConnections[opened].sndNXT > 0) {
            pipOutgoingTCP(tcpConnections[opened], 0, ACK);
            return;
        }

        extraBytes = segACK - tcpConnections[opened].sndUNA;
        tcpConnections[opened].sndUNA = segACK;
        tcpConnections[opened].tx.free += extraBytes;

        if (tcpConnections[opened].tx.free == extraBytes) {
            if (tcpConnections[opened].appStatus == APP_WRITING) {
                copyDataFromWrite2(tcpConnections[opened], tcpConnections[opened].appWaiting);
                tcpConnections[opened].appStatus = APP_NOT_WAITING;
                loadOutgoingData(tcpConnections[opened]);
            }
        }
        if (tcpConnections[opened].state == FINWAIT1) {
            printf("FINWAIT1: txfree: %d\n", tcpConnections[opened].tx.free);
            if (tcpConnections[opened].tx.free == BUFFERSIZETX + 1) {
                tcpConnections[opened].state = FINWAIT2;
                return;
            }
        } else if (tcpConnections[opened].state == CLOSING) {
            tcpConnections[opened].state = TIMEWAIT;
            if (tcpConnections[opened].appStatus != APP_NOT_WAITING) {
                appSendClose(tcpConnections[opened]);     // Acknowledge the close.
            }
            return;
        }


        // 7 Process the data.

        if (length != 0) {
            if (!storeIncomingData(tcpConnections[opened], packet, tcpOffset, length)) {
                flags &= ~FIN;   // make sure we do not acknowledge FIN if not all consumed.
            }
//            flags &= ~FIN;   // make sure we do not acknowledge FIN if not all consumed.
            if (1||tcpConnections[opened].ackRequired) {
                pipOutgoingTCP(tcpConnections[opened], 0, ACK);
            } else {
                tcpConnections[opened].ackRequired = 1;
            }
        }

        // 8 Check the FIN bit

        if (flags & FIN) {
            switch(tcpConnections[opened].state) {
            case SYNRCVD:
            case ESTAB:
                tcpConnections[opened].rcvNXT ++;
//                printhexln(tcpConnections[opened].rcvNXT);
                pipOutgoingTCP(tcpConnections[opened], 0, ACK);
                tcpConnections[opened].state = CLOSEWAIT;
                break;
            case FINWAIT1:
                tcpConnections[opened].state = CLOSING;
                break;
            case FINWAIT2:
                tcpConnections[opened].rcvNXT ++;
                pipOutgoingTCP(tcpConnections[opened], 0, ACK);
                tcpConnections[opened].state = TIMEWAIT;
                break;
            }
            if (tcpConnections[opened].appStatus == APP_WRITING ||
                tcpConnections[opened].appStatus == APP_READING) {
                appSendClose(tcpConnections[opened]);
            }
            return;
        }
        break;
    case TIMEWAIT:
    case TIMEWAIT0:
    case TIMEWAIT1:
    case TIMEWAIT2:
        if (flags & FIN) {    // This is a result of a missed packet.
            tcpConnections[opened].rcvNXT++;      // TODO
            pipOutgoingTCP(tcpConnections[opened], 0, ACK);
            tcpConnections[opened].state = TIMEWAIT;
            return;
        }
        error("TIMEWAIT", flags);
        break;
    }
}

static void setAppWaiting(struct tcpConnection &conn, streaming chanend app, int appStatus) {
    int x;
    asm(" or %0, %1, %2" : "=r" (x): "r" (app), "r" (app));
    conn.appWaiting = x;
    conn.appStatus = appStatus;
}

static void initConnection(struct tcpConnection &conn, int state) {
    conn.state = state;
    conn.rx.rd = 0;
    conn.rx.wr = 0;
    conn.rx.free = BUFFERSIZERX;
    conn.rx.length = 0;
    conn.tx.rd = 0;
    conn.tx.wr = 0;
    conn.tx.free = BUFFERSIZETX;
    conn.tx.length = 0;
    conn.rcvWND = BUFFERSIZERX/2;
    conn.sndWND = 0;
#ifdef PIP_DEBUG_TCP
    conn.wtx = 0;
    conn.wrx = 0;
    for(int i = 0; i <PIP_DEBUG_TCP; i++) {
        conn.ltx[i].flags = -1;
        conn.lrx[i].flags = -1;
    }
#endif
}

static void doAccept(struct tcpConnection &conn, streaming chanend app) {
    initConnection(conn, LISTEN);
    setAppWaiting(conn, app, APP_ACCEPTING);
}

unsigned inputShortRev(streaming chanend app) {
    int x;
    app :> x;
    return ((unsigned)byterev(x)) >> 16;
}

static void doConnect(struct tcpConnection &conn, streaming chanend app) {
    timer t;
    int t0;

    initConnection(conn, SYNSENT);
    conn.localIP = myIP;
    app :> conn.remoteIP;
    conn.remotePortRev = inputShortRev(app);
    conn.localPortRev = inputShortRev(app);
    t :> t0;
    conn.sndNXT = t0;
    conn.sndUNA = t0 - 1;
    pipOutgoingTCP(conn, 0, SYN);
    conn.sndNXT++;

    setAppWaiting(conn, app, APP_CONNECTING);
}

static void doClose(struct tcpConnection &conn, streaming chanend app) {
    switch(conn.state) {
    case CLOSED:
        soutct(app, PIP_TCP_ACK_CT); 
        return;
    case SYNSENT:
    case LISTEN:
        conn.state = CLOSED;
        soutct(app, PIP_TCP_ACK_CT); 
        return;
    case SYNRCVD:
    case ESTAB:
        conn.state = FINWAIT1;
        pipOutgoingTCP(conn, 0, FIN|ACK);
        conn.sndNXT++;
        setAppWaiting(conn, app, APP_CLOSING);
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
        pipOutgoingTCP(conn, 0, FIN|ACK); //Diagram only shows FIN but also req ACK
//        conn.sndNXT++;
        setAppWaiting(conn, app, APP_CLOSING);
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
        if (conn.rx.free != BUFFERSIZERX) {
            copyDataForRead(conn, app);
        } else {
            setAppWaiting(conn, app, APP_READING);
        }
        return;
    case CLOSEWAIT:
        soutct(app, PIP_TCP_CLOSED_CT);
        return;
    }
}

static void doWrite(struct tcpConnection &conn, streaming chanend app) {
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
        if (conn.tx.free != 0) {
            copyDataFromWrite(conn, app);
            loadOutgoingData(conn);
        } else {
            setAppWaiting(conn, app, APP_WRITING);
        }
        return;
    case CLOSEWAIT:
        soutct(app, PIP_TCP_CLOSED_CT);
        return;
    }
}

// Application interface to TCP, comes in two parts
// 1) implemented on the Stack side. (1 function)
// 2) implemented on the Application side. (series of functions)

void pipApplicationTCP(streaming chanend app, int cmd) {
    int connectionNumber;
    app :> connectionNumber;
    switch(cmd) {
    case PIP_TCP_ACCEPT:
        doAccept(tcpConnections[connectionNumber], app);
        break;
#ifdef PIP_TCP_CONNECT
    case PIP_TCP_CONNECT_V:
        doConnect(tcpConnections[connectionNumber], app);
        break;
#endif
    case PIP_TCP_CLOSE:
        doClose(tcpConnections[connectionNumber], app);
        break;
    case PIP_TCP_READ:
        doRead(tcpConnections[connectionNumber], app);
        break;
    case PIP_TCP_WRITE:
        doWrite(tcpConnections[connectionNumber], app);
        break;
    }
}


void pipApplicationAccept(streaming chanend stack, unsigned connection) {
    int ack;
    stack <: PIP_TCP_ACCEPT;
    stack <: connection;
    ack = sinct(stack);
}

#ifdef PIP_TCP_CONNECT
void pipApplicationConnect(streaming chanend stack, unsigned connection, int remoteIP, int remotePort, int localPort) {
    int ack;
    stack <: PIP_TCP_CONNECT_V;
    stack <: connection;
    stack <: remoteIP;
    stack <: remotePort;
    stack <: localPort;
    ack = sinct(stack);
}
#endif

void pipApplicationClose(streaming chanend stack, unsigned connection) {
    int ack;
    stack <: PIP_TCP_CLOSE;
    stack <: connection;
    ack = sinct(stack);
}

int pipApplicationRead(streaming chanend stack, unsigned connection,
                       unsigned char buffer[], unsigned maxBytes) {
    int ack, actualBytes;
    stack <: PIP_TCP_READ;
    stack <: connection;
    ack = sinct(stack);
    switch(ack) {
    case PIP_TCP_ACK_CT:
        stack <: maxBytes;
        stack :> actualBytes;
        for(int i = 0; i < actualBytes; i++) {
            stack :> buffer[i];
        }
        return actualBytes;
    case PIP_TCP_ERROR_CT:
        return -1;
    case PIP_TCP_CLOSED_CT:
        return 0;
    }
    printstr("AppRead proto error\n");
    printintln(ack);
    return -1;
}

int pipApplicationWrite(streaming chanend stack, unsigned connection,
                       unsigned char buffer[], unsigned maxBytes) {
    int ack, actualBytes, bytesWritten = 0, start = 0;
    while(maxBytes > 0) {
        stack <: PIP_TCP_WRITE;
        stack <: connection;
        ack = sinct(stack);
        switch(ack) {
        case PIP_TCP_ACK_CT:
            stack <: maxBytes;
            stack :> actualBytes;
            for(int i = 0; i < actualBytes; i++) {
                stack <: buffer[start + i];
            }
            maxBytes -= actualBytes;
            bytesWritten += actualBytes;
            start += actualBytes;
            break;
        case PIP_TCP_ERROR_CT:
            return -1;
        case PIP_TCP_CLOSED_CT:
            return bytesWritten;
        default:
            printstr("AppWrite proto error\n");
            printintln(ack);
            break;
        }
    }
    return bytesWritten;
}


static int byterevShort(int i) {
    return ((unsigned)byterev(i)) >> 16;
}

static void printIP(char str[], unsigned int i) {
    printf("%s %d.%d.%d.%d", str, (i>>24)&0xff, (i>>16)&0xff, (i>>8)&0xff, i&0xff);
}

#ifdef PIP_DEBUG_TCP
static void skipEmptyHistory(struct packetHistory ph[], int &w, int &l) {
    l = PIP_DEBUG_TCP;
    while(l > 0 && ph[w].flags == -1) {
        l--;
        w++;
        if (w == PIP_DEBUG_TCP) {
            w = 0;
        }
    }
}

static void printOne(int isTx, struct packetHistory ph[], int &w, int &l) {
    static unsigned int otime = 0;
    unsigned int ctime = ph[w].time;
    printf("%cx %d: ", isTx ? 't' : 'r', w);
    if (isTx) {
        printf(" Seq: %10u 0x%08x", ph[w].seq, ph[w].seq);
        printf(" Ack: %10u 0x%08x", ph[w].ack, ph[w].ack);
    } else {
        printf(" Ack: %10u 0x%08x", ph[w].ack, ph[w].ack);
        printf(" Seq: %10u 0x%08x", ph[w].seq, ph[w].seq);
    }
    printf(" Flags: %02x", ph[w].flags);
    printf(" Length: %4d", ph[w].length);
    printf(" State: %2d", ph[w].state);
    printf(" Time: %10u (%d us)\n", ctime, (ctime - otime)/100);
    otime = ctime;
    l--;
    w++;
    if (w == PIP_DEBUG_TCP) {
        w = 0;
    }
}

void pipDebugTCPPrint() {
    int wtx, wrx, lentx, lenrx;
    for(int i = 0; i < TCPCONNECTIONS; i++) {
        printf("Connection %d; === State %d ===\n", i, tcpConnections[i].state);
        printf("Ports: local %d remote %d\n", byterevShort(tcpConnections[i].localPortRev),  byterevShort(tcpConnections[i].remotePortRev));
        printIP("IP: local ", tcpConnections[i].localIP);
        printIP(" remote ", tcpConnections[i].remoteIP);
        printf("\nmaaxSS :%d\nsnd.NXT %d (0x%08x)\n", tcpConnections[i].maxSegmentSize,tcpConnections[i].sndNXT, tcpConnections[i].sndNXT);
        printf("snd.UNA %10u (0x%08x)\n", tcpConnections[i].sndUNA, tcpConnections[i].sndUNA);
        printf("rcv.NXT %10u (0x%08x)\n", tcpConnections[i].rcvNXT, tcpConnections[i].rcvNXT);
        printf("rcv.WND %10u (0x%08x)\n", tcpConnections[i].rcvWND, tcpConnections[i].rcvWND);
        printf("snd.WND %10u (0x%08x)\n", tcpConnections[i].sndWND, tcpConnections[i].sndWND);
        printf("appWaiting: 0x%08x\n",  tcpConnections[i].appWaiting);
        printf("appStatus: %d\n",  tcpConnections[i].appStatus);
        printf("ackRequired: %d\n",  tcpConnections[i].ackRequired);
        printf("timeouts: %d\nPacket history of last %d RX and TX transactions:\n",  tcpConnections[i].timeouts, PIP_DEBUG_TCP);
        wtx = tcpConnections[i].wtx;
        wrx = tcpConnections[i].wrx;
        skipEmptyHistory(tcpConnections[i].ltx, wtx, lentx);
        skipEmptyHistory(tcpConnections[i].lrx, wrx, lenrx);
        while(lentx != 0 || lenrx != 0) {
            if (lenrx == 0) {
                printOne(1, tcpConnections[i].ltx, wtx, lentx);
            } else if (lentx == 0) {
                printOne(0, tcpConnections[i].lrx, wrx, lenrx);
            } else if (((int)(tcpConnections[i].ltx[wtx].time - tcpConnections[i].lrx[wrx].time)) > 0) {
                printOne(0, tcpConnections[i].lrx, wrx, lenrx);
            } else {
                printOne(1, tcpConnections[i].ltx, wtx, lentx);
            }
        }
   }
}
#endif
