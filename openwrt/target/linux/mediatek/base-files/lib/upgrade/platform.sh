#
# Copyright (C) 2016 OpenWrt.org
#
# mediatek arm/arm64 kernel image partition name
PART_NAME=Kernel

platform_do_upgrade() {
	local board="$(cat /tmp/sysinfo/board_name)"
	##$ write image file to Kernel parttition directly ###
	case "$board" in
	*)
		echo "platform_do_upgrade here"
		default_do_upgrade "$ARGV"
		;;
	esac

	return 0
}

platform_check_image() {
	local img_file="$1"
	local board=$(cat /tmp/sysinfo/board_name)

	case "$board" in
	*)
		echo "platform_check_image here."
		;;
	esac

	return 0
}

platform_pre_upgrade() {
	echo "platform_pre_upgrade here."
}
