/**
 * Module:  module_xtcp
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    uip_server.h
 *
 * The copyrights, all other intellectual and industrial 
 * property rights are retained by XMOS and/or its licensors. 
 * Terms and conditions covering the use of this code can
 * be found in the Xmos End User License Agreement.
 *
 * Copyright XMOS Ltd 2009
 *
 * In the case where this code is a modification of existing code
 * under a separate license, the separate license terms are shown
 * below. The modifications to the code are still covered by the 
 * copyright notice above.
 *
 **/                                   
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
