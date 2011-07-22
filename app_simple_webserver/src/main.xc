/**
 * Module:  app_simple_webserver
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    main.xc
 *
 * The copyrights, all other intellectual and industrial 
 * property rights are retained by XMOS and/or its licensors. 
 * Terms and conditions covering the use of this code can
 * be found in the Xmos End User License Agreement.
 *
 * Copyright XMOS Ltd 2009
 *
 * In the case where this code is a modification of existing code
 * under a separate license, the separate license terms are shown
 * below. The modifications to the code are still covered by the 
 * copyright notice above.
 *
 **/                                   
#include <platform.h>
#include "uip_server.h"
#include "xhttpd.h"
#include "getmac.h"
#include "ethernet_server.h"
//***** Ethernet Configuration ****

on stdcore[2]: clock clk_mii_ref = XS1_CLKBLK_REF;

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

on stdcore[2]: clock clk_smi = XS1_CLKBLK_5;

// static ip config
// change this to suit your network
xtcp_ipconfig_t ipconfig = {
  {192,168,0,100}, // ip address
  {255,255,0,0},   // netmask
  {0,0,0,0}        // gateway    
};


int main(void) 
{
  chan mac_rx[1], mac_tx[1], xtcp[1], connect_status;
  par {
    on stdcore[1]: uip_server(mac_rx[0], mac_tx[0], 
                              xtcp, 1, ipconfig, connect_status);
  
    on stdcore[3]: xhttpd(xtcp[0]);

    on stdcore[2]:
    {
      int mac_address[2];
      ethernet_getmac_otp((mac_address, char[]));
      phy_init(clk_smi, clk_mii_ref, 
#ifdef PORT_ETH_RST_N               
               p_mii_resetn,
#else
               null,
#endif
               smi,
               mii);
      ethernet_server(mii, clk_mii_ref, mac_address, 
                      mac_rx, 1, 
                      mac_tx, 1,
                      smi,
                      connect_status);
    }
    
  }
}
