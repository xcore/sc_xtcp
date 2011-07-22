/**
 * Module:  module_xtcp
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    igmp.h
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
#ifndef _igmp_h_
#define _igmp_h_

void igmp_init();

void igmp_periodic();
void igmp_in();
void igmp_join_group(uip_ipaddr_t addr);
void igmp_leave_group(uip_ipaddr_t addr);
int igmp_check_addr(uip_ipaddr_t addr);
#endif // _igmp_h_
