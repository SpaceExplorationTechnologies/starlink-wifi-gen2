include $(TOPDIR)/rules.mk

PKG_NAME:=trace-cmd
PKG_VERSION:=v2.8.3
PKG_RELEASE=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://git.kernel.org/pub/scm/utils/trace-cmd/trace-cmd.git/snapshot/$(PKG_SOURCE)
PKG_HASH:=706564b147bd966957431c4ca70032947d59c5d977c1ac2942b1b5a8668e7a6f
PKG_INSTALL:=1
PKG_USE_MIPS16:=0
PKG_LICENSE:=GPL-2.0

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/trace-cmd
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Linux trace command line utility
  DEPENDS:=+librt
endef

define Package/trace-cmd-extra
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Extra plugins for trace-cmd
  DEPENDS:=
endef

MAKE_FLAGS += \
	NO_PYTHON=1 \
	prefix=/usr

PLUGINS_DIR := $(PKG_INSTALL_DIR)/usr/lib/trace-cmd/plugins
PLUGINS_MAIN := function hrtimer mac80211 sched_switch

TARGET_CFLAGS += --std=gnu99 -D_GNU_SOURCE

define Package/trace-cmd/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib/trace-cmd/plugins
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/trace-cmd $(1)/usr/bin/
	$(CP) \
		$(patsubst %,$(PLUGINS_DIR)/plugin_%.so,$(PLUGINS_MAIN)) \
		$(1)/usr/lib/trace-cmd/plugins
endef

define Package/trace-cmd-extra/install
	$(INSTALL_DIR) $(1)/usr/lib/trace-cmd/plugins
	$(CP) \
		$$(patsubst %,$(PLUGINS_DIR)/plugin_%.so, \
			$$(filter-out $(PLUGINS_MAIN), \
				$$(patsubst $(PLUGINS_DIR)/plugin_%.so,%, \
					$$(wildcard $(PLUGINS_DIR)/plugin_*.so)))) \
		$(1)/usr/lib/trace-cmd/plugins
endef

$(eval $(call BuildPackage,trace-cmd))
$(eval $(call BuildPackage,trace-cmd-extra))
