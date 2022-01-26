#
# Copyright (C) 2006-2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=crypto-hw-eip93
PKG_RELEASE:=0.8

include $(INCLUDE_DIR)/package.mk

define Package/crypto-hw-eip93/Default
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Cryptographic API modules
endef

define KernelPackage/crypto-hw-eip93
  $(call Package/crypto-hw-eip93/Default)
  SECTION:=kernel
  DEPENDS:=@TARGET_ramips_mt7621 +kmod-crypto-authenc +kmod-crypto-des
  TITLE:=Mediatek EIP93 crypto engine.
  FILES:=$(PKG_BUILD_DIR)/crypto-hw-eip93.ko
  AUTOLOAD:=$(call AutoProbe,crypto-hw-eip93)
endef

define KernelPackage/crypto-hw-eip93/description
Asynchronous Kernel module to enable EIP-93 Crypto Engine
in the Mediatek MT7621 SoC.
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
        CROSS_COMPILE="$(TARGET_CROSS)" \
        ARCH="$(LINUX_KARCH)" \
        SUBDIRS="$(PKG_BUILD_DIR)" \
        EXTRA_CFLAGS="$(BUILDFLAGS)" \
        modules
endef

$(eval $(call KernelPackage,crypto-hw-eip93))
