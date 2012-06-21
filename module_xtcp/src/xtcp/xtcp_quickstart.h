#ifndef __xtcp_quickstart_h__
#define __xtcp_quickstart_h__
#include "uip_server.h"
#include <xccompat.h>
#include "ethernet_quickstart.h"

void ethernet_xtcp_server(REFERENCE_PARAM(xtcp_ipconfig_t, ipconfig),
                          chanend xtcp[],
                          int n);


#endif // __xtcp_quickstart_h__
