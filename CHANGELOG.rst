sc_xtcp Change Log
==================

3.1.4
~~~~~
  * Updated ethernet dependency to version 2.2.5

3.1.3
~~~~~
  * Updated ethernet dependency to version 2.2.4
  * Fixed corner case errors/improved robustness in DHCP protocol handling

3.1.2
~~~~~
  * Fixed auto-ip bug for 2-core xtcp server

3.1.1
~~~~~
  * Minor code demo app fixes (port structures should be declared on
    specific tiles)

3.1.0
~~~~~
  * Compatible with 2.2 module_ethernet
  * Updated to new intializer api and integrated ethernet server

3.0.1
~~~~~

   * Updated to use latest sc_ethernet package

3.0.0
~~~~~
   * Fixed bugs in DHCP and multicast UDP
   * Updated packaging, makefiles and documentation
   * Updated to use latest sc_ethernet package

2.0.1
~~~~~

   * Further memory improvements
   * Additional conditional compilation
   * Fix to zeroconf with netbios option enabled

2.0.0
~~~~~

   * Memory improvements
   * Fix error whereby UDP packets with broadcast destination were not received
   * An initial implementation of a TFTP server

1.3.1
~~~~~

   * Initial implementation

