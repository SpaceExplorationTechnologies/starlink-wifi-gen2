#!/bin/bash -ex

cd "$(dirname "$0")/../uboot"

export CROSS_COMPILE=/usr/bin/arm-linux-gnueabihf-

if [[ "$NO_CLEAN" != "1" ]]; then
    make clean
fi

make spacex_v2_defconfig
make V=s FIT_KEY=../keys/fit_key.crt

cp u-boot.bin ../bin/$VERSION/v2-$VERSION-uboot.bin
