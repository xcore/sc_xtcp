// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/** Function that initialises the DHCP client. To be called prior to any
 * other function. Usually called from toplevel.
 */
void pipInitDHCP();

/** Function that processes an incoming DHCP packet. May trigger an
 * outgoing packet. Normally called from IPv4 layer.
 *
 * \param packet packet data.
 *
 * \param srcIP  source IP address according to IP header.
 *
 * \param dstIP  destination IP address according to IP header.
 *
 * \param offset index in packet array where the first byte of the
 *               DHCP packet resides.
 *
 * \param length length of the total packet in bytes.
 */
void pipIncomingDHCP(unsigned short packet[], unsigned srcIP, unsigned dstIP, int offset, int length);

/** Function that creates a DHCP message.
 * 
 * \param firstMessage flag to indicate that this is the first DHCP message
 *                     (DHCP-DISCOVER), set to 0 to indicate that it is a
 *                     DHCP-REQUEST.
 * 
 * \param proposedIP   The suggested IP address - set to 0 if no IP address
 *                     is suggested.
 *
 * \param serverIP     The server IP address - set to 0 if not known.
 */
void pipCreateDHCP(int firstMessage,
                   unsigned proposedIP,
                   unsigned serverIP);

/** Function that is called to signal a timeout on DHCP timer T2. Called
 * from timer.xc, set by init and incoming.
 */
void pipTimeOutDHCPT1();

/** Function that is called to signal a timeout on DHCP timer T2. Called
 * from timer.xc, set by init and incoming.
 */
void pipTimeOutDHCPT2();
