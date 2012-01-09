// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#define ARPENTRIES 10

struct arp {
    unsigned int  ipAddress;
    unsigned char macAddr[8];     // last two bytes are flags
};

extern struct arp pipArpTable[ARPENTRIES];


/** Function to innitialise ARP. To be called prior to any of the above
 * functions. Typically called from toplevel.
 */
void pipInitARP();

/** Function that deals with an incoming ARP packet.
 *
 * \param packet contains the packet data.
 * 
 * \param offset points to the ARP header in the packet.
 */
void pipIncomingARP(unsigned short packet[], unsigned offset);

/** Function that is called to signal a timeout on ARP - typically to
 * remove some ARP entries. Called from timer.xc, set by pipInitARP() and
 * pipTimeOutARP().
 */
void pipTimeOutARP();

/** Function that creates an ARP packet.
 * 
 * \param reply      whether to generate a reply or request message.
 *                   Set to 0 for generating an ARP request.
 *
 * \param ipAddress  IP address to which the ARP is targetted
 *
 * \param macAddress Ignored for a request, set to known hardware
 *                   address for a request.
 *
 * \param offset     start index of address inside the macAddress
 *                   array
 */
void pipCreateARP(unsigned reply, unsigned ipAddress, unsigned char macAddress[], unsigned offset);

/** Function that stores an arp entry
 *
 * \param ipAddress  IP address to which the ARP is targetted
 *
 * \param macAddress Ignored for a request, set to known hardware
 *                   address for a request.
 *
 * \param offset     start index of address inside the macAddress
 *                   array
 */
void pipARPStoreEntry(unsigned ipAddress, unsigned char macAddress[], unsigned offset);
