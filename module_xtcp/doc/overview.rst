Overview
========

The XMOS TCP/IP component provides a IP/UDP/TCP stack that connects to
the XMOS ethernet component. It enables several
clients to connect to it and send and receive on multiple TCP or UDP
connections. The stack has been designed for a low memory 
embedded programming environment and despite its low memory footprint
provides a complete stack including ARP, IPv4, IPv6, UDP, TCP, DHCP, IPv4LL,
ICMP and IGMP protocols.

.. note:: Please contact XMOS support for IPv6.

The stack is based on the open-source stack uIP with modifications to
work efficiently on the XMOS architecture and communicate between tasks
using xC channels.

The TCP stack can either interface to a separate ethernet MAC or work
with an integrated MAC taking only 2 logical cores.

Seperate MAC + TCP/IP stack properties
+++++++++++++++++++++++++++++++++++++++++++++++++++++++

  * Layer 2 packets can be sent and received independently of layer 3
  * Integrated support for high priority Qtagged packets
  * Integrated support for 802.1 Qav rate control
  * Packet filtering in an independent logical core
  * Works on a 400 MHz part

Two core ethernet plus integrated TCP/IP stack properties
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  * Uses only 2 logical cores
  * High throughput
  * Uses lower memory footprint
  * Only TCP/IP sourced packets can be transmitted
  * 500 MHz parts only (MII core requires 62.5 MIPS)

Component Summary
+++++++++++++++++

 +-------------------------------------------------------------------+
 |                        **Functionality**                          |
 +-------------------------------------------------------------------+
 |  Provides a lightweight IP/UDP/TCP stack                          |
 +-------------------------------------------------------------------+
 |                       **Supported Standards**                     |
 +-------------------------------------------------------------------+
 | IPv4, IPv6, UDP, TCP, DHCP, IPv4LL, ICMP, IGMP                    |
 +-------------------------------------------------------------------+
 |                       **Supported Devices**                       |
 +------------------------------+------------------------------------+
 | | XMOS Devices               | | XS1-G4                           | 
 |                              | | XS1-L2                           |
 |                              | | XS1-L1                           |
 +------------------------------+------------------------------------+
 |                       **Requirements**                            |
 +------------------------------+------------------------------------+
 | XMOS Desktop Tools           | v12.0 or later                     |
 +------------------------------+------------------------------------+
 | XMOS Ethernet Component      | 2.2.0 or later                     |
 +------------------------------+------------------------------------+

