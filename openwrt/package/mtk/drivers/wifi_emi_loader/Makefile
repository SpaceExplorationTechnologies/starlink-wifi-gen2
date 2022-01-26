# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=wifi_emi_loader
PKG_REVISION:=c29ba717
PKG_SOURCE:=$(PKG_NAME)-$(PKG_REVISION).tar.bz2
P4REV:=
PKG_VERSION:=
PKG_SOURCE_URL:=
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:= \
	MTK_EMI_CHIP_LEOPARD

include $(INCLUDE_DIR)/package.mk

ifeq ($(CONFIG_MTK_EMI_CHIP_LEOPARD),y)
CHIP = mt7626
else
CHIP = $(SUBTARGET)
endif

define KernelPackage/wifi_emi_loader
	CATEGORY:=MTK Properties
	TITLE:=MTK wifi EMI loader driver
	FILES:=$(PKG_BUILD_DIR)/wifi_emi_loader.ko
	AUTOLOAD:=$(call AutoLoad,10,wifi_emi_loader,1)
	SUBMENU:=Drivers
	MENU:=1
endef


define KernelPackage/wifi_emi_loader/config
if PACKAGE_kmod-wifi_emi_loader
	config MTK_EMI_CHIP_LEOPARD
	bool "Leopard Wifi EMI"
	default n
endif
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" V=1 \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		CHIP="$(CHIP)" \
		$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=$(CONFIG_$(c)))) \
		modules
endef

$(eval $(call KernelPackage,wifi_emi_loader))
