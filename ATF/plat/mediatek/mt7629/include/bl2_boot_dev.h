/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL2_BOOT_DEV_H
#define BL2_BOOT_DEV_H

#include <drivers/io/io_driver.h>
#include <drivers/io/io_block.h>

extern const io_block_spec_t mtk_boot_dev_fip0_spec;
extern const io_block_spec_t mtk_boot_dev_fip1_spec;

void mtk_boot_dev_setup(const io_dev_connector_t **boot_dev_con,
			uintptr_t *boot_dev_handle);

#endif /* BL2_BOOT_DEV_H */
