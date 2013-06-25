PIP: Pico-IP
============

Pico IP, hereafter called PIP, is a TCP/IP stack designed for XMOS XCore
devices. It is designed to have a small memory foot-print, to be easily
readable, to be modular in that parts of PIP can be compiled to, for
example, only implement UDP, and to not use dynamic memory. In addition, it
was written from scratch under the XCore license.

PIP is not implemented for performance; although it can be adapted to
execute faster at the expense of a larger memory foot-print and reduced
readability.

This documentation covers the programmer's API and the developers guide;
before that, we discuss the features and limitations of PIP, and the
general design philosophy of PIP that is important for both users and
developers.

Features and limitations
------------------------

PIP implements the following:

* ARP

* ICMP echo/reply

* IPv4 with single gateway

* UDP (UDP can be used embedded in the stack or from a separate thread)

* TCP (TCP must be used from a separate thread)

* DHCP and link-local (169.254.x.x)

* Boot over TFTP

PIP has the following restrictions:

* No support for IP fragments

* No support for IP, TCP or UDP options

Known limitations that are yet to be implemented:

* No DNS or MDNS

* TFTP has yet to implement the actual jump to 0x10000; to be taken from
  the USB bootloader.

Known issues:

* DHCP packets may appear to originate from 169.254.x.x addresses rather
  than 0.0.0.0

* At present the stack uses streaming channels which is not good for multi
  core implementations; to be replaced with ordinary channels and
  transactions.

* Reset is currently part of the toplevel - this should be taken out and
  moved to outside of PIP.

* No SMI line management at present. Arguably it should support the notion
  of the interface being up or down.

Known tests that are to be performed:

* Corner cases in TCP

* Gateway

* TFTP boot

* ARP timeouts

PIP design philosophy
---------------------

PIP is designed to not use dynamic memory, and to be small and
readable. Its interface is therefore different from uip + xtcp.

TCP interface
'''''''''''''

The TCP interface is designed to be user-friendly. It assumes that
applications that use TCP run in a separate thread, and use one of the
``accept()``, ``connect()``, ``read()``, ``write()``, and ``close()``
calls. Note that there is no ``socket()`` call to dynamically create a
socket, and instead all sockets are pre-allocated at compile-time. The
number of sockets is not limited other than by the amount of memory taken
up by buffers.

UDP interface
'''''''''''''

There are two interfaces for UDP.

The UDP interface is designed for UDP sockets to be completely embedded in
this thread. It is assumed that no scheduling is required in those apps;
for example, DHCP or TFTP simply reacts to UDP packets by transmitting
another UDP packet.

An alternative interface exposes UDP over a channel.

Configuration
'''''''''''''

Most parameters are built-time parameters, to enable the designer to slice
out the bit of the stack that they need. The build options are defined in
the API below.

.. toctree::

   pip-api
   pip-developer
