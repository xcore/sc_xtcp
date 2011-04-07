// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>
#include "uip_server.h"
#include "demo_protocol.h"
#include "getmac.h"
#include "ethernet_server.h"

#ifndef ETHERNET_CORE
#define ETHERNET_CORE 2
#endif

// Ethernet Ports
on stdcore[ETHERNET_CORE]: port otp_data = XS1_PORT_32B; 		// OTP_DATA_PORT
on stdcore[ETHERNET_CORE]: out port otp_addr = XS1_PORT_16C;	// OTP_ADDR_PORT
on stdcore[ETHERNET_CORE]: port otp_ctrl = XS1_PORT_16D;		// OTP_CTRL_PORT


on stdcore[ETHERNET_CORE]: clock clk_smi = XS1_CLKBLK_5;

on stdcore[ETHERNET_CORE]: mii_interface_t mii =
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
  };


#ifdef PORT_ETH_RSTN
#define PORT_ETH_RST_N PORT_ETH_RSTN
#endif

#ifdef PORT_ETH_RST_N
on stdcore[ETHERNET_CORE]: out port p_mii_resetn = PORT_ETH_RST_N;
on stdcore[ETHERNET_CORE]: smi_interface_t smi = { PORT_ETH_MDIO, PORT_ETH_MDC, 0 };
#else
on stdcore[ETHERNET_CORE]: smi_interface_t smi = { PORT_ETH_RST_N_MDIO, PORT_ETH_MDC, 1 };
#endif

#if 1
// Static IP Config - change this to suit your network (current is link local address)
xtcp_ipconfig_t ipconfig =
{
  {169,254,3,2},   // ip address
  {255,255,0,0},   // netmask
  {0,0,0,0}        // gateway
};
#else
// Use dynamic addressing DHCP
xtcp_ipconfig_t ipconfig = {{0,0,0,0},{0,0,0,0},{0,0,0,0}};
#endif

// Program entry point
int main(void)
{
	chan mac_rx[1], mac_tx[1], xtcp[1], connect_status;

	par
	{
        // The ethernet server
		on stdcore[ETHERNET_CORE]:
		{
                  int mac_address[2];

                  ethernet_getmac_otp(otp_data, otp_addr, otp_ctrl,
                                      (mac_address, char[]));


                  phy_init(clk_smi,
#ifdef PORT_ETH_RST_N
                           p_mii_resetn,
#else
                           null,
#endif
                           smi, mii);

                  ethernet_server(mii, mac_address,
                                  mac_rx, 1, mac_tx, 1, smi,
                                  connect_status);
		}

		// The TCP/IP server thread
		on stdcore[0]: uip_server(mac_rx[0], mac_tx[0],
                                          xtcp, 1, ipconfig,
                                          connect_status);

		// The server thread
		on stdcore[0]: demo_protocold(xtcp[0]);


	}
        return 0;
}
