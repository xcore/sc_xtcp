/**
 * Module:  module_xtcp
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    autoip.h
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
#ifndef _autoip_h_
#define _autoip_h_

void autoip_init(int seed);
void autoip_arp_in();
void autoip_start();
void autoip_stop();
void autoip_configured(uip_ipaddr_t ipaddr);
void autoip_periodic();
#endif //_autoip_h_
