// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <string.h>
#include <print.h>
#include <stdio.h>

#include "uip_xtcp.h"
#include "xtcp_client.h"
#include "xtcp_server.h"
#include "xtcp_server_impl.h"

#include "uipopt.h"
#include "contiki-net.h"



#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#define DHCPC_SERVER_PORT  67
#define DHCPC_CLIENT_PORT  68

#define DHCPV6_SERVER_PORT  547
#define DHCPV6_CLIENT_PORT  546


#ifndef NULL
#define NULL ((void *) 0)
#endif

#define MAX_GUID 200
static int guid = 1;		// Globally Unique Identifier => identification
							// of a connection.

#if ((UIP_UDP_CONNS+UIP_CONNS) > MAX_GUID)
  #error "Cannot have more connections than GUIDs"
#endif

struct xtcp_cons_t{
	chanend *links;						// pointer to the chanend of the clients
	int nr;								// number of connected clients
	int prev_ifstate[MAX_XTCP_CLIENTS];	// last interface state (up/down) informed
										// to the connected application
}xtcp_cons;

#define NUM_TCP_LISTENERS 10
#define NUM_UDP_LISTENERS 10

#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UDPBUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

/**
 * \internal Structure for holding a TCP port and the link number.
 */
struct listener_info_t {
  int active;			// 1 = active, 0 = inactive
  int port_number;		// TCP port number
  int linknum;			//
};



struct listener_info_t tcp_listeners[NUM_TCP_LISTENERS] = {{0}};
struct listener_info_t udp_listeners[NUM_UDP_LISTENERS] = {{0}};

extern u16_t uip_slen;	// max length of a package

/* ****************************************************************************
 *
 * Functions to handle tcp/upd listeners
 *
 * ************************************************************************** */
__attribute__ ((noinline))
static int get_listener_linknum(struct listener_info_t listeners[],
                                int n,
                                int local_port)
{
  int i, linknum = -1;
  for (i=0;i<n;i++)
    if (listeners[i].active &&
        local_port == listeners[i].port_number) {
      linknum = listeners[i].linknum;
      break;
    }
  return linknum;
}

static void unregister_listener(struct listener_info_t listeners[],
                                int linknum,
                                int port_number,
                                int n){

  int i;
  for (i=0;i<n;i++){
    if (listeners[i].port_number == HTONS(port_number) &&
        listeners[i].active)
      {
        listeners[i].active = 0;
      }
  }
}
/* -------------------------------------------------------------------------- */
static void register_listener(struct listener_info_t listeners[],
                              int linknum,
                              int port_number,
                              int n)
{
  int i;

    for (i=0;i<n;i++)
      if (!listeners[i].active)
        break;

    if (i==n) {
      // Error: max number of listeners reached
    }
    else {
      listeners[i].active = 1;
      listeners[i].port_number = HTONS(port_number);
      listeners[i].linknum = linknum;
    }
}

/* -------------------------------------------------------------------------- */
void xtcpd_unlisten(int linknum, int port_number){
  unregister_listener(tcp_listeners, linknum, port_number, NUM_TCP_LISTENERS);
  uip_unlisten(HTONS(port_number));
}

/* -------------------------------------------------------------------------- */
void xtcpd_listen(int linknum, int port_number, xtcp_protocol_t p)
{
	switch(p){
	case XTCP_PROTOCOL_TCP:
	    register_listener(tcp_listeners, linknum, port_number, NUM_TCP_LISTENERS);
	    uip_listen(HTONS(port_number));
		break;

	case XTCP_PROTOCOL_UDP:
	    register_listener(udp_listeners, linknum, port_number, NUM_UDP_LISTENERS);
	    uip_udp_listen(HTONS(port_number));
		break;

	default:
		PRINTF("xtcpd_listen: Unknown protocol.");
		break;
	}

  return;
}
/* ************************************************************************** */


/* -----------------------------------------------------------------------------
 *
 * -------------------------------------------------------------------------- */
static struct xtcpd_state_t  *lookup_xtcpd_state(int conn_id) {
  int i=0;
  for (i=0;i<((UIP_CONNS>UIP_UDP_CONNS) ? UIP_CONNS : UIP_UDP_CONNS);i++) {
    if (i < UIP_CONNS) {
      xtcpd_state_t *s = (xtcpd_state_t *) &(uip_conns[i].appstate);
      if (s->conn.id == conn_id) return s;
    }
    if (i < UIP_UDP_CONNS) {
        xtcpd_state_t *s = (xtcpd_state_t *) &(uip_udp_conns[i].appstate);
        if (s->conn.id == conn_id) return s;
    }
  }
  return NULL;
}

/* -----------------------------------------------------------------------------
 *
 * -------------------------------------------------------------------------- */
static void init_xtcpd_state(xtcpd_state_t *s,
							 xtcp_protocol_t protocol,
							 xtcp_ipaddr_t remote_addr,
							 int local_port,
							 int remote_port,
							 void *conn) {
  int linknum;
  int connect_request = s->s.connect_request;
  int connection_type = s->conn.connection_type;

  if (connect_request) {
    linknum = s->linknum;
  } else {
    connection_type = XTCP_SERVER_CONNECTION;
    if (protocol == XTCP_PROTOCOL_TCP) {
      linknum = get_listener_linknum(tcp_listeners, NUM_TCP_LISTENERS, local_port);
    }
    else {
      linknum = get_listener_linknum(udp_listeners, NUM_UDP_LISTENERS, local_port);
    }
  }

  memset(s, 0, sizeof(xtcpd_state_t));

  // Find and use a GUID that is not being used by another connection
  while (lookup_xtcpd_state(guid) != NULL)
  {
    guid++;
    if (guid > MAX_GUID)
      guid = 1;
  }

  s->conn.connection_type = connection_type;
  s->linknum = linknum;
  s->conn.id = guid;
  s->conn.local_port = HTONS(local_port);
  s->conn.remote_port = HTONS(remote_port);
  s->conn.protocol = protocol;
  s->s.uip_conn = (int) conn;
#ifdef XTCP_ENABLE_PARTIAL_PACKET_ACK
  s->s.accepts_partial_ack = 0;
#endif
  XTCP_IPADDR_CPY(s->conn.remote_addr.u8, remote_addr.u8);
}

/* -----------------------------------------------------------------------------
 *
 * -------------------------------------------------------------------------- */
static int do_xtcpd_send(chanend c,
                  xtcp_event_type_t event,
                  xtcpd_state_t *s,
                  unsigned char data[],
                  int mss)
{
  int len;
#ifdef XTCP_ENABLE_PARTIAL_PACKET_ACK
  int outstanding=0;
  if (!uip_udpconnection()) {
    if (!s->s.accepts_partial_ack && uip_conn->len > 1)
      return 0;

    outstanding = uip_conn->len;
    if (outstanding == 1)
      outstanding = 0;
    s->conn.outstanding = outstanding;

  }
#endif

  xtcpd_service_clients_until_ready(s->linknum, xtcp_cons.links, xtcp_cons.nr);
  len = xtcpd_send(c,event,s,data,mss);

#ifdef XTCP_ENABLE_PARTIAL_PACKET_ACK
  if (!uip_udpconnection()) {
    if (outstanding != 0 &&
        len > outstanding) {
      len = len - outstanding;
      memmove((char *) uip_appdata,
              &((char *)uip_appdata)[outstanding],
              len);
    }
    else if (outstanding > 0) {
      len = 0;
    }
  }
#endif
  return len;
}

/* -----------------------------------------------------------------------------
 *
 * -------------------------------------------------------------------------- */
static void xtcpd_event(xtcp_event_type_t event,
                        xtcpd_state_t *s)
{
  if (s->linknum != -1) {
    xtcpd_service_clients_until_ready(s->linknum, xtcp_cons.links, xtcp_cons.nr);
    xtcpd_send_event(xtcp_cons.links[s->linknum], event, s);
  }
}

/* -----------------------------------------------------------------------------
 *
 * -------------------------------------------------------------------------- */
void xtcpd_bind_local(int linknum, int conn_id, int port_number)
{
  xtcpd_state_t *s = lookup_xtcpd_state(conn_id);
  s->conn.local_port = port_number;
  if (s->conn.protocol == XTCP_PROTOCOL_UDP)
    ((struct uip_udp_conn *) s->s.uip_conn)->lport = HTONS(port_number);
  else
    ((struct uip_conn *) s->s.uip_conn)->lport = HTONS(port_number);
}

/* -----------------------------------------------------------------------------
 *
 * -------------------------------------------------------------------------- */
void xtcpd_bind_remote(int linknum,
                       int conn_id,
                       xtcp_ipaddr_t addr,
                       int port_number)
{
  xtcpd_state_t *s = lookup_xtcpd_state(conn_id);
  if (s->conn.protocol == XTCP_PROTOCOL_UDP) {
    struct uip_udp_conn *conn = (struct uip_udp_conn *) s->s.uip_conn;
    s->conn.remote_port = port_number;
    conn->rport = HTONS(port_number);
    XTCP_IPADDR_CPY(s->conn.remote_addr.u8, addr.u8);
    XTCP_IPADDR_CPY(conn->ripaddr.u8, addr.u8);
//    conn->ripaddr.u16[0] = (addr[1] << 8) | addr[0];
//    conn->ripaddr.u16[1] = (addr[3] << 8) | addr[2];
  }
}

/* -----------------------------------------------------------------------------
 *
 * -------------------------------------------------------------------------- */
void xtcpd_connect(int linknum, int port_number, xtcp_ipaddr_t addr,
                   xtcp_protocol_t p) {
  uip_ipaddr_t uipaddr;
#if UIP_CONF_IPV4
  uip_ipaddr(&uipaddr, addr.u8[0], addr.u8[1], addr.u8[2], addr.u8[3]);
#elif UIP_CONF_IPV6
  uip_ip6addr(&uipaddr, addr.u16[0], addr.u16[1], addr.u16[2], addr.u16[3],
		                addr.u16[4], addr.u16[5], addr.u16[6], addr.u16[7]);
#endif /* UIP_CONF_IPVx */
  if (p == XTCP_PROTOCOL_TCP) {
    struct uip_conn *conn = uip_connect(&uipaddr, HTONS(port_number));
    if (conn != NULL) {
         xtcpd_state_t *s = (xtcpd_state_t *) &(conn->appstate);
         s->linknum = linknum;
         s->s.connect_request = 1;
         s->conn.connection_type = XTCP_CLIENT_CONNECTION;
       }
  }
  else {
    struct uip_udp_conn *conn;
    conn = uip_udp_new(&uipaddr, HTONS(port_number));
    if (conn != NULL) {
      xtcpd_state_t *s = (xtcpd_state_t *) &(conn->appstate);
      s->linknum = linknum;
      s->s.connect_request = 1;
      s->conn.connection_type = XTCP_CLIENT_CONNECTION;
    }
  }
  return;
}

/* -----------------------------------------------------------------------------
 *
 * -------------------------------------------------------------------------- */
void xtcpd_init_send(int linknum, int conn_id)
{
  xtcpd_state_t *s = lookup_xtcpd_state(conn_id);

  if (s != NULL) {
    s->s.send_request++;
  }
}

/* -------------------------------------------------------------------------- */
void xtcpd_init_send_from_uip(struct uip_conn *conn)
{
  xtcpd_state_t *s = &(conn->appstate);
  s->s.send_request++;
}

/* -------------------------------------------------------------------------- */
void xtcpd_set_appstate(int linknum, int conn_id, xtcp_appstate_t appstate)
{
  xtcpd_state_t *s = lookup_xtcpd_state(conn_id);
  if (s != NULL) {
    s->conn.appstate = appstate;
  }
}

/* -------------------------------------------------------------------------- */
void xtcpd_abort(int linknum, int conn_id)
{
  xtcpd_state_t *s = lookup_xtcpd_state(conn_id);
  if (s != NULL) {
    s->s.abort_request = 1;
  }
}

/* -------------------------------------------------------------------------- */
void xtcpd_close(int linknum, int conn_id)
{
  xtcpd_state_t *s = lookup_xtcpd_state(conn_id);
  if (s != NULL) {
    s->s.close_request = 1;
  }
}

/* -------------------------------------------------------------------------- */
void xtcpd_ack_recv_mode(int conn_id)
{
  xtcpd_state_t *s = lookup_xtcpd_state(conn_id);
  if (s != NULL) {
    s->s.ack_recv_mode = 1;
  }
}

/* -------------------------------------------------------------------------- */
void xtcpd_ack_recv(int conn_id)
{
  xtcpd_state_t *s = lookup_xtcpd_state(conn_id);
  if (s != NULL) {
    ((struct uip_conn *) s->s.uip_conn)->tcpstateflags &= ~UIP_STOPPED;
    s->s.ack_request = 1;
  }
}

/* -------------------------------------------------------------------------- */
void xtcpd_pause(int conn_id)
{
  xtcpd_state_t *s = lookup_xtcpd_state(conn_id);
  if (s != NULL) {
    ((struct uip_conn *) s->s.uip_conn)->tcpstateflags |= UIP_STOPPED;
  }
}

/* -------------------------------------------------------------------------- */
void xtcpd_unpause(int conn_id)
{
  xtcpd_state_t *s = lookup_xtcpd_state(conn_id);
  if (s != NULL) {
    ((struct uip_conn *) s->s.uip_conn)->tcpstateflags &= ~UIP_STOPPED;
    s->s.ack_request = 1;
  }
}

/* -------------------------------------------------------------------------- */
#ifdef XTCP_ENABLE_PARTIAL_PACKET_ACK
void xtcpd_accept_partial_ack(int conn_id)
{
  xtcpd_state_t *s = lookup_xtcpd_state(conn_id);
  if (s != NULL) {
    s->s.accepts_partial_ack = 1;
  }
}
#endif

/* -----------------------------------------------------------------------------
 *
 * -------------------------------------------------------------------------- */
void xtcpd_set_poll_interval(int linknum, int conn_id, int poll_interval)
{
  xtcpd_state_t *s = lookup_xtcpd_state(conn_id);
  if (s != NULL && s->conn.protocol == XTCP_PROTOCOL_UDP) {
    s->s.poll_interval = poll_interval;
    timer_set(&(s->s.tmr), poll_interval * CLOCK_SECOND/1000);
  }
}

/* -----------------------------------------------------------------------------
 *
 * -------------------------------------------------------------------------- */
static void uip_xtcpd_handle_poll(xtcpd_state_t *s)
{
 if (s->s.ack_request) {
   uip_flags |= UIP_NEWDATA;
   uip_slen = 0;
   s->s.ack_request = 0;
 }

  if (s->s.abort_request) {
    /* ----------------------------------- */
    if (uip_udpconnection()) {
      uip_udp_conn->lport = 0;
      xtcpd_event(XTCP_CLOSED, s);
    } else {
      uip_abort();
    }
    s->s.abort_request = 0;
  } else if (s->s.close_request) {
    /* ----------------------------------- */
    if (uip_udpconnection()) {
      uip_udp_conn->lport = 0;
      xtcpd_event(XTCP_CLOSED, s);
    }
    else
      uip_close();
    s->s.close_request = 0;
  } else if (s->s.connect_request) {
      /* ----------------------------------- */
      if (uip_udpconnection()) {
      init_xtcpd_state(s,
                       XTCP_PROTOCOL_UDP,
                       *((xtcp_ipaddr_t *) (&uip_udp_conn->ripaddr)),
                       uip_udp_conn->lport,
                       uip_udp_conn->rport,
                       uip_udp_conn);
      xtcpd_event(XTCP_NEW_CONNECTION, s);
      s->s.connect_request = 0;
    }
  } else if (s->s.send_request) {
     /* ----------------------------------- */
    int len;
    if (s->linknum != -1) {
      len = do_xtcpd_send(xtcp_cons.links[s->linknum],
                       XTCP_REQUEST_DATA,
                       s,
                       uip_appdata,
                       uip_udpconnection() ? XTCP_CLIENT_BUF_SIZE : uip_mss());
      uip_send(uip_appdata, len);
    }
    s->s.send_request--;
  } else if (s->s.poll_interval != 0 &&
           timer_expired(&(s->s.tmr))){
    /* ----------------------------------- */
    xtcpd_event(XTCP_POLL, s);
    timer_set(&(s->s.tmr), s->s.poll_interval);
  }
}


/* -----------------------------------------------------------------------------
 * xtcpd_appcall
 *
 * this function is called, when a package comes in for an upper layer
 * application.
 * -------------------------------------------------------------------------- */
void
xtcpd_appcall(void)
{
  xtcpd_state_t *s;

  /* --------------- DHCP (v4)  --------------- */
  if (uip_udpconnection() &&
      (uip_udp_conn->lport == HTONS(DHCPC_CLIENT_PORT) ||
       uip_udp_conn->lport == HTONS(DHCPC_SERVER_PORT))) {
#if UIP_USE_DHCP
    dhcpc_appcall();
#endif
    return;
  }

  /* --------- set up a new connection  ---------- */
  if (uip_udpconnection()){
    s = (xtcpd_state_t *) &(uip_udp_conn->appstate);
    if (uip_newdata()) {
    	// Set remote port to upper layer state
      s->conn.remote_port = HTONS(UDPBUF->srcport);
      uip_ipaddr_copy(s->conn.remote_addr.u8, BUF->srcipaddr.u8);
    }
  } else if (uip_conn == NULL) {
      // dodgy uip_conn
      return;
  } else {
    s = (xtcpd_state_t *) &(uip_conn->appstate);
  }

  /* ------ passing new connection event up to the upper xtcp layer  ---- */
  if (uip_connected()) {
    if (!uip_udpconnection()) {
      init_xtcpd_state(s,
                       XTCP_PROTOCOL_TCP,
                       *((xtcp_ipaddr_t *) (&uip_conn->ripaddr)),
                       uip_conn->lport,
                       uip_conn->rport,
                       uip_conn);
      xtcpd_event(XTCP_NEW_CONNECTION, s);
    } else {
      init_xtcpd_state(s,
                       XTCP_PROTOCOL_UDP,
                       *((xtcp_ipaddr_t *) (&uip_udp_conn->ripaddr)),
                       uip_udp_conn->lport,
                       uip_udp_conn->rport,
                       uip_udp_conn);
      xtcpd_event(XTCP_NEW_CONNECTION, s);
    }
  }

  /* --------------- new data event to deliver  --------------- */
  if (uip_newdata() && uip_len > 0) {
    if (s->linknum != -1) {
      xtcpd_service_clients_until_ready(s->linknum, xtcp_cons.links, xtcp_cons.nr);
      xtcpd_recv(xtcp_cons.links, s->linknum, xtcp_cons.nr,
                 s,
                 uip_appdata,
                 uip_datalen());
      if (!uip_udpconnection() && s->s.ack_recv_mode) {
        uip_stop();
      }
    }
  } else if (uip_aborted()) {
    xtcpd_event(XTCP_ABORTED, s);
    return;
  } else if (uip_timedout()) {
    xtcpd_event(XTCP_TIMED_OUT, s);
    return;
  }

  /* ------------ passing acknowleg event to upper layer  ------------- */
  if (uip_acked()) {
    int len;
    if (s->linknum != -1) {
      len =
        do_xtcpd_send(xtcp_cons.links[s->linknum],
                      XTCP_SENT_DATA,
                      s,
                      uip_appdata,
                      uip_udpconnection() ? XTCP_CLIENT_BUF_SIZE : uip_mss());

      uip_send(uip_appdata, len);
    }
  }

  /* -------------- retransmit the last package  -------------- */
  if (uip_rexmit()) {
    int len;
    if (s->linknum != -1) {
      xtcpd_service_clients_until_ready(s->linknum, xtcp_cons.links, xtcp_cons.nr);
#ifdef XTCP_ENABLE_PARTIAL_PACKET_ACK
      s->conn.outstanding = 0;
#endif
      len = xtcpd_send(xtcp_cons.links[s->linknum],
                       XTCP_RESEND_DATA,
                       s,
                       uip_appdata,
                       uip_udpconnection() ? XTCP_CLIENT_BUF_SIZE : uip_mss());
      if (len != 0)
        uip_send(uip_appdata, len);
    }
  }

  /* --------------- poll a connection --------------- */
  if (uip_poll()) {
    uip_xtcpd_handle_poll(s);
  }

#if XTCP_ENABLE_PUSH_FLAG_NOTIFICATION
  if (uip_tcp_push()) {
    xtcpd_event(XTCP_PUSH_DATA, s);
  }
#endif

  /* ------------- connection close event  ------------ */
  if (uip_closed()) {
    if (!s->s.closed){
      s->s.closed = 1;

      xtcpd_event(XTCP_CLOSED, s);
    }
    return;
  }

}

/* -----------------------------------------------------------------------------
 * TODO: CHSC
 * ICMP6 call
 *
 * If we receive a ICMPv6 telegram, the application could be informed. In the
 * future, here should be a mechanism like the xtcpd_appcall one, which
 * informs the application about the ICMPv6 telegrams.
 *
 * One interessting event could be Router Solicitation (133) or Neigbor
 * Solicitation (135)
 * -------------------------------------------------------------------------- */
#if UIP_CONF_IPV6
void
xtcpd_icmp6_call(uint8_t type){
	PRINTF("%s: type: %i\n", __func__, type);
	switch(type){
    case ICMP6_NA:
    case ICMP6_RA:
		uip_xtcp_up();
		break;

	default:
		break;
	}
}
#endif

/* -----------------------------------------------------------------------------
 * General stack information (set/get) functions
 * -------------------------------------------------------------------------- */
static int uip_ifstate = 0;
void xtcpd_get_ipconfig(xtcp_ipconfig_t *ipconfig)
{
#if UIP_CONF_IPV4
  ipconfig->v = 4;
  memcpy(&ipconfig->ipaddr, &uip_hostaddr, sizeof(xtcp_ipconfig_t));
  memcpy(&ipconfig->netmask, &uip_netmask, sizeof(xtcp_ipconfig_t));
  memcpy(&ipconfig->gateway, &uip_draddr, sizeof(xtcp_ipconfig_t));
#elif UIP_CONF_IPV6
  //TODO CHSC: Implement IPv6 get
  memset(ipconfig, 0, sizeof(xtcp_ipconfig_t));
  ipconfig->v = 6;

  uip_ds6_addr_t *temp_uip_ds6_addr;
  temp_uip_ds6_addr = uip_ds6_get_global(-1);

  if(temp_uip_ds6_addr != NULL)
    memcpy(&ipconfig->ipaddr, &temp_uip_ds6_addr->ipaddr, sizeof(uip_ipaddr_t));

  /* dump uip_ds6_if */
  PRINTF("Dump uip_ds6_if:\n");
  PRINTF("  link_mtu:               %i\n", uip_ds6_if.link_mtu);
  PRINTF("  cur_hop_limit:          %i\n", uip_ds6_if.cur_hop_limit);
  PRINTF("  base_reachable_time:    %i\n", uip_ds6_if.base_reachable_time);
  PRINTF("  reachable_time:         %i\n", uip_ds6_if.reachable_time);
  PRINTF("  retrans_timer:          %i\n", uip_ds6_if.retrans_timer);
  PRINTF("  maxdadns:               %i\n", uip_ds6_if.maxdadns);
  PRINTF("--ADDR LIST\n");
  for(int i=0; i<UIP_DS6_ADDR_NB; i++){
	  PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
	  PRINTF("  is used: %s\n", uip_ds6_if.addr_list[i].isused ? "yes" : "no");
  }
  PRINTF("--AADDR LIST\n");
  for(int i=0; i<UIP_DS6_AADDR_NB; i++){
	  PRINT6ADDR(&uip_ds6_if.aaddr_list[i].ipaddr);
	  PRINTF("  is used: %s\n", uip_ds6_if.aaddr_list[i].isused ? "yes" : "no");
  }
  PRINTF("--MADDR LIST\n");
  for(int i=0; i<UIP_DS6_MADDR_NB; i++){
	  PRINT6ADDR(&uip_ds6_if.maddr_list[i].ipaddr);
	  PRINTF("  is used: %s\n", uip_ds6_if.maddr_list[i].isused ? "yes" : "no");
  }

#endif
}

void xtcpd_get_mac_address(unsigned char mac_addr[]){
	memcpy(mac_addr, uip_lladdr.addr, sizeof(uip_lladdr));
}

/* -----------------------------------------------------------------------------
 * Inform the connect applications about a state change in the state
 * of the link
 * -------------------------------------------------------------------------- */
void xtcpd_uip_checkstate(void)
{
  for (int i=0;i<xtcp_cons.nr;i++) {
    if (uip_ifstate != xtcp_cons.prev_ifstate[i]) {
    	  /* queue configuration change */
    	  if (uip_ifstate) {
    	    xtcpd_queue_event(xtcp_cons.links[i], i, XTCP_IFUP);
    	  }
    	  else {
    	    xtcpd_queue_event(xtcp_cons.links[i], i, XTCP_IFDOWN);
    	  }
      xtcp_cons.prev_ifstate[i] = uip_ifstate;
    }
  }
}

/* -----------------------------------------------------------------------------
 * Interface state functions
 * -------------------------------------------------------------------------- */
void uip_xtcp_up(void) {
  uip_ifstate = 1;
}

void uip_xtcp_down(void) {
  uip_ifstate = 0;
}

int uip_xtcp_get_ifstate()
{
  return uip_ifstate;
}


/* -----------------------------------------------------------------------------
 *	IGMP functions
 * -------------------------------------------------------------------------- */
void xtcpd_join_group(xtcp_ipaddr_t addr)
{
#if UIP_IGMP
  uip_ipaddr_t ipaddr;
  uip_ipaddr(ipaddr, addr[0], addr[1], addr[2], addr[3]);
  igmp_join_group(ipaddr);
#endif
}

void xtcpd_leave_group(xtcp_ipaddr_t addr)
{
#if UIP_IGMP
  uip_ipaddr_t ipaddr;
  uip_ipaddr(ipaddr, addr[0], addr[1], addr[2], addr[3]);
  igmp_leave_group(ipaddr);
#endif
}

/* -----------------------------------------------------------------------------
 * Initialise xtcpd
 * -------------------------------------------------------------------------- */
void xtcpd_init(chanend xtcp_links_init[], int n)
{
  int i;
  xtcp_cons.links = xtcp_links_init;
  xtcp_cons.nr = n;
  uip_ifstate = 0;
  if(n>MAX_XTCP_CLIENTS)
    PRINTF("%s: Panic - to many connected applications!\n", __func__);
  for(i=0;i<MAX_XTCP_CLIENTS;i++)
    xtcp_cons.prev_ifstate[i] = 0;
  xtcpd_server_init();
}
