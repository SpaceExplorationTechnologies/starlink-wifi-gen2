/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat_ipi.h>
#include <versal_def.h>
#include <plat_private.h>
#include <common/debug.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables.h>
#include <plat/common/platform.h>

/*
 * Table of regions to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to
 * configure_mmu_elx() will give the available subset of that,
 */
const mmap_region_t plat_versal_mmap[] = {
	MAP_REGION_FLAT(DEVICE0_BASE, DEVICE0_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DEVICE1_BASE, DEVICE1_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CRF_BASE, CRF_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(FPD_MAINCCI_BASE, FPD_MAINCCI_SIZE, MT_DEVICE | MT_RW |
			MT_SECURE),
	{ 0 }
};

const mmap_region_t *plat_versal_get_mmap(void)
{
	return plat_versal_mmap;
}

static void versal_print_platform_name(void)
{
	NOTICE("ATF running on Xilinx %s\n", PLATFORM_NAME);
}

void versal_config_setup(void)
{
	uint32_t val;

	/* Configure IPI data for versal */
	versal_ipi_config_table_init();

	versal_print_platform_name();

	/* Global timer init - Program time stamp reference clk */
	val = mmio_read_32(VERSAL_CRL_TIMESTAMP_REF_CTRL);
	val |= VERSAL_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_BIT;
	mmio_write_32(VERSAL_CRL_TIMESTAMP_REF_CTRL, val);

	/* Clear reset of timestamp reg */
	mmio_write_32(VERSAL_CRL_RST_TIMESTAMP_OFFSET, 0x0);

	/* Program freq register in System counter and enable system counter. */
	mmio_write_32(VERSAL_IOU_SCNTRS_BASE_FREQ, VERSAL_CPU_CLOCK);
	mmio_write_32(VERSAL_IOU_SCNTRS_COUNTER_CONTROL_REG,
		      VERSAL_IOU_SCNTRS_CONTROL_EN);

	generic_delay_timer_init();
}

unsigned int plat_get_syscnt_freq2(void)
{
	return VERSAL_CPU_CLOCK;
}

