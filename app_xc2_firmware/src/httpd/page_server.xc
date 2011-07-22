/**
 * Module:  app_xc2_firmware
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    page_server.xc
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

#include <safestring.h>
#include "ethernet_tx_client.h"
#include "page_server.h"
#include "xc2_firmware_config.h"

extern unsigned int ip_config_cache;
extern unsigned int default_ip_config_cache;


enum page_server_cmd {
  PAGE_SERVER_CMD_OPEN,
  PAGE_SERVER_CMD_NEXT_CMD,
  PAGE_SERVER_CMD_GET_DATA,
  PAGE_SERVER_CMD_EOF,
  PAGE_SERVER_SET_IPADDR,
  PAGE_SERVER_SET_IPCONFIG,
  PAGE_SERVER_SET_NETMASK,
  PAGE_SERVER_REFRESH_IP,
};


int page_server_open(chanend c, char str[], REFERENCE_PARAM(page_server_state_t, s)) {
  int n = safestrlen(str);
  int ret;
  c <: PAGE_SERVER_CMD_OPEN;
  master {
    c <: n;
    for (int i=0;i<n;i++)
      c <: str[i];
  }
  c <: s;
  c :> s;
  c :> ret;
  return ret;
}

void page_server_next_cmd(chanend c, REFERENCE_PARAM(page_server_state_t, s)) {
  c <: PAGE_SERVER_CMD_NEXT_CMD;
  c <: s;
  c :> s;
}

void page_server_get_data(chanend c,
                          char dest[],
                          REFERENCE_PARAM(page_server_state_t, s),
                          int n) {
  c <: PAGE_SERVER_CMD_GET_DATA;
  c <: s;
  c <: n;
  slave {
    for (int i=0;i<n;i++)
      c :> dest[i];
  }
}


int page_server_eof(chanend c, REFERENCE_PARAM(page_server_state_t, s))
{
  int ret;
  c <: PAGE_SERVER_CMD_EOF;
  c <: s;
  c :> ret;
  return ret;
}


void page_server_set_ipconfig(chanend c, unsigned int x)
{
  c <: PAGE_SERVER_SET_IPCONFIG;
  c <: x;
}

void page_server_set_ipaddr(chanend c,  unsigned int x) {
  c <: PAGE_SERVER_SET_IPADDR;
  c <: x;
}

void page_server_set_netmask(chanend c,  unsigned int x) {
  c <: PAGE_SERVER_SET_NETMASK;
  c <: x;
}

void page_server_refresh_ip(chanend c) {
  c <: PAGE_SERVER_REFRESH_IP;
}

void page_server(chanend page_svr, chanend button_info, chanend mac_tx,
                 chanend config_ch)
{
  page_server_state_t s;
  unsigned char macaddr[6];

  page_server_init();

  mac_get_macaddr(mac_tx, macaddr);
  
  httpd_set_macaddr_string(macaddr);

  httpd_set_ip_addr_string(get_default_ip_addr(config_ch), 
                           DEFAULT_IPADDR_STRING_NUM);

  httpd_set_ip_addr_string(get_default_netmask(config_ch), 
                           NETMASK_STRING_NUM);
  
  default_ip_config_cache = get_default_ip_config(config_ch);
  ip_config_cache = get_ip_config(config_ch);

  while (1) {
    select {
    case page_svr :> int cmd:
      switch (cmd) 
        {
        case PAGE_SERVER_CMD_OPEN: {
          char str[31];
          int n;
          int ret;
          slave {
            page_svr :> n;
            for (int i=0;i<n;i++) 
              if (i < 30)
                page_svr :> str[i];
              else
                page_svr :> char;
          }
          if (n > 30) n = 30;
          str[n] = 0;
          page_svr :> s;
          ret = page_server_local_open(str, s);
          page_svr <: s;
          page_svr <: ret;
          } 
          break;           
        case PAGE_SERVER_CMD_NEXT_CMD: {
          page_svr :> s;
          
          next_cmd(s, button_info);
          page_svr <: s;
          }
          break;
        case PAGE_SERVER_CMD_GET_DATA: {
          int n;
          page_svr :> s;
          page_svr :> n;
          page_server_send_data(page_svr, s, n);
          }
          break;
        case PAGE_SERVER_CMD_EOF: {          
          page_svr :> s;
          page_svr <: page_server_local_eof(s);
          }
          break;
        case PAGE_SERVER_SET_IPCONFIG: {
          int x;
          page_svr :> x;
          set_default_ip_config(config_ch, x);
          commit_config(config_ch);
          default_ip_config_cache=0;
          }        
          break;
        case PAGE_SERVER_SET_IPADDR: {
          int x;
          page_svr :> x;
          set_default_ip_addr(config_ch, x);
          httpd_set_ip_addr_string(x, DEFAULT_IPADDR_STRING_NUM);
          commit_config(config_ch);
          }        
          break;
        case PAGE_SERVER_SET_NETMASK: {
          int x;
          page_svr :> x;
          set_default_netmask(config_ch, x);
          httpd_set_ip_addr_string(x, NETMASK_STRING_NUM);
          commit_config(config_ch);
          }        
          break;
        case PAGE_SERVER_REFRESH_IP: {
          httpd_set_ip_addr_string(get_ip_addr(config_ch), IPADDR_STRING_NUM);
          break;
        }
      }
      break;
    }
    
  }
  
}

void page_server_send(chanend c, char data[], int n)
{
  master {
    for (int i=0;i<n;i++)
      c <: data[i] ;
  }
  return;
}
