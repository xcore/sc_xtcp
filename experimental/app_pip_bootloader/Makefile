TARGET = XP-ETHERNET-TEST
APP_NAME =
XCC_FLAGS = -Os -g -report 
USED_MODULES = module_pip module_ethernet module_otp_board_info module_ethernet_board_support

# This change to the module path is so that this application can be in the
# tests sub-directory in it's git repo
ifeq ($(notdir $(abspath ..)),experimental)
PATHSEP = $(if $(findstring Windows, $(OS))$(findstring WINDOWS,$(OS)),;,:)
XMOS_MODULE_PATH := $(XMOS_MODULE_PATH)$(PATHSEP)../../..
endif

#=============================================================================
# The following part of the Makefile includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.

XMOS_MAKE_PATH ?= ../..
include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
