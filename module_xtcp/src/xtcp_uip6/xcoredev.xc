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

#else /* XTCP_SEPARATE_MAC */

#include "net/uip.h"
#include "mii_client.h"

// Global variables from uip_server_support
extern unsigned short uip_len;

#define xuip_buf32 (uip_aligned_buf.u32)

#ifndef UIP_MAX_TRANSMIT_SIZE
#define UIP_MAX_TRANSMIT_SIZE 1520
#endif

#pragma unsafe arrays
void xcoredev_send(chanend tx)
{
#ifdef UIP_SINGLE_SERVER_DOUBLE_BUFFER_TX
    static int txbuf0[(UIP_MAX_TRANSMIT_SIZE+3)/4];
    static int txbuf1[(UIP_MAX_TRANSMIT_SIZE+3)/4];
    static int tx_buf_in_use=0;
    static int n=0;
    int len = uip_len;
    unsigned nWords;
    if (len<60) {
        for (int i=len;i<60;i++)
            (xuip_buf32, unsigned char[])[i] = 0;
        len=60;
    }
    nWords = (len+3)>>2;

    if (len > UIP_MAX_TRANSMIT_SIZE) {
#ifdef UIP_DEBUG_MAX_TRANSMIT_SIZE
        printstr("Error: Trying to send too big a packet: ");
        printint(len);
        printstr(" bytes.\n");
#endif
        return;
    }
    switch (n) {
    case 0:
        for (unsigned i=0; i<nWords; ++i) { txbuf0[i] = xuip_buf32[i]; }
        if (tx_buf_in_use) mii_out_packet_done(tx, txbuf1);
        mii_out_packet(tx, txbuf0, 0, len);
        n = 1;
        break;
    case 1:
        for (unsigned i=0; i<nWords; ++i) { txbuf1[i] = xuip_buf32[i]; }
        if (tx_buf_in_use) mii_out_packet_done(tx, txbuf0);
        mii_out_packet(tx, txbuf1, 0, len);
        n = 0;
        break;
    }
    tx_buf_in_use=1;
#else
    static int txbuf[(UIP_MAX_TRANSMIT_SIZE+3)/4];
    static int tx_buf_in_use=0;
    unsigned nWords;
    int len=uip_len;

#if BUF_32BIT_ALIGN
    //XXX CHSC: Alignment Hack
    for(int i = 0; i<uip_len; i++){
      uip_aligned_buf.u8[i] = uip_aligned_buf.u8[i+2];
    }
    // End of alignment hack
#endif /* BUF_32BIT_ALIGN */

    if (tx_buf_in_use)
        mii_out_packet_done(tx);
    if (len<60) {
        for (int i=len;i<60;i++){
            (xuip_buf32, unsigned char[])[i] = 0;
        }
        len=60;
    }
    nWords = (len+3)>>2;
    for (unsigned i=0; i<nWords; ++i) { txbuf[i] = xuip_buf32[i]; }
    mii_out_packet(tx, txbuf, 0, len);
    tx_buf_in_use=1;
#endif /* UIP_SINGLE_SERVER_DOUBLE_BUFFER_TX */
}
#endif /* XTCP_SEPARATE_MAC */
