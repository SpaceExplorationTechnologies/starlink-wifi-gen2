#
# Copyright (C) 2009 OpenWrt.org
#

SUBTARGET:=mt7621
BOARDNAME:=MT7621 based boards
FEATURES+=nand ramdisk rtc usb
CPU_TYPE:=24kc

KERNEL_PATCHVER:=4.4

define Target/Description
	Build firmware images for Ralink MT7621 based boards.
endef

