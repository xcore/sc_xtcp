#ifndef __demo_protocol_h__
#define __demo_protocol_h__

#include "xtcp_client.h"
#include "xtcp_buffered_client.h"

#define DEMO_PROTOCOL_PORT 15533
#define DEMO_PROTOCOL_HDR_SIZE 1
#define DEMO_PROTOCOL_MAX_MSG_SIZE 100
#define DEMO_PROTOCOL_RXBUF_LEN 2048
#define DEMO_PROTOCOL_TXBUF_LEN 1024

#define DEMO_PROTOCOL_CONN_TIMEOUT_MS (500) // timeout due to inactivity
                                            //every 500 ms


#ifndef __XC__

typedef struct demo_protocol_state_t
{
  int active;
  int got_header;
  int len;
  int last_used;
  int conn_id;
  xtcp_bufinfo_t bufinfo;
  char inbuf[DEMO_PROTOCOL_RXBUF_LEN];
  char outbuf[DEMO_PROTOCOL_TXBUF_LEN]; 
} demo_protocol_state_t;

#endif


void demo_protocol_process_message(chanend tcp_svr,
                                   xtcp_connection_t *conn,
                                   demo_protocol_state_t *st,
                                   char *msg);

int decode_demo_protocol_header(char *hdr);

#endif //__demo_protocol_h__
