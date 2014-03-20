// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * $Id: clock-arch.c,v 1.2 2006/06/12 08:00:31 adam Exp $
 */

/**
 * \file
 *         Implementation of architecture-specific clock functionality
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "clock-arch.h"
#include <print.h>

/* This assumes that this is called within 10s periods */
static int init = 1;
static int time = 0;
static unsigned int prev_timestamp = 0;

static unsigned long clock_second = 0;	// system clock seconds. Used as timebase for stimer

/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  timer tmr;
  unsigned t;

  tmr :> t;
  t = t - (t % 100000);

  if (init) {
    time = 0;
    init = 0;
  }
  else {
    unsigned diff = (signed) t - (signed) prev_timestamp;
    time += diff/100000;
  }

  prev_timestamp = t;
  return time;

#if 0
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

/*---------------------------------------------------------------------------*/
/* This function has to be called every second */
void upd_clock_seconds(void){
	clock_second++;
}

/* Set system time base. */
void clock_set_seconds(unsigned long sec){
	clock_second = sec;
}

unsigned long clock_seconds(void){
	return clock_second;
}

/*---------------------------------------------------------------------------*/
