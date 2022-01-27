// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 *
 * Generic image boot helper
 */

#include <errno.h>
#include <image.h>
#include <mmc.h>
#include <part.h>
#include <part_efi.h>
#include <linux/types.h>
#include <linux/mtd/mtd.h>
#include <linux/sizes.h>

#include "upgrade_helper.h"
#include "spacex_mtk_efuse_check.h"

static const char SX_AUTOFUSE[] = "AUTOFUSE";
static const size_t SX_AUTOFUSE_LEN = sizeof(SX_AUTOFUSE) - 1;
static const size_t SX_AUTOFUSE_OFFSET = 0x600000;

static int _boot(ulong data_load_addr)
{
	char cmd[64];
	int ret;

#ifdef CONFIG_FIT_SIGNATURE
	/* SpaceX: Use bootcount and MTK's efuse secure-boot enabled bit to determine
	 * which boot chain we are on.
	 *
	 * Bootcount is stored in I2C's transfer length register for easy access here.
	 * Even parity means we are FIP0, else we are FIP1. If we are FIP0 we need to
	 * boot OpenWRT 0, else OpenWRT 1.
	 * 
	 * The secure-boot enabled bit (MTK_EFUSE_FIELD_SBC_EN, the second bit in
	 * the register at 0x10206060) determines whether we use a secured or unsecured
	 * device tree. */
	int32_t boot_count = *(volatile int32_t*)I2C_TRANSFER_LEN;

	uint8_t secure;
	int efuse_ret;
	efuse_ret = mtk_efuse_read(MTK_EFUSE_FIELD_SBC_EN, (uint8_t *)&secure,
			     sizeof(secure));
	if (efuse_ret) {
		// If something went wrong, assume prod mode.
		secure = 1;
	}

    // Look for auto-fuse settings, to enable auto-fuse, the factory uses the
    // NAND image with the string AUTOFUSE at a known offset. If this string is
    // found, set prod mode automatically. When the kernel is booted in
    // prod-mode, it will fuse during early-init if unfused.
	struct mtd_info *mtdInfo;
	char data[SX_AUTOFUSE_LEN + 1];
	memset(data, 0, sizeof(data));
	mtdInfo = board_get_mtd_device();
	if (mtdInfo) {
		ret = mtd_read_generic(mtdInfo, SX_AUTOFUSE_OFFSET, data, SX_AUTOFUSE_LEN);
		if (ret) {
			// If mtd read failed, assume prod mode.
			printf("Error mtd_read_generic: %d\n", ret);
			secure = 1;
		}
	} else {
		// If get_mtd_device failed, assume prod mode.
		secure = 1;
	}

	// Check for autofuse flag in 8 bytes from NAND, if strings are equal, set
	// prod mode.
	if (strncmp(SX_AUTOFUSE, data, SX_AUTOFUSE_LEN) == 0 && boot_count < 4) {
		printf("Auto-fuse is set and boot_count is under 4, load prod config\n");
		secure = 1;
	}

	/* verify FIT image */
	/* SpaceX: Load config dev0, dev1, prod0, or prod1 depending on boot_count and secure.
	 * Each config points at a different device tree and loadable scripts. Each device tree
	 * names a different MTD parition "ActiveKernel". OpenWrt will unpack to find its Rootfs */
	snprintf(cmd, sizeof(cmd), "bootm start 0x%lx#config@%s%d", data_load_addr, secure ? "prod" : "dev", boot_count & 0x1);
	ret = run_command(cmd, 0);
	if (ret)
		return ret;

	/* Run u-boot script to get dm-verity information */
	/* SpaceX: This script sets the bootargs of the OpenWRT kernel, the args are used
	 * to verify the rootfs. Depending on which OpenWrt we are booting, the rootfs
	 * will be in a different MTD block, so we choose the script that has the correct MTD
	 * block specified.
	 * Scripts are generated at: openwrt/scripts/prepare-dm-verity-uboot-script.sh */
	snprintf(cmd, sizeof(cmd), "source 0x%lx:script@%d", data_load_addr, boot_count & 0x1);
	ret = run_command(cmd, 0);
	if (ret)
		return ret;

	/* SpaceX: Append the boot_count to the args, so the application code has easy access by
	 * parsing /proc/cmdline */
	snprintf(cmd, sizeof(cmd), "setenv bootargs \"$bootargs bootcount=%d\"", boot_count);
	ret = run_command(cmd, 0);
	if (ret)
		return ret;

	/* Boot for real */
	snprintf(cmd, sizeof(cmd), "bootm 0x%lx#config@%s%d", data_load_addr, secure ? "prod" : "dev", boot_count & 0x1);
	ret = run_command(cmd, 0);
	if (ret)
		return ret;

#else
	/* boot processes for normal boot */
	snprintf(cmd, sizeof(cmd), "bootm 0x%lx", data_load_addr);
	ret = run_command(cmd, 0);
	if (ret)
		return ret;
#endif
	return ret;
}

#ifdef CONFIG_MMC
static int _boot_from_mmc(u32 dev, struct mmc *mmc, u64 offset)
{
	ulong data_load_addr;
	u32 size;
	int ret;

	/* Set load address */
#if defined(CONFIG_SYS_LOAD_ADDR)
	data_load_addr = CONFIG_SYS_LOAD_ADDR;
#elif defined(CONFIG_LOADADDR)
	data_load_addr = CONFIG_LOADADDR;
#endif

	ret = _mmc_read(mmc, offset, (void *)data_load_addr, mmc->read_bl_len);
	if (ret)
		return ret;

	switch (genimg_get_format((const void *)data_load_addr)) {
#if defined(CONFIG_LEGACY_IMAGE_FORMAT)
	case IMAGE_FORMAT_LEGACY:
		size = image_get_image_size((const void *)data_load_addr);
		break;
#endif
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		size = fit_get_size((const void *)data_load_addr);
		break;
#endif
	default:
		printf("Error: no Image found in MMC device %u at 0x%llx\n",
		       dev, offset);
		return -EINVAL;
	}

	ret = _mmc_read(mmc, offset, (void *)data_load_addr, size);
	if (ret)
		return ret;

	return _boot(data_load_addr);
}

int boot_from_mmc_generic(u32 dev, int part, u64 offset)
{
	struct mmc *mmc;

	mmc = _mmc_get_dev(dev, part, false);
	if (!mmc)
		return -ENODEV;

	return _boot_from_mmc(dev, mmc, offset);
}

#ifdef CONFIG_PARTITIONS
int boot_from_mmc_partition(u32 dev, int part, const char *part_name)
{
	struct disk_partition dpart;
	struct mmc *mmc;
	int ret;

	mmc = _mmc_get_dev(dev, part, false);
	if (!mmc)
		return -ENODEV;

	ret = _mmc_find_part(mmc, part_name, &dpart);
	if (ret)
		return ret;

	return _boot_from_mmc(dev, mmc, (u64)dpart.start * dpart.blksz);
}
#endif  /* CONFIG_PARTITIONS */
#endif /* CONFIG_GENERIC_MMC */

#ifdef CONFIG_MTD
int boot_from_mtd(struct mtd_info *mtd, u64 offset)
{
	ulong data_load_addr;
	u32 size;
	int ret;

	/* Set load address */
#if defined(CONFIG_SYS_LOAD_ADDR)
	data_load_addr = CONFIG_SYS_LOAD_ADDR;
#elif defined(CONFIG_LOADADDR)
	data_load_addr = CONFIG_LOADADDR;
#endif

	ret = mtd_read_generic(mtd, offset, (void *)data_load_addr,
			       mtd->writesize);
	if (ret)
		return ret;

	switch (genimg_get_format((const void *)data_load_addr)) {
#if defined(CONFIG_LEGACY_IMAGE_FORMAT)
	case IMAGE_FORMAT_LEGACY:
		size = image_get_image_size((const void *)data_load_addr);
		break;
#endif
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		size = fit_get_size((const void *)data_load_addr);
		break;
#endif
	default:
		printf("Error: no Image found in %s at 0x%llx\n", mtd->name,
		       offset);
		return -EINVAL;
	}

	ret = mtd_read_generic(mtd, offset, (void *)data_load_addr, size);
	if (ret)
		return ret;

	return _boot(data_load_addr);
}
#endif

#ifdef CONFIG_DM_SPI_FLASH
int boot_from_snor(struct spi_flash *snor, u32 offset)
{
	ulong data_load_addr;
	u32 size;
	int ret;

	/* Set load address */
#if defined(CONFIG_SYS_LOAD_ADDR)
	data_load_addr = CONFIG_SYS_LOAD_ADDR;
#elif defined(CONFIG_LOADADDR)
	data_load_addr = CONFIG_LOADADDR;
#endif

	ret = snor_read_generic(snor, offset, (void *)data_load_addr,
				snor->page_size);
	if (ret)
		return ret;

	switch (genimg_get_format((const void *)data_load_addr)) {
#if defined(CONFIG_LEGACY_IMAGE_FORMAT)
	case IMAGE_FORMAT_LEGACY:
		size = image_get_image_size((const void *)data_load_addr);
		break;
#endif
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		size = fit_get_size((const void *)data_load_addr);
		break;
#endif
	default:
		printf("Error: no Image found in SPI-NOR at 0x%x\n", offset);
		return -EINVAL;
	}

	ret = snor_read_generic(snor, offset, (void *)data_load_addr, size);
	if (ret)
		return ret;

	return _boot(data_load_addr);
}
#endif
