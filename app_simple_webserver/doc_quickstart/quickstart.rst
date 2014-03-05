Simple webserver demo quickstart guide
======================================
This application demonstrates an simple webserver running on an XMOS device using the Ethernet sliceCARD ``XA-SK-E100``. The web page hosted by this application is stored in the program memory.

Host computer setup
-------------------

For IPv4 addressing
~~~~~~~~~~~~~~~~~~~
A computer with:

* Windows / Mac OS / Linux operating system.
* Internet browser (Internet Explorer, Chrome, Firefox, etc.)
* An Ethernet port or connected to a network router with spare Ethernet port.
* Download and install the xTIMEcomposer studio from XMOS xTIMEcomposer downloads webpage.

For IPv6 addressing
~~~~~~~~~~~~~~~~~~~
This demo with IPv6 addressing can be run on a Linux (Ubuntu 12.04 LTS) operating system. The following setup guide is for *Ubuntu 12.04 LTS* OS.

#. Open a terminal.
#. Open *sysctl.conf* file by typing the following command in the terminal::

     sudo gedit /etc/sysctl.conf

   Uncomment the line which contains ``net.ipv6.conf.all.forwarding=1``. Save and close this file.

#. In the terminal, type the following command::

     sudo sysctl net/ipv6/conf/all/forwarding=1

   This should pass through successfully and return to the prompt.

#. Install *radvd* using the following command in the terminal::

     sudo apt-get install radvd

   Wait till radvd is installed and terminal control returns to prompt.

#. Edit *radvd.conf*. This file does not exist after a fresh install of radvd. Create/edit by typing this command in the terminal::

     sudo gedit /etc/radvd.conf

#. File *radvd.conf* opens. Replace its contents with the following one::

     interface eth0
     {
       AdvSendAdvert on;
       prefix 2001:db8::0/64
       {
       };
     };

   Save and close this file.

#. Open the *interfaces* file by typing the following command in the terminal::

     sudo gedit /etc/network/interfaces

#. Add these line in the *interfaces* file::

     iface eth0 inet6 auto
     address 2001:db8::1/64

   Save and close this file.

#. In the terminal, type the following command::

     sudo ip addr add 2001:db8::1/64 dev eth0

#. Restart networking by typing the following command in the terminal::

     sudo service network-manager restart

#. Restart radvd by typing the following command in the terminal::

     sudo /etc/init.d/radvd restart

Hardware setup
--------------
Required hardware:

* sliceKIT L16 core board (XP-SKC-L16)
* Ethernet sliceCARD (XA-SK-E100)
* XTAG2 (XTAG-2)
* XTAG2 adapter (XA-SK-XTAG2)
* Ethernet cable
* 12V DC power supply

Setup:

#. Connect the adapter to the core board.
#. Set the ``XMOS LINK`` to ON on the adapter.
#. Connect XTAG2 to ``XSYS`` side of the adapter.
#. Connect the other end of XTAG2 to your computer using a USB cable.
#. Connect the Ethernet sliceCARD to the sliceKIT core board using the connector marked with the ``CIRCLE`` (indicated by a white color circle (or) ``J6``) slot.
#. Connect the other end of Ethernet sliceCARD and host computer using Ethernet cable.

   .. note:: For IPv6 demo, this connection should be made directly to the host computer's Ethernet port.

#. Connect the 12V power supply to the core board and switch it ON.

.. figure:: images/hardware_setup.*

   Hardware setup

Import and build the application
--------------------------------
Importing the Simple webserver demo application:

* Open the xTIMEcomposer studio and ensure that it is operating in online mode.
* Open the *Edit* perspective (Window -> Open Perspective -> XMOS Edit).
* Open the *xSOFTip* view from (Window -> Show View -> xSOFTip). An *xSOFTip* window appears on the bottom-left.
* Search for *Simple Webserver Demo*.
* Click and drag it into the *Project Explorer* window. Doing this will open a *Import xTIMEcomposer Software* window. Click on *Finish* to download and complete the import.
* This will also automatically import dependencies for this application.
* The application is called as *app_simple_webserver* in the *Project Explorer* window.

Selecting between IPv4 and IPv6 addressing:

* Click on the small (downward) arrow next to the Build icon (indicated by a 'Hammer' picture).
* This will show a list of build configurations. Currently: IPV4 and IPV6.
* To use IPv4 addressing, select IPV4.
* To use IPv6 addressing, select IPV6.

Building the Simple webserver demo application:

* Click on the *app_simple_webserver* item in the *Project Explorer* window.
* Click on the *Build* (indicated by a 'Hammer' picture) icon.
* Check the *Console* window to verify that the application has built successfully.

Run the application
-------------------

To run the application using xTIMEcomposer studio:

* In the *Project Explorer* window, locate the *app_simple_webserver_IPVn.xe* in the (app_simple_webserver -> Binaries).

  .. note:: where n in *app_simple_webserver_IPVn.xe* is 4 for IPv4 and 6 for IPV6 addressing.
  
* Right click on *app_simple_webserver_IPVn.xe* and click on (Run As -> xCORE Application).
* A *Select Device* window appears.
* Select *XMOS XTAG-2 connected to L1* and click OK.

Demo:

* The following message appears in the *Console* window of the xTIMEcomposer studio::

   **WELCOME TO THE SIMPLE WEBSERVER DEMO**

* At this point, the XMOS device is trying to acquire an IP address in the network. Wait for some time (approximately 10 seconds) for the following message to appear in the *Console* window. Note, the IP address may be different based on your network.

  For IPv4 addressing::

    ipv4ll: 169.254.10.130

  For IPv6 addressing::

    IPV6 Address = [2001:db8::222:97ff:fe00:5260]

* Open a web browser (Firefox, etc...) in your host computer and enter the above IP address in the address bar of the browser. It opens a *Hello World* web page as hosted by the simple webserver running on the XMOS device.


Next steps
----------

* Look at the ``Embedded Webserver Function Library`` - a module for adding a website to your application. This includes adding dynamic content in the website.

Troubleshooting
---------------

* **XMOS device does not acquire an IP address even after waiting for 10+ seconds**

  - Ensure that the Ethernet cable between Ethernet sliceCARD and the Ethernet port is connected correctly.
  - Ensure that the sliceKIT core board is powered by a 12V power supply.
  - In the host computer, open a terminal and check if an IP address is acquired by the host.

    On Ubuntu / Mac: type the command::

      ifconfig eth0

    check the output that it has IPv4 (inet) and/or IPv6 (inet6 Global and inet6 local) addresses. Check for inet6 addresses, if using IPv6 addressing. If not, try to restart the network manager.

    On Ubuntu / Mac: type the command::

      sudo ifconfig eth0 down
      sudo ifconfig eth0 up

* **IP address acquired by the XMOS device. But the browser does not load web page.**

  On Ubuntu / Mac (IPv4 addressing): Check if the IPv4 Settings -> Method is *Link-Local Only*. Use this, if the demo is connected directly to host computer's Ethernet port.

  On Ubuntu (IPv6 addressing): Check if the Global inet6 address is present for eth0 (using ``ifconfig eth0``). If not, assign one using::

    sudo ip addr add 2001:db8::1/64 dev eth0

* **IPv6 Demo runs OK. But when the computer is restarted, it boots without any network configuration**

  For Ubuntu: do a network manager restart using::

    sudo service network-manager restart
    sudo ifconfig eth0 down
    sudo ifconfig eth0 up


