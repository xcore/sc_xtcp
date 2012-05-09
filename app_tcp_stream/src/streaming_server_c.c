#include <print.h>
#include "xtcp_client.h"
#include <xccompat.h>
#include "c_io.h"
#include "mutual_thread_comm.h"
#include "streaming_server.h"

#define STCP_PORT_BASE 8002
#define FIFO_SIZE 32
#define STCP_MIN_PACKET_SIZE 16
#define STCP_MAX_PACKET_SIZE 128
#define STCP_NUM_SOCKETS 8

typedef struct fifo_t {
  char data[FIFO_SIZE];
  int rdptr;
  int wrptr;
} fifo_t;

static void initFifo(fifo_t *fifo) {
  fifo->rdptr = 0;
  fifo->wrptr = 0;
}


static int isFifoEmpty(fifo_t *fifo) {
  return (fifo->rdptr == fifo->wrptr);
}

static int isFifoFull(fifo_t *fifo) {
  int new_wrptr;
  new_wrptr = fifo->wrptr + 1;
  if (new_wrptr >= FIFO_SIZE)
    new_wrptr = 0;
  return (new_wrptr == fifo->rdptr);
}

static int getFillLevel(fifo_t *fifo) {
  int fill = fifo->wrptr - fifo->rdptr;

  if (fill < 0)
    fill += FIFO_SIZE;

  return fill;
}

static void pushItem(fifo_t *fifo, int datum) {
  int new_wrptr;

  new_wrptr = fifo->wrptr+1;
  if (new_wrptr >= FIFO_SIZE)
    new_wrptr = 0;

  if (new_wrptr != fifo->rdptr) {
    fifo->data[fifo->wrptr] = datum;
    fifo->wrptr = new_wrptr;
  }
  else {
    // drop it
  }
}

static int popItem(fifo_t *fifo)
{
  int datum = fifo->data[fifo->rdptr];

  if (fifo->rdptr != fifo->wrptr) {
    fifo->rdptr = fifo->rdptr + 1;
    if (fifo->rdptr >= FIFO_SIZE)
      fifo->rdptr = 0;
  }
  return datum;
}

typedef struct stcp_state_t {
  int active;
  int conn_id;
  fifo_t rx_buffer;
  fifo_t tx_buffer;
  int sending;
  mutual_comm_state_t *mstate;
  chanend c_data;
  int chan_id;
} stcp_state_t;


static stcp_state_t g_state[STCP_NUM_SOCKETS];

static void stcpInitState(stcp_state_t *state,
                          chanend c_data,
                          mutual_comm_state_t *mstate,
                          int i)
{
  state->active = 0;
  initFifo(&state->rx_buffer);
  initFifo(&state->tx_buffer);
  state->sending = 0;
  state->mstate = mstate;
  state->c_data = c_data;
  state->chan_id = i;
}

void stcpInit(chanend c_xtcp, chanend c_data, mutual_comm_state_t *mstate) {
  for (int i=0;i<STCP_NUM_SOCKETS;i++) {
    xtcp_listen(c_xtcp, STCP_PORT_BASE+i, XTCP_PROTOCOL_TCP);
    stcpInitState(&g_state[i], c_data, mstate, i);
  }
}

static void stcpInitConnectionState(chanend c_xtcp, xtcp_connection_t *conn)
{
  int i = conn->local_port - STCP_PORT_BASE;
  stcp_state_t *state = &g_state[i];

  // If we are already connected we boot off the old connection for the
  // new! This is good for testing but probably not what you want for a
  // real application

  printstrln("stcp:new connection");

  if (state->active) {
    xtcp_connection_t old_conn;
    old_conn.id = state->conn_id;
    xtcp_abort(c_xtcp, &old_conn);
  }

  state->active = 1;
  state->conn_id = conn->id;
  state->sending = 0;
  initFifo(&state->tx_buffer);

  xtcp_set_connection_appstate(c_xtcp, conn, (xtcp_appstate_t) state);
}

static void stcpFreeConnectionState(xtcp_connection_t *conn) {
  stcp_state_t *state = (stcp_state_t *) conn->appstate;

  state->active = 0;
}

static void stcpRecv(chanend c_xtcp, xtcp_connection_t *conn)
{
  stcp_state_t *state = (stcp_state_t *) conn->appstate;
  char data[XTCP_CLIENT_BUF_SIZE];
  int len;
  len = xtcp_recv(c_xtcp, data);

  if (state->chan_id ==0) {
    for (int i=0;i<len;i++) {
      if (!isFifoFull(&state->rx_buffer)) {
        pushItem(&state->rx_buffer, data[i]);
        mutual_comm_notify(state->c_data, state->mstate);
      }
      else {
#ifdef STCP_DEBUG
        printstrln("stcp: rx buffer overflow");
#endif
        //drop the data
      }
    }
  }
}

static void stcpSend(chanend c_xtcp, xtcp_connection_t *conn)
{
  char buf[STCP_MAX_PACKET_SIZE];
  int len;
  stcp_state_t *state = (stcp_state_t *) conn->appstate;

  len = getFillLevel(&state->tx_buffer);

  if (len > STCP_MAX_PACKET_SIZE)
    len = STCP_MAX_PACKET_SIZE;

  for (int i=0;i<len;i++)
    buf[i] = popItem(&state->tx_buffer);

  xtcp_send(c_xtcp, buf, len);

  if (len == 0)
    state->sending = 0;

}

void stcpHandleEvent(chanend c_xtcp, xtcp_connection_t *conn)
{
  // Ignore events that are not directly relevant to us
  switch (conn->event) 
    {
    case XTCP_IFUP:
    case XTCP_IFDOWN:
    case XTCP_ALREADY_HANDLED:
      return;
    default:
      break;
    } 

  if (conn->local_port >= STCP_PORT_BASE &&
      conn->local_port < (STCP_PORT_BASE + STCP_NUM_SOCKETS))
    {
      switch (conn->event)
        {
        case XTCP_NEW_CONNECTION:
          stcpInitConnectionState(c_xtcp, conn);
          break;
        case XTCP_RECV_DATA:
          stcpRecv(c_xtcp, conn);
          break;
        case XTCP_SENT_DATA:
        case XTCP_REQUEST_DATA:
        case XTCP_RESEND_DATA:
          stcpSend(c_xtcp, conn);
          break;
        case XTCP_TIMED_OUT:
        case XTCP_ABORTED:
        case XTCP_CLOSED:
          stcpFreeConnectionState(conn);
          break;
        default:
          // Ignore anything else
          break;
        }
    }
}



void stcpSendDataToClient(chanend c_data, mutual_comm_state_t *mstate)
{
  stcp_state_t *state = &g_state[0];
  if (!isFifoEmpty(&state->rx_buffer))
    {
      xc_abi_outuint(c_data, popItem(&state->rx_buffer));
      if (!isFifoEmpty(&state->rx_buffer)) {
        mutual_comm_notify(c_data, mstate);
      }
    }
  else {
    // Something has gone wrong here, we shouldn't be asked for data unless we have
    // some
  }
}

void stcpFlushBuffer(chanend c_xtcp)
{
  for (int i=0;i<STCP_NUM_SOCKETS;i++) {
    stcp_state_t *state = &g_state[i];
    xtcp_connection_t conn;
    if (state->active && !state->sending && !isFifoEmpty(&state->tx_buffer)) {
      conn.id = state->conn_id;
      xtcp_init_send(c_xtcp, &conn);
      state->sending = 1;
    }
  }
}

void stcpGetDataFromClient(chanend c_xtcp, chanend c_data)
{

  int datum;
  xtcp_connection_t conn;

  datum = xc_abi_inuint(c_data);
  for (int i=0;i<STCP_NUM_SOCKETS;i++) {
    stcp_state_t *state = &g_state[i];
    if (!isFifoFull(&state->tx_buffer)) {
      int fill;
      pushItem(&state->tx_buffer, datum);
      fill = getFillLevel(&state->tx_buffer);
      if (fill >= STCP_MIN_PACKET_SIZE && state->active && !state->sending) {
        conn.id = state->conn_id;
        xtcp_init_send(c_xtcp, &conn);
        state->sending = 1;
      }
    }
    else {
#ifdef STCP_DEBUG
      printstrln("stcp: tx buffer overflow");
#endif
      // Drop the data
    }
  }
}
