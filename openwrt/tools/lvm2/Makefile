#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=lvm2
PKG_VERSION:=2.02.180

PKG_SOURCE:=LVM2.$(PKG_VERSION).tgz
PKG_SOURCE_URL:=ftp://sources.redhat.com/pub/lvm2/old
PKG_HASH:=24997e26dfc916151707c9da504d38d0473bec3481a8230b676bc079041bead6

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--enable-write_install \
	--enable-pkgconfig \
	--disable-cmdlib \
	--disable-dmeventd \
	--disable-applib \
	--disable-fsadm \
	--disable-readline \
	--disable-selinux

define Host/Prepare
	$(call Host/Prepare/Default)
	mv $(HOST_BUILD_DIR)/../LVM2.$(PKG_VERSION)/* $(HOST_BUILD_DIR)/
	rmdir $(HOST_BUILD_DIR)/../LVM2.$(PKG_VERSION)
endef

define Host/Compile
	$(call Host/Compile/Default,device-mapper)
endef

define Host/Install
	$(call Host/Compile/Default,install_device-mapper)
endef

$(eval $(call HostBuild))
