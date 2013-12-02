// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef _mac_cusom_filter_h_
#define _mac_cusom_filter_h_
#include "xtcp_conf_derived.h"

#if ETHERNET_USE_XTCP_FILTER
#define MAC_FILTER_ARPIP 0x1

inline int mac_custom_filter(unsigned int buf[])
{
  int result = 0;
  unsigned short etype = (unsigned short) buf[3];
  int qhdr = (etype == 0x0081);

  if (qhdr) {
    // has a 802.1q tag - read etype from next word
    etype = (unsigned short) buf[4];
  }

  switch (etype) {
    case 0x0608:
    case 0x0008:
      result = MAC_FILTER_ARPIP;
      break;
    default:
      break;
  }

  return result;
}
#endif
#endif
