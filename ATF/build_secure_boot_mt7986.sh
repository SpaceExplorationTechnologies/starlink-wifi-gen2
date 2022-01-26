#!/bin/sh

export TOP=`pwd`

rm -rf ./out
mkdir ./out

make PLAT=mt7986 clean

cd $TOP/tools/dev/warm_reset
make clean
make
cp warm_reset $TOP/out
cd $TOP

make CROSS_COMPILE=/usr/bin/aarch64-linux-gnu-	\
PLAT=mt7986				\
LOG_LEVEL=40				\
FPGA=1					\
BL33=./u-boot.bin			\
MBEDTLS_DIR=./mbedtls-mbedtls-2.16.2/	\
TRUSTED_BOARD_BOOT=1 			\
GENERATE_COT=1				\
ROT_KEY=./fip_private_key.pem		\
BROM_SIGN_KEY=./bl2_private_key.pem	\
BOOT_DEVICE=snand			\
all fip
cp build/mt7986/release/bl2/bl2.elf $TOP/out
cp build/mt7986/release/bl2.img $TOP/out
cp build/mt7986/release/bl2.bin $TOP/out
cp build/mt7986/release/bl31/bl31.elf $TOP/out
cp build/mt7986/release/fip.bin $TOP/out
