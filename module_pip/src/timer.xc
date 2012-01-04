// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include <print.h>
#include "timer.h"
#include "dhcp.h"
#include "arp.h"

/* 
 * TImes are represented by a pair of numbers, an epoch and a time value.
 * The time value ticks in 100 MHz reference clocks, the epoch value in
 * 2^30 reference clocks (approx 10.73 seconds). The epochs run out after
 * 1400 years; sufficient for our ethernet stack. 
 *
 * Note that there is a * 2-bit overlap between the epoch and the timer;
 * this makes it easier to * deal with "timerafter" semantics.
 */
static struct {
    int timerNumber;
    int time;
    int epoch;
} timeOut[10];

#define EPOCH_BIT    30

static int timeOuts = 0;
static timer globalTimer;
static int epoch = 0, timeOutValue = 0, waitingForEpoch = 0;

static void numberZeroTimedOut() {
    int timerNumber = timeOut[0].timerNumber;
    timeOuts--;
    for(int i = 0; i < timeOuts; i++) {
        timeOut[i] = timeOut[i+1];
    }
    switch(timerNumber) {
#ifdef PIP_DHCP
    case PIP_DHCP_TIMER_T1:
        pipDhcpTimeOutT1();
        break;
    case PIP_DHCP_TIMER_T2:
        pipDhcpTimeOutT2();
        break;
#endif
#ifdef PIP_ARP
    case PIP_ARP_TIMER:
        pipArpTimeOut();
        break;
#endif
    }
}

static void setTimeOutValue() {
    waitingForEpoch = timeOut[0].epoch != epoch;
    if (waitingForEpoch) {
        timeOutValue = ((epoch+1) << EPOCH_BIT);
    } else {
        timeOutValue = timeOut[0].time;
    }
}

select pipTimeOut(timer t) {
//(timeOut[0].epoch < epoch) => default:                    // We somehow missed an epoch...
//    numberZeroTimedOut();
//    break;
case globalTimer when timerafter(timeOutValue) :> unsigned now:
    if (!waitingForEpoch) {
        numberZeroTimedOut();
    }
    if ((epoch & ((1<<(32-EPOCH_BIT))-1)) != (now >> EPOCH_BIT)) {
        epoch++;
    }
    setTimeOutValue();
    break;
}

static int epochAfter(int e1, int t1, int e2, int t2) {
    if (e1 > e2) return 1;
    if (e1 < e2) return 0;
    if ( ((int) (t1 - t2)) > 0) return 1;
    return 0;
}

void pipSetTimeOut(int timerNumber, int secsDelay, int tenNSDelay, int fuzz) {
    unsigned int wakeUp, wakeUpEpoch;
    unsigned int currentTime, currentEpoch;
    int i;
    unsigned h, l;

    globalTimer :> currentTime;
    currentEpoch = epoch;
    if ((epoch & ((1<<(32-EPOCH_BIT))-1)) != (currentTime >> EPOCH_BIT)) {
        currentEpoch++;
    }
    if (fuzz) {
        tenNSDelay += (currentTime & 1023) << 16;
    }
    {h,l} = mac(100000000, secsDelay, currentEpoch >> (32-EPOCH_BIT), tenNSDelay);
    {h,l} = mac(1, currentTime, h, l);
    wakeUpEpoch = h << (32 - EPOCH_BIT) | l >> EPOCH_BIT;
    wakeUp = l;

    for(i = timeOuts - 1; i >= 0; i--) {
        if (epochAfter(wakeUpEpoch, wakeUp, timeOut[i].epoch, timeOut[i].time)) {
            break;
        }
        timeOut[i+1] = timeOut[i];
    }
    timeOut[i+1].time = wakeUp;               // This may overwrite element zero
    timeOut[i+1].timerNumber = timerNumber;   // if loop terminated
    timeOut[i+1].epoch = wakeUpEpoch;         // This may overwrite element zero
    timeOuts++;
    for(; i >=0; i--) {
        if (timeOut[i].timerNumber == timerNumber) {
            for(int j = i; j < timeOuts-1; j++) {
                timeOut[j] = timeOut[j+1];
            }
            timeOuts--;
            break;
        }
    }
    setTimeOutValue();
}
