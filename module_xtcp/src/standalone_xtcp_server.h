#ifndef __standalone_xtcp_server_h__
#define __standalone_xtcp_server_h__

#include "xccompat.h"
#include "uip_server.h"
/**  xtcp TCP/IP server.
 *
 *  \param mac_rx           Rx channel connected to ethernet server
 *  \param mac_tx           Tx channel connected to ethernet server
 *  \param xtcp             Client channel array
 *  \param num_xtcp_clients The number of clients connected to the server
 *  \param ipconfig         An data structure representing the IP config
 *                          (ip address, netmask and gateway) of the device.
 *                          Leave NULL for automatic address allocation.
 *  \param connect_status   This chanend needs to be connected to the connect
 *                          status output of the ethernet mac.
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
void
xtcp_server(chanend mac_rx,
           chanend mac_tx,
           chanend xtcp[],
           int num_xtcp_clients,
           NULLABLE_REFERENCE_PARAM(xtcp_ipconfig_t, ipconfig));

#define xtcp_server xtcp_server_uip

#endif // __standalone_xtcp_server_h__
