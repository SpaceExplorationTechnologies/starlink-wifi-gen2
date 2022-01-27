#!/bin/sh

# Modify input/output image name
BOOTLOADER_NAME=u-boot-mtk.bin
KERNEL_NAME=lede-kernel.bin
OUTPUT_NAME=flashimage.bin

BLOCK_SIZE=65536

# We assume that the MAX size of MT7629 NOR image is 16 Mbytes.
declare -i FLASH_SIZE=16*1024*1024
#declare -i COUNT_NUM="$FLASH_SIZE"/"$BLOCK_SIZE"

# Generate a single image
echo "Make SPI NOR Image block size 65536 Bytes"
rm -rf $OUTPUT_NAME
# Only padding to kernel partition start - 0xB0000
dd if=/dev/zero of=$OUTPUT_NAME bs=$BLOCK_SIZE seek=0 count=11
#dd if=/dev/zero of=$OUTPUT_NAME bs=$BLOCK_SIZE seek=0 count=$COUNT_NUM

if [ -e $BOOTLOADER_NAME ]; then
	dd if=$BOOTLOADER_NAME of=$OUTPUT_NAME bs=$BLOCK_SIZE seek=0 conv=notrunc
else
	echo "ERROR: no $BOOTLOADER_NAME"
	rm -rf $OUTPUT_NAME
	exit 1
fi

if [ -e $KERNEL_NAME ]; then
#	dd if=$KERNEL_NAME of=$OUTPUT_NAME bs=$BLOCK_SIZE seek=11 conv=notrunc
	cat "$KERNEL_NAME" >> "$OUTPUT_NAME"
else
	echo "ERROR: no $KERNEL_NAME"
	rm -rf $OUTPUT_NAME
	exit 1
fi

declare -i SINGLE_IMAGE_SIZE=`ls "$OUTPUT_NAME" -la | awk '{print $5}'`
if [ $FLASH_SIZE -lt $SINGLE_IMAGE_SIZE ]; then
	echo "ERROR: $OUTPUT_NAME size is bigger than $FLASH_SIZE"
	rm -rf $OUTPUT_NAME
	exit 1
else
	echo "$OUTPUT_NAME size is $SINGLE_IMAGE_SIZE."
fi

echo "Make SPI NOR flashimage.bin completed..."
