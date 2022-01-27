/* SPDX-License-Identifier:	GPL-2.0+ */
/*
 * Copyright (C) 2021 MediaTek Incorporation. All Rights Reserved.
 *
 * SpaceX: This file contains the portions of the mtk_efuse driver from
 * OpenWRT necessary to perform an SMC to read the secure-boot-enabled bit
 * from the eFuse registers.
 */

#ifndef _SPACEX_MTK_EFUSE_CHECK_H_
#define _SPACEX_MTK_EFUSE_CHECK_H_

#include <linux/arm-smccc.h>

#define MTK_EFUSE_SUCCESS                       0x00000000
#define MTK_EFUSE_ERROR_EFUSE_FIELD_DISABLED    0x00000003

#define MTK_SIP_EFUSE_GET_LEN   0x82000501
#define MTK_SIP_EFUSE_GET_DATA  0x82000503
#define MTK_SIP_EFUSE_READ      0x82000505

#define MTK_EFUSE_FIELD_SBC_EN  13

int mtk_efuse_smc(u32 smc_fid,
			 u32 x1,
			 u32 x2,
			 u32 x3,
			 u32 x4,
			 struct arm_smccc_res *res);

int mtk_efuse_read(u32 efuse_field,
			 u8 *read_buffer,
			 u32 read_buffer_len);

struct mtd_info *board_get_mtd_device(void);

#endif /* _SPACEX_MTK_EFUSE_CHECK_H_ */
