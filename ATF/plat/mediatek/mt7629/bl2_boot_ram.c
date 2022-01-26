/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_memmap.h>
#include <lib/mmio.h>
#include <bl2_boot_dev.h>

#define MT7629_DRAM_BASE		0x40000000

#define FIP_BASE			0x20000
#define FIP_SIZE			0x200000

#define DEBUGGER_HOOK_ADDR		0x100200

const io_block_spec_t mtk_boot_dev_fip_spec = {
	.offset	= MT7629_DRAM_BASE + FIP_BASE,
	.length = FIP_SIZE,
};

void mtk_boot_dev_setup(const io_dev_connector_t **boot_dev_con,
			uintptr_t *boot_dev_handle)
{
	int result;

#ifdef RAM_BOOT_DEBUGGER_HOOK
	/* debugger hook */
	mmio_write_32(DEBUGGER_HOOK_ADDR, 0);

	while (mmio_read_32(DEBUGGER_HOOK_ADDR) == 0)
		;
#endif

	result = register_io_dev_memmap(boot_dev_con);
	assert(result == 0);

	result = io_dev_open(*boot_dev_con, (uintptr_t)NULL, boot_dev_handle);
	assert(result == 0);

	/* Ignore improbable errors in release builds */
	(void)result;
}
