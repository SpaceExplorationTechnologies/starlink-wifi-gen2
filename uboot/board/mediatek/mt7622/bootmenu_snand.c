// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <linux/mtd/mtd.h>
#include <linux/sizes.h>
#include "../common/upgrade_helper.h"
#include "../common/boot_helper.h"
#include "../common/autoboot_helper.h"
#include "../common/colored_print.h"

#include <nmbm/nmbm.h>
#include <nmbm/nmbm-mtd.h>

enum upgrade_part_type {
	UPGRADE_PART_BL2,
	UPGRADE_PART_FIP,
	UPGRADE_PART_FW,
};

struct upgrade_part {
	u64 offset;
	size_t size;
};

static const struct upgrade_part upgrade_parts[] = {
	[UPGRADE_PART_BL2] = {
		.offset = 0,
		.size = 0x80000,
	},
	[UPGRADE_PART_FIP] = {
		.offset = 0x80000,
		.size = 0x200000,
	},
	[UPGRADE_PART_FW] = {
		.offset = 0x400000,
		.size = 0x3000000,
	},
};

static struct mtd_info *board_get_mtd_device(void)
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

static int write_part(enum upgrade_part_type pt, const void *data, size_t size,
		      bool verify)
{
	const struct upgrade_part *part = &upgrade_parts[pt];
	struct mtd_info *mtd;
	int ret;

	mtd = board_get_mtd_device();
	if (!mtd)
		return -ENODEV;

	ret = mtd_erase_generic(mtd, part->offset, size);
	if (ret)
		return ret;

	ret = mtd_write_generic(mtd, part->offset, part->size, data, size,
				verify);

	put_mtd_device(mtd);

	return ret;
}

static int write_bl2(void *priv, const struct data_part_entry *dpe,
		     const void *data, size_t size)
{
	return write_part(UPGRADE_PART_BL2, data, size, true);
}

static int write_fip(void *priv, const struct data_part_entry *dpe,
		     const void *data, size_t size)
{
	return write_part(UPGRADE_PART_FIP, data, size, true);
}

static int write_firmware(void *priv, const struct data_part_entry *dpe,
			  const void *data, size_t size)
{
	return write_part(UPGRADE_PART_FW, data, size, true);
}

static int write_flash_image(void *priv, const struct data_part_entry *dpe,
			     const void *data, size_t size)
{
	struct mtd_info *mtd = board_get_mtd_device();
	int ret;

	if (!mtd)
		return -ENODEV;

	ret = mtd_erase_generic(mtd, 0, size);
	if (ret)
		return ret;

	ret = mtd_write_generic(mtd, 0, 0, data, size, true);

	put_mtd_device(mtd);

	return ret;
}

static int erase_env(void *priv, const struct data_part_entry *dpe,
		     const void *data, size_t size)
{
	struct mtd_info *mtd = board_get_mtd_device();
	int ret;

	if (!mtd)
		return -ENODEV;

	ret = mtd_erase_env(mtd, CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE);

	put_mtd_device(mtd);

	return ret;
}

static const struct data_part_entry snand_parts[] = {
	{
		.name = "ATF BL2",
		.abbr = "bl2",
		.env_name = "bootfile.bl2",
		.write = write_bl2,
	},
	{
		.name = "ATF FIP",
		.abbr = "fip",
		.env_name = "bootfile.fip",
		.write = write_fip,
		.post_action = UPGRADE_ACTION_CUSTOM,
		.do_post_action = erase_env,
	},
	{
		.name = "Firmware",
		.abbr = "fw",
		.env_name = "bootfile",
		.post_action = UPGRADE_ACTION_BOOT,
		.write = write_firmware,
	},
	{
		.name = "Single image",
		.abbr = "simg",
		.env_name = "bootfile.simg",
		.write = write_flash_image,
	},
};

void board_upgrade_data_parts(const struct data_part_entry **dpes, u32 *count)
{
	*dpes = snand_parts;
	*count = ARRAY_SIZE(snand_parts);
}

int board_boot_default(void)
{
	const struct upgrade_part *part = &upgrade_parts[UPGRADE_PART_FW];
	struct mtd_info *mtd = board_get_mtd_device();
	int ret;

	if (!mtd)
		return -ENODEV;

	ret = boot_from_mtd(mtd, part->offset);

	put_mtd_device(mtd);

	return ret;
}

static const struct bootmenu_entry snand_bootmenu_entries[] = {
	{
		.desc = "Startup system (Default)",
		.cmd = "mtkboardboot"
	},
	{
		.desc = "Upgrade firmware",
		.cmd = "mtkupgrade fw"
	},
	{
		.desc = "Upgrade ATF BL2",
		.cmd = "mtkupgrade bl2"
	},
	{
		.desc = "Upgrade ATF FIP",
		.cmd = "mtkupgrade fip"
	},
	{
		.desc = "Upgrade single image",
		.cmd = "mtkupgrade simg"
	},
	{
		.desc = "Load image",
		.cmd = "mtkload"
	}
};

void board_bootmenu_entries(const struct bootmenu_entry **menu, u32 *count)
{
	*menu = snand_bootmenu_entries;
	*count = ARRAY_SIZE(snand_bootmenu_entries);
}
