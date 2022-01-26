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
bl31					
cp build/mt7986/release/bl31/bl31.elf $TOP/out
