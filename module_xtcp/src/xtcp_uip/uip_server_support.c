// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <print.h>
#include <xccompat.h>
#include <string.h>

#include "uip.h"
#include "uip_arp.h"
#include "uip-split.h"
#include "uip_xtcp.h"
#include "autoip.h"

// This is the buffer where TCP constructs its packets
unsigned int uip_buf32[(UIP_BUFSIZE + 5) >> 2];
u8_t *uip_buf = (u8_t *) &uip_buf32[0];

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define TCPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

#if UIP_LOGGING == 1
void uip_log(char m[]) {
	printstr("uIP log message: ");
	printstr(m);
	printstr("\n");
}
#endif

#ifdef XTCP_VERBOSE_DEBUG
__attribute__ ((noinline)) void uip_printip4(const uip_ipaddr_t ip4) {
	printint(uip_ipaddr1(ip4));
	printstr(".");
	printint(uip_ipaddr2(ip4));
	printstr(".");
	printint(uip_ipaddr3(ip4));
	printstr(".");
	printint(uip_ipaddr4(ip4));
}
#endif

int uip_static_ip = 0;
xtcp_ipconfig_t uip_static_ipconfig;

static int dhcp_done = 0;

void xtcp_tx_buffer(chanend mac_tx) {
	uip_split_output(mac_tx);
	uip_len = 0;
}

void uip_server_init(chanend xtcp[], int num_xtcp, xtcp_ipconfig_t* ipconfig, unsigned char mac_address[6])
{
	if (ipconfig != NULL)
		memcpy(&uip_static_ipconfig, ipconfig, sizeof(xtcp_ipconfig_t));

	memcpy(&uip_ethaddr, mac_address, 6);

	uip_init();

#if UIP_IGMP
	igmp_init();
#endif

	if (ipconfig != NULL && (*((int*)ipconfig->ipaddr) != 0)) {
		uip_static_ip = 1;
	}

	if (ipconfig == NULL)
	{
		uip_ipaddr_t ipaddr;
		uip_ipaddr(ipaddr, 0, 0, 0, 0);
		uip_sethostaddr(ipaddr);
		uip_setdraddr(ipaddr);
		uip_setnetmask(ipaddr);
	} else {

		uip_sethostaddr(ipconfig->ipaddr);
		uip_setdraddr(ipconfig->gateway);
		uip_setnetmask(ipconfig->netmask);

#ifdef XTCP_VERBOSE_DEBUG
		printstr("Address: ");uip_printip4(uip_hostaddr);printstr("\n");
		printstr("Gateway: ");uip_printip4(uip_draddr);printstr("\n");
		printstr("Netmask: ");uip_printip4(uip_netmask);printstr("\n");
#endif
	}

	{
#if UIP_USE_AUTOIP
		int hwsum = mac_address[0] + mac_address[1] + mac_address[2]
				+ mac_address[3] + mac_address[4] + mac_address[5];
		autoip_init(hwsum + (hwsum << 16) + (hwsum << 24));
#endif
#if UIP_USE_DHCP
		dhcpc_init(uip_ethaddr.addr, 6);
#endif
		xtcpd_init(xtcp, num_xtcp);
	}
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

void xtcpd_check_connection_poll(chanend mac_tx)
{
	for (int i = 0; i < UIP_CONNS; i++) {
		if (uip_conn_needs_poll(&uip_conns[i])) {
			uip_poll_conn(&uip_conns[i]);
			if (uip_len > 0) {
				uip_arp_out( NULL);
				xtcp_tx_buffer(mac_tx);
			}
		}
	}

	for (int i = 0; i < UIP_UDP_CONNS; i++) {
		if (uip_udp_conn_needs_poll(&uip_udp_conns[i])) {
			uip_udp_periodic(i);
			if (uip_len > 0) {
				uip_arp_out(&uip_udp_conns[i]);
				xtcp_tx_buffer(mac_tx);
			}
		}
	}
}

void xtcp_process_incoming_packet(chanend mac_tx, int length)
{
	if (BUF->type == htons(UIP_ETHTYPE_IP)) {
		uip_len = length;
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
		uip_len = length;
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
}

void xtcp_process_udp_acks(chanend mac_tx)
{
	for (int i = 0; i < UIP_UDP_CONNS; i++) {
		if (uip_udp_conn_has_ack(&uip_udp_conns[i])) {
			uip_udp_ackdata(i);
			if (uip_len > 0) {
				uip_arp_out(&uip_udp_conns[i]);
				xtcp_tx_buffer(mac_tx);
			}
		}
	}
}

void xtcp_process_periodic_timer(chanend mac_tx)
{
#if UIP_IGMP
	igmp_periodic();
	if(uip_len > 0) {
		xtcp_tx_buffer(mac_tx);
	}
#endif
	for (int i = 0; i < UIP_UDP_CONNS; i++) {
		uip_udp_periodic(i);
		if (uip_len > 0) {
			uip_arp_out(&uip_udp_conns[i]);
			xtcp_tx_buffer(mac_tx);
		}
	}

	for (int i = 0; i < UIP_CONNS; i++) {
		uip_periodic(i);
		if (uip_len > 0) {
			uip_arp_out( NULL);
			xtcp_tx_buffer(mac_tx);
		}
	}
}

#if UIP_USE_DHCP
void dhcpc_configured(const struct dhcpc_state *s) {
#ifdef XTCP_VERBOSE_DEBUG
	printstr("dhcp: ");uip_printip4(s->ipaddr);printstr("\n");
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
		printstr("ipv4ll: ");
		uip_printip4(autoip_ipaddr);
		printstr("\n");
#endif
		uip_sethostaddr(autoip_ipaddr);
		uip_ipaddr(ipaddr, 255, 255, 0, 0);
		uip_setnetmask(ipaddr);
		uip_ipaddr(ipaddr, 0, 0, 0, 0);
		uip_setdraddr(ipaddr);
		uip_xtcp_up();
	}
}
#endif

void uip_linkup() {
	if (get_uip_xtcp_ifstate())
		uip_xtcp_down();

	if (uip_static_ip) {
#if UIP_CONF_IPV6
		uip_ipaddr_t ipaddr;
		uip_ipaddr(ipaddr,
				uip_static_ipconfig.ipaddr[0],
				uip_static_ipconfig.ipaddr[1],
				uip_static_ipconfig.ipaddr[2],
				uip_static_ipconfig.ipaddr[3]);
		uip_sethostaddr(ipaddr);
		uip_ipaddr(ipaddr,
				uip_static_ipconfig.gateway[0],
				uip_static_ipconfig.gateway[1],
				uip_static_ipconfig.gateway[2],
				uip_static_ipconfig.gateway[3]);
		uip_setdraddr(ipaddr);
		uip_ipaddr(ipaddr,
				uip_static_ipconfig.netmask[0],
				uip_static_ipconfig.netmask[1],
				uip_static_ipconfig.netmask[2],
				uip_static_ipconfig.netmask[3]);
		uip_setnetmask(ipaddr);
#else
		uip_sethostaddr(uip_static_ipconfig.ipaddr);
		uip_setdraddr(uip_static_ipconfig.gateway);
		uip_setnetmask(uip_static_ipconfig.netmask);
#endif
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
}

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
