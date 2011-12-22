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

extern char notifySeen;

static int timeOut[10];
static int timeOuts = 0;

static short txbuf[400];
static int txbufLength = 0;

static void theServer(chanend cIn, chanend cOut, chanend cNotifications, chanend appIn, chanend appOut) {
    int havePacket = 0;
    int outBytes;
    int nBytes, a, timeStamp;
    int b[3200];
    timer t;

    miiBufferInit(cIn, cNotifications, b, 3200);
    miiOutInit(cOut);
    
    while (1) {
        select {
        case inuchar_byref(cNotifications, notifySeen):
            break;
        case timeOuts != 0 => t when timerafter(timeOut[0]):
            timeOuts--;
            for(int i = 0; i < timeOuts; i++) {
                timeOut[i] = timeOut[i+1];
            }
            pipTimeOut();
            break;
        }
        if (!havePacket) {
            {a,nBytes,timeStamp} = miiGetInBuffer();
            if (a != 0) {
                pipIncomingEthernet(a, nBytes);
                miiFreeInBuffer(a);
                miiRestartBuffer();
            }
        }
        if (txbufLength != 0) {
            miiOutPacket(cOut, (txbuf, unsigned int[]), 0, txbufLength);
            miiOutPacketDone(cOut);
            txbufLength = 0;
        }
    } 
}

void txInt(int offset, int x) {
    txbuf[offset] = x >> 16;
    txbuf[offset+1] = x >> 16;
    offset = offset * 2 + 4;
    if (offset > txbufLength) txbufLength = offset;
}

void txShort(int offset, int short) {
    txbuf[offset] = x;
    offset = offset * 2 + 2;
    if (offset > txbufLength) txbufLength = offset;
}

void txData(int offset, char data[], int dataOffset, int n) {
    for(int i = 0; i < n; i++) {
        (txbuf, unsigned char[])[i+offset*2] = data[i+dataOffset];
    }
    offset = offset * 2 + n;
    if (offset > txbufLength) txbufLength = offset;
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
