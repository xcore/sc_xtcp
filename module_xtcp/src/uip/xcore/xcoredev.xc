/**
 * Module:  module_xtcp
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    xcoredev.xc
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
#include <print.h>
#include <xs1.h>
#include "ethernet_rx_client.h"
#include "ethernet_tx_client.h"
#include "ethernet_rx_filter.h"
#ifdef MAC_CUSTOM_FILTER
#include "mac_custom_filter.h"
#endif


extern unsigned short uip_len;
extern unsigned int uip_buf32[];

static unsigned char mac_addr[6];

/*---------------------------------------------------------------------------*/
void
xcoredev_init(chanend rx, chanend tx)
{
#ifndef MAC_CUSTOM_FILTER
  FrameFilterFormat_t filter={0};
  int i;
  int result;
#endif

  mac_get_macaddr(tx, mac_addr);

  // Configure the mac link to send the server anything
  // arp or ip


#ifdef MAC_CUSTOM_FILTER
   ethernet_rx_set_custom_filter(rx, MAC_FILTER_ARPIP);
#else
  // arp
   filter.filterOpcode = FILTER_OPCODE_OR;
   for (i = 0; i < NUM_BYTES_IN_FRAME_FILTER; i++)
   {
      filter.DMAC_filterMask[i] = 0xff;
      filter.DMAC_filterCompare[i] = 0x0;
      switch (i)
        {
        case 0:
          filter.VLANET_filterMask[i] = 0xff;
          filter.VLANET_filterCompare[i] = 0x08;
          break;
        case 1:
          filter.VLANET_filterMask[i] = 0xff;
          filter.VLANET_filterCompare[i] = 0x06;
          break;
        default:
          filter.VLANET_filterMask[i] = 0;
          filter.VLANET_filterCompare[i] = 0;
          break;
        }
   }

   result = ethernet_rx_frame_filter_set(rx, 0, filter);

   // ip
   filter.filterOpcode = FILTER_OPCODE_OR;
   for (i = 0; i < NUM_BYTES_IN_FRAME_FILTER; i++)
   {
      filter.DMAC_filterMask[i] = 0xff;
      filter.DMAC_filterCompare[i] = 0x0;
      switch (i)
        {
        case 0:
          filter.VLANET_filterMask[i] = 0xff;
          filter.VLANET_filterCompare[i] = 0x08;
          break;
        case 1:
          filter.VLANET_filterMask[i] = 0xff;
          filter.VLANET_filterCompare[i] = 0x00;
          break;
        default:
          filter.VLANET_filterMask[i] = 0;
          filter.VLANET_filterCompare[i] = 0;
          break;
        }
   }

   result = ethernet_rx_frame_filter_set(rx, 1, filter);
#endif

}

/*---------------------------------------------------------------------------*/
#pragma unsafe arrays
unsigned int
xcoredev_read(chanend rx, int n)
{
  int ret;
  unsigned int src_port;
  int len;
  int nwords = (n+3) >> 2;
  unsigned char tmp;
  select 
    {
    case inuchar_byref(rx, tmp):    
      (void) inuchar(rx);
      (void) inuchar(rx);
      (void) inct(rx);
      master {
        rx <: ETHERNET_RX_FRAME_REQ;
      }
      slave {
        int twords;
        unsigned int ts;
        rx :> src_port;
        rx :> len;
        twords = (len+3)>>2;
        for (int i=0;i<twords;i++) {
          unsigned int data;
          rx :> data;
          if (i < nwords)
            uip_buf32[i] = data;
        }
        rx :> ts;
      }         
      ret = len <= n ? len : 0;
      break;
    default:
      ret = 0;
      break;
    }
  
  return ret;
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
