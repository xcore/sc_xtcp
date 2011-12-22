// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

// RFC 826

void pipIncomingARP(unsigned short packet[], int offset) {
    int oper = packet[offset + 3];
    int tha; // TODO.
    int localAddress; // TODO.

    if (oper == 1) {             // REQUEST
        if (tha == localAddress) { // for us.
            // store sha and spa.
            // reply with our IP address and MAC address.
        }
    } else if (oper == 2) {      // REPLY
        // search for spa in our ARP db, if found, pair up with sha
    }
}
