// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef _mdns_h_
#define _mdns_h_
#include <xccompat.h>
#include <xtcp_client.h>

#ifndef MDNS_MAX_NAME_LENGTH
#define MDNS_MAX_NAME_LENGTH             100
#endif

#ifndef MDNS_NUM_TABLE_ENTRIES
#define MDNS_NUM_TABLE_ENTRIES            5
#endif

/** Result codes for the mdns event hander
 *
 *  This describes events that can be returned by the TCP handler.
 */
typedef unsigned mdns_event;

#define mdns_ifup (0x1)				//!< The TCP interface has come up, and mdns has started
#define mdns_ifdown (0x2)			//!< The TCP interface has gone down, no futher activity possible
#define mdns_entry_lost (0x4)		//!< We have had to change our name because of a name clash
#define mdns_entry_active (0x8)		//!< One of our entries is now considered active
#define mdns_name_error	(0x10)		//!< An error decoding a name has occurred

/** Initialize Zeroconf.
 *
 *  This function should be called before any other mdns functions.
 *
 *  \param tcp_svr chanend connected to the xtcp server
 *
 **/
void mdns_init(chanend tcp_svr);

/** Handle a mdns TCP/IP event
 *
 *  This function will process a tcp/ip event (following a call
 *  to xtcp_event()). If the related connection is an MDNS connection
 *  it will handle the packet and set the connection event to
 *  ``ALREADY_HANDLED``.
 *
 *  \param tcp_svr chanend connected to the TCP/IP server.
 *  \param conn    the connection data structure filled in by xtcp_event()
 *  \return        one of several constants describing events that have occurred
 *
 **/
mdns_event mdns_xtcp_handler(chanend tcp_svr,
                       REFERENCE_PARAM(xtcp_connection_t, conn));

/** Register the canonical name for the host.
 *
 *  This function registers the canonical name (i.e. the name
 *  returned as response to PTR requests) for the host.
 *
 *  \param name the name to register
 *
 **/
void mdns_register_canonical_name(char name[]);

/** Register a name for the host.
 *
 *  This function registers a name for the host. After this call
 *  the zeroconf protocol will try and reserve this name. After that point
 *  you can refer to the host on the network with this name.
 *
 *  If the name is already reserved by another node on the network, a
 *  unique number will be added to the name. For example, if the name
 *  "xc2.local" was requested and already exists on the network. The
 *  library will try and reserver "xc2-1.local" and then "xc2-2.local" etc.
 *
 *  \param name the name to register
 *
 **/
void mdns_register_name(char name[]);

/** Register a Zeroconf service.
 *
 *  This function registers a service to advertise.
 *
 *  \param name a human readable name of the service instance
 *              e.g. "My Web Server"
 *  \param srv_type the type of the service "_http._tcp"
 *  \param srv_port the port the service is to be advertised on
 *  \param txt the associated TXT record of the service.
 *
 **/
void mdns_register_service(char name[], char srv_type[],
                           int srv_port, char txt[]);


#endif // _mdns_h_
