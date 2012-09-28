// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>
#include <print.h>
#include <stdio.h>
#include "smi.h"
#include "mii_driver.h"
#include "mii.h"
#include "xscope.h"
#include "pipServer.h"
#include "tcpApplication.h"
#include "udpApplication.h"
#include "pip_conf.h"
#include "otp_board_info.h"

#define ETHCORE 1

#define PORT_ETH_FAKE    XS1_PORT_8C

#define PORT_ETH_RST_N  PORT_CORE1_SHARED

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

on stdcore[ETHCORE]: otp_ports_t otp_ports = OTP_PORTS_INITIALIZER;

static void httpServer(streaming chanend tcpStack) {
    unsigned char buf[12];
    int l, total;
    timer t;
    int t0, t1;
    while(1) {
        total = 0;
        pipApplicationAccept(tcpStack, 0);
        t :> t0;
        while((l = pipApplicationRead(tcpStack, 0, buf, 10)) > 0) {
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

#define CLIENT_SOCKET 1

static void httpClient(streaming chanend tcpStack) {
    unsigned char buf[102];
    int l, total;
    timer t;
    int t0;
    int portnr = 22449;
    t :> t0;
    t when timerafter(t0+1000000000) :> t0;
    printstr("Starting HTTP nuke\n");
    for(int i = 0; i < 100000; i++) {
        portnr = (portnr + 1) & 0xF7FF;
        total = 0;
        t :> t0;
        t when timerafter(t0+500000) :> t0;
        l = pipApplicationConnect(tcpStack, CLIENT_SOCKET, 0xC0A82101, 80, portnr);
        if (!l) {
            printstr("Connect returned ");
            printintln(l);
            continue;
        }
        t :> t0;
        if ((l=pipApplicationWrite(tcpStack, CLIENT_SOCKET, "GET /\r\n\r\n", 9)) != 9) {
            printstr("AppWrite returned ");
            printintln(l);
            pipApplicationClose(tcpStack, CLIENT_SOCKET);
            continue;
        }
        while((l = pipApplicationRead(tcpStack, CLIENT_SOCKET, buf, 100)) > 0) {
            total += l;
            buf[l] = '.';
            buf[l+1] = 0;
//            printstr(buf);
        }
        pipApplicationClose(tcpStack, CLIENT_SOCKET);
        if ((i & 63) == 0) printstr(".");
    }
}

static void cupsServer(streaming chanend c) {
    while(1) {
        char buffer[1024];
        unsigned rIP, rPort;
        int l = pipApplicationReadUDP(c, buffer, 0, 1023, rIP, rPort, 631);
        buffer[l] = 0;
        printf("Got CUPS packet len %d: <%s>\n", l, buffer);
        pipApplicationWriteUDP(c, buffer, 0, 100, rIP, 56789, 631);
    }
}

void xscope_user_init() {
    if (get_core_id() == 0) {
        xscope_register(1, XSCOPE_DISCRETE, "n", XSCOPE_UINT, "i");
    }
    xscope_config_io(XSCOPE_IO_BASIC);
}

int main(void) {
	streaming chan tcpApps[PIP_TCP_CHANNELS];
	streaming chan udpApps[1];

	par
	{
	 	on stdcore[ETHCORE]: {
            otp_board_info_get_mac(otp_ports, 0, myMacAddress);
                        p_mii_resetn <: 2;
	 		pipServer(clk_smi, p_mii_resetn, smi, mii, tcpApps, udpApps);
	 	}

	 	on stdcore[ETHCORE]: httpClient(tcpApps[0]);

	 	on stdcore[0]: cupsServer(udpApps[0]);
    }
    return 0;
}
