// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include <xclib.h>
#include <print.h>
#include "linklocal.h"
#include "rx.h"
#include "tx.h"
#include "ethernet.h"
#include "timer.h"
#include "arp.h"
#include "ipv4.h"


#define MAX_CONFLICTS       10
#define RATE_LIMIT_INTERVAL 60

#define STATE_WAITING         0
#define STATE_PROBED_ONCE     1
#define STATE_PROBED_TWICE    2
#define STATE_PROBED_THRICE   3
#define STATE_ANNOUNCED_ONCE  4
#define STATE_ANNOUNCED_TWICE 5
#define STATE_GOT_IT          6

#define randomPolynomial      0xEDB88320  // Ethernet polynomial - saves space

static unsigned int random = 0;
static unsigned int randomIPAddress;
static int state;
static int conflicts;

static void initiateAddress() {
    random = (myMacAddress, unsigned int [])[0];
}

static void advanceAddress() {
    crc32(random, myMacAddress[4], randomPolynomial);
    crc32(random, myMacAddress[5], randomPolynomial);
    randomIPAddress = 0xA9FE0000 | (random & 0xFF);
    randomIPAddress |= (1+((random >> 8) % 254)) << 8;
}

void pipInitLinkLocal() {
    if (state != STATE_WAITING) {
        return;
    }
    state = STATE_WAITING;
    initiateAddress();
    advanceAddress();
    pipSetTimeOut(PIP_LINK_LOCAL_TIMER, 0, 100000, PIP_FUZZ_1S);
}

void pipDisableLinkLocal() {
    state = STATE_WAITING;
    pipResetTimeOut(PIP_LINK_LOCAL_TIMER);
}

int pipIncomingLinkLocalARP(int oper, int ipAddress, unsigned char macAddress[], int offset) {

    if (ipAddress != randomIPAddress) {
        return 0;
    }
    // TODO: check that it is not my MAC address.
    myIP = 0;
    state = STATE_WAITING;
    advanceAddress();
    conflicts++;
    if (conflicts >= MAX_CONFLICTS) {
        conflicts = 0;
        pipSetTimeOut(PIP_LINK_LOCAL_TIMER, 60, 0, PIP_FUZZ_10S);
    } else {
        pipSetTimeOut(PIP_LINK_LOCAL_TIMER, 0, 100000, PIP_FUZZ_1S);
    }
    return 1;
}

static void progressState(int newState, int wait, int fuzz) {
    char zeros[1] = {0};
    state = newState;
    pipCreateARP(0, randomIPAddress, zeros, -1);
    pipSetTimeOut(PIP_LINK_LOCAL_TIMER, wait, 0, fuzz);
}

void pipTimeOutLinkLocal() {
    switch(state) {
    case STATE_WAITING:
        progressState(STATE_PROBED_ONCE,   1, PIP_FUZZ_1S);
        break;
    case STATE_PROBED_ONCE:
        progressState(STATE_PROBED_TWICE,  1, PIP_FUZZ_1S);
        break;
    case STATE_PROBED_TWICE:
        progressState(STATE_PROBED_THRICE, 2, 0);
        break;
    case STATE_PROBED_THRICE:
        myIP = randomIPAddress;
        progressState(STATE_ANNOUNCED_ONCE, 2, 0);
        break;
    case STATE_ANNOUNCED_ONCE:
        myIP = randomIPAddress;
        progressState(STATE_ANNOUNCED_TWICE, 2, 0);
        break;
    case STATE_ANNOUNCED_TWICE:
        state = STATE_GOT_IT;
        pipAssignIPv4(randomIPAddress, 0, 0);
        break;
    }
}

