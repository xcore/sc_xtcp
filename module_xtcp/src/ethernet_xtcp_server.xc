#include "ethernet_xtcp_server.h"
#include "uip_single_server.h"
#include "xtcp_conf_derived.h"
#if !XTCP_SEPARATE_MAC

void ethernet_xtcp_server(ethernet_xtcp_ports_t &ports,
                          xtcp_ipconfig_t &ipconfig,
                          chanend c_xtcp[],
                          int n)
{
  char mac_address[6];
  otp_board_info_get_mac(ports.otp_ports, 0, mac_address);
  // Start server
  eth_phy_reset(ports.eth_rst);
  uip_single_server(null, ports.smi, ports.mii, c_xtcp, 1,
                    ipconfig, mac_address);
}

#endif
