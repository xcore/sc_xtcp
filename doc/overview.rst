Overview
========

The XMOS TCP/IP component provides a IP/UDP/TCP stack that connects to
the XMOS ethernet component. It enables several
clients to connect to it and send and receive on multiple TCP or UDP
connections. The stack has been designed for a low memory 
embedded programming environment and despite its low memory footprint
provides a complete stack including ARP, IP, UDP, TCP, DHCP, IPv4LL,
ICMP and IGMP protocols.

The stack is based on the open-source stack uIP with modifications to
work efficiently on XMOS architecture and communicate between threads
using XC channels.

The TCP stack interfaces to the 5-thread XMOS ethernet stack via a pair
of channels.  Alternatively, an integrated 2-thread Ethernet plus TCP/IP
is available for use in resource limited applications.

5 thread Ethernet plus separate TCP/IP stack properties
+++++++++++++++++++++++++++++++++++++++++++++++++++++++

  * Layer 2 packets can be sent and received independently of layer 3
  * Integrated support for high priority Qtagged packets
  * Integrated support for 802.1 Qav rate control
  * Packet filtering in an independent threads
  * Works on a 400 MHz part

Two thread ethernet plus integrated TCP/IP stack properties
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  * Uses only 2 threads
  * High throughput
  * Uses lower memory footprint
  * Only TCP/IP sourced packets can be transmitted
  * 500 MHz parts only (MII thread requires 62.5 MIPS)

Component Summary
+++++++++++++++++

 +-------------------------------------------------------------------+
 |                        **Functionality**                          |
 +-------------------------------------------------------------------+
 |  Provides a lightweight IP/UDP/TCP stack                          |
 +-------------------------------------------------------------------+
 |                       **Supported Standards**                     |
 +-------------------------------------------------------------------+
 | IP, UDP, TCP, DHCP, IPv4LL, ICMP, IGMP                            |
 +-------------------------------------------------------------------+
 |                       **Supported Devices**                       |
 +------------------------------+------------------------------------+
 | | XMOS Devices               | | XS1-G4                           | 
 |                              | | XS1-L2                           |
 |                              | | XS1-L1                           |
 +------------------------------+------------------------------------+
 |                       **Requirements**                            |
 +------------------------------+------------------------------------+
 | XMOS Desktop Tools           | v10.4 or later                     |
 +------------------------------+------------------------------------+
 | XMOS Ethernet Component      | 2v0                                |
 +------------------------------+------------------------------------+
 |                       **Licensing and Support**                   |
 +-------------------------------------------------------------------+
 | Component code provided without charge from XMOS.                 |
 | Component code is maintained by XMOS.                             |
 +-------------------------------------------------------------------+

Resource requirements
=====================

The resource requirements for the XTCP stack alone are:

+--------------------------------------------------+
| Resource     | Usage                             |
+==============+===================================+
| Channels     | 1 per client                      |
+--------------+-----------------------------------+
| Memory       | Between 20 and 45 KBytes          |
+--------------+-----------------------------------+
| Timers       | 2                                 |
+--------------+-----------------------------------+
| Clocks       | 0                                 |
+--------------------------------------------------+

When used with the single thread ethernet MII module,
the combined usage is:

+--------------------------------------------------+
| Resource     | Usage                             |
+==============+===================================+
| Channels     | 3 plus 1 per client               |
+--------------+-----------------------------------+
| Memory       | Between 26 and 50 KBytes          |
+--------------+-----------------------------------+
| Timers       | 4                                 |
+--------------+-----------------------------------+
| Clocks       | 1                                 |
+--------------------------------------------------+

The memory usage depends on the selection of different options
at compile time, and on the amount of buffering chosen.

