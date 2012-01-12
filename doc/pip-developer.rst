PIP developers guide
====================

Structure
---------

The structure of the PIP stack closely resembles the structure of the
networking stack that it implements. For each component in the stack, there
is a corresponding file that contains the implementation of that
components. The components are as follows:

* toplevel
  * ethernet
    * IPv4
      * UDP
        * DHCP
        * TFTP
      * TCP
      * ICMP
    * ARP
  * timers
  * TCP channels

Timers and TCP channels are not strictly components of the stack, but they
are called from a select statement at the top level.

Each component comprises at least functions ``pipIncomingXXX()`` and
``pipOutgoingXXX()``. The top-level function receives a packet that is then
interpreted through various ``Incoming`` functions until it is either
rejected or results in an ``Outgoing`` packet. Each incoming packet (or
timer, or TCP channel operation) can result in at most one outgoing packet
that is created by calls to the ``tx`` module.

For example, if a DHCPOffer message comes in, then this is first passed to
``pipIncomingEthernet()``, which will inspect the ethernet-type, and pass
it on to ``pipIncomingIPv4()``, which will check the IP header and
destination address, inspect the IP-type and pass it on to
``pipIncomingUDP()``, which will check the UDP checksum and destination
port, and pass it on to ``pipIncomingDHCP()``, which will check the DHCP
message type and deal with it if appropriate. This may seem like an
elaborate process, but it leads to a clean code base.

As a rule, functions receive packets as an array of unsigned shorts, with
an index pointing to the first short that is appropriate for the level.
This enables functions to assume that all data is short-aligned.

The top-level function enables a single packet to be sent on every
iteration. That is, a received packet or time-out can cause a single packet
to be sent. THis can cause trouble, for example if the IP stack does not
have an address ready in the ARP table, in this case we simply drop the IP
packet (somebody will time out if it was important).

Timers
------

All time-outs are handled by the timer module. Timers have a compile-time
allocated index; at present this index is fixed, but in future it could
depend on which modules are present enabling a more efficient timer
implementation. The timer module maintains a list of time-outs that are
scheduled, and will call a function ``pipTimeOutXXX()`` when timer XXX has
expired. It has one interface function to set a time-out, and one to reset
a time-out. Setting a time-out of a timer that is already set will change
the time-out to the new value. Resetting a time-out will remove it from the
timer lists.

Timers are set as a number of seconds and a number of 10ns ticks, with an
optional *fuzz*. The longest time out that can be set is a few decades; the
timers itself will come to a halt 1500 years after start-up, which is
deemed to be sufficient. 

At most one ``TimeOut`` function is called on every iteration of top-level,
enabling each ``TimeOut`` function to send a packet. Typically, a ``TimeOut``
function will also a function to set the timer, implementing a rotating
timeout.

Packet transmission
-------------------





Design style
------------

The number of ``#ifdef`` is kept to a minimum, giving a good chance on
readable code. Places where they are unavoiable is in packet processing,
where tests on, and calls to, the relevant ``Incoming`` functions are
guarded by an ``#ifdef``. The defines that can be used to govern the stack
are listed in the user documentation.


Details on the interfaces
-------------------------

Timers
``````

This module provides timing facilities to all other parts of the networking
stack. 
Include the file ``timer.h`` to get access to this module.

Timer identifiers
+++++++++++++++++

.. doxygendefine:: PIP_DHCP_TIMER_T1   
.. doxygendefine:: PIP_DHCP_TIMER_T2   
.. doxygendefine:: PIP_ARP_TIMER       
.. doxygendefine:: PIP_TCP_TIMER_TIMEWAIT
.. doxygendefine:: PIP_TFTP_TIMER      

Fuzz factors
++++++++++++

.. doxygendefine:: PIP_FUZZ_NONE       
.. doxygendefine:: PIP_FUZZ_1MS        
.. doxygendefine:: PIP_FUZZ_10MS       
.. doxygendefine:: PIP_FUZZ_100MS      
.. doxygendefine:: PIP_FUZZ_1S         
.. doxygendefine:: PIP_FUZZ_10S        

API
+++

.. doxygenfunction:: pipTimeOut
.. doxygenfunction:: pipSetTimeOut
.. doxygenfunction:: pipResetTimeOut

Checksum
````````

These functions are used to compute the one's complement checksum.
Include the file ``checksum.h`` to get access to this module.

.. doxygenfunction:: onesAdd
.. doxygenfunction:: onesChecksum

Packet transmission
```````````````````

These functions are used to create a packet for transmission; they are
called from all ``Outgoing`` functions.
Include the file ``tx.h`` to get access to this module.

.. doxygenfunction:: txInt
.. doxygenfunction:: txShort
.. doxygenfunction:: txShortRev
.. doxygenfunction:: txShortZeroes
.. doxygenfunction:: txByte
.. doxygenfunction:: txData
.. doxygenfunction:: txClear
.. doxygenfunction:: txPrint
.. doxygenfunction:: doTx

Ethernet
````````

This module is used to handle Ethernet traffic.
Include the file ``ethernet.h`` to get access to this module.

Constants
+++++++++

.. doxygendefine:: PIP_ETHTYPE_IPV4_REV
.. doxygendefine:: PIP_ETHTYPE_ARP_REV

Global variables
++++++++++++++++

.. doxygenvariable:: myMacAddress

API
+++

.. doxygenfunction:: pipIncomingEthernet
.. doxygenfunction:: pipOutgoingEthernet


IPv4
````

This module is used to handle IPv4 traffic.
Include the file ``ipv4.h`` to get access to this module.

Constants
+++++++++

.. doxygendefine:: PIP_IPTYPE_TCP 
.. doxygendefine:: PIP_IPTYPE_UDP 
.. doxygendefine:: PIP_IPTYPE_ICMP
.. doxygendefine:: PIP_IPTYPE_IGMP

Global variables
++++++++++++++++

.. doxygenvariable:: myIP
.. doxygenvariable:: mySubnetIP

API
+++

.. doxygenfunction:: pipIncomingIPv4(unsigned short packet[], int offset);
.. doxygenfunction:: pipOutgoingIPv4(int ipType, unsigned ipDst, int length);


ARP
```

This module is used to handle ARP traffic.
Include the file ``arp.h`` to get access to this module.

Constants
+++++++++

.. doxygendefine:: ARPENTRIES

Types
+++++

.. doxygenstruct:: arp

Global variables
++++++++++++++++

.. doxygenvariable:: pipArpTable

API
+++

.. doxygenfunction:: pipInitARP
.. doxygenfunction:: pipIncomingARP
.. doxygenfunction:: pipTimeOutARP
.. doxygenfunction:: pipCreateARP
.. doxygenfunction:: pipARPStoreEntry


TCP
```

These functions are used to handle TCP traffic.
Include the file ``tcp.h`` to get access to this module.

.. doxygenfunction:: pipInitTCP
.. doxygenfunction:: pipIncomingTCP
.. doxygenfunction:: pipTimeoutTCPTimewait
.. doxygenfunction:: pipApplicationTCP

UDP
```

These functions are used to handle UDP traffic.
Include the file ``udp.h`` to get access to this module.

.. doxygenfunction:: pipIncomingUDP
.. doxygenfunction:: pipOutgoingUDP

ICMP
````

These functions are used to handle ICMP traffic.
Include the file ``icmp.h`` to get access to this module.

.. doxygenfunction:: pipIncomingICMP

DHCP
````

These functions are used to handle DHCP traffic. If TFTP is enabled, then a
call to TFTP will be made once an address has been acquired.
Include the file ``dhcp.h`` to get access to this module.


.. doxygenfunction:: pipInitDHCP
.. doxygenfunction:: pipIncomingDHCP
.. doxygenfunction:: pipCreateDHCP
.. doxygenfunction:: pipTimeOutDHCPT1
.. doxygenfunction:: pipTimeOutDHCPT2


TFTP
````

This module is used to handle TFTP traffic.
Include the file ``tftp.h`` to get access to this module.


Constants
+++++++++

.. doxygendefine:: TFTP_SERVER_PORT

Global variables
++++++++++++++++

.. doxygenvariable:: pipPortTFTP
.. doxygenvariable:: pipIpTFTP

API
+++

.. doxygenfunction:: pipInitTFTP
.. doxygenfunction:: pipIncomingTFTP
.. doxygenfunction:: pipTimeOutTFTP
