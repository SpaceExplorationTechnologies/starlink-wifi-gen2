#!/bin/bash -ex

cd "$(dirname "$0")/../openwrt"

echo "$OVERLAY" > package/base-files/files/etc/overlay
echo "$VERSION" > package/base-files/files/etc/version
cp spacex-mtk-openwrt.config .config

if [[ "$NO_CLEAN" != "1" ]]; then
    make dirclean
fi

make -j4 FORCE_UNSAFE_CONFIGURE=1

cp bin/targets/mediatek/mt7629/lede-mediatek-mt7629-SPACEX-WIFI-V2-squashfs-sysupgrade.bin ../bin/$VERSION/v2-$VERSION-openwrt.img
