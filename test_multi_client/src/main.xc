// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>
#if (XSCOPE_ENABLED)
#include <xscope.h>
#endif
#include "print.h"
#include "uip_server.h"
#include "getmac.h"
#include "ethernet_server.h"


// Ethernet Ports
on stdcore[ETH_CORE_ID]: port otp_data = XS1_PORT_32B; // OTP_DATA_PORT
on stdcore[ETH_CORE_ID]: out port otp_addr = XS1_PORT_16C; // OTP_ADDR_PORT
on stdcore[ETH_CORE_ID]: port otp_ctrl = XS1_PORT_16D; // OTP_CTRL_PORT


on stdcore[ETH_CORE_ID]: clock clk_smi = XS1_CLKBLK_5;

on stdcore[ETH_CORE_ID]: mii_interface_t mii =
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
on stdcore[ETH_CORE_ID]: out port p_mii_resetn = PORT_ETH_RST_N;
on stdcore[ETH_CORE_ID]: smi_interface_t smi = {PORT_ETH_MDIO, PORT_ETH_MDC, 0};
#else
on stdcore[ETH_CORE_ID]: smi_interface_t smi = {PORT_ETH_RST_N_MDIO, PORT_ETH_MDC, 1};
#endif

// IP Config - change this to suit your network.  Leave with all
// 0 values to use DHCP/AutoIP
xtcp_ipconfig_t ipconfig = {
		{ 0, 0, 0, 0 }, // ip address (eg 192,168,0,2)
		{ 0, 0, 0, 0 }, // netmask (eg 255,255,255,0)
		{ 0, 0, 0, 0 } // gateway (eg 192,168,0,1)
};


#define RX_BUFFER_SIZE 300
#define INCOMING_PORT 15533
#define BROADCAST_INTERVAL 600000000
#define BROADCAST_PORT 15534

/** Simple UDP reflection thread.
 *
 * This thread does two things:
 *
 *   - Reponds to incoming packets on port INCOMING_PORT and
 *     with a packet with the same content back to the sender.
 *   - Periodically sends out a fixed packet to a broadcast IP address.
 *
 */
void udp_reflect(chanend c_xtcp, int port0, int port1)
{
  xtcp_connection_t conn;
  xtcp_connection_t responding_connection;
  xtcp_connection_t broadcast_connection;

  xtcp_ipaddr_t broadcast_addr = {255,255,255,255};
  int send_flag = 0;

  int broadcast_send_flag = 0;

  timer tmr;
  unsigned int time;

  char rx_buffer[RX_BUFFER_SIZE];
  char tx_buffer[RX_BUFFER_SIZE];
  char broadcast_buffer[RX_BUFFER_SIZE];

  int response_len;
  int broadcast_len;

  responding_connection.id = -1;
  broadcast_connection.id = -1;

  xtcp_listen(c_xtcp, port0, XTCP_PROTOCOL_UDP);

  tmr :> time;
  while (1) {
    select {

    case xtcp_event(c_xtcp, conn):
      switch (conn.event)
        {
        case XTCP_IFUP:
          xtcp_connect(c_xtcp,
                       port1,
                       broadcast_addr,
                       XTCP_PROTOCOL_UDP);
          break;
        case XTCP_IFDOWN:
          if (responding_connection.id != -1) {
            xtcp_close(c_xtcp, responding_connection);
            responding_connection.id = -1;
          }
          if (broadcast_connection.id != -1) {
            xtcp_close(c_xtcp, broadcast_connection);
            responding_connection.id = -1;
          }
          break;
        case XTCP_NEW_CONNECTION:
          if (XTCP_IPADDR_CMP(conn.remote_addr, broadcast_addr)) {
            printstr("New broadcast connection established:");
            printintln(conn.id);
            broadcast_connection = conn;
          }
          else {
            printstr("New connection to listening port:");
            printintln(conn.local_port);
            if (responding_connection.id == -1) {
              responding_connection = conn;
            }
            else {
              printstr("Cannot handle new connection");
              xtcp_close(c_xtcp, conn);
            }
          }
          break;
        case XTCP_RECV_DATA:
          response_len = xtcp_recv_count(c_xtcp, rx_buffer, RX_BUFFER_SIZE);
          printstr("Got data: ");
          printint(response_len);
          printstrln(" bytes");

          for (int i=0;i<response_len;i++)
            tx_buffer[i] = rx_buffer[i];

          if (!send_flag) {
            xtcp_init_send(c_xtcp, conn);
            send_flag = 1;
            printstrln("Responding");
          }
          else {
            // Cannot respond here since the send buffer is being used
          }
          break;
      case XTCP_REQUEST_DATA:
      case XTCP_RESEND_DATA:

        if (conn.id == broadcast_connection.id) {
          xtcp_send(c_xtcp, broadcast_buffer, broadcast_len);
        }
        else {
          xtcp_send(c_xtcp, tx_buffer, response_len);
        }
        break;
      case XTCP_SENT_DATA:
        xtcp_complete_send(c_xtcp);
        if (conn.id == broadcast_connection.id) {
          printstr("Sent Broadcast on conn "); printintln(conn.id);
          broadcast_send_flag = 0;
        }
        else {
          printstrln("Sent Response");
          xtcp_close(c_xtcp, conn);
          responding_connection.id = -1;
          send_flag = 0;
        }
        break;
      case XTCP_TIMED_OUT:
      case XTCP_ABORTED:
      case XTCP_CLOSED:
        printstr("Closed connection:");
        printintln(conn.id);
        break;
      case XTCP_ALREADY_HANDLED:
          break;
      }
      break;

    case tmr when timerafter(time + BROADCAST_INTERVAL) :> void:

      if (broadcast_connection.id != -1 && !broadcast_send_flag)  {
        broadcast_len = 100;
        xtcp_init_send(c_xtcp, broadcast_connection);
        broadcast_send_flag = 1;
      }
      tmr :> time;
      break;
    }
  }
}

#if (XSCOPE_ENABLED)
void xscope_user_init(void)
{
  xscope_register(0, 0, "", 0, "");
}
#endif

#define NUM_CLIENTS 4

// Program entry point
int main(void) {
	chan mac_rx[1], mac_tx[1], xtcp[NUM_CLIENTS], connect_status;

	par
	{
		// The ethernet server
		on stdcore[ETH_CORE_ID]:
		{
			int mac_address[2];

      #if (XSCOPE_ENABLED)
      xscope_config_io(XSCOPE_IO_BASIC);
      #endif

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
		on stdcore[0]: uip_server(mac_rx[0], mac_tx[0], xtcp, NUM_CLIENTS, ipconfig, connect_status);

    par (int i=0; i < NUM_CLIENTS; i++)
      on stdcore[0]: udp_reflect(xtcp[i], INCOMING_PORT+i, BROADCAST_PORT+i);

	}
	return 0;
}
