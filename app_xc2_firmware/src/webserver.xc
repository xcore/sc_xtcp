// Copyright (c) 2011, XMOS Ltd., All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include <platform.h>

#include "getmac.h"
#include "ethernet_server.h"
#include "leds.h"
#include "buttons.h"
#include "xc2_firmware_config.h"
#include "uip_server.h"
#include "xtcp_client.h"
#include "xtcp_server.h"
#include "httpd.h"
#include "page_server.h"
#include "ethernet_rx_client.h"

#include <print.h>
#include <xs1.h>

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


on stdcore[0]: port led00 = XS1_PORT_1I;
on stdcore[0]: port led01 = XS1_PORT_1J;
on stdcore[1]: port led10 = XS1_PORT_1I;
on stdcore[1]: port led11 = XS1_PORT_1J;
on stdcore[2]: port led20 = XS1_PORT_1I;
on stdcore[2]: port led21 = XS1_PORT_1J;
on stdcore[3]: port led30 = XS1_PORT_1I;
on stdcore[3]: port led31 = XS1_PORT_1J;

on stdcore[0]: port buttonA = XS1_PORT_4C;
on stdcore[0]: port buttonB = XS1_PORT_4D;

on stdcore[0]: port uart_tx = PORT_UART_TX;

int main(void) {
  chan mac_rx[1], mac_tx[2];
  chan c_led0, c_led1, c_led2, c_led3, led_svr0, led_svr1;
  chan button_ch, button_startup;
  chan config_ch[2];
  chan xtcp[2];
  chan page_svr;

  chan connect_status;

  par {
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
                      mac_tx, 2,
                      smi,
                      connect_status);
    }


    on stdcore[2]:
    { int use_static_ip;     
      int init_button = get_button_startup(button_startup);
      xtcp_ipconfig_t ipconfig;
     
      if (init_button==1)
        use_static_ip = 1;
      else if (init_button==2)
        use_static_ip = 0;
      else
        use_static_ip = get_ip_config(config_ch[0]);
      
      set_ip_config(config_ch[0], use_static_ip);

      if (use_static_ip) {
        printstr("Using Static IP config\n");       
        xtcp_uint_to_ipaddr(ipconfig.ipaddr, get_ip_addr(config_ch[0]));
        xtcp_uint_to_ipaddr(ipconfig.netmask, 
                            get_default_netmask(config_ch[0]));
        xtcp_uint_to_ipaddr(ipconfig.gateway, 0);
      }
      else {
        printstr("Using Dynamic IP config\n");
        for (int i=0;i<4;i++)
          ipconfig.ipaddr[i] = 0;
      }
      xtcpd_init(xtcp, 2);
      enable_ipconfig_from_ipserver(config_ch[0]);      
      mac_set_drop_packets(mac_rx[0], 0);
      uip_server(mac_rx[0], mac_tx[0], xtcp, 2, ipconfig, connect_status);
    }
    
    on stdcore[3]: xhttpd(xtcp[0],
                          page_svr,
                          led_svr0);

    on stdcore[1]: page_server(page_svr, button_ch, mac_tx[1],  config_ch[1]);

    on stdcore[0]: flash_leds(c_led0, led00, led01);
    on stdcore[1]: flash_leds(c_led1, led10, led11);
    on stdcore[2]: flash_leds(c_led2, led20, led21);
    on stdcore[3]: flash_leds(c_led3, led30, led31);
    on stdcore[0]: led_server(led_svr0, led_svr1, c_led0, c_led1, c_led2, c_led3);
    on stdcore[0]: button_monitor(button_ch, button_startup, buttonA, buttonB);
    on stdcore[0]: xc2_firmware_config(config_ch,
                                       2,
                                       xtcp[1],
                                       led_svr1);
    
  }

  return 0;
}
