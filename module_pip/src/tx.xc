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
