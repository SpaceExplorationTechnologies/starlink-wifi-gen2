# Copyright (c) 2020-2021, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# RD-N2 platform uses GIC-Clayton which is based on GICv4.1
GIC_ENABLE_V4_EXTN	:=	1

include plat/arm/css/sgi/sgi-common.mk

RDN2_BASE		=	plat/arm/board/rdn2

PLAT_INCLUDES		+=	-I${RDN2_BASE}/include/

SGI_CPU_SOURCES		:=	lib/cpus/aarch64/neoverse_n2.S

PLAT_BL_COMMON_SOURCES	+=	${CSS_ENT_BASE}/sgi_plat_v2.c

BL1_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${RDN2_BASE}/rdn2_err.c

BL2_SOURCES		+=	${RDN2_BASE}/rdn2_plat.c		\
				${RDN2_BASE}/rdn2_security.c		\
				${RDN2_BASE}/rdn2_err.c			\
				lib/utils/mem_region.c			\
				drivers/arm/tzc/tzc400.c		\
				plat/arm/common/arm_tzc400.c		\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${RDN2_BASE}/rdn2_plat.c		\
				${RDN2_BASE}/rdn2_topology.c		\
				drivers/cfi/v2m/v2m_flash.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL1_SOURCES		+=	${RDN2_BASE}/rdn2_trusted_boot.c
BL2_SOURCES		+=	${RDN2_BASE}/rdn2_trusted_boot.c
endif

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=	${RDN2_BASE}/fdts/${PLAT}_fw_config.dts	\
				${RDN2_BASE}/fdts/${PLAT}_tb_fw_config.dts
FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))

FDT_SOURCES		+=	${RDN2_BASE}/fdts/${PLAT}_nt_fw_config.dts
NT_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${NT_FW_CONFIG},--nt-fw-config))

override CTX_INCLUDE_AARCH32_REGS	:= 0
override ENABLE_AMU			:= 1
