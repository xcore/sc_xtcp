// Copyright (c) 2011, XMOS Ltd., All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#define TRANSITION_DELAY 1000000
#include <print.h>

int get_button_startup(chanend startup) 
{
  int val;
 startup :> val;
  return val;
}

void button_monitor(chanend client, chanend startup, port keyA, port keyB) 
{
  int keyvalA = 0;
  int keyvalB = 0;
  unsigned int countA = 0;
  unsigned int countB = 0;
  timer tmr;
  unsigned int t;
  
  keyA :> keyvalA;
  keyB :> keyvalB;
  
  if (!(keyvalA&1))
    startup <: 1;
  else
    if (!(keyvalB&1))
      startup <: 2;
    else
      startup <: 0;
    
  while (1) {
    select 
      {
      case keyA when pinsneq(keyvalA) :> keyvalA:
        if ((keyvalA&1) == 0) 
          countA++;  
        tmr :> t;
        tmr when timerafter(t + TRANSITION_DELAY) :> int _;
        break;

      case keyB when pinsneq(keyvalB) :> keyvalB:
        if ((keyvalB&1) == 0) 
          countB++;  
        tmr :> t;
        tmr when timerafter(t + TRANSITION_DELAY) :> int _;
        break;
        
      case client :> int request:
        switch (request) 
          {
          case 0:
            client <: countA;
          break;
          case 1:
            client <: countB;
            break;
          }
        break;
      }
  }
}


unsigned int getButtonCount(chanend c, int b) 
{
  unsigned int val;
  c <: b;
  c :> val; 
  return val;
}
