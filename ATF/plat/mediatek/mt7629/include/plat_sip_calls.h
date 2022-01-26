/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_SIP_CALLS_H
#define PLAT_SIP_CALLS_H

/*******************************************************************************
 * Plat SiP function constants
 ******************************************************************************/
#define MTK_PLAT_SIP_NUM_CALLS		8

#define MTK_SIP_PWR_ON_MTCMOS		0x82000402
#define MTK_SIP_PWR_OFF_MTCMOS		0x82000403
#define MTK_SIP_PWR_MTCMOS_SUPPORT	0x82000404

/*
 *  MTK_SIP_EFUSE_GET_LEN - get data length of efuse field
 *
 *  parameters
 *  @x1:	efuse field
 *
 *  return
 *  @r0:	status
 *  @r1:	data length
 */
#define MTK_SIP_EFUSE_GET_LEN		0x82000501

/*
 *  MTK_SIP_EFUSE_SEND_DATA - send data to efuse buffer
 *
 *  parameters
 *  @x1:	data offset, 0 ~ 24 bytes
 *  @x2:	data length, 0 ~ 8 bytes
 *  @x3:	data, bytes 0 to 3
 *  @x4:	data, bytes 4 to 7
 *
 *  return
 *  @r0:	status
 *  @r1:	data length
 */
#define MTK_SIP_EFUSE_SEND_DATA		0x82000502

/*
 *  MTK_SIP_EFUSE_GET_DATA - get data from efuse buffer
 *
 *  parameters
 *  @x1:	data offset, 0 ~ 24 bytes
 *  @x2:	data length, 0 ~ 8 bytes
 *
 *  return
 *  @r0:	status
 *  @r1:	data length
 *  @r2:	data, bytes 0 to 3
 *  @r3:	data, bytes 4 to 7
 */
#define MTK_SIP_EFUSE_GET_DATA		0x82000503

/*
 *  MTK_SIP_EFUSE_WRITE - write efuse field
 *
 *  parameters
 *  @x1:	efuse field
 *
 *  return
 *  @r0:	status
 */
#define MTK_SIP_EFUSE_WRITE		0x82000504

/*
 *  MTK_SIP_EFUSE_READ - read efuse field
 *
 *  parameters
 *  @x1:	efuse field
 *
 *  return
 *  @r0:	status
 */
#define MTK_SIP_EFUSE_READ		0x82000505

#endif /* PLAT_SIP_CALLS_H */
