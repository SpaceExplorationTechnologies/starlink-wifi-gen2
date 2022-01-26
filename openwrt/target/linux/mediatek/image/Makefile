#
# Copyright (C) 2012-2015 OpenWrt.org
# Copyright (C) 2016-2017 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

# build dtb
define Build/dtb
        $(call Image/BuildDTB,$(DEVICE_DTS_DIR)/$(DEVICE_DTS).dts,$(DEVICE_DTS_DIR)/$(DEVICE_DTS).dtb)
	$(CP) $(DEVICE_DTS_DIR)/$(DEVICE_DTS).dtb $(BIN_DIR)/
endef

# SpaceX: build dtb for dev/unsecure chain 0
define Build/dtb-dev0-spacex
        $(call Image/BuildDTB,$(DEVICE_DTS_DIR)/$(DEVICE_DTS_DEV_0).dts,$(DEVICE_DTS_DIR)/$(DEVICE_DTS_DEV_0).dtb)
	$(CP) $(DEVICE_DTS_DIR)/$(DEVICE_DTS_DEV_0).dtb $(BIN_DIR)/
endef

# SpaceX: build dtb for dev/unsecure chain 1
define Build/dtb-dev1-spacex
        $(call Image/BuildDTB,$(DEVICE_DTS_DIR)/$(DEVICE_DTS_DEV_1).dts,$(DEVICE_DTS_DIR)/$(DEVICE_DTS_DEV_1).dtb)
	$(CP) $(DEVICE_DTS_DIR)/$(DEVICE_DTS_DEV_1).dtb $(BIN_DIR)/
endef

# SpaceX: build dtb for prod/secure chain 0
define Build/dtb-prod0-spacex
        $(call Image/BuildDTB,$(DEVICE_DTS_DIR)/$(DEVICE_DTS_PROD_0).dts,$(DEVICE_DTS_DIR)/$(DEVICE_DTS_PROD_0).dtb)
	$(CP) $(DEVICE_DTS_DIR)/$(DEVICE_DTS_PROD_0).dtb $(BIN_DIR)/
endef

# SpaceX: build dtb for prod/secure chain 1
define Build/dtb-prod1-spacex
        $(call Image/BuildDTB,$(DEVICE_DTS_DIR)/$(DEVICE_DTS_PROD_1).dts,$(DEVICE_DTS_DIR)/$(DEVICE_DTS_PROD_1).dtb)
	$(CP) $(DEVICE_DTS_DIR)/$(DEVICE_DTS_PROD_1).dtb $(BIN_DIR)/
endef

# build squashfs-hashed
define Build/squashfs-hashed
	$(CP) $(KDIR)/root.squashfs $(KDIR)/root.squashfs-hashed
	$(TOPDIR)/scripts/make-squashfs-hashed.sh $(KDIR)/root.squashfs-hashed $(STAGING_DIR_HOST) $(TOPDIR) $(KDIR)/root.squashfs-hashed-summary
	cat $(KDIR)/root.squashfs-hashed-summary | \
		${TOPDIR}/scripts/prepare-dm-verity-uboot-script.sh "$(HASHED_BOOT_DEVICE)" "$(BASIC_KERNEL_CMDLINE)" \
			> $(KDIR)/$(DEVICE_NAME)-u-boot-script.sh
endef

# build squashfs-hashed, then create two uboot loadable scripts, one for each chain. The scripts differ
# only in what mtdblock (8 or 9) the rootfs is in. U-boot then sets the kernel commandline args so it
# verifies the hash of the correct block.
define Build/squashfs-hashed-spacex
	$(CP) $(KDIR)/root.squashfs $(KDIR)/root.squashfs-hashed
	$(TOPDIR)/scripts/make-squashfs-hashed.sh $(KDIR)/root.squashfs-hashed $(STAGING_DIR_HOST) $(TOPDIR) $(KDIR)/root.squashfs-hashed-summary
	cat $(KDIR)/root.squashfs-hashed-summary | \
		${TOPDIR}/scripts/prepare-dm-verity-uboot-script.sh "$(HASHED_BOOT_DEVICE_0)" "$(BASIC_KERNEL_CMDLINE)" \
			> $(KDIR)/$(DEVICE_NAME)-u-boot-script-0.sh
	cat $(KDIR)/root.squashfs-hashed-summary | \
		${TOPDIR}/scripts/prepare-dm-verity-uboot-script.sh "$(HASHED_BOOT_DEVICE_1)" "$(BASIC_KERNEL_CMDLINE)" \
			> $(KDIR)/$(DEVICE_NAME)-u-boot-script-1.sh
endef
DEFAULT_DEVICE_VARS += HASHED_BOOT_DEVICE BASIC_KERNEL_CMDLINE

# append squashfs-hashed
define Build/append-squashfs-hashed
	cat $(KDIR)/root.squashfs-hashed >> $@
endef

# build fit-ar-ver
get_fit_ar_ver = \
  $(if $(wildcard $(2)),$(shell rm -rf $(2))) \
  $(if $(wildcard $(1)),$(info $(shell $(STAGING_DIR_HOST)/bin/ar-table create_ar_conf $(1) $(2)))) \
  $(if $(wildcard $(2)),$(eval include $(2))) \
  $(if $(FIT_AR_VER),$(info FIT_AR_VER = $(FIT_AR_VER)))

define Build/fit-ar-ver
	$(call get_fit_ar_ver,$(AR_TABLE_XML),$(AUTO_AR_CONF))
endef

# build signed fit
define Build/fit-sign
	$(TOPDIR)/scripts/mkits.sh \
		-D $(DEVICE_NAME) -o $@.its -k $@ \
		-d $(word 2,$(1)) -C $(word 1,$(1)) \
		-a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-A $(LINUX_KARCH) -v $(LINUX_VERSION) \
		-s $(KDIR)/$(DEVICE_NAME)-u-boot-script.sh \
		$(if $(FIT_KEY_NAME),-S $(FIT_KEY_NAME)) \
		$(if $(FIT_AR_VER),-r $(FIT_AR_VER))
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $@.its \
		$(if $(FIT_KEY_DIR),-k $(FIT_KEY_DIR)) \
		-r $@.new
	@mv $@.new $@
endef

# build signed fit, with 2 configs, device trees and u-boot loadable scripts.
define Build/fit-sign-spacex
	$(TOPDIR)/scripts/mkits-spacex.sh \
		-D $(DEVICE_NAME) -o $@.its -k $@ \
		-C $(word 1,$(1)) \
		-d $(word 2,$(1)) -b $(word 3,$(1)) \
		-t $(word 4,$(1)) -T $(word 5,$(1)) \
		-a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-A $(LINUX_KARCH) -v $(LINUX_VERSION) \
		-s $(KDIR)/$(DEVICE_NAME)-u-boot-script-0.sh \
		-u $(KDIR)/$(DEVICE_NAME)-u-boot-script-1.sh \
		$(if $(FIT_KEY_NAME),-S $(FIT_KEY_NAME)) \
		$(if $(FIT_AR_VER),-r $(FIT_AR_VER))
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $@.its \
		$(if $(FIT_KEY_DIR),-k $(FIT_KEY_DIR)) \
		-r $@.new
	@mv $@.new $@
endef
DEFAULT_DEVICE_VARS += FIT_KEY_NAME FIT_KEY_DIR

# default all platform image(fit) build
define Device/Default
  PROFILES = Default $$(DEVICE_NAME)
  KERNEL_NAME := Image
  FIT_KEY_DIR :=
  FIT_KEY_NAME :=
  FILESYSTEMS := squashfs
  DEVICE_DTS_DIR := $(DTS_DIR)
  KERNEL = dtb | kernel-bin | lzma | fit lzma $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := append-kernel | pad-to 128k | append-rootfs | pad-rootfs | append-metadata
endef
DEFAULT_DEVICE_VARS += AR_TABLE_XML AUTO_AR_CONF

include $(SUBTARGET).mk

$(eval $(call BuildImage))

