/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h> 
#include <linux/reset.h>

#define INFRA_PHY 0x710
#define GE_FE_PHY_EN 0x10000820
#define GE_FE_PHY_DIS(x) (((x) & 0x1f) << 5)
#define MTK_SMI_ADDR_MASD 0x1f
#define MTK_INTERNAL_GPHY_ADDR 0

#define MTK_ESW_BASE 0x18000
#define MTK_ESW_LED_CTRL 0x168
#define  MTK_ESW_EPHY_MDIO_ADDR(x) (((x) & 0x1f) << 16)
#define  MTK_ESW_EPHY_MDIO_MASK MTK_ESW_EPHY_MDIO_ADDR(~0)

struct mtk_gphy {
        struct device		*dev;
        struct mii_bus		*bus;
        struct regmap		*infra;
        struct regmap		*eth;
	struct reset_control	*reset;
	unsigned int		smi_base;
};

static struct mtk_gphy *_gphy;

static unsigned int mii_mgr_read(unsigned int phy_addr,unsigned int phy_register,unsigned int *read_data)
{
        struct mii_bus *bus = _gphy->bus;

	/* default smi_base is 0, and ephy addresses are 1,2,3,4. */
	if (phy_addr < PHY_MAX_ADDR && phy_addr >= 0) {
		/* skip gphy address if set. */
		if (phy_addr != MTK_INTERNAL_GPHY_ADDR)
			phy_addr = (_gphy->smi_base + phy_addr) &
				   MTK_SMI_ADDR_MASD;
	} else {
		return 0;
	}

        *read_data = mdiobus_read(bus, phy_addr, phy_register);

        return 0;
}

static unsigned int mii_mgr_write(unsigned int phy_addr,unsigned int phy_register,unsigned int write_data)
{
        struct mii_bus *bus =  _gphy->bus;

	/* default smi_base is 0, and ephy addresses are 1,2,3,4. */
	if (phy_addr < PHY_MAX_ADDR && phy_addr >= 0) {
		/* skip gphy address if set. */
		if (phy_addr != MTK_INTERNAL_GPHY_ADDR)
			phy_addr = (_gphy->smi_base + phy_addr) &
				   MTK_SMI_ADDR_MASD;
	} else {
		return 0;
	}

        mdiobus_write(bus, phy_addr, phy_register, write_data);

        return 0;
}

#include "mtk_gphy_cal.c"

static int leopard_set_ephy_base(unsigned int base)
{
	unsigned int val;

	/* HW default, skip setup flow */
	if (base == 0)
		return -1;

	if (IS_ERR(_gphy->eth)) {
		dev_info(_gphy->dev, "gphy no eth support\n");
		return -1;
	}

	if (IS_ERR(_gphy->reset)) {
		dev_info(_gphy->dev, "Couldn't get our reset line\n");
		return -1;
	}

	reset_control_deassert(_gphy->reset);
	regmap_read(_gphy->eth, MTK_ESW_BASE + MTK_ESW_LED_CTRL, &val);
	val &= ~MTK_ESW_EPHY_MDIO_MASK;
	val |= MTK_ESW_EPHY_MDIO_ADDR(base);
	regmap_write(_gphy->eth, MTK_ESW_BASE + MTK_ESW_LED_CTRL, val);

	reset_control_assert(_gphy->reset);
	usleep_range(1000, 1100);
	reset_control_deassert(_gphy->reset);

	return 0;
}

static const struct of_device_id mtk_gphy_match[] = {
        { .compatible = "mediatek,eth-fe-gphy" },
        {},
};

MODULE_DEVICE_TABLE(of, mtk_gphy_match);

static int gphy_probe(struct platform_device *pdev)
{
        struct device_node *np = pdev->dev.of_node;
        struct device_node *mdio;
        struct mii_bus *mdio_bus;
        struct mtk_gphy *gphy;
	const __be32 *_id;
	bool fe_cal;
	int ret;

        mdio = of_parse_phandle(np, "mediatek,mdio", 0);

        if (!mdio)
                return -EINVAL;

        mdio_bus = of_mdio_find_bus(mdio);


        if (!mdio_bus)
                return -EPROBE_DEFER;

        gphy = devm_kzalloc(&pdev->dev, sizeof(struct mtk_gphy), GFP_KERNEL);

        if (!gphy)
                return -ENOMEM;


        gphy->dev = &pdev->dev;

        gphy->bus = mdio_bus;

	gphy->infra = syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
                                                           "mediatek,infracfg");
	gphy->eth = syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
                                                           "mediatek,eth");
	gphy->reset = devm_reset_control_get(&pdev->dev, "mtk_gephy");

	_id = of_get_property(np, "reg", NULL);
	if (_id)
		gphy->smi_base = be32_to_cpup(_id);

	_gphy = gphy;

	ret = leopard_set_ephy_base(gphy->smi_base);
	if (ret)
		gphy->smi_base = 0;

	fe_cal = of_property_read_bool(pdev->dev.of_node, "mediatek,fe-cal");

	if (IS_ERR(gphy->infra))
		dev_info(&pdev->dev, "gphy no infra support\n");
        else {
		/* enable fe/gphy for calibration */
		if (fe_cal)
			regmap_write(gphy->infra, INFRA_PHY, GE_FE_PHY_EN);
		else
			regmap_write(gphy->infra, INFRA_PHY,
				     GE_FE_PHY_EN | GE_FE_PHY_DIS(0x1d));
        }

	dev_info(&pdev->dev, "esw smi base is 0x%x, esw calibration %s\n",
		 _gphy->smi_base, fe_cal ? "enabled" : "disabled");

	leopard_ephy_cal(fe_cal);

        platform_set_drvdata(pdev, gphy);

        return 0;
}

static int gphy_remove(struct platform_device *pdev)
{
        platform_set_drvdata(pdev, NULL);      

        return 0;
}

static struct platform_driver fe_gphy_driver = {
        .probe = gphy_probe,
        .remove = gphy_remove,
        .driver = {
                .name = "mtk-fe-gphy",
                .owner = THIS_MODULE,
                .of_match_table = mtk_gphy_match,
        },
};

module_platform_driver(fe_gphy_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mark Lee <marklee0201@gmail.com>");
MODULE_DESCRIPTION("mtk internal gphy driver");

