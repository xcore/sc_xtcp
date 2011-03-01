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

