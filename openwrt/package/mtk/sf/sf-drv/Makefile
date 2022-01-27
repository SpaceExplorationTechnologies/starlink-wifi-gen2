#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=sf
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/sf
  CATEGORY:=MTK Properties
  SUBMENU:=MT7622/29 serial flash utilities
  TITLE:=MT7622/29 serial flash helper
  FILES:=$(PKG_BUILD_DIR)/sf.ko
  DEPENDS:=@TARGET_mediatek_mt7622||TARGET_mediatek_mt7629
  KCONFIG:=
endef

define KernelPackage/sf/description
  Helper driver used for MT7622 to access extra serial falsh features
  e.g. Unique ID and SFDP
endef

EXTRA_KCONFIG:= \
	CONFIG_MT7622_SF=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \

ifeq ($(CONFIG_TARGET_mediatek_mt7622),y)
EXTRA_CFLAGS += -DMT7622=1
else
ifeq ($(CONFIG_TARGET_mediatek_mt7629),y)
EXTRA_CFLAGS += -DMT7629=1
endif
endif

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/sf
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/sf.h $(1)/usr/include/sf/
endef

$(eval $(call KernelPackage,sf))
