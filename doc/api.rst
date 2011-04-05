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


