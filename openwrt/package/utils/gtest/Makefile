#
# Copyright (C) 2007-2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gtest
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk


define Package/gtest
  SECTION:=sound
  CATEGORY:=Sound
  TITLE:=Google Testing(Gtest)
  DEPENDS:=
endef

CMAKE_FLAGS += -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_BUILD_TYPE=DEBUG
CMAKE_FLAGS += -DCMAKE_C_COMPILER=$(CMAKE_C_COMPILER) -DCMAKE_CXX_COMPILER=$(CMAKE_CXX_COMPILER)
CMAKE_FLAGS += -DCMAKE_FIND_ROOT_PATH=$(STAGING_DIR)/usr -DCMAKE_INSTALL_PREFIX=/usr

define Build/Prepare
	$(CP) $(TOPDIR)/package/utils/gtest/src/* $(PKG_BUILD_DIR)
	cd $(PKG_BUILD_DIR) && cmake $(CMAKE_FLAGS)
endef

define Package/gtest/description
 Google Testing and Mocking Framework.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Package/gtest/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/ipkg-install/usr/lib/libgtestd.a $(STAGING_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/ipkg-install/usr/lib/libgtest_maind.a $(STAGING_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/ipkg-install/usr/include/gtest $(STAGING_DIR)/usr/include

endef

$(eval $(call BuildPackage,gtest))

