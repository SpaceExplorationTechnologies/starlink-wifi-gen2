#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uart_launcher
PKG_REVISION:=mp
PKG_SOURCE:=$(PKG_NAME)-$(PKG_REVISION).tar.gz
PKG_RELEASE:=1
PKG_VERSION:=mp

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/uart_launcher
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=launcher for bluetooth uart driver
  SUBMENU:=Applications
  DEPENDS:=+kmod-btmtk_uart
endef

define Package/uart_launcher/description
  The application will launch BT uart driver
endef



MAKE_FLAGS += CONFIG_SUPPORT_OPENWRT=y

define Build/Compile
	echo "MAKE_FLAGS:"$(MAKE_FLAGS)
	$(MAKE) -C $(PKG_BUILD_DIR) $(MAKE_FLAGS)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin/
	$(CP)   $(PKG_BUILD_DIR)/uart_launcher $(1)/usr/bin/
endef

define Package/uart_launcher/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uart_launcher $(1)/usr/bin/
endef

$(eval $(call BuildPackage,uart_launcher))
