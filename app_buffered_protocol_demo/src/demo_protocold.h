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
