// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/* Experimental features.
   The features in this file are **experimental**,
   not supported and not known to work if enabled. Any guarantees of
   the robustness of the component made by XMOS do not hold if these features
   are used.
 */
#include "xtcp_client.h"
#include "xtcp_bufinfo.h"
#include "string.h"
#include "print.h"

void xtcp_buffered_set_rx_buffer(chanend tcp_svr,
                                 xtcp_connection_t *conn,
                                 xtcp_bufinfo_t *bufinfo,
                                 char *buf,
                                 int buflen)
{
  bufinfo->rx_new_event = 1;
  bufinfo->rx_buf = buf;
  bufinfo->rx_end = buf + buflen;
  bufinfo->rx_wrptr = buf;
  bufinfo->rx_rdptr = buf;
}

void xtcp_buffered_set_tx_buffer(chanend tcp_svr,
                                 xtcp_connection_t *conn,
                                 xtcp_bufinfo_t *bufinfo,
                                 char *buf,
                                 int buflen,
                                 int lowmark)
{
  bufinfo->tx_buf = buf;
  bufinfo->tx_end = buf + buflen;
  bufinfo->tx_rdptr = buf;
  bufinfo->tx_prev_rdptr = buf;
  bufinfo->tx_wrptr = buf;
  bufinfo->tx_lowmark = lowmark;
}



int xtcp_buffered_recv_upto(chanend tcp_svr,
                            xtcp_connection_t *conn,
                            xtcp_bufinfo_t *bufinfo,
                            char **buf,
                            char delim,
                            int *overflow)
{
  int len=0;
  int found=0;

  if (bufinfo->rx_new_event) {
    int len = xtcp_recv(tcp_svr, bufinfo->rx_wrptr);
    bufinfo->rx_wrptr += len;
    bufinfo->rx_new_event = 0;
  }

  // search for the delimiter
  for(char *p=bufinfo->rx_rdptr;!found && p<bufinfo->rx_wrptr;p++) {
    if (*p == delim) {
      len = p - bufinfo->rx_rdptr + 1;
      found = 1;
    }
  }

  if (!found)
    {
      // this is the end of the current recv event
      int remaining = bufinfo->rx_wrptr - bufinfo->rx_rdptr;
      int space_left;

      memmove(bufinfo->rx_buf, bufinfo->rx_rdptr, remaining);

      bufinfo->rx_rdptr = bufinfo->rx_buf;
      bufinfo->rx_wrptr = bufinfo->rx_buf + remaining;

      space_left = bufinfo->rx_end - bufinfo->rx_wrptr;

      if (space_left < XTCP_CLIENT_BUF_SIZE) {
        *overflow = 1;
        len = remaining;
        *buf = bufinfo->rx_buf;
        bufinfo->rx_wrptr = bufinfo->rx_buf;
        bufinfo->rx_rdptr = bufinfo->rx_wrptr;
      }
      else {
        bufinfo->rx_new_event = 1;
        len = 0;
      }
    }
  else {
    *buf = bufinfo->rx_rdptr;

    bufinfo->rx_rdptr += len;
  }

  return len;
}



int xtcp_buffered_recv(chanend tcp_svr,
                       xtcp_connection_t *conn,
                       xtcp_bufinfo_t *bufinfo,
                       char **buf,
                       int len,
                       int *overflow)
{
  if (bufinfo->rx_new_event) {
    int len = xtcp_recv(tcp_svr, bufinfo->rx_wrptr);
    bufinfo->rx_wrptr += len;
    bufinfo->rx_new_event = 0;
  }


  if (bufinfo->rx_wrptr - bufinfo->rx_rdptr < len)
    {
      // this is the end of the current recv event
      int remaining = bufinfo->rx_wrptr - bufinfo->rx_rdptr;
      int space_left;

      memmove(bufinfo->rx_buf, bufinfo->rx_rdptr, remaining);

      bufinfo->rx_rdptr = bufinfo->rx_buf;
      bufinfo->rx_wrptr = bufinfo->rx_buf + remaining;

      space_left = bufinfo->rx_end - bufinfo->rx_wrptr;

      if (space_left < XTCP_CLIENT_BUF_SIZE) {
        *overflow = 1;
        len = remaining;
        *buf = bufinfo->rx_buf;
        bufinfo->rx_wrptr = bufinfo->rx_buf;
        bufinfo->rx_rdptr = bufinfo->rx_wrptr;
      }
      else {
        bufinfo->rx_new_event = 1;
        len = 0;
      }
    }
  else {
    *buf = bufinfo->rx_rdptr;

    bufinfo->rx_rdptr += len;
  }

  return len;
}




int xtcp_buffered_send(chanend tcp_svr,
                       xtcp_connection_t *conn,
                       xtcp_bufinfo_t *bufinfo,
                       char *buf,
                       int len)
{
  int space_left;

  space_left = bufinfo->tx_end - bufinfo->tx_wrptr;


  if (space_left < len) {
    int remaining = bufinfo->tx_wrptr - bufinfo->tx_prev_rdptr;
    int shift = bufinfo->tx_prev_rdptr - bufinfo->tx_buf;
    memmove(bufinfo->tx_buf, bufinfo->tx_prev_rdptr, remaining);

    bufinfo->tx_prev_rdptr -= shift;
    bufinfo->tx_rdptr -= shift;
    bufinfo->tx_wrptr -= shift;
    space_left += shift;
  }


  if (space_left < len) {
    return 0;
  }

  memcpy(bufinfo->tx_wrptr, buf, len);

  if (bufinfo->tx_rdptr == bufinfo->tx_wrptr) {
    xtcp_init_send(tcp_svr, conn);
  }

  bufinfo->tx_wrptr += len;

  space_left -= len;

  space_left += (bufinfo->tx_prev_rdptr - bufinfo->tx_buf);


  if (space_left < bufinfo->tx_lowmark) {
    xtcp_pause(tcp_svr, conn);
  }

  return 1;
}


void xtcp_buffered_send_handler(chanend tcp_svr, xtcp_connection_t *conn,
                                xtcp_bufinfo_t *bufinfo)
{
  int space_left, len;

  if (conn->event == XTCP_RESEND_DATA) {
    xtcp_send(tcp_svr,
              bufinfo->tx_prev_rdptr,
              bufinfo->tx_rdptr - bufinfo->tx_prev_rdptr);
    return;
  }

  space_left =
    (bufinfo->tx_end - bufinfo->tx_wrptr) +
    (bufinfo->tx_prev_rdptr - bufinfo->tx_buf);


  len = bufinfo->tx_wrptr - bufinfo->tx_rdptr;
  if (len > conn->mss)
    len = conn->mss;

  xtcp_send(tcp_svr, bufinfo->tx_rdptr, len);

  if (space_left < bufinfo->tx_lowmark &&
      space_left + len >= bufinfo->tx_lowmark)
    xtcp_unpause(tcp_svr, conn);


  bufinfo->tx_prev_rdptr = bufinfo->tx_rdptr;
  bufinfo->tx_rdptr += len;
  return;
}

int xtcp_buffered_send_buffer_remaining(xtcp_bufinfo_t *bufinfo)
{
  int space_left;

  space_left =
    (bufinfo->tx_end - bufinfo->tx_wrptr) +
    (bufinfo->tx_prev_rdptr - bufinfo->tx_buf);

  return (space_left);
}
