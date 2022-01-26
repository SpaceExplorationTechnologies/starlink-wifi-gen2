#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mtk-efuse-nl-drv
PKG_RELEASE:=1

PKG_MAINTAINER:=Alvin Kuo <Alvin.Kuo@mediatek.com>

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mtk-efuse-nl-drv
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=MT7622/MT7629 eFuse driver
  SUBMENU:=Drivers
  FILES:=$(PKG_BUILD_DIR)/mtk_efuse.ko
  DEPENDS:=@TARGET_mediatek_mt7622||TARGET_mediatek_mt7629
  AUTOLOAD:=$(call AutoLoad,81,mtk_efuse)
endef

define KernelPackage/mtk-efuse-nl-drv/description
  MT7622/MT7629 eFuse driver
endef

MAKE_OPTS:= \
        ARCH="$(LINUX_KARCH)" \
        CROSS_COMPILE="$(TARGET_CROSS)" \
        SUBDIRS="$(PKG_BUILD_DIR)" \

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,mtk-efuse-nl-drv))
