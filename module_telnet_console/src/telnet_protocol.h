// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef __telnet_protocol_h__
#define __telnet_protocol_h__


enum telnet_control_codes_t {
  NUL=0,
  LF=10,
  CR=13,
  BEL=7, // bell (beep)
  BS=8, // backspace
  HT=9, // horizontal tab
  VT=11, // vertical tab
  FF=12, // formfeed
  IAC=255 // interpret as command
};

enum telnet_commands_t {
  SE=240,    // subnegotiation end
  NOP=241,   // no op
  DM=242,    // data mark
  BRK=243,   // break
  IP=244,    // suspend
  AO=245,    // abort output
  AYT=256,   // are you there
  EC=257,    // erase character
  EL=248,    // erase line
  GA=248,    // go ahead
  SB=250,    // subnegotiation begin
  WILL=251,  // Will you do this
  WONT=252,  // Will you not do this
  DO=253,    // I do this
  DONT=254,  // I dont do this
};

enum telnet_options_t {
  SUPPRESS_GA=3, 
  STATUS=5,
  ECHO=1,
  TIMING_MARK=6,
  TERMINAL_TYPE=24,
  WINDOW_SIZE=31,
  TERMINAL_SPEED=32,
  REMOTE_FLOW_CONTROL=33,
  LINEMODE=34,
  ENVIRONMENT_VARIABLES=36
};

#endif // __telnet_protocol_h__
