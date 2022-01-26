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
	cmm_info.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include "rt_config.h"
#ifdef TXRX_STAT_SUPPORT
#include "hdev/hdev_basic.h"
#endif

#define MCAST_WCID_TO_REMOVE 0 /* Pat: TODO */

INT MCSMappingRateTable[] = {
	2,  4, 11, 22, 12,  18,  24,  36, 48,  72,  96, 108, 109, 110, 111, 112,/* CCK and OFDM */
	13, 26, 39, 52, 78, 104, 117, 130, 26,  52,  78, 104, 156, 208, 234, 260,
	39, 78, 117, 156, 234, 312, 351, 390, /* BW 20, 800ns GI, MCS 0~23 */
	27, 54, 81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540,
	81, 162, 243, 324, 486, 648, 729, 810, /* BW 40, 800ns GI, MCS 0~23 */
	14, 29, 43, 57, 87, 115, 130, 144, 29, 59,   87, 115, 173, 230, 260, 288,
	43, 87, 130, 173, 260, 317, 390, 433, /* BW 20, 400ns GI, MCS 0~23 */
	30, 60, 90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600,
	90, 180, 270, 360, 540, 720, 810, 900, /* BW 40, 400ns GI, MCS 0~23 */

	/*for 11ac:20 Mhz 800ns GI*/
	6,  13, 19, 26,  39,  52,  58,  65,  78,  0,     /*1ss mcs 0~8*/
	13, 26, 39, 52,  78,  104, 117, 130, 156, 0,     /*2ss mcs 0~8*/
	19, 39, 58, 78,  117, 156, 175, 195, 234, 260,   /*3ss mcs 0~9*/
	26, 52, 78, 104, 156, 208, 234, 260, 312, 0,     /*4ss mcs 0~8*/

	/*for 11ac:40 Mhz 800ns GI*/
	13,	27,	40,	54,	 81,  108, 121, 135, 162, 180,   /*1ss mcs 0~9*/
	27,	54,	81,	108, 162, 216, 243, 270, 324, 360,   /*2ss mcs 0~9*/
	40,	81,	121, 162, 243, 324, 364, 405, 486, 540,  /*3ss mcs 0~9*/
	54,	108, 162, 216, 324, 432, 486, 540, 648, 720, /*4ss mcs 0~9*/

	/*for 11ac:80 Mhz 800ns GI*/
	29,	58,	87,	117, 175, 234, 263, 292, 351, 390,   /*1ss mcs 0~9*/
	58,	117, 175, 243, 351, 468, 526, 585, 702, 780, /*2ss mcs 0~9*/
	87,	175, 263, 351, 526, 702, 0,	877, 1053, 1170, /*3ss mcs 0~9*/
	117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560, /*4ss mcs 0~9*/

	/*for 11ac:160 Mhz 800ns GI*/
	58,	117, 175, 234, 351, 468, 526, 585, 702, 780, /*1ss mcs 0~9*/
	117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560, /*2ss mcs 0~9*/
	175, 351, 526, 702, 1053, 1404, 1579, 1755, 2160, 0, /*3ss mcs 0~8*/
	234, 468, 702, 936, 1404, 1872, 2106, 2340, 2808, 3120, /*4ss mcs 0~9*/

	/*for 11ac:20 Mhz 400ns GI*/
	7,	14,	21,	28,  43,  57,   65,	 72,  86,  0,    /*1ss mcs 0~8*/
	14,	28,	43,	57,	 86,  115,  130, 144, 173, 0,    /*2ss mcs 0~8*/
	21,	43,	65,	86,	 130, 173,  195, 216, 260, 288,  /*3ss mcs 0~9*/
	28,	57,	86,	115, 173, 231,  260, 288, 346, 0,    /*4ss mcs 0~8*/

	/*for 11ac:40 Mhz 400ns GI*/
	15,	30,	45,	60,	 90,  120,  135, 150, 180, 200,  /*1ss mcs 0~9*/
	30,	60,	90,	120, 180, 240,  270, 300, 360, 400,  /*2ss mcs 0~9*/
	45,	90,	135, 180, 270, 360,  405, 450, 540, 600, /*3ss mcs 0~9*/
	60,	120, 180, 240, 360, 480,  540, 600, 720, 800, /*4ss mcs 0~9*/

	/*for 11ac:80 Mhz 400ns GI*/
	32,	65,	97,	130, 195, 260,  292, 325, 390, 433,  /*1ss mcs 0~9*/
	65,	130, 195, 260, 390, 520,  585, 650, 780, 866, /*2ss mcs 0~9*/
	97,	195, 292, 390, 585, 780,  0,	 975, 1170, 1300, /*3ss mcs 0~9*/
	130, 260, 390, 520, 780, 1040,	1170, 1300, 1560, 1733, /*4ss mcs 0~9*/

	/*for 11ac:160 Mhz 400ns GI*/
	65,	130, 195, 260, 390, 520,  585, 650, 780, 866, /*1ss mcs 0~9*/
	130, 260, 390, 520, 780, 1040,	1170, 1300, 1560, 1733, /*2ss mcs 0~9*/
	195, 390, 585, 780, 1170, 1560,	1755, 1950, 2340, 0, /*3ss mcs 0~8*/
	260, 520, 780, 1040, 1560, 2080,	2340, 2600, 3120, 3466, /*4ss mcs 0~9*/

	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37
}; /* 3*3 */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

extern struct wifi_fwd_func_table wf_drv_tbl;

#endif

/*
    ==========================================================================
    Description:
	Get Driver version.

    Return:
    ==========================================================================
*/
INT show_driverinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Driver version: %s\n", AP_DRIVER_VERSION));
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Driver version: %s\n", STA_DRIVER_VERSION));
#endif /* CONFIG_STA_SUPPORT */

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("FW ver: 0x%x, HW ver: 0x%x, CHIP ID: 0x%x\n", pAd->FWVersion, pAd->HWVersion, pAd->ChipID));

	show_patch_info(pAd);
	show_fw_info(pAd);

	return TRUE;
}

#if defined(BB_SOC) && defined(TCSUPPORT_WLAN_SW_RPS)
extern int (*ecnt_set_wifi_rps_hook)(int RxOn, int WLanCPU, int TxOn, int LanCPU);
extern int rx_detect_flag;

INT	Set_RxMaxTraffic_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG flag;

	flag = os_str_tol(arg, 0, 10);
	printk("old pAd->rxThreshold = %d Mbps\n", pAd->rxThreshold);
	pAd->rxThreshold = flag;
	printk("new pAd->rxThreshold = %d Mbps\n", pAd->rxThreshold);

	return TRUE;
}

INT	Set_rx_detect_flag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG flag;

	flag = os_str_tol(arg, 0, 10);
	rx_detect_flag = flag;
	printk("rx_detect_flag = %d\n", rx_detect_flag);
	if (rx_detect_flag == 0) {
		if (ecnt_set_wifi_rps_hook)
			ecnt_set_wifi_rps_hook(0, 0, 0, 0);
	}

	return TRUE;
}
#endif

/*
    ==========================================================================
    Description:
	Set Country Region.
	This command will not work, if the field of CountryRegion in eeprom is programmed.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryRegion_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int retval;
	UCHAR BandIdx, IfIdx;
	CHANNEL_CTRL *pChCtrl;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	IfIdx = pObj->ioctl_if;

	if (pObj->ioctl_if_type == INT_MBSSID || pObj->ioctl_if_type == INT_MAIN) {
#ifdef CONFIG_AP_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
#endif
	}
#ifdef CONFIG_STA_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
	}
	else if (pObj->ioctl_if_type == INT_MSTA) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
	}
#endif
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[Set_CountryRegion_Proc]: pObj->ioctl_if_type = %d!!\n", pObj->ioctl_if_type));
		return FALSE;
	}
#ifdef RF_LOCKDOWN

	/* Check RF lock Status */
	if (chip_check_rf_lock_down(pAd)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! Cannot config CountryRegion status!!\n",
				 __func__));
		return TRUE;
	}

#endif /* RF_LOCKDOWN */
#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif /* EXT_BUILD_CHANNEL_LIST */
	retval = RT_CfgSetCountryRegion(pAd, arg, BAND_24G);

	if (retval == FALSE)
		return FALSE;

	/* If country region is set, driver needs to be reset*/
	/* Change channel state to NONE */
	BandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
	BuildChannelList(pAd, wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_CountryRegion_Proc::(CountryRegion=%d)\n",
			 pAd->CommonCfg.CountryRegion));
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set Country Region for A band.
	This command will not work, if the field of CountryRegion in eeprom is programmed.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryRegionABand_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int retval;
	UCHAR BandIdx, IfIdx;
	CHANNEL_CTRL *pChCtrl;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	IfIdx = pObj->ioctl_if;

	if (pObj->ioctl_if_type == INT_MBSSID || pObj->ioctl_if_type == INT_MAIN) {
#ifdef CONFIG_AP_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
#endif
	}
#ifdef CONFIG_STA_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
	}
	else if (pObj->ioctl_if_type == INT_MSTA) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
	}
#endif
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[Set_CountryRegionABand_Proc]: pObj->ioctl_if_type = %d!!\n", pObj->ioctl_if_type));
		return FALSE;
	}
#ifdef RF_LOCKDOWN

	/* Check RF lock Status */
	if (chip_check_rf_lock_down(pAd)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! Cannot config CountryRegion status!!\n",
				 __func__));
		return TRUE;
	}

#endif /* RF_LOCKDOWN */
#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif /* EXT_BUILD_CHANNEL_LIST */
	retval = RT_CfgSetCountryRegion(pAd, arg, BAND_5G);

	if (retval == FALSE)
		return FALSE;

	/* If Country Region is set, channel list needs to be rebuilt*/
	/* Change channel state to NONE */
	BandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
	BuildChannelList(pAd, wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_CountryRegionABand_Proc::(CountryRegion=%d)\n",
			 pAd->CommonCfg.CountryRegionForABand));
	return TRUE;
}

INT	Set_Cmm_WirelessMode_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	INT	success = TRUE;
	LONG cfg_mode = os_str_tol(arg, 0, 10);
	UCHAR wmode = cfgmode_2_wmode((UCHAR)cfg_mode);
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT
	UINT32 i = 0;
	struct wifi_dev *TmpWdev = NULL;
#endif
	CHANNEL_CTRL *pChCtrl;
	UCHAR BandIdx;

	if (!wmode_valid_and_correct(pAd, &wmode)) {
		success = FALSE;
		goto error;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
		if (!wdev)
			return FALSE;

		BandIdx = HcGetBandByWdev(wdev);

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			TmpWdev = &pAd->ApCfg.MBSSID[i].wdev;
			/*update wmode*/
			if (TmpWdev && (BandIdx == HcGetBandByWdev(TmpWdev)))
				TmpWdev->PhyMode = wmode;

			HcAcquireRadioForWdev(pAd, TmpWdev);
			RTMPSetPhyMode(pAd, TmpWdev, wmode);
			RTMPUpdateRateInfo(wmode, &TmpWdev->rate);
			UpdateBeaconHandler(pAd, TmpWdev, BCN_UPDATE_IE_CHG);
		}
		pAd->CommonCfg.cfg_wmode = wmode;
		/* Change channel state to NONE */
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
		SCAN_INFO *ScanInfo = NULL;
		BSS_TABLE *ScanTab = NULL;
		wdev = &pAd->StaCfg[pObj->ioctl_if].wdev;
		wdev->PhyMode = wmode;
		ScanInfo = &wdev->ScanInfo;
		ScanTab = get_scan_tab_by_wdev(pAd, wdev);
		success = RT_CfgSetWirelessMode(pAd, arg, wdev);

		if (!success)
			goto error;

		HcAcquireRadioForWdev(pAd, wdev);
		/* Change channel state to NONE */
		BandIdx = HcGetBandByWdev(wdev);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
		BuildChannelList(pAd, wdev);
		RTMPSetPhyMode(pAd, wdev, wmode);
		RTMPUpdateRateInfo(wmode, &wdev->rate);
		BssTableInit(ScanTab);
		ScanInfo->LastScanTime = 0;
#ifdef DOT11_N_SUPPORT

		if (WMODE_CAP_N(wmode)) {
			pAd->CommonCfg.BACapability.field.AutoBA = TRUE;
			pAd->CommonCfg.REGBACapability.field.AutoBA = TRUE;
		} else {
			pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
			pAd->CommonCfg.REGBACapability.field.AutoBA = FALSE;
		}

#endif /* DOT11_N_SUPPORT */

		/* Set AdhocMode rates*/
		if (pStaCfg->BssType == BSS_ADHOC) {
			MlmeUpdateTxRates(pAd, FALSE, 0);
			UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IF_STATE_CHG);
			AsicEnableIbssSync(
				pAd,
				pAd->CommonCfg.BeaconPeriod,
				HW_BSSID_0,
				OPMODE_ADHOC);
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_Cmm_WirelessMode_Proc::(BSS%d=%d)\n", pObj->ioctl_if,
			 wdev->PhyMode));
	return success;
error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_WirelessMode_Proc::parameters out of range\n"));
	return success;
}

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
/*
    ==========================================================================
    Description:
	Set Wireless Mode for MBSS
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_MBSS_WirelessMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return Set_Cmm_WirelessMode_Proc(pAd, arg);
}
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

/*
    ==========================================================================
    Description:
	Set Wireless Mode
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_WirelessMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return Set_Cmm_WirelessMode_Proc(pAd, arg);
}

#ifdef RT_CFG80211_SUPPORT
INT Set_DisableCfg2040Scan_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->cfg80211_ctrl.FlgCfg8021Disable2040Scan = (UCHAR) os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pAd->cfg80211_ctrl.FlgCfg8021Disable2040Scan  %d\n",
			 pAd->cfg80211_ctrl.FlgCfg8021Disable2040Scan));
	return TRUE;
}
#endif

/*
*	==========================================================================
*	Description:
*	Set ProbeRspTimes
*	Return:
*	TRUE if all parameters are OK, FALSE otherwise
*	==========================================================================
*/
INT Set_Probe_Rsp_Times_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 ProbeRspTimes = (UCHAR) os_str_tol(arg, 0, 10);

	if ((ProbeRspTimes > 10) || (ProbeRspTimes < 1)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("Set_Probe_Rsp_Times_Proc! INVALID, ProbeRspTimes(%d) should be <1~10>\n",
			ProbeRspTimes));
		return FALSE;
	}

	cap->ProbeRspTimes = ProbeRspTimes;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Set_Probe_Rsp_Times_Proc! ProbeRspTimes = %d\n", cap->ProbeRspTimes));

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set Channel
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_Channel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	UCHAR Channel = (UCHAR) os_str_tol(arg, 0, 10);

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: wdev == NULL! if_type %d, if_idx = %d\n", __func__,
				 pObj->ioctl_if_type,
				 if_idx));
		return FALSE;
	}

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	DfsDedicatedExamineSetNewCh(pAd, Channel);
	DedicatedZeroWaitStop(pAd, TRUE);
#endif

#ifdef CONFIG_MAP_SUPPORT
	wdev->quick_ch_change = FALSE;
#endif

	return rtmp_set_channel(pAd, wdev, Channel);
}

#ifdef CONFIG_MAP_SUPPORT



/*
*	==========================================================================
*	Description:
*		This routine reset the entire MAC table. All packets pending in
*		the power-saving queues are freed here.
*	==========================================================================
*/
VOID MacTableResetNonMapWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	int i;
#ifdef CONFIG_AP_SUPPORT
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT Reason;
	struct _BSS_STRUCT *mbss;
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pMacEntry;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MacTableResetWdev\n"));

	/* TODO:Carter, check why start from 1 */
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pMacEntry = &pAd->MacTab.Content[i];

		if (pMacEntry->wdev != wdev)
			continue;
#ifdef CONFIG_MAP_SUPPORT
	if ((IS_MAP_TURNKEY_ENABLE(pAd)) &&
		((pMacEntry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)) &&
		(wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_BSS))))
		continue;

#endif
		if (IS_ENTRY_CLIENT(pMacEntry)) {
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
			pMacEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				/* Before reset MacTable, send disassociation packet to client.*/
				if (pMacEntry->Sst == SST_ASSOC) {
					/*	send out a De-authentication request frame*/
					NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

					if (NStatus != NDIS_STATUS_SUCCESS) {
						MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							(" MlmeAllocateMemory fail  ..\n"));
						/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
						return;
					}

					Reason = REASON_NO_LONGER_VALID;
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						("Send DeAuth (Reason=%d) to %02x:%02x:%02x:%02x:%02x:%02x\n",
							 Reason, PRINT_MAC(pMacEntry->Addr)));
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pMacEntry->Addr,
									 wdev->if_addr,
									 wdev->bssid);
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
									  sizeof(HEADER_802_11), &DeAuthHdr,
									  2, &Reason,
									  END_OF_ARGS);
					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
					MlmeFreeMemory(pOutBuffer);
					RtmpusecDelay(5000);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}

		/* Delete a entry via WCID */
		MacTableDeleteEntry(pAd, i, pMacEntry->Addr);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		mbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
#ifdef WSC_AP_SUPPORT
		{
			BOOLEAN Cancelled;

			RTMPCancelTimer(&mbss->wdev.WscControl.EapolTimer, &Cancelled);
			mbss->wdev.WscControl.EapolTimerRunning = FALSE;
			NdisZeroMemory(mbss->wdev.WscControl.EntryAddr, MAC_ADDR_LEN);
			mbss->wdev.WscControl.EapMsgRunning = FALSE;
		}
#endif /* WSC_AP_SUPPORT */
		mbss->StaCount = 0;
	}
#endif /* CONFIG_AP_SUPPORT */
}

void update_ch_by_wdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		/* disconnect all STA in case of 2.4G, and non MAP sta in case of 5G*/
		if (wdev->channel > 14)
			MacTableResetNonMapWdev(pAd, wdev);
		else
			MacTableResetWdev(pAd, wdev);

		if (WMODE_CAP_5G(wdev->PhyMode)) {
#ifdef MT_DFS_SUPPORT /* Jelly20150217 */
			WrapDfsRadarDetectStop(pAd);
			/* Zero wait hand off recovery for CAC period + interface down case */
			DfsZeroHandOffRecovery(pAd, wdev);
#endif
		}

	}
	wlan_operate_init(wdev);
	wdev->quick_ch_change = FALSE;
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
}


void ap_phy_rrm_init_byRf(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR i = 0;
	struct wifi_dev *tdev;
	UCHAR band_idx = HcGetBandByWdev(wdev);

	for (i = 0; i < WDEV_NUM_MAX; i++) {

		tdev = pAd->wdev_list[i];
		if (tdev && HcIsRadioAcq(tdev) && (band_idx == HcGetBandByWdev(tdev))) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Wlan_operate_init for : %s\n", (char *)tdev->if_dev->name));
			update_ch_by_wdev(pAd, tdev);
		} else if ((wdev->wdev_type == WDEV_TYPE_STA) &&
				(tdev != NULL) &&
				(tdev->wdev_type == WDEV_TYPE_AP) &&
				(tdev->if_up_down_state == 0) &&
				(((tdev->channel > 14) && (wdev->channel > 14)) ||
				((tdev->channel <= 14) && (wdev->channel <= 14)))) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Wlan_operate_init--1 for : %s\n", (char *)tdev->if_dev->name));
			update_ch_by_wdev(pAd, tdev);
		}
	}
	if (wdev->channel < 14) {
		/*Do Obss scan*/
		UINT8 idx;
		BSS_STRUCT *pCurMbss = NULL;

		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];

			/* check MBSS status is up */
			if (!pCurMbss->wdev.if_up_down_state)
				continue;

			if (wdev->channel <  14) {
				/* check MBSS work on the same RF(channel) */
				if (pCurMbss->wdev.channel == wdev->channel) {
					ap_over_lapping_scan(pAd, pCurMbss);
					break;
				}
			}
		}
	}
}




/*
*    ==========================================================================
*    Description:
*	Enable/disable quick Channel change feature
*    Return:
*	TRUE if all parameters are OK, FALSE otherwise
*    ==========================================================================
*/
INT Set_Map_Channel_En_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR MapChannelEn = (UCHAR) os_str_tol(arg, 0, 10);
	INT32 success = TRUE;	/*FALSE = 0*/

	pAd->bMAPQuickChChangeEn = MapChannelEn;
	return success;
}


/*
*    ==========================================================================
*    Description:
*	Set Channel quickly without AP start/stop
*    Return:
*	TRUE if all parameters are OK, FALSE otherwise
*    ==========================================================================
*/
INT Set_Map_Channel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	UCHAR Channel = (UCHAR) os_str_tol(arg, 0, 10);
	INT32 success = FALSE;	/*FALSE = 0*/
	if (pAd->bMAPQuickChChangeEn == FALSE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MAPQuickChChange feaure not enabled!!"));
		return Set_Channel_Proc(pAd, arg);
	}

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: wdev == NULL! if_type %d, if_idx = %d\n", __func__,
				 pObj->ioctl_if_type,
				 if_idx));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s\n", __func__));

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	DfsDedicatedExamineSetNewCh(pAd, Channel);
	DedicatedZeroWaitStop(pAd, TRUE);
#endif
	wdev->quick_ch_change = TRUE;

	success = rtmp_set_channel(pAd, wdev, Channel);
	if (wdev->wdev_type == WDEV_TYPE_STA)
		wdev->quick_ch_change = FALSE;

	return success;
}
#endif


INT	rtmp_set_channel(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel)
{
	INT32 Success = TRUE;
	UCHAR OriChannel;
	UCHAR PhyMode;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = NULL;
	PNET_DEV ndev_ap_if = NULL;
	UCHAR i = 0;
#endif
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: wdev == NULL!\n", __func__));
		return FALSE;
	}

	if (IsHcRadioCurStatOffByWdev(wdev))
		return Success;

	pDot11h = wdev->pDot11_H;
	PhyMode = wdev->PhyMode;
	OriChannel = wdev->channel;

	/* check if this channel is valid*/
	if (ChannelSanityByWdev(pAd, wdev, Channel) == TRUE) {
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			/* Save the channel on MlmeAux for CntlOidRTBssidProc used. */
			pStaCfg->MlmeAux.Channel = Channel;
			/*apply channel directly*/
			wlan_operate_set_prim_ch(wdev, Channel);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): CtrlChannel(%d), CentralChannel(%d)\n",
					 __func__, Channel,
					 wlan_operate_get_cen_ch_1(wdev)));
		}
#endif /* CONFIG_STA_SUPPORT */
		Success = TRUE;
	} else {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			Channel = FirstChannel(pAd, wdev);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 ("This channel is out of channel list, set as the first channel(%d)\n ", Channel));
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		Success = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("This channel is out of channel list, nothing to do!\n "));
#endif /* CONFIG_STA_SUPPORT */
	}

	/*used for not support MCC*/
	wdev->channel = Channel;
	/*sync to other device*/
	wdev_sync_prim_ch(wdev->sys_handle, wdev);

#ifdef CONFIG_AP_SUPPORT
		if (pAd->CommonCfg.bIEEE80211H == TRUE) {
			if (CheckNonOccupancyChannel(pAd, wdev, RDD_CHECK_NOP_BY_WDEV) == FALSE) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (
					"%s: Can not update channel(%d), restoring old channel(%d)\n",
						__func__, wdev->channel, OriChannel));
				wdev->channel = OriChannel;
				wdev_sync_prim_ch(wdev->sys_handle, wdev);
				return FALSE;
			}
		}
#endif

	if (pDot11h == NULL)
		return FALSE;
#ifdef CONFIG_AP_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	} else if (IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA)) {
		/* for APCLI, find first BSS with same channel */
		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			if ((pAd->ApCfg.MBSSID[i].wdev.channel == wdev->channel) &&
					(pAd->ApCfg.MBSSID[i].wdev.if_up_down_state != 0)) {
				pMbss = &pAd->ApCfg.MBSSID[i];
				break;
			}
		}
	}
	if (pMbss != NULL)
		ndev_ap_if = pMbss->wdev.if_dev;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_AUTO_CONNECT_SUPPORT

		if (pAd->ApCfg.ApCliAutoConnectChannelSwitching == FALSE)
			pAd->ApCfg.ApCliAutoConnectChannelSwitching = TRUE;

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (Success == TRUE) {
			if ((Channel > 14)
				&& (pAd->CommonCfg.bIEEE80211H == TRUE))
				pDot11h->org_ch = OriChannel;

			if ((Channel > 14)
				&& (pAd->CommonCfg.bIEEE80211H == TRUE)
			   ) {
					if (pMbss == NULL) {
						 /*AP Interface is not present and CLI wants to change channel*/
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("Only Change CLI Channel to %d!\n", wdev->channel));
						wlan_operate_set_prim_ch(wdev, wdev->channel);
#ifdef APCLI_AUTO_CONNECT_SUPPORT
						pAd->ApCfg.ApCliAutoConnectChannelSwitching = FALSE;
#endif
						return Success;
					}
				if ((pDot11h->RDMode == RD_SILENCE_MODE)
					|| ((ndev_ap_if != NULL) && (!RTMP_OS_NETDEV_STATE_RUNNING(ndev_ap_if)))) {
					pDot11h->RDMode = RD_SWITCHING_MODE;
#ifdef CONFIG_MAP_SUPPORT
					if (/*IS_MAP_ENABLE(pAd) &&*/ wdev->quick_ch_change == FALSE) {
#endif
						if (pMbss != NULL)
							APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
#ifdef MT_DFS_SUPPORT
						if (DfsStopWifiCheck(pAd)) {
							MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Stop AP Startup\n", __func__));
							return FALSE;
						}
#endif
					if (pMbss != NULL)
						APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
						if (IS_SUPPORT_DEDICATED_ZEROWAIT_DFS(pAd)) {
							DfsSetCacRemainingTime(pAd, wdev);
							DfsDedicatedScanStart(pAd);
						}
#endif

#ifdef CONFIG_MAP_SUPPORT
					} else {
						ap_phy_rrm_init_byRf(pAd, wdev);
					}
#endif
				} else {

					NotifyChSwAnnToPeerAPs(pAd, ZERO_MAC_ADDR, pAd->CurrentAddress, 1, Channel);
					pDot11h->RDMode = RD_SWITCHING_MODE;
					pDot11h->CSCount = 0;
					pDot11h->new_channel = Channel;

					if (HcUpdateCsaCntByChannel(pAd, OriChannel) != 0) {
						return Success;
					}
				}
			} else {
					wlan_operate_set_prim_ch(wdev, wdev->channel);
					APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
					APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	if (Success == TRUE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_Channel_Proc_by_Wdev::(Channel=%d)\n", Channel));
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd))
			wapp_send_ch_change_rsp(pAd, wdev, wdev->channel);
#endif
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		pAd->ApCfg.ApCliAutoConnectChannelSwitching = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
	return Success;
}

#ifdef CONFIG_MT7663_DPD_RE_CAL_SUPPORT
INT Set_DPDReCal_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Channel = (UCHAR) os_str_tol(arg, 0, 10);

	if (MT7663DPDRecal(pAd, Channel))
		return TRUE;
	else
		return FALSE;
}
#endif

#if defined(OFFCHANNEL_SCAN_FEATURE) || defined(TXRX_STAT_SUPPORT) || defined(CUSTOMER_RSG_FEATURE)
VOID ReadChannelStats(
	IN PRTMP_ADAPTER   pAd)
{
		UINT32	OBSSAirtime, MyTxAirtime, MyRxAirtime, CCABusyTime;
		UINT32 	PD_CNT, MDRDY_CNT, CCKFalseCCACount, OFDMFalseCCACount;
		UINT32	CrValue;
#ifdef TXRX_STAT_SUPPORT
		INT64 TxBeaconCount;
		struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
#endif
		UINT32	Time, TimeDiff;
		ULONG   TNow;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("[%d][%s]: Band 0\n", __LINE__, __func__));

		/* OBSS Air time */
		HW_IO_READ32(pAd->hdev_ctrl, RMAC_MIBTIME5, &CrValue);
		OBSSAirtime = CrValue;
		pAd->Ch_Stats[DBDC_BAND0].Obss_Time = OBSSAirtime;
		pAd->ChannelStats.MibUpdateOBSSAirtime[DBDC_BAND0] += CrValue;

		/* My Tx Air time */
		HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR36, &CrValue);
		MyTxAirtime = (CrValue & 0xffffff);
		pAd->Ch_Stats[DBDC_BAND0].Tx_Time = MyTxAirtime;
		pAd->ChannelStats.MibUpdateMyTxAirtime[DBDC_BAND0] += CrValue;

		/* My Rx Air time */
		HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR37, &CrValue);
		MyRxAirtime = (CrValue & 0xffffff);
		pAd->Ch_Stats[DBDC_BAND0].Rx_Time = MyRxAirtime;
		pAd->ChannelStats.MibUpdateMyRxAirtime[DBDC_BAND0] += CrValue;

		/* EDCCA time */
		RTMP_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR18, &CrValue);
		pAd->ChannelStats.MibUpdateEDCCAtime[DBDC_BAND0] += CrValue;
		CCABusyTime = (CrValue & 0xffffff);

		/* FALSE CCA Count */
		HW_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS0, &CrValue); /* PD count */
		PD_CNT = CrValue;
		pAd->ChannelStats.MibUpdatePdCount[DBDC_BAND0] += CrValue;
		HW_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS5, &CrValue); /* MDRDY count */
		MDRDY_CNT = CrValue;
		pAd->ChannelStats.MibUpdateMdrdyCount[DBDC_BAND0] += CrValue;
		CCKFalseCCACount = (PD_CNT & 0xffff) - (MDRDY_CNT & 0xffff);
		OFDMFalseCCACount = ((PD_CNT & 0xffff0000) >> 16) - ((MDRDY_CNT & 0xffff0000) >> 16);

		/* reset PD and MDRDY count */
		HW_IO_READ32(pAd->hdev_ctrl, PHY_BAND0_PHYMUX_5, &CrValue);
		CrValue &= 0xff8fffff;
		HW_IO_WRITE32(pAd->hdev_ctrl, PHY_BAND0_PHYMUX_5, CrValue); /* Reset */
		CrValue |= 0x500000;
		HW_IO_WRITE32(pAd->hdev_ctrl, PHY_BAND0_PHYMUX_5, CrValue); /* Enable */

		/* Ch Busy time band 0 */
		pAd->Ch_BusyTime[DBDC_BAND0] = OBSSAirtime + MyTxAirtime + MyRxAirtime;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("[%d][%s]: ChannelBusyTime  band 0 : %u\n",
					__LINE__, __func__, pAd->Ch_BusyTime[DBDC_BAND0]));

#ifdef TXRX_STAT_SUPPORT
		pAd->ChannelStats.Radio100msecCounts++;
		if ((pAd->ChannelStats.Radio100msecCounts % 10) == 0) {
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR0, &CrValue);
			TxBeaconCount = (CrValue & 0xffff);
			ctrl->rdev[DBDC_BAND0].pRadioCtrl->TxBeaconPacketCount.QuadPart += TxBeaconCount;

			if (pAd->CommonCfg.dbdc_mode) {
				HW_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR0, &CrValue);
				TxBeaconCount = (CrValue & 0xffff);
				ctrl->rdev[DBDC_BAND1].pRadioCtrl->TxBeaconPacketCount.QuadPart += TxBeaconCount;
			}
			pAd->ChannelStats.Radio100msecCounts = 10;
		}
#endif

#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			/* Band 1 */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("[%d][%s]: Band 1\n", __LINE__, __func__));
			/* OBSS Time */
			HW_IO_READ32(pAd->hdev_ctrl, RMAC_MIBTIME6, &CrValue);
			OBSSAirtime = CrValue;
			pAd->Ch_Stats[DBDC_BAND1].Obss_Time = OBSSAirtime;
			pAd->ChannelStats.MibUpdateOBSSAirtime[DBDC_BAND1] += CrValue;

			/* My Tx Air time */
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR36, &CrValue);
			MyTxAirtime = (CrValue & 0xffffff);
			pAd->Ch_Stats[DBDC_BAND1].Tx_Time = MyTxAirtime;
			pAd->ChannelStats.MibUpdateMyTxAirtime[DBDC_BAND1] += CrValue;

			/* My Rx Air time */
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR37, &CrValue);
			MyRxAirtime = (CrValue & 0xffffff);
			pAd->Ch_Stats[DBDC_BAND1].Rx_Time = MyRxAirtime;
			pAd->ChannelStats.MibUpdateMyRxAirtime[DBDC_BAND1] += CrValue;

			/* Ch Busy time band 1 */
			pAd->Ch_BusyTime[DBDC_BAND1] = OBSSAirtime + MyTxAirtime + MyRxAirtime;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("[%d][%s]: ChannelBusyTime  band 1 : %u\n",
						__LINE__, __func__, pAd->Ch_BusyTime[DBDC_BAND1]));
		}
#endif
			/* Reset OBSS Air time */
			HW_IO_READ32(pAd->hdev_ctrl, RMAC_MIBTIME0, &CrValue);
			CrValue |= 1 << RX_MIBTIME_CLR_OFFSET;
			CrValue |= 1 << RX_MIBTIME_EN_OFFSET;
			HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MIBTIME0, CrValue);

			NdisGetSystemUpTime(&TNow);
			Time = jiffies_to_usecs(TNow);

			TimeDiff = Time - pAd->ChannelStats.LastReadTime;
			pAd->ChannelStats.MeasurementDuration = TimeDiff;
			pAd->ChannelStats.TotalDuration += TimeDiff;
			pAd->ChannelStats.LastReadTime = Time;

			/*Ch Busy time*/
			pAd->ChannelStats.ChBusytime += OBSSAirtime + MyTxAirtime + MyRxAirtime;
			pAd->ChannelStats.ChBusyTime1secValue += OBSSAirtime + MyTxAirtime + MyRxAirtime;

			/*AP Activity time*/
			pAd->ChannelStats.ChannelApActivity += MyTxAirtime + MyRxAirtime;
			pAd->ChannelStats.ChannelApActivity1secValue += MyTxAirtime + MyRxAirtime;

			pAd->ChannelStats.CCABusytime += CCABusyTime;
			pAd->ChannelStats.CCABusyTime1secValue += CCABusyTime;

			pAd->ChannelStats.FalseCCACount1secValue += CCKFalseCCACount + OFDMFalseCCACount;
			pAd->ChannelStats.FalseCCACount += CCKFalseCCACount + OFDMFalseCCACount;
}
#endif



/*
    ==========================================================================
    Description:
	Set Short Slot Time Enable or Disable
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ShortSlot_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int retval;

	retval = RT_CfgSetShortSlot(pAd, arg);

	if (retval == TRUE)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_ShortSlot_Proc::(ShortSlot=%d)\n",
				 pAd->CommonCfg.bUseShortSlotTime));

	return retval;
}

/*
    ==========================================================================
    Description:
	Set Tx power
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_TxPower_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG    TxPower;
	INT     status = FALSE;
	UINT8   BandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BandIdx = %d\n", __func__, BandIdx));

	/* sanity check for Band index */
	if (BandIdx >= DBDC_BAND_NUM)
		return FALSE;

	TxPower = simple_strtol(arg, 0, 10);

	if (TxPower <= 100) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			pAd->CommonCfg.ucTxPowerPercentage[BandIdx] = TxPower;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			pAd->CommonCfg.ucTxPowerDefault[BandIdx] = TxPower;
			pAd->CommonCfg.ucTxPowerPercentage[BandIdx] = pAd->CommonCfg.ucTxPowerDefault[BandIdx];
		}
#endif /* CONFIG_STA_SUPPORT */
		status = TRUE;
	} else
		status = FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_TxPower_Proc: BandIdx: %d, (TxPowerPercentage=%d)\n",
			 BandIdx, pAd->CommonCfg.ucTxPowerPercentage[BandIdx]));
	return status;
}

/*
    ==========================================================================
    Description:
	Set 11B/11G Protection
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_BGProtection_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	switch (os_str_tol(arg, 0, 10)) {
	case 0: /*AUTO*/
		pAd->CommonCfg.UseBGProtection = 0;
		break;

	case 1: /*Always On*/
		pAd->CommonCfg.UseBGProtection = 1;
		break;

	case 2: /*Always OFF*/
		pAd->CommonCfg.UseBGProtection = 2;
		break;

	default:  /*Invalid argument */
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		ApUpdateCapabilityAndErpIe(pAd, &pAd->ApCfg.MBSSID[pObj->ioctl_if]);
	}
#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_BGProtection_Proc::(BGProtection=%ld)\n",
			 pAd->CommonCfg.UseBGProtection));
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set TxPreamble
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_TxPreamble_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RT_802_11_PREAMBLE	Preamble;

	Preamble = (RT_802_11_PREAMBLE)os_str_tol(arg, 0, 10);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)

	if (Preamble == Rt802_11PreambleAuto)
		return FALSE;

#endif /* CONFIG_AP_SUPPORT */

	switch (Preamble) {
	case Rt802_11PreambleShort:
		pAd->CommonCfg.TxPreamble = Preamble;
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		MlmeSetTxPreamble(pAd, Rt802_11PreambleShort);
#endif /* CONFIG_STA_SUPPORT */
		break;

	case Rt802_11PreambleLong:
#ifdef CONFIG_STA_SUPPORT
	case Rt802_11PreambleAuto:
		/*
			If user wants AUTO, initialize to LONG here, then change according to AP's
			capability upon association
		*/
#endif /* CONFIG_STA_SUPPORT */
		pAd->CommonCfg.TxPreamble = Preamble;
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		MlmeSetTxPreamble(pAd, Rt802_11PreambleLong);
#endif /* CONFIG_STA_SUPPORT */
		break;

	default: /*Invalid argument */
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_TxPreamble_Proc::(TxPreamble=%ld)\n",
			 pAd->CommonCfg.TxPreamble));
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set RTS Threshold
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
static VOID set_rts_len_thld(struct wifi_dev *wdev, UINT32 length)
{
	wlan_operate_set_rts_len_thld(wdev, length);
}

INT Set_RTSThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 length = MAX_RTS_THRESHOLD;

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Usage:\niwpriv raN set RTSThreshold=[length]\n"));
		return FALSE;
	}

	if (!wdev)
		return FALSE;

	length = os_str_tol(arg, 0, 10);
	set_rts_len_thld(wdev, length);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: set wdev%d rts length threshold=%d(0x%x)\n", __func__, wdev->wdev_idx, length, length));
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set Fragment Threshold
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_FragThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 frag_thld;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, obj->ioctl_if, obj->ioctl_if_type);

	if (!arg)
		return FALSE;

	if (!wdev)
		return FALSE;

	frag_thld = os_str_tol(arg, 0, 10);

	if (frag_thld > MAX_FRAG_THRESHOLD || frag_thld < MIN_FRAG_THRESHOLD)
		frag_thld = MAX_FRAG_THRESHOLD;
	else if ((frag_thld % 2) == 1)
		frag_thld -= 1;

	wlan_operate_set_frag_thld(wdev, frag_thld);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s::set wdev%d FragThreshold=%d)\n", __func__, wdev->wdev_idx, frag_thld));
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set TxBurst
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_TxBurst_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG TxBurst;

	TxBurst = os_str_tol(arg, 0, 10);

	if (TxBurst == 1)
		pAd->CommonCfg.bEnableTxBurst = TRUE;
	else if (TxBurst == 0)
		pAd->CommonCfg.bEnableTxBurst = FALSE;
	else
		return FALSE;  /*Invalid argument */

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_TxBurst_Proc::(TxBurst=%d)\n",
			 pAd->CommonCfg.bEnableTxBurst));
	return TRUE;
}
INT Set_MaxTxPwr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR IfIdx, BandIdx;
	UCHAR MaxTxPwr = 0;
	CHANNEL_CTRL *pChCtrl;

	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	IfIdx = pObj->ioctl_if;
#ifdef CONFIG_AP_SUPPORT
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN))
		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
#endif
#ifdef CONFIG_STA_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI)
		wdev = &pAd->StaCfg[IfIdx].wdev;
	else if (pObj->ioctl_if_type == INT_MSTA)
		wdev = &pAd->StaCfg[IfIdx].wdev;
#endif
	if (wdev == NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("[Set_MaxTxPwr_Proc]: pObj->ioctl_if_type = %d!!\n",
				pObj->ioctl_if_type));
			return FALSE;
		}

	MaxTxPwr = (UCHAR) simple_strtol(arg, 0, 10);

	if ((MaxTxPwr > 0) && (MaxTxPwr < 0xff)) {
		pAd->MaxTxPwr = MaxTxPwr;
		BandIdx = HcGetBandByWdev(wdev);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
		BuildChannelList(pAd, wdev);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Set MaxTxPwr = %d\n", MaxTxPwr));
		return TRUE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("ERROR: wrong power announced(MaxTxPwr=%d)\n", MaxTxPwr));
	return FALSE;

}

#ifdef RTMP_MAC_PCI
INT Set_ShowRF_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int ShowRF = os_str_tol(arg, 0, 10);

	if (ShowRF == 1)
		pAd->ShowRf = TRUE;
	else
		pAd->ShowRf = FALSE;

	return TRUE;
}
#endif /* RTMP_MAC_PCI */

#ifdef AGGREGATION_SUPPORT
/*
    ==========================================================================
    Description:
	Set TxBurst
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_PktAggregate_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG aggre;

	aggre = os_str_tol(arg, 0, 10);

	if (aggre == 1)
		pAd->CommonCfg.bAggregationCapable = TRUE;
	else if (aggre == 0)
		pAd->CommonCfg.bAggregationCapable = FALSE;
	else
		return FALSE;  /*Invalid argument */

#ifdef CONFIG_AP_SUPPORT
#ifdef PIGGYBACK_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		pAd->CommonCfg.bPiggyBackCapable = pAd->CommonCfg.bAggregationCapable;
		AsicSetPiggyBack(pAd, pAd->CommonCfg.bPiggyBackCapable);
	}
#endif /* PIGGYBACK_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_PktAggregate_Proc::(AGGRE=%d)\n",
			 pAd->CommonCfg.bAggregationCapable));
	return TRUE;
}
#endif

/*
    ==========================================================================
    Description:
	Set IEEE80211H.
	This parameter is 1 when needs radar detection, otherwise 0
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_IEEE80211H_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG ieee80211h;

	ieee80211h = os_str_tol(arg, 0, 10);

	if (ieee80211h == 1)
		pAd->CommonCfg.bIEEE80211H = TRUE;
	else if (ieee80211h == 0)
		pAd->CommonCfg.bIEEE80211H = FALSE;
	else
		return FALSE;  /*Invalid argument */

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_IEEE80211H_Proc::(IEEE80211H=%d)\n",
			 pAd->CommonCfg.bIEEE80211H));
	return TRUE;
}

#ifdef EXT_BUILD_CHANNEL_LIST
/*
    ==========================================================================
    Description:
	Set Country Code.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_ExtCountryCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) == NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s can only be used when interface is down.\n", __func__));
		return TRUE;
	}

	if (strlen(arg) == 2) {
		NdisMoveMemory(pAd->CommonCfg.CountryCode, arg, 2);
		pAd->CommonCfg.bCountryFlag = TRUE;
	} else {
		NdisZeroMemory(pAd->CommonCfg.CountryCode, sizeof(pAd->CommonCfg.CountryCode));
		pAd->CommonCfg.bCountryFlag = FALSE;
	}

	{
		UCHAR CountryCode[3] = {0};

		NdisMoveMemory(CountryCode, pAd->CommonCfg.CountryCode, 2);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_CountryCode_Proc::(bCountryFlag=%d, CountryCode=%s)\n",
				 pAd->CommonCfg.bCountryFlag,
				 CountryCode));
	}

	return TRUE;
}
/*
    ==========================================================================
    Description:
	Set Ext DFS Type
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_ExtDfsType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR	 *pDfsType = &pAd->CommonCfg.DfsType;

	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) == NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s can only be used when interface is down.\n", __func__));
		return TRUE;
	}

	if (!strcmp(arg, "CE"))
		*pDfsType = CE;
	else if (!strcmp(arg, "FCC"))
		*pDfsType = FCC;
	else if (!strcmp(arg, "JAP"))
		*pDfsType = JAP;
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Unsupported DFS type:%s (Legal types are: CE, FCC, JAP)\n",
				 arg));

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Add new channel list
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_ChannelListAdd_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CH_DESP		inChDesp;
	PCH_REGION pChRegion = NULL;

	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) == NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s can only be used when interface is down.\n", __func__));
		return TRUE;
	}

	/* Get Channel Region (CountryCode)*/
	{
		INT loop = 0;

		while (strcmp((RTMP_STRING *) ChRegion[loop].CountReg, "") != 0) {
			if (strncmp((RTMP_STRING *) ChRegion[loop].CountReg, pAd->CommonCfg.CountryCode, 2) == 0) {
				pChRegion = &ChRegion[loop];
				break;
			}

			loop++;
		}

		if (pChRegion == NULL) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CountryCode is not configured or not valid\n"));
			return TRUE;
		}
	}
	/* Parsing the arg, IN:arg; OUT:inChRegion */
	{
		UCHAR strBuff[64], count = 0;
		PUCHAR	pStart, pEnd, tempIdx, tempBuff[5];

		if (strlen(arg) < 64)
			NdisCopyMemory(strBuff, arg, strlen(arg));

		pStart = rtstrchr(strBuff, '[');

		if (pStart != NULL) {
			pEnd = rtstrchr(pStart++, ']');

			if (pEnd != NULL) {
				tempBuff[count++] = pStart;

				for (tempIdx = pStart; tempIdx != pEnd; tempIdx++) {
					if (*tempIdx == ',') {
						*tempIdx = '\0';
						tempBuff[count++] = ++tempIdx;
					}
				}

				*(pEnd) = '\0';

				if (count != 5) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Input Error. Too more or too less parameters.\n"));
					return TRUE;
				} else {
					inChDesp.FirstChannel = (UCHAR) os_str_tol(tempBuff[0], 0, 10);
					inChDesp.NumOfCh = (UCHAR) os_str_tol(tempBuff[1], 0, 10);
					inChDesp.MaxTxPwr = (UCHAR) os_str_tol(tempBuff[2], 0, 10);
					inChDesp.Geography = (!strcmp(tempBuff[3], "BOTH") ? BOTH : (!strcmp(tempBuff[3], "IDOR") ? IDOR : ODOR));
					inChDesp.DfsReq = (!strcmp(tempBuff[4], "TRUE") ? TRUE : FALSE);
				}
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Missing End \"]\"\n"));
				return TRUE;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Invalid input format.\n", __func__));
			return TRUE;
		}
	}
	/* Add entry to Channel List*/
	{
		UCHAR EntryIdx;
		PCH_DESP pChDesp = NULL;
		UCHAR CountryCode[3] = {0};

		if (pAd->CommonCfg.pChDesp == NULL) {
			os_alloc_mem(pAd,  &pAd->CommonCfg.pChDesp, MAX_PRECONFIG_DESP_ENTRY_SIZE * sizeof(CH_DESP));
			pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;

			if (pChDesp) {
				for (EntryIdx = 0; pChRegion->pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++) {
					if (EntryIdx == (MAX_PRECONFIG_DESP_ENTRY_SIZE - 2)) { /* Keep an NULL entry in the end of table*/
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Table is full.\n"));
						return TRUE;
					}

					NdisCopyMemory(&pChDesp[EntryIdx], &pChRegion->pChDesp[EntryIdx], sizeof(CH_DESP));
				}

				/* Copy the NULL entry*/
				NdisCopyMemory(&pChDesp[EntryIdx], &pChRegion->pChDesp[EntryIdx], sizeof(CH_DESP));
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("os_alloc_mem failded.\n"));
				return FALSE;
			}
		} else {
			pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;

			for (EntryIdx = 0; pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++) {
				if (EntryIdx ==  (MAX_PRECONFIG_DESP_ENTRY_SIZE - 2)) { /* Keep an NULL entry in the end of table*/
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Table is full.\n"));
					return TRUE;
				}
			}
		}

		NdisMoveMemory(CountryCode, pAd->CommonCfg.CountryCode, 2);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Add channel lists {%u, %u, %u, %s, %s} to %s.\n",
				 inChDesp.FirstChannel,
				 inChDesp.NumOfCh,
				 inChDesp.MaxTxPwr,
				 (inChDesp.Geography == BOTH) ? "BOTH" : (inChDesp.Geography == IDOR) ?  "IDOR" : "ODOR",
				 (inChDesp.DfsReq == TRUE) ? "TRUE" : "FALSE",
				 CountryCode));
		NdisCopyMemory(&pChDesp[EntryIdx], &inChDesp, sizeof(CH_DESP));
		pChDesp[++EntryIdx].FirstChannel = 0;
	}
	return TRUE;
}

INT Set_ChannelListShow_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCH_REGION	pChRegion = NULL;
	UCHAR		EntryIdx, CountryCode[3] = {0};
	/* Get Channel Region (CountryCode)*/
	{
		INT loop = 0;

		while (strcmp((RTMP_STRING *) ChRegion[loop].CountReg, "") != 0) {
			if (strncmp((RTMP_STRING *) ChRegion[loop].CountReg, pAd->CommonCfg.CountryCode, 2) == 0) {
				pChRegion = &ChRegion[loop];
				break;
			}

			loop++;
		}

		if (pChRegion == NULL) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CountryCode is not configured or not valid\n"));
			return TRUE;
		}
	}
	NdisMoveMemory(CountryCode, pAd->CommonCfg.CountryCode, 2);

	if (pAd->CommonCfg.DfsType == MAX_RD_REGION)
		pAd->CommonCfg.DfsType = pChRegion->op_class_region;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("=========================================\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CountryCode:%s\n", CountryCode));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("DfsType:%s\n",
			 (pAd->CommonCfg.DfsType == JAP) ? "JAP" :
			 ((pAd->CommonCfg.DfsType == FCC) ? "FCC" : "CE")));

	if (pAd->CommonCfg.pChDesp != NULL) {
		PCH_DESP pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;

		for (EntryIdx = 0; pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%u. {%3u, %2u, %2u, %s, %5s}.\n",
					 EntryIdx,
					 pChDesp[EntryIdx].FirstChannel,
					 pChDesp[EntryIdx].NumOfCh,
					 pChDesp[EntryIdx].MaxTxPwr,
					 (pChDesp[EntryIdx].Geography == BOTH) ? "BOTH" : (pChDesp[EntryIdx].Geography == IDOR) ?  "IDOR" : "ODOR",
					 (pChDesp[EntryIdx].DfsReq == TRUE) ? "TRUE" : "FALSE"));
		}
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Default channel list table:\n"));

		for (EntryIdx = 0; pChRegion->pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%u. {%3u, %2u, %2u, %s, %5s}.\n",
					 EntryIdx,
					 pChRegion->pChDesp[EntryIdx].FirstChannel,
					 pChRegion->pChDesp[EntryIdx].NumOfCh,
					 pChRegion->pChDesp[EntryIdx].MaxTxPwr,
					 (pChRegion->pChDesp[EntryIdx].Geography == BOTH) ? "BOTH" : (pChRegion->pChDesp[EntryIdx].Geography == IDOR) ?  "IDOR" :
					 "ODOR",
					 (pChRegion->pChDesp[EntryIdx].DfsReq == TRUE) ? "TRUE" : "FALSE"));
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("=========================================\n"));
	return TRUE;
}

INT Set_ChannelListDel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR EntryIdx, TargetIdx, NumOfEntry;
	PCH_REGION	pChRegion = NULL;
	PCH_DESP pChDesp = NULL;

	TargetIdx = os_str_tol(arg, 0, 10);

	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) == NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s can only be used when interface is down.\n", __func__));
		return TRUE;
	}

	/* Get Channel Region (CountryCode)*/
	{
		INT loop = 0;

		while (strcmp((RTMP_STRING *) ChRegion[loop].CountReg, "") != 0) {
			if (strncmp((RTMP_STRING *) ChRegion[loop].CountReg, pAd->CommonCfg.CountryCode, 2) == 0) {
				pChRegion = &ChRegion[loop];
				break;
			}

			loop++;
		}

		if (pChRegion == NULL) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CountryCode is not configured or not valid\n"));
			return TRUE;
		}
	}

	if (pAd->CommonCfg.pChDesp == NULL) {
		os_alloc_mem(pAd,  &pAd->CommonCfg.pChDesp, MAX_PRECONFIG_DESP_ENTRY_SIZE * sizeof(CH_DESP));

		if (pAd->CommonCfg.pChDesp) {
			pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;

			for (EntryIdx = 0; pChRegion->pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++) {
				if (EntryIdx == (MAX_PRECONFIG_DESP_ENTRY_SIZE - 2)) { /* Keep an NULL entry in the end of table*/
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Table is full.\n"));
					return TRUE;
				}

				NdisCopyMemory(&pChDesp[EntryIdx], &pChRegion->pChDesp[EntryIdx], sizeof(CH_DESP));
			}

			/* Copy the NULL entry*/
			NdisCopyMemory(&pChDesp[EntryIdx], &pChRegion->pChDesp[EntryIdx], sizeof(CH_DESP));
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("os_alloc_mem failded.\n"));
			return FALSE;
		}
	} else
		pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;

	if (!strcmp(arg, "default")) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Default table used.\n"));

		if (pAd->CommonCfg.pChDesp != NULL)
			os_free_mem(pAd->CommonCfg.pChDesp);

		pAd->CommonCfg.pChDesp = NULL;
		pAd->CommonCfg.DfsType = MAX_RD_REGION;
	} else if (!strcmp(arg, "all")) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Remove all entries.\n"));

		for (EntryIdx = 0; EntryIdx < MAX_PRECONFIG_DESP_ENTRY_SIZE; EntryIdx++)
			NdisZeroMemory(&pChDesp[EntryIdx], sizeof(CH_DESP));
	} else if (TargetIdx < (MAX_PRECONFIG_DESP_ENTRY_SIZE - 1)) {
		for (EntryIdx = 0; pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++) {
			if (EntryIdx ==  (MAX_PRECONFIG_DESP_ENTRY_SIZE - 2)) { /* Keep an NULL entry in the end of table */
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Last entry should be NULL.\n"));
				pChDesp[EntryIdx].FirstChannel = 0;
				return TRUE;
			}
		}

		NumOfEntry = EntryIdx;

		if (TargetIdx >= NumOfEntry) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Out of table range.\n"));
			return TRUE;
		}

		for (EntryIdx = TargetIdx; EntryIdx < NumOfEntry; EntryIdx++)
			NdisCopyMemory(&pChDesp[EntryIdx], &pChDesp[EntryIdx + 1], sizeof(CH_DESP));

		NdisZeroMemory(&pChDesp[EntryIdx], sizeof(CH_DESP)); /*NULL entry*/
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Entry %u deleted.\n", TargetIdx));
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Entry not found.\n"));

	return TRUE;
}
#endif /* EXT_BUILD_CHANNEL_LIST  */

#ifdef WSC_INCLUDED
INT	Set_WscGenPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL   pWscControl = NULL;
	POS_COOKIE  pObj;
	UCHAR       apidx;

	if (pAd == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pAd == NULL!\n", __func__));
		return TRUE;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

		if (pObj->ioctl_if_type == INT_APCLI) {
			pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("IF(apcli%d) Set_WscGenPinCode_Proc:: This command is from apcli interface now.\n", apidx));
		} else
#endif /* APCLI_SUPPORT */
		{
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("IF(ra%d) Set_WscGenPinCode_Proc:: This command is from ra interface now.\n", apidx));
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		{
			pWscControl = &pAd->StaCfg[pObj->ioctl_if].wdev.WscControl;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	if (pWscControl == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pWscControl == NULL!\n", __func__));
		return TRUE;
	}

	if (pWscControl->WscEnrollee4digitPinCode) {
		pWscControl->WscEnrolleePinCodeLen = 4;
		pWscControl->WscEnrolleePinCode = WscRandomGen4digitPinCode(pAd);
	} else {
		pWscControl->WscEnrolleePinCodeLen = 8;
#ifdef P2P_SSUPPORT
#endif /* P2P_SUPPORT */
			pWscControl->WscEnrolleePinCode = WscRandomGeneratePinCode(pAd, apidx);
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscGenPinCode_Proc:: Enrollee PinCode\t\t%08u\n",
			 pWscControl->WscEnrolleePinCode));
	return TRUE;
}

#ifdef BB_SOC
INT	Set_WscResetPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL   pWscControl = NULL;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		{
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("IF(ra%d) Set_WscResetPinCode_Proc:: This command is from ra interface now.\n", apidx));
		}
		pWscControl->WscEnrolleePinCode = GenerateWpsPinCode(pAd, 0, apidx);
	}
#endif /* CONFIG_AP_SUPPORT // */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscResetPinCode_Proc:: Enrollee PinCode\t\t%08u\n",
			 pWscControl->WscEnrolleePinCode));
	return TRUE;
}
#endif

INT Set_WscVendorPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL   pWscControl = NULL;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT
	UCHAR       apidx = pObj->ioctl_if;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

		if (pObj->ioctl_if_type == INT_APCLI) {
			pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscVendorPinCode_Proc() for apcli(%d)\n", apidx));
		} else
#endif /* APCLI_SUPPORT */
		{
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscVendorPinCode_Proc() for ra%d!\n", apidx));
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			pWscControl = &pAd->StaCfg[pObj->ioctl_if].wdev.WscControl;
	}
#endif /* CONFIG_STA_SUPPORT */

	if (!pWscControl)
		return FALSE;
	else
		return RT_CfgSetWscPinCode(pAd, arg, pWscControl);
}
#endif /* WSC_INCLUDED */

#ifdef DBG
INT rx_temp_dbg;

static PCHAR cat_str[32] = {
	"MISC",
	"INIT",
	"HW",
	"FW",
	"HIF",
	"FPGA",
	"TEST",
	"RA",
	"AP",
	"CLIENT",
	"TX",
	"RX",
	"CFG",
	"MLME",
	"PROTO",
	"SEC",
	"PS",
	"POWER",
	"COEX",
	"P2P",
	"TOKEN",
	"CMW",
};

static PCHAR sub_cat_str[32][32] = {
	{"MISC"}, /* misc */
	{"MISC"}, /* initialization/shutdown */
	{"MISC", "SA"}, /* MAC/BBP/RF/Chip */
	{"MISC"}, /* FW related command, response, CR that FW care about */
	{"MISC", "PCI", "USB", "SDIO"}, /* Host interface: usb/sdio/pcie/rbus */
	{"MISC"}, /* FPGA Chip verify, DVT */
	{"MISC"}, /* ATE, QA, UT, FPGA?, TDT, SLT, WHQL, and other TEST */
	{"MISC"}, /* Rate Adaption/Throughput related */
	{"MISC", "MBSS", "WDS"}, /* AP, MBSS, WDS */
	{"MISC", "ADHOC", "APCLI", "MESH"}, /* STA, ApClient, AdHoc, Mesh */
	{"MISC", "TMAC"}, /* Tx data path */
	{"MISC"}, /* Rx data path */
	{"MISC"}, /* ioctl/oid/profile/cfg80211/Registry */
	{"MISC"}, /* 802.11 fundamental connection flow, auth, assoc, disconnect, etc */
	{"MISC", "ACM", "BA", "TDLS", "WNM", "IGMP", "MAT", "RRM", "DFS", "FT", "SCAN", "FTM"}, /* protocol, ex. TDLS */
	{"MISC", "EY", "WPS", "WAPI", "PMF", "SAE", "SUITEB", "OWE"}, /* security/key/WPS/WAPI/PMF/11i related*/
	{"MISC", "UAPSD"}, /* power saving/UAPSD */
	{"MISC"}, /* power Setting, Single Sku, Temperature comp, etc */
	{"MISC"}, /* BT, BT WiFi Coex, LTE, TVWS*/
	{"MISC"}, /* P2P, Miracast */
	{"MISC", "INFO", "PROFILE", "TRACE"}, /* token */
	{"MISC"}, /* CMW Link Test related */
};

/*
    ==========================================================================
    Description:
	For Debug information
	Change DebugLevel
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_Debug_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 dbg_lvl = 0;
	UINT32 dbg_cat = 0;
	UINT32 dbg_sub_cat = 0;
	UINT32 i;
	UINT32 j;
	RTMP_STRING *str  = NULL;
	RTMP_STRING *str2  = NULL;
	UCHAR para = 3;
	UCHAR show_usage_para = 0;

	if (arg == NULL || strlen(arg) == 0)
		goto format_error;

	str = strsep(&arg, ":");

	if (arg == NULL) {
		para = 1;
		if (rtstrcasecmp(str, "?") == TRUE)
			show_usage_para = 1;
	} else {
		str2 = strsep(&arg, ":");
		if (arg == NULL) {
			para = 2;
			if (rtstrcasecmp(str2, "?") == TRUE)
				show_usage_para = 2;
		} else if (rtstrcasecmp(arg, "?") == TRUE)
			show_usage_para = 3;
	}

	dbg_lvl = os_str_tol(str, 0, 10);
	if (para >= 2)
		dbg_cat = os_str_tol(str2, 0, 10);
	if (para >= 3)
		dbg_sub_cat = os_str_tol(arg, 0, 10);

	if (show_usage_para == 1) {
		MTWF_PRINT("usage and current state:\n");
		for (j = 0; j < 32; j++) {
			if (cat_str[j] == NULL)
				break;
			for (i = DBG_LVL_MAX; i != 0; i--)
				if (DebugSubCategory[i][j] != 0) {
					MTWF_PRINT("%2d:%s(L%d", j, cat_str[j], i);
					if (DebugSubCategory[i][j] != DBG_SUBCAT_EN_ALL_MASK)
						MTWF_PRINT("*");
					MTWF_PRINT(")\t");
					break;
				}
			if ((j + 1) % 4 == 0)
				MTWF_PRINT("\n");
		}

		MTWF_PRINT("\n");
		return TRUE;
	} else if (show_usage_para == 2) {
		MTWF_PRINT("usage and current state for DebugLevel %d:\n", dbg_lvl);
		for (j = 0; j < 32; j++) {
			if (cat_str[j] == NULL)
				break;
			MTWF_PRINT("%2d:%s(0x%08x)\t", j, cat_str[j], DebugSubCategory[dbg_lvl][j]);
			if ((j + 1) % 4 == 0)
				MTWF_PRINT("\n");
		}

		MTWF_PRINT("\n");
		return TRUE;
	} else if (show_usage_para == 3) {
		MTWF_PRINT("usage and current state for DebugLevel %d, Category %d(%s):\n", dbg_lvl, dbg_cat, cat_str[dbg_cat]);
		for (j = 0; j < 32; j++) {
			if (sub_cat_str[dbg_cat][j] == NULL)
				break;
			MTWF_PRINT("%2d:%s(", j, sub_cat_str[dbg_cat][j]);
			if (DebugSubCategory[dbg_lvl][dbg_cat] & (0x1 << j))
				MTWF_PRINT("on)\t");
			else
				MTWF_PRINT("off)\t");
			if ((j + 1) % 4 == 0)
				MTWF_PRINT("\n");
		}

		MTWF_PRINT("\n");
		return TRUE;
	}

	if (dbg_lvl <= DBG_LVL_MAX) {
		if (para == 1) {
			DebugLevel = dbg_lvl;
			for (i = 0; i <= DBG_LVL_MAX; i++) {
				UINT32 tmp = (i <= dbg_lvl) ? DBG_SUBCAT_EN_ALL_MASK : 0;

				for (j = 0; j < 32; j++)
					DebugSubCategory[i][j] = tmp;
			}
		} else if (para == 2) {
			if (dbg_cat > DBG_CAT_MAX)
				goto format_error;
			for (i = 0; i <= DBG_LVL_MAX; i++) {
				UINT32 tmp = (i <= dbg_lvl) ? DBG_SUBCAT_EN_ALL_MASK : 0;

				MTWF_PRINT("%s(): change cat%d(level %d) from %x to ", __func__,
					dbg_cat, i, DebugSubCategory[i][dbg_cat]);
				DebugSubCategory[i][dbg_cat] = tmp;
				MTWF_PRINT("%x\n", DebugSubCategory[i][dbg_cat]);
			}
		} else if (para == 3) {
			if (dbg_sub_cat > 31)
				goto format_error;
			for (i = 0; i <= DBG_LVL_MAX; i++) {
				MTWF_PRINT("%s(): change cat%d(level %d) from %x to ", __func__,
					dbg_cat, i, DebugSubCategory[i][dbg_cat]);
				if (i <= dbg_lvl)
					DebugSubCategory[i][dbg_cat] |= (0x1 << dbg_sub_cat);
				else
					DebugSubCategory[i][dbg_cat] &= ~(0x1 << dbg_sub_cat);
				MTWF_PRINT("%x\n", DebugSubCategory[i][dbg_cat]);
			}
		}
	} else
		goto format_error;

	MTWF_PRINT("%s(): (DebugLevel = %d)\n", __func__, DebugLevel);
	return TRUE;

format_error:
	MTWF_PRINT("Format error! correct format:\n");
	MTWF_PRINT("iwpriv ra0 set Debug=[DebugLevel]:[DebugCat]:[DebugSubCat]\n");
	MTWF_PRINT("\t[DebugLevel]:0~6 or ?\n");
	MTWF_PRINT("\t[DebugCat]:0~31 or ?, optional\n");
	MTWF_PRINT("\t[DebugSubCat]:0~31 or ?, optional\n");
	MTWF_PRINT("EX: 1.iwpriv ra0 set Debug=2\n");
	MTWF_PRINT("\t DebugSubCategory[0~2][0~31] = 0xffffffff, DebugSubCategory[3~6][0~31] = 0\n");
	MTWF_PRINT("    2.iwpriv ra0 set Debug=4:5\n");
	MTWF_PRINT("\t DebugSubCategory[0~4][5] = 0xffffffff, DebugSubCategory[5~6][5] = 0\n");
	MTWF_PRINT("    3.iwpriv ra0 set Debug=3:10:7\n");
	MTWF_PRINT("\t DebugSubCategory[0~3][10] |= (0x1 << 7), DebugSubCategory[4~6][10] &= ~(0x1 << 7)\n");
	MTWF_PRINT("    4.iwpriv ra0 set Debug=?\n");
	MTWF_PRINT("\t query category list and current debuglevel value for each category\n");
	MTWF_PRINT("    5.iwpriv ra0 set Debug=3:?\n");
	MTWF_PRINT("\t query category list and current subcategory bitmap value for each category at DebugLevel 3\n");
	MTWF_PRINT("    6.iwpriv ra0 set Debug=2:8:?\n");
	MTWF_PRINT("\t query subcategory list and current subcategory on/off state for category 8 at DebugLevel 2\n");

	return FALSE;
}

/*
    ==========================================================================
    Description:
	Change DebugCategory
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_DebugCategory_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 category = (UINT32)os_str_tol(arg, 0, 16);

	DebugCategory = category;
	MTWF_PRINT("%s(): Set DebugCategory = 0x%x\n", __func__, DebugCategory);
	return TRUE;
}

static BOOLEAN ascii2hex(RTMP_STRING *in, UINT32 *out)
{
	UINT32 hex_val, val;
	CHAR *p, asc_val;

	hex_val = 0;
	p = (char *)in;

	while ((*p) != 0) {
		val = 0;
		asc_val = *p;

		if ((asc_val >= 'a') && (asc_val <= 'f'))
			val = asc_val - 87;
		else if ((*p >= 'A') && (asc_val <= 'F'))
			val = asc_val - 55;
		else if ((asc_val >= '0') && (asc_val <= '9'))
			val = asc_val - 48;
		else
			return FALSE;

		hex_val = (hex_val << 4) + val;
		p++;
	}

	*out = hex_val;
	return TRUE;
}

#if defined(CONFIG_ARCH_MT7623) || defined(CONFIG_ARCH_MT8590) || defined(CONFIG_ARCH_MT7622)
BOOLEAN mt_mac_cr_range_mapping(RTMP_ADAPTER *pAd, UINT32 *mac_addr);
#endif

/*
    ==========================================================================
    Description:
	Read / Write MAC
    Arguments:
	pAd                    Pointer to our adapter
	wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 mac 0        ==> read MAC where Addr=0x0
	       2.) iwpriv ra0 mac 0=12     ==> write MAC where Addr=0x0, value=12
    ==========================================================================
*/
VOID RTMPIoctlMAC(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	RTMP_STRING *seg_str, *addr_str, *val_str, *range_str;
	RTMP_STRING *mpool, *msg;
	RTMP_STRING *arg, *ptr;
	UINT32 macVal = 0;
	UINT32 macValue;
	BOOLEAN bIsPrintAllMAC = FALSE, bFromUI, is_write, is_range;
	UINT32 IdMac, mac_s = 0x1000, mac_e = 0x1700, mac_range = 0xffff;
	BOOLEAN Ret;
#ifdef RTMP_MAC_PCI
	BOOLEAN IsMapAddrNeed = FALSE;
#endif
	os_alloc_mem(NULL, (UCHAR **)&mpool, sizeof(CHAR) * (4096 + 256 + 12));

	if (!mpool)
		return;

#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		mac_range = 0xfffff;

#endif /* MT_MAC */
	bFromUI = ((wrq->u.data.flags & RTPRIV_IOCTL_FLAG_UI) == RTPRIV_IOCTL_FLAG_UI) ? TRUE : FALSE;
	msg = (RTMP_STRING *)((ULONG)(mpool + 3) & (ULONG)~0x03);
	arg = (RTMP_STRING *)((ULONG)(msg + 4096 + 3) & (ULONG)~0x03);
	memset(msg, 0x00, 4096);
	memset(arg, 0x00, 256);

	if (wrq->u.data.length > 1) {
#ifdef LINUX
		INT Status = NDIS_STATUS_SUCCESS;

		Status = copy_from_user(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
#else
		NdisMoveMemory(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
#endif /* LINUX */
		arg[255] = 0x00;
	}

	ptr = arg;

	if ((ptr != NULL) && (strlen(ptr) > 0)) {
		while ((*ptr != 0) && (*ptr == 0x20)) /* remove space */
			ptr++;

	/* The below code tries to access user space buffer directly,
	 * hence remove it .
	 */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():after trim space, ptr len=%zu, pointer(%p)=%s!\n",
				 __func__, strlen(ptr), ptr, ptr));
	}

	if ((ptr == NULL) || strlen(ptr) == 0) {
		bIsPrintAllMAC = TRUE;
		goto print_all;
	}

	{
		while ((seg_str = strsep((char **)&ptr, ",")) != NULL) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("seg_str[%zu]=%s\n", strlen(seg_str), seg_str));
			is_write = FALSE;
			addr_str = seg_str;
			val_str = NULL;
			val_str = strchr(seg_str, '=');

			if (val_str != NULL) {
				*val_str++ = 0;
				is_write = 1;
			} else
				is_write = 0;

			if (addr_str) {
				range_str = strchr(addr_str, '-');

				if (range_str != NULL) {
					*range_str++ = 0;
					is_range = 1;
				} else
					is_range = 0;

				if ((ascii2hex(addr_str, &mac_s) == FALSE)) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid MAC CR Addr, str=%s\n", addr_str));
					break;
				}

#ifdef MT_MAC

				if (IS_HIF_TYPE(pAd, HIF_MT)) {
					Ret = mt_mac_cr_range_mapping(pAd, &mac_s);
#ifdef RTMP_MAC_PCI
					IsMapAddrNeed = (Ret) ? FALSE : TRUE;
#endif
				}

#endif /* MT_MAC */

				/* check range only if it doesn't need remap */
#ifdef RTMP_MAC_PCI
				if (!IsMapAddrNeed)
#endif
					if (mac_s >= mac_range) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MAC CR Addr[0x%x] out of range[0x%x], str=%s\n",
								 mac_s, mac_range, addr_str));
						break;
					}

				if (is_range) {
					if (ascii2hex(range_str, &mac_e) == FALSE) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid Range End MAC CR Addr[0x%x], str=%s\n",
								 mac_e, range_str));
						break;
					}

#ifdef MT_MAC

					if (IS_HIF_TYPE(pAd, HIF_MT)) {
						Ret = mt_mac_cr_range_mapping(pAd, &mac_e);
#ifdef RTMP_MAC_PCI
						IsMapAddrNeed = (Ret) ? FALSE : TRUE;
#endif
					}

#endif /* MT_MAC */

					/* check range only if it doesn't need remap */
#ifdef RTMP_MAC_PCI
					if (!IsMapAddrNeed)
#endif
						if (mac_e >= mac_range) {
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MAC CR Addr[0x%x] out of range[0x%x], str=%s\n",
									 mac_e, mac_range, range_str));
							break;
						}

					if (mac_e < mac_s) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid Range MAC Addr[%s - %s] => [0x%x - 0x%x]\n",
								 addr_str, range_str, mac_s, mac_e));
						break;
					}
				} else
					mac_e = mac_s;
			}

			if (val_str) {
				if ((strlen(val_str) == 0) || ascii2hex(val_str, &macVal) == FALSE) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid MAC value[0x%s]\n", val_str));
					break;
				}
			}

			if (is_write) {
#ifdef RTMP_MAC_PCI

				if (IsMapAddrNeed && ((mac_s < MT_PCI_REMAP_ADDR_1) || (mac_s > MT_PCI_REMAP_ADDR_2 + REMAP_2_OFFSET_MASK))) {
					UINT32 RemapBase, RemapOffset;
					UINT32 RestoreValue;

#if  defined(MT7626) || defined(MT7663)
					RTMP_IO_READ32(pAd->hdev_ctrl, 0x700c, &RestoreValue);
					RemapBase = mac_s & 0xffff0000;
					RemapOffset = mac_s & 0xffff;
					RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x700c, RemapBase);
					RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x90000 + RemapOffset, macVal);
					RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x700c, RestoreValue);
#else
					RTMP_IO_READ32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, &RestoreValue);
					RemapBase = GET_REMAP_2_BASE(mac_s) << 19;
					RemapOffset = GET_REMAP_2_OFFSET(mac_s);
					RTMP_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, RemapBase);
					RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x80000 + RemapOffset, macVal);
					RTMP_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, RestoreValue);
#endif
				} else
					RTMP_IO_WRITE32(pAd->hdev_ctrl, mac_s, macVal);

#else
				RTMP_IO_WRITE32(pAd->hdev_ctrl, mac_s, macVal);
#endif
				sprintf(msg + strlen(msg), "[0x%04x]:%08x  ", mac_s, macVal);

				if (!bFromUI)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("MacAddr=0x%x, MacValue=0x%x\n", mac_s, macVal));
			} else {
				for (IdMac = mac_s; IdMac <= mac_e; IdMac += 4) {
#ifdef RTMP_MAC_PCI

					if (IsMapAddrNeed && ((IdMac < MT_PCI_REMAP_ADDR_1) || (IdMac > MT_PCI_REMAP_ADDR_2 + REMAP_2_OFFSET_MASK))) {
						UINT32 RemapBase, RemapOffset;
						UINT32 RestoreValue;

#if  defined(MT7626) || defined(MT7663)
						RTMP_IO_READ32(pAd->hdev_ctrl, 0x700c, &RestoreValue);
						RemapBase = IdMac & 0xffff0000;
						RemapOffset = IdMac & 0xffff;
						RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x700c, RemapBase);
						RTMP_IO_READ32(pAd->hdev_ctrl, 0x90000 + RemapOffset, &macVal);
						RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x700c, RestoreValue);
#else
						RTMP_IO_READ32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, &RestoreValue);
						RemapBase = GET_REMAP_2_BASE(IdMac) << 19;
						RemapOffset = GET_REMAP_2_OFFSET(IdMac);
						RTMP_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, RemapBase);
						RTMP_IO_READ32(pAd->hdev_ctrl, 0x80000 + RemapOffset, &macVal);
						RTMP_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, RestoreValue);
#endif
					} else
						RTMP_IO_READ32(pAd->hdev_ctrl, IdMac, &macVal);

#else
					RTMP_IO_READ32(pAd->hdev_ctrl, IdMac, &macVal);
#endif
					sprintf(msg + strlen(msg), "[0x%04x]:%08x  ", IdMac, macVal);

					if (!bFromUI)
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MacAddr=0x%x, MacValue=0x%x\n", IdMac, macVal));
				}
			}


			if (ptr)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("NextRound: ptr[%zu]=%s\n", strlen(ptr), ptr));
		}
	}

print_all:

	if (bIsPrintAllMAC) {
		mac_s = 0x1000;
		mac_e = 0x1700;
		{
			for (IdMac = mac_s; IdMac < mac_e; IdMac += 4) {
				RTMP_IO_READ32(pAd->hdev_ctrl, IdMac, &macValue);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%08x = %08x\n", IdMac, macValue));
			}
		}
	}

	if (strlen(msg) == 1)
		sprintf(msg + strlen(msg), "===>Error command format!");

#ifdef LINUX
	/* Copy the information into the user buffer */
	wrq->u.data.length = strlen(msg);

	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: copy_to_user() fail\n", __func__));

#endif /* LINUX */
	os_free_mem(mpool);

	if (!bFromUI)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("<==%s()\n", __func__));
}

#endif /* DBG */
#ifdef RATE_PRIOR_SUPPORT
INT Set_RatePrior_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG RatePrior;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s()\n", __func__));

	RatePrior = simple_strtol(arg, 0, 10);
	if (RatePrior == 1) {
		pAd->LowRateCtrl.RatePrior = 1;
		pAd->LowRateCtrl.LowRateRatioThreshold = 2;/*20%*/
		pAd->LowRateCtrl.LowRateCountPeriod = 5;
		pAd->LowRateCtrl.TotalCntThreshold = 50;
		pAd->LowRateCtrl.BlackListTimeout = 30;
	} else {
		pAd->LowRateCtrl.RatePrior = 0;
	}
	return TRUE;
}

INT Set_BlackListTimeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT BlackListTimeout;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s()\n", __func__));
	BlackListTimeout = simple_strtol(arg, 0, 10);
	pAd->LowRateCtrl.BlackListTimeout = BlackListTimeout;

	return TRUE;
}

INT Set_LowRateRatio_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT LowRateRatioThreshold;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s()\n", __func__));
	LowRateRatioThreshold = simple_strtol(arg, 0, 10);
	pAd->LowRateCtrl.LowRateRatioThreshold = LowRateRatioThreshold;

	return TRUE;
}

INT Set_LowRateCountPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT LowRateCountPeriod;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s()\n", __func__));
	LowRateCountPeriod = simple_strtol(arg, 0, 10);
	pAd->LowRateCtrl.LowRateCountPeriod = LowRateCountPeriod;

	return TRUE;
}

INT Set_TotalCntThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT TotalCntThreshold;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s()\n", __func__));
	TotalCntThreshold = simple_strtol(arg, 0, 10);
	pAd->LowRateCtrl.TotalCntThreshold = TotalCntThreshold;

	return TRUE;
}

#endif /*RATE_PRIOR_SUPPORT*/

#ifdef RANDOM_PKT_GEN
INT32 RandomTxCtrl;
UINT32 Qidmapping[16] = {0};
UINT32 pause_period;
UINT8 random_drop = FALSE;

INT Set_TxCtrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 testcase = 0;
	RTMP_STRING *pfp  = NULL;
	UINT32 tmp_value = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pfp = strsep(&arg, ":");

	if (pfp == NULL)
		return FALSE;

	RandomTxCtrl = os_str_tol(pfp, 0, 10);

	if (arg != NULL)
		testcase = os_str_toul(arg, 0, 16);

#ifdef DBDC_MODE
	/* Config WMM_0TO3_BAND_SEL for DBDC Mode */
	if (pAd->CommonCfg.dbdc_mode) {
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
		UINT8 band_idx = HcGetBandByWdev(wdev);

		MAC_IO_READ32(pAd->hdev_ctrl, CFG_DBDC_CTRL0, &tmp_value);
		if (RandomTxCtrl == 0) {
			tmp_value = ((tmp_value & ~(0xf << 22)) | (0x2 << 22));
		} else {
			if (band_idx == DBDC_BAND0)
				tmp_value &= ~(0xF << 22);
			else
				tmp_value |= (0xF << 22);
		}
		MAC_IO_WRITE32(pAd->hdev_ctrl, CFG_DBDC_CTRL0, tmp_value);
	}
#endif
	MTWF_PRINT("%s(): (RandomTxCtrl = %d) testcase: 0x%x\n", __func__, RandomTxCtrl, testcase);
	if (RandomTxCtrl == 0) {
		UINT i;
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP0, 0x42104210);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP1, 0x42104210);
		for (i = 0; i < cap->qos.WmmHwNum; i++) {
			Qidmapping[i] = 0;
			Qidmapping[i + 1] = 1;
			Qidmapping[i + 2] = 2;
			Qidmapping[i + 4] = 4;
		}
	} else if (RandomTxCtrl == 1) {
		UINT i;
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
		    HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP0, 0x7654e210);
		    HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP1, 0xeedcba98);
		    for	(i = 0; i < cap->qos.WmmHwNum * 4; i++) {
			Qidmapping[i] = i % (cap->qos.WmmHwNum * 4);
			if (Qidmapping[i] == 3 || Qidmapping[i] == 15)
				Qidmapping[i] = 14;
		    }
		} else {
			MTWF_PRINT("%s(): Need to turn on DBDC mode in the profile!\n", __func__);
		}
#else
		    HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP0, 0x7654b210);
		    HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP1, 0xb210ba98);
		    for (i = 0; i < cap->qos.WmmHwNum * 4; i++) {
			Qidmapping[i] = i % 12;
			if (Qidmapping[i] == 3)
				Qidmapping[i] = 11;
		    }
#endif
	} else if (RandomTxCtrl == 2) {
		UINT i;
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP0, 0xcde40124);
			HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP1, 0x456789ab);
			for (i = 0; i < cap->qos.WmmHwNum * 4; i++) {
				Qidmapping[i] = (19 - i) % (cap->qos.WmmHwNum * 4);
				if (Qidmapping[i] == 3 || Qidmapping[i] == 15)
					Qidmapping[i] = 4;
			}
		} else {
			MTWF_PRINT("%s(): Need to turn on DBDC mode in the profile!\n", __func__);
		}
#else
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP0, 0x89ab0124);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP1, 0x01244567);
		for (i = 0; i < cap->qos.WmmHwNum * 4; i++) {
			Qidmapping[i] = (15 - i) % 12;
			if (Qidmapping[i] == 3)
				Qidmapping[i] = 4;
		}
#endif
       }
	if (testcase & BIT0) {
		/* default setting */
		HIF_DMASHDL_IO_READ32(pAd, MT_HIF_DMASHDL_CTRL_SIGNAL, &tmp_value);
		tmp_value |= DMASHDL_HIF_ASK_SUB_ENA;
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_CTRL_SIGNAL, tmp_value);
		HIF_DMASHDL_IO_READ32(pAd, MT_HIF_DMASHDL_PKT_MAX_SIZE, &tmp_value);
		tmp_value &= ~(PLE_PKT_MAX_SIZE_MASK | PSE_PKT_MAX_SIZE_MASK);
		tmp_value |= PLE_PKT_MAX_SIZE_NUM(0x1);
		tmp_value |= PSE_PKT_MAX_SIZE_NUM(0x8);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_PKT_MAX_SIZE, tmp_value);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_SHDL_SET0, 0x6012345f);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_SHDL_SET1, 0xedcba987);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_SLOT_SET0, 0x76543210);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_SLOT_SET1, 0xfedcba98);
		HIF_DMASHDL_IO_READ32(pAd, MT_HIF_DMASHDL_PAGE_SET, &tmp_value);
		tmp_value |= DMASHDL_GROUP_SEQ_ORDER_TYPE;
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_PAGE_SET, tmp_value);
		pause_period = 0;
		random_drop = 0;
		HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE0, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE1, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE2, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE3, 0);
	}

	if (testcase & BIT1) {
		/* disable cr_hif_ask_sub_ena, ple_packet_max_size = 6 */
		HIF_DMASHDL_IO_READ32(pAd, MT_HIF_DMASHDL_CTRL_SIGNAL, &tmp_value);
		tmp_value &= ~DMASHDL_HIF_ASK_SUB_ENA;
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_CTRL_SIGNAL, tmp_value);
		HIF_DMASHDL_IO_READ32(pAd, MT_HIF_DMASHDL_PKT_MAX_SIZE, &tmp_value);
		tmp_value &= ~PLE_PKT_MAX_SIZE_MASK;
		tmp_value |= PLE_PKT_MAX_SIZE_NUM(0x8);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_PKT_MAX_SIZE, tmp_value);
	}

	if (testcase & BIT2) {
		/* modify schedular priority(0x5000a0b0, 0x5000a0b4, 0x5000a0c4, 0x5000a0c8) */
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_SHDL_SET0, 0x6012345f);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_SHDL_SET1, 0xedcba987);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_SLOT_SET0, 0x6012345f);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_SLOT_SET1, 0xedcba987);
	}

	if (testcase & BIT3) {
		/* disable User program group sequence type control (0x5000a00c[16]) */
		HIF_DMASHDL_IO_READ32(pAd, MT_HIF_DMASHDL_PAGE_SET, &tmp_value);
		tmp_value &= ~DMASHDL_GROUP_SEQ_ORDER_TYPE;
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_PAGE_SET, tmp_value);
	}

	if (testcase & BIT4) {
		if (pause_period == 0)
			pause_period = 120;
		else {
			pause_period = 0;
			HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE0, 0);
			HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE1, 0);
			HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE2, 0);
			HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE3, 0);
		}
	}

	if (testcase & BIT5)
		random_drop = (random_drop == 1) ? 0 : 1;

	if (testcase & BIT6)
		random_drop = (random_drop == 2) ? 0 : 2;

	return TRUE;
}

VOID regular_pause_umac(RTMP_ADAPTER *pAd)
{
	if (pause_period == 0)
		return;

	if ((pAd->Mlme.PeriodicRound % (pause_period * 2)) == 0) {
		HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE0, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE1, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE2, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE3, 0);
	} else if ((pAd->Mlme.PeriodicRound % pause_period) == 0) {
		HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE0, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE1, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE2, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE3, 0xffffffff);
	} else if (random_drop
			   && ((pAd->Mlme.PeriodicRound + pause_period / 2) % (pause_period * 2)) == 0) {
		UINT32 ple_stat[4] = {0};
		INT32 i, j;
		UINT32 hfid;
		UINT32 deq_fid;

		HW_IO_READ32(pAd->hdev_ctrl, PLE_AC0_QUEUE_EMPTY_0, &ple_stat[0]);
		HW_IO_READ32(pAd->hdev_ctrl, PLE_AC1_QUEUE_EMPTY_0, &ple_stat[1]);
		HW_IO_READ32(pAd->hdev_ctrl, PLE_AC2_QUEUE_EMPTY_0, &ple_stat[2]);
		HW_IO_READ32(pAd->hdev_ctrl, PLE_AC3_QUEUE_EMPTY_0, &ple_stat[3]);

		for (j = 0; j < 4; j++) {
			for (i = 0; i < 32; i++) {
				if (((ple_stat[j] & (0x1 << i)) >> i) == 0) {
					UINT32 fl_que_ctrl[4] = {0};

					fl_que_ctrl[0] |= (0x1 << 31);
					fl_que_ctrl[0] |= (0x2 << 14);
					fl_que_ctrl[0] |= (j << 8);
					fl_que_ctrl[0] |= i;
					HW_IO_WRITE32(pAd->hdev_ctrl, PLE_FL_QUE_CTRL_0, fl_que_ctrl[0]);
					HW_IO_READ32(pAd->hdev_ctrl, PLE_FL_QUE_CTRL_2, &fl_que_ctrl[1]);
					HW_IO_READ32(pAd->hdev_ctrl, PLE_FL_QUE_CTRL_3, &fl_que_ctrl[2]);
					hfid = fl_que_ctrl[1] & 0xfff;

					if (hfid == 0xfff)
						continue;

					fl_que_ctrl[0] |= (0x2 << 16);

					if (random_drop == 2)
						fl_que_ctrl[0] |= (0x9 << 20);

					fl_que_ctrl[1] = hfid << 16 | hfid;
					HW_IO_WRITE32(pAd->hdev_ctrl, PLE_C_DE_QUEUE_1, fl_que_ctrl[1]);

					if (random_drop == 2) {
						fl_que_ctrl[3] = 0x3 << 30;
						fl_que_ctrl[3] |= 0x1f << 24;
						HW_IO_WRITE32(pAd->hdev_ctrl, PLE_C_DE_QUEUE_2, fl_que_ctrl[3]);
					}

					HW_IO_WRITE32(pAd->hdev_ctrl, PLE_C_DE_QUEUE_0, fl_que_ctrl[0]);
					HW_IO_READ32(pAd->hdev_ctrl, PLE_C_DE_QUEUE_3, &fl_que_ctrl[2]);
					deq_fid = fl_que_ctrl[2] & 0xfff;

					if (deq_fid == 0xfff || random_drop == 2)
						continue;

					fl_que_ctrl[0] = (0x1 << 31);
					fl_que_ctrl[0] |= (0x1 << 16);
					fl_que_ctrl[0] |= (0x3 << 14);
					fl_que_ctrl[0] |= (0x1f << 8);
					fl_que_ctrl[1] = (deq_fid << 16) | deq_fid;
					HW_IO_WRITE32(pAd->hdev_ctrl, PLE_C_EN_QUEUE_1, fl_que_ctrl[1]);
					HW_IO_WRITE32(pAd->hdev_ctrl, PLE_C_EN_QUEUE_0, fl_que_ctrl[0]);
				}
			}
		}
	}
}
#endif
#ifdef CSO_TEST_SUPPORT
INT32 CsCtrl;
INT Set_CsCtrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (arg == NULL)
		return FALSE;

	CsCtrl = os_str_tol(arg, 0, 10);

	if (CsCtrl & BIT0)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("IPV4 checksum overwrite enable!\n"));

	if (CsCtrl & BIT1)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TCP checksum overwrite enable!\n"));

	if (CsCtrl & BIT2)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("UDP checksum overwrite enable!\n"));

	if (CsCtrl & BIT3)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Tx Debug log enable!\n"));

	if (CsCtrl & BIT4)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rx Debug log enable!\n"));

	return TRUE;
}
#endif

INT	Show_WifiSysInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	wifi_sys_dump(pAd);
	return TRUE;
}

INT Show_DescInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef RTMP_MAC_PCI
	INT32 i, QueIdx;
	RXD_STRUC *pRxD;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD, RxD;
	TXD_STRUC *pDestTxD, TxD;
#endif /* RT_BIG_ENDIAN */
	struct hif_pci_tx_ring *pTxRing;
	PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *pRxRing;
	struct hif_pci_tx_ring *pCtrlRing = &hif->ctrl_ring;

	PUCHAR pDMAHeaderBufVA;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	UINT16 rx0_ring_size = GET_RX0_RING_SIZE(cap);
	UINT16 rx1_ring_size = GET_RX1_RING_SIZE(cap);

	if (arg != NULL && strlen(arg))
		QueIdx = os_str_tol(arg, 0, 10);
	else
		QueIdx = 0xff;

	switch (QueIdx) {
	case 0:
	case 1:
	case 2:
	case 3:
		pTxRing = &hif->TxRing[QueIdx];
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Tx Ring %d ---------------------------------\n", QueIdx));

		for (i = 0; i < tx_ring_size; i++) {
			pDMAHeaderBufVA = (UCHAR *)pTxRing->Cell[i].DmaBuf.AllocVa;
#ifdef RT_BIG_ENDIAN
			pDestTxD = (TXD_STRUC *)pTxRing->Cell[i].AllocVa;
			TxD = *pDestTxD;
			pTxD = &TxD;
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);

			if (pDMAHeaderBufVA)
				MTMacInfoEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, 32);
			else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt is null\n"));

#else
			pTxD = (TXD_STRUC *)pTxRing->Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Desc #%d\n", i));

			if (pTxD)
				dump_txd(pAd, pTxD);
			else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXD null!!\n"));

			if (pDMAHeaderBufVA) {
				asic_dump_tmac_info(pAd, pDMAHeaderBufVA);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt physical address = %x\n",
						 (UINT32)pTxRing->Cell[i].PacketPa));
#ifdef RT_BIG_ENDIAN
				MTMacInfoEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, 32);
#endif
			} else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt is null\n"));
		}

		break;

	case 5:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Control ring---------------------------------------------\n"));

		for (i = 0; i < MGMT_RING_SIZE; i++) {
			pDMAHeaderBufVA = (UCHAR *)pCtrlRing->Cell[i].DmaBuf.AllocVa;
#ifdef RT_BIG_ENDIAN
			pDestTxD = (TXD_STRUC *)pCtrlRing->Cell[i].AllocVa;
			TxD = *pDestTxD;
			pTxD = &TxD;
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);

			if (pDMAHeaderBufVA)
				MTMacInfoEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, 32);
			else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt is null\n"));

#else
			pTxD = (TXD_STRUC *)pCtrlRing->Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Desc #%d\n", i));

			if (pTxD)
				dump_txd(pAd, pTxD);
			else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXD null!!\n"));

			if (pDMAHeaderBufVA) {
				asic_dump_tmac_info(pAd, pDMAHeaderBufVA);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt physical address = %x\n",
						 (UINT32)pCtrlRing->Cell[i].PacketPa));
#ifdef RT_BIG_ENDIAN
				MTMacInfoEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, 32);
#endif
			} else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt is null\n"));
		}

		break;

	case 12:
		for (QueIdx = 0; QueIdx < GET_NUM_OF_RX_RING(cap); QueIdx++) {
			UINT16 RxRingSize = (QueIdx == 0) ? rx0_ring_size : rx1_ring_size;

			pRxRing = &hif->RxRing[QueIdx];
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rx Ring %d ---------------------------------\n", QueIdx));

			for (i = 0; i < RxRingSize; i++) {
#ifdef RT_BIG_ENDIAN
				pDestRxD = (RXD_STRUC *)pRxRing->Cell[i].AllocVa;
				RxD = *pDestRxD;
				pRxD = &RxD;
				RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
				pRxD = (RXD_STRUC *)pRxRing->Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Desc #%d\n", i));

				if (pRxD) {
					dump_rxd(pAd, pRxD);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pRxD->DDONE = %x\n", pRxD->DDONE));
				} else
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RXD null!!\n"));
			}
		}

		break;

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknown QueIdx(%d)\n", QueIdx));
		break;
	}

#endif /* RTMP_MAC_PCI */
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Reset statistics counter

    Arguments:
	pAd            Pointer to our adapter
	arg

    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ResetStatCounter_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;

	if (wdev != NULL)
		ucBand = HcGetBandByWdev(wdev);

	if (ucBand >= DBDC_BAND_NUM)
		return FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_ResetStatCounter_Proc\n"));
	/* add the most up-to-date h/w raw counters into software counters*/
	NICUpdateRawCountersNew(pAd);

	NdisZeroMemory(&pAd->WlanCounters[ucBand], sizeof(COUNTER_802_11));
	NdisZeroMemory(&pAd->Counters8023, sizeof(COUNTER_802_3));
	NdisZeroMemory(&pAd->RalinkCounters, sizeof(COUNTER_RALINK));
	pAd->mcli_ctl.last_tx_cnt = 0;
	pAd->mcli_ctl.last_tx_fail_cnt = 0;
#ifdef INT_STATISTIC
	pAd->INTCNT = 0;
#ifdef MT_MAC
	pAd->INTWFMACINT0CNT = 0;
	pAd->INTWFMACINT1CNT = 0;
	pAd->INTWFMACINT2CNT = 0;
	pAd->INTWFMACINT3CNT = 0;
	pAd->INTWFMACINT4CNT = 0;
	pAd->INTBCNDLY = 0;
	pAd->INTBMCDLY = 0;
#endif
	pAd->INTTxCoherentCNT = 0;
	pAd->INTRxCoherentCNT = 0;
	pAd->INTFifoStaFullIntCNT = 0;
	pAd->INTMGMTDLYCNT = 0;
	pAd->INTRXDATACNT = 0;
	pAd->INTRXCMDCNT = 0;
	pAd->INTHCCACNT = 0;
	pAd->INTAC3CNT = 0;
	pAd->INTAC2CNT = 0;
	pAd->INTAC1CNT = 0;
	pAd->INTAC0CNT = 0;
	pAd->INTPreTBTTCNT = 0;
	pAd->INTTBTTIntCNT = 0;
	pAd->INTGPTimeOutCNT = 0;
	pAd->INTAutoWakeupIntCNT = 0;
#endif
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	{
		/* clear TX success/fail count in MCU */
		EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;

		MtCmdGetTxStatistic(pAd, GET_TX_STAT_TOTAL_TX_CNT, ucBand, 0, &rTxStatResult);
	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
#ifdef CONFIG_ATE
	/* Clear TX success count in ATE mode */
	pAd->ATECtrl.TxDoneCount = 0;
	if (ATE_ON(pAd)) {
		struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
		TESTMODE_SET_PARAM(ATECtrl, ucBand, TxDoneCount, 0);
		NdisZeroMemory(&ATECtrl->rx_stat, sizeof(struct _ATE_RX_STATISTIC));
	}
#ifdef CONFIG_QA
	MT_ATEUpdateRxStatistic(pAd, 2, NULL);
#endif /* CONFIG_QA */
#endif /* CONFIG_ATE */
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
#ifdef TXBF_SUPPORT
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->FlgHwTxBfCap) {
		int i;

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
			NdisZeroMemory(&pAd->MacTab.Content[i].TxBFCounters, sizeof(pAd->MacTab.Content[i].TxBFCounters));
	}
}
#endif /* TXBF_SUPPORT */
	return TRUE;
}

BOOLEAN RTMPCheckStrPrintAble(
	IN  CHAR *pInPutStr,
	IN  UCHAR strLen)
{
	UCHAR i = 0;

	for (i = 0; i < strLen; i++) {
		if ((pInPutStr[i] < 0x20) || (pInPutStr[i] > 0x7E))
			return FALSE;
	}

	return TRUE;
}

/*
	========================================================================

	Routine Description:
		Remove WPA Key process

	Arguments:
		pAd					Pointer to our adapter
		pBuf							Pointer to the where the key stored

	Return Value:
		NDIS_SUCCESS					Add key successfully

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
#ifdef CONFIG_STA_SUPPORT
VOID RTMPSetDesiredRates(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, LONG Rates)
{
	NDIS_802_11_RATES aryRates;

	memset(&aryRates, 0x00, sizeof(NDIS_802_11_RATES));

	switch (wdev->PhyMode) {
	case (UCHAR)(WMODE_A): /* A only*/
		switch (Rates) {
		case 6000000: /*6M*/
			aryRates[0] = 0x0c; /* 6M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_0;
			break;

		case 9000000: /*9M*/
			aryRates[0] = 0x12; /* 9M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_1;
			break;

		case 12000000: /*12M*/
			aryRates[0] = 0x18; /* 12M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_2;
			break;

		case 18000000: /*18M*/
			aryRates[0] = 0x24; /* 18M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_3;
			break;

		case 24000000: /*24M*/
			aryRates[0] = 0x30; /* 24M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_4;
			break;

		case 36000000: /*36M*/
			aryRates[0] = 0x48; /* 36M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_5;
			break;

		case 48000000: /*48M*/
			aryRates[0] = 0x60; /* 48M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_6;
			break;

		case 54000000: /*54M*/
			aryRates[0] = 0x6c; /* 54M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_7;
			break;

		case -1: /*Auto*/
		default:
			aryRates[0] = 0x6c; /* 54Mbps*/
			aryRates[1] = 0x60; /* 48Mbps*/
			aryRates[2] = 0x48; /* 36Mbps*/
			aryRates[3] = 0x30; /* 24Mbps*/
			aryRates[4] = 0x24; /* 18M*/
			aryRates[5] = 0x18; /* 12M*/
			aryRates[6] = 0x12; /* 9M*/
			aryRates[7] = 0x0c; /* 6M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			break;
		}

		break;

	case (UCHAR)(WMODE_B | WMODE_G): /* B/G Mixed*/
	case (UCHAR)(WMODE_B): /* B only*/
	case (UCHAR)(WMODE_A | WMODE_B | WMODE_G): /* A/B/G Mixed*/
	default:
		switch (Rates) {
		case 1000000: /*1M*/
			aryRates[0] = 0x02;
			wdev->DesiredTransmitSetting.field.MCS = MCS_0;
			break;

		case 2000000: /*2M*/
			aryRates[0] = 0x04;
			wdev->DesiredTransmitSetting.field.MCS = MCS_1;
			break;

		case 5000000: /*5.5M*/
			aryRates[0] = 0x0b; /* 5.5M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_2;
			break;

		case 11000000: /*11M*/
			aryRates[0] = 0x16; /* 11M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_3;
			break;

		case 6000000: /*6M*/
			aryRates[0] = 0x0c; /* 6M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_0;
			break;

		case 9000000: /*9M*/
			aryRates[0] = 0x12; /* 9M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_1;
			break;

		case 12000000: /*12M*/
			aryRates[0] = 0x18; /* 12M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_2;
			break;

		case 18000000: /*18M*/
			aryRates[0] = 0x24; /* 18M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_3;
			break;

		case 24000000: /*24M*/
			aryRates[0] = 0x30; /* 24M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_4;
			break;

		case 36000000: /*36M*/
			aryRates[0] = 0x48; /* 36M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_5;
			break;

		case 48000000: /*48M*/
			aryRates[0] = 0x60; /* 48M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_6;
			break;

		case 54000000: /*54M*/
			aryRates[0] = 0x6c; /* 54M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_7;
			break;

		case -1: /*Auto*/
		default:
			if (wdev->PhyMode == WMODE_B) {
				/*B Only*/
				aryRates[0] = 0x16; /* 11Mbps*/
				aryRates[1] = 0x0b; /* 5.5Mbps*/
				aryRates[2] = 0x04; /* 2Mbps*/
				aryRates[3] = 0x02; /* 1Mbps*/
			} else {
				/*(B/G) Mixed or (A/B/G) Mixed*/
				aryRates[0] = 0x6c; /* 54Mbps*/
				aryRates[1] = 0x60; /* 48Mbps*/
				aryRates[2] = 0x48; /* 36Mbps*/
				aryRates[3] = 0x30; /* 24Mbps*/
				aryRates[4] = 0x16; /* 11Mbps*/
				aryRates[5] = 0x0b; /* 5.5Mbps*/
				aryRates[6] = 0x04; /* 2Mbps*/
				aryRates[7] = 0x02; /* 1Mbps*/
			}

			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			break;
		}

		break;
	}

	NdisZeroMemory(wdev->rate.DesireRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisMoveMemory(wdev->rate.DesireRate, &aryRates, sizeof(NDIS_802_11_RATES));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 (" RTMPSetDesiredRates (%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x)\n",
			  wdev->rate.DesireRate[0], wdev->rate.DesireRate[1],
			  wdev->rate.DesireRate[2], wdev->rate.DesireRate[3],
			  wdev->rate.DesireRate[4], wdev->rate.DesireRate[5],
			  wdev->rate.DesireRate[6], wdev->rate.DesireRate[7]));
	/* Changing DesiredRate may affect the MAX TX rate we used to TX frames out*/
	MlmeUpdateTxRates(pAd, FALSE, 0);
}
#endif /* CONFIG_STA_SUPPORT */

#if defined(CONFIG_STA_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT)
NDIS_STATUS RTMPWPARemoveKeyProc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PVOID			pBuf)
{
	PNDIS_802_11_REMOVE_KEY pKey;
	ULONG					KeyIdx;
	NDIS_STATUS			Status = NDIS_STATUS_FAILURE;
	BOOLEAN	bTxKey;		/* Set the key as transmit key*/
	BOOLEAN	bPairwise;		/* Indicate the key is pairwise key*/
	BOOLEAN	bKeyRSC;		/* indicate the receive  SC set by KeyRSC value.*/
	/* Otherwise, it will set by the NIC.*/
	BOOLEAN	bAuthenticator; /* indicate key is set by authenticator.*/
	INT		i;
#ifdef APCLI_SUPPORT
#endif/*APCLI_SUPPORT*/
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("---> RTMPWPARemoveKeyProc\n"));
	pKey = (PNDIS_802_11_REMOVE_KEY) pBuf;
	KeyIdx = pKey->KeyIndex & 0xff;
	/* Bit 31 of Add-key, Tx Key*/
	bTxKey = (pKey->KeyIndex & 0x80000000) ? TRUE : FALSE;
	/* Bit 30 of Add-key PairwiseKey*/
	bPairwise = (pKey->KeyIndex & 0x40000000) ? TRUE : FALSE;
	/* Bit 29 of Add-key KeyRSC*/
	bKeyRSC = (pKey->KeyIndex & 0x20000000) ? TRUE : FALSE;
	/* Bit 28 of Add-key Authenticator*/
	bAuthenticator = (pKey->KeyIndex & 0x10000000) ? TRUE : FALSE;

	/* 1. If bTx is TRUE, return failure information*/
	if (bTxKey == TRUE)
		return NDIS_STATUS_INVALID_DATA;

	/* 2. Check Pairwise Key*/
	if (bPairwise) {
		/* a. If BSSID is broadcast, remove all pairwise keys.*/
		/* b. If not broadcast, remove the pairwise specified by BSSID*/
		for (i = 0; i < SHARE_KEY_NUM; i++) {
#ifdef APCLI_SUPPORT
#endif/*APCLI_SUPPORT*/
			{
#ifdef CONFIG_STA_SUPPORT

				if (MAC_ADDR_EQUAL(pAd->SharedKey[BSS0][i].BssId, pKey->BSSID)) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPWPARemoveKeyProc(KeyIdx=%d)\n", i));
					pAd->SharedKey[BSS0][i].KeyLen = 0;
					pAd->SharedKey[BSS0][i].CipherAlg = CIPHER_NONE;
					AsicRemoveSharedKeyEntry(pAd, BSS0, (UCHAR)i);
					Status = NDIS_STATUS_SUCCESS;
					break;
				}

#endif/*CONFIG_STA_SUPPORT*/
			}
		}
	}
	/* 3. Group Key*/
	else {
		/* a. If BSSID is broadcast, remove all group keys indexed*/
		/* b. If BSSID matched, delete the group key indexed.*/
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPWPARemoveKeyProc(KeyIdx=%ld)\n", KeyIdx));
		pAd->SharedKey[BSS0][KeyIdx].KeyLen = 0;
		pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_NONE;
		AsicRemoveSharedKeyEntry(pAd, BSS0, (UCHAR)KeyIdx);
		Status = NDIS_STATUS_SUCCESS;
	}

	return Status;
}
#endif /* defined(CONFIG_STA_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT) */

#ifdef CONFIG_STA_SUPPORT
/*
	========================================================================

	Routine Description:
		Remove All WPA Keys

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPWPARemoveAllKeys(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	ASIC_SEC_INFO Info = {0};
	MAC_TABLE_ENTRY *pEntry = NULL;

	pEntry = GetAssociatedAPByWdev(pAd, wdev);

	if (!pEntry)
		return;

	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_REMOVE_PAIRWISE_KEY;
	Info.Wcid = pEntry->wcid;
	HW_ADDREMOVE_KEYTABLE(pAd, &Info);
}
#endif /* CONFIG_STA_SUPPORT */

/*
	========================================================================
	Routine Description:
		Change NIC PHY mode. Re-association may be necessary

	Arguments:
		pAd - Pointer to our adapter
		phymode  -

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	========================================================================
*/
VOID RTMPSetPhyMode(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR phymode)
{
	INT i;
	UCHAR RfIC = wmode_2_rfic(phymode);
	UCHAR Channel = (wdev->channel) ? wdev->channel : HcGetChannelByRf(pAd, RfIC);
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);


	/* sanity check user setting*/
	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (Channel == pChCtrl->ChList[i].Channel)
			break;
	}

	if (i == pChCtrl->ChListNum) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)

		if (Channel != 0)
			Channel = FirstChannel(pAd, wdev);

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		Channel = FirstChannel(pAd, wdev);
#endif /* CONFIG_STA_SUPPORT */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): channel out of range, use first ch=%d\n",
				 __func__, Channel));
		wdev->channel = Channel;
		wlan_operate_set_prim_ch(wdev, wdev->channel);
	}

	MlmeUpdateTxRatesWdev(pAd, FALSE, wdev);
	/* CFG_TODO */
#ifdef DOT11_N_SUPPORT
	SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
}

#ifdef MIN_PHY_RATE_SUPPORT
VOID RTMPUpdateRateInfoMinPhyRate(
	struct dev_rate_info *rate
	)
{
	struct dev_rate_info temprate;
	UCHAR Index = 0;
	UCHAR SuppRateIdx = 0, ExtRateIdx = 0;

	if ((rate->LimitClientSupportRate == TRUE) && (rate->MinPhyDataRate != 0)) {
		NdisCopyMemory(temprate.SupRate, rate->SupRate, MAX_LEN_OF_SUPPORTED_RATES);
		NdisCopyMemory(temprate.ExtRate, rate->ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
		NdisCopyMemory(temprate.DesireRate, rate->DesireRate, MAX_LEN_OF_SUPPORTED_RATES);

		temprate.SupRateLen = rate->SupRateLen;
		temprate.ExtRateLen = rate->ExtRateLen;

		NdisZeroMemory(rate->SupRate, MAX_LEN_OF_SUPPORTED_RATES);
		NdisZeroMemory(rate->ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
		NdisZeroMemory(rate->DesireRate, MAX_LEN_OF_SUPPORTED_RATES);

		/*Supported Rate*/
		for (Index = 0; Index < temprate.SupRateLen; Index++) {
			if ((temprate.SupRate[Index] & 0x7F) >= (rate->MinPhyDataRate * 2)) {
				rate->SupRate[SuppRateIdx] = temprate.SupRate[Index];
				SuppRateIdx++;
			}
		}

		/*Extended Supported Rate*/
		for (Index = 0; Index < temprate.ExtRateLen; Index++) {
			if ((temprate.ExtRate[Index] & 0x7F) >= (rate->MinPhyDataRate * 2)) {
				if (SuppRateIdx < 8) {
					rate->SupRate[SuppRateIdx] = temprate.ExtRate[Index];
					SuppRateIdx++;
				} else {
					rate->ExtRate[ExtRateIdx] = temprate.ExtRate[Index];
					ExtRateIdx++;
				}
			}
		}
		rate->SupRateLen = SuppRateIdx;
		rate->ExtRateLen = ExtRateIdx;

		/*Desired Supported Rate*/
		SuppRateIdx = 0;
		for (Index = 0; Index < MAX_LEN_OF_SUPPORTED_RATES; Index++) {
			if ((temprate.DesireRate[Index] & 0x7F) >= (rate->MinPhyDataRate * 2)) {
				rate->DesireRate[SuppRateIdx] = temprate.DesireRate[Index];
				SuppRateIdx++;
			}
		}
	}
}
#endif /* MIN_PHY_RATE_SUPPORT */

VOID RTMPUpdateRateInfo(
	UCHAR phymode,
	struct dev_rate_info *rate
)
{
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	INT i;
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

	NdisZeroMemory(rate->SupRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(rate->ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(rate->DesireRate, MAX_LEN_OF_SUPPORTED_RATES);

	switch (phymode) {
	case (WMODE_B):
		rate->SupRate[0]  = 0x82;	  /* 1 mbps, in units of 0.5 Mbps, basic rate */
		rate->SupRate[1]  = 0x84;	  /* 2 mbps, in units of 0.5 Mbps, basic rate */
		rate->SupRate[2]  = 0x8B;	  /* 5.5 mbps, in units of 0.5 Mbps, basic rate */
		rate->SupRate[3]  = 0x96;	  /* 11 mbps, in units of 0.5 Mbps, basic rate */
		rate->SupRateLen  = 4;
		rate->ExtRateLen  = 0;
		rate->DesireRate[0]  = 2;	   /* 1 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[1]  = 4;	   /* 2 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[2]  = 11;    /* 5.5 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[3]  = 22;    /* 11 mbps, in units of 0.5 Mbps*/
		/*pAd->CommonCfg.HTPhyMode.field.MODE = MODE_CCK;  This MODE is only FYI. not use*/
		/*update MlmeTransmit rate*/
		rate->MlmeTransmit.field.MCS = MCS_0;
		rate->MlmeTransmit.field.BW = BW_20;
		rate->MlmeTransmit.field.MODE = MODE_CCK;
		break;

	/*
		In current design, we will put supported/extended rate element in
		beacon even we are 11n-only mode.
		Or some 11n stations will not connect to us if we do not put
		supported/extended rate element in beacon.
	*/
	case (WMODE_G):
	case (WMODE_B | WMODE_G):
	case (WMODE_A | WMODE_B | WMODE_G):
#ifdef DOT11_N_SUPPORT
	case (WMODE_GN):
	case (WMODE_A | WMODE_B | WMODE_G | WMODE_GN | WMODE_AN):
	case (WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN | WMODE_AC):
	case (WMODE_B | WMODE_G | WMODE_GN):
	case (WMODE_G | WMODE_GN):
#endif /* DOT11_N_SUPPORT */
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
		if (rate->Eap_SupRate_En == TRUE) {
			for (i = 0; i < rate->EapSupRateLen; i++)
				rate->SupRate[i] = rate->EapSupRate[i];

			rate->SupRateLen = rate->EapSupRateLen;

			for (i = 0; i < rate->EapExtSupRateLen; i++)
				rate->ExtRate[i] = rate->EapExtSupRate[i];

			rate->ExtRateLen = rate->EapExtSupRateLen;
		} else
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
		{
			rate->SupRate[0]  = 0x82;	  /* 1 mbps, in units of 0.5 Mbps, basic rate*/
			rate->SupRate[1]  = 0x84;	  /* 2 mbps, in units of 0.5 Mbps, basic rate*/
			rate->SupRate[2]  = 0x8B;	  /* 5.5 mbps, in units of 0.5 Mbps, basic rate*/
			rate->SupRate[3]  = 0x96;	  /* 11 mbps, in units of 0.5 Mbps, basic rate*/
			rate->SupRate[4]  = 0x12;	  /* 9 mbps, in units of 0.5 Mbps*/
			rate->SupRate[5]  = 0x24;	  /* 18 mbps, in units of 0.5 Mbps*/
			rate->SupRate[6]  = 0x48;	  /* 36 mbps, in units of 0.5 Mbps*/
			rate->SupRate[7]  = 0x6c;	  /* 54 mbps, in units of 0.5 Mbps*/
			rate->SupRateLen  = 8;
			rate->ExtRate[0]  = 0x0C;	  /* 6 mbps, in units of 0.5 Mbps*/
			rate->ExtRate[1]  = 0x18;	  /* 12 mbps, in units of 0.5 Mbps*/
			rate->ExtRate[2]  = 0x30;	  /* 24 mbps, in units of 0.5 Mbps*/
			rate->ExtRate[3]  = 0x60;	  /* 48 mbps, in units of 0.5 Mbps*/
			rate->ExtRateLen  = 4;
		}
		rate->DesireRate[0]  = 2;	   /* 1 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[1]  = 4;	   /* 2 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[2]  = 11;    /* 5.5 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[3]  = 22;    /* 11 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[4]  = 12;    /* 6 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[5]  = 18;    /* 9 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[6]  = 24;    /* 12 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[7]  = 36;    /* 18 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[8]  = 48;    /* 24 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[9]  = 72;    /* 36 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[10] = 96;    /* 48 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[11] = 108;   /* 54 mbps, in units of 0.5 Mbps*/
		/*update MlmeTransmit rate*/
		rate->MlmeTransmit.field.MCS = MCS_0;
		rate->MlmeTransmit.field.BW = BW_20;
		rate->MlmeTransmit.field.MODE = MODE_CCK;
		break;

	case (WMODE_A):
#ifdef DOT11_N_SUPPORT
	case (WMODE_A | WMODE_AN):
	case (WMODE_A | WMODE_G | WMODE_GN | WMODE_AN):
	case (WMODE_AN):
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
	case (WMODE_A | WMODE_AN | WMODE_AC):
	case (WMODE_AN | WMODE_AC):
#endif /* DOT11_VHT_AC */
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
		if (rate->Eap_SupRate_En == TRUE) {
			for (i = 0; i < rate->EapSupRateLen; i++)
				rate->SupRate[i] = rate->EapSupRate[i];

			rate->SupRateLen = rate->EapSupRateLen;
		} else
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
		{
			rate->SupRate[0]  = 0x8C;	  /* 6 mbps, in units of 0.5 Mbps, basic rate*/
			rate->SupRate[1]  = 0x12;	  /* 9 mbps, in units of 0.5 Mbps*/
			rate->SupRate[2]  = 0x98;	  /* 12 mbps, in units of 0.5 Mbps, basic rate*/
			rate->SupRate[3]  = 0x24;	  /* 18 mbps, in units of 0.5 Mbps*/
			rate->SupRate[4]  = 0xb0;	  /* 24 mbps, in units of 0.5 Mbps, basic rate*/
			rate->SupRate[5]  = 0x48;	  /* 36 mbps, in units of 0.5 Mbps*/
			rate->SupRate[6]  = 0x60;	  /* 48 mbps, in units of 0.5 Mbps*/
			rate->SupRate[7]  = 0x6c;	  /* 54 mbps, in units of 0.5 Mbps*/
			rate->SupRateLen  = 8;
		}
		rate->ExtRateLen  = 0;
		rate->DesireRate[0]  = 12;    /* 6 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[1]  = 18;    /* 9 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[2]  = 24;    /* 12 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[3]  = 36;    /* 18 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[4]  = 48;    /* 24 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[5]  = 72;    /* 36 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[6]  = 96;    /* 48 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[7]  = 108;   /* 54 mbps, in units of 0.5 Mbps*/
		/*pAd->CommonCfg.HTPhyMode.field.MODE = MODE_OFDM;  This MODE is only FYI. not use*/
		/*update MlmeTransmit rate*/
		rate->MlmeTransmit.field.MCS = MCS_RATE_6;
		rate->MlmeTransmit.field.BW = BW_20;
		rate->MlmeTransmit.field.MODE = MODE_OFDM;
		break;

	default:
		break;
	}

#ifdef MIN_PHY_RATE_SUPPORT
	RTMPUpdateRateInfoMinPhyRate(rate);
#endif /* MIN_PHY_RATE_SUPPORT */
}

#ifdef GN_MIXMODE_SUPPORT
VOID RTMPUpdateGNRateInfo(
	UCHAR phymode,
	struct dev_rate_info *rate
	)
{

	NdisZeroMemory(rate->SupRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(rate->ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(rate->DesireRate, MAX_LEN_OF_SUPPORTED_RATES);

	rate->SupRate[0]  = 0x8C;	  /* 6 mbps, in units of 0.5 Mbps, basic rate*/
	rate->SupRate[1]  = 0x12;	  /* 9 mbps, in units of 0.5 Mbps*/
	rate->SupRate[2]  = 0x98;	  /* 12 mbps, in units of 0.5 Mbps, basic rate*/
	rate->SupRate[3]  = 0x24;	  /* 18 mbps, in units of 0.5 Mbps*/
	rate->SupRate[4]  = 0xb0;	  /* 24 mbps, in units of 0.5 Mbps, basic rate*/
	rate->SupRate[5]  = 0x48;	  /* 36 mbps, in units of 0.5 Mbps*/
	rate->SupRate[6]  = 0x60;	  /* 48 mbps, in units of 0.5 Mbps*/
	rate->SupRate[7]  = 0x6c;	  /* 54 mbps, in units of 0.5 Mbps*/
	rate->SupRateLen  = 8;
	rate->ExtRateLen  = 0;

	rate->DesireRate[0]  = 12;    /* 6 mbps, in units of 0.5 Mbps*/
	rate->DesireRate[1]  = 18;    /* 9 mbps, in units of 0.5 Mbps*/
	rate->DesireRate[2]  = 24;    /* 12 mbps, in units of 0.5 Mbps*/
	rate->DesireRate[3]  = 36;    /* 18 mbps, in units of 0.5 Mbps*/
	rate->DesireRate[4]  = 48;    /* 24 mbps, in units of 0.5 Mbps*/
	rate->DesireRate[5]  = 72;    /* 36 mbps, in units of 0.5 Mbps*/
	rate->DesireRate[6]  = 96;    /* 48 mbps, in units of 0.5 Mbps*/
	rate->DesireRate[7]  = 108;   /* 54 mbps, in units of 0.5 Mbps*/

	/*update MlmeTransmit rate*/
	rate->MlmeTransmit.field.MCS = MCS_RATE_6;
	rate->MlmeTransmit.field.BW = BW_20;
	rate->MlmeTransmit.field.MODE = MODE_OFDM;

}
#endif /* GN_MIXMODE_SUPPORT */

/*
	========================================================================
	Description:
		Add Client security information into ASIC WCID table and IVEIV table.
    Return:
	========================================================================
*/
VOID RTMPAddWcidAttributeEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR BssIdx,
	IN UCHAR KeyIdx,
	IN UCHAR CipherAlg,
	IN MAC_TABLE_ENTRY *pEntry)
{
	UINT32		WCIDAttri = 0;
	USHORT		offset;
	UCHAR		IVEIV = 0;
	USHORT		Wcid = 0;
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN		IEEE8021X = FALSE;
	struct wifi_dev *wdev = NULL;
#endif /* CONFIG_AP_SUPPORT */

	/* TODO: shiang-7603!! fix me */
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT76x6(pAd) || IS_MT7637(pAd)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): MT7603 Not support yet!\n", __func__));
		return;
	}

	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

			if (BssIdx >= MIN_NET_DEVICE_FOR_APCLI) {
				if (pEntry)
					BssIdx -= MIN_NET_DEVICE_FOR_APCLI;
				else {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							 ("RTMPAddWcidAttributeEntry: AP-Client link doesn't need to set Group WCID Attribute.\n"));
					return;
				}
			} else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
				if (BssIdx >= MIN_NET_DEVICE_FOR_WDS) {
					if (pEntry)
						BssIdx = BSS0;
					else {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
								 ("RTMPAddWcidAttributeEntry: WDS link doesn't need to set Group WCID Attribute.\n"));
						return;
					}
				} else
#endif /* WDS_SUPPORT */
				{
					if (BssIdx >= pAd->ApCfg.BssidNum) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 ("RTMPAddWcidAttributeEntry: The BSS-index(%d) is out of range for MBSSID link.\n", BssIdx));
						return;
					}
				}

			/* choose wcid number*/
			if (pEntry)
				Wcid = pEntry->wcid;
			else {
				wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
				GET_GroupKey_WCID(wdev, Wcid);
			}

#ifdef DOT1X_SUPPORT

			if ((BssIdx < pAd->ApCfg.BssidNum) && (BssIdx < MAX_MBSSID_NUM(pAd)) && (BssIdx < HW_BEACON_MAX_NUM)) {
				if (!pEntry) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPAddWcidAttributeEntry: pEntry is Null\n"));
					return;
				}

				IEEE8021X = pEntry->SecConfig.IEEE8021X;
			}

#endif /* DOT1X_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (BssIdx > BSS0) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("RTMPAddWcidAttributeEntry: The BSS-index(%d) is out of range for Infra link.\n", BssIdx));
				return;
			}

			/*
				1.	In ADHOC mode, the AID is wcid number. And NO mesh link exists.
				2.	In Infra mode, the AID:1 MUST be wcid of infra STA.
									the AID:2~ assign to mesh link entry.
			*/

			if (pEntry)
				Wcid = pEntry->wcid;
			else
				Wcid = MCAST_WCID_TO_REMOVE;
		}
#endif /* CONFIG_STA_SUPPORT */
	}

	/* Update WCID attribute table*/
	{
		UINT32 wcid_attr_base = 0, wcid_attr_size = 0;

		offset = wcid_attr_base + (Wcid * wcid_attr_size);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/*
				1.	Wds-links and Mesh-links always use Pair-wise key table.
				2.	When the CipherAlg is TKIP, AES or the dynamic WEP is enabled,
					it needs to set key into Pair-wise Key Table.
				3.	The pair-wise key security mode is set NONE, it means as no security.
			*/
			if (pEntry && (IS_ENTRY_WDS(pEntry) || IS_ENTRY_MESH(pEntry)))
				WCIDAttri = (BssIdx << 4) | (CipherAlg << 1) | PAIRWISEKEYTABLE;
			else if ((pEntry) &&
					 ((CipherAlg == CIPHER_TKIP) ||
					  (CipherAlg == CIPHER_AES) ||
					  (CipherAlg == CIPHER_NONE) ||
					  (IEEE8021X == TRUE)))
				WCIDAttri = (BssIdx << 4) | (CipherAlg << 1) | PAIRWISEKEYTABLE;
			else
				WCIDAttri = (BssIdx << 4) | (CipherAlg << 1) | SHAREDKEYTABLE;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (pEntry && IS_ENTRY_MESH(pEntry))
				WCIDAttri = (CipherAlg << 1) | PAIRWISEKEYTABLE;

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			else if (((pEntry) || IS_ENTRY_TDLS(pEntry)) &&
					 ((CipherAlg == CIPHER_TKIP) ||
					  (CipherAlg == CIPHER_AES) ||
					  (CipherAlg == CIPHER_NONE)))
				WCIDAttri = (CipherAlg << 1) | PAIRWISEKEYTABLE;

#endif /* defined(DOT11Z_TDLS_SUPPORT) */
			else
				WCIDAttri = (CipherAlg << 1) | SHAREDKEYTABLE;
		}
#endif /* CONFIG_STA_SUPPORT */
		RTMP_IO_WRITE32(pAd->hdev_ctrl, offset, WCIDAttri);
	}
	/* Update IV/EIV table*/
	{
		UINT32 iveiv_tb_base = 0, iveiv_tb_size = 0;

		offset = iveiv_tb_base + (Wcid * iveiv_tb_size);

		/* WPA mode*/
		if ((CipherAlg == CIPHER_TKIP) || (CipherAlg == CIPHER_AES)) {
			/* Eiv bit on. keyid always is 0 for pairwise key */
			IVEIV = (KeyIdx << 6) | 0x20;
		} else {
			/* WEP KeyIdx is default tx key. */
			IVEIV = (KeyIdx << 6);
		}

		/* For key index and ext IV bit, so only need to update the position(offset+3).*/
		RTMP_IO_WRITE8(pAd->hdev_ctrl, offset + 3, IVEIV);
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPAddWcidAttributeEntry: WCID #%d, KeyIndex #%d, Alg=%s\n",
			 Wcid, KeyIdx, CipherName[CipherAlg]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("	WCIDAttri = 0x%x\n",  WCIDAttri));
}

/*
    ==========================================================================
    Description:
	Parse encryption type
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
    ==========================================================================
*/
RTMP_STRING *GetEncryptType(CHAR enc)
{
	if (enc == Ndis802_11WEPDisabled)
		return "NONE";

	if (enc == Ndis802_11WEPEnabled)
		return "WEP";

	if (enc == Ndis802_11TKIPEnable)
		return "TKIP";

	if (enc == Ndis802_11AESEnable)
		return "AES";

	if (enc == Ndis802_11TKIPAESMix)
		return "TKIPAES";

	else
		return "UNKNOW";
}

RTMP_STRING *GetAuthMode(CHAR auth)
{
	if (auth == Ndis802_11AuthModeOpen)
		return "OPEN";

	if (auth == Ndis802_11AuthModeShared)
		return "SHARED";

	if (auth == Ndis802_11AuthModeAutoSwitch)
		return "AUTOWEP";

	if (auth == Ndis802_11AuthModeWPA)
		return "WPA";

	if (auth == Ndis802_11AuthModeWPAPSK)
		return "WPAPSK";

	if (auth == Ndis802_11AuthModeWPANone)
		return "WPANONE";

	if (auth == Ndis802_11AuthModeWPA2)
		return "WPA2";

	if (auth == Ndis802_11AuthModeWPA2PSK)
		return "WPA2PSK";

	if (auth == Ndis802_11AuthModeWPA1WPA2)
		return "WPA1WPA2";

	if (auth == Ndis802_11AuthModeWPA1PSKWPA2PSK)
		return "WPA1PSKWPA2PSK";

	return "UNKNOW";
}


/*
    ==========================================================================
    Description:
	Get site survey results
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
			1.) UI needs to wait 4 seconds after issue a site survey command
			2.) iwpriv ra0 get_site_survey
			3.) UI needs to prepare at least 4096bytes to get the results
    ==========================================================================
*/
#define	LINE_LEN	(4+33+20+23+9+7+7+3)	/* Channel+SSID+Bssid+Security+Signal+WiressMode+ExtCh+NetworkType*/

#ifdef CONFIG_STA_SUPPORT
#ifdef WSC_STA_SUPPORT
#define	WPS_LINE_LEN	(4+5)	/* WPS+DPID*/
#endif /* WSC_STA_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
#define DOT11R_LINE_LEN	(5+9+10)	/* MDId+FToverDS+RsrReqCap*/
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
VOID RTMPCommSiteSurveyData(
	IN  RTMP_STRING *msg,
	IN  BSS_ENTRY * pBss,
	IN  UINT32 MsgLen)
{
	INT         Rssi = 0;
	UINT        Rssi_Quality = 0;
	NDIS_802_11_NETWORK_TYPE    wireless_mode;
	CHAR		Ssid[MAX_LEN_OF_SSID + 1];
	RTMP_STRING SecurityStr[32] = {0};
	/*Channel*/
	sprintf(msg + strlen(msg), "%-4d", pBss->Channel);
	/*SSID*/
	NdisZeroMemory(Ssid, (MAX_LEN_OF_SSID + 1));

	if (RTMPCheckStrPrintAble((PCHAR)pBss->Ssid, pBss->SsidLen))
		NdisMoveMemory(Ssid, pBss->Ssid, pBss->SsidLen);
	else {
		INT idx = 0;

		sprintf(Ssid, "0x");

		for (idx = 0; (idx < 14) && (idx < pBss->SsidLen); idx++)
			sprintf(Ssid + 2 + (idx * 2), "%02X", (UCHAR)pBss->Ssid[idx]);
	}

	sprintf(msg + strlen(msg), "%-33s", Ssid);
	/*BSSID*/
	sprintf(msg + strlen(msg), "%02x:%02x:%02x:%02x:%02x:%02x   ",
			pBss->Bssid[0],
			pBss->Bssid[1],
			pBss->Bssid[2],
			pBss->Bssid[3],
			pBss->Bssid[4],
			pBss->Bssid[5]);
	/*Security*/
	RTMPZeroMemory(SecurityStr, 32);
	sprintf(SecurityStr, "%s/%s", GetAuthModeStr(pBss->AKMMap), GetEncryModeStr(pBss->PairwiseCipher));
	sprintf(msg + strlen(msg), "%-23s", SecurityStr);
	/* Rssi*/
	Rssi = (INT)pBss->Rssi;

	if (Rssi >= -50)
		Rssi_Quality = 100;
	else if (Rssi >= -80)    /* between -50 ~ -80dbm*/
		Rssi_Quality = (UINT)(24 + ((Rssi + 80) * 26) / 10);
	else if (Rssi >= -90)   /* between -80 ~ -90dbm*/
		Rssi_Quality = (UINT)(((Rssi + 90) * 26) / 10);
	else    /* < -84 dbm*/
		Rssi_Quality = 0;

	sprintf(msg + strlen(msg), "%-9d", Rssi_Quality);
	/* Wireless Mode*/
	wireless_mode = NetworkTypeInUseSanity(pBss);

	if (wireless_mode == Ndis802_11FH ||
		wireless_mode == Ndis802_11DS)
		sprintf(msg + strlen(msg), "%-7s", "11b");
	else if (wireless_mode == Ndis802_11OFDM5)
		sprintf(msg + strlen(msg), "%-7s", "11a");
	else if (wireless_mode == Ndis802_11OFDM5_N)
		sprintf(msg + strlen(msg), "%-7s", "11a/n");
	else if (wireless_mode == Ndis802_11OFDM5_AC)
		sprintf(msg + strlen(msg), "%-8s", "11a/n/ac");
	else if (wireless_mode == Ndis802_11OFDM24)
		sprintf(msg + strlen(msg), "%-7s", "11b/g");
	else if (wireless_mode == Ndis802_11OFDM24_N)
		sprintf(msg + strlen(msg), "%-7s", "11b/g/n");
	else
		sprintf(msg + strlen(msg), "%-7s", "unknow");

	/* Ext Channel*/
	if (pBss->AddHtInfoLen > 0) {
		if (pBss->AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE)
			sprintf(msg + strlen(msg), "%-7s", " ABOVE");
		else if (pBss->AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_BELOW)
			sprintf(msg + strlen(msg), "%-7s", " BELOW");
		else
			sprintf(msg + strlen(msg), "%-7s", " NONE");
	} else
		sprintf(msg + strlen(msg), "%-7s", " NONE");

	/*Network Type		*/
	if (pBss->BssType == BSS_ADHOC)
		sprintf(msg + strlen(msg), "%-3s", " Ad");
	else
		sprintf(msg + strlen(msg), "%-3s", " In");

	/* SSID Length */
	sprintf(msg + strlen(msg), " %-8d", pBss->SsidLen);

	sprintf(msg + strlen(msg), "\n");
	return;
}

static
BOOLEAN ascii2int(RTMP_STRING *in, UINT32 *out)
{
	UINT32 decimal_val, val;
	CHAR *p, asc_val;

	decimal_val = 0;
	p = (char *)in;

	while ((*p) != 0) {
		val = 0;
		asc_val = *p;

		if ((asc_val >= '0') && (asc_val <= '9'))
			val = asc_val - 48;
		else
			return FALSE;

		decimal_val = (decimal_val * 10) + val;
		p++;
	}

	*out = decimal_val;
	return TRUE;
}

#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
VOID RTMPIoctlGetSiteSurvey(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	RTMP_STRING *msg;
	INT		i = 0;
	INT			WaitCnt;
	INT		Status = 0;
	INT         max_len = LINE_LEN;
	RTMP_STRING *this_char;
	UINT32		bss_start_idx;
	BSS_ENTRY *pBss;
	UINT32 TotalLen, BufLen = IW_SCAN_MAX_DATA;
	POS_COOKIE pObj = (POS_COOKIE)pAdapter->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAdapter, pObj->ioctl_if, pObj->ioctl_if_type);
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAdapter, wdev);
#ifdef CONFIG_STA_SUPPORT
	STA_ADMIN_CONFIG *pApCliEntry = NULL;
#ifdef WSC_STA_SUPPORT
	max_len += WPS_LINE_LEN;
#endif /* WSC_STA_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	max_len += DOT11R_LINE_LEN;
#endif /* DOT11R_FT_SUPPORT */
	if (wdev->wdev_type == WDEV_TYPE_STA) {
		pApCliEntry = &pAdapter->StaCfg[wdev->func_idx];
		pApCliEntry->apcliNeedEnable = FALSE;
	}
#endif /*CONFIG_STA_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&this_char, wrq->u.data.length + 1);
	if (!this_char) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return;
	}

	if (copy_from_user(this_char, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: copy_from_user() fail!!!\n", __func__));
		os_free_mem(this_char);
		return;
	}
	this_char[wrq->u.data.length] = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Before check, this_char = %s\n"
			 , __func__, this_char));

	if (ascii2int(this_char, &bss_start_idx) == FALSE)
		bss_start_idx = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): After check, this_char = %s, out = %d\n"
			 , __func__, this_char, bss_start_idx));
	TotalLen = sizeof(CHAR) * ((MAX_LEN_OF_BSS_TABLE) * max_len) + 100;
	BufLen = IW_SCAN_MAX_DATA;
	os_alloc_mem(NULL, (PUCHAR *)&msg, TotalLen);

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPIoctlGetSiteSurvey - msg memory alloc fail.\n"));
		os_free_mem(this_char);
		return;
	}

	memset(msg, 0, TotalLen);

	if (ScanTab->BssNr == 0) {
		sprintf(msg, "No BssInfo\n");
		wrq->u.data.length = strlen(msg);
		Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPIoctlGetSiteSurvey - wrq->u.data.length = %d\n",
				 wrq->u.data.length));
		os_free_mem(this_char);
		os_free_mem((PUCHAR)msg);
		return;
	}

	if (bss_start_idx > (ScanTab->BssNr - 1)) {
		sprintf(msg, "BssInfo Idx(%d) is out of range(0~%d)\n",
				bss_start_idx, (ScanTab->BssNr - 1));
		wrq->u.data.length = strlen(msg);
		Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPIoctlGetSiteSurvey - wrq->u.data.length = %d\n",
				 wrq->u.data.length));
		os_free_mem((PUCHAR)msg);
		os_free_mem(this_char);
		return;
	}

	sprintf(msg, "%s", "\n");
	sprintf(msg + strlen(msg), "Total=%-4d", ScanTab->BssNr);
	sprintf(msg + strlen(msg), "%s", "\n");
	sprintf(msg + strlen(msg), "%-4s%-4s%-33s%-20s%-23s%-9s%-7s%-7s%-3s%-8s\n",
			"No", "Ch", "SSID", "BSSID", "Security", "Siganl(%)", "W-Mode", " ExtCH", " NT", " SSID_Len");
#ifdef WSC_INCLUDED
	sprintf(msg + strlen(msg) - 1, "%-4s%-5s\n", " WPS", " DPID");
#endif /* WSC_INCLUDED */
	sprintf(msg + strlen(msg) - 1, "%-10s\n", " BcnRept");
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	sprintf(msg + strlen(msg) - 1, "%-5s%-9s%-10s\n", " MDId", " FToverDS", " RsrReqCap");
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	WaitCnt = 0;
#ifdef CONFIG_STA_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA)
	pAdapter->StaCfg[wdev->func_idx].bSkipAutoScanConn = TRUE;
#endif /* CONFIG_STA_SUPPORT */

		while ((scan_in_run_state(pAdapter, wdev) == TRUE) && (WaitCnt++ < 200)) {
#ifdef CONFIG_STA_SUPPORT
			if (pApCliEntry && (pApCliEntry->ApcliInfStat.Enable == TRUE)) {
				Set_ApCli_Enable_Proc(pAdapter, "0");
				pApCliEntry->apcliNeedEnable = TRUE;
			}
#endif /* CONFIG_STA_SUPPORT */
			OS_WAIT(500);
		}


	for (i = bss_start_idx; i < ScanTab->BssNr; i++) {
		pBss = &ScanTab->BssEntry[i];

		if (pBss->Channel == 0)
			break;

		if ((strlen(msg) + 100) >= BufLen)
			break;

		/*No*/
		sprintf(msg + strlen(msg), "%-4d", i);
		RTMPCommSiteSurveyData(msg, pBss, TotalLen);
#ifdef WSC_INCLUDED

		/*WPS*/
		if (pBss->WpsAP & 0x01)
			sprintf(msg + strlen(msg) - 1, "%-4s", " YES");
		else
			sprintf(msg + strlen(msg) - 1, "%-4s", "  NO");

		if (pBss->WscDPIDFromWpsAP == DEV_PASS_ID_PIN)
			sprintf(msg + strlen(msg), "%-5s", " PIN");
		else if (pBss->WscDPIDFromWpsAP == DEV_PASS_ID_PBC)
			sprintf(msg + strlen(msg), "%-5s", " PBC");
		else
			sprintf(msg + strlen(msg), "%-5s", " ");

#endif /* WSC_INCLUDED */
		sprintf(msg + strlen(msg), "%-7s\n", pBss->FromBcnReport ? " YES" : " NO");
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT

		if (pBss->bHasMDIE) {
			sprintf(msg + strlen(msg) - 1, " %02x%02x", pBss->FT_MDIE.MdId[0], pBss->FT_MDIE.MdId[1]);

			if (pBss->FT_MDIE.FtCapPlc.field.FtOverDs)
				sprintf(msg + strlen(msg), "%-9s", " TRUE");
			else
				sprintf(msg + strlen(msg), "%-9s", " FALSE");

			if (pBss->FT_MDIE.FtCapPlc.field.RsrReqCap)
				sprintf(msg + strlen(msg), "%-10s\n", " TRUE");
			else
				sprintf(msg + strlen(msg), "%-10s\n", " FALSE");
		}

#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	}

#ifdef CONFIG_STA_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA)
		pAdapter->StaCfg[wdev->func_idx].bSkipAutoScanConn = FALSE;
#endif /* CONFIG_STA_SUPPORT */
	wrq->u.data.length = strlen(msg);
#ifdef CONFIG_STA_SUPPORT
	if (pApCliEntry && (pApCliEntry->apcliNeedEnable == TRUE)) {
		Set_ApCli_Enable_Proc(pAdapter, "1");
		pApCliEntry->apcliNeedEnable = FALSE;
	}
#endif /*CONFIG_STA_SUPPORT*/
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPIoctlGetSiteSurvey - wrq->u.data.length = %d\n",
			 wrq->u.data.length));
	os_free_mem((PUCHAR)msg);
	os_free_mem(this_char);
}
#endif

USHORT RTMPGetLastTxRate(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *pEntry)
{
	HTTRANSMIT_SETTING lastTxRate;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;

	MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
	lastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
	lastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
	lastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
	lastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
	lastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

	if (lastTxRate.field.MODE == MODE_VHT)
		lastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
	else if (lastTxRate.field.MODE == MODE_OFDM)
		lastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
	else
		lastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

#else
	lastTxRate.word = pEntry->HTPhyMode.word;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	return lastTxRate.word;
}

VOID RTMPIoctlGetMacTableStaInfo(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT i;
	RT_802_11_MAC_TABLE *pMacTab = NULL;
	PRT_802_11_MAC_ENTRY pDst;
	MAC_TABLE_ENTRY *pEntry;
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pMacTab, sizeof(RT_802_11_MAC_TABLE));

	if (pMacTab == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return;
	}

	NdisZeroMemory(pMacTab, sizeof(RT_802_11_MAC_TABLE));

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &(pAd->MacTab.Content[i]);

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
			pDst = &pMacTab->Entry[pMacTab->Num];
			pDst->ApIdx = pEntry->func_tb_idx;
			COPY_MAC_ADDR(pDst->Addr, &pEntry->Addr);
			pDst->Aid = (UCHAR)pEntry->Aid;
			pDst->Psm = pEntry->PsMode;
#ifdef DOT11_N_SUPPORT
			pDst->MimoPs = pEntry->MmpsMode;
#endif /* DOT11_N_SUPPORT */
			/* Fill in RSSI per entry*/
			pDst->AvgRssi0 = pEntry->RssiSample.AvgRssi[0];
			pDst->AvgRssi1 = pEntry->RssiSample.AvgRssi[1];
			pDst->AvgRssi2 = pEntry->RssiSample.AvgRssi[2];
			/* the connected time per entry*/
			pDst->ConnectedTime = pEntry->StaConnectTime;
			pDst->TxRate.word = RTMPGetLastTxRate(pAd, pEntry);
			pDst->LastRxRate = pEntry->LastRxRate;
			pMacTab->Num += 1;
		}
	}

	wrq->u.data.length = sizeof(RT_802_11_MAC_TABLE);

	if (copy_to_user(wrq->u.data.pointer, pMacTab, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: copy_to_user() fail\n", __func__));

	if (pMacTab != NULL)
		os_free_mem(pMacTab);
}

VOID RTMPIoctlGetDriverInfo(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	RTMP_STRING *msg;
	UINT32 TotalLen = 4096;

	os_alloc_mem(NULL, (PUCHAR *)&msg, TotalLen);
	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("RTMPIoctlGetDriverInfo - msg memory alloc fail.\n"));
		return;
	}

	NdisZeroMemory(msg, TotalLen);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		sprintf(msg, "Driver version: %s \n", AP_DRIVER_VERSION);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		sprintf(msg+strlen(msg), "Driver version: %s \n", STA_DRIVER_VERSION);
#endif /* CONFIG_STA_SUPPORT */

	sprintf(msg+strlen(msg), "FW ver: 0x%x, HW ver: 0x%x, CHIP ID: 0x%x\n",
			pAd->FWVersion, pAd->HWVersion, pAd->ChipID);

	wrq->u.data.length = strlen(msg);
	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: copy_to_user() fail\n", __func__));

	os_free_mem(msg);
}


#define	MAC_LINE_LEN	(1+14+4+4+4+4+10+10+10+6+6)	/* "\n"+Addr+aid+psm+datatime+rxbyte+txbyte+current tx rate+last tx rate+"\n" */
VOID RTMPIoctlGetMacTable(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT i;
	/*	RT_802_11_MAC_TABLE MacTab;*/
	RT_802_11_MAC_TABLE *pMacTab = NULL;
	RT_802_11_MAC_ENTRY *pDst;
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;
	char *msg;
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pMacTab, sizeof(RT_802_11_MAC_TABLE));

	if (pMacTab == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return;
	}

	NdisZeroMemory(pMacTab, sizeof(RT_802_11_MAC_TABLE));
	pMacTab->Num = 0;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &(pAd->MacTab.Content[i]);
		tr_entry = &(pAd->MacTab.tr_entry[i]);
		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
		{
			pDst = &pMacTab->Entry[pMacTab->Num];
			pDst->ApIdx = (UCHAR)pEntry->func_tb_idx;
			COPY_MAC_ADDR(pDst->Addr, &pEntry->Addr);
			pDst->Aid = (UCHAR)pEntry->Aid;
			pDst->Psm = pEntry->PsMode;
#ifdef DOT11_N_SUPPORT
			pDst->MimoPs = pEntry->MmpsMode;
#endif /* DOT11_N_SUPPORT */
			/* Fill in RSSI per entry*/
			pDst->AvgRssi0 = pEntry->RssiSample.AvgRssi[0];
			pDst->AvgRssi1 = pEntry->RssiSample.AvgRssi[1];
			pDst->AvgRssi2 = pEntry->RssiSample.AvgRssi[2];
			/* the connected time per entry*/
			pDst->ConnectedTime = pEntry->StaConnectTime;
			pDst->TxRate.word = RTMPGetLastTxRate(pAd, pEntry);
#ifdef RTMP_RBUS_SUPPORT

			if (IS_RBUS_INF(pAd))
				pDst->LastRxRate = pEntry->LastRxRate;

#endif /* RTMP_RBUS_SUPPORT */
			pMacTab->Num += 1;
		}
	}

	wrq->u.data.length = sizeof(RT_802_11_MAC_TABLE);

	if (copy_to_user(wrq->u.data.pointer, pMacTab, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: copy_to_user() fail\n", __func__));

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR) * (GET_MAX_UCAST_NUM(pAd)*MAC_LINE_LEN));

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Alloc memory failed\n", __func__));
		goto LabelOK;
	}

	memset(msg, 0, GET_MAX_UCAST_NUM(pAd)*MAC_LINE_LEN);
	sprintf(msg, "%s", "\n");
	sprintf(msg + strlen(msg), "%-14s%-4s%-4s%-4s%-4s%-6s%-6s%-10s%-10s%-10s\n",
			"MAC", "AP",  "AID", "PSM", "AUTH", "CTxR", "LTxR", "LDT", "RxB", "TxB");

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
		{
			if ((strlen(msg) + MAC_LINE_LEN) >= (GET_MAX_UCAST_NUM(pAd)*MAC_LINE_LEN))
				break;

			sprintf(msg + strlen(msg), "%02x%02x%02x%02x%02x%02x  ", PRINT_MAC(pEntry->Addr));
			sprintf(msg + strlen(msg), "%-4d", (int)pEntry->func_tb_idx);
			sprintf(msg + strlen(msg), "%-4d", (int)pEntry->Aid);
			sprintf(msg + strlen(msg), "%-4d", (int)pEntry->PsMode);
			sprintf(msg + strlen(msg), "%-4d", (int)pEntry->AuthState);
			sprintf(msg + strlen(msg), "%-6d", RateIdToMbps[pAd->MacTab.Content[i].CurrTxRate]);
			sprintf(msg + strlen(msg), "%-6d", 0/*RateIdToMbps[pAd->MacTab.Content[i].HTPhyMode.word]*/); /* ToDo*/
			sprintf(msg + strlen(msg), "%-10d", 0/*pAd->MacTab.Content[i].HSCounter.LastDataPacketTime*/); /* ToDo*/
			sprintf(msg + strlen(msg), "%-10d", 0/*pAd->MacTab.Content[i].HSCounter.TotalRxByteCount*/); /* ToDo*/
			sprintf(msg + strlen(msg), "%-10d\n", 0/*pAd->MacTab.Content[i].HSCounter.TotalTxByteCount*/); /* ToDo*/
		}
	}

	/* for compatible with old API just do the printk to console*/
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s", msg));
	os_free_mem(msg);
LabelOK:

	if (pMacTab != NULL)
		os_free_mem(pMacTab);
}

#if defined(INF_AR9) || defined(BB_SOC)
#if defined(AR9_MAPI_SUPPORT) || defined(BB_SOC)
#ifdef CONFIG_AP_SUPPORT
VOID RTMPAR9IoctlGetMacTable(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT i;
	char *msg;

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR) * (GET_MAX_UCAST_NUM(pAd)*MAC_LINE_LEN));

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Alloc memory failed\n", __func__));
		return;
	}

	memset(msg, 0, GET_MAX_UCAST_NUM(pAd) * MAC_LINE_LEN);
	sprintf(msg, "%s", "\n");
	sprintf(msg + strlen(msg), "%-14s%-4s%-4s%-4s%-4s%-6s%-6s%-10s%-10s%-10s\n",
			"MAC", "AP",  "AID", "PSM", "AUTH", "CTxR", "LTxR", "LDT", "RxB", "TxB");

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
			if ((strlen(msg) + MAC_LINE_LEN) >= (GET_MAX_UCAST_NUM(pAd) * MAC_LINE_LEN))
				break;

			sprintf(msg + strlen(msg), "%02x%02x%02x%02x%02x%02x  ",
					pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
					pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);
			sprintf(msg + strlen(msg), "%-4d", (int)pEntry->func_tb_idx);
			sprintf(msg + strlen(msg), "%-4d", (int)pEntry->Aid);
			sprintf(msg + strlen(msg), "%-4d", (int)pEntry->PsMode);
			sprintf(msg + strlen(msg), "%-4d", (int)pEntry->AuthState);
			sprintf(msg + strlen(msg), "%-6d", RateIdToMbps[pAd->MacTab.Content[i].CurrTxRate]);
			sprintf(msg + strlen(msg), "%-6d", 0/*RateIdToMbps[pAd->MacTab.Content[i].HTPhyMode.word]*/); /* ToDo*/
			sprintf(msg + strlen(msg), "%-10d", 0/*pAd->MacTab.Content[i].HSCounter.LastDataPacketTime*/); /* ToDo*/
			sprintf(msg + strlen(msg), "%-10d", 0/*pAd->MacTab.Content[i].HSCounter.TotalRxByteCount*/); /* ToDo*/
			sprintf(msg + strlen(msg), "%-10d\n", 0/*pAd->MacTab.Content[i].HSCounter.TotalTxByteCount*/); /* ToDo*/
		}
	}

	/* for compatible with old API just do the printk to console*/
	wrq->u.data.length = strlen(msg);

	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s", msg));

	os_free_mem(msg);
}

VOID RTMPIoctlGetSTAT2(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	char *msg;
	BSS_STRUCT *pMbss;
	INT apidx;

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR) * (pAd->ApCfg.BssidNum * (14 * 128)));

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Alloc memory failed\n", __func__));
		return;
	}

	memset(msg, 0, pAd->ApCfg.BssidNum * (14 * 128));
	sprintf(msg, "%s", "\n");

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		pMbss = &pAd->ApCfg.MBSSID[apidx];
		sprintf(msg + strlen(msg), "ra%d\n", apidx);
		sprintf(msg + strlen(msg), "bytesTx = %ld\n", (pMbss->TransmittedByteCount));
		sprintf(msg + strlen(msg), "bytesRx = %ld\n", (pMbss->ReceivedByteCount));
		sprintf(msg + strlen(msg), "pktsTx = %ld\n", pMbss->TxCount);
		sprintf(msg + strlen(msg), "pktsRx = %ld\n", pMbss->RxCount);
		sprintf(msg + strlen(msg), "errorsTx = %ld\n", pMbss->TxErrorCount);
		sprintf(msg + strlen(msg), "errorsRx = %ld\n", pMbss->RxErrorCount);
		sprintf(msg + strlen(msg), "discardPktsTx = %ld\n", pMbss->TxDropCount);
		sprintf(msg + strlen(msg), "discardPktsRx = %ld\n", pMbss->RxDropCount);
		sprintf(msg + strlen(msg), "ucPktsTx = %ld\n", pMbss->ucPktsTx);
		sprintf(msg + strlen(msg), "ucPktsRx = %ld\n", pMbss->ucPktsRx);
		sprintf(msg + strlen(msg), "mcPktsTx = %ld\n", pMbss->mcPktsTx);
		sprintf(msg + strlen(msg), "mcPktsRx = %ld\n", pMbss->mcPktsRx);
		sprintf(msg + strlen(msg), "bcPktsTx = %ld\n", pMbss->bcPktsTx);
		sprintf(msg + strlen(msg), "bcPktsRx = %ld\n", pMbss->bcPktsRx);
	}

	wrq->u.data.length = strlen(msg);

	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s", msg));

	os_free_mem(msg);
}

VOID RTMPIoctlGetRadioDynInfo(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	char *msg;
	BSS_STRUCT *pMbss;
	INT status, bandwidth;
	struct wifi_dev *wdev;
	UCHAR op_ht_bw;
	UCHAR ht_gi;

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR) * (4096));

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Alloc memory failed\n", __func__));
		return;
	}

	memset(msg, 0, 4096);
	sprintf(msg, "%s", "\n");
	pMbss = &pAd->ApCfg.MBSSID[0];
	wdev = &pMbss->wdev;
	op_ht_bw = wlan_operate_get_ht_bw(wdev);

	if (IsHcRadioCurStatOffByWdev(wdev))
		status = 0;
	else
		status = 1;

	if (op_ht_bw  == BW_40)
		bandwidth = 1;
	else
		bandwidth = 0;

	ht_gi = wlan_config_get_ht_gi(wdev);
	sprintf(msg + strlen(msg), "status = %d\n", status);
	sprintf(msg + strlen(msg), "channelsInUse = %d\n", pAd->ChannelListNum);
	sprintf(msg + strlen(msg), "channel = %d\n", wdev->channel);
	sprintf(msg + strlen(msg), "chanWidth = %d\n", bandwidth);
	sprintf(msg + strlen(msg), "guardIntvl = %d\n", ht_gi);
	sprintf(msg + strlen(msg), "MCS = %d\n", wdev->DesiredTransmitSetting.field.MCS);
	wrq->u.data.length = strlen(msg);

	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s", msg));

	os_free_mem(msg);
}
#endif/*CONFIG_AP_SUPPORT*/
#endif/*AR9_MAPI_SUPPORT*/
#endif/* INF_AR9 */

INT Set_AP_SlotTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx = 0, SlotTime;
	struct wifi_dev *wdev = NULL;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Incorrect BSS!!\n",  __func__));
		return FALSE;
	}
	if (wdev->channel > 14) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:5G only support slottime 9us\n",  __func__));
		return FALSE;
	}
	SlotTime = (UINT8) os_str_tol(arg, 0, 10);
	if ((SlotTime < 9) || (SlotTime > 25)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Invalid arguments! 9~25\n",  __func__));
		return FALSE;
	}

	if (SlotTime == 9)
		wdev->bUseShortSlotTime = TRUE;
	else
		wdev->bUseShortSlotTime = FALSE;

	wdev->SlotTimeValue = SlotTime;
	HW_SET_SLOTTIME(pAd, wdev->bUseShortSlotTime, wdev->channel, wdev);

	return TRUE;
}

#ifdef DOT11_N_SUPPORT
INT	Set_BASetup_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR mac[6], tid;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	/*
		The BASetup inupt string format should be xx:xx:xx:xx:xx:xx-d,
			=>The six 2 digit hex-decimal number previous are the Mac address,
			=>The seventh decimal number is the tid value.
	*/
	/*MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n%s\n", arg));*/

	if (strlen(arg) <
		19) /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.*/
		return FALSE;

	token = strchr(arg, DASH);

	if ((token != NULL) && (strlen(token) > 1)) {
		tid = (UCHAR) os_str_tol((token + 1), 0, 10);

		if (tid > (NUM_OF_TID - 1))
			return FALSE;

		*token = '\0';

		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++) {
			if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
				return FALSE;

			AtoH(token, (&mac[i]), 1);
		}

		if (i != 6)
			return FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
				 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], tid));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pEntry = MacTableLookup(pAd, (PUCHAR) mac);
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pEntry = MacTableLookup2(pAd, (PUCHAR) mac, NULL);
#endif

		if (pEntry) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nSetup BA Session: Tid = %d\n", tid));
			ba_ori_session_setup(pAd, pEntry, tid, 0, 100, TRUE);
		}

		return TRUE;
	}

	return FALSE;
}

INT	Set_BADecline_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG bBADecline;

	bBADecline = os_str_tol(arg, 0, 10);

	if (bBADecline == 0)
		pAd->CommonCfg.bBADecline = FALSE;
	else if (bBADecline == 1)
		pAd->CommonCfg.bBADecline = TRUE;
	else {
		return FALSE; /*Invalid argument*/
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_BADecline_Proc::(BADecline=%d)\n",
			 pAd->CommonCfg.bBADecline));
	return TRUE;
}

INT	Set_BAOriTearDown_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR mac[6], tid;
	UCHAR wcid;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	/*MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n%s\n", arg));*/
	/*
		The BAOriTearDown inupt string format should be xx:xx:xx:xx:xx:xx-d,
			=>The six 2 digit hex-decimal number previous are the Mac address,
			=>The seventh decimal number is the tid value.
	*/
	if (strlen(arg) <
		19) { /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.*/
		/* another acceptable format wcid-tid */
		token = strchr(arg, DASH);

		if ((token != NULL) && (strlen(token) > 1)) {
			tid = os_str_tol((token + 1), 0, 10);

			if (tid > (NUM_OF_TID - 1)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tid=%d is wrong\n\r", tid));
				return FALSE;
			}

			*token = '\0';
			wcid = os_str_tol(arg, 0, 10);

			if (wcid >= 128) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wcid=%d is wrong\n\r", wcid));
				return FALSE;
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tear down ori ba,wcid=%d,tid=%d\n\r", wcid, tid));
			ba_ori_session_tear_down(pAd, wcid, tid, FALSE, TRUE);
			return TRUE;
		}

		return FALSE;
	}

	token = strchr(arg, DASH);

	if ((token != NULL) && (strlen(token) > 1)) {
		tid = os_str_tol((token + 1), 0, 10);

		if (tid > (NUM_OF_TID - 1))
			return FALSE;

		*token = '\0';

		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++) {
			if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
				return FALSE;

			AtoH(token, (&mac[i]), 1);
		}

		if (i != 6)
			return FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x",
				 PRINT_MAC(mac), tid));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pEntry = MacTableLookup(pAd, (PUCHAR) mac);
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pEntry = MacTableLookup2(pAd, (PUCHAR) mac, NULL);
#endif

		if (pEntry) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nTear down Ori BA Session: Tid = %d\n", tid));
			ba_ori_session_tear_down(pAd, pEntry->wcid, tid, FALSE, TRUE);
		}

		return TRUE;
	}

	return FALSE;
}

INT	Set_BARecTearDown_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR mac[6], tid;
	UCHAR wcid;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	/*MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n%s\n", arg));*/
	/*
		The BARecTearDown inupt string format should be xx:xx:xx:xx:xx:xx-d,
			=>The six 2 digit hex-decimal number previous are the Mac address,
			=>The seventh decimal number is the tid value.
	*/
	if (strlen(arg) <
		19) { /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.*/
		/* another acceptable format wcid-tid */
		token = strchr(arg, DASH);

		if ((token != NULL) && (strlen(token) > 1)) {
			tid = os_str_tol((token + 1), 0, 10);

			if (tid > (NUM_OF_TID - 1)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tid=%d is wrong\n\r", tid));
				return FALSE;
			}

			*token = '\0';
			wcid = os_str_tol(arg, 0, 10);

			if (wcid >= 128) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wcid=%d is wrong\n\r", wcid));
				return FALSE;
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tear down rec ba,wcid=%d,tid=%d\n\r", wcid, tid));
			ba_rec_session_tear_down(pAd, wcid, tid, FALSE);
			return TRUE;
		}

		return FALSE;
	}

	token = strchr(arg, DASH);

	if ((token != NULL) && (strlen(token) > 1)) {
		tid = os_str_tol((token + 1), 0, 10);

		if (tid > (NUM_OF_TID - 1))
			return FALSE;

		*token = '\0';

		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++) {
			if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
				return FALSE;

			AtoH(token, (&mac[i]), 1);
		}

		if (i != 6)
			return FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x",
				 PRINT_MAC(mac), tid));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pEntry = MacTableLookup(pAd, (PUCHAR) mac);
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pEntry = MacTableLookup2(pAd, (PUCHAR) mac, NULL);
#endif

		if (pEntry) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nTear down Rec BA Session: Tid = %d\n", tid));
			ba_rec_session_tear_down(pAd, pEntry->wcid, tid, FALSE);
		}

		return TRUE;
	}

	return FALSE;
}

INT	Set_HtBw_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *tdev;
	UCHAR Bandidx = 0;
	UCHAR i = 0;
	ULONG HtBw;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Bandidx = HcGetBandByWdev(wdev);
	HtBw = os_str_tol(arg, 0, 10);
	if ((HtBw != BW_40) && (HtBw != BW_20))
		return FALSE;  /*Invalid argument */

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev && (Bandidx == HcGetBandByWdev(tdev))) {
			if (HtBw == BW_40) {
				wlan_config_set_ht_bw(tdev, BW_40);
				wlan_operate_set_ht_bw(tdev, HT_BW_40, wlan_operate_get_ext_cha(tdev));
			} else if (HtBw == BW_20) {
				wlan_config_set_ht_bw(tdev, BW_20);
				wlan_operate_set_ht_bw(tdev, HT_BW_20, EXTCHA_NONE);
			}
			SetCommonHtVht(pAd, tdev);
		}
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtBw_Proc::(HtBw=%d)\n", wlan_config_get_ht_bw(wdev)));
	return TRUE;
}

INT	Set_HtMcs_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN bAutoRate = FALSE;
#endif /* CONFIG_STA_SUPPORT */
	UCHAR HtMcs = MCS_AUTO, Mcs_tmp, ValidMcs = 15;
#ifdef DOT11_VHT_AC
	RTMP_STRING *mcs_str, *ss_str;
	UCHAR ss = 0, mcs = 0;
#endif /* DOT11_VHT_AC */
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

#ifdef DOT11_VHT_AC
	if (!wdev)
		return FALSE;

	ss_str = arg;
	mcs_str = rtstrchr(arg, ':');

	if (mcs_str != NULL) {
		*mcs_str = 0;
		mcs_str++;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): ss_str=%s, mcs_str=%s\n",
				 __func__, ss_str, mcs_str));

		if (strlen(ss_str) && strlen(mcs_str)) {
			mcs = os_str_tol(mcs_str, 0, 10);
			ss = os_str_tol(ss_str, 0, 10);

			if ((ss <= wlan_operate_get_tx_stream(wdev)) && (mcs <= 7))
				HtMcs = ((ss - 1) << 4) | mcs;
			else {
				HtMcs = MCS_AUTO;
				ss = 0;
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): %dSS-MCS%d, Auto=%s\n",
					 __func__, ss, mcs,
					 (HtMcs == MCS_AUTO && ss == 0) ? "TRUE" : "FALSE"));
			Set_FixedTxMode_Proc(pAd, "VHT");
		}
	} else
#endif /* DOT11_VHT_AC */
	{
		Mcs_tmp = os_str_tol(arg, 0, 10);

		if (Mcs_tmp <= ValidMcs || Mcs_tmp == 32)
			HtMcs = Mcs_tmp;
		else
			HtMcs = MCS_AUTO;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
		wdev->DesiredTransmitSetting.field.MCS = HtMcs;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtMcs_Proc::(HtMcs=%d) for ra%d\n",
				 wdev->DesiredTransmitSetting.field.MCS, pObj->ioctl_if));
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		MAC_TABLE_ENTRY *pEntry = NULL;

		wdev = &pAd->StaCfg[pObj->ioctl_if].wdev;
		pEntry = GetAssociatedAPByWdev(pAd, wdev);
		ASSERT(pEntry);
		wdev->DesiredTransmitSetting.field.MCS = HtMcs;
		wdev->bAutoTxRateSwitch = (HtMcs == MCS_AUTO) ? TRUE : FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtMcs_Proc::(HtMcs=%d, bAutoTxRateSwitch = %d)\n",
				 wdev->DesiredTransmitSetting.field.MCS, wdev->bAutoTxRateSwitch));

		if ((!WMODE_CAP_N(wdev->PhyMode)) ||
			(pEntry->HTPhyMode.field.MODE < MODE_HTMIX)) {
			if ((wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO) &&
				(HtMcs <= 3) &&
				(wdev->DesiredTransmitSetting.field.FixedTxMode == FIXED_TXMODE_CCK))
				RTMPSetDesiredRates(pAd, wdev, (LONG) (RateIdToMbps[HtMcs] * 1000000));
			else if ((wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO) &&
					 (HtMcs <= 7) &&
					 (wdev->DesiredTransmitSetting.field.FixedTxMode == FIXED_TXMODE_OFDM))
				RTMPSetDesiredRates(pAd, wdev, (LONG) (RateIdToMbps[HtMcs + 4] * 1000000));
			else
				bAutoRate = TRUE;

			if (bAutoRate) {
				wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
				RTMPSetDesiredRates(pAd, wdev, -1);
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtMcs_Proc::(FixedTxMode=%d)\n",
					 wdev->DesiredTransmitSetting.field.FixedTxMode));
		}

		if (ADHOC_ON(pAd))
			return TRUE;
	}
#endif /* CONFIG_STA_SUPPORT */
	SetCommonHtVht(pAd, wdev);
#ifdef WFA_VHT_PF
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		INT idx;

		NdisAcquireSpinLock(&pAd->MacTabLock);

		/* TODO:Carter, check why start from 1 */
		for (idx = 1; VALID_UCAST_ENTRY_WCID(pAd, idx); idx++) {
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[idx];

			if (IS_ENTRY_CLIENT(pEntry) && (pEntry->func_tb_idx == pObj->ioctl_if)) {
				if ((HtMcs == MCS_AUTO) &&  ss == 0) {
					UCHAR TableSize = 0;

					pEntry->bAutoTxRateSwitch = TRUE;
					/* TODO: MlmeSelectTxRateTable() and  MlmeNewTxRate() are not supported on MT_MAC */
					MlmeSelectTxRateTable(pAd, pEntry, &pEntry->pTable, &TableSize, &pEntry->CurrTxRateIndex);
					MlmeNewTxRate(pAd, pEntry);
#ifdef NEW_RATE_ADAPT_SUPPORT

					if (!ADAPT_RATE_TABLE(pEntry->pTable))
#endif /* NEW_RATE_ADAPT_SUPPORT */
						pEntry->HTPhyMode.field.ShortGI = GI_800;
				} else {
					pEntry->HTPhyMode.field.MCS = pMbss->HTPhyMode.field.MCS;
					pEntry->bAutoTxRateSwitch = FALSE;
					/* If the legacy mode is set, overwrite the transmit setting of this entry. */
					RTMPUpdateLegacyTxSetting((UCHAR)pMbss->DesiredTransmitSetting.field.FixedTxMode, pEntry);
				}
			}
		}

		NdisReleaseSpinLock(&pAd->MacTabLock);
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* WFA_VHT_PF */
	return TRUE;
}

INT	Set_HtGi_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG HtGi;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	HtGi = os_str_tol(arg, 0, 10);

	if ((HtGi != GI_400) && (HtGi != GI_800))
		return FALSE;

	wlan_config_set_ht_gi(wdev, HtGi);
	SetCommonHtVht(pAd, wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Set_HtGi_Proc::(ShortGI=%d)\n", wlan_config_get_ht_gi(wdev)));
	return TRUE;
}

INT	Set_HtTxBASize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Size;

	Size = os_str_tol(arg, 0, 10);

	if (Size <= 0 || Size >= 64)
		Size = 8;

	pAd->CommonCfg.TxBASize = Size - 1;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_HtTxBASize ::(TxBASize= %d)\n", Size));
	return TRUE;
}

INT	Set_HtDisallowTKIP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 1)
		pAd->CommonCfg.HT_DisallowTKIP = TRUE;
	else
		pAd->CommonCfg.HT_DisallowTKIP = FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtDisallowTKIP_Proc ::%s\n",
			 (pAd->CommonCfg.HT_DisallowTKIP == TRUE) ? "enabled" : "disabled"));
	return TRUE;
}

INT	Set_HtOpMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value == HTMODE_GF)
		pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_GF;
	else if (Value == HTMODE_MM)
		pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_MM;
	else
		return FALSE; /*Invalid argument */

	SetCommonHtVht(pAd, wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtOpMode_Proc::(HtOpMode=%d)\n",
			 pAd->CommonCfg.RegTransmitSetting.field.HTMODE));
	return TRUE;
}

INT	Set_HtLdpc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value > 1 || Value < 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Invalid arguments!\n", __func__));
		return FALSE;
	}

	wlan_config_set_ht_ldpc(wdev, (UCHAR)Value);
	wlan_operate_set_ht_ldpc(wdev, (UCHAR)Value);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(HtLdpc=%d)\n",
		__func__, wlan_config_get_ht_ldpc(wdev)));
	return TRUE;
}


INT	Set_HtStbc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value == STBC_USE)
		wlan_config_set_ht_stbc(wdev, STBC_USE);
	else if (Value == STBC_NONE)
		wlan_config_set_ht_stbc(wdev, STBC_NONE);
	else
		return FALSE; /*Invalid argument */

	SetCommonHtVht(pAd, wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_Stbc_Proc::(HtStbc=%d)\n", wlan_config_get_ht_stbc(wdev)));
	return TRUE;
}

/*configure useage*/
INT	set_extcha_for_wdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR value)
{
	value = value ? EXTCHA_ABOVE : EXTCHA_BELOW;
	wlan_config_set_ext_cha(wdev, value);
	SetCommonHtVht(pAd, wdev);
	return TRUE;
}

INT	Set_HtExtcha_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *tdev;
	UCHAR Bandidx = 0;
	UCHAR i = 0;
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR ext_cha;

	if (!wdev)
		return FALSE;

	Bandidx = HcGetBandByWdev(wdev);
	Value = os_str_tol(arg, 0, 10);

	if (Value != 0 && Value != 1)
		return FALSE;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev && (Bandidx == HcGetBandByWdev(tdev)))
			set_extcha_for_wdev(pAd, tdev, Value);
	}
	ext_cha = wlan_config_get_ext_cha(wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtExtcha_Proc::(HtExtcha=%d)\n", ext_cha));
	return TRUE;
}

INT	Set_HtMpduDensity_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	Value = os_str_tol(arg, 0, 10);

	if (!wdev)
		return FALSE;

	if (Value <= 7)
		wlan_config_set_min_mpdu_start_space(wdev, Value);
	else
		wlan_config_set_min_mpdu_start_space(wdev, MPDU_DENSITY_NO_RESTRICT);

	SetCommonHtVht(pAd, NULL);

	Value = wlan_config_get_min_mpdu_start_space(wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("Set_HtMpduDensity_Proc::(HtMpduDensity=%d)\n", (UCHAR)Value));
	return TRUE;
}

INT	Set_HtBaWinSize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);
#ifdef CONFIG_AP_SUPPORT
	/* Intel IOT*/
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	Value = 64;
#endif /* CONFIG_AP_SUPPORT */

	if (Value >= 1 && Value <= 64) {
		pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = Value;
		pAd->CommonCfg.BACapability.field.RxBAWinLimit = Value;
#ifdef COEX_SUPPORT
		pAd->CommonCfg.REGBACapability.field.TxBAWinLimit = Value;
		pAd->CommonCfg.BACapability.field.TxBAWinLimit = Value;
#endif /* COEX_SUPPORT */

	} else {
		pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = 64;
		pAd->CommonCfg.BACapability.field.RxBAWinLimit = 64;
#ifdef COEX_SUPPORT
		pAd->CommonCfg.REGBACapability.field.TxBAWinLimit = 64;
		pAd->CommonCfg.BACapability.field.TxBAWinLimit = 64;
#endif /* COEX_SUPPORT */
	}

	SetCommonHtVht(pAd, wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtBaWinSize_Proc::(HtBaWinSize=%d)\n",
			 pAd->CommonCfg.BACapability.field.RxBAWinLimit));
	return TRUE;
}

INT	Set_HtRdg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value != 0 && IS_ASIC_CAP(pAd, fASIC_CAP_RDG))
		pAd->CommonCfg.bRdg = TRUE;
	else
		pAd->CommonCfg.bRdg = FALSE;

	SetCommonHtVht(pAd, wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Set_HtRdg_Proc::(HtRdg=%d)\n", pAd->CommonCfg.bRdg));
	return TRUE;
}

INT	Set_HtLinkAdapt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0)
		pAd->bLinkAdapt = FALSE;
	else if (Value == 1)
		pAd->bLinkAdapt = TRUE;
	else
		return FALSE; /*Invalid argument*/

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtLinkAdapt_Proc::(HtLinkAdapt=%d)\n", pAd->bLinkAdapt));
	return TRUE;
}

INT	Set_HtAmsdu_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);
	wlan_config_set_amsdu_en(wdev, Value);
	SetCommonHtVht(pAd, wdev);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtAmsdu_Proc::(HtAmsdu=%d)\n",
			 wlan_config_get_amsdu_en(wdev)));
	return TRUE;
}

INT	Set_HtAutoBa_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0) {
		pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
		pAd->CommonCfg.BACapability.field.Policy = BA_NOTUSE;
	} else if (Value == 1) {
		pAd->CommonCfg.BACapability.field.AutoBA = TRUE;
		pAd->CommonCfg.BACapability.field.Policy = IMMED_BA;
	} else
		return FALSE; /*Invalid argument*/

	pAd->CommonCfg.REGBACapability.field.AutoBA = pAd->CommonCfg.BACapability.field.AutoBA;
	pAd->CommonCfg.REGBACapability.field.Policy = pAd->CommonCfg.BACapability.field.Policy;
	SetCommonHtVht(pAd, wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtAutoBa_Proc::(HtAutoBa=%d)\n",
			 pAd->CommonCfg.BACapability.field.AutoBA));
	return TRUE;
}

INT	Set_HtProtect_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);
	wlan_config_set_ht_protect_en(wdev, Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtProtect_Proc::(HtProtect=%d)\n", Value));
	return TRUE;
}

INT	Set_SendSMPSAction_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR mac[6], mode;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	/*MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n%s\n", arg));*/
	/*
		The BARecTearDown inupt string format should be xx:xx:xx:xx:xx:xx-d,
			=>The six 2 digit hex-decimal number previous are the Mac address,
			=>The seventh decimal number is the mode value.
	*/
	if (strlen(arg) <
		19) /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and mode value in decimal format.*/
		return FALSE;

	token = strchr(arg, DASH);

	if ((token != NULL) && (strlen(token) > 1)) {
		mode = os_str_tol((token + 1), 0, 10);

		if (mode > MMPS_DISABLE)
			return FALSE;

		*token = '\0';

		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++) {
			if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
				return FALSE;

			AtoH(token, (&mac[i]), 1);
		}

		if (i != 6)
			return FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x",
				 PRINT_MAC(mac), mode));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pEntry = MacTableLookup(pAd, mac);
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pEntry = MacTableLookup2(pAd, mac, NULL);
#endif

		if (pEntry) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nSendSMPSAction SMPS mode = %d\n", mode));
			SendSMPSAction(pAd, pEntry->wcid, mode);
		}

		return TRUE;
	}

	return FALSE;
}

INT	Set_HtMIMOPSmode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	UCHAR mmps = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value > 3)
		Value = 3;
	wlan_config_set_mmps(wdev, Value);
	SetCommonHtVht(pAd, wdev);
	mmps = wlan_config_get_mmps(wdev);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtMIMOPSmode_Proc::(MIMOPS mode=%d)\n",
			 mmps));
	return TRUE;
}

#ifdef CONFIG_AP_SUPPORT
/*
    ==========================================================================
    Description:
	Set Tx Stream number
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_HtTxStream_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG	Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
	struct wifi_dev *TmpWdev = NULL;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucTxPath = pAd->Antenna.field.TxPath, band_idx = 0;
	UINT32 i = 0;

	if (!wdev)
		return FALSE;
	band_idx = HcGetBandByWdev(wdev);

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {

		if (band_idx == DBDC_BAND0)
			ucTxPath = pAd->dbdc_band0_tx_path;
		else
			ucTxPath = pAd->dbdc_band1_tx_path;
	}
#endif

	Value = os_str_tol(arg, 0, 10);
	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		TmpWdev = &pAd->ApCfg.MBSSID[i].wdev;
		if (TmpWdev && (band_idx == HcGetBandByWdev(TmpWdev))) {
			if ((Value >= 1) && (Value <= ucTxPath)) {
				wlan_config_set_tx_stream(TmpWdev, Value);
				wlan_operate_set_tx_stream(TmpWdev, Value);
			} else {
				wlan_config_set_tx_stream(TmpWdev, ucTxPath);
				wlan_operate_set_tx_stream(TmpWdev, ucTxPath);
			}

			SetCommonHtVht(pAd, TmpWdev);
		}
	}
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
		APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtTxStream_Proc::(Tx Stream=%d)\n",
			 wlan_operate_get_tx_stream(wdev)));
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set Rx Stream number
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_HtRxStream_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG	Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
	struct wifi_dev *TmpWdev = NULL;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucRxPath = pAd->Antenna.field.RxPath, band_idx = 0;
	UINT32 i = 0;

	if (!wdev)
		return FALSE;
	band_idx = HcGetBandByWdev(wdev);

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {

		if (band_idx == DBDC_BAND0)
			ucRxPath = pAd->dbdc_band0_rx_path;
		else
			ucRxPath = pAd->dbdc_band1_rx_path;
	}
#endif

	Value = os_str_tol(arg, 0, 10);
	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		TmpWdev = &pAd->ApCfg.MBSSID[i].wdev;
		if (TmpWdev && (band_idx == HcGetBandByWdev(TmpWdev))) {
			if ((Value >= 1) && (Value <= ucRxPath)) {
				wlan_config_set_rx_stream(TmpWdev, Value);
				wlan_operate_set_rx_stream(TmpWdev, Value);
			} else {
				wlan_config_set_rx_stream(TmpWdev, ucRxPath);
				wlan_operate_set_rx_stream(TmpWdev, ucRxPath);
			}

			SetCommonHtVht(pAd, TmpWdev);
		}
	}
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
		APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtRxStream_Proc::(Rx Stream=%d)\n",
			 wlan_operate_get_rx_stream(wdev)));
	return TRUE;
}

#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
INT	Set_GreenAP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0)
		greenap_proc(pAd, greenap, FALSE);
	else if (Value == 1)
		greenap_proc(pAd, greenap, TRUE);
	else
		return FALSE; /*Invalid argument*/

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_GreenAP_Proc::(greenap_cap=%d)\n",
			 greenap_get_capability(greenap)));
	return TRUE;
}
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT set_pcie_aspm_dym_ctrl_cap_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0) {
		mt_asic_pcie_aspm_dym_ctrl(pAd, DBDC_BAND0, FALSE, FALSE);
		if (pAd->CommonCfg.dbdc_mode)
			mt_asic_pcie_aspm_dym_ctrl(pAd, DBDC_BAND1, FALSE, FALSE);
		set_pcie_aspm_dym_ctrl_cap(pAd, FALSE);
	} else if (Value == 1) {
		set_pcie_aspm_dym_ctrl_cap(pAd, TRUE);
	} else {
		return FALSE; /*Invalid argument*/
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s=%d\n",
		__func__,
		get_pcie_aspm_dym_ctrl_cap(pAd)));

	return TRUE;
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

INT	Set_ForceShortGI_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0)
		pAd->WIFItestbed.bShortGI = FALSE;
	else if (Value == 1)
		pAd->WIFItestbed.bShortGI = TRUE;
	else
		return FALSE; /*Invalid argument*/

	SetCommonHtVht(pAd, wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_ForceShortGI_Proc::(ForceShortGI=%d)\n",
			 pAd->WIFItestbed.bShortGI));
	return TRUE;
}

INT	Set_ForceGF_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0)
		pAd->WIFItestbed.bGreenField = FALSE;
	else if (Value == 1)
		pAd->WIFItestbed.bGreenField = TRUE;
	else
		return FALSE; /*Invalid argument*/

	SetCommonHtVht(pAd, wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_ForceGF_Proc::(ForceGF=%d)\n",
			 pAd->WIFItestbed.bGreenField));
	return TRUE;
}

INT	Set_HtMimoPs_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0)
		pAd->CommonCfg.bMIMOPSEnable = FALSE;
	else if (Value == 1)
		pAd->CommonCfg.bMIMOPSEnable = TRUE;
	else
		return FALSE; /*Invalid argument*/

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtMimoPs_Proc::(HtMimoPs=%d)\n",
			 pAd->CommonCfg.bMIMOPSEnable));
	return TRUE;
}

#ifdef DOT11N_DRAFT3
INT Set_HT_BssCoex_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *pParam)
{
	UCHAR bBssCoexEnable = os_str_tol(pParam, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	pAd->CommonCfg.bBssCoexEnable = ((bBssCoexEnable == 1) ? TRUE : FALSE);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set bBssCoexEnable=%d!\n", pAd->CommonCfg.bBssCoexEnable));

	if ((pAd->CommonCfg.bBssCoexEnable == FALSE)
		&& pAd->CommonCfg.bRcvBSSWidthTriggerEvents) {
		/* switch back 20/40 */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set bBssCoexEnable:  Switch back 20/40.\n"));
		pAd->CommonCfg.bRcvBSSWidthTriggerEvents = FALSE;

		if ((HcIsRfSupport(pAd, RFIC_24GHZ)) && (wlan_config_get_ht_bw(wdev) == BW_40))
			wlan_operate_set_ht_bw(wdev, HT_BW_40, wlan_config_get_ext_cha(wdev));

	}

	return TRUE;
}

INT Set_HT_BssCoexApCntThr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *pParam)
{
	pAd->CommonCfg.BssCoexApCntThr = os_str_tol(pParam, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set BssCoexApCntThr=%d!\n", pAd->CommonCfg.BssCoexApCntThr));
	return TRUE;
}
#endif /* DOT11N_DRAFT3 */

#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
INT	Set_VhtBw_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *tdev;
	UCHAR Bandidx = 0;
	UCHAR i = 0;
	ULONG vht_cw;
	UCHAR vht_bw = VHT_BW_80;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Bandidx = HcGetBandByWdev(wdev);
	vht_cw = os_str_tol(arg, 0, 10);

	if (wdev->channel <= 14)
		goto direct_done;

	if (vht_cw <= VHT_BW_8080)
		vht_bw = vht_cw;
	else
		vht_bw = VHT_BW_2040;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];

		if (tdev && (Bandidx == HcGetBandByWdev(tdev))) {
				wlan_config_set_vht_bw(tdev, vht_bw);
				if (!WMODE_CAP_AC(tdev->PhyMode))
					goto direct_done;
				wlan_operate_set_vht_bw(tdev, vht_bw);
		}
	}
direct_done:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_VhtBw_Proc::(VHT_BW=%d)\n", vht_bw));
	return TRUE;
}

INT set_VhtBwSignal_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR *bwsig_str[] = {"NONE", "STATIC", "DYNAMIC"};
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	ULONG bw_signal = os_str_tol(arg, 0, 10);

	if (bw_signal > BW_SIGNALING_DYNAMIC)
		bw_signal = BW_SIGNALING_DISABLE;
	wlan_config_set_vht_bw_sig(wdev, bw_signal);

	AsicSetRtsSignalTA(pAd, bw_signal);

	if (bw_signal > BW_SIGNALING_DISABLE) {
		UINT32 value = 0;

		MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TCR, &value);
		value |= DCH_DET_DIS;
		MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_TCR, value);

		if (IS_MT7615(pAd)) {
			if (bw_signal == BW_SIGNALING_DYNAMIC)
				MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_AALCR0, 0x02020202);
		}
	} else {
		UINT32 value = 0;

		MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TCR, &value);
		value &= (~DCH_DET_DIS);
		MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_TCR, value);

		if (IS_MT7615(pAd)) {
			if (bw_signal == BW_SIGNALING_DYNAMIC)
				MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_AALCR0, 0x00000000);
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("vht_bw_signal = %s\n", bwsig_str[bw_signal]));

	return TRUE;
}

INT	Set_VhtLdpc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value > 1 || Value < 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Invalid arguments!\n", __func__));
		return FALSE;
	}

	wlan_config_set_vht_ldpc(wdev, (UCHAR)Value);
	wlan_operate_set_vht_ldpc(wdev, (UCHAR)Value);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(VhtLdpc=%d)\n",
		__func__, wlan_config_get_vht_ldpc(wdev)));
	return TRUE;
}

INT	Set_VhtStbc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value == STBC_USE)
		wlan_config_set_vht_stbc(wdev, STBC_USE);
	else if (Value == STBC_NONE)
		wlan_config_set_vht_stbc(wdev, STBC_NONE);
	else
		return FALSE; /*Invalid argument */

	SetCommonHtVht(pAd, wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_VhtStbc_Proc::(VhtStbc=%d)\n",
			 wlan_config_get_vht_stbc(wdev)));
	return TRUE;
}

INT	Set_VhtDisallowNonVHT_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0)
		pAd->CommonCfg.bNonVhtDisallow = FALSE;
	else
		pAd->CommonCfg.bNonVhtDisallow = TRUE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_VhtDisallowNonVHT_Proc::(bNonVhtDisallow=%d)\n",
			 pAd->CommonCfg.bNonVhtDisallow));
	return TRUE;
}
#endif /* DOT11_VHT_AC */


INT	Set_FixedTxMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT	fix_tx_mode = RT_CfgSetFixedTxPhyMode(arg);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	wdev = &pAd->StaCfg[pObj->ioctl_if].wdev;
#endif /* CONFIG_STA_SUPPORT */

	if (wdev)
		wdev->DesiredTransmitSetting.field.FixedTxMode = fix_tx_mode;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():(FixedTxMode=%d)\n",
			 __func__, fix_tx_mode));
	return TRUE;
}

#ifdef CONFIG_APSTA_MIXED_SUPPORT
INT	Set_OpMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = os_str_tol(arg, 0, 10);
#ifdef RTMP_MAC_PCI

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
#endif /* RTMP_MAC_PCI */
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Can not switch operate mode on interface up !!\n"));
			return FALSE;
		}

	if (Value == 0)
		pAd->OpMode = OPMODE_STA;
	else if (Value == 1)
		pAd->OpMode = OPMODE_AP;
	else
		return FALSE; /*Invalid argument*/

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_OpMode_Proc::(OpMode=%s)\n",
			 pAd->OpMode == 1 ? "AP Mode" : "STA Mode"));
	return TRUE;
}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
/*
	========================================================================
	Routine Description:
		Set the enable/disable the stream mode

	Arguments:
		1:	enable for 1SS
		2:	enable for 2SS
		3:	enable for 1SS and 2SS
		0:	disable

	Notes:
		Currently only support 1SS
	========================================================================
*/
INT Set_StreamMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 streamWord, reg, regAddr;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
				 __func__));
		return FALSE;
	}

	if (cap->FlgHwStreamMode == FALSE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("chip not supported feature\n"));
		return FALSE;
	}

	pAd->CommonCfg.StreamMode = (os_str_tol(arg, 0, 10) & 0x3);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():StreamMode=%d\n", __func__, pAd->CommonCfg.StreamMode));
	streamWord = StreamModeRegVal(pAd);

	for (regAddr = TX_CHAIN_ADDR0_H; regAddr <= TX_CHAIN_ADDR3_H; regAddr += 8) {
		RTMP_IO_READ32(pAd->hdev_ctrl, regAddr, &reg);
		reg &= (~0x000F0000);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, regAddr, streamWord | reg);
	}

	return TRUE;
}

INT Set_StreamModeMac_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return FALSE;
}

INT Set_StreamModeMCS_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->CommonCfg.StreamModeMCS = os_str_tol(arg, 0, 16);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():StreamModeMCS=%02X\n",
			 __func__, pAd->CommonCfg.StreamModeMCS));
	return TRUE;
}
#endif /* STREAM_MODE_SUPPORT */

INT Set_LongRetryLimit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR LongRetryLimit = (UCHAR)os_str_tol(arg, 0, 10);

	AsicSetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_LONG, LongRetryLimit);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF Set_LongRetryLimit_Proc::(LongRetryLimit=0x%x)\n",
			 LongRetryLimit));
	return TRUE;
}

INT Set_ShortRetryLimit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR ShortRetryLimit = (UCHAR)os_str_tol(arg, 0, 10);

	AsicSetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_SHORT, ShortRetryLimit);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF Set_ShortRetryLimit_Proc::(ShortRetryLimit=0x%x)\n",
			 ShortRetryLimit));
	return TRUE;
}

INT Set_AutoFallBack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return RT_CfgSetAutoFallBack(pAd, arg);
}


#ifdef MEM_ALLOC_INFO_SUPPORT
INT Show_MemInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ShowMemAllocInfo();
	return TRUE;
}

INT Show_PktInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ShowPktAllocInfo();
	return TRUE;
}
#endif /* MEM_ALLOC_INFO_SUPPORT */

INT	Show_SSID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	UCHAR	ssid_str[33];
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	NdisZeroMemory(&ssid_str[0], 33);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		NdisMoveMemory(&ssid_str[0],
					   pAd->ApCfg.MBSSID[pObj->ioctl_if].Ssid,
					   pAd->ApCfg.MBSSID[pObj->ioctl_if].SsidLen);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		NdisMoveMemory(&ssid_str[0],
					   pAd->StaCfg[pObj->ioctl_if].Ssid,
					   pAd->StaCfg[pObj->ioctl_if].SsidLen);
	}
#endif /* CONFIG_STA_SUPPORT */
	snprintf(pBuf, BufLen, "\t%s", ssid_str);
	return 0;
}

static VOID GetWirelessMode(UCHAR PhyMode, UCHAR *pBuf, UCHAR BufLen)
{
	switch (PhyMode) {
	case (WMODE_B | WMODE_G):
		snprintf(pBuf, BufLen, "\t11B/G");
		break;

	case (WMODE_B):
		snprintf(pBuf, BufLen, "\t11B");
		break;

	case (WMODE_A):
		snprintf(pBuf, BufLen, "\t11A");
		break;

	case (WMODE_A | WMODE_B | WMODE_G):
		snprintf(pBuf, BufLen, "\t11A/B/G");
		break;

	case (WMODE_G):
		snprintf(pBuf, BufLen, "\t11G");
		break;
#ifdef DOT11_N_SUPPORT

	case (WMODE_A | WMODE_B | WMODE_G | WMODE_GN | WMODE_AN):
		snprintf(pBuf, BufLen, "\t11A/B/G/N");
		break;

	case (WMODE_GN):
		snprintf(pBuf, BufLen, "\t11N only with 2.4G");
		break;

	case (WMODE_G | WMODE_GN):
		snprintf(pBuf, BufLen, "\t11G/N");
		break;

	case (WMODE_A | WMODE_AN):
		snprintf(pBuf, BufLen, "\t11A/N");
		break;

	case (WMODE_B | WMODE_G | WMODE_GN):
		snprintf(pBuf, BufLen, "\t11B/G/N");
		break;

	case (WMODE_A | WMODE_G | WMODE_GN | WMODE_AN):
		snprintf(pBuf, BufLen, "\t11A/G/N");
		break;

	case (WMODE_AN):
		snprintf(pBuf, BufLen, "\t11N only with 5G");
		break;
#endif /* DOT11_N_SUPPORT */

	default:
		snprintf(pBuf, BufLen, "\tUnknow Value(%d)", PhyMode);
		break;
	}
}

INT	Show_WirelessMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	GetWirelessMode(wdev->PhyMode, pBuf, BufLen);
	return 0;
}

INT	Show_TxBurst_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.bEnableTxBurst ? "TRUE" : "FALSE");
	return 0;
}

INT	Show_TxPreamble_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	switch (pAd->CommonCfg.TxPreamble) {
	case Rt802_11PreambleShort:
		snprintf(pBuf, BufLen, "\tShort");
		break;

	case Rt802_11PreambleLong:
		snprintf(pBuf, BufLen, "\tLong");
		break;

	case Rt802_11PreambleAuto:
		snprintf(pBuf, BufLen, "\tAuto");
		break;

	default:
		snprintf(pBuf, BufLen, "\tUnknown Value(%lu)", pAd->CommonCfg.TxPreamble);
		break;
	}

	return 0;
}

INT	Show_TxPower_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	UINT8   BandIdx = 0;
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
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	wdev = &pAd->StaCfg[0].wdev;
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BandIdx = %d\n", __func__, BandIdx));

	/* sanity check for Band index */
	if (BandIdx >= DBDC_BAND_NUM)
		return 1;

	snprintf(pBuf, BufLen, "\t%u", pAd->CommonCfg.ucTxPowerPercentage[BandIdx]);
	return 0;
}

INT	Show_Channel_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev->channel > 14)
		snprintf(pBuf, BufLen, "\t5G Band: %d\n", wdev->channel);
	else
		snprintf(pBuf, BufLen, "\t2.4G Band: %d\n", wdev->channel);

	return 0;
}

INT	Show_BGProtection_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	switch (pAd->CommonCfg.UseBGProtection) {
	case 1: /*Always On*/
		snprintf(pBuf, BufLen, "\tON");
		break;

	case 2: /*Always OFF*/
		snprintf(pBuf, BufLen, "\tOFF");
		break;

	case 0: /*AUTO*/
		snprintf(pBuf, BufLen, "\tAuto");
		break;

	default:
		snprintf(pBuf, BufLen, "\tUnknow Value(%lu)", pAd->CommonCfg.UseBGProtection);
		break;
	}

	return 0;
}

INT	Show_RTSThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 oper_len_thld;
	UINT32 conf_len_thld;

	if (!wdev)
		return 0;

	conf_len_thld = wlan_config_get_rts_len_thld(wdev);
	oper_len_thld = wlan_operate_get_rts_len_thld(wdev);
	snprintf(pBuf, BufLen, "\tRTSThreshold:: conf=%d, oper=%d", conf_len_thld, oper_len_thld);
	return 0;
}

INT	Show_FragThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	UINT32 conf_frag_thld;
	UINT32 oper_frag_thld;
	POS_COOKIE pobj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pobj->ioctl_if, pobj->ioctl_if_type);

	if (!wdev)
		return 0;

	conf_frag_thld = wlan_config_get_frag_thld(wdev);
	oper_frag_thld = wlan_operate_get_frag_thld(wdev);
	snprintf(pBuf, BufLen, "\tFrag thld:: conf=%u, oper=%u", conf_frag_thld, oper_frag_thld);
	return 0;
}

#ifdef DOT11_N_SUPPORT
INT	Show_HtBw_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wlan_config_get_ht_bw(wdev) == BW_40)
		snprintf(pBuf, BufLen, "\t40 MHz");
	else
		snprintf(pBuf, BufLen, "\t20 MHz");

	return 0;
}

INT	Show_HtMcs_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	wdev = &pAd->StaCfg[pObj->ioctl_if].wdev;
#endif /* CONFIG_STA_SUPPORT */

	if (wdev)
		snprintf(pBuf, BufLen, "\t%u", wdev->DesiredTransmitSetting.field.MCS);

	return 0;
}

INT	Show_HtGi_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR ht_gi = GI_400;
	UCHAR *msg[3] = {"GI_800", "GI_400", "GI_Unknown"};

	if (!wdev)
		return 0;

	ht_gi = wlan_config_get_ht_gi(wdev);

	if (ht_gi > GI_400)
		ht_gi = 2; /*Unknown GI*/

	snprintf(pBuf, BufLen, "\ti%s", msg[ht_gi]);
	return 0;
}

INT	Show_HtOpMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	switch (pAd->CommonCfg.RegTransmitSetting.field.HTMODE) {
	case HTMODE_GF:
		snprintf(pBuf, BufLen, "\tGF");
		break;

	case HTMODE_MM:
		snprintf(pBuf, BufLen, "\tMM");
		break;

	default:
		snprintf(pBuf, BufLen, "\tUnknow Value(%u)", pAd->CommonCfg.RegTransmitSetting.field.HTMODE);
		break;
	}

	return 0;
}

INT	Show_HtExtcha_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR ext_cha;

	ext_cha = wlan_config_get_ext_cha(wdev);

	switch (ext_cha) {
	case EXTCHA_BELOW:
		snprintf(pBuf, BufLen, "\tBelow");
		break;

	case EXTCHA_ABOVE:
		snprintf(pBuf, BufLen, "\tAbove");
		break;

	default:
		snprintf(pBuf, BufLen, "\tUnknow Value(%u)", ext_cha);
		break;
	}

	return 0;
}

INT	Show_HtMpduDensity_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR mpdu_density = 0;

	if (wdev)
		mpdu_density = wlan_config_get_min_mpdu_start_space(wdev);
	snprintf(pBuf, BufLen, "\t%u", mpdu_density);
	return 0;
}

INT	Show_HtBaWinSize_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%u", pAd->CommonCfg.BACapability.field.RxBAWinLimit);
	return 0;
}

INT	Show_HtRdg_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.bRdg ? "TRUE" : "FALSE");
	return 0;
}

INT	Show_HtAmsdu_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR amsdu_en = 0;

	if (wdev)
		amsdu_en = wlan_config_get_amsdu_en(wdev);
	snprintf(pBuf, BufLen, "\t%s", (amsdu_en) ? "TRUE" : "FALSE");
	return 0;
}

INT	Show_HtAutoBa_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.BACapability.field.AutoBA ? "TRUE" : "FALSE");
	return 0;
}
#endif /* DOT11_N_SUPPORT */

INT	Show_CountryRegion_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%d", pAd->CommonCfg.CountryRegion);
	return 0;
}

INT	Show_CountryRegionABand_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%d", pAd->CommonCfg.CountryRegionForABand);
	return 0;
}

INT	Show_CountryCode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.CountryCode);
	return 0;
}

#ifdef AGGREGATION_SUPPORT
INT	Show_PktAggregate_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.bAggregationCapable ? "TRUE" : "FALSE");
	return 0;
}
#endif /* AGGREGATION_SUPPORT */

INT	Show_WmmCapable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	snprintf(pBuf, BufLen, "\t%s", pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.bWmmCapable ? "TRUE" : "FALSE");
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	snprintf(pBuf, BufLen, "\t%s", pAd->StaCfg[pObj->ioctl_if].wdev.bWmmCapable ? "TRUE" : "FALSE");
#endif /* CONFIG_STA_SUPPORT */
	return 0;
}

INT	Show_IEEE80211H_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.bIEEE80211H ? "TRUE" : "FALSE");
	return 0;
}

#ifdef CONFIG_STA_SUPPORT
INT	Show_NetworkType_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	switch (pStaCfg->BssType) {
	case BSS_ADHOC:
		snprintf(pBuf, BufLen, "\tAdhoc");
		break;

	case BSS_INFRA:
		snprintf(pBuf, BufLen, "\tInfra");
		break;

	case BSS_ANY:
		snprintf(pBuf, BufLen, "\tAny");
		break;

	case BSS_MONITOR:
		snprintf(pBuf, BufLen, "\tMonitor");
		break;

	default:
		sprintf(pBuf, "\tUnknow Value(%d)", pStaCfg->BssType);
		break;
	}

	return 0;
}

#ifdef WSC_STA_SUPPORT
INT	Show_WpsPbcBand_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	switch (pStaCfg->wdev.WscControl.WpsApBand) {
	case PREFERRED_WPS_AP_PHY_TYPE_2DOT4_G_FIRST:
		snprintf(pBuf, BufLen, "\t2.4G");
		break;

	case PREFERRED_WPS_AP_PHY_TYPE_5_G_FIRST:
		snprintf(pBuf, BufLen, "\t5G");
		break;

	case PREFERRED_WPS_AP_PHY_TYPE_AUTO_SELECTION:
		snprintf(pBuf, BufLen, "\tAuto");
		break;

	default:
		snprintf(pBuf, BufLen, "\tUnknow Value(%d)", pStaCfg->wdev.WscControl.WpsApBand);
		break;
	}

	return 0;
}
#endif /* WSC_STA_SUPPORT */

INT	Show_WPAPSK_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	if ((pStaCfg->WpaPassPhraseLen >= 8) &&
		(pStaCfg->WpaPassPhraseLen < 64))
		snprintf(pBuf, BufLen, "\tWPAPSK = %s", pStaCfg->WpaPassPhrase);
	else {
		INT idx;

		snprintf(pBuf, BufLen, "\tWPAPSK = ");

		for (idx = 0; idx < 32; idx++)
			snprintf(pBuf + strlen(pBuf), BufLen - strlen(pBuf), "%02X", pStaCfg->WpaPassPhrase[idx]);
	}

	return 0;
}

INT	Show_AutoReconnect_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	snprintf(pBuf, BufLen, "\tAutoReconnect = %d", pStaCfg->bAutoReconnect);
	return 0;
}

#endif /* CONFIG_STA_SUPPORT */

INT	Show_STA_RAInfo_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	sprintf(pBuf, "\n");
#ifdef NEW_RATE_ADAPT_SUPPORT
	sprintf(pBuf + strlen(pBuf), "LowTrafficThrd: %d\n", pAd->CommonCfg.lowTrafficThrd);
	sprintf(pBuf + strlen(pBuf), "TrainUpRule: %d\n", pAd->CommonCfg.TrainUpRule);
	sprintf(pBuf + strlen(pBuf), "TrainUpRuleRSSI: %d\n", pAd->CommonCfg.TrainUpRuleRSSI);
	sprintf(pBuf + strlen(pBuf), "TrainUpLowThrd: %d\n", pAd->CommonCfg.TrainUpLowThrd);
	sprintf(pBuf + strlen(pBuf), "TrainUpHighThrd: %d\n", pAd->CommonCfg.TrainUpHighThrd);
#endif /* NEW_RATE_ADAPT_SUPPORT // */
#ifdef STREAM_MODE_SUPPORT
	sprintf(pBuf + strlen(pBuf), "StreamMode: %d\n", pAd->CommonCfg.StreamMode);
	sprintf(pBuf + strlen(pBuf), "StreamModeMCS: 0x%04x\n", pAd->CommonCfg.StreamModeMCS);
#endif /* STREAM_MODE_SUPPORT // */
#ifdef TXBF_SUPPORT
	sprintf(pBuf + strlen(pBuf), "ITxBfEn: %d\n", pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn);
	sprintf(pBuf + strlen(pBuf), "ITxBfTimeout: %ld\n", pAd->CommonCfg.ITxBfTimeout);
	sprintf(pBuf + strlen(pBuf), "ETxBfTimeout: %ld\n", pAd->CommonCfg.ETxBfTimeout);
	sprintf(pBuf + strlen(pBuf), "CommonCfg.ETxBfEnCond: %ld\n", pAd->CommonCfg.ETxBfEnCond);
	sprintf(pBuf + strlen(pBuf), "ETxBfNoncompress: %d\n", pAd->CommonCfg.ETxBfNoncompress);
	sprintf(pBuf + strlen(pBuf), "ETxBfIncapable: %d\n", pAd->CommonCfg.ETxBfIncapable);
#endif /* TXBF_SUPPORT // */
	return 0;
}

static INT dump_mac_table(RTMP_ADAPTER *pAd, UINT32 ent_type, BOOLEAN bReptCli)
{
	INT i;
	ULONG DataRate = 0;
	ULONG DataRate_r = 0;
	ULONG max_DataRate = 0;
	INT sta_cnt = 0;
	INT apcli_cnt = 0;
	INT rept_cnt = 0;
	UCHAR	tmp_str[30];
	INT		temp_str_len = sizeof(tmp_str);
	ADD_HT_INFO_IE *addht;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	printk("\n");
#ifdef CONFIG_HOTSPOT_R2
	printk("\n%-19s%-6s%-5s%-4s%-4s%-4s%-7s%-20s%-12s%-9s%-12s%-9s%-10s%-7s%-10s%-7s\n",
		   "MAC", "MODE", "AID", "BSS", "PSM", "WMM", "MIMOPS", "RSSI0/1/2/3", "PhMd",      "BW",      "MCS",      "SGI",
		   "STBC",      "Idle", "Rate",     "QosMap");
#else
	printk("\n%-19s%-6s%-5s%-4s%-4s%-4s%-7s%-20s%-12s%-9s%-12s%-9s%-10s%-7s%-10s\n",
		   "MAC", "MODE", "AID", "BSS", "PSM", "WMM", "MIMOPS", "RSSI0/1/2/3", "PhMd(T/R)", "BW(T/R)", "MCS(T/R)", "SGI(T/R)",
		   "STBC(T/R)", "Idle", "Rate(T/R)");
#endif /* CONFIG_HOTSPOT_R2 */

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if ((ent_type == ENTRY_NONE)) {
			/* dump all MacTable entries */
			if (pEntry->EntryType == ENTRY_NONE)
				continue;
		} else {
			/* dump MacTable entries which match the EntryType */
			if (pEntry->EntryType != ent_type)
				continue;

			if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
				&& (pEntry->Sst != SST_ASSOC))
				continue;

#ifdef MAC_REPEATER_SUPPORT

			if (bReptCli == FALSE) {
				/* only dump the apcli entry which not a RepeaterCli */
				if (IS_ENTRY_REPEATER(pEntry) && (pEntry->bReptCli == TRUE))
					continue;
			}

#endif /* MAC_REPEATER_SUPPORT */
		}

		if (pEntry->func_tb_idx != pObj->ioctl_if)
			continue;

		if (IS_ENTRY_CLIENT(pEntry))
			sta_cnt++;

		if (IS_ENTRY_PEER_AP(pEntry))
			apcli_cnt++;

		if (IS_ENTRY_REPEATER(pEntry))
			rept_cnt++;

		addht = wlan_operate_get_addht(pEntry->wdev);
#ifdef DOT11_N_SUPPORT
		printk("HT Operating Mode : %d\n", addht->AddHtInfo2.OperaionMode);
		printk("\n");
#endif /* DOT11_N_SUPPORT */
		DataRate = 0;
		getRate(pEntry->HTPhyMode, &DataRate);
		printk("%02X:%02X:%02X:%02X:%02X:%02X  ", PRINT_MAC(pEntry->Addr));
		printk("%-6x", pEntry->EntryType);
		printk("%-5d", (int)pEntry->Aid);
		printk("%-4d", (int)pEntry->func_tb_idx);
		printk("%-4d", (int)pEntry->PsMode);
		printk("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE));
#ifdef DOT11_N_SUPPORT
		printk("%-7d", (int)pEntry->MmpsMode);
#endif /* DOT11_N_SUPPORT */
		snprintf(tmp_str, temp_str_len, "%d/%d/%d/%d", pEntry->RssiSample.AvgRssi[0],
				 pEntry->RssiSample.AvgRssi[1],
				 pEntry->RssiSample.AvgRssi[2],
				 pEntry->RssiSample.AvgRssi[3]);
		printk("%-20s", tmp_str);
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

		if (cap->fgRateAdaptFWOffload == TRUE && (pEntry->bAutoTxRateSwitch == TRUE)) {
			UCHAR phy_mode, rate, bw, sgi, stbc;
			UCHAR phy_mode_r, rate_r, bw_r, sgi_r, stbc_r;
#ifdef DOT11_VHT_AC
			UCHAR vht_nss;
			UCHAR vht_nss_r;
#endif
			UINT32 RawData;
			UINT32 RawData_r;
			UINT32 lastTxRate = pEntry->LastTxRate;
			UINT32 lastRxRate = pEntry->LastRxRate;

			if (pEntry->bAutoTxRateSwitch == TRUE) {
				EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
				HTTRANSMIT_SETTING LastTxRate;
				HTTRANSMIT_SETTING LastRxRate;

				MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
				LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
				LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
				LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
				LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
				LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

				if (LastTxRate.field.MODE == MODE_VHT)
					LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
				else if (LastTxRate.field.MODE == MODE_OFDM)
					LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
				else
					LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

				lastTxRate = (UINT32)(LastTxRate.word);
				LastRxRate.word = (USHORT)lastRxRate;
				RawData = lastTxRate;
				phy_mode = (RawData >> 13) & 0x7;
				rate = RawData & 0x3F;
				bw = (RawData >> 7) & 0x3;
				sgi = (RawData >> 9) & 0x1;
				stbc = ((RawData >> 10) & 0x1);
				/* ---- */
				RawData_r = lastRxRate;
				phy_mode_r = (RawData_r >> 13) & 0x7;
				rate_r = RawData_r & 0x3F;
				bw_r = (RawData_r >> 7) & 0x3;
				sgi_r = (RawData_r >> 9) & 0x1;
				stbc_r = ((RawData_r >> 10) & 0x1);
				snprintf(tmp_str, temp_str_len, "%s/%s", get_phymode_str(phy_mode), get_phymode_str(phy_mode_r));
				printk("%-12s", tmp_str);
				snprintf(tmp_str, temp_str_len, "%s/%s", get_bw_str(bw), get_bw_str(bw_r));
				printk("%-9s", tmp_str);
#ifdef DOT11_VHT_AC

				if (phy_mode == MODE_VHT) {
					vht_nss = ((rate & (0x3 << 4)) >> 4) + 1;
					rate = rate & 0xF;
					snprintf(tmp_str, temp_str_len, "%dS-M%d/", vht_nss, rate);
				} else
#endif /* DOT11_VHT_AC */
					snprintf(tmp_str, temp_str_len, "%d/", rate);

#ifdef DOT11_VHT_AC

				if (phy_mode_r == MODE_VHT) {
					vht_nss_r = ((rate_r & (0x3 << 4)) >> 4) + 1;
					rate_r = rate_r & 0xF;
					snprintf(tmp_str + strlen(tmp_str), temp_str_len - strlen(tmp_str), "%dS-M%d", vht_nss_r, rate_r);
				} else
#endif /* DOT11_VHT_AC */
#if DOT11_N_SUPPORT
					if (phy_mode_r >= MODE_HTMIX)
						snprintf(tmp_str + strlen(tmp_str), temp_str_len - strlen(tmp_str), "%d", rate_r);
					else
#endif
						if (phy_mode_r == MODE_OFDM) {
							if (rate_r == TMI_TX_RATE_OFDM_6M)
								LastRxRate.field.MCS = 0;
							else if (rate_r == TMI_TX_RATE_OFDM_9M)
								LastRxRate.field.MCS = 1;
							else if (rate_r == TMI_TX_RATE_OFDM_12M)
								LastRxRate.field.MCS = 2;
							else if (rate_r == TMI_TX_RATE_OFDM_18M)
								LastRxRate.field.MCS = 3;
							else if (rate_r == TMI_TX_RATE_OFDM_24M)
								LastRxRate.field.MCS = 4;
							else if (rate_r == TMI_TX_RATE_OFDM_36M)
								LastRxRate.field.MCS = 5;
							else if (rate_r == TMI_TX_RATE_OFDM_48M)
								LastRxRate.field.MCS = 6;
							else if (rate_r == TMI_TX_RATE_OFDM_54M)
								LastRxRate.field.MCS = 7;
							else
								LastRxRate.field.MCS = 0;

							snprintf(tmp_str + strlen(tmp_str), temp_str_len - strlen(tmp_str), "%d", LastRxRate.field.MCS);
						} else if (phy_mode_r == MODE_CCK) {
							if (rate_r == TMI_TX_RATE_CCK_1M_LP)
								LastRxRate.field.MCS = 0;
							else if (rate_r == TMI_TX_RATE_CCK_2M_LP)
								LastRxRate.field.MCS = 1;
							else if (rate_r == TMI_TX_RATE_CCK_5M_LP)
								LastRxRate.field.MCS = 2;
							else if (rate_r == TMI_TX_RATE_CCK_11M_LP)
								LastRxRate.field.MCS = 3;
							else if (rate_r == TMI_TX_RATE_CCK_2M_SP)
								LastRxRate.field.MCS = 1;
							else if (rate_r == TMI_TX_RATE_CCK_5M_SP)
								LastRxRate.field.MCS = 2;
							else if (rate_r == TMI_TX_RATE_CCK_11M_SP)
								LastRxRate.field.MCS = 3;
							else
								LastRxRate.field.MCS = 0;

							snprintf(tmp_str + strlen(tmp_str), temp_str_len - strlen(tmp_str), "%d", LastRxRate.field.MCS);
						}

				printk("%-12s", tmp_str);
				snprintf(tmp_str, temp_str_len, "%d/%d", sgi, sgi_r);
				printk("%-9s", tmp_str);
				snprintf(tmp_str, temp_str_len, "%d/%d",  stbc, stbc_r);
				printk("%-10s", tmp_str);
				getRate(LastTxRate, &DataRate);
				getRate(LastRxRate, &DataRate_r);
			}
		} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
		{
			printk("%-12s", get_phymode_str(pEntry->HTPhyMode.field.MODE));
			printk("%-9s", get_bw_str(pEntry->HTPhyMode.field.BW));
#ifdef DOT11_VHT_AC

			if (pEntry->HTPhyMode.field.MODE == MODE_VHT)
				snprintf(tmp_str, temp_str_len, "%dS-M%d", ((pEntry->HTPhyMode.field.MCS >> 4) + 1),
						 (pEntry->HTPhyMode.field.MCS & 0xf));
			else
#endif /* DOT11_VHT_AC */
				snprintf(tmp_str, temp_str_len, "%d", pEntry->HTPhyMode.field.MCS);

			printk("%-12s", tmp_str);
			printk("%-9d", pEntry->HTPhyMode.field.ShortGI);
			printk("%-10d", pEntry->HTPhyMode.field.STBC);
		}

		printk("%-7d", (int)(pEntry->StaIdleTimeout - pEntry->NoDataIdleCount));
		snprintf(tmp_str, temp_str_len, "%d/%d", (int)DataRate, (int)DataRate_r);
		printk("%-10s", tmp_str);
#ifdef CONFIG_HOTSPOT_R2
		printk("%-7d", (int)pEntry->QosMapSupport);
#endif
		printk("%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount,
			   (pEntry->DebugTxCount) ? ((pEntry->DebugTxCount - pEntry->DebugFIFOCount) * 100 / pEntry->DebugTxCount) : 0);
#ifdef CONFIG_HOTSPOT_R2

		if (pEntry->QosMapSupport) {
			int k = 0;

			printk("DSCP Exception:\n");

			for (k = 0; k < pEntry->DscpExceptionCount / 2; k++)
				printk("[Value: %4d] [UP: %4d]\n", pEntry->DscpException[k] & 0xff, (pEntry->DscpException[k] >> 8) & 0xff);

			printk("DSCP Range:\n");

			for (k = 0; k < 8; k++)
				printk("[UP :%3d][Low Value: %4d] [High Value: %4d]\n", k, pEntry->DscpRange[k] & 0xff,
					   (pEntry->DscpRange[k] >> 8) & 0xff);
		}

#endif
		/* +++Add by shiang for debug */
		printk("%69s%-12s", "MaxCap:", get_phymode_str(pEntry->MaxHTPhyMode.field.MODE));
		printk("%-9s", get_bw_str(pEntry->MaxHTPhyMode.field.BW));
#ifdef DOT11_VHT_AC

		if (pEntry->MaxHTPhyMode.field.MODE == MODE_VHT)
			snprintf(tmp_str, temp_str_len, "%dS-M%d", ((pEntry->MaxHTPhyMode.field.MCS >> 4) + 1),
					 (pEntry->MaxHTPhyMode.field.MCS & 0xf));
		else
#endif /* DOT11_VHT_AC */
			snprintf(tmp_str, temp_str_len, "%d", pEntry->MaxHTPhyMode.field.MCS);

		printk("%-12s", tmp_str);
		printk("%-9d", pEntry->MaxHTPhyMode.field.ShortGI);
		printk("%-10d", pEntry->MaxHTPhyMode.field.STBC);
		getRate(pEntry->MaxHTPhyMode, &max_DataRate);
		printk("%-7s", "-");
		printk("%-10d", (int)max_DataRate);
#ifdef HTC_DECRYPT_IOT
		printk("%20s%-10d", "HTC_ICVErr:", pEntry->HTC_ICVErrCnt);
		printk("%20s%-10s", "HTC_AAD_OM_Force:", pEntry->HTC_AAD_OM_Force ? "YES" : "NO");
#endif /* HTC_DECRYPT_IOT */
		printk("  wdev%d\n", (int)pEntry->wdev->wdev_idx);
		/* ---Add by shiang for debug */
		printk("\n");
	}

	printk("sta_cnt=%d\n\r", sta_cnt);
	printk("apcli_cnt=%d\n\r", apcli_cnt);
	printk("rept_cnt=%d\n\r", rept_cnt);
#ifdef OUI_CHECK_SUPPORT
	printk("oui_mgroup=%d\n\r", pAd->MacTab.oui_mgroup_cnt);
	printk("repeater_wcid_error_cnt=%d\n\r", pAd->MacTab.repeater_wcid_error_cnt);
	printk("repeater_bm_wcid_error_cnt=%d\n\r", pAd->MacTab.repeater_bm_wcid_error_cnt);
#endif /*OUI_CHECK_SUPPORT*/
#ifdef HTC_DECRYPT_IOT
	printk("HTC_ICV_Err_TH=%d\n\r", pAd->HTC_ICV_Err_TH);
#endif /* HTC_DECRYPT_IOT */
	return TRUE;
}

INT Show_MacTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ent_type = ENTRY_CLIENT;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): arg=%s\n", __func__, (arg == NULL ? "" : arg)));

	if (arg && strlen(arg)) {
		if (rtstrcasecmp(arg, "sta") == TRUE)
			ent_type = ENTRY_CLIENT;
		else if (rtstrcasecmp(arg, "ap") == TRUE)
			ent_type = ENTRY_AP;
		else
			ent_type = ENTRY_NONE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump MacTable entries info, EntType=0x%x\n", ent_type));
	return dump_mac_table(pAd, ent_type, FALSE);
}

#ifdef ACL_BLK_COUNT_SUPPORT
INT Show_ACLRejectCount_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
	{
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR apidx = pObj->ioctl_if;

		if (arg && strlen(arg)) {
			if (rtstrcasecmp(arg, "1") == TRUE) {
				int count;

				if (pAd->ApCfg.MBSSID[apidx].AccessControlList.Policy == 2) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("ACL: Policy=%lu(0:Dis,1:White,2:Black),ACL: Num=%lu\n",
							pAd->ApCfg.MBSSID[apidx].AccessControlList.Policy,
							pAd->ApCfg.MBSSID[apidx].AccessControlList.Num));
					for (count = 0; count < pAd->ApCfg.MBSSID[apidx].AccessControlList.Num; count++) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("MAC:%02x:%02x:%02x:%02x:%02x:%02x , Reject_Count: %lu\n",
						PRINT_MAC(pAd->ApCfg.MBSSID[apidx].AccessControlList.Entry[count].Addr),
						pAd->ApCfg.MBSSID[apidx].AccessControlList.Entry[count].Reject_Count));
					}
				} else {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("ERROR:Now Policy=%lu(0:Disable,1:White List,2:Black List)\n",
						pAd->ApCfg.MBSSID[apidx].AccessControlList.Policy));
				}
			}
		}
		return TRUE;
	}
#endif/*ACL_BLK_COUNT_SUPPORT*/

INT Show_PSTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ent_type = ENTRY_CLIENT;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): arg=%s\n", __func__, (arg == NULL ? "" : arg)));

	if (arg && strlen(arg)) {
		if (rtstrcasecmp(arg, "sta") == TRUE)
			ent_type = ENTRY_CLIENT;
		else if (rtstrcasecmp(arg, "ap") == TRUE)
			ent_type = ENTRY_AP;
		else
			ent_type = ENTRY_NONE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump MacTable entries info, EntType=0x%x\n", ent_type));
	if (chip_dbg->dump_ps_table)
		return chip_dbg->dump_ps_table(pAd->hdev_ctrl, ent_type, FALSE);
	else
		return FALSE;
}

INT Show_BaTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *tok;
	ULONG first_index, second_index;

	tok = rstrtok(arg, ":");

	if (tok) {
		first_index = os_str_toul(tok, NULL, 10);

		switch (first_index) {
		case 0:
			ba_resource_dump_all(pAd);
			break;
		case 1:
			ba_reordering_resource_dump_all(pAd);
			break;
		case 2:
			while (tok) {
				tok = rstrtok(NULL, ":");
				if (tok) {
					second_index = os_str_toul(tok, NULL, 10);
					ba_reodering_resource_dump(pAd, second_index);
				}
			}
			break;
		}
	} else {
		ba_resource_dump_all(pAd);
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump BaTable info arg = %s\n", arg));
	return TRUE;
}

#ifdef MT_MAC
INT show_wtbl_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i, start, end, idx = -1;
	/* WTBL_ENTRY wtbl_entry; */

	if (arg == NULL)
		return TRUE;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): arg=%s\n", __func__, (arg == NULL ? "" : arg)));

	if (strlen(arg)) {
		idx = os_str_toul(arg, NULL, 10);
		start = end = idx;
	} else {
		start = 0;
		end = pAd->mac_ctrl.wtbl_entry_cnt[0] - 1;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WTBL entries info, start=%d, end=%d, idx=%d\n",
			 start, end, idx));
	dump_wtbl_base_info(pAd);

	for (i = start; i <= end; i++) {
		dump_wtbl_info(pAd, i);
	}

	return TRUE;
}

INT show_wtbltlv_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *Param;
	UCHAR ucWcid = 0;
	UCHAR ucCmdId = 0;
	UCHAR ucAction = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s::param=%s\n", __func__, arg));

	if (arg == NULL)
		goto error;

	Param = rstrtok(arg, ":");

	if (Param != NULL)
		ucWcid = os_str_tol(Param, 0, 10);
	else
		goto error;

	Param = rstrtok(NULL, ":");

	if (Param != NULL)
		ucCmdId = os_str_tol(Param, 0, 10);
	else
		goto error;

	Param = rstrtok(NULL, ":");

	if (Param != NULL)
		ucAction = os_str_tol(Param, 0, 10);
	else
		goto error;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s():ucWcid(%d), CmdId(%d), Action(%d)\n",  __func__, ucWcid, ucCmdId, ucAction));
	mt_wtbltlv_debug(pAd, ucWcid, ucCmdId, ucAction);
	return TRUE;
error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: param = %s not correct\n", __func__, arg));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: iwpriv ra0 show wtbltlv=Wcid,CmdId,Action\n", __func__));
	return 0;
}

INT show_amsdu_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (IS_ASIC_CAP(pAd, fASIC_CAP_HW_TX_AMSDU))
		dump_dmac_amsdu_info(pAd, arg);

	return TRUE;
}

INT show_mib_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->dump_mib_info)
		return chip_dbg->dump_mib_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

#ifdef DBDC_MODE
INT32 ShowDbdcProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	HcShowBandInfo(pAd);
	return TRUE;
}
#endif

INT32 ShowChCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	HcShowChCtrlInfo(pAd);
	return TRUE;
}
#ifdef GREENAP_SUPPORT
INT32 ShowGreenAPProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	HcShowGreenAPInfo(pAd);
	return TRUE;
}
#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT32 show_pcie_aspm_dym_ctrl_cap_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\tflag_pcie_aspm_dym_ctrl_cap=%d\n",
		get_pcie_aspm_dym_ctrl_cap(pAd)));

	return TRUE;
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

static UINT16 txop_to_ms(UINT16 *txop_level)
{
	UINT16 ms = (*txop_level) >> 5;

	ms += ((*txop_level) & (1 << 4)) ? 1 : 0;
	return ms;
}

static void dump_txop_level(UINT16 *txop_level, UINT32 len)
{
	UINT32 prio;

	for (prio = 0; prio < len; prio++) {
		UINT16 ms = txop_to_ms(txop_level + prio);

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" {%x:0x%x(%ums)} ", prio, *(txop_level + prio), ms));
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
}

static void dump_tx_burst_info(struct _RTMP_ADAPTER *pAd)
{
	struct wifi_dev **wdev = pAd->wdev_list;
	EDCA_PARM *edca_param = NULL;
	UINT32 idx = 0;
	UCHAR wmm_idx = 0;
	UCHAR bss_idx = 0xff;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("[%s]\n", __func__));

	do {
		if (wdev[idx] == NULL)
			break;

		if (bss_idx != wdev[idx]->bss_info_argument.ucBssIndex) {
			edca_param = HcGetEdca(pAd, wdev[idx]);

			if (edca_param == NULL)
				break;

			wmm_idx = HcGetWmmIdx(pAd, wdev[idx]);
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("<bss_%x>\n", wdev[idx]->bss_info_argument.ucBssIndex));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" |-[wmm_idx]: %x\n", wmm_idx));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" |-[bitmap]: %08x\n", wdev[idx]->bss_info_argument.prio_bitmap));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" |-[prio:level]:"));
			dump_txop_level(wdev[idx]->bss_info_argument.txop_level, MAX_PRIO_NUM);
			bss_idx = wdev[idx]->bss_info_argument.ucBssIndex;
		}

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" |---<wdev_%x>\n", idx));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("      |-[bitmap]: %08x\n", wdev[idx]->prio_bitmap));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("      |-[prio:level]:"));
		dump_txop_level(wdev[idx]->txop_level, MAX_PRIO_NUM);
		idx++;
	} while (idx < WDEV_NUM_MAX);
}

INT32 show_tx_burst_info(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	dump_tx_burst_info(pAd);
	return TRUE;
}

INT32 show_wifi_sys(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	wifi_sys_dump(pAd);
	return TRUE;
}

INT32 show_wmm_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	hc_show_edca_info(pAd->hdev_ctrl);
	return TRUE;
}

INT32 ShowTmacInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_tmac_info)
		return chip_dbg->show_tmac_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT32 ShowAggInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_agg_info)
		return chip_dbg->show_agg_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT32 ShowArbInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_arb_info)
		return chip_dbg->show_arb_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT ShowManualTxOP(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 txop;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CURRENT: ManualTxOP = %d\n", pAd->CommonCfg.ManualTxop));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       : bEnableTxBurst = %d\n", pAd->CommonCfg.bEnableTxBurst));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       : MacTab.Size = %d\n", pAd->MacTab.Size));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       : RDG_ACTIVE = %d\n", RTMP_TEST_FLAG(pAd,
			 fRTMP_ADAPTER_RDG_ACTIVE)));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR1, &txop);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       : AC0 TxOP = 0x%x\n", GET_AC0LIMIT(txop)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       : AC1 TxOP = 0x%x\n", GET_AC1LIMIT(txop)));
	return TRUE;
}

INT show_dmasch_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_dmasch_info)
		return chip_dbg->show_dmasch_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT32 ShowPseInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_pse_info)
		return chip_dbg->show_pse_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT32 ShowPseData(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *Param;
	UINT8 StartFID, FrameNums;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	Param = rstrtok(arg, ",");

	if (Param != NULL)
		StartFID = os_str_tol(Param, 0, 10);
	else
		goto error;

	Param = rstrtok(NULL, ",");

	if (Param != NULL)
		FrameNums = os_str_tol(Param, 0, 10);
	else
		goto error;

	if (chip_dbg->show_pse_data)
		return chip_dbg->show_pse_data(pAd->hdev_ctrl, StartFID, FrameNums);
	else
		return FALSE;

error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: param = %s not correct\n", __func__, arg));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: iwpriv ra0 show psedata=startfid,framenums\n", __func__));
	return 0;
}

INT ShowPLEInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_ple_info)
		return chip_dbg->show_ple_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT show_TXD_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT fid;

	if (arg == NULL)
		return FALSE;

	if (strlen(arg) == 0)
		return FALSE;

	fid = simple_strtol(arg, 0, 16);
	return ShowTXDInfo(pAd, fid);
}
#define UMAC_FID_FAULT	0xFFF
#define DUMP_MEM_SIZE 64
INT ShowTXDInfo(RTMP_ADAPTER *pAd, UINT fid)
{
	INT i = 0;
	UINT8 data[DUMP_MEM_SIZE];
	UINT32 Addr = 0;

	if (fid >= UMAC_FID_FAULT)
		return FALSE;

	os_zero_mem(data, DUMP_MEM_SIZE);
	Addr = 0xa << 28 | fid << 16; /* TXD addr: 0x{a}{fid}{0000}*/
	MtCmdMemDump(pAd, Addr, &data[0]);

	for (i = 0; i < DUMP_MEM_SIZE; i = i + 4)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DW%02d: 0x%02x%02x%02x%02x\n", i / 4, data[i + 3], data[i + 2],
				 data[i + 1], data[i]));

	asic_dump_tmac_info(pAd, &data[0]);
	return TRUE;
}
INT show_mem_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT addr = os_str_tol(arg, 0, 16);
	UINT8 data[DUMP_MEM_SIZE];
	INT i = 0;

	os_zero_mem(data, DUMP_MEM_SIZE);
	MtCmdMemDump(pAd, addr, &data[0]);

	for (i = 0; i < DUMP_MEM_SIZE; i = i + 4)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("addr 0x%08x: 0x%02x%02x%02x%02x\n", addr + i, data[i + 3],
				 data[i + 2], data[i + 1], data[i]));

	return TRUE;
}
INT show_protect_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_protect_info)
		return chip_dbg->show_protect_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT show_cca_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_cca_info)
		return chip_dbg->show_cca_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;

}

#ifdef CUT_THROUGH
INT ShowCutThroughInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	dump_ct_token_list(pAd->PktTokenCb, CUT_THROUGH_TYPE_TX);
	return TRUE;
}
#endif /* CUT_THROUGH */
#endif /*MT_MAC*/

INT Show_sta_tr_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT idx;
	STA_TR_ENTRY *tr_entry;

	for (idx = 0; idx < MAX_LEN_OF_TR_TABLE; idx++) {
		tr_entry = &pAd->MacTab.tr_entry[idx];

		if (IS_VALID_ENTRY(tr_entry))
			TRTableEntryDump(pAd, idx, __func__, __LINE__);
	}

	return TRUE;
}

INT show_stainfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	ULONG DataRate = 0, irqflags;
	UCHAR mac_addr[MAC_ADDR_LEN];
	RTMP_STRING *token;
	CHAR sep[1] = {':'};
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Input string=%s\n",
			 __func__, arg));

	for (i = 0, token = rstrtok(arg, &sep[0]); token; token = rstrtok(NULL, &sep[0]), i++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): token(len=%zu) =%s\n",
				 __func__, strlen(token), token));

		if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
			return FALSE;

		AtoH(token, (&mac_addr[i]), 1);
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): i= %d\n", __func__, i));

	if (i != 6)
		return FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nAddr %02x:%02x:%02x:%02x:%02x:%02x\n",
			 PRINT_MAC(mac_addr)));
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	pEntry = MacTableLookup(pAd, (UCHAR *)mac_addr);
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	pEntry = MacTableLookup2(pAd, (UCHAR *)mac_addr, NULL);
#endif

	if (!pEntry)
		return FALSE;

	if (IS_ENTRY_NONE(pEntry)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Invalid MAC address!\n"));
		return FALSE;
	}

	printk("\n");
	printk("EntryType : %d\n", pEntry->EntryType);
	printk("Entry Capability:\n");
	printk("\tPhyMode:%-10s\n", get_phymode_str(pEntry->MaxHTPhyMode.field.MODE));
	printk("\tBW:%-6s\n", get_bw_str(pEntry->MaxHTPhyMode.field.BW));
	printk("\tDataRate:\n");
#ifdef DOT11_VHT_AC

	if (pEntry->MaxHTPhyMode.field.MODE == MODE_VHT)
		printk("%dS-M%d", ((pEntry->MaxHTPhyMode.field.MCS >> 4) + 1), (pEntry->MaxHTPhyMode.field.MCS & 0xf));
	else
#endif /* DOT11_VHT_AC */
		printk(" %-6d", pEntry->MaxHTPhyMode.field.MCS);

	printk(" %-6d", pEntry->MaxHTPhyMode.field.ShortGI);
	printk(" %-6d\n", pEntry->MaxHTPhyMode.field.STBC);
	printk("Entry Operation Features\n");
	printk("\t%-4s%-4s%-4s%-4s%-8s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s%-7s%-7s\n",
		   "AID", "BSS", "PSM", "WMM", "MIMOPS", "RSSI0", "RSSI1",
		   "RSSI2", "PhMd", "BW", "MCS", "SGI", "STBC", "Idle", "Rate");
	DataRate = 0;
	getRate(pEntry->HTPhyMode, &DataRate);
	printk("\t%-4d", (int)pEntry->Aid);
	printk("%-4d", (int)pEntry->func_tb_idx);
	printk("%-4d", (int)pEntry->PsMode);
	printk("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE));
#ifdef DOT11_N_SUPPORT
	printk("%-8d", (int)pEntry->MmpsMode);
#endif /* DOT11_N_SUPPORT */
	printk("%-7d", pEntry->RssiSample.AvgRssi[0]);
	printk("%-7d", pEntry->RssiSample.AvgRssi[1]);
	printk("%-7d", pEntry->RssiSample.AvgRssi[2]);
	printk("%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE));
	printk("%-6s", get_bw_str(pEntry->HTPhyMode.field.BW));
#ifdef DOT11_VHT_AC

	if (pEntry->HTPhyMode.field.MODE == MODE_VHT)
		printk("%dS-M%d", ((pEntry->HTPhyMode.field.MCS >> 4) + 1), (pEntry->HTPhyMode.field.MCS & 0xf));
	else
#endif /* DOT11_VHT_AC */
		printk("%-6d", pEntry->HTPhyMode.field.MCS);

	printk("%-6d", pEntry->HTPhyMode.field.ShortGI);
	printk("%-6d", pEntry->HTPhyMode.field.STBC);
	printk("%-7d", (int)(pEntry->StaIdleTimeout - pEntry->NoDataIdleCount));
	printk("%-7d", (int)DataRate);
	printk("%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount,
		   (pEntry->DebugTxCount) ? ((pEntry->DebugTxCount - pEntry->DebugFIFOCount) * 100 / pEntry->DebugTxCount) : 0);
	printk("\n");
	ASSERT(pEntry->wcid <= GET_MAX_UCAST_NUM(pAd));
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	printk("Entry TxRx Info\n");
	printk("\tEntryType : %d\n", tr_entry->EntryType);
	printk("\tHookingWdev : %p\n", tr_entry->wdev);
	printk("\tIndexing : FuncTd=%d, WCID=%d\n", tr_entry->func_tb_idx, tr_entry->wcid);
	printk("Entry TxRx Features\n");
	printk("\tIsCached, PortSecured, PsMode, LockTx, VndAth\n");
	printk("\t%d\t%d\t%d\t%d\t%d\n", tr_entry->isCached, tr_entry->PortSecured,
		   tr_entry->PsMode, tr_entry->LockEntryTx,
		   tr_entry->bIAmBadAtheros);
	printk("\t%-6s%-6s%-6s%-6s%-6s%-6s%-6s\n", "TxQId", "PktNum", "QHead", "QTail", "EnQCap", "DeQCap", "PktSeq");

	for (i = 0; i < WMM_QUE_NUM;  i++) {
		RTMP_IRQ_LOCK(&tr_entry->txq_lock[i], irqflags);
		printk("\t%d %6d  %p  %6p %d %d %d\n",
			   i,
			   tr_entry->tx_queue[i].Number,
			   tr_entry->tx_queue[i].Head,
			   tr_entry->tx_queue[i].Tail,
			   tr_entry->enq_cap, tr_entry->deq_cap,
			   tr_entry->TxSeq[i]);
		RTMP_IRQ_UNLOCK(&tr_entry->txq_lock[i], irqflags);
	}

	RTMP_IRQ_LOCK(&tr_entry->ps_queue_lock, irqflags);
	printk("\tpsQ %6d  %p  %p %d %d  NoQ:%d\n",
		   tr_entry->ps_queue.Number,
		   tr_entry->ps_queue.Head,
		   tr_entry->ps_queue.Tail,
		   tr_entry->enq_cap, tr_entry->deq_cap,
		   tr_entry->NonQosDataSeq);
	RTMP_IRQ_UNLOCK(&tr_entry->ps_queue_lock, irqflags);
	printk("\n");
	return TRUE;
}

extern INT show_radio_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_devinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR *pstr;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Device MAC\n"));

	if (pAd->OpMode == OPMODE_AP)
		pstr = "AP";
	else if (pAd->OpMode == OPMODE_STA)
		pstr = "STA";
	else
		pstr = "Unknown";

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Operation Mode: %s\n", pstr));
	show_radio_info_proc(pAd, arg);
	return TRUE;
}

INT show_wdev_info(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT idx;

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx] == wdev)
			break;
	}

	if (idx >= WDEV_NUM_MAX) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERR! Cannot found required wdev(%p)!\n", wdev));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WDEV Instance(%d) Info:\n", idx));
	return TRUE;
}

CHAR *wdev_type_str[] = {"AP", "STA", "ADHOC", "WDS", "MESH", "GO", "GC", "APCLI", "REPEATER", "P2P_DEVICE", "Unknown"};

RTMP_STRING *wdev_type2str(int type)
{
	switch (type) {
	case WDEV_TYPE_AP:
		return wdev_type_str[0];

	case WDEV_TYPE_STA:
		return wdev_type_str[1];

	case WDEV_TYPE_ADHOC:
		return wdev_type_str[2];

	case WDEV_TYPE_WDS:
		return wdev_type_str[3];

	case WDEV_TYPE_MESH:
		return wdev_type_str[4];

	case WDEV_TYPE_GO:
		return wdev_type_str[5];

	case WDEV_TYPE_GC:
		return wdev_type_str[6];

	/*case WDEV_TYPE_APCLI:
		return wdev_type_str[7];*/

	case WDEV_TYPE_REPEATER:
		return wdev_type_str[8];

	case WDEV_TYPE_P2P_DEVICE:
		return wdev_type_str[9];

	default:
		return wdev_type_str[10];
	}
}

INT show_sysinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT idx;
	UINT32 total_size = 0, cntr_size;
	struct wifi_dev *wdev;
	UCHAR ext_cha;
	PCI_HIF_T *hif = NULL;
#ifdef CONFIG_STA_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	BSS_TABLE *ScanTab = NULL;
	wdev = &pStaCfg->wdev;
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);
#endif
	hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Device Instance\n"));

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWDEV %02d:", idx));

		if (pAd->wdev_list[idx]) {
			UCHAR *str = NULL;

			wdev = pAd->wdev_list[idx];
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t\tName/Type:%s/%s\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev),
					 wdev_type2str(wdev->wdev_type)));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tWdev(list) Idx:%d\n", wdev->wdev_idx));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tMacAddr:%02x:%02x:%02x:%02x:%02x:%02x\n",
					 PRINT_MAC(wdev->if_addr)));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBSSID:%02x:%02x:%02x:%02x:%02x:%02x\n",
					 PRINT_MAC(wdev->bssid)));
			str = wmode_2_str(wdev->PhyMode);

			if (str) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPhyMode:%s\n", str));
				os_free_mem(str);
			}

			ext_cha = wlan_config_get_ext_cha(wdev);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tChannel:%d,ExtCha:%d\n", wdev->channel, ext_cha));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPortSecured/ForbidTx: %d(%sSecured)/%lx\n",
					 wdev->PortSecured,
					 (wdev->PortSecured == WPA_802_1X_PORT_SECURED ? "" : "Not"),
					 wdev->forbid_data_tx));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEdcaIdx:%d\n", wdev->EdcaIdx));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tif_dev:0x%p\tfunc_dev:[%d]0x%p\tsys_handle:0x%p\n",
					 wdev->if_dev, wdev->func_idx, wdev->func_dev, wdev->sys_handle));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tIgmpSnoopEnable:%d\n", wdev->IgmpSnoopEnable));
#ifdef LINUX

			if (wdev->if_dev) {
				UINT idx, q_num;
				UCHAR *mac_str = RTMP_OS_NETDEV_GET_PHYADDR(wdev->if_dev);

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("\t\tOS NetDev status(%s[%d]-%02x:%02x:%02x:%02x:%02x:%02x):\n",
						  RtmpOsGetNetDevName(wdev->if_dev),
						  RtmpOsGetNetIfIndex(wdev->if_dev),
						  PRINT_MAC(mac_str)));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tdev->state: 0x%lx\n", RtmpOSGetNetDevState(wdev->if_dev)));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tdev->flag: 0x%x\n", RtmpOSGetNetDevFlag(wdev->if_dev)));
				q_num = RtmpOSGetNetDevQNum(wdev->if_dev);

				for (idx = 0; idx < q_num; idx++) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							 ("\t\t\tdev->queue[%d].state: 0x%lx\n", idx,
							  RtmpOSGetNetDevQState(wdev->if_dev, idx)));
				}
			}

#endif /* LINUX */
		} else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Memory Statistics:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsize>\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpAd = \t\t%zu bytes\n\n", sizeof(*pAd)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tCommonCfg = \t%zu bytes\n", sizeof(pAd->CommonCfg)));
	total_size += sizeof(pAd->CommonCfg);
#ifdef CONFIG_AP_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tApCfg = \t%zu bytes\n", sizeof(pAd->ApCfg)));
	total_size += sizeof(pAd->ApCfg);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\t\tMBSSID = \t%zu B (PerMBSS =%zu B, Total MBSS Num= %d)\n",
			 sizeof(pAd->ApCfg.MBSSID), sizeof(struct _BSS_STRUCT), HW_BEACON_MAX_NUM));
#ifdef APCLI_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\t\t\t\tAPCLI = \t%zu bytes (PerAPCLI =%zu bytes, Total APCLI Num= %d)\n",
			  sizeof(pAd->StaCfg), sizeof(struct _STA_ADMIN_CONFIG), MAX_APCLI_NUM));
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef RTMP_MAC_PCI
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tTxRing = \t%zu bytes\n", sizeof(hif->TxRing)));
	total_size += sizeof(hif->TxRing);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tRxRing = \t%zu bytes\n", sizeof(hif->RxRing)));
	total_size += sizeof(hif->RxRing);
#ifdef CONFIG_ANDES_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tCtrlRing = \t%zu bytes\n", sizeof(hif->ctrl_ring)));
	total_size += sizeof(hif->ctrl_ring);
#endif /* CONFIG_ANDES_SUPPORT */
#endif /* RTMP_MAC_PCI */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tMlme = \t%zu bytes\n", sizeof(pAd->Mlme)));
	total_size += sizeof(pAd->Mlme);
#ifdef CONFIG_STA_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tMlmeAux = \t%zu bytes\n", sizeof(pStaCfg->MlmeAux)));
	total_size += sizeof(pStaCfg->MlmeAux);
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tMacTab = \t%zu bytes\n", sizeof(pAd->MacTab)));
	total_size += sizeof(pAd->MacTab);
#ifdef DOT11_N_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tBATable = \t%zu bytes\n", sizeof(pAd->BATable)));
	total_size += sizeof(pAd->BATable);
#endif /* DOT11_N_SUPPORT */
	cntr_size = sizeof(pAd->Counters8023) + sizeof(pAd->WlanCounters) +
				sizeof(pAd->RalinkCounters) + /* sizeof(pAd->DrsCounters) */ +
				sizeof(pAd->PrivateInfo);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tCounter** = \t%d bytes\n", cntr_size));
	total_size += cntr_size;
#ifdef CONFIG_STA_SUPPORT
#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tScanTab = \t%zu bytes\n", sizeof(ScanTab)));
	total_size += sizeof(ScanTab);
#endif
#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsize> Total = \t\t%d bytes, Others = %zu bytes\n\n",
			 total_size, sizeof(*pAd) - total_size));
	return TRUE;
}

void wifi_dump_info(void)
{
	RTMP_ADAPTER *pAd = NULL;
	struct net_device *ndev = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s--------------------\n", __func__));
	ndev = dev_get_by_name(&init_net, "ra0");

	if (ndev) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------RA0--------\n"));
		pAd = ((struct mt_dev_priv *)netdev_priv(ndev))->sys_handle;
		show_tpinfo_proc(pAd, "");
		show_trinfo_proc(pAd, "");
		ShowPLEInfo(pAd, "");
		ShowPseInfo(pAd, "");
		Show_PSTable_Proc(pAd, "");
#ifdef CONFIG_AP_SUPPORT
		set_qiscdump_proc(pAd, "");
#endif /*CONFIG_AP_SUPPORT*/
		show_swqinfo(pAd, "");
#ifdef ERR_RECOVERY
		ShowSerProc2(pAd, "");
#endif
#ifdef FQ_SCH_SUPPORT
		show_fq_info(pAd, "");
#endif
	}

	ndev = dev_get_by_name(&init_net, "rai0");

	if (ndev) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------RAI0--------\n"));
		pAd = ((struct mt_dev_priv *)netdev_priv(ndev))->sys_handle;
		show_tpinfo_proc(pAd, "");
		show_trinfo_proc(pAd, "");
		ShowPLEInfo(pAd, "");
		Show_PSTable_Proc(pAd, "");
#ifdef FQ_SCH_SUPPORT
		show_fq_info(pAd, "");
#endif
	}

	ndev = dev_get_by_name(&init_net, "rae0");

	if (ndev) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------RAE0--------\n"));
		pAd = ((struct mt_dev_priv *)netdev_priv(ndev))->sys_handle;
		show_tpinfo_proc(pAd, "");
		show_trinfo_proc(pAd, "");
		ShowPLEInfo(pAd, "");
		Show_PSTable_Proc(pAd, "");
#ifdef FQ_SCH_SUPPORT
		show_fq_info(pAd, "");
#endif
	}
}
#ifndef MT76XX_COMBO_DUAL_DRIVER_SUPPORT
EXPORT_SYMBOL(wifi_dump_info);
#endif

#ifdef CONFIG_TP_DBG
INT Set_TPDbg_Level(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT dbg;
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;

	dbg = simple_strtol(arg, 0, 10);
	tp_dbg->debug_flag = dbg;
	if (!(dbg & TP_DEBUG_TIMING)) {
		memset(tp_dbg->TRDoneTimesRec, 0x0, sizeof(tp_dbg->TRDoneTimesRec));
		memset(tp_dbg->TRDoneInterval, 0x0, sizeof(tp_dbg->TRDoneInterval));
	}
	MTWF_PRINT("%s(): (TPDebugLevel = %d)\n", __func__, dbg);

	return TRUE;
}

INT show_TPDbg_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 tp_dbg_slot_idx;
	UCHAR dump = 0;
	UCHAR dbg_detail_lvl = DBG_LVL_TRACE;
	UCHAR time_slot_num = TP_DBG_TIME_SLOT_NUMS;

	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;

	if (arg != NULL)
		dump = os_str_toul(arg, 0, 16);

	if (dump == 1)
		dbg_detail_lvl = DBG_LVL_OFF;
	else if (dump == 2)
		dbg_detail_lvl = DBG_LVL_TRACE;
	else if (dump == 3) {
		/* show less information for MSP debugging */
		dbg_detail_lvl = DBG_LVL_OFF;
		time_slot_num = TP_DBG_TIME_SLOT_NUMS/2;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl, ("\tRxDropPacket Count = %d\n", tp_dbg->RxDropPacket));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl,
			 ("\n\tTimeSlot \tTxIsr \tRxIsr/Rx1Isr/RxDlyIsr \t\tTxIoRead/TxIoWrite \tRxIoRead/RxIoWrite \tRx1IoRead/Rx1IoWrite\n"));

	for (tp_dbg_slot_idx = 0; tp_dbg_slot_idx < time_slot_num; tp_dbg_slot_idx++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl,
				 ("\t%d \t\t%d \t%5d/%6d/%8d \t\t%8d/%9d \t%8d/%9d \t%9d/%10d\n",
				tp_dbg_slot_idx, tp_dbg->IsrTxCntRec[tp_dbg_slot_idx],
				tp_dbg->IsrRxCntRec[tp_dbg_slot_idx],
				tp_dbg->IsrRx1CntRec[tp_dbg_slot_idx],
				tp_dbg->IsrRxDlyCntRec[tp_dbg_slot_idx],
				tp_dbg->IoReadTxRec[tp_dbg_slot_idx],
				tp_dbg->IoWriteTxRec[tp_dbg_slot_idx],
				tp_dbg->IoReadRxRec[tp_dbg_slot_idx],
				tp_dbg->IoWriteRxRec[tp_dbg_slot_idx],
				tp_dbg->IoReadRx1Rec[tp_dbg_slot_idx],
				tp_dbg->IoWriteRx1Rec[tp_dbg_slot_idx]));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl,
			("\n\tTimeSlot \tRx0Cnt_A/Cnt_B/Cnt_C/Cnt_D \t\tRx1Cnt_A/Cnt_B/Cnt_C/Cnt_D \t\tTRdone \t\tTRdoneInterval\n"));

	for (tp_dbg_slot_idx = 0; tp_dbg_slot_idx < time_slot_num; tp_dbg_slot_idx++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl, ("\t%d \t\t%8d/%5d/%5d/%5d \t\t%8d/%5d/%5d/%5d \t\t%d \t\t%d\n",
				 tp_dbg_slot_idx, tp_dbg->MaxProcessCntRxRecA[tp_dbg_slot_idx],
				tp_dbg->MaxProcessCntRxRecB[tp_dbg_slot_idx],
				tp_dbg->MaxProcessCntRxRecC[tp_dbg_slot_idx],
				tp_dbg->MaxProcessCntRxRecD[tp_dbg_slot_idx],
				tp_dbg->MaxProcessCntRx1RecA[tp_dbg_slot_idx],
				tp_dbg->MaxProcessCntRx1RecB[tp_dbg_slot_idx],
				tp_dbg->MaxProcessCntRx1RecC[tp_dbg_slot_idx],
				tp_dbg->MaxProcessCntRx1RecD[tp_dbg_slot_idx],
				tp_dbg->TRDoneTimesRec[tp_dbg_slot_idx],
				tp_dbg->TRDoneInterval[tp_dbg_slot_idx]));
	}
	return TRUE;
}
#endif /* CONFIG_TP_DBG */

INT show_tpinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 i;
#ifdef CUT_THROUGH
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)(pAd->PktTokenCb);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);

#ifdef CUT_THROUGH_DBG
	UINT8 SlotIndex;
#endif
#endif
#ifdef DBG_AMSDU
	UINT8 slot_index;
	STA_TR_ENTRY *tr_entry = NULL;
	MAC_TABLE_ENTRY *mac_table_entry = NULL;
#endif
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tr_delay_control *tr_delay_ctl = &pAd->tr_ctl.tr_delay_ctl;

	UCHAR dump = 0;
	UCHAR dbg_lvl = DBG_LVL_OFF;
	UCHAR dbg_detail_lvl = DBG_LVL_TRACE;
	PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	if (arg != NULL)
		dump = os_str_toul(arg, 0, 16);

	if (dump == 1) {
		dbg_lvl = DBG_LVL_OFF;
		dbg_detail_lvl = DBG_LVL_OFF;
	} else if (dump == 2) {
		dbg_lvl = DBG_LVL_TRACE;
		dbg_detail_lvl = DBG_LVL_TRACE;
	}

#ifdef CUT_THROUGH
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("TxFreeToken Configuration\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("\tTxFreeToken Number = %d\n",
			 pktTokenCb->tx_id_list.list->FreeTokenCnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("\tTxFreeToken LowMark = %d\n", pktTokenCb->TxTokenLowWaterMark));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("\tTxFreeToken HighMark = %d\n", pktTokenCb->TxTokenHighWaterMark));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("\tTotalTxUsedToken Number = %d\n",
			 pktTokenCb->tx_id_list.list->TotalTxUsedTokenCnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("\tTotalTxBackToken Number = %d\n",
			 pktTokenCb->tx_id_list.list->TotalTxBackTokenCnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("\tTotalTxTokenEvent Number = %d\n",
			 pktTokenCb->tx_id_list.list->TotalTxTokenEventCnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("\tTotalTxToken(From CR4) Number = %d\n",
			 pktTokenCb->tx_id_list.list->TotalTxTokenCnt));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxTokenFull Count = %d\n", pktTokenCb->TxTokenFullCnt));
#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl, ("TxFreeToken Usage\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl,
			 ("\tTimeSlot \tUsedTokenCnt \tBackTokenCnt \tAgg0_31 \tAgg32_63 \tAgg64_95 \tAgg96_127\n"));

	for (SlotIndex = 0; SlotIndex < TIME_SLOT_NUMS; SlotIndex++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl, ("\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d\n", SlotIndex,
				 pktTokenCb->tx_id_list.list->UsedTokenCntRec[SlotIndex],
				 pktTokenCb->tx_id_list.list->BackTokenCntRec[SlotIndex],
				 pktTokenCb->tx_id_list.list->FreeAgg0_31Rec[SlotIndex],
				 pktTokenCb->tx_id_list.list->FreeAgg32_63Rec[SlotIndex],
				 pktTokenCb->tx_id_list.list->FreeAgg64_95Rec[SlotIndex],
				 pktTokenCb->tx_id_list.list->FreeAgg96_127Rec[SlotIndex]));
	}

#endif

	for (i = 0; i < num_of_tx_ring; i++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("\tRing %d TxRing LowMark = %d\n", i,
						hif->TxRing[i].tx_ring_low_water_mark));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("\tRing %d TxRing HighMark = %d\n", i,
						hif->TxRing[i].tx_ring_high_water_mark));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("\tRing %d TxRing State = %d\n", i,
						hif->TxRing[i].tx_ring_state));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRing %d TxRingFull Count = %d\n", i,
						hif->TxRing[i].tx_ring_full_cnt));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWrong Wlan Index Num = %d\n", pAd->wrong_wlan_idx_num));
#ifdef DBG_AMSDU
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl, ("TX AMSDU Usage\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl, ("\tTimeSlot \tamsdu_1 \tamsdu_2 \tamsdu_3 \tamsdu_4\n"));

	for (i = 0; i < MAX_LEN_OF_TR_TABLE; i++) {
		tr_entry = &pAd->MacTab.tr_entry[i];
		mac_table_entry = &pAd->MacTab.Content[i];
		if (!IS_ENTRY_NONE(tr_entry)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl, ("\ttr_entry index = %d, amsdu_limit_len_adjust = %d\n", i, mac_table_entry->amsdu_limit_len_adjust));
			for (slot_index = 0; slot_index < TIME_SLOT_NUMS; slot_index++) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl, ("\t%d \t\t%d \t\t%d \t\t%d \t\t%d\n", slot_index,
						 tr_entry->amsdu_1_rec[slot_index],
						 tr_entry->amsdu_2_rec[slot_index],
						 tr_entry->amsdu_3_rec[slot_index],
						 tr_entry->amsdu_4_rec[slot_index]));
			}
		}
	}
#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("RxFreeToken Configuration\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("\tRxFreeToken Number = %d\n",
			 pktTokenCb->rx_id_list.list->FreeTokenCnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("\tRxFreeToken LowMark = %d\n", pktTokenCb->RxTokenLowWaterMark));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_lvl, ("\tRxFreeToken HighMark = %d\n", pktTokenCb->RxTokenHighWaterMark));
#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl, ("RxFreeToken Usage\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl,
			 ("\tTimeSlot \tUsedTokenCnt \tBackTokenCnt \tAgg0_31 \tAgg32_63 \tAgg64_95 \tAgg96_127 \tDropCnt\n"));

	for (SlotIndex = 0; SlotIndex < TIME_SLOT_NUMS; SlotIndex++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, dbg_detail_lvl, ("\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d\n",
				 SlotIndex,
				 pktTokenCb->rx_id_list.list->UsedTokenCntRec[SlotIndex],
				 pktTokenCb->rx_id_list.list->BackTokenCntRec[SlotIndex],
				 pktTokenCb->rx_id_list.list->FreeAgg0_31Rec[SlotIndex],
				 pktTokenCb->rx_id_list.list->FreeAgg32_63Rec[SlotIndex],
				 pktTokenCb->rx_id_list.list->FreeAgg64_95Rec[SlotIndex],
				 pktTokenCb->rx_id_list.list->FreeAgg96_127Rec[SlotIndex],
				 pktTokenCb->rx_id_list.list->DropPktCntRec[SlotIndex]));
	}

#endif
#endif

#ifdef CONFIG_TP_DBG
	show_TPDbg_info_proc(pAd, arg);
#endif /* CONFIG_TP_DBG */

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsw_q_drop cnt = %d\n", pAd->tr_ctl.tx_sw_q_drop));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tnet_if_stop_cnt = %d\n", pAd->tr_ctl.net_if_stop_cnt));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rx_delay_en = %d\n", tr_delay_ctl->rx_delay_en));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_delay_en = %d\n", tr_delay_ctl->tx_delay_en));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rx_icv_err_cnt = %d\n", tr_ctl->rx_icv_err_cnt));

	return TRUE;
}

INT show_trinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG flags;
	NDIS_SPIN_LOCK *lock;
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	UINT8 num_of_rx_ring = GET_NUM_OF_RX_RING(cap);
	PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	if (IS_RBUS_INF(pAd) || IS_PCI_INF(pAd)) {
		UINT32 tbase[num_of_tx_ring], tcnt[num_of_tx_ring];
		UINT32 tcidx[num_of_tx_ring], tdidx[num_of_tx_ring];
		UINT32 rbase[num_of_rx_ring], rcnt[num_of_rx_ring];
		UINT32 rcidx[num_of_rx_ring], rdidx[num_of_rx_ring];
		UINT32 mbase[4] = {0}, mcnt[4] = {0}, mcidx[4] = {0}, mdidx[4] = {0};
		UINT32 sys_ctrl[4];
		UINT32 cr_int_src, cr_int_mask, cr_delay_int, cr_wpdma_glo_cfg;
#ifdef MT7626_E2_SUPPORT
		UINT32 md_sys_ctrl[4];
		UINT32 md_cr_int_src, md_cr_int_mask, md_cr_delay_int, md_cr_wpdma_glo_cfg;
#endif
		INT idx;
		INT TxHwRingNum = num_of_tx_ring;
		INT RxHwRingNum = num_of_rx_ring;
#ifdef ERR_RECOVERY

		if (IsStopingPdma(&pAd->ErrRecoveryCtl))
			return TRUE;

#endif /* ERR_RECOVERY */

		for (idx = 0; idx < TxHwRingNum; idx++) {
			lock = &hif->TxRing[idx].ring_lock;
			RTMP_IRQ_LOCK(lock, flags);
			HIF_IO_READ32(pAd->hdev_ctrl, hif->TxRing[idx].hw_desc_base, &tbase[idx]);
			HIF_IO_READ32(pAd->hdev_ctrl, hif->TxRing[idx].hw_cnt_addr, &tcnt[idx]);
			HIF_IO_READ32(pAd->hdev_ctrl, hif->TxRing[idx].hw_cidx_addr, &tcidx[idx]);
			HIF_IO_READ32(pAd->hdev_ctrl, hif->TxRing[idx].hw_didx_addr, &tdidx[idx]);
			RTMP_IRQ_UNLOCK(lock, flags);
		}

		lock = &hif->ctrl_ring.ring_lock;
		RTMP_IRQ_LOCK(lock, flags);
		HIF_IO_READ32(pAd->hdev_ctrl, hif->ctrl_ring.hw_desc_base, &mbase[1]);
		HIF_IO_READ32(pAd->hdev_ctrl, hif->ctrl_ring.hw_cnt_addr, &mcnt[1]);
		HIF_IO_READ32(pAd->hdev_ctrl, hif->ctrl_ring.hw_cidx_addr, &mcidx[1]);
		HIF_IO_READ32(pAd->hdev_ctrl, hif->ctrl_ring.hw_didx_addr, &mdidx[1]);
		RTMP_IRQ_UNLOCK(lock, flags);
		lock = &hif->fwdl_ring.ring_lock;
		RTMP_IRQ_LOCK(lock, flags);
		HIF_IO_READ32(pAd->hdev_ctrl, hif->fwdl_ring.hw_desc_base, &mbase[2]);
		HIF_IO_READ32(pAd->hdev_ctrl, hif->fwdl_ring.hw_cnt_addr, &mcnt[2]);
		HIF_IO_READ32(pAd->hdev_ctrl, hif->fwdl_ring.hw_cidx_addr, &mcidx[2]);
		HIF_IO_READ32(pAd->hdev_ctrl, hif->fwdl_ring.hw_didx_addr, &mdidx[2]);
		RTMP_IRQ_UNLOCK(lock, flags);

		for (idx = 0; idx < RxHwRingNum; idx++) {
			lock = &hif->RxRing[idx].ring_lock;
			RTMP_IRQ_LOCK(lock, flags);
			HIF_IO_READ32(pAd->hdev_ctrl, hif->RxRing[idx].hw_desc_base, &rbase[idx]);
			HIF_IO_READ32(pAd->hdev_ctrl, hif->RxRing[idx].hw_cnt_addr, &rcnt[idx]);
			HIF_IO_READ32(pAd->hdev_ctrl, hif->RxRing[idx].hw_cidx_addr, &rcidx[idx]);
			HIF_IO_READ32(pAd->hdev_ctrl, hif->RxRing[idx].hw_didx_addr, &rdidx[idx]);
			RTMP_IRQ_UNLOCK(lock, flags);
		}

		cr_int_src = cr_int_mask = cr_wpdma_glo_cfg = cr_delay_int = 0;
#ifdef MT_MAC

		/* TODO: shiang-7603 */
		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			cr_int_src = MT_INT_SOURCE_CSR;
			cr_int_mask = MT_INT_MASK_CSR;
			cr_delay_int = MT_DELAY_INT_CFG;
			cr_wpdma_glo_cfg = MT_WPDMA_GLO_CFG;
#ifdef MT7626_E2_SUPPORT
			md_cr_int_src = MT_INT2_SOURCE_CSR;
			md_cr_int_mask = MT_INT2_MASK_CSR;
			md_cr_delay_int = MT_MD_WPDMA_DELAY_INT_CFG;
			md_cr_wpdma_glo_cfg = MT_MD_WPDMA_GLO_CFG;
#endif
		}

#endif /* MT_MAC */
		HIF_IO_READ32(pAd->hdev_ctrl, cr_int_src, &sys_ctrl[0]);
		HIF_IO_READ32(pAd->hdev_ctrl, cr_int_mask, &sys_ctrl[1]);
		HIF_IO_READ32(pAd->hdev_ctrl, cr_delay_int, &sys_ctrl[2]);
		HIF_IO_READ32(pAd->hdev_ctrl, cr_wpdma_glo_cfg, &sys_ctrl[3]);
#ifdef MT7626_E2_SUPPORT
		HIF_IO_READ32(pAd->hdev_ctrl, md_cr_int_src, &md_sys_ctrl[0]);
		HIF_IO_READ32(pAd->hdev_ctrl, md_cr_int_mask, &md_sys_ctrl[1]);
		HIF_IO_READ32(pAd->hdev_ctrl, md_cr_delay_int, &md_sys_ctrl[2]);
		HIF_IO_READ32(pAd->hdev_ctrl, md_cr_wpdma_glo_cfg, &md_sys_ctrl[3]);
#endif
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxRing Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRingIdx Reg \tBase \t\tCnt \tCIDX \tDIDX \tSWIDX\n"));

		for (idx = 0; idx < TxHwRingNum; idx++) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t%d \t0x%04x \t0x%08x \t0x%x \t0x%x \t0x%x \t0x%x\n",
				idx, hif->TxRing[idx].hw_desc_base, tbase[idx],
				tcnt[idx], tcidx[idx], tdidx[idx], hif->TxRing[idx].TxSwFreeIdx));
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRingIdx \tTx Free TxD resource\n"));

		for (idx = 0; idx < TxHwRingNum; idx++) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t%d\n",
					 idx, asic_get_tx_resource_free_num(pAd, idx)));
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nRxRing Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRingIdx Reg \tBase \t\tCnt \tCIDX \tDIDX \tSWIDX\n"));

		for (idx = 0; idx < RxHwRingNum; idx++) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t%d \t0x%04x \t0x%08x \t0x%x \t0x%x \t0x%x \t0x%x\n",
				idx, hif->RxRing[idx].hw_desc_base, rbase[idx],
				rcnt[idx], rcidx[idx], rdidx[idx], hif->RxRing[idx].RxSwReadIdx));
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRingIdx \tRx Pending RX Packet\n"));

		for (idx = 0; idx < RxHwRingNum; idx++) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t\t%d\n",
					 idx, GET_RXRING_PENDINGNO(pAd, idx)));
		}

#ifdef CONFIG_ANDES_SUPPORT
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nCtrlRing Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRingIdx Reg \tBase \t\tCnt \tCIDX \tDIDX \tSWIDX\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t0x%04x \t0x%08x \t0x%x \t0x%x \t0x%x \t0x%x\n",
				 0, hif->ctrl_ring.hw_desc_base, mbase[1], mcnt[1], mcidx[1], mdidx[1], hif->ctrl_ring.TxSwFreeIdx));
#endif /* CONFIG_ANDES_SUPPORT */

		/*remain the MT7615xxxxx chips judgement here, in case of the chip is MT7628/7603 which has not fwdl ring.*/
		if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_P18(pAd) || IS_MT7663(pAd) || IS_AXE(pAd) || IS_MT7626(pAd)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nFwDwloadRing Configuration\n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRingIdx Reg \tBase \t\tCnt \tCIDX \tDIDX \tSWIDX\n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t0x%04x \t0x%08x \t0x%x \t0x%x \t0x%x \t0x%x\n",
					 0, hif->fwdl_ring.hw_desc_base, mbase[2], mcnt[2], mcidx[2], mdidx[2], hif->fwdl_ring.TxSwFreeIdx));
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Interrupt Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tIntCSR \tIntMask \tDelayINT\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t0x%x \t0x%x \t0x%x\n", sys_ctrl[0], sys_ctrl[1], sys_ctrl[2]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tIntPending \tintDisableMask\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t0x%x \t0x%x\n", hif->IntPending,
				 hif->intDisableMask));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DMA Configuration(0x%x)\n", sys_ctrl[3]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTx/RxDMAEn=%d/%d, \tTx/RxDMABusy=%d/%d\n",
				 sys_ctrl[3] & 0x1, sys_ctrl[3] & 0x4,
				 sys_ctrl[3] & 0x2, sys_ctrl[3] & 0x8));
#ifdef MT7626_E2_SUPPORT
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MD Interrupt Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tIntCSR \tIntMask \tDelayINT\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t0x%x \t0x%x \t0x%x\n", md_sys_ctrl[0], md_sys_ctrl[1], md_sys_ctrl[2]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tIntPending \tintDisableMask\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t0x%x \t0x%x\n", hif->IntPending_md,
				 hif->intDisableMask_md));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DMA Configuration(0x%x)\n", md_sys_ctrl[3]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTx/RxDMAEn=%d/%d, \tTx/RxDMABusy=%d/%d\n",
				 md_sys_ctrl[3] & 0x1, md_sys_ctrl[3] & 0x4,
				 md_sys_ctrl[3] & 0x2, md_sys_ctrl[3] & 0x8));
#endif
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}

#endif /* defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT) */
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
#ifdef RTMP_PCI_SUPPORT

		if (IS_PCI_INF(pAd)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PDMA Info\n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("\tPDMAMonitorEn=%d, TxRCntr = %ld, TxDMACheckTimes = %d,  RxRCounter = %ld, RxDMACheckTimes = %d, PDMARFailCount = %ld\n",
					  pAd->PDMAWatchDogEn, pAd->TxDMAResetCount,
					  pAd->TxDMACheckTimes, pAd->RxDMAResetCount,
					  pAd->RxDMACheckTimes, pAd->PDMAResetFailCount));
		}

#endif
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PSE Info\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tPSEMonitorEn=%d, RCounter = %lu, RxPseCheckTimes = %d, PSETriggerType1Count = %lu, PSETriggerType2Count = %lu, PSERFailCount = %lu\n",
				  pAd->PSEWatchDogEn, pAd->PSEResetCount,
				  pAd->RxPseCheckTimes, pAd->PSETriggerType1Count,
				  pAd->PSETriggerType2Count, pAd->PSEResetFailCount));
	}

#endif
#ifdef INT_STATISTIC
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("INT_CNT=%lu INT_TxCoherent_CNT=%lu INT_RxCoherent_CNT=%lu INT_FifoStaFullInt_CNT=%lu INT_MGMTDLY_CNT=%lu INT_RXDATA_CNT=%lu pAd->INT_RXCMD_CNT=%lu INT_HCCA_CNT=%lu INT_AC3_CNT=%lu INT_AC2_CNT=%lu INT_AC1_CNT=%lu INT_AC0_CNT=%lu INT_PreTBTT_CNT=%lu INT_TBTTInt_CNT=%lu INT_GPTimeOut_CNT=%lu INT_Radar_CNT=%lu\n",
			  pAd->INTCNT,
			  pAd->INTTxCoherentCNT,
			  pAd->INTRxCoherentCNT,
			  pAd->INTFifoStaFullIntCNT,
			  pAd->INTMGMTDLYCNT,
			  pAd->INTRXDATACNT,
			  pAd->INTRXCMDCNT,
			  pAd->INTHCCACNT,
			  pAd->INTAC3CNT,
			  pAd->INTAC2CNT,
			  pAd->INTAC1CNT,
			  pAd->INTAC0CNT,
			  pAd->INTPreTBTTCNT,
			  pAd->INTTBTTIntCNT,
			  pAd->INTGPTimeOutCNT,
			  pAd->INTRadarCNT));
#ifdef MT_MAC
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("INTWFMACINT0CNT=%lu INTWFMACINT1CNT=%lu INTWFMACINT2CNT=%lu INTWFMACINT3CNT=%lu INTWFMACINT4CNT=%lu INTBCNDLY=%lu INTBMCDLY=%lu\n",
			  pAd->INTWFMACINT0CNT,
			  pAd->INTWFMACINT1CNT,
			  pAd->INTWFMACINT2CNT,
			  pAd->INTWFMACINT3CNT,
			  pAd->INTWFMACINT4CNT,
			  pAd->INTBCNDLY,
			  pAd->INTBMCDLY));
#endif
#endif
	return TRUE;
}

INT show_swqinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct qm_ops *ops = pAd->qm_ops;

	if (ops->dump_all_sw_queue) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: show_swqinfo\n", __func__));

		ops->dump_all_sw_queue(pAd);
	}

	return TRUE;
}

INT show_txqinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct qm_ops *ops = pAd->qm_ops;
	CHAR *param;
	UCHAR wcid = 0;
	UCHAR q_idx = 0;
	enum PACKET_TYPE pkt_type = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s::param = %s\n", __func__, arg));

	if (arg == NULL)
		goto error;

	param = rstrtok(arg, ":");

	if (param != NULL)
		wcid = os_str_tol(param, 0, 10);
	else
		goto error;

	param = rstrtok(NULL, ":");

	if (param != NULL)
		pkt_type = os_str_tol(param, 0, 10);
	else
		goto error;

	param = rstrtok(NULL, ":");

	if (param != NULL)
		q_idx = os_str_tol(param, 0, 10);
	else
		goto error;

	if (ops->sta_dump_queue) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s::wcid = %d, pkt_type = %d, q_idx = %d\n",
			  __func__, wcid, pkt_type, q_idx));

		ops->sta_dump_queue(pAd, wcid, pkt_type, q_idx);
	}

	return TRUE;

error:

	return 0;
}

#ifdef WSC_STA_SUPPORT
INT	Show_WpsManufacturer_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	sprintf(pBuf, "\tManufacturer = %s", pStaCfg->wdev.WscControl.RegData.SelfInfo.Manufacturer);
	return 0;
}

INT	Show_WpsModelName_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	sprintf(pBuf, "\tModelName = %s", pStaCfg->wdev.WscControl.RegData.SelfInfo.ModelName);
	return 0;
}

INT	Show_WpsDeviceName_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	sprintf(pBuf, "\tDeviceName = %s", pStaCfg->wdev.WscControl.RegData.SelfInfo.DeviceName);
	return 0;
}

INT	Show_WpsModelNumber_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	sprintf(pBuf, "\tModelNumber = %s", pStaCfg->wdev.WscControl.RegData.SelfInfo.ModelNumber);
	return 0;
}

INT	Show_WpsSerialNumber_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	sprintf(pBuf, "\tSerialNumber = %s", pStaCfg->wdev.WscControl.RegData.SelfInfo.SerialNumber);
	return 0;
}
#endif /* WSC_STA_SUPPORT */

#ifdef SINGLE_SKU
INT	Show_ModuleTxpower_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\tModuleTxpower = %d", pAd->CommonCfg.ModuleTxpower);
	return 0;
}
#endif /* SINGLE_SKU */

#ifdef APCLI_SUPPORT
INT RTMPIoctlConnStatus(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i = 0;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	BOOLEAN bConnect = FALSE;
	struct wifi_dev *wdev = NULL;
#ifdef MAC_REPEATER_SUPPORT
	MBSS_TO_CLI_LINK_MAP_T *pMbssToCliLinkMap = NULL;
	INT	MbssIdx;
#endif
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>RTMPIoctlConnStatus\n"));

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	wdev = &pAd->StaCfg[ifIndex].wdev;
	if (!wdev)
		return FALSE;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================================\n"));

	if (((GetAssociatedAPByWdev(pAd, wdev)) != NULL) && (pAd->StaCfg[ifIndex].SsidLen != 0)) {
		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
			STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[i];

			if (IS_ENTRY_PEER_AP(pEntry)
				&& (pEntry->Sst == SST_ASSOC)
				&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
				if (pEntry->wdev == &pAd->StaCfg[ifIndex].wdev) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							 ("ApCli%d         Connected AP : %02X:%02X:%02X:%02X:%02X:%02X   SSID:%s\n",
							  ifIndex, PRINT_MAC(pEntry->Addr), pAd->StaCfg[ifIndex].Ssid));
					bConnect = TRUE;
				}
			}

#ifdef MAC_REPEATER_SUPPORT
			else if (IS_ENTRY_REPEATER(pEntry)
					 && (pEntry->Sst == SST_ASSOC)
					 && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
				if (pEntry->wdev == &pAd->StaCfg[ifIndex].wdev) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							 ("Rept[wcid=%-3d] Connected AP : %02X:%02X:%02X:%02X:%02X:%02X   SSID:%s\n",
							  i, PRINT_MAC(pEntry->Addr), pAd->StaCfg[ifIndex].Ssid));
					bConnect = TRUE;
				}
			}

#endif
		}

		if (!bConnect)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ApCli%d Connected AP : Disconnect\n", ifIndex));
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ApCli%d Connected AP : Disconnect\n", ifIndex));

#ifdef MAC_REPEATER_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ApCli%d CliLinkMap ra:", ifIndex));

	for (MbssIdx = 0; MbssIdx < pAd->ApCfg.BssidNum; MbssIdx++) {
		pMbssToCliLinkMap = &pAd->ApCfg.MbssToCliLinkMap[MbssIdx];

		if (pMbssToCliLinkMap->cli_link_wdev == &pAd->StaCfg[ifIndex].wdev)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d ", MbssIdx));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\r"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Ignore repeater MAC address\n\r"));

	for (i = 0; i < MAX_IGNORE_AS_REPEATER_ENTRY_NUM; i++) {
		INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;

		pEntry = &pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntry[i];

		if (pEntry->bInsert)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%d]%02X:%02X:%02X:%02X:%02X:%02X\n\r", i,
					 PRINT_MAC(pEntry->MacAddr)));
	}

#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\r"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================================\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<==RTMPIoctlConnStatus\n"));
	return TRUE;
}
#endif/*APCLI_SUPPORT*/

INT32 getLegacyOFDMMCSIndex(UINT8 MCS)
{
	INT32 mcs_index = MCS;
	return mcs_index;
}

void  getRate(HTTRANSMIT_SETTING HTSetting, ULONG *fLastTxRxRate)

{
	UINT8					Antenna = 0;
	UINT8					MCS = HTSetting.field.MCS;
	int rate_count = sizeof(MCSMappingRateTable) / sizeof(int);
	int rate_index = 0;
	int value = 0;
#ifdef DOT11_VHT_AC

	if (HTSetting.field.MODE >= MODE_VHT) {
		MCS = HTSetting.field.MCS & 0xf;
		Antenna = (HTSetting.field.MCS >> 4) + 1;

		if (HTSetting.field.BW == BW_20) {
			rate_index = 112 + ((Antenna - 1) * 10) +
						 ((UCHAR)HTSetting.field.ShortGI * 160) +
						 ((UCHAR)MCS);
		} else if (HTSetting.field.BW == BW_40) {
			rate_index = 152 + ((Antenna - 1) * 10) +
						 ((UCHAR)HTSetting.field.ShortGI * 160) +
						 ((UCHAR)MCS);
		} else if (HTSetting.field.BW == BW_80) {
			rate_index = 192 + ((Antenna - 1) * 10) +
						 ((UCHAR)HTSetting.field.ShortGI * 160) +
						 ((UCHAR)MCS);
		} else if (HTSetting.field.BW == BW_160) {
			rate_index = 232 + ((Antenna - 1) * 10) +
						 ((UCHAR)HTSetting.field.ShortGI * 160) +
						 ((UCHAR)MCS);
		}
	} else
#endif /* DOT11_VHT_AC */
#ifdef DOT11_N_SUPPORT
		if (HTSetting.field.MODE >= MODE_HTMIX) {
			MCS = HTSetting.field.MCS;

			if ((HTSetting.field.MODE == MODE_HTMIX)
				|| (HTSetting.field.MODE == MODE_HTGREENFIELD))
				Antenna = (MCS >> 3) + 1;

			/* map back to 1SS MCS , multiply by antenna numbers later */
			if (MCS > 7)
				MCS %= 8;

			rate_index = 16 + ((UCHAR)HTSetting.field.BW * 24) + ((UCHAR)HTSetting.field.ShortGI * 48) + ((UCHAR)MCS);
		} else
#endif /* DOT11_N_SUPPORT */
			if (HTSetting.field.MODE == MODE_OFDM)
				rate_index = getLegacyOFDMMCSIndex(HTSetting.field.MCS) + 4;
			else if (HTSetting.field.MODE == MODE_CCK)
				rate_index = (UCHAR)(HTSetting.field.MCS);

	if (rate_index < 0)
		rate_index = 0;

	if (rate_index >= rate_count)
		rate_index = rate_count - 1;

	if (HTSetting.field.MODE != MODE_VHT)
		value = (MCSMappingRateTable[rate_index] * 5) / 10;
	else
		value =  MCSMappingRateTable[rate_index];

#if defined(DOT11_VHT_AC) || defined(DOT11_N_SUPPORT)

	if (HTSetting.field.MODE >= MODE_HTMIX && HTSetting.field.MODE < MODE_VHT)
		value *= Antenna;

#endif /* DOT11_VHT_AC */
	*fLastTxRxRate = (ULONG)value;
	return;
}

void getRateEtc(HTTRANSMIT_SETTING HTSetting,
                ULONG *Rate,
                UINT8 *Antenna,
                UINT8 *Bandwidth,
                UINT8 *MCS,
                UINT16 *GuardNS)
{
	int rate_count = sizeof(MCSMappingRateTable) / sizeof(int);
	int rate_index = 0;
	int value = 0;
	UINT8 ShortGI = HTSetting.field.ShortGI;
	UINT8 BW = HTSetting.field.BW;

	*Antenna = 0;
	*Bandwidth = (BW == BW_20) ? 20 : (BW == BW_40) ? 40 : (BW == BW_80) ? 80 : -1;
	*MCS = HTSetting.field.MCS;
	*GuardNS = ShortGI ? 400 : 800;

#ifdef DOT11_VHT_AC
	if (HTSetting.field.MODE >= MODE_VHT) {
		*MCS = HTSetting.field.MCS & 0xf;
		*Antenna = (HTSetting.field.MCS >> 4) + 1;
		if (BW == BW_20) {
			rate_index = 112 + ((*Antenna - 1) * 10) + (ShortGI * 160) + (*MCS);
		} else if (BW == BW_40) {
			rate_index = 152 + ((*Antenna - 1) * 10) + (ShortGI * 160) + (*MCS);
		} else if (BW == BW_80) {
			rate_index = 192 + ((*Antenna - 1) * 10) + (ShortGI * 160) + (*MCS);
		} else if (BW == BW_160) {
			rate_index = 232 + ((*Antenna - 1) * 10) + (ShortGI * 160) + (*MCS);
		}
	} else
#endif /* DOT11_VHT_AC */
#ifdef DOT11_N_SUPPORT
		if (HTSetting.field.MODE >= MODE_HTMIX) {
			*MCS = HTSetting.field.MCS;

			if ((HTSetting.field.MODE == MODE_HTMIX)
				|| (HTSetting.field.MODE == MODE_HTGREENFIELD))
				*Antenna = (*MCS >> 3) + 1;

			/* map back to 1SS MCS , multiply by antenna numbers later */
			if (*MCS > 7)
				*MCS %= 8;

			rate_index = 16 + (BW * 24) + (ShortGI * 48) + (*MCS);
		} else
#endif /* DOT11_N_SUPPORT */
			if (HTSetting.field.MODE == MODE_OFDM)
				rate_index = getLegacyOFDMMCSIndex(HTSetting.field.MCS) + 4;
			else if (HTSetting.field.MODE == MODE_CCK)
				rate_index = *MCS;

	if (rate_index < 0)
		rate_index = 0;

	if (rate_index >= rate_count)
		rate_index = rate_count - 1;

	if (HTSetting.field.MODE != MODE_VHT)
		value = (MCSMappingRateTable[rate_index] * 5) / 10;
	else
		value =  MCSMappingRateTable[rate_index];

#if defined(DOT11_VHT_AC) || defined(DOT11_N_SUPPORT)

	if (HTSetting.field.MODE >= MODE_HTMIX && HTSetting.field.MODE < MODE_VHT)
		value *= *Antenna;

#endif /* DOT11_VHT_AC */
	*Rate = (ULONG)value;
	return;
}

#ifdef MT_MAC
INT Show_TxVinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_txv_info)
		return chip_dbg->show_txv_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}
#endif

#ifdef TXBF_SUPPORT

/*
	Set_InvTxBfTag_Proc - Invalidate BF Profile Tags
		usage: "iwpriv ra0 set InvTxBfTag=n"
		Reset Valid bit and zero out MAC address of each profile. The next profile will be stored in profile 0
*/
INT	Set_InvTxBfTag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}

#ifdef MT_MAC
/*
	Set_ETxBfCodebook_Proc - Set ETxBf Codebook
	usage: iwpriv ra0 set ETxBfCodebook=0 to 3
*/
INT Set_ETxBfCodebook_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG t = os_str_tol(arg, 0, 10);

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
				 __func__));
		return FALSE;
	}

	if (t > 3) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_ETxBfCodebook_Proc: value > 3!\n"));
		return FALSE;
	}

	return TRUE;
}

/*
	Set_ETxBfCoefficient_Proc - Set ETxBf Coefficient
		usage: iwpriv ra0 set ETxBfCoefficient=0 to 3
*/
INT Set_ETxBfCoefficient_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG t = os_str_tol(arg, 0, 10);

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
				 __func__));
		return FALSE;
	}

	if (t > 3) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_ETxBfCoefficient_Proc: value > 3!\n"));
		return FALSE;
	}

	return TRUE;
}

/*
	Set_ETxBfGrouping_Proc - Set ETxBf Grouping
		usage: iwpriv ra0 set ETxBfGrouping=0 to 2
*/
INT Set_ETxBfGrouping_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG t = os_str_tol(arg, 0, 10);

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
				 __func__));
		return FALSE;
	}

	if (t > 2) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_ETxBfGrouping_Proc: value > 2!\n"));
		return FALSE;
	}

	return TRUE;
}
#endif /* MT_MAC */

/*
	Set_ETxBfNoncompress_Proc - Set ETxBf Noncompress option
		usage: iwpriv ra0 set ETxBfNoncompress=0 or 1
*/
INT Set_ETxBfNoncompress_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG t = os_str_tol(arg, 0, 10);

	if (t > 1) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_ETxBfNoncompress_Proc: value > 1!\n"));
		return FALSE;
	}

	pAd->CommonCfg.ETxBfNoncompress = t;
	return TRUE;
}

/*
	Set_ETxBfIncapable_Proc - Set ETxBf Incapable option
		usage: iwpriv ra0 set ETxBfIncapable=0 or 1
*/
INT Set_ETxBfIncapable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG t = os_str_tol(arg, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	if (t > 1)
		return FALSE;

	pAd->CommonCfg.ETxBfIncapable = t;
#ifdef MT_MAC
	mt_WrapSetETxBFCap(pAd, wdev, &pAd->CommonCfg.HtCapability.TxBFCap);
#endif /* MT_MAC */
	return TRUE;
}

/*
	Set_ITxBfDivCal_Proc - Calculate ITxBf Divider Calibration parameters
	usage: iwpriv ra0 set ITxBfDivCal=dd
			0=>display calibration parameters
			1=>update EEPROM values
			2=>update BBP R176
			10=>display calibration parameters and dump capture data
			11=>Skip divider calibration, just capture and dump capture data
*/
INT	Set_ITxBfDivCal_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int calFunction;
	UINT32 value, value1, restore_value, loop = 0;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	/* backup mac 1004 value */
	RTMP_IO_READ32(pAd->hdev_ctrl, 0x1004, &restore_value);
	/* Backup the original RTS retry count and then set to 0 */
	/* RTMP_IO_READ32(pAd->hdev_ctrl, 0x1344, &pAd->rts_tx_retry_num); */
	/* disable mac tx/rx */
	value = restore_value;
	value &= ~0xC;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x1004, value);
	/* set RTS retry count = 0 */
	RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x1344, 0x00092B00);

	/* wait mac 0x1200, bbp 0x2130 idle */
	do {
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x1200, &value);
		value &= 0x1;
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x2130, &value1);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:: Wait until MAC 0x1200 bit0 and BBP 0x2130 become 0\n",
				 __func__));
		RtmpusecDelay(1);
		loop++;
	} while (((value != 0) || (value1 != 0)) && (loop < 300));

	if (loop >= 300) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s:: Wait until MAC 0x1200 bit0 and BBP 0x2130 become 0 > 300 times\n", __func__));
		return FALSE;
	}

	calFunction = os_str_tol(arg, 0, 10);
	ops->fITxBfDividerCalibration(pAd, calFunction, 0, NULL);
	/* enable TX/RX */
	RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x1004, restore_value);
	/* Restore RTS retry count */
	/* RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x1344, pAd->rts_tx_retry_num); */
	return TRUE;
}


#ifdef MT_MAC
/*
	Set_ETxBfEnCond_Proc - enable/disable ETxBF
	usage: iwpriv ra0 set ETxBfEnCond=dd
		0=>disable, 1=>enable
	Note: After use this command, need to re-run apStartup()/LinkUp() operations to sync all status.
		  If ETxBfIncapable!=0 then we don't need to reassociate.
*/
INT	Set_ETxBfEnCond_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i, ucETxBfEn;
	UCHAR ucStatus = FALSE;
	MAC_TABLE_ENTRY		*pEntry;
	TXBF_STATUS_INFO    TxBfInfo;
	struct wifi_dev *wdev = NULL;

	ucETxBfEn = os_str_tol(arg, 0, 10);

	if (ucETxBfEn > 1)
		return FALSE;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &pAd->MacTab.Content[i];

		if (!IS_ENTRY_NONE(pEntry)) {
			wdev = pEntry->wdev;
			TxBfInfo.ucTxPathNum = pAd->Antenna.field.TxPath;
			TxBfInfo.ucRxPathNum = pAd->Antenna.field.RxPath;
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				UINT8 band_idx = HcGetBandByWdev(wdev);

				if (band_idx == DBDC_BAND0) {
					TxBfInfo.ucTxPathNum = pAd->dbdc_band0_tx_path;
					TxBfInfo.ucRxPathNum = pAd->dbdc_band0_rx_path;
				} else {
					TxBfInfo.ucTxPathNum = pAd->dbdc_band1_tx_path;
					TxBfInfo.ucRxPathNum = pAd->dbdc_band1_rx_path;
				}
			}
#endif
			TxBfInfo.ucPhyMode   = wdev->PhyMode;
			TxBfInfo.u2Channel   = wdev->channel;
			TxBfInfo.pHtTxBFCap  = &pAd->CommonCfg.HtCapability.TxBFCap;
			TxBfInfo.cmmCfgETxBfIncapable = pAd->CommonCfg.ETxBfIncapable;
			TxBfInfo.cmmCfgETxBfNoncompress = pAd->CommonCfg.ETxBfNoncompress;
#ifdef VHT_TXBF_SUPPORT
			TxBfInfo.pVhtTxBFCap = &pAd->CommonCfg.vht_cap_ie.vht_cap;
#endif
			TxBfInfo.u4WTBL1 = pAd->mac_ctrl.wtbl_base_addr[0] +  i * pAd->mac_ctrl.wtbl_entry_size[0];
			TxBfInfo.u4WTBL2 = pAd->mac_ctrl.wtbl_base_addr[1] +  i * pAd->mac_ctrl.wtbl_entry_size[1];
			TxBfInfo.ucETxBfTxEn = ucETxBfEn;
			TxBfInfo.ucITxBfTxEn  = FALSE;
			TxBfInfo.ucWcid = i;
			TxBfInfo.ucBW = pEntry->HTPhyMode.field.BW;
			TxBfInfo.ucNDPARate = 2; /* MCS2 */
			ucStatus = AsicTxBfEnCondProc(pAd, &TxBfInfo);
		}
	}

	return ucStatus;
}

INT set_txbf_stop_report_poll_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	para[6];

	para[0] = os_str_tol(arg, 0, 10);

	if (para[0] != 0 && para[0] != 1) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Wrong format!\niwpirv ra0 set TxBfStopReportPoll=N\nN=1: Stop Rpt Poll\nN=0: Re-enable Rpt Poll\n"));
		return FALSE;
	}

	para[1] = '\0';

	return txbf_config(pAd, BF_CONFIG_TYPE_STOP_REPORT_POLL, &para[0]);
}

INT Set_TxBfTxApply(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR	 *value;
	UCHAR	Input[5];
	INT		i;
	CHAR    ucWlanIdx;
	BOOLEAN fgETxBf, fgITxBf, fgMuTxBf, fgPhaseCali;
	BOOLEAN fgStatus = TRUE;

	if (strlen(arg) != 14)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucWlanIdx   = Input[0];
	fgETxBf     = Input[1];
	fgITxBf     = Input[2];
	fgMuTxBf    = Input[3];
	fgPhaseCali = Input[4];
	CmdTxBfTxApplyCtrl(pAd,
					   ucWlanIdx,
					   fgETxBf,
					   fgITxBf,
					   fgMuTxBf,
					   fgPhaseCali);
	return fgStatus;
}

#ifdef TXBF_DYNAMIC_DISABLE
INT Set_TxBfDynamicDisable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN fgStatus = TRUE;
	BOOLEAN fgDisable;

	fgDisable = simple_strtol(arg, 0, 10);

	DynamicTxBfDisable(pAd, fgDisable);

	return fgStatus;
}
#endif /* TXBF_DYNAMIC_DISABLE */

INT set_txbf_dynamic_mechanism_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN fgStatus = TRUE;

	if (strlen(arg) == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("bfdm Usage:\niwpriv ra0 set bfdm=On_Off_Bit_Map\nBit0: Dynamic BFee Adaption\n"));
	} else {
		pAd->bfdm.bfdm_bitmap = simple_strtol(arg, 0, 10);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bfdm_bitmap=%d\n", pAd->bfdm.bfdm_bitmap));
	}

	return fgStatus;
}

INT	Set_Trigger_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR			Input[7];
	CHAR			*value;
	INT				i;
	BOOLEAN         fgStatus = FALSE;
	UCHAR           ucSu_Mu, ucMuNum, ucWlanId[4];
	UINT32          u4SndInterval;

	if (strlen(arg) != 20)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucSu_Mu       = Input[0];
	ucMuNum       = Input[1];
	u4SndInterval = (UINT32) Input[2];
	u4SndInterval = u4SndInterval << 2;
	ucWlanId[0]   = Input[3];
	ucWlanId[1]   = Input[4];
	ucWlanId[2]   = Input[5];
	ucWlanId[3]   = Input[6];

	if (pAd->Antenna.field.TxPath <= 1)
		return FALSE;

	if (mt_Trigger_Sounding_Packet(pAd,
								   TRUE,
								   u4SndInterval,
								   ucSu_Mu,
								   ucMuNum,
								   ucWlanId) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT	Set_Stop_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN         fgStatus = FALSE;

	if (mt_Trigger_Sounding_Packet(pAd,
								   FALSE,
								   0,
								   0,
								   0,
								   NULL) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT	Set_TxBfPfmuMemAlloc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR			Input[2];
	CHAR			*value;
	INT				i;
	BOOLEAN         fgStatus = FALSE;
	UCHAR           ucSu_Mu, ucWlanIdx;

	if (strlen(arg) != 5)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucSu_Mu   = Input[0];
	ucWlanIdx = Input[1];

	if (CmdPfmuMemAlloc(pAd,
						ucSu_Mu,
						ucWlanIdx) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT	Set_TxBfPfmuMemRelease(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN         fgStatus = FALSE;
	UCHAR           ucWlanIdx;

	ucWlanIdx  = os_str_tol(arg, 0, 10);

	if (CmdPfmuMemRelease(pAd, ucWlanIdx) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT	Set_TxBfPfmuMemAllocMapRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN         fgStatus = FALSE;

	if (CmdPfmuMemAllocMapRead(pAd) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT Set_StaRecBfUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR			 *value;
	UCHAR			 Input[23];
	INT				 i;
	CHAR             BssIdx, WlanIdx;
	PMAC_TABLE_ENTRY pEntry;
	BOOLEAN          fgStatus = FALSE;

	if (strlen(arg) != 68)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	WlanIdx                          = Input[0];
	BssIdx                           = Input[1];
	pEntry                           = &pAd->MacTab.Content[WlanIdx];

	if (pEntry == NULL)
		return FALSE;

	pEntry->rStaRecBf.u2PfmuId       = Input[2];
	pEntry->rStaRecBf.fgSU_MU        = Input[3];
	pEntry->rStaRecBf.u1TxBfCap      = Input[4];
	pEntry->rStaRecBf.ucNdpaRate     = Input[5];
	pEntry->rStaRecBf.ucNdpRate      = Input[6];
	pEntry->rStaRecBf.ucReptPollRate = Input[7];
	pEntry->rStaRecBf.ucTxMode       = Input[8];
	pEntry->rStaRecBf.ucNc           = Input[9];
	pEntry->rStaRecBf.ucNr           = Input[10];
	pEntry->rStaRecBf.ucCBW          = Input[11];
	pEntry->rStaRecBf.ucSEIdx        = Input[12];
	pEntry->rStaRecBf.ucTotMemRequire = Input[13];
	pEntry->rStaRecBf.ucMemRequire20M = Input[14];
	pEntry->rStaRecBf.ucMemRow0      = Input[15];
	pEntry->rStaRecBf.ucMemCol0      = Input[16];
	pEntry->rStaRecBf.ucMemRow1      = Input[17];
	pEntry->rStaRecBf.ucMemCol1      = Input[18];
	pEntry->rStaRecBf.ucMemRow2      = Input[19];
	pEntry->rStaRecBf.ucMemCol2      = Input[20];
	pEntry->rStaRecBf.ucMemRow3      = Input[21];
	pEntry->rStaRecBf.ucMemCol3      = Input[22];
	/* Default setting */
	pEntry->rStaRecBf.u2SmartAnt     = 0;
	pEntry->rStaRecBf.ucSoundingPhy  = 1;
	pEntry->rStaRecBf.uciBfTimeOut   = 0xFF;
	pEntry->rStaRecBf.uciBfDBW       = 0;
	pEntry->rStaRecBf.uciBfNcol      = 0;
	pEntry->rStaRecBf.uciBfNrow      = 0;
	{
		STA_REC_CFG_T StaCfg;

		os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));
		StaCfg.MuarIdx = 0;
		StaCfg.ConnectionState = TRUE;
		StaCfg.ConnectionType = 0;
		StaCfg.u4EnableFeature = (1 << STA_REC_BF);
		StaCfg.ucBssIndex = BssIdx;
		StaCfg.ucWlanIdx = WlanIdx;
		StaCfg.pEntry = pEntry;

		if (CmdExtStaRecUpdate(pAd, StaCfg) == STATUS_TRUE)
			fgStatus = TRUE;
	}
	return fgStatus;
}

INT Set_StaRecBfRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR             WlanIdx;
	BOOLEAN          fgStatus = FALSE;

	WlanIdx = os_str_tol(arg, 0, 10);

	if (CmdETxBfStaRecRead(pAd,
						   WlanIdx) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT Set_TxBfAwareCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN fgBfAware, fgStatus = FALSE;

	fgBfAware = os_str_tol(arg, 0, 10);

	if (CmdTxBfAwareCtrl(pAd, fgBfAware) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT set_dynsnd_en_intr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN intr_en;
	INT status = 0;

	intr_en = os_str_tol(arg, 0, 10);

	if (cmd_txbf_en_dynsnd_intr(pAd, intr_en) == STATUS_TRUE)
		status = 1;

	return status;
}

#ifdef CFG_SUPPORT_MU_MIMO
INT set_dynsnd_cfg_dmcs(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *tmp = NULL;
	INT status = 0;
	UINT8 mcs_index, mcs_th;

	tmp = strsep(&arg, ":");

	if (tmp != NULL)
		mcs_index = os_str_tol(tmp, 0, 10);
	else
		goto error;

	tmp = strsep(&arg, "");

	if (tmp != NULL)
		mcs_th = os_str_tol(tmp, 0, 10);
	else
		goto error;

	if (cmd_txbf_cfg_dynsnd_dmcsth(pAd, mcs_index, mcs_th) == STATUS_TRUE)
		status = 1;

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(status = %d\n", __func__, status));
	return status;
}

INT set_dynsnd_en_mu_intr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *tmp = NULL;
	INT status = 0;
	BOOLEAN mu_intr_en;
	UINT8 pfid;

	tmp = strsep(&arg, ":");

	if (tmp != NULL)
		mu_intr_en = os_str_tol(tmp, 0, 10);
	else
		goto error;

	tmp = strsep(&arg, "");

	if (tmp != NULL)
		pfid = os_str_tol(tmp, 0, 10);
	else
		goto error;

	if (cmd_txbf_en_dynsnd_pfid_intr(pAd, mu_intr_en, pfid) == STATUS_TRUE)
		status = 1;

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(status = %d\n", __func__, status));
	return status;
}
#endif /* CFG_SUPPORT_MU_MIMO */

INT Set_StaRecCmmUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PMAC_TABLE_ENTRY pEntry;
	CHAR			 *value;
	UCHAR			 Input[9];
	INT				 i;
	CHAR             BssIdx, WlanIdx;
	BOOLEAN          fgStatus = FALSE;

	if (strlen(arg) != 26)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	WlanIdx       = Input[0];
	BssIdx        = Input[1];
	pEntry = &pAd->MacTab.Content[WlanIdx];

	if (pEntry == NULL)
		return FALSE;

	pEntry->Aid     = Input[2];
	pEntry->Addr[0] = Input[3];
	pEntry->Addr[1] = Input[4];
	pEntry->Addr[2] = Input[5];
	pEntry->Addr[3] = Input[6];
	pEntry->Addr[4] = Input[7];
	pEntry->Addr[5] = Input[8];
	{
		STA_REC_CFG_T StaCfg;

		os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));
		StaCfg.MuarIdx = 0;
		StaCfg.ConnectionState = TRUE;
		StaCfg.ConnectionType = CONNECTION_INFRA_AP;
		StaCfg.u4EnableFeature = (1 << STA_REC_BASIC_STA_RECORD);
		StaCfg.ucBssIndex = BssIdx;
		StaCfg.ucWlanIdx = WlanIdx;
		StaCfg.pEntry = pEntry;
		StaCfg.IsNewSTARec = TRUE;

		if (CmdExtStaRecUpdate(pAd, StaCfg) == STATUS_TRUE)
			fgStatus = TRUE;
	}
	return fgStatus;
}

INT Set_BssInfoUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR			 *value;
	UCHAR			 Input[8];
	UCHAR            Bssid[MAC_ADDR_LEN];
	INT				 i;
	CHAR             OwnMacIdx, BssIdx;
	BOOLEAN          fgStatus = FALSE;
	BSS_INFO_ARGUMENT_T bss_info_argument;

	if (strlen(arg) != 23)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	OwnMacIdx = Input[0];
	BssIdx    = Input[1];
	Bssid[0]  = Input[2];
	Bssid[1]  = Input[3];
	Bssid[2]  = Input[4];
	Bssid[3]  = Input[5];
	Bssid[4]  = Input[6];
	Bssid[5]  = Input[7];
	NdisZeroMemory(&bss_info_argument, sizeof(BSS_INFO_ARGUMENT_T));
#if defined(MT7626)
	if (ATE_ON(pAd)) {
		struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
		UCHAR control_band_idx = ATECtrl->control_band_idx;
		struct wifi_dev *wdev;
		INT32 wdev_idx;

		/* omac binding is fixed, need to get from wdev_idx*/
		wdev_idx = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, wdev_idx);
		wdev = pAd->wdev_list[wdev_idx];
		OwnMacIdx = HcGetOmacIdx(pAd, wdev);
		bss_info_argument.WmmIdx = HcGetWmmIdx(pAd, wdev);
		bss_info_argument.u4BssInfoFeature |= BSS_INFO_OWN_MAC_FEATURE;
	}
#endif
	bss_info_argument.OwnMacIdx = OwnMacIdx;
	bss_info_argument.ucBssIndex = BssIdx;
	os_move_mem(bss_info_argument.Bssid, Bssid, MAC_ADDR_LEN);
	bss_info_argument.ucBcMcWlanIdx = 0; /* MCAST_WCID which needs to be modified by Patrick; */
	bss_info_argument.NetworkType = NETWORK_INFRA;
	bss_info_argument.u4ConnectionType = CONNECTION_INFRA_AP;
	bss_info_argument.CipherSuit = CIPHER_SUIT_NONE;
	bss_info_argument.bss_state = BSS_ACTIVE;
	bss_info_argument.u4BssInfoFeature |= BSS_INFO_BASIC_FEATURE;

	if (AsicBssInfoUpdate(pAd, &bss_info_argument) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT Set_DevInfoUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR			 *value;
	UCHAR			 Input[8];
	UCHAR            OwnMacAddr[MAC_ADDR_LEN];
	INT				 i;
	CHAR             OwnMacIdx;
	BOOLEAN          fgStatus = FALSE;
	UINT8		     BandIdx = 0;

	if (strlen(arg) != 23)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}
#if defined(MT7626)
	if (ATE_ON(pAd)) {
		struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
		UCHAR control_band_idx = ATECtrl->control_band_idx;
		struct wifi_dev *wdev;
		INT32 wdev_idx;
		/* omac is fixed, need to get from wdev_idx*/
		wdev_idx = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, wdev_idx);
		wdev = pAd->wdev_list[wdev_idx];
		OwnMacIdx = HcGetOmacIdx(pAd, wdev);
	} else {
		OwnMacIdx = Input[0];
	}
#else
	OwnMacIdx     = Input[0];
#endif
	OwnMacAddr[0] = Input[1];
	OwnMacAddr[1] = Input[2];
	OwnMacAddr[2] = Input[3];
	OwnMacAddr[3] = Input[4];
	OwnMacAddr[4] = Input[5];
	OwnMacAddr[5] = Input[6];
#if defined(MT7626)
	if (ATE_ON(pAd)) {
		struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

		BandIdx = ATECtrl->control_band_idx;
	} else {
		BandIdx = Input[7];
	}
#else
	BandIdx = Input[7];
#endif

	if (AsicDevInfoUpdate(
			pAd,
			OwnMacIdx,
			OwnMacAddr,
			BandIdx,
			TRUE,
			DEVINFO_ACTIVE_FEATURE) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}
#endif /* MT_MAC */

#if defined(MT_MAC)
INT	Set_ITxBfEn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i, ucITxBfEn;
	INT   u4Status = FALSE;
	MAC_TABLE_ENTRY		*pEntry;
	TXBF_STATUS_INFO    TxBfInfo;
	struct wifi_dev *wdev = NULL;

	ucITxBfEn = os_str_tol(arg, 0, 10);

	if (ucITxBfEn > 1)
		return FALSE;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &pAd->MacTab.Content[i];

		if (!IS_ENTRY_NONE(pEntry)) {
			wdev = pEntry->wdev;
			TxBfInfo.ucTxPathNum = pAd->Antenna.field.TxPath;
			TxBfInfo.ucRxPathNum = pAd->Antenna.field.RxPath;
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				UINT8 band_idx = HcGetBandByWdev(wdev);

				if (band_idx == DBDC_BAND0) {
					TxBfInfo.ucTxPathNum = pAd->dbdc_band0_tx_path;
					TxBfInfo.ucRxPathNum = pAd->dbdc_band0_rx_path;
				} else {
					TxBfInfo.ucTxPathNum = pAd->dbdc_band1_tx_path;
					TxBfInfo.ucRxPathNum = pAd->dbdc_band1_rx_path;
				}
			}
#endif
			TxBfInfo.ucPhyMode   = wdev->PhyMode;
			TxBfInfo.u2Channel   = wdev->channel;
			TxBfInfo.pHtTxBFCap  = &pAd->CommonCfg.HtCapability.TxBFCap;
			TxBfInfo.cmmCfgETxBfIncapable  = pAd->CommonCfg.ETxBfIncapable;
			TxBfInfo.cmmCfgETxBfNoncompress = pAd->CommonCfg.ETxBfNoncompress;
#ifdef VHT_TXBF_SUPPORT
			TxBfInfo.pVhtTxBFCap = &pAd->CommonCfg.vht_cap_ie.vht_cap;
#endif
			TxBfInfo.u4WTBL1 = pAd->mac_ctrl.wtbl_base_addr[0] + i * pAd->mac_ctrl.wtbl_entry_size[0];
			TxBfInfo.u4WTBL2 = pAd->mac_ctrl.wtbl_base_addr[1] + i * pAd->mac_ctrl.wtbl_entry_size[1];
			TxBfInfo.ucETxBfTxEn = FALSE;
			TxBfInfo.ucITxBfTxEn = ucITxBfEn;
			TxBfInfo.ucWcid      = i;
			TxBfInfo.ucBW        = pEntry->HTPhyMode.field.BW;
			TxBfInfo.ucNDPARate  = 2; /* MCS2 */
			u4Status = AsicTxBfEnCondProc(pAd, &TxBfInfo);
		}
	}

	return u4Status;
}

#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */

#ifdef VHT_TXBF_SUPPORT
/*
	The VhtNDPA sounding inupt string format should be xx:xx:xx:xx:xx:xx-d,
		=>The six 2 digit hex-decimal number previous are the Mac address,
		=>The seventh decimal number is the MCS value.
*/
INT Set_VhtNDPA_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR mac[6];
	UINT mcs;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n%s\n", arg));

	/*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and MCS value in decimal format.*/
	if (strlen(arg) < 19)
		return FALSE;

	token = strchr(arg, DASH);

	if ((token != NULL) && (strlen(token) > 1)) {
		mcs = (UINT)os_str_tol((token + 1), 0, 10);
		*token = '\0';

		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++) {
			if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
				return FALSE;

			AtoH(token, (&mac[i]), 1);
		}

		if (i != 6)
			return FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
				 PRINT_MAC(mac), mcs));
		pEntry = MacTableLookup(pAd, (PUCHAR) mac);

		if (pEntry) {
#ifdef SOFT_SOUNDING
			pEntry->snd_rate.field.MODE = MODE_VHT;
			pEntry->snd_rate.field.BW = (mcs / 100) > BW_80 ? BW_80 : (mcs / 100);
			mcs %= 100;
			pEntry->snd_rate.field.MCS = ((mcs / 10) << 4 | (mcs % 10));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s():Trigger VHT NDPA Sounding=%02x:%02x:%02x:%02x:%02x:%02x, snding rate=VHT-%sHz, %dSS-MCS%d\n",
					  __func__, PRINT_MAC(mac),
					  get_bw_str(pEntry->snd_rate.field.BW),
					  (pEntry->snd_rate.field.MCS >> 4) + 1,
					  pEntry->snd_rate.field.MCS & 0xf));
#endif
			trigger_vht_ndpa(pAd, pEntry);
		}

		return TRUE;
	}

	return FALSE;
}
#endif /* VHT_TXBF_SUPPORT */

#if defined(MT_MAC)
#ifdef TXBF_SUPPORT

INT Set_TxBfProfileTag_Help(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
				 "========================================================================================================================\n"
				 "TxBfProfile Tag1 setting example :\n"
				 "iwpriv ra0 set TxBfProfileTagPfmuIdx  =xx\n"
				 "iwpriv ra0 set TxBfProfileTagBfType   =xx (0: iBF; 1: eBF)\n"
				 "iwpriv ra0 set TxBfProfileTagBw       =xx (0/1/2/3 : BW20/40/80/160NC)\n"
				 "iwpriv ra0 set TxBfProfileTagSuMu     =xx (0:SU, 1:MU)\n"
				 "iwpriv ra0 set TxBfProfileTagInvalid  =xx (0: valid, 1: invalid)\n"
				 "iwpriv ra0 set TxBfProfileTagMemAlloc =xx:xx:xx:xx:xx:xx:xx:xx (mem_row, mem_col), ..\n"
				 "iwpriv ra0 set TxBfProfileTagMatrix   =nrow:nol:ng:LM\n"
				 "iwpriv ra0 set TxBfProfileTagSnr      =SNR_STS0:SNR_STS1:SNR_STS2:SNR_STS3\n"
				 "\n\n"
				 "TxBfProfile Tag2 setting example :\n"
				 "iwpriv ra0 set TxBfProfileTagSmtAnt   =xx (11:0)\n"
				 "iwpriv ra0 set TxBfProfileTagSeIdx    =xx\n"
				 "iwpriv ra0 set TxBfProfileTagRmsdThrd =xx\n"
				 "iwpriv ra0 set TxBfProfileTagMcsThrd  =xx:xx:xx:xx:xx:xx (MCS TH L1SS:S1SS:L2SS:....)\n"
				 "iwpriv ra0 set TxBfProfileTagTimeOut  =xx\n"
				 "iwpriv ra0 set TxBfProfileTagDesiredBw=xx (0/1/2/3 : BW20/40/80/160NC)\n"
				 "iwpriv ra0 set TxBfProfileTagDesiredNc=xx\n"
				 "iwpriv ra0 set TxBfProfileTagDesiredNr=xx\n"
				 "\n\n"
				 "Read TxBf profile Tag :\n"
				 "iwpriv ra0 set TxBfProfileTagRead     =xx (PFMU ID)\n"
				 "\n"
				 "Write TxBf profile Tag :\n"
				 "iwpriv ra0 set TxBfProfileTagWrite    =xx (PFMU ID)\n"
				 "When you use one of relative CMD to update one of tag parameters, you should call TxBfProfileTagWrite to update Tag\n"
				 "\n\n"
				 "Read TxBf profile Data	:\n"
				 "iwpriv ra0 set TxBfProfileDataRead    =xx (PFMU ID)\n"
				 "\n"
				 "Write TxBf profile Data :\n"
				 "iwpriv ra0 set TxBfProfileDataWrite   =BW :subcarrier:phi11:psi2l:Phi21:Psi31:Phi31:Psi41:Phi22:Psi32:Phi32:Psi42:Phi33:Psi43\n"
				 "iwpriv ra0 set TxBfProfileDataWriteAll=Profile ID : BW (BW       : 0x00 (20M) , 0x01 (40M), 0x02 (80M), 0x3 (160M)\n"
				 "When you use CMD TxBfProfileDataWrite to update profile data per subcarrier, you should call TxBfProfileDataWriteAll to update all of\n"
				 "subcarrier's profile data.\n\n"
				 "Read TxBf profile PN	:\n"
				 "iwpriv ra0 set TxBfProfilePnRead      =xx (PFMU ID)\n"
				 "\n"
				 "Write TxBf profile PN :\n"
				 "iwpriv ra0 set TxBfProfilePnWrite     =Profile ID:BW:1STS_Tx0:1STS_Tx1:1STS_Tx2:1STS_Tx3:2STS_Tx0:2STS_Tx1:2STS_Tx2:2STS_Tx3:3STS_Tx1:3STS_Tx2:3STS_Tx3\n"
				 "========================================================================================================================\n"));
	return TRUE;
}

INT Set_TxBfProfileTag_PfmuIdx(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;

	profileIdx    = os_str_tol(arg, 0, 10);
	TxBfProfileTag_PfmuIdx(&pAd->rPfmuTag1, profileIdx);
	return TRUE;
}

INT Set_TxBfProfileTag_BfType(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   ucBfType;

	ucBfType    = os_str_tol(arg, 0, 10);
	TxBfProfileTag_TxBfType(&pAd->rPfmuTag1, ucBfType);
	return TRUE;
}

INT Set_TxBfProfileTag_DBW(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   ucBw;

	ucBw    = os_str_tol(arg, 0, 10);
	TxBfProfileTag_DBW(&pAd->rPfmuTag1, ucBw);
	return TRUE;
}

INT Set_TxBfProfileTag_SuMu(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   ucSuMu;

	ucSuMu    = os_str_tol(arg, 0, 10);
	TxBfProfileTag_SuMu(&pAd->rPfmuTag1, ucSuMu);
	return TRUE;
}

INT Set_TxBfProfileTag_InValid(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   InValid;

	InValid    = os_str_tol(arg, 0, 10);
	TxBfProfileTag_InValid(&pAd->rPfmuTag1, InValid);
	return TRUE;
}

INT Set_TxBfProfileTag_Mem(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   Input[8];
	CHAR	 *value;
	INT	i;
	UCHAR   aMemAddrColIdx[4], aMemAddrRowIdx[4];

	/* mem col0:row0:col1:row1:col2:row2:col3:row3 */
	if (strlen(arg) != 23)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	aMemAddrColIdx[0] = Input[0];
	aMemAddrRowIdx[0] = Input[1];
	aMemAddrColIdx[1] = Input[2];
	aMemAddrRowIdx[1] = Input[3];
	aMemAddrColIdx[2] = Input[4];
	aMemAddrRowIdx[2] = Input[5];
	aMemAddrColIdx[3] = Input[6];
	aMemAddrRowIdx[3] = Input[7];
	TxBfProfileTag_Mem(&pAd->rPfmuTag1, aMemAddrColIdx, aMemAddrRowIdx);
	return TRUE;
}

INT Set_TxBfProfileTag_Matrix(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   Input[8];
	CHAR	 *value;
	INT	i;
	UCHAR   ucNrow, ucNcol, ucNgroup, ucLM, ucCodeBook, ucHtcExist;

	/* nrow:nol:ng:LM:CodeBook:HtcExist */
	if (strlen(arg) != 17)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucNrow     = Input[0];
	ucNcol     = Input[1];
	ucNgroup   = Input[2];
	ucLM       = Input[3];
	ucCodeBook = Input[4];
	ucHtcExist = Input[5];
	TxBfProfileTag_Matrix(&pAd->rPfmuTag1,
						  ucNrow,
						  ucNcol,
						  ucNgroup,
						  ucLM,
						  ucCodeBook,
						  ucHtcExist);
	return TRUE;
}

INT Set_TxBfProfileTag_SNR(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   Input[4];
	CHAR	 *value;
	INT	i;
	UCHAR   ucSNR_STS0, ucSNR_STS1, ucSNR_STS2, ucSNR_STS3;

	if (strlen(arg) != 11)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucSNR_STS0 = Input[0];
	ucSNR_STS1 = Input[1];
	ucSNR_STS2 = Input[2];
	ucSNR_STS3 = Input[3];
	TxBfProfileTag_SNR(&pAd->rPfmuTag1,
					   ucSNR_STS0,
					   ucSNR_STS1,
					   ucSNR_STS2,
					   ucSNR_STS3);
	return TRUE;
}

INT Set_TxBfProfileTag_SmartAnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT   u2SmartAnt;

	u2SmartAnt    = os_str_tol(arg, 0, 10);
	TxBfProfileTag_SmtAnt(&pAd->rPfmuTag2, u2SmartAnt);
	return TRUE;
}

INT Set_TxBfProfileTag_SeIdx(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT   ucSeIdx;

	ucSeIdx    = os_str_tol(arg, 0, 10);
	TxBfProfileTag_SeIdx(&pAd->rPfmuTag2, ucSeIdx);
	return TRUE;
}

INT Set_TxBfProfileTag_RmsdThrd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT   ucRmsdThrd;

	ucRmsdThrd    = os_str_tol(arg, 0, 10);
	TxBfProfileTag_RmsdThd(&pAd->rPfmuTag2, ucRmsdThrd);
	return TRUE;
}

INT Set_TxBfProfileTag_McsThrd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   Input[6];
	CHAR	 *value;
	INT	i;
	UCHAR   ucMcsLss[3], ucMcsSss[3];

	if (strlen(arg) != 17)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucMcsLss[0] = Input[0];
	ucMcsSss[0] = Input[1];
	ucMcsLss[1] = Input[2];
	ucMcsSss[1] = Input[3];
	ucMcsLss[2] = Input[4];
	ucMcsSss[2] = Input[5];
	TxBfProfileTag_McsThd(&pAd->rPfmuTag2,
						  ucMcsLss,
						  ucMcsSss);
	return TRUE;
}

INT Set_TxBfProfileTag_TimeOut(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT   ucTimeOut;

	ucTimeOut    = os_str_tol(arg, 0, 10);
	TxBfProfileTag_TimeOut(&pAd->rPfmuTag2, ucTimeOut);
	return TRUE;
}

INT Set_TxBfProfileTag_DesiredBW(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT   ucDesiredBW;

	ucDesiredBW    = os_str_tol(arg, 0, 10);
	TxBfProfileTag_DesiredBW(&pAd->rPfmuTag2, ucDesiredBW);
	return TRUE;
}

INT Set_TxBfProfileTag_DesiredNc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT   ucDesiredNc;

	ucDesiredNc    = os_str_tol(arg, 0, 10);
	TxBfProfileTag_DesiredNc(&pAd->rPfmuTag2, ucDesiredNc);
	return TRUE;
}

INT Set_TxBfProfileTag_DesiredNr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT   ucDesiredNr;

	ucDesiredNr    = os_str_tol(arg, 0, 10);
	TxBfProfileTag_DesiredNr(&pAd->rPfmuTag2, ucDesiredNr);
	return TRUE;
}

INT Set_TxBfProfileTagRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;
	BOOLEAN fgBFer;
	UCHAR   Input[2];
	CHAR	 *value;
	INT	i;

	if (strlen(arg) != 5)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	profileIdx = Input[0];
	fgBFer     = Input[1];
	return TxBfProfileTagRead(pAd, profileIdx, fgBFer);
}

INT Set_TxBfProfileTagWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;

	profileIdx = os_str_tol(arg, 0, 10);
	return TxBfProfileTagWrite(pAd,
							   &pAd->rPfmuTag1,
							   &pAd->rPfmuTag2,
							   profileIdx);
}

INT Set_TxBfProfileDataRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR	 *value;
	UCHAR   Input[4];
	INT	i;
	UCHAR   profileIdx, subcarrIdx_H, subcarrIdx_L;
	BOOLEAN fgBFer;
	USHORT  subcarrIdx;

	/* Profile Select : Subcarrier Select */
	if (strlen(arg) != 11)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	profileIdx   = Input[0];
	fgBFer       = Input[1];
	subcarrIdx_H = Input[2];
	subcarrIdx_L = Input[3];
	subcarrIdx = ((USHORT)(subcarrIdx_H << 8) | (USHORT)subcarrIdx_L);
	return TxBfProfileDataRead(pAd, profileIdx, fgBFer, subcarrIdx);
}

INT Set_TxBfProfileDataWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;
	USHORT  subcarrierIdx;
	USHORT  Input[18];
	CHAR	*value, value_T[12], onebyte;
	UCHAR   strLen;
	INT	i;
	PFMU_DATA rPfmuProfileData;
	PUCHAR  pProfile;

	os_zero_mem(Input, 36);

	/* Profile Select : Subcarrier Select */
	if (strlen(arg) != 60)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		strLen = strlen(value);

		if (strLen & 1) {
			strcpy(value_T, "0");
			strncat(value_T, value, strLen);
			AtoH(value_T, (PCHAR)(&Input[i]), 2);
			Input[i] = be2cpu16(Input[i]);
		} else if (strLen == 2) {
			AtoH(value, (PCHAR)(&onebyte), 1);
			Input[i] = ((USHORT)onebyte) & ((USHORT)0x00FF);
		} else
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s:Error: Un-expected string len!!!!!\n", __func__));
	}

	profileIdx    = Input[0];
	subcarrierIdx = Input[1];
	rPfmuProfileData.rField.u2Phi11  = Input[2];
	rPfmuProfileData.rField.ucPsi21  = Input[3];
	rPfmuProfileData.rField.u2Phi21  = Input[4];
	rPfmuProfileData.rField.ucPsi31  = Input[5];
	rPfmuProfileData.rField.u2Phi31  = Input[6];
	rPfmuProfileData.rField.ucPsi41  = Input[7];
	rPfmuProfileData.rField.u2Phi22  = Input[8];
	rPfmuProfileData.rField.ucPsi32  = Input[9];
	rPfmuProfileData.rField.u2Phi32  = Input[10];
	rPfmuProfileData.rField.ucPsi42  = Input[11];
	rPfmuProfileData.rField.u2Phi33  = Input[12];
	rPfmuProfileData.rField.ucPsi43  = Input[13];
	rPfmuProfileData.rField.u2dSNR00 = Input[14];
	rPfmuProfileData.rField.u2dSNR01 = Input[15];
	rPfmuProfileData.rField.u2dSNR02 = Input[16];
	rPfmuProfileData.rField.u2dSNR03 = Input[17];
	pProfile = (PUCHAR)&rPfmuProfileData;
#ifdef RT_BIG_ENDIAN
	RTMPEndianChange(pProfile, sizeof(PFMU_DATA));
#endif
	return TxBfProfileDataWrite(pAd, profileIdx, subcarrierIdx, pProfile);
}

INT Set_TxBfProfileData20MAllWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* Input Cmd Argument Parsing */
	USHORT	input[360];     /* Input[] array stores absolute data */
	CHAR	*value, value_t[12], one_byte; /* *value stores every input argument
between : */
	UCHAR	str_len;
	INT i;
	struct  _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UCHAR	control_band_idx;
	UINT8   tx_path = pAd->Antenna.field.TxPath;
	CHAR    sub_num;
	INT     sub_num_idx, arg_len;
	UCHAR	profile_idx;
	USHORT	sub_carr_id;
	UINT16  angle_ph11, angle_ph21, angle_ph31, angle_ph41;
	INT16   phi11,     phi21,     phi31;
	BOOLEAN fg_status = FALSE, fg_final_raw_data = FALSE;

	/* Init Array 720 bytes */
	os_zero_mem(input, 720);

	arg_len = strlen(arg);

	/* Absolute Phi Value Processing */
	if ((strlen(arg) != 183) && (strlen(arg) != 5)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: False: Command inputs not meet the Command format Length!\n",
__func__));

		return FALSE;
	}

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++
) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: False: Command input arguments aren't Hex format!\n", __func__));
			return FALSE;
		}

		str_len = strlen(value);

		if (str_len & 1) {
			strcpy(value_t, "0");
			strncat(value_t, value, str_len);
			AtoH(value_t, (PCHAR)(&input[i]), 2);
			input[i] = be2cpu16(input[i]);
		} else if (str_len == 2) {
			AtoH(value, (PCHAR)(&one_byte), 1);
			input[i] = ((USHORT)one_byte) & ((USHORT)0x00FF);
		} else if (str_len == 4) {
			AtoH(value, (PCHAR)(&input[i]), 2);
			input[i] = be2cpu16(input[i]);
		} else
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL,  DBG_LVL_ERROR,
					("%s: Error: Un-expected Argument Length!\n", __func__));
	}

	/* Check if the input is the last raw data or not */
	fg_final_raw_data = FALSE;
	if (arg_len == 5) {
		if (input[1] == 0x00F0) {
			pAd->profile_data_cnt = 0;
			fg_status = TRUE;
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL,  DBG_LVL_TRACE,
					("%s: Status: Start to Input Profile Data!\n", __func__));

			return fg_status;
		}

		if (input[1] == 0x00FF) {
			fg_final_raw_data = TRUE;
			profile_idx = input[0];
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL,  DBG_LVL_TRACE,
					("%s: Status: End to Input Profile Data!\n", __func__));
		}
	}

	/* Relative Phi Value Processing */
	control_band_idx = ATECtrl->control_band_idx;
#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		if (control_band_idx == DBDC_BAND0)
			tx_path = pAd->dbdc_band0_tx_path;
		else
			tx_path = pAd->dbdc_band1_tx_path;
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL,  DBG_LVL_TRACE,
			("%s: Status: Config_DBDC_MODE=%d, control_band_idx=%d, \t"
			"dbdc_band0_tx_path=%d, dbdc_band1_tx_path=%d,  \t"
			"Tx Path = %d!\n", __func__, pAd->CommonCfg.dbdc_mode, control_band_idx,
pAd->dbdc_band0_tx_path, pAd->dbdc_band1_tx_path, tx_path));
#endif

	if (fg_final_raw_data == FALSE) {
		/* Input Array Structure Assignment */
		for (sub_num = 0 ; sub_num < 8 ; sub_num++) {
			sub_num_idx = sub_num * 5;
			sub_carr_id = input[sub_num_idx];

			if (sub_carr_id < 32)
				sub_carr_id += 224;
			else
				sub_carr_id -= 32;

			angle_ph11  = input[sub_num_idx+1];
			angle_ph21  = input[sub_num_idx+2];
			angle_ph31  = input[sub_num_idx+3];
			angle_ph41  = input[sub_num_idx+4];

			switch (tx_path) {
			case NSTS_2:
				phi11    = (INT16)(angle_ph21 - angle_ph11);
				phi21    = 0;
				phi31    = 0;

				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s:: SubCarrier ID=%d, angle_ph21 = %x, angle_ph11 = %x, phi11 = %x\n",
						__func__, sub_carr_id, angle_ph21, angle_ph11, phi11));
				break;

			case NSTS_3:
				phi11    = (INT16)(angle_ph31 - angle_ph11);
				phi21    = (INT16)(angle_ph31 - angle_ph21);
				phi31    = 0;

				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s:: SubCarrier ID=%d, angle_ph31 = %x, angle_ph21 = %x, angle_ph11 = %x, phi11 = %x, phi21 = %x\n",
						__func__, sub_carr_id, angle_ph31, angle_ph21, angle_ph11, phi11, phi21));
				break;

			case NSTS_4:
			default:
#ifdef DBDC_MODE
				if (pAd->CommonCfg.dbdc_mode) {
					phi11    = (INT16)(angle_ph21 - angle_ph11);
					phi21    = 0;
					phi31    = 0;
				} else
#endif
				{
					phi11    = (INT16)(angle_ph41 - angle_ph11);
					phi21    = (INT16)(angle_ph41 - angle_ph21);
					phi31    = (INT16)(angle_ph41 - angle_ph31);
				}

				break;
			}
			pAd->profile_data[pAd->profile_data_cnt + sub_num].u2SubCarrIdx =
sub_carr_id;
			pAd->profile_data[pAd->profile_data_cnt + sub_num].i2Phi11 = phi11;
			pAd->profile_data[pAd->profile_data_cnt + sub_num].i2Phi21 = phi21;
			pAd->profile_data[pAd->profile_data_cnt + sub_num].i2Phi31 = phi31;
		}

		pAd->profile_data_cnt += sub_num;
		fg_status = TRUE;

	} else {
		if (CmdETxBfPfmuProfileDataWrite20MAll(pAd,
												profile_idx,
												(PUCHAR)&pAd->profile_data[0]) == STATUS_TRUE)
		fg_status = TRUE;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL,  DBG_LVL_ERROR,
				("%s: Status: Cmd Send to FW!\n", __func__));
	}

	return fg_status;

}

INT Set_TxBfProfilePnRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;

	profileIdx = os_str_tol(arg, 0, 10);
	return TxBfProfilePnRead(pAd, profileIdx);
}

INT Set_TxBfProfilePnWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;
	UCHAR   ucBw;
	CHAR    *value, value_T[12], onebyte;
	UCHAR   strLen;
	SHORT   Input[14] = {0};
	INT	status, i;
	PFMU_PN rPfmuPn;
	PFMU_PN_DBW80_80M rPfmuPn160M;
	PUCHAR  pPfmuPn = NULL;

	/* Profile Select : Subcarrier Select */
	if (strlen(arg) != 55)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		strLen = strlen(value);

		if (strLen & 1) {
			strcpy(value_T, "0");
			strncat(value_T, value, strLen);
			AtoH(value_T, (PCHAR)(&Input[i]), 2);
			Input[i] = be2cpu16(Input[i]);
		} else if (strLen == 2) {
			AtoH(value, (PCHAR)(&onebyte), 1);
			Input[i] = ((USHORT)onebyte) & ((USHORT)0x00FF);
		} else
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s:Error: Un-expected string len!!!!!\n", __func__));
	}

	profileIdx    = Input[0];
	ucBw          = Input[1];

	if (ucBw != P_DBW160M) {
		os_zero_mem(&rPfmuPn, sizeof(rPfmuPn));
		rPfmuPn.rField.u2CMM_1STS_Tx0    = Input[2];
		rPfmuPn.rField.u2CMM_1STS_Tx1    = Input[3];
		rPfmuPn.rField.u2CMM_1STS_Tx2    = Input[4] & 0x3FF;
		rPfmuPn.rField.u2CMM_1STS_Tx2Msb = Input[4] >> 11;
		rPfmuPn.rField.u2CMM_1STS_Tx3    = Input[5];
		rPfmuPn.rField.u2CMM_2STS_Tx0    = Input[6];
		rPfmuPn.rField.u2CMM_2STS_Tx1    = Input[7] & 0x1FF;
		rPfmuPn.rField.u2CMM_2STS_Tx1Msb = Input[7] >> 10;
		rPfmuPn.rField.u2CMM_2STS_Tx2    = Input[8];
		rPfmuPn.rField.u2CMM_2STS_Tx3    = Input[9];
		rPfmuPn.rField.u2CMM_3STS_Tx0    = Input[10] & 0x0FF;
		rPfmuPn.rField.u2CMM_3STS_Tx0Msb = Input[10] >> 9;
		rPfmuPn.rField.u2CMM_3STS_Tx1    = Input[11];
		rPfmuPn.rField.u2CMM_3STS_Tx2    = Input[12];
		rPfmuPn.rField.u2CMM_3STS_Tx3    = Input[13] & 0x07F;
		rPfmuPn.rField.u2CMM_3STS_Tx3Msb = Input[13] >> 8;
		pPfmuPn = (PUCHAR) (&rPfmuPn);
		status = TxBfProfilePnWrite(pAd, profileIdx, ucBw, pPfmuPn);
	} else {
		os_zero_mem(&rPfmuPn160M, sizeof(rPfmuPn160M));
		rPfmuPn160M.rField.u2DBW160_1STS_Tx0    = Input[2];
		rPfmuPn160M.rField.u2DBW160_1STS_Tx1    = Input[3];
		rPfmuPn160M.rField.u2DBW160_2STS_Tx0    = Input[4] & 0x3FF;
		rPfmuPn160M.rField.u2DBW160_2STS_Tx0Msb = Input[4] >> 11;
		rPfmuPn160M.rField.u2DBW160_2STS_Tx1    = Input[5];
		pPfmuPn = (PUCHAR) (&rPfmuPn160M);
		status = TxBfProfilePnWrite(pAd, profileIdx, ucBw, pPfmuPn);
	}

	return status;
}

INT Set_TxBfQdRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8	subcarrIdx;

	subcarrIdx = (INT8)simple_strtol(arg, 0, 10);
	TxBfQdRead(pAd, subcarrIdx);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	RF test switch mode.

	iwpriv ra0 set TxBfFbRptDbgInfo = ucAction : Input[1]

	ucAction
	0: BF_READ_AND_CLEAR_FBK_STAT_INFO
	1: BF_READ_FBK_STAT_INFO
	2: BF_SET_POLL_PFMU_INTR_STAT_TIMEOUT
	   Also set Input[1] as PollPFMUIntrStatTimeOut
	3: BF_SET_PFMU_DEQ_INTERVAL
	   Also set Input[1] as FbRptDeQInterval

	Return:
    ==========================================================================
*/
INT Set_TxBfFbRptDbgInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8 *value = NULL;
	UINT8 i, Input[3] = {0};
	EXT_CMD_TXBF_FBRPT_DBG_INFO_T ETxBfFbRptData;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		Input[i] = (UINT8)simple_strtol(value, 0, 10);
	}

	os_zero_mem(&ETxBfFbRptData, sizeof(EXT_CMD_TXBF_FBRPT_DBG_INFO_T));

	ETxBfFbRptData.ucAction = Input[0];

	switch (ETxBfFbRptData.ucAction) {
	case BF_SET_POLL_PFMU_INTR_STAT_TIMEOUT:
		ETxBfFbRptData.ucPollPFMUIntrStatTimeOut = Input[1];
		break;
	case BF_SET_PFMU_DEQ_INTERVAL:
		ETxBfFbRptData.ucFbRptDeQInterval = Input[1];
		break;
	case BF_DYNAMIC_PFMU_UPDATE:
		ETxBfFbRptData.ucWlanIdx = Input[1];
		ETxBfFbRptData.ucPFMUUpdateEn = Input[2];
		break;
	default:
		break;
	}

	TxBfFbRptDbgInfo(pAd, (P_EXT_CMD_TXBF_FBRPT_DBG_INFO_T)&ETxBfFbRptData);

	return TRUE;
}



INT Set_TxBfAidUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	UINT16 u2Aid = 0;

	if (arg != NULL)
		u2Aid = (UINT16)os_str_toul(arg, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Argument is NULL\n", __func__));
		Ret = FALSE;
		goto error;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: Aid:%u\n", __func__, u2Aid));
	if (CmdETxBfAidSetting(pAd, u2Aid, 0))
		Ret = FALSE;
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: CMD %s\n", __func__, Ret ? "Success":"Fail"));
	return Ret;
}

#endif  /* TXBF_SUPPORT */
#endif  /* MT_MAC */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
INT Set_WifiFwd_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int active = os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::active=%d\n", __func__, active));

	if (active == 0) {
		if (wf_drv_tbl.wf_fwd_pro_halt_hook)
			wf_drv_tbl.wf_fwd_pro_halt_hook();
	} else  {
		if (wf_drv_tbl.wf_fwd_pro_active_hook)
			wf_drv_tbl.wf_fwd_pro_active_hook();
	}

	return TRUE;
}

INT WifiFwdSet(
	IN int disabled)
{
	if (disabled != 0) {
		if (wf_drv_tbl.wf_fwd_pro_disabled_hook)
			wf_drv_tbl.wf_fwd_pro_disabled_hook();
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::disabled=%d\n", __func__, disabled));
	return TRUE;
}

INT Set_WifiFwd_Down(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int disable = os_str_tol(arg, 0, 10);

	WifiFwdSet(disable);
	return TRUE;
}

INT Set_WifiFwdAccessSchedule_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int active = os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::active=%d\n", __func__, active));

	if (active == 0) {
		if (wf_drv_tbl.wf_fwd_access_schedule_halt_hook)
			wf_drv_tbl.wf_fwd_access_schedule_halt_hook();
	} else  {
		if (wf_drv_tbl.wf_fwd_access_schedule_active_hook)
			wf_drv_tbl.wf_fwd_access_schedule_active_hook();
	}

	return TRUE;
}

INT Set_WifiFwdHijack_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int active = os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::active=%d\n", __func__, active));

	if (active == 0) {
		if (wf_drv_tbl.wf_fwd_hijack_halt_hook)
			wf_drv_tbl.wf_fwd_hijack_halt_hook();
	} else  {
		if (wf_drv_tbl.wf_fwd_hijack_active_hook)
			wf_drv_tbl.wf_fwd_hijack_active_hook();
	}

	return TRUE;
}

INT Set_WifiFwdBpdu_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int active = os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::active=%d\n", __func__, active));

	if (active == 0) {
		if (wf_drv_tbl.wf_fwd_bpdu_halt_hook)
			wf_drv_tbl.wf_fwd_bpdu_halt_hook();
	} else {
		if (wf_drv_tbl.wf_fwd_bpdu_active_hook)
			wf_drv_tbl.wf_fwd_bpdu_active_hook();
	}

	return TRUE;
}

INT Set_WifiFwdRepDevice(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int rep = os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::rep=%d\n", __func__, rep));

	if (wf_drv_tbl.wf_fwd_get_rep_hook)
		wf_drv_tbl.wf_fwd_get_rep_hook(rep);

	return TRUE;
}

INT Set_WifiFwdShowEntry(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	if (wf_drv_tbl.wf_fwd_show_entry_hook)
		wf_drv_tbl.wf_fwd_show_entry_hook();

	return TRUE;
}

INT Set_WifiFwdDeleteEntry(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int idx = os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::idx=%d\n", __func__, idx));

	if (wf_drv_tbl.wf_fwd_delete_entry_hook)
		wf_drv_tbl.wf_fwd_delete_entry_hook(idx);

	return TRUE;
}

INT Set_PacketSourceShowEntry(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	if (wf_drv_tbl.packet_source_show_entry_hook)
		wf_drv_tbl.packet_source_show_entry_hook();

	return TRUE;
}

INT Set_PacketSourceDeleteEntry(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int idx = os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::idx=%d\n", __func__, idx));

	if (wf_drv_tbl.packet_source_delete_entry_hook)
		wf_drv_tbl.packet_source_delete_entry_hook(idx);

	return TRUE;
}

#define BRIDGE_INTF_NAME_MAX_SIZE 10

INT Set_WifiFwdBridge_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{

	UINT8 Length = 0;

	Length = strlen(arg);

	if (Length > BRIDGE_INTF_NAME_MAX_SIZE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s:Bridge Intf Name too large =%s,size:%d\n", __func__, arg, Length));
		return FALSE;

	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s:Bridge Intf Name =%s,size:%d\n", __func__, arg, Length));

	if (wf_drv_tbl.wf_fwd_set_bridge_hook)
		wf_drv_tbl.wf_fwd_set_bridge_hook(arg, Length);

	return TRUE;
}

#endif /* CONFIG_WIFI_PKT_FWD */

#ifdef DOT11_N_SUPPORT
void assoc_ht_info_debugshow(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN UCHAR ht_cap_len,
	IN HT_CAPABILITY_IE * pHTCapability)
{
	HT_CAP_INFO			*pHTCap;
	HT_CAP_PARM		*pHTCapParm;
	EXT_HT_CAP_INFO		*pExtHT;
#ifdef TXBF_SUPPORT
	HT_BF_CAP			*pBFCap;
#endif /* TXBF_SUPPORT */

	if (pHTCapability && (ht_cap_len > 0)) {
		pHTCap = &pHTCapability->HtCapInfo;
		pHTCapParm = &pHTCapability->HtCapParm;
		pExtHT = &pHTCapability->ExtHtCapInfo;
#ifdef TXBF_SUPPORT
		pBFCap = &pHTCapability->TxBFCap;
#endif /* TXBF_SUPPORT */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Peer - 11n HT Info\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tHT Cap Info:\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\t\t HT_RX_LDPC(%d), BW(%d), MIMOPS(%d), GF(%d), ShortGI_20(%d), ShortGI_40(%d)\n",
				  pHTCap->ht_rx_ldpc, pHTCap->ChannelWidth, pHTCap->MimoPs, pHTCap->GF,
				  pHTCap->ShortGIfor20, pHTCap->ShortGIfor40));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\t\t TxSTBC(%d), RxSTBC(%d), DelayedBA(%d), A-MSDU(%d), CCK_40(%d)\n",
				  pHTCap->TxSTBC, pHTCap->RxSTBC, pHTCap->DelayedBA, pHTCap->AMsduSize, pHTCap->CCKmodein40));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\t PSMP(%d), Forty_Mhz_Intolerant(%d), L-SIG(%d)\n",
				 pHTCap->PSMP, pHTCap->Forty_Mhz_Intolerant, pHTCap->LSIGTxopProSup));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tHT Parm Info:\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\t MaxRx A-MPDU Factor(%d), MPDU Density(%d)\n",
				 pHTCapParm->MaxRAmpduFactor, pHTCapParm->MpduDensity));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tHT MCS set:\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\t\t RxMCS(%02x %02x %02x %02x %02x) MaxRxMbps(%d) TxMCSSetDef(%02x)\n",
				  pHTCapability->MCSSet[0], pHTCapability->MCSSet[1], pHTCapability->MCSSet[2],
				  pHTCapability->MCSSet[3], pHTCapability->MCSSet[4],
				  (pHTCapability->MCSSet[11] << 8) + pHTCapability->MCSSet[10],
				  pHTCapability->MCSSet[12]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tExt HT Cap Info:\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\t\t PCO(%d), TransTime(%d), MCSFeedback(%d), +HTC(%d), RDG(%d)\n",
				  pExtHT->Pco, pExtHT->TranTime, pExtHT->MCSFeedback, pExtHT->PlusHTC, pExtHT->RDGSupport));
#ifdef TXBF_SUPPORT
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tTX BF Cap:\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\t\t ImpRxCap(%d), RXStagSnd(%d), TXStagSnd(%d), RxNDP(%d), TxNDP(%d) ImpTxCap(%d)\n",
				  pBFCap->TxBFRecCapable, pBFCap->RxSoundCapable, pBFCap->TxSoundCapable,
				  pBFCap->RxNDPCapable, pBFCap->TxNDPCapable, pBFCap->ImpTxBFCapable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\t\t Calibration(%d), ExpCSICapable(%d), ExpComSteerCapable(%d), ExpCSIFbk(%d), ExpNoComBF(%d) ExpComBF(%d)\n",
				  pBFCap->Calibration, pBFCap->ExpCSICapable, pBFCap->ExpComSteerCapable,
				  pBFCap->ExpCSIFbk, pBFCap->ExpNoComBF, pBFCap->ExpComBF));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\t\t MinGrouping(%d), CSIBFAntSup(%d), NoComSteerBFAntSup(%d), ComSteerBFAntSup(%d), CSIRowBFSup(%d) ChanEstimation(%d)\n",
				  pBFCap->MinGrouping, pBFCap->CSIBFAntSup, pBFCap->NoComSteerBFAntSup,
				  pBFCap->ComSteerBFAntSup, pBFCap->CSIRowBFSup, pBFCap->ChanEstimation));
#endif /* TXBF_SUPPORT */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\nPeer - MODE=%d, BW=%d, MCS=%d, ShortGI=%d, MaxRxFactor=%d, MpduDensity=%d, MIMOPS=%d, AMSDU=%d\n",
				  pEntry->HTPhyMode.field.MODE, pEntry->HTPhyMode.field.BW,
				  pEntry->HTPhyMode.field.MCS, pEntry->HTPhyMode.field.ShortGI,
				  pEntry->MaxRAmpduFactor, pEntry->MpduDensity,
				  pEntry->MmpsMode, pEntry->AMsduSize));
#ifdef DOT11N_DRAFT3
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tExt Cap Info:\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tBss2040CoexistMgmt=%d\n",
				 pEntry->BSS2040CoexistenceMgmtSupport));
#endif /* DOT11N_DRAFT3 */
	}
}

INT	Set_BurstMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = os_str_tol(arg, 0, 10);
	pAd->CommonCfg.bRalinkBurstMode = ((Value == 1) ? TRUE : FALSE);
	AsicSetRalinkBurstMode(pAd, pAd->CommonCfg.bRalinkBurstMode);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_BurstMode_Proc ::%s\n",
			 (pAd->CommonCfg.bRalinkBurstMode == TRUE) ? "enabled" : "disabled"));
	return TRUE;
}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
VOID assoc_vht_info_debugshow(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN VHT_CAP_IE * vht_cap,
	IN VHT_OP_IE * vht_op)
{
	VHT_CAP_INFO *cap_info;
	VHT_MCS_SET *mcs_set;
	struct vht_opinfo *op_info;
	VHT_MCS_MAP *mcs_map;
	struct wifi_dev *wdev = pEntry->wdev;
	UCHAR PhyMode = wdev->PhyMode;

	if (!WMODE_CAP_AC(PhyMode))
		return;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Peer - 11AC VHT Info\n"));

	if (vht_cap) {
		cap_info = &vht_cap->vht_cap;
		mcs_set = &vht_cap->mcs_set;
		hex_dump("peer vht_cap raw data", (UCHAR *)cap_info, sizeof(VHT_CAP_INFO));
		hex_dump("peer vht_mcs raw data", (UCHAR *)mcs_set, sizeof(VHT_MCS_SET));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tVHT Cap Info:\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\t\tMaxMpduLen(%d), BW(%d), SGI_80M(%d), RxLDPC(%d), TxSTBC(%d), RxSTBC(%d), +HTC-VHT(%d)\n",
				  cap_info->max_mpdu_len, cap_info->ch_width, cap_info->sgi_80M, cap_info->rx_ldpc, cap_info->tx_stbc,
				  cap_info->rx_stbc, cap_info->htc_vht_cap));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\t\tMaxAmpduExp(%d), VhtLinkAdapt(%d), RxAntConsist(%d), TxAntConsist(%d)\n",
				  cap_info->max_ampdu_exp, cap_info->vht_link_adapt, cap_info->rx_ant_consistency, cap_info->tx_ant_consistency));
		mcs_map = &mcs_set->rx_mcs_map;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tRxMcsSet: HighRate(%d), RxMCSMap(%d,%d,%d,%d,%d,%d,%d)\n",
				 mcs_set->rx_high_rate, mcs_map->mcs_ss1, mcs_map->mcs_ss2, mcs_map->mcs_ss3,
				 mcs_map->mcs_ss4, mcs_map->mcs_ss5, mcs_map->mcs_ss6, mcs_map->mcs_ss7));
		mcs_map = &mcs_set->tx_mcs_map;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tTxMcsSet: HighRate(%d), TxMcsMap(%d,%d,%d,%d,%d,%d,%d)\n",
				 mcs_set->tx_high_rate, mcs_map->mcs_ss1, mcs_map->mcs_ss2, mcs_map->mcs_ss3,
				 mcs_map->mcs_ss4, mcs_map->mcs_ss5, mcs_map->mcs_ss6, mcs_map->mcs_ss7));
#ifdef VHT_TXBF_SUPPORT
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tETxBfCap: Bfer(%d), Bfee(%d), SndDim(%d)\n",
				 cap_info->bfer_cap_su, cap_info->bfee_cap_su, cap_info->num_snd_dimension));
#endif
	}

	if (vht_op) {
		op_info = &vht_op->vht_op_info;
		mcs_map = &vht_op->basic_mcs_set;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tVHT OP Info:\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tChannel Width(%d), CenteralFreq1(%d), CenteralFreq2(%d)\n",
				 op_info->ch_width, op_info->ccfs_0, op_info->ccfs_1));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\t\tBasicMCSSet(SS1:%d, SS2:%d, SS3:%d, SS4:%d, SS5:%d, SS6:%d, SS7:%d)\n",
				  mcs_map->mcs_ss1, mcs_map->mcs_ss2, mcs_map->mcs_ss3,
				  mcs_map->mcs_ss4, mcs_map->mcs_ss5, mcs_map->mcs_ss6,
				  mcs_map->mcs_ss7));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));
}
#endif /* DOT11_VHT_AC */

INT Set_RateAdaptInterval(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ra_time, ra_qtime;
	RTMP_STRING *token;
	char sep = ':';
	ULONG irqFlags;
	/*
		The ra_interval inupt string format should be d:d, in units of ms.
			=>The first decimal number indicates the rate adaptation checking period,
			=>The second decimal number indicates the rate adaptation quick response checking period.
	*/
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():%s\n", __func__, arg));
	token = strchr(arg, sep);

	if (token != NULL) {
		*token = '\0';

		if (strlen(arg) && strlen(token + 1)) {
			ra_time = os_str_tol(arg, 0, 10);
			ra_qtime = os_str_tol(token + 1, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Set RateAdaptation TimeInterval as(%d:%d) ms\n",
					 __func__, ra_time, ra_qtime));
			RTMP_IRQ_LOCK(&pAd->irq_lock, irqFlags);
			pAd->ra_interval = ra_time;
			pAd->ra_fast_interval = ra_qtime;
#ifdef CONFIG_AP_SUPPORT

			if (pAd->ApCfg.ApQuickResponeForRateUpTimerRunning == TRUE) {
				BOOLEAN Cancelled;

				RTMPCancelTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, &Cancelled);
				pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;
			}

#endif /* CONFIG_AP_SUPPORT  */
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqFlags);
			return TRUE;
		}
	}

	return FALSE;
}

INT Set_VcoPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	cap->VcoPeriod = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("VCO Period = %d seconds\n", cap->VcoPeriod));
	return TRUE;
}

#ifdef SNIFFER_SUPPORT
INT Set_MonitorMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_SNIFFER_MODE_T SnifferFWCmd;
#endif /* CONFIG_HW_HAL_OFFLOAD */
	POS_COOKIE pObj;
	struct wifi_dev *wdev;

	pAd->monitor_ctrl.CurrentMonitorMode = os_str_tol(arg, 0, 10);
	pObj = (POS_COOKIE)pAd->OS_Cookie;
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
	SnifferFWCmd.ucDbdcIdx = 0;
#ifdef DBDC_MODE
	SnifferFWCmd.ucDbdcIdx = HcGetBandByWdev(wdev);
#endif

	if (pAd->monitor_ctrl.CurrentMonitorMode > MONITOR_MODE_FULL || pAd->monitor_ctrl.CurrentMonitorMode < MONITOR_MODE_OFF)
		pAd->monitor_ctrl.CurrentMonitorMode = MONITOR_MODE_OFF;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("set Current Monitor Mode = %d , range(%d ~ %d)\n"
			  , pAd->monitor_ctrl.CurrentMonitorMode, MONITOR_MODE_OFF, MONITOR_MODE_FULL));

	switch (pAd->monitor_ctrl.CurrentMonitorMode) {
	case MONITOR_MODE_OFF:			/* reset to normal */
		pAd->monitor_ctrl.bMonitorOn = FALSE;
#ifdef CONFIG_HW_HAL_OFFLOAD
		SnifferFWCmd.ucSnifferEn = 0;
		MtCmdSetSnifferMode(pAd, &SnifferFWCmd);
#else
		AsicSetRxFilter(pAd);
#endif /* CONFIG_HW_HAL_OFFLOAD */
		break;

	case MONITOR_MODE_REGULAR_RX:			/* report probe_request only , normal rx filter */
		pAd->monitor_ctrl.bMonitorOn = TRUE;
#ifdef CONFIG_HW_HAL_OFFLOAD
		SnifferFWCmd.ucSnifferEn = 1;
		MtCmdSetSnifferMode(pAd, &SnifferFWCmd);
#else
		AsicSetRxFilter(pAd);
#endif /* CONFIG_HW_HAL_OFFLOAD */
		break;

	case MONITOR_MODE_FULL:			/* fully report, Enable Rx with promiscuous reception */
		pAd->monitor_ctrl.bMonitorOn = TRUE;
#ifdef CONFIG_HW_HAL_OFFLOAD
		SnifferFWCmd.ucSnifferEn = 1;
		MtCmdSetSnifferMode(pAd, &SnifferFWCmd);
#else
		AsicSetRxFilter(pAd);
#endif /* CONFIG_HW_HAL_OFFLOAD */
		break;
	}

	return TRUE;
}

INT Set_MonitorFilterSize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->monitor_ctrl.FilterSize = os_str_tol(arg, 0, 10);
	if (pAd->monitor_ctrl.FilterSize < sizeof(struct mtk_radiotap_header))
		pAd->monitor_ctrl.FilterSize = RX_BUFFER_SIZE_MIN + sizeof(struct mtk_radiotap_header);
	return TRUE;
}

INT Set_MonitorFrameType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->monitor_ctrl.FrameType = os_str_tol(arg, 0, 10);
	if (pAd->monitor_ctrl.FrameType > FC_TYPE_DATA)
		pAd->monitor_ctrl.FrameType = FC_TYPE_RSVED;
	return TRUE;
}

INT Set_MonitorMacFilter_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT ret = TRUE;
	RTMP_STRING *this_char = NULL;
	RTMP_STRING *value = NULL;
	INT idx = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--> %s()\n", __func__));

	while ((this_char = strsep((char **)&arg, ";")) != NULL) {
		if (*this_char == '\0') {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("An unnecessary delimiter entered!\n"));
			continue;
		}
		/* the acceptable format of MAC address is like 01:02:03:04:05:06 with length 17 */
		if (strlen(this_char) != 17) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("illegal MAC address length! (acceptable format 01:02:03:04:05:06 length 17)\n"));
			continue;
		}

		for (idx = 0, value = rstrtok(this_char, ":"); value; value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("illegal MAC address format or octet!\n"));
				break;
			}

			AtoH(value, &pAd->monitor_ctrl.MacFilterAddr[idx++], 1);
		}

		if (idx != MAC_ADDR_LEN)
			continue;
	}

	for (idx = 0; idx < MAC_ADDR_LEN; idx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02X ", pAd->monitor_ctrl.MacFilterAddr[idx]));

	pAd->monitor_ctrl.MacFilterOn = TRUE;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<-- %s()\n", __func__));
	return ret;
}

INT Set_MonitorMacFilterOff_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->monitor_ctrl.MacFilterOn = FALSE;
	return TRUE;
}

#endif /* SNIFFER_SUPPORT */

#ifdef SINGLE_SKU
INT Set_ModuleTxpower_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 Value;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Do NOT accept this command after interface is up.\n"));
		return FALSE;
	}

	Value = (UINT16)os_str_tol(arg, 0, 10);
	pAd->CommonCfg.ModuleTxpower = Value;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF Set_ModuleTxpower_Proc::(ModuleTxpower=%d)\n",
			 pAd->CommonCfg.ModuleTxpower));
	return TRUE;
}
#endif /* SINGLE_SKU */


INT set_no_bcn(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG no_bcn;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	no_bcn = os_str_tol(arg, 0, 10);

	if (wdev) {
		if (no_bcn)
			UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_DISABLE_TX);
		else
			UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_ENABLE_TX);
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Set no beacon as:%d\n",
			 __func__, (no_bcn ? 1 : 0)));
	return TRUE;
}

#if defined(WFA_VHT_PF) || defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA) ||  defined(MT7637_FPGA)
INT set_force_amsdu(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->force_amsdu = (os_str_tol(arg, 0, 10) > 0 ? TRUE : FALSE);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): force_amsdu=%d\n",
			 __func__, pAd->force_amsdu));
	return TRUE;
}
#endif /* defined(WFA_VHT_PF) || defined(MT7603_FPGA) */

#ifdef WFA_VHT_PF
INT set_vht_nss_mcs_cap(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *token, sep[2] = {':', '-'};
	UCHAR val[3] = {0}, ss, mcs_l, mcs_h, mcs_cap, status = FALSE;
	INT idx = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():intput string=%s\n", __func__, arg));
	ss = mcs_l = mcs_h = 0;

	while (arg) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():intput string[len=%d]=%s\n", __func__, strlen(arg), arg));

		if (idx < 2) {
			token = rtstrchr(arg, sep[idx]);

			if (!token) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("cannot found token '%c' in string \"%s\"!\n", sep[idx], arg));
				return FALSE;
			}

			*token++ = 0;
		} else
			token = NULL;

		if (strlen(arg)) {
			val[idx] = (UCHAR)os_str_toul(arg, NULL, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():token string[len=%d]=%s, val[%d]=%d\n",
					 __func__, strlen(arg), arg, idx, val[idx]));
			idx++;
		}

		arg = token;

		if (idx == 3)
			break;
	}

	if (idx < 3)
		return FALSE;

	ss = val[0];
	mcs_l = val[1];
	mcs_h = val[2];
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ss=%d, mcs_l=%d, mcs_h=%d\n", ss, mcs_l, mcs_h));

	if (ss && mcs_h) {
		if (ss <= cap->mcs_nss.max_nss)
			pAd->CommonCfg.vht_nss_cap = ss;
		else
			pAd->CommonCfg.vht_nss_cap = cap->mcs_nss.max_nss;

		switch (mcs_h) {
		case 7:
			mcs_cap = VHT_MCS_CAP_7;
			break;

		case 8:
			mcs_cap = VHT_MCS_CAP_8;
			break;

		case 9:
			mcs_cap = VHT_MCS_CAP_9;
			break;

		default:
			mcs_cap = VHT_MCS_CAP_9;
			break;
		}

		if (mcs_h <= cap->mcs_nss.max_vht_mcs)
			pAd->CommonCfg.vht_mcs_cap = mcs_cap;
		else
			pAd->CommonCfg.vht_mcs_cap = cap->mcs_nss.max_vht_mcs;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():ss=%d, mcs_cap=%d, vht_nss_cap=%d, vht_mcs_cap=%d\n",
				 __func__, ss, mcs_cap,
				 pAd->CommonCfg.vht_nss_cap,
				 pAd->CommonCfg.vht_mcs_cap));
		status = TRUE;
	}

	return status;
}

INT set_vht_nss_mcs_opt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():intput string=%s\n", __func__, arg));
	return Set_HtMcs_Proc(pAd, arg);
}

INT set_vht_opmode_notify_ie(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *token;
	UINT ss, bw;
	BOOLEAN status = FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():intput string=%s\n", __func__, arg));
	token = rtstrchr(arg, ':');

	if (!token)
		return FALSE;

	*token = 0;
	token++;

	if (strlen(arg) && strlen(token)) {
		ss = os_str_toul(arg, NULL, 10);
		bw = os_str_toul(token, NULL, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():ss=%d, bw=%d\n", __func__, ss, bw));

		if (ss > 0 && ss <= 2)
			pAd->vht_pf_op_ss = ss;
		else
			pAd->vht_pf_op_ss = pAd->Antenna.field.RxPath;

		switch (bw) {
		case 20:
			pAd->vht_pf_op_bw = BAND_WIDTH_20;
			break;

		case 40:
			pAd->vht_pf_op_bw = BAND_WIDTH_40;
			break;

		case 80:
		default:
			pAd->vht_pf_op_bw = BAND_WIDTH_80;
			break;
		}

		status = TRUE;
	}

	pAd->force_vht_op_mode = status;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():force_vht_op_mode=%d, vht_pf_op_ss=%d, vht_pf_op_bw=%d\n",
			 __func__, pAd->force_vht_op_mode, pAd->vht_pf_op_ss, pAd->vht_pf_op_bw));
	return status;
}

INT set_force_operating_mode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->force_vht_op_mode = (os_str_tol(arg, 0, 10) > 0 ? TRUE : FALSE);

	if (pAd->force_vht_op_mode == TRUE) {
		pAd->vht_pf_op_ss = 1; /* 1SS */
		pAd->vht_pf_op_bw = BAND_WIDTH_20; /* 20M */
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): force_operating_mode=%d\n",
			 __func__, pAd->force_vht_op_mode));

	if (pAd->force_vht_op_mode == TRUE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tforce_operating_mode as %dSS in 20MHz BW\n",
				 pAd->vht_pf_op_ss));
	}

	return TRUE;
}

INT set_force_noack(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->force_noack = (os_str_tol(arg, 0, 10) > 0 ? TRUE : FALSE);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): force_noack=%d\n",
			 __func__, pAd->force_noack));
	return TRUE;
}

INT set_force_vht_sgi(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->vht_force_sgi = (os_str_tol(arg, 0, 10) > 0 ? TRUE : FALSE);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): vht_force_sgi=%d\n",
			 __func__, pAd->vht_force_sgi));
	return TRUE;
}

INT set_force_vht_tx_stbc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucTxPath = pAd->Antenna.field.TxPath;

	if (!wdev)
		return FALSE;

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		UINT8 band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0)
			ucTxPath = pAd->dbdc_band0_tx_path;
		else
			ucTxPath = pAd->dbdc_band1_tx_path;
	}
#endif

	pAd->vht_force_tx_stbc = (os_str_tol(arg, 0, 10) > 0 ? TRUE : FALSE);

	if (ucTxPath < 2) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Tx Path=%d is not enough for TxSTBC!\n",
				 __func__, ucTxPath));
		pAd->vht_force_tx_stbc = 0;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): vht_force_tx_stbc=%d\n",
			 __func__, pAd->vht_force_tx_stbc));
	return TRUE;
}

INT set_force_ext_cca(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG cca_cfg;
	UINT32 mac_val;

	cca_cfg = (os_str_tol(arg, 0, 10) > 0 ? TRUE : FALSE);

	if (cca_cfg)
		mac_val = 0x04101b3f;
	else
		mac_val = 0x583f;

	RTMP_IO_WRITE32(pAd->hdev_ctrl, TXOP_CTRL_CFG, mac_val);
	return TRUE;
}
#endif /* WFA_VHT_PF */

#ifdef DOT11_N_SUPPORT

#define MAX_AGG_CNT	8

/* DisplayTxAgg - display Aggregation statistics from MAC */
void DisplayTxAgg(RTMP_ADAPTER *pAd)
{
	ULONG totalCount;
	ULONG aggCnt[MAX_AGG_CNT + 2];
	int i;

	AsicReadAggCnt(pAd, aggCnt, sizeof(aggCnt) / sizeof(ULONG));
	totalCount = aggCnt[0] + aggCnt[1];

	if (totalCount > 0)
		for (i = 0; i < MAX_AGG_CNT; i++)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d MPDU=%ld (%ld%%)\n", i + 1, aggCnt[i + 2],
					 aggCnt[i + 2] * 100 / totalCount));

	printk("====================\n");
}
#endif /* DOT11_N_SUPPORT */

#ifdef REDUCE_TCP_ACK_SUPPORT
INT Set_ReduceAckEnable_Proc(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_STRING     *pParam)
{
	if (pParam == NULL)
		return FALSE;

	ReduceAckSetEnable(pAdapter, os_str_tol(pParam, 0, 10));
	return TRUE;
}

INT Show_ReduceAckInfo_Proc(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_STRING     *pParam)
{
	ReduceAckShow(pAdapter);
	return TRUE;
}

INT Set_ReduceAckProb_Proc(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_STRING     *pParam)
{
	if (pParam == NULL)
		return FALSE;

	ReduceAckSetProbability(pAdapter, os_str_tol(pParam, 0, 10));
	return TRUE;
}
#endif

#ifdef RTMP_RBUS_SUPPORT
#ifdef LED_CONTROL_SUPPORT
INT Set_WlanLed_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING *arg)
{
#if defined(RTMP_PCI_SUPPORT) && defined(RTMP_RBUS_SUPPORT)

	if (!IS_RBUS_INF(pAd)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : Support RBUS interface only\n", __func__));
		return TRUE;
	}

#endif /* defined(RTMP_PCI_SUPPORT) && defined(RTMP_RBUS_SUPPORT) */
	BOOLEAN bWlanLed;

	bWlanLed = (BOOLEAN) os_str_tol(arg, 0, 10);
	{
		if (bWlanLed)
			RTMPStartLEDMode(pAd);
		else
			RTMPExitLEDMode(pAd);
	};
	return TRUE;
}
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

#ifdef MT_MAC
static INT32 SetMTRF(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT RFIdx, Offset, Value, Rv;

	if (Arg) {
#if defined(MT7626)
		/* Input value of RFIdx as Hexadecimal for setting 2phy */
		Rv = sscanf(Arg, "%x-%x-%x", (int *)&RFIdx, (int *)&Offset, (int *)&Value);
#else
		Rv = sscanf(Arg, "%d-%x-%x", (int *)&RFIdx, (int *)&Offset, (int *)&Value);
#endif /* defined(MT7626) */
		/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RfIdx = %d, Offset = 0x%04x, Value = 0x%08x\n", RFIdx, Offset, Value)); */

		if (Rv == 2) {
			Value = 0;
			MtCmdRFRegAccessRead(pAd, (UINT32)RFIdx, (UINT32)Offset, (UINT32 *)&Value);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():%d 0x%04x 0x%08x\n", __func__, RFIdx, Offset, Value));
		}

		if (Rv == 3) {
			MtCmdRFRegAccessWrite(pAd, (UINT32)RFIdx, (UINT32)Offset, (UINT32)Value);
			Value = 0;
			MtCmdRFRegAccessRead(pAd, (UINT32)RFIdx, (UINT32)Offset, (UINT32 *)&Value);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():%d 0x%04x 0x%08x\n", __func__, RFIdx, Offset, Value));
		}
	}

	return TRUE;
}
#endif

INT32 SetRF(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
#ifdef MT_MAC
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->rf_type == RF_MT)
		Ret = SetMTRF(pAd, Arg);

#endif
	return Ret;
}

static struct {
	RTMP_STRING *name;
	INT (*show_proc)(RTMP_ADAPTER *pAd, RTMP_STRING *arg, ULONG BufLen);
} *PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC, RTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC[] = {
#ifdef DBG
	{"SSID",					Show_SSID_Proc},
	{"WirelessMode",			Show_WirelessMode_Proc},
	{"TxBurst",					Show_TxBurst_Proc},
	{"TxPreamble",				Show_TxPreamble_Proc},
	{"TxPower",					Show_TxPower_Proc},
	{"Channel",					Show_Channel_Proc},
	{"BGProtection",			Show_BGProtection_Proc},
	{"RTSThreshold",			Show_RTSThreshold_Proc},
	{"FragThreshold",			Show_FragThreshold_Proc},
#ifdef DOT11_N_SUPPORT
	{"HtBw",					Show_HtBw_Proc},
	{"HtMcs",					Show_HtMcs_Proc},
	{"HtGi",					Show_HtGi_Proc},
	{"HtOpMode",				Show_HtOpMode_Proc},
	{"HtExtcha",				Show_HtExtcha_Proc},
	{"HtMpduDensity",			Show_HtMpduDensity_Proc},
	{"HtBaWinSize",		        Show_HtBaWinSize_Proc},
	{"HtRdg",			Show_HtRdg_Proc},
	{"HtAmsdu",			Show_HtAmsdu_Proc},
	{"HtAutoBa",		        Show_HtAutoBa_Proc},
#endif /* DOT11_N_SUPPORT */
	{"CountryRegion",			Show_CountryRegion_Proc},
	{"CountryRegionABand",		Show_CountryRegionABand_Proc},
	{"CountryCode",				Show_CountryCode_Proc},
#ifdef AGGREGATION_SUPPORT
	{"PktAggregate",			Show_PktAggregate_Proc},
#endif

	{"WmmCapable",				Show_WmmCapable_Proc},

	{"IEEE80211H",				Show_IEEE80211H_Proc},
#ifdef CONFIG_STA_SUPPORT
	{"NetworkType",				Show_NetworkType_Proc},
#ifdef WSC_STA_SUPPORT
	{"WpsApBand",				Show_WpsPbcBand_Proc},
	{"Manufacturer",			Show_WpsManufacturer_Proc},
	{"ModelName",				Show_WpsModelName_Proc},
	{"DeviceName",				Show_WpsDeviceName_Proc},
	{"ModelNumber",				Show_WpsModelNumber_Proc},
	{"SerialNumber",			Show_WpsSerialNumber_Proc},
#endif /* WSC_STA_SUPPORT */
	{"WPAPSK",					Show_WPAPSK_Proc},
	{"AutoReconnect",			Show_AutoReconnect_Proc},
	{"secinfo",				Show_STASecurityInfo_Proc},
#endif /* CONFIG_STA_SUPPORT */
#ifdef SINGLE_SKU
	{"ModuleTxpower",			Show_ModuleTxpower_Proc},
#endif /* SINGLE_SKU */
#endif /* DBG */
	{"rainfo",					Show_STA_RAInfo_Proc},
	{NULL, NULL}
};

INT RTMPShowCfgValue(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *pName,
	IN	RTMP_STRING *pBuf,
	IN	UINT32			MaxLen)
{
	INT	Status = 0;

	for (PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC = RTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC;
		 PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name; PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC++) {
		if (!strcmp(pName, PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name)) {
			if (PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->show_proc(pAd, pBuf, MaxLen))
				Status = -EINVAL;

			break;  /*Exit for loop.*/
		}
	}

	if (PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name == NULL) {
		snprintf(pBuf, MaxLen, "\n");

		for (PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC = RTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC;
			 PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name; PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC++) {
			if ((strlen(pBuf) + strlen(PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name)) >= MaxLen)
				break;

			sprintf(pBuf, "%s%s\n", pBuf, PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name);
		}
	}

	return Status;
}

INT32 ShowBBPInfo(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ShowAllBBP(pAd);
	return TRUE;
}

INT32 ShowRFInfo(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ShowAllRF(pAd);
	return 0;
}

#define WIFI_INTERRUPT_NUM_MAX  1

INT32 ShowWifiInterruptCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	const UCHAR WifiIntMaxNum = WIFI_INTERRUPT_NUM_MAX;
	const CHAR WifiIntDesc[WIFI_INTERRUPT_NUM_MAX][32] = {"Wifi Abnormal counter"};
	UINT32 WifiIntCnt[WIFI_INTERRUPT_NUM_MAX];
	UINT32 WifiIntMask = 0xF;
	UCHAR BandIdx;
	UINT32 WifiIntIdx;

	os_zero_mem(WifiIntCnt, sizeof(WifiIntCnt));

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		MtCmdGetWifiInterruptCnt(pAd, BandIdx, WifiIntMaxNum, WifiIntMask, WifiIntCnt);

		for (WifiIntIdx = 0; WifiIntIdx < WifiIntMaxNum; WifiIntIdx++)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Band %u:%s = %u\n", BandIdx, WifiIntDesc[WifiIntIdx],
					 WifiIntCnt[WifiIntIdx]));
	}

	return TRUE;
}

#ifdef BACKGROUND_SCAN_SUPPORT
INT set_background_scan(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BgndscanType = os_str_tol(arg, 0, 10);

	BackgroundScanStart(pAd, BgndscanType);
	return TRUE;
}

INT set_background_scan_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32	bgndscanduration = 0; /* ms */
	UINT32	bgndscaninterval = 0; /* second */
	UINT32	bgndscannoisyth = 0;
	UINT32	bgndscanchbusyth = 0;
	UINT32	DriverTrigger = 0;
	UINT32	bgndscansupport = 0;
	UINT32	ipith = 0;
	INT32	Recv = 0;

	Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d", &(bgndscanduration), &(bgndscaninterval), &(bgndscannoisyth),
				  &(bgndscanchbusyth), &(ipith), &(DriverTrigger), &(bgndscansupport));

	if (Recv != 7) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error!\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("iwpriv ra0 set bgndscancfg=[Scan_Duration]-[Partial_Scan_Interval]-[Noisy_TH]-[BusyTime_TH]-[IPI_TH]-[Driver_trigger_Support]-[BGND_Support]\n"));
	} else {
		pAd->BgndScanCtrl.ScanDuration = bgndscanduration;
		pAd->BgndScanCtrl.PartialScanInterval = bgndscaninterval;
		pAd->BgndScanCtrl.NoisyTH = bgndscannoisyth;
		pAd->BgndScanCtrl.ChBusyTimeTH = bgndscanchbusyth;
		pAd->BgndScanCtrl.DriverTrigger = (BOOL)DriverTrigger;
		pAd->BgndScanCtrl.BgndScanSupport = (BOOL)bgndscansupport;
		pAd->BgndScanCtrl.IPIIdleTimeTH = (BOOL)ipith;
	}

	return TRUE;
}

INT set_background_scan_test(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	i;
	CHAR *value = 0;
	MT_BGND_SCAN_CFG BgndScanCfg;

	os_zero_mem(&BgndScanCfg, sizeof(MT_BGND_SCAN_CFG));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0: /* ControlChannel */
			BgndScanCfg.ControlChannel = os_str_tol(value, 0, 10);
			break;

		case 1: /*  CentralChannel */
			BgndScanCfg.CentralChannel = os_str_tol(value, 0, 10);
			break;

		case 2: /* BW */
			BgndScanCfg.Bw = os_str_tol(value, 0, 10);
			break;

		case 3: /* TxStream */
			BgndScanCfg.TxStream = os_str_tol(value, 0, 10);
			break;

		case 4: /* RxPath */
			BgndScanCfg.RxPath = os_str_tol(value, 0, 16);
			break;

		case 5: /* Reason */
			BgndScanCfg.Reason = os_str_tol(value, 0, 10);
			break;

		case 6: /* BandIdx */
			BgndScanCfg.BandIdx = os_str_tol(value, 0, 10);
			break;

		default:
			break;
		}
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s  Bandidx=%d, BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d\n",
			  __func__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
			  BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));
	BackgroundScanTest(pAd, BgndScanCfg);
	return TRUE;
}
INT set_background_scan_notify(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *value = 0;
	MT_BGND_SCAN_NOTIFY BgScNotify;
	int i;

	os_zero_mem(&BgScNotify, sizeof(MT_BGND_SCAN_NOTIFY));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0: /* Notify function */
			BgScNotify.NotifyFunc = os_str_tol(value, 0, 10);
			break;

		case 1: /*  Status */
			BgScNotify.BgndScanStatus = os_str_tol(value, 0, 10);
			break;

		default:
			break;
		}
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s  NotifyFunc=%d, BgndScanStatus=%d\n",
			 __func__, BgScNotify.NotifyFunc, BgScNotify.BgndScanStatus));
	MtCmdBgndScanNotify(pAd, BgScNotify);
	return TRUE;
}

INT show_background_scan_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" Background scan support = %d\n", pAd->BgndScanCtrl.BgndScanSupport));

	if (pAd->BgndScanCtrl.BgndScanSupport == TRUE) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("===== Configuration =====\n"));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" Channel busy time Threshold = %d\n", pAd->BgndScanCtrl.ChBusyTimeTH));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" Noisy Threshold = %d\n", pAd->BgndScanCtrl.NoisyTH));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" IPI Idle Threshold (*8us) = %d\n", pAd->BgndScanCtrl.IPIIdleTimeTH));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" Scan Duration = %d ms\n", pAd->BgndScanCtrl.ScanDuration));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" Partial Scan Interval = %d second\n", pAd->BgndScanCtrl.PartialScanInterval));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" DriverTrigger support= %d\n", pAd->BgndScanCtrl.DriverTrigger));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("===== Status / Statistic =====\n"));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" One sec channel busy time = %d\n",
				 pAd->OneSecMibBucket.ChannelBusyTimeCcaNavTx[0]));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" One sec primary channel busy time = %d\n",
				 pAd->OneSecMibBucket.ChannelBusyTime[0]));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" One sec My Tx Airtime = %d\n", pAd->OneSecMibBucket.MyTxAirtime[0]));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" One sec My Rx Airtime = %d\n", pAd->OneSecMibBucket.MyRxAirtime[0]));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" IPI Idle time = %d\n", pAd->BgndScanCtrl.IPIIdleTime));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" Noisy = %d\n", pAd->BgndScanCtrl.Noisy));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" Current state = %ld\n", pAd->BgndScanCtrl.BgndScanStatMachine.CurrState));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (" Scan type = %d\n", pAd->BgndScanCtrl.ScanType));
		/* MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, */
		/* (" Interval count = %d\n", pAd->BgndScanCtrl.BgndScanIntervalCount)); */
		/* MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, */
		/* (" Interval = %d\n", pAd->BgndScanCtrl.BgndScanInterval)); */
	}

	return TRUE;
}
#endif /* BACKGROUND_SCAN_SUPPORT */

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
/*
    ==========================================================================
    Description:
	RF test switch mode.

	iwpriv ra0 set RBIST_SwitchMode = ModeEnable

	ModeEnable
	0: OPERATION_NORMAL_MODE
	1: OPERATION_RFTEST_MODE
	2: OPERATION_ICAP_MODE
	4: OPERATION_WIFI_SPECTRUM

    Return:
    ==========================================================================
*/
INT32 Set_RBIST_Switch_Mode(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
#ifdef CONFIG_ATE
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
#endif/* CONFIG_ATE */
	UINT8 ModeEnable = 0;

	ModeEnable = simple_strtol(arg, 0, 10);
	if (ModeEnable == OPERATION_NORMAL_MODE) {
#ifdef CONFIG_ATE
		ATECtrl->Mode &= ~fATE_IN_RFTEST;
#endif/* CONFIG_ATE */
		MtCmdRfTestSwitchMode(pAd, OPERATION_NORMAL_MODE, 0,
							  RF_TEST_DEFAULT_RESP_LEN);
	} else if (ModeEnable == OPERATION_RFTEST_MODE) {
#ifdef CONFIG_ATE
		ATECtrl->Mode |= fATE_IN_RFTEST;
#endif/* CONFIG_ATE */
		MtCmdRfTestSwitchMode(pAd, OPERATION_RFTEST_MODE, 0,
							  RF_TEST_DEFAULT_RESP_LEN);
	} else if (ModeEnable == OPERATION_ICAP_MODE) {
#ifdef CONFIG_ATE
		ATECtrl->Mode |= fATE_IN_RFTEST;
#endif/* CONFIG_ATE */
		MtCmdRfTestSwitchMode(pAd, OPERATION_ICAP_MODE, 0,
							  RF_TEST_DEFAULT_RESP_LEN);
	} else if (ModeEnable == OPERATION_WIFI_SPECTRUM) {
#ifdef CONFIG_ATE
		ATECtrl->Mode &= ~fATE_IN_RFTEST;
#endif/* CONFIG_ATE */
		MtCmdRfTestSwitchMode(pAd, OPERATION_WIFI_SPECTRUM, 0,
							  RF_TEST_DEFAULT_RESP_LEN);
	} else {
#ifdef CONFIG_ATE
		ATECtrl->Mode &= ~fATE_IN_RFTEST;
#endif/* CONFIG_ATE */
		MtCmdRfTestSwitchMode(pAd, OPERATION_NORMAL_MODE, 0,
							  RF_TEST_DEFAULT_RESP_LEN);
	}

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set parameters when ICap/Wifi-spectrum is started or stopped.

	iwpriv ra0 set RBIST_CaptureStart
	= Mode : Trigger : RingCapEn : TriggerEvent : CaptureNode : CaptureLen :
	  CapStopCycle : BW : MACTriggerEvent : SourceAddr. : Band : PhyIdx

    Return:
    ==========================================================================
*/
INT32 Set_RBIST_Capture_Start(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	INT32 i, j, retval;
	RTMP_STRING Temp1[2] = {0};
	INT8 *pTemp1 = Temp1;
	INT8 *value = NULL;
	UINT32 Temp2[6] = {0};
	UINT32 Mode = 0, Trig = 0, RingCapEn = 0, BBPTrigEvent = 0, CapNode = 0;
	UINT32 CapLen = 0, CapStopCycle = 0, MACTrigEvent = 0, PhyIdx = 0;
	UINT32 SrcAddrLSB = 0, SrcAddrMSB = 0, BandIdx = 0, BW = 0;
	RBIST_CAP_START_T *prRBISTInfo = NULL;

	/* Dynamic allocate memory for prRBISTInfo */
	retval = os_alloc_mem(pAd, (UCHAR **)&prRBISTInfo, sizeof(RBIST_CAP_START_T));
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s : Not enough memory for dynamic allocating !!\n", __func__));
		goto error;
	}
	os_zero_mem(prRBISTInfo, sizeof(RBIST_CAP_START_T));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case CAP_MODE:
			Mode = simple_strtol(value, 0, 16);
			break;

		case CAP_TRIGGER:
			Trig = simple_strtol(value, 0, 16);
			prRBISTInfo->fgTrigger = Trig;
			break;

		case CAP_RING_MODE:
			RingCapEn = simple_strtol(value, 0, 16);
			prRBISTInfo->fgRingCapEn = RingCapEn;
			break;

		case CAP_BBP_EVENT:
			BBPTrigEvent = simple_strtol(value, 0, 16);
			prRBISTInfo->u4TriggerEvent = BBPTrigEvent;
			break;

		case CAP_NODE:
			CapNode = simple_strtol(value, 0, 16);
			prRBISTInfo->u4CaptureNode = CapNode;
			break;

		case CAP_LENGTH:
			CapLen = simple_strtol(value, 0, 16);
			prRBISTInfo->u4CaptureLen = CapLen;
			break;

		case CAP_STOP_CYCLE:
			CapStopCycle = simple_strtol(value, 0, 16);
			prRBISTInfo->u4CapStopCycle = CapStopCycle;
			break;

		case CAP_BW:
			BW = simple_strtol(value, 0, 16);
			prRBISTInfo->u4BW = BW;
			break;

		case CAP_MAC_EVENT:
			MACTrigEvent = simple_strtol(value, 0, 16);
			prRBISTInfo->u4MACTriggerEvent = MACTrigEvent;
			break;

		case CAP_SOURCE_ADDR:
			for (j = 0; j < 6; j++) {
				RTMPMoveMemory(pTemp1, value, 2);
				Temp2[j] = simple_strtol(pTemp1, 0, 16);
				value += 2;
			}

			SrcAddrLSB = (Temp2[0] | (Temp2[1] << 8) |
						  (Temp2[2] << 16) | (Temp2[3] << 24));
			SrcAddrMSB = (Temp2[4] | (Temp2[5] << 8) | (0x1 << 16));
			prRBISTInfo->u4SourceAddressLSB = SrcAddrLSB;
			prRBISTInfo->u4SourceAddressMSB = SrcAddrMSB;
			break;

		case CAP_BAND:
			BandIdx = simple_strtol(value, 0, 16);
			prRBISTInfo->u4BandIdx = BandIdx;
			break;

		case CAP_PHY:
			PhyIdx = simple_strtol(value, 0, 16);
			prRBISTInfo->u4PhyIdx = PhyIdx;
			break;

		default:
			break;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s :\n Mode = 0x%08x\n"
			" Trigger = 0x%08x\n RingCapEn = 0x%08x\n TriggerEvent = 0x%08x\n CaptureNode = 0x%08x\n"
			" CaptureLen = 0x%08x\n CapStopCycle = 0x%08x\n BW = 0x%08x\n MACTriggerEvent = 0x%08x\n"
			" SourceAddrLSB = 0x%08x\n SourceAddrMSB = 0x%08x\n Band = 0x%08x\n PhyIdx = 0x%08x\n", __func__,
			Mode, Trig, RingCapEn, BBPTrigEvent, CapNode, CapLen, CapStopCycle, BW, MACTrigEvent, SrcAddrLSB,
			SrcAddrMSB, BandIdx, PhyIdx));

	if (Mode == ICAP_MODE) {
#ifdef INTERNAL_CAPTURE_SUPPORT
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->ICapStart != NULL)
			ops->ICapStart(pAd, (UINT8 *)prRBISTInfo);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : The function is not hooked !!\n", __func__));
		}
#endif /* INTERNAL_CAPTURE_SUPPORT */
	} else if (Mode == WIFI_SPECTRUM_MODE) {
#ifdef WIFI_SPECTRUM_SUPPORT
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->SpectrumStart != NULL)
			ops->SpectrumStart(pAd, (UINT8 *)prRBISTInfo);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : The function is not hooked !!\n", __func__));
		}
#endif /* WIFI_SPECTRUM_SUPPORT */
	}

error:
	if (prRBISTInfo != NULL)
		os_free_mem(prRBISTInfo);

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Query ICap/Wifi-spectrum status.

	iwpriv ra0 set RBIST_CaptureStatus = Choice

	 Choice
	 0: ICAP_MODE
	 1: WIFI_SPECTRUM_MODE

    Return:
    ==========================================================================
*/
INT32 Get_RBIST_Capture_Status(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	INT32 Choice;

	Choice = simple_strtol(arg, 0, 10);
	switch (Choice) {
	case ICAP_MODE:
#ifdef INTERNAL_CAPTURE_SUPPORT
	{
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->ICapStatus != NULL)
			ops->ICapStatus(pAd);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : The function is not hooked !!\n", __func__));
		}
	}
#endif /* INTERNAL_CAPTURE_SUPPORT */
		break;

	case WIFI_SPECTRUM_MODE:
#ifdef WIFI_SPECTRUM_SUPPORT
	{
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->SpectrumStatus != NULL)
			ops->SpectrumStatus(pAd);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : The function is not hooked !!\n", __func__));
		}
	}
#endif /* WIFI_SPECTRUM_SUPPORT */
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Not support for %d this selection !!\n", __func__, Choice));
		break;
	}

	return TRUE;
}

/*
    ==========================================================================
    Description:
	 Get ICap/Wifi-spectrum RBIST sysram raw data .

	 iwpriv ra0 set RBIST_RawDataProc = Choice

	 Choice
	 0: ICAP_MODE
	    a. Get ICap RBIST sysram raw data by unsolicited event.(on-the-fly)
	    b. Re-arrange ICap sysram buffer by wrapper.
	    c. Parsing ICap I/Q data.
	    d. Dump L32bit/M32bit/H32bit to file.
	 1: WIFI_SPECTRUM_MODE
	    a. Get Wifi-spectrum RBIST sysram raw data by unsolicited event.(on-the-fly)
	    b. Parsing Wifi-spectrum I/Q data.
	    c. Dump I/Q/LNA/LPF data to file.
    Return:
    ==========================================================================
*/
INT32 Get_RBIST_Raw_Data_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	UINT32 Choice;
	INT32 Status = CAP_FAIL;

	Choice = simple_strtol(arg, 0, 10);
	switch (Choice) {
	case ICAP_MODE:
#ifdef INTERNAL_CAPTURE_SUPPORT
	{
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->ICapCmdRawDataProc != NULL)
			Status = ops->ICapCmdRawDataProc(pAd);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : The function is not hooked !!\n", __func__));
		}
	}
#endif /* INTERNAL_CAPTURE_SUPPORT */
		break;
	case WIFI_SPECTRUM_MODE:
#ifdef WIFI_SPECTRUM_SUPPORT
	{
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->SpectrumCmdRawDataProc != NULL)
			Status = ops->SpectrumCmdRawDataProc(pAd);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : The function is not hooked !!\n", __func__));
		}
	}
#endif /* WIFI_SPECTRUM_SUPPORT */
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Not support for %d this selection !!\n", __func__, Choice));
		break;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(Status = %d)\n", __func__, Status));

	return TRUE;
}

/*
    ==========================================================================
    Description:
	 Get ICap I/Q data which is stored in IQ_Array captured by
	 WF0/WF1/WF2/WF3.

    Return:
    ==========================================================================
*/
INT32 Get_RBIST_IQ_Data(
	IN RTMP_ADAPTER *pAd,
	IN PINT32 pData,
	IN PINT32 pDataLen,
	IN UINT32 IQ_Type,
	IN UINT32 WF_Num)
{
	UINT32 i, CapNode, TotalCnt, Len;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	P_RBIST_IQ_DATA_T pIQ_Array = pAd->pIQ_Array;

	/* Initialization of pData and pDataLen buffer */
	Len = ICAP_EVENT_DATA_SAMPLE * sizeof(INT32);
	os_zero_mem(pData, Len);
	os_zero_mem(pDataLen, sizeof(INT32));

	/* Query current capture node */
	CapNode = Get_System_CapNode_Info(pAd);
	/* Update total count of IQ sample */
	if (IS_MT7626(pAd)) {
		if ((CapNode == pChipCap->ICapWF01PackedADC)
			|| (CapNode == pChipCap->ICapWF02PackedADC)
			|| (CapNode == pChipCap->ICapWF12PackedADC))
			TotalCnt = pChipCap->ICapADCIQCnt;
		else
			TotalCnt = pChipCap->ICapIQCIQCnt;
	} else {
		if (CapNode == pChipCap->ICapPackedADC)
			TotalCnt = pChipCap->ICapADCIQCnt;
		else
			TotalCnt = pChipCap->ICapIQCIQCnt;
	}

	/* Update initial value of ICapDataCnt */
	if ((TotalCnt > pAd->ICapCapLen) && (pAd->ICapDataCnt == 0))
		pAd->ICapDataCnt = TotalCnt - pAd->ICapCapLen;

	/* Store I or Q data(1KBytes) to data buffer */
	for (i = 0; i < ICAP_EVENT_DATA_SAMPLE; i++) {
		UINT32 idx = pAd->ICapDataCnt;

		/* If it is the last one of I or Q data, just stop querying */
		if (pAd->ICapDataCnt == TotalCnt)
			break;

		/* Store I/Q data to data buffer */
		pData[i] = pIQ_Array[idx].IQ_Array[WF_Num][IQ_Type];
		/* Update data counter */
		pAd->ICapDataCnt++;
	}

	/* Update data length */
	*pDataLen = i;

	/* Reset data counter */
	if (*pDataLen == 0)
		pAd->ICapDataCnt = 0;

	return TRUE;
}

/*
    ==========================================================================
    Description:
	 Get ICap I/Q data which is captured by WF0 or WF1 or WF2 or WF3.

	 iwpriv ra0 set RBIST_IQDataProc = IQ_Type : WF_Num : ICap_Len

	 IQ_Type
	 0: I_TYPE/1: Q_TYPE
	 WF_Num
	 0: WF0/1: WF1/2: WF2/3: WF3
	 ICap_Len(Unit: I or Q sample cnt)

	 a. Store I/Q data which is captured by WF0/WF1/WF2/WF3 to data buffer.
	 b. Dump I/Q data to file.

    Return:
    ==========================================================================
*/
#define CAP_IQ_Type			0
#define CAP_WF_Num			1
#define CAP_Len				2
INT32 Get_RBIST_IQ_Data_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	INT i, retval;
	UINT32 IQ_Type = 0, WF_Num = 0, Len;
	PINT32 pData = NULL, pDataLen = NULL;
	RTMP_STRING *value = NULL;
	RTMP_STRING *pSrc_IQ = NULL;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case CAP_IQ_Type:
			IQ_Type = simple_strtol(value, 0, 10);
			break;

		case CAP_WF_Num:
			WF_Num = simple_strtol(value, 0, 10);
			break;
		case CAP_Len:
			pAd->ICapCapLen = simple_strtol(value, 0, 10);
			break;
		default:
			break;
		}
	}

	/* Dynamic allocate memory for pSrc_IQ */
	retval = os_alloc_mem(pAd, (UCHAR **)&pSrc_IQ, sizeof(RTMP_STRING));
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s : Not enough memory for dynamic allocating !!\n", __func__));
		goto error;
	}

	/* Dynamic allocate memory for 1KByte data buffer */
	Len = ICAP_EVENT_DATA_SAMPLE * sizeof(INT32);
	retval = os_alloc_mem(pAd, (UCHAR **)&pData, Len);
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s : Not enough memory for dynamic allocating !!\n", __func__));
		goto error;
	}

	/* Dynamic allocate memory for data length */
	retval = os_alloc_mem(pAd, (UCHAR **)&pDataLen, sizeof(INT32));
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s : Not enough memory for dynamic allocating !!\n", __func__));
		goto error;
	}

	/* Fill in title for console log */
	if (IQ_Type == CAP_I_TYPE)
		sprintf(pSrc_IQ, "Icap_%s%d", "I", WF_Num);
	else if (IQ_Type == CAP_Q_TYPE)
		sprintf(pSrc_IQ, "Icap_%s%d", "Q", WF_Num);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", pSrc_IQ));

	/* Initialization of ICapDataCnt */
	pAd->ICapDataCnt = 0;

	while (1) {
#ifdef INTERNAL_CAPTURE_SUPPORT
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		/* Query I/Q data from buffer */
		if (ops->ICapGetIQData != NULL)
			ops->ICapGetIQData(pAd, pData, pDataLen, IQ_Type, WF_Num);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : The function is not hooked !!\n", __func__));
		}
#endif /* INTERNAL_CAPTURE_SUPPORT */

		/* If data length is zero, it means the end of data querying */
		if (*pDataLen == 0)
			break;

		/* Print data log to console */
		for (i = 0; i < *pDataLen; i++)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d\n", pData[i]));
	}

error:
	if (pData != NULL)
		os_free_mem(pData);

	if (pDataLen != NULL)
		os_free_mem(pDataLen);

	if (pSrc_IQ != NULL)
		os_free_mem(pSrc_IQ);

	return TRUE;
}

/*
    ==========================================================================
    Description:
	 Get Icap/Wifi-spectrum capture node information.

    Return: Value of capture node.
    ==========================================================================
*/
UINT32 Get_System_CapNode_Info(
	IN RTMP_ADAPTER *pAd)
{
	UINT32 CapNode = 0, Value = 0;

	if (IS_MT7622(pAd) || IS_MT7615(pAd))
		PHY_IO_READ32(pAd->hdev_ctrl, CR_DBGSGD_MODE, &Value);
	else
		PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_CTRL_CR_DBGSGD_MODE_ADDR, &Value);

	CapNode = Value & BITS(CR_SGD_MODE1, CR_SGD_DBG_SEL);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s : CaptureNode = 0x%08x\n", __func__, CapNode));

	return CapNode;
}

/*
    ==========================================================================
    Description:
	 Get current band central frequency information.

    Return: Value of central frequency(MHz).
    ==========================================================================
*/
UINT32 Get_System_CenFreq_Info(
	IN RTMP_ADAPTER *pAd,
	IN UINT32 CapNode)
{
	UINT32 ChIdx, CenFreq = 0;
	UINT8 CenCh = 0;
	struct freq_oper oper;
	UCHAR rfic = RFIC_24GHZ;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pAd->CommonCfg.dbdc_mode) { /* Dual Band */
#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
		if ((CapNode == pChipCap->SpectrumWF0ADC)  || (CapNode == pChipCap->SpectrumWF1ADC)
			|| (CapNode == pChipCap->SpectrumWF0FIIQ) || (CapNode == pChipCap->SpectrumWF1FIIQ)
			|| (CapNode == pChipCap->SpectrumWF0FDIQ) || (CapNode == pChipCap->SpectrumWF1FDIQ))
			rfic = RFIC_24GHZ;
		else if ((CapNode == pChipCap->SpectrumWF2ADC)  || (CapNode == pChipCap->SpectrumWF3ADC)
				 || (CapNode == pChipCap->SpectrumWF2FIIQ) || (CapNode == pChipCap->SpectrumWF3FIIQ)
				 || (CapNode == pChipCap->SpectrumWF2FDIQ) || (CapNode == pChipCap->SpectrumWF3FDIQ))
			rfic =  RFIC_5GHZ;
#endif
	} else { /* Single Band */
		if (HcGetRadioChannel(pAd) <= 14)
			rfic = RFIC_24GHZ;
		else
			rfic = RFIC_5GHZ;
	}

	if (hc_radio_query_by_rf(pAd, rfic, &oper) != HC_STATUS_OK) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s : can't find radio for RFIC:%d\n", __func__, rfic));
	}

	CenCh = oper.cen_ch_1;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s : CentralCh = %d\n", __func__, CenCh));

	for (ChIdx = 0; ChIdx < CH_HZ_ID_MAP_NUM; ChIdx++) {
		if (CenCh == CH_HZ_ID_MAP[ChIdx].channel) {
			CenFreq = CH_HZ_ID_MAP[ChIdx].freqKHz;
			break;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s : CentralFreq = %d\n", __func__, CenFreq));

	return CenFreq;
}

/*
    ==========================================================================
    Description:
	 Get current band bandwidth information.

    Return: Value of capture BW.
	    CAP_BW_20                            0
	    CAP_BW_40                            1
	    CAP_BW_80                            2
    ==========================================================================
*/
UINT8 Get_System_Bw_Info(
	IN RTMP_ADAPTER *pAd,
	IN UINT32 CapNode)
{
	INT8 Bw = 0, CapBw = 0;
	struct freq_oper oper;
	UCHAR rfic = RFIC_24GHZ;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pAd->CommonCfg.dbdc_mode) { /* Dual Band */
		if ((CapNode == pChipCap->SpectrumWF0ADC)  || (CapNode == pChipCap->SpectrumWF1ADC)
			|| (CapNode == pChipCap->SpectrumWF0FIIQ) || (CapNode == pChipCap->SpectrumWF1FIIQ)
			|| (CapNode == pChipCap->SpectrumWF0FDIQ) || (CapNode == pChipCap->SpectrumWF1FDIQ))
			rfic = RFIC_24GHZ;
		else if ((CapNode == pChipCap->SpectrumWF2ADC)  || (CapNode == pChipCap->SpectrumWF3ADC)
				 || (CapNode == pChipCap->SpectrumWF2FIIQ) || (CapNode == pChipCap->SpectrumWF3FIIQ)
				 || (CapNode == pChipCap->SpectrumWF2FDIQ) || (CapNode == pChipCap->SpectrumWF3FDIQ))
			rfic = RFIC_5GHZ;
	} else { /* Single Band */
		if (HcGetRadioChannel(pAd) <= 14)
			rfic = RFIC_24GHZ;
		else
			rfic = RFIC_5GHZ;
	}

	if (hc_radio_query_by_rf(pAd, rfic, &oper) != HC_STATUS_OK) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s : can't find radio for RFIC:%d\n", __func__, rfic));

		return -1;
	}

		Bw = oper.bw;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s : Bw = %d\n", __func__, Bw));

	switch (Bw) {
	case CMD_BW_20:
		CapBw = CAP_BW_20;
		break;

	case CMD_BW_40:
		CapBw = CAP_BW_40;
		break;

	case CMD_BW_80:
		CapBw = CAP_BW_80;
		break;

	case CMD_BW_160:
		CapBw = CAP_BW_80;
		break;

	case CMD_BW_8080:
		CapBw = CAP_BW_80;
		break;

	default:
		CapBw = CAP_BW_20;
		break;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s : CaptureBw = %d\n", __func__, CapBw));

	return CapBw;
}

/*
    ==========================================================================
    Description:
	 Used for getting current band wireless information.

	 iwpriv ra0 set WirelessInfo = Choice

	 Choice
	 0: CentralFreq
	 1: Bw

    Return:
    ==========================================================================
*/
#define CEN_FREQ			0
#define SYS_BW				1
INT32 Get_System_Wireless_Info(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	UINT32 CapNode = 0;
	UINT16 CenFreq = 0;
	UINT8 Bw = 0;
	INT32 Choice;

	Choice = simple_strtol(arg, 0, 10);
	switch (Choice) {
	case CEN_FREQ:
		CapNode = Get_System_CapNode_Info(pAd);
		CenFreq = Get_System_CenFreq_Info(pAd, CapNode);
		break;

	case SYS_BW:
		CapNode = Get_System_CapNode_Info(pAd);
		Bw = Get_System_Bw_Info(pAd, CapNode);
		break;

	default:
		break;
	}

	return TRUE;
}
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

/*
    ==========================================================================
    Description:
	 Set IRR ADC parameters.

    Return:
    ==========================================================================
*/
INT Set_IRR_ADC(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	CHAR    *value = 0;
	UINT32  ChannelFreq = 0;
	UINT8   AntIndex = 0;
	UINT8   BW = 0;
	UINT8   SX = 0;
	UINT8   DbdcIdx = 0;
	UINT8   RunType = 0;
	UINT8   FType = 0;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			AntIndex = os_str_tol(value, 0, 10);

			switch (AntIndex) {
			case QA_IRR_WF0:
				AntIndex = WF0;
				break;

			case QA_IRR_WF1:
				AntIndex = WF1;
				break;

			case QA_IRR_WF2:
				AntIndex = WF2;
				break;

			case QA_IRR_WF3:
				AntIndex = WF3;
				break;
			}

			break;

		case 1:
			ChannelFreq = os_str_tol(value, 0, 10);
			break;

		case 2:
			BW = os_str_tol(value, 0, 10);
			break;

		case 3:
			SX = os_str_tol(value, 0, 10);
			break;

		case 4:
			DbdcIdx = os_str_tol(value, 0, 10);
			break;

		case 5:
			RunType = os_str_tol(value, 0, 10);
			break;

		case 6:
			FType = os_str_tol(value, 0, 10);
			break;

		default:
			break;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <SetADC> Input Checking Log\n\
					--------------------------------------------------------------\n\
					ChannelFreq = %d \n\
					AntIndex = %d \n\
					BW = %d \n\
					SX= %d \n\
					DbdcIdx = %d \n\
					RunType = %d \n\
					FType = %d \n\n", __func__, \
			 ChannelFreq, \
			 AntIndex, \
			 BW, \
			 SX, \
			 DbdcIdx, \
			 RunType, \
			 FType));
	MtCmdRfTestSetADC(pAd, ChannelFreq, AntIndex, BW, SX, DbdcIdx, RunType, FType);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	 Set IRR Rx Gain parameters.

    Return:
    ==========================================================================
*/
INT Set_IRR_RxGain(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	CHAR    *value = 0;
	UINT8   LPFG = 0;
	UINT8   LNA = 0;
	UINT8   DbdcIdx = 0;
	UINT8   AntIndex = 0;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			LPFG = os_str_tol(value, 0, 10);
			break;

		case 1:
			LNA = os_str_tol(value, 0, 10);
			break;

		case 2:
			DbdcIdx = os_str_tol(value, 0, 10);
			break;

		case 3:
			AntIndex = os_str_tol(value, 0, 10);

			switch (AntIndex) {
			case QA_IRR_WF0:
				AntIndex = WF0;
				break;

			case QA_IRR_WF1:
				AntIndex = WF1;
				break;

			case QA_IRR_WF2:
				AntIndex = WF2;
				break;

			case QA_IRR_WF3:
				AntIndex = WF3;
				break;
			}

			break;

		default:
			break;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <SetRxGain> Input Checking Log\n\
					--------------------------------------------------------------\n\
					LPFG = %d \n\
					LNA = %d \n\
					DbdcIdx = %d \n\
					AntIndex= %d \n\n", __func__, \
			 LPFG, \
			 LNA, \
			 DbdcIdx, \
			 AntIndex));
	MtCmdRfTestSetRxGain(pAd, LPFG, LNA, DbdcIdx, AntIndex);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	 Set IRR TTG parameters.

    Return:
    ==========================================================================
*/
INT Set_IRR_TTG(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	CHAR    *value = 0;
	UINT32  ChannelFreq = 0;
	UINT32  ToneFreq = 0;
	UINT8   TTGPwrIdx = 0;
	UINT8	XtalFreq = 0;
	UINT8   DbdcIdx = 0;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			TTGPwrIdx = os_str_tol(value, 0, 10);
			break;

		case 1:
			ToneFreq = os_str_tol(value, 0, 10);
			break;

		case 2:
			ChannelFreq = os_str_tol(value, 0, 10);
			break;

		case 3:
			XtalFreq = os_str_tol(value, 0, 10);
			break;

		case 4:
			DbdcIdx = os_str_tol(value, 0, 10);
			break;

		default:
			break;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <SetTTG> Input Checking Log\n\
					--------------------------------------------------------------\n\
					ChannelFreq = %d \n\
					ToneFreq = %d \n\
					TTGPwrIdx = %d \n\
					DbdcIdx= %d \n\n", __func__, \
			 ChannelFreq, \
			 ToneFreq, \
			 TTGPwrIdx, \
			 DbdcIdx));
	MtCmdRfTestSetTTG(pAd, ChannelFreq, ToneFreq, TTGPwrIdx, XtalFreq, DbdcIdx);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	 Set IRR TTGOnOff parameters.

    Return:
    ==========================================================================
*/
INT Set_IRR_TTGOnOff(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	CHAR    *value = 0;
	UINT8   TTGEnable = 0;
	UINT8   DbdcIdx = 0;
	UINT8   AntIndex = 0;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			TTGEnable = os_str_tol(value, 0, 10);
			break;

		case 1:
			DbdcIdx = os_str_tol(value, 0, 10);
			break;

		case 2:
			AntIndex = os_str_tol(value, 0, 10);

			switch (AntIndex) {
			case QA_IRR_WF0:
				AntIndex = WF0;
				break;

			case QA_IRR_WF1:
				AntIndex = WF1;
				break;

			case QA_IRR_WF2:
				AntIndex = WF2;
				break;

			case QA_IRR_WF3:
				AntIndex = WF3;
				break;
			}

			break;

		default:
			break;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <SetTTGOnOff> Input Checking Log\n\
					--------------------------------------------------------------\n\
					TTGEnable = %d \n\
					DbdcIdx = %d \n\
					AntIndex = %d \n\n", __func__, \
			 TTGEnable, \
			 DbdcIdx, \
			 AntIndex));
	MtCmdRfTestSetTTGOnOff(pAd, TTGEnable, DbdcIdx, AntIndex);
	return TRUE;
}

INT set_manual_protect(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *token;
	UINT32 wdev_idx = 0, mode = 0;

	if (arg == NULL)
		goto err1;

	if (arg != NULL) {
		token = strsep(&arg, "-");
		wdev_idx = os_str_tol(token, 0, 10);

		if (pAd->wdev_list[wdev_idx] == NULL)
			goto err2;
	}

	while (arg != NULL) {
		token = strsep(&arg, "+");

		if (!strcmp(token, "erp"))
			mode |= SET_PROTECT(ERP);
		else if (!strcmp(token, "no"))
			mode |= SET_PROTECT(NO_PROTECTION);
		else if (!strcmp(token, "non_member"))
			mode |= SET_PROTECT(NON_MEMBER_PROTECT);
		else if (!strcmp(token, "ht20"))
			mode |= SET_PROTECT(HT20_PROTECT);
		else if (!strcmp(token, "non_ht_mixmode"))
			mode |= SET_PROTECT(NON_HT_MIXMODE_PROTECT);
		else if (!strcmp(token, "longnav"))
			mode |= SET_PROTECT(LONG_NAV_PROTECT);
		else if (!strcmp(token, "gf"))
			mode |= SET_PROTECT(GREEN_FIELD_PROTECT);
		else if (!strcmp(token, "rifs"))
			mode |= SET_PROTECT(RIFS_PROTECT);
		else if (!strcmp(token, "rdg"))
			mode |= SET_PROTECT(RDG_PROTECT);
		else if (!strcmp(token, "force_rts"))
			mode |= SET_PROTECT(FORCE_RTS_PROTECT);
		else
			goto err3;
	}

	pAd->wdev_list[wdev_idx]->protection = mode;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" <<< manual trigger >>>\n HWFLAG_ID_UPDATE_PROTECT\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("   -- wdev_%d->protection: 0x%08x\n",
			  wdev_idx, pAd->wdev_list[wdev_idx]->protection));
	HwCtrlSetFlag(pAd, HWFLAG_ID_UPDATE_PROTECT);
	goto end;
err3:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 (" -no mode [ERROR 3]\n"));
	goto err1;
err2:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 (" -no wdev_idx: 0x%x [ERROR 2]\n", wdev_idx));
err1:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Usage:\niwpriv ra0 set protect=[wdev_idx]-[mode]+...\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("       mode: [erp|no|non_member|ht20|non_ht_mixmode|longnav|gf|rifs|rdg|force_rts]\n"));
end:
	return TRUE;
}

INT set_cca_en(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->set_cca_en)
		return chip_dbg->set_cca_en(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT show_timer_list(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMPShowTimerList(pAd);
	return TRUE;
}

INT show_wtbl_state(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	HcWtblRecDump(pAd);
	return TRUE;
}

/*
*
*/
UINT VIRTUAL_IF_INC(RTMP_ADAPTER *pAd)
{
	UINT cnt;
	ULONG flags = 0;

	OS_SPIN_LOCK_IRQSAVE(&pAd->VirtualIfLock, &flags);
	cnt = pAd->VirtualIfCnt++;
	OS_SPIN_UNLOCK_IRQRESTORE(&pAd->VirtualIfLock, &flags);
	return cnt;
}

/*
*
*/
UINT VIRTUAL_IF_DEC(RTMP_ADAPTER *pAd)
{
	UINT cnt;
	ULONG flags = 0;

	OS_SPIN_LOCK_IRQSAVE(&pAd->VirtualIfLock, &flags);
	cnt = pAd->VirtualIfCnt--;
	OS_SPIN_UNLOCK_IRQRESTORE(&pAd->VirtualIfLock, &flags);
	return cnt;
}

/*
*
*/
UINT VIRTUAL_IF_NUM(RTMP_ADAPTER *pAd)
{
	UINT cnt;

	cnt = pAd->VirtualIfCnt;
	return cnt;
}

INT Set_Rx_Vector_Control(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	BOOLEAN Enable = 1;
	UCHAR ucBandIdx = 0;
	UCHAR concurrent_bands = HcGetAmountOfBand(pAd);
	UINT8 i;
	/* obtain Band index */
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
	struct wifi_dev *wdev;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BandIdx = %d\n", __func__, ucBandIdx));
#endif /* CONFIG_AP_SUPPORT */

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));

	if (arg)
		Enable = os_str_tol(arg, 0, 10);

	Enable = (Enable == 0 ? 0 : 1);

	/* Turn off MibBucket */
	for (i = 0; i < concurrent_bands; i++)
		pAd->OneSecMibBucket.Enabled[i] = !Enable;
	pAd->MsMibBucket.Enabled = !Enable;

	/* Mac Enable*/
	AsicSetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, Enable);

	/* Rxv Enable*/
	AsicSetRxvFilter(pAd, Enable, ucBandIdx);

	if (Enable)
		pAd->parse_rxv_stat_enable = 1;
	else
		pAd->parse_rxv_stat_enable = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	return TRUE;
}

static VOID Parse_Rx_Rssi_CR(PRTMP_ADAPTER pAd, struct _RX_STATISTIC_CR *RxStat, INT type, UINT32 value)
{
	UINT32 IBRssi0 = 0, IBRssi1 = 0, WBRssi0 = 0, WBRssi1 = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: Value : %02x\n", __func__, value));

	if (IS_MT7615(pAd)) {
		IBRssi0 = (value & 0xFF000000) >> 24;
		if (IBRssi0 >= 128)
			IBRssi0 -= 256;
		WBRssi0 = (value & 0x00FF0000) >> 16;
		if (WBRssi0 >= 128)
			WBRssi0 -= 256;
		IBRssi1 = (value & 0x0000FF00) >> 8;
		if (IBRssi1 >= 128)
			IBRssi1 -= 256;
		WBRssi1 = (value & 0x000000FF);
		if (WBRssi1 >= 128)
			WBRssi1 -= 256;
	} else {
		IBRssi1 = (value & 0xFF000000) >> 24;
		if (IBRssi1 >= 128)
		    IBRssi1 -= 256;
		WBRssi1 = (value & 0x00FF0000) >> 16;
		if (WBRssi1 >= 128)
			WBRssi1 -= 256;
		IBRssi0 = (value & 0x0000FF00) >> 8;
		if (IBRssi0 >= 128)
			IBRssi0 -= 256;
		WBRssi0 = (value & 0x000000FF);
		if (WBRssi0 >= 128)
			WBRssi0 -= 256;
	}

	if (type == HQA_RX_STAT_RSSI || type == HQA_RX_STAT_RSSI_BAND1) {
		RxStat->Inst_IB_RSSSI[0] =  IBRssi0;
		RxStat->Inst_WB_RSSSI[0] =  WBRssi0;
		RxStat->Inst_IB_RSSSI[1] =  IBRssi1;
		RxStat->Inst_WB_RSSSI[1] =  WBRssi1;
	} else {
		RxStat->Inst_IB_RSSSI[2] =  IBRssi0;
		RxStat->Inst_WB_RSSSI[2] =  WBRssi0;
		RxStat->Inst_IB_RSSSI[3] =  IBRssi1;
		RxStat->Inst_WB_RSSSI[3] =  WBRssi1;
	}
}

INT Show_Rx_Statistic(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
#define MSG_LEN 2048
#define ENABLE 1
#define DISABLE 0
#define BAND0 0
#define BAND1 1
	RX_STATISTIC_RXV *rx_stat_rxv = &pAd->rx_stat_rxv;
	RX_STATISTIC_CR rx_stat_cr;
	UINT32 value = 0, i = 0, set = 1;
	UINT32 Status;
	UINT32 CurrBand0FCSErr, CurrBand0MDRDY;
	static UINT32 PreBand0FCSErr, PreBand0MDRDY;
#ifdef DBDC_MODE
	UINT32 CurrBand1FCSErr, CurrBand1MDRDY;
	static UINT32 PreBand1FCSErr, PreBand1MDRDY;
#endif/*DBDC_MODE*/
	RTMP_STRING *msg;
	UCHAR ucBandIdx = 0;

#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
	struct wifi_dev *wdev;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: BandIdx = %d\n", __func__, ucBandIdx));

	if (arg)
		set = os_str_tol(arg, 0, 10);

	set = (set == 0 ? 0 : 1);
	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);
	memset(msg, 0x00, MSG_LEN);
	sprintf(msg, "\n");

	switch (set) {
	case RESET_COUNTER:
		sprintf(msg + strlen(msg), "Reset counter !!\n");
#ifdef CONFIG_HW_HAL_OFFLOAD
		/*Disable PHY Counter*/
		MtCmdSetPhyCounter(pAd, DISABLE, BAND0);
#ifdef DBDC_MODE

		if (pAd->CommonCfg.dbdc_mode == TRUE)
			MtCmdSetPhyCounter(pAd, DISABLE, BAND1);

#endif /*DBDC_MODE*/
#endif/*CONFIG_HW_HAL_OFFLOAD*/
		PreBand0FCSErr = 0;
		PreBand0MDRDY = 0;
		pAd->AccuOneSecRxBand0FcsErrCnt = 0;
		pAd->AccuOneSecRxBand0MdrdyCnt = 0;
		pAd->AccuOneSecRxBand1FcsErrCnt = 0;
		pAd->AccuOneSecRxBand1MdrdyCnt = 0;
		break;

	case SHOW_RX_STATISTIC:
#ifdef CONFIG_HW_HAL_OFFLOAD
		/*Enable PHY Counter*/
		MtCmdSetPhyCounter(pAd, ENABLE, BAND0);
#ifdef DBDC_MODE

		if (pAd->CommonCfg.dbdc_mode == TRUE)
			MtCmdSetPhyCounter(pAd, ENABLE, BAND1);

#endif /*DBDC_MODE*/
#endif/*CONFIG_HW_HAL_OFFLOAD*/

		/*Band0 PHY Counter */
		value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PHY_FCSERRCNT);
		rx_stat_cr.FCSErr_OFDM = (value >> 16);
		rx_stat_cr.FCSErr_CCK = (value & 0xFFFF);
		value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PD);
		rx_stat_cr.OFDM_PD = (value >> 16);
		rx_stat_cr.CCK_PD = (value & 0xFFFF);
		value = MtAsicGetRxStat(pAd, HQA_RX_STAT_CCK_SIG_SFD);
		rx_stat_cr.CCK_SIG_Err = (value >> 16);
		rx_stat_cr.CCK_SFD_Err = (value & 0xFFFF);
		value = MtAsicGetRxStat(pAd, HQA_RX_STAT_OFDM_SIG_TAG);
		rx_stat_cr.OFDM_SIG_Err = (value >> 16);
		rx_stat_cr.OFDM_TAG_Err = (value & 0xFFFF);

#if defined(MT7626)
		if (ucBandIdx == BAND1 && IS_MT7626(pAd)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Read BandIdx = %d, RSSI, ACI hit\n", __func__, ucBandIdx));
			/*IBRSSI0 WBRSSI0 IBRSSI1 WBRSSI1*/
			value = MtAsicGetRxStat(pAd, HQA_RX_STAT_RSSI_BAND1);
			Parse_Rx_Rssi_CR(pAd, &rx_stat_cr, HQA_RX_STAT_RSSI_BAND1, value);

			/*IBRSSI2 WBRSSI2 IBRSSI3 WBRSSI3*/
			value = MtAsicGetRxStat(pAd, HQA_RX_STAT_RSSI_RX23_BAND1);
			Parse_Rx_Rssi_CR(pAd, &rx_stat_cr, HQA_RX_STAT_RSSI_RX23_BAND1, value);

			value = MtAsicGetRxStat(pAd, HQA_RX_STAT_ACI_HITL_BAND1);
			rx_stat_cr.ACIHitLow = ((value >> 18) & 0x1);
			value = MtAsicGetRxStat(pAd, HQA_RX_STAT_ACI_HITH_BAND1);
			rx_stat_cr.ACIHitHigh = ((value >> 18) & 0x1);
		} else
#endif /* defined(MT7626) */
		{
			/*IBRSSI0 WBRSSI0 IBRSSI1 WBRSSI1*/
			value = MtAsicGetRxStat(pAd, HQA_RX_STAT_RSSI);
			Parse_Rx_Rssi_CR(pAd, &rx_stat_cr, HQA_RX_STAT_RSSI, value);

			/*IBRSSI2 WBRSSI2 IBRSSI3 WBRSSI3*/
			value = MtAsicGetRxStat(pAd, HQA_RX_STAT_RSSI_RX23);
			Parse_Rx_Rssi_CR(pAd, &rx_stat_cr, HQA_RX_STAT_RSSI_RX23, value);

			value = MtAsicGetRxStat(pAd, HQA_RX_STAT_ACI_HITL);
			rx_stat_cr.ACIHitLow = ((value >> 18) & 0x1);
			value = MtAsicGetRxStat(pAd, HQA_RX_STAT_ACI_HITH);
			rx_stat_cr.ACIHitHigh = ((value >> 18) & 0x1);
		}

		value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PHY_MDRDYCNT);
		rx_stat_cr.PhyMdrdyOFDM = (value >> 16);
		if (IS_MT7622(pAd)) {
			/* HW issue and SW workaround */
			rx_stat_cr.PhyMdrdyCCK =  ((value & 0xFFFF)/2);
		} else
			rx_stat_cr.PhyMdrdyCCK =  (value & 0xFFFF);

		/*Band0 MAC Counter*/
		CurrBand0FCSErr = pAd->AccuOneSecRxBand0FcsErrCnt;
		rx_stat_cr.RxMacFCSErrCount = CurrBand0FCSErr - PreBand0FCSErr;
		PreBand0FCSErr = CurrBand0FCSErr;
		CurrBand0MDRDY = pAd->AccuOneSecRxBand0MdrdyCnt;
		rx_stat_cr.RxMacMdrdyCount = CurrBand0MDRDY - PreBand0MDRDY;
		PreBand0MDRDY = CurrBand0MDRDY;
		rx_stat_cr.RxMacFCSOKCount = rx_stat_cr.RxMacMdrdyCount - rx_stat_cr.RxMacFCSErrCount;

		sprintf(msg + strlen(msg), "\x1b[41m%s : \x1b[m\n", __func__);
		sprintf(msg + strlen(msg), "FreqOffsetFromRx   = %d\n", rx_stat_rxv->FreqOffsetFromRx);

		for (i = 0; i < 4; i++)
			sprintf(msg + strlen(msg), "RCPI_%d             = %d\n", i, rx_stat_rxv->RCPI[i]);

		for (i = 0; i < 4; i++)
			sprintf(msg + strlen(msg), "FAGC_RSSI_IB_%d     = %d\n",  i, rx_stat_rxv->FAGC_RSSI_IB[i]);

		for (i = 0; i < 4; i++)
			sprintf(msg + strlen(msg), "FAGC_RSSI_WB_%d     = %d\n",  i, rx_stat_rxv->FAGC_RSSI_WB[i]);

		for (i = 0; i < 4; i++)
			sprintf(msg + strlen(msg), "Inst_IB_RSSI_%d     = %d\n",  i, rx_stat_cr.Inst_IB_RSSSI[i]);

		for (i = 0; i < 4; i++)
			sprintf(msg + strlen(msg), "Inst_WB_RSSI_%d     = %d\n",  i, rx_stat_cr.Inst_WB_RSSSI[i]);

		sprintf(msg + strlen(msg), "SNR                = %d\n",  rx_stat_rxv->SNR[0]);
		sprintf(msg + strlen(msg), "ACIHitHigh         = %u\n",  rx_stat_cr.ACIHitHigh);
		sprintf(msg + strlen(msg), "ACIHitLow          = %u\n",  rx_stat_cr.ACIHitLow);
		sprintf(msg + strlen(msg), "\x1b[41mFor Band0Index : \x1b[m\n");
		sprintf(msg + strlen(msg), "MacMdrdyCount      = %u\n",  rx_stat_cr.RxMacMdrdyCount);
		sprintf(msg + strlen(msg), "MacFCSErrCount     = %u\n",  rx_stat_cr.RxMacFCSErrCount);
		sprintf(msg + strlen(msg), "MacFCSOKCount      = %u\n",  rx_stat_cr.RxMacFCSOKCount);
		sprintf(msg + strlen(msg), "CCK_PD             = %u\n",  rx_stat_cr.CCK_PD);
		sprintf(msg + strlen(msg), "CCK_SFD_Err        = %u\n",  rx_stat_cr.CCK_SFD_Err);
		sprintf(msg + strlen(msg), "CCK_SIG_Err        = %u\n",  rx_stat_cr.CCK_SIG_Err);
		sprintf(msg + strlen(msg), "CCK_FCS_Err        = %u\n",  rx_stat_cr.FCSErr_CCK);
		sprintf(msg + strlen(msg), "OFDM_PD            = %u\n",  rx_stat_cr.OFDM_PD);
		sprintf(msg + strlen(msg), "OFDM_SIG_Err       = %u\n",  rx_stat_cr.OFDM_SIG_Err);
		sprintf(msg + strlen(msg), "OFDM_FCS_Err       = %u\n",  rx_stat_cr.FCSErr_OFDM);
#ifdef DBDC_MODE

		if (pAd->CommonCfg.dbdc_mode == TRUE) {
			/*Band1 MAC Counter*/
			CurrBand1FCSErr = pAd->AccuOneSecRxBand1FcsErrCnt;
			rx_stat_cr.RxMacFCSErrCount_band1 = CurrBand1FCSErr - PreBand1FCSErr;
			PreBand1FCSErr = CurrBand1FCSErr;
			CurrBand1MDRDY = pAd->AccuOneSecRxBand1MdrdyCnt;
			rx_stat_cr.RxMacMdrdyCount_band1 = CurrBand1MDRDY - PreBand1MDRDY;
			PreBand1MDRDY = CurrBand1MDRDY;
			rx_stat_cr.RxMacFCSOKCount_band1 = rx_stat_cr.RxMacMdrdyCount_band1 - rx_stat_cr.RxMacFCSErrCount_band1;
			/*Band1 PHY Counter*/
			value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PHY_MDRDYCNT_BAND1);
			rx_stat_cr.PhyMdrdyOFDM_band1 = (value >> 16);
			rx_stat_cr.PhyMdrdyCCK_band1 = (value & 0xFFFF);
			value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PD_BAND1);
			rx_stat_cr.OFDM_PD_band1 = (value >> 16);
			rx_stat_cr.CCK_PD_band1 = (value & 0xFFFF);
			value = MtAsicGetRxStat(pAd, HQA_RX_STAT_CCK_SIG_SFD_BAND1);
			rx_stat_cr.CCK_SIG_Err_band1 = (value >> 16);
			rx_stat_cr.CCK_SFD_Err_band1 = (value & 0xFFFF);
			value = MtAsicGetRxStat(pAd, HQA_RX_STAT_OFDM_SIG_TAG_BAND1);
			rx_stat_cr.OFDM_SIG_Err_band1 = (value >> 16);
			rx_stat_cr.OFDM_TAG_Err_band1 = (value & 0xFFFF);
			sprintf(msg + strlen(msg), "\x1b[41mFor Band1Index : \x1b[m\n");
			sprintf(msg + strlen(msg), "MacMdrdyCount      = %u\n",  rx_stat_cr.RxMacMdrdyCount_band1);
			sprintf(msg + strlen(msg), "MacFCSErrCount     = %u\n",  rx_stat_cr.RxMacFCSErrCount_band1);
			sprintf(msg + strlen(msg), "MacFCSOKCount      = %u\n",  rx_stat_cr.RxMacFCSOKCount_band1);
			sprintf(msg + strlen(msg), "CCK_PD             = %u\n",  rx_stat_cr.CCK_PD_band1);
			sprintf(msg + strlen(msg), "CCK_SFD_Err        = %u\n",  rx_stat_cr.CCK_SFD_Err_band1);
			sprintf(msg + strlen(msg), "CCK_SIG_Err        = %u\n",  rx_stat_cr.CCK_SIG_Err_band1);
			sprintf(msg + strlen(msg), "OFDM_PD            = %u\n",  rx_stat_cr.OFDM_PD_band1);
			sprintf(msg + strlen(msg), "OFDM_SIG_Err       = %u\n",  rx_stat_cr.OFDM_SIG_Err_band1);
		}

#endif/*DBDC_MODE*/
		break;
	}

	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	return TRUE;
}

#ifdef SMART_CARRIER_SENSE_SUPPORT
INT Show_SCSinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i = 0;
	UCHAR       concurrent_bands = HcGetAmountOfBand(pAd);

	if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7663(pAd) || IS_MT7626(pAd)) {
		for (i = 0; i < concurrent_bands; i++) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("************** Bnad%d  Information*************\n", i));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d SCSEnable = %d\n", i, pAd->SCSCtrl.SCSEnable[i]));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d SCSStatus = %d\n", i, pAd->SCSCtrl.SCSStatus[i]));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d SCSMinRssi = %d\n", i, pAd->SCSCtrl.SCSMinRssi[i]));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d CckPdBlkTh = %d (%ddBm)\n", i, pAd->SCSCtrl.CckPdBlkTh[i], (pAd->SCSCtrl.CckPdBlkTh[i] - 256)));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d OfdmPdBlkTh = %d(%ddBm)\n", i, pAd->SCSCtrl.OfdmPdBlkTh[i], (pAd->SCSCtrl.OfdmPdBlkTh[i] - 512) / 2));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d Traffic TH = %d\n", i, pAd->SCSCtrl.SCSTrafficThreshold[i]));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d MinRssiTolerance = %d\n", i, pAd->SCSCtrl.SCSMinRssiTolerance[i]));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d SCSThTolerance = %d\n", i, pAd->SCSCtrl.SCSThTolerance[i]));
			/* MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, */
			/* (" Bnad%d OFDM Support = %d\n", i, pAd->SCSCtrl.OfdmPdSupport[i])); */
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d One sec TxByte = %d\n", i, pAd->SCSCtrl.OneSecTxByteCount[i]));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d One sec RxByte = %d\n", i, pAd->SCSCtrl.OneSecRxByteCount[i]));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d RTS count = %d\n", i, pAd->SCSCtrl.RtsCount[i]));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d RTS retry count = %d\n", i, pAd->SCSCtrl.RtsRtyCount[i]));

			/* SCS_Gen3 support RTS Drop Count at Band0 */
			if (pAd->SCSCtrl.SCSGeneration == SCS_Gen3 && i == 0) {
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 (" Bnad0 RTS   MPDU drop count = %d\n", pAd->SCSCtrl.RTS_MPDU_DROP_CNT));
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 (" Bnad0 Retry MPDU drop count = %d\n", pAd->SCSCtrl.Retry_MPDU_DROP_CNT));
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 (" Bnad0 LTO   MPDU drop count = %d\n", pAd->SCSCtrl.LTO_MPDU_DROP_CNT));
			}

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("=========CCK=============\n"));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d CCK false-CCA= %d\n", i, pAd->SCSCtrl.CckFalseCcaCount[i]));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d CCK false-CCA up bond= %d\n", i, pAd->SCSCtrl.CckFalseCcaUpBond[i]));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d CCK false-CCA low bond= %d\n", i, pAd->SCSCtrl.CckFalseCcaLowBond[i]));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d CCK fixed RSSI boundary= %d (%ddBm)\n", i, pAd->SCSCtrl.CckFixedRssiBond[i],
					  (pAd->SCSCtrl.CckFixedRssiBond[i] - 256)));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("=========OFDM=============\n"));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d OFDM false-CCA= %d\n", i, pAd->SCSCtrl.OfdmFalseCcaCount[i]));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d OFDM false-CCA up bond= %d\n", i, pAd->SCSCtrl.OfdmFalseCcaUpBond[i]));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d OFDM false-CCA low bond= %d\n", i, pAd->SCSCtrl.OfdmFalseCcaLowBond[i]));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 (" Bnad%d OFDM fixed RSSI boundary= %d(%ddBm)\n", i, pAd->SCSCtrl.OfdmFixedRssiBond[i],
					  (pAd->SCSCtrl.OfdmFixedRssiBond[i] - 512) / 2));
		}
	}

	return TRUE;
}
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
INT	Set_Led_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR thisChar;
	long led_param[8];
	INT i = 0, j = 0;

	printk("\n %s ==> arg = %s\n", __func__, arg);
	memset(led_param, 0, sizeof(long) * 8);

	while ((thisChar = strsep((char **)&arg, "-")) != NULL) {
		led_param[i] = os_str_tol(thisChar, 0, 10);
		i++;

		if (i >= 8)
			break;
	}

	printk("\n%s\n", __func__);

	for (j = 0; j < i; j++)
		printk("%02x\n", (UINT)led_param[j]);

#if defined(MT7615) || defined(MT7663) || defined(MT7626)
	AndesLedEnhanceOP(pAd, led_param[0], led_param[1], led_param[2], led_param[3], led_param[4], led_param[5], led_param[6],
					  led_param[7]);
#endif
	return TRUE;
}
#endif

#ifdef SINGLE_SKU_V2
INT TxPowerSKUCtrl(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN fgTxPowerSKUEn,
	IN UCHAR ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxPowerSKUCtrl(pAd, fgTxPowerSKUEn, ucBandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TxPowerBfBackoffCtrl(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN fgTxBFBackoffEn,
	IN UCHAR ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxBfBackoffCtrl(pAd, fgTxBFBackoffEn, ucBandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}
#endif /* SINGLE_SKU_V2 */

#ifdef TXPWRMANUAL
INT TxPowerManualCtrl(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN fgPwrManCtrl,
	IN UINT8   u1TxPwrModeManual,
	IN UINT8   u1TxPwrBwManual,
	IN UINT8   u1TxPwrRateManual,
	IN INT8    i1TxPwrValueManual,
	IN UCHAR   ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxPowerManualCtrl(pAd, fgPwrManCtrl, u1TxPwrModeManual, u1TxPwrBwManual, u1TxPwrRateManual, i1TxPwrValueManual, ucBandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}
#endif /* TXPWRMANUAL */

INT TxPowerPercentCtrl(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN fgTxPowerPercentEn,
	IN UCHAR ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxPowerPercentCtrl(pAd, fgTxPowerPercentEn, ucBandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TxPowerDropCtrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 ucPowerDrop,
	IN UCHAR ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxPowerDropCtrl(pAd, ucPowerDrop, ucBandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TxCCKStreamCtrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 u1CCKTxStream,
	IN UCHAR ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("u1CCKTxStream = %d \n", u1CCKTxStream));

	/* Work around - profile setting*/
	if (!u1CCKTxStream)
		u1CCKTxStream = 1;

	/* sanity check for input parameter range */
	if (u1CCKTxStream >= WF_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
		return FALSE;
	}

	/* Only for 7622 */
	if (IS_MT7622(pAd)) {
		if (MtCmdTxCCKStream(pAd, u1CCKTxStream, ucBandIdx) == RETURN_STATUS_TRUE)
			fgStatus = TRUE;
	}

	return fgStatus;
}

INT ThermoCompCtrl(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN fgThermoCompEn,
	IN UCHAR ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdThermoCompCtrl(pAd, fgThermoCompEn, ucBandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TxPowerRfTxAnt(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 ucTxAntIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxPwrRfTxAntCtrl(pAd, ucTxAntIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TxPowerShowInfo(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR            ucTxPowerInfoCatg,
	IN UINT8            ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxPwrShowInfo(pAd, ucTxPowerInfoCatg, ucBandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

#ifdef WIFI_EAP_FEATURE
INT SetEdccaThreshold(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 edcca_threshold,
	IN UINT8 BandIdx
)
{
	INT status = 0;

	if (RETURN_STATUS_TRUE == MtCmdSetEdccaThreshold(pAd, edcca_threshold, BandIdx))
		status = 1;

	return status;
}

INT InitIPICtrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 BandIdx
)
{
	INT status = 0;

	if (RETURN_STATUS_TRUE == MtCmdInitIPICtrl(pAd, BandIdx))
		status = 1;

	return status;
}

INT GetIPIValue(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 BandIdx
)
{
	INT status = 0;

	if (RETURN_STATUS_TRUE == MtCmdGetIPIValue(pAd, BandIdx))
		status = 1;

	return status;
}

INT SetDataTxPwrOffset(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 WlanIdx,
	IN INT8 TxPwr_Offset,
	IN UINT8 BandIdx
)
{
	INT status = 0;

	if (RETURN_STATUS_TRUE == MtCmdSetDataTxPwrOffset(pAd, WlanIdx, TxPwr_Offset, BandIdx))
		status = 1;

	return status;
}

INT SetFwRaTable(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 BandIdx,
	IN UINT8 TblType,
	IN UINT8 TblIndex,
	IN UINT16 TblLength,
	PUCHAR Buffer
)
{
	INT status = 0;

	if (MtCmdSetRaTable(pAd, BandIdx, TblType, TblIndex, TblLength, Buffer) == RETURN_STATUS_TRUE)
		status = 1;

	return status;
}

INT GetRaTblInfo(
	PRTMP_ADAPTER pAd,
	UINT8 BandIdx,
	UINT8 TblType,
	UINT8 TblIndex,
	UINT8 ReadnWrite
)
{
	INT status = 0;

	if (MtCmdGetRaTblInfo(pAd, BandIdx, TblType, TblIndex, ReadnWrite) == RETURN_STATUS_TRUE)
		status = 1;

	return status;
}
#endif /* WIFI_EAP_FEATURE */

#ifdef WIFI_GPIO_CTRL
INT SetGpioCtrl(PRTMP_ADAPTER pAd, UINT8 GpioIdx, BOOLEAN GpioEn)
{
	INT status = 0;

	if (MtCmdSetGpioCtrl(pAd, GpioIdx, GpioEn) == RETURN_STATUS_TRUE)
		status = 1;

	return status;
}

INT SetGpioValue(PRTMP_ADAPTER pAd, UINT8 GpioIdx, UINT8 GpioVal)
{
	INT status = 0;

	if (MtCmdSetGpioVal(pAd, GpioIdx, GpioVal) == RETURN_STATUS_TRUE)
		status = 1;

	return status;
}
#endif /* WIFI_GPIO_CTRL */

INT TOAECtrlCmd(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR                TOAECtrl
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTOAECalCtrl(pAd, TOAECtrl) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT EDCCACtrlCmd(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR            ucBandIdx,
	IN UCHAR            EDCCACtrl
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdEDCCACtrl(pAd, ucBandIdx, EDCCACtrl) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT MuPwrCtrlCmd(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN fgMuTxPwrManEn,
	IN CHAR cMuTxPwr,
	IN UINT8 u1BandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdMuPwrCtrl(pAd, fgMuTxPwrManEn, cMuTxPwr, u1BandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT BFNDPATxDCtrlCmd(
	IN PRTMP_ADAPTER        pAd,
	IN BOOLEAN              fgNDPA_ManualMode,
	IN UINT8                ucNDPA_TxMode,
	IN UINT8                ucNDPA_Rate,
	IN UINT8                ucNDPA_BW,
	IN UINT8                ucNDPA_PowerOffset
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdBFNDPATxDCtrl(pAd, fgNDPA_ManualMode, ucNDPA_TxMode, ucNDPA_Rate, ucNDPA_BW,
						   ucNDPA_PowerOffset) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TemperatureCtrl(
	IN PRTMP_ADAPTER	pAd,
	IN BOOLEAN              fgManualMode,
	IN CHAR		cTemperature
)
{
	BOOLEAN  fgStatus = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand;

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wdev is invalid\n", __func__));
		return FALSE;
	}

	ucBand = HcGetBandByWdev(wdev);

	if (MtCmdTemperatureCtrl(pAd, fgManualMode, ucBand, cTemperature) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT ThermalItemInfo(
	IN PRTMP_ADAPTER	pAd
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdThermalItemInfo(pAd) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

#ifdef TX_POWER_CONTROL_SUPPORT
INT TxPwrUpCtrl(
	IN PRTMP_ADAPTER   pAd,
	IN UINT8           ucBandIdx,
	IN CHAR            cPwrUpCat,
	IN CHAR            cPwrUpValue[POWER_UP_CATEGORY_RATE_NUM]
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxPwrUpCtrl(pAd, ucBandIdx, cPwrUpCat, cPwrUpValue) ==
			RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}
#endif /* TX_POWER_CONTROL_SUPPORT */

/* [channel_band] 0: 2.4G, 1: 5G*/
UINT8 TxPowerGetChBand(
	IN UINT8				ucBandIdx,
	IN UINT8				CentralCh
)
{
	UINT8	ChannelBand = 0;

	if (CentralCh >= 14)
		ChannelBand = 1;
	else {
		if (ucBandIdx == 0)
			ChannelBand = 0;
		else
			ChannelBand = 1;
	}

	return ChannelBand;
}

#ifdef TPC_SUPPORT
INT TxPowerTpcFeatureCtrl(
	IN PRTMP_ADAPTER	pAd,
	IN struct wifi_dev		*wdev,
	IN INT8					TpcPowerValue
)
{
	BOOLEAN  fgStatus = FALSE;
	return fgStatus;
}

INT TxPowerTpcFeatureForceCtrl(
	IN PRTMP_ADAPTER	pAd,
	IN INT8					TpcPowerValue,
	IN UINT8				ucBandIdx,
	IN UINT8				CentralChannel
)
{
	BOOLEAN  fgStatus = FALSE;
	return fgStatus;
}
#endif /* TPC_SUPPORT */

VOID cp_support_is_enabled(PRTMP_ADAPTER pAd)
{
	if ((pAd->cp_support >= 1) && (pAd->cp_support <= 3)) {
		if (pAd->cp_have_cr4 == TRUE) {
			MtCmdSetCPSEnable(pAd, HOST2CR4, pAd->cp_support);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set CR4 CP_SUPPORT to Mode %d.\n", __func__, pAd->cp_support));
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set Driver CP_SUPPORT to Mode %d.\n", __func__, pAd->cp_support));
		}
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("The CP Mode is invaild. Mode should be 1~3.\n"));
	}
}

INT set_cp_support_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	UINT32 rv, Mode;

	if (arg) {
		rv = sscanf(arg, "%d", &Mode);

		if ((rv > 0) && (Mode >= 1) && (Mode <= 3)) {
			pAd->cp_support = Mode;

			if (pAd->cp_have_cr4 == TRUE) {
				MtCmdSetCPSEnable(pAd, HOST2CR4, Mode);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set CR4 CP_SUPPORT to Mode %d.\n", __func__, Mode));
			} else {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set Driver CP_SUPPORT to Mode %d.\n", __func__, Mode));
			}
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("The Mode is invaild. Mode should be 1~3.\n"));
			return FALSE;
		}
	} else
		return FALSE;

	return TRUE;
}
#ifdef WIFI_EAP_FEATURE
#ifdef CHUTIL_SUPPORT
INT Show_ChUtil_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#if defined(MT7615) || defined(MT7622) || defined(MT7663) || defined(MT7626)
       INT i = 0;
       UCHAR       concurrent_bands = HcGetAmountOfBand(pAd);

       if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7663(pAd) || IS_MT7626(pAd)) {
		for (i = 0; i < concurrent_bands; i++) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("====Bnad%d Enable = %d====\n", i, pAd->OneSecMibBucket.Enabled[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Channel Busy Time = %d\n", pAd->OneSecMibBucket.ChannelBusyTimeCcaNavTx[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("My Tx Air Time = %d\n", pAd->OneSecMibBucket.MyTxAirtime[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("My Rx Air Time = %d\n", pAd->OneSecMibBucket.MyRxAirtime[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("OBSS Air Time = %d\n", pAd->OneSecMibBucket.OBSSAirtime[i]));
		}
	}
#endif
	return TRUE;
}
#endif
#endif
INT Show_MibBucket_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#if defined(MT7615) || defined(MT7622) || defined(MT7663) || defined(MT7626)
	INT i = 0;
	UCHAR       concurrent_bands = HcGetAmountOfBand(pAd);

	if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7663(pAd) || IS_MT7626(pAd)) {
		for (i = 0; i < concurrent_bands; i++) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("====Bnad%d Enable = %d====\n", i, pAd->OneSecMibBucket.Enabled[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Channel Busy Time = %d\n", pAd->OneSecMibBucket.ChannelBusyTimeCcaNavTx[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Primary Channel Busy Time = %d\n", pAd->OneSecMibBucket.ChannelBusyTime[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("OBSS Air Time = %d\n", pAd->OneSecMibBucket.OBSSAirtime[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("My Tx Air Time = %d\n", pAd->OneSecMibBucket.MyTxAirtime[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("My Rx Air Time = %d\n", pAd->OneSecMibBucket.MyRxAirtime[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("EDCCA Time = %d\n", pAd->OneSecMibBucket.EDCCAtime[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("PD count = %x\n", pAd->OneSecMibBucket.PdCount[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("MDRDY Count = %x\n", pAd->OneSecMibBucket.MdrdyCount[i]));
		}
	}

#endif
	return TRUE;
}

#ifdef GN_MIXMODE_SUPPORT
VOID gn_mixmode_is_enable(PRTMP_ADAPTER pAd)
{
	if (pAd->CommonCfg.GNMixMode) {
		MtCmdSetGNMixModeEnable(pAd, HOST2CR4, pAd->CommonCfg.GNMixMode);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: set CR4/N9 GN MIXMODE Enable to %d.\n", __func__, pAd->red_en));
	}
}
#endif /* GN_MIXMODE_SUPPORT */

#ifdef DHCP_UC_SUPPORT
static UINT32  checksum(PUCHAR buf, INT32  nbytes, UINT32 sum)
{
	UINT32 i;

	/* Checksum all the pairs of bytes first... */
	for (i = 0; i < (nbytes & ~1U); i += 2) {
		sum += (UINT16)ntohs(*((u_int16_t *)(buf + i)));

		if (sum > 0xFFFF)
			sum -= 0xFFFF;
	}

	/*
	 * If there's a single byte left over, checksum it, too.
	 * Network byte order is big-endian, so the remaining byte is
	 * the high byte.
	 */
	if (i < nbytes) {
		sum += buf[i] << 8;

		if (sum > 0xFFFF)
			sum -= 0xFFFF;
	}

	return sum;
}

static UINT32 wrapsum(UINT32 sum)
{
	UINT32 ret;

	sum = ~sum & 0xFFFF;
	ret = htons(sum);
	return ret;
}
UINT16 RTMP_UDP_Checksum(IN PNDIS_PACKET pSkb)
{
	PUCHAR pPktHdr, pLayerHdr;
	PUCHAR pPseudo_Hdr;
	PUCHAR pPayload_Hdr;
	PUCHAR pUdpHdr;
	UINT16 udp_chksum;
	UINT16 udp_len;
	UINT16 payload_len;

	pPktHdr = GET_OS_PKT_DATAPTR(pSkb);

	if (IS_VLAN_PACKET(GET_OS_PKT_DATAPTR(pSkb)))
		pLayerHdr = (pPktHdr + MAT_VLAN_ETH_HDR_LEN);
	else
		pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);

	pUdpHdr = pLayerHdr + 20;
	pPseudo_Hdr = pUdpHdr - 8;
	pPayload_Hdr = pUdpHdr + 8;
	udp_chksum = (*((UINT16 *) (pUdpHdr + 6)));
	udp_len = ntohs(*((UINT16 *) (pUdpHdr + 4)));
	payload_len = udp_len - 8;
	udp_chksum = wrapsum(
					 checksum(
						 pUdpHdr,
						 8,
						 checksum(
							 pPayload_Hdr,
							 payload_len,
							 checksum(
								 (unsigned char *)pPseudo_Hdr,
								 2 * 4,
								 17 + udp_len)))
				 );
	return udp_chksum;
}
#endif /* DHCP_UC_SUPPORT */

#ifdef ERR_RECOVERY
INT32 ShowSerProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E R , stat=0x%08X\n",
			 __func__, ErrRecoveryCurStat(&pAd->ErrRecoveryCtl)));
	/* Dump SER related CRs */
	RTMPHandleInterruptSerDump(pAd);
	/* print out ser log timing */
	SerTimeLogDump(pAd);
	return TRUE;
}

INT32 ShowSerProc2(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	ShowSerProc(pAd, arg);

#ifdef DUMMY_N9_HEART_BEAT
	if (1) {
		UINT32 reg_tmp_val;

		MAC_IO_READ32(pAd->hdev_ctrl, DUMMY_N9_HEART_BEAT, &reg_tmp_val);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HeartBeat 0x%x = %d\n",
			DUMMY_N9_HEART_BEAT, reg_tmp_val));
	}
#endif

	/* We will get more info from FW */
	CmdExtSER(pAd, SER_ACTION_QUERY, 0);

	return TRUE;
}

#endif

#ifdef MT7626
INT32 ShowCCKCountProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pAd->FixRateErr1 = %d\n", pAd->FixRateErr1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pAd->FixRateErr2 = %d\n", pAd->FixRateErr2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pAd->FixRateErr3 = %d\n", pAd->FixRateErr3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pAd->FixRateErr4 = %d\n", pAd->FixRateErr4));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pAd->FixRateErr5 = %d\n", pAd->FixRateErr5));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pAd->FixRateErr6 = %d\n", pAd->FixRateErr6));
	return TRUE;
}
#endif

INT32 ShowBcnProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef CONFIG_AP_SUPPORT
	UINT32 band_idx = 0;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	for (band_idx = 0; band_idx < 2; band_idx++) {
		if (arg != NULL && band_idx != os_str_toul(arg, 0, 10))
			continue;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, Band %d\n", __func__, band_idx));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================\n"));
		if (chip_dbg->show_bcn_info)
			chip_dbg->show_bcn_info(pAd->hdev_ctrl, band_idx);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================\n"));
#endif
	return TRUE;
}

#ifdef TX_POWER_CONTROL_SUPPORT
#if defined(MT7626) || defined(AXE) || defined(MT7915)
INT32 ShowTxPowerBoostInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR	*value = 0;
	INT status = TRUE;
	CHAR	cPwrUpCat = 0;
	CHAR	cPwrUpValue[POWER_UP_CATEGORY_RATE_NUM] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR	 apidx = pObj->ioctl_if;
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
INT32 ShowTxPowerBoostInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_8 ucRateIdx;
	UINT_8 ucBandIdx;
	struct	wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;
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

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Invalid Band Index!!!\n"));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("=======================================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Power Up Table (Band%d)\n", ucBandIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("=======================================================\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CCK\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(M0M1)-(M2M3)\n"));
	for (ucRateIdx = 0; ucRateIdx < RATE_POWER_CCK_NUM; ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ",
			pAd->CommonCfg.cPowerUpCckOfdm[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("OFDM\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("(M0M1)-(M2M3)-(M4M5)-(M6  )-(M7  )\n"));
	for (ucRateIdx = RATE_POWER_CCK_NUM;
			ucRateIdx < (RATE_POWER_CCK_NUM + RATE_POWER_OFDM_NUM);
			ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ",
			pAd->CommonCfg.cPowerUpCckOfdm[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT20\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("(M0  )-(M32 )-(M1M2)-(M3M4)-(M5  )-(M6  )-(M7  )\n"));
	for (ucRateIdx = 0; ucRateIdx < RATE_POWER_HT20_NUM; ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ",
			pAd->CommonCfg.cPowerUpHt20[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT40\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("(M0  )-(M32)- (M1M2)-(M3M4)-(M5  )-(M6  )-(M7  )\n"));
	for (ucRateIdx = 0; ucRateIdx < RATE_POWER_HT40_NUM; ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ",
			pAd->CommonCfg.cPowerUpHt40[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT20\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("(M0  )-(M1M2)-(M3M4)-(M5M6)-(M7  )-(M8  )-(M9  )\n"));
	for (ucRateIdx = 0; ucRateIdx < RATE_POWER_VHT20_NUM; ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ",
			pAd->CommonCfg.cPowerUpVht20[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT40\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("(M0  )-(M1M2)-(M3M4)-(M5M6)-(M7  )-(M8  )-(M9  )\n"));
	for (ucRateIdx = 0; ucRateIdx < RATE_POWER_VHT40_NUM; ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ",
			pAd->CommonCfg.cPowerUpVht40[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT80\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("(M0  )-(M1M2)-(M3M4)-(M5M6)-(M7  )-(M8  )-(M9  )\n"));
	for (ucRateIdx = 0; ucRateIdx < RATE_POWER_VHT80_NUM; ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ",
			pAd->CommonCfg.cPowerUpVht80[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT160\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("(M0  )-(M1M2)-(M3M4)-(M5M6)-(M7  )-(M8  )-(M9  )\n"));
	for (ucRateIdx = 0; ucRateIdx < RATE_POWER_VHT160_NUM; ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ",
			pAd->CommonCfg.cPowerUpVht160[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	return TRUE;
}
#endif
#endif /* TX_POWER_CONTROL_SUPPORT */

#ifdef ETSI_RX_BLOCKER_SUPPORT
INT32 ShowRssiThInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: c1RWbRssiHTh: %d, c1RWbRssiLTh: %d, c1RIbRssiLTh: %d,c1WBRssiTh4R: %d\n", __FUNCTION__,
															pAd->c1RWbRssiHTh, pAd->c1RWbRssiLTh, pAd->c1RIbRssiLTh, pAd->c1WBRssiTh4R));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	return TRUE;
}
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */

#ifdef EEPROM_RETRIEVE_SUPPORT
static UINT8 g_dump_content[MAX_EEPROM_BUFFER_SIZE];
INT32 show_e2p_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int status = 0;
	PCHAR pch = NULL;
	UINT16 dump_offset = 0x0;
	UINT16 dump_size = 0x20;
	UINT8 *dump_content = &g_dump_content[0];
	int i;
	if (arg != NULL) {
		pch = strsep(&arg, "-");
		if (pch != NULL)
			dump_offset = os_str_tol(pch, 0, 10);

		pch = strsep(&arg, "-");
		if (pch != NULL)
			dump_size = os_str_tol(pch, 0, 10);
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\x1b[1;33m %s: eeprom_type=%d, offset=%d, size=%d\x1b[m \n",
			 __func__, pAd->eeprom_type, dump_offset, dump_size));

	MtCmdEfusBufferModeGet(pAd, EEPROM_EFUSE, dump_offset, dump_size, dump_content);

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump\n\r"));
	for (i = 0; i < dump_size; i++) {
		if ((i%32) == 0)
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\r"));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x ", dump_content[i]));
	}
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\r"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\x1b[1;33m %s: End \x1b[m \n", __func__));
	return TRUE;
}
#endif /* EEPROM_RETRIEVE_SUPPORT */
#ifdef IXIA_SUPPORT
INT Set_chkT_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int dbg;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s()\n", __func__));
	dbg = simple_strtol(arg, 0, 10);
	pAd->chkTmr = dbg;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("<==%s(pAd->chkTmr = %d)\n", __func__, pAd->chkTmr));

	return TRUE;
}
INT Set_pkt_threshld_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int dbg;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s()\n", __func__));
	dbg = simple_strtol(arg, 0, 10);
	pAd->pktthld = dbg;
	if (dbg == 0) {
		dectlen_l = 8;
		dectlen_m = 8;
		dectlen_h = 8;
	} else {
		dectlen_l = 88;
		dectlen_m = 512;
		dectlen_h = 1518;
	}
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("<==%s(pAd->pktthld = %d)\n", __func__, pAd->pktthld));
	return TRUE;
}

unsigned short dectlen_l = 88;
unsigned short dectlen_m = 512;
unsigned short dectlen_h = 1518;
INT Set_statistic_pktlen_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int d1, d2, d3;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("==>%s Before(dectlen_l: %d, dectlen_m: %d, dectlen_h: %d)\n",
		__func__, dectlen_l, dectlen_m, dectlen_h));
	sscanf(arg, "%d-%d-%d", &d1, &d2, &d3);
	dectlen_l = (unsigned short)d1;
	dectlen_m = (unsigned short)d2;
	dectlen_h = (unsigned short)d3;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("<==%s After(dectlen_l: %d, dectlen_m: %d, dectlen_h: %d)\n",
		__func__, dectlen_l, dectlen_m, dectlen_h));
	return TRUE;
}
INT	Set_Rssi_Threshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT rssithval;

	rssithval = simple_strtol(arg, 0, 10);
	pAd->DeltaRssiTh = rssithval;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s(): pAd->DeltaRssiTh = %d\n",
		__func__, pAd->DeltaRssiTh));
	return TRUE;
}
INT	Set_IXIA_TX_MODE_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT Mode;

	Mode = simple_strtol(arg, 0, 10);
	/*Force max tx cnt*/
	if (Mode == 1)
		pAd->ixiaCtrl.iForceMTO = 0;
	/*force Multi-client MAC to One MAC*/
	if (Mode == 2)
		pAd->ixiaCtrl.iForceMTO = 1;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s(%d): iForceMTO(%d)\n",
		__func__, Mode, pAd->ixiaCtrl.iForceMTO));
	return TRUE;
}
INT	Set_MinRssi_Threshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR rssithval;

	rssithval = (CHAR)simple_strtol(arg, 0, 10);
	pAd->MinRssiTh = rssithval;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s(): pAd->MinRssiTh = %d\n",
		__func__, pAd->MinRssiTh));
	return TRUE;
}

INT Set_TxSwqCtrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Ctrl;

	Ctrl = (UCHAR)simple_strtol(arg, 0, 10);
	if (Ctrl > 7) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Value out of range(%d).\n", Ctrl));
		return TRUE;
	}
	pAd->ixiaCtrl.itxCtrl = Ctrl;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cmd(%d), ixiatxCtrl(%d).\n", Ctrl, pAd->ixiaCtrl.itxCtrl));
	return TRUE;
}
INT	Set_vowMode_Manual_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT md;

	md = simple_strtol(arg, 0, 10);
	pAd->ixiaCtrl.itxCtrl = IXIA_CTL_FORCE_MAX;
	if (md == 0) {
		/*Manually control ATF off*/
		pAd->vow_cfg.en_airtime_fairness = FALSE;
		vow_set_feature_all(pAd);
	} else if (md == 1) {
			/*Manually control ATF on*/
		pAd->vow_cfg.en_airtime_fairness = TRUE;
		vow_set_feature_all(pAd);
	} else {
		/*auto detect ixia mode*/
		pAd->ixiaCtrl.itxCtrl = 0;
	}
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("==>%s(): MD = %d, itxCtrl = %d\n", __func__, md, pAd->ixiaCtrl.itxCtrl));
	return TRUE;
}

#endif /*IXIA_SUPPORT*/
