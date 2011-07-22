/**
 * Module:  app_simple_webserver
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
#include <xs1.h>
#include <print.h>
#include "httpd.h"
#include "xtcp_client.h"

void xhttpd(chanend tcp_svr) 
{
  xtcp_connection_t conn;
  httpd_init();

  xtcp_listen(tcp_svr, 80, XTCP_PROTOCOL_TCP);
  xtcp_ask_for_event(tcp_svr);
  while(1) {
    select 
      {
      case xtcp_event(tcp_svr, conn):
        httpd_handle_event(tcp_svr, conn);
        xtcp_ask_for_event(tcp_svr);
        break;
      }
  }
}
