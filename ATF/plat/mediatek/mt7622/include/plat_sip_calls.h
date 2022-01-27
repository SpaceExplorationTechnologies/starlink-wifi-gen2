/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_SIP_CALLS_H
#define PLAT_SIP_CALLS_H

/*******************************************************************************
 * Plat SiP function constants
 ******************************************************************************/
#define MTK_PLAT_SIP_NUM_CALLS		11

#define MTK_SIP_PWR_ON_MTCMOS		0x82000402
#define MTK_SIP_PWR_OFF_MTCMOS		0x82000403
#define MTK_SIP_PWR_MTCMOS_SUPPORT	0x82000404

/* Efuse Function ID */
#define MTK_SIP_EFUSE_READ			0xC2000501
#define MTK_SIP_EFUSE_WRITE			0xC2000502
#define MTK_SIP_EFUSE_WRITE_SBC_PUBK0_HASH	0xC2000510
#define MTK_SIP_EFUSE_WRITE_SBC_PUBK1_HASH	0xC2000511
#define MTK_SIP_EFUSE_WRITE_SBC_PUBK2_HASH	0xC2000512
#define MTK_SIP_EFUSE_WRITE_SBC_PUBK3_HASH	0xC2000513

/* Anti-Rollback Function ID */
#define MTK_SIP_CHECK_FIT_AR_VER		0xC2000520
#define MTK_SIP_UPDATE_EFUSE_AR_VER             0xC2000521

#endif /* PLAT_SIP_CALLS_H */
