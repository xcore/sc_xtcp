// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/** Function that initialises the Link Local client. To be called prior to
 * any other function. Usually called from dhcp or toplevel (depending on
 * whether DHCP is enabled)
 */
void pipInitLinkLocal();

/** Function that is called to signal a timeout on the Link Local timer. Called
 * from timer.xc, set by init and incoming.
 */
void pipTimeOutLinkLocal();

/** Function that is called to determine whether an incoming ARP packet is
 * for the link local protocol. This function returns 1 if it is, and
 * should act on it, and 0 if it is not.
 *
 * \param oper either 256 (request) or 512 (reply)
 *
 * \param ipAddress the address in the ARP packet.
 *
 * \param macAddress the mac address in the ARP packet
 *
 * \param offset the index in the mac address array where the mac address is stored
 *
 * \returns a boolean stating that this ARP packet was for the link local layer.
 */
int pipIncomingLinkLocalARP(int oper, int ipAddress, unsigned char macAddress, int offset);

/** Function that disables the Link Local client.
 */
void pipDisableLinkLocal();

