/** Defines a high level interface to module_tftp from the application.
 *
 *  The following functions must be implemented, even if they are left blank.
 *
 **/
#ifndef TFTP_APP_H_
#define TFTP_APP_H_

#include <xccompat.h>

/** This is called at the beginning of a new TFTP connection and allows the
 *  application layer to perform any necessary initialisation tasks before it
 *  is ready to receive data.
 *
 *  On return, the application should expect to receive multiple calls to
 *  tftp_app_process_data_block(), followed by a final call to
 *  tftp_app_transfer_complete(), to signal that the last block of data has
 *  been received and the TFTP transfer is complete.
 *
 *  \return   0 on success, non-zero if the application is not ready to
 *        receive a new connection or an error occurred.
 **/
int tftp_app_transfer_begin(void);

/** This function is called from TFTP every time a valid TFTP DATA packet is received
 *  via the protocol. TFTP only passes unique data blocks to the application i.e.
 *  duplicate data received due to problems with the protocol will NOT trigger this
 *  function.
 *
 *  It allows the application to process or store the data as needed.
 *
 *  NOTE: TFTP will not ACK the DATA packet associated with this data block until
 *  this function returns success. Hence, the processing delay associated with
 *  this function should be taken into consideration, to ensure that the source
 *  does not timeout.
 *
 *  \param data     A pointer to the received data (excluding any TFTP headers)
 *  \param num_bytes  The number of bytes in the data array
 *  \return       0 if success, non-zero if the application wishes to
 *            signal a critical error to TFTP that signals a premature
 *            termination of the TFTP transfer and closes the connection.
 *
 **/
int tftp_app_process_data_block(REFERENCE_PARAM(unsigned char, data), int num_bytes);

/** This function is called once the last block of data has been received and
 *  ACKed by TFTP. The application should perform any housekeeping to de-initialise
 *  any process that was initialised in tftp_app_transfer_begin().
 *
 *  On return, the active TFTP connection is closed.
 *
 */
void tftp_app_transfer_complete(void);

/** This function is called when an error in the TFTP protocol has occurred that
 *  will cause premature termination of the active connection.
 *
 *  The application should expect the TFTP connection to close on return from this
 *  function, and hence should perform any housekeeping to safely terminate any
 *  process that was started in tftp_app_transfer_begin(). There will be no further
 *  calls to tftp_app_process_data_block() from TFTP for this connection.
 *
 **/
void tftp_app_transfer_error(void);


#endif /* TFTP_APP_H_ */
