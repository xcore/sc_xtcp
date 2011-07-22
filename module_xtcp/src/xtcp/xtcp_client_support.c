// Copyright (c) 2011, XMOS Ltd., All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

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
                    
