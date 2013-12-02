// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>


/**
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: dhcpc.c,v 1.2 2006/06/11 21:46:37 adam Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <print.h>
#include "uip.h"
#include "dhcpc.h"
#include "timer.h"
#include "pt.h"
#include "autoip.h"

#define STATE_INITIAL         0
#define STATE_SENDING         1
#define STATE_OFFER_RECEIVED  2
#define STATE_CONFIG_RECEIVED 3
#define STATE_DISABLED        4

static struct dhcpc_state s;

struct dhcp_msg {
  u8_t op, htype, hlen, hops;
  u8_t xid[4];
  u16_t secs, flags;
  u8_t ciaddr[4];
  u8_t yiaddr[4];
  u8_t siaddr[4];
  u8_t giaddr[4];
  u8_t chaddr[16];
#ifndef UIP_CONF_DHCP_LIGHT
  u8_t sname[64];
  u8_t file[128];
#endif
  u8_t options[312];
};

#define BOOTP_BROADCAST 0x8000

#define DHCP_REQUEST        1
#define DHCP_REPLY          2
#define DHCP_HTYPE_ETHERNET 1
#define DHCP_HLEN_ETHERNET  6
#define DHCP_MSG_LEN      236

#define DHCPC_SERVER_PORT  67
#define DHCPC_CLIENT_PORT  68

#define DHCPDISCOVER  1
#define DHCPOFFER     2
#define DHCPREQUEST   3
#define DHCPDECLINE   4
#define DHCPACK       5
#define DHCPNAK       6
#define DHCPRELEASE   7

#define DHCP_OPTION_SUBNET_MASK   1
#define DHCP_OPTION_ROUTER        3
#define DHCP_OPTION_DNS_SERVER    6
#define DHCP_OPTION_REQ_IPADDR   50
#define DHCP_OPTION_LEASE_TIME   51
#define DHCP_OPTION_MSG_TYPE     53
#define DHCP_OPTION_SERVER_ID    54
#define DHCP_OPTION_REQ_LIST     55
#define DHCP_OPTION_END         255

#if UIP_USE_DHCP

static u8_t xid[4];
static unsigned int rand_seed;
static unsigned int rand_startup;
static const u8_t magic_cookie[4] = {99, 130, 83, 99};
/*---------------------------------------------------------------------------*/
static u8_t *
add_msg_type(u8_t *optptr, u8_t type)
{
  *optptr++ = DHCP_OPTION_MSG_TYPE;
  *optptr++ = 1;
  *optptr++ = type;
  return optptr;
}
/*---------------------------------------------------------------------------*/
static u8_t *
add_server_id(u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_SERVER_ID;
  *optptr++ = 4;
  memcpy(optptr, s.serverid, 4);
  return optptr + 4;
}
/*---------------------------------------------------------------------------*/
static u8_t *
add_req_ipaddr(u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_REQ_IPADDR;
  *optptr++ = 4;
  memcpy(optptr, s.ipaddr, 4);
  return optptr + 4;
}
/*---------------------------------------------------------------------------*/
static u8_t *
add_req_options(u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_REQ_LIST;
  *optptr++ = 3;
  *optptr++ = DHCP_OPTION_SUBNET_MASK;
  *optptr++ = DHCP_OPTION_ROUTER;
  *optptr++ = DHCP_OPTION_DNS_SERVER;
  return optptr;
}
/*---------------------------------------------------------------------------*/
static u8_t *
add_end(u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_END;
  return optptr;
}
/*---------------------------------------------------------------------------*/
__attribute__ ((noinline)) static void
create_msg(register struct dhcp_msg *m)
{
  m->op = DHCP_REQUEST;
  m->htype = DHCP_HTYPE_ETHERNET;
  m->hlen = s.mac_len;
  m->hops = 0;
  memcpy(m->xid, xid, sizeof(m->xid));
  m->secs = 0;
  m->flags = HTONS(BOOTP_BROADCAST); /*  Broadcast bit. */

  if (s.state != STATE_CONFIG_RECEIVED)
  {
    memset(m->ciaddr, 0, sizeof(m->ciaddr));
  }
  else
  {
    memcpy(m->ciaddr, uip_hostaddr, sizeof(m->ciaddr));
  }

#ifndef UIP_CONF_DHCP_LIGHT
  memset(m->yiaddr, 0, sizeof(m->yiaddr)+sizeof(m->siaddr)+sizeof(m->giaddr)+sizeof(m->chaddr)+sizeof(m->sname)+sizeof(m->file));
#else
  memset(m->yiaddr, 0, sizeof(m->yiaddr)+sizeof(m->siaddr)+sizeof(m->giaddr)+sizeof(m->chaddr));
#endif

  memcpy(m->chaddr, s.mac_addr, s.mac_len);
  memcpy(m->options, magic_cookie, sizeof(magic_cookie));
}
/*---------------------------------------------------------------------------*/
static void
send_discover(void)
{
  u8_t *end;
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;
  create_msg(m);

  end = add_msg_type(&m->options[4], DHCPDISCOVER);
  end = add_req_options(end);
  end = add_end(end);

  uip_send(uip_appdata, end - (u8_t *)uip_appdata);

}
/*---------------------------------------------------------------------------*/
void
send_request(void)
{
  u8_t *end;
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;

  create_msg(m);

  end = add_msg_type(&m->options[4], DHCPREQUEST);
  end = add_server_id(end);
  end = add_req_ipaddr(end);
  end = add_end(end);

  uip_send(uip_appdata, end - (u8_t *)uip_appdata);
}
/*---------------------------------------------------------------------------*/
static u8_t
parse_options(u8_t *optptr, int len)
{
  u8_t *end = optptr + len;
  u8_t type = 0;

  while(optptr < end) {
    switch(*optptr) {
    case DHCP_OPTION_SUBNET_MASK:
      memcpy(s.netmask, optptr + 2, 4);
      break;
    case DHCP_OPTION_ROUTER:
      memcpy(s.default_router, optptr + 2, 4);
      break;
    case DHCP_OPTION_DNS_SERVER:
      memcpy(s.dnsaddr, optptr + 2, 4);
      break;
    case DHCP_OPTION_MSG_TYPE:
      type = *(optptr + 2);
      break;
    case DHCP_OPTION_SERVER_ID:
      memcpy(s.serverid, optptr + 2, 4);
      break;
    case DHCP_OPTION_LEASE_TIME:
      memcpy(s.lease_time, optptr + 2, 4);
      // Swap the bytes to correct the network ordering.
      unsigned char val;
      val = (s.lease_time[1] >> 8);
      s.lease_time[1] <<= 8;
      s.lease_time[1] |= val;
      val = (s.lease_time[0] >> 8);
      s.lease_time[0] <<= 8;
      s.lease_time[0] |= val;
      break;
    case DHCP_OPTION_END:
      return type;
    }

    optptr += optptr[1] + 2;
  }
  return type;
}

/*---------------------------------------------------------------------------*/
static u8_t
parse_msg(void)
{
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;

  if(m->op == DHCP_REPLY &&
     memcmp(m->xid, xid, sizeof(xid)) == 0 &&
     memcmp(m->chaddr, s.mac_addr, s.mac_len) == 0) {
    u8_t type = 0;
    memcpy(s.ipaddr, m->yiaddr, 4);
    type = parse_options(&m->options[4], uip_datalen());

    return type;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/

static int msg_for_me(void)
{
    struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;
    u8_t *optptr = &m->options[4];
    u8_t *end = (u8_t*)uip_appdata + uip_datalen();

    if (m->op == DHCP_REPLY &&
        memcmp(m->xid, &xid, sizeof(xid)) == 0 &&
        memcmp(m->chaddr, s.mac_addr, s.mac_len) == 0)
    {
        while(optptr < end)
        {
            if (*optptr == DHCP_OPTION_MSG_TYPE)
            {
                return *(optptr + 2);
            } else if (*optptr == DHCP_OPTION_END)
            {
                return -1;
            }
            optptr += optptr[1] + 2;
        }
    }
    return -1;
}


static
PT_THREAD(handle_dhcp(void))
{
    unsigned int ticks;
    int msg;

    PT_BEGIN(&s.pt);

    if (s.state == STATE_DISABLED)
        PT_RESTART(&s.pt);

    // Random startup delay as described in spec
  initwait:
    rand_startup = rand() % 8192; // 0 - 8 seconds
    s.ticks = rand_startup;
    timer_set(&s.timer, s.ticks);

    do
    {
        PT_YIELD(&s.pt);
    } while (!timer_expired(&s.timer));

  init:
    s.state = STATE_SENDING;
    s.ticks = CLOCK_SECOND;

    while (1)
    {
        send_discover();
        timer_set(&s.timer, s.ticks);

        do
        {
            PT_YIELD(&s.pt);
            if (uip_newdata())
            {
              msg = msg_for_me();
              if (msg == DHCPOFFER)
              {
                parse_msg();
                s.state = STATE_OFFER_RECEIVED;
                goto selecting;
              }
            }

        } while (!timer_expired(&s.timer));

#if UIP_USE_AUTOIP
        if (s.ticks == CLOCK_SECOND * 4)
        {
            autoip_start();
        }
#endif

        if (s.ticks < CLOCK_SECOND * 60)
        {
            s.ticks *= 2;
        }
    }

  selecting:
    s.ticks = CLOCK_SECOND;
    do
    {
        send_request();
        timer_set(&s.timer, s.ticks);

        do
        {
            PT_YIELD(&s.pt);
            if (uip_newdata())
            {
              msg = msg_for_me();
              if (msg == DHCPACK)
              {
                parse_msg();
                s.state = STATE_CONFIG_RECEIVED;
                goto bound;
              }
              else if (msg == DHCPNAK)
              {
                goto initwait;
              }
            }
        } while (!timer_expired(&s.timer));

        if (s.ticks <= CLOCK_SECOND * 10)
        {
            s.ticks += CLOCK_SECOND;
        }
        else goto init;

    } while (s.state != STATE_CONFIG_RECEIVED);

  bound:
    dhcpc_configured(&s);

    #define MAX_TICKS ((unsigned int)(~(0))/2)
    #define MAX_TICKS32 (~((unsigned int)0))
    #define IMIN(a, b) ((a) < (b) ? (a) : (b))

    if((s.lease_time[0]*65536ul + s.lease_time[1])*CLOCK_SECOND/2 <= MAX_TICKS32)
    {
        s.ticks = (s.lease_time[0]*65536ul +s.lease_time[1])*CLOCK_SECOND/2;
    }
    else
    {
        s.ticks = MAX_TICKS32;
    }

    while(s.ticks > 0)
    {
        ticks = IMIN(s.ticks, MAX_TICKS);
        s.ticks -= ticks;
        timer_set(&s.timer, ticks);
        PT_YIELD_UNTIL(&s.pt, timer_expired(&s.timer));
    }

    if((s.lease_time[0]*65536ul + s.lease_time[1])*CLOCK_SECOND/2 <= MAX_TICKS32)
    {
        s.ticks = (s.lease_time[0]*65536ul +s.lease_time[1])*CLOCK_SECOND/2;
    }
    else
    {
        s.ticks = MAX_TICKS32;
    }

    // Renew
    do
    {
        send_request();
        ticks = IMIN(s.ticks / 2, MAX_TICKS);
        s.ticks -= ticks;
        timer_set(&s.timer, ticks);

        do
        {
            PT_YIELD(&s.pt);

            if (uip_newdata())
            {
              msg = msg_for_me();
              if (msg == DHCPACK)
              {
                parse_msg();
                goto bound;
              }
              else if (msg == DHCPNAK)
              {
                goto init;
              }
            }
        } while (!timer_expired(&s.timer));

    } while (s.ticks >= CLOCK_SECOND*3);

    // Lease expired
    goto init;

    PT_END(&s.pt);

}
/*---------------------------------------------------------------------------*/
void
dhcpc_init(const void *mac_addr, int mac_len)
{
  uip_ipaddr_t addr;

  s.mac_addr = mac_addr;
  s.mac_len  = mac_len;

  s.state = STATE_DISABLED;

  xid[0] = ((unsigned char *)mac_addr)[2];
  xid[1] = ((unsigned char *)mac_addr)[3];
  xid[2] = ((unsigned char *)mac_addr)[4];
  xid[3] = ((unsigned char *)mac_addr)[5];

  memcpy(&rand_seed, xid, (size_t)4);

  srand(rand_seed);
  rand();

  uip_ipaddr(addr, 255,255,255,255);
  s.conn = uip_udp_new(&addr, HTONS(DHCPC_SERVER_PORT));

  if(s.conn != NULL) {
    uip_udp_bind(s.conn, HTONS(DHCPC_CLIENT_PORT));
  }
  PT_INIT(&s.pt);
}

void dhcpc_start()
{
  s.state = STATE_INITIAL;
  uip_udp_listen(HTONS(DHCPC_CLIENT_PORT));
  PT_INIT(&s.pt);
}

void dhcpc_stop()
{
  s.state = STATE_DISABLED;
  uip_udp_unlisten(HTONS(DHCPC_CLIENT_PORT));
  PT_INIT(&s.pt);
}

/*---------------------------------------------------------------------------*/
void
dhcpc_appcall(void)
{
  handle_dhcp();
}
/*---------------------------------------------------------------------------*/
void
dhcpc_request(void)
{
  u16_t ipaddr[2];

  if(s.state == STATE_INITIAL)
  {
    uip_ipaddr(ipaddr, 0,0,0,0);
    uip_sethostaddr(ipaddr);
  }
}
/*---------------------------------------------------------------------------*/

#endif

