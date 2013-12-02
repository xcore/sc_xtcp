// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include "uip.h"
#include "autoip.h"
#include "uip_arp.h"
#include "timer.h"
#include "clock-arch.h"
#include <print.h>
#include <string.h>

struct arp_hdr {
  struct uip_eth_hdr ethhdr;
  u16_t hwtype;
  u16_t protocol;
  u8_t hwlen;
  u8_t protolen;
  u16_t opcode;
  struct uip_eth_addr shwaddr;
  u16_t sipaddr[2];
  struct uip_eth_addr dhwaddr;
  u16_t dipaddr[2];
};

#define ARP_REQUEST 1
#define ARP_REPLY   2

#define ARP_HWTYPE_ETH 1

static struct arp_hdr autoip_arp_pkt = {
		{ { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } },
		  { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
		    HTONS(UIP_ETHTYPE_ARP) },
		HTONS(ARP_HWTYPE_ETH),
		HTONS(UIP_ETHTYPE_IP),
		6,
		4,
		HTONS(ARP_REQUEST),
		{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
		{ 0x0000, 0x0000 },
		{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
		{ 0x0000, 0x0000 }
};

// Values taken from RFC 3927

#define PROBE_WAIT           1  // second   (initial random delay)
#define PROBE_NUM            3  //          (number of probe packets)
#define PROBE_MIN            1  // second   (minimum delay till repeated probe)
#define PROBE_MAX            2  // seconds  (maximum delay till repeated probe)
#define ANNOUNCE_WAIT        2  // seconds  (delay before announcing)
#define ANNOUNCE_NUM         2  //          (number of announcement packets)
#define ANNOUNCE_INTERVAL    2  // seconds  (time between announcement packets)
#define MAX_CONFLICTS       10  //          (max conflicts before rate limiting)
#define RATE_LIMIT_INTERVAL 60  // seconds  (delay between successive attempts)
#define DEFEND_INTERVAL     10  // seconds  (minimum interval between defensive
                                //           ARPs).
enum autoip_machine_state {
  DISABLED,
  NO_ADDRESS,
  WAIT_FOR_PROBE,
  PROBING,
  WAIT_FOR_ANNOUNCE,
  ANNOUNCING,
  CONFIGURED
};

struct autoip_state_t {
  enum autoip_machine_state state;
  int probes_sent;
  int announces_sent;
  int num_conflicts;
  int limit_rate;
  unsigned int rand;
  unsigned int seed;
  struct uip_timer timer;
  uip_ipaddr_t ipaddr;
};

#define BUF   ((struct arp_hdr *)&uip_buf[0])

#if UIP_USE_AUTOIP

static const unsigned int a=1664525;
static const unsigned int c=1013904223;

static struct autoip_state_t my_autoip_state;
static struct autoip_state_t *autoip_state = &my_autoip_state;

__attribute__ ((noinline))
static void rand(struct autoip_state_t* s)
{
	s->rand = a*s->rand+c;
}

__attribute__ ((noinline))
void autoip_init(int seed)
{
  autoip_state->state = DISABLED;
  autoip_state->probes_sent = 0;
  autoip_state->announces_sent = 0;
  autoip_state->num_conflicts = 0;
  autoip_state->limit_rate = 0;
  autoip_state->seed = seed;
  autoip_state->rand = seed;
}

static void random_timer_set(struct uip_timer *t,
                      int a,
                      int b)
{
  long long x;
  rand(autoip_state);
  x = autoip_state->rand * (b-a);
  x = x >> 32;
  timer_set(t, a + x);
}

__attribute__ ((noinline))
static void create_arp_packet()
{
	memcpy(BUF, &autoip_arp_pkt, sizeof(autoip_arp_pkt));
	memcpy(BUF->ethhdr.src.addr, uip_ethaddr.addr, 6);
	memcpy(BUF->shwaddr.addr, uip_ethaddr.addr, 6);

	uip_appdata = &uip_buf[UIP_TCPIP_HLEN + UIP_LLH_LEN];
	uip_len = sizeof(struct arp_hdr);
}

static void send_probe()
{
  create_arp_packet();
  uip_ipaddr_copy(BUF->dipaddr, autoip_state->ipaddr);

  autoip_state->probes_sent++;
  random_timer_set(&autoip_state->timer,
                   PROBE_MIN * CLOCK_SECOND,
                   PROBE_MAX * CLOCK_SECOND);
}

static void send_announce()
{
  create_arp_packet();
  uip_ipaddr_copy(BUF->dipaddr, autoip_state->ipaddr);
  uip_ipaddr_copy(BUF->sipaddr, autoip_state->ipaddr);

  autoip_state->announces_sent++;
  timer_set(&autoip_state->timer, ANNOUNCE_INTERVAL * CLOCK_SECOND);
}

void autoip_periodic()
{
  switch (autoip_state->state)
    {
    case DISABLED:
      break;
    case NO_ADDRESS:
      {
        int r1,r2;
        if (!autoip_state->limit_rate || timer_expired(&autoip_state->timer)) {
          rand(autoip_state);
          r1 = autoip_state->rand & 0xff;
          r2 = (autoip_state->rand & 0xff00) >> 8;
          uip_ipaddr(&(autoip_state->ipaddr),169,254,r1,r2);
          autoip_state->state = WAIT_FOR_PROBE;
          random_timer_set(&autoip_state->timer, 0, PROBE_WAIT * CLOCK_SECOND);
        }
        break;
      }
    case WAIT_FOR_PROBE:
      if (timer_expired(&autoip_state->timer)) {
        autoip_state->state = PROBING;
        send_probe();
      }
      break;
    case PROBING:
      if (timer_expired(&autoip_state->timer))
        {
          if (autoip_state->probes_sent == PROBE_NUM) {
            // configured
            autoip_state->state = WAIT_FOR_ANNOUNCE;
            timer_set(&autoip_state->timer, ANNOUNCE_WAIT * CLOCK_SECOND);
          }
          else
            send_probe();
        }
      break;
    case WAIT_FOR_ANNOUNCE:
      if (timer_expired(&autoip_state->timer)) {
        if (autoip_state->num_conflicts == 0) {
          autoip_state->state = ANNOUNCING;
          send_announce();
        }
        else {
          autoip_state->state = NO_ADDRESS;
          autoip_state->probes_sent = 0;
          autoip_state->announces_sent = 0;
          autoip_state->num_conflicts = 0;
          autoip_state->limit_rate =
            autoip_state->limit_rate ||
            (autoip_state->num_conflicts > MAX_CONFLICTS);
          timer_set(&autoip_state->timer, RATE_LIMIT_INTERVAL * CLOCK_SECOND);
        }
      }
      break;
    case ANNOUNCING:
      send_announce();
      if (autoip_state->announces_sent == ANNOUNCE_NUM) {
        autoip_state->state = CONFIGURED;
        autoip_configured(autoip_state->ipaddr);
      }
      break;
    case CONFIGURED:
      break;
    }
  return;
}

void autoip_arp_in()
{
  switch (autoip_state->state)
    {
      case WAIT_FOR_PROBE:
      case PROBING:
      case WAIT_FOR_ANNOUNCE:
        if (uip_ipaddr_cmp(BUF->sipaddr, autoip_state->ipaddr)) {
          autoip_state->num_conflicts++;
        }
        break;
    default:
      break;
    }
  return;
}

void autoip_start()
{
  if (autoip_state->state == DISABLED) {
	autoip_init(autoip_state->seed);
    rand(autoip_state);
    autoip_state->state = NO_ADDRESS;
  }
}

void autoip_stop()
{
  autoip_state->state = DISABLED;
}

#endif

