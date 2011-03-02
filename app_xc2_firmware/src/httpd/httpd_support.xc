// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

unsigned int getLocalTimer() {
  timer tmr;
  unsigned int t;
  tmr :> t;
  return t;
}
