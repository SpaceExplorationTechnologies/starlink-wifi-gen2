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
	UPGRADE_PART_FIP0,
	UPGRADE_PART_FIP1,
	UPGRADE_PART_FW0,
	UPGRADE_PART_FW1,
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
	[UPGRADE_PART_FIP0] = {
		.offset = 0x80000,
		.size = 0x200000,
	},
	[UPGRADE_PART_FIP1] = {
		.offset = 0x280000,
		.size = 0x200000,
	},
	[UPGRADE_PART_FW0] = {
		.offset = 0x02000000,
		.size = 0x02000000,
	},
	[UPGRADE_PART_FW1] = {
		.offset = 0x04000000,
		.size = 0x02000000,
	},
};

/*
 * layout: 512k(bl2),2048k(fip),512k(config),1024k(factory),-(firmware)
 */

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

static int write_fip0(void *priv, const struct data_part_entry *dpe,
		     const void *data, size_t size)
{
	return write_part(UPGRADE_PART_FIP0, data, size, true);
}

static int write_fip1(void *priv, const struct data_part_entry *dpe,
		     const void *data, size_t size)
{
	return write_part(UPGRADE_PART_FIP1, data, size, true);
}

static int write_firmware0(void *priv, const struct data_part_entry *dpe,
			  const void *data, size_t size)
{
	return write_part(UPGRADE_PART_FW0, data, size, true);
}

static int write_firmware1(void *priv, const struct data_part_entry *dpe,
			  const void *data, size_t size)
{
	return write_part(UPGRADE_PART_FW1, data, size, true);
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
		.name = "ATF FIP 0",
		.abbr = "fip0",
		.env_name = "bootfile.fip",
		.write = write_fip0,
		.post_action = UPGRADE_ACTION_CUSTOM,
		.do_post_action = erase_env,
	},
	{
		.name = "ATF FIP 1",
		.abbr = "fip1",
		.env_name = "bootfile.fip",
		.write = write_fip1,
		.post_action = UPGRADE_ACTION_CUSTOM,
		.do_post_action = erase_env,
	},
	{
		.name = "OpenwWrt Firmware 0",
		.abbr = "fw0",
		.env_name = "bootfile",
		.post_action = UPGRADE_ACTION_BOOT,
		.write = write_firmware0,
	},
	{
		.name = "OpenWrt Firmware 1",
		.abbr = "fw1",
		.env_name = "bootfile",
		.post_action = UPGRADE_ACTION_BOOT,
		.write = write_firmware1,
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
	const struct upgrade_part *part = &upgrade_parts[UPGRADE_PART_FW0];
	struct mtd_info *mtd = board_get_mtd_device();
	int ret;

	if (!mtd)
		return -ENODEV;

	ret = boot_from_mtd(mtd, part->offset);

	put_mtd_device(mtd);

	return ret;
}

int board_boot_a_b(void)
{
	// Retreive boot_count from I2C Transfer Length register.
	int32_t boot_count = *(volatile int32_t*)I2C_TRANSFER_LEN;
	cprintln(NORMAL, "Got bootcount: %d!, loading OpenWrt %d", boot_count, boot_count & 0x1);

	const struct upgrade_part *part;
	if (boot_count & 0x1) {
		part = &upgrade_parts[UPGRADE_PART_FW1];
	} else {
		part = &upgrade_parts[UPGRADE_PART_FW0];
	}

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
		.desc = "Upgrade OpenWrt firmware 0",
		.cmd = "mtkupgrade fw0"
	},
	{
		.desc = "Upgrade OpenWrt firmware 1",
		.cmd = "mtkupgrade fw1"
	},
	{
		.desc = "Upgrade ATF BL2",
		.cmd = "mtkupgrade bl2"
	},
	{
		.desc = "Upgrade ATF FIP 0",
		.cmd = "mtkupgrade fip0"
	},
	{
		.desc = "Upgrade ATF FIP 1",
		.cmd = "mtkupgrade fip1"
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
