#include <xs1.h>
#include "xtcp_client.h"
#include "xtcp_buffered_client.h"
#include "demo_protocol.h"
#include "print.h"


#define NUM_DEMO_PROTOCOL_CONNECTIONS 3

#define DEMO_PROTOCOL_CONN_TIMEOUT_TIMER_TICKS \
  (DEMO_PROTOCOL_CONN_TIMEOUT_MS * XS1_TIMER_KHZ)


demo_protocol_state_t connection_states[NUM_DEMO_PROTOCOL_CONNECTIONS];

void demo_protocol_init(chanend tcp_svr)
{
  for(int i=0; i < NUM_DEMO_PROTOCOL_CONNECTIONS;i++) {
    connection_states[i].active = 0;
  }
  xtcp_listen(tcp_svr, DEMO_PROTOCOL_PORT, XTCP_PROTOCOL_TCP);
  xtcp_listen(tcp_svr, DEMO_PROTOCOL_PORT, XTCP_PROTOCOL_UDP);
}


void demo_protocol_recv(chanend tcp_svr,
                        xtcp_connection_t *conn)
{
  struct demo_protocol_state_t *st = (struct demo_protocol_state_t *) conn->appstate;
  int len=0;    
  do {
    if (!st->got_header) {
      char *hdr;
      int overflow=0;
      len = xtcp_buffered_recv(tcp_svr, 
                               conn, 
                               &st->bufinfo,
                               &hdr, 
                               DEMO_PROTOCOL_HDR_SIZE, 
                               &overflow);
      if (overflow) {
        xtcp_abort(tcp_svr, conn);
        return;
      }
      if (len) {
        st->len = decode_demo_protocol_header(hdr);
        if (st->len <= 0 || st->len > DEMO_PROTOCOL_MAX_MSG_SIZE) {
          xtcp_abort(tcp_svr, conn);
          return;
        }
        st->got_header = 1;
      }
    }
    else {
      char *msg;
      int overflow=0;
      len = xtcp_buffered_recv(tcp_svr,
                               conn, 
                               &st->bufinfo,
                               &msg, 
                               st->len, 
                               &overflow);
      if (overflow) {
        xtcp_abort(tcp_svr, conn);
        return;
      }      
      if (len) {
        demo_protocol_process_message(tcp_svr, conn, st, msg);        
        st->got_header = 0;
      }
    }    
  } while (len);

  return;
}


void demo_protocol_send(chanend tcp_svr,
                  xtcp_connection_t *conn)
{
  struct demo_protocol_state_t *st = (struct demo_protocol_state_t *) conn->appstate;
  xtcp_buffered_send_handler(tcp_svr, conn, &st->bufinfo);
  return;
}

static void demo_protocol_init_state(chanend tcp_svr,
                                     xtcp_connection_t *conn,
                                     int timestamp)
{
  int i;


  for (i=0;i<NUM_DEMO_PROTOCOL_CONNECTIONS;i++) {
    if (!connection_states[i].active)
      break;
  }

  if (i==NUM_DEMO_PROTOCOL_CONNECTIONS)
    xtcp_abort(tcp_svr, conn);
  else {
    connection_states[i].active = 1;
    connection_states[i].got_header = 0;
    connection_states[i].last_used = timestamp;   
    connection_states[i].conn_id = conn->id;   
    xtcp_set_connection_appstate(tcp_svr, conn, 
                                 (xtcp_appstate_t) &connection_states[i]);    
    xtcp_buffered_set_rx_buffer(tcp_svr, 
                                conn, 
                                &connection_states[i].bufinfo,
                                connection_states[i].inbuf,
                                DEMO_PROTOCOL_RXBUF_LEN);
    xtcp_buffered_set_tx_buffer(tcp_svr, 
                                conn, 
                                &connection_states[i].bufinfo,
                                connection_states[i].outbuf,
                                DEMO_PROTOCOL_TXBUF_LEN,
                                DEMO_PROTOCOL_MAX_MSG_SIZE);    
  }
  return;
}


static void demo_protocol_free_state(xtcp_connection_t *conn)
{
  struct demo_protocol_state_t *st = (struct demo_protocol_state_t *) conn->appstate;

  if (st) {
    st->active = 0;
  }
}



void demo_protocol_handle_event(chanend tcp_svr, 
                                xtcp_connection_t *conn,
                                int timestamp)
{
  // We have received an event from the TCP stack, so respond 
  // appropriately

  // Ignore events that are not directly relevant to us
  switch (conn->event) 
    {
    case XTCP_IFUP:
    case XTCP_IFDOWN:
    case XTCP_ALREADY_HANDLED:
      return;
    default:
      break;
    }


  if (conn->local_port == DEMO_PROTOCOL_PORT) {
    struct demo_protocol_state_t *st = 
      (struct demo_protocol_state_t *) conn->appstate;
    switch (conn->event)
      {
      case XTCP_NEW_CONNECTION:
        demo_protocol_init_state(tcp_svr, conn, timestamp);
        break;          
      case XTCP_RECV_DATA:
        if (st) {
          st->last_used = timestamp;
          demo_protocol_recv(tcp_svr, conn);      
        }
        else 
          xtcp_ignore_recv(tcp_svr);
        break;        
      case XTCP_SENT_DATA:        
      case XTCP_REQUEST_DATA:
      case XTCP_RESEND_DATA:
        if (st) {
          st->last_used = timestamp;
          demo_protocol_send(tcp_svr, conn);
        }
        else
          xtcp_ignore_send(tcp_svr);
        break;         
      case XTCP_TIMED_OUT:
      case XTCP_ABORTED:
      case XTCP_CLOSED:
        demo_protocol_free_state(conn);
        break;
      default:
        // Ignore anything else
        break;
      }
    conn->event = XTCP_ALREADY_HANDLED;
  }

  return;
}


void demo_protocol_periodic(chanend tcp_svr, int t)
{
  // timeout any inactive connections
  for (int i=0;i<NUM_DEMO_PROTOCOL_CONNECTIONS;i++) {
    struct demo_protocol_state_t *st = &connection_states[i];
    xtcp_connection_t conn;
    if (st->active) {
      if ((t - st->last_used) > DEMO_PROTOCOL_CONN_TIMEOUT_TIMER_TICKS) {
        conn.id = st->conn_id;
        xtcp_close(tcp_svr, &conn);
      }
    }
  }    
}
