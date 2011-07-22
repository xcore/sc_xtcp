/**
 * Module:  app_xc2_firmware
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    xc2_firmware_config.h
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

void set_ip_config(chanend config_svr, unsigned int val);
void set_ip_addr(chanend config_svr, unsigned int val);
unsigned int get_ip_config(chanend config_svr);
unsigned int get_ip_addr(chanend config_svr);

void set_default_ip_config(chanend config_svr, unsigned int val);
void set_default_ip_addr(chanend config_svr, unsigned int val);
unsigned int get_default_ip_config(chanend config_svr);
unsigned int get_default_ip_addr(chanend config_svr);
void set_default_netmask(chanend config_svr, unsigned int val);
unsigned int get_default_netmask(chanend config_svr);
void enable_ipconfig_from_ipserver(chanend config_svr);

void commit_config(chanend config_svr);
void xc2_firmware_config(chanend config_ch[],
                         int num_config,
                         chanend xtcp_svr,
                         chanend led_svr);
