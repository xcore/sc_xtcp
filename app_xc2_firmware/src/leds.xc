/**
 * Module:  app_xc2_firmware
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    leds.xc
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
#include <print.h>
void flash_leds(chanend c, port led0, port led1) 
{
  while (1) {
    int ledNum;
    int val;
    c :> ledNum;
    c :> val;
    switch (ledNum) 
      {
      case 0:
        led0 <: val;
        break;
      case 1:
        led1 <: val;
        break;
      default:
        break;
      }
  }
}
#define MS 100000

#define FLASH(x,y) x,y,1,150*MS,x,y,0,25*MS

#define FLASH_LEFT \
0,0,1,0,\
0,1,1,0,\
1,0,1,0,\
1,1,1,250*MS,\
0,0,0,0,\
0,1,0,0,\
1,0,0,0,\
1,1,0,50*MS

#define FLASH_RIGHT \
2,0,1,0,\
2,1,1,0,\
3,0,1,0,\
3,1,1,250*MS,\
2,0,0,0,\
2,1,0,0,\
3,0,0,0,\
3,1,0,50*MS

int pattern1[] = 
  {   
    FLASH(0,1),
    FLASH(0,0),
    FLASH(1,1),
    FLASH(1,0),
    FLASH(2,1),
    FLASH(2,0),
    FLASH(3,0),
    FLASH(3,1),
    FLASH(0,1),
    FLASH(0,0),
    FLASH(1,1),
    FLASH(1,0),
    -1
  };

int pattern2[] = 
  {   
    FLASH_LEFT,
    FLASH_RIGHT,
    FLASH_LEFT,
    FLASH_RIGHT,
    -1
  };


unsigned int pattern_next_val(unsigned int pattern, unsigned int &index) 
{
  unsigned int val;
  switch (pattern) 
    {
    case 1:
      val = pattern1[index];
      index++;
      break;
    case 2:
      val = pattern2[index];
      index++;
      break;
    default:
      break;
  }
  return val;
}

typedef enum {
  LED_SET_PATTERN,
  LED_SET_CONNECTED
} led_cmd;


static void handleCmd(chanend client,
                      int cmd,
                      int &pattern,      
                      unsigned int &cmdIndex,
                      unsigned int &nextEvent,
                      unsigned int &connected,
                      chanend led0,
                      chanend led1,
                      chanend led2,
                      chanend led3)
{
  timer tmr;
  switch (cmd) 
    {
    case LED_SET_PATTERN:
      {
        int newPattern;
        client :> newPattern;
        if (pattern == -1 && newPattern >= 1 && newPattern <= 2) {
          led1 <: 0;
          led1 <: 0;
          led2 <: 1;
          led2 <: 0;
          pattern = newPattern;
          cmdIndex = 0;
          tmr :> nextEvent;
        }
        break;
      }
    case LED_SET_CONNECTED:
      client :> connected;
      if (connected) {
        led2 <: 1;
        led2 <: 1;
      }
      else {
        led2 <: 1;
        led2 <: 0;
      }
      break;
    default:
      break;
    }
  return;
}

void led_server(chanend client0,
                chanend client1,
                chanend led0,
                chanend led1,
                chanend led2,
                chanend led3)
{
  timer tmr;
  unsigned int nextEvent;
  int pattern;
  unsigned int cmdIndex;
  unsigned int connected=0;
  
  pattern = 1;
  cmdIndex = 0;
  tmr :> nextEvent;
 
  while (1) {
    select {
    case client0 :> int cmd:
      handleCmd(client0, cmd, pattern, cmdIndex, nextEvent,
                connected, led0, led1, led2, led3);
      break;
    case client1 :> int cmd:
      handleCmd(client1, cmd, pattern, cmdIndex, nextEvent,
                connected, led0, led1, led2, led3);
      break;
    case (pattern != -1) => tmr when timerafter(nextEvent) :> int _:
      {
        unsigned int timerInc;
        unsigned int link;
        unsigned int led;
        unsigned int val;       

        link = pattern_next_val(pattern, cmdIndex);
        if (link == -1) {
          led1 <: 0;
          led1 <: 1;
          if (connected) {
            led2 <: 1;
            led2 <: 1;
          }
          else {
            led2 <: 1;
            led2 <: 0;
          }
          pattern = -1;          
        }
        else {
          led = pattern_next_val(pattern, cmdIndex);
          val = pattern_next_val(pattern, cmdIndex);
          timerInc = pattern_next_val(pattern, cmdIndex);
          nextEvent += timerInc;

          switch (link) 
            {
            case 0:
              led0 <: led;
              led0 <: val;
              break;
            case 1:
              led1 <: led;
              led1 <: val;
              break;
            case 2:
              led2 <: led;
              led2 <: val;
              break;
            case 3:
              led3 <: led;
              led3 <: val;
              break;
            }

        }
        break;
      }
    }
  }
}

void led_pattern(chanend led_svr, int pattern) 
{
  led_svr <: LED_SET_PATTERN;
  led_svr <: pattern;
}

void led_set_connected(chanend led_svr, int val) 
{
  led_svr <: LED_SET_CONNECTED;
  led_svr <: val;
}
