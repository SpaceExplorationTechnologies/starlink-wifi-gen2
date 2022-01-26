/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/common/platform.h>
#include <common/debug.h>
#include <spacex_stsafe.h>

extern char mtk_rotpk_hash[], mtk_rotpk_hash_end[];

int plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
	*key_ptr = mtk_rotpk_hash;
	*key_len = mtk_rotpk_hash_end - mtk_rotpk_hash;
	*flags = ROTPK_IS_HASH;

	return 0;
}

int plat_get_nv_ctr(void *cookie, unsigned int *nv_ctr)
{
	*nv_ctr = required_rollback_version;

	return 0;
}

int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
	uint32_t current_stsafe_ver = required_rollback_version;
	uint32_t desired_stsafe_ver = nv_ctr;

	int rc = update_anti_rollback(current_stsafe_ver, desired_stsafe_ver);
	if (rc) return rc;

	// If the update succeeded, update required_rollback_version so rollback checks
	// of other images in this boot don't keep decrementing STSAFE.
	required_rollback_version = desired_stsafe_ver;
	return 0;
}

int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	return get_mbedtls_heap_helper(heap_addr, heap_size);
}
