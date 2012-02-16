// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>


#include <xs1.h>
#include <xclib.h>
#include <print.h>
#include "config.h"
#include "miiDriver.h"
#include "mii.h"
#include "miiClient.h"
#include "tx.h"

short txbuf[400];
static int txbufLength = 0;

void doTx(chanend cOut) {
    if (txbufLength != 0) {
        if (txbufLength < 64) {
            txbufLength = 64;
        }
        miiOutPacket(cOut, (txbuf, int[]), 0, txbufLength);
        miiOutPacketDone(cOut);
        txbufLength = 0;
    }
}

void adjustBufLength(int offset) {
    if (offset > txbufLength) {
        txbufLength = offset;
    }
}

void txInt(int offset, int x) {
    txbuf[offset] = x;
    txbuf[offset+1] = x >> 16;
    adjustBufLength(offset * 2 + 4);
}

void txShort(int offset, int x) {
    txbuf[offset] = x;
    adjustBufLength(offset * 2 + 2);
}

void txShortRev(int offset, int x) {
    txShort(offset, ((unsigned)byterev(x))>>16);
}

void txByte(int offset, int x) {
    (txbuf, unsigned char[])[offset] = x;
    adjustBufLength(offset + 1);
}

void txData(int offset, char data[], int dataOffset, int n) {
    for(int i = 0; i < n; i++) {
        (txbuf, unsigned char[])[i+offset*2] = data[i+dataOffset];
    }
    adjustBufLength(offset * 2 + n);
}

void txShortZeroes(int offset, int len) {
    for(int i = 0; i < len; i++) {
        txbuf[offset+i] = 0;
    }
    adjustBufLength((offset + len) * 2);
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

