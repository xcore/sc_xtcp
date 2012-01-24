// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

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

void telnetd_init_conn(chanend tcp_svr);

void telnetd_init_state(chanend tcp_svr, REFERENCE_PARAM(xtcp_connection_t, conn));

void telnetd_handle_event(chanend tcp_svr, 
                          REFERENCE_PARAM(xtcp_connection_t, conn));


int telnetd_send_line(chanend tcp_svr,
                      int i,
                      char line[]);

int telnetd_send(chanend tcp_svr,
                 int i,
                 char line[]);

int fetch_connection_state_index(int conn_id);

void telnet_buffered_send_handler(chanend tcp_svr, REFERENCE_PARAM(xtcp_connection_t, conn));

void telnetd_recv(chanend tcp_svr, REFERENCE_PARAM(xtcp_connection_t, conn));

void telnetd_recv_line(chanend tcp_svr,
                       int i,
                       char line[],
                       int len);
                       
void telnetd_sent_line(chanend tcp_svr,
                       int i);

void telnetd_new_connection(chanend tcp_svr, int id);

void telnetd_connection_closed(chanend tcp_svr, int id);

void telnetd_free_state(REFERENCE_PARAM(xtcp_connection_t, conn));

#ifndef __XC__
void register_callback(void (*fnCallBack)(xtcp_connection_t *conn, char data));
#endif


#endif // __telnetd_h__
