#include <platform.h>
#include <print.h>
#include "uip_server.h"
#include "getmac.h"
#include "ethernet_server.h"
#include "xtcp_client.h"



// Ethernet Ports
on stdcore[2]: port otp_data = XS1_PORT_32B; 		// OTP_DATA_PORT
on stdcore[2]: out port otp_addr = XS1_PORT_16C;	// OTP_ADDR_PORT
on stdcore[2]: port otp_ctrl = XS1_PORT_16D;		// OTP_CTRL_PORT

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
on stdcore[2]: smi_interface_t smi = { PORT_ETH_MDIO, PORT_ETH_MDC, 0 };
#else
on stdcore[2]: smi_interface_t smi = { PORT_ETH_RST_N_MDIO, PORT_ETH_MDC, 1 };
#endif


// Static IP Config - change this to suit your network
#if 0
xtcp_ipconfig_t ipconfig =
{
  {192,168,0,100}, // ip address
  {255,255,0,0},   // netmask
  {0,0,0,0}        // gateway
};
#endif

xtcp_ipconfig_t ipconfig = {{0}};


void client_test(chanend tcp_svr)
{
  xtcp_connection_t conn;
  xtcp_ipaddr_t host = {10,0,102,172};
  unsigned char buf[1024];
  int len;
  for (int i=0;i<1024;i++) 
    buf[i] = 'a' + i % 26;

  xtcp_wait_for_ifup(tcp_svr);
  printstr("interface up\n");
 
  xtcp_connect(tcp_svr, 8000, host, XTCP_PROTOCOL_TCP);
  printstr("trying to connect...\n");

  conn = xtcp_wait_for_connection(tcp_svr);
  printstr("connected.\n");

  xtcp_write(tcp_svr, conn, buf, 1024);
  printstr("data sent.\n");

  len = xtcp_read(tcp_svr, conn, buf, 26);
  printstr("data received.\n");
  printintln(len);

  xtcp_close(tcp_svr, conn);
  printstr("done.\n");
}

// Program entry point
int main(void)
{
  chan mac_rx[1], mac_tx[1], xtcp[1], connect_status;
  
  par
    {
      // XCore 0
      on stdcore[0]: client_test(xtcp[0]);
      
      // XCore 2
      on stdcore[2]: uip_server(mac_rx[0], mac_tx[0], xtcp, 1, ipconfig, connect_status);
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
                        mac_rx, 1, mac_tx, 1, smi, connect_status);
      }
      
    }
  return 0;
}
