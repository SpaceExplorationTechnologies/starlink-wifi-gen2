/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author: Landen Chao <landen.chao@mediatek.com>
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/phy.h>

#define PHY_ID_MTK_GPHY		0x03a29441

MODULE_DESCRIPTION("MTK GePHY driver");
MODULE_AUTHOR("Landen Chao <landen.chao@mediatek.com>");
MODULE_LICENSE("GPL");

static int mt753x_gphy_config_init(struct phy_device *phydev)
{
	int val;

	/* Enable pause capability of internal phy to match mac capability */
	val = phy_read(phydev, MII_ADVERTISE);
	val |= (ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
	phy_write(phydev, MII_ADVERTISE, val);

	return 0;
}

static struct phy_driver mtk_gphy_driver[] = { {
	.phy_id		= PHY_ID_MTK_GPHY,
	.phy_id_mask	= 0x0fffff00,
	.name		= "mtk_gphy",
	.soft_reset	= genphy_no_soft_reset,
	.config_init	= mt753x_gphy_config_init,
	.features	= (PHY_GBIT_FEATURES | SUPPORTED_MII |
			   SUPPORTED_Pause | SUPPORTED_Asym_Pause),
	.config_aneg	= genphy_config_aneg,
	.aneg_done	= genphy_aneg_done,
	.read_status	= genphy_read_status,
	.suspend	= genphy_suspend,
	.resume		= genphy_resume,
	.driver		= { .owner = THIS_MODULE,},
} };

module_phy_driver(mtk_gphy_driver);

static struct mdio_device_id __maybe_unused mtk_gphy_tbl[] = {
	{ PHY_ID_MTK_GPHY, 0x0fffff00 },
	{ }
};

MODULE_DEVICE_TABLE(mdio, mtk_gphy_tbl);
