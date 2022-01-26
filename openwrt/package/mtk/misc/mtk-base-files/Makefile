#
# Copyright (C) 2016 MediaTek
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=mtk-base-files
PKG_RELEASE:=1
PKG_BUILD_DEPENDS:=base-files
PKG_FILE_DEPENDS:=
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/mtk-base-files
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Misc
  DEPENDS:=+base-files +lua
  TITLE:=Base filesystem for MTK OpenWrt
  VERSION:=$(PKG_RELEASE)-$(REVISION)
endef

define Package/mtk-base-files/description
 This package contains a base filesystem and system scripts for MTK OpenWrt.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile/Default
endef

Build/Compile = $(Build/Compile/Default)

define Package/mtk-base-files/install
	$(CP) ./files/* $(1)/

	-cp -f $(TOPDIR)/.config $(1)/etc/openwrt.config
	-cp -f $(LINUX_DIR)/.config $(1)/etc/kernel.config
	-cp -f $(LINUX_DIR)/.config $(TOPDIR)/tmp/kernel.config
	if [ -d $(TOPDIR)/.git ]; then echo `git rev-parse HEAD | cut -c-8` > $(1)/etc/sdk.version ;fi
	if [ -f $(LINUX_DIR)/.p4rev ]; then cp $(LINUX_DIR)/.p4rev $(1)/etc/kernel.p4rev ;fi
	if [ -f $(TOPDIR)/.mtk.openwrt.ver ]; then {\
		cat $(TOPDIR)/.mtk.openwrt.ver >> $(1)/etc/banner.mtk;\
	} fi
	if [ -f $(1)/etc/sdk.version ]; then {\
		echo -n " SDK Revision : " >> $(1)/etc/banner.mtk;\
		cat $(1)/etc/sdk.version >> $(1)/etc/banner.mtk;\
	} fi
	if [ -f $(1)/etc/kernel.p4rev ]; then {\
		echo -n " Kernel P4Rev : " >> $(1)/etc/banner.mtk;\
		cat $(1)/etc/kernel.p4rev >> $(1)/etc/banner.mtk;\
	} fi
	echo " -----------------------------------------------------" >> $(1)/etc/banner.mtk
endef

$(eval $(call BuildPackage,mtk-base-files))
