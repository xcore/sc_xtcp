// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/*
 * This include file contains the dependencies for all defines; for example
 * DHCP requires UDP requires IPV4 requires ARP.
 */

#ifdef __pip_conf_h_exists__
#include "pip_conf.h"
#endif

#ifdef PIP_DEBUG_TCP
#ifndef PIP_ICMP
#define PIP_ICMP 1
#endif
#endif


// Application level

#ifdef PIP_TFTP
#define PIP_UDP 1
#endif

#ifdef PIP_DHCP_DONT_WAIT
#define PIP_DHCP 1
#endif

#ifdef PIP_DHCP
#define PIP_UDP 1
#endif

#ifdef PIP_LINK_LOCAL
#define PIP_ARP 1
#endif



// TCP/UDP Level

#ifdef PIP_TCP_CONNECT
#define PIP_TCP 1
#endif

#ifdef PIP_TCP
#define PIP_IPV4 1
#endif

#ifdef PIP_UDP
#define PIP_IPV4 1
#endif

#ifdef PIP_ICMP
#define PIP_IPV4 1
#endif



// IP level

#ifdef PIP_IPV4
#define PIP_ARP 1
#endif
