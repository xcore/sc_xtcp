// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>


/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: uip-split.c,v 1.2 2006/06/12 08:00:30 adam Exp $
 */

#include <string.h>

#include "uip-split.h"
#include "uip.h"

#include "uip_arch.h"

#include <xccompat.h>
#include "xcoredev.h"

#include <xclib.h>
#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

#ifndef UIP_PACKET_SPLIT_THRESHOLD
#define UIP_PACKET_SPLIT_THRESHOLD ((UIP_BUFSIZE) / 2)
#endif

#define ACTUAL_UIP_PACKET_SPLIT_THRESHOLD (UIP_PACKET_SPLIT_THRESHOLD > 4 ? UIP_PACKET_SPLIT_THRESHOLD : 4)


/*-----------------------------------------------------------------------------*/
#if UIP_SLIDING_WINDOW
extern int uip_do_split;
#endif

static void
uip_split_output_send(chanend mac_tx)
{
	/* Recalculate the TCP checksum. */
	BUF->tcpchksum = 0;
	BUF->tcpchksum = ~(uip_tcpchksum());

#if !UIP_CONF_IPV6
	/* Recalculate the IP checksum. */
	BUF->ipchksum = 0;
	BUF->ipchksum = ~(uip_ipchksum());
#endif
	uip_len += UIP_LLH_LEN;

	/* Transmit the first packet. */
	xcoredev_send(mac_tx);
}


// The purpose of this function is to defeat Neagles Algorithm.  For packets
// over half of the size of the max frame size (which we assume to be part of
// a data stream and therefore need to be ACKed quickly) we break the packet
// in two, which means that the Neagles Algorithm effect of ACKing only every
// other packet will generate ACKs for us for each transmission.
//
// Without this, since we do not have a sliding window, and therefore do not
// transmit packet 2 until packet 1 has been acknowledged, we have to wait
// until the other end times out of the 300ms delay before sending us an ACK.
void
uip_split_output(chanend mac_tx)
{
	u16_t tcplen, len1, len2;

	if (BUF->proto == UIP_PROTO_TCP) {
#if UIP_SLIDING_WINDOW
          if (uip_do_split)
#else
          int data_len = uip_len - UIP_TCPIP_HLEN - UIP_LLH_LEN;
          if (data_len > ACTUAL_UIP_PACKET_SPLIT_THRESHOLD)
#endif
            {
			tcplen = uip_len - UIP_TCPIP_HLEN - UIP_LLH_LEN;
			/* Split the segment in two, making sure the first segment is an
			 * integer number of words */
			len1 = (tcplen / 2) & ~3;
			len2 = tcplen - len1;

			/* Create the first packet. This is done by altering the length
			 field of the IP header and updating the checksums. */
			uip_len = len1 + UIP_TCPIP_HLEN;
#if UIP_CONF_IPV6
			/* For IPv6, the IP length field does not include the IPv6 IP header length. */
			BUF->len[0] = ((uip_len - UIP_IPH_LEN) >> 8);
			BUF->len[1] = ((uip_len - UIP_IPH_LEN) & 0xff);
#else
			BUF->len[0] = uip_len >> 8;
			BUF->len[1] = uip_len & 0xff;
#endif

			uip_split_output_send(mac_tx);

			/* Now, create the second packet. To do this, it is not enough to
			 just alter the length field, but we must also update the TCP
			 sequence number and point the uip_appdata to a new place in
			 memory. This place is determined by the length of the first
			 packet (len1). */
			uip_len = len2 + UIP_TCPIP_HLEN;
#if UIP_CONF_IPV6
			/* For IPv6, the IP length field does not include the IPv6 IP header length. */
			BUF->len[0] = ((uip_len - UIP_IPH_LEN) >> 8);
			BUF->len[1] = ((uip_len - UIP_IPH_LEN) & 0xff);
#else
			BUF->len[0] = uip_len >> 8;
			BUF->len[1] = uip_len & 0xff;
#endif

			/* Sadly we must copy the packet contents down to the bottom of the packet.
			 *
			 * We know that the data sections are on a short word boundary, and not
			 * on a word boundary
			 */
			((u16_t*) uip_appdata)[0] = ((u16_t *) (uip_appdata + len1))[0];
			{
				unsigned* dst = (unsigned*) (uip_appdata + 2);
				unsigned* src = (unsigned*) (uip_appdata + 2 + len1);
				for (unsigned i = 0; i < (len2 / 4 + 1); ++i) {
					dst[i] = src[i];
				}
			}

			uip_add32(BUF->seqno, len1);
			xtcp_copy_word(BUF->seqno, uip_acc32);

			uip_split_output_send(mac_tx);
		} else {
			// We didn't compute the checksum earlier
			BUF->tcpchksum = 0;
			BUF->tcpchksum = ~(uip_tcpchksum());
			xcoredev_send(mac_tx);
		}
	} else {
		xcoredev_send(mac_tx);
	}
}
/*-----------------------------------------------------------------------------*/
