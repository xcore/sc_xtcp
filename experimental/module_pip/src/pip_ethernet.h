// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/** The 16-bit value that represents the ethernet type for an IPV4 packet,
 * in reverse byte order.
 */
#define PIP_ETHTYPE_IPV4_REV 0x0008

/** The 16-bit value that represents the ethernet type for an ARP packet,
 * in reverse byte order.
 */
#define PIP_ETHTYPE_ARP_REV  0x0608

/** Function that processes an incoming Ethernet packet.
 *
 * \param packet packetdata.
 */
void pipIncomingEthernet(unsigned short packet[]);
void pipIncomingEthernetC(int a);
 


/** Function that completes the outgoing packet ready to go onto the Ethernet cable.
 * It sets the source and destination mac addresses, and the type of the packet.
 *
 * \param macDest   array containing the destination mac address.
 *
 * \param offset    the index in the array where the mac address resides.
 *
 * \param revType   the type of the packet, in reverse byte order. Use one of
 *                  the predefined types above.
 */
void pipOutgoingEthernet(unsigned char macDest[], int offset, int revType);


extern unsigned char myMacAddress[6];
