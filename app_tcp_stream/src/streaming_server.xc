#include "xtcp_client.h"
#include "streaming_server.h"
#include "mutual_thread_comm.h"
#include <xs1.h>

#define STCP_FLUSH_TIMEOUT 20000000

void streaming_server(chanend c_xtcp,
                      chanend c_data)
{
  mutual_comm_state_t mstate;
  timer tmr;
  int t;
  tmr :> t;
  mutual_comm_init_state(mstate);
  stcpInit(c_xtcp, c_data, mstate);

  while (1) {
    xtcp_connection_t conn;
    int is_data_request;

#pragma ordered
    select
      {
      case xtcp_event(c_xtcp, conn):
        stcpHandleEvent(c_xtcp, conn);
        break;
      case mutual_comm_transaction(c_data, is_data_request, mstate):
        if (is_data_request)
          stcpSendDataToClient(c_data, mstate);
        else
          stcpGetDataFromClient(c_xtcp, c_data);

        mutual_comm_complete_transaction(c_data, is_data_request, mstate);
        break;
      case tmr when timerafter(t) :> int tmp:
        stcpFlushBuffer(c_xtcp);
        t += STCP_FLUSH_TIMEOUT;
        break;
      }
  }
}
