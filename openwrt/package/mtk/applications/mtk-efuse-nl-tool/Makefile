#
# Copyright (C) 2021 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtk-efuse-nl-tool
PKG_RELEASE:=1

PKG_MAINTAINER:=Alvin Kuo <Alvin.Kuo@mediatek.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/mtk-efuse-nl-tool
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=MT7622/MT7629 eFuse tool
  SUBMENU:=Applications
  DEPENDS:=+libnl-tiny +kmod-mtk-efuse-nl-drv @TARGET_mediatek_mt7622||TARGET_mediatek_mt7629
endef

define Package/mtk-efuse-nl-tool/description
  MT7622/MT7629 eFuse tool
endef

define Build/Configure
endef

TARGET_CFLAGS += \
	-D_GNU_SOURCE \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(PKG_BUILD_DIR)

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS) -lnl-tiny -lm"
endef

define Package/mtk-efuse-nl-tool/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mtk-efuse-tool $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,mtk-efuse-nl-tool))
