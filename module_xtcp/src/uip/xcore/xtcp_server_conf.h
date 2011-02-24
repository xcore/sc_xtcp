#include "timer.h"

typedef struct xtcp_server_state_t {
  int send_request;
  int abort_request;
  int close_request;  
  int poll_interval;
  int connect_request;
  int closed;
  struct uip_timer tmr;
  int uip_conn;
  int ack_recv_mode;
} xtcp_server_state_t;
