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
BOOT_DEVICE=ram				\
bl2
cp build/mt7986/release/bl2/bl2.elf $TOP/out
cp build/mt7986/release/bl2.bin $TOP/out
