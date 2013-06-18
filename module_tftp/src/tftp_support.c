#include "tftp.h"
#include "tftp_app.h"
#include <platform.h>
#include <string.h>
#include "xassert.h"
#include "print.h"
#include "ethernet_server.h"

static unsigned short prev_block_num = 0;

static int tftp_make_ack_pkt(unsigned char *tx_buf, unsigned short block_num)
{
  tftp_ack_t *pkt = (tftp_ack_t*) &tx_buf[0];

  pkt->opcode = hton16((unsigned short) TFTP_OPCODE_ACK);
  pkt->block_number = hton16(block_num);

#if TFTP_DEBUG_PRINT
        printstr("TFTP: Gen ACK, block #");
        printintln(block_num);
#endif

  return TFTP_MIN_PKT_SIZE;
}

static int tftp_make_error_pkt(unsigned char *tx_buf, unsigned short code, char *msg, int *error)
{
  tftp_error_t *pkt = (tftp_error_t*) &tx_buf[0];

  if (error != 0)
  {
    *error = 1;
  }

  // The length of the error message must always be less than TFTP_ERROR_MSG_MAX_LENGTH
  assert(strlen(msg) < TFTP_ERROR_MSG_MAX_LENGTH);

  pkt->opcode = hton16((unsigned short) TFTP_OPCODE_ERROR);
  pkt->error_code = hton16(code);

  strcpy(pkt->error_msg, msg);

#if TFTP_DEBUG_PRINT
        printstr("TFTP: Gen ERROR, code ");
        printintln(code);
#endif

  return (TFTP_MIN_PKT_SIZE + strlen(msg) + TFTP_NULL_BYTE);
}

int tftp_process_app_error(unsigned char *tx_buf)
{
  return tftp_make_error_pkt(tx_buf, TFTP_ERROR_NOT_DEFINED, "Application error", 0);
}

int tftp_process_packet(unsigned char *tx_buf, unsigned char *rx_buf, int num_bytes, unsigned short *block_num_glob, int *error, int *complete)
{
  tftp_packet_t *pkt = (tftp_packet_t*) &rx_buf[0];

  u16_t opcode = ntoh16(pkt->opcode);

  switch (opcode)
  {
    case TFTP_OPCODE_RRQ: // Read Request
    {
      // We don't support read requests - reply with an error packet
      return tftp_make_error_pkt(tx_buf, TFTP_ERROR_NOT_DEFINED, "Read not supported", error);

    }
    case TFTP_OPCODE_WRQ: // Write Request
    {
      char *filename;
      char *mode;
      filename = (char *) pkt->payload;

#if !TFTP_ACCEPT_ANY_FILENAME
      // Check that the requested filename matches what we expect
      if (strncmp(filename, TFTP_IMAGE_FILENAME, strlen(TFTP_IMAGE_FILENAME)) != 0)
      {
        return tftp_make_error_pkt(tx_buf, TFTP_ERROR_NOT_DEFINED, "Invalid filename", error);
      }
#endif

      // Generate a pointer to the mode string
      mode = filename + strlen(filename) + TFTP_NULL_BYTE;

      // Must be a binary transfer
      if (strncmp(mode, "octet", 5) != 0)
      {
        return tftp_make_error_pkt(tx_buf, TFTP_ERROR_NOT_DEFINED, "Invalid transfer mode", error);
      }

      // ACK with data block number zero
      return tftp_make_ack_pkt(tx_buf, 0);
    }
    case TFTP_OPCODE_DATA: // Data Packet
    {
      unsigned short block_num;
      tftp_data_t *data_pkt = (tftp_data_t*) &rx_buf[0];

      if (num_bytes < (TFTP_BLOCK_SIZE + TFTP_MIN_PKT_SIZE))
      {
        // last block
        *complete = 1;
      }

      block_num = ntoh16(data_pkt->block_num);

      *block_num_glob = block_num;

      // Check that we've received the correct block of data and it's not a duplicate
      if (block_num == (prev_block_num + 1))
      {
        prev_block_num = block_num;

#if TFTP_DEBUG_PRINT
        printstr("TFTP: Rcvd data, block #");
        printintln(block_num);
#endif

        if ((block_num * TFTP_BLOCK_SIZE) >= TFTP_MAX_FILE_SIZE)
        {
          // We have received more data that the allowed maximum - send an error
          return tftp_make_error_pkt(tx_buf, TFTP_ERROR_DISK_FULL, "", error);
        }

        // Here the data is passed to the application for processing. It can signal an error to TFTP
        // by returning a non-zero value.
        if (tftp_app_process_data_block(data_pkt->data, num_bytes - TFTP_MIN_PKT_SIZE) != 0)
        {
          // We send an access violation error, but this could be modified to send a custom
          // error from the application layer */
          return tftp_make_error_pkt(tx_buf, TFTP_ERROR_ACCESS_VIOLATION, "", error);
        }
      }
      else
      {
#if TFTP_DEBUG_PRINT
        printstr("TFTP: Rvcd invalid data, block #");
        printintln(block_num);
#endif

      }

      // Make the ACK packet for the received data
      return tftp_make_ack_pkt(tx_buf, block_num);
    }
    case TFTP_OPCODE_ACK: // Acknowledgement
    {
      // We never expect to receive an ACK packet from the active connection
      return tftp_make_error_pkt(tx_buf, TFTP_ERROR_ILLEGAL_OPERATION, "", error);
    }
    case TFTP_OPCODE_ERROR: // Error packet
    {
      return -1;
    }
    default:
    {
      // Error: Not a valid TFTP opcode
      return tftp_make_error_pkt(tx_buf, TFTP_ERROR_ILLEGAL_OPERATION, "", error);
    }
  }

}

