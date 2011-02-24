#ifndef _UIP_XTCP_H_
#define _UIP_XTCP_H_

void uip_xtcp_checkstate();
void uip_xtcp_up();
void uip_xtcp_down();
void uip_xtcp_checklink(chanend connect_status);
int get_uip_xtcp_ifstate();
void uip_linkdown();
void uip_linkup();
void uip_xtcp_null_events();
#endif // _UIP_XTCP_H_
