/**
 * Module:  app_xc2_firmware
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    buttons.xc
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
