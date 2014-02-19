/*****************************************************************************
*
* Filename:         uip_single_server.xc
* Author:           David Lacey
*                   Christian Schlittler
* Version:          1.0
* Creation date:	15.10.2013
*
* Copyright:        Christian Schlittler, christian.schlittler@gmx.ch
*                   2013, ZHAW School of Engineering
*                   2013, XMOS Ltd, All rights reserved
*                   This software is freely distributable under a derivative of the
*                   University of Illinois/NCSA Open Source License posted in
*
* Project:          XMOS and 6LoWPAN
* Target:           XMOS sliceKIT with RFSlice extension
* Compiler:         xcc with xTIMEcomposer 13.0.0beta
*
* -----------------------------------------------------------------------------
*
* History:
*
* This code was written by David Lacey to run the uIP protocol stack in V1.0. Now
* an extension of the protocol stack to IPv6 is needed, so a newer version of
* uip is used. Therefore, some adaption had to be made in this file.
*
* -----------------------------------------------------------------------------
*
* Usage:
*
**************************************************************************** */

// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>



#include <xs1.h>
#include <xclib.h>
#include <safestring.h>
#include <print.h>
#include "xtcp_conf_derived.h"
#include "uip.h"
#include "uip-conf.h"

#define SIZE_ESTIMATE 0

#if !XTCP_SEPARATE_MAC

#include "xtcp_client.h"

#ifndef UIP_SINGLE_THREAD_RX_BUFFER_SIZE
#define UIP_SINGLE_THREAD_RX_BUFFER_SIZE (3200*4)
#endif

#define UIP_MIN_SINGLE_THREAD_RX_BUFFER_SIZE (1524*4)

#if (UIP_SINGLE_THREAD_RX_BUFFER_SIZE) < (UIP_MIN_SINGLE_THREAD_RX_BUFFER_SIZE)
#warning UIP_SINGLE_THREAD_RX_BUFFER_SIZE is set too small for correct operation
#endif



#include "xtcp_server.h"
#include "uip_xtcp.h"
#include "uip.h"
#include "uip_server_support.h"

#include "smi.h"
#include "uip_single_server.h"
#include "mii_driver.h"
#include "mii_client.h"

extern int uip_static_ip;
extern xtcp_ipconfig_t uip_static_ipconfig;

/* -----------------------------------------------------------------------------
 * function to make the uIPv6 stack running
 *
 * The stack has been extracted from the contiki os
 * and ported to use it standalone on the XMOS
 * platform together with the standard XMOS
 * Ethernet MAC.
 *
 * This functions are necessary the run the TCP/IP
 * stack.
 * -------------------------------------------------------------------------- */
extern void etimer_request_poll(void); 	// Function form etimer
extern int process_run(void);			// poll the processces

/*----------------------------------------------------------------------------*/
#if !SIZE_ESTIMATE
// Global variables from uip_server_support
extern unsigned short uip_len;

#define XTCP_UIP_BUF32	1
#define xuip_buf32 (uip_aligned_buf.u32)

// Global functions from the uip stack
extern void uip_arp_timer(void);
extern void autoip_periodic();
extern void igmp_periodic();
#endif /* SIZE_ESTIMATE */

#pragma unsafe arrays
static void
copy_packet(uint32_t dst[], unsigned src, unsigned len)
{
	unsigned word_len = (len+3) >> 2;
	unsigned d;
	for (unsigned i=0; i<word_len; ++i)
	{
		asm( "ldw %0, %1[%2]" : "=r"(d) : "r"(src) , "r"(i) );
		asm( "stw %0, %1[%2]" :: "r"(d) , "r"(dst) , "r"(i) );
	}
}

/*------------------------------------------------------------------------------
 * theServer
 *
 * This function handles the incoming and outgoing messages. This is the
 * nerve centre of the TPC/IP protocol and the Ethernet MAC.
 *
 * \param mac_rx			Receive channel of the Ethernet MAC
 * \param mac_tx			Transmit channel to the Ethernet MAC
 * \param cNotifications
 * \param smi
 *
 * \param xtcp				Channels to register applications listen/write to
 * \param num_xtcp			Number of xtcp channels.
 * \param ipconfig			IP address. Depends if it is used with
 * 							IPv4 or IPv6.
 *
 * \param mac_address		the 48-bit Ethernet MAC address
 * ---------------------------------------------------------------------------*/

static void theServer(chanend mac_rx, chanend mac_tx,
					  chanend cNotifications,
                      smi_interface_t &smi,
                      chanend xtcp[], int num_xtcp,
                      xtcp_ipconfig_t& ipconfig,
                      char mac_address[6]) {
#define TIMEOUT		10000000		// 10'000'000 x 10ns = 0.1 sec.
	int address, length, timeStamp;

	timer tmr;
	unsigned timeout;
#if UIP_CONF_IPV4
	unsigned arp_timer=0;
#endif
	unsigned stimer_upd=0;

#if UIP_USE_AUTOIP
	unsigned autoip_timer=0;
#endif

	// Control structure for MII LLD
	struct miiData miiData;

	// The Receive packet buffer
	int b[UIP_SINGLE_THREAD_RX_BUFFER_SIZE/4];
#if !SIZE_ESTIMATE
	uip_server_init(xtcp, num_xtcp, ipconfig, mac_address);
#endif
	mii_buffer_init(miiData, mac_rx, cNotifications, b, UIP_SINGLE_THREAD_RX_BUFFER_SIZE/4);
	mii_out_init(mac_tx);

	/* Time base for the protocol stack */
	tmr :> timeout;
	timeout += TIMEOUT;

	while (1) {
#if !SIZE_ESTIMATE
		xtcpd_service_clients(xtcp, num_xtcp);

		xtcpd_check_connection_poll(mac_tx);

		xtcpd_uip_checkstate();
		xtcp_process_udp_acks(mac_tx);

		/* ***********************************
		 * call of the etimer module
		 *
		 * this could be optimised if you would
		 * use a dedicated xCore timer for this.
		 * *********************************** */
		etimer_request_poll();

		/* ***********************************
		 * periodic call of the process
		 * *********************************** */
		process_run();
#endif /* SIZE_ESTIMATE */
		select {
		/* ***********************************
		 * Do some periodical stuff
		 * *********************************** */
		case tmr when timerafter(timeout) :> unsigned:
			timeout += TIMEOUT;

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

#if UIP_CONF_IPV4
			if (++arp_timer == 100) {
				arp_timer=0;
				uip_arp_timer();
			}
#endif /* UIP_CONF_IPV4 */

#if UIP_USE_AUTOIP
			if (++autoip_timer == 5) {
				autoip_timer = 0;
				autoip_periodic();
				if (uip_len > 0) {
					xtcp_tx_buffer(mac_tx);
				}
			}
#endif
#if !SIZE_ESTIMATE
			/* update second timer */
			stimer_upd = (stimer_upd + 1)%10;
			if(stimer_upd == 0){
				upd_clock_seconds();
			}

			xtcp_process_timer(mac_tx, XTCP_TMR_PERIODIC);
#endif /* SIZE_ESTIMATE */
			break;

		/* ************************************
		 * A package is coming in from the MAC
		 * ************************************ */
		case inuchar_byref(cNotifications, miiData.notifySeen):
			do {
				{address,length,timeStamp} = mii_get_in_buffer(miiData);
				if (address != 0) {
					static unsigned pcnt=1;
					if (length <= UIP_BUFSIZE) {
						uip_len = length;
#if !SIZE_ESTIMATE
						copy_packet(xuip_buf32, address, length);
#endif
#if BUF_32BIT_ALIGN
						/* XXX CHSC: Alignment hack: shift the hole buffer 2 position to align it on 32bit  */
						for(int i = sizeof(uip_aligned_buf)-3; i>1; i--){
							uip_aligned_buf.u8[i+2] = uip_aligned_buf.u8[i];
						}
#endif
						/* Deliver an incoming packet to the TCP/IP stack */
#if !SIZE_ESTIMATE
						xtcpip_input(mac_tx);
#endif
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

/*-----------------------------------------------------------------------------
 * This function is just here to stop the compiler warning about an unused
 * chanend.
 *----------------------------------------------------------------------------*/
static inline void doNothing(chanend c) {
  asm(""::"r"(c));
}

/*-----------------------------------------------------------------------------
 * uip_single_server
 *
 * \param	p_mii_resetn
 * \param	smi
 * \param	mii
 * \param	xtcp
 * \param	num_xtcp
 * \param	ipconfig
 * \param	mac_address
 *---------------------------------------------------------------------------*/
void uip_single_server(out port ?p_mii_resetn,
                       smi_interface_t &smi,
                       mii_interface_lite_t &mii,
                       chanend xtcp[],
                       int num_xtcp,
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
/*---------------------------------------------------------------------------*/
#endif /* !XTCP_SEPARATE_MAC */
