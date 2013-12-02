// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/*
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 */

#ifndef _netbios_protocol_h_
#define _netbios_protocol_h_

/** default port number for "NetBIOS Name service */
#define NETBIOS_PORT     137

/** size of a NetBIOS name */
#define NETBIOS_NAME_LEN 16

/** The Time-To-Live for NetBIOS name responds (in seconds)
 * Default is 300000 seconds (3 days, 11 hours, 20 minutes) */
#define NETBIOS_NAME_TTL 300000

/** NetBIOS header flags */
#define NETB_HFLAG_RESPONSE           0x8000
#define NETB_HFLAG_OPCODE             0x7800
#define NETB_HFLAG_OPCODE_NAME_QUERY  0x0000
#define NETB_HFLAG_AUTHORATIVE        0x0400
#define NETB_HFLAG_TRUNCATED          0x0200
#define NETB_HFLAG_RECURS_DESIRED     0x0100
#define NETB_HFLAG_RECURS_AVAILABLE   0x0080
#define NETB_HFLAG_BROADCAST          0x0010
#define NETB_HFLAG_REPLYCODE          0x0008
#define NETB_HFLAG_REPLYCODE_NOERROR  0x0000

/** NetBIOS name flags */
#define NETB_NFLAG_UNIQUE             0x8000
#define NETB_NFLAG_NODETYPE           0x6000
#define NETB_NFLAG_NODETYPE_HNODE     0x6000
#define NETB_NFLAG_NODETYPE_MNODE     0x4000
#define NETB_NFLAG_NODETYPE_PNODE     0x2000
#define NETB_NFLAG_NODETYPE_BNODE     0x0000

/** NetBIOS message header */
struct netbios_hdr {
  u16_t trans_id;
  u16_t flags;
  u16_t questions;
  u16_t answerRRs;
  u16_t authorityRRs;
  u16_t additionalRRs;
};

#define ENCODED_NETBIOS_NAME_LEN (NETBIOS_NAME_LEN*2)+1

/** NetBIOS message name part */
struct netbios_name_hdr {
  u8_t  nametype;
  u8_t  encname[ENCODED_NETBIOS_NAME_LEN];
  u16_t type;
  u16_t class;
  u32_t ttl;
  u16_t datalen;
  u16_t flags;
  u8_t addr[4];
}  __attribute__((packed));


/** NetBIOS message */
struct netbios_resp
{
  struct netbios_hdr      resp_hdr;
  struct netbios_name_hdr resp_name;
} __attribute__((packed));

#endif // _netbios_protocol_h_
