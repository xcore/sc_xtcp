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

int timeOut[10];
int timeOuts = 0;

static void theServer(chanend cIn, chanend cOut, chanend cNotifications, chanend appIn, chanend appOut) {
    int havePacket = 0;
    int outBytes;
    int nBytes, a, timeStamp;
    int b[3200];
    int txbuf[400];
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
        case appOut :> outBytes:
            for(int i = 0; i < ((outBytes + 3) >>2); i++) {
                appOut :> txbuf[i];
            }
            if(outBytes < 64) {
                printstr("ERR ");
                printhexln(outBytes);
            }
            miiOutPacket(cOut, txbuf, 0, outBytes);
            miiOutPacketDone(cOut);
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
    } 
}

void miiSingleServer(clock clk_smi,
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
