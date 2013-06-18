#ifndef TFTP_H_
#define TFTP_H_

#include "uip_server.h"
#include "nettypes.h"
#include "tftp_conf.h"

/* TFTP opcodes */
#define TFTP_OPCODE_RRQ           1
#define TFTP_OPCODE_WRQ           2
#define TFTP_OPCODE_DATA          3
#define TFTP_OPCODE_ACK           4
#define TFTP_OPCODE_ERROR         5

/* TFTP error codes */
#define TFTP_ERROR_NOT_DEFINED      0
#define TFTP_ERROR_FILE_NOT_FOUND     1
#define TFTP_ERROR_ACCESS_VIOLATION   2
#define TFTP_ERROR_DISK_FULL      3
#define TFTP_ERROR_ILLEGAL_OPERATION  4
#define TFTP_ERROR_UNKNOWN_TID      5
#define TFTP_ERROR_FILE_EXISTS      6
#define TFTP_ERROR_NO_SUCH_USER     7

/* Misc constants */
#define TFTP_NULL_BYTE          1
#define TFTP_MIN_PKT_SIZE       4

#define TFTP_RX_BUFFER_SIZE (TFTP_MIN_PKT_SIZE + TFTP_BLOCK_SIZE)
#define TFTP_TX_BUFFER_SIZE (TFTP_MIN_PKT_SIZE + TFTP_ERROR_MSG_MAX_LENGTH)

#define TFTP_SOURCE_TID_SEED      51337

typedef struct
{
  n16_t opcode;
  unsigned char payload[TFTP_BLOCK_SIZE + TFTP_MIN_PKT_SIZE];
} tftp_packet_t;

typedef struct
{
  n16_t opcode;
  n16_t block_number;
} tftp_ack_t;

typedef struct
{
  n16_t opcode;
  n16_t error_code;
  char error_msg[TFTP_ERROR_MSG_MAX_LENGTH];
} tftp_error_t;

typedef struct
{
  n16_t opcode;
  n16_t block_num;
  unsigned char data[TFTP_BLOCK_SIZE];
} tftp_data_t;


/** This function must be called once at device start-up before an xtcp event
 *  is handled by tftp_handle_event(). It initialises TFTP state and invokes
 *  xtcp to listen on TFTP_DEFAULT_PORT.
 *
 *  \param c_xtcp Chanend of the channel connected to xtcp server
 *
 **/
void tftp_init(chanend c_xtcp);

/** This should be called every time an event is signalled from the xtcp server
 *  via the xtcp_event() select case, e.g.
 *
 *  \verbatim
 *    case xtcp_event(c_xtcp, conn):
 *    {
 *      ...
 *      tftp_handle_event(c_xtcp, conn);
 *      ...
 *      break;
 *    }
 *  \endverbatim
 *
 *  It is the only entry point into the TFTP state machine, which decodes the
 *  packets and generates any necessary reply. It invokes the functions
 *  defined in tftp_app.h, where the user should implement their higher level
 *  application code.
 *
 *  NOTE: This implementation is designed to only accept a single TFTP connection
 *  at a time.
 *
 *  \param c_xtcp Chanend of the channel connected to xtcp server
 *  \param conn   The connection related to the current event (from xtcp_event)
 *
 **/
void tftp_handle_event(chanend c_xtcp, xtcp_connection_t conn);

/** This function is called by tftp_handle_event() when the application layer
 *  indicates via tftp_app_transfer_begin() that it is not ready to accept an TFTP
 *  transfer, or there has been an error.
 *
 *  It generates an error packet in tx_buf[].
 *
 *  \param tx_buf A global transmit packet buffer of size TFTP_TX_BUFFER_SIZE
 *  \return     The number of bytes generated in tx_buf.
 *
 **/
int tftp_process_app_error(unsigned char tx_buf[]);

// \return  0 on success, non zero on failure.

/** Called from tftp_handle_event, this function decodes the TFTP packet and
 *  generates the reply packet in the tx_buf, if one is needed.
 *
 *  \param tx_buf       A global transmit packet buffer of size TFTP_TX_BUFFER_SIZE
 *  \param rx_buf       A receive buffer containing the packet data to be processed
 *  \param num_bytes    The number of valid bytes in rx_buf
 *  \param block_num    A pointer to a global variable for returning the TFTP block
 *                      number associated with a received DATA packet
 *  \param signal_error   A pointer to a global variable for returning an error signal
 *                        to tftp_handle_event(), if this function has created an
 *                        error packet in response to the received packet
 *  \param signal_complete  A pointer to a global variable for returning a signal when
 *                          the very last TFTP DATA packet has been received
 *  \return         The number of bytes in tx_buf (the reply packet), or -1 to
 *                  indicate no reply should be sent and the connection should
 *                  be closed.
 *
 **/
int tftp_process_packet(unsigned char tx_buf[], unsigned char rx_buf[], int num_bytes,
            REFERENCE_PARAM(unsigned short, block_num),
            REFERENCE_PARAM(int, signal_error),
            REFERENCE_PARAM(int, signal_complete));

#endif /* TFTP_H_ */
