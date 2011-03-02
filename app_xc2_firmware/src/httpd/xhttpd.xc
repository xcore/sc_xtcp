// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include "httpd.h"
#include "xtcp_client.h"
#include <xs1.h>
#include <print.h>
#include "ethernet_tx_client.h"
#include "mdns.h"
#include "page_server.h"

#define PAGE_REFRESH_INTERVAL 25000000

void xhttpd(chanend tcp_svr, 
            chanend page_svr,
            chanend led_svr) 
{

  timer tmr;
  unsigned int t;
  
  httpd_init(tcp_svr);
  mdns_init(tcp_svr);

  while(1) {
	    xtcp_connection_t conn;
    select 
      {
      case xtcp_event(tcp_svr, conn):         
        if (conn.event == XTCP_IFUP) {
          xtcp_ipconfig_t ipconfig;
          xtcp_get_ipconfig(tcp_svr, ipconfig);

          printstr("Registering mdns name\n");
          mdns_register_canonical_name("xc2");
          mdns_register_name("www.xc2");
          mdns_register_service("XC-2 Demo Webserver", "_http._tcp.local", 80, "");
        }

        mdns_xtcp_handler(tcp_svr, conn);
        http_xtcp_handler(tcp_svr, conn, page_svr, led_svr);        
        break;
      case tmr when timerafter(t+PAGE_REFRESH_INTERVAL) :> t:
        page_server_refresh_ip(page_svr);
        break;        
      }
  }
  
}
