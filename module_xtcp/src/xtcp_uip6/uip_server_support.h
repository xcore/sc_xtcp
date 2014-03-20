/*****************************************************************************
*
* Filename:         uip_server_support.h
* Author:           Christian Schlittler
* Version:          1.0
* Creation date:	25.10.2013
*
* Copyright:        Christian Schlittler, christian.schlittler@gmx.ch
*
* Project:			XMOS and 6LoWPAN
* Target:			XMOS sliceKIT with RFSlice extension
* Compiler:
*
* -----------------------------------------------------------------------------
*
* History:
*
* -----------------------------------------------------------------------------
*
* Usage:
*
**************************************************************************** */

#ifndef UIP_SERVER_SUPPORT_H_
#define UIP_SERVER_SUPPORT_H_

#include <xccompat.h>
#include <xtcp_client.h>

typedef enum xtcp_tmr_event_type_t {
	XTCP_TMR_PERIODIC,
} xtcp_tmr_event_type_t;

void uip_server_init(chanend xtcp[], int num_xtcp,
                     REFERENCE_PARAM(xtcp_ipconfig_t, ipconfig),
                     unsigned char mac_address[6]);
void xtcpd_check_connection_poll(chanend mac_tx);
void xtcp_tx_buffer(chanend mac_tx);
void xtcp_process_incoming_packet(chanend mac_tx);
void xtcp_process_udp_acks(chanend mac_tx);
void xtcp_process_timer(chanend mac_tx, xtcp_tmr_event_type_t event);

void xtcpip_input(chanend mac_tx);
void xtcpip_ipv6_output(chanend mac_tx);


#endif /* UIP_SERVER_SUPPORT_H_ */
