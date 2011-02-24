#ifndef _httpd_h_
#define _httpd_h_

#include "xtcp_client.h"

#define HTTP_SERVER_PORT 80

void httpd_init(chanend tcp_svr);

void xhttpd(chanend tcpip_svr,
            chanend page_svr,
            chanend led_svr);


void http_xtcp_handler(chanend tcp_svr,
                       REFERENCE_PARAM(xtcp_connection_t, conn),
                       chanend page_svr,
                       chanend led_svr);

#endif // _httpd_h_
