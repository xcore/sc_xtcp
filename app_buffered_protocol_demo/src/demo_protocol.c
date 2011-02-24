#include "xtcp_client.h"
#include "xtcp_buffered_client.h"
#include "demo_protocol.h"
#include "print.h"

int decode_demo_protocol_header(char *hdr) {
  return *hdr;
}

void demo_protocol_process_message(chanend tcp_svr,
                                   xtcp_connection_t *conn,
                                   demo_protocol_state_t *st,
                                   char *msg)
{
  char reply[7] = {0,1,2,3,4,5,6};
  int success;
  printstrln(&msg[1]);
  success = xtcp_buffered_send(tcp_svr, conn, &st->bufinfo, reply, 7);

  if (!success) 
    printstr("send buffer overflow\n");

  return;
}





