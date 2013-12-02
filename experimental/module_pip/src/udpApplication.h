// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/** Function to be called to read a UDP packet. There is no input buffering
 * on UDP when this interface is used - and when this function is not
 * called UDP traffic on this port will be lost.
 *
 * \param stack channel to the UDP stack
 *
 * \param buffer array to store the packet in
 *
 * \param start  first location in the buffer to write into
 *
 * \param maxBytes maximum number of bytes to write into the buffer
 *
 * \param remoteIP the remote IP address is returned in this argument
 *
 * \param remotePort the remote port number is returned in this argument
 *
 * \param localPort the localPort to listen on.
 */
int pipApplicationReadUDP(streaming chanend stack,
                          unsigned char buffer[], unsigned start,
                          unsigned maxBytes,
                          unsigned int &remoteIP, unsigned int &remotePort,
                          unsigned int localPort);

/** Function to be called to write a UDP packet. Packets always originate
 * from the current IP address.
 *
 * \param stack channel to the UDP stack
 *
 * \param buffer array to read the packet from
 *
 * \param start  first location in the buffer to read from
 *
 * \param maxBytes number of bytes to write into the packet. Note that this parameter must be less than the maximum outgoing packet size minus 42.
 *
 * \param remoteIP the remote IP address to send to
 *
 * \param remotePort the remote port number to send to
 *
 * \param localPort the localPort to supply to the other end
 */
void pipApplicationWriteUDP(streaming chanend stack,
                            unsigned char buffer[], unsigned start,
                            unsigned maxBytes,
                            unsigned int remoteIP, unsigned int remotePort,
                            unsigned int localPort);
