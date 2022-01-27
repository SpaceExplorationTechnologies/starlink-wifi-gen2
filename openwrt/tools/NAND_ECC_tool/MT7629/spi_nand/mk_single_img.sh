#!/bin/sh

. ./flash.info

rm -rf flashimage.bin
rm -rf flashimage.ecc

# Modify input/output image name
UBOOT_NAME=u-boot-mtk.bin
KERNEL_NAME=kernel-image.bin
OUTPUT_NAME=flashimage.bin

# Modify PAGE_SIZE according to SPI Nand device Spec
declare -i PAGE_SIZE=2048
declare -i BLOCK_SIZE=$[$PAGE_SIZE * 64]
declare -i HDR_LY_SIZE=$[$PAGE_SIZE * 2]

hexdump "$UBOOT_NAME" -n 10 -C | grep BOOTLOADER
if [ $? -eq 0 ]; then
	echo "Header included! Remove the old header use new header!"
	dd bs=2048 skip=2 if="$UBOOT_NAME" of=uboot.no_hdr
else
	echo "Header not included, add header!"
	cp "$UBOOT_NAME" uboot.no_hdr
fi

./sbch h "$FLASH_NAME" 0 hdr_ly.binary 1 64 0

# pad device header & layout to u-boot-mtk.bin
declare -i UBOOT_SIZE=`ls uboot.no_hdr -la | awk '{print $5}'`
dd if=uboot.no_hdr of=hdr_ly.binary bs=1 seek="$HDR_LY_SIZE" count="$UBOOT_SIZE" conv=notrunc
cp hdr_ly.binary "$UBOOT_NAME"

# Pad Uboot
# Modify below if Partition layput is changed
# Uboot + config + factorty = 0x100000 + 0X40000 + 0X80000 = 1835008
# so UBOOT Need to pad to 14 blocks
declare -i UBOOT_SZ=`ls "$UBOOT_NAME"  -la | awk '{print $5}'`
declare -i UBOOT_PAD=(1835008/"$BLOCK_SIZE")-"$UBOOT_SZ"/"$BLOCK_SIZE"
./sbch i "$FLASH_NAME" "$UBOOT_NAME"  u-boot.pad 0 0 "$UBOOT_PAD"

# Kernel is the last image, no pad is necessary

# Generate a single image by attach padded images
cp u-boot.pad "$OUTPUT_NAME"
cat "$KERNEL_NAME" >> "$OUTPUT_NAME"

rm *.pad -rf
rm *.binary -rf

. ./gen_ecc.sh