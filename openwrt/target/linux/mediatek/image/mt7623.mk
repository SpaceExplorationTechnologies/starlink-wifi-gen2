#FIT loadaddr
KERNEL_LOADADDR := 0x80008000

define Device/7623a-nand
  DEVICE_TITLE := MTK7623a NAND AP
  DEVICE_DTS := mt7623a-evb
  DEVICE_PACKAGES := luci ppp-mod-pptp xl2tpd wireless-tools \
		luci-app-mtk switch qdma \
		kmod-usb-core kmod-usb-storage kmod-usb2 kmod-usb3 \
		luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4 \
		kmod-sdhci-mtk
endef

define Device/7623n-emmc
  DEVICE_TITLE := MTK7623n eMMC AP
  DEVICE_DTS := mt7623n-evb
  DEVICE_PACKAGES := luci ppp-mod-pptp xl2tpd wireless-tools \
		luci-app-mtk switch qdma \
		kmod-usb-core kmod-usb-storage kmod-usb2 kmod-usb3 \
		luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4 \
		kmod-sdhci-mtk
endef

TARGET_DEVICES += 7623a-nand 7623n-emmc

define Device/7623a-emmc
  DEVICE_TITLE := MTK7623a eMMC AP
  DEVICE_DTS := mt7623a-emmc-evb
  DEVICE_PACKAGES := luci ppp-mod-pptp xl2tpd wireless-tools \
		luci-app-mtk switch qdma \
		kmod-usb-core kmod-usb-storage kmod-usb2 kmod-usb3 \
		luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4 \
		kmod-sdhci-mtk
endef

define Device/7623n-emmc-gphy
  DEVICE_TITLE := MTK7623n eMMC Gphy AP
  DEVICE_DTS := mt7623n-emmc-gphy-evb
  DEVICE_PACKAGES := luci ppp-mod-pptp xl2tpd wireless-tools \
		luci-app-mtk switch qdma \
		kmod-usb-core kmod-usb-storage kmod-usb2 kmod-usb3 \
		luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4 \
		kmod-sdhci-mtk
endef

define Device/7623n-nand
  DEVICE_TITLE := MTK7623n NAND AP
  DEVICE_DTS := mt7623n-nand-evb
  DEVICE_PACKAGES := luci ppp-mod-pptp xl2tpd wireless-tools \
		luci-app-mtk switch qdma \
		kmod-usb-core kmod-usb-storage kmod-usb2 kmod-usb3 \
		luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4 \
		kmod-sdhci-mtk
endef

TARGET_DEVICES += 7623a-emmc 7623n-emmc-gphy 7623n-nand

