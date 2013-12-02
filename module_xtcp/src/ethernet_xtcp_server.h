#ifndef __ethernet_xtcp_server_h__
#define __ethernet_xtcp_server_h__
#include "uip_server.h"
#include <xccompat.h>
#include "otp_board_info.h"
#include "ethernet.h"

typedef struct ethernet_xtcp_ports_s {
  otp_ports_t otp_ports;
  smi_interface_t smi;
  mii_interface_lite_t mii;
  ethernet_reset_interface_t eth_rst;
} ethernet_xtcp_ports_t;

void ethernet_xtcp_server(REFERENCE_PARAM(ethernet_xtcp_ports_t, ports),
                          REFERENCE_PARAM(xtcp_ipconfig_t, ipconfig),
                          chanend xtcp[],
                          int n);

#endif // __ethernet_xtcp_server_h__
