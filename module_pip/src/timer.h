// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>


#define PIP_DHCP_TIMER_T1            0
#define PIP_DHCP_TIMER_T2            1
#define PIP_ARP_TIMER                2
#define PIP_TCP_TIMER_TIMEWAIT       3
#define PIP_TFTP_TIMER               4
#define PIP_LINK_LOCAL_TIMER         5

#define PIP_FUZZ_NONE                0
#define PIP_FUZZ_1MS                 0x0001FFFF
#define PIP_FUZZ_10MS                0x000FFFFF
#define PIP_FUZZ_100MS               0x007FFFFF
#define PIP_FUZZ_1S                  0x07FFFFFF
#define PIP_FUZZ_10S                 0x3FFFFFFF

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
 * \param fuzz         Random delay to be added. Set to zero for no random delay,
 *                     otherwise should be a bit mask. For each '1' in the
 *                     bit mask a random delay will be added in the
 *                     corresponding bit position on tenNSDelay. Use one of
 *                     the preset bit masks PIP_FUZZ_... defined earlier
 *                     for random delays of 1ms to 10s.
 */
void pipSetTimeOut(int timerNumber, int secsDelay, int tenNSdelay, int fuzz);

/** Function to reset a timeout. The timerNumber must be one of the
 * preallocated timer values defined above.
 *
 * \param timerNumber  integer constant denoting which time out to reset.
 *                     The function to call is hardcoded in the timer.
 */
void pipResetTimeOut(int timerNumber);

void pipPrintTimeOuts();
