/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include "dram_sub_func.h"
#include "rcar_def.h"

#if RCAR_SYSTEM_SUSPEND
/* Local defines */
#define DRAM_BACKUP_GPIO_USE		0
#include "iic_dvfs.h"
#if PMIC_ROHM_BD9571
#define	PMIC_SLAVE_ADDR			0x30U
#define	PMIC_BKUP_MODE_CNT		0x20U
#define	PMIC_QLLM_CNT			0x27U
#define	BIT_BKUP_CTRL_OUT		BIT(4)
#define	BIT_QLLM_DDR0_EN		BIT(0)
#define	BIT_QLLM_DDR1_EN		BIT(1)
#endif

#define GPIO_BKUP_REQB_SHIFT_SALVATOR	9U	/* GP1_9 (BKUP_REQB) */
#define GPIO_BKUP_TRG_SHIFT_SALVATOR	8U	/* GP1_8 (BKUP_TRG) */
#define GPIO_BKUP_REQB_SHIFT_EBISU	14U	/* GP6_14(BKUP_REQB) */
#define GPIO_BKUP_TRG_SHIFT_EBISU	13U	/* GP6_13(BKUP_TRG) */
#define GPIO_BKUP_REQB_SHIFT_CONDOR	1U	/* GP3_1 (BKUP_REQB) */
#define GPIO_BKUP_TRG_SHIFT_CONDOR	0U	/* GP3_0 (BKUP_TRG) */

#define DRAM_BKUP_TRG_LOOP_CNT		1000U
#endif

void rcar_dram_get_boot_status(uint32_t *status)
{
#if RCAR_SYSTEM_SUSPEND
	uint32_t reg_data;
	uint32_t product;
	uint32_t shift;
	uint32_t gpio;

	product = mmio_read_32(PRR) & PRR_PRODUCT_MASK;
	if (product == PRR_PRODUCT_V3H) {
		shift = GPIO_BKUP_TRG_SHIFT_CONDOR;
		gpio = GPIO_INDT3;
	} else if (product == PRR_PRODUCT_E3) {
		shift = GPIO_BKUP_TRG_SHIFT_EBISU;
		gpio = GPIO_INDT6;
	} else {
		shift = GPIO_BKUP_TRG_SHIFT_SALVATOR;
		gpio = GPIO_INDT1;
	}

	reg_data = mmio_read_32(gpio);
	if (reg_data & BIT(shift))
		*status = DRAM_BOOT_STATUS_WARM;
	else
		*status = DRAM_BOOT_STATUS_COLD;
#else	/* RCAR_SYSTEM_SUSPEND */
	*status = DRAM_BOOT_STATUS_COLD;
#endif	/* RCAR_SYSTEM_SUSPEND */
}

int32_t rcar_dram_update_boot_status(uint32_t status)
{
	int32_t ret = 0;
#if RCAR_SYSTEM_SUSPEND
	uint32_t reg_data;
#if PMIC_ROHM_BD9571
#if DRAM_BACKUP_GPIO_USE == 0
	uint8_t bkup_mode_cnt = 0U;
#else
	uint32_t reqb, outd;
#endif
	uint8_t qllm_cnt = 0U;
	int32_t i2c_dvfs_ret = -1;
#endif
	uint32_t loop_count;
	uint32_t product;
	uint32_t trg;
	uint32_t gpio;

	product = mmio_read_32(PRR) & PRR_PRODUCT_MASK;
	if (product == PRR_PRODUCT_V3H) {
#if DRAM_BACKUP_GPIO_USE == 1
		reqb = GPIO_BKUP_REQB_SHIFT_CONDOR;
		outd = GPIO_OUTDT3;
#endif
		trg = GPIO_BKUP_TRG_SHIFT_CONDOR;
		gpio = GPIO_INDT3;
	} else if (product == PRR_PRODUCT_E3) {
#if DRAM_BACKUP_GPIO_USE == 1
		reqb = GPIO_BKUP_REQB_SHIFT_EBISU;
		outd = GPIO_OUTDT6;
#endif
		trg = GPIO_BKUP_TRG_SHIFT_EBISU;
		gpio = GPIO_INDT6;
	} else {
#if DRAM_BACKUP_GPIO_USE == 1
		reqb = GPIO_BKUP_REQB_SHIFT_SALVATOR;
		outd = GPIO_OUTDT1;
#endif
		trg = GPIO_BKUP_TRG_SHIFT_SALVATOR;
		gpio = GPIO_INDT1;
	}

	if (status == DRAM_BOOT_STATUS_WARM) {
#if DRAM_BACKUP_GPIO_USE == 1
		mmio_setbits_32(outd, BIT(reqb));
#else
#if PMIC_ROHM_BD9571
		/* Set BKUP_CRTL_OUT=High (BKUP mode cnt register) */
		i2c_dvfs_ret = rcar_iic_dvfs_receive(PMIC_SLAVE_ADDR,
						     PMIC_BKUP_MODE_CNT,
						     &bkup_mode_cnt);
		if (i2c_dvfs_ret) {
			ERROR("BKUP mode cnt READ ERROR.\n");
			ret = DRAM_UPDATE_STATUS_ERR;
		} else {
			bkup_mode_cnt &= (uint8_t)~BIT_BKUP_CTRL_OUT;
			i2c_dvfs_ret = rcar_iic_dvfs_send(PMIC_SLAVE_ADDR,
							  PMIC_BKUP_MODE_CNT,
							  bkup_mode_cnt);
			if (i2c_dvfs_ret) {
				ERROR("BKUP mode cnt WRITE ERROR. value = %d\n",
				      bkup_mode_cnt);
				ret = DRAM_UPDATE_STATUS_ERR;
			}
		}
#endif /* PMIC_ROHM_BD9571 */
#endif /* DRAM_BACKUP_GPIO_USE == 1 */
		/* Wait BKUP_TRG=Low */
		loop_count = DRAM_BKUP_TRG_LOOP_CNT;
		while (loop_count > 0) {
			reg_data = mmio_read_32(gpio);
			if (!(reg_data & BIT(trg)))
				break;
			loop_count--;
		}

		if (!loop_count) {
			ERROR("\nWarm booting...\n"
			      " The potential of BKUP_TRG did not switch to Low.\n"
			      " If you expect the operation of cold boot,\n"
			      " check the board configuration (ex, Dip-SW) and/or the H/W failure.\n");
			ret = DRAM_UPDATE_STATUS_ERR;
		}
	}
#if PMIC_ROHM_BD9571
	if (!ret) {
		qllm_cnt = BIT_QLLM_DDR0_EN | BIT_QLLM_DDR1_EN;
		i2c_dvfs_ret = rcar_iic_dvfs_send(PMIC_SLAVE_ADDR,
						  PMIC_QLLM_CNT,
						  qllm_cnt);
		if (i2c_dvfs_ret) {
			ERROR("QLLM cnt WRITE ERROR. value = %d\n", qllm_cnt);
			ret = DRAM_UPDATE_STATUS_ERR;
		}
	}
#endif
#endif
	return ret;
}
