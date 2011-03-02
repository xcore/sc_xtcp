// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef __xtcp_buffered_client_h__
#define __xtcp_buffered_client_h__
#include "xtcp_client.h"
#include "xtcp_bufinfo.h"

// The buffered API is for C only
#ifndef __XC__

void xtcp_buffered_set_rx_buffer(chanend tcp_svr, 
                                 xtcp_connection_t *conn, 
                                 xtcp_bufinfo_t *bufinfo,
                                 char *buf,
                                 int buflen);

void xtcp_buffered_set_tx_buffer(chanend tcp_svr, 
                                 xtcp_connection_t *conn, 
                                 xtcp_bufinfo_t *bufinfo,
                                 char *buf,
                                 int buflen,
                                 int lowmark);

void xtcp_buffered_send_handler(chanend tcp_svr, 
                                xtcp_connection_t *conn,
                                 xtcp_bufinfo_t *bufinfo);


int xtcp_buffered_recv(chanend tcp_svr,
                       xtcp_connection_t *conn, 
                       xtcp_bufinfo_t *bufinfo,
                       char **buf,
                       int len,
                       int *overflow);

int xtcp_buffered_send(chanend tcp_svr, 
                       xtcp_connection_t *conn, 
                       xtcp_bufinfo_t *bufinfo,
                       char *buf, 
                       int len);

int xtcp_buffered_send_buffer_remaining(xtcp_bufinfo_t *bufinfo);

#endif

#endif //x __xtcp_buffered_client_h__
