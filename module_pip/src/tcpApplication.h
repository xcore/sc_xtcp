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
