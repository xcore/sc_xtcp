PIP user guide
==============

Compile time defines
--------------------

Compile time defines are used to include support for all optional parts of
the networking stack. Note that there is no automatic inclusion, eg, if
DHCP support is required one has to include DHCP, UDP, IPv4 and ARP.

Level 2:
''''''''

*-DPIP_IPv4*
  Define this to include IPv4.

*-DPIP_ARP*
  Define this to include ARP.

*-DPIP_GATEWAY*
  Define this to support gateway functionality. A single gateway is
  accepted from DHCP and used to route IP traffic to that is not on our
  subnet.

Level 3:
''''''''

*-DPIP_TCP*
  Define this to include the TCP stack.

*-DPIP_UDP*
  Define this to include the UDP stack.

*-DPIP_ICMP*
  Define this to include code that repsonds to ICMP-Echo (also
  known as PING) requests.

Level 4:
''''''''

*-DPIP_TFTP* Define this to include the TFTP boot client. This will cause
  the network stack to request a file ``/x`` from the server if a
  150-option ("IP address of TFTP server") is present, load the contents of
  this file at address 0x10000, and jump to it. [Note: this last bit is yet
  to be implemented].

*-DPIP_DHCP*
  Define this to include the DHCP client. This will request an IP address
  over DHCP. See also *PIP_LINK_LOCAL* below.

*-DPIP_LINK_LOCAL*
  Define this to include link local address assignment. This will try and obtain
  an IP address in the 169.254.x.x range. See also *PIP_DHCP* above.


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

.. doxygenfunction:: pipApplicationClose

.. doxygenfunction:: pipApplicationRead

.. doxygenfunction:: pipApplicationWrite

UDP
'''

To be provided.
