// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef _demo_protocold_h_
#define _demo_protocold_h_
#include "xccompat.h"
#include "xtcp_client.h"

void demo_protocold(chanend tcp_svr);

// Used internally to the server
void demo_protocol_init(chanend tcp_svr);

void demo_protocol_periodic(chanend tcp_svr, int t);

void demo_protocol_handle_event(chanend tcp_svr,
                                REFERENCE_PARAM(xtcp_connection_t, conn),
                                int timestamp);



#endif // _demo_protocold_h_
