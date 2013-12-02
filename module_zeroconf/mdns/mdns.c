// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>


#include "xtcp_client.h"
#include "mdns.h"
#include <print.h>
#include <string.h>

typedef enum mdns_entry_type_t {
  MDNS_CANONICAL_NAME_ENTRY,
  MDNS_NAME_ENTRY,
  MDNS_SRV_ENTRY
} mdns_entry_type_t;



#define timeafter(A, B) ((int)((B) - (A)) < 0)

typedef unsigned int u32_t;
typedef unsigned short u16_t;
typedef unsigned char u8_t;

#define HTONS(x) (__builtin_bswap32(x) >> 16)
#define NTOHS(x) (__builtin_bswap32(x) >> 16)
#define HTONL(x) __builtin_bswap32(x)

#include "mdns_protocol.h"
#if MDNS_NETBIOS
#include "netbios_protocol.h"
#endif

typedef enum mdns_state_t {
  UNUSED,
  DISABLED,
  PROBE_WAIT,
  PROBE_PARTIAL_SENT,
  PROBE_SENT,
  ANNOUNCE_WAIT,
  ANNOUNCE_SENT,
  ACTIVE
} mdns_state_t;

#define PENDING_HOST_RESPONSE (1 << 0)
#define PENDING_PTR_RESPONSE (1 << 1)
#if MDNS_NETBIOS
#define PENDING_NETBIOS_RESPONSE (1 << 2)
#endif
#define PENDING_SRV_RESPONSE (1 << 3)
#define PENDING_TXT_RESPONSE (1 << 4)

struct mdns_table_entry {
  mdns_state_t state;
  int pending;
  char name[MDNS_MAX_NAME_LENGTH];
  char name_prefix[MDNS_MAX_NAME_LENGTH];
  char name_postfix[MDNS_MAX_NAME_LENGTH];
  int srv_port;
  xtcp_ipaddr_t ipaddr;
  int counter;
  unsigned int timeout;
  int num_sent;
  mdns_entry_type_t entry_type;
#if MDNS_NETBIOS
  u16_t netbios_trans_id;
  u8_t  netbios_nametype;
  u16_t netbios_type;
  u16_t netbios_class;
  char netbios_enc_name[ENCODED_NETBIOS_NAME_LEN];
#endif
};
//__attribute__((packed));

static struct mdns_table_entry mdns_table[MDNS_NUM_TABLE_ENTRIES];


static char * mdns_itoa(char *str, int x)
{
  char *str0 = str;
  char *end;
  do {
    int digit = x % 10;
    *str = digit + '0';
    str++;
    x = x / 10;
  } while (x != 0);

  end = str;
  str--;
  for (;str0 < str;str0++,str--) {
    char tmp;
    tmp = *str;
    *str = *str0;
    *str0 = tmp;
  }

  return end;
}


#if MDNS_NETBIOS
static void convert_to_lower_case(char *str, int n)
{
  int i=0;
  while (*str != '\0' && i < n) {
    if (*str >= 'A' && *str <= 'Z')
      *str = *str + 0x20;
    str++;
    i++;
  }
  return;
}
#endif

static char *
mdns_parse_name(char *query, char *limit,
                char *msg)
{
  char n;
  char *saved_endptr = NULL;
  if (query > limit)
    return NULL;
  do {
    n = *query++;
    if (query > limit)
      return NULL;
    /** @see RFC 1035 - 4.1.4. Message compression */
    if ((n & 0xc0) == 0xc0) {
      /* Compressed name */
      int offset = ((n & 0x3f) << 8) + (*query++);
      if (!saved_endptr)
        saved_endptr = query;
      query = msg + offset;
      if (query > limit)
        return NULL;

    } else {
      /* Not compressed name */
      while (n > 0) {
        ++query;
        --n;
        if (query > limit)
          return NULL;
      };
    }
  } while (*query != 0);

  if (saved_endptr)
    query = saved_endptr;

  return query + 1;
}


static u8_t
mdns_compare_name(char *query, char *response,
                  char *msg)
{
  char n;

  do {
    n = *response++;
    /** @see RFC 1035 - 4.1.4. Message compression */
    if ((n & 0xc0) == 0xc0) {
      /* Compressed name */
      int offset = ((n & 0x3f) << 8) + (*response++);
      response = msg + offset;
      break;
    } else {
      /* Not compressed name */
      while (n > 0 && *query != 0) {
        if ((unsigned char) (*query) > (unsigned char) (*response)) {
          return 1;
        }
        else if ((unsigned char) (*query) < (unsigned char) (*response)) {
          return -1;
        }
        ++response;
        if (*query)
          ++query;
        --n;
      };
      if (*query)
        ++query;
    }
  } while (*response != 0 && *query != 0);

  if (*query)
    return 1;

  if (*response)
    return -1;

  return 0;
}







static int
mdns_parse_rev_ip(char *query, xtcp_ipaddr_t addr,
                  char *msg)
{
  char n;
  int res = 1;
  int curField = 0;

  do {
    n = *query++;
    /** @see RFC 1035 - 4.1.4. Message compression */
    if ((n & 0xc0) == 0xc0) {
      /* Compressed name */
      int offset = ((n & 0x3f) << 8) + (*query++);
      query = msg + offset;
      break;
    } else {
      int i = 0;
      int sum = 0;
      int base = 1;
      query += n;

      if (curField < 4 && res) {
        if (n <= 3) {
          for (i=0;i<n;i++) {
            query--;
            if (*query < 0x30 || *query > 0x39)
              res = 0;
            else
              sum += (*query-0x30)*base;
            base = base*10;
          }
          if (sum > 255)
            res = 0;
          else {
            addr[3-curField] = sum;
          }
          query += n;
        }
        else
          res = 0;
        curField++;
      }
    }
  } while (*query != 0 && curField < 4);


  res &= (mdns_compare_name("in-addr.arpa", query, msg) == 0);

  return res;
}


static void handle_mdns_ptr_query(chanend tcp_svr,
                                   xtcp_connection_t *conn,
                                  char *mdns_payload,
                                  char *msg)
{
  int i;
  char *query_name = (mdns_payload);
  xtcp_ipaddr_t addr;
  for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++) {
    if (mdns_table[i].state == ACTIVE &&
        (mdns_table[i].entry_type == MDNS_NAME_ENTRY ||
         mdns_table[i].entry_type == MDNS_CANONICAL_NAME_ENTRY) &&
        mdns_parse_rev_ip(query_name, addr, msg) &&
        XTCP_IPADDR_CMP(addr, mdns_table[i].ipaddr))
      {
        mdns_table[i].pending |= PENDING_PTR_RESPONSE;
        xtcp_init_send(tcp_svr, conn);
        break;
      }
    if (mdns_table[i].state == ACTIVE &&
        (mdns_table[i].entry_type == MDNS_SRV_ENTRY) &&
        mdns_compare_name(mdns_table[i].name_postfix, query_name, msg)==0)
      {
        mdns_table[i].pending |= PENDING_PTR_RESPONSE;
        xtcp_init_send(tcp_svr, conn);
        break;
      }



  }
  return;
}

static void update_name(struct mdns_table_entry *e)
{
  char *name = (char *) &e->name[0];
  strcpy(name, e->name_prefix);
  name = name + strlen(e->name_prefix);
  if (e->counter != 0) {
    if (e->entry_type == MDNS_SRV_ENTRY) {
      *name++ = ' ';
      *name++ = '(';
      name = mdns_itoa(name, e->counter);
      *name++ = ')';
    }
    else {
      *name++ = '-';
      name = mdns_itoa(name, e->counter);
    }
  }
  *name++ = '.';
  strcpy(name, e->name_postfix);
  return;
}

static int mdns_compare_host_answer(struct mdns_answer *ans,
                                    struct mdns_table_entry *e)
{
  unsigned int ans_class = NTOHS(ans->class) & ~MDNS_RRCLASS_FLUSH;
  unsigned int ans_type = NTOHS(ans->type);
  u8_t *rdata = (u8_t *) (ans+1);
  int i;
  int len = NTOHS(ans->len);
  if (ans_class > MDNS_RRCLASS_IN)
    return 1;
  else if (ans_class < MDNS_RRCLASS_IN)
    return -1;

  if (ans_type > MDNS_RRTYPE_A)
    return 1;
  else if (ans_type < MDNS_RRTYPE_A)
    return -1;

  for(i=0;i<len && i<4;i++) {
    if (rdata[i] > e->ipaddr[i])
      return 1;
    else
      if (rdata[i] < e->ipaddr[i])
        return -1;
  }

  if (len > 4)
    return 1;
  else if (len < 4)
    return -1;

  return 0;
}

static int mdns_compare_answer(struct mdns_answer *ans,
                               unsigned int rrtype,
                               const char *txt)
{
  unsigned int ans_class = NTOHS(ans->class) & ~MDNS_RRCLASS_FLUSH;
  unsigned int ans_type = NTOHS(ans->type);
  u8_t *rdata = (u8_t *) (ans+1);
  int i;
  int len = NTOHS(ans->len);
  int txtlen = strlen(txt);
  if (ans_class > MDNS_RRCLASS_IN)
    return 1;
  else if (ans_class < MDNS_RRCLASS_IN)
    return -1;

  if (ans_type > rrtype)
    return 1;
  else if (ans_type < rrtype)
    return -1;


  for(i=0;i<len && i<txtlen;i++) {
    if (rdata[i] > txt[i])
      return 1;
    else
      if (rdata[i] < txt[i])
        return -1;
  }


  if (len > txtlen)
    return 1;
  else if (len < txtlen)
    return -1;

  return 0;

}

static int mdns_compare_srv_answer(struct mdns_answer *ans,
                                   struct mdns_table_entry *e)
{
  return mdns_compare_answer(ans, MDNS_RRTYPE_SRV, e->name);
}

static int mdns_compare_txt_answer(struct mdns_answer *ans,
                                   struct mdns_table_entry *e)
{
  return mdns_compare_answer(ans, MDNS_RRTYPE_TXT, "");
}

static int mdns_answer_conflict(struct mdns_answer *ans,
                                struct mdns_table_entry *e)
{
  switch (e->entry_type)
    {
    case MDNS_CANONICAL_NAME_ENTRY:
    case MDNS_NAME_ENTRY:
      return (mdns_compare_host_answer(ans, e) != 0);
      break;
    case MDNS_SRV_ENTRY:
      if (ans->type == HTONS(MDNS_RRTYPE_TXT))
        return (mdns_compare_txt_answer(ans, e) != 0);
      if (ans->type == HTONS(MDNS_RRTYPE_SRV))
        return (mdns_compare_srv_answer(ans, e) != 0);

      return 0;
      break;
    }
  return 0;
}

static mdns_event handle_mdns_response(chanend tcp_svr,
                                 xtcp_connection_t *conn,
                                 char *query_name,
                                 struct mdns_answer *ans,
                                 char *msg)
{
  int i;
  mdns_event result = 0;

  // detect any conflict with out proposed unique records
  for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++) {

    if (mdns_table[i].state != DISABLED &&
        mdns_table[i].state != UNUSED &&
        mdns_compare_name(mdns_table[i].name, query_name, msg)==0)
      {
        int conflict = mdns_answer_conflict(ans, &mdns_table[i]);

        if (conflict) {
          // We have a conflict - revert to probing
          if (mdns_table[i].state != PROBE_WAIT ||
              mdns_table[i].state != ANNOUNCE_WAIT)
            xtcp_init_send(tcp_svr, conn);

          // If we already probing, change the name
          if (mdns_table[i].state == PROBE_WAIT ||
              mdns_table[i].state == PROBE_SENT) {
            mdns_table[i].counter++;
            update_name(&mdns_table[i]);
            result |= mdns_entry_lost;
          }
          mdns_table[i].state = PROBE_WAIT;
          mdns_table[i].num_sent = 0;
        }
        else {
          // Someone else has responded with our info
          // supress any pending responses unless the ttl is
          // wrong
          if (HTONL(ans->ttl) > 255/2) {
            mdns_table[i].pending = 0;
          }
        }
    }
  }

  return result;
}

static int answer_is_lower(struct mdns_answer *ans1,
                           struct mdns_answer *ans2)
{
  int ans1_len = NTOHS(ans1->len);
  int ans2_len = NTOHS(ans2->len);
  char *ans1_rdata = (char *) (ans1+1);
  char *ans2_rdata = (char *) (ans2+1);
  int i;

  if (NTOHS(ans1->class) < NTOHS(ans2->class))
    return 1;

  if (NTOHS(ans1->type) < NTOHS(ans2->type))
    return 1;

  for(i=0;i<ans1_len && i<ans2_len;i++)
    {
      if (ans1_rdata[i] < ans2_rdata[i])
        return 1;
    }

  return (ans1_len < ans2_len);
}

static struct mdns_answer *
find_lowest_matching_answer(struct mdns_table_entry *e,
                            char *dptr,
                            int num_rr,
                            struct mdns_answer *ignore,
                            char *eom,
                            char *msg)
{
  int i;
  struct mdns_answer *lowest = NULL;
  for(i=0;i<num_rr;i++) {
      struct mdns_answer *ans =
        (struct mdns_answer *) mdns_parse_name((char *)dptr, eom, msg);
      char *rdata = (char *) (ans+1);
      int len = NTOHS(ans->len);
      if (mdns_compare_name(e->name, dptr, msg)==0 &&
          ans != ignore)
        {
          if (lowest == NULL || answer_is_lower(ans, lowest))
            lowest = ans;
        }

      dptr = (rdata + len);
  }
  return lowest;
}

static int loses_tie_break(struct mdns_table_entry *e,
                           char *auth,
                           int n,
                           char *eom,
                           char *msg)
{
  switch (e->entry_type)
    {
    case MDNS_CANONICAL_NAME_ENTRY:
    case MDNS_NAME_ENTRY: {
      struct mdns_answer *ans =
        find_lowest_matching_answer(e, auth, n, NULL, eom, msg);
      if (ans == NULL)
        return 0;
      return (mdns_compare_host_answer(ans, e) > 0);
      break;
      }
    case MDNS_SRV_ENTRY: {
      struct mdns_answer *ans =
        find_lowest_matching_answer(e, auth, n, NULL, eom, msg);
      int res;
      if (ans == NULL)
        return 0;
      res = mdns_compare_txt_answer(ans, e);
      if (res < 0)
        return 0;
      if (res > 0)
        return 1;
      ans = find_lowest_matching_answer(e, auth, n, ans, eom, msg);
      return (mdns_compare_srv_answer(ans, e) > 0);
      break;
      }
      break;
    }
  return 0;
}

 static mdns_event handle_mdns_auth(chanend tcp_svr,
                              xtcp_connection_t *conn,
                              char *auth,
                              int n,
                              char *eom,
                              char *msg)
{
  mdns_event result = 0;
  int i;
  for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++) {
    if (mdns_table[i].state == PROBE_WAIT ||
        mdns_table[i].state == PROBE_SENT) {

      if (loses_tie_break(&mdns_table[i], auth, n, eom, msg))
        {
          // We have a conflict - revert to probing
          if (mdns_table[i].state != PROBE_WAIT)
            xtcp_init_send(tcp_svr, conn);

          // Change the name
          mdns_table[i].counter++;
          update_name(&mdns_table[i]);
          mdns_table[i].state = PROBE_WAIT;
          mdns_table[i].num_sent = 0;
          result |= mdns_entry_lost;
        }
    }
  }
  return result;
}

static mdns_event mdns_recv(chanend tcp_svr, xtcp_connection_t *conn)
{
  mdns_event result = 0;
  char data[XTCP_CLIENT_BUF_SIZE];
  int len;
  struct mdns_hdr *hdr = (struct mdns_hdr *) &data[0];
  char *dptr;
  char *eom;
  char *msg = &data[0];
  len = xtcp_recv(tcp_svr, data);

  dptr = &data[sizeof(struct mdns_hdr)];
  eom  = &data[len];


  //  if ((hdr->flags1 & MDNS_FLAG1_RESPONSE)==0)
{
    // we have a query...
    int nquestions = NTOHS(hdr->numquestions);
    int nanswers = NTOHS(hdr->numanswers);
    int nauth  = NTOHS(hdr->numauthrr);
    int i;


    for (i=0;i<nquestions;i++) {
      struct mdns_query* qry;
      qry =
        (struct mdns_query *) mdns_parse_name((char *)dptr, eom, msg);

      if (qry == NULL)
        break;


      if (NTOHS(qry->class) == MDNS_RRCLASS_IN) {
        if (NTOHS(qry->type) == MDNS_RRTYPE_PTR) {
          handle_mdns_ptr_query(tcp_svr, conn, dptr, msg);
        }
        else {
          for (int j=0;j<MDNS_NUM_TABLE_ENTRIES;j++) {
            if (mdns_table[j].state == ACTIVE &&
                mdns_compare_name(mdns_table[j].name, dptr, msg)==0)
              {
                int qtype = NTOHS(qry->type);
                switch (mdns_table[j].entry_type)
                  {
                  case MDNS_NAME_ENTRY:
                  case MDNS_CANONICAL_NAME_ENTRY:
                    if (qtype == MDNS_RRTYPE_A || qtype == MDNS_RRTYPE_ANY)
                      {
                        mdns_table[j].pending |= PENDING_HOST_RESPONSE;
                        xtcp_init_send(tcp_svr, conn);
                      }
                    break;
                  case MDNS_SRV_ENTRY:
                    if (qtype == MDNS_RRTYPE_SRV || qtype == MDNS_RRTYPE_ANY)
                      {
                        mdns_table[j].pending |= PENDING_SRV_RESPONSE;
                        xtcp_init_send(tcp_svr, conn);
                      }
                    if (qtype == MDNS_RRTYPE_TXT || qtype == MDNS_RRTYPE_ANY)
                      {
                        mdns_table[j].pending |= PENDING_TXT_RESPONSE;
                        xtcp_init_send(tcp_svr, conn);
                      }
                    break;
                }
                break;
              }
          }
        }
      }

      dptr = ((char *) qry) + sizeof(struct mdns_query);
    }


    for (i=0;i<nanswers;i++) {
      struct mdns_answer* ans;
      ans =
        (struct mdns_answer *) mdns_parse_name((char *)dptr, eom, msg);

      if (ans==NULL)
        break;

      result |= handle_mdns_response(tcp_svr, conn, dptr, ans, msg);

      dptr = ((char *) ans) + sizeof(struct mdns_answer) + (NTOHS(ans->len));
    }

    result |= handle_mdns_auth(tcp_svr, conn, dptr, nauth, eom, msg);

  }

  return result;
}

#if MDNS_NETBIOS

/** NetBIOS decoding name */
static int
decode_netbios_name(char *name_dec,  char *name_enc)
{
  char *pname;
  char  cname;
  char  cnbname;
  int   index = 0;

  /* Start decoding netbios name. */
  pname  = name_enc;
  for (;;) {
    /* Every two characters of the first level-encoded name
     * turn into one character in the decoded name. */
    cname = *pname;
    if (cname == '\0')
      break;    /* no more characters */
    if (cname == '.')
      break;    /* scope ID follows */
    if (cname < 'A' || cname > 'Z') {
      /* Not legal. */
      return -1;
    }
    cname -= 'A';
    cnbname = cname << 4;
    pname++;

    cname = *pname;
    if (cname == '\0' || cname == '.') {
      /* No more characters in the name - but we're in
       * the middle of a pair.  Not legal. */
      return -1;
    }
    if (cname < 'A' || cname > 'Z') {
      /* Not legal. */
      return -1;
    }
    cname -= 'A';
    cnbname |= cname;
    pname++;

    /* Do we have room to store the character? */
    if (index < NETBIOS_NAME_LEN) {
      /* Yes - store the character. */
      name_dec[index++] = (cnbname!=' '?cnbname:'\0');
    }
  }

  return 0;
}


#ifdef USE_NETBIOS_ENCODE
static int encode_netbios_name(char *name_enc, char *name_dec, int len)
{

  char         *pname;
  char          cname;
  unsigned char ucname;
  int           index = 0;

  /* Start encoding netbios name. */
  pname = name_dec;

  for (;;) {
    /* Every two characters of the first level-encoded name
     * turn into one character in the decoded name. */
    cname = *pname;
    if (cname == '\0')
      break;    /* no more characters */
    if (cname == '.')
      break;    /* scope ID follows */
    if ((cname < 'A' || cname > 'Z') && (cname < '0' || cname > '9')) {
      /* Not legal. */
      return -1;
    }

    /* Do we have room to store the character? */
    if (index >= len) {
      return -1;
    }

    /* Yes - store the character. */
    ucname = cname;
    name_enc[index++] = ('A'+((ucname>>4) & 0x0F));
    name_enc[index++] = ('A'+( ucname     & 0x0F));
    pname++;
  }

  /* Fill with "space" coding */
  for (;index<len-1;) {
    name_enc[index++] = 'C';
    name_enc[index++] = 'A';
  }

  /* Terminate string */
  name_enc[index]='\0';

  return 0;
}
#endif

static mdns_event netbios_recv(chanend tcp_svr, xtcp_connection_t *conn)
{
  char data[XTCP_CLIENT_BUF_SIZE];
  int len;
  char   decoded_name[NETBIOS_NAME_LEN+1];
  struct netbios_hdr *netbios_hdr = (struct netbios_hdr *) &data[0];
  struct netbios_name_hdr* netbios_name_hdr = (struct netbios_name_hdr*)(netbios_hdr+1);
  len = xtcp_recv(tcp_svr, data);
  u16_t flags;
  int i;
  flags = NTOHS(netbios_hdr->flags);

  if ((flags & NETB_HFLAG_OPCODE) == NETB_HFLAG_OPCODE_NAME_QUERY &&
      (flags & NETB_HFLAG_RESPONSE) == 0 &&
      (NTOHS(netbios_hdr->questions) == 1)) {
    int err;
    err = decode_netbios_name( decoded_name,
                               (char*)(netbios_name_hdr->encname));

    if (err)
      return mdns_name_error;

    convert_to_lower_case(decoded_name, NETBIOS_NAME_LEN+1);

    for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++) {
          if (mdns_table[i].state == ACTIVE &&
              (strcmp(decoded_name, mdns_table[i].name)==0))
            {
              mdns_table[i].pending |= PENDING_NETBIOS_RESPONSE;
              mdns_table[i].netbios_trans_id = netbios_hdr->trans_id;
              mdns_table[i].netbios_nametype = netbios_name_hdr->nametype;
              mdns_table[i].netbios_type     = netbios_name_hdr->type;
              mdns_table[i].netbios_class    = netbios_name_hdr->class;
              memcpy(&mdns_table[i].netbios_enc_name[0], netbios_name_hdr->encname, ENCODED_NETBIOS_NAME_LEN);
              xtcp_init_send(tcp_svr, conn);
              break;
            }
        }
  }
  return 0;
}

static void mdns_send_netbios_response(chanend tcp_svr,
                                      xtcp_connection_t *conn,
                                      struct mdns_table_entry *e)
{
  char data[XTCP_CLIENT_BUF_SIZE];
  struct netbios_resp *resp = (struct netbios_resp*)&data[0];
  resp->resp_hdr.trans_id      = e->netbios_trans_id;
  resp->resp_hdr.flags         = HTONS(NETB_HFLAG_RESPONSE |
                                       NETB_HFLAG_OPCODE_NAME_QUERY |
                                       NETB_HFLAG_AUTHORATIVE |
                                       NETB_HFLAG_RECURS_DESIRED);
  resp->resp_hdr.questions     = 0;
  resp->resp_hdr.answerRRs     = HTONS(1);
  resp->resp_hdr.authorityRRs  = 0;
  resp->resp_hdr.additionalRRs = 0;

  /* prepare NetBIOS header datas */

  memcpy(&(resp->resp_name.encname),
         &(e->netbios_enc_name),
         ENCODED_NETBIOS_NAME_LEN);
  resp->resp_name.nametype     = e->netbios_nametype;
  resp->resp_name.type         = e->netbios_type;
  resp->resp_name.class        = e->netbios_class;
  resp->resp_name.ttl          = HTONL(NETBIOS_NAME_TTL);
  resp->resp_name.datalen      = HTONS(sizeof(resp->resp_name.flags)+sizeof(resp->resp_name.addr));
  resp->resp_name.flags        = HTONS(NETB_NFLAG_NODETYPE_BNODE);
  resp->resp_name.addr[0]      = e->ipaddr[0];
  resp->resp_name.addr[1]      = e->ipaddr[1];
  resp->resp_name.addr[2]      = e->ipaddr[2];
  resp->resp_name.addr[3]      = e->ipaddr[3];
  xtcp_send(tcp_svr, data, sizeof(struct netbios_resp));
  return;
}

#endif // MDNS_NETBIOS

static int mdns_started = 0;

void mdns_init(chanend tcp_svr)
{
  int i;
  xtcp_ipaddr_t mdns_addr = {224,0,0,251};
  xtcp_connect(tcp_svr, MDNS_SERVER_PORT, mdns_addr, XTCP_PROTOCOL_UDP);

  for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++) {
    mdns_table[i].state = UNUSED;
  }
}

static void mdns_start_entries(chanend tcp_svr, xtcp_connection_t *conn)
{
  int i;
  for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++)
    if (mdns_table[i].state != UNUSED) {
      mdns_table[i].state = PROBE_WAIT;
      mdns_table[i].num_sent = 0;
      xtcp_init_send(tcp_svr, conn);
    }

  mdns_started = 2;
}

void mdns_start(chanend tcp_svr, xtcp_ipaddr_t ipaddr)
{
  int i,j;
  xtcp_ipaddr_t multicast_group = {224,0,0,251};
  xtcp_join_multicast_group(tcp_svr, multicast_group);

  for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++)
    {
      for (j=0;j<4;j++)
        mdns_table[i].ipaddr[j] = ipaddr[j];
    }

  mdns_started = 1;
  return;
}

static int mdns_encode_name(char *dest,
                             char *src)
{
  char * nptr;
  char * dest0 = dest;
  int n=0;
  do {
    nptr = dest;
    dest++;
    n = 0;
    while (*src != '.' && *src != 0) {
      *dest = *src;
      dest++;
      src++;
      n++;
    }
    if (*src != 0)
      src++;
    *nptr = n;
  }
  while (*src != 0);
  *dest = 0;
  dest++;

  return (dest - dest0);
}


static char *mdns_send_host_response(char *dptr,
                                              struct mdns_table_entry *e)
{
  int nnamelen;
  struct mdns_answer *ans;
  int len;
  u8_t *ipaddr;

  nnamelen = mdns_encode_name(dptr, e->name);

  ans = (struct mdns_answer *) (dptr + nnamelen);
  ans->type = HTONS(MDNS_RRTYPE_A);
  ans->class = HTONS(MDNS_RRCLASS_IN | MDNS_RRCLASS_FLUSH);
  ans->len = HTONS(4);
  ans->ttl = HTONL(255);
  ipaddr = (u8_t *) (dptr + nnamelen + sizeof(struct mdns_answer));
  ipaddr[0] = e->ipaddr[0];
  ipaddr[1] = e->ipaddr[1];
  ipaddr[2] = e->ipaddr[2];
  ipaddr[3] = e->ipaddr[3];

  len = nnamelen+sizeof(struct mdns_answer) + 4;
  return (dptr+len);
}



static int mdns_encode_ip(char *str, xtcp_ipaddr_t addr)
{
  int i;
  char *str0 = str;
  for (i=3;i>=0;i--) {
    char * str0 = str;
    str++;
    str = mdns_itoa(str, addr[i]);
    *str0 = (str-str0-1);
  }
  *str++ = 7;
  strcpy(str, "in-addr");
  str += 7;
  *str++ = 4;
  strcpy(str, "arpa");
  str += 4;
  *str++ = 0;
  return (str-str0);
}

static char *mdns_get_canonical_name() {
  int i;
  for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++) {
    if (mdns_table[i].state != PROBE_WAIT &&
        mdns_table[i].entry_type == MDNS_CANONICAL_NAME_ENTRY) {
      return ((char *) &mdns_table[i].name);
    }
  }
  return NULL;
}

static char *mdns_send_ptr_response(char *dptr,
                                             struct mdns_table_entry *e)
{
  int nnamelen;
  struct mdns_answer *ans;
  int len;
  int ans_name_len = 0;
  char *ans_name;

  if (e->entry_type == MDNS_SRV_ENTRY)
    nnamelen = mdns_encode_name(dptr, e->name_postfix);
  else
    nnamelen = mdns_encode_ip(dptr, e->ipaddr);
  ans = (struct mdns_answer *) (dptr + nnamelen);
  ans->type = HTONS(MDNS_RRTYPE_PTR);
  ans->class = HTONS(MDNS_RRCLASS_IN);
  ans->ttl = HTONL(255);
  ans_name = (char *) (dptr + nnamelen+sizeof(struct mdns_answer));
  ans_name_len = mdns_encode_name(ans_name, e->name);
  ans->len = HTONS(ans_name_len);

  len = nnamelen+sizeof(struct mdns_answer);
  len += ans_name_len;
  return (dptr+len);
}


static char *mdns_send_srv_response(char *dptr,
                                             struct mdns_table_entry *e)
{
  int nnamelen;
  struct mdns_answer *ans;
  struct mdns_srv_hdr *srv;
  int len;
  int target_name_len = 0;
  char *target_name;

  nnamelen = mdns_encode_name(dptr, e->name);
  ans = (struct mdns_answer *) (dptr + nnamelen);
  ans->type = HTONS(MDNS_RRTYPE_SRV);
  ans->class = HTONS(MDNS_RRCLASS_IN);
  ans->ttl = HTONL(255);
  srv = (struct mdns_srv_hdr *) (dptr + nnamelen + sizeof(struct mdns_answer));
  srv->priority = 0;
  srv->weight = 0;
  srv->port = HTONS(e->srv_port);
  target_name = (char *) (dptr + nnamelen + sizeof(struct mdns_answer) + sizeof(struct mdns_srv_hdr));
  target_name_len = mdns_encode_name(target_name,
                                     mdns_get_canonical_name());

  ans->len = HTONS(sizeof(struct mdns_srv_hdr) + target_name_len);

  len = nnamelen+sizeof(struct mdns_answer);
  len += sizeof(struct mdns_srv_hdr) + target_name_len;
  return (dptr + len);
}



static char *mdns_send_txt_response(char *dptr,
                                             struct mdns_table_entry *e)
{
  int nnamelen;
  struct mdns_answer *ans;
  int len;
  nnamelen = mdns_encode_name(dptr, e->name);
  ans = (struct mdns_answer *) (dptr + nnamelen);
  ans->type = HTONS(MDNS_RRTYPE_TXT);
  ans->class = HTONS(MDNS_RRCLASS_IN);
  ans->ttl = HTONL(255);
  ans->len = HTONS(0);

  len = nnamelen+sizeof(struct mdns_answer);
  return (dptr + len);
}


static char *mdns_send_probe(char *dptr,
                                      struct mdns_table_entry *e)
{
  int nnamelen;
  struct mdns_query *qry;
  int len;
  nnamelen = mdns_encode_name(dptr, e->name);
  qry = (struct mdns_query *) (dptr+nnamelen);
  qry->type = HTONS(255);
  qry->class = HTONS(MDNS_RRCLASS_IN);
  len = nnamelen + sizeof(struct mdns_query);
  return (dptr + len);
}

static int probe_size(struct mdns_table_entry *e)
{
  return (sizeof(struct mdns_query) + strlen(e->name));
}

#define MAX_QUESTION_LENGTH (sizeof(struct mdns_query) + MDNS_MAX_NAME_LENGTH)

static int mdns_send_probes(chanend tcp_svr, xtcp_connection_t *conn, unsigned int t)
{
  int i;
  char data[XTCP_CLIENT_BUF_SIZE];
  struct mdns_hdr *hdr = (struct mdns_hdr *) &data[0];
  char *dptr = &data[sizeof(struct mdns_hdr)];
  int num_probes = 0;
  int num_auth_records = 0;

  for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++) {
    if (mdns_table[i].state == PROBE_WAIT) {
      int space_left = XTCP_CLIENT_BUF_SIZE - (dptr - &data[0]);
      if (probe_size(&mdns_table[i]) < space_left) {
        dptr = mdns_send_probe(dptr, &mdns_table[i]);
        mdns_table[i].state = PROBE_PARTIAL_SENT;
        num_probes++;
      }
    }
  }


  for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++) {
    if (mdns_table[i].state == PROBE_PARTIAL_SENT) {
        if (mdns_table[i].entry_type == MDNS_SRV_ENTRY) {
          dptr = mdns_send_ptr_response(dptr, &mdns_table[i]);
          dptr = mdns_send_srv_response(dptr, &mdns_table[i]);
          dptr = mdns_send_txt_response(dptr, &mdns_table[i]);
          num_auth_records+=3;
        }
        else {
          dptr = mdns_send_host_response(dptr, &mdns_table[i]);
          num_auth_records++;
        }
        mdns_table[i].num_sent++;
        mdns_table[i].state = PROBE_SENT;
        if (mdns_table[i].counter < 15)
          mdns_table[i].timeout = t + 250 * 1000 * 100;
        else
          mdns_table[i].timeout = t + 2500 * 1000 * 100;
    }
  }



  hdr->id = 0;
  hdr->flags1 = 0;
  hdr->flags2 = 0;
  hdr->numquestions = HTONS(num_probes);
  hdr->numanswers = HTONS(0);
  hdr->numauthrr = HTONS(num_auth_records);
  hdr->numextrarr = HTONS(0);

  if (num_probes > 0) {
    xtcp_send(tcp_svr, data, dptr-&data[0]);
  }

  return (num_probes > 0);
}

static int mdns_response_size(struct mdns_table_entry *e)
{
  return (sizeof(struct mdns_answer) + 2*strlen(e->name) + 4);
}

static int mdns_send_responses(chanend tcp_svr,
                               xtcp_connection_t *conn,
                               unsigned int t)
{
  int i;
  char data[XTCP_CLIENT_BUF_SIZE];
  struct mdns_hdr *hdr = (struct mdns_hdr *) &data[0];
  char *dptr = &data[sizeof(struct mdns_hdr)];
  int num_answers = 0;

  for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++) {
    int space_left = XTCP_CLIENT_BUF_SIZE - (dptr - &data[0]);
    if (mdns_table[i].state != UNUSED &&
        mdns_response_size(&mdns_table[i]) < space_left) {

      if (mdns_table[i].state == ANNOUNCE_WAIT) {
        if (mdns_table[i].entry_type == MDNS_SRV_ENTRY) {
          dptr = mdns_send_ptr_response(dptr, &mdns_table[i]);
          dptr = mdns_send_srv_response(dptr, &mdns_table[i]);
          dptr = mdns_send_txt_response(dptr, &mdns_table[i]);
          num_answers+=3;
        }
        else {
          dptr = mdns_send_host_response(dptr, &mdns_table[i]);
          num_answers++;
        }
        mdns_table[i].num_sent++;
        mdns_table[i].state = ANNOUNCE_SENT;
        mdns_table[i].timeout = t + 1000 * 1000 * 100;

      }

      if (mdns_table[i].pending & PENDING_HOST_RESPONSE) {
        dptr = mdns_send_host_response(dptr, &mdns_table[i]);
        mdns_table[i].pending ^= PENDING_HOST_RESPONSE;
        num_answers++;
      }

      if (mdns_table[i].pending & PENDING_PTR_RESPONSE) {
        dptr = mdns_send_ptr_response(dptr, &mdns_table[i]);
        mdns_table[i].pending ^= PENDING_PTR_RESPONSE;
        num_answers++;
      }

      if (mdns_table[i].pending & PENDING_SRV_RESPONSE) {
        dptr = mdns_send_srv_response(dptr, &mdns_table[i]);
        mdns_table[i].pending ^= PENDING_SRV_RESPONSE;
        num_answers++;
      }

      if (mdns_table[i].pending & PENDING_TXT_RESPONSE) {
        dptr = mdns_send_txt_response(dptr, &mdns_table[i]);
        mdns_table[i].pending ^= PENDING_TXT_RESPONSE;
        num_answers++;
      }
    }
  }

  hdr->id = 0;
  hdr->flags1 = MDNS_FLAG1_RESPONSE | MDNS_FLAG1_AUTHORATIVE ;
  hdr->flags2 = 0;
  hdr->numquestions = HTONS(0);
  hdr->numanswers = HTONS(num_answers);
  hdr->numauthrr = HTONS(0);
  hdr->numextrarr = HTONS(0);

  if (num_answers > 0) {
    xtcp_send(tcp_svr, data, dptr-&data[0]);
  }

  return (num_answers > 0);
}


static int mdns_send_netbios_responses(chanend tcp_svr,
                                      xtcp_connection_t *conn,
                                      unsigned int t)
{
  int i;
  for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++) {

#if MDNS_NETBIOS
    if (mdns_table[i].pending & PENDING_NETBIOS_RESPONSE) {
      mdns_send_netbios_response(tcp_svr, conn, &mdns_table[i]);
      mdns_table[i].pending ^= PENDING_NETBIOS_RESPONSE;
      return 1;
    }
#endif
  }
  return 0;
}


void mdns_send(chanend tcp_svr, xtcp_connection_t *conn, unsigned int t)
{
  int res;

  res = mdns_send_probes(tcp_svr, conn, t);
  if (res)
    return;

  res = mdns_send_responses(tcp_svr, conn, t);
  if (res)
    return;

  res = mdns_send_netbios_responses(tcp_svr, conn, t);

  if (res)
    return;

  // nothing to send
  xtcp_send(tcp_svr, NULL, 0);
  return;

}

static mdns_event mdns_periodic(chanend tcp_svr, xtcp_connection_t *conn, unsigned int t)
{
  mdns_event result = 0;
  int i;

  if (mdns_started == 1)
    mdns_start_entries(tcp_svr, conn);

  for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++)
    switch (mdns_table[i].state)
      {
      case UNUSED:
      case DISABLED:
      case ACTIVE:
      case PROBE_WAIT:
      case ANNOUNCE_WAIT:
        break;
      case PROBE_SENT:
        if (timeafter(t,mdns_table[i].timeout)) {
          if (mdns_table[i].num_sent < 3) {
            mdns_table[i].state = PROBE_WAIT;
            xtcp_init_send(tcp_svr, conn);
          }
          else {
            mdns_table[i].state = ANNOUNCE_WAIT;
            mdns_table[i].num_sent = 0;
            xtcp_init_send(tcp_svr, conn);
          }
        }
        break;
      case ANNOUNCE_SENT:
        if (timeafter(t,mdns_table[i].timeout)) {
          if (mdns_table[i].num_sent < 3) {
            mdns_table[i].state = ANNOUNCE_WAIT;
            xtcp_init_send(tcp_svr, conn);
          }
          else {
            mdns_table[i].state = ACTIVE;
            result |= mdns_entry_active;
          }
        }
        break;
      default:
        break;
      }
  return result;
}


mdns_event mdns_handle_event(chanend tcp_svr,
                       xtcp_connection_t *conn,
                       unsigned int t) {

  mdns_event result = 0;

  switch (conn->event)
    {
    case XTCP_IFUP:
      {
        xtcp_ipconfig_t ipconfig;
        xtcp_get_ipconfig(tcp_svr, &ipconfig);

        mdns_start(tcp_svr, ipconfig.ipaddr);
      }
      return mdns_ifup;
    case XTCP_IFDOWN:
    	return mdns_ifdown;
    case XTCP_ALREADY_HANDLED:
      return 0;
    default:
      break;
  }

  if (conn->remote_port == MDNS_SERVER_PORT &&
      conn->event == XTCP_NEW_CONNECTION) {
#if MDNS_NETBIOS
    xtcp_ipaddr_t broadcast_addr = {255,255,255,255};
#endif

    xtcp_set_poll_interval(tcp_svr, conn, 125);
    xtcp_bind_local(tcp_svr, conn, MDNS_SERVER_PORT);
#if MDNS_NETBIOS
    xtcp_connect(tcp_svr, NETBIOS_PORT, broadcast_addr, XTCP_PROTOCOL_UDP);
#endif
    }

#if MDNS_NETBIOS
  if (conn->remote_port == NETBIOS_PORT &&
      conn->event == XTCP_NEW_CONNECTION)
    {
      xtcp_set_poll_interval(tcp_svr, conn, 125);
      xtcp_bind_local(tcp_svr, conn, NETBIOS_PORT);
    }
#endif

  if (conn->local_port == MDNS_SERVER_PORT
#if MDNS_NETBIOS
      || conn->local_port == NETBIOS_PORT
#endif
    )
    {
      switch (conn->event)
        {
        case XTCP_POLL:
          result |= mdns_periodic(tcp_svr, conn, t);
          break;
        case XTCP_RECV_DATA:
          if (conn->local_port == MDNS_SERVER_PORT) {
            result |= mdns_recv(tcp_svr, conn);
          }
#if MDNS_NETBIOS
          else if (conn->local_port == NETBIOS_PORT) {
            result |= netbios_recv(tcp_svr, conn);
          }
#endif
          break;
        case XTCP_REQUEST_DATA:
        case XTCP_SENT_DATA:
        case XTCP_RESEND_DATA:
          mdns_send(tcp_svr, conn, t);
          break;
        default:
          break;
        }
      conn->event = XTCP_ALREADY_HANDLED;
    }
  return result;
}


static void mdns_add_entry(char name_prefix[],
                    char name_postfix[],
                    xtcp_ipaddr_t ipaddr,
                    int srv_port,
                    mdns_entry_type_t entry_type)
{
  int i,j;
  for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++)
    if (mdns_table[i].state != UNUSED &&
        (strcmp(name_postfix, mdns_table[i].name_postfix)==0) &&
        (strcmp(name_prefix, mdns_table[i].name_prefix)==0) )
      break;

  if (i==MDNS_NUM_TABLE_ENTRIES)
    for (i=0;i<MDNS_NUM_TABLE_ENTRIES;i++)
      if (mdns_table[i].state == UNUSED)
        break;

  if (i==MDNS_NUM_TABLE_ENTRIES)
    return;

  mdns_table[i].entry_type = entry_type;
  mdns_table[i].state = DISABLED;
  mdns_table[i].pending = 0;
  strcpy(mdns_table[i].name_prefix, name_prefix);
  strcpy(mdns_table[i].name_postfix, name_postfix);
  if (entry_type == MDNS_SRV_ENTRY) {
    strcpy(mdns_table[i].name_postfix + strlen(name_postfix),".local");
  }
  mdns_table[i].counter = 0;
  update_name(&mdns_table[i]);
  mdns_table[i].srv_port = srv_port;
  if (ipaddr != NULL)
    for (j=0;j<4;j++)
      mdns_table[i].ipaddr[j] = ipaddr[j];
  return;
}

void mdns_register_canonical_name(char name[])
{
  mdns_add_entry(name,"local",NULL,0,MDNS_CANONICAL_NAME_ENTRY);
}

void mdns_register_name(char name[])
{
  mdns_add_entry(name,"local",NULL,0,MDNS_NAME_ENTRY);
}

void mdns_register_service(char name[], char srv_type[],
                           int srv_port, char txt[])
{
  mdns_add_entry(name,srv_type,NULL,srv_port, MDNS_SRV_ENTRY);
}
