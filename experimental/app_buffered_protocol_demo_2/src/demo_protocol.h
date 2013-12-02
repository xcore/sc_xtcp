// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef __demo_protocol_h__
#define __demo_protocol_h__

#include "xtcp_client.h"
#include "xtcp_buffered_client.h"

#define DEMO_PROTOCOL_PORT 15533
#define DEMO_PROTOCOL_MAX_MSG_SIZE 100
#define DEMO_PROTOCOL_RXBUF_LEN 2048
#define DEMO_PROTOCOL_TXBUF_LEN 1024

#define DEMO_PROTOCOL_CONN_TIMEOUT_MS (60000) // timeout due to inactivity after 1 minute


#ifndef __XC__

typedef struct demo_protocol_state_t
{
  int active;
  int len;
  int last_used;
  int conn_id;
  xtcp_bufinfo_t bufinfo;
  char inbuf[DEMO_PROTOCOL_RXBUF_LEN];
  char outbuf[DEMO_PROTOCOL_TXBUF_LEN];
} demo_protocol_state_t;

#endif

void demo_protocold(chanend tcp_svr);

// Used internally to the server
void demo_protocol_init(chanend tcp_svr);

void demo_protocol_periodic(chanend tcp_svr, int t);

void demo_protocol_handle_event(chanend tcp_svr,
                                REFERENCE_PARAM(xtcp_connection_t, conn),
                                int timestamp);


#endif //__demo_protocol_h__
