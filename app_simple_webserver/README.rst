Simple webserver demo
=====================

:scope: Early Development
:description: A demo of the TCP/IP stack that provides a simple webserver
:keywords: ethernet, tcp/ip, webserver, http

This application uses the tcp/ip stack to produce a simple
webserver. It is here as an illustration on how to use the xtcp stack.

To use this demo, first edit the main.xc source file and change the 
following declaration:

xtcp_ipconfig_t ipconfig = ...

This declares a structure with the ip configuration of the demo. The
first elemet is the ip address, the second the netmask and the third
the gateway. Change these setting to something that is routable on the
network you are testing on.

Now build the project and connect the XC-2 to the network. You should
be able to connect to the ip address you specified with a web browser
and see a "Hello World" message.

<Add description of software block>
