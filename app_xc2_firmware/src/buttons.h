/**
 * Module:  app_xc2_firmware
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    buttons.h
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
void button_monitor(chanend client, chanend startup, port keyA, port keyB);
#endif

unsigned int getButtonCount(chanend, int);

#define getButtonCountA(c) getButtonCount(c, 0)
#define getButtonCountB(c) getButtonCount(c, 1)

int get_button_startup(chanend startup);
