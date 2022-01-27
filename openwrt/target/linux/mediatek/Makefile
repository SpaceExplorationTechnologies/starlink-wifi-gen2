# Copyright (c) 2015 OpenWrt.org
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=mediatek
BOARDNAME:=MediaTek ARM SoC
SUBTARGETS:=mt7622 mt7623 mt7629
FEATURES:=squashfs nand ubifs
MAINTAINER:=John Crispin <john@phrozen.org>

KERNEL_PATCHVER:=4.4
KERNEL_REVISION:=bfa1b1db

KERNELNAME:=Image dtbs 
#zImage


define Target/Description
	Build images for Mediatek ARM-basd SoC.
endef
include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	wpad-mini

$(eval $(call BuildTarget))
