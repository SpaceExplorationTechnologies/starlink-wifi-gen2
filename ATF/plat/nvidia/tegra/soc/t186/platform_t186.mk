#
# Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
# Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# platform configs
ENABLE_ROC_FOR_ORDERING_CLIENT_REQUESTS	:= 1
$(eval $(call add_define,ENABLE_ROC_FOR_ORDERING_CLIENT_REQUESTS))

ENABLE_CHIP_VERIFICATION_HARNESS	:= 0
$(eval $(call add_define,ENABLE_CHIP_VERIFICATION_HARNESS))

RESET_TO_BL31				:= 1

PROGRAMMABLE_RESET_ADDRESS		:= 0

COLD_BOOT_SINGLE_CPU			:= 1

RELOCATE_BL32_IMAGE			:= 1

# platform settings
TZDRAM_BASE				:= 0x30000000
$(eval $(call add_define,TZDRAM_BASE))

PLATFORM_CLUSTER_COUNT			:= 2
$(eval $(call add_define,PLATFORM_CLUSTER_COUNT))

PLATFORM_MAX_CPUS_PER_CLUSTER		:= 4
$(eval $(call add_define,PLATFORM_MAX_CPUS_PER_CLUSTER))

MAX_XLAT_TABLES				:= 25
$(eval $(call add_define,MAX_XLAT_TABLES))

MAX_MMAP_REGIONS			:= 30
$(eval $(call add_define,MAX_MMAP_REGIONS))

# platform files
PLAT_INCLUDES		+=	-Iplat/nvidia/tegra/include/t186 \
				-I${SOC_DIR}/drivers/include

BL31_SOURCES		+=	${TEGRA_GICv2_SOURCES}			\
				drivers/ti/uart/aarch64/16550_console.S	\
				lib/cpus/aarch64/denver.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				${TEGRA_DRIVERS}/bpmp_ipc/intf.c	\
				${TEGRA_DRIVERS}/bpmp_ipc/ivc.c		\
				${TEGRA_DRIVERS}/gpcdma/gpcdma.c	\
				${TEGRA_DRIVERS}/memctrl/memctrl_v2.c	\
				${TEGRA_DRIVERS}/smmu/smmu.c		\
				${SOC_DIR}/drivers/mce/mce.c		\
				${SOC_DIR}/drivers/mce/ari.c		\
				${SOC_DIR}/drivers/mce/nvg.c		\
				${SOC_DIR}/drivers/mce/aarch64/nvg_helpers.S \
				$(SOC_DIR)/drivers/se/se.c		\
				${SOC_DIR}/plat_memctrl.c		\
				${SOC_DIR}/plat_psci_handlers.c		\
				${SOC_DIR}/plat_setup.c			\
				${SOC_DIR}/plat_secondary.c		\
				${SOC_DIR}/plat_sip_calls.c		\
				${SOC_DIR}/plat_smmu.c			\
				${SOC_DIR}/plat_trampoline.S

# Enable workarounds for selected Cortex-A57 erratas.
A57_DISABLE_NON_TEMPORAL_HINT	:=	1
ERRATA_A57_806969		:=	1
ERRATA_A57_813419		:=	1
ERRATA_A57_813420		:=	1
ERRATA_A57_826974		:=	1
ERRATA_A57_826977		:=	1
ERRATA_A57_828024		:=	1
ERRATA_A57_829520		:=	1
ERRATA_A57_833471		:=	1

# Enable higher performance Non-cacheable load forwarding
A57_ENABLE_NONCACHEABLE_LOAD_FWD	:=	1
