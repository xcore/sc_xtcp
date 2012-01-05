// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

select pipTimeOut(timer t);
void pipSetTimeOut(int timerNumber, int secsDelay, int tenNSdelay, int fuzz);

#define PIP_DHCP_TIMER_T1   0
#define PIP_DHCP_TIMER_T2   1

#define PIP_ARP_TIMER       2

#define PIP_TCP_TIMER       3
