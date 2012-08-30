// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef   _xtcp_cmd_h_
#define   _xtcp_cmd_h_

#define XTCP_CMD_TOKEN 128

typedef enum xtcp_cmd_t {
  XTCP_CMD_LISTEN,
  XTCP_CMD_UNLISTEN,
  XTCP_CMD_CONNECT,
  XTCP_CMD_BIND_LOCAL,
  XTCP_CMD_BIND_REMOTE,
  XTCP_CMD_INIT_SEND,
  XTCP_CMD_SET_APPSTATE,
  XTCP_CMD_CLOSE,
  XTCP_CMD_ABORT,
  XTCP_CMD_TIMED_OUT,
  XTCP_CMD_SET_POLL_INTERVAL,
  XTCP_CMD_JOIN_GROUP,
  XTCP_CMD_LEAVE_GROUP,
  XTCP_CMD_GET_MAC_ADDRESS,
  XTCP_CMD_GET_IPCONFIG,
  XTCP_CMD_ACK_RECV,
  XTCP_CMD_ACK_RECV_MODE,
  XTCP_CMD_PAUSE,
  XTCP_CMD_UNPAUSE,
  XTCP_CMD_UPDATE_BUFINFO,
  XTCP_CMD_ACCEPT_PARTIAL_ACK
} xtcp_cmd_t;

#endif // _xtcp_cmd_h_
