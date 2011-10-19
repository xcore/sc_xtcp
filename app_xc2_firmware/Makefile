# The TARGET variable determines what target system the application is
# compiled for. It either refers to an XN file in the source directories
# or a valid argument for the --target option when compiling
TARGET = XC-2

# The APP_NAME variable determines the name of the final .xe file. It should
# not include the .xe postfix. If left blank the name will default to
# the project name
APP_NAME =

# The USED_MODULES variable lists other module used by the application.
USED_MODULES = module_xtcp module_zeroconf module_ethernet module_locks

# This uses the tcp mac filtering in module_xtcp
USE_XTCP_MAC_CUSTOM_FILTER=1

# The flags passed to xcc when building the application
# You can also set the following to override flags for a particular language:
# XCC_XC_FLAGS, XCC_C_FLAGS, XCC_ASM_FLAGS, XCC_CPP_FLAGS
# If the variable XCC_MAP_FLAGS is set it overrides the flags passed to
# xcc for the final link (mapping) stage.
ifeq "$(CONFIG)" "Debug"
XCC_FLAGS = -Xmapper --map -Xmapper map.out -save-temps -g -O0 -DMDNS_NETBIOS=1 -DXC2_FIRMWARE_VERSION=2v0
else
XCC_FLAGS = -O3 -save-temps -g -DMDNS_NETBIOS=1 -DXC2_FIRMWARE_VERSION=2v0
endif

EXCLUDE_FILES = httpd-fsdata.c app.c fsdata.c fsdata_old.c

# The VERBOSE variable, if set to 1, enables verbose output from the make
# system.
VERBOSE = 0

#=============================================================================
# The following part of the Makefile includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.

XMOS_MAKE_PATH ?= ../..
include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
