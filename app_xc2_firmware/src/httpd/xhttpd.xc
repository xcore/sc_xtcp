/**
 * Module:  app_xc2_firmware
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    xhttpd.xc
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
#include "httpd.h"
#include "xtcp_client.h"
#include <xs1.h>
#include <print.h>
#include "ethernet_tx_client.h"
#include "mdns.h"
#include "page_server.h"


void xhttpd(chanend tcp_svr, 
            chanend page_svr,
            chanend led_svr) 
{

  timer tmr;
  unsigned int t;
  int refresh_ip = 0;

  xtcp_listen(tcp_svr, HTTP_SERVER_PORT, XTCP_PROTOCOL_TCP);
  

  httpd_init();
  mdns_init(tcp_svr);

  xtcp_ask_for_conn_or_config_event(tcp_svr);
  while(1) {
    xtcp_connection_t conn;
    xtcp_conn_or_config_t event_type;
    xtcp_ipconfig_t ipconfig;
    xtcp_config_event_t config_event;
    select 
      {
      case xtcp_conn_or_config_event(tcp_svr,
                                     event_type,
                                     config_event, ipconfig,
                                     conn):
        switch (event_type)
          {
          case XTCP_CONFIG_EVENT:
            if (config_event == XTCP_IFUP) {              
              mdns_add_entry("xc2","local",
                             ipconfig.ipaddr, 
                             MDNS_CANONICAL_NAME_ENTRY);
              mdns_add_entry("www.xc2","local",
                             ipconfig.ipaddr, 
                             MDNS_NAME_ENTRY);
              mdns_add_entry("XC-2 Demo Webserver","_http._tcp.local",
                             null, 
                             MDNS_SRV_ENTRY);

              mdns_start(tcp_svr);
            }
            break;
          case XTCP_CONN_EVENT:
            switch (conn.local_port) 
              {
              case HTTP_SERVER_PORT:
                httpd_handle_event(tcp_svr, conn,
                                   page_svr,
                                   led_svr);           
                break;
              case MDNS_SERVER_PORT:
#if MDNS_NETBIOS
              case NETBIOS_PORT:
#endif
                { unsigned int t;
                  tmr :> t;
                  mdns_handle_event(tcp_svr, conn, t);
                }
                break;
              default:
                printintln(conn.local_port);
                break;
              }              
            break;
          }
        if (refresh_ip) {
          page_server_refresh_ip(page_svr);
          refresh_ip = 0;
        }
        xtcp_ask_for_conn_or_config_event(tcp_svr);
        break;
      case tmr when timerafter(t+25000000) :> t:
        refresh_ip = 1;
        break;        
      }
  }
  
}
