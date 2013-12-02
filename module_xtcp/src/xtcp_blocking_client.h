// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef _xtcp_blocking_client_h_
#define _xtcp_blocking_client_h_

/** \file xtcp_blocking_client.h
 *  \brief Contains the write and read functions for a blocking TCP client
 */

/** \brief block until the xtcp interface has come up
 *
 *  This means, amongst other things, that it has acquired an
 *  IP address using whatever scheme was necessary
 */
void xtcp_wait_for_ifup(chanend tcp_svr);

/** \brief Block until a connection attempt to is made
 */
xtcp_connection_t xtcp_wait_for_connection(chanend tcp_svr);

//!@{
//! \name Blocking client API

/** \brief Write a buffer of data to a TCP connection
 *
 *  This is a blocking write of data to the given xtcp connection
 *
 *  \return			1 for success, 0 for failure
 *  \param tcp_svr	The xtcp control channel
 *  \param conn		The xtcp server connection structure
 *  \param buf		The buffer to write
 *  \param len		The length of data to send
 */
int xtcp_write(chanend tcp_svr,
               REFERENCE_PARAM(xtcp_connection_t, conn),
               unsigned char buf[],
               int len);

/** \brief Receive data from xtcp connection
 *
 *  This is a blocking read from the xtcp stack
 *
 *  \return			The number of bytes received
 *  \param tcp_svr	The xtcp control channel
 *  \param conn		The xtcp server connection structure
 *  \param buf		The buffer to read into
 *  \param minlen	The minimim length of data to receive
 */
int xtcp_read(chanend tcp_svr,
              REFERENCE_PARAM(xtcp_connection_t, conn),
              unsigned char buf[],
              int minlen);

//!@}

#endif
