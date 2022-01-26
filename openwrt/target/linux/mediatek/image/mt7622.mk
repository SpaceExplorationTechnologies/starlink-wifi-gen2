#FIT loadaddr
KERNEL_LOADADDR := 0x41080000

define Device/MTK-AC2600-MT7531
  DEVICE_TITLE := MTK7622 ac2600 mt7531 RFB
  DEVICE_DTS := mt7622-mt7531-ac2600
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
                        wireless-tools block-mount luci luci-app-mtk luci-app-samba \
                        ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
                        kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AC2600-MT7531

define Device/MTK-AC2600-RFB1
  DEVICE_TITLE := MTK7622 ac2600 RFB1 AP
  DEVICE_DTS := mt7622-ac2600
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
			kmod-ata-core kmod-ata-ahci-mtk \
			wireless-tools block-mount luci luci-app-mtk luci-app-samba \
			ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
			kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AC2600-RFB1

define Device/MTK-AC2600-Raeth
  DEVICE_TITLE := MTK7622 ac2600 Raeth version
  DEVICE_DTS := mt7622-ac2600-raeth
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
                        wireless-tools block-mount luci luci-app-mtk luci-app-samba \
                        ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
                        kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AC2600-Raeth

define Device/MTK-AC4300-MT7531
  DEVICE_TITLE := MTK7622 ac4300 mt7531 RFB
  DEVICE_DTS := mt7622-mt7531-ac4300
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
                        wireless-tools block-mount luci luci-app-mtk luci-app-samba \
                        ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
                        kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AC4300-MT7531

define Device/MTK-AC4300-MT7531-GMAC1
  DEVICE_TITLE := MTK7622 ac4300 mt7531 GMAC1
  DEVICE_DTS := mt7622-mt7531-ac4300-gmac1
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
                        wireless-tools block-mount luci luci-app-mtk luci-app-samba \
                        ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
                        kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AC4300-MT7531-GMAC1

define Device/MTK-AC4300-MT7531-SB
  DEVICE_TITLE := MTK7622 ac4300 mt7531 RFB SECUREBOOT
  DEVICE_DTS := mt7622-mt7531-ac4300-sb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
                        wireless-tools block-mount luci luci-app-mtk luci-app-samba \
                        ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
                        kmod-sdhci-mtk
  FIT_KEY_DIR := $(TOPDIR)/../../keys
  FIT_KEY_NAME := fit_key
  AR_TABLE_XML := $(TOPDIR)/../../ar_table.xml
  AUTO_AR_CONF := $(TOPDIR)/../../auto_ar_conf.mk
  HASHED_BOOT_DEVICE := /dev/mtdblock6
  BASIC_KERNEL_CMDLINE := console=ttyS0,115200n1 rootfstype=squashfs loglevel=8
  KERNEL = dtb | kernel-bin | lzma | squashfs-hashed | fit-ar-ver | fit-sign lzma $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
  IMAGE/sysupgrade.bin := append-kernel | pad-to 128k | append-squashfs-hashed | pad-rootfs | append-metadata
endef
TARGET_DEVICES += MTK-AC4300-MT7531-SB

define Device/MTK-AC4300-MT7531-NMBM-SB
  DEVICE_TITLE := MTK7622 ac4300 mt7531 RFB SECUREBOOT (NMBM enabled)
  DEVICE_DTS := mt7622-mt7531-ac4300-nmbm-sb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
                        wireless-tools block-mount luci luci-app-mtk luci-app-samba \
                        ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
                        kmod-sdhci-mtk
  FIT_KEY_DIR := $(TOPDIR)/../../keys
  FIT_KEY_NAME := fit_key
  AR_TABLE_XML := $(TOPDIR)/../../ar_table.xml
  AUTO_AR_CONF := $(TOPDIR)/../../auto_ar_conf.mk
  HASHED_BOOT_DEVICE := /dev/mtdblock7
  BASIC_KERNEL_CMDLINE := console=ttyS0,115200n1 rootfstype=squashfs loglevel=8
  KERNEL = dtb | kernel-bin | lzma | squashfs-hashed | fit-ar-ver | fit-sign lzma $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
  IMAGE/sysupgrade.bin := append-kernel | pad-to 128k | append-squashfs-hashed | pad-rootfs | append-metadata
endef
TARGET_DEVICES += MTK-AC4300-MT7531-NMBM-SB

define Device/MTK-AC4300-RFB1
  DEVICE_TITLE := MTK7622 ac4300 RFB1 AP
  DEVICE_DTS := mt7622-ac4300
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
			wireless-tools block-mount luci luci-app-mtk luci-app-samba \
			ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
			kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AC4300-RFB1

define Device/MTK-AC4300-Raeth
  DEVICE_TITLE := MTK7622 ac4300 Raeth version
  DEVICE_DTS := mt7622-ac4300-raeth
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
			wireless-tools block-mount luci luci-app-mtk luci-app-samba \
			ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
			kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AC4300-Raeth

define Device/MTK-AX3200
  DEVICE_TITLE := MTK7622 ax3200 AP
  DEVICE_DTS := mt7622-ax3200
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
                        wireless-tools block-mount luci luci-app-mtk luci-app-samba \
                        ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
                        kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AX3200

define Device/MTK-AX3200-MT7531
  DEVICE_TITLE := MTK7622 ax3200 mt7531 AP
  DEVICE_DTS := mt7622-mt7531-ax3200
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
                        wireless-tools block-mount luci luci-app-mtk luci-app-samba \
                        ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
                        kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AX3200-MT7531

define Device/MTK-AX3200-MT7531-GMAC1
  DEVICE_TITLE := MTK7622 ax3200 mt7531 GMAC1
  DEVICE_DTS := mt7622-mt7531-ax3200-gmac1
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
                        wireless-tools block-mount luci luci-app-mtk luci-app-samba \
                        ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
                        kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AX3200-MT7531-GMAC1

define Device/MTK-AX3600
  DEVICE_TITLE := MTK7622 ax3600 AP
  DEVICE_DTS := mt7622-ax3600
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
                        wireless-tools block-mount luci luci-app-mtk luci-app-samba \
                        ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
                        kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AX3600

define Device/MTK-AX3600-MT7531
  DEVICE_TITLE := MTK7622 ax3600 mt7531 AP
  DEVICE_DTS := mt7622-mt7531-ax3600
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
                        wireless-tools block-mount luci luci-app-mtk luci-app-samba \
                        ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
                        kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AX3600-MT7531

define Device/MTK-AX3600-MT7531-GMAC1
  DEVICE_TITLE := MTK7622 ax3600 mt7531 GMAC1
  DEVICE_DTS := mt7622-mt7531-ax3600-gmac1
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
                        wireless-tools block-mount luci luci-app-mtk luci-app-samba \
                        ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
                        kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AX3600-MT7531-GMAC1

define Device/MTK-AX4200
  DEVICE_TITLE := MTK7622 ax4200 AP
  DEVICE_DTS := mt7622-ax3600
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
                        wireless-tools block-mount luci luci-app-mtk luci-app-samba \
                        ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
                        kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AX4200

define Device/MTK-AX4200-MT7531
  DEVICE_TITLE := MTK7622 AX4200 mt7531 AP
  DEVICE_DTS := mt7622-mt7531-ax3600
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
                        wireless-tools block-mount luci luci-app-mtk luci-app-samba \
                        ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
                        kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AX4200-MT7531
