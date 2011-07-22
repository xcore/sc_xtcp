/**
 * Module:  module_xtcp
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    xtcp_server.xc
 *
 * The copyrights, all other intellectual and industrial 
 * property rights are retained by XMOS and/or its licensors. 
 * Terms and conditions covering the use of this code can
 * be found in the Xmos End User License Agreement.
 *
 * Copyright XMOS Ltd 2009
 *
 * In the case where this code is a modification of existing code
 * under a separate license, the separate license terms are shown
 * below. The modifications to the code are still covered by the 
 * copyright notice above.
 *
 **/                                   
#include <xs1.h>
#include "xtcp_cmd.h"
#include "xtcp_client.h"
#include "xtcp_server.h"
#include "xtcp_server_impl.h"


static void handle_xtcp_cmd(chanend c,
                            int i,
                            xtcp_cmd_t cmd,
                            int conn_id)
{
  switch (cmd) 
    {
    case XTCP_CMD_LISTEN: {
      int port_number;
      xtcp_protocol_t protocol;
      slave {
        c :> port_number;
        c :> protocol;
      }
      xtcpd_listen(i, port_number, protocol);
      break;
    }
    case XTCP_CMD_UNLISTEN: {
      int port_number;
      slave {
        c :> port_number;
      }
      xtcpd_unlisten(i, port_number);
      break;
    }
    case XTCP_CMD_CONNECT: {
      int port_number;
      xtcp_ipaddr_t ipaddr;
      xtcp_protocol_t protocol;
      slave {        
        c :> port_number;
        for (int j=0;j<4;j++)
          c :> ipaddr[j];
        c :> protocol;
        xtcpd_connect(i, port_number, ipaddr, protocol);
      }
      break;
    }
    case XTCP_CMD_BIND_REMOTE : {
      xtcp_ipaddr_t ipaddr;
      int port_number;
      slave {
        for (int j=0;j<4;j++)
          c :> ipaddr[j];
        c :> port_number;
      }
      xtcpd_bind_remote(i, conn_id, ipaddr, port_number);
      break;
    }
    case XTCP_CMD_BIND_LOCAL : {
      int port_number;
      slave {
        c :> port_number;
      }
      xtcpd_bind_local(i, conn_id, port_number);
      break;
    }
    case XTCP_CMD_ASK:
      xtcpd_ask(i);
      break;
    case XTCP_CMD_ASK_CONFIG:
      xtcpd_ask_config(i);
      break;
    case XTCP_CMD_ASK_BOTH:
      xtcpd_ask(i);
      xtcpd_ask_config(i);
      break;
    case XTCP_CMD_INIT_SEND: 
      xtcpd_init_send(i, conn_id);
      break;    
    case XTCP_CMD_SET_APPSTATE: {
      xtcp_appstate_t appstate;
      slave {
        c :> appstate;
      }
      xtcpd_set_appstate(i, conn_id, appstate);
      break;
    }
    case XTCP_CMD_ABORT:
      xtcpd_abort(i, conn_id);
      break;
    case XTCP_CMD_CLOSE: 
      xtcpd_close(i, conn_id);
      break;
    case XTCP_CMD_REQUEST_NULL_EVENT: 
      xtcpd_request_null_event(i, conn_id);
      break;
    case XTCP_CMD_SET_POLL_INTERVAL: {
      int poll_interval;
      slave {
        c :> poll_interval;
      }
      xtcpd_set_poll_interval(i, conn_id, poll_interval);
      }
      break;
    case XTCP_CMD_JOIN_GROUP: {
      xtcp_ipaddr_t ipaddr;
      slave {
        c :> ipaddr[0];
        c :> ipaddr[1];
        c :> ipaddr[2];
        c :> ipaddr[3];
      }
      xtcpd_join_group(ipaddr);
      }
      break;
    case XTCP_CMD_LEAVE_GROUP: {
      xtcp_ipaddr_t ipaddr;
      slave {
        c :> ipaddr[0];
        c :> ipaddr[1];
        c :> ipaddr[2];
        c :> ipaddr[3];
      }
      xtcpd_leave_group(ipaddr);
      }
      break;     
    case XTCP_CMD_GET_MAC_ADDRESS: {
    	unsigned char mac_addr[6];
    	xtcpd_get_mac_address(mac_addr);
    	c <: mac_addr[0];
    	c <: mac_addr[1];
    	c <: mac_addr[2];
    	c <: mac_addr[3];
    	c <: mac_addr[4];
    	c <: mac_addr[5];
    }
    break;
  }
}

int xtcpd_service_client0(chanend xtcp, int i)
{
  int activity = 1;
  unsigned char cmd;
  unsigned int conn_id;
  select 
      {
      case inuchar_byref(xtcp, cmd):
//      case (int i=0;i<num_xtcp;i++) xtcp[i] :> cmd:
    	conn_id = inuchar(xtcp);
      	(void)inuchar(xtcp);
      	(void)inct(xtcp);
        handle_xtcp_cmd(xtcp, i, cmd, conn_id);
        break;
      default:
        activity = 0;
        break;
      }
  return activity;
}

void xtcpd_service_clients(chanend xtcp[], int num_xtcp){
    int activity = 1;
	while (activity) {
	activity = 0;
	 for (int i=0;i<num_xtcp;i++)
	 	activity |= xtcpd_service_client0(xtcp[i], i);
	 	
  }	
}

void xtcpd_send_event(chanend c,
                      xtcp_event_type_t event,
                      xtcpd_state_t &s)
{
  s.conn.event = event;
  master {
    c <: XTCP_CONN_EVENT;
    c <: s.conn;
  }
}

void xtcpd_recv(chanend c,
                xtcpd_state_t &s,
                unsigned char data[],
                int datalen)
{
  s.conn.event = XTCP_RECV_DATA;
  master {
    c <: XTCP_CONN_EVENT;
    c <: s.conn;
  }
  master {
    c <: datalen;
    for (int i=0;i<datalen;i++)
      c <: data[i];
  }
}


int xtcpd_send(chanend c,                
               xtcp_event_type_t event,
               xtcpd_state_t &s,
               unsigned char data[],
               int mss)
{
  int len;
  s.conn.event = event;
  s.conn.mss = mss;
  master {
    c <: XTCP_CONN_EVENT;
    c <: s.conn;
  }
  master {
    c :> len;
    for (int i=0;i<len;i++)
      c :> data[i];
  }
  return len;
}

void xtcpd_send_config_event(chanend c, 
                             xtcp_config_event_t event,
                             xtcp_ipconfig_t &ipconfig)
{
  master {
    c <: XTCP_CONFIG_EVENT;
    c <: event;
    c <: ipconfig;
  }
}

void xtcpd_send_null_event(chanend c)
{
  xtcp_connection_t conn;
  conn.event = XTCP_NULL;
  master {
    c <: XTCP_CONN_EVENT;
    c <: conn;
  }
}
