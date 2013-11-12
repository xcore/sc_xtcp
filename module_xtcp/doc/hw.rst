H/W Development Platforms
=========================

For initial development of ethernet applications the following XMOS
development platforms can be used:

  * XK-AVB-LC-SYS AVB Audio Endpoint
  * XP-SKC-L16 (sliceKIT L16 Core Board) plus XA-SK-AUDIO plus XA-SK-XTAG2 (sliceKIT xTAG adaptor) plus xTAG2 (debug adaptor)

For developing an application specific board with ethernet please
refer to the hardware guides for the above boards with example
schematics, BOMs, design guidelines etc.

Note that the 2 logical core version of the stack relies on the single thread MII ethernet component, which requires 62.5MIPS to run correctly.  Consequently, the 2 logical core version of the stack will not run on a 400MHz device.

