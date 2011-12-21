// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include "tcp.h"

// RFC 793

struct tcpConnection {
    short srcPort, dstPort;
    int srcIP, dstIP;
    short opened;
    short maxSegmentSize;
};

struct tcpConnection incomingConnections[] = {
    {0, 80, 0, 0, 0},
};

void pipIncomingTCP(unsigned short packet[], int offset) {
    int dataOffset = packet[offset + 6] & 0xF;
    int tcpOffset = offset + dataOffset * 2;
    
    // Check TCP header.

    // Compare source port, dest port, and source IP against table.

    // Set data ready for appropriate entry.
}
