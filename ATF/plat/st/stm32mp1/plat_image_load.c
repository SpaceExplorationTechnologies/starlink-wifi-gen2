/*
 * Copyright (c) 2016-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/desc_image_load.h>
#include <plat/common/platform.h>

/*******************************************************************************
 * This function flushes the data structures so that they are visible
 * in memory for the next BL image.
 ******************************************************************************/
void plat_flush_next_bl_params(void)
{
	flush_bl_params_desc();
}

/*******************************************************************************
 * This function returns the list of loadable images.
 ******************************************************************************/
bl_load_info_t *plat_get_bl_image_load_info(void)
{
	bl_mem_params_node_t *bl33 = get_bl_mem_params_node(BL33_IMAGE_ID);
	uint32_t ddr_ns_size = stm32mp_get_ddr_ns_size();

	/* Max size is non-secure DDR end address minus image_base */
	bl33->image_info.image_max_size = STM32MP_DDR_BASE + ddr_ns_size -
					  bl33->image_info.image_base;

	return get_bl_load_info_from_mem_params_desc();
}

/*******************************************************************************
 * This function returns the list of executable images.
 ******************************************************************************/
bl_params_t *plat_get_next_bl_params(void)
{
	return get_next_bl_params_from_mem_params_desc();
}
