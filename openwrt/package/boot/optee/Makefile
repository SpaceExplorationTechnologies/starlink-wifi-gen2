#
# Copyright (C) 2020 Mediatek Ltd.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=optee
PKG_VERSION:=3.8.0
PKG_RELEASE:=3.8.0

include $(INCLUDE_DIR)/package.mk

define Package/optee
  SECTION:=boot
  CATEGORY:=Boot Loaders
  SUBMENU:=Trusted Execution Environment
  TITLE:=optee
endef

define Package/optee/description
 this package contain optee_os, optee_client and optee_test.
 you can reference https://optee.readthedocs.io/en/latest/building/index.html
 to get detailed information about how to build and run the OPTEE.
endef

OPTEE_OS_NAME:=optee_os
OPTEE_OS_RELEASE:=3.8.0
OPTEE_OS_SOURCE=$(OPTEE_OS_NAME)-$(OPTEE_OS_RELEASE).tar.bz2

define Download/optee-os
  FILE:=$(OPTEE_OS_SOURCE)
  PROTO:=git
  URL:=http://gerrit.mediatek.inc:8080/gateway/security/optee_os
  VERSION:=mt7622-mp_branch
  SUBDIR:=$(OPTEE_OS_NAME)
endef

OPTEE_CLIENT_NAME:=optee_client
OPTEE_CLIENT_RELEASE:=3.8.0
OPTEE_CLIENT_SOURCE=$(OPTEE_CLIENT_NAME)-$(OPTEE_CLIENT_RELEASE).tar.bz2

define Download/optee-client
  FILE:=$(OPTEE_CLIENT_SOURCE)
  PROTO:=git
  URL:=http://gerrit.mediatek.inc:8080/gateway/security/optee_client
  VERSION:=master
  SUBDIR:=$(OPTEE_CLIENT_NAME)
endef

OPTEE_TEST_NAME:=optee_test
OPTEE_TEST_RELEASE:=3.8.0
OPTEE_TEST_SOURCE=$(OPTEE_TEST_NAME)-$(OPTEE_TEST_RELEASE).tar.bz2

define Download/optee-test
  FILE:=$(OPTEE_TEST_SOURCE)
  PROTO:=git
  URL:=http://gerrit.mediatek.inc:8080/gateway/security/optee_test
  VERSION:=master
  SUBDIR:=$(OPTEE_TEST_NAME)
endef

define Build/Prepare
	[ -d $(PKG_BUILD_DIR) ] && rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)

	$(eval $(call Download,optee-os))
	$(eval $(call Download,optee-client))
	$(eval $(call Download,optee-test))

	$(TAR) -C $(PKG_BUILD_DIR) -xf $(DL_DIR)/$(OPTEE_OS_SOURCE)
	$(TAR) -C $(PKG_BUILD_DIR) -xf $(DL_DIR)/$(OPTEE_CLIENT_SOURCE)
	$(TAR) -C $(PKG_BUILD_DIR) -xf $(DL_DIR)/$(OPTEE_TEST_SOURCE)
endef

MAKE_VARS:= \
	CROSS_COMPILE=aarch64-linux-gnu- \
	CROSS_COMPILE_core=aarch64-linux-gnu- \
	CROSS_COMPILE_ta_arm32=arm-linux-gnueabihf- \
	CROSS_COMPILE_ta_arm64=aarch64-linux-gnu-

define Build/Compile
	$(MAKE_VARS) \
	$(MAKE) -C $(PKG_BUILD_DIR)/$(OPTEE_OS_NAME) \
		CFG_ARM64_core=y \
		CFG_TEE_BENCHMARK=n \
		O=out/arm \
		PLATFORM=mediatek-mt7622 \
		all

	$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/$(OPTEE_CLIENT_NAME) \
		CFG_TEE_BENCHMARK=n \
		CFG_TA_TEST_PATH=y \
		EXPORT_DIR=$(PKG_BUILD_DIR)/target \
		O=out \
		all

	$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/$(OPTEE_TEST_NAME) \
		TA_DEV_KIT_DIR=$(PKG_BUILD_DIR)/optee_os/out/arm/export-ta_arm64 \
		OPTEE_CLIENT_EXPORT=$(PKG_BUILD_DIR)/target/usr \
		DESTDIR=$(PKG_BUILD_DIR)/target/usr \
		O=$(PKG_BUILD_DIR)/$(OPTEE_TEST_NAME)/out \
		all install
endef

define Build/Install
	mkdir -p $(PKG_BUILD_DIR)/target/lib
	mv $(PKG_BUILD_DIR)/target/usr/lib/optee_armtz $(PKG_BUILD_DIR)/target/lib/
	mv $(PKG_BUILD_DIR)/target/usr/sbin/tee-supplicant $(PKG_BUILD_DIR)/target/usr/bin/
endef

define Package/optee/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/target/* $(1)/
	$(CP) ./files/optee.init $(1)/etc/init.d/optee
endef

$(eval $(call BuildPackage,optee))
