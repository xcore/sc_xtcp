/**
 * Module:  app_xc2_firmware
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    leds.h
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

#ifdef __XC__
void flash_leds(chanend c, port led0, port led1);
#endif

void led_server(chanend client0,
                chanend client1,
                chanend led0,
                chanend led1,
                chanend led2,
                chanend led3);

void led_pattern(chanend led_svr,
                 int pattern);

void led_set_connected(chanend led_svr, int val);
