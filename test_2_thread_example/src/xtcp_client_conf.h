// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#define XTCP_BUFFERED_API 1
#define UIP_USE_SINGLE_THREADED_ETHERNET
#define XTCP_VERBOSE_DEBUG

#define XTCP_EXCLUDE_UNLISTEN
#define XTCP_EXCLUDE_CONNECT
#define XTCP_EXCLUDE_BIND_REMOTE
#define XTCP_EXCLUDE_BIND_LOCAL
#define XTCP_EXCLUDE_SET_APPSTATE
#define XTCP_EXCLUDE_ABORT
#define XTCP_EXCLUDE_SET_POLL_INTERVAL
#define XTCP_EXCLUDE_JOIN_GROUP
#define XTCP_EXCLUDE_LEAVE_GROUP
#define XTCP_EXCLUDE_GET_MAC_ADDRESS
#define XTCP_EXCLUDE_GET_IPCONFIG
#define XTCP_EXCLUDE_ACK_RECV
#define XTCP_EXCLUDE_ACK_RECV_MODE
#define XTCP_EXCLUDE_PAUSE
#define XTCP_EXCLUDE_UNPAUSE
