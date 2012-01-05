// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>


#include <xs1.h>
#include <xclib.h>
#include <print.h>
#include "miiDriver.h"
#include "mii.h"
#include "miiClient.h"
#include "pipServer.h"
#include "tx.h"
#include "ethernet.h"
#include "dhcp.h"
#include "timer.h"

extern char notifySeen;

short txbuf[400];
static int txbufLength = 0;

static void theServer(chanend cIn, chanend cOut, chanend cNotifications, chanend appIn, chanend appOut) {
    int havePacket = 0;
    int nBytes, a, timeStamp;
    int b[3200];
    timer t, t2;
    int thetime;

    miiBufferInit(cIn, cNotifications, b, 3200);
    miiOutInit(cOut);

    t2 :> thetime;

    pipDhcpInit();
    if (txbufLength != 0) {
        miiOutPacket(cOut, (txbuf, int[]), 0, txbufLength);
        miiOutPacketDone(cOut);
        txbufLength = 0;
    }
    while (1) {
        select {
        case pipTimeOut(t);
        case inuchar_byref(cNotifications, notifySeen):
            break;
        }
        if (!havePacket) {
            {a,nBytes,timeStamp} = miiGetInBuffer();
            if (a != 0) {
                pipIncomingEthernetC(a);//TODO, nBytes);
                miiFreeInBuffer(a);
                miiRestartBuffer();
            }
        }
        if (txbufLength != 0) {
            if (txbufLength < 64) {
                txbufLength = 64;
            }
            miiOutPacket(cOut, (txbuf, int[]), 0, txbufLength);
            miiOutPacketDone(cOut);
            txbufLength = 0;
        }
    } 
}

void txInt(int offset, int x) {
    txbuf[offset] = x;
    txbuf[offset+1] = x >> 16;
    offset = offset * 2 + 4;
    if (offset > txbufLength) txbufLength = offset;
}

void txShort(int offset, int x) {
    txbuf[offset] = x;
    offset = offset * 2 + 2;
    if (offset > txbufLength) txbufLength = offset;
}

void txByte(int offset, int x) {
    (txbuf, unsigned char[])[offset] = x;
    offset = offset + 1;
    if (offset > txbufLength) txbufLength = offset;
}

void txData(int offset, char data[], int dataOffset, int n) {
    for(int i = 0; i < n; i++) {
        (txbuf, unsigned char[])[i+offset*2] = data[i+dataOffset];
    }
    offset = offset * 2 + n;
    if (offset > txbufLength) txbufLength = offset;
}

void txShortZeroes(int offset, int len) {
    for(int i = 0; i < len; i++) {
        txbuf[offset+i] = 0;
    }
    offset = (offset + len) * 2;
    if (offset > txbufLength) txbufLength = offset;
}

void txClear() {
    txbufLength = 0;
}

void txPrint() {
    for(int i = 0; i < txbufLength; i++) {
        printhex((txbuf, unsigned char[])[i]);
        printstr(" ");
        if ((i & 15) == 15) {
            printstr("\n");
        }
    }
    printstr("\n");
}

unsigned shortrev(unsigned x) {
    return ((unsigned)byterev(x))>>16;
}

void pipServer(clock clk_smi,
               out port ?p_mii_resetn,
               smi_interface_t &smi,
               mii_interface_t &m,
               chanend appIn, chanend appOut, chanend server) {
    chan cIn, cOut;
    chan notifications;
    par {
        miiDriver(clk_smi, p_mii_resetn, smi, m, cIn, cOut, 0);
        theServer(cIn, cOut, notifications, appIn, appOut);
    }
}
