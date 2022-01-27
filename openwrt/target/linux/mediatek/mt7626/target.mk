#
# Copyright (C) 2009 OpenWrt.org
#
SUBTARGET:=mt7626
BOARDNAME:=MT7626 based boards
FEATURES+=ramdisk
CPU_TYPE:=cortex-a7

KERNEL_PATCHVER:=4.4

define Target/Description
	Build images for MediaTek MT7626 based boards.
endef

