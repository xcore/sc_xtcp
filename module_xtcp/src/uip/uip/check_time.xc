#include "print.h"
timer tmr;

int time;

void check_time1()
{
 tmr :> time;
}

void check_time2()
{
  int now;
 tmr :> now;
  if ((now - time) > 10000)
    printstr("!");
}
