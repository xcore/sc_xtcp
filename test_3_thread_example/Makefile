TARGET ?= XP-MC-CTRL-L2
APP_NAME =
XCC_FLAGS = -g -O3 -lxscope
USED_MODULES = module_xtcp module_mii_singlethread \
               module_mii_singlethread_server module_ethernet_smi

USE_XTCP_MAC_CUSTOM_FILTER = 1


#=============================================================================
# The following part of the Makefile includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.

XMOS_MAKE_PATH ?= ../..
include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
