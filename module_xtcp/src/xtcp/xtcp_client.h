// Copyright (c) 2011, XMOS Ltd., All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef   _xtcp_client_h_
#define   _xtcp_client_h_
#include <xccompat.h>
#include "xtcp_client_conf.h"

#ifndef __XC__
typedef struct xtcp_transfer_state_t {
  unsigned char *dptr;
  unsigned char *prev_dptr;
  unsigned char *endptr;
  int len;
  int left;
  int prev_left;
  int n;
} xtcp_transfer_state_t;
#endif

typedef unsigned int xtcp_appstate_t;

typedef unsigned char xtcp_ipaddr_t[4];

typedef enum xtcp_conn_or_config_t {
  XTCP_CONN_EVENT,
  XTCP_CONFIG_EVENT
} xtcp_conn_or_config_t;


typedef struct xtcp_ipconfig_t {
  xtcp_ipaddr_t ipaddr;
  xtcp_ipaddr_t netmask;
  xtcp_ipaddr_t gateway;
} xtcp_ipconfig_t;

typedef enum xtcp_protocol_t {
  XTCP_PROTOCOL_TCP,
  XTCP_PROTOCOL_UDP
} xtcp_protocol_t;

typedef enum xtcp_config_event_t {
  XTCP_IFUP,
  XTCP_IFDOWN
} xtcp_config_event_t;

typedef enum xtcp_event_type_t {
  XTCP_NEW_CONNECTION,
  XTCP_RECV_DATA,
  XTCP_REQUEST_DATA,
  XTCP_SENT_DATA,  
  XTCP_RESEND_DATA,
  XTCP_TIMED_OUT,
  XTCP_ABORTED,
  XTCP_CLOSED,
  XTCP_POLL,
  XTCP_NULL
} xtcp_event_type_t;

typedef enum xtcp_connection_type_t {
  XTCP_CLIENT_CONNECTION,
  XTCP_SERVER_CONNECTION
} xtcp_connection_type_t;

typedef struct xtcp_connection_t {
  int id;
  xtcp_protocol_t protocol;
  xtcp_connection_type_t connection_type;
  xtcp_event_type_t event;
  xtcp_appstate_t appstate;
  xtcp_ipaddr_t remote_addr;
  unsigned int local_port;  
  unsigned int remote_port;  
  unsigned int accepted;
  unsigned int mss;
} xtcp_connection_t;


#define XTCP_IPADDR_CPY(dest, src) do { dest[0] = src[0]; \
                                        dest[1] = src[1]; \
                                        dest[2] = src[2]; \
                                        dest[3] = src[3]; \
                                      } while (0)

#define XTCP_IPADDR_CMP(a, b) (a[0] == b[0] && \
                               a[1] == b[1] && \
                               a[2] == b[2] && \
                               a[3] == b[3])

/**
 * 
 * Convert a unsigned integer representation of an ip address into
 * the xtcp_ipaddr_t type.
 * 
 */
void xtcp_uint_to_ipaddr(xtcp_ipaddr_t ipaddr, unsigned int i);

/**
 * Listen to a particular incoming port. After this call,
 * when a connection is established a XTCP_NEW_CONNECTION event is
 * signalled.
 *
 */
void xtcp_listen(chanend tcp_svr, int port_number, xtcp_protocol_t p);

/**
 * Stop listening to a particular incoming port. Applies to TCP
 * connections only.
 *
 */
void xtcp_unlisten(chanend tcp_svr, int port_number);

/**
 *
 * Try to connect to a remote port.
 * 
 */
void xtcp_connect(chanend tcp_svr, 
                  int port_number, 
                  xtcp_ipaddr_t ipaddr,
                  xtcp_protocol_t p);


/**
 * Bind the local end of a connection to a particular port
 */
void xtcp_bind_local(chanend tcp_svr, 
                     REFERENCE_PARAM(xtcp_connection_t,  conn),
                     int port_number);

/**
 * Bind the remote end of a connection to a particular port and
 * ip address. This is only valid for XTCP_PROTOCOL_UDP connections.
 */
void xtcp_bind_remote(chanend tcp_svr, 
                      REFERENCE_PARAM(xtcp_connection_t, conn), 
                      xtcp_ipaddr_t addr, int port_number);

/** 
 * Ask the tcp server for the next event. After making this call
 * you should not make any other calls to the tcp server until
 * the next event has arrived.
 */
void xtcp_ask_for_event(chanend tcp_svr);

/** 
 * Ask the tcp server for the next change of configuration event. 
 * After making this call you should not make any other
 * calls to the tcp server until
 * the next event has arrived.
 */
void xtcp_ask_for_config_event(chanend tcp_svr);


/** 
 * Ask the tcp server for the next connection event *or* 
 * change of configuration event. 
 * After making this call you should not make any other
 * calls to the tcp server until
 * the next event has arrived.
 */
void xtcp_ask_for_conn_or_config_event(chanend tcp_svr);

/**
 * Receive the next connect event. This can be used in a select statement.
 * Upon receiving the event, the xtcp_connection_t  structure conn
 * is instatiated with information of the event and the connection
 * it is on.
 */
#ifdef __XC__
transaction xtcp_event(chanend tcp_svr, xtcp_connection_t &conn);
#endif


/**
 * Receive the next connect event. This can be used in a select statement.
 * Upon receiving the event, the xtcp_ipconfign_t  structure conn
 * is instatiated with information of the event and the connection
 * it is on.
 */
#ifdef __XC__
transaction xtcp_config_event(chanend tcp_svr, 
                              xtcp_config_event_t &event,
                              xtcp_ipconfig_t &ipconfig);
#endif

/**
 * Receive the next connecttion or config event. 
 * This can be used in a select statement.
 * Upon receiving the event, the the relevant structure(s)
 * is instatiated with information of the event.
 */
#ifdef __XC__
transaction xtcp_conn_or_config_event(chanend tcp_svr, 
                                      xtcp_conn_or_config_t &event_type,
                                      xtcp_config_event_t &event,
                                      xtcp_ipconfig_t &ipconfig,
                                      xtcp_connection_t &conn);
#endif



/**
 * Initiate sending data on a connection. After making this call, the
 * server will respond with a XTCP_REQUEST_DATA event when it is
 * ready to accept data.
 */
void xtcp_init_send(chanend tcp_svr, 
                    REFERENCE_PARAM(xtcp_connection_t, conn));

/**
 * Set the connections application state data item. After this call, 
 * subsequent events on this connection will have the appstate field
 * of the connection set
 */
void xtcp_set_connection_appstate(chanend tcp_svr, 
                                  REFERENCE_PARAM(xtcp_connection_t, conn), 
                                  xtcp_appstate_t appstate);

/**
 * Close a connection. 
 */
void xtcp_close(chanend tcp_svr,
                REFERENCE_PARAM(xtcp_connection_t,conn));
/**
 * Abort a connection.
 */
void xtcp_abort(chanend tcp_svr,
                REFERENCE_PARAM(xtcp_connection_t,conn));


/** 
 * This function sends a buffer over a connection. Only part of the buffer may 
 * be sent and the function will need to be called again on subsequent 
 * XTCP_SENT_DATA or XTCP_RETRANSMIT_DATA events. The state of the
 * transfer is kept in the ts data structure. The function will
 * return 1 when it has completed sending the data.
 */

#ifndef __XC__
int xtcp_send_buffer(chanend tcp_svr,
                     REFERENCE_PARAM(xtcp_connection_t, conn),
                     REFERENCE_PARAM(xtcp_transfer_state_t, ts),
                     unsigned char data[],
                     int len);
#endif
                    
/**
 *  Receive data from the server. This should be called after a 
 *  XTCP_RECV_DATA event.
 */
int xtcp_recv(chanend tcp_svr, unsigned char data[]);


/** Send data to the server. This should be called after a 
 *  XTCP_REQUEST_DATA, XTCP_SENT_DATA or XTCP_RETRANSMIT_DATA event 
 *  (alternatively xtcp_write_buf can be called). 
 *  To finish sending this must be called with a length  of zero.
 */
void xtcp_send(chanend tcp_svr,
               unsigned char data[],
               int len);

/** Request a null event on another link.
 *  Advanced use: this command will cause another link connected to
 *  the xtcp server that has asked for an event to receive a null event.
 *  This can be used to asynchronously schedule activity over a link.
 */ 
void xtcp_request_null_event(chanend tcp_svr, int link);

/** Set the poll interval for a udp connection in seconds. If this is called
 *  then the udp connection will cause a poll event every poll_interval
 *  milliseconds.
 */
void xtcp_set_poll_interval(chanend tcp_svr,
                            REFERENCE_PARAM(xtcp_connection_t, conn),
                            int poll_interval);

/** Subscribe to a particular ip multicast group address
 */
void xtcp_join_multicast_group(chanend tcp_svr,
                               xtcp_ipaddr_t addr);

/** Unsubscribe to a particular ip multicast group address
 */
void xtcp_leave_multicast_group(chanend tcp_svr,
                               xtcp_ipaddr_t addr);

void xtcp_get_mac_address(chanend tcp_svr, unsigned char mac_addr[]);

#endif // _xtcp_client_h_

