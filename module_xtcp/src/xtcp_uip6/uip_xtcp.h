// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef _UIP_XTCP_H_
#define _UIP_XTCP_H_

#include <xccompat.h>
#include <stdint.h>

void xtcpd_uip_checkstate(void);
void uip_xtcp_up(void);
void uip_xtcp_down(void);
void uip_xtcp_checklink(chanend connect_status);
int uip_xtcp_get_ifstate(void);
void uip_linkdown(void);
void uip_linkup(void);
void uip_xtcp_null_events(void);

/**
 * \brief Output packet to layer 2
 * The eventual parameter is the MAC address of the destination.
 */
#if UIP_CONF_IPV6
uint8_t xtcpip_output(uip_lladdr_t *, chanend mac_tx);
#else
uint8_t xtcpip_output(chanend mac_tx);
#endif

/**
 * \brief This function does address resolution and then calls tcpip_output
 */
#if UIP_CONF_IPV6
void xtcpip_ipv6_output(chanend mac_tx);
#endif

#endif // _UIP_XTCP_H_
