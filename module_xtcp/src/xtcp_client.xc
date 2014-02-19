// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include <print.h>
#include <xccompat.h>
#include "xtcp_client.h"
#include "xtcp_cmd.h"

static void send_cmd(chanend c, xtcp_cmd_t cmd, int conn_id)
{
  outct(c, XTCP_CMD_TOKEN);
  outct(c, XS1_CT_PAUSE);
  chkct(c, XS1_CT_END);
  chkct(c, XS1_CT_END);
  outuint(c, cmd);
  outuint(c, conn_id);
  outct(c, XS1_CT_END);
  chkct(c, XS1_CT_END);
}

void xtcp_listen(chanend tcp_svr, int port_number, xtcp_protocol_t p) {
  send_cmd(tcp_svr, XTCP_CMD_LISTEN, 0);
  master {
    tcp_svr <: port_number;
    tcp_svr <: p;
  }
}

void xtcp_unlisten(chanend c_xtcp, int port_number) {
  send_cmd(c_xtcp, XTCP_CMD_UNLISTEN, 0);
  master {
    c_xtcp <: port_number;
  }
}

void xtcp_connect(chanend c_xtcp,
                  int port_number,
                  xtcp_ipaddr_t ipaddr,
                  xtcp_protocol_t p)
{
  send_cmd(c_xtcp, XTCP_CMD_CONNECT, 0);
  master {
	  c_xtcp <: port_number;
#if UIP_CONF_IPV6
    for(int i=0;i<sizeof(xtcp_ipaddr_t);i++)
      c_xtcp <: ipaddr.u8[i];
#else
    for(int i=0;i<4;i++)
    	c_xtcp <: ipaddr[i];
#endif
    c_xtcp <: p;
  }
}

void xtcp_bind_local(chanend c_xtcp, xtcp_connection_t &conn,
                     int port_number)
{
  send_cmd(c_xtcp, XTCP_CMD_BIND_LOCAL, conn.id);
  master {
	  c_xtcp <: port_number;
  }
}

void xtcp_bind_remote(chanend c_xtcp, xtcp_connection_t &conn,
                      xtcp_ipaddr_t addr, int port_number)
{
  send_cmd(c_xtcp, XTCP_CMD_BIND_REMOTE, conn.id);
  master {
#if UIP_CONF_IPV6
    for (int i=0;i<sizeof(xtcp_ipaddr_t);i++)
    	c_xtcp <: addr.u8[i];
#else
    for (int i=0;i<4;i++)
    	c_xtcp <: addr[i];
#endif
    c_xtcp <: port_number;
  }
}

#pragma unsafe arrays
transaction xtcp_event(chanend c_xtcp, xtcp_connection_t &conn)
{
  for(int i=0;i<sizeof(conn)>>2;i++) {
	  c_xtcp :> (conn,unsigned int[])[i];
  }
}

void do_xtcp_event(chanend c_xtcp, xtcp_connection_t &conn) {
  slave xtcp_event(c_xtcp, conn);
}

void xtcp_init_send(chanend c_xtcp,
                    REFERENCE_PARAM(xtcp_connection_t, conn))
{
  send_cmd(c_xtcp, XTCP_CMD_INIT_SEND, conn.id);
}

void xtcp_set_connection_appstate(chanend c_xtcp,
                                  REFERENCE_PARAM(xtcp_connection_t, conn),
                                  xtcp_appstate_t appstate)
{
  send_cmd(c_xtcp, XTCP_CMD_SET_APPSTATE, conn.id);
  master {
	  c_xtcp <: appstate;
  }
}

void xtcp_close(chanend c_xtcp,
                REFERENCE_PARAM(xtcp_connection_t,conn))
{
  send_cmd(c_xtcp, XTCP_CMD_CLOSE, conn.id);
}

void xtcp_ack_recv(chanend c_xtcp,
                   REFERENCE_PARAM(xtcp_connection_t,conn))
{
  send_cmd(c_xtcp, XTCP_CMD_ACK_RECV, conn.id);
}

void xtcp_ack_recv_mode(chanend c_xtcp,
                        REFERENCE_PARAM(xtcp_connection_t,conn))
{
  send_cmd(c_xtcp, XTCP_CMD_ACK_RECV_MODE, conn.id);
}


void xtcp_abort(chanend c_xtcp,
                REFERENCE_PARAM(xtcp_connection_t,conn))
{
  send_cmd(c_xtcp, XTCP_CMD_ABORT, conn.id);
}

void xtcp_pause(chanend c_xtcp,
                REFERENCE_PARAM(xtcp_connection_t,conn))
{
  send_cmd(c_xtcp, XTCP_CMD_PAUSE, conn.id);
}

void xtcp_unpause(chanend c_xtcp,
                  REFERENCE_PARAM(xtcp_connection_t,conn))
{
  send_cmd(c_xtcp, XTCP_CMD_UNPAUSE, conn.id);
}



int xtcp_recvi(chanend c_xtcp, unsigned char data[], int index)
{
	int len;
	slave
	{
		c_xtcp <: 1;
		c_xtcp :> len;
		for (int i=index;i<index+len;i++)
			c_xtcp :> data[i];
	}

        chkct(c_xtcp, XS1_CT_END);

	return len;
}

#pragma unsafe arrays
int xtcp_recv_count(chanend c_xtcp, char data[], int count)
 {
	int len, rxc;
	slave
	{
		c_xtcp <: 1;
		c_xtcp :> len;
		rxc = (count < len) ? count : len;

		for (int i=0;i<len;i++) {
			char c;
			c_xtcp :> c;
			if (i<rxc) data[i] = c;
		}
	}

        chkct(c_xtcp, XS1_CT_END);

	return len;
}

int xtcp_recv(chanend c_xtcp, unsigned char data[]) {
	return xtcp_recvi(c_xtcp, data, 0);
}


void xtcp_ignore_recv(chanend c_xtcp)
{
  int len;
  char tmp;
  slave {
    c_xtcp <: 1;
    c_xtcp :> len;
    for (int i=0;i<len;i++)
      c_xtcp :> tmp;
  }
  return;
}


void xtcp_sendi(chanend c_xtcp,
                NULLABLE_ARRAY_OF(unsigned char, data),
                int index,
                int len)
{
  slave {
    c_xtcp <: len;
    for (int i=index;i<index+len;i++)
      c_xtcp <: data[i];
  }
}

void xtcp_send(chanend c_xtcp,
               NULLABLE_ARRAY_OF(unsigned char, data),
               int len)
{
  xtcp_sendi(c_xtcp, data, 0, len);
}

#if !UIP_CONF_IPV6
void xtcp_uint_to_ipaddr(xtcp_ipaddr_t ipaddr, unsigned int i) {
  ipaddr[0] = i & 0xff;
  i >>= 8;
  ipaddr[1] = i & 0xff;
  i >>= 8;
  ipaddr[2] = i & 0xff;
  i >>= 8;
  ipaddr[3] = i & 0xff;
}
#endif

void xtcp_set_poll_interval(chanend c_xtcp,
                            REFERENCE_PARAM(xtcp_connection_t, conn),
                            int poll_interval)
{
  send_cmd(c_xtcp, XTCP_CMD_SET_POLL_INTERVAL, conn.id);
  master {
    c_xtcp <: poll_interval;
  }
}

#if !UIP_CONF_IPV6
void xtcp_join_multicast_group(chanend c_xtcp,
                               xtcp_ipaddr_t addr)
{
  send_cmd(c_xtcp, XTCP_CMD_JOIN_GROUP, 0);
  master {
    c_xtcp <: addr[0];
    c_xtcp <: addr[1];
    c_xtcp <: addr[2];
    c_xtcp <: addr[3];
  }
}

void xtcp_leave_multicast_group(chanend c_xtcp,
                               xtcp_ipaddr_t addr)
{
  send_cmd(c_xtcp, XTCP_CMD_LEAVE_GROUP, 0);
  master {
    c_xtcp <: addr[0];
    c_xtcp <: addr[1];
    c_xtcp <: addr[2];
    c_xtcp <: addr[3];
  }
}
#endif

void xtcp_get_mac_address(chanend c_xtcp, unsigned char mac_addr[])
{
	send_cmd(c_xtcp, XTCP_CMD_GET_MAC_ADDRESS, 0);
	c_xtcp :> mac_addr[0];
	c_xtcp :> mac_addr[1];
	c_xtcp :> mac_addr[2];
	c_xtcp :> mac_addr[3];
	c_xtcp :> mac_addr[4];
	c_xtcp :> mac_addr[5];
}

void xtcp_get_ipconfig(chanend c_xtcp,
                       xtcp_ipconfig_t &ipconfig)
{
#if UIP_CONF_IPV6
  char *c_ptr;
#endif
  send_cmd(c_xtcp, XTCP_CMD_GET_IPCONFIG, 0);
#if UIP_CONF_IPV6
  c_ptr = (char *)&ipconfig;
#endif
  slave {
#if UIP_CONF_IPV6
	  for(int i=0; i<sizeof(xtcp_ipconfig_t); i++)
		  c_xtcp :> c_ptr[i];
#else
    c_xtcp :> ipconfig.ipaddr[0];
    c_xtcp :> ipconfig.ipaddr[1];
    c_xtcp :> ipconfig.ipaddr[2];
    c_xtcp :> ipconfig.ipaddr[3];
    c_xtcp :> ipconfig.netmask[0];
    c_xtcp :> ipconfig.netmask[1];
    c_xtcp :> ipconfig.netmask[2];
    c_xtcp :> ipconfig.netmask[3];
    c_xtcp :> ipconfig.gateway[0];
    c_xtcp :> ipconfig.gateway[1];
    c_xtcp :> ipconfig.gateway[2];
    c_xtcp :> ipconfig.gateway[3];
#endif
  }
}

extern inline void xtcp_complete_send(chanend c_xtcp);

void xtcp_accept_partial_ack(chanend c_xtcp,
                             REFERENCE_PARAM(xtcp_connection_t,conn))
{
  send_cmd(c_xtcp, XTCP_CMD_ACCEPT_PARTIAL_ACK, conn.id);
}
