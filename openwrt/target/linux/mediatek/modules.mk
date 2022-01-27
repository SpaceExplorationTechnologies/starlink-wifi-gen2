#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/hw_nat
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MTK Hardware NAT
  KCONFIG:=CONFIG_RA_HW_NAT
  DEPENDS:=@TARGET_mediatek
  FILES:=$(LINUX_DIR)/net/nat/hw_nat/hw_nat.ko
  AUTOLOAD:=$(call AutoProbe,hw_nat)
endef

define KernelPackage/hw_nat/install

	$(INSTALL_DIR) $(1)/lib/modules/ralink/ 
	$(INSTALL_DIR) $(STAGING_DIR_ROOT)/lib/modules/$(LINUX_VERSION)
	cp $(1)/lib/modules/$(LINUX_VERSION)/hw_nat.ko $(STAGING_DIR_ROOT)/lib/modules/$(LINUX_VERSION)/hw_nat.ko
	mv $(1)/lib/modules/$(LINUX_VERSION)/hw_nat.ko $(1)/lib/modules/ralink/
endef


$(eval $(call KernelPackage,hw_nat))

define KernelPackage/ata-ahci-mtk
  TITLE:=Mediatek AHCI Serial ATA support
  KCONFIG:=CONFIG_AHCI_MTK
  FILES:= \
	$(LINUX_DIR)/drivers/ata/ahci_mtk.ko \
	$(LINUX_DIR)/drivers/ata/libahci_platform.ko
  AUTOLOAD:=$(call AutoLoad,40,libahci libahci_platform ahci_mtk,1)
  $(call AddDepends/ata)
endef

define KernelPackage/ata-ahci-mtk/description
 Mediatek AHCI Serial ATA host controllers
endef

$(eval $(call KernelPackage,ata-ahci-mtk))

define KernelPackage/sdhci-mtk
  SUBMENU:=Other modules
  TITLE:=Mediatek SDHCI driver
  DEPENDS:=@TARGET_mediatek +kmod-sdhci
  KCONFIG:=CONFIG_MMC_MTK 
  FILES:= \
	$(LINUX_DIR)/drivers/mmc/host/mtk-sd.ko
  AUTOLOAD:=$(call AutoProbe,mtk-sd,1)
endef

$(eval $(call KernelPackage,sdhci-mtk))

define KernelPackage/crypto-hw-mtk
  TITLE:= MediaTek's Crypto Engine module
  DEPENDS:=@TARGET_mediatek
  KCONFIG:= \
	CONFIG_CRYPTO_HW=y \
	CONFIG_CRYPTO_AES=y \
	CONFIG_CRYPTO_AEAD=y \
	CONFIG_CRYPTO_SHA1=y \
	CONFIG_CRYPTO_SHA256=y \
	CONFIG_CRYPTO_SHA512=y \
	CONFIG_CRYPTO_HMAC=y \
	CONFIG_CRYPTO_DEV_MEDIATEK
  FILES:=$(LINUX_DIR)/drivers/crypto/mediatek/mtk-crypto.ko
  AUTOLOAD:=$(call AutoLoad,90,mtk-crypto)
  $(call AddDepends/crypto)
endef

define KernelPackage/crypto-hw-mtk/description
  MediaTek's EIP97 Cryptographic Engine driver.
endef

$(eval $(call KernelPackage,crypto-hw-mtk))

define KernelPackage/sound-soc-mt7622
  TITLE:=MT7622 SoC sound support
  KCONFIG:=\
	CONFIG_SND_SOC_MEDIATEK \
	CONFIG_SND_SOC_MT7622 \
	CONFIG_SND_SOC_MT7622_WM8960 \
	CONFIG_SND_SOC_MT7622_DUMMY=n \
	CONFIG_SND_SOC_MT2701=n \
	CONFIG_SND_SOC_MT8173=n \
	CONFIG_MT_SND_SOC_8521P=n \
	CONFIG_SND_SOC_MT7623_WM8960=n \
	CONFIG_SND_SOC_MT8521P_EVB=n \
	CONFIG_SND_SOC_MT8521P_RT5640=n \
	CONFIG_SND_SOC_MT8521P_CS42448=n
  FILES:= \
	$(LINUX_DIR)/sound/soc/mediatek/common/snd-soc-mtk-common.ko \
	$(LINUX_DIR)/sound/soc/mediatek/mt7622/mt7622-wm8960.ko \
	$(LINUX_DIR)/sound/soc/mediatek/mt7622/snd-soc-mt7622-afe.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8960.ko
  AUTOLOAD:=$(call AutoLoad,57,snd-soc-wm8960 mt7622-wm8960 snd-soc-mtk-common snd-soc-mt7622-afe)
  DEPENDS:=@TARGET_mediatek_mt7622 +kmod-sound-soc-core
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-mt7622/description
 Support for MT7622 Platform sound
endef

$(eval $(call KernelPackage,sound-soc-mt7622))

define KernelPackage/mediatek_hnat
  SUBMENU:=Network Devices
  TITLE:=Mediatek HNAT module
  DEPENDS:=@TARGET_mediatek +kmod-nf-conntrack
  KCONFIG:= \
	CONFIG_NET_MEDIATEK_HNAT \
	CONFIG_NET_MEDIATEK_HW_QOS=n
  FILES:= \
        $(LINUX_DIR)/drivers/net/ethernet/mediatek/mtk_hnat/mtkhnat.ko
endef

define KernelPackage/mediatek_hnat/description
  Kernel modules for MediaTek HW NAT offloading
endef

$(eval $(call KernelPackage,mediatek_hnat))

