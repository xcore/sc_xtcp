// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>

#include <print.h>
#include <xscope.h>

#include "smi.h"
#include "getmac.h"
#include "uip_server.h"
#include "miiSingleServer.h"

#define PORT_ETH_FAKE    XS1_PORT_8C

#define PORT_ETH_RST_N_MDIO  XS1_PORT_1P

on stdcore[0]: struct otp_ports p = { XS1_PORT_32B, XS1_PORT_16C, XS1_PORT_16D };

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
on stdcore[0]: smi_interface_t smi = { 0, PORT_ETH_MDIO, PORT_ETH_MDC };
on stdcore[0]: clock clk_smi = XS1_CLKBLK_5;


xtcp_ipconfig_t ipconfig = {
#if 1
		{ 192, 168, 0, 10 }, // ip address (eg 192,168,0,2)
		{ 255, 255, 255, 0 }, // netmask (eg 255,255,255,0)
		{ 0, 0, 0, 0 } // gateway (eg 192,168,0,1)
#else
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 }
#endif
};

#define RX_BUFFER_SIZE 1200
#define INCOMING_PORT_CONTINUOUS 100
#define INCOMING_PORT_SINGLE 101
#define INCOMING_PORT_TCP 102


/*
 *  This thread implements three 'server' like services.
 *
 *  On UDP 100 there is a listening port that does not close any connections
 *  that are opened by remote machines.  One of the test scripts opens a single
 *  socket to this port, and streams data to it.  This mimics a continuously
 *  open type UDP connection, for instance a UDP media stream, where the close
 *  action is as the result of a higher layer connection management action.
 *
 *  On UDP 101 there is a listening port that closes the connection every time
 *  a piece of data is received.  One test script repeatedly opens a socket,
 *  sends a single piece of data, and then closes the socket.  This mimics a
 *  discovery type protocol, where units are sending single packet 'here i am'
 *  messages to each other.
 *
 *  On TCP 100, there is a listening socket which does not close the connection.
 *  The test script opens the connection, streams data into the port, then closes
 *  it.  This mimics a long term data sink, such as a data logger, or TCP based
 *  media renderer.
 */
void udp_server(chanend c_xtcp)
{
  xtcp_connection_t conn;
  unsigned total_len=0;

  char rx_buffer[RX_BUFFER_SIZE];

  int response_len;

  xtcp_listen(c_xtcp, INCOMING_PORT_CONTINUOUS, XTCP_PROTOCOL_UDP);
  xtcp_listen(c_xtcp, INCOMING_PORT_SINGLE, XTCP_PROTOCOL_UDP);
  xtcp_listen(c_xtcp, INCOMING_PORT_TCP, XTCP_PROTOCOL_TCP);

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
		  break;
		case XTCP_RECV_DATA:
		  response_len = xtcp_recv_count(c_xtcp, rx_buffer, RX_BUFFER_SIZE);
		  switch (conn.local_port) {
		  case INCOMING_PORT_SINGLE:
			  xscope_probe_data(0, (rx_buffer,unsigned[])[0]);
			  xtcp_close(c_xtcp, conn);
			  break;
		  case INCOMING_PORT_CONTINUOUS:
			  xscope_probe_data(0, (rx_buffer,unsigned[])[0]);
			  break;
		  case INCOMING_PORT_TCP:
			  total_len += response_len;
			  xscope_probe_data(0, total_len);
			  break;
		  }
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
	chan mac_rx[1], mac_tx[1], xtcp[1], connect_status;

	par
	{
	 	on stdcore[0]: {
            char mac_address[6];

            xscope_register(1, XSCOPE_DISCRETE, "n", XSCOPE_UINT, "i");
            xscope_config_io(XSCOPE_IO_BASIC);

            ethernet_getmac_otp(p, mac_address);

	 		// Bring PHY out of reset
	 		p_reset <: 0x2;

	 		// Start server
	 		miiSingleServer(clk_smi, null, smi, mii, mac_rx[0], mac_tx[0], connect_status, mac_address);
	 	}

		// The TCP/IP server thread
		on stdcore[0]: uip_server(mac_rx[0], mac_tx[0],
				xtcp, 1, ipconfig,
				connect_status);

		on stdcore[0]: udp_server(xtcp[0]);
	}
	return 0;
}
