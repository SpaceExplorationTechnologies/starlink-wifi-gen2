/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt7626.c
*/

#include "rt_config.h"
#include "chip/mt7626_cr.h"
#include "mcu/mt7626_firmware.h"
#ifdef NEED_ROM_PATCH
#include "mcu/mt7626_rom_patch_e1.h"
#endif /* NEED_ROM_PATCH */
#include "eeprom/mt7626_e2p.h"
#ifdef CONFIG_OF
#include <linux/of_platform.h>
#endif /* CONFIG_OF */
#ifdef WIFI_RAM_EMI_SUPPORT
#include "mcu/mt7626_emi_bin2h.h"
#endif /* WIFI_RAM_EMI_SUPPORT */
#include "bgnd_scan_cmm.h"

#ifdef CONFIG_AP_SUPPORT
#define DEFAULT_BIN_FILE "/etc_ro/wlan/MT7626_EEPROM1.bin"
#else
#define DEFAULT_BIN_FILE "/etc/MT7626_EEPROM1.bin"
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_RT_SECOND_CARD
#define SECOND_BIN_FILE "/etc_ro/wlan/MT7626_EEPROM2.bin"
#endif /* CONFIG_RT_SECOND_CARD */
#ifdef CONFIG_RT_THIRD_CARD
#define THIRD_BIN_FILE "/etc_ro/wlan/MT7626_EEPROM3.bin"
#endif /* CONFIG_RT_THIRD_CARD */

BOOLEAN mt7626_cap_md_support;

const struct hif_pci_ring_desc mt7626_tx_ring_layout[] = {
#ifdef MEMORY_SHRINK
	{0, 0,	CONN_HIF_PDMA_TX_RING0_BASE,	256, TX_RING_DATA}, /* AC0 */
	{0, 1,	CONN_HIF_PDMA_TX_RING1_BASE,	256, TX_RING_DATA}, /* AC1 */
	{0, 2,	CONN_HIF_PDMA_TX_RING2_BASE,	256, TX_RING_DATA}, /* AC2 */
	{0, 3,	CONN_HIF_PDMA_TX_RING3_BASE,	128, TX_RING_FWDL},
	{0, 4,	CONN_HIF_PDMA_TX_RING4_BASE,	256, TX_RING_DATA}, /* AC3 */
	{0, 5,	CONN_HIF_PDMA_TX_RING5_BASE,	256, TX_RING_DATA}, /* ALTX */
#else
	{0, 0,	CONN_HIF_PDMA_TX_RING0_BASE,	1024, TX_RING_DATA}, /* AC0 */
	{0, 1,	CONN_HIF_PDMA_TX_RING1_BASE,	1024, TX_RING_DATA}, /* AC1 */
	{0, 2,	CONN_HIF_PDMA_TX_RING2_BASE,	1024, TX_RING_DATA}, /* AC2 */
	{0, 3,	CONN_HIF_PDMA_TX_RING3_BASE,	128, TX_RING_FWDL},
	{0, 4,	CONN_HIF_PDMA_TX_RING4_BASE,	1024, TX_RING_DATA}, /* AC3 */
	{0, 5,	CONN_HIF_PDMA_TX_RING5_BASE,	1024, TX_RING_DATA}, /* ALTX */
#endif
#ifdef RANDOM_PKT_GEN
	{0, 6,	CONN_HIF_PDMA_TX_RING6_BASE,	512, TX_RING_DATA},
	{0, 7,	CONN_HIF_PDMA_TX_RING7_BASE,	512, TX_RING_DATA},
	{0, 8,	CONN_HIF_PDMA_TX_RING8_BASE,	512, TX_RING_DATA},
	{0, 9,	CONN_HIF_PDMA_TX_RING9_BASE,	512, TX_RING_DATA},
	{0, 10,	CONN_HIF_PDMA_TX_RING10_BASE,	512, TX_RING_DATA},
	{0, 11,	CONN_HIF_PDMA_TX_RING11_BASE,	512, TX_RING_DATA},
	{0, 12,	CONN_HIF_PDMA_TX_RING12_BASE,	512, TX_RING_DATA},
	{0, 13,	CONN_HIF_PDMA_TX_RING13_BASE,	512, TX_RING_DATA},
	{0, 14,	CONN_HIF_PDMA_TX_RING14_BASE,	512, TX_RING_DATA},
#endif
	{0, 15,	CONN_HIF_PDMA_TX_RING15_BASE,	128, TX_RING_CMD}
};
#define MT7626_TX_RING_NUM	ARRAY_SIZE(mt7626_tx_ring_layout)

const struct hif_pci_ring_desc mt7626_rx_ring_layout[] = {
#ifdef MEMORY_SHRINK
	{0, 0, CONN_HIF_PDMA_RX_RING0_BASE, 256, RX_RING_DATA},
	{0, 1, CONN_HIF_PDMA_RX_RING1_BASE, 256, RX_RING_EVENT},
#else
	{0, 0, CONN_HIF_PDMA_RX_RING0_BASE, 512, RX_RING_DATA},
	{0, 1, CONN_HIF_PDMA_RX_RING1_BASE, 512, RX_RING_EVENT},
#endif
#ifdef MT7626_E2_SUPPORT
	{0, 2, CONN_HIF_PDMA_RX_RING2_BASE, 512, RX_RING_DATA2},
	{0, 3, CONN_HIF_PDMA_RX_RING3_BASE, 512, RX_RING_EVENT2},
#endif
};
#define MT7626_RX_RING_NUM	ARRAY_SIZE(mt7626_rx_ring_layout)

#ifdef PRE_CAL_MT7626_SUPPORT
UINT16 MT7626_DPD_FLATNESS_ABAND_BW20_FREQ[] = {
	5180, 5200, 5220, 5240, 5260, 5280, 5300, 5320,
	5500, 5520, 5540, 5560, 5580, 5600, 5620, 5640,
	5660, 5680, 5700, 5745, 5765, 5785, 5805, 5825};
UINT16 MT7626_DPD_FLATNESS_GBAND_BW20_FREQ[] = {2422, 2442, 2462};
UINT16 MT7626_DPD_FLATNESS_BW20_FREQ[] = {
	5180, 5200, 5220, 5240, 5260, 5280, 5300, 5320,
	5500, 5520, 5540, 5560, 5580, 5600, 5620, 5640,
	5660, 5680, 5700, 5745, 5765, 5785, 5805, 5825, 2422, 2442, 2462};

UINT16 MT7626_DPD_FLATNESS_ABAND_BW20_CH[] = {
	 36,  40,  44,  48,  52,  56,  60,  64,
	100, 104, 108, 112, 116, 120, 124, 128,
	132, 136, 140, 149, 153, 157, 161, 165};
UINT16 MT7626_DPD_FLATNESS_GBAND_BW20_CH[] = {3, 7, 11};
UINT16 MT7626_DPD_FLATNESS_BW20_CH[] = {
	 36,  40,  44,  48,  52,  56,  60,  64,
	100, 104, 108, 112, 116, 120, 124, 128,
	132, 136, 140, 149, 153, 157, 161, 165, 3, 7, 11};

UINT16 MT7626_DPD_FLATNESS_ABAND_BW20_SIZE = (sizeof(MT7626_DPD_FLATNESS_ABAND_BW20_FREQ) / sizeof(UINT16));
UINT16 MT7626_DPD_FLATNESS_GBAND_BW20_SIZE = (sizeof(MT7626_DPD_FLATNESS_GBAND_BW20_FREQ) / sizeof(UINT16));
UINT16 MT7626_DPD_FLATNESS_BW20_FREQ_SIZE  = (sizeof(MT7626_DPD_FLATNESS_BW20_FREQ) / sizeof(UINT16));

UINT16 MT7626_DPD_FLATNESS_ABAND_BW20_CH_SIZE = (sizeof(MT7626_DPD_FLATNESS_ABAND_BW20_CH) / sizeof(UINT16));
UINT16 MT7626_DPD_FLATNESS_GBAND_BW20_CH_SIZE = (sizeof(MT7626_DPD_FLATNESS_GBAND_BW20_CH) / sizeof(UINT16));
UINT16 MT7626_DPD_FLATNESS_B20_CH_SIZE        = (sizeof(MT7626_DPD_FLATNESS_BW20_CH) / sizeof(UINT16));
#endif /* PRE_CAL_MT7626_SUPPORT */

#ifdef INTERNAL_CAPTURE_SUPPORT
extern RBIST_DESC_T MT7626_ICAP_DESC[];
extern UINT8 MT7626_ICapBankNum;
#endif /* INTERNAL_CAPTURE_SUPPORT */

#ifdef WIFI_SPECTRUM_SUPPORT
extern RBIST_DESC_T MT7626_SPECTRUM_DESC[];
extern UINT8 MT7626_SpectrumBankNum;
#endif /* WIFI_SPECTRUM_SUPPORT */

#ifdef MT7626_FPGA
REG_CHK_PAIR hif_dft_cr[] = {
	{HIF_BASE + 0x00, 0xffffffff, 0x76030001},
	{HIF_BASE + 0x04, 0xffffffff, 0x1b},
	{HIF_BASE + 0x10, 0xffffffff, 0x3f01},
	{HIF_BASE + 0x20, 0xffffffff, 0xe01001e0},
	{HIF_BASE + 0x24, 0xffffffff, 0x1e00000f},

	{HIF_BASE + 0x200, 0xffffffff, 0x0},
	{HIF_BASE + 0x204, 0xffffffff, 0x0},
	{HIF_BASE + 0x208, 0xffffffff, 0x10001870},
	{HIF_BASE + 0x20c, 0xffffffff, 0x0},
	{HIF_BASE + 0x210, 0xffffffff, 0x0},
	{HIF_BASE + 0x214, 0xffffffff, 0x0},
	{HIF_BASE + 0x218, 0xffffffff, 0x0},
	{HIF_BASE + 0x21c, 0xffffffff, 0x0},
	{HIF_BASE + 0x220, 0xffffffff, 0x0},
	{HIF_BASE + 0x224, 0xffffffff, 0x0},
	{HIF_BASE + 0x234, 0xffffffff, 0x0},
	{HIF_BASE + 0x244, 0xffffffff, 0x0},
	{HIF_BASE + 0x300, 0xffffffff, 0x0},
	{HIF_BASE + 0x304, 0xffffffff, 0x0},
	{HIF_BASE + 0x308, 0xffffffff, 0x0},
	{HIF_BASE + 0x30c, 0xffffffff, 0x0},
	{HIF_BASE + 0x310, 0xffffffff, 0x0},
	{HIF_BASE + 0x314, 0xffffffff, 0x0},
	{HIF_BASE + 0x318, 0xffffffff, 0x0},
	{HIF_BASE + 0x31c, 0xffffffff, 0x0},
	{HIF_BASE + 0x320, 0xffffffff, 0x0},
	{HIF_BASE + 0x324, 0xffffffff, 0x0},
	{HIF_BASE + 0x328, 0xffffffff, 0x0},
	{HIF_BASE + 0x32c, 0xffffffff, 0x0},
	{HIF_BASE + 0x330, 0xffffffff, 0x0},
	{HIF_BASE + 0x334, 0xffffffff, 0x0},
	{HIF_BASE + 0x338, 0xffffffff, 0x0},
	{HIF_BASE + 0x33c, 0xffffffff, 0x0},

	{HIF_BASE + 0x400, 0xffffffff, 0x0},
	{HIF_BASE + 0x404, 0xffffffff, 0x0},
	{HIF_BASE + 0x408, 0xffffffff, 0x0},
	{HIF_BASE + 0x40c, 0xffffffff, 0x0},
	{HIF_BASE + 0x410, 0xffffffff, 0x0},
	{HIF_BASE + 0x414, 0xffffffff, 0x0},
	{HIF_BASE + 0x418, 0xffffffff, 0x0},
	{HIF_BASE + 0x41c, 0xffffffff, 0x0},
};


INT mt7626_chk_hif_default_cr_setting(RTMP_ADAPTER *pAd)
{
	UINT32 val;
	INT i;
	BOOLEAN match = TRUE;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Default CR Setting Checking for HIF!\n", __func__));

	for (i = 0; i < sizeof(hif_dft_cr) / sizeof(REG_CHK_PAIR); i++) {
		RTMP_IO_READ32(pAd->hdev_ctrl, hif_dft_cr[i].Register, &val);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t Reg(%x): Current=0x%x(0x%x), Default=0x%x, Mask=0x%x, Match=%s\n",
				 hif_dft_cr[i].Register, val, (val & hif_dft_cr[i].Mask),
				 hif_dft_cr[i].Value, hif_dft_cr[i].Mask,
				 ((val & hif_dft_cr[i].Mask) != hif_dft_cr[i].Value) ? "No" : "Yes"));

		if ((val & hif_dft_cr[i].Mask) != hif_dft_cr[i].Value)
			match = FALSE;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Checking Done, Result=> %s match!\n",
			 __func__, match == TRUE ? "All" : "No"));
	return match;
}


REG_CHK_PAIR top_dft_cr[] = {
	{TOP_CFG_BASE + 0x1000, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1004, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1008, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1010, 0xffffffff, 0x0},

	{TOP_CFG_BASE + 0x1100, 0xffffffff, 0x26110310},
	{TOP_CFG_BASE + 0x1108, 0x0000ff00, 0x1400},
	{TOP_CFG_BASE + 0x110c, 0x00000000, 0x0},
	{TOP_CFG_BASE + 0x1110, 0x0f0f00ff, 0x02090040},
	{TOP_CFG_BASE + 0x1124, 0xf000f00f, 0x00000008},
	{TOP_CFG_BASE + 0x1130, 0x000f0000, 0x0},
	{TOP_CFG_BASE + 0x1134, 0x00000000, 0x0},
	{TOP_CFG_BASE + 0x1140, 0x00ff00ff, 0x0},

	{TOP_CFG_BASE + 0x1200, 0x00000000, 0x0},
	{TOP_CFG_BASE + 0x1204, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1208, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x120c, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1210, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1214, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1218, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x121c, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1220, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1224, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1228, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x122c, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1234, 0x00ffffff, 0x0},
	{TOP_CFG_BASE + 0x1238, 0x00ffffff, 0x0},
	{TOP_CFG_BASE + 0x123c, 0xffffffff, 0x5c1fee80},
	{TOP_CFG_BASE + 0x1240, 0xffffffff, 0x6874ae05},
	{TOP_CFG_BASE + 0x1244, 0xffffffff, 0x00fb89f1},

	{TOP_CFG_BASE + 0x1300, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1304, 0xffffffff, 0x8f020006},
	{TOP_CFG_BASE + 0x1308, 0xffffffff, 0x18010000},
	{TOP_CFG_BASE + 0x130c, 0xffffffff, 0x0130484f},
	{TOP_CFG_BASE + 0x1310, 0xffffffff, 0xff000004},
	{TOP_CFG_BASE + 0x1314, 0xffffffff, 0xf0000084},
	{TOP_CFG_BASE + 0x1318, 0x00000000, 0x0},
	{TOP_CFG_BASE + 0x131c, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1320, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1324, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1328, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x132c, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1330, 0xffffffff, 0x00007800},
	{TOP_CFG_BASE + 0x1334, 0x00000000, 0x0},
	{TOP_CFG_BASE + 0x1338, 0xffffffff, 0x0000000a},
	{TOP_CFG_BASE + 0x1400, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1404, 0xffffffff, 0x00005180},
	{TOP_CFG_BASE + 0x1408, 0xffffffff, 0x00001f00},
	{TOP_CFG_BASE + 0x140c, 0xffffffff, 0x00000020},
	{TOP_CFG_BASE + 0x1410, 0xffffffff, 0x0000003a},
	{TOP_CFG_BASE + 0x141c, 0xffffffff, 0x0},

	{TOP_CFG_BASE + 0x1500, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1504, 0xffffffff, 0x0},
};

INT mt7626_chk_top_default_cr_setting(RTMP_ADAPTER *pAd)
{
	UINT32 val;
	INT i;
	BOOLEAN match = TRUE;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Default CR Setting Checking for TOP!\n", __func__));

	for (i = 0; i < sizeof(top_dft_cr) / sizeof(REG_CHK_PAIR); i++) {
		RTMP_IO_READ32(pAd->hdev_ctrl, top_dft_cr[i].Register, &val);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t Reg(%x): Current=0x%x(0x%x), Default=0x%x, Mask=0x%x, Match=%s\n",
				 top_dft_cr[i].Register, val, (val & top_dft_cr[i].Mask),
				 top_dft_cr[i].Value, top_dft_cr[i].Mask,
				 ((val & top_dft_cr[i].Mask) != top_dft_cr[i].Value) ? "No" : "Yes"));

		if ((val & top_dft_cr[i].Mask) != top_dft_cr[i].Value)
			match = FALSE;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Checking Done, Result=> %s match!\n",
			 __func__, match == TRUE ? "All" : "No"));
	return match;
}
#endif /* MT7626_FPGA */


static VOID mt7626_bbp_adjust(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	/*do nothing, change to use radio_resource control*/
	/*here should do bbp setting only, bbp is full-offload to fw*/
}

#ifdef PRE_CAL_MT7626_SUPPORT
void mt7626_apply_dpd_flatness_data(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	USHORT			doCal1 = 0;
	UINT8			i = 0;
	UINT8			Band = 0;
	UINT16			CentralFreq = 0;

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Currently not in FLASH or BIN MODE,return.\n", __func__));
		return;
	}

#ifdef RTMP_FLASH_SUPPORT
	if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
		rtmp_ee_flash_read(pAd, 0x32, &doCal1);
	}
#endif

	if (((doCal1 & (1 << 1)) != 0) && ((doCal1 & (1 << 2)) != 0)) {
		if (SwChCfg.CentralChannel == 14) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s: CH 14 don't need DPD , return!!!\n", __func__));
			return;
		} else if (SwChCfg.CentralChannel < 14) {
			Band = GBAND;

			if (SwChCfg.CentralChannel >= 1 && SwChCfg.CentralChannel <= 4)
				CentralFreq = 2422;
			else if (SwChCfg.CentralChannel >= 5 && SwChCfg.CentralChannel <= 9)
				CentralFreq = 2442;
			else if (SwChCfg.CentralChannel >= 10 && SwChCfg.CentralChannel <= 13)
				CentralFreq = 2462;
			else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: can't find cent freq for CH %d , should not happen!!!\n",
						  __func__, SwChCfg.CentralChannel));
		} else {
			Band = ABAND;

			if (SwChCfg.Bw == BW_20) {
				CentralFreq = SwChCfg.CentralChannel * 5 + 5000;
			} else if (SwChCfg.Bw == BW_160 || SwChCfg.Bw == BW_8080) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: not support BW8080 or BW160. return\n", __func__));
				return;
			} else {
				UINT32 Central = SwChCfg.CentralChannel * 5 + 5000;
				UINT32 CentralMinus10M = (SwChCfg.CentralChannel - 2) * 5 + 5000;

				if (ChannelFreqToGroup(Central) != ChannelFreqToGroup(CentralMinus10M)) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("==== Different Group Central %d @ group %d Central-10 @ group %d !!\n"
							  , Central, ChannelFreqToGroup(Central), ChannelFreqToGroup(CentralMinus10M)));
					CentralFreq = (SwChCfg.CentralChannel + 2) * 5 + 5000;
				} else
					CentralFreq = (SwChCfg.CentralChannel - 2) * 5 + 5000;
			}
		}
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: eeprom 0x32 bit 0 is 0, do runtime cal , skip TX reload\n", __func__));
		return;
	}

	/* Find if CentralFreq is exist in DPD+Flatness pre-k table */
	for (i = 0; i < MT7626_DPD_FLATNESS_BW20_FREQ_SIZE; i++) {
		if (MT7626_DPD_FLATNESS_BW20_FREQ[i] == CentralFreq) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s: %d is in DPD-Flatness cal table, index = %d\n", __func__, CentralFreq, i));
			break;
		}
	}

	if (i == MT7626_DPD_FLATNESS_BW20_FREQ_SIZE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: Unexpected freq (%d)\n", __func__, CentralFreq));
		return;
	}

	if (Band == GBAND) {
		MtCmdSetDpdFlatnessCal_7626(pAd, i, DPD_FLATNESS_2G_CAL_SIZE);
	} else {
		MtCmdSetDpdFlatnessCal_7626(pAd, i, DPD_FLATNESS_5G_CAL_SIZE);
	}
}
#endif /* PRE_CAL_MT7626_SUPPORT */

/* TODO: Star */
static void mt7626_switch_channel(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	/* update power limit table */
#if defined(SINGLE_SKU) || defined(SINGLE_SKU_V2)
	MtPwrLimitTblChProc(pAd, SwChCfg.BandIdx, SwChCfg.Channel_Band, SwChCfg.ControlChannel, SwChCfg.CentralChannel);
#endif

#ifdef PRE_CAL_MT7626_SUPPORT
    mt7626_apply_dpd_flatness_data(pAd, SwChCfg);
#endif

	MtCmdChannelSwitch(pAd, SwChCfg);
	MtCmdSetTxRxPath(pAd, SwChCfg);
	pAd->LatchRfRegs.Channel = SwChCfg.CentralChannel;
}

#ifdef NEW_SET_RX_STREAM
static INT mt7626_set_RxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx)
{
	UINT32 path = 0;
	UINT i;

	if (StreamNums > 3) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():illegal StreamNums(%d)\n",
				  __func__, StreamNums));
		StreamNums = 3;
	}

	for (i = 0; i < StreamNums; i++)
		path |= 1 << i;

	return MtCmdSetRxPath(pAd, path, BandIdx);
}
#endif

static inline VOID bufferModeFieldSet(RTMP_ADAPTER *pAd, EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd, UINT16 addr)
{
	pCmd->BinContent[addr] = pAd->EEPROMImage[addr];
}


static VOID mt7626_bufferModeEfuseFill(RTMP_ADAPTER *pAd, EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 i = 0;

	pCmd->ucCount = cap->EFUSE_BUFFER_CONTENT_SIZE;
	os_zero_mem(&pCmd->BinContent[0], cap->EFUSE_BUFFER_CONTENT_SIZE);

	for (i = 0x0; i <= EFUSE_CONTENT_END; i++)
		bufferModeFieldSet(pAd, pCmd, i);
}

#ifdef CAL_FREE_IC_SUPPORT
static inline BOOLEAN check_valid(RTMP_ADAPTER *pAd, UINT16 Offset)
{
	UINT16 Value = 0;
	BOOLEAN NotValid;

	if ((Offset % 2) != 0) {
		NotValid = rtmp_ee_efuse_read16(pAd, Offset - 1, &Value);

		if (NotValid)
			return FALSE;

		if (((Value >> 8) & 0xff) == 0x00)
			return FALSE;

		NotValid = rtmp_ee_efuse_read16(pAd, Offset + 1, &Value);

		if (NotValid)
			return FALSE;

		if ((Value & 0xff) == 0x00)
			return FALSE;
	} else {
		NotValid = rtmp_ee_efuse_read16(pAd, Offset, &Value);

		if (NotValid)
			return FALSE;

		if (Value == 0x00)
			return FALSE;
	}

	return TRUE;
}
static BOOLEAN mt7626_is_cal_free_ic(RTMP_ADAPTER *pAd)
{
	UINT16 Value = 0;
	BOOLEAN NotValid;

	NotValid = rtmp_ee_efuse_read16(pAd, 0x52, &Value);

	if (NotValid)
		return FALSE;

	if (((Value >> 8) & 0xff) == 0x00)
		return FALSE;

	NotValid = rtmp_ee_efuse_read16(pAd, 0x54, &Value);

	if (NotValid)
		return FALSE;

	if (Value == 0x00)
		return FALSE;

	if (check_valid(pAd, 0x56) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x5c) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x62) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x68) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x6e) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x73) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x78) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x7d) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x82) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x87) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x8c) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x91) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x96) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x9b) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0xa0) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0xa5) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0xaa) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0xaf) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0xb4) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0xb9) == FALSE)
		return FALSE;

	NotValid = rtmp_ee_efuse_read16(pAd, 0xf4, &Value);

	if (NotValid)
		return FALSE;

	if ((Value & 0xff) == 0x00)
		return FALSE;

	NotValid = rtmp_ee_efuse_read16(pAd, 0xf6, &Value);

	if (NotValid)
		return FALSE;

	if (((Value >> 8) & 0xff) == 0x00)
		return FALSE;

	if (check_valid(pAd, 0x140) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x145) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x14a) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x14f) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x154) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x159) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x15e) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x163) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x168) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x16c) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x172) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x177) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x17c) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x181) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x186) == FALSE)
		return FALSE;

	if (check_valid(pAd, 0x18b) == FALSE)
		return FALSE;

	return TRUE;
}

static inline VOID cal_free_data_get_from_addr(RTMP_ADAPTER *ad, UINT16 Offset)
{
	UINT16 value;

	if ((Offset % 2) != 0) {
		rtmp_ee_efuse_read16(ad, Offset - 1, &value);
		ad->EEPROMImage[Offset] = (value >> 8) & 0xFF;
		rtmp_ee_efuse_read16(ad, Offset + 1, &value);
		ad->EEPROMImage[Offset + 1] =  value & 0xFF;
	} else {
		rtmp_ee_efuse_read16(ad, Offset, &value);
		*(UINT16 *)(&ad->EEPROMImage[Offset]) = le2cpu16(value);
	}
}

static VOID mt7626_cal_free_data_get(RTMP_ADAPTER *ad)

{
	UINT16 value;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	/* 0x53 0x54 0x55 0x56 0x57 0x5C 0x5D */
	rtmp_ee_efuse_read16(ad, 0x52, &value);
	ad->EEPROMImage[0x53] = (value >> 8) & 0xFF;
	rtmp_ee_efuse_read16(ad, 0x54, &value);
	*(UINT16 *)(&ad->EEPROMImage[0x54]) = le2cpu16(value);
	rtmp_ee_efuse_read16(ad, 0x56, &value);
	*(UINT16 *)(&ad->EEPROMImage[0x56]) = le2cpu16(value);
	rtmp_ee_efuse_read16(ad, 0x5c, &value);
	*(UINT16 *)(&ad->EEPROMImage[0x5c]) = le2cpu16(value);
	/* 0x62 0x63 0x68 0x69 0x6E 0x6F */
	cal_free_data_get_from_addr(ad, 0x62);
	cal_free_data_get_from_addr(ad, 0x68);
	cal_free_data_get_from_addr(ad, 0x6e);
	/* 0x73 0x74 0x78 0x79 0x7D 0x7E */
	cal_free_data_get_from_addr(ad, 0x73);
	cal_free_data_get_from_addr(ad, 0x78);
	cal_free_data_get_from_addr(ad, 0x7d);
	/* 0x82 0x83 0x87 0x88 0x8C 0x8D */
	cal_free_data_get_from_addr(ad, 0x82);
	cal_free_data_get_from_addr(ad, 0x87);
	cal_free_data_get_from_addr(ad, 0x8c);
	/* 0x91 0x92 0x96 0x97 0x9B 0x9C */
	cal_free_data_get_from_addr(ad, 0x91);
	cal_free_data_get_from_addr(ad, 0x96);
	cal_free_data_get_from_addr(ad, 0x9b);
	/* 0xA0 0xA1 0xA5 0xA6 0xAA 0xAB 0xAF 0xB0 0xB4 0xB5 0xB9 0xBA */
	cal_free_data_get_from_addr(ad, 0xa0);
	cal_free_data_get_from_addr(ad, 0xa5);
	cal_free_data_get_from_addr(ad, 0xaa);
	cal_free_data_get_from_addr(ad, 0xaf);
	cal_free_data_get_from_addr(ad, 0xb4);
	cal_free_data_get_from_addr(ad, 0xb9);
	/* 0xF4 0xF7  */
	rtmp_ee_efuse_read16(ad, 0xf4, &value);
	ad->EEPROMImage[0xf4]  = value & 0xFF;
	rtmp_ee_efuse_read16(ad, 0xf6, &value);
	ad->EEPROMImage[0xf7] = (value >> 8) & 0xFF;
	/* 0x140 0x141 0x145 0x146 0x14A 0x14B 0x14F 0x150*/
	cal_free_data_get_from_addr(ad, 0x140);
	cal_free_data_get_from_addr(ad, 0x145);
	cal_free_data_get_from_addr(ad, 0x14a);
	cal_free_data_get_from_addr(ad, 0x14f);
	/* 0x154 0x155 0x159 0x15A 0x15E 0x15F */
	cal_free_data_get_from_addr(ad, 0x154);
	cal_free_data_get_from_addr(ad, 0x159);
	cal_free_data_get_from_addr(ad, 0x15e);
	/* 0x163 0x164 0x168 0x169 0x16D 0x16E*/
	cal_free_data_get_from_addr(ad, 0x163);
	cal_free_data_get_from_addr(ad, 0x168);
	cal_free_data_get_from_addr(ad, 0x16d);
	/* 0x172 0x173 0x177 0x178 0x17C 0x17D*/
	cal_free_data_get_from_addr(ad, 0x172);
	cal_free_data_get_from_addr(ad, 0x177);
	cal_free_data_get_from_addr(ad, 0x17c);
	/* 0x181 0x182 0x186 0x187 0x18B 0x18C*/
	cal_free_data_get_from_addr(ad, 0x181);
	cal_free_data_get_from_addr(ad, 0x186);
	cal_free_data_get_from_addr(ad, 0x18b);
}
#endif /* CAL_FREE_IC_SUPPORT */

static INT32 mt7626_dma_shdl_init(RTMP_ADAPTER *pAd)
{
	UINT32 value;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	HIF_DMASHDL_IO_READ32(pAd, MT_HIF_DMASHDL_PKT_MAX_SIZE, &value);
	value &= ~(PLE_PKT_MAX_SIZE_MASK | PSE_PKT_MAX_SIZE_MASK);
	value |= PLE_PKT_MAX_SIZE_NUM(0x1);
	value |= PSE_PKT_MAX_SIZE_NUM(0x8);
	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_PKT_MAX_SIZE, value);

	/* Enable refill Control Group 0, 1, 2, 4, 5 */
	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_REFILL_CTRL, 0xffc80000);
	/* Group 0, 1, 2, 4, 5, 15 joint the ask round robin */
	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_OPTION_CTRL, 0x70068037);
	/*Each group min quota must larger then PLE_PKT_MAX_SIZE_NUM*/
	value = DMASHDL_MIN_QUOTA_NUM(0x40);
	value |= DMASHDL_MAX_QUOTA_NUM(0x800);

	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_GROUP0_CTRL, value);
	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_GROUP1_CTRL, value);
	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_GROUP2_CTRL, value);
	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_GROUP4_CTRL, value);
	value = DMASHDL_MIN_QUOTA_NUM(0x40);
	value |= DMASHDL_MAX_QUOTA_NUM(0x40);
	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_GROUP5_CTRL, value);

	value = DMASHDL_MIN_QUOTA_NUM(0x20);
	value |= DMASHDL_MAX_QUOTA_NUM(0x20);
	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_GROUP15_CTRL, value);

	if ((cap->qm == GENERIC_QM) || (cap->qm == GENERIC_FAIR_QM)) {
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP0, 0x42104210);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP1, 0x42104210);
		/* ALTX0 and ALTX1 QID mapping to group 5 */
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP2, 0x00050005);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP3, 0x0);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_SHDL_SET0, 0x6012345f);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_SHDL_SET1, 0xedcba987);
	}
	return TRUE;
}

#ifdef CFG_SUPPORT_MU_MIMO
#ifdef MANUAL_MU
INT mu_update_profile_tb(RTMP_ADAPTER *pAd, INT profile_id, UCHAR wlan_id)
{
}

INT mu_update_grp_table(RTMP_ADAPTER *pAd, INT grp_id)
{
	return TRUE;
}


INT mu_update_cluster_tb(RTMP_ADAPTER *pAd, UCHAR c_id, UINT32 *m_ship, UINT32 *u_pos)
{
	UINT32 entry_base, mac_val, offset;

	ASSERT(c_id <= 31);
	MAC_IO_READ32(pAd->hdev_ctrl, MU_MUCR1, &mac_val);

	if (c_id < 16)
		mac_val &= (~MUCR1_CLUSTER_TAB_REMAP_CTRL_MASK);
	else
		mac_val |= MUCR1_CLUSTER_TAB_REMAP_CTRL_MASK;

	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_MUCR1, mac_val);
	entry_base = MU_CLUSTER_TABLE_BASE  + (c_id & (~0x10)) * 24;
	/* update membership */
	MAC_IO_WRITE32(pAd->hdev_ctrl, entry_base + 0x0, m_ship[0]);
	MAC_IO_WRITE32(pAd->hdev_ctrl, entry_base + 0x4, m_ship[1]);
	/* Update user position */
	MAC_IO_WRITE32(pAd->hdev_ctrl, entry_base + 0x8, u_pos[0]);
	MAC_IO_WRITE32(pAd->hdev_ctrl, entry_base + 0xc, u_pos[1]);
	MAC_IO_WRITE32(pAd->hdev_ctrl, entry_base + 0x10, u_pos[2]);
	MAC_IO_WRITE32(pAd->hdev_ctrl, entry_base + 0x14, u_pos[3]);
	return TRUE;
}


INT mu_get_wlanId_ac_len(RTMP_ADAPTER *pAd, UINT32 wlan_id, UINT ac)
{
	return TRUE;
}


INT mu_get_mu_tx_retry_cnt(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


INT mu_get_pfid_tx_stat(RTMP_ADAPTER *pAd)
{
}

INT mu_get_gpid_rate_per_stat(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


INT mt7626_mu_init(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;
	/****************************************************************************
		MU Part
	****************************************************************************/
	/* After power on initial setting,  AC legnth clear */
	MAC_IO_READ32(pAd->hdev_ctrl, MU_MUCR4, &mac_val);
	mac_val = 0x1;
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_MUCR4, mac_val); /* 820fe010= 0x0000_0001 */
	/* PFID table */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_PROFILE_TABLE_BASE + 0x0, 0x1e000);  /* 820fe780= 0x0001_e000 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_PROFILE_TABLE_BASE + 0x4, 0x1e103);  /* 820fe784= 0x0001_e103 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_PROFILE_TABLE_BASE + 0x8, 0x1e205);  /* 820fe788= 0x0001_e205 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_PROFILE_TABLE_BASE + 0xc, 0x1e306);  /* 820fe78c= 0x0001_e306 */
	/* Cluster table */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_CLUSTER_TABLE_BASE + 0x0, 0x0);  /* 820fe400= 0x0000_0000 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_CLUSTER_TABLE_BASE + 0x8, 0x0);  /* 820fe408= 0x0000_0000 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_CLUSTER_TABLE_BASE + 0x20, 0x2);  /* 820fe420= 0x0000_0002 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_CLUSTER_TABLE_BASE + 0x28, 0x0);  /* 820fe428= 0x0000_0000 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_CLUSTER_TABLE_BASE + 0x40, 0x2);  /* 820fe440= 0x0000_0002 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_CLUSTER_TABLE_BASE + 0x48, 0x4);  /* 820fe448= 0x0000_0004 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_CLUSTER_TABLE_BASE + 0x60, 0x0);  /* 820fe460= 0x0000_0000 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_CLUSTER_TABLE_BASE + 0x68, 0x0);  /* 820fe468= 0x0000_0000 */
	/* Group rate table */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_GRP_TABLE_RATE_MAP + 0x0, 0x4109);  /* 820ff000= 0x0000_4109 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_GRP_TABLE_RATE_MAP + 0x4, 0x99);  /* 820ff004= 0x0000_0099 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_GRP_TABLE_RATE_MAP + 0x8, 0x800000f0);  /* 820ff008= 0x8000_00f0 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_GRP_TABLE_RATE_MAP + 0xc, 0x99);  /* 820ff00c= 0x0000_0099 */
	/* SU Tx minimum setting */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_MUCR2, 0x10000001);  /* 820fe008= 0x1000_0001 */
	/* MU max group search entry = 1 group entry */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_MUCR1, 0x0);  /* 820fe004= 0x0000_0000 */
	/* MU enable */
	MAC_IO_READ32(pAd->hdev_ctrl, MU_MUCR0, &mac_val);
	mac_val |= 1;
	MAC_IO_WRITE32(pAd->hdev_ctrl, MU_MUCR0, 0x1);  /* 820fe000= 0x1000_0001 */
	/****************************************************************************
		M2M Part
	****************************************************************************/
	/* Enable M2M MU temp mode */
	MAC_IO_READ32(pAd->hdev_ctrl, RMAC_M2M_BAND_CTRL, &mac_val);
	mac_val |= (1 << 16);
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_M2M_BAND_CTRL, mac_val);
	/****************************************************************************
		AGG Part
	****************************************************************************/
	/* 820f20e0[15] = 1 or 0 all need to be verified, because
	    a). if primary is the fake peer, and peer will not ACK to us, cannot setup the TxOP
	    b). Or can use CTS2Self to setup the TxOP
	*/
	MAC_IO_READ32(pAd->hdev_ctrl, AGG_MUCR, &mac_val);
	mac_val &= (~MUCR_PRIM_BAR_MASK);
	/* mac_val |= (1 << MUCR_PRIM_BAR_BIT); */
	MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_MUCR, mac_val);  /* 820fe000= 0x1000_0001 */
	return TRUE;
}
#endif /* MANUAL_MU */
#endif /* CFG_SUPPORT_MU_MIMO */

#ifndef MAC_INIT_OFFLOAD
#endif /* MAC_INIT_OFFLOAD */

/* need to confirm with DE, wilsonl */
static VOID mt7626_init_mac_cr(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __func__));
#ifndef MAC_INIT_OFFLOAD
	/* need to confirm with DE, wilsonl */
	/* done, confirmed by Austin */
	/* Set TxFreeEvent packet only go through CR4 */
	HW_IO_READ32(pAd->hdev_ctrl, PLE_HIF_REPORT, &mac_val);
	mac_val |= 0x1;
	HW_IO_WRITE32(pAd->hdev_ctrl, PLE_HIF_REPORT, mac_val);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s(): Set TxRxEventPkt path 0x%0x = 0x%08x\n",
			  __func__, PLE_HIF_REPORT, mac_val));
	HW_IO_READ32(pAd->hdev_ctrl, PP_PAGECTL_2, &mac_val);
	mac_val &= ~(PAGECTL_2_CUT_PG_CNT_MASK);
	mac_val |= 0x30;
	HW_IO_WRITE32(pAd->hdev_ctrl, PP_PAGECTL_2, mac_val);
#if defined(COMPOS_WIN) || defined(COMPOS_TESTMODE_WIN)
#else
	/* TxS Setting */
	InitTxSTypeTable(pAd);
#endif
	MtAsicSetTxSClassifyFilter(pAd, TXS2HOST, TXS2H_QID1, TXS2HOST_AGGNUMS, 0x00, 0);
#endif /*MAC_INIT_OFFLOAD*/
	/* MAC D0 2x / MAC D0 1x clock enable */
	MAC_IO_READ32(pAd->hdev_ctrl, CFG_CCR, &mac_val);
	mac_val |= (BIT31 | BIT25);
	MAC_IO_WRITE32(pAd->hdev_ctrl, CFG_CCR, mac_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: MAC D0 2x 1x initial(val=%x)\n", __func__, mac_val));
	/*  Disable RX Header Translation */
	MAC_IO_READ32(pAd->hdev_ctrl, DMA_DCR0, &mac_val);
	mac_val &= ~(DMA_DCR0_RX_HDR_TRANS_EN_BIT | DMA_DCR0_RX_HDR_TRANS_MODE_BIT |
				 DMA_DCR0_RX_RM_VLAN_BIT | DMA_DCR0_RX_INS_VLAN_BIT |
				 DMA_DCR0_RX_HDR_TRANS_CHK_BSSID);
#ifdef HDR_TRANS_RX_SUPPORT

	if (IS_ASIC_CAP(pAd, fASIC_CAP_RX_HDR_TRANS)) {
		UINT32 mac_val2;

		mac_val |= DMA_DCR0_RX_HDR_TRANS_EN_BIT | DMA_DCR0_RX_RM_VLAN_BIT |
				   DMA_DCR0_RX_HDR_TRANS_CHK_BSSID;
		/* TODO: UnifiedSW, take care about Windows for translation mode! */
		/* mac_val |= DMA_DCR0_RX_HDR_TRANS_MODE_BIT; */
		MAC_IO_READ32(pAd->hdev_ctrl, DMA_DCR1, &mac_val2);
		mac_val2 |= RHTR_AMS_VLAN_EN;
		MAC_IO_WRITE32(pAd->hdev_ctrl, DMA_DCR1, mac_val2);
	}

#endif /* HDR_TRANS_RX_SUPPORT */
	MAC_IO_WRITE32(pAd->hdev_ctrl, DMA_DCR0, mac_val);
	/* CCA Setting */
	MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TRCR0, &mac_val);
	mac_val &= ~CCA_SRC_SEL_MASK;
	mac_val |= CCA_SRC_SEL(0x2);
	mac_val &= ~CCA_SEC_SRC_SEL_MASK;
	mac_val |= CCA_SEC_SRC_SEL(0x0);
	MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_TRCR0, mac_val);
	MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TRCR0, &mac_val);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s(): TMAC_TRCR0=0x%x\n", __func__, mac_val));
	/* ---Add by shiang for MT7615 RFB ED issue */
	/* Set BAR rate as 0FDM 6M default, remove after fw set */
	MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_ACR0, 0x04b10496);
	/*Add by Star for zero delimiter*/
	MAC_IO_READ32(pAd->hdev_ctrl, TMAC_CTCR0, &mac_val);
	mac_val &= ~INS_DDLMT_REFTIME_MASK;
	mac_val |= INS_DDLMT_REFTIME(0x3f);
	mac_val |= DUMMY_DELIMIT_INSERTION;
	mac_val |= INS_DDLMT_DENSITY(3);
	MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_CTCR0, mac_val);
	/* Temporary setting for RTS */
	/*if no protect should enable for CTS-2-Self, WHQA_00025629*/
		MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_PCR1, 0x060003e8);
		MAC_IO_READ32(pAd->hdev_ctrl, AGG_SCR, &mac_val);
		mac_val |= NLNAV_MID_PTEC_DIS;
		MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_SCR, mac_val);
	/*Default disable rf low power beacon mode*/
#define WIFI_SYS_PHY 0x10000
#define RF_LOW_BEACON_BAND0 (WIFI_SYS_PHY+0x1900)
#define RF_LOW_BEACON_BAND1 (WIFI_SYS_PHY+0x1d00)
	PHY_IO_READ32(pAd->hdev_ctrl, RF_LOW_BEACON_BAND0, &mac_val);
	mac_val &= ~(0x3 << 8);
	mac_val |= (0x2 << 8);
	PHY_IO_WRITE32(pAd->hdev_ctrl, RF_LOW_BEACON_BAND0, mac_val);
	PHY_IO_READ32(pAd->hdev_ctrl, RF_LOW_BEACON_BAND1, &mac_val);
	mac_val &= ~(0x3 << 8);
	mac_val |= (0x2 << 8);
	PHY_IO_WRITE32(pAd->hdev_ctrl, RF_LOW_BEACON_BAND1, mac_val);

	/* map the 0x8208xxxx to 0x4xxxx for PHY1 */
	MAC_IO_READ32(pAd->hdev_ctrl, 0x7010, &mac_val);
	MAC_IO_WRITE32(pAd->hdev_ctrl, 0x7010, ((mac_val & (0xffff0000)) | 0x8208));

#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	{
		UINT32 bbp_val, i;
		struct wifi_dev *wdev = NULL;
		UCHAR ucETxBfCap = 0;

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			wdev = &pAd->ApCfg.MBSSID[i].wdev;

			if (wdev && (wdev->channel > 14)) {
				ucETxBfCap = wlan_config_get_etxbf(wdev);
				if (ucETxBfCap)
					break;
			}
		}
		if (ucETxBfCap) {
			/* CR_COM_RFINTF_CKO_ALLON_DAC to be 1 */
			MAC_IO_READ32(pAd->hdev_ctrl, 0x44064, &bbp_val);
			MAC_IO_WRITE32(pAd->hdev_ctrl, 0x44064, bbp_val | 0x2000000);
		}
	}
#endif
	/* Dynamic TXOP truncation Enable */
	/*
	DYN_TXOP_TRUNC_MPDU_TH_0[15..8] - (RW) Dynamic TXOP truncation MPDU number Threshold for TX Bit Rate <= 97.5Mbps                 AMSDU=1  AMPDU=28     truncate=0x1c
	DYN_TXOP_TRUNC_MPDU_TH_1[23..16] - (RW) Dynamic TXOP truncation MPDU number Threshold for 97.5Mbps < TX Bit Rate <= 195Mbps      AMSDU=1  AMPDU=52     truncate=0x34
	DYN_TXOP_TRUNC_MPDU_TH_2[31..24] - (RW) Dynamic TXOP truncation MPDU number Threshold for 195Mbps < TX Bit Rate <= 325Mbps       AMSDU=1  AMPDU=92     truncate=0x5c
	DYN_TXOP_TRUNC_MPDU_TH_3[7..0] - (RW) Dynamic TXOP truncation MPDU number Threshold for 325Mbps < TX Bit Rate <= 433Mbps         AMSDU=2  AMPDU=128    truncate=0x40
	DYN_TXOP_TRUNC_MPDU_TH_4[15..8] - (RW) Dynamic TXOP truncation MPDU number Threshold for 433Mbps < TX Bit Rate <= 866.7Mbps      AMSDU=2   AMPDU=256   truncate=0x80
	DYN_TXOP_TRUNC_MPDU_TH_5[23..16] - (RW) Dynamic TXOP truncation MPDU number Threshold for 866.7Mbps < TX Bit Rate <= 1300Mbps    AMSDU=3   AMPDU=338   truncate=0x70
	*/
	MAC_IO_WRITE32(pAd->hdev_ctrl, WF_AGG_BASE+0x160, 0x5c341c02);
	MAC_IO_WRITE32(pAd->hdev_ctrl, WF_AGG_BASE+0x164, 0x70708040);

#ifdef MT7626_E2_SUPPORT
	if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD)) {
		/* init BN0RCF0 to hif ring2 */
		MAC_IO_READ32(pAd->hdev_ctrl, DMA_BN0RCFR1, &mac_val); /* Rx Classify filter 0 */
		mac_val |= 0x10000;
		MAC_IO_WRITE32(pAd->hdev_ctrl, DMA_BN0RCFR1, mac_val);
	}
#endif
}






static VOID MT7626BBPInit(RTMP_ADAPTER *pAd)
{
	BOOLEAN isDBDC = TRUE, band_vld[2];
	INT idx, cbw[2] = {0};
	INT cent_ch[2] = {0}, prim_ch[2] = {0}, prim_ch_idx[2] = {0};
	INT band[2] = {0};
	INT txStream[2] = {0};
	UCHAR use_bands;

	band_vld[0] = TRUE;
	cbw[0] = RF_BW_20;
	cent_ch[0] = 1;
	prim_ch[0] = 1;
	band[0] = BAND_24G;
	txStream[0] = 3;
#ifdef DOT11_VHT_AC
	prim_ch_idx[0] = vht_prim_ch_idx(cent_ch[0], prim_ch[0], cbw[0]);
#endif /* DOT11_VHT_AC */

	if (isDBDC) {
		band_vld[1] = TRUE;
		band[1] = BAND_5G;
		cbw[1] = RF_BW_20;
		cent_ch[1] = 36;
		prim_ch[1] = 36;
#ifdef DOT11_VHT_AC
		prim_ch_idx[1] = vht_prim_ch_idx(cent_ch[1], prim_ch[1], cbw[1]);
#endif /* DOT11_VHT_AC */
		txStream[1] = 3;
		use_bands = 2;
	} else {
		band_vld[1] = FALSE;
		use_bands = 1;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s():BBP Initialization.....\n", __func__));

	for (idx = 0; idx < 2; idx++) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tBand %d: valid=%d, isDBDC=%d, Band=%d, CBW=%d, CentCh/PrimCh=%d/%d, prim_ch_idx=%d, txStream=%d\n",
				  idx, band_vld[idx], isDBDC, band[idx], cbw[idx], cent_ch[idx], prim_ch[idx],
				  prim_ch_idx[idx], txStream[idx]));
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() todo\n", __func__));
}


static void mt7626_init_rf_cr(RTMP_ADAPTER *ad)
{
}

/* Read power per rate */
void mt7626_get_tx_pwr_per_rate(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() todo\n", __func__));
}


void mt7626_get_tx_pwr_info(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() todo\n", __func__));
}

static void mt7626_antenna_default_reset(
	struct _RTMP_ADAPTER *pAd,
	EEPROM_ANTENNA_STRUC *pAntenna)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 max_nss = cap->mcs_nss.max_nss;
#ifdef CONNAC_EFUSE_FORMAT_SUPPORT
	P_EFUSE_INFO_MODULE_SYSTEM_T prSys = pAd->EfuseInfoAll.prSys;
#endif /* CONNAC_EFUSE_FORMAT_SUPPORT */

	pAntenna->word = 0;
	pAd->RfIcType = RFIC_7626;
#ifdef CONNAC_EFUSE_FORMAT_SUPPORT
	/* MT7626 should always DBDC mode on */
	if (prSys->u1DbdcForceMode) {

		/* Tx */
		if ((prSys->u1Dbdc0TxPath > 0) && (prSys->u1Dbdc0TxPath <= max_nss)) {
			pAd->dbdc_band0_tx_path = prSys->u1Dbdc0TxPath;
		} else {
			pAd->dbdc_band0_tx_path = max_nss;
		}

		if ((prSys->u1Dbdc1TxPath > 0) && (prSys->u1Dbdc1TxPath <= max_nss)) {
			pAd->dbdc_band1_tx_path = prSys->u1Dbdc1TxPath;
		} else {
			pAd->dbdc_band1_tx_path = max_nss;
		}
		/* Rx */
		if ((prSys->u1Dbdc0RxPath > 0) && (prSys->u1Dbdc0RxPath <= max_nss)) {
			pAd->dbdc_band0_rx_path = prSys->u1Dbdc0RxPath;
		} else {
			pAd->dbdc_band0_rx_path = max_nss;
		}

		if ((prSys->u1Dbdc1RxPath > 0) && (prSys->u1Dbdc1RxPath <= max_nss)) {
			pAd->dbdc_band1_rx_path = prSys->u1Dbdc1RxPath;
		} else {
			pAd->dbdc_band1_rx_path = max_nss;
		}


		/* sync band0's setting to pAntenna->field.TxPath and pAntenna->field.RxPath */
		pAntenna->field.TxPath = pAd->dbdc_band0_tx_path;
		pAntenna->field.RxPath = pAd->dbdc_band0_rx_path;

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() Tx/Rx PATH E2P BAND0: TxPath/RxPath=%d/%d, BAND1: TxPath/RxPath=%d/%d\n",
			__func__, prSys->u1Dbdc0TxPath, prSys->u1Dbdc0RxPath, prSys->u1Dbdc1TxPath, prSys->u1Dbdc1RxPath));
	} else {
		/*
			MT7626 should not happened here!
		*/
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() Tx/Rx PATH hard code to cap->mcs_nss.max_nss=%d!!\n", __func__, max_nss));
		pAd->dbdc_band0_tx_path = pAd->dbdc_band1_tx_path = prSys->u1Dbdc0TxPath = prSys->u1Dbdc1TxPath = pAntenna->field.TxPath = max_nss;
		pAd->dbdc_band0_rx_path = pAd->dbdc_band1_rx_path = prSys->u1Dbdc0RxPath = prSys->u1Dbdc1RxPath = pAntenna->field.RxPath = max_nss;
	}
#endif /* CONNAC_EFUSE_FORMAT_SUPPORT */

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): TxPath = %d, RxPath = %d\n",
		__func__, pAntenna->field.TxPath, pAntenna->field.RxPath));

#ifdef DBDC_MODE
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): DBDC BAND0 TxPath = %d, RxPath = %d\n",
		__func__, pAd->dbdc_band0_tx_path, pAd->dbdc_band0_rx_path));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): DBDC BAND1 TxPath = %d, RxPath = %d\n",
		__func__, pAd->dbdc_band1_tx_path, pAd->dbdc_band1_rx_path));
#endif
}


static VOID mt7626_fw_prepare(RTMP_ADAPTER *pAd)
{
	struct fwdl_ctrl *ctrl = &pAd->MCUCtrl.fwdl_ctrl;

#ifdef NEED_ROM_PATCH
	if (IS_MT7626_FW_VER_E1(pAd)) {
		ctrl->patch_profile[WM_CPU].source.header_ptr = mt7626_rom_patch_e1;
		ctrl->patch_profile[WM_CPU].source.header_len = sizeof(mt7626_rom_patch_e1);
		ctrl->patch_profile[WM_CPU].source.bin_name = MT7626_ROM_PATCH_BIN_FILE_NAME_E1;
	} else {
		/* Use E1 rom patch as default */
		ctrl->patch_profile[WM_CPU].source.header_ptr = mt7626_rom_patch_e1;
		ctrl->patch_profile[WM_CPU].source.header_len = sizeof(mt7626_rom_patch_e1);
		ctrl->patch_profile[WM_CPU].source.bin_name = MT7626_ROM_PATCH_BIN_FILE_NAME_E1;
	}
#endif /* NEED_ROM_PATCH */

	ctrl->fw_profile[WM_CPU].source.header_ptr = MT7626_FirmwareImage;
	ctrl->fw_profile[WM_CPU].source.header_len = sizeof(MT7626_FirmwareImage);
	ctrl->fw_profile[WM_CPU].source.bin_name = MT7626_BIN_FILE_NAME;
}

static VOID mt7626_fwdl_datapath_setup(RTMP_ADAPTER *pAd, BOOLEAN init)
{
	WPDMA_GLO_CFG_STRUC GloCfg;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (init == TRUE) {
		HIF_IO_READ32(pAd->hdev_ctrl, MT_WPDMA_GLO_CFG, &GloCfg.word);
		GloCfg.MT7626_field.fw_ring_bp_tx_sch = 1;
		HIF_IO_WRITE32(pAd->hdev_ctrl, MT_WPDMA_GLO_CFG, GloCfg.word);

		ops->pci_kick_out_cmd_msg = AndesMTPciKickOutCmdMsgFwDlRing;
	} else {
		ops->pci_kick_out_cmd_msg = AndesMTPciKickOutCmdMsg;

		HIF_IO_READ32(pAd->hdev_ctrl, MT_WPDMA_GLO_CFG, &GloCfg.word);
		GloCfg.MT7626_field.fw_ring_bp_tx_sch = 0;
		HIF_IO_WRITE32(pAd->hdev_ctrl, MT_WPDMA_GLO_CFG, GloCfg.word);
	}
}

#ifdef CONFIG_STA_SUPPORT
static VOID mt7626_init_dev_nick_name(RTMP_ADAPTER *ad)
{
	snprintf((RTMP_STRING *) ad->nickname, sizeof(ad->nickname), "mt7626_sta");
}
#endif /* CONFIG_STA_SUPPORT */


#ifdef TXBF_SUPPORT
void mt7626_setETxBFCap(
	IN  RTMP_ADAPTER      *pAd,
	IN  TXBF_STATUS_INFO * pTxBfInfo)
{
	HT_BF_CAP *pTxBFCap = pTxBfInfo->pHtTxBFCap;

	if (pTxBfInfo->cmmCfgETxBfEnCond > 0) {
		switch (pTxBfInfo->cmmCfgETxBfEnCond) {
		case SUBF_ALL:
		default:
			pTxBFCap->RxNDPCapable         = TRUE;
			pTxBFCap->TxNDPCapable         = (pTxBfInfo->ucRxPathNum > 1) ? TRUE : FALSE;
			pTxBFCap->ExpNoComSteerCapable = FALSE;
			pTxBFCap->ExpComSteerCapable   = TRUE;/* !pTxBfInfo->cmmCfgETxBfNoncompress; */
			pTxBFCap->ExpNoComBF           = 0; /* HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->ExpComBF             =
				HT_ExBF_FB_CAP_IMMEDIATE;/* pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->MinGrouping          = 3;
			pTxBFCap->NoComSteerBFAntSup   = 0;
			pTxBFCap->ComSteerBFAntSup     = 3;
			pTxBFCap->TxSoundCapable       = FALSE;  /* Support staggered sounding frames */
			pTxBFCap->ChanEstimation       = pTxBfInfo->ucRxPathNum - 1;
			break;

		case SUBF_BFER:
			pTxBFCap->RxNDPCapable         = FALSE;
			pTxBFCap->TxNDPCapable         = (pTxBfInfo->ucRxPathNum > 1) ? TRUE : FALSE;
			pTxBFCap->ExpNoComSteerCapable = FALSE;
			pTxBFCap->ExpComSteerCapable   = TRUE;/* !pTxBfInfo->cmmCfgETxBfNoncompress; */
			pTxBFCap->ExpNoComBF           = 0; /* HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->ExpComBF             =
				HT_ExBF_FB_CAP_IMMEDIATE;/* pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->MinGrouping          = 3;
			pTxBFCap->NoComSteerBFAntSup   = 0;
			pTxBFCap->ComSteerBFAntSup     = 3;
			pTxBFCap->TxSoundCapable       = FALSE;  /* Support staggered sounding frames */
			pTxBFCap->ChanEstimation       = pTxBfInfo->ucRxPathNum - 1;
			break;

		case SUBF_BFEE:
			pTxBFCap->RxNDPCapable         = TRUE;
			pTxBFCap->TxNDPCapable         = FALSE;
			pTxBFCap->ExpNoComSteerCapable = FALSE;
			pTxBFCap->ExpComSteerCapable   = TRUE;/* !pTxBfInfo->cmmCfgETxBfNoncompress; */
			pTxBFCap->ExpNoComBF           = 0; /* HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->ExpComBF             =
				HT_ExBF_FB_CAP_IMMEDIATE;/* pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->MinGrouping          = 3;
			pTxBFCap->NoComSteerBFAntSup   = 0;
			pTxBFCap->ComSteerBFAntSup     = 3;
			pTxBFCap->TxSoundCapable       = FALSE;  /* Support staggered sounding frames */
			pTxBFCap->ChanEstimation       = pTxBfInfo->ucRxPathNum - 1;
			break;
		}
	} else
		memset(pTxBFCap, 0, sizeof(*pTxBFCap));
}

#ifdef VHT_TXBF_SUPPORT
void mt7626_setVHTETxBFCap(
	IN  RTMP_ADAPTER *pAd,
	IN  TXBF_STATUS_INFO * pTxBfInfo)
{
	VHT_CAP_INFO *pTxBFCap = pTxBfInfo->pVhtTxBFCap;

	/*
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: cmmCfgETxBfEnCond = %d\n", __FUNCTION__, (UCHAR)pTxBfInfo->cmmCfgETxBfEnCond));
	*/

	if (pTxBfInfo->cmmCfgETxBfEnCond > 0) {
		switch (pTxBfInfo->cmmCfgETxBfEnCond) {
		case SUBF_ALL:
		default:
			pTxBFCap->bfee_cap_su       = 1;
			pTxBFCap->bfer_cap_su       = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
#ifdef CFG_SUPPORT_MU_MIMO

			switch (pAd->CommonCfg.MUTxRxEnable) {
			case MUBF_OFF:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_BFER:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
				break;

			case MUBF_BFEE:
				pTxBFCap->bfee_cap_mu = 1;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_ALL:
				pTxBFCap->bfee_cap_mu = 1;
				pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
				break;

			default:
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: set wrong parameters\n", __func__));
				break;
			}

#else
			pTxBFCap->bfee_cap_mu = 0;
			pTxBFCap->bfer_cap_mu = 0;
#endif /* CFG_SUPPORT_MU_MIMO */
			if (pTxBFCap->bfee_sts_cap)
				pTxBFCap->bfee_sts_cap	= (pTxBFCap->bfee_sts_cap < 3) ? pTxBFCap->bfee_sts_cap : 3;
			else
				pTxBFCap->bfee_sts_cap  = 3;
			pTxBFCap->num_snd_dimension = pTxBfInfo->ucTxPathNum - 1;
			break;

		case SUBF_BFER:
			pTxBFCap->bfee_cap_su       = 0;
			pTxBFCap->bfer_cap_su       = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
#ifdef CFG_SUPPORT_MU_MIMO

			switch (pAd->CommonCfg.MUTxRxEnable) {
			case MUBF_OFF:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_BFER:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
				break;

			case MUBF_BFEE:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_ALL:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
				break;

			default:
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: set wrong parameters\n", __func__));
				break;
			}

#else
			pTxBFCap->bfee_cap_mu = 0;
			pTxBFCap->bfer_cap_mu = 0;
#endif /* CFG_SUPPORT_MU_MIMO */
			pTxBFCap->bfee_sts_cap      = 0;
			pTxBFCap->num_snd_dimension = pTxBfInfo->ucTxPathNum - 1;
			break;

		case SUBF_BFEE:
			pTxBFCap->bfee_cap_su       = 1;
			pTxBFCap->bfer_cap_su       = 0;
#ifdef CFG_SUPPORT_MU_MIMO

			switch (pAd->CommonCfg.MUTxRxEnable) {
			case MUBF_OFF:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_BFER:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_BFEE:
				pTxBFCap->bfee_cap_mu = 1;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_ALL:
				pTxBFCap->bfee_cap_mu = 1;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			default:
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: set wrong parameters\n", __func__));
				break;
			}

#else
			pTxBFCap->bfee_cap_mu = 0;
			pTxBFCap->bfer_cap_mu = 0;
#endif /* CFG_SUPPORT_MU_MIMO */
			if (pTxBFCap->bfee_sts_cap)
				pTxBFCap->bfee_sts_cap	= (pTxBFCap->bfee_sts_cap < 3) ? pTxBFCap->bfee_sts_cap : 3;
			else
				pTxBFCap->bfee_sts_cap	= 3;
			pTxBFCap->num_snd_dimension = pTxBfInfo->ucTxPathNum - 1;
			break;
		}
	} else {
		pTxBFCap->num_snd_dimension = 0;
		pTxBFCap->bfee_cap_mu       = 0;
		pTxBFCap->bfee_cap_su       = 0;
		pTxBFCap->bfer_cap_mu       = 0;
		pTxBFCap->bfer_cap_su       = 0;
		pTxBFCap->bfee_sts_cap      = 0;
	}
}

#endif /* VHT_TXBF_SUPPORT */
#endif /* TXBF_SUPPORT */

UCHAR *mt7626_get_default_bin_image_file(RTMP_ADAPTER *pAd)
{
#ifdef MULTI_INF_SUPPORT
	if (multi_inf_get_idx(pAd) == 0) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Use %dst %s default bin.\n", multi_inf_get_idx(pAd), DEFAULT_BIN_FILE));
		return DEFAULT_BIN_FILE;
	}
#if defined(MT_SECOND_CARD)
	else if (multi_inf_get_idx(pAd) == 1) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Use %dst %s default bin.\n", multi_inf_get_idx(pAd), SECOND_BIN_FILE));
		return SECOND_BIN_FILE;
	}
#endif /* MT_SECOND_CARD */
#if defined(MT_THIRD_CARD)
	else if (multi_inf_get_idx(pAd) == 2) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Use %dst %s default bin.\n", multi_inf_get_idx(pAd), THIRD_BIN_FILE));
		return THIRD_BIN_FILE;
	}
#endif /* MT_THIRD_CARD */
	else
#endif /* MULTI_INF_SUPPORT */
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Use the default %s bin image!\n", DEFAULT_BIN_FILE));

		return DEFAULT_BIN_FILE;
	}

	return NULL;
}

static INT hif_init_WPDMA(RTMP_ADAPTER *pAd)
{
	WPDMA_GLO_CFG_STRUC GloCfg;

	HIF_IO_READ32(pAd->hdev_ctrl, MT_WPDMA_GLO_CFG, &GloCfg.word);

	GloCfg.word = 0x10001870;

	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_WPDMA_GLO_CFG, GloCfg.word);

	return TRUE;
}

static INT hif_set_WPDMA(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable)
{
	WPDMA_GLO_CFG_STRUC GloCfg;
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 WPDMABurstSIZE;
#ifdef CONFIG_DELAY_INT
	UINT32 Value;
#endif
#ifdef MT7626_E2_SUPPORT
	MD_WPDMA_GLO_CFG_STRUC MD_GloCfg;
#ifdef CONFIG_DELAY_INT
	UINT32 Value2;
#endif
#endif

	WPDMABurstSIZE = chip_cap->WPDMABurstSIZE;
	HIF_IO_READ32(pAd->hdev_ctrl, MT_WPDMA_GLO_CFG, &GloCfg.word);
#ifdef CONFIG_DELAY_INT
	HIF_IO_READ32(pAd->hdev_ctrl, MT_DELAY_INT_CFG, &Value);
#endif

	switch (TxRx) {
	case PDMA_TX:
		if (enable == TRUE) {
			GloCfg.MT7626_field.EnableTxDMA = 1;
			GloCfg.MT7626_field.EnTXWriteBackDDONE = 1;
			GloCfg.MT7626_field.WPDMABurstSIZE = WPDMABurstSIZE;
			GloCfg.MT7626_field.multi_dma_en = MULTI_DMA_EN_FEATURE_2_PREFETCH;
#ifdef CONFIG_DELAY_INT
			Value |= TX_DLY_INT_EN;
			Value &= ~TX_MAX_PINT_MASK;
			Value |= TX_MAX_PINT(TX_PENDING_INT_NUMS);
			Value &= ~TX_MAX_PTIME_MASK;
			Value |= TX_MAX_PTIME(TX_PENDING_INT_TIME);
#endif
		} else {
			GloCfg.MT7626_field.EnableTxDMA = 0;
#ifdef CONFIG_DELAY_INT
			Value &= ~TX_DLY_INT_EN;
			Value &= ~TX_MAX_PINT_MASK;
			Value &= ~TX_MAX_PTIME_MASK;
#endif
		}

		break;

	case PDMA_RX:
		if (enable == TRUE) {
			GloCfg.MT7626_field.EnableRxDMA = 1;
			GloCfg.MT7626_field.WPDMABurstSIZE = WPDMABurstSIZE;
			GloCfg.MT7626_field.multi_dma_en = MULTI_DMA_EN_FEATURE_2_PREFETCH;
#ifdef CONFIG_DELAY_INT
			Value |= RX_DLY_INT_EN;
			Value &= ~RX_MAX_PINT_MASK;
			Value |= RX_MAX_PINT(RX_PENDING_INT_NUMS);
			Value &= ~RX_MAX_PTIME_MASK;
			Value |= RX_MAX_PTIME(RX_PENDING_INT_TIME);
#endif
#ifdef MT7626_E2_SUPPORT
			if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD)) {
				MD_GloCfg.MT7626_field.MD_EnableRxDMA = 1;
				MD_GloCfg.MT7626_field.MD_RX_RING2_ENA = 1;
#ifdef CONFIG_DELAY_INT
				Value2 |= RX_DLY_INT_EN;
				Value2 &= ~RX_MAX_PINT_MASK;
				Value2 |= RX_MAX_PINT(RX_PENDING_INT_NUMS);
				Value2 &= ~RX_MAX_PTIME_MASK;
				Value2 |= RX_MAX_PTIME(RX_PENDING_INT_TIME);
#endif
			}
#endif
		} else {
			GloCfg.MT7626_field.EnableRxDMA = 0;
#ifdef CONFIG_DELAY_INT
			Value &= ~RX_DLY_INT_EN;
			Value &= ~RX_MAX_PINT_MASK;
			Value &= ~RX_MAX_PTIME_MASK;
#endif
#ifdef MT7626_E2_SUPPORT
			if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD)) {
				MD_GloCfg.MT7626_field.MD_EnableRxDMA = 0;
				MD_GloCfg.MT7626_field.MD_RX_RING2_ENA = 0;
#ifdef CONFIG_DELAY_INT
				Value2 &= ~RX_DLY_INT_EN;
				Value2 &= ~RX_MAX_PINT_MASK;
				Value2 &= ~RX_MAX_PTIME_MASK;
#endif
			}
#endif
		}

		break;

	case PDMA_TX_RX:
		if (enable == TRUE) {
			GloCfg.MT7626_field.EnableTxDMA = 1;
			GloCfg.MT7626_field.EnableRxDMA = 1;
			GloCfg.MT7626_field.EnTXWriteBackDDONE = 1;
			GloCfg.MT7626_field.WPDMABurstSIZE = WPDMABurstSIZE;
			GloCfg.MT7626_field.multi_dma_en = MULTI_DMA_EN_FEATURE_2_PREFETCH;

#ifdef CONFIG_DELAY_INT
			Value |= TX_DLY_INT_EN;
			Value &= ~TX_MAX_PINT_MASK;
			Value |= TX_MAX_PINT(TX_PENDING_INT_NUMS);
			Value &= ~TX_MAX_PTIME_MASK;
			Value |= TX_MAX_PTIME(TX_PENDING_INT_TIME);
			Value |= RX_DLY_INT_EN;
			Value &= ~RX_MAX_PINT_MASK;
			Value |= RX_MAX_PINT(RX_PENDING_INT_NUMS);
			Value &= ~RX_MAX_PTIME_MASK;
			Value |= RX_MAX_PTIME(RX_PENDING_INT_TIME);
#endif
#ifdef MT7626_E2_SUPPORT
			if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD)) {
				MD_GloCfg.MT7626_field.MD_EnableRxDMA = 1;
				MD_GloCfg.MT7626_field.MD_RX_RING2_ENA = 1;
#ifdef CONFIG_DELAY_INT
				Value2 |= RX_DLY_INT_EN;
				Value2 &= ~RX_MAX_PINT_MASK;
				Value2 |= RX_MAX_PINT(RX_PENDING_INT_NUMS);
				Value2 &= ~RX_MAX_PTIME_MASK;
				Value2 |= RX_MAX_PTIME(RX_PENDING_INT_TIME);
#endif
			}
#endif
		} else {
			GloCfg.MT7626_field.EnableRxDMA = 0;
			GloCfg.MT7626_field.EnableTxDMA = 0;
#ifdef CONFIG_DELAY_INT
			Value &= ~TX_DLY_INT_EN;
			Value &= ~TX_MAX_PINT_MASK;
			Value &= ~TX_MAX_PTIME_MASK;
			Value &= ~RX_DLY_INT_EN;
			Value &= ~RX_MAX_PINT_MASK;
			Value &= ~RX_MAX_PTIME_MASK;
#endif
#ifdef MT7626_E2_SUPPORT
			if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD)) {
				MD_GloCfg.MT7626_field.MD_EnableRxDMA = 0;
				MD_GloCfg.MT7626_field.MD_RX_RING2_ENA = 0;
#ifdef CONFIG_DELAY_INT
				Value2 &= ~RX_DLY_INT_EN;
				Value2 &= ~RX_MAX_PINT_MASK;
				Value2 &= ~RX_MAX_PTIME_MASK;
#endif
			}
#endif
		}

		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown path (%d\n", __func__, TxRx));
		break;
	}

	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_WPDMA_GLO_CFG, GloCfg.word);
#ifdef CONFIG_DELAY_INT
	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_DELAY_INT_CFG, Value);
#endif
#ifdef MT7626_E2_SUPPORT
	if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD)) {
		HIF_IO_WRITE32(pAd->hdev_ctrl, MT_MD_WPDMA_GLO_CFG, MD_GloCfg.word);
#ifdef CONFIG_DELAY_INT
		HIF_IO_WRITE32(pAd->hdev_ctrl, MT_MD_WPDMA_DELAY_INT_CFG, Value2);
#endif
	}
#endif

#define WPDMA_DISABLE -1

	if (!enable)
		TxRx = WPDMA_DISABLE;

	WLAN_HOOK_CALL(WLAN_HOOK_DMA_SET, pAd, &TxRx);
	return TRUE;
}

static BOOLEAN hif_wait_WPDMA_idle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us)
{
	INT i = 0;

	UINT32 value = 0;
	do {
		HIF_IO_READ32(pAd->hdev_ctrl, MT_CONN_HIF_BUSY_STATUS, &value);

		if ((value & CONN_HIF_BUSY) == 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>	DMAIdle, CONN_HIF_BUSY_STATUS=0x%x\n", value));
			return TRUE;
		}

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return FALSE;

		RtmpusecDelay(wait_us);
	} while ((i++) < round);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>  DMABusy\n"));
	return FALSE;
}

static BOOLEAN hif_reset_WPDMA(RTMP_ADAPTER *pAd)
{
	UINT32 value = 0;

	/* pdma0 hw reset (w/ dma scheduler)
		activate: toggle (active low)
		scope: PDMA + DMASCH + Tx/Rx FIFO
		PDMA:
			logic reset: Y
			register reset: Y
		DMASCH:
			logic reset: Y
			register reset: Y
	*/
	HIF_IO_READ32(pAd->hdev_ctrl, MT_CONN_HIF_RST, &value);
	value &= ~(CONN_HIF_LOGIC_RST_N | DMASHDL_ALL_RST_N);
	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_CONN_HIF_RST, value);
	HIF_IO_READ32(pAd->hdev_ctrl, MT_CONN_HIF_RST, &value);
	value |= (CONN_HIF_LOGIC_RST_N | DMASHDL_ALL_RST_N);
	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_CONN_HIF_RST, value);

	return TRUE;
}
static INT32 get_fw_sync_value(RTMP_ADAPTER *pAd)
{
	UINT32 value;

	RTMP_IO_READ32(pAd->hdev_ctrl, CONN_ON_MISC, &value);
	value = (value & 0x0000000E) >> 1;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: current sync CR = 0x%x\n", __func__, value));
	return value;
}

#ifdef CONFIG_FWOWN_SUPPORT
static VOID fw_own(RTMP_ADAPTER *pAd)
{
	/* Write 1 to MT_CONN_HIF_ON_LPCTL bit 0 to set FW own */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Write 1 to MT_CONN_HIF_ON_LPCTL bit 0 to set FW own\n", __func__));
	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_CONN_HIF_ON_LPCTL, MT_HOST_SET_OWN);
	pAd->bDrvOwn = FALSE;
}

static INT32 driver_own(RTMP_ADAPTER *pAd)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	UINT32 retrycnt = 0;
	UINT32 counter = 0;
	UINT32 Value;
#define MAX_RETRY_CNT 4

	do {
		retrycnt++;
		if (pAd->bDrvOwn == TRUE) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Return since already in Driver Own...\n", __func__));
			return Ret;
		}

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Try to Clear FW Own...\n", __func__));
		/* Write CR to get driver own */
		HIF_IO_WRITE32(pAd->hdev_ctrl, MT_CONN_HIF_ON_LPCTL, MT_HOST_CLR_OWN);
		/* Poll driver own status */
		counter = 0;
		while (counter < FW_OWN_POLLING_COUNTER) {
			RtmpusecDelay(1000);

			if (pAd->bDrvOwn == TRUE)
				break;
			counter++;
		};

		if (counter == FW_OWN_POLLING_COUNTER) {
			HIF_IO_READ32(pAd->hdev_ctrl, MT_CONN_HIF_ON_LPCTL, &Value);

			if (!(Value & MT_HOST_SET_OWN))
				pAd->bDrvOwn = TRUE;
		}

		if (pAd->bDrvOwn)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Success to clear FW Own\n", __func__));
		else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Fail to clear FW Own (%d)\n", __func__, counter));

			if (retrycnt >= MAX_RETRY_CNT)
				Ret = NDIS_STATUS_FAILURE;
		}
	} while (pAd->bDrvOwn == FALSE && retrycnt < MAX_RETRY_CNT);

	return Ret;
}
#endif

static VOID mtd_isr(RTMP_ADAPTER *pAd)
{
	UINT32 IntSource = 0x00000000L;
	POS_COOKIE pObj;
	PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct tm_ops *tm_ops = pAd->tm_hif_ops;
#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif
	unsigned long flags = 0;

	pObj = (POS_COOKIE)pAd->OS_Cookie;
	pci_hif->bPCIclkOff = FALSE;
	HIF_IO_READ32(pAd->hdev_ctrl, MT_INT_SOURCE_CSR, &IntSource);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_SOURCE_CSR, IntSource);
		return;
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
		UINT32 reg;

		/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
		reg = IntSource & (MT_INT_CMD | MT_INT_RX | MT_INT_RX_DLY |
					WF_MAC_INT_3 | MT_INT_RX_COHE);

		if (!reg) {
			HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_SOURCE_CSR, IntSource);
			return;
		}
	}

	/* Do nothing if NIC doesn't exist */
	if (IntSource == 0xffffffff) {
		RTMP_SET_FLAG(pAd, (fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS));
		HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_SOURCE_CSR, IntSource);
		return;
	}

	if (IntSource & MT_TxCoherent)
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, (">>>TxCoherent<<<\n"));

	if (IntSource & MT_RxCoherent)
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, (">>>RxCoherent<<<\n"));

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);

	if (IntSource & MT_INT_TX_DONE) {
		if ((pci_hif->intDisableMask & (IntSource & MT_INT_TX_DONE)) == 0)
			tm_ops->schedule_task(pAd, TX_DONE_TASK);

		pci_hif->IntPending |= (IntSource & MT_INT_TX_DONE);

#ifdef CONFIG_TP_DBG
		tp_dbg->IsrTxCnt++;
#endif
	}

	if (IntSource & MT_INT_RX_DATA) {
		if (!(IntSource & MT_INT_RX_DLY))
			IntSource &= ~MT_INT_RX_DATA;
		else
			 pci_hif->IntPending |= MT_INT_RX_DATA;
#ifdef CONFIG_TP_DBG
		tp_dbg->IsrRxCnt++;
#endif
	}

	if (IntSource & MT_INT_RX_CMD) {
		if (!(IntSource & MT_INT_RX_DLY))
			IntSource &= ~MT_INT_RX_CMD;
		else
			pci_hif->IntPending |= MT_INT_RX_CMD;
#ifdef CONFIG_TP_DBG
		tp_dbg->IsrRx1Cnt++;
#endif
	}

	if (IntSource & MT_INT_RX_DLY) {
		/* let kernel to check the task schedule or not, to prevent Rx Ring Full no tasklet scheduled */
		/* if ((pci_hif->intDisableMask & MT_INT_RX_DLY) == 0) */
			tm_ops->schedule_task(pAd, TR_DONE_TASK);

		pci_hif->IntPending |= MT_INT_RX_DLY;

#ifdef CONFIG_TP_DBG
		tp_dbg->IsrRxDlyCnt++;
#endif
	}

	if (IntSource & MT_INT_SUBSYS_INT_STS) {
		if ((pci_hif->intDisableMask & (IntSource & MT_INT_SUBSYS_INT_STS)) == 0)
			tm_ops->schedule_task(pAd, SUBSYS_INT_TASK);
	}

	if (IntSource & MT_INT_MCU2HOST_SW_INT_STS) {
		UINT32 value;

		RTMP_IO_READ32(pAd->hdev_ctrl, MT_MCU2HOST_SW_INT_STA, &value);
#ifdef ERR_RECOVERY
		if (value & MT7663_ERROR_DETECT_MASK) {
			/* updated ErrRecovery Status. */
			pAd->ErrRecoveryCtl.status = value;

			/* Clear MCU CMD status*/
			RTMP_IO_WRITE32(pAd->hdev_ctrl, MT_MCU2HOST_SW_INT_STA, MT7663_ERROR_DETECT_MASK);

			/* Trigger error recovery process with fw reload. */
			tm_ops->schedule_task(pAd, ERROR_RECOVERY_TASK);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R  , status=0x%08X\n", __func__, value));
			RTMPHandleInterruptSerDump(pAd);
		}
#endif /* ERR_RECOVERY */

#ifdef DYNAMIC_STEERING_LOADING
		/* use SW int to instead ipi */
		if (!pAd->disable_auto_rps) {
			RTMP_IO_READ32(pAd->hdev_ctrl, MT_MCU2HOST_SW_INT_STA, &value);
			if ((value & 0x80) == 0x80) {
				struct tm_ops *tm_ops2 = pAd->tm_qm_ops;

				RTMP_IO_WRITE32(pAd->hdev_ctrl, MT_MCU2HOST_SW_INT_STA, 0x80);
				tm_ops2->schedule_task(pAd, TX_COMBO_DEQ_TASK);
			}
		}
#endif /* DYNAMIC_STEERING_LOADING */
	}

	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_SOURCE_CSR, IntSource);
	mt_int_disable(pAd, IntSource);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}

#ifdef MT7626_E2_SUPPORT
/* only handle ring2,ring3 interrupt */
static VOID mtd_isr_md(RTMP_ADAPTER *pAd)
{
	UINT32 IntSource = 0x00000000L;
	POS_COOKIE pObj;
	PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct tm_ops *tm_ops = pAd->tm_hif_ops;
#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif
	unsigned long flags = 0;

	pObj = (POS_COOKIE)pAd->OS_Cookie;
	pci_hif->bPCIclkOff = FALSE;
	HIF_IO_READ32(pAd->hdev_ctrl, MT_INT2_SOURCE_CSR, &IntSource);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT2_SOURCE_CSR, IntSource);
		return;
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
		UINT32 reg;

		/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
		reg = IntSource & (MT_INT2_RX);

		if (!reg) {
			return;
		}
	}

	/* Do nothing if NIC doesn't exist */
	if (IntSource == 0xffffffff) {
		RTMP_SET_FLAG(pAd, (fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS));
		HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_SOURCE_CSR, IntSource);
		return;
	}

	RTMP_INT_LOCK(&pAd->LockInterrupt_md, flags);
	IntSource = IntSource & (MT_INT2_RX | MT_INT2_RX_DLY);

	if (IntSource & MT_INT2_RX_DATA) {
		if (!(IntSource & MT_INT2_RX_DLY))
			IntSource &= ~MT_INT2_RX_DATA;
		else
			pci_hif->IntPending_md |= MT_INT2_RX_DATA;
#ifdef CONFIG_TP_DBG
		tp_dbg->IsrRxCnt++;
#endif
	}

	if (IntSource & MT_INT2_RX_DLY) {
		if ((pci_hif->intDisableMask_md & MT_INT2_RX_DLY) == 0)
			tm_ops->schedule_task(pAd, RX2_DONE_TASK);

		pci_hif->IntPending_md |= MT_INT2_RX_DLY;

#ifdef CONFIG_TP_DBG
		tp_dbg->IsrRxDlyCnt++;
#endif
	}

	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT2_SOURCE_CSR, IntSource);
	mt_int_md_disable(pAd, IntSource);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt_md, flags);
}
#endif
static struct dly_ctl_cfg mt7626_rx_dly_ctl_ul_tbl[] = {
	{0, 0x811c},
	{600, 0xa01c},
	{800, 0xc01f},
};

static struct dly_ctl_cfg mt7626_rx_dly_ctl_dl_tbl[] = {
	{0, 0x811c},
};

/*
*	init sub-layer interrupt enable mask
*/
static VOID mt7626_irq_init(RTMP_ADAPTER *pAd)
{
#ifdef ERR_RECOVERY
	HW_IO_WRITE32(pAd->hdev_ctrl, MT_MCU2HOST_SW_INT_ENA, MT7663_ERROR_DETECT_MASK);
#endif

#ifdef DYNAMIC_STEERING_LOADING
	{
		UINT32 value;
		HW_IO_READ32(pAd->hdev_ctrl, MT_MCU2HOST_SW_INT_ENA, &value);
		HW_IO_WRITE32(pAd->hdev_ctrl, MT_MCU2HOST_SW_INT_ENA, 0x80 | value);
	}
#endif
}

#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT
static void mt7626_eeprom_info_extract(RTMP_ADAPTER *pAd, VOID *eeprom)
{
	UCHAR i;
	P_EFUSE_INFO_T prEfuseInfo = (P_EFUSE_INFO_T)eeprom;

#ifdef EEPROM_RETRIEVE_SUPPORT
#define EFUSE_VALID_BIT 		(1 << 7)
	UINT8 dump_content[0x20];
	/* Get CONNAC A-Die efuse content */
	MtCmdEfusBufferModeGet(pAd, EEPROM_EFUSE, 0x40, 0x20, dump_content);

	hex_dump("mt7626_eeprom_info_extract() buffer ==>", dump_content, 32);

	/* check 0x40 */
	if (dump_content[0x0] & EFUSE_VALID_BIT) {
		prEfuseInfo->arADieCal[0].ucFreqOffset = dump_content[0x0];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), merge offset 0x40 from %02x to %02x\n", __func__, prEfuseInfo->arADieCal[0].ucFreqOffset, dump_content[0x0]));
	}
	/* check 0x45 */
	if (dump_content[0x5] & EFUSE_VALID_BIT) {
		prEfuseInfo->arADieCal[0].ucRcalResult = dump_content[0x5];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), merge offset 0x45 from %02x to %02x\n", __func__, prEfuseInfo->arADieCal[0].ucRcalResult, dump_content[0x5]));
	}
	/* check 0x46 */
	if (dump_content[0x6] & EFUSE_VALID_BIT) {
		prEfuseInfo->arADieCal[0].THADCAnalogPart = dump_content[0x6];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), merge offset 0x46 from %02x to %02x\n", __func__, prEfuseInfo->arADieCal[0].THADCAnalogPart, dump_content[0x6]));
	}
	/* check 0x47 */
	if (dump_content[0x7] & EFUSE_VALID_BIT) {
		prEfuseInfo->arADieCal[0].THADCSlop = dump_content[0x7];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), merge offset 0x47 from %02x to %02x\n", __func__, prEfuseInfo->arADieCal[0].THADCSlop, dump_content[0x7]));
	}
	/* check 0x48 */
	if (dump_content[0x8] & EFUSE_VALID_BIT) {
		prEfuseInfo->arADieCal[0].TempSensorCal = dump_content[0x8];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), merge offset 0x48 from %02x to %02x\n", __func__, prEfuseInfo->arADieCal[0].TempSensorCal, dump_content[0x8]));
	}
       /* check 0x4F */
	if (dump_content[0xf]) {
		prEfuseInfo->arADieCal[0].Bin = dump_content[0xf];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), merge offset 0x4f from %02x to %02x\n", __func__, prEfuseInfo->arADieCal[0].Bin, dump_content[0xf]));
	}

	/* check 0x50 */
	if (dump_content[0x10] & EFUSE_VALID_BIT) {
		prEfuseInfo->arADieCal[1].ucFreqOffset = dump_content[0x10];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), merge offset 0x50 from %02x to %02x\n", __func__, prEfuseInfo->arADieCal[1].ucFreqOffset, dump_content[0x10]));
	}
	/* check 0x55 */
	if (dump_content[0x15] & EFUSE_VALID_BIT) {
		prEfuseInfo->arADieCal[1].ucRcalResult = dump_content[0x15];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), merge offset 0x55 from %02x to %02x\n", __func__, prEfuseInfo->arADieCal[1].ucRcalResult, dump_content[0x15]));
	}
	/* check 0x56 */
	if (dump_content[0x16] & EFUSE_VALID_BIT) {
		prEfuseInfo->arADieCal[1].THADCAnalogPart = dump_content[0x16];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), merge offset 0x56 from %02x to %02x\n", __func__, prEfuseInfo->arADieCal[1].THADCAnalogPart, dump_content[0x16]));
	}
	/* check 0x57 */
	if (dump_content[0x17] & EFUSE_VALID_BIT) {
		prEfuseInfo->arADieCal[1].THADCSlop = dump_content[0x17];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), merge offset 0x57 from %02x to %02x\n", __func__, prEfuseInfo->arADieCal[1].THADCSlop, dump_content[0x17]));
	}
	/* check 0x58 */
	if (dump_content[0x18] & EFUSE_VALID_BIT) {
		prEfuseInfo->arADieCal[1].TempSensorCal = dump_content[0x18];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), merge offset 0x58 from %02x to %02x\n", __func__, prEfuseInfo->arADieCal[1].TempSensorCal, dump_content[0x18]));
	}
       /* check 0x5F */
	if (dump_content[0x1F]) {
		prEfuseInfo->arADieCal[1].Bin = dump_content[0x1f];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), merge offset 0x5f from %02x to %02x\n", __func__, prEfuseInfo->arADieCal[1].Bin, dump_content[0x1f]));
	}

#endif /* EEPROM_RETRIEVE_SUPPORT */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s : sizeof(EFUSE_INFO_T)=%lu\n", __func__, sizeof(EFUSE_INFO_T)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), sizeof(EFUSE_INFO_MODULE_SYSTEM_T)=%lu\n", __func__, sizeof(EFUSE_INFO_MODULE_SYSTEM_T)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), sizeof(EFUSE_INFO_MODULE_TX_POWER_T)=%lu\n", __func__, sizeof(EFUSE_INFO_MODULE_TX_POWER_T)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), sizeof(EFUSE_INFO_MODULE_2G4_COMMON_T)=%lu\n", __func__, sizeof(EFUSE_INFO_MODULE_2G4_COMMON_T)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), sizeof(EFUSE_INFO_MODULE_2G4_WIFI_PATH_T)=%lu\n", __func__, sizeof(EFUSE_INFO_MODULE_2G4_WIFI_PATH_T)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), sizeof(EFUSE_INFO_MODULE_IBF_CAL_T)=%lu\n", __func__, sizeof(EFUSE_INFO_MODULE_IBF_CAL_T)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), sizeof(EFUSE_INFO_MODULE_5G_COMMON_T)=%lu\n", __func__, sizeof(EFUSE_INFO_MODULE_5G_COMMON_T)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), sizeof(EFUSE_INFO_MODULE_5G_WIFI_PATH_T)=%lu\n", __func__, sizeof(EFUSE_INFO_MODULE_5G_WIFI_PATH_T)));
	pAd->EfuseInfoAll.pr2G4Cmm = &prEfuseInfo->r2G4Cmm;
	pAd->EfuseInfoAll.pr2G4IBfParam = &prEfuseInfo->rIBfCal.r2G4IBfParam;
	pAd->EfuseInfoAll.pr2G4RxLatency = NULL;
	pAd->EfuseInfoAll.pr2G4TxPower = &prEfuseInfo->rTxPower.r2G4TxPower;
	pAd->EfuseInfoAll.pr5GCmm = &prEfuseInfo->r5GCmm;
	pAd->EfuseInfoAll.pr5GIBfParam = &prEfuseInfo->rIBfCal.r5GIBfParam;
	pAd->EfuseInfoAll.pr5GRxLatency = NULL;
	pAd->EfuseInfoAll.pr5GTxPower = &prEfuseInfo->rTxPower.r5GTxPower;
	pAd->EfuseInfoAll.prDelayCompCtrl = NULL;

#if (CONNAC_WIFI_TX_POWER_EFUSE_TYPE != EFUSE_INFO_TX_POWER_CUSTOMIZED_TYPE_1)
	pAd->EfuseInfoAll.prNTxPowerOffset = &prEfuseInfo->rTxPower.rNTxPowerOffset;
#endif
	pAd->EfuseInfoAll.prSys = &prEfuseInfo->rSys;

	for (i = 0; i < MAX_ANTENNA_NUM; i++) {
		pAd->EfuseInfoAll.pr2G4WFPath[i] = &prEfuseInfo->ar2G4WFPath[i];
		pAd->EfuseInfoAll.pr5GWFPath[i] = &prEfuseInfo->ar5GWFPath[i];
	}
}
#endif /* CONNAC_EFUSE_FORMAT_SUPPORT */

#ifdef WIFI_RAM_EMI_SUPPORT
static INT32 mt7626_parse_emi_phy_addr(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

#ifndef CONFIG_PROPRIETARY_DRIVER
#ifdef CONFIG_OF
#define OF_EMI_RESERVED_MEMORY_STR "mediatek,leopard-N9-reserved"

	struct device_node *node = NULL;
	UINT32 emi_phyaddr_info[4] = {0, 0, 0, 0};

	node = of_find_compatible_node(NULL, NULL, OF_EMI_RESERVED_MEMORY_STR);
	if (!node) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: can't found node of %s from dts\n", __func__, OF_EMI_RESERVED_MEMORY_STR));
		return NDIS_STATUS_FAILURE;
	}

	if (of_property_read_u32_array(node, "reg", emi_phyaddr_info, ARRAY_SIZE(emi_phyaddr_info))) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: can't get emi physical address from dts\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	if (pChipCap->emi_phy_addr != emi_phyaddr_info[1])
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("%s: default emi physical address is different from dts\n", __func__));
	pChipCap->emi_phy_addr = emi_phyaddr_info[1];

	if (pChipCap->emi_phy_addr_size != emi_phyaddr_info[3])
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("%s: default emi physical address size is different from dts\n", __func__));
	pChipCap->emi_phy_addr_size = emi_phyaddr_info[3];
#endif /* CONFIG_OF */
#endif /* CONFIG_PROPRIETARY_DRIVER */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: emi physical base: 0x%08x, size: 0x%08x\n",
				__func__, pChipCap->emi_phy_addr, pChipCap->emi_phy_addr_size));

	return NDIS_STATUS_SUCCESS;
}

static INT32 mt7626_fw_ram_emi_dl(RTMP_ADAPTER *pAd)
{
	volatile UINT8 __iomem *vir_addr = NULL, *target_vir_addr = NULL;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->emi_phy_addr == 0 || pChipCap->emi_phy_addr_size == 0) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: emi physical address or size is zero\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	vir_addr = ioremap_nocache(pChipCap->emi_phy_addr, pChipCap->emi_phy_addr_size);
	if (vir_addr != 0) {
		/* wifi ram ilm emi download */
		target_vir_addr = vir_addr + pChipCap->ram_ilm_emi_addr_offset;
		memmove(target_vir_addr, wifi_emi_ram_ilm, sizeof(wifi_emi_ram_ilm));

		/* wifi ram dlm emi download */
		target_vir_addr = vir_addr + pChipCap->ram_dlm_emi_addr_offset;
		memmove(target_vir_addr, wifi_emi_ram_dlm, sizeof(wifi_emi_ram_dlm));

		iounmap(vir_addr);

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wifi ram emi download successfully\n", __func__));
	} else {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: ioremap fail!\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* WIFI_RAM_EMI_SUPPORT */

#ifdef RF_LOCKDOWN
static BOOLEAN mt7626_check_RF_lock_down(RTMP_ADAPTER *pAd)
{
	BOOL RFlockDown;
	if (pAd->EEPROMImage[RF_LOCKDOWN_EEPROME_BLOCK_OFFSET] & RF_LOCKDOWN_EEPROME_BIT)
		RFlockDown = TRUE;
	else
		RFlockDown = FALSE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: \033[32m RFlockDown %s \033[0m \n", __func__, (RFlockDown)?"Enable":"Disable"));

	return RFlockDown;
}

static BOOLEAN mt7626_Config_Effuse_Country(RTMP_ADAPTER *pAd)
{
	UCHAR   Buffer0, Buffer1;
	UCHAR   CountryCode[2];

	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Country Region 2G */
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Buffer0 = pAd->EEPROMImage[COUNTRY_REGION_2G_EEPROME_OFFSET];

	/* Check the RF lock status */
	if (Buffer0 != 0xFF) {
		/* Check Validation bit for content */
		if (((Buffer0) & (COUNTRY_REGION_VALIDATION_MASK)) >> (COUNTRY_REGION_VALIDATION_OFFSET))
			pAd->CommonCfg.CountryRegion = ((Buffer0) & (COUNTRY_REGION_CONTENT_MASK));
	}

	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Country Region 5G */
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Buffer1 = pAd->EEPROMImage[COUNTRY_REGION_5G_EEPROME_OFFSET];

	/* Check the RF lock status */
	if (Buffer1 != 0xFF) {
		/* Check Validation bit for content */
		if (((Buffer1) & (COUNTRY_REGION_VALIDATION_MASK)) >> (COUNTRY_REGION_VALIDATION_OFFSET))
			pAd->CommonCfg.CountryRegionForABand = ((Buffer1) & (COUNTRY_REGION_CONTENT_MASK));
	}

	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Country Code */
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	CountryCode[0] = pAd->EEPROMImage[COUNTRY_CODE_BYTE0_EEPROME_OFFSET];
	CountryCode[1] = pAd->EEPROMImage[COUNTRY_CODE_BYTE1_EEPROME_OFFSET];

	/* Check the RF lock status */
	if ((CountryCode[0] != 0xFF) && (CountryCode[1] != 0xFF)) {
		/* Check Validation for content */
		if ((CountryCode[0] != 0x00) && (CountryCode[1] != 0x00)) {
			pAd->CommonCfg.CountryCode[0] = CountryCode[0];
			pAd->CommonCfg.CountryCode[1] = CountryCode[1];
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pAd->CommonCfg.CountryCode[0]: 0x%x, %c ",
					 pAd->CommonCfg.CountryCode[0], pAd->CommonCfg.CountryCode[0]));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pAd->CommonCfg.CountryCode[1]: 0x%x, %c ",
					 pAd->CommonCfg.CountryCode[1], pAd->CommonCfg.CountryCode[1]));
		}
	}

	return TRUE;
}
#endif /* RF_LOCKDOWN */


static RTMP_CHIP_OP MT7626_ChipOp = {0};
static RTMP_CHIP_CAP MT7626_ChipCap = {0};


static VOID mt7626_chipCap_init(void)
{
	MT7626_ChipCap.TXWISize = sizeof(TMAC_TXD_L); /* 32 */
	MT7626_ChipCap.RXWISize = 28;
	MT7626_ChipCap.tx_hw_hdr_len = MT7626_ChipCap.TXWISize;/* + sizeof(CR4_TXP_MSDU_INFO); */
	MT7626_ChipCap.rx_hw_hdr_len = MT7626_ChipCap.RXWISize;
	MT7626_ChipCap.num_of_tx_ring = MT7626_TX_RING_NUM;
	MT7626_ChipCap.num_of_rx_ring = MT7626_RX_RING_NUM;
#ifdef MT7626_E2_SUPPORT
	if (!mt7626_cap_md_support)
		MT7626_ChipCap.num_of_rx_ring = MT7626_RX_RING_NUM-2;
#endif
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	MT7626_ChipCap.max_v2_bcn_num = 8;
#endif
#ifdef MEMORY_SHRINK
	MT7626_ChipCap.tx_ring_size = 256;
	MT7626_ChipCap.rx0_ring_size = 256;
	MT7626_ChipCap.rx1_ring_size = 256;
#else
	MT7626_ChipCap.tx_ring_size = 1024;
	MT7626_ChipCap.rx0_ring_size = 512;
	MT7626_ChipCap.rx1_ring_size = 512;
#endif
#ifdef MT7626_E2_SUPPORT
	MT7626_ChipCap.rx2_ring_size = 512;
	MT7626_ChipCap.rx3_ring_size = 512;
#endif
	MT7626_ChipCap.asic_caps = (fASIC_CAP_PMF_ENC | fASIC_CAP_MCS_LUT
				    | fASIC_CAP_CT | fASIC_CAP_HW_DAMSDU);

#ifdef MT7626_E2_SUPPORT
	if (mt7626_cap_md_support == TRUE)
		MT7626_ChipCap.asic_caps |= fASIC_CAP_MD;
#endif
#ifdef DOT11_VHT_AC
	MT7626_ChipCap.mcs_nss.max_vht_mcs = VHT_MCS_CAP_9;
#ifdef G_BAND_256QAM
	MT7626_ChipCap.mcs_nss.g_band_256_qam = TRUE;
#endif
#endif /* DOT11_VHT_AC */

#ifdef RX_CUT_THROUGH
	MT7626_ChipCap.asic_caps |= fASIC_CAP_BA_OFFLOAD;
#endif
#ifdef HDR_TRANS_TX_SUPPORT
	MT7626_ChipCap.asic_caps |= fASIC_CAP_TX_HDR_TRANS;
#endif
#ifdef HDR_TRANS_RX_SUPPORT
	MT7626_ChipCap.asic_caps |= fASIC_CAP_RX_HDR_TRANS;
#endif
#ifdef RX_SCATTER
	MT7626_ChipCap.asic_caps |= fASIC_CAP_RX_DMA_SCATTER;
#endif

#ifdef DBDC_MODE
	MT7626_ChipCap.asic_caps |= fASIC_CAP_DBDC;
#endif /* DBDC_MODE */

	MT7626_ChipCap.asic_caps |= fASIC_CAP_RX_DLY;
	MT7626_ChipCap.phy_caps = (fPHY_CAP_24G | fPHY_CAP_5G | \
							   fPHY_CAP_HT | fPHY_CAP_VHT | \
							   fPHY_CAP_TXBF | fPHY_CAP_LDPC | fPHY_CAP_MUMIMO | \
							   fPHY_CAP_BW40 | fPHY_CAP_BW80 | fPHY_CAP_DUALPHY);

	MT7626_ChipCap.hw_ops_ver = HWCTRL_OP_TYPE_V2;
	MT7626_ChipCap.hif_type = HIF_MT;
	MT7626_ChipCap.mac_type = MAC_MT;
	MT7626_ChipCap.MCUType = ANDES;
	MT7626_ChipCap.rf_type = RF_MT;
	MT7626_ChipCap.pRFRegTable = NULL;
	MT7626_ChipCap.pBBPRegTable = NULL;
	MT7626_ChipCap.bbpRegTbSize = 0;
	MT7626_ChipCap.MaxNumOfRfId = MAX_RF_ID;
	MT7626_ChipCap.MaxNumOfBbpId = 200;
	MT7626_ChipCap.WtblHwNum = MT7626_MT_WTBL_SIZE;
	MT7626_ChipCap.FlgIsHwWapiSup = TRUE;
	MT7626_ChipCap.FlgIsHwAntennaDiversitySup = FALSE;
#ifdef STREAM_MODE_SUPPORT
	MT7626_ChipCap.FlgHwStreamMode = FALSE;
#endif
#ifdef TXBF_SUPPORT
	MT7626_ChipCap.FlgHwTxBfCap = TXBF_HW_CAP | TXBF_HW_2BF;
#endif
	MT7626_ChipCap.SnrFormula = SNR_FORMULA4;
	MT7626_ChipCap.mcs_nss.max_nss = 3;
	/* todo Ellis */
#ifdef RTMP_EFUSE_SUPPORT
	MT7626_ChipCap.EFUSE_USAGE_MAP_START = 0x3c0;
	MT7626_ChipCap.EFUSE_USAGE_MAP_END = 0x3fb;
	MT7626_ChipCap.EFUSE_USAGE_MAP_SIZE = 60;
	MT7626_ChipCap.EFUSE_RESERVED_SIZE = 59;	/* Cal-Free is 22 free block */
#endif
	MT7626_ChipCap.EEPROM_DEFAULT_BIN = MT7626_E2PImage;
	MT7626_ChipCap.EEPROM_DEFAULT_BIN_SIZE = sizeof(MT7626_E2PImage);
	MT7626_ChipCap.EFUSE_BUFFER_CONTENT_SIZE = 1024; /* 0x0 ~ 0x3FF */
#ifdef CARRIER_DETECTION_SUPPORT
	MT7626_ChipCap.carrier_func = TONE_RADAR_V2;
#endif
#ifdef RTMP_MAC_PCI
	MT7626_ChipCap.WPDMABurstSIZE = 3;
#endif
	MT7626_ChipCap.ProbeRspTimes = 2;
#ifdef NEW_MBSSID_MODE
#ifdef ENHANCE_NEW_MBSSID_MODE
	MT7626_ChipCap.MBSSIDMode = MBSSID_MODE4;
#else
	MT7626_ChipCap.MBSSIDMode = MBSSID_MODE1;
#endif /* ENHANCE_NEW_MBSSID_MODE */
#else
	MT7626_ChipCap.MBSSIDMode = MBSSID_MODE0;
#endif /* NEW_MBSSID_MODE */
#ifdef DOT11W_PMF_SUPPORT
	/* sync with Ellis, wilsonl */
	MT7626_ChipCap.FlgPMFEncrtptMode = PMF_ENCRYPT_MODE_2;
#endif /* DOT11W_PMF_SUPPORT */
#ifdef CONFIG_ANDES_SUPPORT
#ifdef NEED_ROM_PATCH
	MT7626_ChipCap.need_load_patch = BIT(WM_CPU);
#else
	MT7626_ChipCap.need_load_patch = 0;
#endif
	MT7626_ChipCap.need_load_fw = BIT(WM_CPU);
	MT7626_ChipCap.load_patch_flow = PATCH_FLOW_V1;
	MT7626_ChipCap.load_fw_flow = FW_FLOW_V1; /*FW_FLOW_V2_COMPRESS_IMG;*/
	MT7626_ChipCap.patch_format = PATCH_FORMAT_V1;
	MT7626_ChipCap.fw_format = FW_FORMAT_V3;
	MT7626_ChipCap.load_patch_method = BIT(HEADER_METHOD);
	MT7626_ChipCap.load_fw_method = BIT(HEADER_METHOD);
	MT7626_ChipCap.rom_patch_offset = MT7626_ROM_PATCH_START_ADDRESS;
	MT7626_ChipCap.decompress_tmp_addr = 0x203b000UL;
#endif
#ifdef UNIFY_FW_CMD /* todo wilsonl */
	MT7626_ChipCap.cmd_header_len = sizeof(FW_TXD) + sizeof(TMAC_TXD_L);
#else
	MT7626_ChipCap.cmd_header_len = 12; /* sizeof(FW_TXD) */
#endif
	MT7626_ChipCap.cmd_padding_len = 0;
	MT7626_ChipCap.ppdu.TxAggLimit = 64;
	MT7626_ChipCap.ppdu.RxBAWinSize = 64;
	MT7626_ChipCap.ppdu.ht_max_ampdu_len_exp = 3;
#ifdef DOT11_VHT_AC
	MT7626_ChipCap.ppdu.max_mpdu_len = MPDU_7991_OCTETS;
	MT7626_ChipCap.ppdu.vht_max_ampdu_len_exp = 7;
#endif /* DOT11_VHT_AC */
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	/* enabled later, wilsonl */
	MT7626_ChipCap.fgRateAdaptFWOffload = TRUE;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	MT7626_ChipCap.qos.WmmHwNum = MT7626_MT_WMM_SIZE; /* for multi-wmm */
	MT7626_ChipCap.PDA_PORT = MT7626_PDA_PORT;
	MT7626_ChipCap.ppdu.SupportAMSDU = TRUE;
	/* sync with Pat, wilsonl */
	MT7626_ChipCap.APPSMode = APPS_MODE2;
	MT7626_ChipCap.CtParseLen = MT7626_CT_PARSE_LEN;
	MT7626_ChipCap.qm = GENERIC_QM;
	MT7626_ChipCap.qm_tm = TASKLET_METHOD;
	MT7626_ChipCap.hif_tm = TASKLET_METHOD;
	MT7626_ChipCap.qos.wmm_detect_method = WMM_DETECT_METHOD1;
	/* for interrupt enable mask */

	if ((MT7626_ChipCap.asic_caps & fASIC_CAP_RX_DLY) != 0) {
		MT7626_ChipCap.int_enable_mask = MT_CoherentInt | MT_MacInt | MT_INT_RX_DLY |
						MT_INT_T3_DONE | MT_INT_T15_DONE | MT_FW_CLR_OWN_INT;


#ifdef MT7626_E2_SUPPORT
		if ((MT7626_ChipCap.asic_caps & fASIC_CAP_MD) != 0) {
			MT7626_ChipCap.int_md_enable_mask = (1 << 9);
		}
#endif
	} else {
		MT7626_ChipCap.int_enable_mask = MT_CoherentInt | MT_RxINT | MT_MacInt | MT_RxINT | MT_INT_T0_DONE |
						MT_INT_T1_DONE | MT_INT_T2_DONE | MT_INT_T3_DONE |
						MT_INT_T4_DONE | MT_INT_T5_DONE | MT_INT_T15_DONE |
						MT_FW_CLR_OWN_INT;
#ifdef DBDC_TXRING_SUPPORT
		MT7626_ChipCap.int_enable_mask |= MT_CoherentInt | MT_MacInt | MT_INT_RX_DLY |
								 MT_INT_T3_DONE | MT_INT_T15_DONE | MT_FW_CLR_OWN_INT|
								 MT_INT_T6_DONE | MT_INT_T7_DONE | MT_INT_T8_DONE |
								 MT_INT_T10_DONE;
#endif /*DBDC_TXRING_SUPPORT*/

#ifdef MT7626_E2_SUPPORT
		if ((MT7626_ChipCap.asic_caps & fASIC_CAP_MD) != 0) {
			MT7626_ChipCap.int_md_enable_mask = (1 << 0);
		}
#endif
	}

	MT7626_ChipCap.hif_pkt_type[HIF_TX_IDX0] = TX_DATA;
	MT7626_ChipCap.hif_pkt_type[HIF_TX_IDX1] = TX_DATA;
	MT7626_ChipCap.hif_pkt_type[HIF_TX_IDX2] = TX_DATA;
	MT7626_ChipCap.hif_pkt_type[HIF_TX_IDX3] = TX_FW_DL;
	MT7626_ChipCap.hif_pkt_type[HIF_TX_IDX4] = TX_DATA;
	MT7626_ChipCap.hif_pkt_type[HIF_TX_IDX5] = TX_ALTX;
	MT7626_ChipCap.hif_pkt_type[HIF_TX_IDX15] = TX_CMD;

	MT7626_ChipCap.hif_pci_ring_layout.tx_ring_layout = mt7626_tx_ring_layout;
	MT7626_ChipCap.hif_pci_ring_layout.rx_ring_layout = mt7626_rx_ring_layout;
#if defined(ERR_RECOVERY) || defined(CONFIG_FWOWN_SUPPORT)
	MT7626_ChipCap.int_enable_mask |= MT_INT_MCU2HOST_SW_INT_STS;
#endif /* ERR_RECOVERY || CONFIG_FWOWN_SUPPORT */
#ifdef DYNAMIC_STEERING_LOADING
	MT7626_ChipCap.int_enable_mask |= MT_INT_MCU2HOST_SW_INT_STS;
#endif
	MT7626_ChipCap.band_cnt = 2;
#ifdef HW_TX_AMSDU_SUPPORT
	MT7626_ChipCap.asic_caps |= fASIC_CAP_HW_TX_AMSDU;
#endif /* HW_TX_AMSDU_SUPPORT */


#ifdef HW_TX_AMSDU_SUPPORT
	/* doesn't support SW AMSDU on GE QM v2*/
	if (MT7626_ChipCap.asic_caps & fASIC_CAP_HW_TX_AMSDU)
		MT7626_ChipCap.qm_version = QM_V2;
	else
#endif /* HW_TX_AMSDU_SUPPORT */
		MT7626_ChipCap.qm_version = QM_V1;
#if (defined(VOW_SUPPORT) && defined(VOW_DVT)) || defined(RANDOM_PKT_GEN)
	/* For HW function DVT, GE QM v2 need to develop furthermore, using QM v1 directly*/
	MT7626_ChipCap.qm_version = QM_V1;
#endif


#ifdef INTERNAL_CAPTURE_SUPPORT
	MT7626_ChipCap.ICapADCIQCnt = MT7626_ICAP_TWO_WAY_ADC_IQ_DATA_CNT;
	MT7626_ChipCap.ICapIQCIQCnt = MT7626_ICAP_THREE_WAY_IQC_IQ_DATA_CNT;
	MT7626_ChipCap.ICapWF01PackedADC = MT7626_CAP_WF01_PACKED_ADC;
	MT7626_ChipCap.ICapWF12PackedADC = MT7626_CAP_WF12_PACKED_ADC;
	MT7626_ChipCap.ICapWF02PackedADC = MT7626_CAP_WF02_PACKED_ADC;
	MT7626_ChipCap.ICapMaxIQCnt = MT7626_ICAP_TWO_WAY_ADC_IQ_DATA_CNT;
#endif /* INTERNAL_CAPTURE_SUPPORT */
	MT7626_ChipCap.txd_type = TXD_V2;
	MT7626_ChipCap.tx_delay_support = FALSE;
	MT7626_ChipCap.tx_delay_mode = TX_DELAY_HW_MODE;
}

#ifdef DBDC_MODE
static UCHAR MT7626BandGetByIdx(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	switch (BandIdx) {
	case 0:
		return RFIC_24GHZ;
		break;

	case 1:
		return RFIC_5GHZ;
		break;

	default:
		return RFIC_DUAL_BAND;
	}
}
#endif
static VOID mt7626_chipOp_init(void)
{
	MT7626_ChipOp.AsicRfInit = mt7626_init_rf_cr;
	MT7626_ChipOp.AsicBbpInit = MT7626BBPInit;
	MT7626_ChipOp.AsicMacInit = mt7626_init_mac_cr;
	MT7626_ChipOp.AsicReverseRfFromSleepMode = NULL;
	MT7626_ChipOp.AsicHaltAction = NULL;
	/* BBP adjust */
	MT7626_ChipOp.ChipBBPAdjust = mt7626_bbp_adjust;
	/* AGC */
	MT7626_ChipOp.ChipSwitchChannel = mt7626_switch_channel;

#ifdef SMART_CARRIER_SENSE_SUPPORT
#ifdef SCS_FW_OFFLOAD
	MT7626_ChipOp.SmartCarrierSense = SmartCarrierSense_Gen5;
#else
	MT7626_ChipOp.SmartCarrierSense = SmartCarrierSense_Gen4;
#endif /* SCS_FW_OFFLOAD */
	MT7626_ChipOp.ChipSetSCS = SetSCS;
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#ifdef NEW_SET_RX_STREAM
	MT7626_ChipOp.ChipSetRxStream = mt7626_set_RxStream;
#endif
	MT7626_ChipOp.AsicAntennaDefaultReset = mt7626_antenna_default_reset;
#ifdef CONFIG_STA_SUPPORT
	MT7626_ChipOp.NetDevNickNameInit = mt7626_init_dev_nick_name;
#endif
	MT7626_ChipOp.get_bin_image_file = mt7626_get_default_bin_image_file;
#ifdef CAL_FREE_IC_SUPPORT
	/* do not need, turn off compile flag, wilsonl */
	MT7626_ChipOp.is_cal_free_ic = mt7626_is_cal_free_ic;
	MT7626_ChipOp.cal_free_data_get = mt7626_cal_free_data_get;
#endif /* CAL_FREE_IC_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
	MT7626_ChipOp.ToneRadarProgram = ToneRadarProgram_v2;
#endif
	MT7626_ChipOp.DisableTxRx = NULL; /* 302 */
#ifdef RTMP_PCI_SUPPORT
	/* sync with Pat, Hammin, wilsonl */
	/* MT7626_ChipOp.AsicRadioOn = RT28xxPciAsicRadioOn; */
	/* MT7626_ChipOp.AsicRadioOff = RT28xxPciAsicRadioOff; */
#endif
#ifdef RF_LOCKDOWN
	MT7626_ChipOp.check_RF_lock_down = mt7626_check_RF_lock_down;
	MT7626_ChipOp.Config_Effuse_Country = mt7626_Config_Effuse_Country;
#endif /* RF_LOCKDOWN */
#ifdef MT_WOW_SUPPORT
	/* do not need, turn off compile flag, wilsonl */
	MT7626_ChipOp.AsicWOWEnable = MT76xxAndesWOWEnable;
	MT7626_ChipOp.AsicWOWDisable = MT76xxAndesWOWDisable;
	/* MT7626_ChipOp.AsicWOWInit = MT76xxAndesWOWInit, */
#endif /* MT_WOW_SUPPORT */
	MT7626_ChipOp.show_pwr_info = NULL;
	MT7626_ChipOp.bufferModeEfuseFill = mt7626_bufferModeEfuseFill;
	MT7626_ChipOp.MtCmdTx = MtCmdSendMsg;
	MT7626_ChipOp.prepare_fwdl_img = mt7626_fw_prepare;
	MT7626_ChipOp.fwdl_datapath_setup = mt7626_fwdl_datapath_setup;
#ifdef DBDC_MODE
	MT7626_ChipOp.BandGetByIdx = MT7626BandGetByIdx;
#endif
#ifdef GREENAP_SUPPORT
	MT7626_ChipOp.EnableAPMIMOPS = enable_greenap;
	MT7626_ChipOp.DisableAPMIMOPS = disable_greenap;
#endif /* GREENAP_SUPPORT */
#ifdef TXBF_SUPPORT
	MT7626_ChipOp.TxBFInit                 = mt_WrapTxBFInit;
	MT7626_ChipOp.ClientSupportsETxBF      = mt_WrapClientSupportsETxBF;
	MT7626_ChipOp.iBFPhaseComp             = mt_iBFPhaseComp;
	MT7626_ChipOp.iBFPhaseCalInit          = mt_iBFPhaseCalInit;
	MT7626_ChipOp.iBFPhaseFreeMem          = mt_iBFPhaseFreeMem;
	MT7626_ChipOp.iBFPhaseCalE2PUpdate     = mt_iBFPhaseCalE2PUpdate;
	MT7626_ChipOp.iBFPhaseCalReport        = mt_iBFPhaseCalReport;
	MT7626_ChipOp.iBfCaleBfPfmuMemAlloc    = mt7626_eBFPfmuMemAlloc;
	MT7626_ChipOp.iBfCaliBfPfmuMemAlloc    = mt7626_iBFPfmuMemAlloc;
	MT7626_ChipOp.setETxBFCap              = mt7626_setETxBFCap;
	MT7626_ChipOp.BfStaRecUpdate           = mt_AsicBfStaRecUpdate;
	MT7626_ChipOp.BfStaRecRelease          = mt_AsicBfStaRecRelease;
	MT7626_ChipOp.BfPfmuMemAlloc           = CmdPfmuMemAlloc;
	MT7626_ChipOp.BfPfmuMemRelease         = CmdPfmuMemRelease;
	MT7626_ChipOp.TxBfTxApplyCtrl          = CmdTxBfTxApplyCtrl;
	MT7626_ChipOp.BfApClientCluster        = CmdTxBfApClientCluster;
	MT7626_ChipOp.BfHwEnStatusUpdate       = CmdTxBfHwEnableStatusUpdate;
	MT7626_ChipOp.BfModuleEnCtrl           = CmdTxBfModuleEnCtrl;
	MT7626_ChipOp.BfeeHwCtrl               = NULL;
	MT7626_ChipOp.bfee_adaption            = NULL;
#ifdef CONFIG_STA_SUPPORT
	MT7626_ChipOp.archSetAid               = MtAsicSetAid;
#endif /* CONFIG_STA_SUPPORT */
#ifdef VHT_TXBF_SUPPORT
	MT7626_ChipOp.ClientSupportsVhtETxBF     = mt_WrapClientSupportsVhtETxBF;
	MT7626_ChipOp.setVHTETxBFCap             = mt7626_setVHTETxBFCap;
#endif /* VHT_TXBF_SUPPORT */
#endif /* TXBF_SUPPORT */
#ifdef INTERNAL_CAPTURE_SUPPORT
	MT7626_ChipOp.ICapStart = MtCmdRfTestICapStart;
	MT7626_ChipOp.ICapStatus = MtCmdRfTestGen2ICapStatus;
	MT7626_ChipOp.ICapCmdRawDataProc = MtCmdRfTestICapRawDataProc;
	MT7626_ChipOp.ICapGetIQData = Get_RBIST_IQ_Data;
	MT7626_ChipOp.ICapEventRawDataHandler = ExtEventICapIQDataHandler;
#endif /* INTERNAL_CAPTURE_SUPPORT */
#ifdef WIFI_SPECTRUM_SUPPORT
	MT7626_ChipOp.SpectrumStart = MtCmdWifiSpectrumStart;
	MT7626_ChipOp.SpectrumStatus = MtCmdWifiSpectrumGen2Status;
	MT7626_ChipOp.SpectrumCmdRawDataProc = MtCmdWifiSpectrumRawDataProc;
	MT7626_ChipOp.SpectrumEventRawDataHandler = ExtEventWifiSpectrumRawDataHandler;
#endif /* WIFI_SPECTRUM_SUPPORT */
	MT7626_ChipOp.dma_shdl_init = mt7626_dma_shdl_init;
#ifdef MT7626_FPGA
	MT7626_ChipOp.chk_hif_default_cr_setting = mt7626_chk_hif_default_cr_setting;
	MT7626_ChipOp.chk_top_default_cr_setting = mt7626_chk_top_default_cr_setting;
#endif
#ifdef CONFIG_ATE
	MT7626_ChipOp.set_ampdu_wtbl = mt_ate_wtbl_cfg_v2;
#endif /* CONFIG_ATE */
	MT7626_ChipOp.irq_init = mt7626_irq_init;
	MT7626_ChipOp.hif_init_dma = hif_init_WPDMA;
	MT7626_ChipOp.hif_set_dma = hif_set_WPDMA;
	MT7626_ChipOp.hif_wait_dma_idle = hif_wait_WPDMA_idle;
	MT7626_ChipOp.hif_reset_dma = hif_reset_WPDMA;
	MT7626_ChipOp.get_fw_sync_value = get_fw_sync_value;
	MT7626_ChipOp.read_chl_pwr = NULL;
	MT7626_ChipOp.parse_RXV_packet = parse_RXV_packet_v2;
	MT7626_ChipOp.txs_handler = txs_handler_v2;
#ifdef CONFIG_FWOWN_SUPPORT
	MT7626_ChipOp.driver_own = driver_own;
	MT7626_ChipOp.fw_own = fw_own;
#endif
	MT7626_ChipOp.hw_isr = mtd_isr;
#ifdef MT7626_E2_SUPPORT
	MT7626_ChipOp.hw_isr_md = mtd_isr_md;
#endif

#ifdef CONNAC_EFUSE_FORMAT_SUPPORT
	MT7626_ChipOp.eeprom_extract = mt7626_eeprom_info_extract;
#endif /* CONNAC_EFUSE_FORMAT_SUPPORT */
#ifdef WIFI_RAM_EMI_SUPPORT
	MT7626_ChipOp.parse_emi_phy_addr = mt7626_parse_emi_phy_addr;
	MT7626_ChipOp.fw_ram_emi_dl = mt7626_fw_ram_emi_dl;
#endif /* WIFI_RAM_EMI_SUPPORT */
#ifdef BACKGROUND_SCAN_SUPPORT
	MT7626_ChipOp.set_off_ch_scan = mt_off_ch_scan;
#endif
}

#ifdef DYNAMIC_STEERING_LOADING
INT mt7626_combo_tx_rps_check(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket, struct wifi_dev *wdev)
{
#define MT_MCU2HOST_SW_INT_SET	(MT_HIF_BASE + 0x010c)
	if (!wdev)
		return 0;
	RTMP_SET_PACKET_WDEV(pPacket, wdev->wdev_idx);
	ge_combo_enq_tx_dataq_pkt(pAd, pPacket);
	if (pAd->First_combo_tx_cpu == pAd->rx_intr_cpu) {
		/* set sw int to trigger TX_COMBO_DEQ_TASK */
		RTMP_IO_WRITE32(pAd->hdev_ctrl, MT_MCU2HOST_SW_INT_SET, 0x80);
	} else {
		struct tm_ops *tm_ops = pAd->tm_qm_ops;

		tm_ops->schedule_task(pAd, TX_COMBO_DEQ_TASK);
	}
	return 1;
}

/* tx peak mode,dynamic tunning accroding to TxRpsRatiotbl[0][] */
/* rx peak mode,dynamic tunning accroding to TxRpsRatiotbl[1][] */
/* bi-di peak mode,dynamic tunning accroding to TxRpsRatiotbl[2][] */
#define MAX_RATIO 20
#define MAX_COMBO_TX_CNT 1024 /* maximum packet cnt for each tasklet schedule*/
#define MAX_COMBO_2ND_TX_CNT 512 /* maximum packet cnt for each tasklet schedule*/
#define MAX_COMBO_TX_SWITCH_THRESHOLD 128

static UINT32 TxRpsRatiotbl[3][MAX_RATIO+1] = {
	/* tr ratio, for tx traffic */
/*	0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20*/
	0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7,
	/* rt ratio, for tx traffic*/
/*	0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20*/
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* rt ratio, for bi-di traffic*/
/*	0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20*/
	0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 5, 5, 5, 6, 6, 6, 6,

};

static UINT32 TxRpsRatiotbl_E1[3][MAX_RATIO+1] = {
	/* tr ratio, for tx traffic */
/*	0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20*/
	0, 5, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	/* rt ratio, for tx traffic*/
/*	0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20*/
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* rt ratio, for bi-di traffic*/
/*	0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20*/
	0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6,

};

#ifdef MT7626_E2_SUPPORT
static UINT32 TxRpsRatiotbl_E2[3][MAX_RATIO+1] = {
	/* tr ratio, for tx traffic */
/*	0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20*/
	0, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	/* rt ratio, for tx traffic*/
/*	0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20*/
	0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* rt ratio, for bi-di traffic*/
/*	0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20*/
	0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,

};
#endif

static int hit_rx_cpu_bound_cnt = 100000;
static int hit_tx_cpu_bound_cnt = 30000;
static int hit_tcp_ul_rx_cpu_condition = 15000;
static int hit_tcp_ul_tx_cpu_condition = 7000;
static int hit_tcp_dl_rx_cpu_condition = 100;
static int hit_tcp_dl_tx_cpu_condition = 15000;
static int hit_rx_cpu_low_bound_cnt = 20000; /* 245M */
static int hit_tx_cpu_low_bound_cnt = 14000; /* 172M */
#define CONN_TX_TP	800	/*800M*/
#define CONN_RX_TP	800 /*800M*/
#define CONN_BIDI_TP	150 /*150M*/
#define CONN_LOW_TP		50 /*50M*/
#define MIN_CNT_FOR_RATIO_MEASUREMENT 5000

#define SINGLE_5G_TP	850	/*850M*/
#define SINGLE_2G_TP	400	/*400M*/
#define SINGLE_5G_BIDI_TP	600 /*600M*/
#define SINGLE_2G_BIDI_TP	250 /*250M*/

/* 5G peak tx delta     tx/rx=078781/004620 */
/* 5G peak rx delta     tx/rx=011157/080176 */
#define SINGLE_5G_PEAK_TP_CNT	75000
#define SINGLE_2G_PEAK_TP_CNT	27000
#define DELTA_CONCURRENT_PEAK_TP_CNT	10000
/* 5G one pair tx delta     tx/rx=043044/002802 */
/* 5G one pair rx delta     tx/rx=005390/042904 */
#define SINGLE_5G_ONE_PAIR_TP_CNT	38000
/* 2G one pair tx delta     tx/rx=014234/002163 */
/* 2G one pair rx delta     tx/rx=002163/014234 */
#define SINGLE_2G_ONE_PAIR_TP_CNT	13000
/* 5G one pair bidi delta     tx/rx=038426/021913 */
VOID mt7626_combo_tx_rps_algo(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct peak_tp_ctl *peak_tp_ctl = NULL;
	UCHAR band_idx = 0;
	INT total_tx_tp = 0;
	INT total_rx_tp = 0;
	UINT32 total_tx_cnt = 0;
	UINT32 total_rx_cnt = 0;
	UINT32 pre_total_tx_cnt;
	UINT32 pre_total_rx_cnt;
	UINT32 changed = 0;
	static UINT32 tr_ratio_stable;
	static UINT32 rt_ratio_stable;
	UINT32 one_pair_running = 0;
	UINT32 has_tp_flag = 0;
	static UINT32 first_combo_buf_max_cnt_bk = MAX_COMBO_TX_CNT;
	static UINT32 second_combo_buf_max_cnt_bk = MAX_COMBO_2ND_TX_CNT;

	UINT32 concurrent_tput_flag = 0;
	UINT32 single_5g_tput_flag = 0;
	UINT32 single_2g_tput_flag = 0;
	UINT32 single_5g_one_pair_tput_flag = 0;
	UINT32 single_2g_one_pair_tput_flag = 0;
	UINT32 bidi_tput_flag = 0;
	UINT32 tx_tput_flag = 0;
	UINT32 rx_tput_flag = 0;
	UINT32 cpu_bound_flag = 0;

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		peak_tp_ctl = &pAd->peak_tp_ctl[band_idx];

		if (peak_tp_ctl->client_nums == 0) {
			pAd->con_tx_tp_running = 0;
			pAd->con_rx_tp_running = 0;
			continue;
		}
		if (band_idx == 0) {
			pAd->tp_2g_tx = peak_tp_ctl->main_tx_tp;
			pAd->tp_2g_rx = peak_tp_ctl->main_rx_tp;
		}
		if (band_idx == 1) {
			pAd->tp_5g_tx = peak_tp_ctl->main_tx_tp;
			pAd->tp_5g_rx = peak_tp_ctl->main_rx_tp;
		}
		total_tx_tp += peak_tp_ctl->main_tx_tp;
		total_rx_tp += peak_tp_ctl->main_rx_tp;
	}
	/* check tput direction, tx or rx or bidi*/
	if (total_tx_tp >= total_rx_tp) {
		if (total_tx_tp) {
			has_tp_flag = 1;
			tx_tput_flag = 1;
			if (total_rx_tp && ((total_tx_tp / total_rx_tp) <= 3))
				bidi_tput_flag = 1;
		}
	}
	if (total_tx_tp <= total_rx_tp) {
		if (total_rx_tp) {
			has_tp_flag = 1;
			rx_tput_flag = 1;
			if (total_tx_tp && ((total_rx_tp / total_tx_tp) <= 3))
				bidi_tput_flag = 1;
		}
	}

	/* calculate tr,rt ratio */
	total_tx_cnt = pAd->First_combo_cnt_core0 + pAd->First_combo_cnt_core1;
	total_rx_cnt = pAd->First_combo_rx_cnt_core0 + pAd->First_combo_rx_cnt_core1;
	pre_total_tx_cnt = pAd->First_combo_cnt_core0_old + pAd->First_combo_cnt_core1_old;
	pre_total_rx_cnt = pAd->First_combo_rx_cnt_core0_old + pAd->First_combo_rx_cnt_core1_old;

	/* total packet count overflow, ignore this time and reset all packet to 0 */
	if (total_tx_cnt < pre_total_tx_cnt) {
		/* cnt overflow,reset to 0*/
		pAd->First_combo_cnt_core0 = pAd->First_combo_cnt_core1 = 0;
		pAd->Second_combo_cnt_core0 = pAd->Second_combo_cnt_core1 = 0;
		pAd->First_combo_cnt_core0_old = pAd->First_combo_cnt_core1_old = 0;
		return;
	}
	if (total_rx_cnt < pre_total_rx_cnt) {
		/* cnt overflow,reset to 0*/
		pAd->First_combo_rx_cnt_core0 = pAd->First_combo_rx_cnt_core1 = 0;
		pAd->First_combo_rx_cnt_core0_old = pAd->First_combo_rx_cnt_core1_old = 0;
		return;
	}

	pAd->First_combo_cnt_core0_old = pAd->First_combo_cnt_core0;
	pAd->First_combo_cnt_core1_old = pAd->First_combo_cnt_core1;
	pAd->First_combo_rx_cnt_core0_old = pAd->First_combo_rx_cnt_core0;
	pAd->First_combo_rx_cnt_core1_old = pAd->First_combo_rx_cnt_core1;

	pAd->delta_tx_cnt = (total_tx_cnt - pre_total_tx_cnt) & 0xfffffff;
	pAd->delta_tx_cnt_avg += pAd->delta_tx_cnt;
	pAd->delta_tx_cnt_avg = pAd->delta_tx_cnt_avg >> 1;

	pAd->delta_rx_cnt = (total_rx_cnt - pre_total_rx_cnt) & 0xfffffff;
	pAd->delta_rx_cnt_avg += pAd->delta_rx_cnt;
	pAd->delta_rx_cnt_avg = pAd->delta_rx_cnt_avg >> 1;

	/* calculate tr ratrio */
	if (pAd->delta_tx_cnt > MIN_CNT_FOR_RATIO_MEASUREMENT) {
		if (pAd->delta_tx_cnt > pAd->delta_rx_cnt) {
			pAd->tr_ratio = pAd->delta_tx_cnt*10 / ((pAd->delta_rx_cnt == 0) ? 1 : pAd->delta_rx_cnt);
			if ((pAd->tr_ratio % 10) >= 5) {/*round off*/
				pAd->tr_ratio = (pAd->tr_ratio/10) + 1;
			} else {
				pAd->tr_ratio = (pAd->tr_ratio/10);
			}
		} else {
			pAd->tr_ratio = 0;
		}
	} else {
		pAd->tr_ratio = 0;
	}

	if (pAd->tr_ratio > pAd->tr_ratio_avg) {
		if (pAd->tr_ratio > MAX_RATIO)
			pAd->tr_ratio_avg += MAX_RATIO;
		else
			pAd->tr_ratio_avg += pAd->tr_ratio;

		pAd->tr_ratio_avg = pAd->tr_ratio_avg/2 + pAd->tr_ratio_avg%2;
	} else {
		pAd->tr_ratio_avg += pAd->tr_ratio;
		pAd->tr_ratio_avg = pAd->tr_ratio_avg/2;
	}

	/* calculate rt ratrio */
	if (pAd->delta_rx_cnt > MIN_CNT_FOR_RATIO_MEASUREMENT) {
		if (pAd->delta_rx_cnt > pAd->delta_tx_cnt) {
			pAd->rt_ratio = pAd->delta_rx_cnt*10 / ((pAd->delta_tx_cnt == 0) ? 1 : pAd->delta_tx_cnt);
			if ((pAd->rt_ratio % 10) >= 5) {/*round off*/
				pAd->rt_ratio = (pAd->rt_ratio/10) + 1;
			} else
				pAd->rt_ratio = (pAd->rt_ratio/10);
		} else
			pAd->rt_ratio = 0;
	} else
		pAd->rt_ratio = 0;

	if (pAd->rt_ratio >= pAd->rt_ratio_avg) {
		if (pAd->rt_ratio > MAX_RATIO)
			pAd->rt_ratio_avg += MAX_RATIO;
		else
			pAd->rt_ratio_avg += pAd->rt_ratio;

		pAd->rt_ratio_avg = pAd->rt_ratio_avg/2 + pAd->rt_ratio_avg%2;
	} else {
		pAd->rt_ratio_avg += pAd->rt_ratio;
		pAd->rt_ratio_avg = pAd->rt_ratio_avg/2;
	}

	/* check tr/rt ratio is stable or not */
	if (pAd->tr_ratio_avg == pAd->pre_tr_ratio_avg) {
		if (tr_ratio_stable < 2)
			tr_ratio_stable++;
	} else if (pAd->tr_ratio_avg != pAd->pre_tr_ratio_avg) {
		if (pAd->tr_ratio_avg < 100) {
			if (tr_ratio_stable > 0)
				tr_ratio_stable--;
		} else { /* UDP case */
			if (tr_ratio_stable < 2)
				tr_ratio_stable++;
		}
	}

	if (pAd->rt_ratio_avg == pAd->pre_rt_ratio_avg) {
		if (rt_ratio_stable < 2)
			rt_ratio_stable++;
	} else if (pAd->rt_ratio_avg != pAd->pre_rt_ratio_avg) {
		if (pAd->rt_ratio < 100) {
			if (rt_ratio_stable > 0)
				rt_ratio_stable--;
		} else { /* UDP case */
			if (rt_ratio_stable < 2)
				rt_ratio_stable++;
		}
	}
	pAd->pre_tr_ratio_avg = pAd->tr_ratio_avg;
	pAd->pre_rt_ratio_avg = pAd->rt_ratio_avg;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("delta_tx_cnt_avg/delta_rx_cnt_avg/sum=%d/%0d/%d\n", pAd->delta_tx_cnt_avg, pAd->delta_rx_cnt_avg, (pAd->delta_tx_cnt_avg + pAd->delta_rx_cnt_avg)));

	/* check tput status */
	if ((pAd->delta_tx_cnt_avg + pAd->delta_rx_cnt_avg) > (SINGLE_5G_PEAK_TP_CNT)) {
		cpu_bound_flag = 1;
	}
	if ((pAd->tp_5g_tx + pAd->tp_5g_rx) &&
		(pAd->tp_2g_tx + pAd->tp_2g_rx)) {
		if ((pAd->delta_tx_cnt_avg + pAd->delta_rx_cnt_avg) > (SINGLE_5G_PEAK_TP_CNT + SINGLE_2G_PEAK_TP_CNT)) {
			concurrent_tput_flag = 1;
		}
		if (bidi_tput_flag) {
			/* the detection threshold will be less than single direction*/
			if ((pAd->delta_tx_cnt_avg + pAd->delta_rx_cnt_avg) > (SINGLE_5G_PEAK_TP_CNT + SINGLE_2G_PEAK_TP_CNT - DELTA_CONCURRENT_PEAK_TP_CNT)) {
				concurrent_tput_flag = 1;
			}
		}
	}
	if (pAd->tp_5g_tx + pAd->tp_5g_rx) {
		if ((pAd->delta_tx_cnt_avg + pAd->delta_rx_cnt_avg) > (SINGLE_5G_PEAK_TP_CNT)) {
			single_5g_tput_flag = 1;
		} else if ((pAd->delta_tx_cnt_avg + pAd->delta_rx_cnt_avg) > (SINGLE_5G_ONE_PAIR_TP_CNT)) {
			single_5g_one_pair_tput_flag = 1;
		}
	}
	if (pAd->tp_2g_tx + pAd->tp_2g_rx) {
		if ((pAd->delta_tx_cnt_avg + pAd->delta_rx_cnt_avg) > (SINGLE_2G_PEAK_TP_CNT)) {
			single_2g_tput_flag = 1;
		} else if ((pAd->delta_tx_cnt_avg + pAd->delta_rx_cnt_avg) > (SINGLE_2G_ONE_PAIR_TP_CNT)) {
			single_2g_one_pair_tput_flag = 1;
		}
	}
	pAd->concurrent_tput_flag = concurrent_tput_flag;
	pAd->single_5g_tput_flag = single_5g_tput_flag;
	pAd->single_2g_tput_flag = single_2g_tput_flag;
	pAd->single_5g_one_pair_tput_flag = single_5g_one_pair_tput_flag;
	pAd->single_2g_one_pair_tput_flag = single_2g_one_pair_tput_flag;
	pAd->bidi_tput_flag = bidi_tput_flag;
	pAd->tx_tput_flag = tx_tput_flag;
	pAd->rx_tput_flag = rx_tput_flag;
	if (concurrent_tput_flag == 1) {
		if (bidi_tput_flag) {
			pAd->con_tx_tp_running = 1;
			pAd->con_rx_tp_running = 1;
		} else if (tx_tput_flag) {
			pAd->con_tx_tp_running = 1;
			pAd->con_rx_tp_running = 0;
		} else if (rx_tput_flag) {
			pAd->con_tx_tp_running = 0;
			pAd->con_rx_tp_running = 1;
		} else {
			pAd->con_tx_tp_running = 0;
			pAd->con_rx_tp_running = 0;
		}
	} else {
		pAd->con_tx_tp_running = 0;
		pAd->con_rx_tp_running = 0;
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("concurrent/5g/2g=%d/%d/%d\n",
			 concurrent_tput_flag, single_5g_tput_flag, single_2g_tput_flag));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("one pair 5g/2g=%d/%d\n",
			 single_5g_one_pair_tput_flag, single_2g_one_pair_tput_flag));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("bidi/tx/rx=%d/%d/%d\n",
			 bidi_tput_flag, tx_tput_flag, rx_tput_flag));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("delta tx/rx=%06d/%06d\n", pAd->delta_tx_cnt, pAd->delta_rx_cnt));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ratio tr/rt=%06d/%06d\n", pAd->tr_ratio, pAd->rt_ratio));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("delta_avg tx/rx=%06d/%06d\n", pAd->delta_tx_cnt_avg, pAd->delta_rx_cnt_avg));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ratio_avg tr/rt=%06d/%06d\n", pAd->tr_ratio_avg, pAd->rt_ratio_avg));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ratio stable tr/rt=%d/%d\n", tr_ratio_stable, rt_ratio_stable));

#ifdef ONE_PAIR_TUNNING
	if (((single_2g_one_pair_tput_flag == 1) || (single_5g_one_pair_tput_flag == 1)) && (pAd->MacTab.Size == 1))
		one_pair_running = 1;
	else
		one_pair_running = 0;
#endif /* ONE_PAIR_TUNNING */

	/* change RPS ratio */
	if ((pAd->disable_auto_ratio == 0) && ((tr_ratio_stable == 2) || (rt_ratio_stable == 2))) {
		UINT32 tr_ratio_avg_temp, rt_ratio_avg_temp;

		/* one pair: set rx max cnt to 16 */
		if (!one_pair_running) {
			if (pAd->PciHif.RxRing[HIF_RX_IDX0].max_rx_process_cnt != MAX_RX_PROCESS_CNT)
				pAd->PciHif.RxRing[HIF_RX_IDX0].max_rx_process_cnt = MAX_RX_PROCESS_CNT;
#ifdef MT7626_E2_SUPPORT
			if (IS_MT7626_FW_VER_E2(pAd) && (IS_ASIC_CAP(pAd, fASIC_CAP_MD))) {
				if (pAd->PciHif.RxRing[HIF_RX_IDX2].max_rx_process_cnt != MAX_RX_PROCESS_CNT)
					pAd->PciHif.RxRing[HIF_RX_IDX2].max_rx_process_cnt = MAX_RX_PROCESS_CNT;
			}
#endif /* MT7626_E2_SUPPORT */
		} else {
			if (pAd->PciHif.RxRing[HIF_RX_IDX0].max_rx_process_cnt != 16)
				pAd->PciHif.RxRing[HIF_RX_IDX0].max_rx_process_cnt = 16;
#ifdef MT7626_E2_SUPPORT
			if (IS_MT7626_FW_VER_E2(pAd) && (IS_ASIC_CAP(pAd, fASIC_CAP_MD))) {
				if (pAd->PciHif.RxRing[HIF_RX_IDX2].max_rx_process_cnt != 16)
					pAd->PciHif.RxRing[HIF_RX_IDX2].max_rx_process_cnt = 16;
			}
#endif /* MT7626_E2_SUPPORT */
		}
		/*
		 * TCP Up Link Small Packet(packet size less than 512)
		 * Reduce Wifi Tx task process packet number, increase Ethernet Tx/Rx task
		 * use CPU core 0 usage time.
		 *
		 * Root Cause:
		 * TCP ACK packet is too much cause ethernet tasklet don't have enough time to process.
		 * task of Wifi TX on core 0
		 * task of Eth Tx/Rx on core 0
		 * task of Wifi Rx on core 1
		 */
		if (pAd->rt_ratio != 0 &&
				pAd->delta_rx_cnt > hit_tcp_ul_rx_cpu_condition &&
				pAd->delta_tx_cnt > hit_tcp_ul_tx_cpu_condition &&
				pAd->one_sec_rx_max_pkt_size <= 512) {
			if (pAd->First_combo_buf_max_cnt != pAd->tcp_small_packet_combo_buf_max_cnt) {
				first_combo_buf_max_cnt_bk = pAd->First_combo_buf_max_cnt;
				second_combo_buf_max_cnt_bk = pAd->Second_combo_buf_max_cnt;
				pAd->First_combo_buf_max_cnt = pAd->tcp_small_packet_combo_buf_max_cnt;
				pAd->Second_combo_buf_max_cnt = pAd->tcp_small_packet_combo_buf_max_cnt;
			}
			pAd->rps_apply_idx = 5;
			pAd->one_sec_rx_max_pkt_size = 0;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("rps_apply_idx=5 pAd->First_combo_TxRpsRatio=%d pAd->First_combo_buf_max_cnt=%d\n", pAd->First_combo_TxRpsRatio, pAd->First_combo_buf_max_cnt));
			goto rps_ratio_check_done;
		} else {
			/* recover original combo buf max count */
			if (pAd->First_combo_buf_max_cnt == pAd->tcp_small_packet_combo_buf_max_cnt) {
				pAd->First_combo_buf_max_cnt = first_combo_buf_max_cnt_bk;
				pAd->Second_combo_buf_max_cnt = second_combo_buf_max_cnt_bk;
			}
			pAd->one_sec_rx_max_pkt_size = 0;
		}

		/*
		 * TCP 1 Station Down Link Small Packet(packet size less than 512)
		 */
		if (pAd->tr_ratio != 0 && pAd->one_sec_tx_max_pkt_size <= 512 && (pAd->MacTab.Size == 1) &&
			pAd->delta_tx_cnt > hit_tcp_dl_tx_cpu_condition &&
			pAd->delta_rx_cnt > hit_tcp_dl_rx_cpu_condition) {
			changed = 1;
			pAd->rps_apply_idx = 6;
			pAd->First_combo_TxRpsRatio = 10;
			pAd->First_combo_TxRpsRatiobaseF = pAd->First_combo_TxRpsRatio*0x100/10;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("rps_apply_idx=6 pAd->First_combo_TxRpsRatio=%d\n", pAd->First_combo_TxRpsRatio));
			goto rps_ratio_check_done;
		}

		/* one pair or bidi or light traffic , set wifi related tasklet to normal */
		#ifdef ONE_PAIR_TUNNING
		if (bidi_tput_flag || one_pair_running ||
			((has_tp_flag && !concurrent_tput_flag && !single_5g_tput_flag && !single_2g_tput_flag) && (pAd->MacTab.Size == 1)))
			pAd->tasklet_schdule_lo_flag = 1;
		else
			pAd->tasklet_schdule_lo_flag = 0;
		#endif /* ONE_PAIR_TUNNING */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pAd->MacTab.Size=%d max_rx_process_cnt=%d tasklet_schdule_lo_flag=%d\n", pAd->MacTab.Size, pAd->PciHif.RxRing[HIF_RX_IDX0].max_rx_process_cnt, pAd->tasklet_schdule_lo_flag));
		/* dynamic fine tune tx rps from here */
		/* one pair and bidi case: set rps ratio=10 for better tput */
		if (bidi_tput_flag && (one_pair_running && has_tp_flag) && !cpu_bound_flag) {
			changed = 1;
			pAd->First_combo_TxRpsRatio = 10;
			pAd->First_combo_TxRpsRatiobaseF = pAd->First_combo_TxRpsRatio*0x100/10;
			pAd->rps_apply_idx = 1;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("rps_apply_idx=1 pAd->First_combo_TxRpsRatio=%d\n", pAd->First_combo_TxRpsRatio));
			goto rps_ratio_check_done;
		}

		/* concurrent peak: set rps ratio from rps ratio table */
		if ((concurrent_tput_flag == 1) || (cpu_bound_flag) || pAd->MacTab.Size >= 2) {
			if (bidi_tput_flag == 1) {
				/* Bi-Di*/
				if (pAd->tr_ratio_avg != 0) {
					tr_ratio_avg_temp = (pAd->tr_ratio_avg > MAX_RATIO) ? MAX_RATIO:pAd->tr_ratio_avg;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s[%d]%d/%d\n", __func__, __LINE__, pAd->First_combo_TxRpsRatio, TxRpsRatiotbl[2][tr_ratio_avg_temp]));
					if (pAd->First_combo_TxRpsRatio != TxRpsRatiotbl[2][tr_ratio_avg_temp]) {
						changed = 1;
						pAd->First_combo_TxRpsRatio = TxRpsRatiotbl[2][tr_ratio_avg_temp];
						pAd->First_combo_TxRpsRatiobaseF = pAd->First_combo_TxRpsRatio*0x100/10;
					}
				}
				if (pAd->rt_ratio_avg != 0) {
					rt_ratio_avg_temp = (pAd->rt_ratio_avg > MAX_RATIO) ? MAX_RATIO:pAd->rt_ratio_avg;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s[%d]%d/%d\n", __func__, __LINE__, pAd->First_combo_TxRpsRatio, TxRpsRatiotbl[2][rt_ratio_avg_temp]));
					if (pAd->First_combo_TxRpsRatio != TxRpsRatiotbl[2][rt_ratio_avg_temp]) {
						changed = 1;
						pAd->First_combo_TxRpsRatio = TxRpsRatiotbl[2][rt_ratio_avg_temp];
						pAd->First_combo_TxRpsRatiobaseF = pAd->First_combo_TxRpsRatio*0x100/10;
					}
				}
			} else if (tx_tput_flag == 1) {
				/* TX*/
				if (pAd->tr_ratio_avg != 0) {
					tr_ratio_avg_temp = (pAd->tr_ratio_avg > MAX_RATIO) ? MAX_RATIO:pAd->tr_ratio_avg;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s[%d]%d/%d\n", __func__, __LINE__, pAd->First_combo_TxRpsRatio, TxRpsRatiotbl[0][tr_ratio_avg_temp]));
					if (pAd->First_combo_TxRpsRatio != TxRpsRatiotbl[0][tr_ratio_avg_temp]) {
						changed = 1;
						pAd->First_combo_TxRpsRatio = TxRpsRatiotbl[0][tr_ratio_avg_temp];
						pAd->First_combo_TxRpsRatiobaseF = pAd->First_combo_TxRpsRatio*0x100/10;
					}
				}
			} else if (rx_tput_flag == 1) {
				/* RX*/
				if (pAd->rt_ratio_avg != 0) {
					rt_ratio_avg_temp = (pAd->rt_ratio_avg > MAX_RATIO) ? MAX_RATIO:pAd->rt_ratio_avg;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s[%d]%d/%d\n", __func__, __LINE__, pAd->First_combo_TxRpsRatio, TxRpsRatiotbl[1][rt_ratio_avg_temp]));
					if (pAd->First_combo_TxRpsRatio != TxRpsRatiotbl[1][rt_ratio_avg_temp]) {
						changed = 1;
						pAd->First_combo_TxRpsRatio = TxRpsRatiotbl[1][rt_ratio_avg_temp];
						pAd->First_combo_TxRpsRatiobaseF = pAd->First_combo_TxRpsRatio*0x100/10;
					}
				}
			}
			pAd->rps_apply_idx = 2;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("rps_apply_idx=2 pAd->First_combo_TxRpsRatio=%d\n", pAd->First_combo_TxRpsRatio));
			goto rps_ratio_check_done;
		}

		/* normal case: set rps to 10 for better tput */
		if (has_tp_flag == 1) {
			/* small packet case: set rps ratio from rps ratio table */
			if (single_5g_tput_flag || single_2g_tput_flag) {
				/*small packet t-put*/
				if (pAd->delta_tx_cnt_avg > hit_tx_cpu_bound_cnt) {
					/* TX*/
					if (pAd->tr_ratio_avg != 0) {
						tr_ratio_avg_temp = (pAd->tr_ratio_avg > MAX_RATIO) ? MAX_RATIO:pAd->tr_ratio_avg;
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s[%d]%d/%d\n", __func__, __LINE__, pAd->First_combo_TxRpsRatio, TxRpsRatiotbl[0][tr_ratio_avg_temp]));
						if (pAd->First_combo_TxRpsRatio != TxRpsRatiotbl[0][tr_ratio_avg_temp]) {
							if (!one_pair_running) {
								changed = 1;
								pAd->First_combo_TxRpsRatio = TxRpsRatiotbl[0][tr_ratio_avg_temp];
								pAd->First_combo_TxRpsRatiobaseF = pAd->First_combo_TxRpsRatio*0x100/10;
							}
						}
					}
				} else if (pAd->delta_rx_cnt_avg > hit_rx_cpu_bound_cnt) {
					/* RX*/
					if (pAd->rt_ratio_avg != 0) {
						rt_ratio_avg_temp = (pAd->rt_ratio_avg > MAX_RATIO) ? MAX_RATIO:pAd->rt_ratio_avg;
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s[%d]%d/%d\n", __func__, __LINE__, pAd->First_combo_TxRpsRatio, TxRpsRatiotbl[1][rt_ratio_avg_temp]));
						if (pAd->First_combo_TxRpsRatio != TxRpsRatiotbl[1][rt_ratio_avg_temp]) {
							if (!one_pair_running) {
								changed = 1;
								pAd->First_combo_TxRpsRatio = TxRpsRatiotbl[1][rt_ratio_avg_temp];
								pAd->First_combo_TxRpsRatiobaseF = pAd->First_combo_TxRpsRatio*0x100/10;
							}
						}
					}
				}
				pAd->rps_apply_idx = 3;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("rps_apply_idx=3 pAd->First_combo_TxRpsRatio=%d\n", pAd->First_combo_TxRpsRatio));
				goto rps_ratio_check_done;
			}

			/* normal case: set rps to 10 for better tput */
			{
				if (pAd->tr_ratio_avg != 0) {
					tr_ratio_avg_temp = (pAd->tr_ratio_avg > MAX_RATIO) ? MAX_RATIO:pAd->tr_ratio_avg;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s[%d]%d/%d\n", __func__, __LINE__, pAd->First_combo_TxRpsRatio, TxRpsRatiotbl[0][tr_ratio_avg_temp]));
					if (tr_ratio_avg_temp > 10) { /*UDP tx case*/
						if (pAd->First_combo_TxRpsRatio != 10) {
							changed = 1;
							pAd->First_combo_TxRpsRatio = 10;
							pAd->First_combo_TxRpsRatiobaseF = pAd->First_combo_TxRpsRatio*0x100/10;
						}
					}
				}
				#ifdef ONE_PAIR_TUNNING
				if (pAd->rt_ratio_avg != 0) {
					rt_ratio_avg_temp = (pAd->rt_ratio_avg > MAX_RATIO) ? MAX_RATIO:pAd->rt_ratio_avg;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s[%d]%d/%d\n", __func__, __LINE__, pAd->First_combo_TxRpsRatio, TxRpsRatiotbl[1][rt_ratio_avg_temp]));
					if (rt_ratio_avg_temp > 10) { /*UDP rx case*/
						if (pAd->First_combo_TxRpsRatio != 10) {
							changed = 1;
							pAd->First_combo_TxRpsRatio = 10;
							pAd->First_combo_TxRpsRatiobaseF = pAd->First_combo_TxRpsRatio*0x100/10;
						}
					}
				}
				#endif
				if (changed) {
					pAd->rps_apply_idx = 4;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("rps_apply_idx=4 pAd->First_combo_TxRpsRatio=%d\n", pAd->First_combo_TxRpsRatio));
					goto rps_ratio_check_done;
				}
			}
		} else {
			/* restore:set  to orignial rps setting*/
			if (pAd->First_combo_TxRpsRatio != TxRpsRatiotbl[0][0]) {
				changed = 1;
				pAd->First_combo_TxRpsRatio = TxRpsRatiotbl[0][0];
				pAd->First_combo_TxRpsRatiobaseF = pAd->First_combo_TxRpsRatio*0x100/10;
				pAd->rps_apply_idx = 0;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("rps_apply_idx=0 pAd->First_combo_TxRpsRatio=%d\n", pAd->First_combo_TxRpsRatio));
			}
		}
	}
rps_ratio_check_done:

	if (changed == 1) {
		RTMP_GetCurrentSystemTick(&pAd->rps_update_tick);

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pAd->First_combo_TxRpsRatio=%d rps_apply_idx=%d\n", pAd->First_combo_TxRpsRatio, pAd->rps_apply_idx));
		pAd->First_combo_cnt_core0 = pAd->First_combo_cnt_core1 = 0;
		pAd->Second_combo_cnt_core0 = pAd->Second_combo_cnt_core1 = 0;
		pAd->First_combo_rx_cnt_core0 = pAd->First_combo_rx_cnt_core1 = 0;
		pAd->First_combo_cnt_core0_old = pAd->First_combo_cnt_core1_old = 0;
		pAd->First_combo_rx_cnt_core0_old = pAd->First_combo_rx_cnt_core1_old = 0;
		if (pAd->First_combo_TxRpsRatio == 0)
			pAd->First_combo_tx_cpu = 0;
		if (pAd->First_combo_TxRpsRatio == 10)
			pAd->First_combo_tx_cpu = 1;
	}
	pAd->one_sec_tx_max_pkt_size = 0;
	return;
}

INT mt7626_combo_rx_rps_check(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket)
{
	UINT8 cpu_id;

	cpu_id = smp_processor_id();
	if (cpu_id == 0)
		pAd->First_combo_rx_cnt_core0++;
	else
		pAd->First_combo_rx_cnt_core1++;

	return 0;
}

INT mt7626_combo_rx_rps_check_E1(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket)
{
	pAd->First_combo_rx_cnt_core1++;

	return 0;
}

INT mt7626_combo_rx_rps_check_E2(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket)
{
	UINT8 cpu_id;

	cpu_id = smp_processor_id();
	if (cpu_id == 0)
		pAd->First_combo_rx_cnt_core0++;
	else
		pAd->First_combo_rx_cnt_core1++;

	return 0;
}
#endif /* DYNAMIC_STEERING_LOADING */


UINT32 truncate_last_tx_time[MAX_LEN_OF_MAC_TABLE][2];
UINT32 truncate_last_rx_time[MAX_LEN_OF_MAC_TABLE][2];
#define TRUNCATE_AIR_TIME_THRESHOLD 720000
#define TRUNCATE_TX_AIR_TIME_RATIO 70
#define TRUNCATE_RX_AIR_TIME_RATIO 70
#define TRUNCATE_BIDI_AIR_TIME_RATIO 25
VOID mt7626_truncate_algo(RTMP_ADAPTER *pAd)
{
	UINT32 i = 1;
	UINT32 rx, ac, truncate_enable[DBDC_BAND_NUM];
	UINT32 wtbl_offset;
	UINT32 tx_sum, tx, rx_sum, tx_diff_time, rx_diff_time;
	UINT32 truncate_sum_tx_rx_time[DBDC_BAND_NUM];
	UINT32 truncate_sum_tx_time[DBDC_BAND_NUM];
	UINT32 truncate_sum_rx_time[DBDC_BAND_NUM];
	UINT32 band_idx = 0;
	MAC_TABLE *pMacTable;
	UINT32 mac_cr_val;
	UINT32 truncate_band_cnt = 0;

#ifdef VOW_SUPPORT
	if (!pAd->vow_rx_time_cfg.rx_time_en)
#endif /* VOW_SUPPORT */
	{
		/*check if air time rx monitor is enable*/
		UINT32 mac_cr_val;

		MAC_IO_READ32(pAd->hdev_ctrl, WF_RMAC_BASE+0x380, &mac_cr_val);
		if (!(0x40000000 & mac_cr_val))
			MAC_IO_WRITE32(pAd->hdev_ctrl, WF_RMAC_BASE+0x380, (mac_cr_val | 0x40000000));
		MAC_IO_READ32(pAd->hdev_ctrl, WF_WTBL_OFF_BASE+0xc, &mac_cr_val);
		if (!(0x80000000 & mac_cr_val))
			MAC_IO_WRITE32(pAd->hdev_ctrl, WF_WTBL_OFF_BASE+0xc, (mac_cr_val | 0x80000000));
	}

	pMacTable = &pAd->MacTab;
	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		truncate_sum_tx_rx_time[band_idx] = 0;
		truncate_sum_tx_time[band_idx] = 0;
		truncate_sum_rx_time[band_idx] = 0;
	}
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		MAC_TABLE_ENTRY *pEntry = &pMacTable->Content[i];
		STA_TR_ENTRY *tr_entry = &pMacTable->tr_entry[i];

		if (IS_ENTRY_NONE(pEntry))
			continue;
		if (!IS_ENTRY_CLIENT(pEntry))
			continue;
		if (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)
			continue;

		if (pEntry->wdev->channel > 14)
			band_idx = 1;
		else
			band_idx = 0;

		/* airtime */
		wtbl_offset = (i << 8) | 0x3004C;
		tx_sum = rx_sum = 0;

		for (ac = 0; ac < 4; ac++) {
			RTMP_IO_READ32(pAd->hdev_ctrl, wtbl_offset + (ac << 3), &tx);
			tx_sum += tx;
			RTMP_IO_READ32(pAd->hdev_ctrl, wtbl_offset + (ac << 3) + 4, &rx);
			rx_sum += rx;
		}
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("[%d]tx_sum=%u, rx_sum=%u\n", i, tx_sum, rx_sum));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("[%d]pre tx=%u, pre rx=%u\n", i,
			truncate_last_tx_time[i][band_idx],
			truncate_last_rx_time[i][band_idx]));
		/* clear WTBL airtime statistic */
		tx_diff_time = (tx_sum - truncate_last_tx_time[i][band_idx]) & 0xfffff;
		rx_diff_time = (rx_sum - truncate_last_rx_time[i][band_idx]) & 0xfffff;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("[%d]tx_diff_time=%u, rx_diff_time=%u\n", i,
			tx_diff_time, rx_diff_time));
		truncate_last_tx_time[i][band_idx] = tx_sum;
		truncate_last_rx_time[i][band_idx] = rx_sum;
		truncate_sum_tx_rx_time[band_idx] += tx_diff_time + rx_diff_time;
		truncate_sum_tx_time[band_idx] += tx_diff_time;
		truncate_sum_rx_time[band_idx] += rx_diff_time;
	}

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		UINT32 tx_ratio;
		UINT32 rx_ratio;

		pAd->truncate_sum_tx_rx_time[band_idx] = truncate_sum_tx_rx_time[band_idx];
		if (truncate_sum_tx_rx_time[band_idx] > 1000000) {
			tx_ratio = (truncate_sum_tx_time[band_idx]*100/truncate_sum_tx_rx_time[band_idx]);
			rx_ratio = (truncate_sum_rx_time[band_idx]*100/truncate_sum_tx_rx_time[band_idx]);
		} else {
			tx_ratio = (truncate_sum_tx_time[band_idx]*100/1000000);
			rx_ratio = (truncate_sum_rx_time[band_idx]*100/1000000);
		}
		pAd->air_time_tx_ratio[band_idx] = tx_ratio;
		pAd->air_time_rx_ratio[band_idx] = rx_ratio;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("[band_idx=%d]tx_rx_all=%u, tx_all=%u, tx ratio=%d, rx ratio=%d\n",
				band_idx, truncate_sum_tx_rx_time[band_idx],
				truncate_sum_tx_time[band_idx],
				tx_ratio, rx_ratio));
		if (pAd->truncate_sum_tx_rx_time[band_idx] > TRUNCATE_AIR_TIME_THRESHOLD) {
			if (tx_ratio > TRUNCATE_TX_AIR_TIME_RATIO) {
				truncate_enable[band_idx] = 1;
			} else if (rx_ratio > TRUNCATE_RX_AIR_TIME_RATIO) {
				truncate_enable[band_idx] = 0;
			} else if ((tx_ratio > TRUNCATE_BIDI_AIR_TIME_RATIO) && (rx_ratio > TRUNCATE_BIDI_AIR_TIME_RATIO)) {
				truncate_enable[band_idx] = 1;
			} else {
				truncate_enable[band_idx] = 0;
			}
		} else {
			truncate_enable[band_idx] = 0;
		}
	}
	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		if (truncate_enable[band_idx])
			truncate_band_cnt++;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Total Airtime:band 0= %u\n", truncate_sum_tx_rx_time[0]));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Total Airtime:band 1= %u\n", truncate_sum_tx_rx_time[1]));

	if (pAd->truncate_force_disable == 1)
		return;

	if (truncate_band_cnt != 0)
		pAd->truncate_enable = 1;
	else
		pAd->truncate_enable = 0;

	MAC_IO_READ32(pAd->hdev_ctrl, WF_AGG_BASE+0x160, &mac_cr_val);
	if (pAd->truncate_enable)
		MAC_IO_WRITE32(pAd->hdev_ctrl, WF_AGG_BASE+0x160, (mac_cr_val | 0x1));
	else
		MAC_IO_WRITE32(pAd->hdev_ctrl, WF_AGG_BASE+0x160, (mac_cr_val & ~0x1));
}

/*
* Bring out of OFFCHANNEL_SCAN_FEATURE flag,
* to make it generic function, to be used as generic requirement
*/
VOID mt7626_reset_enable_nf_registers(RTMP_ADAPTER *pAd, UCHAR bandidx)
{
	UINT32 Value = 0;
	if (bandidx == DBDC_BAND0) {
		/* Reset band0*/
		MAC_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_12, &Value);
		Value |= (1 << B0IrpiSwCtrlResetOffset);
		Value |= (1 << B0IrpiSwCtrlOnlyOffset);

		MAC_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_12, Value);
		MAC_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_12, Value);
		/* Enable badn0 IPI control */
		HW_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS2, &Value);
		Value |= (B0IpiEnableCtrlValue << B0IpiEnableCtrlOffset);

		MAC_IO_WRITE32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS2, Value);
	} else {
		MAC_IO_READ32(pAd->hdev_ctrl, TALOS_PHY_RXTD_12, &Value);
		Value |= (1 << B1IrpiSwCtrlResetOffset);
		Value |= (1 << B1IrpiSwCtrlOnlyOffset);

		MAC_IO_WRITE32(pAd->hdev_ctrl, TALOS_PHY_RXTD_12, Value);
		MAC_IO_WRITE32(pAd->hdev_ctrl, TALOS_PHY_RXTD_12, Value);

		MAC_IO_READ32(pAd->hdev_ctrl, TALOS_PHY1_CR_BAND0_STSCNT_EN_CTRL, &Value);
		Value |= (B1IpiEnableCtrlValue << B1IpiEnableCtrlOffset);

		MAC_IO_WRITE32(pAd->hdev_ctrl, TALOS_PHY1_CR_BAND0_STSCNT_EN_CTRL, Value);
	}
}

/*
* Bring out of OFFCHANNEL_SCAN_FEATURE flag,
* to make it generic function, to be used as generic requirement
*/
VOID mt7626_calculate_nf(RTMP_ADAPTER *pAd, UCHAR bandidx)
{
	UINT8 NF_index = 0;
	UINT32 total = 0, value[11] = {0};
	INT32 NF, xNF = 0;
	INT16 NF_Power[] = {-92, -89, -86, -83, -80, -75, -70, -65, -60, -55, -52};
	UINT32 band_offset;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s] for bandidx = %d\n", __func__, bandidx));


	if ((bandidx != DBDC_BAND0) && (bandidx != DBDC_BAND1))
		return;

	/*RTMP_SEM_LOCK(&pAd->ScanCtrl.NF_Lock); //not needed for now*/
	if (bandidx == DBDC_BAND0) {
			band_offset = PHY_RXTD1_0;
	} else {
			band_offset = TALOS_PHY1_RXTD1_0;
	}

	for (NF_index = 0; NF_index <= 10; NF_index++) {
	    MAC_IO_READ32(pAd->hdev_ctrl, (band_offset + 4 * NF_index), &value[NF_index]);
		total += value[NF_index];
		xNF += (NF_Power[NF_index] * (INT32)value[NF_index]);
	}

	if (xNF && total)
		NF = xNF/(INT32)total;

#ifdef OFFCHANNEL_SCAN_FEATURE
	if (pAd->ScanCtrl[bandidx].OffChScan == FALSE)
#endif
	{
		pAd->Avg_NFx16[bandidx] = (pAd->Avg_NF[bandidx] != 0) ? (NF - pAd->Avg_NF[bandidx] + pAd->Avg_NFx16[bandidx]) : NF << 4;
		pAd->Avg_NF[bandidx] = pAd->Avg_NFx16[bandidx] >> 4;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: Bandidx : %d xNF : %d Total : %u NF : %d pAd->Avg_NFx16[%d] : %d  pAd->Avg_NF[%d] : %d \n", __func__, bandidx, xNF, total, NF, bandidx, pAd->Avg_NFx16[bandidx], bandidx, pAd->Avg_NF[bandidx]));
	}

#ifdef OFFCHANNEL_SCAN_FEATURE
	if ((pAd->ScanCtrl[bandidx].OffChScan == TRUE)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: Bandidx : %d Before  AvgNF : %d \n", __func__, bandidx, pAd->ChannelInfo.AvgNF));
		pAd->ChannelInfo.AvgNF = NF;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: Bandidx : %d After  AvgNF : %d \n", __func__, bandidx, pAd->ChannelInfo.AvgNF));
	}

	if (pAd->ScanCtrl[bandidx].OffChScan == FALSE)
#endif
	{
		mt7626_reset_enable_nf_registers(pAd, bandidx);
	}

	/*RTMP_SEM_UNLOCK(&pAd->ScanCtrl.NF_Lock);*/
}


VOID mt7626_enable_nf_support(struct _RTMP_ADAPTER *pAd)
{
	int Value, i;

	for (i = 0; i < DBDC_BAND_NUM; i++) {

		pAd->Avg_NF[i] = 0;
		pAd->Avg_NFx16[i] = 0;
		/*band0*/

		if (i == DBDC_BAND0) {
			/*Turn on band 0 IPI*/
			MAC_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_12, &Value);
			Value |= (1 << B0IrpiSwCtrlResetOffset);
			Value |= (1 << B0IrpiSwCtrlOnlyOffset);

			MAC_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_12, Value);
			MAC_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_12, Value);

			/* Enable badn0 IPI control */
			MAC_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS2, &Value);
			Value |= (B0IpiEnableCtrlValue << B0IpiEnableCtrlOffset);

			MAC_IO_WRITE32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS2, Value);
		} else {

			MAC_IO_READ32(pAd->hdev_ctrl, TALOS_PHY_RXTD_12, &Value);
			Value |= (1 << B1IrpiSwCtrlResetOffset);
			Value |= (1 << B1IrpiSwCtrlOnlyOffset);


			MAC_IO_WRITE32(pAd->hdev_ctrl, TALOS_PHY_RXTD_12, Value);
			MAC_IO_WRITE32(pAd->hdev_ctrl, TALOS_PHY_RXTD_12, Value);

			MAC_IO_READ32(pAd->hdev_ctrl, TALOS_PHY1_CR_BAND0_STSCNT_EN_CTRL, &Value);
			Value |= (B1IpiEnableCtrlValue << B1IpiEnableCtrlOffset);

			MAC_IO_WRITE32(pAd->hdev_ctrl, TALOS_PHY1_CR_BAND0_STSCNT_EN_CTRL, Value);
		}
	}
}

static INT mt7626AsicArchOpsInit(RTMP_ADAPTER *pAd, RTMP_ARCH_OP *arch_ops)
{
	arch_ops->asic_ampdu_efficiency_on_off = MtAsicAMPDUEfficiencyAdjustbyFW;
	arch_ops->archGetCrcErrCnt = MtAsicGetCrcErrCnt;
	arch_ops->archGetCCACnt = MtAsicGetCCACnt;
	arch_ops->archGetChBusyCnt = MtAsicGetChBusyCnt;
#ifdef CONFIG_STA_SUPPORT
	arch_ops->archEnableIbssSync = NULL;/* MtAsicEnableBssSyncByDriver; */
#endif /* CONFIG_STA_SUPPORT */
	arch_ops->archSetAutoFallBack = MtAsicSetAutoFallBack;
	arch_ops->archAutoFallbackInit = MtAsicAutoFallbackInit;
	arch_ops->archUpdateProtect = MtAsicUpdateProtectByFw;
	arch_ops->archUpdateRtsThld = MtAsicUpdateRtsThldByFw;
	arch_ops->archSwitchChannel = MtAsicSwitchChannel;
	arch_ops->archSetRDG = NULL;
	arch_ops->archSetDevMac = MtAsicSetDevMacByFw;
	arch_ops->archSetBssid = MtAsicSetBssidByFw;
	arch_ops->archSetStaRec = MtAsicSetStaRecByFw;
	arch_ops->archUpdateStaRecBa = MtAsicUpdateStaRecBaByFw;
	arch_ops->asic_rts_on_off = mt_asic_rts_on_off;
#ifdef CONFIG_AP_SUPPORT
	arch_ops->archSetMbssWdevIfAddr = MtAsicSetMbssWdevIfAddrGen2;
	arch_ops->archSetMbssHwCRSetting = MtDmacSetMbssHwCRSetting;
	arch_ops->archSetExtTTTTHwCRSetting = MtDmacSetExtTTTTHwCRSetting;
	arch_ops->archSetExtMbssEnableCR = MtDmacSetExtMbssEnableCR;
#endif /* CONFIG_AP_SUPPORT */
	arch_ops->archDelWcidTab = MtAsicDelWcidTabByFw;
#ifdef HTC_DECRYPT_IOT
	arch_ops->archSetWcidAAD_OM = MtAsicUpdateStaRecAadOmByFw;
#endif
#ifdef MBSS_AS_WDS_AP_SUPPORT
	arch_ops->archSetWcid4Addr_HdrTrans = MtAsicSetWcid4Addr_HdrTransByFw;
#endif

	arch_ops->archAddRemoveKeyTab = MtAsicAddRemoveKeyTabByFw;
#ifdef BCN_OFFLOAD_SUPPORT
	/* sync with Carter, wilsonl */
	arch_ops->archEnableBeacon = NULL;
	arch_ops->archDisableBeacon = NULL;
	arch_ops->archUpdateBeacon = MtUpdateBcnAndTimToMcu;
#else
	arch_ops->archEnableBeacon = MtDmacAsicEnableBeacon;
	arch_ops->archDisableBeacon = MtDmacAsicDisableBeacon;
	arch_ops->archUpdateBeacon = MtUpdateBeaconToAsic;
#endif
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	arch_ops->archSetReptFuncEnable = MtAsicSetReptFuncEnableByFw;
	arch_ops->archInsertRepeaterEntry = MtAsicInsertRepeaterEntryByFw;
	arch_ops->archRemoveRepeaterEntry = MtAsicRemoveRepeaterEntryByFw;
	arch_ops->archInsertRepeaterRootEntry = MtAsicInsertRepeaterRootEntryByFw;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */
	arch_ops->archSetPiggyBack = MtAsicSetPiggyBack;
	arch_ops->archSetPreTbtt = NULL;/* offload to BssInfoUpdateByFw */
	arch_ops->archSetGPTimer = MtAsicSetGPTimer;
	arch_ops->archSetChBusyStat = MtAsicSetChBusyStat;
	arch_ops->archGetTsfTime = MtAsicGetTsfTimeByFirmware;
	arch_ops->archDisableSync = NULL;/* MtAsicDisableSyncByDriver; */
	arch_ops->archSetSyncModeAndEnable = NULL;/* MtAsicEnableBssSyncByDriver; */
	arch_ops->archSetWmmParam = MtAsicSetWmmParam;
	arch_ops->archGetWmmParam = MtAsicGetWmmParam;
	arch_ops->archSetEdcaParm = MtAsicSetEdcaParm;
	arch_ops->archSetRetryLimit = MtAsicSetRetryLimit;
	arch_ops->archGetRetryLimit = MtAsicGetRetryLimit;
	arch_ops->archSetSlotTime = MtAsicSetSlotTime;
	arch_ops->archGetTxTsc = MtAsicGetTxTscByDriver;
	arch_ops->archAddSharedKeyEntry = MtAsicAddSharedKeyEntry;
	arch_ops->archRemoveSharedKeyEntry = MtAsicRemoveSharedKeyEntry;
	arch_ops->archAddPairwiseKeyEntry = MtAsicAddPairwiseKeyEntry;
	arch_ops->archSetBW = MtAsicSetBW;
	arch_ops->archSetCtrlCh = mt_mac_set_ctrlch;
	arch_ops->archWaitMacTxRxIdle = MtAsicWaitMacTxRxIdle;
#ifdef MAC_INIT_OFFLOAD
	arch_ops->archSetMacTxRx = MtAsicSetMacTxRxByFw;
	arch_ops->archSetRxvFilter = MtAsicSetRxvFilter;
	arch_ops->archSetMacMaxLen = NULL;
	arch_ops->archSetTxStream = NULL;
	arch_ops->archSetRxFilter = NULL;/* MtAsicSetRxFilter; */
#else
	arch_ops->archSetMacTxRx = MtAsicSetMacTxRx;
	arch_ops->archSetMacMaxLen = MtAsicSetMacMaxLen;
	arch_ops->archSetTxStream = MtAsicSetTxStream;
	arch_ops->archSetRxFilter = MtAsicSetRxFilter;
#endif /*MAC_INIT_OFFLOAD*/
	arch_ops->archSetMacWD = MtAsicSetMacWD;
#ifdef MAC_APCLI_SUPPORT
	arch_ops->archSetApCliBssid = MtAsicSetApCliBssid;
#endif /* MAC_APCLI_SUPPORT */
	arch_ops->archTOPInit = MtAsicTOPInit;
	arch_ops->archSetTmrCR = MtSetTmrCRByFw;
	arch_ops->archUpdateRxWCIDTable = MtAsicUpdateRxWCIDTableByFw;

#ifdef DBDC_MODE
	arch_ops->archSetDbdcCtrl = MtAsicSetDbdcCtrlByFw;
	arch_ops->archGetDbdcCtrl = MtAsicGetDbdcCtrlByFw;
#endif /*DBDC_MODE*/

#ifdef DOT11_VHT_AC
	arch_ops->archSetRtsSignalTA = MtAsicSetRtsSignalTA;
#endif /*  DOT11_VHT_AC */

#ifdef TXBF_SUPPORT
	arch_ops->archUpdateClientBfCap = mt_AsicClientBfCap;
#endif /* TXBF_SUPPORT */
	arch_ops->archUpdateBASession = MtAsicUpdateBASessionOffloadByFw;
	arch_ops->archGetTidSn = MtAsicGetTidSnByDriver;
	arch_ops->archSetSMPS = MtAsicSetSMPSByDriver;
	arch_ops->archRxHeaderTransCtl = MtAsicRxHeaderTransCtl;
	arch_ops->archRxHeaderTaranBLCtl = MtAsicRxHeaderTaranBLCtl;
	arch_ops->rx_pkt_process = mt_rx_pkt_process;
#ifdef MT7626_E2_SUPPORT
	if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD))
		arch_ops->rx2_pkt_process = mt_rx2_pkt_process;
#endif
	arch_ops->get_packet_type = mtd_get_packet_type;
	arch_ops->trans_rxd_into_rxblk = mtd_trans_rxd_into_rxblk;
	arch_ops->archSetRxStream = NULL;/* MtAsicSetRxStream; */
#ifdef IGMP_SNOOP_SUPPORT
	arch_ops->archMcastEntryInsert = MulticastFilterTableInsertEntry;
	arch_ops->archMcastEntryDelete = MulticastFilterTableDeleteEntry;
#endif
	arch_ops->write_txp_info = mtd_write_txp_info_by_host_v2;
	arch_ops->write_tmac_info_fixed_rate = mtd_write_tmac_info_fixed_rate;
#ifdef REDUCE_TX_OVERHEAD
	arch_ops->write_tmac_info = mtd_write_tmac_info_by_host_cached;
#else
	arch_ops->write_tmac_info = mtd_write_tmac_info_by_host;
#endif
	arch_ops->dump_tmac_info = mtd_dump_tmac_info;
	arch_ops->write_tx_resource = mtd_pci_write_tx_resource;
	arch_ops->write_frag_tx_resource = mt_pci_write_frag_tx_resource;
	arch_ops->kickout_data_tx = pci_kickout_data_tx;
	arch_ops->get_pkt_from_rx_resource = mtd_pci_get_pkt_from_rx_resource;
	arch_ops->get_pkt_from_rx1_resource = mtd_pci_get_pkt_from_rx_resource;
#ifdef MT7626_E2_SUPPORT
	if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD))
		arch_ops->get_pkt_from_rx2_resource = mtd_pci_get_pkt_from_rx2_resource;
#endif
	arch_ops->get_resource_idx = mtd_pci_get_resource_idx;
	arch_ops->build_resource_idx = mtd_pci_build_resource_idx;
	arch_ops->get_tx_resource_free_num = pci_get_tx_resource_free_num;
	arch_ops->check_hw_resource = mt_ct_check_hw_resource;
	arch_ops->inc_resource_full_cnt = pci_inc_resource_full_cnt;
	arch_ops->get_resource_state = pci_get_resource_state;
	arch_ops->set_resource_state = pci_set_resource_state;
	arch_ops->check_resource_state = pci_check_resource_state;
	arch_ops->get_hif_buf = mt_pci_get_hif_buf;
	arch_ops->hw_tx = mt_ct_hw_tx;
	arch_ops->mlme_hw_tx = mt_ct_mlme_hw_tx;
#ifdef CONFIG_ATE
	arch_ops->ate_hw_tx = mt_ct_ate_hw_tx;
#endif
	arch_ops->rx_done_handle = mtd_rx_done_handle;
#ifdef MT7626_E2_SUPPORT
	if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD))
		arch_ops->rx2_done_handle = mtd_rx2_done_handle;
#endif
	arch_ops->cmd_dma_done_handle = mt_cmd_dma_done_handle;
	arch_ops->fwdl_dma_done_handle = mt_fwdl_dma_done_handle;
	arch_ops->tx_dma_done_handle = mtd_tx_dma_done_handle;

#ifdef RED_SUPPORT
	arch_ops->archRedMarkPktDrop = RedMarkPktDrop;
#endif
#ifdef DYNAMIC_STEERING_LOADING
	arch_ops->arch_combo_tx_rps_check = mt7626_combo_tx_rps_check;
	arch_ops->arch_combo_tx_rps_algo = mt7626_combo_tx_rps_algo;
	arch_ops->arch_combo_rx_rps_check = mt7626_combo_rx_rps_check;
#ifdef MT7626_E2_SUPPORT
	if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD))
		arch_ops->arch_combo_rx_rps_check = mt7626_combo_rx_rps_check_E2;
	else
		arch_ops->arch_combo_rx_rps_check = mt7626_combo_rx_rps_check_E1;
#endif /*MT7626_E2_SUPPORT*/
#endif /* DYNAMIC_STEERING_LOADING */
	arch_ops->arch_truncate_algo = NULL; /*mt7626_truncate_algo;*/

#if defined(NF_SUPPORT) || defined(OFFCHANNEL_SCAN_FEATURE)
	arch_ops->arch_calculate_nf = mt7626_calculate_nf;
	arch_ops->arch_reset_enable_nf_registers = mt7626_reset_enable_nf_registers;
	arch_ops->arch_enable_nf_support = mt7626_enable_nf_support;
#endif

	return TRUE;
}


VOID mt7626_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
	struct tr_delay_control *tr_delay_ctl = &pAd->tr_ctl.tr_delay_ctl;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __func__));
	/* need to set HIF type before get hw related information */
	MT7626_ChipCap.hif_type = HIF_MT;
	NdisMoveMemory(pChipCap, &MT7626_ChipCap, sizeof(RTMP_CHIP_CAP));
	AsicGetMacInfo(pAd, &pAd->ChipID, &pAd->HWVersion, &pAd->FWVersion);

#ifdef MT7626_E2_SUPPORT
	if (IS_MT7626_FW_VER_E2(pAd) && ((INT32)pAd->dev_irq_md > 0))
		mt7626_cap_md_support = FALSE;
	else
#endif
	mt7626_cap_md_support = FALSE;
	mt7626_chipCap_init();
	mt7626_chipOp_init();

	NdisMoveMemory(pChipCap, &MT7626_ChipCap, sizeof(RTMP_CHIP_CAP));
	hc_register_chip_ops(pAd->hdev_ctrl, &MT7626_ChipOp);

	mt7626AsicArchOpsInit(pAd, arch_ops);

	mt7626_chip_dbg_init(chip_dbg);

	mt_phy_probe(pAd);
	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_AGBS);
	/* Following function configure beacon related parameters in pChipCap
	 * FlgIsSupSpecBcnBuf / BcnMaxHwNum / WcidHwRsvNum / BcnMaxHwSize / BcnBase[]
	 */
	/* sync with Cater, wilsonl */
	mt_chip_bcn_parameter_init(pAd);
	/* sync with Cater, wilsonl */
	pChipCap->OmacNums = 5;
	pChipCap->BssNums = 4;
	pChipCap->ExtMbssOmacStartIdx = 0x10;
	pChipCap->RepeaterStartIdx = 0x20;
#ifdef AIR_MONITOR
	pChipCap->MaxRepeaterNum = 16;
#else
	pChipCap->MaxRepeaterNum = 32;
#endif /* AIR_MONITOR */

#ifdef BCN_OFFLOAD_SUPPORT
	pChipCap->fgBcnOffloadSupport = TRUE;
	pChipCap->fgIsNeedPretbttIntEvent = FALSE;
#endif
	/* For calibration log buffer size limitation issue */
	pAd->fgQAtoolBatchDumpSupport = TRUE;
#ifdef RED_SUPPORT
	pAd->red_have_cr4 = FALSE;
#endif /* RED_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	/*VOW CR Address offset - Gen_TALOS*/
	pAd->vow_gen.VOW_GEN = VOW_GEN_TALOS;
#endif /* #ifdef CONFIG_AP_SUPPORT */

	tr_delay_ctl->ul_rx_dly_ctl_tbl = mt7626_rx_dly_ctl_ul_tbl;
	tr_delay_ctl->ul_rx_dly_ctl_tbl_size = (sizeof(mt7626_rx_dly_ctl_ul_tbl) / sizeof(mt7626_rx_dly_ctl_ul_tbl[0]));
	tr_delay_ctl->dl_rx_dly_ctl_tbl = mt7626_rx_dly_ctl_dl_tbl;
	tr_delay_ctl->dl_rx_dly_ctl_tbl_size = (sizeof(mt7626_rx_dly_ctl_dl_tbl) / sizeof(mt7626_rx_dly_ctl_dl_tbl[0]));

	/* For TP tuning dynamically, only apply on DBDC case for MT7626*/
	pChipCap->Ap5GPeakTpTH = 0;
	pChipCap->Ap2GPeakTpTH = 0;
	pChipCap->ApDBDC5GPeakTpTH = 865;
	pChipCap->ApDBDC2GPeakTpTH = 410;
#ifdef DYNAMIC_STEERING_LOADING
	pAd->disable_auto_rps = 0;
	pAd->disable_auto_ratio = 0;

	pAd->First_combo_tx_cpu = 0;
	pAd->First_combo_TxRpsRatio = 0;
	pAd->First_combo_TxRpsRatiobaseF = pAd->First_combo_TxRpsRatio*0x100/10;
	pAd->First_combo_cnt_core0 = 0;
	pAd->First_combo_cnt_core1 = 0;
	pAd->Second_combo_cnt_core0 = 0;
	pAd->Second_combo_cnt_core1 = 0;
	pAd->First_combo_rx_cnt_core0 = 0;
	pAd->First_combo_rx_cnt_core1 = 0;

	pAd->First_combo_buf_max_cnt = MAX_COMBO_TX_CNT;
	pAd->Second_combo_buf_max_cnt = MAX_COMBO_2ND_TX_CNT;
	pAd->First_combo_buf_max_processed_cnt = 0;
	pAd->Second_combo_buf_max_processed_cnt = 0;

	pAd->First_combo_buf_underrun = 0;
	pAd->disable_auto_txop = 0;
	pAd->disable_amsdu_read = 0;
	pAd->rx_intr_cpu = 1;
	pAd->truncate_force_disable = 1;
	pAd->total_tx_process_cnt_for_specific_cpu = MAX_COMBO_TX_SWITCH_THRESHOLD;
	pAd->tcp_small_packet_combo_buf_max_cnt = 16;

	pAd->tasklet_schdule_lo_flag = 0;
#ifdef MT7626_E2_SUPPORT
	if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("7629 E2(MD) rps table\n\r"));
		memcpy(&TxRpsRatiotbl[0][0], &TxRpsRatiotbl_E2[0][0], sizeof(TxRpsRatiotbl));
		pAd->total_tx_process_cnt_for_specific_cpu = 128;
	} else if (IS_MT7626_FW_VER_E2(pAd)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("7629 E2(no MD) rps table\n\r"));
		memcpy(&TxRpsRatiotbl[0][0], &TxRpsRatiotbl_E1[0][0], sizeof(TxRpsRatiotbl));
		pAd->total_tx_process_cnt_for_specific_cpu = 128;
	} else
#endif
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("7629 E1 rps table\n\r"));
		memcpy(&TxRpsRatiotbl[0][0], &TxRpsRatiotbl_E1[0][0], sizeof(TxRpsRatiotbl));
		pAd->total_tx_process_cnt_for_specific_cpu = 128;
	}
#endif /* DYNAMIC_STEERING_LOADING */
#ifdef MT7626
	pAd->FixRateErr1 = 0;
	pAd->FixRateErr2 = 0;
	pAd->FixRateErr3 = 0;
	pAd->FixRateErr4 = 0;
	pAd->FixRateErr5 = 0;
	pAd->FixRateErr6 = 0;
#endif

#ifdef WIFI_RAM_EMI_SUPPORT
	pChipCap->ram_ilm_emi_addr_offset = 0x75000; /* 0xF0075000 */
	pChipCap->ram_dlm_emi_addr_offset = 0x146800;/* 0xF0146800 */
	pChipCap->emi_phy_addr = 0x41000000;
	pChipCap->emi_phy_addr_size = 0x200000;
#endif /* WIFI_RAM_EMI_SUPPORT */

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<--%s()\n", __func__));
}

