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
#include "pipServer.h"
#include "tx.h"
#include "ethernet.h"
#include "dhcp.h"
#include "timer.h"
#include "tcp.h"
#include "udp.h"
#include "arp.h"
#include "icmp.h"
#include "linklocal.h"

extern char notifySeen;

int thetime;

select  doPing(timer t2) {
case t2 when timerafter(thetime+100000000) :> thetime:
//    pipOutgoingICMPPing(0xc0a82101, thetime/100000000);
    pipOutgoingICMPPing(0x0A006618, thetime/100000000);
    break;
}

// TODO: THESE SHOULD NOT BE HERE
extern int epoch, timeOutValue, waitingForEpoch;
#define EPOCH_BIT    30
extern void numberZeroTimedOut();
extern void setTimeOutValue();


static void theServer(chanend cIn, chanend cOut, chanend cNotifications,
                      streaming chanend tcpApps[],
                      streaming chanend udpApps[]) {
    int havePacket = 0;
    int nBytes, a, timeStamp;
    int b[3200];
    timer t, t2;
    struct miiData miiData;

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
    printstr("Dhcp...\n");
    doTx(cOut);
#else
#ifdef PIP_LINK_LOCAL
    pipInitLinkLocal();
#endif
#endif

    while (1) {
        int cmd;
        select {
//        case doPing(t2);
#if COMPILER_REPAIRED
        case pipTimeOut(t);
#else
        case t when timerafter(timeOutValue) :> unsigned now:
            if (!waitingForEpoch) {
                numberZeroTimedOut();
            }
            if ((epoch & ((1<<(32-EPOCH_BIT))-1)) != (now >> EPOCH_BIT)) {
                epoch++;
            }
            setTimeOutValue();
            break;
#endif

        case inuchar_byref(cNotifications, miiData.notifySeen):
            break;
#if PIP_UDP_CHANNELS != 0
        case (int i = 0; i < PIP_UDP_CHANNELS; i++)
            udpApps[i] :> cmd:
            pipApplicationUDP(udpApps[i],cmd,cOut);
            break;
#endif
#ifdef PIP_TCP
        case (int i = 0; i < PIP_TCP_CHANNELS; i++)
            tcpApps[i] :> cmd:
            pipApplicationTCP(tcpApps[i],cmd);
            break;
        pipOutgoingTCPReady => default:
            pipOutgoingPrepareTCP();
            doTx(cOut);
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
               streaming chanend tcpApps[],
               streaming chanend udpApps[]) {
    chan cIn, cOut;
    chan notifications;
    par {
        {
            miiInitialise(clk_smi, p_mii_resetn, smi, m);
            miiDriver(m, cIn, cOut);
        }
        theServer(cIn, cOut, notifications, tcpApps, udpApps);
    }
}
