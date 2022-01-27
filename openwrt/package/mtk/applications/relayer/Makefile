#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=relayer
PKG_REVISION:=440ce87d
PKG_SOURCE:=$(PKG_NAME)-$(PKG_REVISION).tar.gz
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/relayer
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=relayer for bluetooth fw combo tool test
  SUBMENU:=Applications
endef

define Package/relayer/description
  An program to bluetooth fw combo tool test.
endef



MAKE_FLAGS += CONFIG_SUPPORT_OPENWRT=y CONFIG_USER_GATT_APCLI=y CONFIG_DEFAULTS_MEDIATEK_MT7622=y

define Build/Compile
	echo "MAKE_FLAGS:"$(MAKE_FLAGS)
	$(MAKE) -C $(PKG_BUILD_DIR) $(MAKE_FLAGS)
endef

define Build/InstallDev
	mkdir -p $(1)/usr/lib
	$(CP)   $(PKG_BUILD_DIR)/bin/*.so $(1)/usr/lib/
endef

define Package/relayer/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/bin/*.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(CP)   $(PKG_BUILD_DIR)/bin/relayer $(1)/usr/bin/
endef

$(eval $(call BuildPackage,relayer))
