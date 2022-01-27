#!/bin/sh

# Modify input/output image name
PRELOADER_NAME=preloader.bin
ATF_NAME=atf.bin
UBOOT_NAME=u-boot-mtk.bin
KERNEL_NAME=uimage.bin
OUTPUT_NAME=flashimage.bin
FLASH_SIZE=16*1024*1024
BLOCK_SIZE=1024
declare -i COUNT_NUM="$FLASH_SIZE"/"$BLOCK_SIZE"

# Generate a single image
echo "Make SPI NOR Image block size 1024Bytes"
dd if=/dev/zero of=$OUTPUT_NAME bs=$BLOCK_SIZE seek=0 count=$COUNT_NUM

if [ -e $PRELOADER_NAME ]; then
	dd if=$PRELOADER_NAME of=$OUTPUT_NAME bs=$BLOCK_SIZE seek=0
else
	echo "no $PRELOADER_NAME"
	rm -rf $OUTPUT_NAME
	exit 1
fi

if [ -e $ATF_NAME ]; then
	dd if=$ATF_NAME of=$OUTPUT_NAME bs=$BLOCK_SIZE seek=256
else
	echo "no $ATF_NAME"
	rm -rf $OUTPUT_NAME
	exit 1
fi

if [ -e $UBOOT_NAME ]; then
	dd if=$UBOOT_NAME of=$OUTPUT_NAME bs=$BLOCK_SIZE seek=384
else
	echo "no $UBOOT_NAME"
	rm -rf $OUTPUT_NAME
	exit 1
fi

if [ -e $KERNEL_NAME ]; then
#	Warning : AX image set seek=1280
	dd if=$KERNEL_NAME of=$OUTPUT_NAME bs=$BLOCK_SIZE seek=896
else
	echo "no $KERNEL_NAME"
	rm -rf $OUTPUT_NAME
	exit 1
fi

echo "Make SPI NOR Image completed..."
