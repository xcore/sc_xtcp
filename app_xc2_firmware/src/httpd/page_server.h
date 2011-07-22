/**
 * Module:  app_xc2_firmware
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    page_server.h
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
#ifndef _page_server_h_
#define _page_server_h_
#include <xccompat.h>


extern char ipaddr_string[];
extern char default_ipaddr_string[];
extern char default_netmask_string[];
extern char macaddr_string[];


unsigned int getLocalTimer();

void httpd_set_macaddr_string(unsigned char macaddr[]);
void httpd_set_ip_addr_string(unsigned int addr, int num);



typedef enum 
  {
    IPADDR_STRING_NUM,
    DEFAULT_IPADDR_STRING_NUM,
    NETMASK_STRING_NUM,
  } 
ipstrings_t;

typedef enum {
  CMD_END,
  CMD_SET_DPTR,
  CMD_READDATA,
  CMD_MACADDR,
  CMD_IPADDR,
  CMD_TIMER,
  CMD_BUTTONA,
  CMD_BUTTONB,
  CMD_IPMETHOD,
  CMD_DEFAULTIPMETHOD,
  CMD_DEFAULTIPADDR,
  CMD_BIGFILE,
  CMD_DEFAULTNETMASK,
  CMD_SERIALNUM,
  CMD_FIRMWAREVERSION
} file_cmd;

typedef unsigned int voidptr;

typedef struct page_server_state_t {
  voidptr dptr;
  voidptr saved_dptr;
  int left;
  voidptr cmdptr;
  int bfcount;
} page_server_state_t;

int page_server_open(chanend page_svr, char str[], REFERENCE_PARAM(page_server_state_t, s));

void page_server_next_cmd(chanend page_svr, REFERENCE_PARAM(page_server_state_t, s));

void page_server_get_data(chanend page_svr,
                          char dest[],
                          REFERENCE_PARAM(page_server_state_t, s),
                          int n);


int page_server_eof(chanend page_svr, REFERENCE_PARAM(page_server_state_t, s));




void next_cmd(REFERENCE_PARAM(page_server_state_t, pss), chanend button_info);

int page_server_local_open(char str[],
                            REFERENCE_PARAM(page_server_state_t, pss));

int page_server_local_eof(REFERENCE_PARAM(page_server_state_t, pss));

int page_server_send_data(chanend c,
                          REFERENCE_PARAM(page_server_state_t, pss),
                          int n);

void page_server_set_ipconfig(chanend c, unsigned int x);
void page_server_set_ipaddr(chanend c,  unsigned int x);
void page_server_set_netmask(chanend c,  unsigned int x);
void page_server_refresh_ip(chanend c);

void page_server(chanend page_svr, chanend button_info, chanend mac_tx,
                 chanend config_ch);

void page_server_send(chanend c, char data[], int n);

void page_server_init();
#endif // _page_server_h_
