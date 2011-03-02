// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include <print.h>
#include "demo_protocold.h"
#include "xtcp_client.h"

#define DEMO_PROTOCOL_PERIOD_MS (200) // every 200 ms
#define DEMO_PROTOCOL_PERIOD_TIMER_TICKS \
  (DEMO_PROTOCOL_PERIOD_MS * XS1_TIMER_KHZ)

// The main webserver thread
void demo_protocold(chanend tcp_svr)
{
  xtcp_connection_t conn;
  timer tmr;
  int t, timestamp;

  // Initiate the HTTP state
  demo_protocol_init(tcp_svr);

  tmr :> t;
  // Loop forever processing TCP events
  while(1)
    {
      select
        {
        case xtcp_event(tcp_svr, conn):
          tmr :> timestamp;
          demo_protocol_handle_event(tcp_svr, conn, timestamp);
          break;
        case tmr when timerafter(t) :> void:
          demo_protocol_periodic(tcp_svr, t);
          t += DEMO_PROTOCOL_PERIOD_TIMER_TICKS;          
          break;
        }
    }
}

