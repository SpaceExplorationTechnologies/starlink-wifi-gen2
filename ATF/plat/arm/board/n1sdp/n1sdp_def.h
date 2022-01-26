/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef N1SDP_DEF_H
#define N1SDP_DEF_H

/* Non-secure SRAM MMU mapping */
#define N1SDP_NS_SRAM_BASE			(0x06000000)
#define N1SDP_NS_SRAM_SIZE			(0x00010000)
#define N1SDP_MAP_NS_SRAM			MAP_REGION_FLAT(	\
						N1SDP_NS_SRAM_BASE,	\
						N1SDP_NS_SRAM_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

/* SDS Platform information defines */
#define N1SDP_SDS_PLATFORM_INFO_STRUCT_ID	8
#define N1SDP_SDS_PLATFORM_INFO_OFFSET		0
#define N1SDP_SDS_PLATFORM_INFO_SIZE		4
#define N1SDP_MAX_DDR_CAPACITY_GB		64
#define N1SDP_MAX_SLAVE_COUNT			16

/* SDS BL33 image information defines */
#define N1SDP_SDS_BL33_INFO_STRUCT_ID		9
#define N1SDP_SDS_BL33_INFO_OFFSET		0
#define N1SDP_SDS_BL33_INFO_SIZE		12

/* DMC memory command registers */
#define N1SDP_DMC0_MEMC_CMD_REG			0x4E000008
#define N1SDP_DMC1_MEMC_CMD_REG			0x4E100008

/* DMC ERR0CTLR0 registers */
#define N1SDP_DMC0_ERR0CTLR0_REG		0x4E000708
#define N1SDP_DMC1_ERR0CTLR0_REG		0x4E100708

/* Remote DMC memory command registers */
#define N1SDP_REMOTE_DMC0_MEMC_CMD_REG		PLAT_ARM_REMOTE_CHIP_OFFSET +\
							N1SDP_DMC0_MEMC_CMD_REG
#define N1SDP_REMOTE_DMC1_MEMC_CMD_REG		PLAT_ARM_REMOTE_CHIP_OFFSET +\
							N1SDP_DMC1_MEMC_CMD_REG

/* Remote DMC ERR0CTLR0 registers */
#define N1SDP_REMOTE_DMC0_ERR0CTLR0_REG		PLAT_ARM_REMOTE_CHIP_OFFSET +\
							N1SDP_DMC0_ERR0CTLR0_REG
#define N1SDP_REMOTE_DMC1_ERR0CTLR0_REG		PLAT_ARM_REMOTE_CHIP_OFFSET +\
							N1SDP_DMC1_ERR0CTLR0_REG

/* DMC memory commands */
#define N1SDP_DMC_MEMC_CMD_CONFIG		0
#define N1SDP_DMC_MEMC_CMD_READY		3

/* DMC ECC enable bit in ERR0CTLR0 register */
#define N1SDP_DMC_ERR0CTLR0_ECC_EN		0x1

/* Base address of non-secure SRAM where Platform information will be filled */
#define N1SDP_PLATFORM_INFO_BASE		0x06008000

#endif /* N1SDP_DEF_H */
