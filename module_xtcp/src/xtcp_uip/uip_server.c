// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <print.h>
#include <xccompat.h>
#include <string.h>

#include "xtcp_conf_derived.h"

#ifndef UIP_USE_SINGLE_THREADED_ETHERNET

#include "uip.h"
#include "uip_arp.h"
#include "xcoredev.h"
#include "xtcp_server.h"
#include "timer.h"
#include "uip_server.h"
#include "ethernet_rx_client.h"
#include "ethernet_tx_client.h"
#include "uip_xtcp.h"
#include "autoip.h"
#include "igmp.h"

// Functions from the uip_server_support file
extern void uip_printip4(const uip_ipaddr_t ip4);
extern void uip_server_init(chanend xtcp[], int num_xtcp, xtcp_ipconfig_t* ipconfig, unsigned char mac_address[6]);
extern void xtcpd_check_connection_poll(chanend mac_tx);
extern void xtcp_tx_buffer(chanend mac_tx);
extern void xtcp_process_incoming_packet(chanend mac_tx, int length);
extern void xtcp_process_udp_acks(chanend mac_tx);
extern void xtcp_process_periodic_timer(chanend mac_tx);


// Global variables from the uip_server_support file
extern int uip_static_ip;
extern xtcp_ipconfig_t uip_static_ipconfig;

#if XTCP_SEPARATE_MAC

void xtcp_server_uip(chanend mac_rx, chanend mac_tx, chanend xtcp[], int num_xtcp,
                     xtcp_ipconfig_t *ipconfig) {

	struct uip_timer periodic_timer, arp_timer, autoip_timer;
	unsigned char hwaddr[6];

	timer_set(&periodic_timer, CLOCK_SECOND / 10);
	timer_set(&autoip_timer, CLOCK_SECOND / 2);
	timer_set(&arp_timer, CLOCK_SECOND * 10);

	xcoredev_init(mac_rx, mac_tx);

	mac_get_macaddr(mac_tx, hwaddr);

	uip_server_init(xtcp, num_xtcp, ipconfig, hwaddr);

	// Main uIP service loop
	while (1)
	{
		xtcpd_service_clients(xtcp, num_xtcp);

		xtcpd_check_connection_poll(mac_tx);

		uip_xtcp_checkstate();
		uip_len = xcoredev_read(mac_rx, UIP_CONF_BUFFER_SIZE);
		if (uip_len > 0) {
                  xtcp_process_incoming_packet(mac_tx, uip_len);
		}

		xtcp_process_udp_acks(mac_tx);


		if (timer_expired(&arp_timer)) {
			timer_reset(&arp_timer);
			uip_arp_timer();
		}

#if UIP_USE_AUTOIP
		if (timer_expired(&autoip_timer)) {
			timer_reset(&autoip_timer);
			autoip_periodic();
			if (uip_len > 0) {
				xtcp_tx_buffer(mac_tx);
			}
		}
#endif

		if (timer_expired(&periodic_timer)) {

			xtcp_process_periodic_timer(mac_tx);

			timer_reset(&periodic_timer);
		}

	}
	return;
}

#endif // XTCP_SEPARATE_MAC

#endif
