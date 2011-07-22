// Copyright (c) 2011, XMOS Ltd., All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>


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
