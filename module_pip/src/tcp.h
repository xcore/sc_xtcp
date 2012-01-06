// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/** Function that processes an incoming TCP packet. The TCP header starts
 * at the given offset in the packet. The source and destination IP
 * addresses are passed to enable the checksum computation and for
 * application level software. Normally called from the IP stack.
 *
 * \param packet packetdata.
 *
 * \param offset Index of first element in the array that contains the TCP header.
 *
 * \param srcIP  Source IP address from IP header, in host order.
 *
 * \param dstIP  Destination IP address from IP header (the address of this
 *               host if all is well, or some sort of multicast address), in host order.
 *
 * \param length Number of bytes of IPv4 payload, that is the total length of TCP header
 *               and TCP data.
 */
void pipIncomingTCP(unsigned short packet[], int offset, int srcIP, int dstIP, int length);

/** Function that initialises the TCP stack. To be called once prior to
 * calling any other functions. It sets up the various timers needed to
 * cope with resends. Normally called from the top level of the server.
 */
void pipInitTCP();

/** Function to be called when the Timewait-timer expires. Normally called
 * from the main timer.
 */
void pipTimeoutTimewaitTCP();

/** Function to be called to process an application request to the TCP
 * stack. The streaming channel end connects to the application. THis
 * function is to be called when a word has been input from the channel.
 *
 * \param app streaming channel that connects to the application program. A
 *            two way protocol over this channel implements operations such
 *            as accept, close, read and write.
 *
 * \param cmd the word input from the channel.
 */
void pipApplicationTCP(streaming chanend app, int cmd);


#define PIP_TCP_ACK_CT 3
#define PIP_TCP_ERROR_CT 5
#define PIP_TCP_CLOSED_CT 6


#define PIP_TCP_ACCEPT 1
#define PIP_TCP_CLOSE 2
#define PIP_TCP_READ 3
#define PIP_TCP_WRITE 4
