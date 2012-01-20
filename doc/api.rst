.. _sec_api:

API
===

.. _sec_config_defines:

Configuration Defines
---------------------

The following defines can be set by adding the file
``xtcp_client_conf.h`` into your application and setting the defines
within that file.

.. list-table::
   :header-rows: 1
   :widths: 3 2 1
  
   * - Define
     - Description
     - Default
   * - ``XTCP_CLIENT_BUF_SIZE``
     - The buffer size used for incoming packets. This has a maximum
       value of 1472 which can handle any incoming packet. If it is 
       set to less larger incoming packets will be truncated.
     - 1472 
   * - ``UIP_CONF_MAX_CONNECTIONS``
     - The maximum number of UDP or TCP connections the server can
       handle simultaneously.       
     - 20
   * - ``UIP_CONF_MAX_LISTENPORTS``
     - The maximum number of UDP or TCP ports the server can listen to
       simultaneously.     
     - 20
   * - UIP_USE_SINGLE_THREADED_ETHERNET
     - Defining this, and using the uipSingleServer function in the
       multithreaded main, will use the two thread TCP plus Ethernet.
     - Not defined
   * - XTCP_EXCLUDE_LISTEN
     - Exclude support for the listen command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_UNLISTEN
     - Exclude support for the unlisten command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_CONNECT
     - Exclude support for the connect command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_BIND_REMOTE
     - Exclude support for the bind_remote command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_BIND_LOCAL
     - Exclude support for the bind_local command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_INIT_SEND
     - Exclude support for the init_send command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_SET_APPSTATE
     - Exclude support for the set_appstate command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_ABORT
     - Exclude support for the abort command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_CLOSE
     - Exclude support for the close command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_SET_POLL_INTERVAL
     - Exclude support for the set_poll_interval command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_JOIN_GROUP
     - Exclude support for the join_group command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_LEAVE_GROUP
     - Exclude support for the leave_group command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_GET_MAC_ADDRESS
     - Exclude support for the get_mac_address command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_GET_IPCONFIG
     - Exclude support for the get_ipconfig command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_ACK_RECV
     - Exclude support for the ack_recv command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_ACK_RECV_MODE
     - Exclude support for the ack_recv_mode command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_PAUSE
     - Exclude support for the pause command from the server, reducing memory footprint
     - Not defined
   * - XTCP_EXCLUDE_UNPAUSE
     - Exclude support for the unpause command from the server, reducing memory footprint
     - Not defined


Data Structures/Types
---------------------

.. doxygentypedef:: xtcp_ipaddr_t

.. doxygenstruct:: xtcp_ipconfig_t

.. doxygenenum:: xtcp_protocol_t

.. doxygenenum:: xtcp_event_type_t

.. doxygenenum:: xtcp_connection_type_t

.. doxygenstruct:: xtcp_connection_t

Server API
----------

.. doxygenfunction:: uip_server

.. doxygenfunction:: uipSingleServer

.. _sec_client_api:

Client API
----------

Event Receipt
+++++++++++++

.. doxygenfunction:: xtcp_event

Setting Up Connections
++++++++++++++++++++++

.. doxygenfunction:: xtcp_listen
.. doxygenfunction:: xtcp_unlisten
.. doxygenfunction:: xtcp_connect
.. doxygenfunction:: xtcp_bind_local
.. doxygenfunction:: xtcp_bind_remote
.. doxygenfunction:: xtcp_set_connection_appstate

Receiving Data
++++++++++++++

.. doxygenfunction:: xtcp_recv
.. doxygenfunction:: xtcp_recvi
.. doxygenfunction:: xtcp_recv_count

Sending Data
++++++++++++

.. doxygenfunction:: xtcp_init_send
.. doxygenfunction:: xtcp_send
.. doxygenfunction:: xtcp_sendi
.. doxygenfunction:: xtcp_complete_send

Other Connection Management
+++++++++++++++++++++++++++

.. doxygenfunction:: xtcp_set_poll_interval

.. doxygenfunction:: xtcp_close
.. doxygenfunction:: xtcp_abort

.. doxygenfunction:: xtcp_pause
.. doxygenfunction:: xtcp_unpause

Other General Client Functions
++++++++++++++++++++++++++++++

.. doxygenfunction:: xtcp_join_multicast_group
.. doxygenfunction:: xtcp_leave_multicast_group
.. doxygenfunction:: xtcp_get_mac_address
.. doxygenfunction:: xtcp_get_ipconfig

High-level blocking client API
++++++++++++++++++++++++++++++

.. doxygenfunction:: xtcp_wait_for_ifup
.. doxygenfunction:: xtcp_wait_for_connection
.. doxygenfunction:: xtcp_write
.. doxygenfunction:: xtcp_read

High-level buffered client API
++++++++++++++++++++++++++++++

.. doxygenfunction:: xtcp_buffered_set_rx_buffer
.. doxygenfunction:: xtcp_buffered_set_tx_buffer
.. doxygenfunction:: xtcp_buffered_recv
.. doxygenfunction:: xtcp_buffered_recv_upto
.. doxygenfunction:: xtcp_buffered_send
.. doxygenfunction:: xtcp_buffered_send_handler
.. doxygenfunction:: xtcp_buffered_send_buffer_remaining


