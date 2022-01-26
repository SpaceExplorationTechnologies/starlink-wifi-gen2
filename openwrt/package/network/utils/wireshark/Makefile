include $(TOPDIR)/rules.mk

PKG_NAME:=wireshark
PKG_VERSION:=2.4.3
PKG_RELEASE:=1
PKG_SOURCE:=wireshark-$(PKG_VERSION).tar.xz
PKG_BUILD_DIR:=$(BUILD_DIR)/wireshark-$(PKG_VERSION)
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk


define Package/wireshark
  SECTION:=net
  CATEGORY:=Network
  TITLE:=TShark can dump and analyze network traffic
  URL:=https://www.wireshark.org
  DEPENDS:=@USE_GLIBC +libgcrypt +glib2 +libpcap +zlib +libxml2
endef

define Package/wireshark/description
  TShark is a network protocol analyzer.
endef

CONFIGURE_ARGS += \
	--disable-wireshark \
	--disable-warnings-as-errors \
	--disable-usr-local \
	LIBS='-ldl' \
	--with-lua=no \
	--enable-udpdump=no

#define Build/Configure
#endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CC="$(TARGET_CC)"
endef


define Build/Install
	$(call Build/Install/Default,install)
endef


define Package/wireshark/install
	echo $(PKG_INSTALL_DIR)
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/man
	$(CP) $(PKG_INSTALL_DIR)/usr/man/* $(1)/usr/man/
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share/
endef

$(eval $(call BuildPackage,wireshark))
