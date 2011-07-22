// Copyright (c) 2011, XMOS Ltd., All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef _uip_server_h_
#define _uip_server_h_
#include "xtcp_client.h"

// uIP based xtcp server
// Sets up based on the ipconfig parameter, if ipaddr is 
// set to all zeros is tries to obtain an ip address via
// dhcp/ipv4ll

#ifdef __XC__
void
uip_server(chanend mac_rx, 
           chanend mac_tx, 
           chanend xtcp[], 
           int num_xtcp_clients,
           xtcp_ipconfig_t &?ipconfig,
           chanend connect_status);
#else
void
uip_server(chanend mac_rx, 
           chanend mac_tx, 
           chanend xtcp[], 
           int num_xtcp_clients,
           xtcp_ipconfig_t *ipconfig,
           chanend connect_status);
#endif

#endif // _uip_server_h_
