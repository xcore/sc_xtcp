// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include "xtcp_client.h"

void xtcp_wait_for_ifup(chanend tcp_svr)
{
  xtcp_connection_t conn;
  conn.event = XTCP_ALREADY_HANDLED;
  do {
    slave xtcp_event(tcp_svr, conn);
  } while (conn.event != XTCP_IFUP);
  return;
}

xtcp_connection_t xtcp_wait_for_connection(chanend tcp_svr)
{
  xtcp_connection_t conn;
  conn.event = XTCP_ALREADY_HANDLED;
  do {
    slave xtcp_event(tcp_svr, conn);
  } while (conn.event != XTCP_NEW_CONNECTION);
  return conn;
}

int xtcp_write(chanend tcp_svr,
               xtcp_connection_t &conn,
               unsigned char buf[],
               int len)
{
  int finished = 0;
  int success = 1;
  int index = 0, prev = 0;
  int id = conn.id;
  xtcp_init_send(tcp_svr, conn);
  while (!finished) {
    slave xtcp_event(tcp_svr, conn);
    switch (conn.event)
      {
      case XTCP_NEW_CONNECTION:
        xtcp_close(tcp_svr, conn);
        break;
      case XTCP_REQUEST_DATA:
      case XTCP_SENT_DATA:
        { int sendlen = (len - index);
          if (sendlen > conn.mss)
            sendlen = conn.mss;

          xtcp_sendi(tcp_svr, buf, index, sendlen);
          prev = index;
          index += sendlen;
          if (sendlen == 0)
            finished = 1;
        }
        break;
      case XTCP_RESEND_DATA:
        xtcp_sendi(tcp_svr, buf, prev, (index-prev));
        break;
      case XTCP_RECV_DATA:
        slave { tcp_svr <: 0; } // delay packet receive
        if (prev != len)
          success = 0;
        finished = 1;
        break;
      case XTCP_TIMED_OUT:
      case XTCP_ABORTED:
      case XTCP_CLOSED:
        if (conn.id == id) {
          finished = 1;
          success = 0;
        }
        break;
      case XTCP_IFDOWN:
        finished = 1;
        success = 0;
        break;
      }
  }
  return success;
}


int xtcp_read(chanend tcp_svr,
              xtcp_connection_t &conn,
              unsigned char buf[],
              int minlen)
{
  int rlen = 0;
  int id = conn.id;
  while (rlen < minlen) {
    slave xtcp_event(tcp_svr, conn);
    switch (conn.event)
      {
      case XTCP_NEW_CONNECTION:
        xtcp_close(tcp_svr, conn);
        break;
      case XTCP_RECV_DATA:
        {
          int n;
          n = xtcp_recvi(tcp_svr, buf, rlen);
          rlen += n;
        }
        break;
      case XTCP_REQUEST_DATA:
      case XTCP_SENT_DATA:
      case XTCP_RESEND_DATA:
        xtcp_send(tcp_svr, null, 0);
        break;
      case XTCP_TIMED_OUT:
      case XTCP_ABORTED:
      case XTCP_CLOSED:
        if (conn.id == id)
          return -1;
        break;
      case XTCP_IFDOWN:
        return -1;
      }
  }
  return rlen;
}

