// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/**
 * \addtogroup uipopt
 * @{
 */

/**
 * \name Project-specific configuration options
 * @{
 *
 * uIP has a number of configuration options that can be overridden
 * for each project. These are kept in a project-specific uip-conf.h
 * file and all configuration names have the prefix UIP_CONF.
 */

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * $Id: uip-conf.h,v 1.6 2006/06/12 08:00:31 adam Exp $
 */

/**
 * \file
 *         An example uIP configuration file
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __UIP_CONF_H__
#define __UIP_CONF_H__

#include <stdint.h>

#include "xtcp_conf_derived.h"
#include "xtcp_client.h"

#ifndef XTCP_CLIENT_BUF_SIZE
#define XTCP_CLIENT_BUF_SIZE (1472)
#endif

/**
 * 8 bit datatype
 *
 * This typedef defines the 8-bit type used throughout uIP.
 *
 * \hideinitializer
 */
typedef uint8_t u8_t;

/**
 * 16 bit datatype
 *
 * This typedef defines the 16-bit type used throughout uIP.
 *
 * \hideinitializer
 */
typedef uint16_t u16_t;

/**
 * 32 bit datatype
 *
 * This typedef defines the 32-bit type used throughout uIP.
 *
 * \hideinitializer
 */
typedef uint32_t u32_t;

/**
 * Statistics datatype
 *
 * This typedef defines the dataype used for keeping statistics in
 * uIP.
 *
 * \hideinitializer
 */
typedef unsigned short uip_stats_t;

/**
 * Maximum number of TCP connections.
 *
 * \hideinitializer
 */
#ifndef UIP_CONF_MAX_CONNECTIONS
#define UIP_CONF_MAX_CONNECTIONS 10
#endif

/**
 * Maximum number of listening TCP ports.
 *
 * \hideinitializer
 */
#ifndef UIP_CONF_MAX_LISTENPORTS
#define UIP_CONF_MAX_LISTENPORTS 10
#endif

/**
 * uIP buffer size.
 *
 * \hideinitializer
 */
#define UIP_CONF_BUFFER_SIZE     (XTCP_CLIENT_BUF_SIZE + UIP_LLH_LEN + UIP_TCPIP_HLEN)

/**
 * CPU byte order.
 *
 * \hideinitializer
 */
#define UIP_CONF_BYTE_ORDER      LITTLE_ENDIAN

/**
 * Logging on or off
 *
 * \hideinitializer
 */
#ifndef UIP_CONF_LOGGING
#define UIP_CONF_LOGGING        0
#endif

#define UIP_CONF_BROADCAST		1

/**
 * UDP support on or off
 *
 * \hideinitializer
 */
#define UIP_CONF_UDP             1

/**
 * UDP checksums on or off
 *
 * \hideinitializer
 */
#define UIP_CONF_UDP_CHECKSUMS   0

/**
 * uIP statistics on or off
 *
 * \hideinitializer
 */
#ifndef UIP_CONF_STATISTICS
#define UIP_CONF_STATISTICS      0
#endif

#if !defined(UIP_CONF_RECEIVE_WINDOW) && defined(XTCP_MAX_RECEIVE_SIZE)
#define UIP_CONF_RECEIVE_WINDOW XTCP_MAX_RECEIVE_SIZE
#endif


#define UIP_CONF_EXTERNAL_BUFFER     1

#ifndef UIP_USE_AUTOIP
#define UIP_USE_AUTOIP           0
#endif

#ifndef UIP_USE_DHCP
#define UIP_USE_DHCP             0
#endif

#ifndef UIP_IGMP
#define UIP_IGMP                 0
#endif

#ifndef UIP_CONF_ICMP6
#define UIP_CONF_ICMP6           1
#endif

#ifndef RIMEADDR_CONF_SIZE
#define RIMEADDR_CONF_SIZE       8
#endif

#include "xtcp_server.h"

typedef struct xtcpd_state_t uip_tcp_appstate_t;
typedef struct xtcpd_state_t uip_udp_appstate_t;

void xtcpd_appcall(void);
/* UIP_APPCALL: the name of the application function. This function
   must return void and take no arguments (i.e., C type "void
   appfunc(void)"). */
#ifndef UIP_APPCALL
#define UIP_APPCALL     xtcpd_appcall
#endif

#ifndef UIP_UDP_APPCALL
#define UIP_UDP_APPCALL xtcpd_appcall
#endif

void xtcpd_icmp6_call(uint8_t type);
#ifndef UIP_ICMP6_APPCALL
#define UIP_ICMP6_APPCALL xtcpd_icmp6_call
#endif

#if XTCP_ENABLE_PARTIAL_PACKET_ACK
#define UIP_CONF_SLIDING_WINDOW 1
#endif

/* ARP is not used in IPv6  */
#define UIP_CONF_ARPTAB_SIZE		1

/*Boolean flags*/
#define UIP_CONF_IPV6				1
#define UIP_CONF_IPV6_CHECKS		1
#define UIP_CONF_IPV6_QUEUE_PKT		0
#define UIP_CONF_IPV6_REASSEMBLY	0
/*Integer flags*/
#define UIP_NETIF_MAX_ADDRESSES 	3
#define UIP_ND6_MAX_PREFIXES   		3
#define UIP_ND6_MAX_NEIGHBORS   	4
#define UIP_ND6_MAX_DEFROUTER  		2

/* Aligne the uIP buffer. The problem is, that
 * if Ethernet is used, the Ethernet header
 * forces to shift the buffer by two bytes, otherwise
 * the following data structs are no more 32bit aligned. */
#define BUF_32BIT_ALIGN				1	// CHSC XMOS 2013

/*
 * Override uip_add32 (32-bit addition)
 */
#define UIP_ARCH_ADD32 							1

/*
 * Override Contiki's checksum mechanisms by architecture-specific ones
 */
#define UIP_ARCH_CHKSUM 						1

/*
 * If the RPL protocol is needed, enable it here.
 *
 * rfc6550 - IPv6 Routing Protocol for Low-Power and Lossy Networks
 */
#define UIP_CONF_IPV6_RPL                       0

/* Here we include the header file for the application(s) we use in
   our project. */
/*#include "smtp.h"*/
/*#include "hello-world.h"*/
/*#include "telnetd.h"*/
/*#include "webserver.h"*/
/*#include "dhcpc.h" */
/*#include "resolv.h"*/
/*#include "webclient.h"*/

#endif /* __UIP_CONF_H__ */

/** @} */
/** @} */
