H/W Development Platforms
=========================

For initial development of ethernet applications the following XMOS
development platforms can be used:

  * XC-2 Ethernet Kit (http://www.xmos.com/products/development-kits/xc-2-ethernet-kit)
  * XDK XS1-G Development Kit (http://www.xmos.com/products/development-kits/xs1-g-development-kit)
  * XP-DSC-BLDC Motor Control Platform (http://www.xmos.com/development-kits/motor-control-platform)
  * XK-AVB-LC-SYS AVB Audio Endpoint (http://www.xmos.com/products/reference-designs/avbl2)
  * XP-MC-CTRL-L2 Control Board Platform (http://www.xmos.com/development-kits/motor-control-platform)

For developing an application specific board with ethernet please
refer to the hardware guides for the above boards with example
schematics, BOMs, design guidelines etc.

Note that the 2 thread version of the stack relies on the single thread MII ethernet
component, which requires 62.5MIPS to run correctly.  Consequently, the 2-thread
version of the stack will not run on a 400MHz device.

