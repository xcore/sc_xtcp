#include <xs1.h>

#include "mii.h"
#include "smi.h"
#include "xtcp_client.h"

void uipSingleServer(clock clk_smi,
                     out port ?p_mii_resetn,
                     smi_interface_t &smi,
                     mii_interface_t &mii,
                     chanend xtcp[], int num_xtcp,
                     xtcp_ipconfig_t& ipconfig,
                     char mac_address[6]);
