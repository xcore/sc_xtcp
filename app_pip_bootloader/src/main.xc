// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>
#include <print.h>
#include "miiDriver.h"
#include "mii.h"
#include "pipServer.h"
#include "tcpApplication.h"

#define PORT_ETH_FAKE    XS1_PORT_8C

#define PORT_ETH_RST_N  XS1_PORT_4C

mii_interface_t mii =
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

#ifdef PORT_ETH_RST_N
out port p_mii_resetn = PORT_ETH_RST_N;
smi_interface_t smi = { 0x1F, PORT_ETH_MDIO, PORT_ETH_MDC };
#else
smi_interface_t smi = { 0x1F, PORT_ETH_RST_N_MDIO, PORT_ETH_MDC };
#endif

clock clk_smi = XS1_CLKBLK_5;

int main(void) {
	streaming chan tcpApps[1];
	streaming chan udpApps[1];

    pipServer(clk_smi, p_mii_resetn, smi, mii, tcpApps, udpApps);
    return 0;
}
