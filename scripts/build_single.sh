#!/bin/bash -ex

# The single image is a binary blob that can be written to raw NAND on our
# router, starting from byte 0. The intention is that the factory flashes the
# single image during the manufacturing process. Single images are not used to
# perform a router upgrade.
#
# Image Layout, size 0x8000000
# 0x00000000 BL2
# 0x00080000 FIP0
# 0x00280000 FIP1
# 0x00480000 Config
# 0x00500000 Factory Calibration # Not used by our driver
# 0x00600000 AUTOFUSE string offset
# 0x00610000 Starting version record
# 0x02000000 OpenWRT FIT 0
# 0x04000000 OpenWRT FIT 1
# 0x06000000 UBI Partition
#    0x06000000 Config volume
#    0x06800000 Log volume

cd "$(dirname "$0")/../bin"

#64K Blocks, 0x10000
BLOCK_SIZE=65536

# Generate a single image at path
OUT="$VERSION/v2-$VERSION-single.bin"

# Pad up to UBI partition - 0x06000000
dd if=/dev/zero of=$OUT bs=$BLOCK_SIZE seek=0 count=1536

# Write BL2 at 0x0.
dd if="$VERSION/v2-$VERSION-bl2.img" of=$OUT bs=$BLOCK_SIZE seek=0 conv=notrunc

# Write FIP0 (contains the uboot) to output binary, at 0x080000
dd if="$VERSION/v2-$VERSION-fip.bin" of=$OUT bs=$BLOCK_SIZE seek=8 conv=notrunc
# Write FIP1 (contains the uboot) to output binary, at 0x280000
dd if="$VERSION/v2-$VERSION-fip.bin" of=$OUT bs=$BLOCK_SIZE seek=40 conv=notrunc

# Enable autofuse, remove this line to create development single images which do not autofuse.
echo -n "AUTOFUSE" | dd of=$OUT bs=$BLOCK_SIZE seek=96 conv=notrunc
# Record version we were built with.
echo -n "$VERSION" | dd of=$OUT bs=$BLOCK_SIZE seek=97 conv=notrunc

# OpenWRT FIT0
dd if="$VERSION/v2-$VERSION-openwrt.img" of=$OUT bs=$BLOCK_SIZE seek=512 conv=notrunc
# OpenWRT FIT1
dd if="$VERSION/v2-$VERSION-openwrt.img" of=$OUT bs=$BLOCK_SIZE seek=1024 conv=notrunc

# Write storage UBI image
dd if="$VERSION/v2-$VERSION-storage.img" of=$OUT bs=$BLOCK_SIZE seek=1536

# Update convenience symlink.
ln -sf $OUT flashimage.bin
