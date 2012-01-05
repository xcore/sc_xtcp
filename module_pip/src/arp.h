// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#define ARPENTRIES 10

struct arp {
    int ipAddress;
    unsigned char macAddr[8];     // last two bytes are flags
};

extern struct arp pipArpTable[ARPENTRIES];


/** Function that deals with an incoming ARP packet.
 *
 * \param packet contains the packet data.
 * 
 * \param offset points to the ARP header in the packet.
 */

void pipIncomingARP(unsigned short packet[], int offset);



void pipArpTimeOut();



void pipCreateARP(int reply, int tpa, unsigned char tha[], int offset);


void pipARPStoreEntry(int ipAddress, unsigned char macAddress[], int offset);
