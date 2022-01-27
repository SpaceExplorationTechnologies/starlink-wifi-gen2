#
# Copyright (C) 2016 MediaTek
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=wifi-profile
PKG_RELEASE:=1
PKG_BUILD_DEPENDS:=
PKG_FILE_DEPENDS:=
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/wifi-profile
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  DEPENDS:=
  TITLE:=Build WiFi l1profile.data on demand & install wifi l2 profile.
  VERSION:=$(PKG_RELEASE)-$(REVISION)
  MENU:=1
endef

define Package/wifi-profile/description
  This package helps to build l1profile on demand & install wifi l2 profile.
endef

define Package/wifi-profile/config
	if PACKAGE_wifi-profile
	source "$(SOURCE)/l1profile.config.in"
	endif
endef

define Build/Prepare
endef

define Build/Compile
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	python ./make-l1profile.py $(TOPDIR)/.config /dev/null
endef

define Package/wifi-profile/install
	$(INSTALL_DIR) $(1)/etc/wireless/
	python ./make-l1profile.py $(TOPDIR)/.config $(1)/etc/wireless/l1profile.dat > /dev/null

	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/lib/wifi/
	$(INSTALL_DIR) $(1)/etc/wireless/mediatek/
	$(INSTALL_BIN) ./files/mt7615e-sku.dat $(1)/etc/wireless/mediatek/
	$(INSTALL_BIN) ./files/mt7615e-sku-bf.dat $(1)/etc/wireless/mediatek/
	echo $(PKG_VERSION) > $(1)/etc/wireless/mediatek/version
	$(INSTALL_BIN) ./files/mt7615e*.lua $(1)/lib/wifi/
	$(INSTALL_BIN) ./files/wifi_services.lua $(1)/lib/wifi/
	$(INSTALL_BIN) ./files/firmware.sh $(1)/etc/init.d/
	if [ "$$(CONFIG_WIFI_QUICK_SETTING)" = "y" ] ; then \
		$(INSTALL_BIN) ./files/quick_setting.lua $(1)/lib/wifi/; \
	fi

	# Example combination mt7629.
	if [ "$$(CONFIG_first_card_name)" = "MT7629" ] ; then \
		$(INSTALL_BIN) ./files/mt7629/*7629*.* $(1)/etc/wireless/mediatek/; \
		if [ "$$(__CONFIG_MTK_MAP_SUPPORT)" = "y" ] ; then \
			$(INSTALL_BIN) ./files/mt7629/map.1.b0.2g.dat $(1)/etc/wireless/mediatek/mt7629.1.b0.2g.dat; \
			$(INSTALL_BIN) ./files/mt7629/map.1.b1.5g.dat $(1)/etc/wireless/mediatek/mt7629.1.b1.5g.dat; \
		fi; \
	fi

	if [ "$$(CONFIG_first_card_name)" = "MT7626" ] ; then \
		$(INSTALL_BIN) ./files/mt7629/*7626*.* $(1)/etc/wireless/mediatek/; \
	fi

	# Example combination mt7622 + mt7615e/a + mt7615e/a.
	if [ "$$(CONFIG_first_card_name)" = "MT7622" ] ; then \
		$(INSTALL_BIN) ./files/mt7622.1.dat $(1)/etc/wireless/mediatek/; \
		if [ "$$(CONFIG_MTK_MAP_SUPPORT)" = "y"  ] ; then \
			$(INSTALL_BIN) ./files/mt7622_map.1.dat $(1)/etc/wireless/mediatek/mt7622.1.dat; \
		fi; \
		if [ "$$(CONFIG_second_card_name)" = "MT7615" ] ; then \
			$(INSTALL_BIN) ./files/mt7615e.1.dat $(1)/etc/wireless/mediatek/; \
			if [ "$$(CONFIG_MTK_MAP_SUPPORT)" = "y"  ] ; then \
                        	$(INSTALL_BIN) ./files/mt7615e_map.1.dat $(1)/etc/wireless/mediatek/mt7615e.1.dat; \
                	fi; \
			$(INSTALL_BIN) ./files/mt7615a.b0.5g.dat $(1)/etc/wireless/mediatek/mt7615a.1.b0.5g.dat; \
			$(INSTALL_BIN) ./files/mt7615a.b1.5g.dat $(1)/etc/wireless/mediatek/mt7615a.1.b1.5g.dat; \
		fi; \
		if [ "$$(CONFIG_third_card_name)" = "MT7615" ] ; then \
			$(INSTALL_BIN) ./files/mt7615e.2.dat $(1)/etc/wireless/mediatek/; \
			if [ "$$(CONFIG_MTK_MAP_SUPPORT)" = "y"  ] ; then \
                        	$(INSTALL_BIN) ./files/mt7615e_map.2.dat $(1)/etc/wireless/mediatek/mt7615e.2.dat; \
                	fi; \
			$(INSTALL_BIN) ./files/mt7615a.b0.5g.dat $(1)/etc/wireless/mediatek/mt7615a.2.b0.5g.dat; \
			$(INSTALL_BIN) ./files/mt7615a.b1.5g.dat $(1)/etc/wireless/mediatek/mt7615a.2.b1.5g.dat; \
		fi; \
		$(INSTALL_BIN) ./files/mt7615e*.bin $(1)/etc/wireless/mediatek/; \
	fi

	# Example combination mt7623 + mt7615e/a + mt7615e/a.
	if [ "$$(CONFIG_first_card_name)" = "MT7615" ] ; then \
		$(INSTALL_BIN) ./files/mt7615e.1.dat $(1)/etc/wireless/mediatek/; \
		$(INSTALL_BIN) ./files/mt7615a.b0.5g.dat $(1)/etc/wireless/mediatek/mt7615a.1.b0.5g.dat; \
		$(INSTALL_BIN) ./files/mt7615a.b1.5g.dat $(1)/etc/wireless/mediatek/mt7615a.1.b1.5g.dat; \
		if [ "$$(CONFIG_second_card_name)" = "MT7615" ] ; then \
			$(INSTALL_BIN) ./files/mt7615e.2.dat $(1)/etc/wireless/mediatek/; \
			$(INSTALL_BIN) ./files/mt7615a.b0.5g.dat $(1)/etc/wireless/mediatek/mt7615a.2.b0.5g.dat; \
			$(INSTALL_BIN) ./files/mt7615a.b1.5g.dat $(1)/etc/wireless/mediatek/mt7615a.2.b1.5g.dat; \
		fi; \
		$(INSTALL_BIN) ./files/mt7615e*.bin $(1)/etc/wireless/mediatek/; \
	fi

	# Example combination mt7622 + axe.
	if [ "$$(CONFIG_first_card_name)" = "AXE" ] ; then \
		$(INSTALL_BIN) ./files/axe.1.dat $(1)/etc/wireless/mediatek/; \
	fi

	# Example combination mt7622 + harrier.
	if [ "$$(CONFIG_first_card_name)" = "MT7915" -o "$$(CONFIG_second_card_name)" = "MT7915" \
		-o "$$(CONFIG_third_card_name)" = "MT7915" ] ; then \
		$(INSTALL_BIN) ./files/mt7915/mt7915.*.dat $(1)/etc/wireless/mediatek/; \
	fi

	# Example mt7615D
	if [ "$$(CONFIG_first_card_name)" = "MT7615D" -o "$$(CONFIG_second_card_name)" = "MT7615D" ] ; then \
		$(INSTALL_BIN) ./files/mt7615D/*.* $(1)/etc/wireless/mediatek/; \
	fi
	# Example mt7613/mt7663
	if [ "$$(CONFIG_first_card_name)" = "MT7663" -o "$$(CONFIG_second_card_name)" = "MT7663" ] ; then \
		$(INSTALL_BIN) ./files/mt7663/*.* $(1)/etc/wireless/mediatek/; \
	fi

	$(INSTALL_DIR) $(1)/etc/rc.button
	$(INSTALL_BIN) ./files/wps-hotplug.sh $(1)/etc/rc.button/wps
endef

$(eval $(call BuildPackage,wifi-profile))
