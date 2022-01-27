#!/bin/sh

. ./flash.info

# Modify input/output image name
PRELOADER_NAME=preloader_evb7622_64.bin
ATF_NAME=atf.bin
UBOOT_NAME=u-boot-mtk.bin
KERNEL_NAME=root_uImage
OUTPUT_NAME=mt7622_img 

# Modify BLOCK_SIZE in case nand block size is not 0x20000=131072 
declare -i BLOCK_SIZE=131072

# Modify below if Partition layput is changed
# Preloader = 0x80000 = 524288
# ATF = 0x40000 = 262144
# Uboot + Config + Factiry = 0x80000 + 0x80000 + 0x40000 = 1310720
declare -i PRELOADER_SZ=`ls "$PRELOADER_NAME" -la | awk '{print $5}'`
declare -i UBOOT_SZ=`ls "$UBOOT_NAME" -la | awk '{print $5}'`
declare -i ATF_SZ=`ls "$ATF_NAME" -la | awk '{print $5}'`
declare -i PRELOADER_PAD=(524288/"$BLOCK_SIZE")-1-"$PRELOADER_SZ"/"$BLOCK_SIZE"
declare -i ATF_PAD=(262144/"$BLOCK_SIZE")-"$ATF_SZ"/"$BLOCK_SIZE"
declare -i UBOOT_PAD=(1310720/"$BLOCK_SIZE")-"$UBOOT_SZ"/"$BLOCK_SIZE"

# Pad each image

# Preloader partition size is 0x80000
# Device-header has 1 block, so preloader image should have 4 - 1 = 3 block
#echo "$PRELOADER_PAD"
#echo "$ATF_PAD"
#echo "$UBOOT_PAD"

./sbch i "$FLASH_NAME" "$PRELOADER_NAME" "$PRELOADER_NAME".pad 0 0 "$PRELOADER_PAD"
./sbch i "$FLASH_NAME" "$PRELOADER_NAME".pad "$PRELOADER_NAME".img 1 64 0

# ATF's size is 0x40000, image size should be 2 block
./sbch i "$FLASH_NAME" "$ATF_NAME" "$ATF_NAME".pad 0 0 "$ATF_PAD"

# Uboot/Config/RF partition has 0x80000 + 0x80000 + 0x40000 = 0x140000
# so UBOOT Need to pad to 10 blocks
./sbch i "$FLASH_NAME" "$UBOOT_NAME" "$UBOOT_NAME".pad 0 0 "$UBOOT_PAD"

# Kernel is the last image, no pad is necessary

# Generate a single image by attach each padded images
cp "$PRELOADER_NAME".img "$OUTPUT_NAME"
cat "$ATF_NAME".pad >> "$OUTPUT_NAME"
cat "$UBOOT_NAME".pad >> "$OUTPUT_NAME"
cat "$KERNEL_NAME" >> "$OUTPUT_NAME"

rm *.pad -rf
rm "$PRELOADER_NAME".img

. ./gen_ecc.sh

