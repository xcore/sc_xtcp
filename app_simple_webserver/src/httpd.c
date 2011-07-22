/**
 * Module:  app_simple_webserver
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    httpd.c
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
#include <string.h>
#include <print.h>
#include "xtcp_client.h"
#include "httpd.h"
#define NUM_HTTPD_CONNECTIONS 10

typedef struct httpd_state_t {
  int active;
  int conn_id;
  char *dptr;
  int dlen;
  xtcp_transfer_state_t ts;
} httpd_state_t;

httpd_state_t connection_states[NUM_HTTPD_CONNECTIONS];


void httpd_init(void) 
{
  int i;
  for (i=0;i<NUM_HTTPD_CONNECTIONS;i++) {
    connection_states[i].active = 0;
    connection_states[i].dptr = NULL;
  }
}

char page[] = "HTTP/1.0 200 OK\nServer: xc2/pre-1.0 (http://xmos.com)\nContent-type: text/html\n\n<html><head></head><body>Hello World!</body></html>\n\n";

void parse_http_request(httpd_state_t *hs,
                        char *data,
                        int len)
{
  // make it a string
  int i;
  if (hs->dptr != NULL)
    return;

  if (strncmp(data, "GET ", 4) == 0) {
    char *argpos;
    for(i = 0; i < strlen((char *)data+4); i++) {
      if (((char *)data + 4)[i] == ' ' ||
          ((char *)data + 4)[i] == '\r' ||
          ((char *)data + 4)[i] == '\n') {
        ((char *)data + 4)[i] = 0;
      }
    }
    hs->dptr = &page[0];
    hs->dlen = strlen(&page[0]);
    // we have a request    

  }
  else {
    //    hs->dptr = NULL;
  }
     
}


void httpd_recv(chanend tcp_svr,
                xtcp_connection_t *conn)
{
  struct httpd_state_t *hs = (struct httpd_state_t *) conn->appstate;
  unsigned char data[XTCP_CLIENT_BUF_SIZE];
  int len;
  len = xtcp_recv(tcp_svr, data);

  if (hs==NULL || hs->dptr != NULL)
    return;

  parse_http_request(hs, &data[0], len);  
  
  if (hs->dptr != NULL) {
    xtcp_init_send(tcp_svr, conn);
  }
}

void httpd_send(chanend tcp_svr,
                xtcp_connection_t *conn)
{
  struct httpd_state_t *hs = (struct httpd_state_t *) conn->appstate;
  int finished;
  if (hs->dptr == NULL) {
    xtcp_send(tcp_svr, NULL, 0);
  }
  
  finished = xtcp_send_buffer(tcp_svr, 
                              conn, 
                              &(hs->ts),
                              hs->dptr,
                              hs->dlen);
  if (finished)  
    xtcp_close(tcp_svr, conn);
  return;
}


void httpd_init_state(chanend tcp_svr, 
                      xtcp_connection_t *conn)
{
  int i;
  for (i=0;i<NUM_HTTPD_CONNECTIONS;i++) {
    if (!connection_states[i].active) 
      break;    
  }
  
  if (i==NUM_HTTPD_CONNECTIONS) 
    xtcp_abort(tcp_svr, conn);
  else {
    connection_states[i].active = 1;
    connection_states[i].conn_id = conn->id;
    connection_states[i].dptr = NULL;
    xtcp_set_connection_appstate(tcp_svr, conn, (xtcp_appstate_t) &connection_states[i]);
  }
  return;
}
                      
void httpd_free_state(xtcp_connection_t *conn)
{
  int i;
  for (i=0;i<NUM_HTTPD_CONNECTIONS;i++)
    if (connection_states[i].conn_id == conn->id) {
      connection_states[i].active = 0;
    }
}


void httpd_handle_event(chanend tcp_svr,
                        xtcp_connection_t *conn)
{
  switch (conn->event) 
    {
    case XTCP_NEW_CONNECTION:
      httpd_init_state(tcp_svr, conn);
      break;
    case XTCP_RECV_DATA:
      httpd_recv(tcp_svr, conn);
      break;            
    case XTCP_SENT_DATA:
    case XTCP_REQUEST_DATA:
    case XTCP_RESEND_DATA:
      httpd_send(tcp_svr, conn);
      break;      
    case XTCP_TIMED_OUT:
    case XTCP_ABORTED:
    case XTCP_CLOSED:
      httpd_free_state(conn);
      break;
    }
}
