// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/** Function that an application can call to wait and accept a connection
 * on a TCP stream. The streams are pre-allocated and numbered.
 *
 * \param stack      Channel that connects to the TCP server.
 *
 * \param connection Number of the connection to accept on.
 *
 */
void pipApplicationAccept(streaming chanend stack, int connection);

/** Function that an application can call close a TCP stream. The streams
 * are pre-allocated and numbered.
 *
 * \param stack      Channel that connects to the TCP server.
 *
 * \param connection Number of the connection to close.
 *
 */
void pipApplicationClose(streaming chanend stack, int connection);



/** Function that an application can call to read from a TCP stream. The streams
 * are pre-allocated and numbered. The function blocks if there is no data available,
 * otherwise up to the maximum number of bytes are returned.
 *
 * \param stack      Channel that connects to the TCP server.
 *
 * \param connection Number of the connection to read from.
 *
 * \param buffer     Place to store the received data.
 *
 * \param maxBytes   Maximum number of bytes to read.
 *
 */
int pipApplicationRead(streaming chanend stack, int connection,
                       unsigned char buffer[], int maxBytes);

/** Function that an application can call to write to a TCP stream. The
 * streams are pre-allocated and numbered. The function blocks until either
 * all data is submitted for transmission, or until the link is shutdown.
 *
 * \param stack      Channel that connects to the TCP server.
 *
 * \param connection Number of the connection to write to.
 *
 * \param buffer     Array that holds the data to be transmitted.
 *
 * \param nBytes     Number of bytes to write.
 *
 * \returns The number of bytes transmitted. Normally nBytes, unless the
 * TCP stream was prematurely closed.
 */
int pipApplicationWrite(streaming chanend stack, int connection,
                       unsigned char buffer[], int nBytes);
