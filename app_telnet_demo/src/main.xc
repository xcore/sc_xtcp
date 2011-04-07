// Copyright (c) 2011, XMOS Ltd., All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>
#include "uip_server.h"
#include "getmac.h"
#include "ethernet_server.h"
#include <print.h>
#include "telnetd.h"

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


// Static IP Config - change this to suit your network
#if 1
xtcp_ipconfig_t ipconfig =
{
  {169,254,3,2},// ip address
  {255,255,0,0},   // netmask
  {0,0,0,0}        // gateway
};
#else
xtcp_ipconfig_t ipconfig = {0};
#endif



static int active_conn = -1;


void telnetd_recv_line(chanend tcp_svr,
                       int id,
                       char line[],
                       int len)
{
  printstrln(line);
  telnetd_send_line(tcp_svr, id, line);
}
                       
void telnetd_sent_line(chanend tcp_svr, int id)
{
  // do nothing
  return;
}

void telnetd_new_connection(chanend tcp_svr, int id)
{
  char welcome[][50] = {"Welcome to the echo server!",
                        "(echo server... echo server... echo server...)"};
  for (int i=0;i<2;i++)
    telnetd_send_line(tcp_svr, id, welcome[i]);    
  active_conn = id;
}

void telnetd_connection_closed(chanend tcp_svr, int id)
{
  active_conn = -1;
}

void demo(chanend xtcp) {
  xtcp_connection_t conn;
  timer tmr;
  unsigned t;
  tmr :> t;
  telnetd_init(xtcp);
  while (1) {
    select {
    case xtcp_event(xtcp, conn):
      telnetd_handle_event(xtcp, conn);
      break;
    case tmr when timerafter(t) :> void:
      if (active_conn != -1) {
        char buf[11] = "<periodic>";
        telnetd_send_line(xtcp, active_conn, buf);
      }
      t += 1000000000;
      break;
    }
  }
}

#if 0
void demo(chanend xtcp) {
  xtcp_connection_t conn;

   xtcp_listen(xtcp, 23, XTCP_PROTOCOL_TCP);
  while (1) {
    slave xtcp_event(xtcp, conn);
    if (conn.event == XTCP_RECV_DATA) {
      unsigned char data[XTCP_CLIENT_BUF_SIZE];
      int len;

      // Receive the data from the TCP stack
      len = xtcp_recv(xtcp, data);
      for (int i=0;i<len;i++) {        
        printstr(" ");
        printint(data[i]);
      }
    }
  }
}
#endif

// Program entry point
int main(void)
{
	chan mac_rx[1], mac_tx[1], xtcp[1], connect_status;

	par
	{
          on stdcore[0]: demo(xtcp[0]);


          on stdcore[0]: uip_server(mac_rx[0], mac_tx[0], 
                                    xtcp, 1, ipconfig, connect_status);
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
            
            ethernet_server(mii, mac_address, mac_rx, 1, mac_tx, 1, smi, connect_status );
          }

	}
        return 0;
}
