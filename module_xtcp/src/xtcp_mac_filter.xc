// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include "xtcp_mac_filter.h"
#if ETHERNET_USE_XTCP_FILTER
extern int mac_custom_filter(unsigned int buf[]);
#endif
