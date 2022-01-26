/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/utils_def.h>
#include <lib/mmio.h>
#include <mtcmos.h>
#include <mtk_sip_svc.h>
#include <plat_sip_calls.h>
#include <efuse_cmd.h>
#include <string.h>
#include <ar_table.h>

/* Authorized secure register list */
enum {
	SREG_HDMI_COLOR_EN = 0x14000904
};

static const uint32_t authorized_sreg[] = {
	SREG_HDMI_COLOR_EN
};

#define authorized_sreg_cnt	ARRAY_SIZE(authorized_sreg)

uint64_t mt_sip_set_authorized_sreg(uint32_t sreg, uint32_t val)
{
	uint64_t i;

	for (i = 0; i < authorized_sreg_cnt; i++) {
		if (authorized_sreg[i] == sreg) {
			mmio_write_32(sreg, val);
			return MTK_SIP_E_SUCCESS;
		}
	}

	return MTK_SIP_E_INVALID_PARAM;
}

static uint64_t mt_sip_pwr_on_mtcmos(uint32_t val)
{
	uint32_t ret;

	ret = mtcmos_non_cpu_ctrl(1, val);
	if (ret)
		return MTK_SIP_E_INVALID_PARAM;
	else
		return MTK_SIP_E_SUCCESS;
}

static uint64_t mt_sip_pwr_off_mtcmos(uint32_t val)
{
	uint32_t ret;

	ret = mtcmos_non_cpu_ctrl(0, val);
	if (ret)
		return MTK_SIP_E_INVALID_PARAM;
	else
		return MTK_SIP_E_SUCCESS;
}

static uint64_t mt_sip_pwr_mtcmos_support(void)
{
	return MTK_SIP_E_SUCCESS;
}

uint64_t mediatek_plat_sip_handler(uint32_t smc_fid,
				   uint64_t x1,
				   uint64_t x2,
				   uint64_t x3,
				   uint64_t x4,
				   void *cookie,
				   void *handle,
				   uint64_t flags)
{
	uint64_t ret;
	uint64_t read_buffer[4] = { 0 };
	uint64_t write_buffer[4] = { x1, x2, x3, x4 };
	uint32_t image_fit_ar_ver = (uint32_t)x1;
	uint32_t plat_fit_ar_ver = 0;

	switch (smc_fid) {
	case MTK_SIP_PWR_ON_MTCMOS:
		ret = mt_sip_pwr_on_mtcmos((uint32_t)x1);
		SMC_RET1(handle, ret);

	case MTK_SIP_PWR_OFF_MTCMOS:
		ret = mt_sip_pwr_off_mtcmos((uint32_t)x1);
		SMC_RET1(handle, ret);

	case MTK_SIP_PWR_MTCMOS_SUPPORT:
		ret = mt_sip_pwr_mtcmos_support();
		SMC_RET1(handle, ret);

	case MTK_SIP_EFUSE_READ:
		ret = efuse_read((uint32_t)x1,
				 (uint8_t *)read_buffer,
				 (uint32_t)x2);
		if (!ret) {
			SMC_RET4(handle, read_buffer[0], read_buffer[1],
					 read_buffer[2], read_buffer[3]);
		} else {
			SMC_RET1(handle, ret);
		}

	case MTK_SIP_EFUSE_WRITE:
		ret = efuse_write((uint32_t)x1,
				  (const uint8_t *)&x2,
				  (uint32_t)x3);
		SMC_RET1(handle, ret);

	case MTK_SIP_EFUSE_WRITE_SBC_PUBK0_HASH:
		ret = efuse_write((uint32_t)EFUSE_INDEX_SBC_PUBK0_HASH,
				  (const uint8_t *)write_buffer,
				  (uint32_t)EFUSE_LENGTH_HASH);
		SMC_RET1(handle, ret);

	case MTK_SIP_EFUSE_WRITE_SBC_PUBK1_HASH:
		ret = efuse_write((uint32_t)EFUSE_INDEX_SBC_PUBK1_HASH,
				  (const uint8_t *)write_buffer,
				  (uint32_t)EFUSE_LENGTH_HASH);
		SMC_RET1(handle, ret);

	case MTK_SIP_EFUSE_WRITE_SBC_PUBK2_HASH:
		ret = efuse_write((uint32_t)EFUSE_INDEX_SBC_PUBK2_HASH,
				  (const uint8_t *)write_buffer,
				  (uint32_t)EFUSE_LENGTH_HASH);
		SMC_RET1(handle, ret);

	case MTK_SIP_EFUSE_WRITE_SBC_PUBK3_HASH:
		ret = efuse_write((uint32_t)EFUSE_INDEX_SBC_PUBK3_HASH,
				  (const uint8_t *)write_buffer,
				  (uint32_t)EFUSE_LENGTH_HASH);
		SMC_RET1(handle, ret);

	case MTK_SIP_CHECK_FIT_AR_VER:
		ret = mtk_antirollback_get_fit_ar_ver(&plat_fit_ar_ver);
		if (!ret && image_fit_ar_ver >= plat_fit_ar_ver) {
			SMC_RET2(handle, 1, plat_fit_ar_ver);
		} else {
			SMC_RET2(handle, ret, plat_fit_ar_ver);
		}

	case MTK_SIP_UPDATE_EFUSE_AR_VER:
		ret = mtk_antirollback_update_efuse_ar_ver();
		SMC_RET1(handle, ret);

	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}
