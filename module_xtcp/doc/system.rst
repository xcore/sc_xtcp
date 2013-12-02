TCP/IP Stack System Description
===============================

Software Architecture
---------------------

The following Figure shows the architecture of the TCP/IP stack when
attaching to an independent Ethernet MAC through an xC channel:

.. only:: html

  .. figure:: images/xtcp_arch-crop.png
     :align: center

     XTCP software architecture

.. only:: latex

  .. figure:: images/xtcp_arch-crop.pdf
     :figwidth: 50%
     :align: center

     XTCP software architecture

The server runs on a single logical core and connects to the XMOS Ethernet
MAC component. It can then connect to several client
tasks over xC channels. To enable this option the define
``XTCP_USE_SEPARATE_MAC`` needs to be set to ``1`` in the
``xtcp_conf.h`` file in your application and run the
:c:func:`xtcp_server` function.

Alternatively, the TCP/IP server and Ethernet server can be run as
an integrated system on two logical cores. This can be started by
running the :c:func:`ethernet_xtcp_server` function.


IP Configuration
----------------

The server will determine its IP configuration based on the arguments
passed into the :c:func:`xtcp_server` or :c:func:`ethernet_xtcp_server`
function.
If an address is supplied then that address will be used (a static IP address
configuration).

If no address is supplied then the server will first
try to find a DHCP server on the network to obtain an address
automatically. If it cannot obtain an address from DHCP, it will determine
a link local address (in the range 169.254/16) automatically using the
Zeroconf IPV4LL protocol.

To use dynamic address, the :c:func:`xtcp_server` or
:c:func:`ethernet_xtcp_server` function can be passed a *null* to
the ip configuration parameter.

Events and Connections
----------------------

The TCP/IP stack client interface is a low-level event based
interface. This is to allow applications to manage buffering and
connection management in the most efficient way possible for the
application. 

.. only:: html

  .. figure:: images/events-crop.png
     :align: center

     Example event sequence

.. only:: latex

  .. figure:: images/events-crop.pdf
     :figwidth: 50%
     :align: center

     Example event sequence


Each client will receive *events* from the server. These events
usually have an associated *connection*. In addition to receiving
these events the client can send *commands* to the server to initiate
new connections and so on.

The above Figure shows an example event/command sequence of a
client making a connection, sending some data, receiving some data and
then closing the connection. Note that sending and receiving may be
split into several events/commands since the server itself performs no
buffering. 

If the client is handling multiple connections then the server may
interleave events for each connection so the client has to hold a
persistent state for each connection.

The connection and event model is the same from both TCP connections
and UDP connections. Full details of both the possible events and
possible commands can be found in Section :ref:`sec_api`.

TCP and UDP
-----------

The XTCP API treats UDP and TCP connections in the same way. The only
difference is when the protocol is specified on initializing
connections with :c:func:`xtcp_connect` or :c:func:`xtcp_listen`.

New Connections
---------------

New connections are made in two different ways. Either the
:c:func:`xtcp_connect` function is used to initiate a connection with
a remote host as a client or the :c:func:`xtcp_listen` function is
used to listen on a port for other hosts to connect to the application
. In either
case once a connection is established then the
:c:member:`XTCP_NEW_CONNECTION` event is triggered.

In the Berkley sockets API, a listening UDP connection merely reports
data received on the socket, indepedent of the source IP address.  In
XTCP, a :c:member:`XTCP_NEW_CONNECTION` event is sent each time data
arrives from a new source.  The API function :c:func:`xtcp_close`
should be called after the connection is no longer needed.

Receiving Data
--------------

When data is received by a connection, the :c:member:`XTCP_RECV_DATA`
event is triggered and communicated to the client. At this point the
client **must** call the :c:func:`xtcp_recv` function to receive the
data. 

Data is sent from host to client as the UDP or TCP packets come
in. There is no buffering in the server so it will wait for the client
to handle the event before processing new incoming packets.

Sending Data
------------

When sending data, the client is responsible for dividing the data
into chunks for the server and re-transmitting the previous chunk if a
transmission error occurs. 

.. note:: Note that re-transmission may be needed on
          both TCP and UDP connections. On UDP connections, the
          transmission may fail if the server has not yet established
          a connection between the destination IP address and layer 2
          MAC address.
          
The client can initiate a send transaction with the
:c:func:`xtcp_init_send` function. At this point no sending has been
done but the server is notified of a wish to send. The client must
then wait for a :c:member:`XTCP_REQUEST_DATA` event at which point it
must respond with a call to :c:func:`xtcp_send`. 

.. note:: The maximum buffer size that can be sent in one call to 
          `xtcp_send` is contained in the `mss`
          field of the connection structure relating to the event.

After this data is sent to the server, two things can happen: Either
the server will respond with an :c:member:`XTCP_SENT_DATA` event, in
which case the next chunk of data can be sent or with an
:c:member:`XTCP_RESEND_DATA` event in which case the client must
re-transmit the previous chunk of data. 

The command/event exchange continues until the client calls the
:c:func:`xtcp_complete_send` function to finish the send
transaction. After this the server will not trigger any more
:c:member:`XTCP_SENT_DATA` events.

Link Status Events
------------------

As well as events related to connections. The server may also send
link status events to the client. The events :c:member:`XTCP_IFUP` and 
:c:member:`XTCP_IFDOWN` indicate to a client when the link goes up or down.

Configuration
-------------

The server is configured via arguments passed to the
:c:func:`xtcp_server` function and the defines described in Section
:ref:`sec_config_defines`.

Client connections are configured via the client API described in
Section :ref:`sec_config_defines`.


