// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/* Experimental features.
   The features in this file are **experimental**,
   not supported and not known to work if enabled. Any guarantees of
   the robustness of the component made by XMOS do not hold if these features
   are used.
*/
#ifndef __xtcp_buffered_client_h__
#define __xtcp_buffered_client_h__
#include "xtcp_client.h"
#include "xtcp_bufinfo.h"

/** \file xtcp_buffered_client.h
 *  \brief Contains the API for buffered reading an writing from xtcp connections
 */

// The buffered API is for C only
#if !defined(__XC__) || defined(__DOXYGEN__)

//!@{
//!@ \name Configuration functions

/** \brief set the location and size of the receiver buffer
 *
 *  \param tcp_svr   	the xtcp server control channel
 *  \param conn      	a pointer to the xtcp connection info structure
 *  \param bufinfo   	a pointer to the buffered API control structure
 *  \param buf       	a pointer to the buffer to use for received data
 *  \param buflen    	the length of the receive buffer in bytes
 */
void xtcp_buffered_set_rx_buffer(chanend tcp_svr,
                                 xtcp_connection_t *conn,
                                 xtcp_bufinfo_t *bufinfo,
                                 char *buf,
                                 int buflen);

/** \brief set the location and size of the transmission buffer
 *
 *  \note the size of the buffer should probably be no smaller than
 *        XTCP_CLIENT_BUF_SIZE plus the maximum buffered message length. if
 *        it is, then buffer overflow can be detected and data will be lost.
 *
 *  \param tcp_svr   	the xtcp server control channel
 *  \param conn      	a pointer to the xtcp connection info structure
 *  \param bufinfo   	a pointer to the buffered API control structure
 *  \param buf       	a pointer to the buffer to use for received data
 *  \param buflen    	the length of the receive buffer in bytes
 *  \param lowmark		if the number of spare bytes in the buffer falls below this, TCP pauses the stream
 */
void xtcp_buffered_set_tx_buffer(chanend tcp_svr,
                                 xtcp_connection_t *conn,
                                 xtcp_bufinfo_t *bufinfo,
                                 char *buf,
                                 int buflen,
                                 int lowmark);

//!@}

//!@{
//!@ \name Receive functions

/** \brief Pull a buffer of data out of the received data buffer
 *
 *  This pulls a specified length of data from the data buffer.  It is most useful
 *  for protocols where the packet format is known, or at least where variable
 *  sized data blocks are preceeded by a length field.  A good example is DHCP.
 *
 *  When calling this in response to a XTCP_RECV_DATA event, and you must keep
 *  calling it until it returns zero.
 *
 *  The return value is either:
 *  - the requested size, when the data read could be fullfilled.
 *  - zero, when there is not enough data to fullfill the read.
 *  - another value, when there is not enough data to fullfill the read and when
 *    there is not enough space left in the buffer to read another packet into the
 *    buffer to fullfill the read in the future. the overflow flag will be set.
 *
 *  when the user wants to pull N bytes from the buffer, but less than N have
 *  been received into it, then the function returns zero.  In this case, a
 *  calling function would typically not process further until another receive
 *  event was detected, indicating that there is some more data available in
 *  to read, and therefore that the number of bytes requested can now be
 *  fullfilled.
 *
 *
 *  \return				the number of characters received in the buffer, or
 *                      zero if we have used up all of the data, or
 *                      the space available when receiving more data from xtcp would overflow the buffer
 *  \param tcp_svr   	the xtcp server control channel
 *  \param conn      	a pointer to the xtcp connection info structure
 *  \param bufinfo   	a pointer to the buffered API control structure
 *  \param buf			on return this points to the received data.
 *  \param len			length of the buffer to receive into
 *  \param overflow		pointer to an int which is set to non-zero if the buffer overflowed
 *
 *  \note consider the data pointed to by the buf parameter to be read only.  It points into
 *        the allocated buffer
 */
int xtcp_buffered_recv(chanend tcp_svr,
                       xtcp_connection_t *conn,
                       xtcp_bufinfo_t *bufinfo,
                       char **buf,
                       int len,
                       int *overflow);

/** \brief Receive data from the receive buffer, up to a given delimiter character
 *
 *  Many protocols, eg SMTP, FTP, HTTP, have variable length records with delimiters
 *  at the end of the record.  This function can be used to fetch data from that
 *  type of data stream.
 *
 *  When calling this in response to a XTCP_RECV_DATA event, and you must keep
 *  calling it until it returns zero.
 *
 *  The returned length contains the delimiter
 *
 *	\return				the number of characters in the returned data (including delimiter), or
 *						zero when there is nothing to receive, or
 *						the space available when receiving more data from xtcp would overflow the buffer
 *  \param tcp_svr   	the xtcp server control channel
 *  \param conn      	a pointer to the xtcp connection info structure
 *  \param bufinfo   	a pointer to the buffered API control structure
 *  \param buf			on return this points to the received data.
 *  \param delim		a character to receive data until
 *  \param overflow		pointer to an int which is set to non-zero if the buffer overflowed
 *
 */
int xtcp_buffered_recv_upto(chanend tcp_svr,
                            xtcp_connection_t *conn,
                            xtcp_bufinfo_t *bufinfo,
                            char **buf,
                            char delim,
                            int *overflow);

//!@}


//!@{
//!@ \name Transmit functions


/** \brief Add more data to the send buffer
 *
 *  \return				1 if the data was able to be buffered for send, 0 otherwise
 *  \param tcp_svr   	the xtcp server control channel
 *  \param conn      	a pointer to the xtcp connection info structure
 *  \param bufinfo   	a pointer to the buffered API control structure
 *  \param buf			a buffer of data to queue for sending
 *  \param len			the length of the data in the buffer
 *
 */
int xtcp_buffered_send(chanend tcp_svr,
                       xtcp_connection_t *conn,
                       xtcp_bufinfo_t *bufinfo,
                       char *buf,
                       int len);

/** \brief The handler function for transmission requests from the xtcp stack
 *
 *  When one of the following event types is received from the xtcp server channel
 *  then this method should be called.
 *
 *    XTCP_SENT_DATA
 *    XTCP_REQUEST_DATA
 *    XTCP_RESEND_DATA
 *
 *  \param tcp_svr   	the xtcp server control channel
 *  \param conn      	a pointer to the xtcp connection info structure
 *  \param bufinfo   	a pointer to the buffered API control structure
 */
void xtcp_buffered_send_handler(chanend tcp_svr,
                                xtcp_connection_t *conn,
                                xtcp_bufinfo_t *bufinfo);

/** \brief Get the remaining amount of space in the send buffer
 *
 *  A client can use this to determine whether the outgoing buffer has enough
 *  space to accept more data before the call to send that data is made.
 *
 *  \return				the number of bytes remaining in the send buffer
 *  \param bufinfo   	a pointer to the buffered API control structure
 */
int xtcp_buffered_send_buffer_remaining(xtcp_bufinfo_t *bufinfo);


//!@}

#endif

#endif //x __xtcp_buffered_client_h__
