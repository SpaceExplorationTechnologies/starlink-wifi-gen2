#
# Copyright (C) 2009 OpenWrt.org
#
ARCH:=aarch64
SUBTARGET:=mt7622
BOARDNAME:=MT7622 based boards
FEATURES+=usb rtc
CPU_TYPE:=cortex-a53
CPU_SUBTYPE:=neon-vfpv4


define Target/Description
	Build images for MediaTek MT7622 based boards.
endef

