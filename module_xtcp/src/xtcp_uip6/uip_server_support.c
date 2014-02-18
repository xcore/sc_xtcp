// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xccompat.h>
#include <string.h>
#include <stdio.h>

#include "uip_server_support.h"
#include "uip_arp.h"
#include "uip_xtcp.h"

#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "sys/process.h"
#include "net/rime/rimeaddr.h"
#include "uip.h"

#include "contiki-net.h"
#include "net/uip-split.h"
#include "net/uip-packetqueue.h"

#if UIP_CONF_IPV6
#include "net/uip-nd6.h"
#include "net/uip-ds6.h"
#endif /* UIP_CONF_IPV6 */

#if UIP_CONF_IPV6_RPL
#include "rpl/rpl.h"
#endif /* UIP_CONF_IPV6_RPL */

#include "xcoredev.h"

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#if UIP_LOGGING == 1
#include <stdio.h>
void uip_log(char *msg);
#define UIP_LOG(m) uip_log(m)
#else
#define UIP_LOG(m)
#endif

#if !defined(UIP_CONF_IPV4) && !defined(UIP_CONF_IPV6)
#error Unkown IP protocoll version
#endif

/* -----------------------------------------------------------------------------
 * This is the buffer where TCP constructs its packets
 * -------------------------------------------------------------------------- */
// unsigned int uip_buf32[(UIP_BUFSIZE + 5) >> 2]; //XXX chsc: XMOS original.
//#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

uip_buf_t uip_aligned_buf;

#define BUF ((struct uip_eth_hdr *)&uip_buf16(0))
#define TCPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

#if !BUF_32BIT_ALIGN //ORIGINAL
#define UIP_ICMP_BUF ((struct uip_icmp_hdr *)&uip_buf[UIP_LLIPH_LEN + uip_ext_len])
#define UIP_IP_BUF ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_TCP_BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
#else
#define UIP_ICMP_BUF ((struct uip_icmp_hdr *)&uip_buf32(UIP_LLIPH_LEN + uip_ext_len))
#define UIP_IP_BUF ((struct uip_ip_hdr *)&uip_buf32(UIP_LLH_LEN))
#define UIP_TCP_BUF ((struct uip_tcpip_hdr *)&uip_buf32(UIP_LLH_LEN))
#endif

#ifdef XTCP_VERBOSE_DEBUG
#if UIP_CONF_IPV4
__attribute__ ((noinline)) void uip_printip4(const uip_ipaddr_t ip4) {
	printf("%i.%i.%i.%i\n",uip_ipaddr1(ip4), uip_ipaddr2(ip4), uip_ipaddr3(ip4), uip_ipaddr4(ip4));
}
#endif
#endif


int uip_static_ip = 0;
xtcp_ipconfig_t uip_static_ipconfig;
static int dhcp_done = 0;

/* -----------------------------------------------------------------------------
 * Functions prototypes
 * -------------------------------------------------------------------------- */



/* -----------------------------------------------------------------------------
 *
 * Function implementation
 *
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 *
 * -------------------------------------------------------------------------- */


void xtcp_tx_buffer(chanend mac_tx) {
	uip_split_output(mac_tx);
	uip_len = 0;
}


static int needs_poll(xtcpd_state_t *s)
{
  return (s->s.connect_request | s->s.send_request | s->s.abort_request | s->s.close_request | s->s.ack_request);
}

static int uip_conn_needs_poll(struct uip_conn *uip_conn)
{
  xtcpd_state_t *s = (xtcpd_state_t *) &(uip_conn->appstate);
  return needs_poll(s);
}

static int uip_udp_conn_needs_poll(struct uip_udp_conn *uip_udp_conn)
{
  xtcpd_state_t *s = (xtcpd_state_t *) &(uip_udp_conn->appstate);
  return needs_poll(s);
}

/* -----------------------------------------------------------------------------
 *
 * -------------------------------------------------------------------------- */

void xtcpd_check_connection_poll(chanend mac_tx)
{
	for (int i = 0; i < UIP_CONNS; i++) {
		if (uip_conn_needs_poll(&uip_conns[i])) {
			uip_poll_conn(&uip_conns[i]);
#if UIP_CONF_IPV6
            xtcpip_ipv6_output(mac_tx);
#else /* UIP_CONF_IPV6 */
            if (uip_len > 0) {
                uip_arp_out( NULL);
                xtcp_tx_buffer(mac_tx);
            }
#endif /* UIP_CONF_IPV6 */
		}
	}

	for (int i = 0; i < UIP_UDP_CONNS; i++) {
		if (uip_udp_conn_needs_poll(&uip_udp_conns[i])) {
			uip_udp_periodic(i);
#if UIP_CONF_IPV6
             xtcpip_ipv6_output(mac_tx);
#else
            if (uip_len > 0) {
                uip_arp_out(&uip_udp_conns[i]);
                xtcp_tx_buffer(mac_tx);
            }
#endif
		}
	}
}

void xtcp_process_udp_acks(chanend mac_tx)
{
	for (int i = 0; i < UIP_UDP_CONNS; i++) {
		if (uip_udp_conn_has_ack(&uip_udp_conns[i])) {
			uip_udp_ackdata(i);
			if (uip_len > 0) {
#if UIP_CONF_IPV4
				uip_arp_out(&uip_udp_conns[i]);
				xtcp_tx_buffer(mac_tx);
#endif
#if UIP_CONF_IPV6
// #warning "Implementation is missing"
#endif
			}
		}
	}
}

/* -----------------------------------------------------------------------------
 * \brief      Deliver an incoming packet to the TCP/IP stack
 *
 *             This function is called by theServer to
 *             deliver an incoming packet to the TCP/IP stack. The
 *             incoming packet must be present in the uip_buf buffer,
 *             and the length of the packet must be in the global
 *             uip_len variable.
 * -------------------------------------------------------------------------- */
void xtcpip_input(chanend mac_tx)
{
/*_______________*/
#if UIP_CONF_IPV4 /* ORIGINAL_XMOS */
	if (BUF->type == htons(UIP_ETHTYPE_IP)) {
		uip_arp_ipin();
		uip_input();
		if (uip_len > 0) {
			if (uip_udpconnection()
				&& (TCPBUF->proto != UIP_PROTO_ICMP)
				&& (TCPBUF->proto != UIP_PROTO_IGMP))
				uip_arp_out( uip_udp_conn);
			else
				uip_arp_out( NULL);
			xtcp_tx_buffer(mac_tx);
		}
	} else if (BUF->type == htons(UIP_ETHTYPE_ARP)) {
		uip_arp_arpin();

		if (uip_len > 0) {
			xtcp_tx_buffer(mac_tx);
		}
		for (int i = 0; i < UIP_UDP_CONNS; i++) {
			uip_udp_arp_event(i);
			if (uip_len > 0) {
				uip_arp_out(&uip_udp_conns[i]);
				xtcp_tx_buffer(mac_tx);
			}
		}
	}
#endif /* UIP_CONF_IPV4 ORIGINAL_XMOS */
/*_______________*/
	/* contiki tcpip.c */
#if UIP_CONF_IP_FORWARD
  if(uip_len > 0) {
    tcpip_is_forwarding = 1;
    if(uip_fw_forward() == UIP_FW_LOCAL) {
      tcpip_is_forwarding = 0;
      check_for_tcp_syn();
      uip_input();
      if(uip_len > 0) {
#if UIP_CONF_TCP_SPLIT
        uip_split_output(mac_tx);
#else /* UIP_CONF_TCP_SPLIT */
#if UIP_CONF_IPV6
        xtcpip_ipv6_output(mac_tx);
#else
	PRINTF("tcpip packet_input forward output len %d\n", uip_len);
        xtcpip_output(mac_tx);
#endif
#endif /* UIP_CONF_TCP_SPLIT */
      }
    }
    tcpip_is_forwarding = 0;
  }
#else /* UIP_CONF_IP_FORWARD */
  if(uip_len > 0) {
    uip_input();
    if(uip_len > 0) {
#if UIP_CONF_TCP_SPLIT
      uip_split_output(mac_tx);
#else /* UIP_CONF_TCP_SPLIT */
#if UIP_CONF_IPV6
      xtcpip_ipv6_output(mac_tx);
#else
      PRINTF("tcpip packet_input output len %d\n", uip_len);
      tcpip_output();
#endif
#endif /* UIP_CONF_TCP_SPLIT */
    }
  }
#endif /* UIP_CONF_IP_FORWARD */
}


/* -----------------------------------------------------------------------------
 * Output packet to layer 2
 * The eventual parameter is the MAC address of the destination.
 * -------------------------------------------------------------------------- */
#if UIP_CONF_IPV6
uint8_t
xtcpip_output(uip_lladdr_t *lladdr, chanend mac_tx)
{
  /*
   * If L3 dest is multicast, build L2 multicast address
   * as per RFC 2464 section 7
   * else fill with the addrsess in argument
   */
  if(lladdr == NULL) {
	/* the dest must be multicast */
	  (&BUF->dest)->addr[0] = 0x33;
	  (&BUF->dest)->addr[1] = 0x33;
	  (&BUF->dest)->addr[2] = UIP_TCP_BUF->destipaddr.u8[12];
	  (&BUF->dest)->addr[3] = UIP_TCP_BUF->destipaddr.u8[13];
	  (&BUF->dest)->addr[4] = UIP_TCP_BUF->destipaddr.u8[14];
	  (&BUF->dest)->addr[5] = UIP_TCP_BUF->destipaddr.u8[15];
  } else {
	  memcpy(&BUF->dest, lladdr, UIP_LLADDR_LEN);
  }

  memcpy(&BUF->src, &uip_lladdr, UIP_LLADDR_LEN);
  BUF->type = UIP_HTONS(UIP_ETHTYPE_IPV6);

  uip_len += sizeof(struct uip_eth_hdr);
  xcoredev_send(mac_tx);
  return 0;
}
#endif

#if UIP_CONF_IPV4
uint8_t
xtcpip_output(chanend mac_tx){

}
#endif /* UIP_CONF_IPV4 */

/* -----------------------------------------------------------------------------
 * This function does address resolution and then calls xtcpip_output
 * ---------------------------------------------------------------------------*/
#if UIP_CONF_IPV6
void
xtcpip_ipv6_output(chanend mac_tx)
{
	uip_ds6_nbr_t *nbr = NULL;
	uip_ipaddr_t *nexthop;

	if(uip_len == 0) {
		return;
	}

	if(uip_len > UIP_LINK_MTU) {
		UIP_LOG("tcpip_ipv6_output: Packet to big");
		uip_len = 0;
		return;
	}

	if(uip_is_addr_unspecified(&UIP_IP_BUF->destipaddr)){
			UIP_LOG("tcpip_ipv6_output: Destination address unspecified");
		uip_len = 0;
		return;
	}

	if(!uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)) {
		/* Next hop determination */
		nbr = NULL;

		/* We first check if the destination address is on our immediate
			link. If so, we simply use the destination address as our
			nexthop address. */
		if(uip_ds6_is_addr_onlink(&UIP_IP_BUF->destipaddr)){
			nexthop = &UIP_IP_BUF->destipaddr;
		} else {
			uip_ds6_route_t *route;
			/* Check if we have a route to the destination address. */
			route = uip_ds6_route_lookup(&UIP_IP_BUF->destipaddr);

			/* No route was found - we send to the default route instead. */
			if(route == NULL) {
				PRINTF("tcpip_ipv6_output: no route found, using default route\n");
				nexthop = uip_ds6_defrt_choose();
				if(nexthop == NULL) {
#ifdef UIP_FALLBACK_INTERFACE
					PRINTF("FALLBACK: removing ext hdrs & setting proto %d %d\n",
							uip_ext_len, *((uint8_t *)UIP_IP_BUF + 40));
					if(uip_ext_len > 0) {
						extern void remove_ext_hdr(void);
						uint8_t proto = *((uint8_t *)UIP_IP_BUF + 40);
						remove_ext_hdr();
						/* This should be copied from the ext header... */
						UIP_IP_BUF->proto = proto;
					}
					UIP_FALLBACK_INTERFACE.output();
#else
					PRINTF("tcpip_ipv6_output: Destination off-link but no route\n");
#endif /* !UIP_FALLBACK_INTERFACE */
					uip_len = 0;
					return;
				}

			} else {
				/* A route was found, so we look up the nexthop neighbor for
				   the route. */
				nexthop = uip_ds6_route_nexthop(route);

				/* If the nexthop is dead, for example because the neighbor
				   never responded to link-layer acks, we drop its route. */
				if(nexthop == NULL) {
#if UIP_CONF_IPV6_RPL
					/* If we are running RPL, and if we are the root of the
					* network, we'll trigger a global repair berfore we remove
					* the route. */
					rpl_dag_t *dag;
					rpl_instance_t *instance;

					dag = (rpl_dag_t *)route->state.dag;
					if(dag != NULL) {
						instance = dag->instance;

						rpl_repair_root(instance->instance_id);
					}
#endif /* UIP_CONF_RPL */
					uip_ds6_route_rm(route);

					/* We don't have a nexthop to send the packet to, so we drop
					* it. */
					return;
				}
			}
#if TCPIP_CONF_ANNOTATE_TRANSMISSIONS
			if(nexthop != NULL) {
				static uint8_t annotate_last;
				static uint8_t annotate_has_last = 0;

				if(annotate_has_last) {
					printf("#L %u 0; red\n", annotate_last);
				}
				printf("#L %u 1; red\n", nexthop->u8[sizeof(uip_ipaddr_t) - 1]);
				annotate_last = nexthop->u8[sizeof(uip_ipaddr_t) - 1];
				annotate_has_last = 1;
			}
#endif /* TCPIP_CONF_ANNOTATE_TRANSMISSIONS */
		}

		/* End of next hop determination */

#if UIP_CONF_IPV6_RPL
		if(rpl_update_header_final(nexthop)) {
			uip_len = 0;
			return;
		}
#endif /* UIP_CONF_IPV6_RPL */
		nbr = uip_ds6_nbr_lookup(nexthop);
		if(nbr == NULL) {
#if UIP_ND6_SEND_NA
			if((nbr = uip_ds6_nbr_add(nexthop, NULL, 0, NBR_INCOMPLETE)) == NULL) {
				uip_len = 0;
				return;
			} else {
#if UIP_CONF_IPV6_QUEUE_PKT
				/* Copy outgoing pkt in the queuing buffer for later transmit. */
				if(uip_packetqueue_alloc(&nbr->packethandle, UIP_DS6_NBR_PACKET_LIFETIME) != NULL) {
					memcpy(uip_packetqueue_buf(&nbr->packethandle), UIP_IP_BUF, uip_len);
					uip_packetqueue_set_buflen(&nbr->packethandle, uip_len);
				}
#endif
				/* RFC4861, 7.2.2:
				* "If the source address of the packet prompting the solicitation is the
				* same as one of the addresses assigned to the outgoing interface, that
				* address SHOULD be placed in the IP Source Address of the outgoing
				* solicitation.  Otherwise, any one of the addresses assigned to the
				* interface should be used."*/
				if(uip_ds6_is_my_addr(&UIP_IP_BUF->srcipaddr)){
					uip_nd6_ns_output(&UIP_IP_BUF->srcipaddr, NULL, &nbr->ipaddr);
				} else {
					uip_nd6_ns_output(NULL, NULL, &nbr->ipaddr);
				}

				stimer_set(&nbr->sendns, uip_ds6_if.retrans_timer / 1000);
				nbr->nscount = 1;
			}
#endif /* UIP_ND6_SEND_NA */
		} else {
#if UIP_ND6_SEND_NA
			if(nbr->state == NBR_INCOMPLETE) {
				PRINTF("tcpip_ipv6_output: nbr cache entry incomplete\n");
#if UIP_CONF_IPV6_QUEUE_PKT
				/* Copy outgoing pkt in the queuing buffer for later transmit and set
				*  the destination nbr to nbr. */
				if(uip_packetqueue_alloc(&nbr->packethandle, UIP_DS6_NBR_PACKET_LIFETIME) != NULL) {
					memcpy(uip_packetqueue_buf(&nbr->packethandle), UIP_IP_BUF, uip_len);
					uip_packetqueue_set_buflen(&nbr->packethandle, uip_len);
				}
#endif /*UIP_CONF_IPV6_QUEUE_PKT*/
				uip_len = 0;
				return;
			}
			/* Send in parallel if we are running NUD (nbc state is either STALE,
			*   DELAY, or PROBE). See RFC 4861, section 7.7.3 on node behavior. */
			if(nbr->state == NBR_STALE) {
				nbr->state = NBR_DELAY;
				stimer_set(&nbr->reachable, UIP_ND6_DELAY_FIRST_PROBE_TIME);
				nbr->nscount = 0;
				PRINTF("tcpip_ipv6_output: nbr cache entry stale moving to delay\n");
			}
#endif /* UIP_ND6_SEND_NA */

			xtcpip_output(uip_ds6_nbr_get_ll(nbr), mac_tx);

#if UIP_CONF_IPV6_QUEUE_PKT
			/*
			 * Send the queued packets from here, may not be 100% perfect though.
			 * This happens in a few cases, for example when instead of receiving a
			 * NA after sendiong a NS, you receive a NS with SLLAO: the entry moves
			 * to STALE, and you must both send a NA and the queued packet.
			 */
			if(uip_packetqueue_buflen(&nbr->packethandle) != 0) {
				uip_len = uip_packetqueue_buflen(&nbr->packethandle);
				memcpy(UIP_IP_BUF, uip_packetqueue_buf(&nbr->packethandle), uip_len);
				uip_packetqueue_free(&nbr->packethandle);
				xtcpip_output(uip_ds6_nbr_get_ll(nbr), mac_tx);
			}
#endif /*UIP_CONF_IPV6_QUEUE_PKT*/

			uip_len = 0;
			return;
		}
		return;
	}

	/* Multicast IP destination address. */
	xtcpip_output(NULL, mac_tx);
	uip_len = 0;
	uip_ext_len = 0;
}
#endif /* UIP_CONF_IPV6 */
/* -----------------------------------------------------------------------------
 * Process periodical stuff.
 *
 * In contiki, this is handlet by the eventhandler of the tcpip.c file
 * with the process event "PROCESS_EVENT_TIMER".
 * -------------------------------------------------------------------------- */
void xtcp_process_timer(chanend mac_tx, xtcp_tmr_event_type_t event)
{
#if UIP_IGMP
  igmp_periodic();
  if(uip_len > 0) {
    xtcp_tx_buffer(mac_tx);
  }
#endif

  if(event == XTCP_TMR_PERIODIC) {
#if UIP_TCP
    for(int i = 0; i < UIP_CONNS; ++i) {
      if(uip_conn_active(i)) {
        uip_periodic(i);
#if UIP_CONF_IPV6
        xtcpip_ipv6_output(mac_tx);
#else
        if(uip_len > 0) {
          PRINTF("tcpip_output from periodic len %d\n", uip_len);
          tcpip_output();
          PRINTF("tcpip_output after periodic len %d\n", uip_len);
        }
#endif /* UIP_CONF_IPV6 */
      }
    }
#endif /* UIP_TCP */
#if UIP_CONF_IP_FORWARD
    uip_fw_periodic();
#endif /* UIP_CONF_IP_FORWARD */
  }
  /*XXX CHSC HACK*/
#if UIP_CONF_IPV6
#if UIP_CONF_IPV6_REASSEMBLY
        /*
         * check the timer for reassembly
         */
        if(etimer_expired(&uip_reass_timer)) {
          uip_reass_over();
          tcpip_ipv6_output();
        }
#endif /* UIP_CONF_IPV6_REASSEMBLY */
        /*
         * check the different timers for neighbor discovery and
         * stateless autoconfiguration
         */
        /*if(data == &uip_ds6_timer_periodic &&
           etimer_expired(&uip_ds6_timer_periodic)) {
          uip_ds6_periodic();
          tcpip_ipv6_output();
        }*/
#if !UIP_CONF_ROUTER
        if(etimer_expired(&uip_ds6_timer_rs)) {
          uip_ds6_send_rs();
          xtcpip_ipv6_output(mac_tx);
        }
#endif /* !UIP_CONF_ROUTER */
        if(etimer_expired(&uip_ds6_timer_periodic)) {
          uip_ds6_periodic();
          xtcpip_ipv6_output(mac_tx);
        }
#endif /* UIP_CONF_IPV6 */

}
#if UIP_CONF_IPV4
#if UIP_USE_DHCP
void dhcpc_configured(const struct dhcpc_state *s) {
#ifdef XTCP_VERBOSE_DEBUG
	printf("dhcp: ");uip_printip4(s->ipaddr);printf("\n");
#endif
#if UIP_USE_AUTOIP
	autoip_stop();
#endif
	uip_sethostaddr(s->ipaddr);
	uip_setdraddr(s->default_router);
	uip_setnetmask(s->netmask);
	uip_xtcp_up();
	dhcp_done = 1;
}
#endif

#if UIP_USE_AUTOIP
void autoip_configured(uip_ipaddr_t autoip_ipaddr) {
	if (!dhcp_done) {
		uip_ipaddr_t ipaddr;
#ifdef XTCP_VERBOSE_DEBUG
		printf("ipv4ll: ");
		uip_printip4(autoip_ipaddr);
		printf("\n");
#endif /* XTCP_VERBOSE_DEBUG */
		uip_sethostaddr(autoip_ipaddr);
		uip_ipaddr(ipaddr, 255, 255, 0, 0);
		uip_setnetmask(ipaddr);
		uip_ipaddr(ipaddr, 0, 0, 0, 0);
		uip_setdraddr(ipaddr);
		uip_xtcp_up();
	}
}
#endif /* UIP_USE_AUTOIP */


#endif /* UIP_CONF_IPV4 */

/* -----------------------------------------------------------------------------
 * When a cable is plugged in, this function is called
 * ---------------------------------------------------------------------------*/
void uip_linkup() {
	if (uip_xtcp_get_ifstate()){
		uip_xtcp_down();
	}
#if UIP_CONF_IPV4
	if (uip_static_ip) {
		uip_sethostaddr(&uip_static_ipconfig.ipaddr);
		uip_setdraddr(&uip_static_ipconfig.gateway);
		uip_setnetmask(&uip_static_ipconfig.netmask);
		uip_xtcp_up();
	} else {
		dhcp_done = 0;
#if UIP_USE_DHCP
		dhcpc_stop();
#endif
#if UIP_USE_AUTOIP
#if UIP_USE_DHCP
		autoip_stop();
#else
		autoip_start();
#endif
#endif
#if UIP_USE_DHCP
		dhcpc_start();
#endif
	}
#endif /* UIP_CONF_IPVx */
}

/* -----------------------------------------------------------------------------
 * When a cable is unplugged in, this function is called
 * ---------------------------------------------------------------------------*/
void uip_linkdown() {
	dhcp_done = 0;
#if UIP_USE_DHCP
	dhcpc_stop();
#endif
#if UIP_USE_AUTOIP
	autoip_stop();
#endif
	uip_xtcp_down();
}

/* ********************************************************************** */

/* -----------------------------------------------------------------------------
 * Initialise the uip_server
 * -------------------------------------------------------------------------- */
void uip_server_init(chanend xtcp[], int num_xtcp,
					 xtcp_ipconfig_t *ipconfig,
					 unsigned char *mac_address)
{
	if (ipconfig != NULL)
		memcpy(&uip_static_ipconfig, ipconfig, sizeof(xtcp_ipconfig_t));

	/* set the mac_adress */
	memcpy(&uip_lladdr, mac_address, 6);

#if 0 //XXX CHSC: not necessary? Be carefully with erasing the mac address...
	/* The following line sets the uIP's link-layer address. This must be done
	 * before the tcpip_process is started since in its initialisation
	 * routine the function uip_netif_init() will be called from inside
	 * uip_init()and there the default IPv6 address will be set by combining
	 * the link local prefix (fe80::/64)and the link layer address. */
	rimeaddr_copy((rimeaddr_t*) &uip_lladdr.addr, &rimeaddr_node_addr);
#endif
//TODO chsc: port the rtimer module (if really needed)
//	/* rtimers needed for radio cycling */
//	rtimer_init();


	/* Initialise the process module */
	process_init();

	/* etimers must be started before ctimer_init */
	process_start(&etimer_process, NULL);

	ctimer_init();

	/* this calls have to be made before the uip_init
	 * not exactely proved why. CHSC
	 *  */
	etimer_request_poll();
	process_run();

	uip_init();

#if UIP_CONF_IPV6 && UIP_CONF_IPV6_RPL
	rpl_init();
#endif /* UIP_CONF_IPV6_RPL */

#if UIP_IGMP
	igmp_init();
#endif	/* UIP_IGMP */

	if (ipconfig != NULL && (*((int*)ipconfig->ipaddr.u8) != 0)) {
		uip_static_ip = 1;
	}

	if (ipconfig == NULL)
	{
		uip_ipaddr_t ipaddr;
#if UIP_CONF_IPV4
		uip_ipaddr(&ipaddr, 0, 0, 0, 0);
		uip_sethostaddr(&ipaddr);
		uip_setdraddr(&ipaddr);
		uip_setnetmask(&ipaddr);
#elif UIP_CONF_IPV6
		uip_ip6addr(&ipaddr, 0, 0, 0, 0
				           , 0, 0, 0, 0);
#endif	/* UIP_CONF_IPVx */
	} else {
#if UIP_CONF_IPV4
		uip_sethostaddr(&ipconfig->ipaddr);
		uip_setdraddr(&ipconfig->gateway);
		uip_setnetmask(&ipconfig->netmask);
#ifdef XTCP_VERBOSE_DEBUG
		printf("Address: ");uip_printip4(uip_hostaddr);printf("\n");
		printf("Gateway: ");uip_printip4(uip_draddr);printf("\n");
		printf("Netmask: ");uip_printip4(uip_netmask);printf("\n");
#endif /* XTCP_VERBOSE_DEBUG */
#elif UIP_CONF_IPV6

#endif /* UIP_CONF_IPVx */
	}

#if UIP_CONF_IPV4
	{
#if UIP_USE_AUTOIP
		int hwsum = mac_address[0] + mac_address[1] + mac_address[2]
				+ mac_address[3] + mac_address[4] + mac_address[5];
		autoip_init(hwsum + (hwsum << 16) + (hwsum << 24));
#endif
#if UIP_USE_DHCP
		dhcpc_init(uip_lladdr.addr, 6);
#endif
	}
#endif /* UIP_CONF_IPV4 */
	xtcpd_init(xtcp, num_xtcp);
}
