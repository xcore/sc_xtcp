// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>

#include <print.h>
#include <xscope.h>

#include "getmac.h"
#include "uip_single_server.h"

#define PORT_ETH_FAKE    XS1_PORT_8C

on stdcore[0]: port otp_data = XS1_PORT_32B; 		// OTP_DATA_PORT
on stdcore[0]: out port otp_addr = XS1_PORT_16C;	// OTP_ADDR_PORT
on stdcore[0]: port otp_ctrl = XS1_PORT_16D;		// OTP_CTRL_PORT

on stdcore[0]: mii_interface_t mii =
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

on stdcore[0]: port p_reset = PORT_SHARED_RS;
on stdcore[0]: smi_interface_t smi = { PORT_ETH_MDIO, PORT_ETH_MDC, 0 };
on stdcore[0]: clock clk_smi = XS1_CLKBLK_5;


xtcp_ipconfig_t ipconfig = {
#if 0
		{ 192, 168, 0, 10 }, // ip address (eg 192,168,0,2)
		{ 255, 255, 255, 0 }, // netmask (eg 255,255,255,0)
		{ 0, 0, 0, 0 } // gateway (eg 192,168,0,1)
#else
		{ 0,0,0,0 },
		{ 0,0,0,0 },
		{ 0,0,0,0 }
#endif
};

#define RX_BUFFER_SIZE 1200
#define INCOMING_PORT 100

void udp_server(chanend c_xtcp)
{
  xtcp_connection_t conn;

  char rx_buffer[RX_BUFFER_SIZE];

  int response_len;

  // Instruct server to listen and create new connections on the incoming port
  xtcp_listen(c_xtcp, INCOMING_PORT, XTCP_PROTOCOL_UDP);

  while (1) {
	select {

	// Respond to an event from the tcp server
	case xtcp_event(c_xtcp, conn):
	  switch (conn.event)
		{
		case XTCP_IFUP:
		case XTCP_IFDOWN:
		  break;

		case XTCP_NEW_CONNECTION:
			// This is a new connection to the listening port
//			printstr("New connection to listening port: ");
//			printintln(conn.local_port);
		  break;
		case XTCP_RECV_DATA:
		  response_len = xtcp_recv_count(c_xtcp, rx_buffer, RX_BUFFER_SIZE);
		  printuintln((rx_buffer,unsigned[])[0]);
		  xscope_probe_data(0, (rx_buffer,unsigned[])[0]);
		  break;
	  case XTCP_REQUEST_DATA:
	  case XTCP_RESEND_DATA:
	  case XTCP_SENT_DATA:
	  case XTCP_TIMED_OUT:
	  case XTCP_ABORTED:
	  case XTCP_CLOSED:
	  case XTCP_ALREADY_HANDLED:
		  break;
	  }
	  break;
	}
  }
}



int main(void) {
	chan xtcp[1];

	par
	{
	 	on stdcore[0]: {
            char mac_address[6];

            xscope_register(1, XSCOPE_DISCRETE, "n", XSCOPE_UINT, "i");
            xscope_config_io(XSCOPE_IO_BASIC);

            ethernet_getmac_otp(otp_data, otp_addr, otp_ctrl, mac_address);

	 		// Bring PHY out of reset
	 		p_reset <: 0x2;

	 		// Start server
	 		uipSingleServer(clk_smi, null, smi, mii, xtcp, 1, ipconfig, mac_address);
	 	}

		on stdcore[0]: udp_server(xtcp[0]);
	}
	return 0;
}
