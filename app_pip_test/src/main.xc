// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>
#include <print.h>
#include "miiDriver.h"
#include "mii.h"
#include "getmac.h"
//#include "xscope.h"
#include "pipServer.h"
#include "tcpApplication.h"

#define ETHCORE 0

#define PORT_ETH_FAKE    XS1_PORT_8A

#define PORT_ETH_RST_N  PORT_SHARED_RS

on stdcore[ETHCORE]: mii_interface_t mii =
  {
    XS1_CLKBLK_1,
    XS1_CLKBLK_2,

    PORT_ETH_RXCLK,
    PORT_ETH_RXER,
    PORT_ETH_RXD,
    PORT_ETH_RXDV,

    PORT_ETH_TXCLK,
    PORT_ETH_TXEN,
    PORT_ETH_TXD,

    PORT_ETH_FAKE,
  };

on stdcore[ETHCORE]: out port p_mii_resetn = PORT_ETH_RST_N;
on stdcore[ETHCORE]: smi_interface_t smi = { 0, PORT_ETH_MDIO, PORT_ETH_MDC };

on stdcore[ETHCORE]: clock clk_smi = XS1_CLKBLK_5;

on stdcore[ETHCORE]: struct otp_ports p = {XS1_PORT_32B, XS1_PORT_16C, XS1_PORT_16D };

static void httpServer(streaming chanend tcpStack) {
    unsigned char buf[12];
    int l, total;
    timer t;
    int t0, t1;
    while(1) {
        total = 0;
        pipApplicationAccept(tcpStack, 0);
        t :> t0;
        while(l = pipApplicationRead(tcpStack, 0, buf, 10)) {
            total += l;
            buf[l] = '.';
            buf[l+1] = 0;
            printstr(buf);
            if (l & 1) break;
            pipApplicationWrite(tcpStack, 0, buf, l);
        t :> t0;
            for(int i = 0; i < 1 + 0 * 1000; i++) {
                pipApplicationWrite(tcpStack, 0, "==========\r\n=        =\r\n=        =\r\n=        =\r\n==========\r\n", 60);
//                pipApplicationWrite(tcpStack, 0, "0123456789abcdefg\r\n", 19);
            }
        t :> t1;
                printstr("Sent 60000 bytes in ");
                printint((t1-t0)/100);
                printstr(" us");
#if 0
            if (total > 1000) {
                t :> t1;
                printstr("Received ");
                printint(total);
                printstr(" bytes in ");
                printint((t1-t0)/100);
                printstr(" us");
            }
#endif
        }
        pipApplicationClose(tcpStack, 0);
    }
}

int main(void) {
	streaming chan tcpApps;

	par
	{
	 	on stdcore[ETHCORE]: {
	 		ethernet_getmac_otp(p, myMacAddress);
            p_mii_resetn <: 2;            
	 		pipServer(clk_smi, p_mii_resetn, smi, mii, tcpApps);
	 	}

	 	on stdcore[ETHCORE]: httpServer(tcpApps);
    }
    return 0;
}
