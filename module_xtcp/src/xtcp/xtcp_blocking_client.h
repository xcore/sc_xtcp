// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef _xtcp_blocking_client_h_
#define _xtcp_blocking_client_h_

void xtcp_wait_for_ifup(chanend tcp_svr);

xtcp_connection_t 
xtcp_wait_for_connection(chanend tcp_svr);

int xtcp_write(chanend tcp_svr, 
               REFERENCE_PARAM(xtcp_connection_t, conn),
               unsigned char buf[],
               int len);

int xtcp_read(chanend tcp_svr, 
              REFERENCE_PARAM(xtcp_connection_t, conn),
              unsigned char buf[],
              int minlen);

#endif
