// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include "mdns.h"

mdns_event mdns_handle_event(chanend tcp_svr,
                       xtcp_connection_t &conn,
                       unsigned int t);

mdns_event mdns_xtcp_handler(chanend tcp_svr,
                       xtcp_connection_t &conn)
{
  timer tmr;
  unsigned t;
  tmr :> t;
  return mdns_handle_event(tcp_svr, conn, t);
}


