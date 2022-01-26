#
# Copyright (C) 2007-2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=alexa
PKG_VERSION:=1
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_BUILD_DEPENDS:=curl gtest portaudio
ALEXA_SDK:=avs-device-sdk-master

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk


define Package/alexa
  SECTION:=sound
  CATEGORY:=Sound
  TITLE:=Alexa Voice Service(AVS)
  URL:=
  DEPENDS:=+libpthread +libcurl +glib2 +libnghttp2 +liboil +libgstreamer1 +libgst1app +gstreamer1-plugins-base +libstdcpp +libopenssl +libsqlite3 +libmbedtls +alsa-lib
endef

CMAKE_FLAGS += -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_BUILD_TYPE=Debug \
	       -DCMAKE_C_COMPILER=$(CMAKE_C_COMPILER) -DCMAKE_CXX_COMPILER=$(CMAKE_CXX_COMPILER) \
	       -DGTEST_LIBRARY=$(STAGING_DIR)/usr/lib/libgtestd.a \
	       -DGTEST_MAIN_LIBRARY=$(STAGING_DIR)/usr/lib/libgtest_maind.a \
	       -DCURL_LIBRARY=$(STAGING_DIR)/usr/lib/libcurl.a -DCURL_INCLUDE_DIR=$(STAGING_DIR)/usr/include/curl \
	       -DPORTAUDIO=ON -DPORTAUDIO_LIB_PATH=$(STAGING_DIR)/usr/lib/libportaudio.a \
	       -DPORTAUDIO_INCLUDE_DIR=$(STAGING_DIR)/usr/include \
	       -DGSTREAMER_MEDIA_PLAYER=ON \
	       -DGSTREAMER_INCLUDE_DIR=$(STAGING_DIR)/usr/include/gstreamer-1.0 \
	       -DGLIB_INCLUDE_DIR=$(STAGING_DIR)/usr/include/glib-2.0 \
	       -DMBEDTLS_INCLUDE_DIR=$(STAGING_DIR)/usr/include/mbedtls \
	       -DNGHTTP2_INCLUDE_DIR=$(STAGING_DIR)/usr/include/nghttp2 \
	       -DLINK_DIR=$(STAGING_DIR)/usr/lib \
	       -DMBEDCRYPTO_LIBRARY=$(STAGING_DIR)/usr/lib/libmbedcrypto.a \
	       -DMBEDTLS_LIBRARY=$(STAGING_DIR)/usr/lib/libmbedtls.a \
	       -DMBEDX509_LIBRARY=$(STAGING_DIR)/usr/lib/libmbedx509.a \
	       -DNGHTTP2_LIBRARY=$(STAGING_DIR)/usr/lib/libnghttp2.so \

define Build/Prepare
	$(CP) $(TOPDIR)/package/utils/alexa/src/* $(PKG_BUILD_DIR)
	cd $(PKG_BUILD_DIR) && cmake $(ALEXA_SDK) $(CMAKE_FLAGS)
endef

define Package/alexa/description
 Alexa voice assistance for AVS.
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)
endef

define Package/alexa/install
	$(INSTALL_DIR) $(1)/alexa $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ipkg-install/usr/local/lib/*.so $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/SampleApp/src/SampleApp $(1)/alexa
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/Integration/AlexaClientSDKConfig.json $(1)/alexa
	$(INSTALL_BIN) ./files/AlexaClientSDKConfig.json $(1)/alexa
endef

$(eval $(call BuildPackage,alexa))
