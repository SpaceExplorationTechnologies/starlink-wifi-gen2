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

    Abstract:
*/


#include "rt_config.h"
#include "bgnd_scan.h"

/* extern MT_SWITCH_CHANNEL_CFG CurrentSwChCfg[2]; */

BOOLEAN BackgroundScanSkipChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	UCHAR i;
	BOOLEAN result = FALSE;

	for (i = 0; i < pAd->BgndScanCtrl.SkipChannelNum; i++) {
		if (Ch == pAd->BgndScanCtrl.SkipChannelList[i]) {
			result = TRUE;
			break;
		}
	}

	return result;
}

static inline INT GetABandChOffset(
	IN INT Channel)
{
#ifdef A_BAND_SUPPORT

	if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
		(Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
		return 1;
	else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
			 (Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161))
		return -1;

#endif /* A_BAND_SUPPORT */
	return 0;
}

UCHAR BgndSelectBestChannel(RTMP_ADAPTER *pAd)
{
	int i;
	UCHAR BestChannel = 0, BestPercen = 0xff, Percen = 0;

	for (i = 0; i < pAd->BgndScanCtrl.GroupChListNum; i++) {
		if (pAd->BgndScanCtrl.GroupChList[i].SkipGroup == 0) {
			Percen = ((pAd->BgndScanCtrl.GroupChList[i].Max_PCCA_Time) * 100) / (((pAd->BgndScanCtrl.ScanDuration) * 1000) - (pAd->BgndScanCtrl.GroupChList[i].Band0_Tx_Time));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ChIdx=%d control-Channle=%d cen-channel=%d\n", i, pAd->BgndScanCtrl.GroupChList[i].BestCtrlChannel, pAd->BgndScanCtrl.GroupChList[i].CenChannel));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		Max_PCCA=%x, Min_PCCA=%x, Band0_Tx_Time=%x, Percentage=%d\n",
					 pAd->BgndScanCtrl.GroupChList[i].Max_PCCA_Time, pAd->BgndScanCtrl.GroupChList[i].Min_PCCA_Time,
					 pAd->BgndScanCtrl.GroupChList[i].Band0_Tx_Time, Percen));

			if (Percen <= BestPercen) {
				BestPercen = Percen;
				BestChannel = pAd->BgndScanCtrl.GroupChList[i].BestCtrlChannel;
			}
		}
	}

	return BestChannel;
}
VOID NextBgndScanChannel(RTMP_ADAPTER *pAd, UCHAR channel)
{
	int i;
	/* UCHAR next_channel = 0; */
	pAd->BgndScanCtrl.ScanChannel = 0;

	for (i = 0; i < (pAd->BgndScanCtrl.ChannelListNum - 1); i++) {
		if (channel == pAd->BgndScanCtrl.BgndScanChList[i].Channel) {
			/* Record this channel's idx in ChannelList array.*/
			while (i < (pAd->BgndScanCtrl.ChannelListNum - 1)) {
				if (pAd->BgndScanCtrl.BgndScanChList[i + 1].SkipChannel != 1) {
					pAd->BgndScanCtrl.ScanChannel = pAd->BgndScanCtrl.BgndScanChList[i + 1].Channel;
					pAd->BgndScanCtrl.ScanCenChannel = pAd->BgndScanCtrl.BgndScanChList[i + 1].CenChannel;
					pAd->BgndScanCtrl.ChannelIdx = i + 1;
					return;
				} else
					i++;
			}

		}
	}
}

VOID FirstBgndScanChannel(RTMP_ADAPTER *pAd)
{
	int i;

	/* Find the first non skiped channel */
	for (i = 0; i < (pAd->BgndScanCtrl.ChannelListNum - 1); i++) {
		if (pAd->BgndScanCtrl.BgndScanChList[i].SkipChannel != 1) {
			/* Record this channel's idx in ChannelList array.*/
			pAd->BgndScanCtrl.ScanChannel = pAd->BgndScanCtrl.BgndScanChList[i].Channel;
			pAd->BgndScanCtrl.ScanCenChannel = pAd->BgndScanCtrl.BgndScanChList[i].CenChannel;
			pAd->BgndScanCtrl.FirstChannel = pAd->BgndScanCtrl.BgndScanChList[i].Channel;
			pAd->BgndScanCtrl.ChannelIdx = i;
			break;
		}
	}

}

VOID BuildBgndScanChList(RTMP_ADAPTER *pAd)
{
	INT channel_idx, ChListNum = 0;
	UCHAR ch;
	struct wifi_dev *wdev = get_default_wdev(pAd);
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
#ifdef DOT11_VHT_AC
	UCHAR vht_bw = wlan_config_get_vht_bw(wdev);
#endif
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	if (pAd->BgndScanCtrl.IsABand) {
		for (channel_idx = 0; channel_idx < pChCtrl->ChListNum; channel_idx++) {
			ch = pChCtrl->ChList[channel_idx].Channel;

			if (cfg_ht_bw == BW_20) {
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].Channel = pChCtrl->ChList[channel_idx].Channel;
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = pChCtrl->ChList[channel_idx].Channel;
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].DfsReq = pChCtrl->ChList[channel_idx].DfsReq;
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].SkipChannel = BackgroundScanSkipChannelCheck(pAd, ch);
				ChListNum++;
			}

#ifdef DOT11_N_SUPPORT
			else if (((cfg_ht_bw == BW_40)
#ifdef DOT11_VHT_AC
					  && (vht_bw == VHT_BW_2040)
#endif /* DOT11_VHT_AC */
					 )
					 &&N_ChannelGroupCheck(pAd, ch, wdev)) {
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].Channel = pChCtrl->ChList[channel_idx].Channel;

				if (GetABandChOffset(ch) == 1)
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = pChCtrl->ChList[channel_idx].Channel + 2;
				else
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = pChCtrl->ChList[channel_idx].Channel - 2;

				pAd->BgndScanCtrl.BgndScanChList[ChListNum].DfsReq = pChCtrl->ChList[channel_idx].DfsReq;
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].SkipChannel = BackgroundScanSkipChannelCheck(pAd, ch);
				ChListNum++;
			}

#ifdef DOT11_VHT_AC
			else if (vht_bw == VHT_BW_80) {
				if (vht80_channel_group(pAd, ch)) {
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].Channel = pChCtrl->ChList[channel_idx].Channel;
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = vht_cent_ch_freq(ch, VHT_BW_80);
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].DfsReq = pChCtrl->ChList[channel_idx].DfsReq;
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].SkipChannel = BackgroundScanSkipChannelCheck(pAd, ch);
					ChListNum++;
				}
			}

#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
		}
	} else {
		/* 2.4G only support BW20 background scan */
		for (channel_idx = 0; channel_idx < pChCtrl->ChListNum; channel_idx++) {
			pAd->BgndScanCtrl.BgndScanChList[ChListNum].Channel = pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = pChCtrl->ChList[channel_idx].Channel;
			pAd->BgndScanCtrl.BgndScanChList[ChListNum].SkipChannel = BackgroundScanSkipChannelCheck(pAd, pChCtrl->ChList[channel_idx].Channel);
			ChListNum++;
		}
	}

	pAd->BgndScanCtrl.ChannelListNum = ChListNum;
	if (pAd->BgndScanCtrl.ChannelListNum == 0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[BuildBgndScanChList] BandIdx = %d, pAd->BgndScanCtrl.ChannelListNum=%d\n",
			BandIdx, pAd->BgndScanCtrl.ChannelListNum));
	}

	for (channel_idx = 0; channel_idx < pAd->BgndScanCtrl.ChannelListNum; channel_idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Support channel: PrimCh=%d, CentCh=%d, DFS=%d\n",
				 pAd->BgndScanCtrl.BgndScanChList[channel_idx].Channel, pAd->BgndScanCtrl.BgndScanChList[channel_idx].CenChannel,
				 pAd->BgndScanCtrl.BgndScanChList[channel_idx].DfsReq));
	}
}

UINT8 GroupChListSearch(PRTMP_ADAPTER pAd, UCHAR CenChannel)
{
	UCHAR i;
	PBGND_SCAN_CH_GROUP_LIST	GroupChList = pAd->BgndScanCtrl.GroupChList;

	for (i = 0; i < pAd->BgndScanCtrl.GroupChListNum; i++) {
		if (GroupChList->CenChannel == CenChannel)
			return i;

		GroupChList++;
	}

	return 0xff;
}

VOID GroupChListInsert(PRTMP_ADAPTER pAd, PBGND_SCAN_SUPP_CH_LIST pSource)
{
	UCHAR i = pAd->BgndScanCtrl.GroupChListNum;
	PBGND_SCAN_CH_GROUP_LIST	GroupChList = &pAd->BgndScanCtrl.GroupChList[i];

	GroupChList->BestCtrlChannel = pSource->Channel;
	GroupChList->CenChannel = pSource->CenChannel;
	GroupChList->Max_PCCA_Time = GroupChList->Min_PCCA_Time = pSource->PccaTime;
	GroupChList->Band0_Tx_Time = pSource->Band0TxTime;
	GroupChList->SkipGroup = pSource->SkipChannel;
	pAd->BgndScanCtrl.GroupChListNum = i + 1;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Insert new group channel list Number=%d CenChannel=%d BestCtrlChannel=%d Max_PCCA_TIEM=%x, SkipGroup=%d\n",
			 __func__, pAd->BgndScanCtrl.GroupChListNum, GroupChList->CenChannel, GroupChList->BestCtrlChannel,	GroupChList->Max_PCCA_Time, GroupChList->SkipGroup));
}

VOID GroupChListUpdate(PRTMP_ADAPTER pAd, UCHAR index, PBGND_SCAN_SUPP_CH_LIST pSource)
{
	/* UCHAR i; */
	PBGND_SCAN_CH_GROUP_LIST	GroupChList = &pAd->BgndScanCtrl.GroupChList[index];

	if (pSource->PccaTime > GroupChList->Max_PCCA_Time) {
		GroupChList->Max_PCCA_Time = pSource->PccaTime;
		GroupChList->Band0_Tx_Time = pSource->Band0TxTime;
	}

	if (pSource->PccaTime < GroupChList->Min_PCCA_Time) {
		GroupChList->Min_PCCA_Time = pSource->PccaTime;
		GroupChList->BestCtrlChannel = pSource->Channel;
	}

	if (GroupChList->SkipGroup == 0 && pSource->SkipChannel == 1)
		GroupChList->SkipGroup = pSource->SkipChannel;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Update group channel list index=%d CenChannel=%d BestCtrlChannel=%d PCCA_TIEM=%x SkipGroup=%d\n",
			 __func__, pAd->BgndScanCtrl.GroupChListNum, GroupChList->CenChannel, GroupChList->BestCtrlChannel,	GroupChList->Max_PCCA_Time, GroupChList->SkipGroup));
}

VOID GenerateGroupChannelList(PRTMP_ADAPTER pAd)
{
	UCHAR i, ListIndex;
	/* PBGND_SCAN_CH_GROUP_LIST	GroupChList = pAd->BgndScanCtrl.GroupChList; */
	PBGND_SCAN_SUPP_CH_LIST		SuppChList = pAd->BgndScanCtrl.BgndScanChList;
	/* PBACKGROUND_SCAN_CTRL		BgndScanCtrl = &pAd->BgndScanCtrl; */
	/* PBGND_SCAN_SUPP_CH_LIST		SuppChList = BgndScanCtrl->BgndScanChList; */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ChannelListNum=%d\n",
			 __func__, pAd->BgndScanCtrl.ChannelListNum));
	os_zero_mem(pAd->BgndScanCtrl.GroupChList, MAX_NUM_OF_CHANNELS * sizeof(BGND_SCAN_CH_GROUP_LIST));
	pAd->BgndScanCtrl.GroupChListNum = 0; /* reset Group Number. */

	for (i = 0; i < pAd->BgndScanCtrl.ChannelListNum; i++) {
		ListIndex = GroupChListSearch(pAd, SuppChList->CenChannel);

		if (ListIndex == 0xff) /* Not Found */
			GroupChListInsert(pAd, SuppChList);
		else
			GroupChListUpdate(pAd, ListIndex, SuppChList);

		SuppChList++;
	}
}


VOID BackgroundScanStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, BGND_SCAN_MAX_STATE, BGND_SCAN_MAX_MSG, (STATE_MACHINE_FUNC)Drop, BGND_SCAN_IDLE, BGND_SCAN_MACHINE_BASE);
	StateMachineSetAction(Sm, BGND_SCAN_IDLE, BGND_SCAN_REQ, (STATE_MACHINE_FUNC)BackgroundScanStartAction);
	StateMachineSetAction(Sm, BGND_SCAN_IDLE, BGND_PARTIAL_SCAN, (STATE_MACHINE_FUNC)BackgroundScanWaitAction);
	StateMachineSetAction(Sm, BGND_SCAN_LISTEN, BGND_SCAN_TIMEOUT, (STATE_MACHINE_FUNC)BackgroundScanTimeoutAction);
	StateMachineSetAction(Sm, BGND_SCAN_LISTEN, BGND_SCAN_CNCL, (STATE_MACHINE_FUNC)BackgroundScanCancelAction);
	StateMachineSetAction(Sm, BGND_SCAN_LISTEN, BGND_SCAN_DONE, (STATE_MACHINE_FUNC)BackgroundScanWaitAction);
	StateMachineSetAction(Sm, BGND_SCAN_WAIT, BGND_SCAN_REQ, (STATE_MACHINE_FUNC)BackgroundScanPartialAction);
	StateMachineSetAction(Sm, BGND_SCAN_WAIT, BGND_SCAN_CNCL, (STATE_MACHINE_FUNC)BackgroundScanCancelAction);
	StateMachineSetAction(Sm, BGND_SCAN_IDLE, BGND_SWITCH_CHANNEL, (STATE_MACHINE_FUNC)BackgroundSwitchChannelAction);
#ifdef MT_DFS_SUPPORT
	StateMachineSetAction(Sm, BGND_SCAN_IDLE, BGND_DEDICATE_RDD_REQ, (STATE_MACHINE_FUNC)DedicatedZeroWaitStartAction);
	StateMachineSetAction(Sm, BGND_RDD_DETEC, BGND_OUTBAND_RADAR_FOUND, (STATE_MACHINE_FUNC)DedicatedZeroWaitRunningAction);
	StateMachineSetAction(Sm, BGND_RDD_DETEC, BGND_OUTBAND_SWITCH, (STATE_MACHINE_FUNC)DedicatedZeroWaitRunningAction);
#endif
}

VOID BackgroundScanInit(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR PhyMode = 0;
	struct wifi_dev *wdev = get_default_wdev(pAd);
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
#ifdef DOT11_VHT_AC
	UCHAR vht_bw = wlan_config_get_vht_bw(wdev);
#endif
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ===============>\n", __func__));
	/*
	ToDo: Based on current settings to decide support background scan or not.
	Don't support case: DBDC, 80+80
	*/
	/* Scan BW */
	PhyMode = HcGetRadioPhyMode(pAd);
	pAd->BgndScanCtrl.IsABand = (WMODE_CAP_5G(PhyMode)) ? TRUE : FALSE;

	if (pAd->BgndScanCtrl.IsABand) {
#ifdef DOT11_VHT_AC

		if (vht_bw == VHT_BW_80)
			pAd->BgndScanCtrl.ScanBW = BW_80;
		else
#endif /* DOT11_VHT_AC */
			pAd->BgndScanCtrl.ScanBW = cfg_ht_bw;
	} else /* 2.4G */
		pAd->BgndScanCtrl.ScanBW = BW_20;

	/* Decide RxPath&TxStream for background */
	pAd->BgndScanCtrl.RxPath = 0xc;
	pAd->BgndScanCtrl.TxStream = 0x2;
#ifdef MT_DFS_SUPPORT
	pAd->BgndScanCtrl.DfsZeroWaitDuration = DEFAULT_OFF_CHNL_CAC_TIME;/* 120000; 2 min */
#endif

	if (pAd->CommonCfg.dbdc_mode == TRUE) {
#if (RDD_PROJECT_TYPE_2 == 1)
		pAd->BgndScanCtrl.BgndScanSupport = 1;
#else
		pAd->BgndScanCtrl.BgndScanSupport = 0;
#endif
	}

#ifdef DOT11_VHT_AC
	else if (vht_bw == VHT_BW_160 || vht_bw == VHT_BW_8080)
		pAd->BgndScanCtrl.BgndScanSupport = 0;

#endif /* DOT11_VHT_AC */
	else
		pAd->BgndScanCtrl.BgndScanSupport = 1;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IsABand=%d, ScanBW=%d\n", pAd->BgndScanCtrl.IsABand, pAd->BgndScanCtrl.ScanBW));

	if (pAd->BgndScanCtrl.BgndScanSupport) {
		BackgroundScanStateMachineInit(pAd, &pAd->BgndScanCtrl.BgndScanStatMachine, pAd->BgndScanCtrl.BgndScanFunc);
		/* Copy channel list for background. */
		BuildBgndScanChList(pAd);
		RTMPInitTimer(pAd, &pAd->BgndScanCtrl.BgndScanTimer, GET_TIMER_FUNCTION(BackgroundScanTimeout), pAd, FALSE);
		/*RTMPInitTimer(pAd, &pAd->BgndScanCtrl.DfsZeroWaitTimer, GET_TIMER_FUNCTION(DfsZeroWaitTimeout), wdev, FALSE);*/

		/* ToDo: Related CR initialization. */
		if (ops->bgnd_scan_cr_init)
			ops->bgnd_scan_cr_init(pAd);

		/* MtCmdMultipleMacRegAccessWrite */
		pAd->BgndScanCtrl.ScanDuration = DefaultScanDuration;
		/* pAd->BgndScanCtrl.BgndScanInterval = DefaultBgndScanInterval; */
		pAd->BgndScanCtrl.BgndScanIntervalCount = 0;
		pAd->BgndScanCtrl.SkipDfsChannel = FALSE;
		pAd->BgndScanCtrl.PartialScanInterval = DefaultBgndScanPerChInterval; /* 10 seconds */
		pAd->BgndScanCtrl.NoisyTH = DefaultNoisyThreshold;
		pAd->BgndScanCtrl.ChBusyTimeTH = DefaultChBusyTimeThreshold;
		pAd->BgndScanCtrl.DriverTrigger = FALSE;
		pAd->BgndScanCtrl.IPIIdleTimeTH = DefaultIdleTimeThreshold;
	} else
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Background scan doesn't support in current settings....\n"));

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s <===============\n", __func__));
}

VOID BackgroundScanDeInit(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN Cancelled;

	RTMPReleaseTimer(&pAd->BgndScanCtrl.BgndScanTimer, &Cancelled);
	RTMPReleaseTimer(&pAd->BgndScanCtrl.DfsZeroWaitTimer, &Cancelled);
}

VOID BackgroundScanStart(
	IN PRTMP_ADAPTER pAd,
	IN UINT8	BgndscanType)
{
	/* UINT32	Value; */
	/* In-band commad to notify FW(RA) background scan will start. */
	/* Switch channel for Band0 (Include Star Tx/Rx ?) */
	/* Scan channel for Band1 */
	/* Reset Group channel list */
	os_zero_mem(pAd->BgndScanCtrl.GroupChList, sizeof(BGND_SCAN_CH_GROUP_LIST) * MAX_NUM_OF_CHANNELS);
	pAd->BgndScanCtrl.GroupChListNum = 0;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s BgndscanType=%d ===============>\n", __func__, BgndscanType));

	if (BgndscanType && pAd->BgndScanCtrl.ScanType) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Ap Background scan is running ===============>\n", __func__));
		return;
	}

	pAd->BgndScanCtrl.ScanType = BgndscanType; /* 0:Disable 1:partial scan 2:continuous scan */

	if (BgndscanType == TYPE_BGND_PARTIAL_SCAN) {	/* partial scan */
		pAd->BgndScanCtrl.PartialScanIntervalCount = DefaultBgndScanPerChInterval; /* First time hope trigger scan immediately. */
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_PARTIAL_SCAN, 0, NULL, BgndscanType);
	} else if (BgndscanType == TYPE_BGND_CONTINUOUS_SCAN) /* continuous scan */
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, BgndscanType);
	else if (BgndscanType == TYPE_BGND_CONTINUOUS_SCAN_SWITCH_CH) {/* continuous scan and then switch channel*/
		pAd->BgndScanCtrl.IsSwitchChannel = TRUE;
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, TYPE_BGND_CONTINUOUS_SCAN);
	} else {
		pAd->BgndScanCtrl.PartialScanIntervalCount = 0;
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_CNCL, 0, NULL, BgndscanType);
	}

	RTMP_MLME_HANDLER(pAd);
}

VOID BackgroundScanStartAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UINT8 ScanType = (UINT8)(Elem->Priv);
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ===> ScanType=%d\n", __func__, ScanType));
	FirstBgndScanChannel(pAd);
	pAd->BgndScanCtrl.ScanType = ScanType;
#ifdef GREENAP_SUPPORT
	greenap_suspend(pAd, greenap, AP_BACKGROUND_SCAN);
#endif /* GREENAP_SUPPORT */
	BackgroundScanNextChannel(pAd, ScanType); /* 0:Disable 1:partial scan 2:continuous scan */
}

VOID BackgroundScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	UINT8 ScanType = pAd->BgndScanCtrl.ScanType;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ===> ScanType=%d\n", __func__, ScanType));
	MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_TIMEOUT, 0, NULL, ScanType);
	RTMP_MLME_HANDLER(pAd);
}

VOID BackgroundScanTimeoutAction(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	/* UINT32	PCCA_TIME; */
	RTMP_REG_PAIR Reg[5];
	UINT8 ScanType = (UINT8)(Elem->Priv);
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s channel index=%d====ScanTyep=%d ==========>\n", __func__, pAd->BgndScanCtrl.ChannelIdx, ScanType));
	/* Updaet channel info */
	/* RTMP_IO_READ32(pAd->hdev_ctrl, 0x820fd248, &PCCA_TIME); */
	Reg[0].Register = 0x820fd248/*MIB_M1SDR16*/; /* PCCA Time */
	Reg[1].Register = 0x820fd24c/*MIB_M1SDR17*/; /* SCCA Time */
	Reg[2].Register = 0x820fd250/*MIB_M1SDR18*/; /* ED Time */
	Reg[3].Register = 0x820fd094/*MIB_M0SDR35*/; /* Bnad0 TxTime */
	Reg[4].Register = 0x820fd258/*MIB_M1SDR20*/; /* Mdrdy */
	MtCmdMultipleMacRegAccessRead(pAd, Reg, 5);
	pAd->BgndScanCtrl.BgndScanChList[pAd->BgndScanCtrl.ChannelIdx].PccaTime = Reg[0].Value;
	pAd->BgndScanCtrl.BgndScanChList[pAd->BgndScanCtrl.ChannelIdx].SccaTime = Reg[1].Value;
	pAd->BgndScanCtrl.BgndScanChList[pAd->BgndScanCtrl.ChannelIdx].EDCCATime = Reg[2].Value;
	pAd->BgndScanCtrl.BgndScanChList[pAd->BgndScanCtrl.ChannelIdx].Band0TxTime = Reg[3].Value;
	pAd->BgndScanCtrl.BgndScanChList[pAd->BgndScanCtrl.ChannelIdx].Mdrdy = Reg[4].Value;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ChannelIdx [%d].PCCA_TIME=%x, SCCA_TIEM=%x, EDCCA_TIME=%x, Band0TxTime=%x Mdrdy=%x ===============>\n", pAd->BgndScanCtrl.ChannelIdx, Reg[0].Value, Reg[1].Value, Reg[2].Value, Reg[3].Value, Reg[4].Value));
	NextBgndScanChannel(pAd, pAd->BgndScanCtrl.ScanChannel);

	if (pAd->BgndScanCtrl.ScanChannel == 0 || ScanType == TYPE_BGND_CONTINUOUS_SCAN) /* Ready to stop or continuous scan */
		BackgroundScanNextChannel(pAd, ScanType);
	else {	/* Next time partail scan */

		/* return to SynA only */
		if (ops->set_off_ch_scan)
			ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_STOP, ENUM_BGND_BGND_TYPE);

		/* Enable BF, MU */
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
		BfSwitch(pAd, 1);
#endif

#ifdef CFG_SUPPORT_MU_MIMO
		MuSwitch(pAd, 1);
#endif /* CFG_SUPPORT_MU_MIMO */

		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_DONE, 0, NULL, 0);
	}
}

VOID BackgroundScanWaitAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ===============>\n", __func__));
	/* Change state to Wait State. If all conditions match, will trigger partial scan */
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_WAIT;
}
VOID BackgroundScanPartialAction(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	UINT8 ScanType = (UINT8)(Elem->Priv);
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s =====ScanType=%d===>\n", __func__, ScanType));

	if (pAd->BgndScanCtrl.ScanChannel == 0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s First time===========>\n", __func__));
		FirstBgndScanChannel(pAd);
#ifdef GREENAP_SUPPORT
		greenap_suspend(pAd, greenap, AP_BACKGROUND_SCAN);
#endif /* GREENAP_SUPPORT */
	}

	BackgroundScanNextChannel(pAd, ScanType);
}

VOID BackgroundScanCancelAction(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN Cancelled;
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ===============>\n", __func__));
	/* Re-init related parameters */
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;/* Scan Stop */
	pAd->BgndScanCtrl.ScanChannel = 0;
	pAd->BgndScanCtrl.ScanType = 0;
	pAd->BgndScanCtrl.IsSwitchChannel = FALSE;
	RTMPCancelTimer(&pAd->BgndScanCtrl.BgndScanTimer, &Cancelled);
#ifdef GREENAP_SUPPORT
	greenap_resume(pAd, greenap, AP_BACKGROUND_SCAN);
#endif /* GREENAP_SUPPORT */
	/* RTMPCancelTimer(&pAd->BgndScanCtrl.BgndScanNextTimer, &Cancelled); */
}

VOID BackgroundScanNextChannel(
	IN PRTMP_ADAPTER pAd,
	IN UINT8		ScanType)
{
	RTMP_REG_PAIR Reg[5];
	MT_BGND_SCAN_NOTIFY BgScNotify;
	UCHAR	BestChannel = 0;
	MT_SWITCH_CHANNEL_CFG *CurrentSwChCfg;
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* Initialize */
	os_zero_mem(&BgScNotify, sizeof(MT_BGND_SCAN_NOTIFY));

	/* Restore switch channel configuration */
	CurrentSwChCfg = &(pAd->BgndScanCtrl.CurrentSwChCfg[0]);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Scan Channel=%d===============>\n", __func__, pAd->BgndScanCtrl.ScanChannel));

	if (pAd->BgndScanCtrl.ScanChannel == 0) {
		/* return to SynA only */
		if (ops->set_off_ch_scan)
			ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_STOP, ENUM_BGND_BGND_TYPE);

		/* Enable BF, MU */
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
		BfSwitch(pAd, 1);
#endif

#ifdef CFG_SUPPORT_MU_MIMO
		MuSwitch(pAd, 1);
#endif /* CFG_SUPPORT_MU_MIMO */


		GenerateGroupChannelList(pAd);
		BestChannel = BgndSelectBestChannel(pAd);
		pAd->BgndScanCtrl.BestChannel = BestChannel;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Best Channel=%d, IsSwitchChannel=%d Noisy=%d\n", BestChannel, pAd->BgndScanCtrl.IsSwitchChannel, pAd->BgndScanCtrl.Noisy));
		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;/* Scan Stop */
		pAd->BgndScanCtrl.ScanType = 0;/* Scan Complete. */

		if (BestChannel != CurrentSwChCfg->ControlChannel && pAd->BgndScanCtrl.IsSwitchChannel == TRUE) {
			pAd->BgndScanCtrl.IsSwitchChannel = FALSE;
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SWITCH_CHANNEL, 0, NULL, 0);
			RTMP_MLME_HANDLER(pAd);
		}

		pAd->BgndScanCtrl.IsSwitchChannel = FALSE;
#ifdef GREENAP_SUPPORT
		greenap_resume(pAd, greenap, AP_BACKGROUND_SCAN);
#endif /* GREENAP_SUPPORT */
	} else if (ScanType == TYPE_BGND_PARTIAL_SCAN) { /* Partial Scan */
		BgScNotify.NotifyFunc =  (0x2 << 5 | 0xf);
		BgScNotify.BgndScanStatus = 1;/* start */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Background scan Notify NotifyFunc=%x, Status=%d\n", BgScNotify.NotifyFunc, BgScNotify.BgndScanStatus));
		MtCmdBgndScanNotify(pAd, BgScNotify);

		/* Disable BF, MU */
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
		BfSwitch(pAd, 0);
#endif

#ifdef CFG_SUPPORT_MU_MIMO
		MuSwitch(pAd, 0);
#endif /* CFG_SUPPORT_MU_MIMO */

		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_LISTEN;
		/* Split into SynA + SynB */
		if (ops->set_off_ch_scan)
			ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_START, ENUM_BGND_BGND_TYPE);

		/* Read clear below CR */
		Reg[0].Register = 0x820fd248/*MIB_M1SDR16*/; /* PCCA Time */
		Reg[1].Register = 0x820fd24c/*MIB_M1SDR17*/; /* SCCA Time */
		Reg[2].Register = 0x820fd250/*MIB_M1SDR18*/; /* ED Time */
		Reg[3].Register = 0x820fd094/*MIB_M0SDR35*/; /* Bnad0 TxTime */
		Reg[4].Register = 0x820fd258/*MIB_M1SDR20*/; /* Mdrdy */
		MtCmdMultipleMacRegAccessRead(pAd, Reg, 5);
		RTMPSetTimer(&pAd->BgndScanCtrl.BgndScanTimer, pAd->BgndScanCtrl.ScanDuration); /* 200ms timer */
	} else if (pAd->BgndScanCtrl.ScanChannel == pAd->BgndScanCtrl.FirstChannel) {
		BgScNotify.NotifyFunc =  (0x2 << 5 | 0xf);
		BgScNotify.BgndScanStatus = 1;/* start */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Background scan Notify NotifyFunc=%x, Status=%d\n",
				 BgScNotify.NotifyFunc, BgScNotify.BgndScanStatus));
		MtCmdBgndScanNotify(pAd, BgScNotify);

		/* Disable BF, MU */
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
		BfSwitch(pAd, 0);
#endif

#ifdef CFG_SUPPORT_MU_MIMO
		MuSwitch(pAd, 0);
#endif /* CFG_SUPPORT_MU_MIMO */

		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_LISTEN;
		/* Split into SynA + SynB */
		if (ops->set_off_ch_scan)
			ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_START, ENUM_BGND_BGND_TYPE);

		/* Read clear below CR */
		Reg[0].Register = 0x820fd248/*MIB_M1SDR16*/; /* PCCA Time */
		Reg[1].Register = 0x820fd24c/*MIB_M1SDR17*/; /* SCCA Time */
		Reg[2].Register = 0x820fd250/*MIB_M1SDR18*/; /* ED Time */
		Reg[3].Register = 0x820fd094/*MIB_M0SDR35*/; /* Bnad0 TxTime */
		Reg[4].Register = 0x820fd258/*MIB_M1SDR20*/; /* Mdrdy */
		MtCmdMultipleMacRegAccessRead(pAd, Reg, 5);
		RTMPSetTimer(&pAd->BgndScanCtrl.BgndScanTimer, pAd->BgndScanCtrl.ScanDuration); /* 200ms timer */
	} else {
		/* RTMPSetTimer(&pAd->BgndScanCtrl.BgndScanTimer, 200); //200ms timer */
		/* Switch Band1 channel to pAd->BgndScanCtrl.ScanChannel */
		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_LISTEN;

		/* Switch channel of SynB */
		if (ops->set_off_ch_scan)
			ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_RUNNING, ENUM_BGND_BGND_TYPE);

		/* Read clear below CR */
		Reg[0].Register = 0x820fd248/*MIB_M1SDR16*/; /* PCCA Time */
		Reg[1].Register = 0x820fd24c/*MIB_M1SDR17*/; /* SCCA Time */
		Reg[2].Register = 0x820fd250/*MIB_M1SDR18*/; /* ED Time */
		Reg[3].Register = 0x820fd094/*MIB_M0SDR35*/; /* Bnad0 TxTime */
		Reg[4].Register = 0x820fd258/*MIB_M1SDR20*/; /* Mdrdy */
		MtCmdMultipleMacRegAccessRead(pAd, Reg, 5);
		RTMPSetTimer(&pAd->BgndScanCtrl.BgndScanTimer, pAd->BgndScanCtrl.ScanDuration); /* 500ms timer */
	}
}

VOID BackgroundSwitchChannelAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = NULL;
	wdev = pAd->wdev_list[0];
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_CS_ANN;
	printk("Switch to channel to %d\n", pAd->BgndScanCtrl.BestChannel);
	rtmp_set_channel(pAd, wdev, pAd->BgndScanCtrl.BestChannel);
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;
}

VOID BackgroundChannelSwitchAnnouncementAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	printk("Trigger Channel Switch Announcemnet IE\n");
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Trigger Channel Switch Announcemnet IE\n"));
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_CS_ANN; /* Channel switch annoncement. */
	/* HcUpdateCsaCntByChannel(pAd, pAd->BgndScanCtrl.BestChannel); */
	/* pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE; //For temporary */
}

#ifdef MT_DFS_SUPPORT
VOID DedicatedZeroWaitStartAction(
		IN RTMP_ADAPTER * pAd,
		IN MLME_QUEUE_ELEM * Elem)
{
	MT_BGND_SCAN_NOTIFY BgScNotify;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_RDD_DETEC;

	/* Initialize */
	os_zero_mem(&BgScNotify, sizeof(MT_BGND_SCAN_NOTIFY));

	BgScNotify.NotifyFunc =  (0x2 << 5 | 0xf);
	BgScNotify.BgndScanStatus = 1;/*start*/
	MtCmdBgndScanNotify(pAd, BgScNotify);

	/*Disable BF, MU*/
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
	/*BfSwitch(pAd, 0);*/
	DynamicTxBfDisable(pAd, TRUE);
#endif
#ifdef CFG_SUPPORT_MU_MIMO
	MuSwitch(pAd, 0);
#endif /* CFG_SUPPORT_MU_MIMO */

	/* Split into synA + synB */
	if (ops->set_off_ch_scan)
		ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_START, ENUM_BGND_DFS_TYPE);

	/*Start Band1 radar detection*/
	DfsDedicatedOutBandRDDStart(pAd);
}

VOID DedicatedZeroWaitRunningAction(
		IN RTMP_ADAPTER * pAd,
		IN MLME_QUEUE_ELEM * Elem)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	CHAR OutBandCh;

	DfsDedicatedOutBandRDDRunning(pAd);

	OutBandCh = GET_BGND_PARAM(pAd, OUTBAND_CH);
	if (OutBandCh == 0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] No available outband Channel \x1b[m \n",
			__FUNCTION__));
		DedicatedZeroWaitStop(pAd, FALSE);
		return;
	}

	/* Switch channel of synB */
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_RDD_DETEC;
	if (ops->set_off_ch_scan)
		ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_RUNNING, ENUM_BGND_DFS_TYPE);

	/* Start Band1 radar detection */
	DfsDedicatedOutBandRDDStart(pAd);
}

VOID DedicatedZeroWaitStop(
		IN RTMP_ADAPTER * pAd, BOOLEAN apply_cur_ch)
{
	CHAR in_band_ch = 0;
	BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (apply_cur_ch == TRUE) {
		in_band_ch = GET_BGND_PARAM(pAd, INBAND_CH);
	} else {
		in_band_ch = GET_BGND_PARAM(pAd, ORI_INBAND_CH);
	}

	if (!IS_SUPPORT_DEDICATED_ZEROWAIT_DFS(pAd))
		return;

	if (!GET_BGND_STATE(pAd, BGND_RDD_DETEC))
		return;

	if (in_band_ch == 0)
		return;

	BgndScanCtrl->BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;

	DfsDedicatedOutBandRDDStop(pAd);

	/* return to SynA only */
	if (ops->set_off_ch_scan)
		ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_STOP, ENUM_BGND_DFS_TYPE);

	/*Enable BF, MU*/
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
	/*BfSwitch(pAd, 1);*/
	DynamicTxBfDisable(pAd, FALSE);
#endif

#ifdef CFG_SUPPORT_MU_MIMO
	MuSwitch(pAd, 1);
#endif /* CFG_SUPPORT_MU_MIMO */

}
#endif

VOID BackgroundScanTest(
	IN PRTMP_ADAPTER pAd,
	IN MT_BGND_SCAN_CFG BgndScanCfg)
{
	/* Send Commad to MCU */
	MtCmdBgndScan(pAd, BgndScanCfg);
}

VOID ChannelQualityDetection(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 ChBusyTime = 0;
	UINT32 MyTxAirTime = 0;
	UINT32 MyRxAirTime = 0;
	UCHAR BandIdx = 0;
	UINT32      lv0, lv1, lv2, lv3, lv4, lv5, lv6, lv7, lv8, lv9, lv10, CrValue;
	UCHAR Noisy = 0;
	UINT32 TotalIPI = 0;
	/* RTMP_REG_PAIR Reg[11]; */
	/* Phase 1: No traffic stat */
	/* Check IPI & Channel Busy Time */
	BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
	ChBusyTime = pAd->OneSecMibBucket.ChannelBusyTime[BandIdx];
	MyTxAirTime = pAd->OneSecMibBucket.MyTxAirtime[BandIdx];
	MyRxAirTime = pAd->OneSecMibBucket.MyRxAirtime[BandIdx];
	/* 1. Check Open enviroment via IPI (Band0) */
	HW_IO_READ32(pAd->hdev_ctrl, 0x12250, &lv0);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12254, &lv1);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12258, &lv2);
	HW_IO_READ32(pAd->hdev_ctrl, 0x1225c, &lv3);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12260, &lv4);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12264, &lv5);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12268, &lv6);
	HW_IO_READ32(pAd->hdev_ctrl, 0x1226c, &lv7);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12270, &lv8);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12274, &lv9);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12278, &lv10);
	TotalIPI = lv0 + lv1 + lv2 + lv3 + lv4 + lv5 + lv6 + lv7 + lv8 + lv9 + lv10;

	if (TotalIPI != 0)
		Noisy = ((lv9 + lv10) * 100 / (TotalIPI));

	pAd->BgndScanCtrl.Noisy = Noisy;
	pAd->BgndScanCtrl.IPIIdleTime = TotalIPI;
	/* MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, */
	/* ("%s Band0:lv0~5 %d, %d, %d, %d, %d, %d  lv6~10 %d, %d, %d, %d, %d tatol=%d, Noisy=%d, BusyTime=%d, MyTxAir=%d, MyRxAir=%d\n", */
	/* __func__, lv0, lv1, lv2, lv3, lv4, lv5, lv6, lv7, lv8, lv9, lv10, TotalIPI, Noisy, ChBusyTime, MyTxAirTime, MyRxAirTime)); */
	HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_12, &CrValue);
	CrValue |= (1 << B0IrpiSwCtrlOnlyOffset); /*29*/
	HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_12, CrValue);/* Cleaer */
	HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_12, CrValue);/* Trigger again */

	if (BgndScanCtrl->BgndScanStatMachine.CurrState == BGND_SCAN_LISTEN)
		return;

	/* MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Noise =%d, ChBusy=%d, MyTxAirTime=%d, MyRxAirTime=%d\n", Noisy, ChBusyTime, MyTxAirTime, MyRxAirTime)); */
	if ((pAd->BgndScanCtrl.DriverTrigger) && ((Noisy > pAd->BgndScanCtrl.NoisyTH) && (TotalIPI > pAd->BgndScanCtrl.IPIIdleTimeTH))) {
		if (BgndScanCtrl->BgndScanStatMachine.CurrState == BGND_SCAN_IDLE) {
			pAd->BgndScanCtrl.IsSwitchChannel = TRUE;
			pAd->BgndScanCtrl.ScanType = TYPE_BGND_CONTINUOUS_SCAN;
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, TYPE_BGND_CONTINUOUS_SCAN);
			RTMP_MLME_HANDLER(pAd);
		} else if (BgndScanCtrl->BgndScanStatMachine.CurrState == BGND_SCAN_WAIT) {
			pAd->BgndScanCtrl.PartialScanIntervalCount = 0;
			pAd->BgndScanCtrl.IsSwitchChannel = TRUE;
			pAd->BgndScanCtrl.ScanType = TYPE_BGND_CONTINUOUS_SCAN;
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_CNCL, 0, NULL, 0);
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, TYPE_BGND_CONTINUOUS_SCAN);
			RTMP_MLME_HANDLER(pAd);
			/* BackgroundScanStart(pAd, 2); */
		}
	} else if (BgndScanCtrl->BgndScanStatMachine.CurrState == BGND_SCAN_WAIT) {
		pAd->BgndScanCtrl.PartialScanIntervalCount++;

		if  (pAd->BgndScanCtrl.PartialScanIntervalCount >= pAd->BgndScanCtrl.PartialScanInterval
			 && (MyTxAirTime + MyRxAirTime < DefaultMyAirtimeUsageThreshold)) {
			pAd->BgndScanCtrl.PartialScanIntervalCount = 0;
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, TYPE_BGND_PARTIAL_SCAN);
			RTMP_MLME_HANDLER(pAd);
		}
	}
}


#if OFF_CH_SCAN_SUPPORT
VOID mt_off_ch_scan(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR reason,
	IN UCHAR bgnd_scan_type)
{
	EXT_CMD_OFF_CH_SCAN_CTRL_T offch_cmd_cfg;
	MT_BGND_SCAN_NOTIFY bgnd_scan_notify;
	MT_SWITCH_CHANNEL_CFG *curnt_swchcfg;
	UCHAR rx_stream_num = 0;
	UCHAR rx_path = 0;
	UCHAR rx_idx = 0;
	UCHAR control_ch_inband = 0, central_ch_inband = 0, bw_inband = 0;
	UCHAR control_ch_outband = 0, central_ch_outband = 0, bw_outband = 0;
#ifdef MT_DFS_SUPPORT
	CHAR out_band_ch = GET_BGND_PARAM(pAd, OUTBAND_CH);
	CHAR out_band_bw = GET_BGND_PARAM(pAd, OUTBAND_BW);
	CHAR in_band_ch = GET_BGND_PARAM(pAd, INBAND_CH);
	CHAR in_band_bw = GET_BGND_PARAM(pAd, INBAND_BW);
#endif

	/* Initialize */
	os_zero_mem(&offch_cmd_cfg, sizeof(EXT_CMD_OFF_CH_SCAN_CTRL_T));
	os_zero_mem(&bgnd_scan_notify, sizeof(MT_BGND_SCAN_NOTIFY));

	/* Restore switch channel configuration */
	curnt_swchcfg = &(pAd->BgndScanCtrl.CurrentSwChCfg[0]);

	if (bgnd_scan_type == ENUM_BGND_BGND_TYPE) {
		/* SynA */
		control_ch_inband = curnt_swchcfg->ControlChannel;
		central_ch_inband = curnt_swchcfg->CentralChannel;
		bw_inband = curnt_swchcfg->Bw;

		/* SynB */
		control_ch_outband = pAd->BgndScanCtrl.ScanChannel;
		central_ch_outband = pAd->BgndScanCtrl.ScanCenChannel;
		bw_outband = pAd->BgndScanCtrl.ScanBW;
	}
#ifdef MT_DFS_SUPPORT
	else if (bgnd_scan_type == ENUM_BGND_DFS_TYPE) {
		/* SynA */
		control_ch_inband = in_band_ch;
		central_ch_inband = DfsPrimToCent(in_band_ch, in_band_bw);
		bw_inband = in_band_bw;

		/* SynB */
		control_ch_outband = out_band_ch;
		central_ch_outband = DfsPrimToCent(out_band_ch, out_band_bw);
		bw_outband = out_band_bw;
	}
#endif

	switch (reason) {
	case CH_SWITCH_BACKGROUND_SCAN_STOP:
		/* Return to SynA (3x3) only */
		/* RxStream to RxPath */
		rx_stream_num = curnt_swchcfg->RxStream;

		if (rx_stream_num > 3) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s():illegal RxStreamNums(%d)\n",
					  __func__, rx_stream_num));
			rx_stream_num = 3;
		}

		for (rx_idx = 0; rx_idx < rx_stream_num; rx_idx++)
			rx_path |= 1 << rx_idx;

		/* Fill synA offch_cmd_cfg */
		offch_cmd_cfg.work_prim_ch = control_ch_inband;
		offch_cmd_cfg.work_cntrl_ch = central_ch_inband;
		offch_cmd_cfg.work_bw = bw_inband;
		offch_cmd_cfg.work_tx_strm_pth = curnt_swchcfg->TxStream;
		offch_cmd_cfg.work_rx_strm_pth = rx_path;
		offch_cmd_cfg.dbdc_idx = 1;
		offch_cmd_cfg.scan_mode = off_ch_scan_mode_stop;
		offch_cmd_cfg.is_aband = 1;
		offch_cmd_cfg.off_ch_scn_type = off_ch_scan_simple_rx;

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s: work_prim_ch:%d work_bw:%d work_central_ch:%d\n",
				 __func__, offch_cmd_cfg.work_prim_ch, offch_cmd_cfg.work_bw, offch_cmd_cfg.work_cntrl_ch));

		mt_cmd_off_ch_scan(pAd, &offch_cmd_cfg);

		/* Notify RA background scan stop */
		bgnd_scan_notify.NotifyFunc = (curnt_swchcfg->TxStream << 5 | 0xf);
		bgnd_scan_notify.BgndScanStatus = 0; /* stop */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Background scan Notify NotifyFunc=%x, Status=%d\n",
				 bgnd_scan_notify.NotifyFunc, bgnd_scan_notify.BgndScanStatus));

		MtCmdBgndScanNotify(pAd, bgnd_scan_notify);
		break;

	case CH_SWITCH_BACKGROUND_SCAN_START:
		/* Split into synA + synB */
		/* Fill in ext_cmd_param */
		offch_cmd_cfg.mntr_prim_ch = control_ch_outband;
		offch_cmd_cfg.mntr_cntrl_ch = central_ch_outband;
		offch_cmd_cfg.mntr_bw = bw_outband;
		offch_cmd_cfg.mntr_tx_strm_pth = 1;
		offch_cmd_cfg.mntr_rx_strm_pth = 0x4; /* WF2 only */

		offch_cmd_cfg.work_prim_ch = control_ch_inband;
		offch_cmd_cfg.work_cntrl_ch = central_ch_inband;
		offch_cmd_cfg.work_bw = bw_inband;
		offch_cmd_cfg.work_tx_strm_pth = 2;
		offch_cmd_cfg.work_rx_strm_pth = 0x3; /* WF0 and WF1 */

		offch_cmd_cfg.dbdc_idx = 1;
		offch_cmd_cfg.scan_mode = off_ch_scan_mode_start;
		offch_cmd_cfg.is_aband = 1;
		offch_cmd_cfg.off_ch_scn_type = off_ch_scan_simple_rx;

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s: mntr_ch:%d mntr_bw:%d mntr_central_ch:%d\n",
				 __func__, offch_cmd_cfg.mntr_prim_ch, offch_cmd_cfg.mntr_bw, offch_cmd_cfg.mntr_cntrl_ch));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s: work_prim_ch:%d work_bw:%d work_central_ch:%d\n",
				 __func__, offch_cmd_cfg.work_prim_ch, offch_cmd_cfg.work_bw, offch_cmd_cfg.work_cntrl_ch));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s: dbdc_idx:%d scan_mode:%d is_aband:%d\n",
				 __func__, offch_cmd_cfg.dbdc_idx, offch_cmd_cfg.scan_mode, offch_cmd_cfg.is_aband));

		mt_cmd_off_ch_scan(pAd, &offch_cmd_cfg);

		break;

	case CH_SWITCH_BACKGROUND_SCAN_RUNNING:
		/* Switch channel of synB */
		/* Fill in ext_cmd_param */
		offch_cmd_cfg.mntr_prim_ch = control_ch_outband;
		offch_cmd_cfg.mntr_cntrl_ch = central_ch_outband;
		offch_cmd_cfg.mntr_bw = bw_outband;
		offch_cmd_cfg.mntr_tx_strm_pth = 1;
		offch_cmd_cfg.mntr_rx_strm_pth = 0x4; /* WF2 only */

		offch_cmd_cfg.dbdc_idx = 1;
		offch_cmd_cfg.scan_mode = off_ch_scan_mode_running;
		offch_cmd_cfg.is_aband = 1;
		offch_cmd_cfg.off_ch_scn_type = off_ch_scan_simple_rx;

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s: mntr_ch:%d mntr_bw:%d mntr_central_ch:%d\n",
				 __func__, offch_cmd_cfg.mntr_prim_ch, offch_cmd_cfg.mntr_bw, offch_cmd_cfg.mntr_cntrl_ch));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s: dbdc_idx:%d scan_mode:%d is_aband:%d\n",
				 __func__, offch_cmd_cfg.dbdc_idx, offch_cmd_cfg.scan_mode, offch_cmd_cfg.is_aband));

		mt_cmd_off_ch_scan(pAd, &offch_cmd_cfg);

		break;

	default:
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s ERROR reason=%d\n", __func__, reason));
		break;
	}

}
#endif


#if defined(MT_MAC) && defined(TXBF_SUPPORT)
VOID BfSwitch(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR enabled)
{
	INT idx, start_idx, end_idx, wtbl_len;
	UINT32 wtbl_offset, addr;
	UCHAR *wtbl_raw_dw = NULL;
	struct wtbl_entry wtbl_ent;
	struct wtbl_struc *wtbl = &wtbl_ent.wtbl;
	struct wtbl_tx_rx_cap *trx_cap = &wtbl->trx_cap;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	/* Search BF STA and record it. */
	start_idx = 1;
	end_idx = (cap->WtblHwNum - 1);

	if (enabled == 0) { /* Disable */
		wtbl_len = sizeof(WTBL_STRUC);
		os_alloc_mem(pAd, (UCHAR **)&wtbl_raw_dw, wtbl_len);

		if (!wtbl_raw_dw) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s():AllocMem fail!\n",
					  __func__));
			return;
		}

		for (idx = start_idx; idx <= end_idx; idx++) {
			wtbl_ent.wtbl_idx = idx;
			wtbl_ent.wtbl_addr = pAd->mac_ctrl.wtbl_base_addr[0] + idx * pAd->mac_ctrl.wtbl_entry_size[0];

			/* Read WTBL Entries */
			for (wtbl_offset = 0; wtbl_offset <= wtbl_len; wtbl_offset += 4) {
				addr = wtbl_ent.wtbl_addr + wtbl_offset;
				HW_IO_READ32(pAd->hdev_ctrl, addr, (UINT32 *)(&wtbl_raw_dw[wtbl_offset]));
			}

			NdisCopyMemory((UCHAR *)wtbl, &wtbl_raw_dw[0], sizeof(struct wtbl_struc));

			if (trx_cap->wtbl_d2.field.tibf == 1)
				pAd->BgndScanCtrl.BFSTARecord[idx] = 1; /* iBF */
			else if (trx_cap->wtbl_d2.field.tebf == 1)
				pAd->BgndScanCtrl.BFSTARecord[idx] = 2; /* eBF */
			else
				pAd->BgndScanCtrl.BFSTARecord[idx] = 0; /* No BF */

			if (pAd->BgndScanCtrl.BFSTARecord[idx] != 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s():Disable wcid %d BF!\n",
						  __func__, idx));
				CmdTxBfTxApplyCtrl(pAd, idx, 0, 0, 0, 0); /* Disable BF */
			}
		}

		os_free_mem(wtbl_raw_dw);
	} else {/* enable */
		for (idx = start_idx; idx <= end_idx; idx++) {
			if (pAd->BgndScanCtrl.BFSTARecord[idx] == 1) {/* iBF */
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s():Enable wcid %d iBF!\n", __func__, idx));
				CmdTxBfTxApplyCtrl(pAd, idx, 0, 1, 0, 0); /* enable iBF */
			} else if (pAd->BgndScanCtrl.BFSTARecord[idx] == 2) { /* eBF */
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s():Enable wcid %d eBF!\n", __func__, idx));
				CmdTxBfTxApplyCtrl(pAd, idx, 1, 0, 0, 0); /* enable eBF */
			}
		}
	}
}
#endif
#ifdef CFG_SUPPORT_MU_MIMO
VOID MuSwitch(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR enabled)
{
	if (enabled == 0)   /* Disable */
		SetMuEnableProc(pAd, "0");
	else   /* Enable */
		SetMuEnableProc(pAd, "1");
}
#endif /* CFG_SUPPORT_MU_MIMO */
