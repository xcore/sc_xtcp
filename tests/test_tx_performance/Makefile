TARGET ?= SLICEKIT-L2
APP_NAME =
XCC_FLAGS = -g -O3 -fxscope
USED_MODULES = module_xtcp module_ethernet_board_support

ifeq ($(notdir $(abspath ..)),tests)
PATHSEP = $(if $(findstring Windows, $(OS))$(findstring WINDOWS,$(OS)),;,:)
XMOS_MODULE_PATH := $(XMOS_MODULE_PATH)$(PATHSEP)../../..
endif



#=============================================================================
# The following part of the Makefile includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.

XMOS_MAKE_PATH ?= ../..
include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
