// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>

#include <print.h>
#include <xscope.h>
#include "xtcp.h"
#include "ethernet_board_support.h"

// These intializers are taken from the ethernet_board_support.h header for
// XMOS dev boards. If you are using a different board you will need to
// supply explicit port structure intializers for these values
ethernet_xtcp_ports_t xtcp_ports =
  {on ETHERNET_DEFAULT_TILE: OTP_PORTS_INITIALIZER,
     ETHERNET_DEFAULT_SMI_INIT,
     ETHERNET_DEFAULT_MII_INIT_lite,
     ETHERNET_DEFAULT_RESET_INTERFACE_INIT};

xtcp_ipconfig_t ipconfig = {
#if 1
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
#define TX_BUFFER_SIZE 1200

#define INCOMING_PORT_UDP 100
#define INCOMING_PORT_TCP 101


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

  char rx_buffer[RX_BUFFER_SIZE];
  char tx_buffer[TX_BUFFER_SIZE];

  int response_len;
  static unsigned send_count = 0;

  for (unsigned i=0; i<(sizeof(tx_buffer)+3)/4; ++i) (tx_buffer,unsigned[])[i] = i;

  xtcp_listen(c_xtcp, INCOMING_PORT_UDP, XTCP_PROTOCOL_UDP);
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
		  case INCOMING_PORT_UDP:
			  send_count = 0;
			  xtcp_init_send(c_xtcp, conn);
			  break;
		  case INCOMING_PORT_TCP:
			  send_count = 0;
			  xtcp_init_send(c_xtcp, conn);
			  break;
		  }
		  break;
	  case XTCP_REQUEST_DATA:
	  case XTCP_RESEND_DATA:
	  case XTCP_SENT_DATA:
		  if (conn.event != XTCP_RESEND_DATA) {
			  send_count++;
		  }
		  if (send_count > 100000) {
			  // Done sending
			  xtcp_complete_send(c_xtcp);
		  } else {
			  (tx_buffer,unsigned[])[0] = send_count;
			  xtcp_send(c_xtcp, tx_buffer, sizeof(tx_buffer));
		  }
		  break;
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

void xscope_user_init(void) {
  xscope_register(6,
                  XSCOPE_DISCRETE, "0", XSCOPE_UINT, "i",
                  XSCOPE_DISCRETE, "1", XSCOPE_UINT, "i",
                  XSCOPE_DISCRETE, "2", XSCOPE_UINT, "i",
                  XSCOPE_DISCRETE, "3", XSCOPE_UINT, "i",
                  XSCOPE_DISCRETE, "4", XSCOPE_UINT, "i",
                  XSCOPE_DISCRETE, "5", XSCOPE_UINT, "i");
  xscope_config_io(XSCOPE_IO_BASIC);
}


int main(void) {
	chan xtcp[1];

	par
	{
          on ETHERNET_DEFAULT_TILE: ethernet_xtcp_server(xtcp_ports,
                                                         ipconfig,
                                                         xtcp,
                                                         1);

          on stdcore[0]: udp_server(xtcp[0]);
	}
	return 0;
}
