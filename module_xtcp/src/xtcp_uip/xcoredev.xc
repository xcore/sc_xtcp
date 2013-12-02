// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <print.h>
#include <xs1.h>
#include "uip_xtcp.h"
#include "xtcp_conf_derived.h"

#if XTCP_SEPARATE_MAC

#include "ethernet_rx_client.h"
#include "ethernet_tx_client.h"
#include "mac_filter.h"

extern unsigned short uip_len;
extern unsigned int uip_buf32[];

/*---------------------------------------------------------------------------*/
void
xcoredev_init(chanend rx, chanend tx)
{
  // Configure the mac link to send the server anything
  // arp or ip
  mac_set_custom_filter(rx, MAC_FILTER_ARPIP);
  mac_request_status_packets(rx);
}

/*---------------------------------------------------------------------------*/
#pragma unsafe arrays
unsigned int
xcoredev_read(chanend rx, int n)
{
  unsigned int len = 0;
  unsigned int src_port;
  select
    {
    case safe_mac_rx(rx, (uip_buf32, unsigned char[]), len, src_port, n):
      if (len == STATUS_PACKET_LEN) {
        if ((uip_buf32, unsigned char[])[0]) {
          uip_linkup();
        }
        else {
          uip_linkdown();
        }
        return 0;
      }
      break;
    default:
      break;
    }
  return len <= n ? len : 0;
}

/*---------------------------------------------------------------------------*/
void
xcoredev_send(chanend tx)
{
  int len = uip_len;
  if (len != 0) {
    if (len < 64)  {
      for (int i=len;i<64;i++)
        (uip_buf32, unsigned char[])[i] = 0;
      len=64;
    }

    mac_tx(tx, uip_buf32, len, -1);
  }
}
/*---------------------------------------------------------------------------*/

#endif

