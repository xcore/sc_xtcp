// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef __xtcp_bufinfo_h__
#define __xtcp_bufinfo_h__

#ifndef __XC__
typedef struct xtcp_bufinfo_t {

  int   rx_new_event;
  char *rx_buf;
  char *rx_end;
  char *rx_wrptr;
  char *rx_rdptr;

  char *tx_buf;
  char *tx_end;
  char *tx_wrptr;
  char *tx_rdptr;
  char *tx_prev_rdptr;
  int   tx_lowmark;

} xtcp_bufinfo_t;
#endif

#define SIZEOF_BUFINFO (11*4)

#endif //__xtcp_bufinfo_h__
