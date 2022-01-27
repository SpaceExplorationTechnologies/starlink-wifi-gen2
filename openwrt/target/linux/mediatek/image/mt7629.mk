#FIT loadaddr
KERNEL_LOADADDR := 0x40008000

#define Device/MTK-7629-FPGA
#  DEVICE_TITLE := MTK7629 FPGA
#  DEVICE_DTS := mt7629-fpga
#  DEVICE_PACKAGES := regs
#endef

#TARGET_DEVICES += MTK-7629-FPGA

define Device/MTK-7629-EVBv10
  DEVICE_TITLE := MTK7629 EVB v10
  DEVICE_DTS := mt7629-raeth-evb
  DEVICE_PACKAGES := regs luci ppp-mod-pptp xl2tpd wireless-tools \
                luci-app-mtk luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4
endef

TARGET_DEVICES += MTK-7629-EVBv10

define Device/MTK-7629-RAETHv10
  DEVICE_TITLE := MTK7629 RAETH RFBv10
  DEVICE_DTS := mt7629-raeth-rfb1v10
  DEVICE_PACKAGES := regs luci ppp-mod-pptp xl2tpd wireless-tools \
                luci-app-mtk luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4
endef

TARGET_DEVICES += MTK-7629-RAETHv10

define Device/MTK-7629-RAETH
  DEVICE_TITLE := MTK7629 RAETH RFB
  DEVICE_DTS := mt7629-raeth-rfb1
  DEVICE_PACKAGES := regs luci ppp-mod-pptp xl2tpd wireless-tools \
                luci-app-mtk luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4
endef

TARGET_DEVICES += MTK-7629-RAETH

define Device/MTK-7629-RFBv10
  DEVICE_TITLE := MTK7629 RFBv10
  DEVICE_DTS := mt7629-rfb1-v10
  DEVICE_PACKAGES := regs luci ppp-mod-pptp xl2tpd wireless-tools \
		luci-app-mtk luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4
endef

TARGET_DEVICES += MTK-7629-RFBv10

define Device/MTK-7629-RFB
  DEVICE_TITLE := MTK7629 RFB
  DEVICE_DTS := mt7629-rfb1
  DEVICE_PACKAGES := regs luci ppp-mod-pptp xl2tpd wireless-tools \
                luci-app-mtk luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4
endef

TARGET_DEVICES += MTK-7629-RFB

define Device/MT7629-MT7531-EVB1
  DEVICE_TITLE := MTK7629 MT7531 EVB1
  DEVICE_DTS := mt7629-lynx-evb1
  DEVICE_PACKAGES := regs luci ppp-mod-pptp xl2tpd wireless-tools \
                luci-app-mtk luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4
endef

TARGET_DEVICES += MT7629-MT7531-EVB1

define Device/MT7629-MT7531-EVB1-NMBM
  DEVICE_TITLE := MTK7629 MT7531 EVB1 (NMBM enabled)
  DEVICE_DTS := mt7629-lynx-evb1-nmbm
  DEVICE_PACKAGES := regs luci ppp-mod-pptp xl2tpd wireless-tools \
                luci-app-mtk luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4
endef

TARGET_DEVICES += MT7629-MT7531-EVB1-NMBM

define Device/MT7629-MT7531-EVB2
  DEVICE_TITLE := MTK7629 MT7531 EVB2
  DEVICE_DTS := mt7629-lynx-evb2
  DEVICE_PACKAGES := regs luci ppp-mod-pptp xl2tpd wireless-tools \
                luci-app-mtk luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4
endef

TARGET_DEVICES += MT7629-MT7531-EVB2

define Device/MT7629-MT7531-EVB2-NMBM
  DEVICE_TITLE := MTK7629 MT7531 EVB2 (NMBM enabled)
  DEVICE_DTS := mt7629-lynx-evb2-nmbm
  DEVICE_PACKAGES := regs luci ppp-mod-pptp xl2tpd wireless-tools \
                luci-app-mtk luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4
endef

TARGET_DEVICES += MT7629-MT7531-EVB2-NMBM

define Device/MT7629-MT7531-RFB3
  DEVICE_TITLE := MTK7629 MT7531 RFB3
  DEVICE_DTS := mt7629-lynx-rfb3
  DEVICE_PACKAGES := regs luci ppp-mod-pptp xl2tpd wireless-tools \
                luci-app-mtk luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4
endef

TARGET_DEVICES += MT7629-MT7531-RFB3

define Device/MT7629-MT7531-RFB3-NMBM
  DEVICE_TITLE := MTK7629 MT7531 RFB3 (NMBM enabled)
  DEVICE_DTS := mt7629-lynx-rfb3-nmbm
  DEVICE_PACKAGES := regs luci ppp-mod-pptp xl2tpd wireless-tools \
                luci-app-mtk luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4
endef

TARGET_DEVICES += MT7629-MT7531-RFB3-NMBM

define Device/MT7629-MT7531-RFB3-NMBM-SB
  DEVICE_TITLE := MTK7629 MT7531 RFB3 SECUREBOOT (NMBM enabled)
  DEVICE_DTS := mt7629-lynx-rfb3-nmbm-sb
  DEVICE_PACKAGES := regs luci ppp-mod-pptp xl2tpd wireless-tools \
                luci-app-mtk luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4
  FIT_KEY_DIR := $(TOPDIR)/../keys
  FIT_KEY_NAME := fit_key
  AR_TABLE_XML := $(TOPDIR)/../../ar_table.xml
  AUTO_AR_CONF := $(TOPDIR)/../../auto_ar_conf.mk
  HASHED_BOOT_DEVICE := /dev/mtdblock8
  BASIC_KERNEL_CMDLINE := console=ttyS0,115200n1 rootfstype=squashfs loglevel=8
  KERNEL = dtb | kernel-bin | lzma | squashfs-hashed | fit-ar-ver | fit-sign lzma $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
  IMAGE/sysupgrade.bin := append-kernel | pad-to 128k | append-squashfs-hashed | pad-rootfs | append-metadata
endef

TARGET_DEVICES += MT7629-MT7531-RFB3-NMBM-SB

define Device/SPACEX-WIFI-V2
  DEVICE_TITLE := SPACEX WIFI V2
  DEVICE_DTS_DEV_0 := spacex-v2-dev-0
  DEVICE_DTS_DEV_1 := spacex-v2-dev-1
  DEVICE_DTS_PROD_0 := spacex-v2-prod-0
  DEVICE_DTS_PROD_1 := spacex-v2-prod-1
  DEVICE_PACKAGES := regs luci ppp-mod-pptp xl2tpd wireless-tools \
                luci-app-mtk luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4
  FIT_KEY_DIR := $(TOPDIR)/../keys
  FIT_KEY_NAME := fit_key
  AR_TABLE_XML := $(TOPDIR)/../../ar_table.xml
  AUTO_AR_CONF := $(TOPDIR)/../../auto_ar_conf.mk
  HASHED_BOOT_DEVICE_0 := /dev/mtdblock8
  HASHED_BOOT_DEVICE_1 := /dev/mtdblock9
  BASIC_KERNEL_CMDLINE := console=ttyS0,115200n1 rootfstype=squashfs loglevel=8
  KERNEL = dtb-dev0-spacex | dtb-dev1-spacex | dtb-prod0-spacex | dtb-prod1-spacex | kernel-bin | lzma | squashfs-hashed-spacex | fit-ar-ver | fit-sign-spacex lzma $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS_DEV_0).dtb $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS_DEV_1).dtb $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS_PROD_0).dtb $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS_PROD_1).dtb
  IMAGE/sysupgrade.bin := append-kernel | pad-to 128k | append-squashfs-hashed | pad-rootfs | append-metadata
endef

TARGET_DEVICES += SPACEX-WIFI-V2

#install vmlinux for initial develop stage
define Image/Build/Initramfs
	cp $(KDIR)/vmlinux.debug $(BIN_DIR)/$(IMG_PREFIX)-vmlinux
	cp $(KDIR)/vmlinux-initramfs $(BIN_DIR)/$(IMG_PREFIX)-Image-initramfs
endef

