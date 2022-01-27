/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <plat/arm/board/common/v2m_def.h>
#include <plat/arm/common/arm_def.h>
#include <plat/arm/css/common/css_def.h>

/* UART related constants */
#define PLAT_ARM_BOOT_UART_BASE			0x2A400000
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ		50000000

#define PLAT_ARM_RUN_UART_BASE		0x2A410000
#define PLAT_ARM_RUN_UART_CLK_IN_HZ	50000000

#define PLAT_ARM_SP_MIN_RUN_UART_BASE		0x2A410000
#define PLAT_ARM_SP_MIN_RUN_UART_CLK_IN_HZ	50000000

#define PLAT_ARM_CRASH_UART_BASE		PLAT_ARM_RUN_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ		PLAT_ARM_RUN_UART_CLK_IN_HZ

#define PLAT_ARM_DRAM2_BASE			ULL(0x8080000000)
#define PLAT_ARM_DRAM2_SIZE			ULL(0xF80000000)

/* N1SDP remote chip at 4 TB offset */
#define PLAT_ARM_REMOTE_CHIP_OFFSET		(ULL(1) << 42)

#define N1SDP_REMOTE_DRAM1_BASE			ARM_DRAM1_BASE + \
						PLAT_ARM_REMOTE_CHIP_OFFSET
#define N1SDP_REMOTE_DRAM1_SIZE			ARM_DRAM1_SIZE

#define N1SDP_REMOTE_DRAM2_BASE			PLAT_ARM_DRAM2_BASE + \
						PLAT_ARM_REMOTE_CHIP_OFFSET
#define N1SDP_REMOTE_DRAM2_SIZE			PLAT_ARM_DRAM2_SIZE

/*
 * N1SDP platform supports RDIMMs with ECC capability. To use the ECC
 * capability, the entire DDR memory space has to be zeroed out before
 * enabling the ECC bits in DMC620. To access the complete DDR memory
 * along with remote chip's DDR memory, which is at 4 TB offset, physical
 * and virtual address space limits are extended to 43-bits.
 */
#ifdef __aarch64__
#define PLAT_PHY_ADDR_SPACE_SIZE		(1ULL << 43)
#define PLAT_VIRT_ADDR_SPACE_SIZE		(1ULL << 43)
#else
#define PLAT_PHY_ADDR_SPACE_SIZE		(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE		(1ULL << 32)
#endif

#if CSS_USE_SCMI_SDS_DRIVER
#define N1SDP_SCMI_PAYLOAD_BASE			0x45400000
#else
#define PLAT_CSS_SCP_COM_SHARED_MEM_BASE	0x45400000
#endif

#define PLAT_ARM_TRUSTED_SRAM_SIZE		0x00080000	/* 512 KB */
#define PLAT_ARM_MAX_BL31_SIZE			0X20000

/*******************************************************************************
 * N1SDP topology related constants
 ******************************************************************************/
#define N1SDP_MAX_CPUS_PER_CLUSTER		U(2)
#define PLAT_ARM_CLUSTER_COUNT			U(2)
#define PLAT_N1SDP_CHIP_COUNT			U(2)
#define N1SDP_MAX_CLUSTERS_PER_CHIP		U(2)
#define N1SDP_MAX_PE_PER_CPU			U(1)

#define PLATFORM_CORE_COUNT			(PLAT_N1SDP_CHIP_COUNT *	\
						PLAT_ARM_CLUSTER_COUNT *	\
						N1SDP_MAX_CPUS_PER_CLUSTER *	\
						N1SDP_MAX_PE_PER_CPU)

/* System power domain level */
#define CSS_SYSTEM_PWR_DMN_LVL			ARM_PWR_LVL3

/*
 * PLAT_ARM_MMAP_ENTRIES depends on the number of entries in the
 * plat_arm_mmap array defined for each BL stage.
 */
#define PLAT_ARM_MMAP_ENTRIES			9
#define MAX_XLAT_TABLES				10

#define PLATFORM_STACK_SIZE			0x400

#define PLAT_ARM_NSTIMER_FRAME_ID		0
#define PLAT_CSS_MHU_BASE			0x45000000
#define PLAT_MAX_PWR_LVL			2

#define PLAT_ARM_G1S_IRQS			ARM_G1S_IRQS,			\
						CSS_IRQ_MHU
#define PLAT_ARM_G0_IRQS			ARM_G0_IRQS

#define PLAT_ARM_G1S_IRQ_PROPS(grp)		CSS_G1S_IRQ_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)		ARM_G0_IRQ_PROPS(grp)


#define N1SDP_DEVICE_BASE			ULL(0x08000000)
#define N1SDP_DEVICE_SIZE			ULL(0x48000000)
#define N1SDP_REMOTE_DEVICE_BASE		N1SDP_DEVICE_BASE + \
						PLAT_ARM_REMOTE_CHIP_OFFSET
#define N1SDP_REMOTE_DEVICE_SIZE		N1SDP_DEVICE_SIZE

#define N1SDP_MAP_DEVICE		MAP_REGION_FLAT(	\
					N1SDP_DEVICE_BASE,	\
					N1SDP_DEVICE_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE)

#define ARM_MAP_DRAM1			MAP_REGION_FLAT(	\
					ARM_DRAM1_BASE,		\
					ARM_DRAM1_SIZE,		\
					MT_MEMORY | MT_RW | MT_NS)

#define N1SDP_MAP_REMOTE_DEVICE		MAP_REGION_FLAT(		\
					N1SDP_REMOTE_DEVICE_BASE,	\
					N1SDP_REMOTE_DEVICE_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE)

#define N1SDP_MAP_REMOTE_DRAM1		MAP_REGION_FLAT(		\
					N1SDP_REMOTE_DRAM1_BASE,	\
					N1SDP_REMOTE_DRAM1_SIZE,	\
					MT_MEMORY | MT_RW | MT_NS)

#define N1SDP_MAP_REMOTE_DRAM2		MAP_REGION_FLAT(		\
					N1SDP_REMOTE_DRAM2_BASE,	\
					N1SDP_REMOTE_DRAM2_SIZE,	\
					MT_MEMORY | MT_RW | MT_NS)

/* GIC related constants */
#define PLAT_ARM_GICD_BASE			0x30000000
#define PLAT_ARM_GICC_BASE			0x2C000000
#define PLAT_ARM_GICR_BASE			0x300C0000

/* Platform ID address */
#define SSC_VERSION				(SSC_REG_BASE + SSC_VERSION_OFFSET)

/* Secure Watchdog Constants */
#define SBSA_SECURE_WDOG_BASE			UL(0x2A480000)
#define SBSA_SECURE_WDOG_TIMEOUT		UL(100)

/* Number of SCMI channels on the platform */
#define PLAT_ARM_SCMI_CHANNEL_COUNT		U(1)

#endif /* PLATFORM_DEF_H */
