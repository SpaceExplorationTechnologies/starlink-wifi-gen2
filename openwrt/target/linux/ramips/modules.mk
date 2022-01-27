#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

OTHER_MENU:=Other modules

define KernelPackage/pwm-mediatek
  SUBMENU:=Other modules
  TITLE:=MT7628 PWM
  DEPENDS:=@(TARGET_ramips_mt7628||TARGET_ramips_mt7688)
  KCONFIG:= \
	CONFIG_PWM=y \
	CONFIG_PWM_MEDIATEK \
	CONFIG_PWM_SYSFS=y
  FILES:= \
	$(LINUX_DIR)/drivers/pwm/pwm-mediatek.ko
  AUTOLOAD:=$(call AutoProbe,pwm-mediatek)
endef

define KernelPackage/pwm-mediatek/description
  Kernel modules for MediaTek Pulse Width Modulator
endef

$(eval $(call KernelPackage,pwm-mediatek))

define KernelPackage/sdhci-mt7621
  SUBMENU:=Other modules
  TITLE:=MT7621 SD Host Controller driver
  DEPENDS:=@(TARGET_ramips_mt7621) +kmod-mmc
  KCONFIG:= \
	CONFIG_MMC_MT7621
  FILES:= \
	$(LINUX_DIR)/drivers/mmc/host/mt7621-sd/mt7621-sd.ko
  AUTOLOAD:=$(call AutoProbe,mt7621-sd,1)
endef

$(eval $(call KernelPackage,sdhci-mt7621))

I2C_RALINK_MODULES:= \
  CONFIG_I2C_RALINK:drivers/i2c/busses/i2c-ralink

define KernelPackage/i2c-ralink
  $(call i2c_defaults,$(I2C_RALINK_MODULES),59)
  TITLE:=Ralink I2C Controller
  DEPENDS:=kmod-i2c-core @TARGET_ramips \
	@!(TARGET_ramips_mt7621||TARGET_ramips_mt7628||TARGET_ramips_mt7688)
endef

define KernelPackage/i2c-ralink/description
 Kernel modules for enable ralink i2c controller.
endef

$(eval $(call KernelPackage,i2c-ralink))


I2C_MT7621_MODULES:= \
  CONFIG_I2C_MT7621:drivers/i2c/busses/i2c-mt7621

define KernelPackage/i2c-mt7628
  $(call i2c_defaults,$(I2C_MT7621_MODULES),59)
  TITLE:=MT7628/88 I2C Controller
  DEPENDS:=kmod-i2c-core \
	@(TARGET_ramips_mt7628||TARGET_ramips_mt7688)
endef

define KernelPackage/i2c-mt7628/description
 Kernel modules for enable mt7621 i2c controller.
endef

$(eval $(call KernelPackage,i2c-mt7628))

define KernelPackage/dma-ralink
  SUBMENU:=Other modules
  TITLE:=Ralink GDMA Engine
  DEPENDS:=@TARGET_ramips
  KCONFIG:= \
	CONFIG_DMADEVICES=y \
	CONFIG_DW_DMAC_PCI=n \
	CONFIG_DMA_MT7620
  FILES:= \
	$(LINUX_DIR)/drivers/dma/virt-dma.ko \
	$(LINUX_DIR)/drivers/dma/mt7620-gdma.ko
  AUTOLOAD:=$(call AutoLoad,52,mt7620-gdma)
endef

define KernelPackage/dma-ralink/description
 Kernel modules for enable ralink dma engine.
endef

$(eval $(call KernelPackage,dma-ralink))

define KernelPackage/hsdma-mtk
  SUBMENU:=Other modules
  TITLE:=MediaTek HSDMA Engine
  DEPENDS:=@TARGET_ramips @TARGET_ramips_mt7621
  KCONFIG:= \
	CONFIG_DMADEVICES=y \
	CONFIG_DW_DMAC_PCI=n \
	CONFIG_MTK_HSDMA
  FILES:= \
	$(LINUX_DIR)/drivers/dma/virt-dma.ko \
	$(LINUX_DIR)/drivers/dma/mtk-hsdma.ko
  AUTOLOAD:=$(call AutoLoad,53,mtk-hsdma)
endef

define KernelPackage/hsdma-mtk/description
 Kernel modules for enable MediaTek hsdma engine.
endef

$(eval $(call KernelPackage,hsdma-mtk))

define KernelPackage/sound-mt7621
  TITLE:=MT7621 I2S Alsa Driver
  DEPENDS:=@TARGET_ramips +kmod-sound-soc-core +kmod-regmap +kmod-dma-ralink @!TARGET_ramips_rt288x
  KCONFIG:= \
	CONFIG_SND_SOC_MT7621 \
	CONFIG_SND_SIMPLE_CARD \
	CONFIG_SND_SOC_MT7621_WM8960
  FILES:= \
	$(LINUX_DIR)/sound/soc/mediatek/mt7621/snd-soc-ralink-i2s.ko \
	$(LINUX_DIR)/sound/soc/generic/snd-soc-simple-card.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8960.ko
  AUTOLOAD:=$(call AutoLoad,90,snd-soc-wm8960 snd-soc-ralink-i2s snd-soc-simple-card)
  $(call AddDepends/sound)
endef

define KernelPackage/sound-mt7621/description
 Alsa modules for ralink i2s controller.
endef

$(eval $(call KernelPackage,sound-mt7621))

define KernelPackage/hw_nat
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MTK Hardware NAT
  KCONFIG:=CONFIG_RA_HW_NAT
  DEPENDS:=@TARGET_ramips
  FILES:=$(LINUX_DIR)/net/nat/hw_nat/hw_nat.ko
endef
define KernelPackage/hw_nat/install
	$(INSTALL_DIR) $(1)/lib/modules/ralink/
	mv $(1)/lib/modules/$(LINUX_VERSION)/hw_nat.ko $(1)/lib/modules/ralink/
endef
$(eval $(call KernelPackage,hw_nat))

define KernelPackage/mediatek_hnat
  SUBMENU:=Network Devices
  TITLE:=Mediatek HNAT module
  DEPENDS:=@TARGET_ramips +kmod-nf-conntrack
  KCONFIG:= CONFIG_NET_MEDIATEK_HNAT=y
  FILES:= \
        $(LINUX_DIR)/drivers/net/ethernet/mediatek/mtk_hnat/mtkhnat.ko
endef

define KernelPackage/mediatek_hnat/description
  Kernel modules for MediaTek HW NAT offloading
endef

$(eval $(call KernelPackage,mediatek_hnat))
