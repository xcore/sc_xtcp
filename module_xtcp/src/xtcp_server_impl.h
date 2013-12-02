// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

// These functions need to be supplied by the IP stack to
// implement an xtcp server

#ifndef _xtcp_server_impl_h_
#define _xtcp_server_impl_h_
#include <xccompat.h>

void xtcpd_ask(int linknum);
void xtcpd_listen(int linknum, int port_number, xtcp_protocol_t p);
void xtcpd_unlisten(int linknum, int port_number);
void xtcpd_connect(int linknum, int port_number, xtcp_ipaddr_t addr,
                   xtcp_protocol_t p);
void xtcpd_bind_local(int linknum, int conn_id, int port_number);
void xtcpd_bind_remote(int linknum,
                       int conn_id,
                       xtcp_ipaddr_t addr,
                       int port_number);

void xtcpd_init_send(int linknum, int conn_id);
void xtcpd_set_appstate(int linknum, int conn_id, xtcp_appstate_t appstate);
void xtcpd_abort(int linknum, int conn_id);

void xtcpd_request_null_event(int linknum, int requested_linknum);

void xtcpd_close(int linknum, int conn_id);

void xtcpd_ask_config(int linknum);

void xtcpd_set_poll_interval(int linknum, int conn_id, int poll_interval);

void xtcpd_join_group(xtcp_ipaddr_t addr);
void xtcpd_leave_group(xtcp_ipaddr_t addr);
void xtcpd_get_mac_addr(unsigned char mac_addr[]);
void xtcpd_get_ipconfig(REFERENCE_PARAM(xtcp_ipconfig_t, ipconfig));

void xtcpd_ack_recv(int conn_id);
void xtcpd_ack_recv_mode(int conn_id);

void xtcpd_pause(int conn_id);
void xtcpd_unpause(int conn_id);
void xtcpd_accept_partial_ack(int conn_id);

#endif
