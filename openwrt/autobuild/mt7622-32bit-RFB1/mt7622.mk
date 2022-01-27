#FIT loadaddr
KERNEL_LOADADDR := 0x40008000

define Device/MTK-AC2600-RFB1
  DEVICE_TITLE := MTK7622 ac2600 rfb1 AP
  DEVICE_DTS := mt7622-ac2600rfb1
  SUPPORTED_DEVICES := mt7622
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
			kmod-ata-core kmod-ata-ahci-mtk \
			wireless-tools block-mount luci luci-app-mtk luci-app-samba \
			ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
			kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AC2600-RFB1

define Device/MTK-AC4300-RFB1
  DEVICE_TITLE := MTK7622 ac4300 rfb1 AP
  DEVICE_DTS := mt7622-ac4300rfb1
  SUPPORTED_DEVICES := mt7622
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
			wireless-tools block-mount luci luci-app-mtk luci-app-samba \
			ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
			kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AC4300-RFB1

define Device/MTK-AC4300-DEV
  DEVICE_TITLE := MTK7622 ac4300 DEV version
  DEVICE_DTS := mt7622-ac4300-dev
  SUPPORTED_DEVICES := mt7622
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb-storage kmod-usb2 kmod-usb3 \
			wireless-tools block-mount luci luci-app-mtk luci-app-samba \
			ppp-mod-pppol2tp ppp-mod-pptp switch qdma \
			kmod-sdhci-mtk
endef
TARGET_DEVICES += MTK-AC4300-DEV

