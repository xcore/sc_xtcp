// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/*
 * This include file should contain the dependencies between all defines
 * (for example DHCP requires UDP requires IPV4 requires ARP), and the
 * defaults for undefined defines for all defines.
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
#define PIP_TCP PIP_TCP_CONNECT
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



#ifndef PIP_TCP_BUFSIZE_RX
#define PIP_TCP_BUFSIZE_RX 512
#endif

#ifndef PIP_TCP_BUFSIZE_TX
#define PIP_TCP_BUFSIZE_TX 512
#endif

#if (PIP_TCP_BUFSIZE_RX & (PIP_TCP_BUFSIZE_RX - 1)) != 0
#error "PIP_TCP_BUFSIZE_RX must be a power of 2"
#endif

#if (PIP_TCP_BUFSIZE_TX & (PIP_TCP_BUFSIZE_TX - 1)) != 0
#error "PIP_TCP_BUFSIZE_TX must be a power of 2"
#endif

#ifndef PIP_UDP_CHANNELS
#define PIP_UDP_CHANNELS 0
#endif


#ifndef PIP_ETHTX_WORDS
#define PIP_ETHTX_WORDS 200
#endif

#ifndef PIP_ETHRX_WORDS
#define PIP_ETHRX_WORDS 3200
#endif


#ifdef PIP_ARP
#if PIP_ETHTX_WORDS < 22
#define PIP_ETHTX_WORDS 22
#warning "Warning, PIP_ETHTX_WORDS increased to 22 to fit ARP"
#endif
#endif

#ifdef PIP_DHCP
#if PIP_ETHTX_WORDS < 64
#define PIP_ETHTX_WORDS 64
#warning "Warning, PIP_ETHTX_WORDS increased to 64 to fit DHCP"
#endif
#endif
