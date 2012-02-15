PIP user guide
==============

Compile time defines
--------------------

Compile time defines are used to include support for all optional parts of
the networking stack. The compile time defines can be placed in a file
called ``pip_conf.h``, or they can be set with -DXXXX on the xcc command line.

Level 2:
''''''''

*PIP_IPV4*
  Define this to include IPv4.

*PIP_ARP*
  Define this to include ARP.

*PIP_GATEWAY*
  Define this to support gateway functionality. A single gateway is
  accepted from DHCP and used to route IP traffic to that is not on our
  subnet.

Level 3:
''''''''

*PIP_TCP*
  Define this to a value N to include the TCP stack with N simultaneous
  connections that do not support a CONNECT system call.
  This is suitable for server applications.

*PIP_TCP_CONNECT*
  Define this to a value N to include the TCP stack with both an
  ACCEPT and a CONNECT system call with at most N simultaneous TCP
  connections. Only required for applications that
  need to connect to a server.

*PIP_UDP*
  Define this to include the UDP stack.

*PIP_ICMP*
  Define this to include code that repsonds to ICMP-Echo (also
  known as PING) requests.

Level 4:
''''''''

*PIP_TFTP* Define this to include the TFTP boot client. This will cause
  the network stack to request a file ``/x`` from the server if a
  150-option ("IP address of TFTP server") is present, load the contents of
  this file at address 0x10000, and jump to it. [Note: this last bit is yet
  to be implemented].

*PIP_DHCP*
  Define this to include the DHCP client. This will request an IP address
  over DHCP. See also *PIP_LINK_LOCAL* below.

*PIP_DHCP_DONT_WAIT*
  Define this to not wait for a random period of up to 10 seconds before
  asking for an IP address. Do not enable this if you have multiple DHCP
  clients that are all switched on at the same time. Do enable this if you
  are developing a single client and are bored of waiting.

*PIP_LINK_LOCAL*
  Define this to include link local address assignment. This will try and obtain
  an IP address in the 169.254.x.x range. See also *PIP_DHCP* above.

Fine tuning
'''''''''''

*PIP_TCP_BUFFER_RX*
  Define this to the size of the receive buffer in the TCP stack - must be
  a power of 2. Must be at least 512; smaller receive buffers may confuse
  the remote TCP/IP stack. If not set, a default value of 512 will be used.

*PIP_TCP_BUFFER_TX*
  Define this to the size of the transmit buffer in the TCP stack - must be
  a power of 2. If not set, a default value of 512 will be used.

Debugging
'''''''''

*PIP_DEBUG_TCP*
  Define this to an integral number include a debugging
  facility that keeps a history of the header information of the last
  *PIP_DEBUG_TCP* packets. The information is printed off on a PING.

*PIP_DEBUG_TIMER*
  Define this to print off all timer settings on a PING.


If both *PIP_DHCP* and *PIP_LINK_LOCAL* are defined then LINK_LOCAL will be
switched on shortly after DHCP is switched on to give DHCP a chance to
obtain an address. If neither *PIP_DHCP* nor *PIP_LINK_LOCAL* are defined
then no IP address is assigned, and pipAssignIPv4 shall be called to set an
IP address.


API
---

TCP
'''

.. doxygenfunction:: pipApplicationAccept

.. doxygenfunction:: pipApplicationConnect

.. doxygenfunction:: pipApplicationClose

.. doxygenfunction:: pipApplicationRead

.. doxygenfunction:: pipApplicationWrite

UDP
'''

To be provided.
