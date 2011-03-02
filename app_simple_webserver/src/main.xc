// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>
#include "uip_server.h"
#include "xhttpd.h"
#include "getmac.h"
#include "ethernet_server.h"

// Ethernet Ports
on stdcore[2]: port otp_data = XS1_PORT_32B; // OTP_DATA_PORT
on stdcore[2]: out port otp_addr = XS1_PORT_16C; // OTP_ADDR_PORT
on stdcore[2]: port otp_ctrl = XS1_PORT_16D; // OTP_CTRL_PORT


on stdcore[2]: clock clk_smi = XS1_CLKBLK_5;

on stdcore[2]: mii_interface_t mii =
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

#ifdef PORT_ETH_RST_N
on stdcore[2]: out port p_mii_resetn = PORT_ETH_RST_N;
on stdcore[2]: smi_interface_t smi = {PORT_ETH_MDIO, PORT_ETH_MDC, 0};
#else
on stdcore[2]: smi_interface_t smi = {PORT_ETH_RST_N_MDIO, PORT_ETH_MDC, 1};
#endif

// IP Config - change this to suit your network.  Leave with all
// 0 values to use DHCP
xtcp_ipconfig_t ipconfig = {
		{ 0, 0, 0, 0 }, // ip address (eg 192,168,0,2)
		{ 0, 0, 0, 0 }, // netmask (eg 255,255,255,0)
		{ 0, 0, 0, 0 } // gateway (eg 192,168,0,1)
};

// Program entry point
int main(void) {
	chan mac_rx[1], mac_tx[1], xtcp[1], connect_status;

	par
	{
		// The ethernet server
		on stdcore[2]:
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
		on stdcore[3]: uip_server(mac_rx[0], mac_tx[0],
				xtcp, 1, ipconfig,
				connect_status);

		// The webserver thread
		on stdcore[0]: xhttpd(xtcp[0]);

	}
	return 0;
}
