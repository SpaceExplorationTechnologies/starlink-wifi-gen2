#
# Copyright (C) Hua Shao <nossiac@163.com>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=MediaTek Proprietary Configuration.
LUCI_DEPENDS:=+PACKAGE_libmapd:libmapd +datconf-lua
LUCI_MK_PATH:=$(shell test -e ../luci.mk && echo "old")
ifeq ($(LUCI_MK_PATH),old)
include ../luci.mk
else
include ../../luci.mk
endif

# call BuildPackage - OpenWrt buildroot signature
