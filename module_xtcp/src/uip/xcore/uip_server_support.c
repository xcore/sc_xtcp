// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <print.h>
#include <xccompat.h>
#include <string.h>
#include "uip.h"
#include "uip_xtcp.h"
#include "autoip.h"

#if UIP_LOGGING == 1
void uip_log(char m[]) {
	printstr("uIP log message: ");
	printstr(m);
	printstr("\n");
}
#endif

#ifdef XTCP_VERBOSE_DEBUG
void uip_printip4(const uip_ipaddr_t ip4) {
	printint(uip_ipaddr1(ip4));
	printstr(".");
	printint(uip_ipaddr2(ip4));
	printstr(".");
	printint(uip_ipaddr3(ip4));
	printstr(".");
	printint(uip_ipaddr4(ip4));
}
#endif

extern int uip_static_ip;
extern xtcp_ipconfig_t uip_static_ipconfig;


static int dhcp_done = 0;

void dhcpc_configured(const struct dhcpc_state *s) {
#ifdef XTCP_VERBOSE_DEBUG
	printstr("dhcp: ");
	uip_printip4(s->ipaddr);
	printstr("\n");
#endif
	autoip_stop();
	uip_sethostaddr(s->ipaddr);
	uip_setdraddr(s->default_router);
	uip_setnetmask(s->netmask);
	uip_xtcp_up();
	dhcp_done = 1;
}

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

void uip_linkup() {
	if (get_uip_xtcp_ifstate())
		uip_xtcp_down();

	if (uip_static_ip) {
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
		uip_xtcp_up();
	} else {
		dhcp_done = 0;
		dhcpc_stop();
		autoip_stop();
		dhcpc_start();
	}
}

void uip_linkdown() {
	dhcp_done = 0;
	dhcpc_stop();
	autoip_stop();
	uip_xtcp_down();
}
