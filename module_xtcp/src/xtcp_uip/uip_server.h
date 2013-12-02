// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef _uip_server_h_
#define _uip_server_h_
#include "xtcp_client.h"

/**  uIP based xtcp server.
 *
 *  \param mac_rx           Rx channel connected to ethernet server
 *  \param mac_tx           Tx channel connected to ethernet server
 *  \param xtcp             Client channel array
 *  \param num_xtcp_clients The number of clients connected to the server
 *  \param ipconfig         An data structure representing the IP config
 *                          (ip address, netmask and gateway) of the device.
 *                          Leave NULL for automatic address allocation.
 *
 *  This function implements an xtcp tcp/ip server in a logical core.
 *  It uses a port of the uIP stack which is then interfaces over the
 *  xtcp channel array.
 *
 *  The IP setup is based on the ipconfig parameter. If this
 *  parameter is NULL then an automatic IP address is found (using dhcp or
 *  ipv4 link local addressing if no dhcp server is present). Otherwise
 *  it uses the ipconfig structure to allocate a static ip address.
 *
 *  The clients can communicate with the server using the API found
 *  in xtcp_client.h
 *
 *  \sa  xtcp_event()
 **/
#ifdef __XC__
void
xtcp_server_uip(chanend mac_rx,
                chanend mac_tx,
                chanend xtcp[],
                int num_xtcp_clients,
                xtcp_ipconfig_t &?ipconfig);
#else
void xtcp_server_uip(chanend mac_rx,
                     chanend mac_tx,
                     chanend xtcp[],
                     int num_xtcp_clients,
                     xtcp_ipconfig_t *ipconfig);
#endif

#define uip_server xtcp_server_uip

#endif // _uip_server_h_
