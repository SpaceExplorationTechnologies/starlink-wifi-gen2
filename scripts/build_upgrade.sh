#!/bin/bash -ex

# This script produces the upgradable artifact that we pass to our running
# software in order to perform an upgrade. It contains the BL2, a FIP (U-boot
# and ATF wrapper), and a FIT (The OpenWRT kernel image and Rootfs).
#
# Image Layout
# 0x00000000 BL2
# 0x00080000 FIP
# 0x00280000 OpenWRT FIT


cd "$(dirname "$0")/../bin"

#64K Blocks, 0x10000
BLOCK_SIZE=65536

# Generate a single image at path
OUT="$VERSION/v2-$VERSION-upgrade.bin"

# Pad to OpenWrt partition start - 0x280000
dd if=/dev/zero of=$OUT bs=$BLOCK_SIZE seek=0 count=40

# Write BL2 to 0x00000000 in output binary.
dd if="$VERSION/v2-$VERSION-bl2.img" of=$OUT bs=$BLOCK_SIZE seek=0 conv=notrunc

# Write FIP0 (contains the uboot) to 0x00080000 in output binary.
dd if="$VERSION/v2-$VERSION-fip.bin" of=$OUT bs=$BLOCK_SIZE seek=8 conv=notrunc

# Write OpenWRT (contains kernel and image) tp 0x00280000 in output binary.
dd if="$VERSION/v2-$VERSION-openwrt.img" of=$OUT bs=$BLOCK_SIZE seek=40

# Update convenience symlink.
ln -sf $OUT upgrade.bin
