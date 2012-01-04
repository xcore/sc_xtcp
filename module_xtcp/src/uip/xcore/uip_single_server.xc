// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifdef UIP_USE_SINGLE_THREADED_ETHERNET

#include <xs1.h>
#include <xclib.h>
#include <safestring.h>

#include "uip_single_server.h"
#include "miiDriver.h"
#include "miiClient.h"

extern char notifySeen;

extern int uip_static_ip;
extern xtcp_ipconfig_t uip_static_ipconfig;

extern void uip_single_server_init(chanend xtcp[], int num_xtcp, xtcp_ipconfig_t& ipconfig, unsigned char mac_address[6]);

// This is the buffer where TCP constructs its packets
unsigned int uip_buf32[(UIP_BUFSIZE + 5) >> 2];
u8_t *uip_buf = (u8_t *) &uip_buf32[0];

static void theServer(chanend cIn, chanend cOut, chanend cNotifications,
		chanend tcp[], int num_xtcp_clients, xtcp_ipconfig_t& ipconfig,
		char mac_address[6]) {
    //int havePacket = 0;
    //int outBytes;
    //int nBytes, a, timeStamp;
    int b[3200];
    //int txbuf[400];

    miiBufferInit(cIn, cNotifications, b, 3200);
    miiOutInit(cOut);

    while (1) {
//        select {
//        case inuchar_byref(cNotifications, notifySeen):
//            break;
//
//        case havePacket => appIn :> int _:
//            for(int i = 0; i < ((nBytes + 3) >>2); i++) {
//                int val;
//                asm("ldw %0, %1[%2]" : "=r" (val) : "r" (a) , "r" (i));
//                appIn <: val;
//            }
//            miiFreeInBuffer(a);
//            miiRestartBuffer();
//            {a,nBytes,timeStamp} = miiGetInBuffer();
//            if (a == 0) {
//                havePacket = 0;
//            } else {
//                outuint(appIn, nBytes);
//            }
//            break;
//
//        case appOut :> outBytes:
//            for(int i = 0; i < ((outBytes + 3) >>2); i++) {
//                appOut :> txbuf[i];
//            }
//            miiOutPacket(cOut, txbuf, 0, outBytes);
//            miiOutPacketDone(cOut);
//            break;
//        }
//
//        if (!havePacket) {
//            {a,nBytes,timeStamp} = miiGetInBuffer();
//            if (a != 0) {
//                havePacket = 1;
//                outuint(appIn, nBytes);
//            }
//        }
    }
}

void uipSingleServer(clock clk_smi,
                     out port ?p_mii_resetn,
                     smi_interface_t &smi,
                     mii_interface_t &mii,
                     chanend xtcp[], int num_xtcp,
                     xtcp_ipconfig_t& ipconfig,
                     char mac_address[6]) {
    chan cIn, cOut;
    chan notifications;
    par {
        miiDriver(clk_smi, p_mii_resetn, smi, mii, cIn, cOut, 0);
        theServer(cIn, cOut, notifications, xtcp, num_xtcp, ipconfig, mac_address);
    }
}

#endif


