#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=cryptsetup
PKG_VERSION:=2.0.6

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/cryptsetup/v$(PKG_VERSION_MAJOR)/
PKG_HASH:=7c51fae0f0e7ea9af0f515b2ac77009fb2969a6619ebab47d097dca38b083d30

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--with-crypto-backend=openssl \
	$(STAGING_DIR_HOST) \
	--disable-kernel_crypto \
	--disable-blkid

define Host/Prepare
	$(call Host/Prepare/Default)
	cp -fpR $(STAGING_DIR_HOST)/include/e2fsprogs/uuid/ $(STAGING_DIR_HOST)/include/
endef

$(eval $(call HostBuild))
