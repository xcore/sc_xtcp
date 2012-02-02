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

* IPv4

* UDP

* TCP

* DHCP

* TFTP

PIP has the following restrictions:

* No support for IP fragments

* No support for IP, TCP or UDP options

Known limitations that are yet to be implemented:

* IPv4 routing to the gateway for traffic that is not destined for the
  current subnet

* Timeouts and resends on TCP-SYN and -FIN packets

* DHCP gateway option

* tcp ``open()`` function, and accompanying SYNSENT state (at present only
  servers can be implemented using ``accept()``)

* No fallback to a private network (using a 169.254.x.x address)

* No DNS or MDNS

* TFTP has yet to implement the actual jump to 0x10000; to be taken from
  the USB bootloader.

PIP design philosophy
---------------------

Since PIP is designed to not use dynamic memory, and to be small and
readable, its interface is different.

TCP interface
'''''''''''''

The TCP interface is designed to be user-friendly. It assumes that
applications that use TCP run in a separate thread, and use one of the
``accept()``, ``read()``, ``write()``, and ``close()`` calls. Note that
there is no ``socket()`` call to dynamically create a socket, and instead
that all sockets are pre-allocated at compile-time. The number of sockets
is not limited other than byt he amount of memory taken up by buffers.

UDP interface
'''''''''''''

The UDP interface is designed for UDP sockets to be completely embedded in
this thread. It is assumed that no scheduling is required in those apps;
for example, DHCP or TFTP simply reacts to UDP packets by transmitting
another UDP packet.



.. toctree::

   pip-api
   pip-developer
