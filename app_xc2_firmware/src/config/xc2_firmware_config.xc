// Copyright (c) 2011, XMOS Ltd., All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include "AtmelOps.h"
#include "SpiPorts.h"
#include <print.h>
#include "xc2_firmware_config.h"
#include "xtcp_client.h"
#include "leds.h"

typedef enum {
  GET_IP_CONFIG,
  SET_IP_CONFIG,
  GET_IP_ADDR,
  SET_IP_ADDR,
  GET_DEFAULT_IP_CONFIG,
  SET_DEFAULT_IP_CONFIG,
  GET_DEFAULT_IP_ADDR,
  SET_DEFAULT_IP_ADDR,
  GET_DEFAULT_NETMASK,
  SET_DEFAULT_NETMASK,
  COMMIT_CONFIG,
  ENABLE_IPCONFIG,
} flash_config_cmd_t;


typedef struct flashConfig_struct_t {
  int ip_config;
  int ip_addr;  
  int netmask;
} flashConfig_t;

static int ip_addr;
static int ip_config;

static flashConfig_t config;

static unsigned char page[256];

int atmel_getId(unsigned int& id);

#define CONFIG_PAGE_ADDR 8*65536-256
#define DEFAULT_IPADDR   0x6400a8c0  /* 192.168.0.100 */
#define DEFAULT_NETMASK  0x0000ffff  /* 255.255.0.0 */
#define DEFAULT_IPCONFIG 13



static void readFromFlash() {
  int i;
  atmel_readPage(CONFIG_PAGE_ADDR,page);  
  //  printstr("READ\n");
  //  for (i=0;i<8;i++) {
  //    printintln(page[i]);
  //  }
  for (i=0;i<sizeof(config);i++) {
    (config,char[])[i] = page[i];
  }
  if (config.ip_addr == 0 || config.ip_addr == 0xffffffff)
    {
      config.ip_addr = DEFAULT_IPADDR;
      config.netmask = DEFAULT_NETMASK;
    }

  if (config.ip_config == 0 || config.ip_config == 0xffffffff) {
    config.ip_config = DEFAULT_IPCONFIG;
  }
}

static void writeToFlash() {
  int i;
  atmel_eraseOne(7);
  for (i=0;i<sizeof(config);i++) {
    page[i] = (config,char[])[i];
  }
  //  printstr("WRITE\n");
  //  for (i=0;i<8;i++) {
  //    printintln(page[i]);
  //  }

  atmel_startWrite();
  atmel_programPage(CONFIG_PAGE_ADDR,page);
  atmel_endWrite();
}


void enable_ipconfig_from_ipserver(chanend config_svr)
{
  config_svr <: ENABLE_IPCONFIG;
}

void set_ip_config(chanend config_svr, unsigned int val) 
{
  config_svr <: SET_IP_CONFIG;
  config_svr <: val;
}
 
void set_ip_addr(chanend config_svr, unsigned int val) 
{
  config_svr <: SET_IP_ADDR;
  config_svr <: val;
}
  
unsigned int get_ip_config(chanend config_svr) 
{
  unsigned int val;
  config_svr <: GET_IP_CONFIG;
  config_svr :> val;
  return val;
}

unsigned int get_ip_addr(chanend config_svr) 
{
  unsigned int val;
  config_svr <: GET_IP_ADDR;
  config_svr :> val;
  return val;
}


void set_default_ip_config(chanend config_svr, unsigned int val) 
{
  config_svr <: SET_DEFAULT_IP_CONFIG;
  config_svr <: val;
}
 
void set_default_ip_addr(chanend config_svr, unsigned int val) 
{
  config_svr <: SET_DEFAULT_IP_ADDR;
  config_svr <: val;
}

 
void set_default_netmask(chanend config_svr, unsigned int val) 
{
  config_svr <: SET_DEFAULT_NETMASK;
  config_svr <: val;
}
  
unsigned int get_default_ip_config(chanend config_svr) 
{
  unsigned int val;
  config_svr <: GET_DEFAULT_IP_CONFIG;
  config_svr :> val;
  return val;
}

unsigned int get_default_ip_addr(chanend config_svr) 
{
  unsigned int val;
  config_svr <: GET_DEFAULT_IP_ADDR;
  config_svr :> val;
  return val;
}

unsigned int get_default_netmask(chanend config_svr) 
{
  unsigned int val;
  config_svr <: GET_DEFAULT_NETMASK;
  config_svr :> val;
  return val;
}


void commit_config(chanend config_svr) 
{
  config_svr <: COMMIT_CONFIG;
  return;
}

static void processCommand(chanend client, int cmd) 
{
  switch (cmd)
    {
    case GET_IP_CONFIG:
      client <: ip_config;
      break;
    case SET_IP_CONFIG:
      client :> ip_config;
      break;
    case GET_IP_ADDR:
      client <: ip_addr;
      break;
    case SET_IP_ADDR:
      client :> ip_addr;
      break;
    case GET_DEFAULT_IP_CONFIG:
      client <: config.ip_config-13;
      break;
    case SET_DEFAULT_IP_CONFIG: {
      unsigned int val;
      client :> val;
      config.ip_config = val+13;
      }      
      break;
    case GET_DEFAULT_IP_ADDR:
      client <: config.ip_addr;
      break;
    case SET_DEFAULT_IP_ADDR:
      client :> config.ip_addr;
      break;
    case GET_DEFAULT_NETMASK:
      client <: config.netmask;
      break;
    case SET_DEFAULT_NETMASK:
      client :> config.netmask;
      break;
    case COMMIT_CONFIG:
      writeToFlash();
      break;
    }
}

void xc2_firmware_config(chanend config_ch[],
                         int num_config_ch,
                         chanend xtcp_svr,
                         chanend led_svr) 
{
  unsigned int id;
  xtcp_ipconfig_t ipconfig;
  xtcp_config_event_t event;
  spiMasterInit();
  atmel_getId(id);
  readFromFlash();
  ip_addr = config.ip_addr;
  ip_config = config.ip_config-13;
  //  config.ip_config = 0;
  //  writeToFlash();

  while (1) {
    select 
      {
      case (int i=0;i<num_config_ch;i++) config_ch[i] :> int cmd:
        if (cmd == ENABLE_IPCONFIG)  {
          xtcp_ask_for_config_event(xtcp_svr);
        }          
        else 
          processCommand(config_ch[i], cmd);
        break;
      case xtcp_config_event(xtcp_svr, event, ipconfig):
        if (event == XTCP_IFUP) {
          printstr("if up: ");
          printint(ipconfig.ipaddr[0]);
          printstr(".");
          printint(ipconfig.ipaddr[1]);
          printstr(".");
          printint(ipconfig.ipaddr[2]);
          printstr(".");
          printintln(ipconfig.ipaddr[3]);
          led_set_connected(led_svr, 1);
          ip_addr = 
            ipconfig.ipaddr[0] |
            (ipconfig.ipaddr[1] << 8) |
            (ipconfig.ipaddr[2] << 16) |
            (ipconfig.ipaddr[3] << 24);
        }
        else {
          printstr("if down\n");
          led_set_connected(led_svr, 0);          
        }                
        xtcp_ask_for_config_event(xtcp_svr);
        break;
      }
  }
  return;
}
