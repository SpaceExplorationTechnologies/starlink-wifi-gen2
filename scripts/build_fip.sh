#!/bin/bash -ex

cd "$(dirname "$0")/../ATF"

export CROSS_COMPILE=/usr/bin/arm-linux-gnueabihf-

if [[ "$NO_CLEAN" != "1" ]]; then
    make distclean
fi

make \
    PLAT=mt7629 \
    ARCH=aarch32 \
    AARCH32_SP=sp_min \
    BOOT_DEVICE=snand \
    NMBM=1 \
    NAND_TYPE=2k+64 \
    BL33=../bin/$VERSION/v2-$VERSION-uboot.bin \
    MBEDTLS_DIR=../tools/mbedtls-mbedtls-2.16.2 \
    TRUSTED_BOARD_BOOT=1 \
    GENERATE_COT=1 \
    ANTI_ROLLBACK_FIP_VERSION=$ANTI_ROLLBACK_REQUIRED_VERSION \
    ROT_KEY=../keys/fip_private_key.pem \
    BROM_SIGN_KEY=../keys/bl2_private_key.pem \
    LOG_LEVEL=40 \
    all fip

cp build/mt7629/release/bl2.img ../bin/$VERSION/v2-$VERSION-bl2.img
cp build/mt7629/release/bl2.img.signkeyhash ../bin/$VERSION/v2-$VERSION-bl2.img.signkeyhash
cp build/mt7629/release/fip.bin ../bin/$VERSION/v2-$VERSION-fip.bin
