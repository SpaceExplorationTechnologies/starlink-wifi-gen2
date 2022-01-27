/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2019, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/

#ifndef __DBG_CTRL_H__
#define __DBG_CTRL_H__

#define DEFAULT_FW_LOG_DESTINATION "/media/sda1/fw_log.bin"

#define FW_BIN_LOG_MAGIC_NUM    0x44E98CAF

enum {
	FW_LOG_2_HOST_CTRL_OFF = 0,
	FW_LOG_2_HOST_CTRL_2_UART = 1,
	FW_LOG_2_HOST_CTRL_2_HOST = 2,
	FW_LOG_2_HOST_CTRL_2_EMI = 4,
	FW_LOG_2_HOST_CTRL_2_HOST_STORAGE = 8,
	FW_LOG_2_HOST_CTRL_2_HOST_ETHNET = 16,
};

#ifdef FW_LOG_DUMP
#define SUPPORTED_FW_LOG_TYPE	0x1F
#define FW_LOG_TYPE_COUNT		5
#else
#define SUPPORTED_FW_LOG_TYPE	0x03
#define FW_LOG_TYPE_COUNT		2
#endif /* FW_LOG_DUMP */

typedef struct _FW_LOG_CTRL {
	UCHAR wmcpu_log_type;
	CHAR fw_log_dest_dir[32];
} FW_LOG_CTRL;

INT set_fw_log_dest_dir(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

NTSTATUS fw_log_to_file(
	IN PRTMP_ADAPTER pAd,
	IN PCmdQElmt CMDQelmt);

VOID fw_log_to_ethernet(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *fw_log,
	IN UINT32 log_len);

#endif /* __DBG_CTRL_H__ */

