// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include <xclib.h>
#include <safestring.h>
#include <print.h>

#include "xtcp_conf_derived.h"

#if !XTCP_SEPARATE_MAC

#include "xtcp_client.h"
#define UIP_IPH_LEN    20    /* Size of IP header */
#define UIP_UDPH_LEN    8    /* Size of UDP header */
#define UIP_TCPH_LEN   20    /* Size of TCP header */
#define UIP_IPUDPH_LEN (UIP_UDPH_LEN + UIP_IPH_LEN)    /* Size of IP +
							  UDP
							  header */
#define UIP_IPTCPH_LEN (UIP_TCPH_LEN + UIP_IPH_LEN)    /* Size of IP +
							  TCP
							  header */
#define UIP_TCPIP_HLEN UIP_IPTCPH_LEN

#define UIP_LLH_LEN     14

#define UIP_BUFSIZE     (XTCP_CLIENT_BUF_SIZE + UIP_LLH_LEN + UIP_TCPIP_HLEN)

#ifndef UIP_USE_AUTOIP
#define UIP_USE_AUTOIP 1
#endif

#ifndef UIP_SINGLE_THREAD_RX_BUFFER_SIZE
#define UIP_SINGLE_THREAD_RX_BUFFER_SIZE (3200*4)
#endif

#define UIP_MIN_SINGLE_THREAD_RX_BUFFER_SIZE (1524*4)

#if (UIP_SINGLE_THREAD_RX_BUFFER_SIZE) < (UIP_MIN_SINGLE_THREAD_RX_BUFFER_SIZE)
#warning UIP_SINGLE_THREAD_RX_BUFFER_SIZE is set too small for correct operation
#endif

#ifndef UIP_MAX_TRANSMIT_SIZE
#define UIP_MAX_TRANSMIT_SIZE 1520
#endif

#include "xtcp_server.h"
#include "uip_xtcp.h"

#include "smi.h"
#include "uip_single_server.h"
#include "mii_driver.h"
#include "mii_client.h"

extern int uip_static_ip;
extern xtcp_ipconfig_t uip_static_ipconfig;

// Functions from uip_server_support
extern void uip_server_init(chanend xtcp[], int num_xtcp, xtcp_ipconfig_t& ipconfig, unsigned char mac_address[6]);
extern void xtcpd_check_connection_poll(chanend mac_tx);
extern void xtcp_tx_buffer(chanend mac_tx);
extern void xtcp_process_incoming_packet(chanend mac_tx, int length);
extern void xtcp_process_udp_acks(chanend mac_tx);
extern void xtcp_process_periodic_timer(chanend mac_tx);

// Global variables from uip_server_support
extern unsigned short uip_len;
extern unsigned int uip_buf32[];

// Global functions from the uip stack
extern void uip_arp_timer(void);
extern void autoip_periodic();
extern void igmp_periodic();

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
      (uip_buf32, unsigned char[])[i] = 0;
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
    for (unsigned i=0; i<nWords; ++i) { txbuf0[i] = uip_buf32[i]; }
    if (tx_buf_in_use) mii_out_packet_done(tx, txbuf1);
    mii_out_packet(tx, txbuf0, 0, len);
    n = 1;
    break;
  case 1:
    for (unsigned i=0; i<nWords; ++i) { txbuf1[i] = uip_buf32[i]; }
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
    if (tx_buf_in_use)
      mii_out_packet_done(tx);
    if (len<60) {
      for (int i=len;i<60;i++)
        (uip_buf32, unsigned char[])[i] = 0;
      len=60;
    }
    nWords = (len+3)>>2;
    for (unsigned i=0; i<nWords; ++i) { txbuf[i] = uip_buf32[i]; }
    mii_out_packet(tx, txbuf, 0, len);
    tx_buf_in_use=1;
#endif
}

#pragma unsafe arrays
void copy_packet(unsigned dst[], unsigned src, unsigned len)
{
	unsigned word_len = (len+3) >> 2;
	unsigned d;
	for (unsigned i=0; i<word_len; ++i)
	{
		asm( "ldw %0, %1[%2]" : "=r"(d) : "r"(src) , "r"(i) );
		asm( "stw %0, %1[%2]" :: "r"(d) , "r"(dst) , "r"(i) );
	}
}

static void theServer(chanend mac_rx, chanend mac_tx, chanend cNotifications,
                      smi_interface_t &smi,
		chanend xtcp[], int num_xtcp, xtcp_ipconfig_t& ipconfig,
		char mac_address[6]) {
    int address, length, timeStamp;

	timer tmr;
	unsigned timeout;
	unsigned arp_timer=0;

#if UIP_USE_AUTOIP
	unsigned autoip_timer=0;
#endif

	// Control structure for MII LLD
	struct miiData miiData;

	// The Receive packet buffer
    int b[UIP_SINGLE_THREAD_RX_BUFFER_SIZE/4];

    uip_server_init(xtcp, num_xtcp, ipconfig, mac_address);

    mii_buffer_init(miiData, mac_rx, cNotifications, b, UIP_SINGLE_THREAD_RX_BUFFER_SIZE/4);
    mii_out_init(mac_tx);

    tmr :> timeout;
    timeout += 10000000;

    while (1) {

      xtcpd_service_clients(xtcp, num_xtcp);
      xtcpd_check_connection_poll(mac_tx);
      uip_xtcp_checkstate();
      xtcp_process_udp_acks(mac_tx);

        select {
		case tmr when timerafter(timeout) :> unsigned:
			timeout += 10000000;

			// Check for the link state
			{
				static int linkstate=0;
				int status = smi_check_link_state(smi);
				if (!status && linkstate) {
				  uip_linkdown();
				}
				if (status && !linkstate) {
				  uip_linkup();
				}
				linkstate = status;
			}

			if (++arp_timer == 100) {
				arp_timer=0;
				uip_arp_timer();
			}

#if UIP_USE_AUTOIP
			if (++autoip_timer == 5) {
				autoip_timer = 0;
				autoip_periodic();
				if (uip_len > 0) {
					xtcp_tx_buffer(mac_tx);
				}
			}
#endif

			xtcp_process_periodic_timer(mac_tx);
			break;


        case inuchar_byref(cNotifications, miiData.notifySeen):
			do {
				{address,length,timeStamp} = mii_get_in_buffer(miiData);
				if (address != 0) {
                                  static unsigned pcnt=1;
                                  if (length <= UIP_BUFSIZE) {
                                    copy_packet(uip_buf32, address, length);
                                    xtcp_process_incoming_packet(mac_tx, length);
                                  }
                                  mii_free_in_buffer(miiData, address);
                                  mii_restart_buffer(miiData);
				}
			} while (address!=0);

            break;
		default:
			break;

        }

    }

}


// This funciton is just here to stop the compiler warning about an unused
// chanend.
static inline void doNothing(chanend c) {
  asm(""::"r"(c));
}

void uip_single_server(out port ?p_mii_resetn,
                       smi_interface_t &smi,
                       mii_interface_lite_t &mii,
                       chanend xtcp[], int num_xtcp,
                       xtcp_ipconfig_t& ipconfig,
                       char mac_address[6]) {
    chan cIn, cOut;
    chan notifications;
	mii_initialise(p_mii_resetn, mii);
#ifndef MII_NO_SMI_CONFIG
	smi_init(smi);
	eth_phy_config(1, smi);
#endif
    par {
      {doNothing(notifications);mii_driver(mii, cIn, cOut);}
      theServer(cIn, cOut, notifications, smi, xtcp, num_xtcp, ipconfig, mac_address);
    }
}




#endif
