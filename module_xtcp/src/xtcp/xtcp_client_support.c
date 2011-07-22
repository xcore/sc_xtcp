/**
 * Module:  module_xtcp
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    xtcp_client_support.c
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
#include <xtcp_client.h>
#include <print.h>

#ifndef NULL
#define NULL ((void *) 0)
#endif

int xtcp_send_buffer(chanend tcp_svr,
                     xtcp_connection_t *conn,
                     xtcp_transfer_state_t *ts,
                     unsigned char *data,
                     int totlen)
{
  int len;
  int finished = 0;

  switch (conn->event) 
    {    
    case XTCP_REQUEST_DATA:
      ts->len = totlen;
      ts->left = totlen;
      ts->dptr = data;        
      break;
    case XTCP_SENT_DATA:
      break;
    case XTCP_RESEND_DATA:
      ts->dptr = ts->prev_dptr;
      ts->left = ts->prev_left;
      break;
    default:
      return 0;
    }
  
  len = conn->mss;
  if (len > ts->left)
    len = ts->left;
  
  xtcp_send(tcp_svr, ts->dptr, len);
  
  if (ts->left == 0) {
    ts->endptr = ts->dptr;
    ts->dptr = NULL;
    finished = 1;
  }
  else {
    ts->prev_dptr = ts->dptr;
    ts->prev_left = ts->left;
    ts->dptr += len;
    ts->left -= len;
  }
     
  return finished;
}
                    
