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
 *
 **/
void mdns_xtcp_handler(chanend tcp_svr, 
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
