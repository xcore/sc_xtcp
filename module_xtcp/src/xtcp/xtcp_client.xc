// Copyright (c) 2011, XMOS Ltd., All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include <xccompat.h>
#include "xtcp_client.h"
#include "xtcp_cmd.h"

static void send_cmd(chanend c, xtcp_cmd_t cmd, int conn_id)
{
	outuchar(c, cmd);
	outuchar(c, conn_id);
	outuchar(c, 0);
	outct(c, XS1_CT_END);
//  c <: cmd;
//  c <: conn_id;
}

void xtcp_listen(chanend tcp_svr, int port_number, xtcp_protocol_t p) {
  send_cmd(tcp_svr, XTCP_CMD_LISTEN, 0);
  master {
    tcp_svr <: port_number;
    tcp_svr <: p;
  }
}

void xtcp_unlisten(chanend tcp_svr, int port_number) {
  send_cmd(tcp_svr, XTCP_CMD_UNLISTEN, 0);
  master {
    tcp_svr <: port_number;
  }
}

void xtcp_connect(chanend tcp_svr, 
                  int port_number, 
                  xtcp_ipaddr_t ipaddr,
                  xtcp_protocol_t p)
{
  send_cmd(tcp_svr, XTCP_CMD_CONNECT, 0);
  master {
    tcp_svr <: port_number;
    for(int i=0;i<4;i++) 
      tcp_svr <: ipaddr[i];
    tcp_svr <: p;
  }
}

void xtcp_bind_local(chanend tcp_svr, xtcp_connection_t &conn, 
                     int port_number)
{
  send_cmd(tcp_svr, XTCP_CMD_BIND_LOCAL, conn.id);
  master {
    tcp_svr <: port_number;
  }
}

void xtcp_bind_remote(chanend tcp_svr, xtcp_connection_t &conn, 
                      xtcp_ipaddr_t addr, int port_number)
{
  send_cmd(tcp_svr, XTCP_CMD_BIND_REMOTE, conn.id);
  master {
    for (int i=0;i<4;i++)
      tcp_svr <: addr[i];
    tcp_svr <: port_number;
  }
}

void xtcp_ask_for_event(chanend tcp_svr) {
  send_cmd(tcp_svr, XTCP_CMD_ASK, 0);
}

void xtcp_ask_for_config_event(chanend tcp_svr) {
  send_cmd(tcp_svr, XTCP_CMD_ASK_CONFIG, 0);
}

void xtcp_ask_for_conn_or_config_event(chanend tcp_svr) {
  send_cmd(tcp_svr, XTCP_CMD_ASK_BOTH, 0);
}


transaction xtcp_event(chanend tcp_svr, xtcp_connection_t &conn)
{
 tcp_svr :> int;
 tcp_svr :> conn;  
}

transaction xtcp_config_event(chanend tcp_svr,
                              xtcp_config_event_t &event,
                              xtcp_ipconfig_t &ipconfig)
{
  tcp_svr :> int;
  tcp_svr :> event;
  tcp_svr :> ipconfig; 
}


transaction xtcp_conn_or_config_event(chanend tcp_svr,
                                      xtcp_conn_or_config_t &event_type,
                                      xtcp_config_event_t &event,
                                      xtcp_ipconfig_t &ipconfig,
                                      xtcp_connection_t &conn)
{
  tcp_svr :> event_type;
  switch (event_type)
    {
    case XTCP_CONFIG_EVENT:
      tcp_svr :> event;
      tcp_svr :> ipconfig;
      break;
    case XTCP_CONN_EVENT:
      tcp_svr :> conn;
      break;
    }  
}

void xtcp_init_send(chanend tcp_svr,                    
                    REFERENCE_PARAM(xtcp_connection_t, conn))
{
  send_cmd(tcp_svr, XTCP_CMD_INIT_SEND, conn.id);
}

void xtcp_set_connection_appstate(chanend tcp_svr, 
                                  REFERENCE_PARAM(xtcp_connection_t, conn), 
                                  xtcp_appstate_t appstate)
{
  send_cmd(tcp_svr, XTCP_CMD_SET_APPSTATE, conn.id);
  master {
    tcp_svr <: appstate;
  }
}

void xtcp_close(chanend tcp_svr,
                REFERENCE_PARAM(xtcp_connection_t,conn)) 
{
  send_cmd(tcp_svr, XTCP_CMD_CLOSE, conn.id);
}

void xtcp_abort(chanend tcp_svr,
                REFERENCE_PARAM(xtcp_connection_t,conn))
{
  send_cmd(tcp_svr, XTCP_CMD_ABORT, conn.id);
}

void xtcp_request_null_event(chanend tcp_svr, int link)
{
  send_cmd(tcp_svr, XTCP_CMD_REQUEST_NULL_EVENT, link);
}


int xtcp_recv(chanend tcp_svr, unsigned char data[]) 
{
  int len;
  slave {
  tcp_svr :> len;
    for (int i=0;i<len;i++)
      tcp_svr :> data[i];
  }
  return len;
}


void xtcp_send(chanend tcp_svr,
               unsigned char data[],
               int len)
{
  slave {
    tcp_svr <: len;
    for (int i=0;i<len;i++)
      tcp_svr <: data[i];
  }
}

void xtcp_uint_to_ipaddr(xtcp_ipaddr_t ipaddr, unsigned int i) {
  ipaddr[0] = i & 0xff;
  i >>= 8;
  ipaddr[1] = i & 0xff;
  i >>= 8;
  ipaddr[2] = i & 0xff;
  i >>= 8;
  ipaddr[3] = i & 0xff;
}

void xtcp_set_poll_interval(chanend tcp_svr,
                            REFERENCE_PARAM(xtcp_connection_t, conn),
                            int poll_interval)
{
  send_cmd(tcp_svr, XTCP_CMD_SET_POLL_INTERVAL, conn.id);
  master {
    tcp_svr <: poll_interval;
  }
}

void xtcp_join_multicast_group(chanend tcp_svr,
                               xtcp_ipaddr_t addr)
{
  send_cmd(tcp_svr, XTCP_CMD_JOIN_GROUP, 0);
  master {
    tcp_svr <: addr[0];
    tcp_svr <: addr[1];
    tcp_svr <: addr[2];
    tcp_svr <: addr[3];
  }
}

void xtcp_leave_multicast_group(chanend tcp_svr,
                               xtcp_ipaddr_t addr)
{
  send_cmd(tcp_svr, XTCP_CMD_LEAVE_GROUP, 0);
  master {
    tcp_svr <: addr[0];
    tcp_svr <: addr[1];
    tcp_svr <: addr[2];
    tcp_svr <: addr[3];
  }
}

void xtcp_get_mac_address(chanend tcp_svr, unsigned char mac_addr[])
{
	send_cmd(tcp_svr, XTCP_CMD_GET_MAC_ADDRESS, 0);
	tcp_svr :> mac_addr[0];
	tcp_svr :> mac_addr[1];
	tcp_svr :> mac_addr[2];
	tcp_svr :> mac_addr[3];
	tcp_svr :> mac_addr[4];
	tcp_svr :> mac_addr[5];
}
