#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=btmtk_uart
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME).tar.gz

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)

define KernelPackage/btmtk_uart
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=bluetooth uart driver
  KCONFIG:= \
    CONFIG_BT=m \
    CONFIG_BT_BREDR=y \
    CONFIG_BT_DEBUGFS=n \
    CONFIG_BT_LE=y
  SUBMENU:=Drivers
  FILES:=$(PKG_BUILD_DIR)/btmtk_uart.ko $(LINUX_DIR)/net/bluetooth/bluetooth.ko
  AUTOLOAD:=$(call AutoProbe,btmtk_uart)
endef

define KernelPackage/btmtk_uart/description
  BT uart driver
endef

include $(INCLUDE_DIR)/package.mk


define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	ARCH="$(LINUX_KARCH)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	modules
endef

define KernelPackage/btmtk_uart/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/bin/mt7915/bt/mt7915_patch_e2_hdr.bin $(1)/lib/firmware/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/bin/mt7915/bt/test_bt_patch.bin $(1)/lib/firmware/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/bin/mt7915/bt/bt_ilm_7915.bin $(1)/lib/firmware/
endef

$(eval $(call KernelPackage,btmtk_uart))
