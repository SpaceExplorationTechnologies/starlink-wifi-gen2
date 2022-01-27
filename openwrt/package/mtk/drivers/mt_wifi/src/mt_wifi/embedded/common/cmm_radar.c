/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    cmm_radar.c

    Abstract:
    CS/DFS common functions.

    Revision History:
    Who       When            What
    --------  ----------      ----------------------------------------------
*/
#include "rt_config.h"
#include "wlan_config/config_internal.h"

/*----- 802.11H -----*/
/*
	========================================================================

	Routine Description:
		Radar channel check routine

	Arguments:
		pAd	Pointer to our adapter

	Return Value:
		TRUE	need to do radar detect
		FALSE	need not to do radar detect

	========================================================================
*/
BOOLEAN RadarChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	INT	i;
	BOOLEAN result = FALSE;

	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			if (Ch == pChCtrl->ChList[i].Channel) {
				result = pChCtrl->ChList[i].DfsReq;
				break;
			}
		}
	}

	return result;
}


/*
	========================================================================

	Routine Description:
		Determine the current radar state

	Arguments:
		pAd	Pointer to our adapter

	Return Value:

	========================================================================
*/
VOID RadarStateCheck(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev)
{
	struct DOT11_H *pDot11h = NULL;
	struct wlan_config *cfg = NULL;
	UCHAR phy_bw = 0;
	UCHAR vht_cent2 = 0;
#ifdef MT_DFS_SUPPORT
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#endif /* MT_DFS_SUPPORT */

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;

	cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (cfg == NULL)
		return;

	if (cfg->ht_conf.ht_bw == HT_BW_20)
		phy_bw = BW_20;
	else if (cfg->ht_conf.ht_bw == HT_BW_40) {
		if (cfg->vht_conf.vht_bw == VHT_BW_2040)
			phy_bw = BW_40;
		else if (cfg->vht_conf.vht_bw == VHT_BW_80)
			phy_bw = BW_80;
		else if (cfg->vht_conf.vht_bw == VHT_BW_160)
			phy_bw = BW_160;
		else if (cfg->vht_conf.vht_bw == VHT_BW_8080)
			phy_bw = BW_8080;
		else
			;
	}
	vht_cent2 = cfg->phy_conf.cen_ch_2;

	if (wdev->csa_count != 0)
		return;

#ifdef MT_DFS_SUPPORT
	if ((pAd->CommonCfg.bIEEE80211H == 1) &&
		DfsRadarChannelCheck(pAd, wdev, vht_cent2, phy_bw)
	) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] RD_SILENCE_MODE \x1b[m \n", __func__));

		pDot11h->RDCount = 0;
		pDot11h->InServiceMonitorCount = 0;
		/*Before Setting RD_SILENCE_MODE, also setting ChMovingTime*/
		/*as it causes a race in ApMlmePeriodicExec*/
		if ((pDfsParam->RDDurRegion == CE)
			&& DfsCacRestrictBand(pAd, pDfsParam->Bw, pDfsParam->Band0Ch,
				pDfsParam->Band1Ch)) {
			/* Weather band channel */
			if (pDfsParam->targetCh != 0)
				pDot11h->ChMovingTime = pDfsParam->targetCacValue;
			else
				pDot11h->ChMovingTime = CAC_WETHER_BAND;
		} else {
			if (pDfsParam->targetCh != 0)
				pDot11h->ChMovingTime = pDfsParam->targetCacValue;
			else
				pDot11h->ChMovingTime = CAC_NON_WETHER_BAND;
		}
		pDot11h->RDMode = RD_SILENCE_MODE;
		if (DfsIsOutBandAvailable(pAd)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] OutBand Available. Set into RD_NORMAL_MODE \x1b[m \n", __func__));
			pDot11h->RDMode = RD_NORMAL_MODE;
		} else if (DfsIsTargetChAvailable(pAd)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Target Channel Bypass CAC. Set into RD_NORMAL_MODE \x1b[m \n", __FUNCTION__));
			pDot11h->RDMode = RD_NORMAL_MODE;

		} else
			;
	} else
#endif
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] RD_NORMAL_MODE \x1b[m \n", __func__));
		/* DFS Zero wait case, OP CH always is normal mode */
		pDot11h->RDMode = RD_NORMAL_MODE;
	}
}

BOOLEAN CheckNonOccupancyChannel(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR ch)
{
	INT i;
	BOOLEAN InNOP = FALSE;
	UCHAR channel = 0;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	if (ch == RDD_CHECK_NOP_BY_WDEV)
		channel = wdev->channel;
	else
		channel = ch;

#ifdef MT_DFS_SUPPORT
	DfsNonOccupancyUpdate(pAd);
#endif

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (pChCtrl->ChList[i].Channel == channel) {
			if (pChCtrl->ChList[i].RemainingTimeForUse > 0) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("ERROR: previous detection of a radar on this channel(Channel=%d).\n",
						  pChCtrl->ChList[i].Channel));
				InNOP = TRUE;
				break;
			}
		}
	}

	if ((InNOP == FALSE)
#ifdef MT_DFS_SUPPORT
		|| DfsStopWifiCheck(pAd)
#endif
	)
		return TRUE;
	else
		return FALSE;
}

ULONG JapRadarType(
	IN PRTMP_ADAPTER pAd)
{
	ULONG		i;
	const UCHAR	Channel[15] = {52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};
	BOOLEAN IsSupport5G = HcIsRfSupport(pAd, RFIC_5GHZ);
	UCHAR Channel5G = HcGetChannelByRf(pAd, RFIC_5GHZ);

	if (pAd->CommonCfg.RDDurRegion != JAP)
		return pAd->CommonCfg.RDDurRegion;

	for (i = 0; i < 15; i++) {
		if (IsSupport5G && Channel5G ==  Channel[i])
			break;
	}

	if (i < 4)
		return JAP_W53;
	else if (i < 15)
		return JAP_W56;
	else
		return JAP; /* W52*/
}


UCHAR get_channel_by_reference(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 mode,
	IN struct wifi_dev *wdev)
{
	UCHAR ch = 0;
	INT ch_idx;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

#ifdef MT_DFS_SUPPORT
	DfsNonOccupancyUpdate(pAd);
#endif

	switch (mode) {
	case 1: {
		USHORT min_time = 0xFFFF;

		/* select channel with least RemainingTimeForUse */
		for (ch_idx = 0; ch_idx <  pChCtrl->ChListNum; ch_idx++) {
			if (pChCtrl->ChList[ch_idx].RemainingTimeForUse < min_time) {
				min_time = pChCtrl->ChList[ch_idx].RemainingTimeForUse;
				ch = pChCtrl->ChList[ch_idx].Channel;
			}
		}

		break;
	}

	default: {
		ch = FirstChannel(pAd, wdev);
		break;
	}
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("%s(): mode = %u, ch = %u\n",
			 __func__, mode, ch));
	return ch;
}


#ifdef CONFIG_AP_SUPPORT
/*
	========================================================================

	Routine Description:
		Channel switching count down process upon radar detection

	Arguments:
		pAd	Pointer to our adapter

	========================================================================
*/
VOID ChannelSwitchingCountDownProc(
	IN PRTMP_ADAPTER	pAd,
	struct wifi_dev *wdev)
{
	UCHAR apIdx = 0xff;
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("%s(): Wdev(%d) Channel Switching...(%d/%d)\n",
			 __func__, wdev->wdev_idx, pDot11h->CSCount, pDot11h->CSPeriod));
	pDot11h->CSCount++;

	if (pDot11h->CSCount >= pDot11h->CSPeriod) {
		if (wdev && (wdev->wdev_type == WDEV_TYPE_AP))
			apIdx = wdev->func_idx;

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("  Type = %d, func_idx = %d\n",
				 wdev->wdev_type, wdev->func_idx));

		RTEnqueueInternalCmd(pAd, CMDTHRED_DOT11H_SWITCH_CHANNEL, &apIdx, sizeof(UCHAR));
	}
}

void update_ch_by_wdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
/*
*
*/
NTSTATUS Dot11HCntDownTimeoutAction(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UCHAR apIdx;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	UCHAR apOper = AP_BSS_OPER_ALL;
	struct DOT11_H *pDot11h = NULL;

	NdisMoveMemory(&apIdx, CMDQelmt->buffer, sizeof(UCHAR));

	/* check apidx valid */
	if (apIdx != 0xff) {
		pMbss = &pAd->ApCfg.MBSSID[apIdx];
		apOper = AP_BSS_OPER_SINGLE;
	}

	if (pMbss == NULL)
		return 0;

	pDot11h = pMbss->wdev.pDot11_H;
	if (pDot11h == NULL)
		return 0;

	/* Normal DFS */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	DedicatedZeroWaitStop(pAd, FALSE);
#endif
	/*If RDMode is not updated then beacon still has CSA IE even after CSA is done*/
	pDot11h->bCSInProgress = TRUE;
	pDot11h->RDMode = RD_SILENCE_MODE;
#ifdef CONFIG_MAP_SUPPORT
		if (pMbss->wdev.quick_ch_change == TRUE && !RadarChannelCheck(pAd, pMbss->wdev.channel)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s %d\n",
							(char *)pMbss->wdev.if_dev->name,
							pMbss->wdev.quick_ch_change));
			update_ch_by_wdev(pAd, &pMbss->wdev);
		} else {
#endif
	APStop(pAd, pMbss, apOper);
#ifdef MT_DFS_SUPPORT
	if (DfsStopWifiCheck(pAd)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Stop AP Startup\n", __func__));
		pDot11h->bCSInProgress = FALSE;
		return 0;
	}
#endif
		APStartUp(pAd, pMbss, apOper);
#ifdef CONFIG_MAP_SUPPORT
		}
#endif
	pDot11h->bCSInProgress = FALSE;
#ifdef MT_DFS_SUPPORT
	if (pAd->CommonCfg.dbdc_mode) {
		MtCmdSetDfsTxStart(pAd, HcGetBandByWdev(&pMbss->wdev));
	} else {
		MtCmdSetDfsTxStart(pAd, DBDC_BAND0);
	}
	DfsSetCacRemainingTime(pAd, &pMbss->wdev);
	DfsReportCollision(pAd);
#ifdef BACKGROUND_SCAN_SUPPORT
	DfsDedicatedScanStart(pAd);
#endif
#endif

#ifdef CONFIG_MAP_SUPPORT
	/* Enabling the apcli interface once main interface is UP */
	if (IS_MAP_TURNKEY_ENABLE(pAd) && (pMbss->mbss_idx == 0)) {
		int idx;
		struct wifi_dev *wdev = NULL;
		for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (wdev && wdev->pDot11_H == pDot11h) {
				pAd->StaCfg[idx].ApcliInfStat.Enable = TRUE;
				break;
			}
		}
	}
#endif
	return 0;
}

#endif /* CONFIG_AP_SUPPORT */




/*
    ==========================================================================
    Description:
	Set channel switch Period
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_CSPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	pAd->Dot11_H[0].CSPeriod = (USHORT) os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Set_CSPeriod_Proc::(CSPeriod=%d)\n", pAd->Dot11_H[0].CSPeriod));
	return TRUE;
}

/*
    ==========================================================================
    Description:
		change channel moving time for DFS testing.

	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 set ChMovTime=[value]
    ==========================================================================
*/
INT Set_ChMovingTime_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	USHORT Value;
	Value = (USHORT) os_str_tol(arg, 0, 10);
	pAd->Dot11_H[0].ChMovingTime = Value;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("%s: %d\n", __func__,
			 pAd->Dot11_H[0].ChMovingTime));
	return TRUE;
}


/*
    ==========================================================================
    Description:
		Reset channel block status.
	Arguments:
	    pAd				Pointer to our adapter
	    arg				Not used

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 set ChMovTime=[value]
    ==========================================================================
*/
INT Set_BlockChReset_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	INT i;
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("%s: Reset channel block status.\n", __func__));

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		for (i = 0; i < pChCtrl->ChListNum; i++)
			pChCtrl->ChList[i].RemainingTimeForUse = 0;
	}

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Initialize the pDot11H of wdev

    Parameters:

    return:
    ==========================================================================
 */
VOID UpdateDot11hForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN attach)
{
	UCHAR bandIdx = 0;

	if (attach) {
		if (wdev) {
			bandIdx = HcGetBandByWdev(wdev);
			wdev->pDot11_H = &pAd->Dot11_H[bandIdx];
		} else {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): no wdev!\n", __func__));
		}
	} else {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s(): Detach wdev=%d_Dot11_H!\n", __func__, wdev->wdev_idx));
		wdev->pDot11_H = NULL;
	}
}

#ifdef MT_DFS_SUPPORT
#if (defined(MT7626) || defined(MT7663))
INT set_radar_min_lpn_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PDFS_RADAR_DETECTION_PARAM prRadarDetectionParam = NULL;
	UINT16 min_lpn_update = simple_strtol(arg, 0, 10);

	prRadarDetectionParam = pAd->CommonCfg.DfsParameter.prRadarDetectionParam;

	if (min_lpn_update <= LPB_SIZE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s():LPN Update %d \n", __func__, min_lpn_update));

		prRadarDetectionParam->fcc_lpn_min = min_lpn_update;
		mt_cmd_set_fcc5_min_lpn(pAd, min_lpn_update);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s():Invalid LPN value %d, please set in range 0 to 32\n", __func__, min_lpn_update));
	}
	return TRUE;
}

INT set_radar_thres_param_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CMD_RDM_RADAR_THRESHOLD_UPDATE_T RadarThreshold = {0};
	INT32 recv = 0;
	UINT32 radar_type_idx = 0;
	UINT32 rt_en = 0, rt_stgr = 0;
	UINT32 rt_crpn_min = 0, rt_crpn_max = 0, rt_crpr_min = 0;
	UINT32 rt_pw_min = 0, rt_pw_max = 0;
	UINT32 rt_crbn_min = 0, rt_crbn_max = 0;
	UINT32 rt_stg_pn_min = 0, rt_stg_pn_max = 0, rt_stg_pr_min = 0;
	UINT32 rt_pri_min = 0, rt_pri_max = 0;

	PSW_RADAR_TYPE_T prRadarType = NULL;
	PDFS_RADAR_DETECTION_PARAM prRadarDetectionParam = NULL;
	PDFS_RADAR_THRESHOLD_PARAM prRadarThresholdParam = NULL;

	prRadarDetectionParam = pAd->CommonCfg.DfsParameter.prRadarDetectionParam;
	prRadarThresholdParam = &prRadarDetectionParam->rRadarThresholdParam;

	if (arg) {
		recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d",
						&(radar_type_idx), &(rt_en), &(rt_stgr), &(rt_crpn_min),
						&(rt_crpn_max), &(rt_crpr_min), &(rt_pw_min), &(rt_pw_max),
						&(rt_pri_min), &(rt_pri_max), &(rt_crbn_min), &(rt_crbn_max),
						&(rt_stg_pn_min), &(rt_stg_pn_max), &(rt_stg_pr_min));

		if (recv != 15) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("Format Error! Please enter in the following format\n"
						"RadarType-RT_ENB-RT_STGR-RT_CRPN_MIN-RT_CRPN_MAX-RT_CRPR_MIN-RT_PW_MIN-RT_PW_MAX-"
						"RT_PRI_MIN-RT_PRI_MAX-RT_CRBN_MIN-RT_CRBN_MAX-RT_STGPN_MIN-RT_STGPN_MAX-RT_STGPR_MIN\n"));
			return TRUE;
		}

		if (radar_type_idx > RT_NUM) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("Radar Type invalid!! Only 0 to %d supported\n", RT_NUM));
			return TRUE;
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s():RadarType = %d\n RT_ENB = %d\n RT_STGR = %d\n "
			"RT_CRPN_MIN = %d\n RT_CRPN_MAX = %d\n RT_CRPR_MIN = %d\n "
			"RT_PW_MIN = %d\n RT_PW_MAX =%d\n RT_PRI_MIN = %d\n "
			"RT_PRI_MAX = %d\n RT_CRBN_MIN = %d\n RT_CRBN_MAX = %d\n"
			"RT_STGPN_MIN = %d\n RT_STGPN_MAX = %d\n RT_STGPR_MIN = %d\n ",
			__func__, radar_type_idx, rt_en, rt_stgr, rt_crpn_min,
			rt_crpn_max, rt_crpr_min, rt_pw_min, rt_pw_max,
			rt_pri_min, rt_pri_max, rt_crbn_min, rt_crbn_max,
			rt_stg_pn_min, rt_stg_pn_max, rt_stg_pr_min));

		memset(&RadarThreshold, 0, sizeof(CMD_RDM_RADAR_THRESHOLD_UPDATE_T));
		RadarThreshold.radar_type_idx = radar_type_idx;
		RadarThreshold.rt_en  = rt_en;
		RadarThreshold.rt_stgr = rt_stgr;
		RadarThreshold.rt_crpn_min =  rt_crpn_min;
		RadarThreshold.rt_crpn_max = rt_crpn_max;
		RadarThreshold.rt_crpr_min = rt_crpr_min;
		RadarThreshold.rt_pw_min = rt_pw_min;
		RadarThreshold.rt_pw_max = rt_pw_max;
		RadarThreshold.rt_pri_min = rt_pri_min;
		RadarThreshold.rt_pri_max = rt_pri_max;
		RadarThreshold.rt_crbn_min = rt_crbn_min;
		RadarThreshold.rt_crbn_max = rt_crbn_max;
		RadarThreshold.rt_stg_pn_min = rt_stg_pn_min;
		RadarThreshold.rt_stg_pn_max = rt_stg_pn_max;
		RadarThreshold.rt_stg_pr_min = rt_stg_pr_min;

		prRadarType = &prRadarThresholdParam->arRadarType[radar_type_idx];
		prRadarType->rt_en = RadarThreshold.rt_en;
		prRadarType->rt_stgr = RadarThreshold.rt_stgr;
		prRadarType->rt_crpn_min = RadarThreshold.rt_crpn_min;
		prRadarType->rt_crpn_max = RadarThreshold.rt_crpn_max;
		prRadarType->rt_crpr_min = RadarThreshold.rt_crpr_min;
		prRadarType->rt_pw_min = RadarThreshold.rt_pw_min;
		prRadarType->rt_pw_max = RadarThreshold.rt_pw_max;
		prRadarType->rt_pri_min = RadarThreshold.rt_pri_min;
		prRadarType->rt_pri_max = RadarThreshold.rt_pri_max;
		prRadarType->rt_crbn_min = RadarThreshold.rt_crbn_min;
		prRadarType->rt_crbn_max = RadarThreshold.rt_crbn_max;
		prRadarType->rt_stg_pn_min = RadarThreshold.rt_stg_pn_min;
		prRadarType->rt_stg_pn_max = RadarThreshold.rt_stg_pn_max;
		prRadarType->rt_stg_pr_min = RadarThreshold.rt_stg_pr_min;

		mt_cmd_set_radar_thres_param(pAd, &RadarThreshold);
	}

	return TRUE;

}

INT set_radar_pls_thres_param_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 recv = 0, pls_pwr_max = 0, pls_pwr_min = 0;
	UINT32 pls_width_max = 0, pri_min_stgr = 0, pri_max_stgr = 0;
	UINT32 pri_min_cr = 0, pri_max_cr = 0;
	PDFS_RADAR_DETECTION_PARAM prRadarDetectionParam = NULL;
	PDFS_PULSE_THRESHOLD_PARAM prPulseThresholdParam = NULL;
	CMD_RDM_PULSE_THRESHOLD_UPDATE_T pls_thres_update = {0};

	if (arg) {
		recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d",
							&(pls_width_max), &(pls_pwr_max), &(pls_pwr_min),
							&(pri_min_stgr), &(pri_max_stgr), &(pri_min_cr), &(pri_max_cr));

		if (recv != 7) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Format Error! Please enter in the following format\n"
					"MaxPulseWidth-MaxPulsePower-MinPulsePower-"
					"MinPRISTGR-MaxPRISTGR-MinPRICR-MaxPRICR\n"));
			return TRUE;
		}
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s():MaxPulseWidth = %d\nMaxPulsePower = %d\nMinPulsePower = %d\n"
					"MinPRISTGR = %d\nMaxPRISTGR = %d\nMinPRICR = %d\nMaxPRICR = %d\n",
					__func__, pls_width_max, pls_pwr_max, pls_pwr_min,
					pri_min_stgr, pri_max_stgr, pri_min_cr, pri_max_cr));

		pls_thres_update.prd_pls_width_max = pls_width_max;
		pls_thres_update.pls_pwr_max = pls_pwr_max;
		pls_thres_update.pls_pwr_min = pls_pwr_min;
		pls_thres_update.pri_min_stgr = pri_min_stgr;
		pls_thres_update.pri_max_stgr = pri_max_stgr;
		pls_thres_update.pri_min_cr = pri_min_cr;
		pls_thres_update.pri_max_cr = pri_max_cr;

		prRadarDetectionParam = pAd->CommonCfg.DfsParameter.prRadarDetectionParam;
		prPulseThresholdParam = &prRadarDetectionParam->rRadarThresholdParam.rPulseThresholdParam;

		prPulseThresholdParam->u4PulseWidthMax = pls_width_max;
		prPulseThresholdParam->i4PulsePwrMax = pls_pwr_max;
		prPulseThresholdParam->i4PulsePwrMin = pls_pwr_min;
		prPulseThresholdParam->u4PRI_MIN_STGR = pri_min_stgr;
		prPulseThresholdParam->u4PRI_MAX_STGR = pri_max_stgr;
		prPulseThresholdParam->u4PRI_MIN_CR = pri_min_cr;
		prPulseThresholdParam->u4PRI_MAX_CR = pri_max_cr;

		mt_cmd_set_pls_thres_param(pAd, &pls_thres_update);
	}

	return TRUE;
}

INT	set_radar_test_pls_pattern_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 recv = 0;
	CHAR *p_pls_params = 0;
	UINT32 pls_num = 0;

	CMD_RDM_TEST_RADAR_PATTERN_T pls_pattern = {0};
	PPERIODIC_PULSE_BUFFER_T pr_pls_buff = NULL;
	/*
	 Ex: 29151901-28-748;29153127-29-760;29154352-29-748;29155577-28-760;29156652-29-751
	*/
	if (arg) {
		for (pls_num = 0, p_pls_params = rstrtok(arg, ";"); (p_pls_params != NULL) && (pls_num < PPB_SIZE); p_pls_params = rstrtok(NULL, ";"), pls_num++) {
			pr_pls_buff = &pls_pattern.prd_pls_buff[pls_num];

			recv = sscanf(p_pls_params, "%d-%hu-%hi", &(pr_pls_buff->prd_strt_time), &(pr_pls_buff->prd_pls_wdth), &(pr_pls_buff->prd_pls_pwr));

			if (recv != 3) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("Format Error! Please enter in the following format\n"
							"StartTime0-PulseWidth0-PulsePower0;StartTime1-PulseWidth1-PulsePower1;...\n"));
				return TRUE;
			}
		}

		pls_pattern.pls_num = pls_num;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:No of pulses = %d\n", __func__, pls_pattern.pls_num));
		mt_cmd_set_test_radar_pattern(pAd, &pls_pattern);
	}

	return TRUE;
}

INT	set_radar_dbg_log_config_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PDFS_RADAR_DETECTION_PARAM prRadarDetectionParam = NULL;
	INT32 recv = 0;
	UINT32 hw_rdd_log_en = 0;
	UINT32 sw_rdd_log_en = 0;
	UINT32 sw_rdd_log_cond = 1;

	prRadarDetectionParam = pAd->CommonCfg.DfsParameter.prRadarDetectionParam;
	if (arg) {
		recv = sscanf(arg, "%d-%d-%d", &(hw_rdd_log_en), &(sw_rdd_log_en), &(sw_rdd_log_cond));

		if (recv != 3) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Format Error! Please enter in the following format\n"
					"HWRDD_LOG_ENB-SWRDD_LOG_ENB-SWRDD_LOG_COND\n"));
			return TRUE;
		}

		if (hw_rdd_log_en != 0)
			prRadarDetectionParam->is_hw_rdd_log_en = TRUE;
		else
			prRadarDetectionParam->is_hw_rdd_log_en = FALSE;
		if (sw_rdd_log_en != 0)
			prRadarDetectionParam->is_sw_rdd_log_en = TRUE;
		else
			prRadarDetectionParam->is_sw_rdd_log_en = FALSE;
		if (sw_rdd_log_cond == 0)
			prRadarDetectionParam->sw_rdd_log_cond = FALSE;
		else
			prRadarDetectionParam->sw_rdd_log_cond = TRUE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s():HWRDD_LOG_ENB = %d, SWRDD_LOG_ENB = %d SWRDD_LOG_COND = %d \n",
			__func__,
			prRadarDetectionParam->is_hw_rdd_log_en,
			prRadarDetectionParam->is_sw_rdd_log_en,
			prRadarDetectionParam->sw_rdd_log_cond));

		mt_cmd_set_rdd_log_config(pAd, hw_rdd_log_en, sw_rdd_log_en, sw_rdd_log_cond);
	}

	return TRUE;
}

INT show_radar_threshold_param_proc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *arg)
{
	UINT8 radar_type_idx = 0;
	PDFS_RADAR_THRESHOLD_PARAM prRadarThresholdParam = NULL;
	PDFS_RADAR_DETECTION_PARAM prRadarDetectionParam = NULL;
	PSW_RADAR_TYPE_T prRadarType = NULL;

	prRadarDetectionParam = pAd->CommonCfg.DfsParameter.prRadarDetectionParam;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_ERROR,
				("pAd->CommonCfg.RDDurRegion = %d", pAd->CommonCfg.RDDurRegion));
	prRadarThresholdParam = &prRadarDetectionParam->rRadarThresholdParam;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("---------------------------------Debug Log Conditions---------------------------------------\n"));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("HWRDD_LOG_ENB = %u\nSWRDD_LOG_ENB = %u\nSWRDD_LOG_COND = %u\n",
					prRadarDetectionParam->is_hw_rdd_log_en,
					prRadarDetectionParam->is_sw_rdd_log_en,
					prRadarDetectionParam->sw_rdd_log_cond));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("-------------------------------Pulse Threshold Parameters-----------------------------------\n"));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("FCC5_LPN = %u\n",
					prRadarDetectionParam->fcc_lpn_min));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PLS_POWER_MIN = %d\n",
														prRadarThresholdParam->rPulseThresholdParam.i4PulsePwrMin));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PLS_POWER_MAX = %d\n",
														prRadarThresholdParam->rPulseThresholdParam.i4PulsePwrMax));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("SP_PW_MAX = %d\n",
														prRadarThresholdParam->rPulseThresholdParam.u4PulseWidthMax));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PRI_MIN_STGR = %d\n",
														prRadarThresholdParam->rPulseThresholdParam.u4PRI_MIN_STGR));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PRI_MAX_STGR = %d\n",
														prRadarThresholdParam->rPulseThresholdParam.u4PRI_MAX_STGR));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PRI_MIN_CR = %d\n",
														prRadarThresholdParam->rPulseThresholdParam.u4PRI_MIN_CR));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PRI_MAX_CR = %d\n",
														prRadarThresholdParam->rPulseThresholdParam.u4PRI_MAX_CR));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("---------------------------------RADAR Threshold Info---------------------------------------\n"));

	for (radar_type_idx = 0; radar_type_idx < RT_NUM; radar_type_idx++) {
		prRadarType = &prRadarThresholdParam->arRadarType[radar_type_idx];
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("RT - %d: ENB = %d, STGR = %d, CRPN_MIN = %d, CRPN_MAX = %d, CRPR_MIN = %d, PW_MIN = %d, PW_MAX = %d,"
					"PRI_MIN = %d, PRI_MAX = %d, CRBN_MIN = %d, CRBN_MAX = %d\n\t"
					"STGPN_MIN = %d, STGPN_MAX = %d, STGPR_MIN = %d\n",
							radar_type_idx,
							prRadarType->rt_en,
							prRadarType->rt_stgr,
							prRadarType->rt_crpn_min,
							prRadarType->rt_crpn_max,
							prRadarType->rt_crpr_min,
							prRadarType->rt_pw_min,
							prRadarType->rt_pw_max,
							prRadarType->rt_pri_min,
							prRadarType->rt_pri_max,
							prRadarType->rt_crbn_min,
							prRadarType->rt_crbn_max,
							prRadarType->rt_stg_pn_min,
							prRadarType->rt_stg_pn_max,
							prRadarType->rt_stg_pr_min
							));
	}
	MTWF_LOG(DBG_CAT_AP, CATPROTO_DFS, DBG_LVL_OFF,
		("---------------------------------------------------------------------------------------------\n"));

	return TRUE;
}
#endif /*(defined(MT7626) || defined(MT7663))*/
#endif
