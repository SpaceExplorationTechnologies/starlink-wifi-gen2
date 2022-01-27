/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2021 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 *
 * Generic image boot helper
 */

#ifndef _BOOT_HELPER_H_
#define _BOOT_HELPER_H_

#include <linux/types.h>

extern int board_boot_default(void);
extern int board_boot_a_b(void);

#ifdef CONFIG_MMC
int boot_from_mmc_generic(u32 dev, int part, u64 offset);

#ifdef CONFIG_PARTITIONS
int boot_from_mmc_partition(u32 dev, int part, const char *part_name);
#endif  /* CONFIG_PARTITIONS */
#endif /* CONFIG_GENERIC_MMC */

#ifdef CONFIG_MTD
#include <linux/mtd/mtd.h>
int boot_from_mtd(struct mtd_info *mtd, u64 offset);
#endif

#ifdef CONFIG_DM_SPI_FLASH
#include <spi_flash.h>
int boot_from_snor(struct spi_flash *snor, u32 offset);
#endif

#endif /* _BOOT_HELPER_H_ */
