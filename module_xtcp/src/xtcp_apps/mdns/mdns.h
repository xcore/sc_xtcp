/**
 * Module:  module_xtcp
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    mdns.h
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
#ifndef _mdns_h_
#define _mdns_h_

#define MDNS_MAX_NAME_LENGTH             100
#define MDNS_NUM_TABLE_ENTRIES            5
#define MDNS_SERVER_PORT 5353 

#if MDNS_NETBIOS
#define NETBIOS_PORT     137
#endif

typedef enum mdns_entry_type_t {
  MDNS_CANONICAL_NAME_ENTRY,
  MDNS_NAME_ENTRY,
  MDNS_SRV_ENTRY
} mdns_entry_type_t;

void mdns_init(chanend tcp_svr);

void mdns_start(chanend tcp_svr);

void mdns_handle_event(chanend tcp_svr, 
                       REFERENCE_PARAM(xtcp_connection_t, conn),
                       unsigned int t);

#ifndef __XC__
void mdns_add_entry(char name_prefix[], 
                    char name_postfix[], 
                    xtcp_ipaddr_t ipaddr, 
                    mdns_entry_type_t entry_type);
#else
void mdns_add_entry(char name_prefix[], 
                    char name_postfix[], 
                    xtcp_ipaddr_t ?ipaddr, 
                    mdns_entry_type_t entry_type);
#endif

#endif // _mdns_h_
