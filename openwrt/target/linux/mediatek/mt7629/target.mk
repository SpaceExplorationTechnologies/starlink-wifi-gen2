#
# Copyright (C) 2009 OpenWrt.org
#
SUBTARGET:=mt7629
BOARDNAME:=MT7629 based boards
FEATURES+=ramdisk
CPU_TYPE:=cortex-a7

KERNEL_PATCHVER:=4.4

define Target/Description
	Build images for MediaTek MT7629 based boards.
endef

