/* SPDX-License-Identifier:	GPL-2.0+ */
/*
 * Copyright (C) 2021 MediaTek Incorporation. All Rights Reserved.
 *
 * SpaceX: This file contains the portions of the mtk_efuse driver from
 * OpenWRT necessary to perform an SMC to read the secure-boot-enabled bit
 * from the eFuse registers.
 */

#include <errno.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/arm-smccc.h>
#include <string.h>
#include <nmbm/nmbm.h>
#include <nmbm/nmbm-mtd.h>
#include <linux/mtd/mtd.h>

#include "colored_print.h"
#include "spacex_mtk_efuse_check.h"

int mtk_efuse_smc(u32 smc_fid,
			 u32 x1,
			 u32 x2,
			 u32 x3,
			 u32 x4,
			 struct arm_smccc_res *res)
{
	/* SMC64 calling convention is used if in 64bits Linux */
	if (sizeof(void *) == 8)
		smc_fid |= (0x1 << 30);

	arm_smccc_smc(smc_fid, x1, x2, x3, x4, 0x0, 0x0, 0x0, res);

	return 0;
}

int mtk_efuse_read(u32 efuse_field,
			  u8 *read_buffer,
			  u32 read_buffer_len)
{
	int ret;
	u32 idx;
	u32 offset;
	u32 efuse_len;
	u32 efuse_data[2] = { 0 };
	static struct arm_smccc_res res;

	/* get efuse length */
	ret = mtk_efuse_smc(MTK_SIP_EFUSE_GET_LEN,
			    efuse_field, 0x0, 0x0, 0x0,
			    &res);
	if (ret < 0)
		return ret;
	else if (res.a0 != MTK_EFUSE_SUCCESS) {
		pr_err("%s : get efuse length fail (%lu)\n",
		       __func__, res.a0);
		if (res.a0 == MTK_EFUSE_ERROR_EFUSE_FIELD_DISABLED)
			pr_err("%s : efuse field (%u) was disabled\n",
			       __func__, efuse_field);
		return -1;
	}
	efuse_len = res.a1;

	/* verify efuse_buffer */
	if (!read_buffer)
		return -EINVAL;
	if (read_buffer_len < efuse_len)
		return -ENOMEM;

	/* issue efuse read */
	ret = mtk_efuse_smc(MTK_SIP_EFUSE_READ,
			    efuse_field, 0x0, 0x0, 0x0,
			    &res);
	if (ret < 0)
		return ret;
	else if (res.a0 != MTK_EFUSE_SUCCESS) {
		pr_err("%s : read efuse fail (%lu)\n",
		       __func__, res.a0);
		return -1;
	}

	/* clean read buffer */
	memset(read_buffer, 0x0, read_buffer_len);

	/*
	 * get efuse data
	 * maximum data length in one time SMC is 8 bytes
	 */
	for (offset = 0; offset < efuse_len; offset += 8) {
		ret = mtk_efuse_smc(MTK_SIP_EFUSE_GET_DATA,
				    offset, 8, 0x0, 0x0,
				    &res);
		if (ret < 0)
			return ret;
		else if (res.a0 != MTK_EFUSE_SUCCESS) {
			pr_err("%s : get efuse data fail (%lu)\n",
			       __func__, res.a0);
			return -1;
		}
		efuse_data[0] = res.a2;
		efuse_data[1] = res.a3;

		for (idx = offset;
		     idx < (offset + 8) && idx < efuse_len;
		     idx++)
		{
			read_buffer[idx] = ((u8 *)efuse_data)[idx - offset];
		}
	}

	return 0;
}

// Stolen helper function to get a handle to the mtd device, used to
// look for the AUTOFUSE string.
struct mtd_info *board_get_mtd_device(void)
{
	struct mtd_info *mtd;

#ifdef CONFIG_ENABLE_NAND_NMBM
	mtd = nmbm_mtd_get_upper_by_index(0);

	if (mtd)
		mtd = get_mtd_device(mtd, -1);

	if (!mtd)
		cprintln(ERROR, "*** NMBM MTD device %u not found! ***", 0);
#else
	mtd = get_mtd_device(NULL, 0);

	if (!mtd)
		cprintln(ERROR, "*** NAND MTD device %u not found! ***", 0);
#endif

	return mtd;
}
