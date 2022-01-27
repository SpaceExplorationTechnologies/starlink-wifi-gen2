#
# Copyright (C) 2009 OpenWrt.org
#

SUBTARGET:=mt7623
BOARDNAME:=MT7623 based boards
FEATURES+=squashfs
CPU_TYPE:=cortex-a7
CPU_SUBTYPE:=neon-vfpv4


KERNEL_PATCHVER:=4.4

define Target/Description
	Build images for MediaTek MT7623 based boards.
endef

