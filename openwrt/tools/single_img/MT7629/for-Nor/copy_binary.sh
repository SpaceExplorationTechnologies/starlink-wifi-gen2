#
# This script is used only for copying images to upgrade directory.
#
#!/bin/sh
rm -rf ./lede-kernel.bin
rm -rf ./u-boot*

cp ../../../../../bootloader/Uboot-arm/spl/u-boot-spl ./
cp ../../../../../bootloader/Uboot-arm/u-boot-mtk.bin ./
cp ../../../../../bootloader/Uboot-arm/u-boot.img ./
cp ../../../../../bootloader/Uboot-arm/u-boot ./

cp ../../../../../lede/bin/targets/mediatek/mt7629/lede-mediatek-mt7629-MTK-7629-EVB-initramfs-kernel.bin ./
mv lede-mediatek-mt7629-MTK-7629-EVB-initramfs-kernel.bin lede-kernel.bin

sh mk_single_img.sh

cp flashimage.bin /tftpboot/dehui/TFTPBOOT/ -f
cp u-boot-spl /tftpboot/dehui/TFTPBOOT/ -f
cp u-boot-mtk.bin /tftpboot/dehui/TFTPBOOT/ -f
cp u-boot.img /tftpboot/dehui/TFTPBOOT/ -f
cp u-boot /tftpboot/dehui/TFTPBOOT/ -f
cp lede-kernel.bin /tftpboot/dehui/TFTPBOOT/ -f
