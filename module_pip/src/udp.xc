// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include <xclib.h>
#include <print.h>
#include "config.h"
#include "udp.h"
#include "udpApplication.h"
#include "checksum.h"
#include "ipv4.h"
#include "dhcp.h"
#include "tftp.h"
#include "tx.h"

// RFC 0768

struct {
    unsigned portNr;
    unsigned channel;
} appUDP[PIP_UDP_CHANNELS];

int nAppUDP = 0;

static void serveUDPChannels(unsigned short packet[],
                             unsigned remoteIP, unsigned localIP,
                             unsigned remotePort, unsigned localPort,
                             unsigned offset, unsigned length) {
    for(int i = 0; i < nAppUDP; i++) {
        if (localPort == appUDP[i].portNr) {
            int maxBytes;
            asm("outct res[%0],3"::"r"(appUDP[i].channel));
            asm("in %0,res[%1]":"=r"(maxBytes):"r"(appUDP[i].channel));
            if (length > maxBytes) {
                length = maxBytes;
            }
            asm("out res[%0],%1"::"r"(appUDP[i].channel),"r"(length));
            asm("out res[%0],%1"::"r"(appUDP[i].channel),"r"(remoteIP));
            asm("out res[%0],%1"::"r"(appUDP[i].channel),"r"(remotePort));
            for(int j = 0; j < length; j++) {
                asm("outt res[%0],%1"::"r"(appUDP[i].channel),"r"((packet, unsigned char[])[offset*2+j]));
            }
            nAppUDP--;
            for(int j = i; j < nAppUDP; j++) {
                appUDP[j] = appUDP[j+1];
            }
            return;
        }
    }
}

void pipIncomingUDP(unsigned short packet[], unsigned offset, unsigned remoteIP, unsigned localIP) {
    int remotePort =  byterev(packet[offset+0]) >> 16;
    int localPort =   byterev(packet[offset+1]) >> 16;
    int totalLength = byterev(packet[offset+2]) >> 16;
    int chkSum;
    
    chkSum = onesChecksum(0x0011 + totalLength + onesAdd(remoteIP, localIP),
                          packet, offset, totalLength);
    if (chkSum != 0) {
        return; /* ignore packet with bad chksum */
    }
    
    // Check destination port, set packet ready for appropriate packet handler.

#if defined(PIP_DHCP)
    if (localPort == 0x0044) {
        pipIncomingDHCP(packet, remoteIP, localIP, offset + 4, totalLength - 8);
        return;
    }
#endif

#if defined(PIP_TFTP)
    if (localPort == pipPortTFTP) {
        pipIncomingTFTP(packet, remoteIP, localIP, remotePort, offset + 4, totalLength - 8);
        return;
    }
#endif

    // Finally check for a user process.

#if PIP_UDP_CHANNELS != 0
    serveUDPChannels(packet, remoteIP, localIP, remotePort, localPort, offset + 4, totalLength - 8);
#endif
}

void pipOutgoingUDP(unsigned remoteIP, unsigned localPort, unsigned remotePort, unsigned length) {
    int totalLength = length + 8;
    int chkSum;
    txShortRev(17, localPort);               // Store source port
    txShortRev(18, remotePort);               // Store source port
    txShortRev(19, totalLength);           // Total length, including header.
    txShort(20, 0);                        // Total length, including header.
    chkSum = onesChecksum(0x0011 + totalLength + onesAdd(myIP, remoteIP), (txbuf, unsigned short[]), 17, totalLength);
    txShort(20, chkSum);                        // Total length, including header.
    pipOutgoingIPv4(PIP_IPTYPE_UDP, remoteIP, totalLength);
}



static void doReadUDP(streaming chanend app, int remotePort) {
    appUDP[nAppUDP].portNr = remotePort;
    asm("add %0,%1,0":"=r"(appUDP[nAppUDP].channel):"r"(app));
    nAppUDP++;
}

static void doWriteUDP(streaming chanend app, int localPort) {
    unsigned remoteIP, remotePort, maxBytes;
    unsigned char c;
    int zero = 42;
    app :> remoteIP;
    app :> remotePort;
    app :> maxBytes;
    for(int i = 0; i < maxBytes; i++) {
        app :> c;
        txByte(zero + i, c);
    }
    pipOutgoingUDP(remoteIP, localPort, remotePort, maxBytes);
}

// Application interface to UDP, comes in two parts
// 1) implemented on the Stack side. (1 function)
// 2) implemented on the Application side. (series of functions)

void pipApplicationUDP(streaming chanend app, int cmd, chanend cOut) {
    int localPort;
    app :> localPort;
    switch(cmd) {
    case PIP_UDP_READ:
        doReadUDP(app, localPort);
        break;
    case PIP_UDP_WRITE:
        doWriteUDP(app, localPort);
        printstr("Writing...\n");
        doTx(cOut);
        break;
    }
}

int pipApplicationReadUDP(streaming chanend stack,
                          unsigned char buffer[], unsigned start,
                          unsigned maxBytes,
                          unsigned int &remoteIP, unsigned int &remotePort,
                          unsigned int localPort) {
    int actualBytes;
    stack <: PIP_UDP_READ;
    stack <: localPort;
    schkct(stack, 3);
    stack <: maxBytes;
    stack :> actualBytes;
    stack :> remoteIP;
    stack :> remotePort;
    for(int i = 0; i < actualBytes; i++) {
        stack :> buffer[start + i];
    }
    return actualBytes;
}

void pipApplicationWriteUDP(streaming chanend stack,
                            unsigned char buffer[], unsigned start, 
                            unsigned maxBytes,
                            unsigned int remoteIP, unsigned int remotePort,
                            unsigned int localPort) {
    stack <: PIP_UDP_WRITE;
    stack <: localPort;
    stack <: remoteIP;
    stack <: remotePort;
    stack <: maxBytes;
    for(int i = 0; i < maxBytes; i++) {
        stack <: buffer[start + i];
    }
}

