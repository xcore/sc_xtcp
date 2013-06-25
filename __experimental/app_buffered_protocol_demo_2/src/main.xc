// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>
#include "xtcp.h"
#include "ethernet_board_support.h"
#include "demo_protocol.h"

// These intializers are taken from the ethernet_board_support.h header for
// XMOS dev boards. If you are using a different board you will need to
// supply explicit port structure intializers for these values
ethernet_xtcp_ports_t xtcp_ports =
  {on ETHERNET_DEFAULT_TILE: OTP_PORTS_INITIALIZER,
     ETHERNET_DEFAULT_SMI_INIT,
     ETHERNET_DEFAULT_MII_INIT_lite,
     ETHERNET_DEFAULT_RESET_INTERFACE_INIT};


#if 1
// Static IP Config - change this to suit your network (current is link local address)
xtcp_ipconfig_t ipconfig =
{
  {169,254,3,2},   // ip address
  {255,255,0,0},   // netmask
  {0,0,0,0}        // gateway
};
#else
// Use dynamic addressing DHCP
xtcp_ipconfig_t ipconfig = {{0,0,0,0},{0,0,0,0},{0,0,0,0}};
#endif

// Program entry point
int main(void)
{
  chan c_xtcp[1];

	par
	{
               on ETHERNET_DEFAULT_TILE: ethernet_xtcp_server(xtcp_ports,
                                                              ipconfig,
                                                              c_xtcp,
                                                              1);


		// The server thread
		on stdcore[0]: demo_protocold(c_xtcp[0]);


	}
        return 0;
}
