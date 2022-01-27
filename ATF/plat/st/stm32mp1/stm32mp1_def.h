/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_DEF_H
#define STM32MP1_DEF_H

#include <common/tbbr/tbbr_img_def.h>
#include <drivers/st/stm32mp1_rcc.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <dt-bindings/reset/stm32mp1-resets.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

#ifndef __ASSEMBLER__
#include <drivers/st/bsec.h>
#include <drivers/st/stm32mp1_clk.h>

#include <boot_api.h>
#include <stm32mp_auth.h>
#include <stm32mp_common.h>
#include <stm32mp_dt.h>
#include <stm32mp_shres_helpers.h>
#include <stm32mp1_dbgmcu.h>
#include <stm32mp1_private.h>
#include <stm32mp1_shared_resources.h>
#endif

/*******************************************************************************
 * CHIP ID
 ******************************************************************************/
#define STM32MP157C_PART_NB	U(0x05000000)
#define STM32MP157A_PART_NB	U(0x05000001)
#define STM32MP153C_PART_NB	U(0x05000024)
#define STM32MP153A_PART_NB	U(0x05000025)
#define STM32MP151C_PART_NB	U(0x0500002E)
#define STM32MP151A_PART_NB	U(0x0500002F)
#define STM32MP157F_PART_NB	U(0x05000080)
#define STM32MP157D_PART_NB	U(0x05000081)
#define STM32MP153F_PART_NB	U(0x050000A4)
#define STM32MP153D_PART_NB	U(0x050000A5)
#define STM32MP151F_PART_NB	U(0x050000AE)
#define STM32MP151D_PART_NB	U(0x050000AF)

#define STM32MP1_REV_B		U(0x2000)
#define STM32MP1_REV_Z		U(0x2001)

/*******************************************************************************
 * PACKAGE ID
 ******************************************************************************/
#define PKG_AA_LFBGA448		U(4)
#define PKG_AB_LFBGA354		U(3)
#define PKG_AC_TFBGA361		U(2)
#define PKG_AD_TFBGA257		U(1)

/*******************************************************************************
 * STM32MP1 memory map related constants
 ******************************************************************************/
#define STM32MP_ROM_BASE		U(0x00000000)
#define STM32MP_ROM_SIZE		U(0x00020000)

#define STM32MP_SYSRAM_BASE		U(0x2FFC0000)
#define STM32MP_SYSRAM_SIZE		U(0x00040000)

#define STM32MP_NS_SYSRAM_SIZE		PAGE_SIZE
#define STM32MP_NS_SYSRAM_BASE		(STM32MP_SYSRAM_BASE + \
					 STM32MP_SYSRAM_SIZE - \
					 STM32MP_NS_SYSRAM_SIZE)

#define STM32MP_SCMI_NS_SHM_BASE	STM32MP_NS_SYSRAM_BASE
#define STM32MP_SCMI_NS_SHM_SIZE	STM32MP_NS_SYSRAM_SIZE

#define STM32MP_SEC_SYSRAM_BASE		STM32MP_SYSRAM_BASE
#define STM32MP_SEC_SYSRAM_SIZE		(STM32MP_SYSRAM_SIZE - \
					 STM32MP_NS_SYSRAM_SIZE)

/* DDR configuration */
#define STM32MP_DDR_BASE		U(0xC0000000)
#define STM32MP_DDR_MAX_SIZE		U(0x40000000)	/* Max 1GB */
#ifdef AARCH32_SP_OPTEE
#define STM32MP_DDR_S_SIZE		U(0x01E00000)	/* 30 MB */
#define STM32MP_DDR_SHMEM_SIZE		U(0x00200000)	/* 2 MB */
#else
#define STM32MP_DDR_S_SIZE		U(0)
#define STM32MP_DDR_SHMEM_SIZE		U(0)
#endif

/* DDR power initializations */
#ifndef __ASSEMBLER__
enum ddr_type {
	STM32MP_DDR3,
	STM32MP_LPDDR2,
	STM32MP_LPDDR3
};
#endif

/* Section used inside TF binaries */
#define STM32MP_PARAM_LOAD_SIZE		U(0x00002400)	/* 9 KB for param */
/* 256 Octets reserved for header */
#define STM32MP_HEADER_SIZE		U(0x00000100)

#define STM32MP_BINARY_BASE		(STM32MP_SEC_SYSRAM_BASE +	\
					 STM32MP_PARAM_LOAD_SIZE +	\
					 STM32MP_HEADER_SIZE)

#define STM32MP_BINARY_SIZE		(STM32MP_SEC_SYSRAM_SIZE -	\
					 (STM32MP_PARAM_LOAD_SIZE +	\
					  STM32MP_HEADER_SIZE))

#ifdef AARCH32_SP_OPTEE
#define STM32MP_BL32_SIZE		U(0)

#define STM32MP_OPTEE_BASE		STM32MP_SEC_SYSRAM_BASE

#define STM32MP_OPTEE_SIZE		(STM32MP_DTB_BASE -  \
					 STM32MP_OPTEE_BASE)
#else
#if STACK_PROTECTOR_ENABLED
#define STM32MP_BL32_SIZE		U(0x00012000)	/* 72 KB for BL32 */
#else
#define STM32MP_BL32_SIZE		U(0x00011000)	/* 68 KB for BL32 */
#endif
#endif

#define STM32MP_BL32_BASE		(STM32MP_SEC_SYSRAM_BASE + \
					 STM32MP_SEC_SYSRAM_SIZE - \
					 STM32MP_BL32_SIZE)

#ifdef AARCH32_SP_OPTEE
#if STACK_PROTECTOR_ENABLED
#define STM32MP_BL2_SIZE		U(0x0001A000)	/* 100 KB for BL2 */
#else
#define STM32MP_BL2_SIZE		U(0x00018000)	/* 92 KB for BL2 */
#endif
#else
#if STACK_PROTECTOR_ENABLED
#define STM32MP_BL2_SIZE		U(0x00019000)	/* 96 KB for BL2 */
#else
#define STM32MP_BL2_SIZE		U(0x00017000)	/* 88 KB for BL2 */
#endif
#endif

#define STM32MP_BL2_BASE		(STM32MP_BL32_BASE - \
					 STM32MP_BL2_SIZE)

/* BL2 and BL32/sp_min require 4 tables */
#define MAX_XLAT_TABLES			U(4)		/* 16 KB for mapping */

/*
 * MAX_MMAP_REGIONS is usually:
 * BL stm32mp1_mmap size + mmap regions in *_plat_arch_setup
 */
#if defined(IMAGE_BL2)
  #define MAX_MMAP_REGIONS		11
#endif
#if defined(IMAGE_BL32)
  #define MAX_MMAP_REGIONS		6
#endif

/* DTB initialization value */
#define STM32MP_DTB_SIZE		U(0x00005000)	/* 20 KB for DTB */

#define STM32MP_DTB_BASE		(STM32MP_BL2_BASE - \
					 STM32MP_DTB_SIZE)

#define STM32MP_BL33_BASE		(STM32MP_DDR_BASE + U(0x100000))

/* Define maximum page size for NAND devices */
#define PLATFORM_MTD_MAX_PAGE_SIZE	U(0x1000)

/*******************************************************************************
 * STM32MP1 RAW partition offset for MTD devices
 ******************************************************************************/
#define STM32MP_NOR_BL33_OFFSET		U(0x00080000)
#ifdef AARCH32_SP_OPTEE
#define STM32MP_NOR_TEEH_OFFSET		U(0x00280000)
#define STM32MP_NOR_TEED_OFFSET		U(0x002C0000)
#define STM32MP_NOR_TEEX_OFFSET		U(0x00300000)
#endif

#define STM32MP_NAND_BL33_OFFSET	U(0x00200000)
#ifdef AARCH32_SP_OPTEE
#define STM32MP_NAND_TEEH_OFFSET	U(0x00600000)
#define STM32MP_NAND_TEED_OFFSET	U(0x00680000)
#define STM32MP_NAND_TEEX_OFFSET	U(0x00700000)
#endif

/*******************************************************************************
 * STM32MP1 device/io map related constants (used for MMU)
 ******************************************************************************/
#define STM32MP1_DEVICE1_BASE		U(0x40000000)
#define STM32MP1_DEVICE1_SIZE		U(0x40000000)

#define STM32MP1_DEVICE2_BASE		U(0x80000000)
#define STM32MP1_DEVICE2_SIZE		U(0x40000000)

/*******************************************************************************
 * STM32MP1 RCC
 ******************************************************************************/
#define RCC_BASE			U(0x50000000)

/*******************************************************************************
 * STM32MP1 PWR
 ******************************************************************************/
#define PWR_BASE			U(0x50001000)

/*******************************************************************************
 * STM32MP1 GPIO
 ******************************************************************************/
#define GPIOA_BASE			U(0x50002000)
#define GPIOB_BASE			U(0x50003000)
#define GPIOC_BASE			U(0x50004000)
#define GPIOD_BASE			U(0x50005000)
#define GPIOE_BASE			U(0x50006000)
#define GPIOF_BASE			U(0x50007000)
#define GPIOG_BASE			U(0x50008000)
#define GPIOH_BASE			U(0x50009000)
#define GPIOI_BASE			U(0x5000A000)
#define GPIOJ_BASE			U(0x5000B000)
#define GPIOK_BASE			U(0x5000C000)
#define GPIOZ_BASE			U(0x54004000)
#define GPIO_BANK_OFFSET		U(0x1000)

/* Bank IDs used in GPIO driver API */
#define GPIO_BANK_A			U(0)
#define GPIO_BANK_B			U(1)
#define GPIO_BANK_C			U(2)
#define GPIO_BANK_D			U(3)
#define GPIO_BANK_E			U(4)
#define GPIO_BANK_F			U(5)
#define GPIO_BANK_G			U(6)
#define GPIO_BANK_H			U(7)
#define GPIO_BANK_I			U(8)
#define GPIO_BANK_J			U(9)
#define GPIO_BANK_K			U(10)
#define GPIO_BANK_Z			U(25)

#define STM32MP_GPIOZ_PIN_MAX_COUNT	8

/*******************************************************************************
 * STM32MP1 UART
 ******************************************************************************/
#define USART1_BASE			U(0x5C000000)
#define USART2_BASE			U(0x4000E000)
#define USART3_BASE			U(0x4000F000)
#define UART4_BASE			U(0x40010000)
#define UART5_BASE			U(0x40011000)
#define USART6_BASE			U(0x44003000)
#define UART7_BASE			U(0x40018000)
#define UART8_BASE			U(0x40019000)
#define STM32MP_UART_BAUDRATE		U(115200)

/* For UART crash console */
#define STM32MP_DEBUG_USART_BASE	UART4_BASE
/* UART4 on HSI@64MHz, TX on GPIOG11 Alternate 6 */
#define STM32MP_DEBUG_USART_CLK_FRQ	64000000
#define DEBUG_UART_TX_GPIO_BANK_ADDRESS	GPIOG_BASE
#define DEBUG_UART_TX_GPIO_BANK_CLK_REG	RCC_MP_AHB4ENSETR
#define DEBUG_UART_TX_GPIO_BANK_CLK_EN	RCC_MP_AHB4ENSETR_GPIOGEN
#define DEBUG_UART_TX_GPIO_PORT		11
#define DEBUG_UART_TX_GPIO_ALTERNATE	6
#define DEBUG_UART_TX_CLKSRC_REG	RCC_UART24CKSELR
#define DEBUG_UART_TX_CLKSRC		RCC_UART24CKSELR_HSI
#define DEBUG_UART_TX_EN_REG		RCC_MP_APB1ENSETR
#define DEBUG_UART_TX_EN		RCC_MP_APB1ENSETR_UART4EN

/*******************************************************************************
 * STM32MP1 ETZPC
 ******************************************************************************/
#define STM32MP1_ETZPC_BASE		U(0x5C007000)

/* ETZPC TZMA IDs */
#define STM32MP1_ETZPC_TZMA_ROM		U(0)
#define STM32MP1_ETZPC_TZMA_SYSRAM	U(1)

#define STM32MP1_ETZPC_TZMA_ALL_SECURE	GENMASK_32(9, 0)

/* ETZPC DECPROT IDs */
#define STM32MP1_ETZPC_STGENC_ID	0
#define STM32MP1_ETZPC_BKPSRAM_ID	1
#define STM32MP1_ETZPC_IWDG1_ID		2
#define STM32MP1_ETZPC_USART1_ID	3
#define STM32MP1_ETZPC_SPI6_ID		4
#define STM32MP1_ETZPC_I2C4_ID		5
#define STM32MP1_ETZPC_RNG1_ID		7
#define STM32MP1_ETZPC_HASH1_ID		8
#define STM32MP1_ETZPC_CRYP1_ID		9
#define STM32MP1_ETZPC_DDRCTRL_ID	10
#define STM32MP1_ETZPC_DDRPHYC_ID	11
#define STM32MP1_ETZPC_I2C6_ID		12
#define STM32MP1_ETZPC_SEC_ID_LIMIT	13

#define STM32MP1_ETZPC_TIM2_ID		16
#define STM32MP1_ETZPC_TIM3_ID		17
#define STM32MP1_ETZPC_TIM4_ID		18
#define STM32MP1_ETZPC_TIM5_ID		19
#define STM32MP1_ETZPC_TIM6_ID		20
#define STM32MP1_ETZPC_TIM7_ID		21
#define STM32MP1_ETZPC_TIM12_ID		22
#define STM32MP1_ETZPC_TIM13_ID		23
#define STM32MP1_ETZPC_TIM14_ID		24
#define STM32MP1_ETZPC_LPTIM1_ID	25
#define STM32MP1_ETZPC_WWDG1_ID		26
#define STM32MP1_ETZPC_SPI2_ID		27
#define STM32MP1_ETZPC_SPI3_ID		28
#define STM32MP1_ETZPC_SPDIFRX_ID	29
#define STM32MP1_ETZPC_USART2_ID	30
#define STM32MP1_ETZPC_USART3_ID	31
#define STM32MP1_ETZPC_UART4_ID		32
#define STM32MP1_ETZPC_UART5_ID		33
#define STM32MP1_ETZPC_I2C1_ID		34
#define STM32MP1_ETZPC_I2C2_ID		35
#define STM32MP1_ETZPC_I2C3_ID		36
#define STM32MP1_ETZPC_I2C5_ID		37
#define STM32MP1_ETZPC_CEC_ID		38
#define STM32MP1_ETZPC_DAC_ID		39
#define STM32MP1_ETZPC_UART7_ID		40
#define STM32MP1_ETZPC_UART8_ID		41
#define STM32MP1_ETZPC_MDIOS_ID		44
#define STM32MP1_ETZPC_TIM1_ID		48
#define STM32MP1_ETZPC_TIM8_ID		49
#define STM32MP1_ETZPC_USART6_ID	51
#define STM32MP1_ETZPC_SPI1_ID		52
#define STM32MP1_ETZPC_SPI4_ID		53
#define STM32MP1_ETZPC_TIM15_ID		54
#define STM32MP1_ETZPC_TIM16_ID		55
#define STM32MP1_ETZPC_TIM17_ID		56
#define STM32MP1_ETZPC_SPI5_ID		57
#define STM32MP1_ETZPC_SAI1_ID		58
#define STM32MP1_ETZPC_SAI2_ID		59
#define STM32MP1_ETZPC_SAI3_ID		60
#define STM32MP1_ETZPC_DFSDM_ID		61
#define STM32MP1_ETZPC_TT_FDCAN_ID	62
#define STM32MP1_ETZPC_LPTIM2_ID	64
#define STM32MP1_ETZPC_LPTIM3_ID	65
#define STM32MP1_ETZPC_LPTIM4_ID	66
#define STM32MP1_ETZPC_LPTIM5_ID	67
#define STM32MP1_ETZPC_SAI4_ID		68
#define STM32MP1_ETZPC_VREFBUF_ID	69
#define STM32MP1_ETZPC_DCMI_ID		70
#define STM32MP1_ETZPC_CRC2_ID		71
#define STM32MP1_ETZPC_ADC_ID		72
#define STM32MP1_ETZPC_HASH2_ID		73
#define STM32MP1_ETZPC_RNG2_ID		74
#define STM32MP1_ETZPC_CRYP2_ID		75
#define STM32MP1_ETZPC_SRAM1_ID		80
#define STM32MP1_ETZPC_SRAM2_ID		81
#define STM32MP1_ETZPC_SRAM3_ID		82
#define STM32MP1_ETZPC_SRAM4_ID		83
#define STM32MP1_ETZPC_RETRAM_ID	84
#define STM32MP1_ETZPC_OTG_ID		85
#define STM32MP1_ETZPC_SDMMC3_ID	86
#define STM32MP1_ETZPC_DLYBSD3_ID	87
#define STM32MP1_ETZPC_DMA1_ID		88
#define STM32MP1_ETZPC_DMA2_ID		89
#define STM32MP1_ETZPC_DMAMUX_ID	90
#define STM32MP1_ETZPC_FMC_ID		91
#define STM32MP1_ETZPC_QSPI_ID		92
#define STM32MP1_ETZPC_DLYBQ_ID		93
#define STM32MP1_ETZPC_ETH_ID		94
#define STM32MP1_ETZPC_RSV_ID		95

#define STM32MP_ETZPC_MAX_ID		96

/*******************************************************************************
 * STM32MP1 TZC (TZ400)
 ******************************************************************************/
#define STM32MP1_TZC_BASE		U(0x5C006000)

#define STM32MP1_TZC_A7_ID		U(0)
#define STM32MP1_TZC_M4_ID		U(1)
#define STM32MP1_TZC_LCD_ID		U(3)
#define STM32MP1_TZC_GPU_ID		U(4)
#define STM32MP1_TZC_MDMA_ID		U(5)
#define STM32MP1_TZC_DMA_ID		U(6)
#define STM32MP1_TZC_USB_HOST_ID	U(7)
#define STM32MP1_TZC_USB_OTG_ID		U(8)
#define STM32MP1_TZC_SDMMC_ID		U(9)
#define STM32MP1_TZC_ETH_ID		U(10)
#define STM32MP1_TZC_DAP_ID		U(15)

#define STM32MP1_FILTER_BIT_ALL		U(3)

/*******************************************************************************
 * STM32MP1 SDMMC
 ******************************************************************************/
#define STM32MP_SDMMC1_BASE		U(0x58005000)
#define STM32MP_SDMMC2_BASE		U(0x58007000)
#define STM32MP_SDMMC3_BASE		U(0x48004000)

#define STM32MP_MMC_INIT_FREQ			U(400000)	/*400 KHz*/
#define STM32MP_SD_NORMAL_SPEED_MAX_FREQ	U(25000000)	/*25 MHz*/
#define STM32MP_SD_HIGH_SPEED_MAX_FREQ		U(50000000)	/*50 MHz*/
#define STM32MP_EMMC_NORMAL_SPEED_MAX_FREQ	U(26000000)	/*26 MHz*/
#define STM32MP_EMMC_HIGH_SPEED_MAX_FREQ	U(52000000)	/*52 MHz*/

/*******************************************************************************
 * STM32MP1 BSEC / OTP
 ******************************************************************************/
#define STM32MP1_OTP_MAX_ID		0x5FU
#define STM32MP1_UPPER_OTP_START	0x20U

#define OTP_MAX_SIZE			(STM32MP1_OTP_MAX_ID + 1U)

/* OTP offsets */
#define DATA0_OTP			U(0)
#define PART_NUMBER_OTP			U(1)
#define NAND_OTP			U(9)
#define PACKAGE_OTP			U(16)
#define HW2_OTP				U(18)

/* OTP mask */
/* DATA0 */
#define DATA0_OTP_SECURED		BIT(6)

/* PART NUMBER */
#define PART_NUMBER_OTP_PART_MASK	GENMASK_32(7, 0)
#define PART_NUMBER_OTP_PART_SHIFT	0

/* PACKAGE */
#define PACKAGE_OTP_PKG_MASK		GENMASK_32(29, 27)
#define PACKAGE_OTP_PKG_SHIFT		27

/* IWDG OTP */
#define HW2_OTP_IWDG_HW_POS		U(3)
#define HW2_OTP_IWDG_FZ_STOP_POS	U(5)
#define HW2_OTP_IWDG_FZ_STANDBY_POS	U(7)

/* HW2 OTP */
#define HW2_OTP_PRODUCT_BELOW_2V5	BIT(13)

/* NAND OTP */
/* NAND parameter storage flag */
#define NAND_PARAM_STORED_IN_OTP	BIT(31)

/* NAND page size in bytes */
#define NAND_PAGE_SIZE_MASK		GENMASK_32(30, 29)
#define NAND_PAGE_SIZE_SHIFT		29
#define NAND_PAGE_SIZE_2K		U(0)
#define NAND_PAGE_SIZE_4K		U(1)
#define NAND_PAGE_SIZE_8K		U(2)

/* NAND block size in pages */
#define NAND_BLOCK_SIZE_MASK		GENMASK_32(28, 27)
#define NAND_BLOCK_SIZE_SHIFT		27
#define NAND_BLOCK_SIZE_64_PAGES	U(0)
#define NAND_BLOCK_SIZE_128_PAGES	U(1)
#define NAND_BLOCK_SIZE_256_PAGES	U(2)

/* NAND number of block (in unit of 256 blocs) */
#define NAND_BLOCK_NB_MASK		GENMASK_32(26, 19)
#define NAND_BLOCK_NB_SHIFT		19
#define NAND_BLOCK_NB_UNIT		U(256)

/* NAND bus width in bits */
#define NAND_WIDTH_MASK			BIT(18)
#define NAND_WIDTH_SHIFT		18

/* NAND number of ECC bits per 512 bytes */
#define NAND_ECC_BIT_NB_MASK		GENMASK_32(17, 15)
#define NAND_ECC_BIT_NB_SHIFT		15
#define NAND_ECC_BIT_NB_UNSET		U(0)
#define NAND_ECC_BIT_NB_1_BITS		U(1)
#define NAND_ECC_BIT_NB_4_BITS		U(2)
#define NAND_ECC_BIT_NB_8_BITS		U(3)
#define NAND_ECC_ON_DIE			U(4)

/* NAND number of planes */
#define NAND_PLANE_BIT_NB_MASK		BIT(14)

/*******************************************************************************
 * STM32MP1 TAMP
 ******************************************************************************/
#define TAMP_BASE			U(0x5C00A000)
#define TAMP_BKP_REGISTER_BASE		(TAMP_BASE + U(0x100))

#if !(defined(__LINKER__) || defined(__ASSEMBLER__))
static inline uint32_t tamp_bkpr(uint32_t idx)
{
	return TAMP_BKP_REGISTER_BASE + (idx << 2);
}
#endif

/*******************************************************************************
 * STM32MP1 DDRCTRL
 ******************************************************************************/
#define DDRCTRL_BASE			U(0x5A003000)

/*******************************************************************************
 * STM32MP1 DDRPHYC
 ******************************************************************************/
#define DDRPHYC_BASE			U(0x5A004000)

/*******************************************************************************
 * STM32MP1 IWDG
 ******************************************************************************/
#define IWDG_MAX_INSTANCE		U(2)
#define IWDG1_INST			U(0)
#define IWDG2_INST			U(1)

#define IWDG1_BASE			U(0x5C003000)
#define IWDG2_BASE			U(0x5A002000)

/*******************************************************************************
 * Miscellaneous STM32MP1 peripherals base address
 ******************************************************************************/
#define BSEC_BASE			U(0x5C005000)
#define CRYP1_BASE			U(0x54001000)
#define DBGMCU_BASE			U(0x50081000)
#define HASH1_BASE			U(0x54002000)
#define I2C4_BASE			U(0x5C002000)
#define I2C6_BASE			U(0x5c009000)
#define RNG1_BASE			U(0x54003000)
#define RTC_BASE			U(0x5c004000)
#define SPI6_BASE			U(0x5c001000)
#define STGEN_BASE			U(0x5c008000)
#define SYSCFG_BASE			U(0x50020000)

/*******************************************************************************
 * Device Tree defines
 ******************************************************************************/
#define DT_BSEC_COMPAT			"st,stm32mp15-bsec"
#define DT_IWDG_COMPAT			"st,stm32mp1-iwdg"
#define DT_PWR_COMPAT			"st,stm32mp1,pwr-reg"
#define DT_RCC_CLK_COMPAT		"st,stm32mp1-rcc"

#endif /* STM32MP1_DEF_H */
