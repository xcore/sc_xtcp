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
#include "tcp.h"
#include "arp.h"

extern char notifySeen;

static void theServer(chanend cIn, chanend cOut, chanend cNotifications, streaming chanend tcpApps) {
    int havePacket = 0;
    int nBytes, a, timeStamp;
    int b[3200];
    timer t, t2;
    int thetime;

    miiBufferInit(cIn, cNotifications, b, 3200);
    miiOutInit(cOut);

    t2 :> thetime;

#ifdef PIP_ARP
    pipInitARP();
#endif

#ifdef PIP_TCP
    pipInitTCP();
#endif

#ifdef PIP_DHCP
    pipDhcpInit();
    doTx(cOut);
#endif

    while (1) {
        int cmd;
        select {
        case pipTimeOut(t);
        case inuchar_byref(cNotifications, notifySeen):
            break;
#ifdef PIP_TCP
        case tcpApps :> cmd:
            pipApplicationTCP(tcpApps,cmd);
            break;
#endif
        }
        if (!havePacket) {
            {a,nBytes,timeStamp} = miiGetInBuffer();
            if (a != 0) {
                pipIncomingEthernetC(a);//TODO, nBytes);
                miiFreeInBuffer(a);
                miiRestartBuffer();
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
        miiDriver(clk_smi, p_mii_resetn, smi, m, cIn, cOut, 0);
        theServer(cIn, cOut, notifications, tcpApps);
    }
}
