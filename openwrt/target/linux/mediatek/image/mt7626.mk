#FIT loadaddr
KERNEL_LOADADDR := 0x40008000

#define Device/MTK-7626-FPGA
#  DEVICE_TITLE := MTK7626 FPGA
#  DEVICE_DTS := mt7626-fpga
#  DEVICE_PACKAGES := regs
#endef

#TARGET_DEVICES += MTK-7626-FPGA

define Device/MTK-7626-EVB
  DEVICE_TITLE := MTK7626 EVB
  DEVICE_DTS := mt7626-evb
  DEVICE_PACKAGES := regs
endef

TARGET_DEVICES += MTK-7626-EVB

define Device/MTK-7626-RFB1
  DEVICE_TITLE := MTK7626 RFB1
  DEVICE_DTS := mt7626-rfb1
  DEVICE_PACKAGES := regs
endef

TARGET_DEVICES += MTK-7626-RFB1

define Device/MTK-7626-RFB2
  DEVICE_TITLE := MTK7626 RFB2
  DEVICE_DTS := mt7626-rfb2
  DEVICE_PACKAGES := regs
endef

TARGET_DEVICES += MTK-7626-RFB2

define Device/MTK-7626-RFB1v11
  DEVICE_TITLE := MTK7626 RFB1(V11)
  DEVICE_DTS := mt7626-rfb1-v11
  DEVICE_PACKAGES := regs luci ppp-mod-pptp xl2tpd wireless-tools \
                luci-app-mtk luci-app-samba block-mount kmod-fs-vfat kmod-fs-ext4
endef

TARGET_DEVICES += MTK-7626-RFB1v11

define Device/MTK-7626-MT7531
  DEVICE_TITLE := MTK7626 MT7531 RFB board
  DEVICE_DTS := mt7626-lynx-rfb
  DEVICE_PACKAGES := regs
endef

TARGET_DEVICES += MTK-7626-MT7531

#install vmlinux for initial develop stage
define Image/Build/Initramfs
	cp $(KDIR)/vmlinux.debug $(BIN_DIR)/$(IMG_PREFIX)-vmlinux
	cp $(KDIR)/vmlinux-initramfs $(BIN_DIR)/$(IMG_PREFIX)-Image-initramfs
endef

