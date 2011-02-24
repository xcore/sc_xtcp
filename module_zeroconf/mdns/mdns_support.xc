#include "mdns.h"

void mdns_handle_event(chanend tcp_svr, 
                       xtcp_connection_t &conn,
                       unsigned int t);

void mdns_xtcp_handler(chanend tcp_svr,
                       xtcp_connection_t &conn)
{
  timer tmr;
  unsigned t;
  tmr :> t;
  mdns_handle_event(tcp_svr, conn, t);
}


