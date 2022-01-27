/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2021
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AIROHA SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AIROHA SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AIROHA'S OPTION, TO REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*******************************************************************************/

/* FILE NAME:  airoha.c
 * PURPOSE:
 *      EN8801S phy driver for Linux
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/phy.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <linux/uaccess.h>

#include "airoha.h"

MODULE_DESCRIPTION("Airoha EN8801S PHY drivers");
MODULE_AUTHOR("Airoha");
MODULE_LICENSE("GPL");

static int preSpeed=0;
/************************************************************************
*                  F U N C T I O N S
************************************************************************/
unsigned int mdiobus_write45(struct mii_bus *bus, u32 port, u32 devad, u32 reg, u16 val)
{
    mdiobus_write(bus, port, MII_MMD_ACC_CTL_REG, devad);
    mdiobus_write(bus, port, MII_MMD_ADDR_DATA_REG, reg);
    mdiobus_write(bus, port, MII_MMD_ACC_CTL_REG, MMD_OP_MODE_DATA | devad);
    mdiobus_write(bus, port, MII_MMD_ADDR_DATA_REG, val);
    return 0;
}

unsigned int mdiobus_read45(struct mii_bus *bus, u32 port, u32 devad, u32 reg, u32 *read_data)
{
    mdiobus_write(bus, port, MII_MMD_ACC_CTL_REG, devad);
    mdiobus_write(bus, port, MII_MMD_ADDR_DATA_REG, reg);
    mdiobus_write(bus, port, MII_MMD_ACC_CTL_REG, MMD_OP_MODE_DATA | devad);
    *read_data = mdiobus_read(bus, port, MII_MMD_ADDR_DATA_REG);
    return 0;
}

/* Airoha MII read function */
unsigned int ecnt_mii_cl22_read(struct mii_bus *ebus, unsigned int phy_addr,unsigned int phy_register,unsigned int *read_data)
{
    *read_data = mdiobus_read(ebus, phy_addr, phy_register);
    return 0;
}

/* Airoha MII write function */
unsigned int ecnt_mii_cl22_write(struct mii_bus *ebus, unsigned int phy_addr, unsigned int phy_register,unsigned int write_data)
{
    mdiobus_write(ebus, phy_addr, phy_register, write_data);
    return 0;
}

/* EN8801 PBUS write function */
void En8801_PbusRegWr(struct mii_bus *ebus, unsigned long pbus_address, unsigned long pbus_data)
{
    ecnt_mii_cl22_write(ebus, EN8801S_PBUS_PHY_ID, 0x1F, (unsigned int)(pbus_address >> 6));
    ecnt_mii_cl22_write(ebus, EN8801S_PBUS_PHY_ID, (unsigned int)((pbus_address >> 2) & 0xf), (unsigned int)(pbus_data & 0xFFFF));
    ecnt_mii_cl22_write(ebus, EN8801S_PBUS_PHY_ID, 0x10, (unsigned int)(pbus_data >> 16));
    return;
}

/* EN8801 PBUS read function */
unsigned long En8801_PbusRegRd(struct mii_bus *ebus, unsigned long pbus_address)
{
    unsigned long pbus_data;
    unsigned int pbus_data_low, pbus_data_high;

    ecnt_mii_cl22_write(ebus, EN8801S_PBUS_PHY_ID, 0x1F, (unsigned int)(pbus_address >> 6));
    ecnt_mii_cl22_read(ebus, EN8801S_PBUS_PHY_ID, (unsigned int)((pbus_address >> 2) & 0xf), &pbus_data_low);
    ecnt_mii_cl22_read(ebus, EN8801S_PBUS_PHY_ID, 0x10, &pbus_data_high);
    pbus_data = (pbus_data_high << 16) + pbus_data_low;
    return pbus_data;
}

/* Use default PBUS_PHY_ID */
/* EN8801 PBUS write function */
void En8801_varPbusRegWr(struct mii_bus *ebus, unsigned long pbus_id,unsigned long pbus_address, unsigned long pbus_data)
{
    ecnt_mii_cl22_write(ebus, pbus_id, 0x1F, (unsigned int)(pbus_address >> 6));
    ecnt_mii_cl22_write(ebus, pbus_id, (unsigned int)((pbus_address >> 2) & 0xf), (unsigned int)(pbus_data & 0xFFFF));
    ecnt_mii_cl22_write(ebus, pbus_id, 0x10, (unsigned int)(pbus_data >> 16));
    return;
}

/* EN8801 PBUS read function */
unsigned long En8801_varPbusRegRd(struct mii_bus *ebus, unsigned long pbus_id, unsigned long pbus_address)
{
    unsigned long pbus_data;
    unsigned int pbus_data_low, pbus_data_high;

    ecnt_mii_cl22_write(ebus, pbus_id, 0x1F, (unsigned int)(pbus_address >> 6));
    ecnt_mii_cl22_read(ebus,  pbus_id, (unsigned int)((pbus_address >> 2) & 0xf), &pbus_data_low);
    ecnt_mii_cl22_read(ebus,  pbus_id, 0x10, &pbus_data_high);
    pbus_data = (pbus_data_high << 16) + pbus_data_low;
    return pbus_data;
}

/* EN8801 Token Ring Write function */
void En8801_TR_RegWr(struct mii_bus *ebus, unsigned long tr_address, unsigned long tr_data)
{
    ecnt_mii_cl22_write(ebus, EN8801S_MDIO_PHY_ID, 0x1F, 0x52b5);       /* page select */
    ecnt_mii_cl22_write(ebus, EN8801S_MDIO_PHY_ID, 0x11, (unsigned int)(tr_data & 0xffff));
    ecnt_mii_cl22_write(ebus, EN8801S_MDIO_PHY_ID, 0x12, (unsigned int)(tr_data >> 16));
    ecnt_mii_cl22_write(ebus, EN8801S_MDIO_PHY_ID, 0x10, (unsigned int)(tr_address | TrReg_WR));
    ecnt_mii_cl22_write(ebus, EN8801S_MDIO_PHY_ID, 0x1F, 0x0);          /* page resetore */
    return;
}

/* EN8801 Token Ring Read function */
unsigned long En8801_TR_RegRd(struct mii_bus *ebus, unsigned long tr_address)
{
    unsigned long tr_data;
    unsigned int tr_data_low, tr_data_high;

    ecnt_mii_cl22_write(ebus, EN8801S_MDIO_PHY_ID, 0x1F, 0x52b5);       /* page select */
    ecnt_mii_cl22_write(ebus, EN8801S_MDIO_PHY_ID, 0x10, (unsigned int)(tr_address | TrReg_RD));
    ecnt_mii_cl22_read(ebus, EN8801S_MDIO_PHY_ID, 0x11, &tr_data_low);
    ecnt_mii_cl22_read(ebus, EN8801S_MDIO_PHY_ID, 0x12, &tr_data_high);
    ecnt_mii_cl22_write(ebus, EN8801S_MDIO_PHY_ID, 0x1F, 0x0);          /* page resetore */
    tr_data = (tr_data_high << 16) + tr_data_low;
    return tr_data;
}

static int en8801s_config_init(struct phy_device *phydev)
{
    gephy_all_REG_LpiReg1Ch      GPHY_RG_LPI_1C;
    gephy_all_REG_dev1Eh_reg324h GPHY_RG_1E_324;
    gephy_all_REG_dev1Eh_reg012h GPHY_RG_1E_012;
    gephy_all_REG_dev1Eh_reg017h GPHY_RG_1E_017;
    u32 reg_value;
    u32 retry;

    reg_value = (En8801_PbusRegRd(phydev->bus, 0xcf8) & 0xfffffffc) | 0x02;
    En8801_PbusRegWr(phydev->bus, 0xcf8, reg_value);
    /* SGMII AN */
    writel(0x10      ,ioremap(0x1b1280e8,4));
    #if defined (MT7622_BOARD)
    writel(0x14813   ,ioremap(0x1b12a028,4));        /*  MT7622 */
    #else
    writel(0x14813   ,ioremap(0x1b128128,4));        /*  MT7629 */
    #endif
    writel(0x31120103,ioremap(0x1b128020,4));
    writel(0x01      ,ioremap(0x1b128008,4));
    writel(0x1340    ,ioremap(0x1b128000,4));
    writel(0x0       ,ioremap(0x1b1280e8,4));
    mdelay(10);

    En8801_PbusRegWr(phydev->bus, 0x10,0xD801);     /* control word */
    En8801_PbusRegWr(phydev->bus, 0x0, 0x0140);
    En8801_PbusRegWr(phydev->bus, 0x0, 0x9140);

    retry = MAX_RETRY;
    while(retry != 0)
    {
        mdelay(10);
        reg_value = phy_read(phydev, MII_PHYSID1);
        if(reg_value == EN8801S_PHY_ID1)
        {
            break;    /* wait GPHY ready */
        }
        retry--;
        if(retry == 0)
        {
            printk(KERN_INFO "[Airoha] EN8801S initialize fail !\n");
            return 0;
        }
    }
    /* Software Reset PHY */
    reg_value = phy_read(phydev, MII_BMCR);
    reg_value |= BMCR_RESET;
    phy_write(phydev, MII_BMCR, reg_value);
    retry = MAX_RETRY;
    do {
        mdelay(10);
        reg_value = phy_read(phydev, MII_BMCR);
        retry--;
        if(retry == 0)
        {
            printk(KERN_INFO "[Airoha] EN8801S initialize fail !\n");
            return 0;
        }
    } while (reg_value & BMCR_RESET);

    En8801_PbusRegWr(phydev->bus, 0x0600, 0x0c000c00);
    En8801_PbusRegWr(phydev->bus, 0x10, 0xD801);
    En8801_PbusRegWr(phydev->bus, 0x0,  0x9140);
    En8801_PbusRegWr(phydev->bus, 0x0A14, 0x0003);
    En8801_PbusRegWr(phydev->bus, 0x0600, 0x0c000c00);
    /* Set FCM control */
    En8801_PbusRegWr(phydev->bus, 0x1404, 0x004b);
    En8801_PbusRegWr(phydev->bus, 0x140c, 0x0007);
    /* Set GPHY Perfomance*/
    /* Token Ring */
    En8801_TR_RegWr(phydev->bus, RgAddr_PMA_01h,     0x6FB90A);
    En8801_TR_RegWr(phydev->bus, RgAddr_PMA_18h,     0x0E2F00);
    En8801_TR_RegWr(phydev->bus, RgAddr_DSPF_06h,    0x2EBAEF);
    En8801_TR_RegWr(phydev->bus, RgAddr_DSPF_11h,    0x040001);
    En8801_TR_RegWr(phydev->bus, RgAddr_DSPF_03h,    0x000004);
    En8801_TR_RegWr(phydev->bus, RgAddr_DSPF_1Ch,    0x003210);
    En8801_TR_RegWr(phydev->bus, RgAddr_DSPF_14h,    0x00024A);
    En8801_TR_RegWr(phydev->bus, RgAddr_DSPF_0Ch,    0x00704D);
    En8801_TR_RegWr(phydev->bus, RgAddr_DSPF_0Dh,    0x02314F);
    En8801_TR_RegWr(phydev->bus, RgAddr_DSPF_10h,    0x005010);
    En8801_TR_RegWr(phydev->bus, RgAddr_DSPF_0Fh,    0x003028);
    En8801_TR_RegWr(phydev->bus, RgAddr_TR_26h,      0x444444);
    En8801_TR_RegWr(phydev->bus, RgAddr_R1000DEC_15h,0x0055A0);
    /* CL22 & CL45 */
    phy_write(phydev, 0x1f, 0x03);
    GPHY_RG_LPI_1C.DATA = phy_read(phydev, RgAddr_LpiReg1Ch);
    GPHY_RG_LPI_1C.DataBitField.smi_deton_th = 0x0C;
    phy_write(phydev, RgAddr_LpiReg1Ch, GPHY_RG_LPI_1C.DATA);
    phy_write(phydev, 0x1f, 0x0);
    mdiobus_write45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1E, 0x122, 0xffff);
    mdiobus_write45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1E, 0x234, 0x0180);
    mdiobus_write45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1E, 0x238, 0x0120);
    mdiobus_write45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1E, 0x120, 0x9014);
    mdiobus_write45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1E, 0x239, 0x0117);
    mdiobus_write45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1E, 0x14A, 0xEE20);
    mdiobus_write45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1E, 0x19B, 0x0111);
    mdiobus_write45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1F, 0x268, 0x07F4);

    mdiobus_read45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1E, 0x324, &reg_value);
    GPHY_RG_1E_324.DATA=(u16)reg_value;
    GPHY_RG_1E_324.DataBitField.smi_det_deglitch_off = 0;
    mdiobus_write45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1E, 0x324, (u32)GPHY_RG_1E_324.DATA);
    mdiobus_write45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1E, 0x19E, 0xC2);
    mdiobus_write45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1E, 0x013, 0x0);

    /* EFUSE */
    En8801_PbusRegWr(phydev->bus, 0x1C08, 0x40000040);
    retry = MAX_RETRY;
    while(retry != 0)
    {
        mdelay(1);
        reg_value = En8801_PbusRegRd(phydev->bus, 0x1C08);
        if((reg_value & (1 << 30)) == 0)
        {
            break;
        }
        retry--;
    }
    reg_value = En8801_PbusRegRd(phydev->bus, 0x1C38);          /* RAW#2 */
    GPHY_RG_1E_012.DataBitField.da_tx_i2mpb_a_tbt = reg_value & 0x03f;
    mdiobus_write45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1E, 0x12, (u32)GPHY_RG_1E_012.DATA);
    GPHY_RG_1E_017.DataBitField.da_tx_i2mpb_b_tbt=(reg_value >> 8) & 0x03f;
    mdiobus_write45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1E, 0x12, (u32)GPHY_RG_1E_017.DATA);

    En8801_PbusRegWr(phydev->bus, 0x1C08, 0x40400040);
    retry = MAX_RETRY;
    while(retry != 0)
    {
        mdelay(1);
        reg_value = En8801_PbusRegRd(phydev->bus, 0x1C08);
        if((reg_value & (1 << 30)) == 0)
        {
            break;
        }
        retry--;
    }
    reg_value = En8801_PbusRegRd(phydev->bus, 0x1C30);          /* RAW#16 */
    GPHY_RG_1E_324.DataBitField.smi_det_deglitch_off = (reg_value >> 12) & 0x01;
    mdiobus_write45(phydev->bus, EN8801S_MDIO_PHY_ID, 0x1E, 0x324, (u32)GPHY_RG_1E_324.DATA);

    printk(KERN_INFO "[Airoha] EN8801S initialize OK ! (0722.2)\n");
    return 0;
}

static int en8801s_read_status(struct phy_device *phydev)
{
    int ret;
    u32 reg_value;

    ret = genphy_read_status(phydev);

    if(phydev->link == 0) preSpeed =0;
    if(preSpeed != phydev->speed && phydev->link == 1)
    {

        preSpeed = phydev->speed;
        En8801_PbusRegWr(phydev->bus, 0x0600, 0x0c000c00);
        if(preSpeed == SPEED_1000)
        {
            En8801_PbusRegWr(phydev->bus, 0x10, 0xD801);
            En8801_PbusRegWr(phydev->bus, 0x0,  0x9140);

            En8801_PbusRegWr(phydev->bus, 0x0A14, 0x0003);
            En8801_PbusRegWr(phydev->bus, 0x0600, 0x0c000c00);
            mdelay(2);      /* delay 2 ms */
            En8801_PbusRegWr(phydev->bus, 0x1404, 0x004b);
            En8801_PbusRegWr(phydev->bus, 0x140c, 0x0007);
        }
        else if(preSpeed == SPEED_100)
        {
            En8801_PbusRegWr(phydev->bus, 0x10, 0xD401);
            En8801_PbusRegWr(phydev->bus, 0x0,  0x9140);

            En8801_PbusRegWr(phydev->bus, 0x0A14, 0x0007);
            En8801_PbusRegWr(phydev->bus, 0x0600, 0x0c11);
            mdelay(2);      /* delay 2 ms */
            En8801_PbusRegWr(phydev->bus, 0x1404, 0x0027);
            En8801_PbusRegWr(phydev->bus, 0x140c, 0x0007);
        }
        else if(preSpeed == SPEED_10)
        {
            En8801_PbusRegWr(phydev->bus, 0x10, 0xD001);
            En8801_PbusRegWr(phydev->bus, 0x0,  0x9140);

            En8801_PbusRegWr(phydev->bus, 0x0A14, 0x000b);
            En8801_PbusRegWr(phydev->bus, 0x0600, 0x0c11);
            mdelay(2);      /* delay 2 ms */
            En8801_PbusRegWr(phydev->bus, 0x1404, 0x0027);
            En8801_PbusRegWr(phydev->bus, 0x140c, 0x0007);
        }
    }
    return ret;
}


static struct phy_driver Airoha_driver[] = {
{
    .phy_id         = 0x03a29416,
    .name           = "Airoha EN8801S",
    .phy_id_mask    = 0x0ffffff0,
    .features       = PHY_GBIT_FEATURES,
    .config_init    = &en8801s_config_init,
    .config_aneg    = &genphy_config_aneg,
    .read_status    = &en8801s_read_status,
    .suspend        = genphy_suspend,
    .resume         = genphy_resume,
} };

module_phy_driver(Airoha_driver);

static struct mdio_device_id __maybe_unused Airoha_tbl[] = {
    { 0x03a29416, 0x0ffffff0 },
    { }
};

MODULE_DEVICE_TABLE(mdio, Airoha_tbl);
