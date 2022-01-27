/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	cmm_cfg.c

    Abstract:
    Ralink WiFi Driver configuration related subroutines

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/

#include "rt_config.h"

static BOOLEAN RT_isLegalCmdBeforeInfUp(RTMP_STRING *SetCmd);
RTMP_STRING *wdev_type2str(int type);

INT ComputeChecksum(UINT PIN)
{
	INT digit_s;
	UINT accum = 0;
	INT ret;

	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10);
	accum += 1 * ((PIN / 1000000) % 10);
	accum += 3 * ((PIN / 100000) % 10);
	accum += 1 * ((PIN / 10000) % 10);
	accum += 3 * ((PIN / 1000) % 10);
	accum += 1 * ((PIN / 100) % 10);
	accum += 3 * ((PIN / 10) % 10);
	digit_s = (accum % 10);
	ret = ((10 - digit_s) % 10);
	return ret;
} /* ComputeChecksum*/

UINT GenerateWpsPinCode(
	IN	PRTMP_ADAPTER	pAd,
	IN  BOOLEAN         bFromApcli,
	IN	UCHAR			apidx)
{
	UCHAR	macAddr[MAC_ADDR_LEN];
	UINT	iPin;
	UINT	checksum;
	NdisZeroMemory(macAddr, MAC_ADDR_LEN);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

		if (bFromApcli)
			NdisMoveMemory(&macAddr[0], pAd->StaCfg[apidx].wdev.if_addr, MAC_ADDR_LEN);
		else
#endif /* APCLI_SUPPORT */
			NdisMoveMemory(&macAddr[0], pAd->ApCfg.MBSSID[apidx].wdev.if_addr, MAC_ADDR_LEN);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	NdisMoveMemory(&macAddr[0], pAd->StaCfg[0].wdev.if_addr, MAC_ADDR_LEN);
#endif /* CONFIG_STA_SUPPORT */
	iPin = macAddr[0] * 256 * 256 + macAddr[4] * 256 + macAddr[5];
	iPin = iPin % 10000000;
	checksum = ComputeChecksum(iPin);
	iPin = iPin * 10 + checksum;
	return iPin;
}

static char *phy_mode_str[] = {"CCK", "OFDM", "HTMIX", "GF", "VHT"};
char *get_phymode_str(int Mode)
{
	if (Mode >= MODE_CCK && Mode <= MODE_VHT)
		return phy_mode_str[Mode];
	else
		return "N/A";
}


/*
    ==========================================================================
    Description:
	Set Country Region to pAd->CommonCfg.CountryRegion.
	This command will not work, if the field of CountryRegion in eeprom is programmed.

    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT RT_CfgSetCountryRegion(RTMP_ADAPTER *pAd, RTMP_STRING *arg, INT band)
{
	LONG region;
	UCHAR *pCountryRegion;

	region = os_str_tol(arg, 0, 10);

	if (band == BAND_24G)
		pCountryRegion = &pAd->CommonCfg.CountryRegion;
	else
		pCountryRegion = &pAd->CommonCfg.CountryRegionForABand;

	/*
		   1. If this value is set before interface up, do not reject this value.
		   2. Country can be set only when EEPROM not programmed
	*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS) && (*pCountryRegion & EEPROM_IS_PROGRAMMED)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("CfgSetCountryRegion():CountryRegion in eeprom was programmed\n"));
		return FALSE;
	}

	if ((region >= 0) &&
		(((band == BAND_24G) && ((region <= REGION_MAXIMUM_BG_BAND) ||
								 (region == REGION_31_BG_BAND) || (region == REGION_32_BG_BAND) || (region == REGION_33_BG_BAND))) ||
		 ((band == BAND_5G) && (region <= REGION_MAXIMUM_A_BAND)))
	   )
		*pCountryRegion = (UCHAR) region;
	else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CfgSetCountryRegion():region(%ld) out of range!\n", region));
		return FALSE;
	}

	return TRUE;
}

static UCHAR CFG_WMODE_MAP[] = {
	PHY_11BG_MIXED, (WMODE_B | WMODE_G), /* 0 => B/G mixed */
	PHY_11B, (WMODE_B), /* 1 => B only */
	PHY_11A, (WMODE_A), /* 2 => A only */
	PHY_11ABG_MIXED, (WMODE_A | WMODE_B | WMODE_G), /* 3 => A/B/G mixed */
	PHY_11G, WMODE_G, /* 4 => G only */
	PHY_11ABGN_MIXED, (WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN), /* 5 => A/B/G/GN/AN mixed */
	PHY_11N_2_4G, (WMODE_GN), /* 6 => N in 2.4G band only */
	PHY_11GN_MIXED, (WMODE_G | WMODE_GN), /* 7 => G/GN, i.e., no CCK mode */
	PHY_11AN_MIXED, (WMODE_A | WMODE_AN), /* 8 => A/N in 5 band */
	PHY_11BGN_MIXED, (WMODE_B | WMODE_G | WMODE_GN), /* 9 => B/G/GN mode*/
	PHY_11AGN_MIXED, (WMODE_G | WMODE_GN | WMODE_A | WMODE_AN), /* 10 => A/AN/G/GN mode, not support B mode */
	PHY_11N_5G, (WMODE_AN), /* 11 => only N in 5G band */
#ifdef DOT11_VHT_AC
	PHY_11VHT_N_ABG_MIXED, (WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN | WMODE_AC), /* 12 => B/G/GN/A/AN/AC mixed*/
	PHY_11VHT_N_AG_MIXED, (WMODE_G | WMODE_GN | WMODE_A | WMODE_AN | WMODE_AC), /* 13 => G/GN/A/AN/AC mixed, no B mode */
	PHY_11VHT_N_A_MIXED, (WMODE_A | WMODE_AN | WMODE_AC), /* 14 => A/AC/AN mixed */
	PHY_11VHT_N_MIXED, (WMODE_AN | WMODE_AC), /* 15 => AC/AN mixed, but no A mode */
#endif /* DOT11_VHT_AC */
	PHY_MODE_MAX, WMODE_INVALID /* default phy mode if not match */
};

static RTMP_STRING *BAND_STR[] = {"Invalid", "2.4G", "5G", "2.4G/5G"};
static RTMP_STRING *WMODE_STR[] = {"", "A", "B", "G", "gN", "aN", "AC", "AX"};

UCHAR *wmode_2_str(UCHAR wmode)
{
	UCHAR *str;
	INT idx, pos, max_len;

	max_len = WMODE_COMP * 3;

	if (os_alloc_mem(NULL, &str, max_len) == NDIS_STATUS_SUCCESS) {
		NdisZeroMemory(str, max_len);
		pos = 0;

		for (idx = 0; idx < WMODE_COMP; idx++) {
			if (wmode & (1 << idx)) {
				if ((strlen(str) +  strlen(WMODE_STR[idx + 1])) >= (max_len - 1))
					break;

				if (strlen(str)) {
					NdisMoveMemory(&str[pos], "/", 1);
					pos++;
				}

				NdisMoveMemory(&str[pos], WMODE_STR[idx + 1], strlen(WMODE_STR[idx + 1]));
				pos += strlen(WMODE_STR[idx + 1]);
			}

			if (strlen(str) >= max_len)
				break;
		}

		return str;
	} else
		return NULL;
}

RT_802_11_PHY_MODE wmode_2_cfgmode(UCHAR wmode)
{
	INT i, mode_cnt = sizeof(CFG_WMODE_MAP) / (sizeof(UCHAR) * 2);

	for (i = 0; i < mode_cnt; i++) {
		if (CFG_WMODE_MAP[i * 2 + 1] == wmode)
			return CFG_WMODE_MAP[i * 2];
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Cannot get cfgmode by wmode(%x)\n",
			 __func__, wmode));
	return 0;
}

UCHAR cfgmode_2_wmode(UCHAR cfg_mode)
{
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cfg_mode=%d\n", cfg_mode));

	if (cfg_mode >= PHY_MODE_MAX)
		cfg_mode =  PHY_MODE_MAX;

	return CFG_WMODE_MAP[cfg_mode * 2 + 1];
}

#ifdef MIN_PHY_RATE_SUPPORT
HTTRANSMIT_SETTING MinPhyRate_2_HtTransmit(UCHAR MinPhyRate)
{
	HTTRANSMIT_SETTING MinPhyTransmit;

	MinPhyTransmit.word = 0;
	MinPhyTransmit.field.BW =  BW_20;
		switch (MinPhyRate) {
		case PHY_RATE_1:
			MinPhyTransmit.field.MODE = MODE_CCK;
			MinPhyTransmit.field.MCS = RATE_1;
			break;
		case PHY_RATE_2:
			MinPhyTransmit.field.MODE = MODE_CCK;
			MinPhyTransmit.field.MCS = RATE_2;
			break;
		case PHY_RATE_5:
			MinPhyTransmit.field.MODE = MODE_CCK;
			MinPhyTransmit.field.MCS = RATE_5_5;
			break;
		case PHY_RATE_11:
			MinPhyTransmit.field.MODE = MODE_CCK;
			MinPhyTransmit.field.MCS = RATE_11;
			break;
		case PHY_RATE_6:
			MinPhyTransmit.field.MODE = MODE_OFDM;
			MinPhyTransmit.field.MCS = OfdmRateToRxwiMCS[RATE_6];
			break;
		case PHY_RATE_9:
			MinPhyTransmit.field.MODE = MODE_OFDM;
			MinPhyTransmit.field.MCS = OfdmRateToRxwiMCS[RATE_9];
			break;
		case PHY_RATE_12:
			MinPhyTransmit.field.MODE = MODE_OFDM;
			MinPhyTransmit.field.MCS = OfdmRateToRxwiMCS[RATE_12];
			break;
		case PHY_RATE_18:
			MinPhyTransmit.field.MODE = MODE_OFDM;
			MinPhyTransmit.field.MCS = OfdmRateToRxwiMCS[RATE_18];
			break;
		case PHY_RATE_24:
			MinPhyTransmit.field.MODE = MODE_OFDM;
			MinPhyTransmit.field.MCS = OfdmRateToRxwiMCS[RATE_24];
			break;
		case PHY_RATE_36:
			MinPhyTransmit.field.MODE = MODE_OFDM;
			MinPhyTransmit.field.MCS = OfdmRateToRxwiMCS[RATE_36];
			break;
		case PHY_RATE_48:
			MinPhyTransmit.field.MODE = MODE_OFDM;
			MinPhyTransmit.field.MCS = OfdmRateToRxwiMCS[RATE_48];
			break;
		case PHY_RATE_54:
			MinPhyTransmit.field.MODE = MODE_OFDM;
			MinPhyTransmit.field.MCS = OfdmRateToRxwiMCS[RATE_54];
			break;
		default:
			MinPhyTransmit.field.MODE = MODE_OFDM;
			MinPhyTransmit.field.MCS = OfdmRateToRxwiMCS[RATE_6];
			break;
		}

	return MinPhyTransmit;
}
#endif /* MIN_PHY_RATE_SUPPORT */

BOOLEAN wmode_valid_and_correct(RTMP_ADAPTER *pAd, UCHAR *wmode)
{
	BOOLEAN ret = TRUE;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (*wmode == WMODE_INVALID)
		*wmode = (WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN | WMODE_AC);

	while (1) {
		if (WMODE_CAP_5G(*wmode) && (!PHY_CAP_5G(cap->phy_caps)))
			*wmode = *wmode & ~(WMODE_A | WMODE_AN | WMODE_AC);
		else if (WMODE_CAP_2G(*wmode) && (!PHY_CAP_2G(cap->phy_caps)))
			*wmode = *wmode & ~(WMODE_B | WMODE_G | WMODE_GN);
		else if (WMODE_CAP_N(*wmode) && ((!PHY_CAP_N(cap->phy_caps)) ||
										 RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N)))
			*wmode = *wmode & ~(WMODE_GN | WMODE_AN);
		else if (WMODE_CAP_AC(*wmode) && (!PHY_CAP_AC(cap->phy_caps)))
			*wmode = *wmode & ~(WMODE_AC);

		if (*wmode == 0) {
			*wmode = (WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN | WMODE_AC);
			break;
		} else
			break;
	}

	return ret;
}

BOOLEAN wmode_band_equal(UCHAR smode, UCHAR tmode)
{
	BOOLEAN eq = FALSE;
	UCHAR *str1, *str2;

	if ((WMODE_CAP_5G(smode) == WMODE_CAP_5G(tmode)) &&
		(WMODE_CAP_2G(smode) == WMODE_CAP_2G(tmode)))
		eq = TRUE;

	str1 = wmode_2_str(smode);
	str2 = wmode_2_str(tmode);

	if (str1)
		os_free_mem(str1);

	if (str2)
		os_free_mem(str2);

	return eq;
}

UCHAR wmode_2_rfic(UCHAR PhyMode)
{
	if (WMODE_CAP_2G(PhyMode) && WMODE_CAP_5G(PhyMode))
		return RFIC_DUAL_BAND;
	else if (WMODE_CAP_2G(PhyMode))
		return RFIC_24GHZ;
	else if (WMODE_CAP_5G(PhyMode))
		return RFIC_5GHZ;

	return RFIC_24GHZ;
}

/*N9 CMD BW value*/
void bw_2_str(UCHAR bw, CHAR *bw_str)
{
	switch (bw) {
	case BW_20:
		sprintf(bw_str, "20");
		break;

	case BW_40:
		sprintf(bw_str, "20/40");
		break;

	case BW_80:
		sprintf(bw_str, "20/40/80");
		break;

	case BW_8080:
		sprintf(bw_str, "20/40/80/160NC");
		break;

	case BW_160:
		sprintf(bw_str, "20/40/80/160C");
		break;

	case BW_10:
		sprintf(bw_str, "10");
		break;

	case BW_5:
		sprintf(bw_str, "5");
		break;

	default:
		sprintf(bw_str, "Invaild");
		break;
	}
}

void extcha_2_str(UCHAR extcha, CHAR *ec_str)
{
	switch (extcha) {
	case EXTCHA_NONE:
		sprintf(ec_str, "NONE");
		break;

	case EXTCHA_ABOVE:
		sprintf(ec_str, "ABOVE");
		break;

	case EXTCHA_BELOW:
		sprintf(ec_str, "BELOW");
		break;

	case EXTCHA_NOASSIGN:
		sprintf(ec_str, "Not assignment");
		break;

	default:
		sprintf(ec_str, "Invaild");
		break;
	}
}

/*
    ==========================================================================
    Description:
	Set Wireless Mode
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/

INT RT_CfgSetWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg, struct wifi_dev *wdev)
{
	LONG cfg_mode;
	UCHAR wmode, *mode_str;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	cfg_mode = os_str_tol(arg, 0, 10);
	/* check if chip support 5G band when WirelessMode is 5G band */
	wmode = cfgmode_2_wmode((UCHAR)cfg_mode);

	if (!wmode_valid_and_correct(pAd, &wmode)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): Invalid wireless mode(%ld, wmode=0x%x), ChipCap(%s)\n",
				  __func__, cfg_mode, wmode,
				  BAND_STR[pChipCap->phy_caps & 0x3]));
		return FALSE;
	}

#ifdef DOT11_VHT_AC
#if defined(MT76x2) || defined(MT7637)

	if (pChipCap->ac_off_mode && WMODE_CAP_AC(wmode)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("it doesn't support VHT AC!\n"));
		wmode &= ~(WMODE_AC);
	}

#endif /* MT76x2 */
#endif /* DOT11_VHT_AC */

	if (wmode_band_equal(wdev->PhyMode, wmode) == TRUE)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wmode_band_equal(): Band Equal!\n"));
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wmode_band_equal(): Band Not Equal!\n"));

	wdev->PhyMode = wmode;
	pAd->CommonCfg.cfg_wmode = wmode;
	mode_str = wmode_2_str(wmode);

	if (mode_str) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Set WMODE=%s(0x%x)\n",
				 __func__, mode_str, wmode));
		os_free_mem(mode_str);
	}

	return TRUE;
}

/* maybe can be moved to GPL code, ap_mbss.c, but the code will be open */
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT

static BOOLEAN wmode_valid(RTMP_ADAPTER *pAd, enum WIFI_MODE wmode)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if ((WMODE_CAP_5G(wmode) && (!PHY_CAP_5G(cap->phy_caps))) ||
		(WMODE_CAP_2G(wmode) && (!PHY_CAP_2G(cap->phy_caps))) ||
		(WMODE_CAP_N(wmode) && RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N))
	   )
		return FALSE;
	else
		return TRUE;
}

/*
    ==========================================================================
    Description:
	Set Wireless Mode for MBSS
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT RT_CfgSetMbssWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT cfg_mode;
	UCHAR wmode;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	cfg_mode = os_str_tol(arg, 0, 10);
	wmode = cfgmode_2_wmode((UCHAR)cfg_mode);

	if ((wmode == WMODE_INVALID) || (!wmode_valid(pAd, wmode))) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): Invalid wireless mode(%d, wmode=0x%x), ChipCap(%s)\n",
				  __func__, cfg_mode, wmode,
				  BAND_STR[pChipCap->phy_caps & 0x3]));
		return FALSE;
	}

	if (WMODE_CAP_5G(wmode) && WMODE_CAP_2G(wmode)) {
		if (pAd->CommonCfg.dbdc_mode == 1) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("AP cannot support 2.4G/5G band mxied mode!\n"));
			return FALSE;
		}
	}

#ifdef DOT11_VHT_AC
#if defined(MT76x2) || defined(MT7637)

	if (pChipCap->ac_off_mode && WMODE_CAP_AC(wmode)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("it doesn't support VHT AC!\n"));
		wmode &= ~(WMODE_AC);
	}

#endif /* MT76x2 */
#endif /* DOT11_VHT_AC */
	pAd->CommonCfg.cfg_wmode = wmode;
	return TRUE;
}
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

static BOOLEAN RT_isLegalCmdBeforeInfUp(RTMP_STRING *SetCmd)
{
	BOOLEAN TestFlag;

	TestFlag =	!strcmp(SetCmd, "Debug") ||
#ifdef CONFIG_APSTA_MIXED_SUPPORT
				!strcmp(SetCmd, "OpMode") ||
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef EXT_BUILD_CHANNEL_LIST
				!strcmp(SetCmd, "CountryCode") ||
				!strcmp(SetCmd, "DfsType") ||
				!strcmp(SetCmd, "ChannelListAdd") ||
				!strcmp(SetCmd, "ChannelListShow") ||
				!strcmp(SetCmd, "ChannelListDel") ||
#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef SINGLE_SKU
				!strcmp(SetCmd, "ModuleTxpower") ||
#endif /* SINGLE_SKU */
				FALSE; /* default */
	return TestFlag;
}

INT RT_CfgSetShortSlot(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG ShortSlot;

	ShortSlot = os_str_tol(arg, 0, 10);

	if (ShortSlot == 1)
		pAd->CommonCfg.bUseShortSlotTime = TRUE;
	else if (ShortSlot == 0)
		pAd->CommonCfg.bUseShortSlotTime = FALSE;
	else
		return FALSE;  /*Invalid argument */

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set WEP KEY base on KeyIdx
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	RT_CfgSetWepKey(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *keyString,
	IN	CIPHER_KEY * pSharedKey,
	IN	INT				keyIdx)
{
	INT				KeyLen;
	INT				i;
	/*UCHAR			CipherAlg = CIPHER_NONE;*/
	BOOLEAN			bKeyIsHex = FALSE;
	/* TODO: Shall we do memset for the original key info??*/
	memset(pSharedKey, 0, sizeof(CIPHER_KEY));
	KeyLen = strlen(keyString);

	switch (KeyLen) {
	case 5: /*wep 40 Ascii type*/
	case 13: /*wep 104 Ascii type*/
#ifdef MT_MAC
	case 16: /*wep 128 Ascii type*/
#endif
		bKeyIsHex = FALSE;
		pSharedKey->KeyLen = KeyLen;
		NdisMoveMemory(pSharedKey->Key, keyString, KeyLen);
		break;

	case 10: /*wep 40 Hex type*/
	case 26: /*wep 104 Hex type*/
#ifdef MT_MAC
	case 32: /*wep 128 Hex type*/
#endif
		for (i = 0; i < KeyLen; i++) {
			if (!isxdigit(*(keyString + i)))
				return FALSE;  /*Not Hex value;*/
		}

		bKeyIsHex = TRUE;
		pSharedKey->KeyLen = KeyLen / 2;
		AtoH(keyString, pSharedKey->Key, pSharedKey->KeyLen);
		break;

	default: /*Invalid argument */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetWepKey(keyIdx=%d):Invalid argument (arg=%s)\n", keyIdx,
				 keyString));
		return FALSE;
	}

	pSharedKey->CipherAlg = ((KeyLen % 5) ? CIPHER_WEP128 : CIPHER_WEP64);
#ifdef MT_MAC

	if (KeyLen == 32)
		pSharedKey->CipherAlg = CIPHER_WEP152;

#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetWepKey:(KeyIdx=%d,type=%s, Alg=%s)\n",
			 keyIdx, (bKeyIsHex == FALSE ? "Ascii" : "Hex"), CipherName[pSharedKey->CipherAlg]));
	return TRUE;
}

INT	RT_CfgSetFixedTxPhyMode(RTMP_STRING *arg)
{
	INT fix_tx_mode = FIXED_TXMODE_HT;
	ULONG value;

	if (rtstrcasecmp(arg, "OFDM") == TRUE)
		fix_tx_mode = FIXED_TXMODE_OFDM;
	else if (rtstrcasecmp(arg, "CCK") == TRUE)
		fix_tx_mode = FIXED_TXMODE_CCK;
	else if (rtstrcasecmp(arg, "HT") == TRUE)
		fix_tx_mode = FIXED_TXMODE_HT;
	else if (rtstrcasecmp(arg, "VHT") == TRUE)
		fix_tx_mode = FIXED_TXMODE_VHT;
	else {
		value = os_str_tol(arg, 0, 10);

		switch (value) {
		case FIXED_TXMODE_CCK:
		case FIXED_TXMODE_OFDM:
		case FIXED_TXMODE_HT:
		case FIXED_TXMODE_VHT:
			fix_tx_mode = value;
			break;
		default:
			fix_tx_mode = FIXED_TXMODE_HT;
		}
	}

	return fix_tx_mode;
}

INT	RT_CfgSetMacAddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg, UCHAR idx)
{
	INT	i, mac_len;
	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(arg);

	if (mac_len != 17) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : invalid length (%d)\n", __func__, mac_len));
		return FALSE;
	}

	if (strcmp(arg, "00:00:00:00:00:00") == 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : invalid mac setting\n", __func__));
		return FALSE;
	}

	if (idx == 0) {
		for (i = 0; i < MAC_ADDR_LEN; i++) {
			AtoH(arg, &pAd->CurrentAddress[i], 1);
			arg = arg + 3;
		}

		pAd->bLocalAdminMAC = TRUE;
	}

#if defined(MT_MAC) && defined(MBSS_SUPPORT)
	else {
		for (i = 0; i < MAC_ADDR_LEN; i++) {
			AtoH(arg, &pAd->ExtendMBssAddr[idx - 1][i], 1);
			arg = arg + 3;
		}

		/* TODO: Carter, is the below code still has its meaning? */
		pAd->bLocalAdminExtendMBssMAC = TRUE;
	}

#endif
	return TRUE;
}

INT	RT_CfgSetTxMCSProc(RTMP_STRING *arg, BOOLEAN *pAutoRate)
{
	INT	Value = os_str_tol(arg, 0, 10);
	INT	TxMcs;

	if ((Value >= 0 && Value <= 23) || (Value == 32)) { /* 3*3*/
		TxMcs = Value;
		*pAutoRate = FALSE;
	} else {
		TxMcs = MCS_AUTO;
		*pAutoRate = TRUE;
	}

	return TxMcs;
}

INT	RT_CfgSetAutoFallBack(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR AutoFallBack = (UCHAR)os_str_tol(arg, 0, 10);

	if (AutoFallBack)
		AutoFallBack = TRUE;
	else
		AutoFallBack = FALSE;

	AsicSetAutoFallBack(pAd, (AutoFallBack) ? TRUE : FALSE);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetAutoFallBack::(AutoFallBack=%d)\n", AutoFallBack));
	return TRUE;
}

#ifdef WSC_INCLUDED
INT	RT_CfgSetWscPinCode(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *pPinCodeStr,
	OUT PWSC_CTRL   pWscControl)
{
	UINT pinCode;

	pinCode = (UINT) os_str_tol(pPinCodeStr, 0, 10); /* When PinCode is 03571361, return value is 3571361.*/

	if (strlen(pPinCodeStr) == 4) {
		pWscControl->WscEnrolleePinCode = pinCode;
		pWscControl->WscEnrolleePinCodeLen = 4;
	} else if (ValidateChecksum(pinCode)) {
		pWscControl->WscEnrolleePinCode = pinCode;
		pWscControl->WscEnrolleePinCodeLen = 8;
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RT_CfgSetWscPinCode(): invalid Wsc PinCode (%d)\n", pinCode));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetWscPinCode():Wsc PinCode=%d\n", pinCode));
	return TRUE;
}
#endif /* WSC_INCLUDED */

/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWNAME.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT RtmpIoctl_rt_ioctl_giwname(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID *pData,
	IN	ULONG					Data)
{
	UCHAR CurOpMode = OPMODE_AP;

	if (CurOpMode == OPMODE_AP) {
			strcpy(pData, "RTWIFI SoftAP");
	}

	return NDIS_STATUS_SUCCESS;
}

VOID rtmp_chip_prepare(RTMP_ADAPTER *pAd)
{
}

extern struct wifi_dev_ops ap_wdev_ops;
extern struct wifi_dev_ops sta_wdev_ops;

static VOID rtmp_netdev_set(RTMP_ADAPTER *pAd, PNET_DEV net_dev)
{
	struct wifi_dev *wdev = NULL;
	INT32 ret = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* set main net_dev */
	pAd->net_dev = net_dev;
#ifdef CONFIG_AP_SUPPORT

	if (pAd->OpMode == OPMODE_AP) {
		BSS_STRUCT *pMbss;

		pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
		ASSERT(pMbss);
		wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
		RTMP_OS_NETDEV_SET_WDEV(net_dev, wdev);
		ret = wdev_init(pAd, wdev, WDEV_TYPE_AP, net_dev, MAIN_MBSSID, (VOID *)pMbss, (VOID *)pAd);

		if (!ret) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
			RtmpOSNetDevFree(pAd->net_dev);
			return;
		}

		ret = wdev_ops_register(wdev, WDEV_TYPE_AP, &ap_wdev_ops,
								cap->qos.wmm_detect_method);

		if (!ret) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("register wdev_ops %s failed, free net device!\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
			RtmpOSNetDevFree(pAd->net_dev);
			return;
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (pAd->OpMode == OPMODE_STA) {
		wdev = &pAd->StaCfg[MAIN_MSTA_ID].wdev;
		RTMP_OS_NETDEV_SET_WDEV(net_dev, wdev);
		ret = wdev_init(pAd, wdev, WDEV_TYPE_STA, net_dev, MAIN_MSTA_ID, (VOID *)&pAd->StaCfg[MAIN_MSTA_ID], (VOID *)pAd);

		if (!ret) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
			RtmpOSNetDevFree(pAd->net_dev);
			return;
		}

		ret = wdev_ops_register(wdev, WDEV_TYPE_STA, &sta_wdev_ops,
								cap->qos.wmm_detect_method);

		if (!ret) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("register wdev_ops %s failed, free net device!\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
			RtmpOSNetDevFree(pAd->net_dev);
			return;
		}
	}

#endif /* CONFIG_STA_SUPPORT */
}

INT RTMP_COM_IoctlHandle(
	IN	VOID					*pAdSrc,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT Status = NDIS_STATUS_SUCCESS, i;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
		wdev = &pStaCfg->wdev;
	}
#endif
	pObj = pObj; /* avoid compile warning */

	switch (cmd) {
	case CMD_RTPRIV_IOCTL_NETDEV_GET:
		/* get main net_dev */
	{
		VOID **ppNetDev = (VOID **)pData;
		*ppNetDev = (VOID *)(pAd->net_dev);
	}
	break;
#ifdef APCLI_CFG80211_SUPPORT
	case CMD_RTPRIV_IOCTL_APCLI_NETDEV_GET:
		/*get apcli net dev for CFG80211 mode */
	{
		VOID **ppNetDev = (VOID **)pData;
		*ppNetDev = (VOID *)(pAd->StaCfg[0].wdev.if_dev);
	}
	break;
#endif /* APCLI_CFG80211_SUPPORT */

	case CMD_RTPRIV_IOCTL_NETDEV_SET: {
		rtmp_netdev_set(pAd, pData);
		break;
	}

	case CMD_RTPRIV_IOCTL_OPMODE_GET:
		/* get Operation Mode */
		*(ULONG *)pData = pAd->OpMode;
		break;

	case CMD_RTPRIV_IOCTL_TASK_LIST_GET:
		/* get all Tasks */
	{
		RT_CMD_WAIT_QUEUE_LIST *pList = (RT_CMD_WAIT_QUEUE_LIST *)pData;

		pList->pMlmeTask = &pAd->mlmeTask;
#ifdef RTMP_TIMER_TASK_SUPPORT
		pList->pTimerTask = &pAd->timerTask;
#endif /* RTMP_TIMER_TASK_SUPPORT */
		pList->pCmdQTask = &pAd->cmdQTask;
#ifdef WSC_INCLUDED
		pList->pWscTask = &pAd->wscTask;
#endif /* WSC_INCLUDED */
	}
	break;
#ifdef RTMP_MAC_PCI

	case CMD_RTPRIV_IOCTL_IRQ_INIT:
		/* init IRQ */
		irq_init(pAd);
		break;
#endif /* RTMP_MAC_PCI */

	case CMD_RTPRIV_IOCTL_IRQ_RELEASE:
		/* release IRQ */
		RTMP_OS_IRQ_RELEASE(pAd, pAd->net_dev);
		break;

	case CMD_RTPRIV_IOCTL_NIC_NOT_EXIST:
		/* set driver state to fRTMP_ADAPTER_NIC_NOT_EXIST */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
		break;

	case CMD_RTPRIV_IOCTL_MCU_SLEEP_CLEAR:
		RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
		break;
#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

	case CMD_RTPRIV_IOCTL_USB_DEV_GET:
		/* get USB DEV */
	{
		VOID **ppUsb_Dev = (VOID **)pData;
		*ppUsb_Dev = (VOID *)(pObj->pUsb_Dev);
	}
	break;

	case CMD_RTPRIV_IOCTL_USB_INTF_GET:
		/* get USB INTF */
	{
		VOID **ppINTF = (VOID **)pData;
		*ppINTF = (VOID *)(pObj->intf);
	}
	break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET:
		/* set driver state to fRTMP_ADAPTER_SUSPEND */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR:
		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
		RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SEND_DISSASSOCIATE:

		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
		if (INFRA_ON(pStaCfg) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))) {
			MLME_DISASSOC_REQ_STRUCT	DisReq;
			MLME_QUEUE_ELEM *MsgElem;

			os_alloc_mem(NULL, (UCHAR **)&MsgElem, sizeof(MLME_QUEUE_ELEM));

			if (MsgElem) {
				COPY_MAC_ADDR(DisReq.Addr, pAd->CommonCfg.Bssid);
				DisReq.Reason =  REASON_DEAUTH_STA_LEAVING;
				MsgElem->Machine = ASSOC_FSM;
				MsgElem->MsgType = ASSOC_FSM_MLME_DISASSOC_REQ;
				MsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
				NdisMoveMemory(MsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));
				/* Prevent to connect AP again in STAMlmePeriodicExec*/
				pAd->MlmeAux.AutoReconnectSsidLen = 32;
				NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
				pStaCfg->CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;
				MlmeDisassocReqAction(pAd, MsgElem);
				os_free_mem(MsgElem);
			}

			/*				RtmpusecDelay(1000);*/
			RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CGIWAP, -1, NULL, NULL, 0);
		}

		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_TEST:
		/* test driver state to fRTMP_ADAPTER_SUSPEND */
		*(UCHAR *)pData = RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_IDLE_RADIO_OFF_TEST:
		/* test driver state to fRTMP_ADAPTER_IDLE_RADIO_OFF */
		*(UCHAR *)pData = RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_OFF:
		ASIC_RADIO_OFF(pAd, SUSPEND_RADIO_OFF);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_ON:
		ASIC_RADIO_ON(pAd, RESUME_RADIO_ON);
		break;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_STATUS:
		*(UCHAR *)pData = (UCHAR)pAd->WOW_Cfg.bEnable;
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_ENABLE:
		ASIC_WOW_ENABLE(pAd, pStaCfg);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_DISABLE:
		ASIC_WOW_DISABLE(pAd, pStaCfg);
		break;
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */
#endif /* CONFIG_PM */
#ifdef CONFIG_STA_SUPPORT
	case CMD_RTPRIV_IOCTL_AP_BSSID_GET:
		if (pStaCfg && pStaCfg->wdev.PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			NdisCopyMemory(pData, pStaCfg->MlmeAux.Bssid, 6);
		else
			return NDIS_STATUS_FAILURE;

		break;
#endif
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET:
		/* set driver state to fRTMP_ADAPTER_SUSPEND */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR:
		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		break;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_OFF:
		ASIC_RADIO_OFF(pAd, SUSPEND_RADIO_OFF);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_ON:
		ASIC_RADIO_ON(pAd, RESUME_RADIO_ON);
		break;
#endif /* CONFIG_STA_SUPPORT */

	case CMD_RTPRIV_IOCTL_SANITY_CHECK:

		/* sanity check before IOCTL */
		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
#ifdef IFUP_IN_PROBE
			|| (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
			|| (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
#endif /* IFUP_IN_PROBE */
		   ) {
			if (pData == NULL ||	RT_isLegalCmdBeforeInfUp((RTMP_STRING *) pData) == FALSE)
				return NDIS_STATUS_FAILURE;
		}

		break;

	case CMD_RTPRIV_IOCTL_SIOCGIWFREQ:
		/* get channel number */
#ifdef WDS_SUPPORT
		if (pObj->ioctl_if_type == INT_WDS)
			wdev = get_wdev_by_idx(pAd, (INT)Data + MIN_NET_DEVICE_FOR_WDS);
		else
#endif
#ifdef APCLI_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI)
				wdev = get_wdev_by_idx(pAd, (INT)Data + MIN_NET_DEVICE_FOR_APCLI);
			else
#endif
				wdev = get_wdev_by_idx(pAd, (INT)Data);

		if (wdev)
			*(ULONG *)pData = wdev->channel;

		break;
#ifdef SNIFFER_SUPPORT

	case CMD_RTPRIV_IOCTL_SNIFF_INIT:
		Monitor_Init(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_SNIFF_OPEN:
		if (Monitor_Open(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_SNIFF_CLOSE:
		if (Monitor_Close(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_SNIFF_REMOVE:
		Monitor_Remove(pAd);
		break;
#endif /*SNIFFER_SUPPORT*/

	case CMD_RTPRIV_IOCTL_BEACON_UPDATE:
		/* update all beacon contents */
#ifdef CONFIG_AP_SUPPORT
		/* TODO: Carter, the oid seems been obsoleted. */
		UpdateBeaconHandler(
			pAd,
			NULL,
			BCN_UPDATE_AP_RENEW);
#endif /* CONFIG_AP_SUPPORT */
		break;

	case CMD_RTPRIV_IOCTL_RXPATH_GET:
		/* get the number of rx path */
		*(ULONG *)pData = pAd->Antenna.field.RxPath;
		break;

	case CMD_RTPRIV_IOCTL_CHAN_LIST_NUM_GET:
		if (wdev)
			BandIdx = HcGetBandByWdev(wdev);
		else {
			BandIdx = BAND0;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[CMD_RTPRIV_IOCTL_CHAN_LIST_NUM_GET] wdev = NULL\n"));
		}
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		*(ULONG *)pData = pChCtrl->ChListNum;
		break;

	case CMD_RTPRIV_IOCTL_CHAN_LIST_GET: {
		UINT32 i;
		UCHAR *pChannel = (UCHAR *)pData;
		if (wdev)
			BandIdx = HcGetBandByWdev(wdev);
		else {
			BandIdx = BAND0;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[CMD_RTPRIV_IOCTL_CHAN_LIST_GET] wdev = NULL\n"));
		}
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

		for (i = 1; i <= pChCtrl->ChListNum; i++) {
			*pChannel = pChCtrl->ChList[i - 1].Channel;
			pChannel++;
		}
	}
	break;

	case CMD_RTPRIV_IOCTL_FREQ_LIST_GET: {
		UINT32 i;
		UINT32 *pFreq = (UINT32 *)pData;
		UINT32 m;
		if (wdev)
			BandIdx = HcGetBandByWdev(wdev);
		else {
			BandIdx = BAND0;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[CMD_RTPRIV_IOCTL_FREQ_LIST_GET] wdev = NULL\n"));
		}
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

		for (i = 1; i <= pChCtrl->ChListNum; i++) {
			m = 2412000;
			MAP_CHANNEL_ID_TO_KHZ(pChCtrl->ChList[i - 1].Channel, m);
			(*pFreq) = m;
			pFreq++;
		}
	}
	break;
#ifdef EXT_BUILD_CHANNEL_LIST

	case CMD_RTPRIV_SET_PRECONFIG_VALUE:
		/* Set some preconfigured value before interface up*/
		pAd->CommonCfg.DfsType = MAX_RD_REGION;
		break;
#endif /* EXT_BUILD_CHANNEL_LIST */

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
	case CMD_RTPRIV_IOCTL_PCI_SUSPEND:
	case CMD_RTPRIV_IOCTL_RBUS_SUSPEND:
#ifdef CONFIG_STA_SUPPORT
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

#ifdef MT_WOW_SUPPORT
		if ((pAd->WOW_Cfg.bEnable == TRUE) && INFRA_ON(pStaCfg)) {
			BOOLEAN InfraAP_BW = FALSE;
			UCHAR BwFallBack = 0;

			if (pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40)
				InfraAP_BW = TRUE;
			else
				InfraAP_BW = FALSE;

			AdjustChannelRelatedValue(pAd,
									  &BwFallBack,
									  BSS0,
									  InfraAP_BW,
									  pStaCfg->MlmeAux.Channel,
									  pStaCfg->MlmeAux.CentralChannel,
									  wdev);
			pAd->WOW_Cfg.bWoWRunning = TRUE;
			ASIC_WOW_ENABLE(pAd, pStaCfg);
			AsicExtPmStateCtrl(pAd, pStaCfg, PM4, ENTER_PM_STATE);
		}
#endif /*MT_WOW_SUPPORT */

#ifdef MT_WOW_SUPPORT
		if (!pAd->WOW_Cfg.bWoWRunning)
#endif /* MT_WOW_SUPPORT */
		{
			MlmeRadioOff(pAd, wdev);
		}

		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
		RTMP_ASIC_INTERRUPT_DISABLE(pAd);
		AsicSetWPDMA(pAd, PDMA_TX_RX, 0);
#ifdef CONFIG_FWOWN_SUPPORT
		FwOwn(pAd);
#endif /* CONFIG_FWOWN_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
		break;

	case CMD_RTPRIV_IOCTL_PCI_RESUME:
	case CMD_RTPRIV_IOCTL_RBUS_RESUME:
#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_FWOWN_SUPPORT
		DriverOwn(pAd);
#endif /* CONFIG_FWOWN_SUPPORT */
		AsicSetWPDMA(pAd, PDMA_TX_RX, 1);
		RTMP_ASIC_INTERRUPT_ENABLE(pAd);
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef MT_WOW_SUPPORT
		if (!pAd->WOW_Cfg.bWoWRunning)
#endif /* MT_WOW_SUPPORT */
		{
			MlmeRadioOn(pAd, wdev);
		}

#ifdef MT_WOW_SUPPORT
		if (pAd->WOW_Cfg.bWoWRunning == TRUE) {
			AsicExtPmStateCtrl(pAd, pStaCfg, PM4, EXIT_PM_STATE);
			ASIC_WOW_DISABLE(pAd, pStaCfg);
			pAd->WOW_Cfg.bWoWRunning = FALSE;
		}
#endif /* MT_WOW_SUPPORT */

		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

		/* Delete all BA sessions after resume */
		if (pStaCfg)
			ba_session_tear_down_all(pAd, pStaCfg->PwrMgmt.ucWcid);

#ifdef HOST_RESUME_DONE_ACK_SUPPORT
		rtmp_host_resume_done_ack(pAd);
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
		break;
#endif /* RTMP_PCI_SUPPORT || RTMP_RBUS_SUPPORT */

#if defined(RTMP_PCI_SUPPORT)
	case CMD_RTPRIV_IOCTL_PCI_CSR_SET:
		pAd->PciHif.CSRBaseAddress = (PUCHAR)Data;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->PciHif.CSRBaseAddress =0x%lx, csr_addr=0x%lx!\n",
				 (ULONG)pAd->PciHif.CSRBaseAddress, (ULONG)Data));
		break;

	case CMD_RTPRIV_IOCTL_PCIE_INIT:
		RTMPInitPCIeDevice(pData, pAd);
		break;
#endif /* RTMP_PCI_SUPPORT */


	case CMD_RTPRIV_IOCTL_CHIP_PREPARE: {
		rtmp_chip_prepare(pAd);
	}
	break;
#ifdef RT_CFG80211_SUPPORT

	case CMD_RTPRIV_IOCTL_CFG80211_CFG_START:
		if (wdev)
			RT_CFG80211_REINIT(pAd, wdev);

#ifndef DISABLE_HOSTAPD_BEACON
		RT_CFG80211_CRDA_REG_RULE_APPLY(pAd);
#endif
		break;
#endif /* RT_CFG80211_SUPPORT */
#ifdef INF_PPA_SUPPORT

	case CMD_RTPRIV_IOCTL_INF_PPA_INIT:
		os_alloc_mem(NULL, (UCHAR **) &(pAd->pDirectpathCb), sizeof(PPA_DIRECTPATH_CB));
		break;

	case CMD_RTPRIV_IOCTL_INF_PPA_EXIT:
		if (ppa_hook_directpath_register_dev_fn && (pAd->PPAEnable == TRUE)) {
			UINT status;

			status = ppa_hook_directpath_register_dev_fn(&pAd->g_if_id, pAd->net_dev, NULL, 0);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Unregister PPA::status=%d, if_id=%d\n", status, pAd->g_if_id));
		}

		os_free_mem(pAd->pDirectpathCb);
		break;
#endif /* INF_PPA_SUPPORT*/

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_UP:
		/* interface up */
	{
		RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s -> CMD_RTPRIV_IOCTL_VIRTUAL_INF_UP\n", __func__));
		pInfConf->virtual_if_up_handler(pInfConf->operation_dev_p);

	}
	break;

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_DOWN:
		/* interface down */
	{
		RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s -> CMD_RTPRIV_IOCTL_VIRTUAL_INF_DOWN\n", __func__));
		VIRTUAL_IF_DEC(pAd);
		pInfConf->virtual_if_down_handler(pInfConf->operation_dev_p);
	}
	break;

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_INIT:
		/* init at first interface up */
	{
		RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s -> CMD_RTPRIV_IOCTL_VIRTUAL_INF_INIT\n", __func__));
		pInfConf->virtual_if_init_handler(pInfConf->operation_dev_p);

	}
	break;

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_DEINIT:
		/* deinit at last interface down */
	{
		RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s -> CMD_RTPRIV_IOCTL_VIRTUAL_INF_DEINIT\n", __func__));
		pInfConf->virtual_if_deinit_handler(pInfConf->operation_dev_p);
	}
	break;

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_GET:
		/* get virtual interface number */
		*(ULONG *)pData = VIRTUAL_IF_NUM(pAd);
		break;

	case CMD_RTPRIV_IOCTL_INF_TYPE_GET:
		/* get current interface type */
		*(ULONG *)pData = pAd->infType;
		break;

	case CMD_RTPRIV_IOCTL_INF_STATS_GET:
		/* get statistics */
	{
		RT_CMD_STATS *pStats = (RT_CMD_STATS *)pData;

		pStats->pStats = pAd->stats;

		if (pAd->OpMode == OPMODE_STA) {
			pStats->rx_packets = pAd->WlanCounters[0].ReceivedFragmentCount.QuadPart;
			pStats->tx_packets = pAd->WlanCounters[0].TransmittedFragmentCount.QuadPart;
			pStats->rx_bytes = pAd->RalinkCounters.ReceivedByteCount;
			pStats->tx_bytes = pAd->RalinkCounters.TransmittedByteCount;
			pStats->rx_errors = pAd->Counters8023.RxErrors;
			pStats->tx_errors = pAd->Counters8023.TxErrors;
			pStats->multicast = pAd->WlanCounters[0].MulticastReceivedFrameCount.QuadPart;   /* multicast packets received*/
			pStats->collisions = 0;  /* Collision packets*/
			pStats->rx_over_errors = pAd->Counters8023.RxNoBuffer;                   /* receiver ring buff overflow*/
			pStats->rx_crc_errors = 0;/*pAd->WlanCounters[0].FCSErrorCount;      recved pkt with crc error*/
			pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
			pStats->rx_fifo_errors = pAd->Counters8023.RxNoBuffer;                   /* recv'r fifo overrun*/
		}

#ifdef CONFIG_AP_SUPPORT
		else if (pAd->OpMode == OPMODE_AP) {
			INT index;
			BOOLEAN found_it = FALSE;
			INT stat_db_source;

			for (index = 0; index < MAX_MBSSID_NUM(pAd); index++) {
				if (pAd->ApCfg.MBSSID[index].wdev.if_dev == (PNET_DEV)(pStats->pNetDev)) {
					found_it = TRUE;
					stat_db_source = 0;
					break;
				}
			}
#ifdef CONFIG_STA_SUPPORT
					if (found_it == FALSE) {
						index = 0;
						if (pAd->StaCfg[index].wdev.if_dev == (PNET_DEV)(pStats->pNetDev)) {
							found_it = TRUE;
							stat_db_source = 1;
						}
					}
#endif /* CONFIG_STA_SUPPORT */
#ifdef APCLI_SUPPORT

			if (found_it == FALSE) {
				for (index = 0; index < pAd->ApCfg.ApCliNum; index++) {
					if (pAd->StaCfg[index].wdev.if_dev == (PNET_DEV)(pStats->pNetDev)) {
						found_it = TRUE;
						stat_db_source = 1;
						break;
					}
				}
			}

#endif

			if (found_it == FALSE) {
				/* reset counters */
				pStats->rx_packets = 0;
				pStats->tx_packets = 0;
				pStats->rx_bytes = 0;
				pStats->tx_bytes = 0;
				pStats->rx_errors = 0;
				pStats->tx_errors = 0;
				pStats->multicast = 0;   /* multicast packets received*/
				pStats->collisions = 0;  /* Collision packets*/
				pStats->rx_over_errors = 0; /* receiver ring buff overflow*/
				pStats->rx_crc_errors = 0; /* recved pkt with crc error*/
				pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
				pStats->rx_fifo_errors = 0; /* recv'r fifo overrun*/
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CMD_RTPRIV_IOCTL_INF_STATS_GET: can not find mbss I/F\n"));
				return NDIS_STATUS_FAILURE;
			}

			if (stat_db_source == 0) {
				pStats->rx_packets = pAd->ApCfg.MBSSID[index].RxCount;
				pStats->tx_packets = pAd->ApCfg.MBSSID[index].TxCount;
				pStats->rx_bytes = pAd->ApCfg.MBSSID[index].ReceivedByteCount;
				pStats->tx_bytes = pAd->ApCfg.MBSSID[index].TransmittedByteCount;
				pStats->rx_errors = pAd->ApCfg.MBSSID[index].RxErrorCount;
				pStats->tx_errors = pAd->ApCfg.MBSSID[index].TxErrorCount;
				pStats->multicast = pAd->ApCfg.MBSSID[index].mcPktsRx; /* multicast packets received */
				pStats->collisions = 0;  /* Collision packets*/
				pStats->rx_over_errors = 0;                   /* receiver ring buff overflow*/
				pStats->rx_crc_errors = 0;/* recved pkt with crc error*/
				pStats->rx_frame_errors = 0;          /* recv'd frame alignment error*/
				pStats->rx_fifo_errors = 0;                   /* recv'r fifo overrun*/
			}
			else if (stat_db_source == 1) {
#ifdef CONFIG_STA_SUPPORT
				pStats->rx_packets = pAd->StaCfg[index].StaStatistic.RxCount;
				pStats->tx_packets = pAd->StaCfg[index].StaStatistic.TxCount;
				pStats->rx_bytes = pAd->StaCfg[index].StaStatistic.ReceivedByteCount;
				pStats->tx_bytes = pAd->StaCfg[index].StaStatistic.TransmittedByteCount;
				pStats->rx_errors = pAd->StaCfg[index].StaStatistic.RxErrorCount;
				pStats->tx_errors = pAd->StaCfg[index].StaStatistic.TxErrorCount;
				pStats->multicast = pAd->StaCfg[index].StaStatistic.mcPktsRx; /* multicast packets received */
				pStats->collisions = 0; /* Collision packets*/
				pStats->rx_over_errors = 0; /* receiver ring buff overflow*/
				pStats->rx_crc_errors = 0;/* recved pkt with crc error*/
				pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
				pStats->rx_fifo_errors = 0;
#endif /* CONFIG_STA_SUPPORT */
#ifdef APCLI_SUPPORT
				pStats->rx_packets = pAd->StaCfg[index].StaStatistic.RxCount;
				pStats->tx_packets = pAd->StaCfg[index].StaStatistic.TxCount;
				pStats->rx_bytes = pAd->StaCfg[index].StaStatistic.ReceivedByteCount;
				pStats->tx_bytes = pAd->StaCfg[index].StaStatistic.TransmittedByteCount;
				pStats->rx_errors = pAd->StaCfg[index].StaStatistic.RxErrorCount;
				pStats->tx_errors = pAd->StaCfg[index].StaStatistic.TxErrorCount;
				pStats->multicast = pAd->StaCfg[index].StaStatistic.mcPktsRx; /* multicast packets received */
				pStats->collisions = 0; /* Collision packets*/
				pStats->rx_over_errors = 0; /* receiver ring buff overflow*/
				pStats->rx_crc_errors = 0;/* recved pkt with crc error*/
				pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
				pStats->rx_fifo_errors = 0; /* recv'r fifo overrun*/
#endif
			}

		}

#endif
	}
	break;

	case CMD_RTPRIV_IOCTL_INF_IW_STATUS_GET:

		/* get wireless statistics */
	{
		UCHAR CurOpMode = OPMODE_AP;
#ifdef CONFIG_AP_SUPPORT
		PMAC_TABLE_ENTRY pMacEntry = NULL;
#endif /* CONFIG_AP_SUPPORT */
		RT_CMD_IW_STATS *pStats = (RT_CMD_IW_STATS *)pData;

		pStats->qual = 0;
		pStats->level = 0;
		pStats->noise = 0;
		pStats->pStats = pAd->iw_stats;
#ifdef CONFIG_STA_SUPPORT

		if (pAd->OpMode == OPMODE_STA) {
			CurOpMode = OPMODE_STA;
		}

#endif /* CONFIG_STA_SUPPORT */

		/*check if the interface is down*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
			return NDIS_STATUS_FAILURE;

#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == OPMODE_AP) {
#ifdef APCLI_SUPPORT

			if ((pStats->priv_flags == INT_APCLI)
			   ) {
				INT ApCliIdx = ApCliIfLookUp(pAd, (PUCHAR)pStats->dev_addr);

				if ((ApCliIdx >= 0) && VALID_WCID(pAd->StaCfg[ApCliIdx].MacTabWCID))
					pMacEntry = &pAd->MacTab.Content[pAd->StaCfg[ApCliIdx].MacTabWCID];
			} else
#endif /* APCLI_SUPPORT */
			{
				/*
					only AP client support wireless stats function.
					return NULL pointer for all other cases.
				*/
				pMacEntry = NULL;
			}
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

		if (CurOpMode == OPMODE_STA)
			pStats->qual = ((pStaCfg->ChannelQuality * 12) / 10 + 10);

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == OPMODE_AP) {
			if (pMacEntry != NULL)
				pStats->qual = ((pMacEntry->ChannelQuality * 12) / 10 + 10);
			else
				pStats->qual = ((pAd->Mlme.ChannelQuality * 12) / 10 + 10);
		}

#endif /* CONFIG_AP_SUPPORT */

		if (pStats->qual > 100)
			pStats->qual = 100;

#ifdef CONFIG_STA_SUPPORT

		if (CurOpMode == OPMODE_STA) {
			pStats->level =
				RTMPMaxRssi(pAd, pStaCfg->RssiSample.AvgRssi[0],
							pStaCfg->RssiSample.AvgRssi[1],
							pStaCfg->RssiSample.AvgRssi[2]);
			pStats->noise = RTMPMaxRssi(pAd, pStaCfg->RssiSample.AvgRssi[0],
									pStaCfg->RssiSample.AvgRssi[1],
									pStaCfg->RssiSample.AvgRssi[2]) -
						RTMPMinSnr(pAd, pAd->StaCfg[0].RssiSample.AvgSnr[0],
								   pStaCfg->RssiSample.AvgSnr[1]);
		}

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == OPMODE_AP) {
			if (pMacEntry != NULL)
				pStats->level =
					RTMPMaxRssi(pAd, pMacEntry->RssiSample.AvgRssi[0],
								pMacEntry->RssiSample.AvgRssi[1],
								pMacEntry->RssiSample.AvgRssi[2]);

			pStats->noise = RTMPMaxRssi(pAd, pAd->ApCfg.RssiSample.AvgRssi[0],
									pAd->ApCfg.RssiSample.AvgRssi[1],
									pAd->ApCfg.RssiSample.AvgRssi[2]) -
						RTMPMinSnr(pAd, pAd->ApCfg.RssiSample.AvgSnr[0],
								   pAd->ApCfg.RssiSample.AvgSnr[1]);
		}

#endif /* CONFIG_AP_SUPPORT */

	}
	break;

	case CMD_RTPRIV_IOCTL_INF_MAIN_CREATE:
		*(VOID **)pData = RtmpPhyNetDevMainCreate(pAd);
		break;

	case CMD_RTPRIV_IOCTL_INF_MAIN_ID_GET:
		*(ULONG *)pData = INT_MAIN;
		break;

	case CMD_RTPRIV_IOCTL_INF_MAIN_CHECK:
		if (Data != INT_MAIN)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_INF_P2P_CHECK:
		if (Data != INT_P2P)
			return NDIS_STATUS_FAILURE;

		break;
#ifdef WDS_SUPPORT

	case CMD_RTPRIV_IOCTL_WDS_INIT:
		WDS_Init(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_WDS_REMOVE:
		WDS_Remove(pAd);
		break;

	case CMD_RTPRIV_IOCTL_WDS_STATS_GET:
		if (Data == INT_WDS) {
			if (WDS_StatsGet(pAd, pData) != TRUE)
				return NDIS_STATUS_FAILURE;
		} else
			return NDIS_STATUS_FAILURE;

		break;
#endif /* WDS_SUPPORT */
#ifdef CONFIG_ATE
#ifdef CONFIG_QA

	case (CMD_RTPRIV_IOCTL_COMMON)CMD_RTPRIV_IOCTL_ATE: {
		struct _HQA_CMD_FRAME *HqaCmdFrame;

		os_alloc_mem_suspend(pAd, (UCHAR **)&HqaCmdFrame, sizeof(*HqaCmdFrame));

		if (!HqaCmdFrame) {
			Status = -ENOMEM;
			break;
		}

		NdisZeroMemory(HqaCmdFrame, sizeof(*HqaCmdFrame));
		Status = copy_from_user((PUCHAR)HqaCmdFrame, wrq->u.data.pointer, wrq->u.data.length);

		if (Status)	{
			Status = -EFAULT;
			goto IOCTL_ATE_ERROR;
		}

		Status = HQA_CMDHandler(pAd, wrq, HqaCmdFrame);
		/* TODO: Check sanity */
IOCTL_ATE_ERROR:
		os_free_mem(HqaCmdFrame);
	}
	break;
#endif /* CONFIG_QA */
#endif /* CONFIG_ATE */

	case CMD_RTPRIV_IOCTL_MAC_ADDR_GET: {
		UCHAR mac_addr[MAC_ADDR_LEN];
		USHORT Addr01, Addr23, Addr45;

		RT28xx_EEPROM_READ16(pAd, 0x04, Addr01);
		RT28xx_EEPROM_READ16(pAd, 0x06, Addr23);
		RT28xx_EEPROM_READ16(pAd, 0x08, Addr45);
		mac_addr[0] = (UCHAR)(Addr01 & 0xff);
		mac_addr[1] = (UCHAR)(Addr01 >> 8);
		mac_addr[2] = (UCHAR)(Addr23 & 0xff);
		mac_addr[3] = (UCHAR)(Addr23 >> 8);
		mac_addr[4] = (UCHAR)(Addr45 & 0xff);
		mac_addr[5] = (UCHAR)(Addr45 >> 8);

		for (i = 0; i < 6; i++)
			*(UCHAR *)(pData + i) = mac_addr[i];

		break;
	}

#ifdef CONFIG_AP_SUPPORT

	case CMD_RTPRIV_IOCTL_SIOCGIWRATE:
		/* handle for SIOCGIWRATEQ */
	{
		RT_CMD_IOCTL_RATE *pRate = (RT_CMD_IOCTL_RATE *)pData;
		HTTRANSMIT_SETTING HtPhyMode;
		UINT8 BW;
		UINT8 Antenna = 0;
		USHORT MCS;
		struct wifi_dev	*wdev = NULL;
#ifdef APCLI_SUPPORT
		MAC_TABLE_ENTRY	*pEntry = NULL;
#endif /* APCLI_SUPPORT */
#ifdef APCLI_SUPPORT
		if (pRate->priv_flags == INT_APCLI) {
			if (pObj->ioctl_if >= MAX_MULTI_STA) {
				Status = NDIS_STATUS_FAILURE;
				MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s -> CMD_RTPRIV_IOCTL_SIOCGIWRATE fail\n", __func__));
				break;
			}
			pEntry = MacTableLookup2(pAd, pAd->StaCfg[pObj->ioctl_if].wdev.bssid,
							 &pAd->StaCfg[pObj->ioctl_if].wdev);

			if (!pEntry) {/* show maximum capability */
				HtPhyMode = pAd->StaCfg[pObj->ioctl_if].wdev.HTPhyMode;
			} else {
				HtPhyMode = pEntry->HTPhyMode;

				if (HtPhyMode.field.MODE == MODE_VHT)
					Antenna = (HtPhyMode.field.MCS >> 4) + 1;
			}

				wdev = &pAd->StaCfg[pObj->ioctl_if].wdev;
		} else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
		if (pRate->priv_flags == INT_WDS) {
			HtPhyMode = pAd->WdsTab.WdsEntry[pObj->ioctl_if].wdev.HTPhyMode;
			wdev = &pAd->WdsTab.WdsEntry[pObj->ioctl_if].wdev;
		} else
#endif /* WDS_SUPPORT */
		{
			HtPhyMode = pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.HTPhyMode;
			wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
		}

		if (!wdev || !wdev->DevInfo.WdevActive)
			break;
		MCS = HtPhyMode.field.MCS;
#ifdef DOT11_N_SUPPORT
		if ((HtPhyMode.field.MODE == MODE_HTMIX)
			|| (HtPhyMode.field.MODE == MODE_HTGREENFIELD)) {
			Antenna = (HtPhyMode.field.MCS >> 3) + 1;
			MCS = MCS & 0xffff;
		}

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
		if (HtPhyMode.field.MODE == MODE_VHT)
			MCS = MCS & 0xf;

		if (HtPhyMode.field.MODE >= MODE_VHT) {
			BW = wlan_operate_get_vht_bw(wdev);

			if ((BW == 0) && wlan_operate_get_ht_bw(wdev)) /* VHT40 */
				BW = 1;
			else if (BW == 1) /* VHT80 */
				BW = 2;
			else if (BW >= 2) /* VHT80-80,VHT160 */
				BW = 3;
		} else
#endif /*DOT11_VHT_AC*/
		if (HtPhyMode.field.MODE >= MODE_HTMIX)
			BW = wlan_operate_get_ht_bw(wdev);
		else
			BW = HtPhyMode.field.BW;
		if (Antenna == 0)
			Antenna = wlan_config_get_tx_stream(wdev);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HtPhyMode.field.MODE=%d\n\r", HtPhyMode.field.MODE));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HtPhyMode.field.ShortGI=%d\n\r", HtPhyMode.field.ShortGI));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HtPhyMode.field.BW=%d\n\r", HtPhyMode.field.BW));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HtPhyMode.field.MCS=%d\n\r", MCS));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BW=%d\n\r", BW));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Antenna=%d\n\r", Antenna));
		RtmpDrvMaxRateGet(pAd, HtPhyMode.field.MODE, HtPhyMode.field.ShortGI,
						  BW, MCS,
						  Antenna,
						  (UINT32 *)&pRate->BitRate);
	}
	break;
#endif /* CONFIG_AP_SUPPORT */

	case CMD_RTPRIV_IOCTL_SIOCGIWNAME:
		RtmpIoctl_rt_ioctl_giwname(pAd, pData, 0);
		break;
#ifdef PROFILE_PATH_DYNAMIC

	case CMD_RTPRIV_IOCTL_PROFILEPATH_SET:
		pAd->profilePath = (CHAR *)Data;
		break;
#endif /* PROFILE_PATH_DYNAMIC */
	}

#ifdef RT_CFG80211_SUPPORT

	if ((cmd >= CMD_RTPRIV_IOCTL_80211_START) &&
		(cmd <= CMD_RTPRIV_IOCTL_80211_END))
		Status = CFG80211DRV_IoctlHandle(pAd, wrq, cmd, subcmd, pData, Data);

#endif /* RT_CFG80211_SUPPORT */

	if (cmd >= CMD_RTPRIV_IOCTL_80211_COM_LATEST_ONE)
		return NDIS_STATUS_FAILURE;

	return Status;
}

/*
    ==========================================================================
    Description:
	Issue a site survey command to driver
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 set site_survey
    ==========================================================================
*/
INT Set_SiteSurvey_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	NDIS_802_11_SSID Ssid;
	POS_COOKIE pObj;
#ifdef CONFIG_AP_SUPPORT
	UCHAR ifIndex;
	struct wifi_dev *wdev = NULL;
#endif
	BOOLEAN ap_scan = TRUE; /* snowpin for ap/sta */

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	/* check if the interface is down */
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("INFO::Network is down!\n"));
		return -ENETDOWN;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
	ifIndex = pObj->ioctl_if;
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI)
		wdev = &pAd->StaCfg[ifIndex].wdev;
	else
#endif
#ifdef CONFIG_STA_SUPPORT /* snowpin for ap/sta ++ */
		if (pObj->ioctl_if_type == INT_APCLI) {
			ap_scan = FALSE;
			wdev = &pAd->StaCfg[ifIndex].wdev;
		} else
#endif /* CONFIG_STA_SUPPORT */ /* snowpin for ap/sta -- */
		if (pObj->ioctl_if_type == INT_MBSSID)
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		else
			wdev = &pAd->ApCfg.MBSSID[0].wdev;

		ASSERT(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (MONITOR_ON(pAd)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!! Driver is in Monitor Mode now !!!\n"));
			return -EINVAL;
		}
		ap_scan = FALSE; /* snowpin for ap/sta */
	}
#endif /* CONFIG_STA_SUPPORT // */
	NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
	if (ap_scan) /* snopwin for ap/sta */
	{
		if ((strlen(arg) > 0) && (strlen(arg) <= MAX_LEN_OF_SSID)) {
			NdisMoveMemory(Ssid.Ssid, arg, strlen(arg));
			Ssid.SsidLength = strlen(arg);
			ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, wdev);
		} else {
			Ssid.SsidLength = 0;
			ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_PASSIVE, FALSE, wdev);
		}

		return TRUE;
	}

#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT // */
#ifdef CONFIG_STA_SUPPORT
	/* snowpin for ap/sta IF_DEV_CONFIG_OPMODE_ON_STA(pAd) */
	if (!ap_scan) {
		Ssid.SsidLength = 0;

		if ((arg != NULL) &&
			(strlen(arg) <= MAX_LEN_OF_SSID)) {
			RTMPMoveMemory(Ssid.Ssid, arg, strlen(arg));
			Ssid.SsidLength = strlen(arg);
		}

		pAd->StaCfg[pObj->ioctl_if].bSkipAutoScanConn = TRUE;
		StaSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, &pAd->StaCfg[pObj->ioctl_if].wdev);
	}
#endif /* CONFIG_STA_SUPPORT // */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_SiteSurvey_Proc\n"));
	return TRUE;
}


INT	Set_Antenna_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ANT_DIVERSITY_TYPE UsedAnt;
	INT i;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==> Set_Antenna_Proc *******************\n"));

	for (i = 0; i < strlen(arg); i++)
		if (!isdigit(arg[i]))
			return -EINVAL;

	UsedAnt = os_str_tol(arg, 0, 10);

	switch (UsedAnt) {

	/* 2: Fix in the PHY Antenna CON1*/
	case ANT_FIX_ANT0:
		AsicSetRxAnt(pAd, 0);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<== Set_Antenna_Proc(Fix in Ant CON1), (%d,%d)\n",
				 pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt));
		break;

	/* 3: Fix in the PHY Antenna CON2*/
	case ANT_FIX_ANT1:
		AsicSetRxAnt(pAd, 1);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<== %s(Fix in Ant CON2), (%d,%d)\n",
				 __func__, pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt));
		break;

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("<== %s(N/A cmd: %d), (%d,%d)\n", __func__, UsedAnt,
				 pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt));
		break;
	}

	return TRUE;
}

#ifdef HW_TX_RATE_LOOKUP_SUPPORT
INT Set_HwTxRateLookUp_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Enable;
	UINT32 MacReg;

	Enable = os_str_tol(arg, 0, 10);
	RTMP_IO_READ32(pAd->hdev_ctrl, TX_FBK_LIMIT, &MacReg);

	if (Enable) {
		MacReg |= 0x00040000;
		pAd->bUseHwTxLURate = TRUE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>UseHwTxLURate (ON)\n"));
	} else {
		MacReg &= (~0x00040000);
		pAd->bUseHwTxLURate = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>UseHwTxLURate (OFF)\n"));
	}

	RTMP_IO_WRITE32(pAd->hdev_ctrl, TX_FBK_LIMIT, MacReg);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("UseHwTxLURate = %d\n", pAd->bUseHwTxLURate));
	return TRUE;
}
#endif /* HW_TX_RATE_LOOKUP_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
#ifdef MULTI_MAC_ADDR_EXT_SUPPORT
INT Set_EnMultiMacAddrExt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Enable = os_str_tol(arg, 0, 10);

	pAd->bUseMultiMacAddrExt = (Enable ? TRUE : FALSE);
	AsicSetReptFuncEnable(pAd, pAd->bUseMultiMacAddrExt);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("UseMultiMacAddrExt = %d, UseMultiMacAddrExt(%s)\n",
			 pAd->bUseMultiMacAddrExt, (Enable ? "ON" : "OFF")));
	return TRUE;
}

INT	Set_MultiMacAddrExt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR tempMAC[6], idx;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	ULONG offset, Addr;
	INT i;

	if (strlen(arg) <
		19) /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.*/
		return FALSE;

	token = strchr(arg, DASH);

	if ((token != NULL) && (strlen(token) > 1)) {
		idx = (UCHAR) os_str_tol((token + 1), 0, 10);

		if (idx > 15)
			return FALSE;

		*token = '\0';

		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++) {
			if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
				return FALSE;

			AtoH(token, (&tempMAC[i]), 1);
		}

		if (i != 6)
			return FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
				 tempMAC[0], tempMAC[1], tempMAC[2], tempMAC[3], tempMAC[4], tempMAC[5], idx));
		offset = 0x1480 + (HW_WCID_ENTRY_SIZE * idx);
		Addr = tempMAC[0] + (tempMAC[1] << 8) + (tempMAC[2] << 16) + (tempMAC[3] << 24);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, offset, Addr);
		Addr = tempMAC[4] + (tempMAC[5] << 8);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, offset + 4, Addr);
		return TRUE;
	}

	return FALSE;
}
#endif /* MULTI_MAC_ADDR_EXT_SUPPORT */
#endif /* MAC_REPEATER_SUPPORT */

INT set_tssi_enable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 tssi_enable = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	tssi_enable = os_str_tol(arg, 0, 10);

	if (tssi_enable == 1) {
		cap->tssi_enable = TRUE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("turn on TSSI mechanism\n"));
	} else if (tssi_enable == 0) {
		cap->tssi_enable = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("turn off TSS mechanism\n"));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("illegal param(%u)\n", tssi_enable));
		return FALSE;
	}

	return TRUE;
}

INT	Set_RadioOn_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR radio;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = NULL;
#endif

	if (!wdev) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: wdev is NULL\n", __func__));
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
		if (!pMbss) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: pMbss is NULL\n", __func__));
			return FALSE;
		}
	}
#endif

	radio = os_str_tol(arg, 0, 10);

	if (!wdev->if_up_down_state) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("==>Set_RadioOn_Proc (%s) but IF is done, ignore!!! (wdev_idx %d)\n",
				  radio ? "ON" : "OFF", wdev->wdev_idx));
		return TRUE;
	}

	if (radio == !IsHcRadioCurStatOffByChannel(pAd, wdev->channel)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("==>Set_RadioOn_Proc (%s) equal to current state, ignore!!! (wdev_idx %d)\n",
				  radio ? "ON" : "OFF", wdev->wdev_idx));
		return TRUE;
	}

	if (radio) {
		MlmeRadioOn(pAd, wdev);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_RadioOn_Proc (ON)\n"));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
#ifdef FT_R1KH_KEEP
			 pAd->ApCfg.FtTab.FT_RadioOff = FALSE;
#endif /* FT_R1KH_KEEP */
		}
#endif
	} else {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef FT_R1KH_KEEP
			 pAd->ApCfg.FtTab.FT_RadioOff = TRUE;
#endif /* FT_R1KH_KEEP */
			APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
		}
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			MSTAStop(pAd, wdev);
		}
#endif
		MlmeRadioOff(pAd, wdev);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_RadioOn_Proc (OFF)\n"));
	}

	return TRUE;
}

#ifdef CUSTOMER_DCC_FEATURE

VOID dump_STA_Assoc_Cap(
	IN	PRTMP_ADAPTER	pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	int i;
	PMAC_TABLE_ENTRY pEntry = NULL;
	RT_STA_MAX_CAP_TABLE *sta_cap = NULL;

	os_alloc_mem(NULL, (UCHAR **)&sta_cap, sizeof(RT_STA_MAX_CAP_TABLE));
	if (sta_cap == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

	NdisZeroMemory(sta_cap, sizeof(RT_STA_MAX_CAP_TABLE));
	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &pAd->MacTab.Content[i];
		if (pEntry && IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
				COPY_MAC_ADDR(sta_cap->STACap[sta_cap->Num].Addr, pEntry->Addr);
				sta_cap->STACap[sta_cap->Num].NSS 	   = (pEntry->MaxHTPhyMode.field.MCS/8)+1;
				sta_cap->STACap[sta_cap->Num].MODE	   = pEntry->MaxHTPhyMode.field.MODE;
				sta_cap->STACap[sta_cap->Num].BW  	   = pEntry->MaxHTPhyMode.field.BW;
				sta_cap->STACap[sta_cap->Num].MCS 	   = pEntry->MaxHTPhyMode.field.MCS;
				sta_cap->STACap[sta_cap->Num].ShortGI  = pEntry->MaxHTPhyMode.field.ShortGI;
#ifdef DOT11_VHT_AC
				if (pEntry->MaxHTPhyMode.field.MODE == MODE_VHT) {
					sta_cap->STACap[sta_cap->Num].NSS 	   = ((pEntry->MaxHTPhyMode.field.MCS >> 4)+1);
					sta_cap->STACap[sta_cap->Num].MCS 	   = (pEntry->MaxHTPhyMode.field.MCS & 0xf);
				}
#endif
	sta_cap->Num++;
		}
	}

	wrq->u.data.length = sizeof(RT_STA_MAX_CAP_TABLE);
	if (copy_to_user(wrq->u.data.pointer, sta_cap, wrq->u.data.length)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}
	if (sta_cap != NULL)
		os_free_mem(sta_cap);
}


VOID RTMPIoctlQuerySTAStat(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	* wrq)
{
	int i;
	PMAC_TABLE_ENTRY pEntry = NULL;
	RT_STA_STATISTICS_TABLE *sta_stat = NULL;
	os_alloc_mem(NULL, (UCHAR **)&sta_stat, sizeof(RT_STA_STATISTICS_TABLE));
	if (sta_stat == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}
	NdisZeroMemory(sta_stat, sizeof(RT_STA_STATISTICS_TABLE));
	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &pAd->MacTab.Content[i];
		if (pEntry && IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
			sta_stat->STAEntry[sta_stat->Num].ApIdx = pEntry->pMbss->mbss_idx;
			COPY_MAC_ADDR(sta_stat->STAEntry[sta_stat->Num].Addr, pEntry->Addr);
			sta_stat->STAEntry[sta_stat->Num].RxCount = pEntry->RxCount;
			sta_stat->STAEntry[sta_stat->Num].TxCount = pEntry->TxCount;
			sta_stat->STAEntry[sta_stat->Num].ReceivedByteCount = pEntry->ReceivedByteCount;
			sta_stat->STAEntry[sta_stat->Num].TransmittedByteCount = pEntry->TransmittedByteCount;
			sta_stat->STAEntry[sta_stat->Num].RxErrorCount = pEntry->RxErrorCount;
			sta_stat->STAEntry[sta_stat->Num].RxDropCount = pEntry->RxDropCount;
			sta_stat->STAEntry[sta_stat->Num].TxErrorCount = pEntry->TxErrorCount;
			sta_stat->STAEntry[sta_stat->Num].TxDropCount = pEntry->TxDropCount;
			sta_stat->STAEntry[sta_stat->Num].TxRetriedPktCount = pEntry->TxRetriedPktCount;
			sta_stat->STAEntry[sta_stat->Num].ChannelUseTime = pEntry->ChannelUseTime;
			sta_stat->Num++;

			/* clear STA Stats */
			pEntry->RxCount = 0;
			pEntry->TxCount = 0;
			pEntry->ReceivedByteCount = 0;
			pEntry->TransmittedByteCount = 0;
			pEntry->RxErrorCount = 0;
			pEntry->RxDropCount = 0;
			pEntry->TxErrorCount = 0;
			pEntry->TxDropCount = 0;
			pEntry->TxRetriedPktCount = 0;
			pEntry->ChannelUseTime = 0;
		}
	}

	wrq->u.data.length = sizeof(RT_STA_STATISTICS_TABLE);
	if (copy_to_user(wrq->u.data.pointer, sta_stat, wrq->u.data.length)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}
	if (sta_stat != NULL)
		os_free_mem(sta_stat);
}
#endif
#ifdef CUSTOMER_RSG_FEATURE
VOID ClearChannelStatsCr(
	IN PRTMP_ADAPTER   pAd)
{
	UINT32 	CrValue;
	UINT32	Time;
	ULONG TNow;
	UCHAR CurrIdx = pAd->MsMibBucket.CurIdx;

	NdisGetSystemUpTime(&TNow);
	Time = jiffies_to_usecs(TNow);
	pAd->ChannelStats.LastReadTime = Time;
	/*OBSS Air time*/
	HW_IO_READ32(pAd->hdev_ctrl, RMAC_MIBTIME5, &CrValue);
	pAd->ChannelStats.MibUpdateOBSSAirtime[CurrIdx] += CrValue;
	/*My Tx Air time*/
	HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR36, &CrValue);
	pAd->ChannelStats.MibUpdateMyTxAirtime[CurrIdx] += CrValue;
	/*My Rx Air time*/
	HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR37, &CrValue);
	pAd->ChannelStats.MibUpdateMyRxAirtime[CurrIdx] += CrValue;
	/*EDCCA time*/
	RTMP_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR18, &CrValue);
	pAd->ChannelStats.MibUpdateEDCCAtime[CurrIdx] += CrValue;
	CrValue = (CrValue & 0xffffff);
	/*Reset OBSS Air time*/
	HW_IO_READ32(pAd->hdev_ctrl, RMAC_MIBTIME0, &CrValue);
	CrValue |= 1 << RX_MIBTIME_CLR_OFFSET;
	CrValue |= 1 << RX_MIBTIME_EN_OFFSET;
	HW_IO_WRITE32(pAd, RMAC_MIBTIME0, CrValue);
	/*FALSE CCA Count*/
	HW_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS0, &CrValue);
	pAd->ChannelStats.MibUpdatePdCount[CurrIdx] += CrValue;
	HW_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS5, &CrValue);
	pAd->ChannelStats.MibUpdateMdrdyCount[CurrIdx] += CrValue;
	/*reset PD and MDRDY count*/
	HW_IO_READ32(pAd->hdev_ctrl, PHY_BAND0_PHYMUX_5, &CrValue);
	CrValue &= 0xff8fffff;
	HW_IO_WRITE32(pAd->hdev_ctrl, PHY_BAND0_PHYMUX_5, CrValue);
	CrValue |= 0x500000;
	HW_IO_WRITE32(pAd->hdev_ctrl, PHY_BAND0_PHYMUX_5, CrValue);
}

VOID ResetChannelStats(
	IN PRTMP_ADAPTER   pAd)
{
	pAd->ChannelStats.LastReadTime = 0;
	pAd->ChannelStats.TotalDuration = 0;
	pAd->ChannelStats.CCABusytime = 0;
	pAd->ChannelStats.ChBusytime = 0;
	pAd->ChannelStats.FalseCCACount = 0;
	pAd->ChannelStats.ChannelApActivity = 0;
}


INT Set_ApEnableRadioChStats_Proc(
		IN	PRTMP_ADAPTER	pAd,
		IN	RTMP_STRING 	* arg)
{
	UINT32 enable;

	if (strlen(arg) > 1) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong argument type\n"));
		return FALSE;
	}
	enable = simple_strtol(arg, 0, 10);
	if ((enable != 1) && (enable != 0)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong argument value\n"));
		return FALSE;
	}

	if (enable) {
		UCHAR idx;
		/* set the EnableChannelStatsCheck value to true and initialize the values to zero */
		pAd->EnableChannelStatsCheck = TRUE;
		pAd->ChannelStats.LastReadTime = 0;
		pAd->ChannelStats.TotalDuration = 0;
		pAd->ChannelStats.msec100counts = 0;
		pAd->ChannelStats.CCABusytime = 0;
		pAd->ChannelStats.ChBusytime = 0;
		pAd->ChannelStats.FalseCCACount = 0;
		pAd->ChannelStats.ChannelApActivity = 0;
		pAd->ChannelStats.ChBusyTimeAvg = 0;
		pAd->ChannelStats.CCABusyTimeAvg = 0;
		pAd->ChannelStats.FalseCCACountAvg = 0;
		pAd->ChannelStats.ChannelApActivityAvg = 0;
		pAd->ChannelStats.ChBusyTime1secValue = 0;
		pAd->ChannelStats.CCABusyTime1secValue = 0;
		pAd->ChannelStats.FalseCCACount1secValue = 0;
		pAd->ChannelStats.ChannelApActivity1secValue = 0;

		for (idx = 0; idx < 2 ; idx++) {
			pAd->ChannelStats.MibUpdateEDCCAtime[idx] = 0;
			pAd->ChannelStats.MibUpdateMdrdyCount[idx] = 0;
			pAd->ChannelStats.MibUpdateMyRxAirtime[idx] = 0;
			pAd->ChannelStats.MibUpdateMyTxAirtime[idx] = 0;
			pAd->ChannelStats.MibUpdateOBSSAirtime[idx] = 0;
			pAd->ChannelStats.MibUpdatePdCount[idx] = 0;
		}
		ClearChannelStatsCr(pAd);

		return TRUE;
	} else if (enable == 0) {
		if (pAd->EnableChannelStatsCheck) {
			/* Set the EnableChannelStatsCheck to FALSE*/
			pAd->EnableChannelStatsCheck = FALSE;
		}
		return TRUE;
	}

	return FALSE;
}
#endif

#ifdef OFFCHANNEL_SCAN_FEATURE
INT Set_ScanResults_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING	* arg)
{
	UINT32 ch_index = 0;

	ch_index = Channel2Index(pAd, pAd->ChannelInfo.ChannelNo, pAd->ChannelInfo.bandidx);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Channel : %d NF value : %ddb \tBusyTime : %dusec\n",
				pAd->ChannelInfo.ChannelNo, pAd->ChannelInfo.AvgNF,
				pAd->ChannelInfo.chanbusytime[ch_index]));

	pAd->ChannelInfo.bandidx = 0;
	pAd->ChannelInfo.ChannelNo = 0;
	pAd->ChannelInfo.AvgNF = 0;
	pAd->ChannelInfo.chanbusytime[ch_index] = 0;
	return TRUE;
}

INT Set_ApScan_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING	* arg)
{
	POS_COOKIE pObj;
	UINT channel = 0;
	UINT timeout = 0;
	INT32 i = 0, j = 0, count = 0;
	CHAR scantype[8];
	CHAR temp[33];
	UCHAR ifIndex;
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	/* check if the interface is down */
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("INFO::Network is down!\n"));
		return -ENETDOWN;
	}

	while (arg[j] != '\0') {
		temp[i] = arg[j++];
		if (temp[i] == ':') {
			switch (++count) {
			case 1:
				temp[i] = '\0';
				if ((strlen(temp) != 0) && (strlen(temp) <= 7)) {
					strcpy(scantype, temp);
					if (strcmp(scantype, "active") && strcmp(scantype, "passive")) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("wrong scan type argument\n"));
						return FALSE;
					}
				} else {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("wrong scan type argument\n"));
					return FALSE;
				}
				i = -1;
				break;
			case 2:
				temp[i] = '\0';
				if ((strlen(temp) != 0) && (strlen(temp) <= 3)) {
					channel = simple_strtol(temp, 0, 10);
					if (!ChannelSanity(pAd, channel)) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("wrong channel number\n"));
						return FALSE;
					}
				} else {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("wrong channel number\n"));
					return FALSE;
				}
				i = -1;
				break;
			default:
				if (count > 2) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("wrong number of arguments\n"));
					return FALSE;
				}
				break;
			}
		} else if (arg[j] == '\0') {
			temp[i+1] = '\0';
			if ((strlen(temp) != 0) && (strlen(temp) <= 10) && (simple_strtol(temp, 0, 10) < 0xffffffff)) {
				timeout = simple_strtol(temp, 0, 10);
			} else if (strlen(temp)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("wrong Timeout value\n"));
				return FALSE;
			}
		}
		i++;
	}
	ifIndex = pObj->ioctl_if;
	if (pObj->ioctl_if_type == INT_MBSSID)
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	else
		wdev = &pAd->ApCfg.MBSSID[0].wdev;
	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
	/* Make compatible with application path */
	ScanCtrl->Num_Of_Channels = 1;
	ScanCtrl->ScanTime[0] = 0;
	ScanCtrl->CurrentGivenChan_Index = 0;
	ScanCtrl->state = OFFCHANNEL_SCAN_START;
	if (!strcmp(scantype, "passive"))
		ApSiteSurveyNew_by_wdev(pAd, channel, timeout, SCAN_PASSIVE, FALSE, wdev);
	else if (!strcmp(scantype, "active"))
		ApSiteSurveyNew_by_wdev(pAd, channel, timeout, SCAN_ACTIVE, FALSE, wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_ApScan_Proc\n"));
	return TRUE;
}
#endif

#ifdef NEW_SET_RX_STREAM
INT	Set_RxStream_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT RxStream;

	RxStream = os_str_tol(arg, 0, 10);
	AsicSetRxStream(pAd, RxStream, 0);
	return TRUE;
}
#endif

INT	Set_Lp_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR lp_enable;
	struct wifi_dev *wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMBSS = NULL;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		pMBSS = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
		wdev = &pMBSS->wdev;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[pObj->ioctl_if].wdev;
	}
#endif /* CONFIG_STA_SUPPORT */

	lp_enable = os_str_tol(arg, 0, 10);

	if (lp_enable) {
		MlmeLpEnter(pAd);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_Lp_Proc (Enetr)\n"));
	} else {
		MlmeLpExit(pAd);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			APStartUp(pAd, pMBSS, AP_BSS_OPER_BY_RF);
		}
#endif /* CONFIG_AP_SUPPORT */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_Lp_Proc (Exit)\n"));
	}

	return TRUE;
}

#ifdef MT_MAC
INT setTmrVerProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	CHAR ver;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	ver = (CHAR)os_str_tol(arg, 0, 10);

	if ((ver < TMR_VER_1_0) || (ver > TMR_VER_2_0)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: wrong setting %d, remain default %d!!\n",
			__func__, ver, pChipCap->TmrHwVer));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: ver = %d, pChipCap->TmrHwVer = %d\n",
		__func__, ver, pChipCap->TmrHwVer));

	return TRUE;
}

INT setTmrEnableProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	LONG enable;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	enable = os_str_tol(arg, 0, 10);

	if ((enable < TMR_DISABLE) || (enable > TMR_RESPONDER)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: enable is incorrect!!\n", __func__));
		return FALSE;
	}

	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: chipcap is not HIF_MT\n", __func__));
		return FALSE;
	}

	TmrCtrl(pAd, (UCHAR)enable, cap->TmrHwVer);
	return TRUE;
}

#ifndef COMPOS_TESTMODE_WIN
INT SetTmrCalProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	struct os_cookie *obj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, obj->ioctl_if, obj->ioctl_if_type);
	UCHAR TmrType = os_str_tol(arg, 0, 10);
	UCHAR Channel = HcGetRadioChannel(pAd);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s(): TMR Calibration, TmrType: %d\n", __func__, TmrType));

	if (IS_HIF_TYPE(pAd, HIF_MT))
		MtSetTmrCal(pAd, TmrType, Channel, wlan_operate_get_bw(wdev));

	return TRUE;
}
#endif



#ifdef RTMP_MAC_PCI
INT Set_PDMAWatchDog_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 Dbg;

	Dbg = os_str_tol(arg, 0, 10);

	if (Dbg == 1)
		pAd->PDMAWatchDogEn = 1;
	else if (Dbg == 0)
		pAd->PDMAWatchDogEn = 0;
	else if (Dbg == 2)
		PDMAResetAndRecovery(pAd);
	else if (Dbg == 3)
		pAd->PDMAWatchDogDbg = 0;
	else if (Dbg == 4)
		pAd->PDMAWatchDogDbg = 1;

	return TRUE;
}

INT SetPSEWatchDog_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 Dbg;

	Dbg = os_str_tol(arg, 0, 10);

	if (Dbg == 1)
		pAd->PSEWatchDogEn = 1;
	else if (Dbg == 0)
		pAd->PSEWatchDogEn = 0;
	else if (Dbg == 2)
		PSEResetAndRecovery(pAd);
	else if (Dbg == 3)
		DumpPseInfo(pAd);

	return TRUE;
}
#endif

INT set_cr4_query(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 option = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: arg = %s\n", __func__, arg));

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	option = os_str_toul(arg, 0, 16);
	MtCmdCr4Query(pAd, option, 0, 0);
	return TRUE;
}

INT set_cr4_set(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 arg0 = 0;
	UINT32 arg1 = 0;
	UINT32 arg2 = 0;
	RTMP_STRING *arg0_ptr = NULL;
	RTMP_STRING *arg1_ptr = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: arg = %s\n", __func__, arg));
	arg0_ptr = strsep(&arg, ":");
	arg1_ptr = strsep(&arg, ":");

	if (arg0_ptr == NULL || arg1_ptr == NULL || arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	arg0 = os_str_toul(arg0_ptr, 0, 16);
	arg1 = os_str_toul(arg1_ptr, 0, 16);
	arg2 = os_str_toul(arg, 0, 16);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: arg0 = 0x%x, arg1 = 0x%x, arg2 = 0x%x\n",
			  __func__, arg0, arg1, arg2));
	MtCmdCr4Set(pAd, arg0, arg1, arg2);
	return TRUE;
}

INT set_cr4_capability(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 option = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: arg = %s\n", __func__, arg));

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	option = os_str_toul(arg, 0, 16);
	MtCmdCr4Capability(pAd, option);
	return TRUE;
}

INT set_cr4_debug(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 option = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 (":%s: arg = %s\n", __func__, arg));

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	option = os_str_toul(arg, 0, 16);
	MtCmdCr4Debug(pAd, option);
	return TRUE;
}

INT dump_cr4_pdma_debug_probe(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 i = 0;
	UINT32 u4Low = 0, u4High = 0;
	UINT32 origonal_cr1_value = 0;

	if (IS_MT7615(pAd)) {
		/* keep the origonal remap cr1 value for restore */
		HW_IO_READ32(pAd->hdev_ctrl, MCU_PCIE_REMAP_1, &origonal_cr1_value);
		/* do PCI-E remap for CR4 PDMA physical base address to 0x40000 */
		HW_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_1, 0x82000000);
	} else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s:Non 7615 don't support this cmd !\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: ======= CR4 PDMA1 Debug Probe ========\n", __func__));

	for (i = 0; i <= 0x10; i++) {
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x40408, (i | (i << 8)));
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x40400, 0x0201);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x40404, &u4Low);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x40400, 0x0403);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x40404, &u4High);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("cr4_dbg_pdma1_%u: 0x%x\n", i, ((u4High & 0xFFFF) << 16) | (u4Low & 0xFFFF)));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: ======== CR4 PDMA2 Debug Probe ========\n", __func__));

	for (i = 0; i <= 0x10; i++) {
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x40408, (i | (i << 8)));
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x40400, 0x0605);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x40404, &u4Low);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x40400, 0x0807);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x40404, &u4High);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("cr4_dbg_pdma2_%u: 0x%x\n", i, ((u4High & 0xFFFF) << 16) | (u4Low & 0xFFFF)));
	}

	if (IS_MT7615(pAd)) {
		/* restore the origonal remap cr1 value */
		HW_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_1, origonal_cr1_value);
	}

	return TRUE;
}

INT dump_remap_cr_content(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 origonal_remap_cr_value = 0;
	UINT32 remapped_register_value = 0;
	UINT32 cr_selected_to_remap = 0;
	UINT32 address_you_want_to_remap = 0;
	UINT32 remap_cr_record_base_address = 0;
	UINT32 offset_between_target_and_remap_cr_base = 0;
	RTMP_STRING *cr_selected_to_remap_ptr = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: arg = %s\n", __func__, arg));

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Invalid parameters!\n", __func__));
		return FALSE;
	}

	cr_selected_to_remap_ptr = strsep(&arg, ":");

	if (!cr_selected_to_remap_ptr || !arg) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s:miss input parameters!\n", __func__));
		return FALSE;
	}

	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT7636(pAd) || IS_MT7637(pAd)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s:Non 7615 don't support this cmd !\n", __func__));
		return FALSE;
	}

	cr_selected_to_remap = os_str_toul(cr_selected_to_remap_ptr, 0, 16);
	address_you_want_to_remap = os_str_toul(arg, 0, 16);

	if (cr_selected_to_remap == 1) {
		/* keep the origonal remap cr1 value for restore */
		HW_IO_READ32(pAd->hdev_ctrl, MCU_PCIE_REMAP_1, &origonal_remap_cr_value);
		/* do PCI-E remap for CR4 PDMA physical base address to 0x40000 */
		HW_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_1, address_you_want_to_remap);
		HW_IO_READ32(pAd->hdev_ctrl, MCU_PCIE_REMAP_1, &remap_cr_record_base_address);

		if ((address_you_want_to_remap - remap_cr_record_base_address) > REMAP_1_OFFSET_MASK) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:Exceed CR1 remap range(offset: 0x%x > mask: 0x%x)!!\n", __func__,
					  (address_you_want_to_remap - remap_cr_record_base_address), REMAP_1_OFFSET_MASK));
			/* restore the origonal remap cr1 value */
			HW_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_1, origonal_remap_cr_value);
			return FALSE;
		}

		offset_between_target_and_remap_cr_base =
			((address_you_want_to_remap - remap_cr_record_base_address)
			 & REMAP_1_OFFSET_MASK);
	} else if (cr_selected_to_remap == 2) {
		/* keep the origonal remap cr2 value for restore */
		HW_IO_READ32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, &origonal_remap_cr_value);
		/* do PCI-E remap the address you want to 0x80000 */
		HW_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, address_you_want_to_remap);
		HW_IO_READ32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, &remap_cr_record_base_address);

		if ((address_you_want_to_remap - remap_cr_record_base_address) > REMAP_2_OFFSET_MASK) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:Exceed CR2 remap range(offset: 0x%x > mask: 0x%x)!!\n", __func__,
					  (address_you_want_to_remap - remap_cr_record_base_address), REMAP_2_OFFSET_MASK));
			/* restore the origonal remap cr2 value */
			HW_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, origonal_remap_cr_value);
			return FALSE;
		}

		offset_between_target_and_remap_cr_base =
			((address_you_want_to_remap - remap_cr_record_base_address)
			 & REMAP_2_OFFSET_MASK);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Error! Unknow remap CR selected\n", __func__));
		/* restore the origonal remap cr2 value */
		HW_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, origonal_remap_cr_value);
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:%s origonal value = 0x%x, remap address = 0x%x,\n"
			  "new remap cr value = 0x%x, offset = 0x%x\n", __func__,
			  ((cr_selected_to_remap == 1) ? "CR1" : "CR2"),
			  origonal_remap_cr_value, address_you_want_to_remap,
			  remap_cr_record_base_address, offset_between_target_and_remap_cr_base));

	if (cr_selected_to_remap == 1) {
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x40000 + offset_between_target_and_remap_cr_base,
					   &remapped_register_value);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s:0x%x = 0x%x\n", __func__,
				  (0x40000 + offset_between_target_and_remap_cr_base),
				  remapped_register_value));
		/* restore the origonal remap cr1 value */
		HW_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_1, origonal_remap_cr_value);
	} else if (cr_selected_to_remap == 2) {
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x80000 + offset_between_target_and_remap_cr_base,
					   &remapped_register_value);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s:0x%x = 0x%x\n", __func__,
				  (0x80000 + offset_between_target_and_remap_cr_base),
				  remapped_register_value));
		/* restore the origonal remap cr2 value */
		HW_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, origonal_remap_cr_value);
	}

	return TRUE;
}

#ifdef MT7626
INT set_cal_control(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Rv;
	INT32 BandIdx = 0;
	INT32 CalFuncIdx = 0;
	INT32 CalItem = 0;
	UINT32 CalItemIdx = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: arg = %s\n", __func__, arg));
	Rv = sscanf(arg, "%d-%d-%d", (int *)&BandIdx, (int *)&CalFuncIdx, (int *)&CalItem);

	if (BandIdx > 1 || CalFuncIdx >= CAL_CTRL_FUNC_NUM) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: Unknown BandIdx = %d CalFuncIdx = %d\n", __func__, BandIdx, CalFuncIdx));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: BandIdx: %d, CalFuncIdx: %d, CalItem: %d\n", __func__, BandIdx, CalFuncIdx, CalItem));

	if (CalFuncIdx == 2) {
		switch (CalItem) {
		case 0:
			CalItemIdx = CAL_SPEC_ID1;
			break;

		case 1:
			CalItemIdx = CAL_SPEC_ID2;
			break;

		case 2:
			CalItemIdx = CAL_SPEC_ID3;
			break;

		case 3:
			CalItemIdx = CAL_SPEC_ID4;
			break;

		case 4:
			CalItemIdx = CAL_SPEC_ID5;
			break;

		case 5:
			CalItemIdx = CAL_SPEC_ID6;
			break;

		case 6:
			CalItemIdx = CAL_SPEC_ID7;
			break;

		case 7:
			CalItemIdx = CAL_SPEC_ID9;
			break;

		case 8:
			CalItemIdx = CAL_SPEC_ID10;
			break;

		case 9:
			CalItemIdx = CAL_SPEC_ID8;
			break;

		case 12:
			CalItemIdx = CAL_ID_NUM;
			break;

		default:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (":%s: Unknown CalItem = %d\n", __func__, CalItem));
			break;
		}
	}
	MtCmdCalCtrl(pAd, BandIdx, CalFuncIdx, CalItemIdx);

	return TRUE;
}
#endif

INT set_re_calibration(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 BandIdx = 0;
	UINT32 CalItem = 0;
	UINT32 CalItemIdx = 0;
	RTMP_STRING *pBandIdx  = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: arg = %s\n", __func__, arg));
	pBandIdx = strsep(&arg, ":");

	if (pBandIdx == NULL || arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	BandIdx = os_str_toul(pBandIdx, 0, 10);
	CalItem = os_str_toul(arg, 0, 10);

	if (BandIdx > 1) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: Unknown BandIdx = %d\n", __func__, BandIdx));
		return FALSE;
	}

	if ((CalItem > 12) || (CalItem == 3) || (CalItem == 4)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: Unknown CalItem = %d\n", __func__, CalItem));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: BandIdx: %d, CalItem: %d\n", __func__, BandIdx, CalItem));

	switch (CalItem) {
	case 0:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: RC_CAL\n", __func__));
		CalItemIdx = RC_CAL;
		break;

	case 1:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: RX_RSSI_DCOC_CAL\n", __func__));
		CalItemIdx = RX_RSSI_DCOC_CAL;
		break;

	case 2:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: RX_DCOC_CAL\n", __func__));
		CalItemIdx = RX_DCOC_CAL;
		break;

	case 5:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: RX_FIIQ_CAL\n", __func__));
		CalItemIdx = RX_FIIQ_CAL;
		break;

	case 6:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: RX_FDIQ_CAL\n", __func__));
		CalItemIdx = RX_FDIQ_CAL;
		break;

	case 7:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: TX_DPD_LINK\n", __func__));
		CalItemIdx = TX_DPD_LINK;
		break;

	case 8:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: TX_LPFG\n", __func__));
		CalItemIdx = TX_LPFG;
		break;

	case 9:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: TX_DCIQC\n", __func__));
		CalItemIdx = TX_DCIQC;
		break;

	case 10:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: TX_IQM\n", __func__));
		CalItemIdx = TX_IQM;
		break;

	case 11:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: TX_PGA\n", __func__));
		CalItemIdx = TX_PGA;
		break;

	case 12:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: CAL_ALL\n", __func__));
		CalItemIdx = CAL_ALL;
		break;

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: Unknown CalItem = %d\n", __func__, CalItem));
		break;
	}

	MtCmdDoCalibration(pAd, RE_CALIBRATION, CalItemIdx, BandIdx);
	return TRUE;
}

INT set_thermal_recal_mode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 Mode;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: arg = %s\n", __FUNCTION__, arg));
	Mode = simple_strtol(arg, 0, 10);

	if (Mode > 2) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: Unknown Mode = %d (0: thermal recal OFF; 1: thermal recal ON; 2: trigger thermal recal)\n",
			__FUNCTION__, Mode));

		return FALSE;
	}

	if ((Mode == 2) && (pAd->CommonCfg.ThermalRecalMode == 0)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: Can't trigger recal in Thermal recal off mode\n", __FUNCTION__));

		return FALSE;
	}

	pAd->CommonCfg.ThermalRecalMode = Mode;

	MtCmdThermalReCalMode(pAd, Mode);

	return TRUE;
}


INT set_fw_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 McuDest = 0;
	UINT32 LogType = 0;
	BOOLEAN invalid = FALSE;
	UINT8 bitmap;
	RTMP_STRING *pMcuDest  = NULL;
	RTMP_STRING *info[] = {
		"Print MCU Log to UART",
		"Send MCU log by Event",
#ifdef FW_LOG_DUMP
		"Send MCU log to EMI(currently not support)",
		"Send MCU log by Event to Storage",
		"Send MCU log by Event to Ethernet"
#endif
	};

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: arg = %s\n", __func__, arg));
	pMcuDest = strsep(&arg, ":");

	if (pMcuDest == NULL || arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	McuDest = os_str_toul(pMcuDest, 0, 10);
	LogType = os_str_toul(arg, 0, 10);

	if (McuDest == 0) {
		if ((LogType & ~SUPPORTED_FW_LOG_TYPE) != 0)
			invalid = TRUE;
#ifdef FW_LOG_DUMP
		else
			pAd->fw_log_ctrl.wmcpu_log_type = LogType;
#endif /* FW_LOG_DUMP */
	}

	if ((McuDest == 1) && ((LogType & ~0x03) != 0))
		invalid = TRUE;

	if (McuDest > 2)
		invalid = TRUE;

	if (invalid) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: Unknown Mcu Dest = %d, Log Type = %x\n",
				  __func__, McuDest, LogType));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			(":%s: Mcu Dest = %s, ",
			__func__, (McuDest == 0 ? "HOST2N9" : "HOST2CR4")));

	if (LogType == 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Log Type = Disable MCU Log Message\n"));
	} else {
		for (bitmap = 0; bitmap < FW_LOG_TYPE_COUNT; bitmap++)
			if (LogType & (1 << bitmap))
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("Log Type = %s\n", info[bitmap]));
	}

	MtCmdFwLog2Host(pAd, McuDest, LogType);
	return TRUE;
}

INT set_fw_dbg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_STRING *dbg_lvl_str = NULL;
	UINT8 dbg_lvl;
	UINT32 dbg_module_idx;

	if (arg == NULL || strlen(arg) == 0)
		goto error;

	dbg_lvl_str = strsep(&arg, ":");
	dbg_lvl = os_str_toul(dbg_lvl_str, 0, 10);

	if (arg == NULL || strlen(arg) == 0) {
		/* imply all modules */
		dbg_module_idx = 0xffffffff;
	} else {
		dbg_module_idx = os_str_toul(arg, 0, 10);
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: set fw debug level to 0x%x for module index 0x%x\n", __func__, dbg_lvl, dbg_module_idx));

	MtCmdFwDbgCtrl(pAd, dbg_lvl, dbg_module_idx);
	return TRUE;

error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Usage: fw_dbg=[debug level bitmap]:[debug module index]\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tdebug level bitmap:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t DBG_CLASS_ERROR (1 << 0)\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t DBG_CLASS_WARN  (1 << 1)\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t DBG_CLASS_STATE (1 << 2)\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t DBG_CLASS_INFO  (1 << 3)\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t DBG_CLASS_LOUD  (1 << 4)\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tdebug module index:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t If not specified, means all modules.\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Example:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t iwpriv ra0 set fw_dbg=3 (turn on ERROR and WARN for all modules\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t iwpriv ra0 set fw_dbg=1:0 (turn on ERROR for module 0\n"));

	return FALSE;
}

INT set_isr_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#define ISR_CMD_CMD_ID_OFFSET 24
#define ISR_CMD_SUBCMD_ID_OFFSET 0
#define ISR_CMD_CMD_ID_MASK 0x7f
#define ISR_CMD_SUBCMD_ID_MASK 0xffffff
	UINT32 cmd = 0;
	UINT32 sub_cmd = 0;
	RTMP_STRING *cmd_str  = NULL;
	UINT32 tmp;
	UINT32 tmp2;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: arg = %s\n", __func__, arg));
	cmd_str = strsep(&arg, ":");

	if (cmd_str == NULL || arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	cmd = os_str_toul(cmd_str, 0, 10);
	sub_cmd = os_str_toul(arg, 0, 10);

	cmd &= ISR_CMD_CMD_ID_MASK;
	sub_cmd &= ISR_CMD_SUBCMD_ID_MASK;

	HW_IO_READ32(pAd->hdev_ctrl, PLE_TO_N9_INT, &tmp);
	tmp2 = (tmp ^ PLE_TO_N9_INT_TOGGLE_MASK) & PLE_TO_N9_INT_TOGGLE_MASK;

	tmp2 |= (cmd << ISR_CMD_CMD_ID_OFFSET);
	tmp2 |= (sub_cmd << ISR_CMD_SUBCMD_ID_OFFSET);

	HW_IO_WRITE32(pAd->hdev_ctrl, PLE_TO_N9_INT, tmp2);

	return TRUE;
}

UINT16 txop0;
UINT16 txop60 = 0x60;
UINT16 txop80 = 0x80;
UINT16 txopfe = 0xfe;
INT set_txop_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_STRING *str  = NULL;
	RTMP_STRING *str2  = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		(":%s: current setting txop 0=%x, txop 60=%x, txop 80=%x, txop fe=%x\n",
			__func__, txop0, txop60, txop80, txopfe));

	str = strsep(&arg, ":");
	if (str == NULL || arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		(":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	do {
		str2 = strsep(&arg, ":");
		if (str2 == NULL)
			str2 = arg;

		switch (os_str_toul(str, 0, 16)) {
		case 0x0:
			txop0 = os_str_toul(str2, 0, 16);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(":%s: txop0 change to %x\n", __func__, txop0));
			break;
		case 0x60:
			txop60 = os_str_toul(str2, 0, 16);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(":%s: txop60 change to %x\n", __func__, txop60));
			break;
		case 0x80:
			txop80 = os_str_toul(str2, 0, 16);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(":%s: txop80 change to %x\n", __func__, txop80));
			break;
		case 0xfe:
			txopfe = os_str_toul(str2, 0, 16);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(":%s: txop60 change to %x\n", __func__, txopfe));
			break;
		default:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(":%s: not support txop%lx\n", __func__, os_str_toul(str, 0, 16)));
			break;
		}

		str = strsep(&arg, ":");
	} while (str != NULL);

	return TRUE;
}

INT set_rts_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#define RTS_NUM_DIS_VALUE 0xff
#define RTS_LEN_DIS_VALUE 0xffffff
#define RTS_NUM_EN_VALUE 0x4
#define RTS_LEN_EN_VALUE 0x92b

	UINT32 rts_mode;
	RTMP_STRING *str  = NULL;
	UCHAR bandidx;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arg == NULL || strlen(arg) == 0)
		goto invalidparameter;

	str = strsep(&arg, ":");

	if (str == NULL || arg == NULL)
		goto invalidparameter;

	bandidx = os_str_toul(str, 0, 10);

	if (bandidx > 1)
		goto invalidparameter;

	rts_mode = os_str_toul(arg, 0, 10);

	if (rts_mode == 0) {
		mt_asic_rts_on_off(pAd, bandidx, RTS_NUM_EN_VALUE, RTS_LEN_EN_VALUE, FALSE);
		pAd->mcli_ctl.c2s_only = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("enable rts\n"));
	} else if (rts_mode == 1) {
		mt_asic_rts_on_off(pAd, bandidx, RTS_NUM_DIS_VALUE, RTS_LEN_DIS_VALUE, FALSE);
		pAd->mcli_ctl.c2s_only = TRUE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("disable rts\n"));
	} else if (rts_mode == 2) {
		arch_ops->asic_rts_on_off = NULL;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("disable dynamic rts on/off alg\n"));
	} else if (rts_mode == 3) {
		arch_ops->asic_rts_on_off = mt_asic_rts_on_off;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("enable dynamic rts on/off alg\n"));
	} else
		goto invalidparameter;

	return TRUE;

invalidparameter:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("rts is %s, asic_rts_on_off = %p\n",
		(pAd->mcli_ctl.c2s_only) ? "off" : "on", arch_ops->asic_rts_on_off));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("format: [bandidx]:[mode]\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("[bandidx]: 0/1\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("[mode]: 0 = enable rts , 1 = disable rts,"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("2 = disable dynamic rts on/off alg, 3 = enable dynamic rts on/off alg\n"));

	return TRUE;
}

/* SER user command */
#define SER_USER_CMD_DISABLE         0
#define SER_USER_CMD_ENABLE          1

#define SER_USER_CMD_ENABLE_MASK_TRACKING_ONLY      (201)
#define SER_USER_CMD_ENABLE_MASK_L1_RECOVER_ONLY    (202)
#define SER_USER_CMD_ENABLE_MASK_L3_RX_ABORT_ONLY   (203)
#define SER_USER_CMD_ENABLE_MASK_L3_TX_ABORT_ONLY   (204)
#define SER_USER_CMD_ENABLE_MASK_L3_TX_DISABLE_ONLY (205)
#define SER_USER_CMD_ENABLE_MASK_L3_BFRECOVER_ONLY  (206)
#define SER_USER_CMD_ENABLE_MASK_RECOVER_ALL        (207)

#define SER_USER_CMD_L1_RECOVER          995 /* Use a magic number to prevent human mistake */
#define SER_USER_CMD_L2_RECOVER          (302)
#define SER_USER_CMD_L3_RX_ABORT         (303)
#define SER_USER_CMD_L3_TX0_ABORT        (304)
#define SER_USER_CMD_L3_TX1_ABORT        (305)
#define SER_USER_CMD_L3_TX0_DISABLE      (306)
#define SER_USER_CMD_L3_TX1_DISABLE      (307)
#define SER_USER_CMD_L3_BF_RECOVER       (308)

INT set_ser(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 cmdId = 0;

	if (arg == NULL)
		goto ser_usage;

	cmdId = os_str_toul(arg, 0, 10);

	switch (cmdId) {
	case SER_USER_CMD_DISABLE:
		CmdExtSER(pAd, SER_ACTION_SET, SER_SET_DISABLE);
		break;

	case SER_USER_CMD_ENABLE:
		CmdExtSER(pAd, SER_ACTION_SET, SER_SET_ENABLE);
		break;

	case SER_USER_CMD_ENABLE_MASK_TRACKING_ONLY:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING);
		break;

	case SER_USER_CMD_ENABLE_MASK_L1_RECOVER_ONLY:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L1_RECOVER);
		break;

	case SER_USER_CMD_ENABLE_MASK_L3_RX_ABORT_ONLY:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_RX_ABORT);
		break;

	case SER_USER_CMD_ENABLE_MASK_L3_TX_ABORT_ONLY:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_TX_ABORT);
		break;

	case SER_USER_CMD_ENABLE_MASK_L3_TX_DISABLE_ONLY:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_TX_DISABLE);
		break;

	case SER_USER_CMD_ENABLE_MASK_L3_BFRECOVER_ONLY:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_BF_RECOVER);
		break;

	case SER_USER_CMD_ENABLE_MASK_RECOVER_ALL:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK,
					(SER_ENABLE_TRACKING | SER_ENABLE_L1_RECOVER | SER_ENABLE_L3_RX_ABORT |
					 SER_ENABLE_L3_TX_ABORT | SER_ENABLE_L3_TX_DISABLE | SER_ENABLE_L3_BF_RECOVER));
		break;

	case SER_USER_CMD_L1_RECOVER:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L1_RECOVER);
		break;

	case SER_USER_CMD_L3_RX_ABORT:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_RX_ABORT);
		break;

	case SER_USER_CMD_L3_TX0_ABORT:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_TX0_ABORT);
		break;

	case SER_USER_CMD_L3_TX1_ABORT:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_TX1_ABORT);
		break;

	case SER_USER_CMD_L3_TX0_DISABLE:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_TX0_DISABLE);
		break;

	case SER_USER_CMD_L3_TX1_DISABLE:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_TX1_DISABLE);
		break;

	case SER_USER_CMD_L3_BF_RECOVER:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_BF_RECOVER);
		break;

	default:
		goto ser_usage;
	}

	return TRUE;
ser_usage:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("iwpriv rax set ser=[command ID]\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("[command ID]\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Disable SER\n", SER_USER_CMD_DISABLE));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Enable SER\n", SER_USER_CMD_ENABLE));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Level 1 recover\n", SER_USER_CMD_L1_RECOVER));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Level 3 rx abort\n", SER_USER_CMD_L3_RX_ABORT));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Level 3 tx0 abort\n", SER_USER_CMD_L3_TX0_ABORT));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Level 3 tx1 abort\n", SER_USER_CMD_L3_TX1_ABORT));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Level 3 tx0 disable\n", SER_USER_CMD_L3_TX0_DISABLE));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Level 3 tx1 disable\n", SER_USER_CMD_L3_TX1_DISABLE));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Level 3 BF recover\n", SER_USER_CMD_L3_BF_RECOVER));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : HW ERROR Tracking only, no recover\n", SER_USER_CMD_ENABLE_MASK_TRACKING_ONLY));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Set L1 recover only\n", SER_USER_CMD_ENABLE_MASK_L1_RECOVER_ONLY));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Set L3 rx abort only\n", SER_USER_CMD_ENABLE_MASK_L3_RX_ABORT_ONLY));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Set L3 tx abort only\n", SER_USER_CMD_ENABLE_MASK_L3_TX_ABORT_ONLY));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Set L3 tx disable only\n", SER_USER_CMD_ENABLE_MASK_L3_TX_DISABLE_ONLY));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Set L3 BF recover only\n", SER_USER_CMD_ENABLE_MASK_L3_BFRECOVER_ONLY));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%d : Set All recover enable\n", SER_USER_CMD_ENABLE_MASK_RECOVER_ALL));

	return TRUE;
}

INT32 set_fw_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	CHAR *value;
	UCHAR *Input;
	UCHAR ExtendID;
	INT i, len;
	BOOLEAN fgStatus = FALSE;
	/* get ExtID */
	value = rstrtok(Arg, ":");
	if (value == NULL)
		return fgStatus;
	AtoH(value, &ExtendID, 1);
	/* get cmd raw data */
	value += 3;
	len = strlen(value) >> 1;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=======Set_FwCmd==========\n"));
	os_alloc_mem(pAd, (UCHAR **)&Input, len);

	for (i = 0; i < len; i++)
		AtoH(value + i * 2, &Input[i], 1);

	/* print cmd raw data */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("EID= 0x%x, CMD[%d] = ", ExtendID, len));

	for (i = 0; i < len; i++)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0x%x ", Input[i]));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	/* send cmd to fw */
	MtCmdSendRaw(pAd, ExtendID, Input, len, CMD_SET);
	os_free_mem((PVOID)Input);
	return fgStatus;
}

INT32 get_fw_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	CHAR *value;
	UCHAR *Input;
	UCHAR ExtendID;
	INT i, len;
	BOOLEAN fgStatus = FALSE;
	/* get ExtID */
	value = rstrtok(Arg, ":");
	if (value == NULL)
		return fgStatus;
	AtoH(value, &ExtendID, 1);
	/* get cmd raw data */
	value += 3;
	len = strlen(value) >> 1;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=======Get_FwCmd==========\n"));
	os_alloc_mem(pAd, (UCHAR **)&Input, len);

	for (i = 0; i < len; i++)
		AtoH(value + i * 2, &Input[i], 1);

	/* print cmd raw data */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("EID= 0x%x, CMD[%d] = ", ExtendID, len));

	for (i = 0; i < len; i++)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0x%x ", Input[i]));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	/* send cmd to fw */
	MtCmdSendRaw(pAd, ExtendID, Input, len, CMD_QUERY);
	os_free_mem((PVOID)Input);
	return fgStatus;
}

#ifdef FW_DUMP_SUPPORT
INT set_fwdump_max_size(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->fw_dump_max_size = os_str_tol(arg, 0, 10);
	return TRUE;
}

INT set_fwdump_path(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_OS_FWDUMP_SETPATH(pAd, arg);
	return TRUE;
}

INT fwdump_print(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 x;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: len = %d\n", __func__, pAd->fw_dump_size));

	for (x = 0; x < pAd->fw_dump_size; x++) {
		if (x % 16 == 0)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0x%04x : ", x));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x ", ((unsigned char)pAd->fw_dump_buffer[x])));

		if (x % 16 == 15)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	return TRUE;
}
#endif

#endif

#ifdef THERMAL_PROTECT_SUPPORT
INT set_thermal_protection_criteria_proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING		*arg)
{
	CHAR	 *value;
	UINT8	 ucParamIdx;
	BOOLEAN  fgHighEn = FALSE, fgLowEn = FALSE, fgRFOffEn = FALSE;
	CHAR     cHighTempTh = 0, cLowTempTh = 0, cRFOffTh = 0;
	UINT32   u4RechkTimer = 0;
	UINT8    ucType = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand;

	/* sanity check for input parameter*/
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No parameters!!\n", __func__));
		goto error;
	}

	/* Parsing input parameter */
	for (ucParamIdx = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), ucParamIdx++) {
		switch (ucParamIdx) {
		case 0:
			fgHighEn = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		case 1:
			fgLowEn = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		case 2:
			fgRFOffEn = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		case 3:
			cHighTempTh = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 4:
			cLowTempTh = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 5:
			cRFOffTh = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 6:
			u4RechkTimer = simple_strtol(value, 0, 10); /* 4-bit format */
			break;

		case 7:
			ucType = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Invalid Parameter Format!!\n", __func__));
			break;
		}
	}

	if (ucParamIdx != 8) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Wrong parameter format!!\n", __func__));
		goto error;
	}

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wdev is invalid\n", __func__));
		return FALSE;
	}

	ucBand = HcGetBandByWdev(wdev);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: fgHighEn: %d, fgLowEn: %d, fgRFOffEn: %d, cHighTempTh: %d, cLowTempTh: %d, cRFOffTh: %d\n", __func__, fgHighEn, fgLowEn, fgRFOffEn, cHighTempTh, cLowTempTh, cRFOffTh));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: u4RechkTimer: %d, ucType: %s\n", __func__, u4RechkTimer, ucType ? "Duty Cycle" : "TxStream"));

#ifdef MT_MAC
	AsicThermalProtect(pAd, ucBand, fgHighEn, cHighTempTh, fgLowEn, cLowTempTh, u4RechkTimer, fgRFOffEn, cRFOffTh, ucType);
#endif /* MT_MAC */

	return TRUE;

error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (KYEL "iwpriv <interface> set tpc=fgHighEn:fgLowEn:fgRFOffEn:cHighTempTh:cLowTempTh:cRFOffTh:u4RechkTimer:ucType\n" KNRM));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("    fgHighEn:    (1-bit format) High Temperature Protect Trigger Enable\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("    fgLowEn:     (1-bit format) Low Temperature Protect Trigger Enable\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("    fgRFOffEn:   (1-bit format) RF off Protect Trigger Enable\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("    cHighTempTh: (3-bit format) High Temperature Protect Trigger point\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("    cLowTempTh:  (3-bit format) Low Temperature Protect Trigger point\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("    cRFOffTh:    (3-bit format) RF off Protect Trigger point\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("    u4RechkTimer:(4-bit format) Thermal Protect Recheck period\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("    ucType:      (1-bit format) Thermal Protect Type (0: TxStream, 1: Duty Cycle)\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (KGRN "Ex: iwpriv ra0 set tpc=1:1:1:080:070:110:0060:1\n" KNRM));

	return FALSE;
}

INT set_thermal_protection_admin_ctrl_duty_proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING		*arg)
{
	CHAR   *value;
	UINT8  ucParamIdx;
	UINT32 u4Lv0Duty = 0, u4Lv1Duty = 0, u4Lv2Duty = 0, u4Lv3Duty = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand;

	/* sanity check for input parameter*/
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No parameters!!\n", __func__));
		goto error0;
	}

	/* Parsing input parameter */
	for (ucParamIdx = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), ucParamIdx++) {
		switch (ucParamIdx) {
		case 0:
			u4Lv0Duty = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 1:
			u4Lv1Duty = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 2:
			u4Lv2Duty = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 3:
			u4Lv3Duty = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Invalid Parameter Format!!\n", __func__));
			break;
		}
	}

	if (ucParamIdx != 4) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Wrong parameter format!!\n", __func__));
		goto error0;
	}

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wdev is invalid\n", __func__));
		return FALSE;
	}

	ucBand = HcGetBandByWdev(wdev);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: ucBand:%d, u4Lv0Duty:%d, u4Lv1Duty:%d, u4Lv2Duty:%d, u4Lv3Duty:%d\n", __func__,
			 ucBand, u4Lv0Duty, u4Lv1Duty, u4Lv2Duty, u4Lv3Duty));

	/* Parameter sanity check */
	if (u4Lv0Duty > 100) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Parameters error! Lv0Duty > 100\n"));
		goto error1;
	}

	if (u4Lv1Duty > u4Lv0Duty) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Parameters error! Lv1Duty > Lv0Duty"));
		goto error1;
	}

	if (u4Lv2Duty > u4Lv1Duty) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Parameters error! Lv2Duty > Lv1Duty"));
		goto error1;
	}

	if (u4Lv3Duty > u4Lv2Duty) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Parameters error! Lv3Duty > Lv2Duty"));
		goto error1;
	}

	AsicThermalProtectAdmitDuty(pAd, ucBand, u4Lv0Duty, u4Lv1Duty, u4Lv2Duty, u4Lv3Duty);

	return TRUE;

error0:

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (KYEL "iwpriv <interface> set tpc_duty=Lv0Duty:Lv1Duty:Lv2Duty:Lv3Duty\n" KNRM));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("    Lv0Duty: (3-bit format) Level 0 Protect Duty cycle\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("    Lv1Duty: (3-bit format) Level 1 Protect Duty cycle\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("    Lv2Duty: (3-bit format) Level 2 Protect Duty cycle\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("    Lv3Duty: (3-bit format) Level 3 Protect Duty cycle\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (KGRN "Ex: iwpriv ra0 set tpc_duty=100:080:050:030\n" KNRM));

	return FALSE;

error1:
	return FALSE;
}


INT get_thermal_protection_admin_ctrl_duty_proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING		* arg)
{
    return AsicThermalProtectAdmitDutyInfo(pAd);
}

#endif /* THERMAL_PROTECT_SUPPORT */

#ifdef CONFIG_DVT_MODE
#define STOP_DP_OUT	0x50029080
INT16 i2SetDvt(RTMP_ADAPTER *pAd, RTMP_STRING *pArg)
{
	/*
	test item=0: normal mode
	test item=1: test tx endpint, param1=
	*/
	INT16	i2ParameterNumber = 0;
	UCHAR	ucTestItem = 0;
	UCHAR	ucTestParam1 = 0;
	UCHAR	ucTestParam2 = 0;
	UINT32	u4Value;

	if (pArg) {
		i2ParameterNumber = sscanf(pArg, "%d,%d,%d", &(ucTestItem), &(ucTestParam1), &(ucTestParam2));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: i2ParameterNumber(%d), ucTestItem(%d), ucTestParam1(%d), ucTestParam2(%d)\n", __func__, i2ParameterNumber,
				  ucTestItem, ucTestParam1, ucTestParam2));
		pAd->rDvtCtrl.ucTestItem = ucTestItem;
		pAd->rDvtCtrl.ucTestParam1 = ucTestParam1;
		pAd->rDvtCtrl.ucTestParam2 = ucTestParam2;

		/* Tx Queue Mode*/
		if (pAd->rDvtCtrl.ucTestItem == 1) {
			pAd->rDvtCtrl.ucTxQMode = pAd->rDvtCtrl.ucTestItem;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucTxQMode(%d)\n", __func__, pAd->rDvtCtrl.ucTxQMode));
		} else if (pAd->rDvtCtrl.ucTestItem == 2) {
			pAd->rDvtCtrl.ucTxQMode = pAd->rDvtCtrl.ucTestItem;
			pAd->rDvtCtrl.ucQueIdx = pAd->rDvtCtrl.ucTestParam1;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucTxQMode(%d), ucQueIdx(%d)\n", __func__,
					 pAd->rDvtCtrl.ucTxQMode, pAd->rDvtCtrl.ucQueIdx));
		}
		/* UDMA Drop CR Access */
		else if (pAd->rDvtCtrl.ucTestItem == 3 && pAd->rDvtCtrl.ucTestParam1 == 0) {
			RTMP_IO_READ32(pAd->hdev_ctrl, STOP_DP_OUT, &u4Value);
			u4Value &= ~(BIT25 | BIT24 | BIT23 | BIT22 | BIT21 | BIT20);
			RTMP_IO_WRITE32(pAd->hdev_ctrl, STOP_DP_OUT, u4Value);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Set Drop=0\n", __func__));
		} else if (pAd->rDvtCtrl.ucTestItem == 3 && pAd->rDvtCtrl.ucTestParam1 == 1) {
			RTMP_IO_READ32(pAd->hdev_ctrl, STOP_DP_OUT, &u4Value);
			u4Value |= (BIT25 | BIT24 | BIT23 | BIT22 | BIT21 | BIT20);
			RTMP_IO_WRITE32(pAd->hdev_ctrl, STOP_DP_OUT, u4Value);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Set Drop=1\n", __func__));
		} else if (pAd->rDvtCtrl.ucTestItem == 4 && pAd->rDvtCtrl.ucTestParam1 == 0) {
			RTMP_IO_READ32(pAd->hdev_ctrl, STOP_DP_OUT, &u4Value);
			u4Value &= ~(BIT9 | BIT8 | BIT7 | BIT6 | BIT5 | BIT4);
			RTMP_IO_WRITE32(pAd->hdev_ctrl, STOP_DP_OUT, u4Value);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Set Stop=0\n", __func__));
		} else if (pAd->rDvtCtrl.ucTestItem == 4 && pAd->rDvtCtrl.ucTestParam1 == 1) {
			RTMP_IO_READ32(pAd->hdev_ctrl, STOP_DP_OUT, &u4Value);
			u4Value |= (BIT9 | BIT8 | BIT7 | BIT6 | BIT5 | BIT4);
			RTMP_IO_WRITE32(pAd->hdev_ctrl, STOP_DP_OUT, u4Value);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Set Stop=1\n", __func__));
		}
		/* ACQTxNumber */
		else if ((pAd->rDvtCtrl.ucTestItem == 5) && (pAd->rDvtCtrl.ucTestParam1 == 0)) {
			UCHAR ucIdx = 0;

			for (ucIdx = 0; ucIdx < 5; ucIdx++)
				pAd->rDvtCtrl.au4ACQTxNum[ucIdx] = 0;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s: Reset au4ACQTxNum EP4_AC0(%d), EP5_AC1(%d), EP6_AC2(%d), EP7_AC3(%d), EP9_AC0(%d)\n",
					  __func__,
					  pAd->rDvtCtrl.au4ACQTxNum[0],
					  pAd->rDvtCtrl.au4ACQTxNum[1],
					  pAd->rDvtCtrl.au4ACQTxNum[2],
					  pAd->rDvtCtrl.au4ACQTxNum[3],
					  pAd->rDvtCtrl.au4ACQTxNum[4]));
		} else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: i2ParameterNumber(%d), ucTestItem(%d), parameters error\n",
					 __func__, i2ParameterNumber, ucTestItem));
	}

	return TRUE;
}
#endif /* CONFIG_DVT_MODE */

#ifdef MT_MAC
VOID StatRateToString(RTMP_ADAPTER *pAd, CHAR *Output, UCHAR TxRx, UINT32 RawData)
{
	extern UCHAR tmi_rate_map_ofdm[];
	extern UCHAR tmi_rate_map_cck_lp[];
	extern UCHAR tmi_rate_map_cck_sp[];
	UCHAR phy_mode, rate, bw, preamble, sgi, vht_nss;
	UCHAR phy_idx, bw_idx = 0;
	CHAR *phyMode[6] = {"CCK", "OFDM", "MM", "GF", "VHT", "unknow"};
	CHAR *FecCoding[2] = {"BCC", "LDPC"};
	CHAR *bwMode[4] = {"BW20", "BW40", "BW80", "BW160/8080"};

	phy_mode = (RawData >> 13) & 0x7;
	rate = RawData & 0x3F;
	bw = (RawData >> 7) & 0x3;
	sgi = (RawData >> 9) & 0x1;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
		preamble = SHORT_PREAMBLE;
	else
		preamble = LONG_PREAMBLE;

	if (phy_mode <= MODE_VHT)
		phy_idx = phy_mode;
	else
		phy_idx = 5;

	if (bw == BW_20)
		bw_idx = 0;
	else if (bw == BW_40)
		bw_idx = 1;
	else if (bw == BW_80)
		bw_idx = 2;
	else if (bw == BW_160)
		bw_idx = 3;

	if (TxRx == 0)
		sprintf(Output + strlen(Output), "Last TX Rate                    = ");
	else
		sprintf(Output + strlen(Output), "Last RX Rate                    = ");

	if (phy_mode == MODE_CCK) {
		if (TxRx == 0) {
			if (preamble)
				rate = tmi_rate_map_cck_lp[rate];
			else
				rate = tmi_rate_map_cck_sp[rate];
		}

		if (rate == TMI_TX_RATE_CCK_1M_LP)
			sprintf(Output + strlen(Output), "1M LP, ");
		else if (rate == TMI_TX_RATE_CCK_2M_LP)
			sprintf(Output + strlen(Output), "2M LP, ");
		else if (rate == TMI_TX_RATE_CCK_5M_LP)
			sprintf(Output + strlen(Output), "5M LP, ");
		else if (rate == TMI_TX_RATE_CCK_11M_LP)
			sprintf(Output + strlen(Output), "11M LP, ");
		else if (rate == TMI_TX_RATE_CCK_2M_SP)
			sprintf(Output + strlen(Output), "2M SP, ");
		else if (rate == TMI_TX_RATE_CCK_5M_SP)
			sprintf(Output + strlen(Output), "5M SP, ");
		else if (rate == TMI_TX_RATE_CCK_11M_SP)
			sprintf(Output + strlen(Output), "11M SP, ");
		else
			sprintf(Output + strlen(Output), "unkonw, ");
	} else if (phy_mode == MODE_OFDM) {
		if (TxRx == 0)
			rate = tmi_rate_map_ofdm[rate];

		if (rate == TMI_TX_RATE_OFDM_6M)
			sprintf(Output + strlen(Output), "6M, ");
		else if (rate == TMI_TX_RATE_OFDM_9M)
			sprintf(Output + strlen(Output), "9M, ");
		else if (rate == TMI_TX_RATE_OFDM_12M)
			sprintf(Output + strlen(Output), "12M, ");
		else if (rate == TMI_TX_RATE_OFDM_18M)
			sprintf(Output + strlen(Output), "18M, ");
		else if (rate == TMI_TX_RATE_OFDM_24M)
			sprintf(Output + strlen(Output), "24M, ");
		else if (rate == TMI_TX_RATE_OFDM_36M)
			sprintf(Output + strlen(Output), "36M, ");
		else if (rate == TMI_TX_RATE_OFDM_48M)
			sprintf(Output + strlen(Output), "48M, ");
		else if (rate == TMI_TX_RATE_OFDM_54M)
			sprintf(Output + strlen(Output), "54M, ");
		else
			sprintf(Output + strlen(Output), "unkonw, ");
	} else if (phy_mode == MODE_VHT) {
		vht_nss = ((rate & (0x3 << 4)) >> 4) + 1;
		rate = rate & 0xF;
		sprintf(Output + strlen(Output), "NSS%d_MCS%d, ", vht_nss, rate);
	} else
		sprintf(Output + strlen(Output), "MCS%d, ", rate);

	sprintf(Output + strlen(Output), "%s, ", bwMode[bw_idx]);
	sprintf(Output + strlen(Output), "%cGI, ", sgi ? 'S' : 'L');
	sprintf(Output + strlen(Output), "%s%s %s\n",
			phyMode[phy_idx],
			((RawData >> 10) & 0x1) ? ", STBC" : " ",
			FecCoding[((RawData >> 6) & 0x1)]);
}

INT Set_themal_sensor(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* 0: get temperature; 1: get adc */
	UINT32 value;
	UINT32 Sensor = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand;

	value = os_str_tol(arg, 0, 10);

	if ((value == 0) || (value == 1)) {
		if (wdev == NULL) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wdev is invalid\n", __func__));
			return FALSE;
		}

		ucBand = HcGetBandByWdev(wdev);

		MtCmdGetThermalSensorResult(pAd, value, ucBand, &Sensor);
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: 0: get temperature; 1: get adc\n", __func__));

	return TRUE;
}


INT set_manual_rdg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32   ret = 0;
	UINT32   init, resp, txop, wcid, band;

	ret = sscanf(arg, "%u-%u-%u-%u-%u",
				 &(init), &(resp), &(txop), &(wcid), &(band));

	if (ret != 5) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Format Error!! should be: iwpriv ra0 set manual_rdg=[init]-[resp]-[txop]-[wcid]-[band]\n"));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\n>> Initiator=%x, Responder=%x, Txop=0x%x, Wcid=%u, BandIdx=%x\n",
				  init, resp, txop, wcid, band));

		AsicSetRDG(pAd, wcid, band, init, resp);
	}

	return TRUE;
}
#endif /* MT_MAC */

#ifdef SCS_FW_OFFLOAD

static VOID ScsEventCallback(struct cmd_msg *msg, char *rsp_payload, UINT16 rsp_payload_len);

static VOID scs_show_info_callback(struct cmd_msg *msg, char *rsp_payload,	UINT16 rsp_payload_len);

/*----------------------------------------------------------------------------*/
/*!
* @brief [CMD] set SCS Phy counters from FW
*
* @param pucParam
*
* @return status
*/
/*----------------------------------------------------------------------------*/
static VOID scs_show_info_callback(
	struct cmd_msg *msg,
	char *rsp_payload,
	UINT16 rsp_payload_len)
{
	PSCS_SHOW_INFO_T pData = NULL;
	UINT_8 i;
	if (rsp_payload == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: error !! rsp_payload is null!!\n", __func__));
		return;
	}

	pData = (PSCS_SHOW_INFO_T) rsp_payload;

	for (i = 0; i < DBDC_BAND_NUM; i++) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("************** Band%d  Information*************\n", i));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("SCSTrafficThreshold =%u\n", pData->rScsInfo.au4SCSTrafficThreshold[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("CckFalseCcaUpBound =%d\n", pData->rScsInfo.ai4CckFalseCcaUpBound[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("CckFalseCcaLowBound =%d\n", pData->rScsInfo.ai4CckFalseCcaLowBound[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("CckFalseCcaCount =%d\n", pData->rScsInfo.ai4CckFalseCcaCount[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OFDMFalseCcaCount =%d\n", pData->rScsInfo.ai4OFDMFalseCcaCount[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OneSecTxByteCount =%u\n", pData->rScsInfo.au4OneSecTxByteCount[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OneSecRxByteCount =%u\n", pData->rScsInfo.au4OneSecRxByteCount[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("RtsCount =%u\n", pData->rScsInfo.au4RtsCount[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("RtsRtyCount =%u\n", pData->rScsInfo.au4RtsRtyCount[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OfdmFalseCcaUpBound =%u\n", (UINT_32)pData->rScsInfo.au2OfdmFalseCcaUpBound[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OfdmFalseCcaLowBound =%u\n", (UINT_32)pData->rScsInfo.au2OfdmFalseCcaLowBound[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("CckFixedRssiBound =%u\n", (UINT_32)pData->rScsInfo.au2CckFixedRssiBound[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OfdmFixedRssiBound =%u\n", (UINT_32)pData->rScsInfo.au2OfdmFixedRssiBound[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OfdmPdBlkTh =%u\n", (UINT_32)pData->rScsInfo.au2OfdmPdBlkTh[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("SCSMinRssi =%d\n", (INT_32)pData->rScsInfo.ai1SCSMinRssi[i]));


			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("SCSStatus =%u\n", (UINT_32)pData->rScsInfo.au1SCSStatus[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("CckPdBlkTh =%u\n", (UINT_32)pData->rScsInfo.au1CckPdBlkTh[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("SCSThTolerance =%u\n", (UINT_32)pData->rScsInfo.au1SCSThTolerance[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("SCSMinRssiTolerance =%u\n", (UINT_32)pData->rScsInfo.au1SCSMinRssiTolerance[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("SCSEnable =%u\n", (UINT_32)pData->rScsInfo.au1SCSEnable[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OfdmPdSupport =%u\n", (UINT_32)pData->rScsInfo.afgOfdmPdSupport[i]));

	}

}


/*
==========================================================================
Description:
	SCS Event Handler

Parameters:
	Standard SCS Paramter

==========================================================================
 */
static VOID ScsEventCallback(
	struct cmd_msg *msg,
	char *rsp_payload,
	UINT16 rsp_payload_len)
{

	UINT32 u4EventId = (*(UINT32 *) rsp_payload);
	char *pData = (rsp_payload);
	UINT16 len = (rsp_payload_len);
#ifdef RT_BIG_ENDIAN
	u4EventId = cpu2le32(u4EventId);
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: u4EventId = %u, len = %u\n", __func__, u4EventId, len));

	switch (u4EventId) {
	case SCS_SHOW_INFO:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: SCS_SHOW_INFO\n", __func__));
		scs_show_info_callback(msg, pData, len);
		break;

	default:
		break;
	}
}

/* ==========================================================================
Description:
	Send SCS data to FW

Parameters:
	Standard SCS  Paramters

==========================================================================
 */
INT SendSCSDataProc(RTMP_ADAPTER *pAd)
{
	INT32 Ret = TRUE;
	UINT_8 u1BandIndex = 0;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = SCS_EVENT_SEND_DATA;
	CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW Param;
	PSMART_CARRIER_SENSE_CTRL    pSCSCtrl;

	pSCSCtrl = &pAd->SCSCtrl;

	if (pSCSCtrl != NULL) {
		for (u1BandIndex = 0; u1BandIndex < DBDC_BAND_NUM; u1BandIndex++) {
			if (pSCSCtrl->SCSEnable[u1BandIndex] == SCS_ENABLE) {
				Param.OneSecRxByteCount[u1BandIndex] = pSCSCtrl->OneSecRxByteCount[u1BandIndex];
				Param.OneSecTxByteCount[u1BandIndex] = pSCSCtrl->OneSecTxByteCount[u1BandIndex];
				Param.SCSMinRssi[u1BandIndex] = pSCSCtrl->SCSMinRssi[u1BandIndex];			}
		}
	} else {
		Ret = FALSE;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(Param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&Param, sizeof(Param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT Show_SCS_FW_Offload_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 u4Index = 0;
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = SCS_SHOW_INFO;
	struct _CMD_ATTRIBUTE attr = {0};
	SCS_SHOW_INFO_T stat_result = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(stat_result));
	if (!msg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Msg allocation failed\n", __func__));
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(stat_result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &stat_result);
	SET_CMD_ATTR_RSP_HANDLER(attr, ScsEventCallback);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));

	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s:(Ret = %d\n", __func__, Ret));

	return Ret;
}

INT GetScsFwCounterProc(RTMP_ADAPTER *pAd)
{
	UINT32 u4Index = 0;
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = SCS_MIB_COUNTERS;
	struct _CMD_ATTRIBUTE attr = {0};
	SCS_SEND_MIB_INFO_T stat_result = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(stat_result));
	if (!msg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Msg allocation failed\n", __func__));
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(stat_result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &stat_result);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);/*ScsEventCallback*/
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));

	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s:(Ret = %d\n", __func__, Ret));

	return Ret;
}

#endif /* SCS_FW_OFFLOAD */

#ifdef SMART_CARRIER_SENSE_SUPPORT
INT Set_SCSEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef SCS_FW_OFFLOAD

	INT32 Ret = TRUE;

	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = SCS_EVENT_SCS_ENABLE;
	CMD_SMART_CARRIER_ENABLE Param = {0};
	UINT_32 BandIdx, SCSEnable;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	SCSEnable = os_str_tol(arg, 0, 10);

	struct wifi_dev *wdev;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);

	Param.BandIdx = (UINT_8) BandIdx;
	Param.SCSEnable = (UINT_8) SCSEnable;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): BandIdx=%d, SCSEnable=%d\n", __func__, BandIdx, SCSEnable));
	if (SCSEnable == SCS_DISABLE) {
		pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_DISABLE;
		pAd->SCSCtrl.SCSStatus[BandIdx] = PD_BLOCKING_OFF;
	} else if (SCSEnable == SCS_ENABLE) {
		pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_ENABLE;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(Param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&Param, sizeof(Param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;

	/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()  BandIdx=%d, u1SCSMinRssiTolerance=%d */
	/* , u1SCSThTolerance=%d, u4SCSTrafficThreshold=%d\n", __func__, */
	/* BandIdx, u1SCSMinRssiTolerance, u1SCSThTolerance, u4SCSTrafficThreshold)); */
	return TRUE;

#else
	UINT32  SCSEnable;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	UCHAR BandIdx = 0;
	struct wifi_dev *wdev;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);
	SCSEnable = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()  BandIdx=%d, SCSEnable=%d\n", __func__, BandIdx, SCSEnable));
	RTMP_CHIP_ASIC_SET_SCS(pAd, BandIdx, SCSEnable);
	return TRUE;
#endif
}

INT Set_SCSCfg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef SCS_FW_OFFLOAD
	INT32  Ret = TRUE;
	INT32  Recv = 0;
	INT_32 BandIdx = 0;
	INT_32 SCSThTolerance;
	INT_32 SCSMinRssiTolerance;
	INT_32 OfdmFalseCcaUpBound;
	INT_32 OfdmFalseCcaLowBound;
	INT_32 CckFixedRssiBound;
	INT_32 OfdmPdSupport;
	INT_32 OfdmFixedRssiBound;
	INT_32 SCSTrafficThreshold;
	INT_32 CckFalseCcaUpBound;
	INT_32 CckFalseCcaLowBound;
	struct wifi_dev *wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = SCS_EVENT_CONFIG_SCS;
	CMD_SMART_CARRIER_SENSE_CONFIG Param = {0};

	BandIdx = HcGetBandByWdev(wdev);

	Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", &(SCSMinRssiTolerance), &(SCSThTolerance), &(SCSTrafficThreshold),
					  &(OfdmPdSupport), &(CckFalseCcaUpBound), &(CckFalseCcaLowBound), &(OfdmFalseCcaUpBound), &(OfdmFalseCcaLowBound), &(CckFixedRssiBound), &(OfdmFixedRssiBound));

		if ((Recv != 10) || (BandIdx > DBDC_BAND_NUM - 1)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Max supported bands = %d (Supported Band Number is out of range)\n", DBDC_BAND_NUM));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Format Error!\n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("iwpriv ra0 set SCSCfg=[MinRssiTolerance]-[ThTolerance]-[TrafficThreshold]-[OfdmSupport]-[CckUpBoundary]-[CckLowBoundary]-[OfdmUpBoundary]-[OfdmLowBoundary]-[FixedCckBoundary]-[FixedOfdmBoundary]"));
		} else {
			Param.u1BandIdx = (UINT_8)BandIdx;
			Param.u1SCSMinRssiTolerance = (UINT_8)SCSMinRssiTolerance;
			Param.u1SCSThTolerance = (UINT_8)SCSThTolerance;
			Param.u4SCSTrafficThreshold = (UINT_32)SCSTrafficThreshold;
			Param.fgOfdmPdSupport = (BOOL)OfdmPdSupport;
			Param.i4CckFalseCcaUpBound = (INT_32)CckFalseCcaUpBound;
			Param.i4CckFalseCcaLowBound = (INT_32)CckFalseCcaLowBound;
			Param.u2OfdmFalseCcaUpBound = (UINT_16)OfdmFalseCcaUpBound;
			Param.u2OfdmFalseCcaLowBound = (UINT_16)OfdmFalseCcaLowBound;
			Param.u2CckFixedRssiBound = (UINT_16)CckFixedRssiBound;
			Param.u2OfdmFixedRssiBound = (UINT_16)OfdmFixedRssiBound;
		}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(Param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&Param, sizeof(Param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;

	/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()  BandIdx=%d, u1SCSMinRssiTolerance=%d */
	/* , u1SCSThTolerance=%d, u4SCSTrafficThreshold=%d\n", __func__, */
	/* BandIdx, u1SCSMinRssiTolerance, u1SCSThTolerance, u4SCSTrafficThreshold)); */
	return TRUE;

#else
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	UCHAR BandIdx = 0;
	INT32 SCSMinRssiTolerance = 0;
	INT32 SCSThTolerance = 0;
	UINT32 SCSTrafficThreshold = 0;
	UINT32 OfdmPdSupport = 0;
	INT32   Recv = 0;
	UINT32 CckUpBond = 0, CckLowBond = 0, CckFixedBond = 0;
	UINT32  OfdmUpBond = 0, OfdmLowBond = 0, OfdmFixedBond = 0;
	struct wifi_dev *wdev;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	BandIdx = HcGetBandByWdev(wdev);

	Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", &(SCSMinRssiTolerance), &(SCSThTolerance), &(SCSTrafficThreshold),
				  &(OfdmPdSupport), &(CckUpBond), &(CckLowBond), &(OfdmUpBond), &(OfdmLowBond), &(CckFixedBond), &(OfdmFixedBond));

	if (Recv != 10) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error!\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("iwpriv ra0 set SCSCfg=[MinRssiTolerance]-[ThTolerance]-[TrafficThreshold]-[OfdmSupport]-[CckUpBoundary]-[CckLowBoundary]-[OfdmUpBoundary]-[OfdmLowBoundary]-[FixedCckBoundary]-[FixedOfdmBoundary]"));
	} else {
		pAd->SCSCtrl.SCSMinRssiTolerance[BandIdx] = SCSMinRssiTolerance;
		pAd->SCSCtrl.SCSThTolerance[BandIdx] = SCSThTolerance;
		pAd->SCSCtrl.SCSTrafficThreshold[BandIdx] = SCSTrafficThreshold;
		pAd->SCSCtrl.OfdmPdSupport[BandIdx] = (UCHAR) OfdmPdSupport;
		pAd->SCSCtrl.CckFalseCcaUpBond[BandIdx] = (UINT16)CckUpBond;
		pAd->SCSCtrl.CckFalseCcaLowBond[BandIdx] = (UINT16)CckLowBond;
		pAd->SCSCtrl.OfdmFalseCcaUpBond[BandIdx] = (UINT16)OfdmUpBond;
		pAd->SCSCtrl.OfdmFalseCcaLowBond[BandIdx] = (UINT16)OfdmLowBond;
		pAd->SCSCtrl.CckFixedRssiBond[BandIdx] = (INT32)CckFixedBond;
		pAd->SCSCtrl.OfdmFixedRssiBond[BandIdx] = (INT32)OfdmFixedBond;
	}

	/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()  BandIdx=%d, SCSMinRssiTolerance=%d */
	/* , SCSThTolerance=%d, SCSTrafficThreshold=%d\n", __func__, */
	/* BandIdx, SCSMinRssiTolerance, SCSThTolerance, SCSTrafficThreshold)); */
	return TRUE;
#endif
}

INT Set_SCSPd_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef SCS_FW_OFFLOAD

	INT32	Recv = 0;
	INT32	Ret = TRUE;
	INT32	BandIdx = 0;
	INT32	CckPdBlkTh = 0;
	INT32	OfdmPdBlkTh = 0;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = SCS_EVENT_SET_MANUAL_PD_TH;
	CMD_SMART_CARRIER_SENSE_CTRL_PD_TH Param = {0};
	struct wifi_dev *wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	BandIdx = HcGetBandByWdev(wdev);

	Recv = sscanf(arg, "%d-%d-%d",	&(BandIdx), &(CckPdBlkTh), &(OfdmPdBlkTh));
	if (Recv != 3 || (CckPdBlkTh < 30 || CckPdBlkTh > 110)
			|| (OfdmPdBlkTh < 30 || OfdmPdBlkTh > 98) || (BandIdx > DBDC_BAND_NUM - 1)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("Format Error or Out of range\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("iwpriv ra0 set SCSCfg=[CckPdBlkTh]-[OfdmPdBlkTh]\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("CckPdBlkTh  Range: 30~110 dBm (Represents a negative number)\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("OfdmPdBlkTh Range: 30~98	dBm (Represents a negative number)\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("Max supported bands = %d (Supported Band Number is out of range)\n", DBDC_BAND_NUM));
		goto error;
	} else {
		Param.BandIdx = (UINT_8)BandIdx;
		Param.SCSEnable = SCS_MANUAL;
		Param.CckPdBlkTh = ((CckPdBlkTh * (-1)) + 256);
		Param.OfdmPdBlkTh = ((OfdmPdBlkTh * (-2)) + 512);

		msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(Param));

		if (!msg) {
			Ret = FALSE;
			goto error;
		}

		SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
		SET_CMD_ATTR_TYPE(attr, EXT_CID);
		SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
		SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
		SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
		AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
		cmd = cpu2le32(cmd);
#endif
		AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
		AndesAppendCmdMsg(msg, (char *)&Param, sizeof(Param));
		AndesSendCmdMsg(pAd, msg);
	}
		error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;

#else
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	UCHAR BandIdx = 0;
	INT32	CckPdBlkTh = 0;
	INT32	OfdmPdBlkTh = 0;
	INT32   Recv = 0;
	UINT32 CrValue;
	struct wifi_dev *wdev;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);

	Recv = sscanf(arg, "%d-%d", &(CckPdBlkTh), &(OfdmPdBlkTh));
	if (Recv != 2 || (CckPdBlkTh < 30 || CckPdBlkTh > 110) || (OfdmPdBlkTh < 30 || OfdmPdBlkTh > 98)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error or Out of range\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("iwpriv ra0 set SCSCfg=[CckPdBlkTh]-[OfdmPdBlkTh]\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CckPdBlkTh  Range: 30~110 dBm (Represents a negative number)\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("OfdmPdBlkTh Range: 30~98  dBm (Represents a negative number)\n"));
	} else {
		pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_MANUAL;
		pAd->SCSCtrl.CckPdBlkTh[BandIdx] = ((CckPdBlkTh * (-1)) + 256);
		pAd->SCSCtrl.OfdmPdBlkTh[BandIdx] = ((OfdmPdBlkTh * (-2)) + 512);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Band%d CckPdBlkTh  = -%ddBm (%d)\n",
			BandIdx, CckPdBlkTh, pAd->SCSCtrl.CckPdBlkTh[BandIdx]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Band%d OfdmPdBlkTh = -%ddBm (%d)\n",
			BandIdx, OfdmPdBlkTh, pAd->SCSCtrl.OfdmPdBlkTh[BandIdx]));
		HW_IO_READ32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR, &CrValue);
		CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);  /* OFDM PD BLOCKING TH */
		CrValue |= (pAd->SCSCtrl.OfdmPdBlkTh[BandIdx] << PdBlkOfmdThOffset);
		HW_IO_WRITE32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR, CrValue);
		HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7, &CrValue);
		CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset); /* Bit[8:1] */
		CrValue |= (pAd->SCSCtrl.CckPdBlkTh[BandIdx] << PdBlkCckThOffset);
		HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7, CrValue);
		HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8, &CrValue);
		CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset); /* Bit[31:24] */
		CrValue |= (pAd->SCSCtrl.CckPdBlkTh[BandIdx] << PdBlkCck1RThOffset);
		HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8, CrValue);
	}
	return TRUE;
#endif
}
#endif /* SMART_CARRIER_SENSE_SUPPORT */

INT SetSKUCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR	*value = 0;
	BOOLEAN fgTxPowerSKUEn = FALSE;
	INT	 status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR	   apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucBandIdx = %d\n", __func__, ucBandIdx));

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

#ifdef RF_LOCKDOWN

	/* Check RF lock Status */
	if (chip_check_rf_lock_down(pAd)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! Cannot config SKU status!!\n",
				 __func__));
		return TRUE;
	}

#endif /* RF_LOCKDOWN */

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!!\n",
				 __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgTxPowerSKUEn = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((fgTxPowerSKUEn != FALSE) && (fgTxPowerSKUEn != TRUE)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxPowerSKUEn: %d\n", __func__, fgTxPowerSKUEn));
	/* Update Profile Info for SKU */
#ifdef SINGLE_SKU_V2
	pAd->CommonCfg.SKUenable[ucBandIdx] = fgTxPowerSKUEn;
#endif /* SINGLE_SKU_V2 */

#ifdef CONFIG_ATE
	/* Update SKU Status in ATECTRL Structure */
	if (ucBandIdx == BAND0)
		ATECtrl->fgTxPowerSKUEn = fgTxPowerSKUEn;
#ifdef DBDC_MODE
	else if (ucBandIdx == BAND1)
		Info->fgTxPowerSKUEn = fgTxPowerSKUEn;
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */

#ifdef SINGLE_SKU_V2
	return TxPowerSKUCtrl(pAd, fgTxPowerSKUEn, ucBandIdx);
#else
	return TRUE;
#endif /* SINGLE_SKU_V2 */
}

INT SetPercentageCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = 0;
	BOOLEAN fgTxPowerPercentEn = FALSE;
	INT     status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucBandIdx = %d\n", __func__, ucBandIdx));

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!!\n",
				 __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgTxPowerPercentEn = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((fgTxPowerPercentEn != FALSE) && (fgTxPowerPercentEn != TRUE)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxPowerPercentEn = %d\n", __func__, fgTxPowerPercentEn));

	/* Update Profile Info for Power Percentage */
	pAd->CommonCfg.PERCENTAGEenable[ucBandIdx] = fgTxPowerPercentEn;

#ifdef CONFIG_ATE
	/* Update Power Percentage Status in ATECTRL Structure */
	if (ucBandIdx == BAND0)
		ATECtrl->fgTxPowerPercentageEn = fgTxPowerPercentEn;
#ifdef DBDC_MODE
	else if (ucBandIdx == BAND1)
		Info->fgTxPowerPercentageEn = fgTxPowerPercentEn;
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */

	return TxPowerPercentCtrl(pAd, fgTxPowerPercentEn, ucBandIdx);
}

INT SetPowerDropCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR	*value = 0;
	UINT8	ucPowerDrop = 0;
	INT     status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */

	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			ucPowerDrop = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucPowerDrop = %d\n", __func__, ucPowerDrop));

	/* Update Profile Info for Power Percentage Drop Value */
	pAd->CommonCfg.ucTxPowerPercentage[ucBandIdx] = ucPowerDrop;

#ifdef CONFIG_ATE
	/* Update Power Percentage Drop Value Status in ATECTRL Structure */
	if (ucBandIdx == BAND0)
		ATECtrl->PercentageLevel = ucPowerDrop;
#ifdef DBDC_MODE
	else if (ucBandIdx == BAND1)
		Info->PercentageLevel = ucPowerDrop;
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */

	return TxPowerDropCtrl(pAd, ucPowerDrop, ucBandIdx);
}

INT SetBfBackoffCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR    *value = 0;
	BOOLEAN fgTxBFBackoffEn = FALSE;
	INT     status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucBandIdx = %d\n", __func__, ucBandIdx));

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

#ifdef RF_LOCKDOWN

	/* Check RF lock Status */
	if (chip_check_rf_lock_down(pAd)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! Cannot config BF Backoff status!!\n", __func__));
		return TRUE;
	}

#endif /* RF_LOCKDOWN */

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgTxBFBackoffEn = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((fgTxBFBackoffEn != FALSE) && (fgTxBFBackoffEn != TRUE)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxBFBackoffEn = %d\n", __func__, fgTxBFBackoffEn));

	/* Update Profile Info for Power Percentage */
	pAd->CommonCfg.BFBACKOFFenable[ucBandIdx] = fgTxBFBackoffEn;

#ifdef CONFIG_ATE
	/* Update Power Percentage Drop Value Status in ATECTRL Structure */
	if (ucBandIdx == BAND0)
		ATECtrl->fgTxPowerBFBackoffEn = fgTxBFBackoffEn;
#ifdef DBDC_MODE
	else if (ucBandIdx == BAND1)
		Info->fgTxPowerBFBackoffEn = fgTxBFBackoffEn;
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */

#ifdef SINGLE_SKU_V2
	return TxPowerBfBackoffCtrl(pAd, fgTxBFBackoffEn, ucBandIdx);
#else
	return TRUE;
#endif /* SINGLE_SKU_V2 */
}

INT SetThermoCompCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR    *value = 0;
	BOOLEAN fgThermoCompEn = 0;
	INT     status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucBandIdx = %d\n", __func__, ucBandIdx));

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!!\n",
				 __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgThermoCompEn = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((fgThermoCompEn != FALSE) && (fgThermoCompEn != TRUE)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgThermoCompEn = %d\n", __func__, fgThermoCompEn));
	return ThermoCompCtrl(pAd, fgThermoCompEn, ucBandIdx);
}

INT SetCCKTxStream(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR    *value = 0;
	UINT8   u1CCKTxStream = 0;
	INT     status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;

#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	wdev = &pAd->StaCfg[0].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_STA_SUPPORT */

	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			u1CCKTxStream = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	/* sanity check for input parameter range */
	if (u1CCKTxStream >= WF_NUM || !u1CCKTxStream) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("EX. iwpriv <interface> set CCKTxStream=1 (1~4)\n"));
		return FALSE;
	}

	/* Update Profile Info for Power Percentage Drop Value */
	pAd->CommonCfg.CCKTxStream[ucBandIdx] = u1CCKTxStream;

	return TxCCKStreamCtrl(pAd, pAd->CommonCfg.CCKTxStream[ucBandIdx], ucBandIdx);
}
#ifdef ANTENNA_CONTROL_SUPPORT
INT Antenna_Control_Init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UINT8 BandIdx = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct mcs_nss_caps *nss_cap = &cap->mcs_nss;

	BandIdx = HcGetBandByWdev(wdev);
	if (pAd->TxStream[BandIdx] && pAd->RxStream[BandIdx]) {
		wlan_config_set_tx_stream(wdev, min(pAd->TxStream[BandIdx], nss_cap->max_nss));
		wlan_config_set_rx_stream(wdev, min(pAd->RxStream[BandIdx], nss_cap->max_nss));
		wlan_operate_set_tx_stream(wdev, min(pAd->TxStream[BandIdx], nss_cap->max_nss));
		wlan_operate_set_rx_stream(wdev, min(pAd->RxStream[BandIdx], nss_cap->max_nss));

		SetCommonHtVht(pAd, wdev);
	}
	return TRUE;
}
INT Show_Antenna_Control_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx = 0;
	struct wifi_dev *wdev;

#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ShowAntennaControl_Info::(TxStream =%d,RxStream =%d)\n",
		 (pAd->TxStream[BandIdx]) ? pAd->TxStream[BandIdx] : wlan_operate_get_tx_stream(wdev),
		 (pAd->RxStream[BandIdx]) ? pAd->RxStream[BandIdx] : wlan_operate_get_rx_stream(wdev)));
	return TRUE;
}
INT Set_Antenna_Control_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx = 0, Txstream = 0, Rxstream = 0;
	struct wifi_dev *wdev;
	BSS_STRUCT *pMbss = NULL;

	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct mcs_nss_caps *nss_cap = &cap->mcs_nss;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Incoorect BSS!! \n", __FUNCTION__));
		return FALSE;
	}
	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
		return FALSE;
	}

	if (0 == simple_strtol(&arg[0], 0, 10)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: Set default Antenna number!!\n", __func__));
			Txstream = Rxstream = nss_cap->max_nss;
			goto set_default;
	}
	if (strlen(arg) != 4) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Please use input format like XTXR (X = 1,2,3,4)!!\n", __func__));
		return FALSE;
	}
	if (((arg[1] == 'T') || (arg[1] == 't')) && ((arg[3] == 'R') || (arg[3] == 'r'))) {
		Txstream = simple_strtol(&arg[0], 0, 10);
		Rxstream = simple_strtol(&arg[2], 0, 10);

		if (Txstream != Rxstream) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s : Tx & Rx Antenna number different, configure same!!\n", __func__));
			return FALSE;
		}

		if (Txstream > nss_cap->max_nss) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s:Wrong Configuration Ant num > MAX Support == %d!!\n", __func__, nss_cap->max_nss));
			return FALSE;
		}

#ifdef DBDC_MODE
		if (Txstream > nss_cap->max_nss) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s:Wrong Configuration Ant num > MAX Support == %d!!\n", __func__, nss_cap->max_nss));
			return FALSE;
		}
#endif
set_default:
		pAd->TxStream[BandIdx]				= Txstream;
		pAd->RxStream[BandIdx]				= Rxstream;
		pAd->bAntennaSetAPEnable[BandIdx]	= 1;
		wlan_config_set_tx_stream(wdev, min(Txstream, nss_cap->max_nss));
		wlan_config_set_rx_stream(wdev, min(Rxstream, nss_cap->max_nss));
		wlan_operate_set_tx_stream(wdev, min(Txstream, nss_cap->max_nss));
		wlan_operate_set_rx_stream(wdev, min(Rxstream, nss_cap->max_nss));

		SetCommonHtVht(pAd, wdev);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Wrong Params, xTxR (x=1 or 2 or 3 or 4!!\n"));
		return FALSE;
	}
	if (pMbss != NULL) {
		APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
		APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
		return TRUE;
	} else
		return FALSE;
}
#endif
#ifdef NF_SUPPORT
INT Show_noise_floor_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx = 0;
	struct wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("NF:%d\n", pAd->Avg_NF[BandIdx]));
	return TRUE;
}
#endif
INT SetRfTxAnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	INT     status = TRUE;
	CHAR	*value = 0;
	UINT8   ucTxAntIdx = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 2) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format with 2-digit.\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			ucTxAntIdx = simple_strtol(value, 0, 10);
			break;
		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
			break;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucTxAntIdx: 0x%x\n", __func__, ucTxAntIdx));
	return TxPowerRfTxAnt(pAd, ucTxAntIdx);
}

INT SetTxPowerInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = 0;
	UCHAR   ucTxPowerInfoCatg = 0;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR  IfIdx = 0;
	IfIdx = pObj->ioctl_if;

	/* Grab Band index */
	if (pObj->ioctl_if_type == INT_MBSSID || pObj->ioctl_if_type == INT_MAIN) {
#ifdef CONFIG_AP_SUPPORT
		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: AP Band = %d !!\n", __func__, ucBandIdx));

#endif
	}
#ifdef CONFIG_STA_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: STA Band = %d !!\n", __func__, ucBandIdx));
	} else if (pObj->ioctl_if_type == INT_MSTA) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: STA Band = %d !!\n", __func__, ucBandIdx));
		}
#endif
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[SetTxPowerInfo]: pObj->ioctl_if_type = %d!!\n"
, pObj->ioctl_if_type));
		return FALSE;
	}

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Invalid Band Index!!\n", __func__));
		goto error;
	}

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		goto error;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			ucTxPowerInfoCatg = simple_strtol(value, 0, 10);
			break;

		default: {
			break;
		}
		}
	}

	/* Sanity check for parameter range */
	if (ucTxPowerInfoCatg >= POWER_INFO_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: parameter out of range!!\n", __func__));
		goto error;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BandIdx: %d, ucTxPowerInfoCatg: %d\n", __func__, ucBandIdx, ucTxPowerInfoCatg));
	return TxPowerShowInfo(pAd, ucTxPowerInfoCatg, ucBandIdx);

error:

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("iwpriv <interface> set TxPowerInfo=[param1]\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	param1: Tx Power Info Category (0~3)\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			0: Tx Power Basic Info\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			1: Backup Power Table\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			2: Tx Power Rate Power Info\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			3: Thermal Compensation Table\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			4: TXV/BBP Power Value (per packet)\n"));

	return FALSE;
}

INT SetTOAECtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	 *value = 0;
	UCHAR   TOAECtrl = 0;
	INT     status = TRUE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			TOAECtrl = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((TOAECtrl != FALSE) && (TOAECtrl != TRUE)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TOAECtrl = %d\n", __func__, TOAECtrl));
	return TOAECtrlCmd(pAd, TOAECtrl);
}

INT SetEDCCACtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	    i;
	CHAR	    *value = 0;
	UCHAR       ucBandIdx = 0;
	UCHAR       EDCCACtrl = 0;
	INT         status = TRUE;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
	struct wifi_dev *wdev;

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BandIdx = %d\n", __func__, ucBandIdx));
#endif /* CONFIG_AP_SUPPORT */

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: BandIdx = %d. Improper Band Index.\n", __func__, ucBandIdx));
		return FALSE;
	}

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			EDCCACtrl = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((EDCCACtrl != FALSE) && (EDCCACtrl != TRUE)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: EDCCACtrl = %d\n", __func__, EDCCACtrl));
	pAd->CommonCfg.ucEDCCACtrl[ucBandIdx] = EDCCACtrl;
	status = EDCCACtrlCmd(pAd, ucBandIdx, pAd->CommonCfg.ucEDCCACtrl[ucBandIdx]);
	return status;
}

INT ShowEDCCAStatus(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: EDCCACtrl[Band0] = %d\n", __func__,
			 pAd->CommonCfg.ucEDCCACtrl[DBDC_BAND0]));
#ifdef DBDC_MODE
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: EDCCACtrl[Band1] = %d\n", __func__,
			 pAd->CommonCfg.ucEDCCACtrl[DBDC_BAND1]));
#endif /*DBDC_MODE*/
	return status;
}

INT SetSKUInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
#ifdef SINGLE_SKU_V2
	/* print out Sku table info */
	if (NDIS_STATUS_SUCCESS != MtShowPwrLimitTable(pAd, POWER_LIMIT_TABLE_TYPE_SKU, DBG_LVL_OFF))
		status = FAIL;
#endif /* SINGLE_SKU_V2 */
	return status;
}

INT SetBFBackoffInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
#ifdef SINGLE_SKU_V2
	/* print out Backoff table info */
	if (NDIS_STATUS_SUCCESS != MtShowPwrLimitTable(pAd, POWER_LIMIT_TABLE_TYPE_BACKOFF, DBG_LVL_OFF))
		status = FAIL;
#endif /* SINGLE_SKU_V2 */
	return status;
}

INT show_BSSEdca_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}
	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pBssEdca is NULL\n"));
		return FALSE;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("   BSSEdca:     AIFSN    CWmin   CWmax    TXOP(us)  ACM\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_BE	  %2d	  %2d	  %2d	   %4d	   %d\n",
							 pBssEdca->Aifsn[0],
							 pBssEdca->Cwmin[0],
							 pBssEdca->Cwmax[0],
							 pBssEdca->Txop[0],
							 pBssEdca->bACM[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_BK	  %2d	  %2d	  %2d	   %4d	   %d\n",
							 pBssEdca->Aifsn[1],
							 pBssEdca->Cwmin[1],
							 pBssEdca->Cwmax[1],
							 pBssEdca->Txop[1],
							 pBssEdca->bACM[1]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_VI	  %2d	  %2d	  %2d	   %4d	   %d\n",
							 pBssEdca->Aifsn[2],
							 pBssEdca->Cwmin[2],
							 pBssEdca->Cwmax[2],
							 pBssEdca->Txop[2],
							 pBssEdca->bACM[2]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_VO	  %2d	  %2d	  %2d	   %4d	   %d\n",
							 pBssEdca->Aifsn[3],
							 pBssEdca->Cwmin[3],
							 pBssEdca->Cwmax[3],
							 pBssEdca->Txop[3],
							 pBssEdca->bACM[3]));

	return TRUE;
}

INT show_APEdca_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("   APEdca:     AIFSN     CWmin   CWmax    TXOP(us)  ACM\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_BE	%2d	  %2d	  %2d	   %4d	     %d\n",
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[0],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[0],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[0],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[0],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].bACM[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_BK	%2d	  %2d	  %2d	   %4d	     %d\n",
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[1],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[1],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[1],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[1],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].bACM[1]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_VI	%2d	  %2d	  %2d	   %4d	     %d\n",
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[2],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[2],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[2],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[2],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].bACM[2]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_VO	%2d	  %2d	  %2d	   %4d	     %d\n",
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[3],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[3],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[3],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[3],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].bACM[3]));

	return TRUE;
}

#ifdef WIFI_EAP_FEATURE
INT SetEDCCAThresholdCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 edcca_threshold = 0;
	UINT8 BandIdx = 0;
	struct wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return 0;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */

	/* sanity check for Band index */
	if (BandIdx >= DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Invalid Band Index!!\n", __func__));
		return 0;
	}

	if (!arg)
		return 0;

	edcca_threshold = os_str_toul(arg, 0, 10);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: EdccaThreshold:%u Band Index:%u\n",
		__func__, edcca_threshold, BandIdx));
	return SetEdccaThreshold(pAd, edcca_threshold, BandIdx);
}

INT SetInitIPICtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   BandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR   apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return 0;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */

	/* sanity check for Band index */
	if (BandIdx >= DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Invalid Band Index!!\n", __func__));
		return 0;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Band Index:%u\n", __func__, BandIdx));
	return InitIPICtrl(pAd, BandIdx);
}

INT ShowIPIValue(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   BandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR   apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return 0;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */

	/* sanity check for Band index */
	if (BandIdx >= DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Invalid Band Index!!\n", __func__));
		return 0;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Band Index:%u\n", __func__, BandIdx));

	return GetIPIValue(pAd, BandIdx);
}

#define WTBL_TX_PWR_OFFSET_VALUE_MIN -16
#define WTBL_TX_PWR_OFFSET_VALUE_MAX 0

INT set_data_txpwr_offset(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 wlan_id = 0;
	INT8 txpwr_offset = 0;
	PCHAR pch = NULL;
	UINT8 BandIdx = 0;
	struct wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	wdev = &pAd->StaCfg[0].wdev;
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_STA_SUPPORT */

	/* sanity check for Band index */
	if (BandIdx >= DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Invalid Band Index!!\n", __func__));
		return FALSE;
	}

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		wlan_id = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	txpwr_offset = (INT8) os_str_tol(arg, 0, 10);

	if (txpwr_offset < WTBL_TX_PWR_OFFSET_VALUE_MIN
			|| txpwr_offset > WTBL_TX_PWR_OFFSET_VALUE_MAX) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: txpwr_offset range [-16,0]\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: wlanid:%u txpwr_offset:%d BandIdx:%u\n",
			__func__, wlan_id, txpwr_offset, BandIdx));

	return SetDataTxPwrOffset(pAd, wlan_id, txpwr_offset, BandIdx);
}

INT update_switch_tbl_to_fw(RTMP_ADAPTER *pAd,
	UINT8 BandIdx, UINT8 CmdTblIndex, CHAR *buf)
{
	CHAR *readline = NULL, *token = NULL, *ptr = NULL;
	INT ElemIdx = 0, TableSize = 0, table_updated = 0, retval = FALSE;
	UINT8 TblIndex = RA_TBL_INDEX_INVALID;
	UINT8 SwRaTable[512];
	PUINT8 Table = NULL;
	UINT32 rv = 0;

	for (readline = ptr = buf; (ptr = os_str_chr(readline, '\n')) != NULL;
			readline = ptr + 1) {

		if ((TblIndex < RA_TBL_INDEX_INVALID) && ElemIdx
				&& !isdigit(readline[0])) {
			Table = (PUINT8) SwRaTable;
			TableSize = ElemIdx * sizeof(SwRaTable[0]);

			if (Table) {
				retval = SetFwRaTable(pAd, BandIdx, eRateSwitchTable, TblIndex,
						TableSize, (PUCHAR)Table);

				if (retval) {
					table_updated++;
					MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("Successfully updated %s\n", token));
				}
			}

			if ((CmdTblIndex != RA_TBL_INDEX_INVALID) && table_updated)
				return table_updated;

			TblIndex = RA_TBL_INDEX_INVALID;
			TableSize = 0;
			Table = NULL;
			token = NULL;
			ElemIdx = 0;
		}

		/* Table Name Parsing */
		if (!strncmp(readline, "Table:", 6)) {
			token = rstrtok(readline + 6, "\n");

			/* sanity check for non-Null pointer */
			if (!token)
				continue;

			TblIndex = getRaTableIndex(eRateSwitchTable, token);

			if ((CmdTblIndex != RA_TBL_INDEX_INVALID)
					&& (TblIndex != CmdTblIndex))
				TblIndex = RA_TBL_INDEX_INVALID;

			if (TblIndex == RA_TBL_INDEX_INVALID)
				continue;

				os_zero_mem(SwRaTable, sizeof(SwRaTable));
		}

		if ((TblIndex < RA_TBL_INDEX_INVALID) && isdigit(readline[0])) {
			rv = sscanf(readline,
					"%hhu\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu"
					"\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu\n",
					&SwRaTable[ElemIdx], &SwRaTable[ElemIdx + 1],
					&SwRaTable[ElemIdx + 2], &SwRaTable[ElemIdx + 3],
					&SwRaTable[ElemIdx + 4], &SwRaTable[ElemIdx + 5],
					&SwRaTable[ElemIdx + 6], &SwRaTable[ElemIdx + 7],
					&SwRaTable[ElemIdx + 8], &SwRaTable[ElemIdx + 9],
					&SwRaTable[ElemIdx + 10], &SwRaTable[ElemIdx + 11],
					&SwRaTable[ElemIdx + 12], &SwRaTable[ElemIdx + 13],
					&SwRaTable[ElemIdx + 14]);

			if (rv > 0)
				ElemIdx += NUM_OF_COL_RATE_SWITCH_TABLE;

		}
	}

	if ((TblIndex < RA_TBL_INDEX_INVALID) && ElemIdx) {
		Table = (PUINT8) SwRaTable;
		TableSize = ElemIdx * sizeof(SwRaTable[0]);
	}

	if (Table) {
		retval = SetFwRaTable(pAd, BandIdx, eRateSwitchTable, TblIndex,
				TableSize, (PUCHAR)Table);

		if (retval) {
			table_updated++;
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("Successfully updated %s\n", token));
		}
	}

	return table_updated;
}

INT udpate_hwfb_tbl_to_fw(RTMP_ADAPTER *pAd,
	UINT8 BandIdx, UINT8 CmdTblIndex, CHAR *buf)
{
	CHAR *readline = NULL, *token = NULL, *ptr = NULL;
	INT ElemIdx = 0, TableSize = 0, table_updated = 0, retval = FALSE;
	UINT8 TblIndex = RA_TBL_INDEX_INVALID;
	UINT16 HwFbRaTable[256];
	PUINT8 Table = NULL;
	UINT32 rv = 0;

	for (readline = ptr = buf; (ptr = os_str_chr(readline, '\n')) != NULL;
			readline = ptr + 1) {

		if ((TblIndex < RA_TBL_INDEX_INVALID) && ElemIdx
				&& !isdigit(readline[0])) {
			Table = (PUINT8) HwFbRaTable;
			TableSize = ElemIdx * sizeof(HwFbRaTable[0]);

			if (Table) {
				retval = SetFwRaTable(pAd, BandIdx, eRateHwFbTable, TblIndex,
						TableSize, (PUCHAR)Table);

				if (retval) {
					table_updated++;
					MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("Successfully updated %s\n", token));
				}
			}

			if ((CmdTblIndex != RA_TBL_INDEX_INVALID) && table_updated)
				return table_updated;

			TblIndex = RA_TBL_INDEX_INVALID;
			TableSize = 0;
			Table = NULL;
			token = NULL;
			ElemIdx = 0;
		}

		/* Table Name Parsing */
		if (!strncmp(readline, "Table:", 6)) {
			token = rstrtok(readline + 6, "\n");

			/* sanity check for non-Null pointer */
			if (!token)
				continue;

			TblIndex = getRaTableIndex(eRateHwFbTable, token);

			if ((CmdTblIndex != RA_TBL_INDEX_INVALID)
					&& (TblIndex != CmdTblIndex))
				TblIndex = RA_TBL_INDEX_INVALID;

			if (TblIndex == RA_TBL_INDEX_INVALID)
				continue;

			os_zero_mem(HwFbRaTable, sizeof(HwFbRaTable));
		}

		if ((TblIndex < RA_TBL_INDEX_INVALID) && isdigit(readline[0])) {
			rv = sscanf(readline,
					"%hu\t%hu\t%hu\t%hu\t%hu\t%hu\t%hu\t%hu\n",
					&HwFbRaTable[ElemIdx], &HwFbRaTable[ElemIdx + 1],
					&HwFbRaTable[ElemIdx + 2], &HwFbRaTable[ElemIdx + 3],
					&HwFbRaTable[ElemIdx + 4], &HwFbRaTable[ElemIdx + 5],
					&HwFbRaTable[ElemIdx + 6], &HwFbRaTable[ElemIdx + 7]);

			if (rv > 0)
				ElemIdx += NUM_OF_COL_RATE_HWFB_TABLE;
		}
	}

	if ((TblIndex < RA_TBL_INDEX_INVALID) && ElemIdx) {
		Table = (PUINT8) HwFbRaTable;
		TableSize = ElemIdx * sizeof(HwFbRaTable[0]);
	}

	if (Table) {
		retval = SetFwRaTable(pAd, BandIdx, eRateHwFbTable, TblIndex,
				TableSize, (PUCHAR)Table);

		if (retval) {
			table_updated++;
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("Successfully updated %s\n", token));
		}
	}

	return table_updated;
}

PCHAR get_fname_for_fw_ratbl (RTMP_ADAPTER *pAd, UINT8 TblType)
{
	CHAR *fname = NULL;

	if (TblType == eRateSwitchTable) {
		if (IS_MT7615(pAd))
			fname = EAP_FW_RA_SWITCH_TBL_UPD_PATH_7615;
		if (IS_MT7622(pAd))
			fname = EAP_FW_RA_SWITCH_TBL_UPD_PATH_7622;
		if (IS_MT7663(pAd))
			fname = EAP_FW_RA_SWITCH_TBL_UPD_PATH_7663;
		if (IS_MT7626(pAd))
			fname = EAP_FW_RA_SWITCH_TBL_UPD_PATH_7626;
	} else if (TblType == eRateHwFbTable) {
		if (IS_MT7615(pAd))
			fname = EAP_FW_RA_HW_FB_TBL_UPD_PATH_7615;
		if (IS_MT7622(pAd))
			fname = EAP_FW_RA_HW_FB_TBL_UPD_PATH_7622;
		if (IS_MT7663(pAd))
			fname = EAP_FW_RA_HW_FB_TBL_UPD_PATH_7663;
		if (IS_MT7626(pAd))
			fname = EAP_FW_RA_HW_FB_TBL_UPD_PATH_7626;
	}

	return fname;
}

INT read_fw_ratbl_from_file(RTMP_ADAPTER *pAd, UINT8 BandIdx, UINT8 TblType, UINT8 CmdTblIndex)
{
	RTMP_OS_FD_EXT srcf;
	ULONG buf_size = MAX_INI_BUFFER_SIZE;
	CHAR *fname = NULL, *buf = NULL;
	INT table_updated = 0, retval = FALSE;

	fname = get_fname_for_fw_ratbl(pAd, TblType);

	if (!fname)
		return FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("file \"%s\"!\n", fname));

	srcf = os_file_open(fname, O_RDONLY, 0);

	if (srcf.Status) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Open file \"%s\" failed!\n", fname));
		return FALSE;
	}

	os_alloc_mem(pAd, (UCHAR **)&buf, buf_size);

	if (!buf)
		goto close_file;

	os_zero_mem(buf, buf_size);

	retval = os_file_read(srcf, buf, MAX_INI_BUFFER_SIZE - 1);

	if (retval <= 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Read file \"%s\" failed(errCode=%d)!\n", fname, retval));
		goto close_file;
	}

	if (TblType == eRateSwitchTable)
		table_updated = update_switch_tbl_to_fw(pAd, BandIdx, CmdTblIndex, buf);

	if (TblType == eRateHwFbTable)
		table_updated = udpate_hwfb_tbl_to_fw(pAd, BandIdx, CmdTblIndex, buf);

close_file:
	if (table_updated) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Successfully %d RA tables updated!\n", table_updated));
		retval = TRUE;
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("RA table not found!\n"));
		retval = FALSE;
	}

	if (buf)
		os_free_mem(buf);

	if (os_file_close(srcf) != 0) {
		retval = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Close file \"%s\" failed(errCode=%d)!\n", fname, retval));
	}

	return retval;
}

INT set_fw_ratbl_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev;
	CHAR *pch = NULL;
	UINT8 BandIdx = 0, TblType = eRateTableMax;
	UINT8 CmdTblIndex = RA_TBL_INDEX_INVALID;

#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	wdev = &pAd->StaCfg[0].wdev;
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_STA_SUPPORT */

	/* sanity check for Band index */
	if (BandIdx >= DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Invalid Band Index!!\n", __func__));
		return FALSE;
	}

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		TblType = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters for TblType!!\n", __func__));
		return FALSE;
	}

	if (TblType >= eRateTableMax) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: TblType value should be less than %d!\n",
				__func__, eRateTableMax));
		return FALSE;
	}

	pch = arg;
	if (pch != NULL)
		CmdTblIndex = (UINT8) os_str_toul(pch, 0, 10);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s: TblType:%u TblIndex:%u BandIdx:%u!\n", __func__,
			 TblType, CmdTblIndex, BandIdx));

	return read_fw_ratbl_from_file(pAd, BandIdx, TblType, CmdTblIndex);
}

INT show_ratbl_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx = 0, TblType = 0, TblIndex = 0, ReadnWrite = 0;
	struct wifi_dev *wdev;
	PCHAR pch = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	wdev = &pAd->StaCfg[0].wdev;
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_STA_SUPPORT */

	/* sanity check for Band index */
	if (BandIdx >= DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Invalid Band Index!!\n", __func__));
		return FALSE;
	}

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		TblType = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters for TblType!!\n", __func__));
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		TblIndex = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters for TblIndex!!\n", __func__));
		return FALSE;
	}

	pch = arg;

	if (pch != NULL)
		ReadnWrite = (UINT8) os_str_toul(pch, 0, 10);

	if (ReadnWrite)
		ReadnWrite = 1;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: Band Index:%u TblType:%u TblIndex:%u RW:%u\n",
			 __func__, BandIdx, TblType, TblIndex, ReadnWrite));

	return GetRaTblInfo(pAd, BandIdx, TblType, TblIndex, ReadnWrite);
}
#endif /* WIFI_EAP_FEATURE */

#ifdef TXD_MGMT_TXPWR_CTRL
#define TX_PWR_OFFSET_VALUE_MIN -16
#define TX_PWR_OFFSET_VALUE_MAX 15

INT set_mgmt_txpwr_offset(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8 txpwr_offset = 0;
	UINT8 txd_txpwr_offset = 0;
	UINT8 txpwr_force_on = 0;
	PCHAR pch = NULL;
	struct wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return 0;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#endif /* CONFIG_AP_SUPPORT */

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		txpwr_force_on = os_str_tol(pch, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: txpwr_force_on:%u\n", __func__, txpwr_force_on));

	if (txpwr_force_on == FALSE) {
		wdev->txd_txpwr_offset = 0;
#ifdef CONFIG_AP_SUPPORT
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_DISABLE_TX);
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_ENABLE_TX);
#endif /* CONFIG_AP_SUPPORT */
		return TRUE;
	}

	txpwr_offset = os_str_tol(arg, 0, 10);

	if (txpwr_offset < TX_PWR_OFFSET_VALUE_MIN
			|| txpwr_offset > TX_PWR_OFFSET_VALUE_MAX) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: txpwr_offset range [-16,+15]\n", __func__));
		return FALSE;
	}

	txd_txpwr_offset = (UINT8)txpwr_offset;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: txd_txpwr_offset: %u\n", __func__, txd_txpwr_offset));
	wdev->txd_txpwr_offset = txd_txpwr_offset;

#ifdef CONFIG_AP_SUPPORT
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_DISABLE_TX);
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_ENABLE_TX);
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}
#endif /* TXD_MGMT_TXPWR_CTRL */

#ifdef WIFI_GPIO_CTRL
INT set_gpio_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *pch = NULL;
	UINT8 gpio_idx;
	BOOLEAN gpio_en = FALSE;

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		gpio_idx = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters for gpio_idx!!\n", __func__));
		return FALSE;
	}

	if (gpio_idx < GPIO_INDEX_MIN_VAL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: gpio_idx value should not be less than %d!\n",
				__func__, GPIO_INDEX_MIN_VAL));
		return FALSE;
	}

	if (gpio_idx > GPIO_INDEX_MAX_VAL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: gpio_idx value should not be greater than %d!\n",
				__func__, GPIO_INDEX_MAX_VAL));
		return FALSE;
	}

	pch = arg;
	if (pch != NULL)
		gpio_en = (BOOLEAN) os_str_toul(pch, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters for gpio_en!!\n", __func__));
		return FALSE;
	}

	if (gpio_en)
		gpio_en = TRUE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: gpio_num:%u Enable:%u\n",
			__func__, gpio_idx, gpio_en));

	return SetGpioCtrl(pAd, gpio_idx, gpio_en);
}

INT set_gpio_value(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *pch = NULL;
	UINT8 gpio_idx, gpio_val;

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		gpio_idx = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters for gpio_idx!!\n", __func__));
		return FALSE;
	}

	if (gpio_idx < GPIO_INDEX_MIN_VAL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: gpio_idx value should not be less than %d!\n",
				__func__, GPIO_INDEX_MIN_VAL));
		return FALSE;
	}

	if (gpio_idx > GPIO_INDEX_MAX_VAL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: gpio_idx value should not be greater than %d!\n",
				__func__, GPIO_INDEX_MAX_VAL));
		return FALSE;
	}

	pch = arg;
	if (pch != NULL)
		gpio_val = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters for gpio_val!!\n", __func__));
		return FALSE;
	}

	if (gpio_val)
		gpio_val = 1;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: gpio_num:%u Value:%u\n",
			__func__, gpio_idx, gpio_val));

	return SetGpioValue(pAd, gpio_idx, gpio_val);
}
#endif /* WIFI_GPIO_CTRL */

#ifdef LINK_TEST_SUPPORT
INT SetLinkTestRxParamCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 25) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			/* 2 symbol representation */
			pAd->ucRssiSigniTh = simple_strtol(value, 0, 10);
			break;
		case 1:
			/* 3 symbol representation */
			pAd->cWBRssiTh = simple_strtol(value, 0, 10);
			break;
		case 2:
			/* 3 symbol representation */
			pAd->cIBRssiTh = simple_strtol(value, 0, 10);
			break;
		case 3:
			/* 2 symbol representation */
			pAd->c8RxCountTh = simple_strtol(value, 0, 10);
			break;
		case 4:
			/* 3 symbol representation */
			pAd->ucTimeOutTh = simple_strtol(value, 0, 10);
			break;
		case 5:
			/* 3 symbol representation */
			pAd->cNrRssiTh = simple_strtol(value, 0, 10);
			break;
		case 6:
			/* 3 symbol representation */
			pAd->cChgTestPathTh = simple_strtol(value, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): ucRssiSigniTh: %d, cWBRssiTh: %d, cIBRssiTh: %d\n", __func__,
																pAd->ucRssiSigniTh, pAd->cWBRssiTh, pAd->cIBRssiTh));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): c8RxCountTh: %lld, ucTimeOutTh: %d\n", __func__,
																pAd->c8RxCountTh, pAd->ucTimeOutTh));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): cNrRssiTh: %d, cChgTestPathTh: %d\n", __func__,
																pAd->cNrRssiTh, pAd->cChgTestPathTh));

	return TRUE;
}

INT SetLinkTestModeCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			pAd->fgTxSpurEn = simple_strtol(value, 0, 10);
			break;
		case 1:
			pAd->fgRxSensitEn = simple_strtol(value, 0, 10);
			break;
		case 2:
			pAd->fgACREn = simple_strtol(value, 0, 10);
			break;
		case 3:
			pAd->fgTxSpeEn = simple_strtol(value, 0, 10);
			break;
		case 4:
			pAd->fgRxRcpiEn = simple_strtol(value, 0, 10);
			break;
		case 5:
			pAd->cMaxInRssiTh = simple_strtol(value, 0, 10);
			break;
		case 6:
			pAd->ucACRConfidenceCntTh = simple_strtol(value, 0, 10);
			break;
		case 7:
			pAd->ucMaxInConfidenceCntTh = simple_strtol(value, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgTxSpurEn: %d, fgRxSensitEn: %d, fgACREn: %d, fgTxSpeEn: %d\n", __func__,
															pAd->fgTxSpurEn, pAd->fgRxSensitEn, pAd->fgACREn, pAd->fgTxSpeEn));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgRxRcpiEn: %d, cMaxInRssiTh: %d, ucACRConfidenceCntTh: %d, ucMaxInConfidenceCntTh: %d\n", __func__,
															pAd->fgRxRcpiEn, pAd->cMaxInRssiTh, pAd->ucACRConfidenceCntTh, pAd->ucMaxInConfidenceCntTh));

	return TRUE;
}

INT SetLinkTestPowerUpTblCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value;
	UINT8	ucTxPwrUpCat = 0;
	UINT8	ucTxPwrUpRate = 0;
	UINT8	ucTxPwrUpValue = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			ucTxPwrUpCat = simple_strtol(value, 0, 10);
			break;
		case 1:
			ucTxPwrUpRate = simple_strtol(value, 0, 10);
			break;
		case 2:
			ucTxPwrUpValue = simple_strtol(value, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucTxPwrUpCat: %d, ucTxPwrUpRate: %d, ucTxPwrUpValue: %d\n", __func__,
															ucTxPwrUpCat, ucTxPwrUpRate, ucTxPwrUpValue));

	/* update Tx Power Up Table */
	pAd->ucTxPwrUpTbl[ucTxPwrUpCat][ucTxPwrUpRate] = ucTxPwrUpValue;

	/* sync Tx Power up table to Firmware */
	MtCmdLinkTestTxPwrUpTblCtrl(pAd, ucTxPwrUpCat, pAd->ucTxPwrUpTbl[ucTxPwrUpCat]);

	return TRUE;
}

INT SetLinkTestPowerUpTblInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  ucRateIdx, ucCatIdx;
	CHAR   cRateInfo[CMW_POWER_UP_RATE_NUM][12] = {"CCK_1M2M", "CCK_5M11M", "OFDM_6M9M", "OFDM_12M18M", "OFDM_24M36M", "OFDM_48M", "OFDM_54M",
												   "HT20_MCS0", "HT20_MCS12", "HT20_MCS34", "HT20_MCS5", "HT20_MCS6", "HT20_MCS7"};

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("						Link Test Power Up Table\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("				2G(S)		5G(S)		2G(D)		5G(D)\n"));

	for (ucRateIdx = 0; ucRateIdx < CMW_POWER_UP_RATE_NUM; ucRateIdx++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:  ", cRateInfo[ucRateIdx]));
		for (ucCatIdx = 0; ucCatIdx < CMW_POWER_UP_CATEGORY_NUM; ucCatIdx++)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		%d", pAd->ucTxPwrUpTbl[ucCatIdx][ucRateIdx]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}

	return TRUE;
}

INT SetLinkTestInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 u4value;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("								 Link Status\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Link Up Done: %d\n", pAd->fgCmwLinkDone));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ChannelBand: %dG(Band0) %dG(Band1)\n", pAd->ucCmwChannelBand[BAND0] ? 2 : 5, pAd->ucCmwChannelBand[BAND1] ? 2 : 5));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CSD State: %d(Band0) %d (Band1)\n", pAd->ucTxCsdState[BAND0], pAd->ucTxCsdState[BAND1]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Power Up State: %d(Band0) %d(Band1)\n", pAd->ucTxPwrBoostState[BAND0], pAd->ucTxPwrBoostState[BAND1]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxStream Specific Rx Path Index: %d(Band0) %d(Band1)\n", pAd->ucRxStreamState[BAND0], pAd->ucRxStreamState[BAND1]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rx Filter State: %d(Band0) %d(Band1)\n", pAd->ucRxFilterstate[BAND0], pAd->ucRxFilterstate[BAND1]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ACR Confidence count and Threshoild: %d(Band0) %d(Band1) %d(Threshold)\n", pAd->ucRxFilterConfidenceCnt[BAND0], pAd->ucRxFilterConfidenceCnt[BAND1], pAd->ucACRConfidenceCntTh));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MaxIn Confidence count and Threshoild: %d(Band0) %d(Band1) %d(Threshold)\n", pAd->ucRxFilterConfidenceCnt[BAND0], pAd->ucRxFilterConfidenceCnt[BAND1], pAd->ucMaxInConfidenceCntTh));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("								Tx Antenna Status\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n\n"));

	/* Read RF CR */
	MtCmdRFRegAccessRead(pAd, (UINT32)WF0, (UINT32)0x48, (UINT32 *)&u4value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WF%d 0x%04x 0x%08x\n", WF0, 0x48, u4value));

	MtCmdRFRegAccessRead(pAd, (UINT32)WF1, (UINT32)0x48, (UINT32 *)&u4value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WF%d 0x%04x 0x%08x\n", WF1, 0x48, u4value));

	MtCmdRFRegAccessRead(pAd, (UINT32)WF2, (UINT32)0x48, (UINT32 *)&u4value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WF%d 0x%04x 0x%08x\n", WF2, 0x48, u4value));

	MtCmdRFRegAccessRead(pAd, (UINT32)WF3, (UINT32)0x48, (UINT32 *)&u4value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WF%d 0x%04x 0x%08x\n", WF3, 0x48, u4value));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));

	return TRUE;
}
#endif /* LINK_TEST_SUPPORT */

#ifdef ETSI_RX_BLOCKER_SUPPORT
/* Set fix WBRSSI/IBRSSI pattern */
INT SetFixWbIbRssiCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL,   DBG_SUBCAT_ALL,   DBG_LVL_ERROR,	("%s: No parameters!!\n",  __func__));
		return  FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL,   DBG_SUBCAT_ALL,   DBG_LVL_ERROR,	("%s: Wrong parameter format!!\n",  __func__));
		return  FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
			/* 1 symbol representation */
		case 0:
				pAd->fgFixWbIBRssiEn = simple_strtol(value, 0, 10);
				break;
			default: {
				MTWF_LOG(DBG_CAT_ALL,   DBG_SUBCAT_ALL,   DBG_LVL_ERROR,    ("%s: Number of parameters exceed expectation !!\n",  __func__));
				return  FALSE;
			}
		}
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Enable CR for RX_BLOCKER: fgFixWbIBRssiEn: %d\n", __func__,
														pAd->fgFixWbIBRssiEn));
	return TRUE;
}
/* Set RSSI threshold */
INT SetRssiThCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
	return FALSE;
	}

	if (strlen(arg) != 15) {
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
	return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
			/* 3 symbol representation */
		case 0:
			pAd->c1RWbRssiHTh = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 1:
			pAd->c1RWbRssiLTh = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 2:
			pAd->c1RIbRssiLTh = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 3:
			pAd->c1WBRssiTh4R = simple_strtol(value, 0, 10);
			break;

		default:
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
			}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: c1RWbRssiHTh: %d, c1RWbRssiLTh: %d, c1RIbRssiLTh: %d,c1WBRssiTh4R: %d\n", __FUNCTION__,
														pAd->c1RWbRssiHTh, pAd->c1RWbRssiLTh, pAd->c1RIbRssiLTh, pAd->c1WBRssiTh4R));

	return TRUE;
}

/* Set fgAdaptRxBlock */
INT SetAdaptRxBlockCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		/* 3 symbol representation */
		case 0:
			pAd->fgAdaptRxBlock = simple_strtol(value, 0, 10);
		break;
		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
			}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgAdaptRxBlock: %d\n", __func__,
														pAd->fgAdaptRxBlock));

	return TRUE;
}

/* Set check counter threshold */
INT SetCheckThCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 11) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		/* 2 symbol representation */
		case 0:
			pAd->u1CheckTime = simple_strtol(value, 0, 10);
			break;
		/* 2 symbol representation */
		case 1:
			pAd->u1To1RCheckCnt = simple_strtol(value, 0, 10);
			break;
		/* 3 symbol representation */
		case 2:
			pAd->u2To1RvaildCntTH = simple_strtol(value, 0, 10);
			break;
		case 3:
			pAd->u2To4RvaildCntTH = simple_strtol(value, 0, 10);
			break;
		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: u1CheckTime: %d, u1To1RCheckCnt: %d, u2To1RvaildCntTH: %d, u2To4RvaildCntTH: %d\n", __func__,
														pAd->u1CheckTime, pAd->u1To1RCheckCnt, pAd->u2To1RvaildCntTH, pAd->u2To4RvaildCntTH));
	return TRUE;

}
/* Set WBRSSI CR */
INT SetWbRssiDirectCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 15) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		/* 3 symbol representation */
		case 0:
			pAd->c1WbRssiWF0 = simple_strtol(value, 0, 10);
			break;
		/* 3 symbol representation */
		case 1:
			pAd->c1WbRssiWF1 = simple_strtol(value, 0, 10);
			break;
		/* 3 symbol representation */
		case 2:
			pAd->c1WbRssiWF2 = simple_strtol(value, 0, 10);
			break;
		/* 3 symbol representation */
		case 3:
			pAd->c1WbRssiWF3 = simple_strtol(value, 0, 10);
			break;
		default:
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
		}
		}
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: c1WbRssiWF0: %d, c1WbRssiWF1: %d, c1WbRssiWF2: %d, c1WbRssiWF3: %d\n", __func__, pAd->c1WbRssiWF0, pAd->c1WbRssiWF1, pAd->c1WbRssiWF2, pAd->c1WbRssiWF3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	return TRUE;
}

/* Set IBRSSI CR */
INT SetIbRssiDirectCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 15) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		/* 3 symbol representation */
		case 0:
			pAd->c1IbRssiWF0 = simple_strtol(value, 0, 10);
			break;
		/* 3 symbol representation */
		case 1:
			pAd->c1IbRssiWF1 = simple_strtol(value, 0, 10);
			break;
		case 2:
			pAd->c1IbRssiWF2 = simple_strtol(value, 0, 10);
			break;
		/* 3 symbol representation */
		case 3:
			pAd->c1IbRssiWF3 = simple_strtol(value, 0, 10);
			break;
		default:
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
		}
		}
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: c1IbRssiWF0: %d, c1IbRssiWF1: %d, c1WbRssiWF2: %d, c1IbRssiWF3: %d\n", __func__,
														pAd->c1IbRssiWF0, pAd->c1IbRssiWF1, pAd->c1IbRssiWF2, pAd->c1IbRssiWF3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	return TRUE;
}
#endif /* end ETSI_RX_BLOCKER_SUPPORT */

INT SetMuTxPower(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT      status = TRUE;
	UINT8    i;
	CHAR     *value = 0;
	BOOLEAN  fgMuTxPwrManEn = FALSE;
	CHAR     cMuTxPwr = 0;
	UINT8    u1BandIdx = 0;
	struct  wifi_dev *wdev;
#if defined(CONFIG_AP_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
#endif /* CONFIG_AP_SUPPORT */
#if defined(CONFIG_AP_SUPPORT)
	UCHAR	  apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	if ((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) {
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		u1BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		wdev = &pAd->StaCfg[0].wdev;
		u1BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucBandIdx = %d\n", __func__, u1BandIdx));

	/* sanity check for Band index */
	if (u1BandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgMuTxPwrManEn = os_str_tol(value, 0, 10);
			break;

		case 1:
			cMuTxPwr = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgMuTxPwrManEn: %d, cMuTxPwr: %d\n", __func__,
		fgMuTxPwrManEn, cMuTxPwr));

	return MuPwrCtrlCmd(pAd, fgMuTxPwrManEn, cMuTxPwr, u1BandIdx);
}

INT SetBFNDPATxDCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT      status = TRUE;
	UINT8    i;
	CHAR     *value = 0;
	BOOLEAN  fgNDPA_ManualMode = FALSE;
	UINT8    ucNDPA_TxMode = 0;
	UINT8    ucNDPA_Rate = 0;
	UINT8    ucNDPA_BW = 0;
	UINT8    ucNDPA_PowerOffset = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgNDPA_ManualMode = os_str_tol(value, 0, 10);
			break;

		case 1:
			ucNDPA_TxMode = os_str_tol(value, 0, 10);
			break;

		case 2:
			ucNDPA_Rate = os_str_tol(value, 0, 10);
			break;

		case 3:
			ucNDPA_BW = os_str_tol(value, 0, 10);
			break;

		case 4:
			ucNDPA_PowerOffset = os_str_tol(value, 0, 10); /* negative value need to use 2's complement */
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: fgNDPA_ManualMode = %d, ucNDPA_TxMode = %d, ucNDPA_Rate = %d, ucNDPA_BW = %d, ucNDPA_PowerOffset = %d\n",
			  __func__, fgNDPA_ManualMode, ucNDPA_TxMode, ucNDPA_Rate, ucNDPA_BW, ucNDPA_PowerOffset));
	return BFNDPATxDCtrlCmd(pAd, fgNDPA_ManualMode, ucNDPA_TxMode, ucNDPA_Rate, ucNDPA_BW, ucNDPA_PowerOffset);
}

INT SetTxPowerCompInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT     status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */
	UINT8  ucPowerTableIdx;
	CHAR   STRING[SKU_TABLE_SIZE][15] = {
		"CCK_1M2M   ", "CCK5M11M   ", "OFDM6M9M   ", "OFDM12M18M ",	"OFDM24M36M ", "OFDM48M    ", "OFDM54M    ",
		"HT20M0     ", "HT20M32    ", "HT20M1M2   ", "HT20M3M4   ", "HT20M5     ", "HT20M6     ", "HT20M7     ",
		"HT40M0     ", "HT40M32    ", "HT40M1M2   ", "HT40M3M4   ",	"HT40M5     ", "HT40M6     ", "HT40M7     ",
		"VHT20M0    ", "VHT20M1M2  ", "VHT20M1M2  ", "VHT20M5M6  ", "VHT20M7    ", "VHT20M8    ", "VHT20M9    ",
		"VHT40M0    ", "VHT40M1M2  ", "VHT40M3M4  ", "VHT40M5M6  ", "VHT40M7    ", "VHT40M8    ", "VHT40M9    ",
		"VHT80M0    ", "VHT80M1M2  ", "VHT80M3M4  ", "VHT80M5M6  ", "VHT80M7    ", "VHT80M8    ", "VHT80M9    ",
		"VHT160M0   ", "VHT160M1M2 ", "VHT160M3M4 ", "VHT160M5M6 ", "VHT160M7   ", "VHT160M8   ", "VHT160M9   "
	};

#ifdef CONFIG_AP_SUPPORT
	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	wdev = &pAd->StaCfg[0].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucBandIdx = %d\n", __func__, ucBandIdx));

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("=============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("                       Tx Power Compenstation Info                 \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("=============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (KGRN "         Band%d        (1SS, 2SS, 3SS, 4SS)                    \n" KNRM, ucBandIdx));

	for (ucPowerTableIdx = 0; ucPowerTableIdx < SKU_TABLE_SIZE; ucPowerTableIdx++) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("         %s : %3d, %3d, %3d, %3d                               \n",
				  &STRING[ucPowerTableIdx][0],
				  pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucPowerTableIdx][0],
				  pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucPowerTableIdx][1],
				  pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucPowerTableIdx][2],
				  pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucPowerTableIdx][3]));
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("-----------------------------------------------------------------------------\n"));

	return status;
}

#ifdef TXPWRMANUAL
/* Manually setting Tx power on a,g,n*/
INT SetTxPwrManualCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8		i;
	CHAR		*value = 0;
	BOOLEAN		fgPwrManCtrl		= FALSE;
	UINT_8		u1TxPwrModeManual   = 0;
	UINT_8		u1TxPwrBwManual	 = 0;
	UINT_8		u1TxPwrRateManual   = 0;
	INT_8		i1TxPwrValueManual  = 0;
	UCHAR		ucBandIdx = 0;
	struct  wifi_dev *wdev;
	BOOLEAN		fgCheckPar		  = TRUE;

#ifdef CONFIG_AP_SUPPORT
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR	   apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

/* obtain Band index */
#ifdef CONFIG_AP_SUPPORT
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_STA_SUPPORT */

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 14) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!! \n", __FUNCTION__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgPwrManCtrl = simple_strtol(value, 0, 10);
			break;
		case 1:
			u1TxPwrModeManual = simple_strtol(value, 0, 10);/* 0~3 */
			break;
		case 2:
			u1TxPwrBwManual = simple_strtol(value, 0, 10);/* 0~3/0~7/0~7 */
			break;
		case 3:
			u1TxPwrRateManual = simple_strtol(value, 0, 10);/* 0~3/0~7/0~7 */
			break;
		case 4:
			i1TxPwrValueManual = simple_strtol(value, 0, 10);/* 3 symbols */
			break;
		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	/* sanity check for Other input parameter */
	switch (u1TxPwrModeManual) {
	case 0:
		if (u1TxPwrRateManual > 3 || u1TxPwrBwManual > 0)
			fgCheckPar = FALSE;
		break;
	case 1:
		if (u1TxPwrRateManual > 7 || u1TxPwrBwManual > 0)
			fgCheckPar = FALSE;
		break;
	case 2:
		if (u1TxPwrBwManual == 20) {
			if (u1TxPwrRateManual > 7)
				fgCheckPar = FALSE;
		} else if (u1TxPwrBwManual == 40) {
			if (u1TxPwrRateManual > 8)
				fgCheckPar = FALSE;
			} else
				fgCheckPar = FALSE;
		break;
	case 3:
		if (u1TxPwrBwManual == 20) {
			if (u1TxPwrRateManual > 9)
				fgCheckPar = FALSE;
		} else if (u1TxPwrBwManual == 40) {
			if (u1TxPwrRateManual > 0)
				fgCheckPar = FALSE;
		} else
			fgCheckPar = FALSE;
		break;
	default:
		fgCheckPar = TRUE;
	}

	/* sanity check for input parameter */
	if (!fgCheckPar) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("-----------------------------------------------------------------------------\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Set wrong parameters\n", __func__));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" CCK   (0) only supports rate setting: 0 ~ 3 and no supports different BW\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" OFDM  (1) only supports rate setting: 0 ~ 7 and no supports different BW\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" HT20  (2) only supports rate setting: MCS 0 ~ 7 \n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" HT40  (2) only supports rate setting: MCS 0 ~ 8 \n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" VHT20 (3) only supports rate setting: 0 ~ 9\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" VHT40 (3) only supports rate setting: 0\n"));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("-----------------------------------------------------------------------------\n"));
		return FALSE;
	} else {
		if (fgPwrManCtrl) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==================== Turn-on the manually control power ====================\n"));
			/* non-VHT mode */
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("------------------------------ CCH/OFDM/HT MODE -----------------------------\n"));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  u1TxPwrModeManual = %2d \n", u1TxPwrModeManual));
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  PHY MODE - 0: CCK ; 1: OFDM; 2: HT; 3: VHT\n"));
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("-----------------------------------------------------------------------------\n"));
				switch (u1TxPwrModeManual) {
				case 0:
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  For CCK: u1TxPwrRateManual = %2d \n", u1TxPwrRateManual));
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  MODULATION - 0: 1 Mbps ; 1: 2 Mbps; 2: 5 Mbps; 3: 11 Mbps\n"));
				break;
				case 1:
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  For OFDM: u1TxPwrRateManual = %2d \n", u1TxPwrRateManual));
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  MODULATION - 0: 6 Mbps ; 1: 9 Mbps; 2: 12 Mbps; 3: 18 Mbps; 4: 24 Mbps; 5: 36 Mbps; 6: 48 Mbps; 7: 54 Mbps\n"));
				break;
				case 2:
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  For HT: u1TxPwrRateManual = %2d \n", u1TxPwrRateManual));
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  MODULATION - HT20: MCS 0 ~ 7; HT40: MCS 0 ~ 8\n"));
				break;
				case 3:
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("  %s: u1VHTTxPwrRateManual = %2d \n", __func__, u1TxPwrRateManual));
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("  MCS - 0 ~ 9 for VHT20 ; MCS - 0 for VHT40\n"));
				break;
				}
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("-----------------------------------------------------------------------------\n"));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  %s: POWER VALUE or POWER VALUE Offset(VHT40) = %2d \n", __func__, i1TxPwrValueManual));
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("-----------------------------------------------------------------------------\n"));
				return TxPowerManualCtrl (pAd, fgPwrManCtrl, u1TxPwrModeManual, u1TxPwrBwManual, u1TxPwrRateManual, i1TxPwrValueManual, ucBandIdx);
			}
		else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==================== Turn-off the manually control power ====================\n"));
			return TxPowerManualCtrl (pAd, fgPwrManCtrl, u1TxPwrModeManual, u1TxPwrBwManual, u1TxPwrRateManual, i1TxPwrValueManual, ucBandIdx);
		}
	}
}
#endif /* TXPWRMANUAL */

INT SetThermalManualCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = 0;
	BOOLEAN fgManualMode = FALSE;
	CHAR	cTemperature = 0;
	INT     status = TRUE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgManualMode = os_str_tol(value, 0, 10);
			break;

		case 1:
			cTemperature = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgManualMode: %d, cTemperature: %d\n", __func__, fgManualMode,
			 cTemperature));
	return TemperatureCtrl(pAd, fgManualMode, cTemperature);
}

INT SetThermalItemInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return ThermalItemInfo(pAd);
}

#ifdef TX_POWER_CONTROL_SUPPORT
#if defined(MT7626) || defined(AXE) || defined(MT7915)
INT SetTxPowerBoostCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = 0;
	INT	status = TRUE;
	CHAR	cPwrUpCat = 0;
	CHAR	cPwrUpValue[POWER_UP_CATEGORY_RATE_NUM] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	UINT8	ucBandIdx = 0;
	struct	wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
		if (wdev == NULL)	{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: fail: wdev == NULL !!\n", __func__));
			return FALSE;
		}
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value;
		value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			cPwrUpCat = simple_strtol(value, 0, 10);
			break;

		case 1:
			cPwrUpValue[0] = simple_strtol(value, 0, 10);
			break;

		case 2:
			cPwrUpValue[1] = simple_strtol(value, 0, 10);
			break;

		case 3:
			cPwrUpValue[2] = simple_strtol(value, 0, 10);
			break;

		case 4:
			cPwrUpValue[3] = simple_strtol(value, 0, 10);
			break;

		case 5:
			cPwrUpValue[4] = simple_strtol(value, 0, 10);
			break;

		case 6:
			cPwrUpValue[5] = simple_strtol(value, 0, 10);
			break;

		case 7:
			cPwrUpValue[6] = simple_strtol(value, 0, 10);
			break;

		case 8:
			cPwrUpValue[7] = simple_strtol(value, 0, 10);
			break;

		case 9:
			cPwrUpValue[8] = simple_strtol(value, 0, 10);
			break;

		case 10:
			cPwrUpValue[9] = simple_strtol(value, 0, 10);
			break;

		case 11:
			cPwrUpValue[10] = simple_strtol(value, 0, 10);
			break;

		case 12:
			cPwrUpValue[11] = simple_strtol(value, 0, 10);
			break;

		default:
			{
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL,
					DBG_LVL_ERROR,
					("%s: set wrong parameters\n",
					__func__));
				break;
			}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: ucBandIdx: %d, cPwrUpCat: %d\n", __func__,
			 ucBandIdx, cPwrUpCat));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: cPwrUpValue: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
			 __func__, cPwrUpValue[0], cPwrUpValue[1],
			 cPwrUpValue[2], cPwrUpValue[3], cPwrUpValue[4],
			 cPwrUpValue[5], cPwrUpValue[6], cPwrUpValue[7], cPwrUpValue[8],
			 cPwrUpValue[9], cPwrUpValue[10], cPwrUpValue[11]));


	/* update power up table structure */
	switch (cPwrUpCat) {
	case POWER_UP_CATE_CCK:
		os_move_mem(pAd->CommonCfg.cPowerUpCck[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_OFDM:
		os_move_mem(pAd->CommonCfg.cPowerUpOfdm[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_HT20:
		os_move_mem(pAd->CommonCfg.cPowerUpHt20[ucBandIdx], cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_HT40:
		os_move_mem(pAd->CommonCfg.cPowerUpHt40[ucBandIdx], cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_VHT20:
		os_move_mem(pAd->CommonCfg.cPowerUpVht20[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_VHT40:
		os_move_mem(pAd->CommonCfg.cPowerUpVht40[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_VHT80:
		os_move_mem(pAd->CommonCfg.cPowerUpVht80[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_VHT160:
		os_move_mem(pAd->CommonCfg.cPowerUpVht160[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_HE26:
		os_move_mem(pAd->CommonCfg.cPowerUpHe26[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_HE52:
		os_move_mem(pAd->CommonCfg.cPowerUpHe52[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_HE106:
		os_move_mem(pAd->CommonCfg.cPowerUpHe106[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_HE242:
		os_move_mem(pAd->CommonCfg.cPowerUpHe242[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_HE484:
		os_move_mem(pAd->CommonCfg.cPowerUpHe484[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_HE996:
		os_move_mem(pAd->CommonCfg.cPowerUpHe996[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_HE996X2:
		os_move_mem(pAd->CommonCfg.cPowerUpHe996X2[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	default:
		break;
	}

	return TxPwrUpCtrl(pAd, ucBandIdx, cPwrUpCat, cPwrUpValue);

}
#else
INT SetTxPowerBoostCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = 0;
	INT	status = TRUE;
	CHAR	cPwrUpCat = 0;
	CHAR	cPwrUpValue[7] = {0, 0, 0, 0, 0, 0, 0};
	UINT8	ucBandIdx = 0;
	struct	wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value;
			value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			cPwrUpCat = simple_strtol(value, 0, 10);
			break;

		case 1:
			cPwrUpValue[0] = simple_strtol(value, 0, 10);
			break;

		case 2:
			cPwrUpValue[1] = simple_strtol(value, 0, 10);
			break;

		case 3:
			cPwrUpValue[2] = simple_strtol(value, 0, 10);
			break;

		case 4:
			cPwrUpValue[3] = simple_strtol(value, 0, 10);
			break;

		case 5:
			cPwrUpValue[4] = simple_strtol(value, 0, 10);
			break;

		case 6:
			cPwrUpValue[5] = simple_strtol(value, 0, 10);
			break;

		case 7:
			cPwrUpValue[6] = simple_strtol(value, 0, 10);
			break;

		default:
			{
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL,
						DBG_LVL_ERROR,
						("%s: set wrong parameters\n",
						 __func__));
				break;
			}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: ucBandIdx: %d, cPwrUpCat: %d\n", __func__,
			 ucBandIdx, cPwrUpCat));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: cPwrUpValue: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
			 __func__, cPwrUpValue[0], cPwrUpValue[1],
			 cPwrUpValue[2], cPwrUpValue[3], cPwrUpValue[4],
			 cPwrUpValue[5], cPwrUpValue[6]));

	/* update power up table structure */
	switch (cPwrUpCat) {
	case POWER_UP_CATE_CCK_OFDM:
		os_move_mem(pAd->CommonCfg.cPowerUpCckOfdm[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_HT20:
		os_move_mem(pAd->CommonCfg.cPowerUpHt20[ucBandIdx], cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_HT40:
		os_move_mem(pAd->CommonCfg.cPowerUpHt40[ucBandIdx], cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_VHT20:
		os_move_mem(pAd->CommonCfg.cPowerUpVht20[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_VHT40:
		os_move_mem(pAd->CommonCfg.cPowerUpVht40[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_VHT80:
		os_move_mem(pAd->CommonCfg.cPowerUpVht80[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	case POWER_UP_CATE_VHT160:
		os_move_mem(pAd->CommonCfg.cPowerUpVht160[ucBandIdx],
				cPwrUpValue,
				sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);
		break;

	default:
		break;
	}

	return TxPwrUpCtrl(pAd, ucBandIdx, cPwrUpCat, cPwrUpValue);
}
#endif
#endif /* TX_POWER_CONTROL_SUPPORT */

#ifdef RF_LOCKDOWN
INT SetCalFreeApply(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT     status = TRUE;
	UINT8   i;
	CHAR    *value = 0;
	UCHAR   CalFreeApply = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			CalFreeApply = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: CalFreeApply = %d\n", __func__, CalFreeApply));
	/* Configure to Global pAd structure */
	pAd->fgCalFreeApply = CalFreeApply;
	return status;
}

INT SetWriteEffuseRFpara(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UCHAR   block[EFUSE_BLOCK_SIZE] = "";
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	USHORT  length = cap->EEPROM_DEFAULT_BIN_SIZE;
	UCHAR   *ptr = pAd->EEPROMImage;
	UCHAR   index, i;
	USHORT  offset = 0;
	UINT    isVaild = 0;
	BOOL    NeedWrite;
	BOOL    WriteStatus;

	/* Only Write to Effuse when RF is not lock down */
	if (!chip_check_rf_lock_down(pAd)) {
		/* Write to Effuse block by block */
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		for (offset = 0; offset < length; offset += EFUSE_BLOCK_SIZE) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("offset 0x%04x:\n", offset));
			NeedWrite = FALSE;
			MtCmdEfuseAccessRead(pAd, offset, &block[0], &isVaild);

			/* Check the Needed contents are different and update the buffer content for write back to Effuse */
			for (index = 0; index < EFUSE_BLOCK_SIZE; index++) {
				/* Obtain the status of this effuse column need to write or not */
				WriteStatus = ops->write_RF_lock_parameter(pAd, offset + index);
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Effuse[0x%04x]: Write(%d)\n", offset + index, WriteStatus));

				if ((block[index] != ptr[index]) && (WriteStatus == TRUE))
					NeedWrite = TRUE;
				else
					continue;

				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("index 0x%04x: ", offset + index));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("orignal block value=0x%04x, write value=0x%04x\n", block[index],
						 ptr[index]));

				if (WriteStatus == TRUE)
					block[index] = ptr[index];
			}

			/* RF Lock Protection */
			if (offset == RF_LOCKDOWN_EEPROME_BLOCK_OFFSET) {
				block[RF_LOCKDOWN_EEPROME_COLUMN_OFFSET] |= RF_LOCKDOWN_EEPROME_BIT;
				NeedWrite = TRUE;
			}

			/* Only write to Effuse when Needed contents are different in Effuse and Flash */
			if (NeedWrite == TRUE) {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("write block content: "));

				for (i = 0; i < EFUSE_BLOCK_SIZE; i++)
					MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%x ", (UINT)block[i]));

				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
				MtCmdEfuseAccessWrite(pAd, offset, &block[0]);
			}

			ptr += EFUSE_BLOCK_SIZE;
		}
	} else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RF is lock now. Cannot write back to Effuse!!\n"));

	return status;
}

INT SetRFBackup(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT     status = TRUE;
	CHAR    *value = 0;
	UCHAR   Param  = 0;
	UCHAR   block[EFUSE_BLOCK_SIZE] = "";
	UCHAR   i;
	USHORT  offset = RF_LOCKDOWN_EEPROME_BLOCK_OFFSET;
	UINT    isVaild = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			Param = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	if (Param == RF_VALIDATION_NUMBER) {
		/* Check RF lock down status */
		if (chip_check_rf_lock_down(pAd)) {
			/* Read Effuse Contents of Block Address 0x120 */
			MtCmdEfuseAccessRead(pAd, offset, &block[0], &isVaild);
			/* Configue Block 0x12C Content (Unlock RF lock) */
			block[RF_LOCKDOWN_EEPROME_COLUMN_OFFSET] &= (~(RF_LOCKDOWN_EEPROME_BIT));
			/* Write to Effuse */
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("write block content: "));

			for (i = 0; i < EFUSE_BLOCK_SIZE; i++)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%x ", (UINT)block[i]));

			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
			MtCmdEfuseAccessWrite(pAd, offset, &block[0]);
		} else
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("No need to unlock!!\n"));
	}

	return status;
}
#endif /* RF_LOCKDOWN */

INT set_hnat_register(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 reg_en;
	INT idx;
	struct wifi_dev *wdev;

	reg_en = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Device Instance\n"));

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWDEV %02d:", idx));

		if (pAd->wdev_list[idx]) {
			wdev = pAd->wdev_list[idx];
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t\tName:%s\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev)));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tWdev(list) Idx:%d\n", wdev->wdev_idx));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t Idx:%d\n", RtmpOsGetNetIfIndex(wdev->if_dev)));
#if defined(CONFIG_FAST_NAT_SUPPORT)

			if (ppe_dev_unregister_hook != NULL &&
				ppe_dev_register_hook != NULL) {
				if (reg_en)
					ppe_dev_register_hook(wdev->if_dev);
				else
					ppe_dev_unregister_hook(wdev->if_dev);
			}
#endif /*CONFIG_FAST_NAT_SUPPORT*/
		} else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}

	return TRUE;
}

INT Set_MibBucket_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR  MibBucketEnable;
	UCHAR       concurrent_bands = HcGetAmountOfBand(pAd);
	UCHAR i = 0;

	MibBucketEnable = os_str_tol(arg, 0, 10);

	/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()  BandIdx=%d, MibBucket Enable=%d\n", __func__, BandIdx, MibBucketEnable)); */
	for (i = 0; i < concurrent_bands; i++)
		pAd->OneSecMibBucket.Enabled[i] = MibBucketEnable;

	pAd->MsMibBucket.Enabled = MibBucketEnable;
	return TRUE;
}

#ifdef PKT_BUDGET_CTRL_SUPPORT
INT Set_PBC_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	 *value = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Set PBC Up bound:\n"));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		pAd->pbc_bound[i] = os_str_tol(value, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d: %d\n", i, pAd->pbc_bound[i]));
	}

	return TRUE;
}
#endif /*PKT_BUDGET_CTRL_SUPPORT*/

void hc_show_radio_info(struct _RTMP_ADAPTER *ad);
void hc_show_hdev_obj(struct wifi_dev *wdev);

/*dump radio information*/
INT show_radio_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	CHAR str[32] = "";
#ifdef DOT11_N_SUPPORT
	CHAR str2[32] = "";
#endif /*DOT11_N_SUPPORT*/
	CHAR *pstr = NULL;
	UCHAR i;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==========BBP radio information==========\n"));
#ifdef DBDC_MODE
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DBDCEn\t: %s\n",
			 (pAd->CommonCfg.dbdc_mode) ? "Enable" : "Disable"));
#endif /*DBDC_MODE*/
	/*show radio info per band*/
	hc_show_radio_info(pAd);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("##########WDEV radio information##########\n"));

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev) {
			UCHAR cfg_ext_cha = wlan_config_get_ext_cha(wdev);
			UCHAR op_ext_cha = wlan_operate_get_ext_cha(wdev);

			pstr = wdev_type2str(wdev->wdev_type);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==========wdev(%d)==========\n", i));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("type\t: %s\n", pstr));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("fun_idx\t: %d\n", wdev->func_idx));
			pstr = wmode_2_str(wdev->PhyMode);

			if (pstr != NULL) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wmode\t: %s\n", pstr));
				os_free_mem(pstr);
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("channel\t: %d\n", wlan_operate_get_prim_ch(wdev)));
#ifdef DOT11_N_SUPPORT

			if (WMODE_CAP_N(wdev->PhyMode)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cen_ch1\t: %d\n", wlan_operate_get_cen_ch_1(wdev)));
				bw_2_str(wlan_config_get_ht_bw(wdev), str);
				bw_2_str(wlan_operate_get_ht_bw(wdev), str2);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ht_bw\t: (%s,%s)\n", str, str2));
				extcha_2_str(cfg_ext_cha, str);
				extcha_2_str(op_ext_cha, str2);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ext_ch\t: (%s,%s)\n", str, str2));
			}

#ifdef DOT11_VHT_AC

			if (WMODE_CAP_AC(wdev->PhyMode)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cen_ch2\t: (%d,%d)\n",
						 wlan_config_get_cen_ch_2(wdev), wlan_operate_get_cen_ch_2(wdev)));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("vht_bw\t: (%d,%d)\n",
						 wlan_config_get_vht_bw(wdev), wlan_operate_get_vht_bw(wdev)));
			}

#endif /*DOT11_VHT_AC*/
#endif /*DOT11_N_SUPPORT*/
			bw_2_str(wlan_operate_get_bw(wdev), str);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bw\t: %s\n", str));
			/*hdev related*/
			hc_show_hdev_obj(wdev);
		}
	}

	return TRUE;
}

INT set_fix_amsdu(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	pAd->amsdu_fix_num = simple_strtol(arg, 0, 10);

	if (pAd->amsdu_fix_num > 0)
		pAd->amsdu_fix = TRUE;
	else
		pAd->amsdu_fix = FALSE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("amsdu fix nums = %u\n", pAd->amsdu_fix_num));
	return TRUE;
}

INT set_rx_max_cnt(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	pAd->PciHif.RxRing[HIF_RX_IDX0].max_rx_process_cnt = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("rx max count = %u\n", pAd->PciHif.RxRing[HIF_RX_IDX0].max_rx_process_cnt));
	return TRUE;
}
#ifdef MT7626_E2_SUPPORT
INT set_rx2_max_cnt(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	pAd->PciHif.RxRing[HIF_RX_IDX2].max_rx_process_cnt = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("rx2 max count = %u\n", pAd->PciHif.RxRing[HIF_RX_IDX2].max_rx_process_cnt));
	return TRUE;
}
#endif
INT set_rx1_max_cnt(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	pAd->PciHif.RxRing[HIF_RX_IDX1].max_rx_process_cnt = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("rx max count = %u\n", pAd->PciHif.RxRing[HIF_RX_IDX1].max_rx_process_cnt));
	return TRUE;
}

INT set_rx_dly_ctl(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	struct tr_delay_control *tr_delay_ctl = &pAd->tr_ctl.tr_delay_ctl;

	tr_delay_ctl->rx_delay_en = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("rx_delay_ctl_en = %u\n", tr_delay_ctl->rx_delay_en));
	return TRUE;
}

INT set_tx_dly_ctl(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	struct tr_delay_control *tr_delay_ctl = &pAd->tr_ctl.tr_delay_ctl;

	tr_delay_ctl->tx_delay_en = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("tx_delay_ctl_en = %u\n", tr_delay_ctl->tx_delay_en));
	return TRUE;
}

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
void set_pcie_aspm_dym_ctrl_cap(
	PRTMP_ADAPTER pAd,
	BOOLEAN flag_pcie_aspm_dym_ctrl)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (flag_pcie_aspm_dym_ctrl) {
		pChipCap->asic_caps |= fASIC_CAP_PCIE_ASPM_DYM_CTRL;
	} else {
		pChipCap->asic_caps &= ~fASIC_CAP_PCIE_ASPM_DYM_CTRL;
	}
}

BOOLEAN get_pcie_aspm_dym_ctrl_cap(
	PRTMP_ADAPTER pAd)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	return ((pChipCap->asic_caps & fASIC_CAP_PCIE_ASPM_DYM_CTRL) ? TRUE : FALSE);
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */


INT Set_BSSAifsn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}

	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pBssEdca is NULL\n"));
		return FALSE;
	}

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] < 2) || (value_temp[i] > 15)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("BSSAifsn[%d]=%d is invalid, BSSAifsn should be configured from 2 to 15\n",
				i, value_temp[i]));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure BSSAifsn\n"));

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pBssEdca->Aifsn[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSSAifsn[%d]=%d\n", i, pBssEdca->Aifsn[i]));
	}

	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	return TRUE;
}

INT Set_BSSCwmin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}

	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pBssEdca is NULL\n"));
		return FALSE;
	}

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] > 15)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("BSSCwmin[%d]=%d is invalid, BSSCwmin should not be over 15\n", i, value_temp[i]));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure BSSCwmin\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pBssEdca->Cwmin[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSSCwmin[%d]=%d\n", i, pBssEdca->Cwmin[i]));
	}

	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	return TRUE;
}

INT Set_BSSCwmax_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}

	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pBssEdca is NULL\n"));
		return FALSE;
	}

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] > 15)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("BSSCwmax[%d]=%d is invalid, BSSCwmax should not be over 15\n", i, value_temp[i]));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure BSSCwmax\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pBssEdca->Cwmax[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSSCwmax[%d]=%d\n", i, pBssEdca->Cwmax[i]));
	}

	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	return TRUE;
}

INT Set_BSSTxop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	USHORT value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}

	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pBssEdca is NULL\n"));
		return FALSE;
	}

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (USHORT)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure BSSTxop\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pBssEdca->Txop[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSSTxop[%d]=%d\n", i, pBssEdca->Txop[i]));
	}

	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	return TRUE;
}

UCHAR ac_queue[] = {
	TxQ_IDX_AC1, /* ACI:0 AC_BE */
	TxQ_IDX_AC0, /* ACI:1 AC_BK */
	TxQ_IDX_AC2, /* ACI:2 AC_VI */
	TxQ_IDX_AC3, /* ACI:3 AC_VO */
};

INT Set_APAifsn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = TRUE;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = HcGetEdca(pAd, wdev);

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] < 1) || (value_temp[i] > 15)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("APAifsn[%d]=%d is invalid, APAifsn should be configured from 1 to 15\n",
				i, value_temp[i]));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure APAifsn\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[i] = value_temp[i];
		if (wmm_edca_param)
			wmm_edca_param->Aifsn[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("APAifsn[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[i]));
		ac_index = ac_queue[i];
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_AIFSN, value_temp[i]) == NDIS_STATUS_FAILURE) {
			success = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("fails to set APAifsn[%d]\n", i));
		 }
	}

	return success;
}

INT Set_APCwmin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = TRUE;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = HcGetEdca(pAd, wdev);

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] > 16)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("APCwmin[%d]=%d is invalid, APCwmin should not be over 16\n", i, value_temp[i]));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure APCwmin\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[i] = value_temp[i];
		if (wmm_edca_param)
			wmm_edca_param->Cwmin[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("APCwmin[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[i]));
		ac_index = ac_queue[i];
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_CWMIN, value_temp[i]) == NDIS_STATUS_FAILURE) {
			success = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("fails to set APCwmin[%d]\n", i));
		 }
	}

	return success;
}

INT Set_APCwmax_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = TRUE;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = HcGetEdca(pAd, wdev);

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] > 16)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("APCwmax[%d]=%d is invalid, APCwmax should not be over 16\n", i, value_temp[i]));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure APCwmax\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[i] = value_temp[i];
		if (wmm_edca_param)
			wmm_edca_param->Cwmax[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("APCwmax[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[i]));
		ac_index = ac_queue[i];
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_CWMAX, value_temp[i]) == NDIS_STATUS_FAILURE) {
			success = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("fails to set APCwmax[%d]\n", i));
		 }
	}

	return success;
}

INT Set_APTxop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = TRUE;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
	USHORT value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = HcGetEdca(pAd, wdev);

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (USHORT)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure APTxop\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[i] = value_temp[i];
		if (wmm_edca_param)
			wmm_edca_param->Txop[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("APTxop[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[i]));
		ac_index = ac_queue[i];
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_TXOP, value_temp[i]) == NDIS_STATUS_FAILURE) {
			success = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("fails to set APTxop[%d]\n", i));
		 }
	}

	return success;
}

#ifdef MGMT_TXPWR_CTRL
/* Tx pwr offset in 0.5db unit*/
#define TX_PWR_OFFSET_WTBL_MIN -31
#define TX_PWR_OFFSET_WTBL_MAX 0

INT set_mgmt_frame_power(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT16  value;
	CHAR   target_pwr = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	RTMP_STRING *tmp;
	struct wifi_dev *wdev;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid argument\n"));
		return FALSE;
	}

	if (*arg == '.') { /* handle .5 arg value*/
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("unable to update tx pwr %s\n", arg));
		return TRUE;
	}

	tmp = rstrtok(arg, ".");

	if (!tmp) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Please enter valid tx pwr\n"));
		return;
	}

	value = os_str_tol(tmp, 0, 10);
	target_pwr = value*2; /* convert to  0.5db scale*/
	tmp = rstrtok(NULL, ".");

	if (tmp) {
		value = os_str_tol(tmp, 0, 10);
		if (value == 5)
			target_pwr += 1;
		else if (target_pwr == 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("unable to update tx pwr\n"));
			return TRUE;
		}
	}

	wdev->MgmtTxPwr = target_pwr;
	update_mgmt_frame_power(pAd, wdev);
	return TRUE;
}

INT update_mgmt_frame_power(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	CHAR   target_pwr, delta_pwr;
	UINT8  OpMode, BandIdx;

	BandIdx = HcGetBandByWdev(wdev);
	OpMode = wdev->rate.MlmeTransmit.field.MODE;

	target_pwr = wdev->MgmtTxPwr;

	if (target_pwr == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("[%s] disable mgmt pwr ctrl\n", __func__));
		wdev->bPwrCtrlEn = FALSE;
		wdev->TxPwrDelta = 0;
		wtbl_update_pwr_offset(pAd, wdev);
		goto exit;
	}

	if (pAd->ApCfg.MgmtTxPwr[BandIdx] == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] Mgmt Tx base pwr zero apidx:%d band-idx:%d\n",
		__func__, wdev->func_idx, BandIdx));
		MtCmdTxPwrShowInfo(pAd, TXPOWER_ALL_RATE_POWER_INFO, BandIdx);
		return FALSE;
	}

	delta_pwr = target_pwr - pAd->ApCfg.MgmtTxPwr[BandIdx];

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s] Target_Pwr=%d Base_Pwr=%d delta_Pwr=%d!!!\n",
		__func__, target_pwr/2, (pAd->ApCfg.MgmtTxPwr[BandIdx])/2, delta_pwr/2));

	if (delta_pwr >= TX_PWR_OFFSET_WTBL_MIN && delta_pwr <= TX_PWR_OFFSET_WTBL_MAX) {
		/* update wtbl tx pwr offset*/
		wdev->bPwrCtrlEn = TRUE;
		wdev->TxPwrDelta = delta_pwr;
		wtbl_update_pwr_offset(pAd, wdev);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] unable to adjust target pwr \n", __func__));
		return FALSE;
	}

exit:
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_DISABLE_TX);
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_ENABLE_TX);

	return TRUE;
}

INT show_mgmt_frame_power(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8	Tx_Pwr, BandIdx;
	struct wifi_dev	*wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);

	Tx_Pwr = pAd->ApCfg.MgmtTxPwr[BandIdx] + wdev->TxPwrDelta;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Tx_Pwr = %d.%d db\n",
		__func__, Tx_Pwr/2, ((Tx_Pwr%2 == 0) ? 0:5)));

	return TRUE;
}
#endif

#ifdef WIFI_EAP_FEATURE
#ifdef AMPDU_CONF_SUPPORT
#define AMPDU_MAX_RETRY_COUNT   0x1F
#define MAX_MPDU_COUNT_FOR_AGG  64


INT Set_AMPDU_Retry_Count(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx = 0, count = 0;
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Incorrect BSS!!\n"));
		return FALSE;
	}
	count = os_str_tol(arg, 0, 10);
	if ((count < 0)  || (count > AMPDU_MAX_RETRY_COUNT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Retry range is: [0:1F]!!\n"));
		return FALSE;
	}

	wdev->bRetryEnable = TRUE;
	wdev->bAMPDURetrynum = count;

	if (count == 0)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set to cannot Tx!!\n", count));
	else if (count == AMPDU_MAX_RETRY_COUNT)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set to unlimited Tx Retry!!\n"));
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("Set Number Tx retry count = %d!!\n", count));

	return TRUE;
}


INT Set_AMPDU_MPDU_Count(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx = 0, count = 0, AALCRIdx = 0;
	UINT32 mac_val;
	struct wifi_dev *wdev = NULL;
	EAP_AGG_LIMIT_SET Eap_agg_limit;

#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Incorrect BSS!!\n"));
		return FALSE;
	}
	count = os_str_tol(arg, 0, 10);
	if (count > MAX_MPDU_COUNT_FOR_AGG) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("Max MPDU in AMPDU support = 64!!\n"));
		return FALSE;
	}
	if ((BandIdx == DBDC_BAND1) ||
		(WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN)) || WMODE_EQUAL(wdev->PhyMode, WMODE_G)))
		AALCRIdx = 1;
	else
		AALCRIdx = 0;

	os_zero_mem(&Eap_agg_limit, sizeof(Eap_agg_limit));
	Eap_agg_limit.AALCRIdx = AALCRIdx;
	Eap_agg_limit.Count = count;
	CmdEapSetAGGLimit(pAd, &Eap_agg_limit, mac_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Number of MPDUs in AMPDU set = %d!!\n", count));
	return TRUE;
}

INT Show_AMPDU_retry_Count(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx = 0;
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Incorrect BSS!!\n"));
			return FALSE;
	}
	if (!(wdev->bRetryEnable))
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("Band = %d Default AMPDU retry count = 15\n", BandIdx));
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("Band = %d AMPDU retry count = %d!!\n", BandIdx, (wdev->bAMPDURetrynum)));

	return TRUE;
}
#define WF_AGG_BASE		0x22000
#define AGG_AALCR              (WF_AGG_BASE + 0x040)   /* 0x21240 */
#define AGG_AALCR1             (WF_AGG_BASE + 0x044)   /* 0x21244 */

INT Show_AMPDU_MPDU_Count(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx = 0, AALCRIdx = 0;
	UINT32 mac_val = 0;
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("Incorrect BSS!!\n"));
			return FALSE;
	}
	if ((BandIdx == DBDC_BAND1) || (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
							|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)))
		MAC_IO_READ32(pAd->hdev_ctrl, AGG_AALCR, &mac_val);
	else
		MAC_IO_READ32(pAd->hdev_ctrl, AGG_AALCR1, &mac_val);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("Number of MPDUs in AMPDU set = %d!!\n", (mac_val)?(mac_val & 0x3f):MAX_MPDU_COUNT_FOR_AGG));

	return TRUE;
}
#endif /* AMPDU_CONF_SUPPORT */

#ifdef ACK_CTS_TIMEOUT_SUPPORT
INT Set_DST_to_ACK_Timeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 mac_val = 0, bTimeout = 0, distance = 0;
	EAP_ACKTIMEOUT_SET Eap_acktimeout;

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("No parameters!!\n"));
		return FALSE;
	}
	distance = os_str_tol(arg, 0, 10);
	pAd->CommonCfg.bDistance = distance;

	/* Calculate, timeout=((distance/speed of light)*2) */
	bTimeout = distance/SPEED_OF_LIGHT_IN_METER;
	bTimeout *= 2;

	if ((bTimeout < 0) || (bTimeout > CCK_OFDM_MAX_ACK_TIMEOUT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("CTS/ACK Timeout Range should between [0xFFFF:0]!!\n"));
		return FALSE;
	}
	os_zero_mem(&Eap_acktimeout, sizeof(Eap_acktimeout));
	Eap_acktimeout.ModuleMode = MODE_CCK;
	Eap_acktimeout.bTimeout = bTimeout;
	/* Configure CCK ACK Timeout CR*/
	CmdEapSetAckTimeout(pAd, &Eap_acktimeout);
	/* Configure OFDM ACK Timeout CR*/
	os_zero_mem(&Eap_acktimeout, sizeof(Eap_acktimeout));
	Eap_acktimeout.ModuleMode = MODE_OFDM;
	Eap_acktimeout.bTimeout = bTimeout;
	CmdEapSetAckTimeout(pAd, &Eap_acktimeout);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("Set CCK/OFDM CTS/ACK Timeout = %d for distance =%d!!\n", bTimeout, pAd->CommonCfg.bDistance));
	return TRUE;
}

INT Set_CCK_ACK_Timeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 mac_val = 0, bTimeout = 0;
	EAP_ACKTIMEOUT_SET Eap_acktimeout;

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("No parameters!!\n"));
		return FALSE;
	}

	bTimeout = os_str_tol(arg, 0, 10);
	if ((bTimeout < 0) || (bTimeout > CCK_OFDM_MAX_ACK_TIMEOUT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("CCK CTS/ACK Time Range [0xFFFF:0]!!\n"));
		return FALSE;
	}

	os_zero_mem(&Eap_acktimeout, sizeof(Eap_acktimeout));
	Eap_acktimeout.ModuleMode = MODE_CCK;
	Eap_acktimeout.bTimeout = bTimeout;
	/* Configure CCK ACK Timeout CR*/
	CmdEapSetAckTimeout(pAd, &Eap_acktimeout);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("CCK CTS/ACK Timeout set = %d!!\n", bTimeout));
	return TRUE;
}

INT Set_OFDM_ACK_Timeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 mac_val = 0, bTimeout = 0;
	EAP_ACKTIMEOUT_SET Eap_acktimeout;

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("No parameters!!\n"));
		return FALSE;
	}

	bTimeout = os_str_tol(arg, 0, 10);
	if ((bTimeout < 0) || (bTimeout > CCK_OFDM_MAX_ACK_TIMEOUT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("OFDM CTS/ACK Timeout Range [0xFFFF:0]!!\n"));
		return FALSE;
	}
	os_zero_mem(&Eap_acktimeout, sizeof(Eap_acktimeout));
	Eap_acktimeout.ModuleMode = MODE_OFDM;
	Eap_acktimeout.bTimeout = bTimeout;
	/* Configure OFDM ACK Timeout CR*/
	CmdEapSetAckTimeout(pAd, &Eap_acktimeout);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("OFDM CTS/ACK Timeout set = %d!!\n", bTimeout));
	return TRUE;
}
#define WF_TMAC_BASE 0x24000
#define TMAC_CDTR (WF_TMAC_BASE + 0x90)
#define TMAC_ODTR (WF_TMAC_BASE + 0x94)

void Show_Distance_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("Distance set = %d m/s!!\n", pAd->CommonCfg.bDistance));
	return TRUE;
}

INT Show_CCK_ACK_Timeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 mac_val = 0;
	/* Read CCK ACK Time CR*/
	MAC_IO_READ32(pAd->hdev_ctrl, TMAC_CDTR, &mac_val);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("CCK CTS/ACK Timeout = %d!!\n", (mac_val & CCK_OFDM_MAX_ACK_TIMEOUT)));
	return TRUE;
}

INT Show_OFDM_ACK_Timeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 mac_val = 0;
	/* Read OFDM ACK time CR */
	MAC_IO_READ32(pAd->hdev_ctrl, TMAC_ODTR, &mac_val);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("OFDM CTS/ACK Timeout = %d!!\n", (mac_val & CCK_OFDM_MAX_ACK_TIMEOUT)));
	return TRUE;
}

#endif /* ACK_CTS_TIMEOUT_SUPPORT */
#endif /* WIFI_EAP_FEATURE */
