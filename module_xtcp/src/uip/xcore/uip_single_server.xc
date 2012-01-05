// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include <xclib.h>
#include <safestring.h>
#include <print.h>

#ifdef __xtcp_client_conf_h_exists__
#include "xtcp_client_conf.h"
#endif

#ifdef UIP_USE_SINGLE_THREADED_ETHERNET

#include "xtcp_server.h"
#include "uip_xtcp.h"

#include "uip_single_server.h"
#include "miiDriver.h"
#include "miiClient.h"

extern char notifySeen;

extern int uip_static_ip;
extern xtcp_ipconfig_t uip_static_ipconfig;

// Functions from uip_server_support
extern void uip_server_init(chanend xtcp[], int num_xtcp, xtcp_ipconfig_t& ipconfig, unsigned char mac_address[6]);
extern void xtcpd_check_connection_poll(chanend mac_tx);
extern void xtcp_tx_buffer(chanend mac_tx);
extern void xtcp_process_incoming_packet(chanend mac_tx);
extern void xtcp_process_udp_acks(chanend mac_tx);

extern unsigned short uip_len;
extern unsigned int uip_buf32[];

// The transmit packet buffer
static int txbuf[400];
static int tx_buf_in_use=0;

void xcoredev_send(chanend tx)
{
	unsigned nWords = (uip_len+3)>>2;
	if (tx_buf_in_use) miiOutPacketDone(tx);
	for (unsigned i=0; i<nWords; ++i) { txbuf[i] = uip_buf32[i]; }
    miiOutPacket(tx, txbuf, 0, (uip_len<60)?60:uip_len);
    tx_buf_in_use=1;
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
		chanend xtcp[], int num_xtcp, xtcp_ipconfig_t& ipconfig,
		char mac_address[6]) {
    int address, length, timeStamp;

	timer tmr;
	unsigned timeout;

	// The Receive packet buffer
    int b[3200];

    uip_server_init(xtcp, num_xtcp, ipconfig, mac_address);

    miiBufferInit(mac_rx, cNotifications, b, 3200);
    miiOutInit(mac_tx);

    tmr :> timeout;
    timeout += 10000000;

    while (1) {
		xtcpd_service_clients(xtcp, num_xtcp);
		xtcpd_check_connection_poll(mac_tx);
		uip_xtcp_checkstate();
		//uip_xtcp_checklink(connect_status);

		xtcp_process_udp_acks(mac_tx);

        select {
		case tmr when timerafter(timeout) :> unsigned:
			timeout += 10000000;
			//		if (timer_expired(&arp_timer)) {
			//			timer_reset(&arp_timer);
			//			uip_arp_timer();
			//		}
			//
			//		if (timer_expired(&autoip_timer)) {
			//			timer_reset(&autoip_timer);
			//			autoip_periodic();
			//			if (uip_len > 0) {
			//				send(mac_tx);
			//			}
			//		}
			//
			//		if (timer_expired(&periodic_timer)) {
			//
			//#if UIP_IGMP
			//			igmp_periodic();
			//			if(uip_len > 0) {
			//				send(mac_tx);
			//			}
			//#endif
			//			for (int i = 0; i < UIP_UDP_CONNS; i++) {
			//				uip_udp_periodic(i);
			//				if (uip_len > 0) {
			//					uip_arp_out(&uip_udp_conns[i]);
			//					send(mac_tx);
			//				}
			//			}
			//
			//			for (int i = 0; i < UIP_CONNS; i++) {
			//				uip_periodic(i);
			//				if (uip_len > 0) {
			//					uip_arp_out( NULL);
			//					send(mac_tx);
			//				}
			//			}
			//
			//			timer_reset(&periodic_timer);
			//		}
			break;


        case inuchar_byref(cNotifications, notifySeen):
			do {
				{address,length,timeStamp} = miiGetInBuffer();
				if (address != 0) {
					static unsigned pcnt=1;
					uip_len = length;
					copy_packet(uip_buf32, address, length);
					xtcp_process_incoming_packet(mac_tx);
		            miiFreeInBuffer(address);
		            miiRestartBuffer();
				}
			} while (address!=0);

            break;

//		default:
//			break;

        }
    }
}

void uipSingleServer(clock clk_smi,
                     out port ?p_mii_resetn,
                     smi_interface_t &smi,
                     mii_interface_t &mii,
                     chanend xtcp[], int num_xtcp,
                     xtcp_ipconfig_t& ipconfig,
                     char mac_address[6]) {
    chan cIn, cOut;
    chan notifications;
    par {
        miiDriver(clk_smi, p_mii_resetn, smi, mii, cIn, cOut, 0);
        theServer(cIn, cOut, notifications, xtcp, num_xtcp, ipconfig, mac_address);
    }
}

#endif


