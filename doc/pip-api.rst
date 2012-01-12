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
  over DHCP. Without this option, no IP address is assigned.


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
