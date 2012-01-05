// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>


#define PIP_DHCP_TIMER_T1            0
#define PIP_DHCP_TIMER_T2            1
#define PIP_ARP_TIMER                2
#define PIP_TCP_TIMER_TIMEWAIT       3

/** Select function to be called in the main top level select loop. It
 * implements timers for all other modules, and calls time-out functions
 * for DHCP, TCP, ARP, etc.
 *
 * \param t  timer on which to implement the main time out.
 */
select pipTimeOut(timer t);

/** Function to set a timeout. The timerNumber must be one of the
 * preallocated timer values defined above. Delay is given in secs and in
 * 100 Mhz reference ticks. Maximum time out is 40 years; maximum up time
 * of a product is 1500 years.
 *
 * \param timerNumber  integer constant denoting which time out to set.
 *                     The function to call is hardcoded in the timer.
 *
 * \param secsDelay    Number of seconds until timer expires.
 *
 * \param tenNSDelay   Number of 10 ns ticks until timer expires
 *                     (should be less than 100000000).
 *
 * \param fuzz         Random delay to be added. Set to zero for no random
 *                     delay, otherwise up to this many seconds will be added. 
 */
void pipSetTimeOut(int timerNumber, int secsDelay, int tenNSdelay, int fuzz);
