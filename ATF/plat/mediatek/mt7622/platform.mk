#
# Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MTK_PLAT		:=	plat/mediatek
MTK_PLAT_SOC		:=	${MTK_PLAT}/${PLAT}

PLAT_INCLUDES		:=	-I${MTK_PLAT}/common/				\
				-I${MTK_PLAT}/common/drivers/uart		\
				-Iinclude/plat/arm/common			\
				-Iinclude/plat/arm/common/aarch64		\
				-I${MTK_PLAT_SOC}/drivers/dram/			\
				-I${MTK_PLAT_SOC}/drivers/pinctrl/		\
				-I${MTK_PLAT_SOC}/drivers/pmic/			\
				-I${MTK_PLAT_SOC}/drivers/pll/			\
				-I${MTK_PLAT_SOC}/drivers/rtc/			\
				-I${MTK_PLAT_SOC}/drivers/spm/			\
				-I${MTK_PLAT_SOC}/drivers/timer/		\
				-I${MTK_PLAT_SOC}/drivers/efuse/include		\
				-I${MTK_PLAT_SOC}/include/

PLAT_BL_COMMON_SOURCES	:=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				plat/arm/common/arm_gicv2.c			\
				plat/common/plat_gicv2.c

BL2_SOURCES		:=	common/desc_image_load.c			\
				drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v2/gicv2_main.c			\
				drivers/arm/gic/v2/gicv2_helpers.c		\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/gpio/gpio.c				\
				drivers/io/io_storage.c				\
				drivers/io/io_block.c				\
				drivers/io/io_fip.c				\
				lib/cpus/aarch64/cortex_a53.S			\
				${MTK_PLAT}/common/drivers/uart/aarch64/hsuart.S	\
				${MTK_PLAT}/common/mtk_plat_common.c		\
				${MTK_PLAT_SOC}/aarch64/plat_helpers.S		\
				${MTK_PLAT_SOC}/aarch64/platform_common.c	\
				${MTK_PLAT_SOC}/bl2_plat_setup.c		\
				${MTK_PLAT_SOC}/drivers/pinctrl/pinctrl.c	\
				${MTK_PLAT_SOC}/drivers/pll/pll.c		\
				${MTK_PLAT_SOC}/drivers/pmic/pmic_wrap.c	\
				${MTK_PLAT_SOC}/drivers/pmic/pmic.c		\
				${MTK_PLAT_SOC}/drivers/spm/mtcmos.c		\
				${MTK_PLAT_SOC}/drivers/timer/cpuxgpt.c		\
				${MTK_PLAT_SOC}/mtk_ar_table.c

HAVE_DRAM_OBJ_FILE	:= 	$(shell test -f ${MTK_PLAT_SOC}/drivers/dram/release/dram.o && echo yes)
ifeq ($(HAVE_DRAM_OBJ_FILE),yes)
ifeq ($(DDR3_FLYBY), 1)
PREBUILT_LIBS		+=	${MTK_PLAT_SOC}/drivers/dram/release/dram-flyby.o
else
PREBUILT_LIBS		+=	${MTK_PLAT_SOC}/drivers/dram/release/dram.o
endif
else
BL2_SOURCES		+=	${MTK_PLAT_SOC}/drivers/dram/dramc_calib.c	\
				${MTK_PLAT_SOC}/drivers/dram/dramc_dqs_gw.c	\
				${MTK_PLAT_SOC}/drivers/dram/emi.c		\
				${MTK_PLAT_SOC}/drivers/dram/memory.c
endif

ifndef BOOT_DEVICE
$(error You must specify the boot device by provide BOOT_DEVICE= to \
	make parameter. Avaliable values: nor emmc sdmmc snand)
else
ifeq ($(BOOT_DEVICE),ram)
BL2_SOURCES		+=	drivers/io/io_memmap.c				\
				${MTK_PLAT_SOC}/bl2_boot_ram.c
BROM_HEADER_TYPE	:=	nor
ifeq ($(RAM_BOOT_DEBUGGER_HOOK), 1)
CPPFLAGS		+=	-DRAM_BOOT_DEBUGGER_HOOK
endif
endif
ifeq ($(BOOT_DEVICE),nor)
BL2_SOURCES		+=	drivers/io/io_memmap.c				\
				${MTK_PLAT_SOC}/bl2_boot_nor.c
BROM_HEADER_TYPE	:=	nor
endif
ifeq ($(BOOT_DEVICE),emmc)
BL2_SOURCES		+=	drivers/mmc/mmc.c				\
				drivers/partition/partition.c			\
				drivers/partition/gpt.c				\
				${MTK_PLAT}/common/drivers/mmc/mtk-sd.c		\
				${MTK_PLAT_SOC}/bl2_boot_mmc.c
BROM_HEADER_TYPE	:=	emmc
CPPFLAGS		+=	-DMSDC_INDEX=0
endif
ifeq ($(BOOT_DEVICE),sdmmc)
BL2_SOURCES		+=	drivers/mmc/mmc.c				\
				drivers/partition/partition.c			\
				drivers/partition/gpt.c				\
				${MTK_PLAT}/common/drivers/mmc/mtk-sd.c		\
				${MTK_PLAT_SOC}/bl2_boot_mmc.c
BROM_HEADER_TYPE	:=	sdmmc
CPPFLAGS		+=	-DMSDC_INDEX=1
DEVICE_HEADER_OFFSET	?=	0x80000
ifeq ($(BROM_SIGN_KEY),)
$(error BootROM signing key is required for SD booting. Please specify BROM_SIGN_KEY)
endif
endif
ifeq ($(BOOT_DEVICE),snand)
include ${MTK_PLAT}/common/drivers/snfi/mtk-snand.mk
BROM_HEADER_TYPE	:=	snand
NAND_TYPE		?=	2k+64
BL2_SOURCES		+=	${MTK_SNAND_SOURCES}				\
				${MTK_PLAT_SOC}/bl2_boot_snand.c
PLAT_INCLUDES		+=	-I${MTK_PLAT}/common/drivers/snfi
CPPFLAGS		+=	-DMTK_MEM_POOL_BASE=0x40100000			\
				-DPRIVATE_MTK_SNAND_HEADER
ifeq ($(NMBM),1)
include lib/nmbm/nmbm.mk
BL2_SOURCES		+=	${NMBM_SOURCES}
CPPFLAGS		+=	-DNMBM=1
ifneq ($(NMBM_MAX_RATIO),)
CPPFLAGS		+=	-DNMBM_MAX_RATIO=$(NMBM_MAX_RATIO)
endif
ifneq ($(NMBM_MAX_RESERVED_BLOCKS),)
CPPFLAGS		+=	-DNMBM_MAX_RESERVED_BLOCKS=$(NMBM_MAX_RESERVED_BLOCKS)
endif
ifneq ($(NMBM_DEFAULT_LOG_LEVEL),)
CPPFLAGS		+=	-DNMBM_DEFAULT_LOG_LEVEL=$(NMBM_DEFAULT_LOG_LEVEL)
endif
endif
endif
ifeq ($(BROM_HEADER_TYPE),)
$(error BOOT_DEVICE has invalid value. Please re-check.)
endif
endif

ifeq ($(DDR3_FLYBY), 1)
CPPFLAGS		+=	-DDDR3_FLYBY
endif

BL31_SOURCES		+=	drivers/arm/cci/cci.c				\
				drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v2/gicv2_main.c			\
				drivers/arm/gic/v2/gicv2_helpers.c		\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				${MTK_PLAT}/common/drivers/uart/aarch64/hsuart.S	\
				${MTK_PLAT}/common/mtk_plat_common.c		\
				${MTK_PLAT}/common/mtk_sip_svc.c		\
				${MTK_PLAT_SOC}/aarch64/plat_helpers.S		\
				${MTK_PLAT_SOC}/aarch64/platform_common.c	\
				${MTK_PLAT_SOC}/bl31_plat_setup.c		\
				${MTK_PLAT_SOC}/drivers/pmic/pmic_wrap.c	\
				${MTK_PLAT_SOC}/drivers/rtc/rtc.c		\
				${MTK_PLAT_SOC}/drivers/spm/mtcmos.c		\
				${MTK_PLAT_SOC}/drivers/spm/spm.c		\
				${MTK_PLAT_SOC}/drivers/spm/spm_hotplug.c	\
				${MTK_PLAT_SOC}/drivers/spm/spm_mcdi.c		\
				${MTK_PLAT_SOC}/drivers/spm/spm_suspend.c	\
				${MTK_PLAT_SOC}/drivers/timer/cpuxgpt.c		\
				${MTK_PLAT_SOC}/plat_pm.c			\
				${MTK_PLAT_SOC}/plat_sip_calls.c		\
				${MTK_PLAT_SOC}/plat_topology.c			\
				${MTK_PLAT_SOC}/plat_mt_gic.c			\
				${MTK_PLAT_SOC}/power_tracer.c			\
				${MTK_PLAT_SOC}/scu.c				\
				${MTK_PLAT_SOC}/mtk_ar_table.c

BL2_BASE		:=	0x201000
CPPFLAGS		+=	-DBL2_BASE=$(BL2_BASE)
CPPFLAGS		+=	-DKERNEL_IS_DEFAULT_64BIT

DOIMAGEPATH		:=	tools/mediatek/bromimage
DOIMAGETOOL		:=	${DOIMAGEPATH}/bromimage
DOVERSIONPATH		:=	tools/mediatek/ar-table
DOVERSIONTOOL		:=	${DOVERSIONPATH}/ar-table
AUTO_AR_TABLE		:=	${MTK_PLAT_SOC}/auto_ar_table.c
AUTO_AR_CONF		:=	${MTK_PLAT_SOC}/auto_ar_conf.mk

HAVE_EFUSE_SRC_FILE	:= 	$(shell test -f ${MTK_PLAT_SOC}/drivers/efuse/src/efuse_cmd.c && echo yes)
ifeq ($(HAVE_EFUSE_SRC_FILE),yes)
PLAT_INCLUDES		+=	-I${MTK_PLAT_SOC}/drivers/efuse/src
BL31_SOURCES		+=	${MTK_PLAT_SOC}/drivers/efuse/src/efuse_cmd.c
else
PREBUILT_LIBS		+=	${MTK_PLAT_SOC}/drivers/efuse/release/efuse_cmd.o
endif

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_826319	:=	1
ERRATA_A53_836870	:=	1
ERRATA_A53_855873	:=	1

# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS	:=	1

$(eval $(call add_define,MTK_SIP_SET_AUTHORIZED_SECURE_REG_ENABLE))

# Do not enable SVE
ENABLE_SVE_FOR_NS		:=	0
MULTI_CONSOLE_API		:=	1

BL2_AT_EL3			:=	1

ifneq (${TRUSTED_BOARD_BOOT},0)

include drivers/auth/mbedtls/mbedtls_crypto.mk
include drivers/auth/mbedtls/mbedtls_x509.mk
ifeq ($(MBEDTLS_DIR),)
$(error You must specify MBEDTLS_DIR when TRUSTED_BOARD_BOOT enabled)
endif

AUTH_SOURCES		:=	drivers/auth/auth_mod.c			\
				drivers/auth/crypto_mod.c		\
				drivers/auth/img_parser_mod.c		\
				drivers/auth/tbbr/tbbr_cot_bl2.c	\
				drivers/auth/tbbr/tbbr_cot_common.c	\
				$(MBEDTLS_DIR)/library/md_wrap.c

BL2_SOURCES		+=	${AUTH_SOURCES}				\
				plat/common/tbbr/plat_tbbr.c		\
				${MTK_PLAT_SOC}/mtk_tbbr.c		\
				${MTK_PLAT_SOC}/mtk_rotpk.S

ROT_KEY			=	$(BUILD_PLAT)/rot_key.pem
ROTPK_HASH		=	$(BUILD_PLAT)/rotpk_sha256.bin

$(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))
$(BUILD_PLAT)/bl1/mtk_rotpk.o: $(ROTPK_HASH)
$(BUILD_PLAT)/bl2/mtk_rotpk.o: $(ROTPK_HASH)

certificates: $(ROT_KEY)
$(ROT_KEY): | $(BUILD_PLAT)
	@echo "  OPENSSL $@"
	$(Q)openssl genrsa 2048 > $@ 2>/dev/null

$(ROTPK_HASH): $(ROT_KEY)
	@echo "  OPENSSL $@"
	$(Q)openssl rsa -in $< -pubout -outform DER 2>/dev/null |\
	openssl dgst -sha256 -binary > $@ 2>/dev/null
endif

all: $(BUILD_PLAT)/bl2.img

ifneq ($(BROM_SIGN_KEY),)
$(BUILD_PLAT)/bl2.img: $(BROM_SIGN_KEY)
endif

$(BUILD_PLAT)/bl2.img: $(BUILD_PLAT)/bl2.bin $(DOIMAGETOOL)
	-$(Q)rm -f $@.signkeyhash
	$(Q)$(DOIMAGETOOL) -f $(BROM_HEADER_TYPE) -a $(BL2_BASE) -d -e		\
		$(if $(BROM_SIGN_KEY), -s sha256+rsa-m1 -k $(BROM_SIGN_KEY))	\
		$(if $(BROM_SIGN_KEY), -p $@.signkeyhash)			\
		$(if $(DEVICE_HEADER_OFFSET), -o $(DEVICE_HEADER_OFFSET))	\
		$(if $(BL2_AR_VER), -r $(BL2_AR_VER))				\
		$(if $(NAND_TYPE), -n $(NAND_TYPE))				\
		$(BUILD_PLAT)/bl2.bin $@

.PHONY: $(BUILD_PLAT)/bl2.img $(AUTO_AR_TABLE) $(AUTO_AR_CONF)

ifeq ($(ANTI_ROLLBACK),1)
ifndef AR_TABLE_XML
$(error You must specify the anti-rollback table XML file by build option \
	AR_TABLE_XML=<path to ar_table.xml>)
endif

TFW_NVCTR_VAL		?=	0
NTFW_NVCTR_VAL		?=	0
BL2_AR_VER		?=	0

BL2_SOURCES		+=	${AUTO_AR_TABLE}

BL31_SOURCES		+=	${AUTO_AR_TABLE}

CPPFLAGS		+=	-DANTI_ROLLBACK

ar_table: $(DOVERSIONTOOL) $(AUTO_AR_TABLE) $(AUTO_AR_CONF)
	$(eval $(shell sed -n 1p $(AUTO_AR_CONF)))
	$(eval $(shell sed -n 2p $(AUTO_AR_CONF)))
	$(eval $(shell sed -n 3p $(AUTO_AR_CONF)))
	$(eval $(call CERT_REMOVE_CMD_OPT,0,--tfw-nvctr))
	$(eval $(call CERT_REMOVE_CMD_OPT,0,--ntfw-nvctr))
	$(eval $(call CERT_ADD_CMD_OPT,$(TFW_NVCTR_VAL),--tfw-nvctr))
	$(eval $(call CERT_ADD_CMD_OPT,$(NTFW_NVCTR_VAL),--ntfw-nvctr))
	@echo "TFW_NVCTR_VAL  = $(TFW_NVCTR_VAL)"
	@echo "NTFW_NVCTR_VAL = $(NTFW_NVCTR_VAL)"
	@echo "BL2_AR_VER     = $(BL2_AR_VER)"

$(AUTO_AR_TABLE): $(DOVERSIONTOOL)
	$(Q)$(DOVERSIONTOOL) create_ar_table $(AR_TABLE_XML) $(AUTO_AR_TABLE)

$(AUTO_AR_CONF): $(DOVERSIONTOOL)
	$(Q)$(DOVERSIONTOOL) create_ar_conf $(AR_TABLE_XML) $(AUTO_AR_CONF)
else
ar_table:
	@echo "Warning: anti-rollback function has been turn-off"
endif

distclean realclean clean: cleandoimage cleandoversion cleanartable

cleandoimage:
	$(Q)$(MAKE) --no-print-directory -C $(DOIMAGEPATH) clean

cleandoversion:
	$(Q)$(MAKE) --no-print-directory -C $(DOVERSIONPATH) clean

cleanartable:
	$(Q)rm -f ${AUTO_AR_TABLE}
	$(Q)rm -f ${AUTO_AR_CONF}

$(DOIMAGETOOL):
	$(Q)$(MAKE) --no-print-directory -C $(DOIMAGEPATH)

$(DOVERSIONTOOL):
	$(Q)$(MAKE) --no-print-directory -C $(DOVERSIONPATH)
