// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef __xtcp_server_h__
#define __xtcp_server_h__
#include <xccompat.h>
#include "xtcp_client.h"
#include "xtcp_server_conf.h"

#if UIP_CONF_IPV6
#include "process.h"
#include "uip-conf.h"
#endif

#define MAX_XTCP_CLIENTS 10

typedef struct xtcpd_state_t {
  unsigned int linknum;
  xtcp_connection_t conn;
  xtcp_server_state_t s;
} xtcpd_state_t;


void xtcpd_init(chanend xtcp[], int num_xtcp);

void xtcpd_send_event(chanend c, xtcp_event_type_t event,
                      REFERENCE_PARAM(xtcpd_state_t, s));

void xtcpd_send_null_event(chanend c);

void xtcpd_service_clients(chanend xtcp[], int num_xtcp);
void xtcpd_service_clients_until_ready(int waiting_link,
                                       chanend xtcp[],
                                       int num_xtcp);

void xtcpd_recv(chanend xtcp[],
                int linknum,
                int num_xtcp,
                REFERENCE_PARAM(xtcpd_state_t, s),
                unsigned char data[],
                int datalen);

int xtcpd_send(chanend c,
               xtcp_event_type_t event,
               REFERENCE_PARAM(xtcpd_state_t, s),
               unsigned char data[],
               int mss);

void xtcpd_get_mac_address(unsigned char []);

void xtcpd_server_init(void);

void xtcpd_queue_event(chanend c, int linknum, int event);
#endif
