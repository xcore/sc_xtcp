#ifndef __telnetd_h__
#define __telnetd_h__
#ifdef __telnetd_conf_h_exists__
#include "telnetd_conf.h"
#endif

#include "xtcp_client.h"
#include "xccompat.h"

#ifndef NUM_TELNETD_CONNECTIONS
// Maximum number of concurrent connections
#define NUM_TELNETD_CONNECTIONS 10
#endif

#ifndef TELNET_LINE_BUFFER_LEN
#define TELNET_LINE_BUFFER_LEN 160
#endif

#define TELNETD_PORT 23

void telnetd_init(chanend tcp_svr);

void telnetd_handle_event(chanend tcp_svr, 
                          REFERENCE_PARAM(xtcp_connection_t, conn));


int telnetd_send_line(chanend tcp_svr,
                      int i,
                      char line[]);

int telnetd_send(chanend tcp_svr,
                 int i,
                 char line[]);

void telnetd_recv_line(chanend tcp_svr,
                       int i,
                       char line[],
                       int len);
                       
void telnetd_sent_line(chanend tcp_svr,
                       int i);

void telnetd_new_connection(chanend tcp_svr, int id);

void telnetd_connection_closed(chanend tcp_svr, int id);

#endif // __telnetd_h__
