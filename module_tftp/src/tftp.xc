#include "tftp.h"
#include "tftp_app.h"
#include <platform.h>
#include "print.h"
#include "uip_server.h"
#include "getmac.h"
#include "ethernet_server.h"

static enum
{
  TFTP_IDLE,
  TFTP_WAITING_FOR_CONNECTION,
  TFTP_WAITING_FOR_DATA,
  TFTP_SENDING_ACK,
  TFTP_COMPLETE
} tftp_state = TFTP_IDLE;

static unsigned short local_tid = TFTP_SOURCE_TID_SEED;

static unsigned short block_num;
static unsigned short prev_block_num;

unsigned char tx_buffer[TFTP_TX_BUFFER_SIZE];

static int num_tx_bytes;

static xtcp_connection_t tftp_conn;

static int signal_error;
static int signal_complete;

void tftp_init(chanend c_xtcp)
{
  signal_error = 0;
  signal_complete = 0;

  block_num = 0;
  prev_block_num = 0;

  num_tx_bytes = 0;

  tftp_conn.id = -1;

  xtcp_listen(c_xtcp, TFTP_DEFAULT_PORT, XTCP_PROTOCOL_UDP);

}

void tftp_close(chanend c_xtcp, xtcp_connection_t conn)
{
  xtcp_close(c_xtcp, conn);

  tftp_conn.id = -1;

  signal_error = 0;
  signal_complete = 0;
  block_num = 0;
  prev_block_num = 0;
  num_tx_bytes = 0;
}

void tftp_handle_event(chanend c_xtcp, xtcp_connection_t conn)
{
  unsigned char rx_buffer[TFTP_RX_BUFFER_SIZE];

  int response_len;

  // If the state is complete, we aren't accepting any new TFTP connections
  if (tftp_state == TFTP_COMPLETE) return;

  // Check if this connection is TFTP
  if (conn.event != XTCP_IFUP && conn.event != XTCP_IFDOWN &&
    conn.local_port != TFTP_DEFAULT_PORT && conn.local_port != local_tid)
  {
    return;
  }

  switch (conn.event)
  {
    case XTCP_IFUP:
    {
#if TFTP_DEBUG_PRINT
        printstrln("TFTP: IP Up");
#endif
      // When the network interface comes up, we are ready to accept a TFTP connection
      tftp_state = TFTP_WAITING_FOR_CONNECTION;

      break;
    }
    case XTCP_IFDOWN:
    {
      // If the interface goes down during a transfer, we should flag an error to the application
      // layer and close the active connection.
      if (tftp_state == TFTP_WAITING_FOR_DATA || tftp_state == TFTP_SENDING_ACK)
      {
#if TFTP_DEBUG_PRINT
        printstrln("TFTP: IP Down");
#endif
        tftp_app_transfer_error();

        tftp_close(c_xtcp, conn);

        tftp_state == TFTP_IDLE;
      }
      break;
    }
    case XTCP_NEW_CONNECTION:
    {
#if TFTP_DEBUG_PRINT
      printstr("TFTP: New connection to listening port ");
      printintln(conn.local_port);
#endif

      if (tftp_state == TFTP_WAITING_FOR_CONNECTION && conn.local_port == TFTP_DEFAULT_PORT)
      {
        tftp_conn = conn;

        // We always reply from a new (random) port
        local_tid++;

        xtcp_bind_local(c_xtcp, tftp_conn, local_tid);

        if (tftp_app_transfer_begin() != 0)
        {
          // The application signalled that it isn't ready to receive data

          // Send an error reply
          num_tx_bytes = tftp_process_app_error(tx_buffer);

          signal_error = 1;

          // Sanity check, this should always be > 0 in this case
          if (num_tx_bytes > 0)
          {
            xtcp_init_send(c_xtcp, conn);
            tftp_state = TFTP_SENDING_ACK;
            break;
          }
        }

        tftp_conn = conn;

        xtcp_set_poll_interval(c_xtcp, tftp_conn, TFTP_TIMEOUT_SECONDS * 1000);

        // Bind the local port to the destination TID of the received packet
        xtcp_bind_local(c_xtcp, tftp_conn, local_tid);

        // We set the state to indicate that we expect the WRQ packet as the next XTCP_RECV_DATA event
        // - not an actual DATA packet in this instance
        tftp_state = TFTP_WAITING_FOR_DATA;
      }

      break;
    }

    case XTCP_RECV_DATA:
    {
      response_len = xtcp_recv_count(c_xtcp, rx_buffer, TFTP_RX_BUFFER_SIZE);

      if (tftp_state == TFTP_WAITING_FOR_DATA && conn.id == tftp_conn.id)
      {
        num_tx_bytes = tftp_process_packet(tx_buffer, rx_buffer, response_len, block_num, signal_error, signal_complete);

        // We always generate a reply (ACK or ERROR) from a received packet,
        // unless it's an error, in which case we close the connection
        if (num_tx_bytes > 0)
        {
          xtcp_init_send(c_xtcp, conn);
          tftp_state = TFTP_SENDING_ACK;
        }
        else
        {
#if TFTP_DEBUG_PRINT
        printstr("TFTP: Received an error");
#endif
          tftp_app_transfer_error();

          tftp_close(c_xtcp, conn);

          tftp_state = TFTP_WAITING_FOR_CONNECTION;
        }
      }

      break;
    }
    case XTCP_REQUEST_DATA:
    case XTCP_RESEND_DATA:
    {
      if (tftp_state == TFTP_SENDING_ACK && num_tx_bytes > 0)
      {
        xtcp_send(c_xtcp, tx_buffer, num_tx_bytes);
      }

      break;
    }
    case XTCP_SENT_DATA:
    {
      xtcp_complete_send(c_xtcp);

      num_tx_bytes = 0;

      if (signal_error)
      {
#if TFTP_DEBUG_PRINT
        printstrln("TFTP: Transfer error");
#endif
        tftp_app_transfer_error();

        tftp_close(c_xtcp, conn);

        tftp_state = TFTP_WAITING_FOR_CONNECTION;
        break;
      }

      if (signal_complete)
      {
#if TFTP_DEBUG_PRINT
        printstrln("TFTP: Transfer complete");
#endif
        tftp_app_transfer_complete();

        tftp_close(c_xtcp, conn);

        tftp_state = TFTP_COMPLETE;
        break;
      }

      tftp_state = TFTP_WAITING_FOR_DATA;

      break;
    }
    case XTCP_POLL:
    {
      // Handles timeouts
      if (tftp_state == TFTP_WAITING_FOR_DATA && prev_block_num == block_num)
      {
#if TFTP_DEBUG_PRINT
        printstrln("TFTP: Connection timed out");
#endif
        tftp_close(c_xtcp, tftp_conn);

        tftp_state = TFTP_WAITING_FOR_CONNECTION;
      }
      else
      {
        prev_block_num = block_num;
      }
      break;
    }
    case XTCP_TIMED_OUT:
    case XTCP_ABORTED:
    case XTCP_CLOSED:
    {
#if TFTP_DEBUG_PRINT
      printstr("TFTP: Closed connection ");
      printintln(conn.id);
#endif
      break;
    }
    case XTCP_ALREADY_HANDLED:
    break;
  }

}
