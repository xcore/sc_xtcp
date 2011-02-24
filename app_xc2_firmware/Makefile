TARGET ?= XC-2
BASE_DIR = .

DEMO_SOURCE_DIRS = src \
		   src/misc \
                   src/config \
                   src/httpd \
                   src/httpd/fsdata 

SOURCE_DIRS += $(DEMO_SOURCE_DIRS)

INCLUDE_DIRS += $(DEMO_SOURCE_DIRS) 

XCC_FLAGS += -Xmapper --map -Xmapper map.out -DMDNS_NETBIOS=1 -O3 -save-temps -g -DXC2_FIRMWARE_VERSION=2v0

EXCLUDE_FILES = httpd-fsdata.c app.c fsdata.c fsdata_old.c

USED_MODULES = module_xtcp.2v0 module_zeroconf.1v0 module_ethernet.1v4 module_locks.1v1

USE_XTCP_MAC_CUSTOM_FILTER = 1

-include ../module_xcommon/build/Makefile.common
-include ../../xcommon/module_xcommon/build/Makefile.common

all: $(BIN_DIR)/xc2_firmware.xe
	@echo Build complete

clean: clean_common
