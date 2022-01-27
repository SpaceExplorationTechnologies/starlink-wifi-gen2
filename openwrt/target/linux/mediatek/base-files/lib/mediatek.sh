#!/bin/sh
#
# Copyright (C) 2016 OpenWrt.org
#

mediatek_board_detect() {
	local machine
	local name

	machine=$(cat /proc/device-tree/model)

	case "$machine" in
	"MediaTek MT7623 evaluation board")
		name="mt7623"
		;;
	"MediaTek MT7623a eMMC evaluation board")
		name="mt7623"
		;;
	"MediaTek MT7623a NAND evaluation board")
		name="mt7623"
		;;
	"MediaTek MT7623n eMMC+Ephy board")
		name="mt7623-gphy"
		;;
	"MediaTek MT7622 AC2600rfb1 board")
		name="mt7622"
		;;
	"MediaTek MT7622 AC4300rfb1 board")
		name="mt7622"
		;;
	"Mediatek MT7622 AC4300-gmac1 board")
		name="mt7622-gmac1_only"
		;;
	"MediaTek MT7622 AX3600 board")
		name="mt7622-ax"
		;;
	"MediaTek MT7622 AX3600-gmac1 board")
		name="mt7622-ax-gmac1_only"
		;;
	"MediaTek MT7629 RFB1 board")
		name="mt7629"
		;;
	esac

	[ -z "$name" ] && name="unknown"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$name" > /tmp/sysinfo/board_name
	echo "$machine" > /tmp/sysinfo/model
}

mediatek_board_name() {
	local name

	[ -f /tmp/sysinfo/board_name ] && name=$(cat /tmp/sysinfo/board_name)
	[ -z "$name" ] && name="unknown"

	echo "$name"
}
