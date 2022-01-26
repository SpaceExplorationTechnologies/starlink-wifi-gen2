// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <command.h>
#include <linux/sizes.h>
#include "../common/upgrade_helper.h"
#include "../common/boot_helper.h"
#include "../common/autoboot_helper.h"
#include "../common/colored_print.h"

enum upgrade_part_type {
	UPGRADE_PART_BL,
	UPGRADE_PART_FW,
};

struct upgrade_part {
	u32 offset;
	size_t size;
};

static const struct upgrade_part upgrade_parts[] = {
	[UPGRADE_PART_BL] = {
		.offset = 0,
		.size = 0x60000,
	},
	[UPGRADE_PART_FW] = {
		.offset = 0xb0000,
		.size = 0xf20000,
	},
};

static inline struct spi_flash *board_get_snor_dev(void)
{
	return snor_get_dev(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS);
}

static int write_part(enum upgrade_part_type pt, const void *data, size_t size,
		      bool verify)
{
	const struct upgrade_part *part = &upgrade_parts[pt];
	struct spi_flash *snor;
	int ret;

	snor = board_get_snor_dev();
	if (!snor)
		return -ENODEV;

	ret = snor_erase_generic(snor, part->offset, size);
	if (ret)
		return ret;

	ret = snor_write_generic(snor, part->offset, part->size, data, size,
				 verify);

	return ret;
}

static int write_bl(void *priv, const struct data_part_entry *dpe,
		    const void *data, size_t size)
{
	return write_part(UPGRADE_PART_BL, data, size, true);
}

static int write_firmware(void *priv, const struct data_part_entry *dpe,
			  const void *data, size_t size)
{
	return write_part(UPGRADE_PART_FW, data, size, false);
}

static int write_flash_image(void *priv, const struct data_part_entry *dpe,
			     const void *data, size_t size)
{
	struct spi_flash *snor;
	int ret;

	snor = board_get_snor_dev();
	if (!snor)
		return -ENODEV;

	ret = snor_erase_generic(snor, 0, size);
	if (ret)
		return ret;

	return snor_write_generic(snor, 0, 0, data, size, false);
}

static int erase_env(void *priv, const struct data_part_entry *dpe,
		     const void *data, size_t size)
{
	struct spi_flash *snor;

	snor = board_get_snor_dev();
	if (!snor)
		return -ENODEV;

	return snor_erase_env(snor, CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE);
}

static const struct data_part_entry snor_parts[] = {
	{
		.name = "Bootloader",
		.abbr = "bl",
		.env_name = "bootfile.bl",
		.write = write_bl,
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
	*dpes = snor_parts;
	*count = ARRAY_SIZE(snor_parts);
}

int board_boot_default(void)
{
	const struct upgrade_part *part = &upgrade_parts[UPGRADE_PART_FW];
	char cmd[64];

	sprintf(cmd, "bootm 0x%lx", 0x30000000 + (ulong)part->offset);

	return run_command(cmd, 0);
}

static const struct bootmenu_entry snor_bootmenu_entries[] = {
	{
		.desc = "Startup system (Default)",
		.cmd = "mtkboardboot"
	},
	{
		.desc = "Upgrade firmware",
		.cmd = "mtkupgrade fw"
	},
	{
		.desc = "Upgrade bootloader",
		.cmd = "mtkupgrade bl"
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
	*menu = snor_bootmenu_entries;
	*count = ARRAY_SIZE(snor_bootmenu_entries);
}
