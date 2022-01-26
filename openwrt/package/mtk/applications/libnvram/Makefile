#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnvram
PKG_REVISION:=9d068860
PKG_SOURCE:=$(PKG_NAME)-$(PKG_REVISION).tar.gz
PKG_VERSION:=0.9.28

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/libnvram
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=mtk nvram library
endef

define Package/libnvranm/description
  mtk nvram library
endef


MAKE_FLAGS += CONFIG_RALINKAPP_USER_NVRAMD=y
TARGET_CFLAGS += $(FPIC) -DCONFIG_RALINKAPP_USER_NVRAMD -DUSE_HOSTCC

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(MAKE_FLAGS) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		shared
endef

define Build/InstallDev
	$(INSTALL_DIR)  $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/flash_api.h  $(1)/usr/include/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/linux_socket.h $(1)/usr/include/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/nvram{,_env,_sock}.h  $(1)/usr/include/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/sys_debug.h  $(1)/usr/include/
	$(INSTALL_DIR)  $(1)/usr/lib
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/libnvram.a   $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libnvram.so* $(1)/usr/lib/
endef

define Package/libnvram/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libnvram.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libnvram))
