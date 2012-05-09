#ifndef _streaming_server_h_
#define _streaming_server_h_
#include <xccompat.h>
#include "xtcp_client.h"
#include "mutual_thread_comm.h"

void stcpInit(chanend c_xtcp,
              chanend c_data,
              REFERENCE_PARAM(mutual_comm_state_t,nstate));

void stcpHandleEvent(chanend c_xtcp, REFERENCE_PARAM(xtcp_connection_t,conn));
void stcpSendDataToClient(chanend c_data,
                          REFERENCE_PARAM(mutual_comm_state_t,nstate));
void stcpGetDataFromClient(chanend c_xtcp, chanend c_data);

void streaming_server(chanend c_xtcp,
                      chanend c_data);

void stcpFlushBuffer(chanend c_xtcp);
#endif //_streaming_server_h_
