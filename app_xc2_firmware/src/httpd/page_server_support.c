/**
 * Module:  app_xc2_firmware
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    page_server_support.c
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
#include <page_server.h>
#include <xccompat.h>
#include <string.h>
#include <stdio.h>
#include "buttons.h"
#include "fs.h"
#include <print.h>

unsigned int getLocalTimer();

#ifndef NULL
#define NULL ((void *) 0)
#endif
typedef unsigned char u8_t;
typedef unsigned int  u32_t;



char ipaddr_string[] = "000.000.000.000";
char default_ipaddr_string[] = "000.000.000.000";
char default_netmask_string[] = "000.000.000.000";
char macaddr_string[] = "00:00:00:00:00:00";
char serialnum_string[] = "0000000000";
char numstring[20];
unsigned int ip_config_cache = 0;
unsigned int default_ip_config_cache = 0;
char ipconfig0_string[] = "Automatic";
char ipconfig1_string[] = "Static";

#define STRINGIFY0(x) #x
#define STRINGIFY(x) STRINGIFY0(x)

char firmware_version_string[] = STRINGIFY(XC2_FIRMWARE_VERSION);
char bigfile_str[5] = "TEST";

static int buttonAcount = 0;
static int buttonBcount = 0;

void httpd_set_macaddr_string(unsigned char macaddr[])
 {
  siprintf(macaddr_string,
           "%02x:%02x:%02x:%02x:%02x:%02x",
           macaddr[0],
           macaddr[1],
           macaddr[2],
           macaddr[3],
           macaddr[4],
           macaddr[5]);

  return;
}

static int prev_ipaddr = 0;

void httpd_set_ip_addr_string(unsigned int addr, int stringNum)
{
  char *str;
  switch (stringNum)
    {
    case IPADDR_STRING_NUM:
      str = ipaddr_string;
      break;
    case DEFAULT_IPADDR_STRING_NUM:
      str = default_ipaddr_string;
      break;
    case NETMASK_STRING_NUM:
      str = default_netmask_string;
      break;
    }

  if (str != ipaddr_string || addr != prev_ipaddr)
     siprintf(str,
              "%u.%u.%u.%u",
              addr & 0xff,
              (addr >> 8) & 0xff,
              (addr >> 16) & 0xff,
              (addr >> 24) & 0xff);

  if (str == ipaddr_string)
   prev_ipaddr = addr;
   return;
}





void next_cmd(page_server_state_t *pss, chanend button_info)
{
  int progress = 0;

  if (pss->bfcount > 0) {
    pss->bfcount--;
    if (pss->bfcount != 0) {
      pss->dptr = (int) bigfile_str;
      pss->left = 4;
    }
    else {
      pss->cmdptr+=4;
    }
  }
    
  if (pss->cmdptr == 0L)
    return;
  
  if (pss->saved_dptr != 0) {
    pss->dptr = pss->saved_dptr;
    pss->saved_dptr = 0;
  }
  
  
  while (!progress) {
    switch (*((int *) pss->cmdptr))
      {
      case CMD_END:
        progress = 1;
        return;
      case CMD_SET_DPTR:      
        pss->cmdptr+=4;
        pss->dptr = *((int *) pss->cmdptr);
        break;
      case CMD_READDATA:
        progress = 1;
        pss->cmdptr+=4;
        pss->left = *((int *) pss->cmdptr);
        break;
      case CMD_MACADDR:
        progress = 1;
        pss->left = strlen(macaddr_string);
        pss->saved_dptr = pss->dptr;
        pss->dptr = (voidptr) macaddr_string;
        break;
      case CMD_TIMER:
        {
          unsigned int tval = getLocalTimer();
          int len;
          progress = 1;
          len = siprintf(numstring,"%u",tval);
          pss->left = len;
          pss->saved_dptr = pss->dptr;
          pss->dptr = (voidptr) numstring;
          break;
        }
      case CMD_BUTTONA:
        {
          unsigned int val = getButtonCountA(button_info);
          int len;
          progress = 1;
          len = siprintf(numstring,"%u",val);
          pss->left = len;
          pss->saved_dptr = pss->dptr;
          pss->dptr = (voidptr) numstring;
          break;
        }
      case CMD_BUTTONB:
        {
          unsigned int val = getButtonCountB(button_info);
          int len;
          progress = 1;
          len = siprintf(numstring,"%u",val);
          pss->left = len;
          pss->saved_dptr = pss->dptr;
          pss->dptr = (voidptr) numstring;
          break;
        }
      case CMD_IPADDR:
        {
          progress = 1;
          pss->left = strlen(ipaddr_string);
          pss->saved_dptr = pss->dptr;
          pss->dptr = (voidptr) ipaddr_string;
          break;          
        }
      case CMD_IPMETHOD:
        {
          progress = 1;
          switch (ip_config_cache)
            {
            case 1:
              pss->left = strlen(ipconfig1_string);
              pss->saved_dptr = pss->dptr;
              pss->dptr = (voidptr) ipconfig1_string;
              break;
            case 0:
            default:
              pss->left = strlen(ipconfig0_string);
              pss->saved_dptr = pss->dptr;
              pss->dptr = (voidptr) ipconfig0_string;
              break;              
            }
          break;
        }
      case CMD_DEFAULTIPADDR:
        {
          progress = 1;
          pss->left = strlen(default_ipaddr_string);
          pss->saved_dptr = pss->dptr;
          pss->dptr = (voidptr) default_ipaddr_string;
          break;          
        }
      case CMD_DEFAULTNETMASK:
        {
          progress = 1;
          pss->left = strlen(default_netmask_string);
          pss->saved_dptr = pss->dptr;
          pss->dptr = (voidptr) default_netmask_string;
          break;          
        }
      case CMD_DEFAULTIPMETHOD:
        {
          progress = 1;
          switch (default_ip_config_cache)
            {
            case 1:
              pss->left = strlen(ipconfig1_string);
              pss->saved_dptr = pss->dptr;
              pss->dptr = (voidptr) ipconfig1_string;
              break;
            case 0:
            default:
              pss->left = strlen(ipconfig0_string);
              pss->saved_dptr = pss->dptr;
              pss->dptr = (voidptr) ipconfig0_string;
              break;              
            }
          break;
        }
      case CMD_BIGFILE:
        {
          progress = 1;
          pss->bfcount = 10*1024;
          pss->dptr = (voidptr) bigfile_str;
          pss->left = 1024;
          break;
        }
      case CMD_FIRMWAREVERSION:
        {
          progress = 1;
          pss->left = strlen(firmware_version_string);
          pss->saved_dptr = pss->dptr;
          pss->dptr = (voidptr) firmware_version_string;
          break;
        }
      case CMD_SERIALNUM:
        {
          progress = 1;
          pss->left = strlen(serialnum_string);
          pss->saved_dptr = pss->dptr;
          pss->dptr = (voidptr) serialnum_string;
          break;
        }
      }
    
    if (*((int *) pss->cmdptr) != CMD_END && 
        *((int *) pss->cmdptr) != CMD_BIGFILE)
      pss->cmdptr+=4;
  }

  return;
}

void page_server_init() {
  unsigned serialNum;
  getSerialNum(&serialNum);
  siprintf(serialnum_string,
           "%u",
           serialNum);
}


int page_server_local_open(char str[],
                            REFERENCE_PARAM(page_server_state_t, pss))
{
  struct fs_file file;
  int ret;
  ret = fs_open(str, &file);
  pss->cmdptr = (voidptr) file.cmdptr;
  return ret;
}

int page_server_local_eof(REFERENCE_PARAM(page_server_state_t, pss))
{
  int *cmdptr = (int *) pss->cmdptr;
  return (cmdptr == NULL || *cmdptr == CMD_END);
}

int page_server_send_data(chanend c,
                          REFERENCE_PARAM(page_server_state_t, pss),
                          int n)
{
  page_server_send(c, (char *) pss->dptr, n);
}
