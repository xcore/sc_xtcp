// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/** Function that is called to start both the TCP/UDP/IP stack and the
 * two-thread Ethernet stack. This function requires two threads: one for
 * MII handling, and the other one for TCP/UDP/IP.
 *
 * \param clk_smi  clock block for the SMI port
 *
 * \param p_mii_resetn  port for resetting
 *
 * \param smi description of SMI ports
 *
 * \param mii description of MII ports
 *
 * \param tcpApps array of channels to each TCP application. This array
 * must have PIP_TCP_CHANNELS entries.
 *
 * \param udpApps array of channels to each UDP application. This array
 * must have PIP_UDP_CHANNELS entries.
 */
void pipServer(clock clk_smi,
               out port ?p_mii_resetn,
               smi_interface_t &smi,
               mii_interface_t &mii,
               streaming chanend tcpApps[],
               streaming chanend udpApps[]
    );

extern unsigned char myMacAddress[6];
