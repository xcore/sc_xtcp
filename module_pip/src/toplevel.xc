// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>


#include <xs1.h>
#include <xclib.h>
#include <print.h>
#include <xscope.h>
#include "miiDriver.h"
#include "mii.h"
#include "miiClient.h"
#include "pipServer.h"
#include "tx.h"
#include "ethernet.h"
#include "dhcp.h"
#include "timer.h"
#include "tcp.h"
#include "arp.h"

extern char notifySeen;


static void theServer(chanend cIn, chanend cOut, chanend cNotifications, streaming chanend tcpApps) {
    int havePacket = 0;
    int nBytes, a, timeStamp;
    int b[3200];
    timer t, t2;
    int thetime;
    struct miiData miiData;

    xscope_register(1, XSCOPE_DISCRETE, "n", XSCOPE_UINT, "i");
    xscope_config_io(XSCOPE_IO_BASIC);
    printstr("HELLO\n");

    miiBufferInit(miiData, cIn, cNotifications, b, 3200);
    miiOutInit(cOut);

    t2 :> thetime;

#ifdef PIP_ARP
    pipInitARP();
#endif

#ifdef PIP_TCP
    pipInitTCP();
#endif

#ifdef PIP_DHCP
    pipInitDHCP();
    doTx(cOut);
#endif

    while (1) {
        int cmd;
        select {
#if 1
        case pipTimeOut(t);
#endif
        case inuchar_byref(cNotifications, miiData.notifySeen):
            break;
#ifdef PIP_TCP
        case tcpApps :> cmd:
            pipApplicationTCP(tcpApps,cmd);
            break;
#endif
        }
        if (!havePacket) {
            {a,nBytes,timeStamp} = miiGetInBuffer(miiData);
            if (a != 0) {
                pipIncomingEthernetC(a);//TODO, nBytes);
                miiFreeInBuffer(miiData, a);
                miiRestartBuffer(miiData);
            }
        }
        doTx(cOut);
    } 
}


void pipServer(clock clk_smi,
               out port ?p_mii_resetn,
               smi_interface_t &smi,
               mii_interface_t &m,
               streaming chanend tcpApps) {
    chan cIn, cOut;
    chan notifications;
    par {
        {
            miiInitialise(clk_smi, p_mii_resetn, smi, m);
            miiDriver(m, cIn, cOut);
        }
        theServer(cIn, cOut, notifications, tcpApps);
    }
}
