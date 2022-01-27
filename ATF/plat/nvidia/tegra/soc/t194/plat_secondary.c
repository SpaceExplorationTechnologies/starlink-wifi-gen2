/*
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include <mce.h>
#include <tegra194_private.h>
#include <tegra_def.h>
#include <tegra_private.h>

extern uint64_t tegra_bl31_phys_base;

#define MISCREG_AA64_RST_LOW		0x2004U
#define MISCREG_AA64_RST_HIGH		0x2008U

#define CPU_RESET_MODE_AA64		1U

/*******************************************************************************
 * Setup secondary CPU vectors
 ******************************************************************************/
void plat_secondary_setup(void)
{
	uint32_t addr_low, addr_high;
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint64_t cpu_reset_handler_base, cpu_reset_handler_size, tzdram_addr;
	uint64_t src_len_bytes = BL_END - tegra_bl31_phys_base;

	INFO("Setting up secondary CPU boot\n");

	tzdram_addr = params_from_bl2->tzdram_base +
		      tegra194_get_cpu_reset_handler_size();

	/*
	 * The BL31 code resides in the TZSRAM which loses state
	 * when we enter System Suspend. Copy the wakeup trampoline
	 * code to TZDRAM to help us exit from System Suspend.
	 */
	cpu_reset_handler_base = tegra194_get_cpu_reset_handler_base();
	cpu_reset_handler_size = tegra194_get_cpu_reset_handler_size();
	memcpy((void *)((uintptr_t)params_from_bl2->tzdram_base),
		(void *)((uintptr_t)cpu_reset_handler_base),
		cpu_reset_handler_size);

	/* TZDRAM base will be used as the "resume" address */
	addr_low = (uint32_t)params_from_bl2->tzdram_base | CPU_RESET_MODE_AA64;
	addr_high = (uint32_t)((params_from_bl2->tzdram_base >> 32U) & 0x7ffU);

	/* write lower 32 bits first, then the upper 11 bits */
	mmio_write_32(TEGRA_MISC_BASE + MISCREG_AA64_RST_LOW, addr_low);
	assert(mmio_read_32(TEGRA_MISC_BASE + MISCREG_AA64_RST_LOW) == addr_low);
	mmio_write_32(TEGRA_MISC_BASE + MISCREG_AA64_RST_HIGH, addr_high);
	assert(mmio_read_32(TEGRA_MISC_BASE + MISCREG_AA64_RST_HIGH) == addr_high);

	/* save reset vector to be used during SYSTEM_SUSPEND exit */
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_RESET_VECTOR_LO,
			addr_low);
	assert(mmio_read_32(TEGRA_SCRATCH_BASE + SCRATCH_RESET_VECTOR_LO) == addr_low);
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_RESET_VECTOR_HI,
			addr_high);
	assert(mmio_read_32(TEGRA_SCRATCH_BASE + SCRATCH_RESET_VECTOR_HI) == addr_high);
	mmio_write_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV72_LO,
						(uint32_t)tzdram_addr);
	assert(mmio_read_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV72_LO) == (uint32_t)tzdram_addr);
	mmio_write_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV72_HI,
						(uint32_t)src_len_bytes);
	assert(mmio_read_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV72_HI) == (uint32_t)src_len_bytes);
}
