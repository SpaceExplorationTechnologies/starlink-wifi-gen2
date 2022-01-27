#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mtk-smc
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mtk-smc
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=MTK simple SMC driver
  SUBMENU:=Drivers
  FILES:=$(PKG_BUILD_DIR)/mtk-smc.ko
  AUTOLOAD:=$(call AutoLoad,81,mtk-smc,1)
  DEPENDS:=@TARGET_mediatek_mt7622
endef

define KernelPackage/mtk-smc/description
  MTK simple SMC driver
endef

MAKE_OPTS:= \
        ARCH="$(LINUX_KARCH)" \
        CROSS_COMPILE="$(TARGET_CROSS)" \
        SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,mtk-smc))
