#ifdef __xtcp_client_conf_h_exists__
#include "xtcp_client_conf.h"
#endif

#ifdef XTCP_USE_QUICKSTART
#include "ethernet_quickstart.h"
#include "xtcp_client.h"
#include "uip_single_server.h"

static otp_ports_t otp_ports = ETH_QUICKSTART_OTP_PORTS_INIT;
static smi_interface_t smi = ETH_QUICKSTART_SMI_INIT;
static mii_interface_t mii = ETH_QUICKSTART_MII_INIT;

void ethernet_xtcp_server(xtcp_ipconfig_t &ipconfig,
                          chanend c_xtcp[],
                          int n)
{
  char mac_address[6];
  ethernet_getmac_otp(otp_ports, mac_address);
  // Start server
  uip_single_server(null, smi, mii, c_xtcp, 1,
                    ipconfig, mac_address);
}

#endif
