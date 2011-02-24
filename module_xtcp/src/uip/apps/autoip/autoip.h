#ifndef _autoip_h_
#define _autoip_h_

void autoip_init(int seed);
void autoip_arp_in();
void autoip_start();
void autoip_stop();
void autoip_configured(uip_ipaddr_t ipaddr);
void autoip_periodic();
#endif //_autoip_h_
