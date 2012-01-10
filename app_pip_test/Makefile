TARGET = XP-DSC-BLDC
APP_NAME =
XCC_FLAGS = -Os -g -report -lxscope \
            -DPIP_ICMP -DPIP_DHCP -DPIP_UDP -DPIP_ARP -DPIP_TCP -DPIP_TFTP
USED_MODULES = module_pip module_mii_singlethread


#=============================================================================
# The following part of the Makefile includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.

XMOS_MAKE_PATH ?= ../..
include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
