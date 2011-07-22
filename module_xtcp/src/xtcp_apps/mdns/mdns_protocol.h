/**
 * Module:  module_xtcp
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    mdns_protocol.h
 *
 * The copyrights, all other intellectual and industrial 
 * property rights are retained by XMOS and/or its licensors. 
 * Terms and conditions covering the use of this code can
 * be found in the Xmos End User License Agreement.
 *
 * Copyright XMOS Ltd 2009
 *
 * In the case where this code is a modification of existing code
 * under a separate license, the separate license terms are shown
 * below. The modifications to the code are still covered by the 
 * copyright notice above.
 *
 **/                                   
#ifndef _mdns_protocol_h_
#define _mdns_protocol_h_

/** MDNS maximum number of retries when asking for a name, before "timeout". */
#define MDNS_MAX_RETRIES           4

/** MDNS resource record max. TTL (one week as default) */
#define MDNS_MAX_TTL               604800

/** MDNS field TYPE used for "Resource Records" */
#define MDNS_RRTYPE_A              1     /* a host address */
#define MDNS_RRTYPE_NS             2     /* an authoritative name server */
#define MDNS_RRTYPE_MD             3     /* a mail destination (Obsolete - use MX) */
#define MDNS_RRTYPE_MF             4     /* a mail forwarder (Obsolete - use MX) */
#define MDNS_RRTYPE_CNAME          5     /* the canonical name for an alias */
#define MDNS_RRTYPE_SOA            6     /* marks the start of a zone of authority */
#define MDNS_RRTYPE_MB             7     /* a mailbox domain name (EXPERIMENTAL) */
#define MDNS_RRTYPE_MG             8     /* a mail group member (EXPERIMENTAL) */
#define MDNS_RRTYPE_MR             9     /* a mail rename domain name (EXPERIMENTAL) */
#define MDNS_RRTYPE_NULL           10    /* a null RR (EXPERIMENTAL) */
#define MDNS_RRTYPE_WKS            11    /* a well known service description */
#define MDNS_RRTYPE_PTR            12    /* a domain name pointer */
#define MDNS_RRTYPE_HINFO          13    /* host information */
#define MDNS_RRTYPE_MINFO          14    /* mailbox or mail list information */
#define MDNS_RRTYPE_MX             15    /* mail exchange */
#define MDNS_RRTYPE_TXT            16    /* text strings */
#define MDNS_RRTYPE_AAAA           28
#define MDNS_RRTYPE_SRV            33
/** MDNS field CLASS used for "Resource Records" */
#define MDNS_RRCLASS_IN            1     /* the Internet */
#define MDNS_RRCLASS_CS            2     /* the CSNET class (Obsolete - used only for examples in some obsolete RFCs) */
#define MDNS_RRCLASS_CH            3     /* the CHAOS class */
#define MDNS_RRCLASS_HS            4     /* Hesiod [Dyer 87] */
#define MDNS_RRCLASS_FLUSH         0x8000 /* Flush bit */


/* MDNS protocol flags */
#define MDNS_FLAG1_RESPONSE        0x80
#define MDNS_FLAG1_OPCODE_STATUS   0x10
#define MDNS_FLAG1_OPCODE_INVERSE  0x08
#define MDNS_FLAG1_OPCODE_STANDARD 0x00
#define MDNS_FLAG1_AUTHORATIVE     0x04
#define MDNS_FLAG1_TRUNC           0x02
#define MDNS_FLAG1_RD              0x01
#define MDNS_FLAG2_RA              0x80
#define MDNS_FLAG2_ERR_MASK        0x0f
#define MDNS_FLAG2_ERR_NONE        0x00
#define MDNS_FLAG2_ERR_NAME        0x03


/** MDNS message header */
struct mdns_hdr {
  u16_t id;
  u8_t flags1;
  u8_t flags2;
  u16_t numquestions;
  u16_t numanswers;
  u16_t numauthrr;
  u16_t numextrarr;
}  __attribute__((packed));


/** MDNS query message structure */
struct mdns_query {
  /* MDNS query record starts with either a domain name or a pointer
     to a name already present somewhere in the packet. */
  u16_t type;
  u16_t class;
}  __attribute__((packed));

/** MDNS answer message structure */
struct mdns_answer {
  /* MDNS answer record starts with either a domain name or a pointer
     to a name already present somewhere in the packet. */
  u16_t type;
  u16_t class;
  u32_t ttl;
  u16_t len;
}  __attribute__((packed));

struct mdns_srv_hdr {
  u16_t priority;
  u16_t weight;
  u16_t port;
} __attribute__((packed));


#endif // _mdns_protocol_h_
