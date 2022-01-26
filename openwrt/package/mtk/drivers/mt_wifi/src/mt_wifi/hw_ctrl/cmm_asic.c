/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_asic.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/


#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Cmm_asic.tmh"
#endif
#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#include "mcu/mt_cmd.h"
#endif
#include "hdev/hdev.h"
#define MCAST_WCID_TO_REMOVE 0 /* Pat: TODO */


static char *hif_2_str[] = {"HIF_RTMP", "HIF_RLT", "HIF_MT", "Unknown"};
VOID AsicNotSupportFunc(RTMP_ADAPTER *pAd, const RTMP_STRING *caller)
{
	RTMP_STRING *str;
	INT32 hif_type = GET_HIF_TYPE(pAd);

	if (hif_type <= HIF_MAX)
		str = hif_2_str[hif_type];
	else
		str = hif_2_str[HIF_MAX];

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): NotSupportedFunc for this arch(%s)!\n",
			 caller, str));
}

#ifndef	COMPOS_TESTMODE_WIN
UINT32 AsicGetCrcErrCnt(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicGetCrcErrCnt(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}


UINT32 AsicGetCCACnt(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicGetCCACnt(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}


UINT32 AsicGetChBusyCnt(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicGetChBusyCnt(pAd, BandIdx);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

INT AsicSetAutoFallBack(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetAutoFallBack(pAd, enable);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicAutoFallbackInit(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicAutoFallbackInit(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


VOID AsicUpdateRtsThld(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT32 pkt_num,
	UINT32 length)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
#ifdef CONFIG_ATE

		if (ATE_ON(pAd))
			return;

#endif /* CONFIG_ATE */

		if (arch_ops->archUpdateRtsThld)
			return arch_ops->archUpdateRtsThld(pAd, wdev, pkt_num, length);
	}

	AsicNotSupportFunc(pAd, __func__);
}


/*
 * ========================================================================
 *
 * Routine Description:
 * Set MAC register value according operation mode.
 * OperationMode AND bNonGFExist are for MM and GF Proteciton.
 * If MM or GF mask is not set, those passing argument doesn't not take effect.
 *
 * Operation mode meaning:
 * = 0 : Pure HT, no preotection.
 * = 0x01; there may be non-HT devices in both the control and extension channel, protection is optional in BSS.
 * = 0x10: No Transmission in 40M is protected.
 * = 0x11: Transmission in both 40M and 20M shall be protected
 * if (bNonGFExist)
 * we should choose not to use GF. But still set correct ASIC registers.
 * ========================================================================
 */
VOID AsicUpdateProtect(
	IN PRTMP_ADAPTER pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
#ifdef CONFIG_ATE

		if (ATE_ON(pAd))
			return;

#endif /* CONFIG_ATE */
		HwCtrlSetFlag(pAd, HWFLAG_ID_UPDATE_PROTECT);
		return;
	}

#endif
}

/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicSwitchChannel(RTMP_ADAPTER *pAd, UCHAR band_idx, struct freq_oper *oper, BOOLEAN bScan)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_SWITCH_CHANNEL_CFG SwChCfg;

		os_zero_mem(&SwChCfg, sizeof(MT_SWITCH_CHANNEL_CFG));
		SwChCfg.bScan = bScan;
		SwChCfg.CentralChannel = oper->cen_ch_1;
		SwChCfg.BandIdx = band_idx;

	/* channel_band 5G as 1*/
	if (oper->cen_ch_1 >= 36 && oper->cen_ch_1 <= 165) {
		if (SwChCfg.Channel_Band == 0) {
			SwChCfg.Channel_Band = 1;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\x1b[41m%s(): 5G Channel:%d, then must be Channel_Band:%d !!\x1b[m\n",
				__func__, oper->cen_ch_1, SwChCfg.Channel_Band));
		}
	}

		SwChCfg.RxStream = pAd->Antenna.field.RxPath;
		SwChCfg.TxStream = pAd->Antenna.field.TxPath;

#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			if (SwChCfg.BandIdx == DBDC_BAND0) {
				SwChCfg.RxStream = pAd->dbdc_band0_rx_path;
				SwChCfg.TxStream = pAd->dbdc_band0_tx_path;
			} else {
				SwChCfg.RxStream = pAd->dbdc_band1_rx_path;
				SwChCfg.TxStream = pAd->dbdc_band1_tx_path;
			}
		}
#endif
#ifdef ANTENNA_CONTROL_SUPPORT
		if (pAd->bAntennaSetAPEnable[band_idx] == 1) {
			pAd->bAntennaSetAPEnable[band_idx] = 0;
		}
		if (pAd->RxStream[band_idx] && pAd->TxStream[band_idx]) {
			SwChCfg.RxStream = pAd->RxStream[band_idx];
			SwChCfg.TxStream = pAd->TxStream[band_idx];
		}
#endif
		SwChCfg.Bw = oper->bw;
		SwChCfg.ControlChannel = oper->prim_ch;
		SwChCfg.OutBandFreq = 0;
#ifdef DOT11_VHT_AC
		SwChCfg.ControlChannel2 = oper->cen_ch_2;
#endif /* DOT11_VHT_AC */
#ifdef MT_DFS_SUPPORT
		SwChCfg.bDfsCheck = DfsSwitchCheck(pAd, SwChCfg.ControlChannel, band_idx);
#endif

		/*update radio info to band*/
		if (!bScan) {
		}

		MtAsicSwitchChannel(pAd, SwChCfg);
	}

#endif
}


/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */

#ifdef CONFIG_STA_SUPPORT
/*
 * ==========================================================================
 * Description:
 * put PHY to sleep here, and set next wakeup timer. PHY doesn't not wakeup
 * automatically. Instead, MCU will issue a TwakeUpInterrupt to host after
 * the wakeup timer timeout. Driver has to issue a separate command to wake
 * PHY up.
 *
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicSleepAutoWakeup(PRTMP_ADAPTER pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
	USHORT	TbttNumToNextWakeUp = 0;
	USHORT	NextDtim = pStaCfg->DtimPeriod;
	ULONG	Now = 0;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d): pStaCfg(0x%p)\n",
			 __func__, __LINE__, pStaCfg));
	NdisGetSystemUpTime(&Now);
	NextDtim -= (USHORT)(Now - pStaCfg->LastBeaconRxTime) / pAd->CommonCfg.BeaconPeriod;
	pStaCfg->ThisTbttNumToNextWakeUp = pStaCfg->DefaultListenCount;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) && (TbttNumToNextWakeUp > NextDtim))
		pStaCfg->ThisTbttNumToNextWakeUp = NextDtim;

	/* if WMM-APSD is failed, try to disable following line*/
	ASIC_STA_SLEEP_AUTO_WAKEUP(pAd, pStaCfg);
}

/*
 * ==========================================================================
 * Description:
 * AsicWakeup() is used whenever Twakeup timer (set via AsicSleepAutoWakeup)
 * expired.
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 * ==========================================================================
 */
VOID AsicWakeup(RTMP_ADAPTER *pAd, BOOLEAN bFromTx, PSTA_ADMIN_CONFIG pStaCfg)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d): pStaCfg(0x%p)\n",
			 __func__, __LINE__, pStaCfg));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("--> AsicWakeup\n"));
	ASIC_STA_WAKEUP(pAd, bFromTx, pStaCfg);
}
#endif /* CONFIG_STA_SUPPORT */



#endif/*COMPOS_TESTMODE_WIN*/

/* Replaced by AsicDevInfoUpdate() */
INT AsicSetDevMac(RTMP_ADAPTER *pAd, UCHAR *addr, UCHAR omac_idx)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Set OwnMac=%02x:%02x:%02x:%02x:%02x:%02x\n",
			 __func__, PRINT_MAC(addr)));
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetDevMac(pAd, omac_idx, addr, 0, TRUE, DEVINFO_ACTIVE_FEATURE);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

BOOLEAN AsicDisableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archDisableBeacon)
		return arch_ops->archDisableBeacon(pAd, wdev);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

BOOLEAN AsicEnableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archEnableBeacon)
		return arch_ops->archEnableBeacon(pAd, wdev);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

BOOLEAN AsicUpdateBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev, UINT16 FrameLen, UCHAR UpdatePktType)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archUpdateBeacon)
		return arch_ops->archUpdateBeacon(pAd, wdev, FrameLen, UpdatePktType);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
INT AsicSetReptFuncEnable(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetReptFuncEnable) {
		if (enable)
			RepeaterCtrlInit(pAd);
		else
			RepeaterCtrlExit(pAd);

		return arch_ops->archSetReptFuncEnable(pAd, enable);
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


VOID AsicInsertRepeaterEntry(RTMP_ADAPTER *pAd, UCHAR CliIdx, UCHAR *pAddr)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archInsertRepeaterEntry)
		arch_ops->archInsertRepeaterEntry(pAd, CliIdx, pAddr);
	else
		AsicNotSupportFunc(pAd, __func__);
}


VOID AsicRemoveRepeaterEntry(RTMP_ADAPTER *pAd, UCHAR CliIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archRemoveRepeaterEntry)
		arch_ops->archRemoveRepeaterEntry(pAd, CliIdx);
	else
		AsicNotSupportFunc(pAd, __func__);
}


VOID AsicInsertRepeaterRootEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN UCHAR *pAddr,
	IN UCHAR ReptCliIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archInsertRepeaterRootEntry)
		arch_ops->archInsertRepeaterRootEntry(pAd, Wcid, pAddr, ReptCliIdx);
	else
		AsicNotSupportFunc(pAd, __func__);
}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */


INT AsicSetRxFilter(RTMP_ADAPTER *pAd)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	MT_RX_FILTER_CTRL_T RxFilter;

	os_zero_mem(&RxFilter, sizeof(MT_RX_FILTER_CTRL_T));
#ifdef CONFIG_STA_SUPPORT

	if (MONITOR_ON(pAd))
#else
	if (FALSE)
#endif
	{
		RxFilter.bPromiscuous = TRUE;
	} else {
		RxFilter.bPromiscuous = FALSE;
		RxFilter.bFrameReport = FALSE;
		RxFilter.filterMask = RX_NDPA | RX_NOT_OWN_BTIM | RX_NOT_OWN_UCAST |
							  RX_RTS | RX_CTS | RX_CTRL_RSV | RX_BC_MC_DIFF_BSSID_A2 |
							  RX_BC_MC_DIFF_BSSID_A3 | RX_BC_MC_OWN_MAC_A3 | RX_PROTOCOL_VERSION |
							  RX_FCS_ERROR;
	} /*Endof Monitor ON*/

	if (arch_ops->archSetRxFilter) {
		INT ret = 0;

		ret = arch_ops->archSetRxFilter(pAd, RxFilter);
		return ret;
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


#ifdef DOT11_N_SUPPORT
INT AsicSetRDG(RTMP_ADAPTER *pAd,
			   UCHAR wlan_idx,
			   UCHAR band_idx,
			   UCHAR init,
			   UCHAR resp)
{
	INT ret = FALSE;
	INT bSupport = FALSE;
	BOOLEAN is_en;
	MT_RDG_CTRL_T rdg;
	RTMP_ARCH_OP *arch_op = hc_get_arch_ops(pAd->hdev_ctrl);

	is_en = (init && resp) ? TRUE : FALSE;

	if (arch_op->archSetRDG) {
		bSupport = TRUE;
		rdg.WlanIdx = wlan_idx;
		rdg.BandIdx = band_idx;
		rdg.Init = init;
		rdg.Resp = resp;
		rdg.Txop = (is_en) ? (0x80) : (0x60);
		rdg.LongNav = (is_en) ? (1) : (0);
		ret = arch_op->archSetRDG(pAd, &rdg);
	}

	if (ret == TRUE) {
		if (is_en)
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
		else
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
	}

	if (!bSupport)
		AsicNotSupportFunc(pAd, __func__);

	return ret;
}
#endif /* DOT11_N_SUPPORT */


/*
 * ========================================================================
 * Routine Description:
 * Set/reset MAC registers according to bPiggyBack parameter
 *
 * Arguments:
 * pAd         - Adapter pointer
 * bPiggyBack  - Enable / Disable Piggy-Back
 *
 * Return Value:
 * None
 *
 * ========================================================================
 */
VOID AsicSetPiggyBack(RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetPiggyBack(pAd, bPiggyBack);

#endif
	AsicNotSupportFunc(pAd, __func__);
}

INT AsicSetPreTbtt(RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR HwBssidIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetPreTbtt) {
		arch_ops->archSetPreTbtt(pAd, enable, HwBssidIdx);
		return TRUE;
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicSetGPTimer(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetGPTimer(pAd, enable, timeout);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetChBusyStat(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetChBusyStat(pAd, enable);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicGetTsfTime(
	RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archGetTsfTime)
		return arch_ops->archGetTsfTime(pAd, high_part, low_part, HwBssidIdx);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicDisableSync(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archDisableSync)
		arch_ops->archDisableSync(pAd, HWBssidIdx);
	else
		AsicNotSupportFunc(pAd, __func__);
}

VOID AsicSetSyncModeAndEnable(
	struct _RTMP_ADAPTER *pAd,
	USHORT BeaconPeriod,
	UCHAR HWBssidIdx,
	UCHAR OPMode)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetSyncModeAndEnable)
		arch_ops->archSetSyncModeAndEnable(pAd, BeaconPeriod, HWBssidIdx, OPMode);
	else
		AsicNotSupportFunc(pAd, __func__);
}

#ifdef CONFIG_STA_SUPPORT
/*
 * ==========================================================================
 * Description:
 * Note:
 * BEACON frame in shared memory should be built ok before this routine
 * can be called. Otherwise, a garbage frame maybe transmitted out every
 * Beacon period.
 *
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicEnableIbssSync(struct _RTMP_ADAPTER *pAd,
						USHORT BeaconPeriod,
						UCHAR HWBssidIdx,
						UCHAR OPMode)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archEnableIbssSync)
		arch_ops->archEnableIbssSync(pAd, BeaconPeriod, HWBssidIdx, OPMode);
	else
		AsicNotSupportFunc(pAd, __func__);
}
#endif /* CONFIG_STA_SUPPORT */



INT AsicSetWmmParam(RTMP_ADAPTER *pAd, UCHAR idx, UINT32 ac, UINT32 type, UINT32 val)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetWmmParam) {
		return arch_ops->archSetWmmParam(pAd, idx, ac, type, val);
	}
	AsicNotSupportFunc(pAd, __func__);
	return NDIS_STATUS_FAILURE;
}


/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicSetEdcaParm(RTMP_ADAPTER *pAd, struct wmm_entry *entry, struct wifi_dev *wdev)
{
	INT i;
	UCHAR EdcaIdx = wdev->EdcaIdx;
	EDCA_PARM *pEdca = NULL;
	PEDCA_PARM pEdcaParm = &entry->edca;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
#ifdef CONFIG_STA_SUPPORT
	MAC_TABLE_ENTRY *pEntry = NULL;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	pEntry = GetAssociatedAPByWdev(pAd, wdev);
#endif

	if (EdcaIdx >= WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): EdcaIdx >= 4\n", __func__));
		return;
	}

	pEdca = &pAd->CommonCfg.APEdcaParm[EdcaIdx];

	if ((pEdcaParm == NULL) || (pEdcaParm->bValid == FALSE)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): NoEDCAParam\n", __func__));
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WMM_INUSED);

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) ||
				IS_ENTRY_PEER_AP(&pAd->MacTab.Content[i]) ||
				IS_ENTRY_REPEATER(&pAd->MacTab.Content[i]))
				/*check clear for this bss only*/
				if (pAd->MacTab.Content[i].wdev == wdev)
					CLIENT_STATUS_CLEAR_FLAG(&pAd->MacTab.Content[i], fCLIENT_STATUS_WMM_CAPABLE);
		}

		os_zero_mem(pEdca, sizeof(EDCA_PARM));
	} else {
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_WMM_INUSED);
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (INFRA_ON(pStaCfg))
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
		}
#endif /* CONFIG_STA_SUPPORT */
		os_move_mem(pEdca, pEdcaParm, sizeof(EDCA_PARM));

		if (!ADHOC_ON(pAd)) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("EDCA [#%d]: AIFSN CWmin CWmax  TXOP(us)  ACM, WMM Set: %d, BandIdx: %d\n",
					  pEdcaParm->EdcaUpdateCount,
					  entry->wmm_set,
					  entry->dbdc_idx));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("     AC_BE      %2d     %2d     %2d      %4d     %d\n",
					 pEdcaParm->Aifsn[0],
					 pEdcaParm->Cwmin[0],
					 pEdcaParm->Cwmax[0],
					 pEdcaParm->Txop[0] << 5,
					 pEdcaParm->bACM[0]));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("     AC_BK      %2d     %2d     %2d      %4d     %d\n",
					 pEdcaParm->Aifsn[1],
					 pEdcaParm->Cwmin[1],
					 pEdcaParm->Cwmax[1],
					 pEdcaParm->Txop[1] << 5,
					 pEdcaParm->bACM[1]));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("     AC_VI      %2d     %2d     %2d      %4d     %d\n",
					 pEdcaParm->Aifsn[2],
					 pEdcaParm->Cwmin[2],
					 pEdcaParm->Cwmax[2],
					 pEdcaParm->Txop[2] << 5,
					 pEdcaParm->bACM[2]));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("     AC_VO      %2d     %2d     %2d      %4d     %d\n",
					 pEdcaParm->Aifsn[3],
					 pEdcaParm->Cwmin[3],
					 pEdcaParm->Cwmax[3],
					 pEdcaParm->Txop[3] << 5,
					 pEdcaParm->bACM[3]));
		}
#ifdef APCLI_SUPPORT
		/* This is added for TGn 5.2.30 */
		if (pAd->bApCliCertTest && pStaCfg && wdev->wdev_type == WDEV_TYPE_STA) {
			/* SSID for TGn TC 5.2.30 */
			UCHAR Ssid[] = "Bg(*^J78";
			UCHAR SsidEqual = 0;
			/* SSID for TGn TC 5.2.43 */
			UCHAR Ssid2[] = "AP1-5.2.43";
			UCHAR Ssid2Equal = 0;
			/* SSID for TGac TC 5.2.61 */
			UCHAR Ssid3[] = "VHT-5.2.61-AP1";
			UCHAR Ssid3Equal = 0;
			/* SSID for TGac TC 5.2.28 */
			UCHAR Ssid4[] = "VHT-5.2.28";
			UCHAR Ssid4Equal = 0;
			/* SSID for TGn TC 5.2.33 */
			UCHAR Ssid5[] = "5.2.33";
			UCHAR Ssid5Equal = 0;

			SsidEqual = SSID_EQUAL(pStaCfg->CfgSsid, pStaCfg->CfgSsidLen, Ssid, strlen(Ssid));
			Ssid2Equal = SSID_EQUAL(pStaCfg->CfgSsid, pStaCfg->CfgSsidLen, Ssid2, strlen(Ssid2));
			Ssid3Equal = SSID_EQUAL(pStaCfg->CfgSsid, pStaCfg->CfgSsidLen, Ssid3, strlen(Ssid3));
			Ssid4Equal = SSID_EQUAL(pStaCfg->CfgSsid, pStaCfg->CfgSsidLen, Ssid4, strlen(Ssid4));
			Ssid5Equal = SSID_EQUAL(pStaCfg->CfgSsid, pStaCfg->CfgSsidLen, Ssid5, strlen(Ssid5));

			/* To tame down the BE aggresiveness increasing the Cwmin */
			if ((SsidEqual || Ssid2Equal) && (pEdcaParm->Cwmin[0] == 4)) {
				pEdcaParm->Cwmin[0]++;
			}

			if (Ssid3Equal) {
				pEdcaParm->Aifsn[0] = 7;
				pEdcaParm->Cwmin[0] += 2;
			}

			if ((Ssid4Equal || Ssid5Equal) && (pEdcaParm->Cwmin[2] == 3)) {
				pEdcaParm->Cwmin[2]++;
				pEdcaParm->Cwmax[2]++;
			}
		}
#endif
	}

#ifdef VOW_SUPPORT
	vow_update_om_wmm(pAd, wdev, entry->wmm_set, pEdcaParm);
#endif /* VOW_SUPPORT */
	if (arch_ops->archSetEdcaParm) {
		arch_ops->archSetEdcaParm(pAd, entry->wmm_set, entry->tx_mode, pEdcaParm);
		return;
	}
	AsicNotSupportFunc(pAd, __func__);
}


VOID AsicTxCntUpdate(RTMP_ADAPTER *pAd, UCHAR Wcid, MT_TX_COUNTER *pTxInfo)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		UINT32 TxSuccess = 0;

		MtAsicTxCntUpdate(pAd, Wcid, pTxInfo);
		TxSuccess = pTxInfo->TxCount - pTxInfo->TxFailCount;

		if (pTxInfo->TxFailCount == 0)
			pAd->RalinkCounters.OneSecTxNoRetryOkCount += pTxInfo->TxCount;
		else
			pAd->RalinkCounters.OneSecTxRetryOkCount += pTxInfo->TxCount;

		pAd->RalinkCounters.OneSecTxFailCount += pTxInfo->TxFailCount;
#ifdef STATS_COUNT_SUPPORT
		pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart += TxSuccess;
		pAd->WlanCounters[0].FailedCount.u.LowPart += pTxInfo->TxFailCount;
#endif /* STATS_COUNT_SUPPORT */
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}


VOID AsicRssiUpdate(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC
	MAC_TABLE_ENTRY *pEntry;
	CHAR RssiSet[3];
	INT i = 0;

	NdisZeroMemory(RssiSet, sizeof(RssiSet));

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (pAd->MacTab.Size == 0) {
				pEntry = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE];
				MtAsicRssiGet(pAd, pEntry->wcid, &RssiSet[0]);

				for (i = 0; i < 3; i++) {
					pEntry->RssiSample.AvgRssi[i] = RssiSet[i];
					pEntry->RssiSample.LastRssi[i] = RssiSet[i];
					pAd->ApCfg.RssiSample.AvgRssi[i] = RssiSet[i];
					pAd->ApCfg.RssiSample.LastRssi[i] = RssiSet[i];
				}
			} else {
				INT32 TotalRssi[3];
				INT j;

				NdisZeroMemory(TotalRssi, sizeof(TotalRssi));

				for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
					pEntry = &pAd->MacTab.Content[i];

					if (IS_VALID_ENTRY(pEntry)) {
						MtAsicRssiGet(pAd, pEntry->wcid, &RssiSet[0]);

						for (j = 0; j < 3; j++) {
							pEntry->RssiSample.AvgRssi[j] = RssiSet[j];
							pEntry->RssiSample.LastRssi[j] = RssiSet[j];
							TotalRssi[j] += RssiSet[j];
						}
					}
				}

				for (i = 0; i < 3; i++)
					pAd->ApCfg.RssiSample.AvgRssi[i] = pAd->ApCfg.RssiSample.LastRssi[i] = TotalRssi[i] / pAd->MacTab.Size;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
				pEntry = &pAd->MacTab.Content[i];

				if (IS_VALID_ENTRY(pEntry)) {
					MtAsicRssiGet(pAd, pEntry->wcid, &RssiSet[0]);

					for (i = 0; i < 3; i++) {
						pEntry->RssiSample.AvgRssi[i] = RssiSet[i];
						pEntry->RssiSample.LastRssi[i] = RssiSet[i];
						pAd->StaCfg[0].RssiSample.AvgRssi[i] = RssiSet[i];
						pAd->StaCfg[0].RssiSample.LastRssi[i] = RssiSet[i];
					}
				}
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	}

	return;
#endif
}

INT AsicSetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetRetryLimit(pAd, type, limit);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


UINT32 AsicGetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicGetRetryLimit(pAd, type);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicSetSlotTime(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bUseShortSlotTime,
	IN UCHAR channel,
	IN struct wifi_dev *wdev)
{
	UINT32 SlotTime = 0;
	UINT32 SifsTime = SIFS_TIME_24G;
	UCHAR BandIdx;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (bUseShortSlotTime)
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
		else
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);

		SlotTime = (bUseShortSlotTime) ? 9 : wdev->SlotTimeValue;
	}
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (channel > 14)
			bUseShortSlotTime = TRUE;

		if (bUseShortSlotTime && STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_SHORT_SLOT_INUSED))
			return;
		else if ((!bUseShortSlotTime) && (!STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_SHORT_SLOT_INUSED)))
			return;

		if (bUseShortSlotTime)
			STA_STATUS_SET_FLAG(pStaCfg, fSTA_STATUS_SHORT_SLOT_INUSED);
		else
			STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_SHORT_SLOT_INUSED);

		SlotTime = (bUseShortSlotTime) ? 9 : 20;

		/* force using short SLOT time for FAE to demo performance when TxBurst is ON*/
		if (((pStaCfg->StaActive.SupportedPhyInfo.bHtEnable == FALSE) && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED)))
#ifdef DOT11_N_SUPPORT
			|| ((pStaCfg->StaActive.SupportedPhyInfo.bHtEnable == TRUE) && (pAd->CommonCfg.BACapability.field.Policy == BA_NOTUSE))
#endif /* DOT11_N_SUPPORT */
		   ) {
			/* In this case, we will think it is doing Wi-Fi test*/
			/* And we will not set to short slot when bEnableTxBurst is TRUE.*/
		} else if (pAd->CommonCfg.bEnableTxBurst) {
			STA_STATUS_SET_FLAG(pStaCfg, fSTA_STATUS_SHORT_SLOT_INUSED);
			SlotTime = 9;
		}

		/* For some reasons, always set it to short slot time.*/
		/* ToDo: Should consider capability with 11B*/
		if (pStaCfg->BssType == BSS_ADHOC) {
			STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_SHORT_SLOT_INUSED);
			SlotTime = 20;
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	BandIdx = HcGetBandByChannel(pAd, channel);
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MtAsicSetSlotTime(pAd, SlotTime, SifsTime, BandIdx);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}


INT AsicSetMacMaxLen(RTMP_ADAPTER *pAd)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetMacMaxLen) {
		INT ret = 0;

		ret = arch_ops->archSetMacMaxLen(pAd);
		return ret;
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


VOID AsicGetTxTsc(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pTxTsc)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archGetTxTsc) {
		arch_ops->archGetTxTsc(pAd, wdev, pTxTsc);
		return;
	}
	AsicNotSupportFunc(pAd, __func__);
}

VOID AsicSetSMPS(RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR smps)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetSMPS)
		arch_ops->archSetSMPS(pAd, Wcid, smps);
	else
		AsicNotSupportFunc(pAd, __func__);
}



/*
 * ========================================================================
 * Description:
 * Add Shared key information into ASIC.
 * Update shared key, TxMic and RxMic to Asic Shared key table
 * Update its cipherAlg to Asic Shared key Mode.
 *
 * Return:
 * ========================================================================
 */
VOID AsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			BssIndex,
	IN UCHAR			KeyIdx,
	IN PCIPHER_KEY		pCipherKey)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MtAsicAddSharedKeyEntry(pAd, BssIndex, KeyIdx, pCipherKey);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}


/*	IRQL = DISPATCH_LEVEL*/
VOID AsicRemoveSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MtAsicRemoveSharedKeyEntry(pAd, BssIndex, KeyIdx);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}

UINT16 AsicGetTidSn(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archGetTidSn)
		return arch_ops->archGetTidSn(pAd, wcid, tid);

	AsicNotSupportFunc(pAd, __func__);
	return 0xffff;
}


VOID AsicUpdateBASession(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid, UINT16 sn, UCHAR basize, BOOLEAN isAdd, INT ses_type)
{
	MAC_TABLE_ENTRY *mac_entry;
	MT_BA_CTRL_T BaCtrl;
	STA_REC_BA_CFG_T StaRecBaCfg;
	VOID *pBaEntry;
	UINT32 i;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	os_zero_mem(&BaCtrl, sizeof(MT_BA_CTRL_T));
	mac_entry = &pAd->MacTab.Content[wcid];
	BaCtrl.BaSessionType = ses_type;
	BaCtrl.BaWinSize = basize;
	BaCtrl.isAdd = isAdd;
	BaCtrl.Sn = sn;
	BaCtrl.Wcid = wcid;
	BaCtrl.Tid = tid;

	if (mac_entry && mac_entry->wdev) {
		BaCtrl.band_idx = HcGetBandByWdev(mac_entry->wdev);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s(): mac_entry=%p!mac_entry->wdev=%p, Set BaCtrl.band_idx=%d\n",
				  __func__, mac_entry, mac_entry->wdev, BaCtrl.band_idx));
	} else {
		BaCtrl.band_idx = 0;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s(): mac_entry=%p!Set BaCtrl.band_idx=%d\n",
				  __func__, mac_entry, BaCtrl.band_idx));
	}

	if (ses_type == BA_SESSION_RECP) {
		/* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
		if (isAdd)
			os_move_mem(&BaCtrl.PeerAddr[0], &mac_entry->Addr[0], MAC_ADDR_LEN);
	}

	if (arch_ops->archUpdateBASession) {
		arch_ops->archUpdateBASession(pAd, BaCtrl);

		if (arch_ops->archUpdateStaRecBa) {
			if (!mac_entry  || !mac_entry->wdev)
				return;

			StaRecBaCfg.baDirection = ses_type;

			if (ses_type == ORI_BA) {
				i = mac_entry->BAOriWcidArray[tid];

				if (pAd->CommonCfg.dbdc_mode && !WMODE_CAP_AC(mac_entry->wdev->PhyMode))
					pAd->BATable.BAOriEntry[i].amsdu_cap = FALSE;

				pBaEntry = &pAd->BATable.BAOriEntry[i];
			} else {
				i = mac_entry->BARecWcidArray[tid];

				if (pAd->CommonCfg.dbdc_mode && !WMODE_CAP_AC(mac_entry->wdev->PhyMode))
					pAd->BATable.BAOriEntry[i].amsdu_cap = FALSE;

				pBaEntry = &pAd->BATable.BARecEntry[i];
			}

			StaRecBaCfg.BaEntry = pBaEntry;
			StaRecBaCfg.BssIdx = mac_entry->wdev->bss_info_argument.ucBssIndex;

			if (IS_ENTRY_REPEATER(mac_entry))
				StaRecBaCfg.MuarIdx = pAd->MacTab.tr_entry[mac_entry->wcid].OmacIdx;
			else
				StaRecBaCfg.MuarIdx = mac_entry->wdev->OmacIdx;

			StaRecBaCfg.tid = tid;
			StaRecBaCfg.BaEnable = (isAdd << tid);
			StaRecBaCfg.WlanIdx = wcid;
			arch_ops->archUpdateStaRecBa(pAd, StaRecBaCfg);
		}

		return;
	}

	AsicNotSupportFunc(pAd, __func__);
	return;
}

VOID AsicUpdateRxWCIDTable(RTMP_ADAPTER *pAd, USHORT WCID, UCHAR *pAddr, BOOLEAN IsBCMCWCID, BOOLEAN IsReset)
{
	MT_WCID_TABLE_INFO_T WtblInfo;
	MAC_TABLE_ENTRY *mac_entry = NULL;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	struct _STA_TR_ENTRY *tr_entry = NULL;

	os_zero_mem(&WtblInfo, sizeof(MT_WCID_TABLE_INFO_T));
	WtblInfo.Wcid = WCID;
		WtblInfo.IsReset = IsReset;
	os_move_mem(&WtblInfo.Addr[0], &pAddr[0], 6);

	if (VALID_UCAST_ENTRY_WCID(pAd, WCID))
		mac_entry = &pAd->MacTab.Content[WCID];

	if ((IsBCMCWCID == TRUE) || WCID == MAX_LEN_OF_MAC_TABLE) {
		/* BC Mgmt or BC/MC data */
		WtblInfo.MacAddrIdx = 0xe;
		WtblInfo.WcidType = MT_WCID_TYPE_BMCAST;
		WtblInfo.CipherSuit = WTBL_CIPHER_NONE;

		if (IF_COMBO_HAVE_AP_STA(pAd) && HcGetWcidLinkType(pAd, WCID) == WDEV_TYPE_STA)
			WtblInfo.WcidType = MT_WCID_TYPE_APCLI_MCAST;
	} else if (mac_entry) {
		if (IS_ENTRY_CLIENT(mac_entry)) {
			/* FIXME: will fix this when set entry fix for sta mode */
			if (mac_entry->wdev->wdev_type == WDEV_TYPE_AP)
				WtblInfo.WcidType = MT_WCID_TYPE_CLI;
			else if (mac_entry->wdev->wdev_type == WDEV_TYPE_STA)
				WtblInfo.WcidType = MT_WCID_TYPE_AP;
		} else if (IS_ENTRY_PEER_AP(mac_entry))
			WtblInfo.WcidType = MT_WCID_TYPE_APCLI;
		else if (IS_ENTRY_REPEATER(mac_entry))
			WtblInfo.WcidType = MT_WCID_TYPE_REPEATER;
		else if (IS_ENTRY_WDS(mac_entry))
			WtblInfo.WcidType = MT_WCID_TYPE_WDS;
		else
			WtblInfo.WcidType = MT_WCID_TYPE_CLI;

		if (IS_ENTRY_REPEATER(mac_entry)) {
			tr_entry = &pAd->MacTab.tr_entry[mac_entry->wcid];
			WtblInfo.MacAddrIdx = tr_entry->OmacIdx;
		} else
			WtblInfo.MacAddrIdx = mac_entry->wdev->OmacIdx;

		WtblInfo.Aid = mac_entry->Aid;
#ifdef TXBF_SUPPORT
		WtblInfo.PfmuId    = pAd->rStaRecBf.u2PfmuId;

		if (IS_HT_STA(mac_entry)) {
			WtblInfo.fgTiBf    = IS_ITXBF_SUP(mac_entry->rStaRecBf.u1TxBfCap);
			WtblInfo.fgTiBf    = (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn == TRUE) ? WtblInfo.fgTiBf : FALSE;
			WtblInfo.fgTeBf    = IS_ETXBF_SUP(mac_entry->rStaRecBf.u1TxBfCap);
			WtblInfo.fgTeBf    = (pAd->CommonCfg.ETxBfEnCond == TRUE) ? WtblInfo.fgTeBf : FALSE;
		}

		if (IS_VHT_STA(mac_entry)) {
			WtblInfo.fgTibfVht = IS_ITXBF_SUP(mac_entry->rStaRecBf.u1TxBfCap);
			WtblInfo.fgTibfVht = (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn == TRUE) ? WtblInfo.fgTibfVht : FALSE;
			WtblInfo.fgTebfVht = IS_ETXBF_SUP(mac_entry->rStaRecBf.u1TxBfCap);
			WtblInfo.fgTebfVht = (pAd->CommonCfg.ETxBfEnCond == TRUE) ? WtblInfo.fgTebfVht : FALSE;
		}

#endif

		if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_RDG_CAPABLE)
			&& CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_RALINK_CHIPSET))
			WtblInfo.aad_om = 1;

		if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_WMM_CAPABLE))
			WtblInfo.SupportQoS = TRUE;

		if (IS_HT_STA(mac_entry)) {
			WtblInfo.SupportHT = TRUE;

			if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_RDG_CAPABLE))
				WtblInfo.SupportRDG = TRUE;

			WtblInfo.SmpsMode = mac_entry->MmpsMode;
			WtblInfo.MpduDensity = mac_entry->MpduDensity;
			WtblInfo.MaxRAmpduFactor = mac_entry->MaxRAmpduFactor;

			if (IS_VHT_STA(mac_entry)) {
				WtblInfo.SupportVHT = TRUE;
				WtblInfo.dyn_bw = wlan_config_get_vht_bw_sig(mac_entry->wdev);
#ifdef TXBF_SUPPORT
				WtblInfo.gid = 63;
#endif
			}
		}

		if (IS_CIPHER_TKIP_Entry(mac_entry)) {
			WtblInfo.DisRHTR = 1;
#ifdef A4_CONN
				if (IS_ENTRY_A4(mac_entry))
				WtblInfo.DisRHTR = 0;
#endif
		}

#ifdef A4_CONN
			WtblInfo.a4_enable = IS_ENTRY_A4(mac_entry);

			if (IS_ENTRY_A4(mac_entry))
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("AsicUpdateRxWCIDTable: Enable A4 in WTBLinfo\n"));

#endif
#ifdef DOT11R_FT_SUPPORT
		if (IS_FT_STA(mac_entry)) {
			WtblInfo.SkipClearPrevSecKey = TRUE;
		}
#endif /* DOT11R_FT_SUPPORT */

#ifdef MBSS_AS_WDS_AP_SUPPORT
	if (mac_entry->wdev->wds_enable)
		WtblInfo.fg4AddrEnable = mac_entry->bEnable4Addr;
#endif

	} else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():mac_entry is NULL!\n", __func__));

	if (arch_ops->archUpdateRxWCIDTable)
		return arch_ops->archUpdateRxWCIDTable(pAd, WtblInfo);

	AsicNotSupportFunc(pAd, __func__);
	return;
}


#ifdef TXBF_SUPPORT
VOID AsicUpdateClientBfCap(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pMacEntry)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archUpdateClientBfCap)
		return arch_ops->archUpdateClientBfCap(pAd, pMacEntry);

	AsicNotSupportFunc(pAd, __func__);
}
#endif /* TXBF_SUPPORT */



/*
 * ==========================================================================
 * Description:
 *
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicDelWcidTab(RTMP_ADAPTER *pAd, UCHAR wcid_idx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archDelWcidTab)
		return arch_ops->archDelWcidTab(pAd, wcid_idx);

	AsicNotSupportFunc(pAd, __func__);
	return;
}

#ifdef MBSS_AS_WDS_AP_SUPPORT
VOID AsicSetWcid4Addr_HdrTrans(RTMP_ADAPTER *pAd, UCHAR wcid_idx, UCHAR IsEnable)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[wcid_idx];
    struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	UCHAR IsApcliEntry = 0;

      if (IS_ENTRY_PEER_AP(pEntry))
		IsApcliEntry = 1;

#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (arch_ops->archSetWcid4Addr_HdrTrans) {

			return arch_ops->archSetWcid4Addr_HdrTrans(pAd, wcid_idx, IsEnable, IsApcliEntry);
		} else {
			AsicNotSupportFunc(pAd, __FUNCTION__);
			return;
		}
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
	return;
}
#endif


#ifdef HTC_DECRYPT_IOT
INT32 AsicSetWcidAAD_OM(RTMP_ADAPTER *pAd, UCHAR wcid_idx, CHAR value)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetWcidAAD_OM)
		return arch_ops->archSetWcidAAD_OM(pAd, wcid_idx, value);

	AsicNotSupportFunc(pAd, __func__);
	return TRUE;
}
#endif /* HTC_DECRYPT_IOT */



VOID AsicAddRemoveKeyTab(
	IN PRTMP_ADAPTER pAd,
	IN ASIC_SEC_INFO *pInfo)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archAddRemoveKeyTab)
		return arch_ops->archAddRemoveKeyTab(pAd, pInfo);

	AsicNotSupportFunc(pAd, __func__);
}


INT AsicSendCommandToMcu(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSendCommandToMcu(pAd, Command, Token, Arg0, Arg1, in_atomic);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


BOOLEAN AsicSendCmdToMcuAndWait(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSendCmdToMcuAndWait(pAd, Command, Token, Arg0, Arg1, in_atomic);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

#ifdef STREAM_MODE_SUPPORT
/* StreamModeRegVal - return MAC reg value for StreamMode setting */
UINT32 StreamModeRegVal(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtStreamModeRegVal(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


/*
 * ========================================================================
 * Description:
 * configure the stream mode of specific MAC or all MAC and set to ASIC.
 *
 * Prameters:
 * pAd           ---
 * pMacAddr ---
 * bClear        --- disable the stream mode for specific macAddr when
 * (pMacAddr!=NULL)
 *
 * Return:
 * ========================================================================
 */
VOID AsicSetStreamMode(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddr,
	IN INT chainIdx,
	IN BOOLEAN bEnabled)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MtAsicSetStreamMode(pAd, pMacAddr, chainIdx, bEnabled);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}


VOID AsicStreamModeInit(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MtAsicStreamModeInit(pAd);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}
#endif /* STREAM_MODE_SUPPORT // */


#ifdef DOT11_N_SUPPORT
INT AsicReadAggCnt(RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicReadAggCnt(pAd, aggCnt, cnt_len);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetRalinkBurstMode(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetRalinkBurstMode(pAd, enable);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicUpdateTxOP(RTMP_ADAPTER *pAd, UINT32 AcNum, UINT32 TxOpVal)
{
	UINT32 last_txop_val;
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (pAd->CommonCfg.ManualTxop)
			return TRUE;

		last_txop_val = MtAsicGetWmmParam(pAd, AcNum, WMM_PARAM_TXOP);

		if (last_txop_val == TxOpVal) {
			/* No need to Update TxOP CR */
			return TRUE;
		} else if (last_txop_val == 0xdeadbeef) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Error CR value for TxOP = 0x%08x\n", __func__,
					 last_txop_val));
			return FALSE;
		}

		MtAsicSetWmmParam(pAd, 0, AcNum, WMM_PARAM_TXOP, TxOpVal);
		return TRUE;
	}

#endif /* MT_MAC */
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


#endif /* DOT11_N_SUPPORT */

INT AsicSetMacTxRx(RTMP_ADAPTER *pAd, INT txrx, BOOLEAN enable)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	INT ret = 0;

	if (arch_ops->archSetMacTxRx) {
		ret = arch_ops->archSetMacTxRx(pAd, txrx, enable, BAND0);

		if (ret != 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): SetMacTxRx failed!\n", __func__));
			return ret;
		}

#ifdef DBDC_MODE

		if (pAd->CommonCfg.dbdc_mode) {
			ret = arch_ops->archSetMacTxRx(pAd, txrx, enable, BAND1);

			if (ret != 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s(): SetMacTxRx failed!\n", __func__));
				return ret;
			}
		}

#endif /*DBDC_MODE*/
		return ret;
	}
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetRxvFilter(RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR ucBandIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	INT ret = 0;

	if (arch_ops->archSetRxvFilter) {
		ret = arch_ops->archSetRxvFilter(pAd, enable, ucBandIdx);

		if (ret != 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): SetRxvTxRx failed!\n", __func__));
			return ret;
		}

		return ret;
	}
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicInitWPDMA(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicInitWPDMA(pAd);
#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicSetWPDMA(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable)
{
#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetWPDMA(pAd, TxRx, enable);
#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


BOOLEAN AsicWaitPDMAIdle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicWaitPDMAIdle(pAd, round, wait_us);
#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

BOOLEAN AsicResetWPDMA(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		MtAsicResetWPDMA(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicSetMacWD(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetMacWD(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicSetTxStream(RTMP_ADAPTER *pAd, UINT32 StreamNum, UCHAR opmode, BOOLEAN up, UCHAR BandIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetTxStream) {
		INT Ret;

		Ret = arch_ops->archSetTxStream(pAd, StreamNum, BandIdx);
		return Ret;
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetRxStream(RTMP_ADAPTER *pAd, UINT32 rx_path, UCHAR BandIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetRxStream) {
		INT Ret;

		Ret = arch_ops->archSetRxStream(pAd, rx_path, BandIdx);
		return Ret;
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetBW(RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetBW(pAd, bw, BandIdx);

#endif /* MT_MAC */
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetCtrlCh(RTMP_ADAPTER *pAd, UINT8 extch)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return mt_mac_set_ctrlch(pAd, extch);

#endif /* MT_MAC */
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

VOID AsicSetTmrCR(RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetTmrCR)
		arch_ops->archSetTmrCR(pAd, enable, BandIdx);
	else
		AsicNotSupportFunc(pAd, __func__);
}

#ifdef MAC_APCLI_SUPPORT
/*
 * ==========================================================================
 * Description:
 * Set BSSID of Root AP
 *
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicSetApCliBssid(RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR index)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetApCliBssid)
		arch_ops->archSetApCliBssid(pAd, pBssid, index);
	else
		AsicNotSupportFunc(pAd, __func__);
}
#endif /* MAC_APCLI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
/* set Wdev Mac Address, some chip arch need to set CR .*/
VOID AsicSetMbssWdevIfAddr(struct _RTMP_ADAPTER *pAd, INT idx, UCHAR *if_addr, INT opmode)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetMbssWdevIfAddr)
		arch_ops->archSetMbssWdevIfAddr(pAd, idx, if_addr, opmode);
	else
		AsicNotSupportFunc(pAd, __func__);
}

/* set Wdev Mac Address, some chip arch need to set CR .*/
VOID AsicSetMbssHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetMbssHwCRSetting)
		arch_ops->archSetMbssHwCRSetting(pAd, mbss_idx, enable);
	else
		AsicNotSupportFunc(pAd, __func__);
}

/* set Wdev Mac Address, some chip arch need to set CR .*/
VOID AsicSetExtMbssEnableCR(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetExtMbssEnableCR)
		arch_ops->archSetExtMbssEnableCR(pAd, mbss_idx, enable);
	else
		AsicNotSupportFunc(pAd, __func__);
}

VOID AsicSetExtTTTTHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetExtTTTTHwCRSetting)
		arch_ops->archSetExtTTTTHwCRSetting(pAd, mbss_idx, enable);
	else
		AsicNotSupportFunc(pAd, __func__);
}
#endif /* CONFIG_AP_SUPPORT */


VOID AsicDMASchedulerInit(RTMP_ADAPTER *pAd, INT mode)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_DMASCH_CTRL_T DmaSchCtrl;

		if (MTK_REV_GTE(pAd, MT7603, MT7603E1) && MTK_REV_LT(pAd, MT7603, MT7603E2))
			DmaSchCtrl.bBeaconSpecificGroup = FALSE;
		else
			DmaSchCtrl.bBeaconSpecificGroup = TRUE;

		DmaSchCtrl.mode = mode;
#ifdef DMA_SCH_SUPPORT
		MtAsicDMASchedulerInit(pAd, DmaSchCtrl);
#endif
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}

INT32 AsicDevInfoUpdate(
	RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 EnableFeature)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s(): Set OwnMac=%02x:%02x:%02x:%02x:%02x:%02x\n",
			  __func__, PRINT_MAC(OwnMacAddr)));

	if (arch_ops->archSetDevMac)
		return arch_ops->archSetDevMac(pAd, OwnMacIdx, OwnMacAddr, BandIdx, Active, EnableFeature);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT32 AsicBssInfoUpdate(
	RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info_argument)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s(): Set Bssid=%02x:%02x:%02x:%02x:%02x:%02x, BssIndex(%d)\n",
			  __func__,
			  PRINT_MAC(bss_info_argument->Bssid),
			  bss_info_argument->ucBssIndex));

	if (arch_ops->archSetBssid)
		return arch_ops->archSetBssid(pAd, bss_info_argument);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT32 AsicExtPwrMgtBitWifi(RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucPwrMgtBit)
{
	MT_PWR_MGT_BIT_WIFI_T rPwtMgtBitWiFi = {0};

	rPwtMgtBitWiFi.ucWlanIdx = ucWlanIdx;
	rPwtMgtBitWiFi.ucPwrMgtBit = ucPwrMgtBit;
	return MtCmdExtPwrMgtBitWifi(pAd, rPwtMgtBitWiFi);
}

INT32 AsicStaRecUpdate(
	RTMP_ADAPTER *pAd,
	STA_REC_CTRL_T *sta_rec_ctrl)
{
	UINT8 WlanIdx = sta_rec_ctrl->WlanIdx;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetStaRec) {
		STA_REC_CFG_T StaCfg;
		PMAC_TABLE_ENTRY pEntry = NULL;
		INT32 ret = 0;

		os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));

		/* Need to provide H/W BC/MC WLAN index to CR4 */
		if (!VALID_UCAST_ENTRY_WCID(pAd, WlanIdx))
			pEntry = NULL;
		else
			pEntry	= &pAd->MacTab.Content[WlanIdx];

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s::Wcid(%d), u4EnableFeature(%d)\n",
				  __func__, sta_rec_ctrl->WlanIdx, sta_rec_ctrl->EnableFeature));

		if (pEntry && !IS_ENTRY_NONE(pEntry)) {
			if (!pEntry->wdev) {
				ASSERT(pEntry->wdev);
				return -1;
			}

			if (IS_ENTRY_REPEATER(pEntry))
				StaCfg.MuarIdx = pAd->MacTab.tr_entry[pEntry->wcid].OmacIdx;
			else
				StaCfg.MuarIdx = pEntry->wdev->OmacIdx;
		} else {
			StaCfg.MuarIdx = 0xe;/* TODO: Carter, check this on TX_HDR_TRANS */
		}

#ifdef TXBF_SUPPORT
		if (pEntry && !IS_ENTRY_NONE(pEntry)
			&& (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))) {
			UINT8 ucTxPath = pAd->Antenna.field.TxPath;

#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				UINT8 band_idx = HcGetBandByWdev(pEntry->wdev);

				if (band_idx == DBDC_BAND0)
					ucTxPath = pAd->dbdc_band0_tx_path;
				else
					ucTxPath = pAd->dbdc_band1_tx_path;
			}
#endif

			if (HcIsBfCapSupport(pEntry->wdev) == TRUE) {
				if (sta_rec_ctrl->EnableFeature & STA_REC_BF_FEATURE) {
					if (ucTxPath > 1)
						AsicBfStaRecUpdate(pAd, pEntry->wdev->PhyMode, sta_rec_ctrl->BssIndex, WlanIdx);

					/*
					 * TxBF Dynamic Mechanism
					 * Executed when STA associated
					 */
					txbf_dyn_mech(pAd);
				}
			}
		}

#endif /* TXBF_SUPPORT */
		StaCfg.ConnectionState = sta_rec_ctrl->ConnectionState;
		StaCfg.ConnectionType = sta_rec_ctrl->ConnectionType;
		StaCfg.u4EnableFeature = sta_rec_ctrl->EnableFeature;
		StaCfg.ucBssIndex = sta_rec_ctrl->BssIndex;
		StaCfg.ucWlanIdx = WlanIdx;
		StaCfg.pEntry = pEntry;
		StaCfg.IsNewSTARec = sta_rec_ctrl->IsNewSTARec;
		os_move_mem(&StaCfg.asic_sec_info, &sta_rec_ctrl->asic_sec_info, sizeof(ASIC_SEC_INFO));
		ret = arch_ops->archSetStaRec(pAd, StaCfg);
		return ret;
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


#ifdef MT_MAC
INT32 AsicRaParamStaRecUpdate(
	RTMP_ADAPTER *pAd,
	UINT8 WlanIdx,
	P_CMD_STAREC_AUTO_RATE_UPDATE_T prParam,
	UINT32 EnableFeature)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetStaRec) {
		STA_REC_CFG_T StaCfg;
		PMAC_TABLE_ENTRY pEntry = NULL;

		os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));

		/* Need to provide H/W BC/MC WLAN index to CR4 */
		if (!VALID_UCAST_ENTRY_WCID(pAd, WlanIdx))
			pEntry = NULL;
		else
			pEntry	= &pAd->MacTab.Content[WlanIdx];

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s::Wcid(%d), u4EnableFeature(%d)\n",
				  __func__, WlanIdx, EnableFeature));

		if (pEntry && !IS_ENTRY_NONE(pEntry)) {
			if (!pEntry->wdev) {
				ASSERT(pEntry->wdev);
				return -1;
			}

			if (IS_ENTRY_REPEATER(pEntry))
				StaCfg.MuarIdx = pAd->MacTab.tr_entry[pEntry->wcid].OmacIdx;
			else
				StaCfg.MuarIdx = pEntry->wdev->OmacIdx;

			StaCfg.ucBssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
		} else {
			StaCfg.MuarIdx = 0xe;/* TODO: Carter, check this on TX_HDR_TRANS */
		}

		StaCfg.ConnectionState = STATE_CONNECTED;
		StaCfg.u4EnableFeature = EnableFeature;
		StaCfg.ucWlanIdx = WlanIdx;
		StaCfg.pEntry = pEntry;
		StaCfg.pRaParam = prParam;
		/*tracking the starec input history*/
		return arch_ops->archSetStaRec(pAd, StaCfg);
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}
#endif /* MT_MAC */

VOID AsicTurnOffRFClk(RTMP_ADAPTER *pAd, UCHAR Channel)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MtAsicTurnOffRFClk(pAd, Channel);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}

INT32 AsicRadioOnOffCtrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 ucRadio)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_PMSTAT_CTRL_T PmStatCtrl = {0};

		PmStatCtrl.PmNumber = PM5;
		PmStatCtrl.DbdcIdx = ucDbdcIdx;

		if (ucRadio == WIFI_RADIO_ON) {
			PmStatCtrl.PmState = EXIT_PM_STATE;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DbdcIdx=%d RadioOn\n",
					 __func__, ucDbdcIdx));
		} else {
			PmStatCtrl.PmState = ENTER_PM_STATE;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DbdcIdx=%d RadioOff\n",
					 __func__, ucDbdcIdx));
		}

		return  MtCmdExtPmStateCtrl(pAd, PmStatCtrl);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

#ifdef GREENAP_SUPPORT
INT32 AsicGreenAPOnOffCtrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN ucGreenAPOn)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_GREENAP_CTRL_T GreenAPCtrl = {0};

		GreenAPCtrl.ucDbdcIdx = ucDbdcIdx;
		GreenAPCtrl.ucGreenAPOn = ucGreenAPOn;
		return  MtCmdExtGreenAPOnOffCtrl(pAd, GreenAPCtrl);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}
#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT32 asic_pcie_aspm_dym_ctrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_PCIE_ASPM_DYM_CTRL_T mt_pcie_aspm_dym_ctrl = {0};

		mt_pcie_aspm_dym_ctrl.ucDbdcIdx = ucDbdcIdx;
		mt_pcie_aspm_dym_ctrl.fgL1Enable = fgL1Enable;
		mt_pcie_aspm_dym_ctrl.fgL0sEnable = fgL0sEnable;
		return  mt_cmd_ext_pcie_aspm_dym_ctrl(pAd, mt_pcie_aspm_dym_ctrl);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

INT32 AsicExtPmStateCtrl(
	RTMP_ADAPTER *pAd,
	PSTA_ADMIN_CONFIG pStaCfg,
	UINT8 ucPmNumber,
	UINT8 ucPmState)
{
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev = &pAd->ApCfg.MBSSID[0].wdev;
	}
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pStaCfg->wdev;
	}
#endif
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_PMSTAT_CTRL_T PmStatCtrl = {0};

		PmStatCtrl.PmNumber = ucPmNumber;
		PmStatCtrl.PmState = ucPmState;

		if (ucPmNumber == PM4) {
#ifdef CONFIG_STA_SUPPORT
			PmStatCtrl.WlanIdx = pStaCfg->PwrMgmt.ucWcid;
			PmStatCtrl.Aid = pStaCfg->StaActive.Aid;
			PmStatCtrl.BcnInterval = pStaCfg->PwrMgmt.ucBeaconPeriod;
			PmStatCtrl.DtimPeriod = pStaCfg->PwrMgmt.ucDtimPeriod;
			PmStatCtrl.BcnLossCount = BEACON_OFFLOAD_LOST_TIME;
			NdisCopyMemory(PmStatCtrl.Bssid, pStaCfg->Bssid, MAC_ADDR_LEN);

			if (wdev) {
				PmStatCtrl.OwnMacIdx = wdev->OmacIdx;
				PmStatCtrl.WmmIdx = HcGetWmmIdx(pAd, wdev);
			}
#endif /* CONFIG_STA_SUPPORT */
		}

		return  MtCmdExtPmStateCtrl(pAd, PmStatCtrl);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

INT32 AsicExtWifiHifCtrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 PmStatCtrl, VOID *pReslt)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return  MtCmdWifiHifCtrl(pAd, ucDbdcIdx, PmStatCtrl, pReslt);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

#ifdef CONFIG_MULTI_CHANNEL

INT32 AsicMccStart(struct _RTMP_ADAPTER *ad,
				   UCHAR channel_1st,
				   UCHAR channel_2nd,
				   UINT32 bw_1st,
				   UINT32 bw_2nd,
				   UCHAR central_1st_seg0,
				   UCHAR central_1st_seg1,
				   UCHAR central_2nd_seg0,
				   UCHAR central_2nd_seg1,
				   UCHAR role_1st,
				   UCHAR role_2nd,
				   USHORT stay_time_1st,
				   USHORT stay_time_2nd,
				   USHORT idle_time,
				   USHORT null_repeat_cnt,
				   UINT32 start_tsf)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_MCC_ENTRT_T entries[2];

		entries[0].BssIdx = 0;
		entries[0].WlanIdx = 1;
		entries[0].WmmIdx = 0;
		entries[0].OwnMACAddressIdx = 0;
		entries[0].Bw = bw_1st;
		entries[0].CentralSeg0 =  central_1st_seg0;
		entries[0].CentralSeg1 =  central_1st_seg1;
		entries[0].Channel = channel_1st;
		entries[0].Role = role_1st;
		entries[0].StayTime = stay_time_1st;
		entries[1].BssIdx = 1;
		entries[1].WlanIdx = 2;
		entries[1].WmmIdx = 1;
		entries[1].OwnMACAddressIdx = 1;
		entries[1].Bw = bw_2nd;
		entries[1].CentralSeg0 =  central_2nd_seg0;
		entries[1].CentralSeg1 =  central_2nd_seg1;
		entries[1].Channel = channel_2nd;
		entries[1].Role = role_2nd;
		entries[1].StayTime = stay_time_2nd;
		return MtCmdMccStart(ad, 2, entries, idle_time, null_repeat_cnt, start_tsf);
	}

#endif
	AsicNotSupportFunc(ad, __func__);
	return 0;
}

#endif




#ifdef THERMAL_PROTECT_SUPPORT
INT32
AsicThermalProtect(
	RTMP_ADAPTER *pAd,
	UINT8 ucBand,
	UINT8 HighEn,
	CHAR HighTempTh,
	UINT8 LowEn,
	CHAR LowTempTh,
	UINT32 RechkTimer,
	UINT8 RFOffEn,
	CHAR RFOffTh,
	UINT8 ucType)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		INT32 ret = 0;

		ret = MtCmdThermalProtect(pAd, ucBand, HighEn, HighTempTh, LowEn, LowTempTh, RechkTimer, RFOffEn, RFOffTh, ucType);

		return ret;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}


INT32
AsicThermalProtectAdmitDuty(
	RTMP_ADAPTER *pAd,
	UINT8 ucBand,
	UINT32 u4Lv0Duty,
	UINT32 u4Lv1Duty,
	UINT32 u4Lv2Duty,
	UINT32 u4Lv3Duty
)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		INT32 ret = 0;

		ret = MtCmdThermalProtectAdmitDuty(pAd, ucBand, u4Lv0Duty, u4Lv1Duty, u4Lv2Duty, u4Lv3Duty);
		return ret;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

INT AsicThermalProtectAdmitDutyInfo(
	IN PRTMP_ADAPTER	pAd
)
{
#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		BOOLEAN  fgStatus = FALSE;

		fgStatus = MtCmdThermalProtectAdmitDutyInfo(pAd);
		fgStatus = TRUE;

		return fgStatus;
	}
#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}
#endif /* THERMAL_PROTECT_SUPPORT */


INT32 AsicGetMacInfo(RTMP_ADAPTER *pAd, UINT32 *ChipId, UINT32 *HwVer, UINT32 *FwVer)
{
	INT32 ret;
#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		ret = MtAsicGetMacInfo(pAd, ChipId, HwVer, FwVer);
		return ret;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

INT32 AsicGetAntMode(RTMP_ADAPTER *pAd, UCHAR *AntMode)
{
	INT32 ret;
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		ret = MtAsicGetAntMode(pAd, AntMode);
		return ret;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

INT32 AsicSetDmaByPassMode(RTMP_ADAPTER *pAd, BOOLEAN isByPass)
{
	INT32 ret;
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		ret = MtAsicSetDmaByPassMode(pAd, isByPass);
		return ret;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

#ifdef DBDC_MODE
INT32 AsicGetDbdcCtrl(RTMP_ADAPTER *pAd, BCTRL_INFO_T *pBctrlInfo)
{
	INT32 ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archGetDbdcCtrl)
		ret = arch_ops->archGetDbdcCtrl(pAd, pBctrlInfo);
	else
		AsicNotSupportFunc(pAd, __func__);

	return ret;
}

INT32 AsicSetDbdcCtrl(RTMP_ADAPTER *pAd, BCTRL_INFO_T *pBctrlInfo)
{
	INT32 ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetDbdcCtrl)
		ret = arch_ops->archSetDbdcCtrl(pAd, pBctrlInfo);
	else
		AsicNotSupportFunc(pAd, __func__);

	return ret;
}

#endif /*DBDC_MODE*/

INT32 AsicRxHeaderTransCtl(RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan,
						   BOOLEAN SwPcP)
{
	INT32 ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archRxHeaderTransCtl)
		ret = arch_ops->archRxHeaderTransCtl(pAd, En, ChkBssid, InSVlan, RmVlan, SwPcP);

	return ret;
}

INT32 AsicRxHeaderTaranBLCtl(RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType)
{
	INT32 ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archRxHeaderTaranBLCtl)
		ret = arch_ops->archRxHeaderTaranBLCtl(pAd, Index, En, EthType);

	return ret;
}

#ifdef IGMP_SNOOP_SUPPORT
BOOLEAN AsicMcastEntryInsert(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr,
							 PNET_DEV dev, UINT8 WlanIndex)
{
	INT32 Ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archMcastEntryInsert)
		Ret = arch_ops->archMcastEntryInsert(pAd, GrpAddr, BssIdx, Type, MemberAddr, dev, WlanIndex);

	return Ret;
}


BOOLEAN AsicMcastEntryDelete(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev,
							 UINT8 WlanIndex)
{
	INT32 Ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archMcastEntryDelete)
		Ret = arch_ops->archMcastEntryDelete(pAd, GrpAddr, BssIdx, MemberAddr, dev, WlanIndex);

	return Ret;
}
#endif

#ifdef DOT11_VHT_AC
INT AsicSetRtsSignalTA(RTMP_ADAPTER *pAd, UCHAR bw_sig)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetRtsSignalTA) {
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode)
			arch_ops->archSetRtsSignalTA(pAd, 1, bw_sig);
#endif /*  DBDC_MODE */
		arch_ops->archSetRtsSignalTA(pAd, 0, bw_sig);
	}
	return TRUE;
}
#endif /*DOT11_VHT_AC*/

VOID RssiUpdate(RTMP_ADAPTER *pAd)
{
	CHAR RSSI[4];
	MAC_TABLE_ENTRY *pEntry;
	INT i = 0;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (pAd->MacTab.Size == 0) {
				pEntry = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE];

				if (IS_VALID_ENTRY(pEntry))
					MtRssiGet(pAd, pEntry->wcid, &RSSI[0]);

				for (i = 0; i < TX_STREAM_PATH; i++) {
					pEntry->RssiSample.AvgRssi[i] = MINIMUM_POWER_VALUE;
					pEntry->RssiSample.LastRssi[i] = MINIMUM_POWER_VALUE;
					pAd->ApCfg.RssiSample.AvgRssi[i] = MINIMUM_POWER_VALUE;
					pAd->ApCfg.RssiSample.LastRssi[i] = MINIMUM_POWER_VALUE;
				}
			} else {
				INT32 TotalRssi[4];
				INT j;

				NdisZeroMemory(TotalRssi, sizeof(TotalRssi));

				for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
					pEntry = &pAd->MacTab.Content[i];

					if (IS_VALID_ENTRY(pEntry)) {
						MtRssiGet(pAd, pEntry->wcid, &RSSI[0]);

						for (j = 0; j < TX_STREAM_PATH; j++) {
							pEntry->RssiSample.AvgRssi[j] = RSSI[j];
							pEntry->RssiSample.LastRssi[j] = RSSI[j];
							TotalRssi[j] += RSSI[j];
						}
					}
				}

				for (i = 0; i < 4; i++) {
					if (pAd->MacTab.Size != 0)
						pAd->ApCfg.RssiSample.AvgRssi[i] = pAd->ApCfg.RssiSample.LastRssi[i] = TotalRssi[i] / pAd->MacTab.Size;
					else
						break;
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
				pEntry = &pAd->MacTab.Content[i];

				if (IS_VALID_ENTRY(pEntry)) {
					INT j;

					MtRssiGet(pAd, pEntry->wcid, &RSSI[0]);

					for (j = 0; j < TX_STREAM_PATH; j++) {
						pEntry->RssiSample.AvgRssi[i] = RSSI[i];
						pEntry->RssiSample.LastRssi[i] = RSSI[i];
						pAd->StaCfg[0].RssiSample.AvgRssi[i] = RSSI[i];
						pAd->StaCfg[0].RssiSample.LastRssi[i] = RSSI[i];
					}
				}
			}
		}
#endif /* CONFIG_STA_SUPPORT */
}

/* end Trace for every 100 ms */
#ifdef ETSI_RX_BLOCKER_SUPPORT
VOID CheckRssi(RTMP_ADAPTER *pAd)
{
	UINT8   u1MaxWRssiIdx;
	UINT8   u1WFBitMap	   = BITMAP_WF_ALL;
	CHAR	c1MaxWbRssi	= MINIMUM_POWER_VALUE;
	UINT32	u4WbRssi	   = 0;
	UINT8	u1CheckIdx;
	UINT32  u4DcrfCr = 0;
	UCHAR   u1BandIdx = 0;

	switch (pAd->u1RxBlockerState) {
	case ETSI_RXBLOCKER4R:

		/* Enable DCRF tracking */
		PHY_IO_READ32(pAd->hdev_ctrl, DCRF_TRACK, &u4DcrfCr);
		u4DcrfCr &= ~(BITS(28, 29));
		u4DcrfCr |= ((0x3 << 28) & BITS(28, 29)); /*Enable DCRF*/
		PHY_IO_WRITE32(pAd->hdev_ctrl, DCRF_TRACK, u4DcrfCr);


		/* confidence count check for 1R transition */
		for (u1CheckIdx = 0; u1CheckIdx < pAd->u1To1RCheckCnt; u1CheckIdx++) {
			/* update Max WBRSSI index */
			u1MaxWRssiIdx = ETSIWbRssiCheck(pAd);

			/* log check Max Rssi Index or not found */
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------------------------------------------------\n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" i1MaxWRssiIdxPrev: %x\n", pAd->i1MaxWRssiIdxPrev));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" u1MaxWRssiIdx: %x\n", u1MaxWRssiIdx));
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------------------------------------------------\n"));
			/* ---------------- */

			/* not found Max WBRSSI Index */
			if (u1MaxWRssiIdx == 0xFF) {
				pAd->u1ValidCnt = 0;
				pAd->i1MaxWRssiIdxPrev = 0xFF;
			}
			/* confidence count increment to 1R state */
			else if (pAd->i1MaxWRssiIdxPrev == u1MaxWRssiIdx) {
				pAd->u1ValidCnt++;
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------------------------------------------------\n"));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" Same index: u1ValidCnt: %d\n", pAd->u1ValidCnt));
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------------------------------------------------\n"));
			}
			/* Max WBRSSI index changed */
			else {
				pAd->u1ValidCnt = 1;
				pAd->i1MaxWRssiIdxPrev = u1MaxWRssiIdx;
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------------------------------------------------\n"));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" Different index: u1ValidCnt: %d\n", pAd->u1ValidCnt));
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------------------------------------------------\n"));
			}

			/* confidence count check */
			if (pAd->u1ValidCnt >= pAd->u2To1RvaildCntTH) {
				/* config Rx index according to bitmap */
				switch (u1MaxWRssiIdx) {
				case 0:
					u1WFBitMap = BITMAP_WF0;
					break;
				case 1:
					u1WFBitMap = BITMAP_WF1;
					break;
				case 2:
					u1WFBitMap = BITMAP_WF2;
					break;
				case 3:
					u1WFBitMap = BITMAP_WF3;
					break;
				default:
					break;
				}

				/* config Rx */
				MtCmdLinkTestRxCtrl(pAd, u1WFBitMap, u1BandIdx);

				/* reset confidence count */
				pAd->u1ValidCnt = 0;
				/* update state */
				pAd->u1RxBlockerState = ETSI_RXBLOCKER1R;
				/* break out for loop */
				break;
			}
		}
		break;

	case ETSI_RXBLOCKER1R:

	/* Disable DCRF tracking */
	PHY_IO_READ32(pAd->hdev_ctrl, DCRF_TRACK, &u4DcrfCr);
	u4DcrfCr &= ~(BITS(28, 29));
	u4DcrfCr |= ((0x0 << 28) & BITS(28, 29));/*Disable DCRF*/
	PHY_IO_WRITE32(pAd->hdev_ctrl, DCRF_TRACK, u4DcrfCr);


#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		if (IS_MT7622(pAd)) { /* for 7622 */
			if (pAd->i1MaxWRssiIdxPrev == WF0 || pAd->i1MaxWRssiIdxPrev == WF1) {
			/* Read WBRSSI (WF0) */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
			c1MaxWbRssi = (u4WbRssi & BITS(0, 7));/* [7:0] */
			} else {
			/* Read WBRSSI (WF2) */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_AGC_DEBUG_2, &u4WbRssi);
			c1MaxWbRssi = (u4WbRssi & BITS(0, 7));/* [7:0] */
			}
		}
		if (IS_MT7615(pAd)) { /* for 7615 */
			if (pAd->i1MaxWRssiIdxPrev == WF0 || pAd->i1MaxWRssiIdxPrev == WF1) {
			/* Read WBRSSI (WF0) */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
			c1MaxWbRssi = ((u4WbRssi >> 16) & BITS(0, 7));/* [23:16] */
			} else {
			/* Read WBRSSI (WF2) */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_AGC_DEBUG_2, &u4WbRssi);
			c1MaxWbRssi = ((u4WbRssi >> 16) & BITS(0, 7));/* [23:16] */
			}
		}
	} else {
		if (IS_MT7622(pAd)) { /* for 7622 */
			/* Read WBRSSI (WF0) */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
			c1MaxWbRssi = (u4WbRssi & BITS(0, 7));/* [7:0] */
		}
		if (IS_MT7615(pAd)) { /* for 7615 */
			/* Read WBRSSI (WF0) */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
			c1MaxWbRssi = ((u4WbRssi >> 16) & BITS(0, 7));/* [23:16] */
		}
	}
#else
	if (IS_MT7622(pAd)) { /* for 7622 */
		/* Read WBRSSI (WF0) */
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
		c1MaxWbRssi = (u4WbRssi & BITS(0, 7)); /* [7:0] */
	}
	if (IS_MT7615(pAd)) { /* for 7615 */
		/* Read WBRSSI (WF0) */
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
		c1MaxWbRssi = ((u4WbRssi >> 16) & BITS(0, 7)); /* [23:16] */
	}
#endif /* DBDC_MODE */


	/* log for check Rssi Read (WBRSSI/IBRSSI) */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------(1R State)----------------------------------- \n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" c1MaxWbRssi: %x \n", c1MaxWbRssi&0xFF));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
	/* ---------------- */

		/* CR risk - no expected 0x80 value on WF0/WF1 ; WF2/WF3  */
		if ((c1MaxWbRssi&0xFF) == 0x80) {

			pAd->u1RxBlockerState = ETSI_RXBLOCKER1R;

		}
		/* No CR risk */
		else {

			/* check whether back to 4R mode */
			if (c1MaxWbRssi < pAd->c1WBRssiTh4R) {
				/* CR risk - Protect unexpected value */
				if (pAd->u14RValidCnt >= pAd->u2To4RvaildCntTH) {

					MtCmdLinkTestRxCtrl(pAd, BITMAP_WF_ALL, u1BandIdx);
					/* update state */
					pAd->u1RxBlockerState = ETSI_RXBLOCKER4R;
					pAd->u14RValidCnt = 1;

					/* log for check Rssi Read (WBRSSI/IBRSSI) */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------(TO 4R State)-------------------------------\n"));
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" u14RValidCnt: %d\n", pAd->u14RValidCnt));
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------------------------------------------------\n"));
					/* ---------------- */
				} else {
					pAd->u1RxBlockerState = ETSI_RXBLOCKER1R;
					pAd->u14RValidCnt++;
					/* log for check Rssi Read (WBRSSI/IBRSSI) */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------(Keep 1R State)------------------------------- \n"));
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" c1MaxWbRssi: %d, c1WBRssiTh4R: %d \n", c1MaxWbRssi, pAd->c1WBRssiTh4R));
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" CR risk!! u14RValidCnt: %d \n", pAd->u14RValidCnt));
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
					/* ---------------- */
				}

			} else
				pAd->u1RxBlockerState = ETSI_RXBLOCKER1R;
		}

		break;
	default:
		break;
	}
}
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */

#define RTS_NUM_DIS_VALUE 0xff
#define RTS_LEN_DIS_VALUE 0xffffff
INT asic_rts_on_off(struct wifi_dev *wdev, BOOLEAN rts_en)
{
	struct _RTMP_ADAPTER *ad;
	UCHAR band_idx;
	UINT32 rts_num;
	UINT32 rts_len;
	struct _RTMP_ARCH_OP *arch_ops;

	if (!wdev)
		return 0;

	ad = wdev->sys_handle;
	band_idx = HcGetBandByWdev(wdev);
	arch_ops = hc_get_arch_ops(ad->hdev_ctrl);


	if (arch_ops->asic_rts_on_off) {
		if (rts_en) {
			rts_num = wlan_operate_get_rts_pkt_thld(wdev);
			rts_len = wlan_operate_get_rts_len_thld(wdev);
		} else {
			rts_num = RTS_NUM_DIS_VALUE;
			rts_len = RTS_LEN_DIS_VALUE;
		}
		return arch_ops->asic_rts_on_off(ad, band_idx, rts_num, rts_len, rts_en);
	}

	AsicNotSupportFunc(ad, __func__);
	return 0;
}

INT AsicAmpduEfficiencyAdjust(struct wifi_dev *wdev, UCHAR	aifs_adjust)
{
	struct _RTMP_ADAPTER *ad;
	UINT32	wmm_idx;
	struct _RTMP_ARCH_OP *arch_ops;

	if (!wdev)
		return 0;

	ad = wdev->sys_handle;
	wmm_idx = HcGetWmmIdx(ad, wdev);
	arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->asic_ampdu_efficiency_on_off)
		return arch_ops->asic_ampdu_efficiency_on_off(ad, wmm_idx, aifs_adjust);

	AsicNotSupportFunc(ad, __func__);
	return 0;
}

#ifdef LINK_TEST_SUPPORT
VOID LinkTestRcpiSet(RTMP_ADAPTER *pAd, UCHAR u1WlanId, UINT8 u1AntIdx, CHAR i1Rcpi)
{
	struct wtbl_entry tb_entry;
	union WTBL_DW28 wtbl_wd28;

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));
	if (!mt_wtbl_get_entry234(pAd, u1WlanId, &tb_entry)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Cannot found WTBL2/3/4 for WCID(%d)\n",
																__func__, u1WlanId));
		return;
	}

	/* Read RCPI from WTBL DW28 */
	HW_IO_READ32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 112, &wtbl_wd28.word);

	switch (u1AntIdx) {
	case BITMAP_WF0:
		wtbl_wd28.field.resp_rcpi_0 = i1Rcpi;
		break;
	case BITMAP_WF1:
		wtbl_wd28.field.resp_rcpi_1 = i1Rcpi;
		break;
	case BITMAP_WF2:
		wtbl_wd28.field.resp_rcpi_2 = i1Rcpi;
		break;
	case BITMAP_WF3:
		wtbl_wd28.field.resp_rcpi_3 = i1Rcpi;
		break;
	}

	/* Write Back RCPI from WTBL DW28 */
	HW_IO_WRITE32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 112, wtbl_wd28.word);

	return;
}

VOID LinkTestPeriodHandler(RTMP_ADAPTER *pAd)
{
	UINT8 u1BandIdx;

	if (pAd->CommonCfg.LinkTestSupport) {
		if (!pAd->fgCmwLinkDone) {
			/* CSD config for state transition */
			for (u1BandIdx = BAND0; u1BandIdx <= pAd->CommonCfg.dbdc_mode; u1BandIdx++)
				LinkTestTxCsdCtrl(pAd, FALSE, u1BandIdx);

			MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_LOUD, ("%s(): NOT Link up!!!\n", __func__));
		} else {
			/* Auto Link Test Control Handler executes with period 100 ms */
			RTMP_AUTO_LINK_TEST(pAd);

			MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_LOUD, ("%s(): Link up!!!\n", __func__));
		}
	}
}

VOID LinkTestTimeSlotLinkHandler(RTMP_ADAPTER *pAd)
{
	BOOLEAN fgCmwLinkStatus = TRUE;
	UINT8 u1WlanId;
	UINT8 u1BandIdx = BAND0;
	struct _MAC_TABLE_ENTRY *pEntry;

	/* get pointer to Entry */
	pEntry = &pAd->MacTab.Content[0];

	/* Test scenario check (only one STA Connect) */
	if (pAd->MacTab.Size != 1)
		fgCmwLinkStatus = FALSE;

	/* Search pEntry Address */
	for (u1WlanId = 1; VALID_UCAST_ENTRY_WCID(pAd, u1WlanId); u1WlanId++) {
		pEntry = &pAd->MacTab.Content[u1WlanId];

		/* APclient and Repeater not apply Link Test mechanism */
		if ((IS_ENTRY_REPEATER(pEntry)) || (IS_ENTRY_PEER_AP(pEntry))) {
			fgCmwLinkStatus = FALSE;
			break;
		}

		if (IS_ENTRY_CLIENT(pEntry)) {
			/* Check Test Instrument Condition */
			fgCmwLinkStatus = LinkTestInstrumentCheck(pAd, pEntry);
			/* Update Band Index */
			u1BandIdx = HcGetBandByWdev(pEntry->wdev);
			break;
		}
	}

	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): fgCmwLinkStatus: %d\n",
															__func__, fgCmwLinkStatus));

	/* Tx Specifi Spatial Extension config */
	LinkTestSpeIdxCtrl(pAd, fgCmwLinkStatus, u1BandIdx);

	/* Tx Bw Handler */
	LinkTestTxBwCtrl(pAd, fgCmwLinkStatus, pEntry);

	/* Tx Csd Handler */
	LinkTestTxCsdCtrl(pAd, fgCmwLinkStatus, u1BandIdx);

	/* Rcpi Computation Method Handler */
	LinkTestRcpiCtrl(pAd, fgCmwLinkStatus, u1BandIdx);

	/* Tx Power Up Handler */
	LinkTestTxPowerCtrl(pAd, fgCmwLinkStatus, u1BandIdx);

	/* Rx Filter Mode control Handler */
	LinkTestAcrCtrl(pAd, fgCmwLinkStatus, u1WlanId, u1BandIdx);

	/* Rx Stream Handler */
	LinkTestRxStreamCtrl(pAd, fgCmwLinkStatus, u1WlanId);
}

VOID LinkTestStaLinkUpHandler(RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN fgCmwLinkStatus = FALSE;

	if (pAd->CommonCfg.LinkTestSupport) {
		/* Check Test Instrument Condition */
		if ((pEntry->MaxHTPhyMode.field.BW == BW_20) && (pAd->MacTab.Size == 1))
			fgCmwLinkStatus = LinkTestInstrumentCheck(pAd, pEntry);
		/* Tx Spur workaround */
		LinkTestTxBwCtrl(pAd, fgCmwLinkStatus, pEntry);
		/* Update Link Up status (Enable) */
		pAd->fgCmwLinkDone = TRUE;
		MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Build Association: Link Up!!\n",
															__func__));
	}
}

VOID LinkTestApClientLinkUpHandler(RTMP_ADAPTER *pAd)
{
	if (pAd->CommonCfg.LinkTestSupport)
		pAd->fgApclientLinkUp = TRUE;
}

BOOLEAN LinkTestInstrumentCheck(RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN fgCmwLinkStatus = TRUE;

	/* Check Condition: Support VHT mode Support */
	if (pEntry->SupportRateMode & SUPPORT_VHT_MODE)
		fgCmwLinkStatus = FALSE;

	/* Check condition: 1 Tx Spatial Stream */
	if ((pEntry->SupportHTMCS > 0xFF) && ((MODE_HTMIX == pEntry->MaxHTPhyMode.field.MODE) || (pEntry->MaxHTPhyMode.field.MODE == MODE_HTGREENFIELD)))
		fgCmwLinkStatus = FALSE;

	/* Check condition: only support BW20 */
	if (pEntry->MaxHTPhyMode.field.BW != BW_20)
		fgCmwLinkStatus = FALSE;

	return fgCmwLinkStatus;
}

VOID LinkTestChannelBandUpdate(RTMP_ADAPTER *pAd, UINT8 u1BandIdx, UINT8 u1ControlChannel)
{
	if (pAd->CommonCfg.LinkTestSupport) {
		/* update channel band info */
		if (u1ControlChannel <= 14) {
			pAd->ucCmwChannelBand[u1BandIdx] = CHANNEL_BAND_2G;
			MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): 2G Channel Band !!\n",
																__func__));
		} else {
			pAd->ucCmwChannelBand[u1BandIdx] = CHANNEL_BAND_5G;
			MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): 5G Channel Band !!\n",
																__func__));
		}

		/* update channe band info flag */
		pAd->fgChannelBandInfoUpdate[u1BandIdx] = TRUE;
	}
}

VOID LinkTestChannelSwitchHandler(RTMP_ADAPTER *pAd, UINT8 u1BandIdx)
{
	if (pAd->CommonCfg.LinkTestSupport) {
		/* Update Link Up status (Disable) */
		pAd->fgCmwLinkDone = FALSE;
		/* Update Apclient Link up Flag */
		pAd->fgApclientLinkUp = FALSE;
		/* clear Timeout Count */
		pAd->ucRxTestTimeoutCount = 0;
		/* Restore to 4R Config */
		MtCmdLinkTestRxCtrl(pAd, BITMAP_WF_ALL, u1BandIdx);
		/* Update specific nR config Status */
		pAd->ucRxStreamState[u1BandIdx] = RX_DEFAULT_RXSTREAM_STATE;
		/* Update specific nR previous config Status */
		pAd->ucRxStreamStatePrev[u1BandIdx] = RX_DEFAULT_RXSTREAM_STATE;
	}
}

VOID LinkTestTxBwSwitch(RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry)
{
	UINT8 u1BandIdx;

	/* Update Band Index */
	u1BandIdx = HcGetBandByWdev(pEntry->wdev);

	/* Backup Primary channel Info */
	pAd->ucPrimChannel[u1BandIdx] = wlan_operate_get_prim_ch(pEntry->wdev);

	/* Backup Central channel Info */
	pAd->ucCentralChannel[u1BandIdx] = wlan_operate_get_cen_ch_1(pEntry->wdev);

	/* Backup Central channel2 Info */
	pAd->ucCentralChannel2[u1BandIdx] = wlan_operate_get_cen_ch_2(pEntry->wdev);

	/* Backup Extend channel Info */
	pAd->ucExtendChannel[u1BandIdx] = wlan_operate_get_ext_cha(pEntry->wdev);

	/* Backup HT Bw Info */
	pAd->ucHtBw[u1BandIdx] = wlan_operate_get_ht_bw(pEntry->wdev);

	/* Backup VHT Bw Info */
	pAd->ucVhtBw[u1BandIdx] = wlan_operate_get_vht_bw(pEntry->wdev);

	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): [BAND%d] PrimChannel: %d, CentralChannel: %d, CentralChannel2: %d, ExtChannel: %d, Ht_Bw: %d, Vht_Bw: %d\n",
																		__func__,
																		u1BandIdx,
																		pAd->ucPrimChannel[u1BandIdx],
																		pAd->ucCentralChannel[u1BandIdx],
																		pAd->ucCentralChannel2[u1BandIdx],
																		pAd->ucExtendChannel[u1BandIdx],
																		pAd->ucHtBw[u1BandIdx],
																		pAd->ucVhtBw[u1BandIdx]));

	/* Config HT BW20 */
	wlan_operate_set_ht_bw(pEntry->wdev, HT_BW_20, EXTCHA_NONE);

	/* Config VHT BW20 */
	wlan_operate_set_vht_bw(pEntry->wdev, VHT_BW_2040);
}

VOID LinkTestTxBwRestore(RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry)
{
	UINT8 u1BandIdx;

	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Restore Bw config !!!\n", __func__));

	/* Update Band Index */
	u1BandIdx = HcGetBandByWdev(pEntry->wdev);

	/* Restore Ht Bw config */
	wlan_operate_set_ht_bw(pEntry->wdev, pAd->ucHtBw[u1BandIdx], pAd->ucExtendChannel[u1BandIdx]);

	/* Restore Vht Bw config */
	wlan_operate_set_vht_bw(pEntry->wdev, pAd->ucVhtBw[u1BandIdx]);
}

VOID LinkTestSpeIdxCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, UINT8 u1BandIdx)
{
	/* check if Spatial Extension workaround enabled */
	if (pAd->fgTxSpeEn) {
		/* state transition for enable/disable Tx Spatial Extension workaround */
		switch (pAd->ucLinkSpeState[u1BandIdx]) {
		case TX_UNDEFINED_SPEIDX_STATE:
		case TX_DEFAULT_SPEIDX_STATE:
			if (fgCmwLinkStatus) {
				/* Enable specific Spatial Extension config for Link test */
				MtCmdLinkTestSeIdxCtrl(pAd, TX_SWITCHING_SPEIDX_STATE);

				/* update Tx Spatial Extension State */
				pAd->ucLinkSpeState[u1BandIdx] = TX_SWITCHING_SPEIDX_STATE;

				/* update Tx Spatial Extension previous State */
				pAd->ucLinkSpeStatePrev[u1BandIdx] = TX_SWITCHING_SPEIDX_STATE;
			}
			break;

		case TX_SWITCHING_SPEIDX_STATE:
			if (!fgCmwLinkStatus) {
				/* Disable specific Spatial Extension config for Link test */
				MtCmdLinkTestSeIdxCtrl(pAd, TX_DEFAULT_SPEIDX_STATE);

				/* update Tx Spatial Extension State */
				pAd->ucLinkSpeState[u1BandIdx] = TX_DEFAULT_SPEIDX_STATE;

				/* update Tx Spatial Extension previous State */
				pAd->ucLinkSpeStatePrev[u1BandIdx] = TX_DEFAULT_SPEIDX_STATE;
			}
			break;

		default:
			break;
		}
	}
}

VOID LinkTestTxBwCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, struct _MAC_TABLE_ENTRY *pEntry)
{
	UINT8 u1BandIdx;

	/* Update Band Index */
	u1BandIdx = HcGetBandByWdev(pEntry->wdev);

	/* check if Tx Spur workaround enabled */
	if (pAd->fgTxSpurEn) {
		/* state transition for enable/disable Tx Spur workaround */
		switch (pAd->ucLinkBwState[u1BandIdx]) {
		case TX_UNDEFINED_BW_STATE:
		case TX_DEFAULT_BW_STATE:

			if (fgCmwLinkStatus) {
				/* Bw switching */
				LinkTestTxBwSwitch(pAd, pEntry);

				/* update Bw and channel Info status flag */
				pAd->fgBwInfoUpdate[u1BandIdx] = TX_SWITCHING_BW_STATE;

				/* update Bw State */
				pAd->ucLinkBwState[u1BandIdx] = TX_SWITCHING_BW_STATE;

				/* update Bw previous State */
				pAd->ucLinkBwStatePrev[u1BandIdx] = TX_SWITCHING_BW_STATE;
			}
			break;

		case TX_SWITCHING_BW_STATE:

			if ((!fgCmwLinkStatus) && (pAd->fgBwInfoUpdate[u1BandIdx])) {
				/* Bw Restore */
				LinkTestTxBwRestore(pAd, pEntry);

				/* reset Bw and channel Info status flag */
				pAd->fgBwInfoUpdate[u1BandIdx] = TX_DEFAULT_BW_STATE;

				/* update Bw State */
				pAd->ucLinkBwState[u1BandIdx] = TX_DEFAULT_BW_STATE;

				/* update Bw previous State */
				pAd->ucLinkBwStatePrev[u1BandIdx] = TX_DEFAULT_BW_STATE;
			}
			break;

		default:
			break;
		}
	}
}

VOID LinkTestTxCsdCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, UINT8 u1BandIdx)
{
	BOOLEAN fgZeroCsd = FALSE;

	/* channel band info updated sanity check */
	if (pAd->fgChannelBandInfoUpdate[u1BandIdx])
		return;

	/* check MAC Table size to determine enabled/disabled status of Tx CSD config */
	if ((pAd->MacTab.Size == 0) || ((pAd->MacTab.Size == 1) && (fgCmwLinkStatus)))
		fgZeroCsd = TRUE;

	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): fgCmwLinkStatus: %d, fgZeroCsd: %d\n",
															__func__, fgCmwLinkStatus, fgZeroCsd));

	/* CSD config for state transition */
	switch (pAd->ucTxCsdState[u1BandIdx]) {
	case TX_UNDEFINED_CSD_STATE:
	case TX_DEFAULT_CSD_STATE:

		if (fgZeroCsd) {
			/* Zero CSD config for Link test */
			MtCmdLinkTestTxCsdCtrl(pAd, TX_ZERO_CSD_STATE, u1BandIdx, pAd->ucCmwChannelBand[u1BandIdx]);

			/* update Tx CSD State */
			pAd->ucTxCsdState[u1BandIdx] = TX_ZERO_CSD_STATE;

			/* update Tx CSD previos State */
			pAd->ucTxCsdStatePrev[u1BandIdx] = TX_ZERO_CSD_STATE;
		}
		break;

	case TX_ZERO_CSD_STATE:

		if (!fgZeroCsd) {
			/* Default CSD config for Link test */
			MtCmdLinkTestTxCsdCtrl(pAd, TX_DEFAULT_CSD_STATE, u1BandIdx, pAd->ucCmwChannelBand[u1BandIdx]);

			/* update Tx CSD State */
			pAd->ucTxCsdState[u1BandIdx] = TX_DEFAULT_CSD_STATE;
		}
		break;

	default:
		break;
	}
}

VOID LinkTestRcpiCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, UINT8 u1BandIdx)
{
	/* check if Rcpi workaround enabled */
	if (pAd->fgRxRcpiEn) {
		/* state transition for enable/disable Rcpi workaround */
		switch (pAd->ucLinkRcpiState[u1BandIdx]) {
		case RX_UNDEFINED_RCPI_STATE:
		case RX_DEFAULT_RCPI_STATE:
			if (fgCmwLinkStatus) {
				/* Enable specific Rcpi config for Link test (Rcpi computation refer to both Response Frame and Data Frame) */
				MtCmdLinkTestRcpiCtrl(pAd, RX_SPECIFIC_RCPI_STATE);

				/* update Rcpi State */
				pAd->ucLinkRcpiState[u1BandIdx] = RX_SPECIFIC_RCPI_STATE;
			}
			break;

		case RX_SPECIFIC_RCPI_STATE:
			if (!fgCmwLinkStatus) {
				/* Disable specific Rcpi config for Link test */
				MtCmdLinkTestRcpiCtrl(pAd, RX_DEFAULT_RCPI_STATE);

				/* update Rcpi State */
				pAd->ucLinkRcpiState[u1BandIdx] = RX_DEFAULT_RCPI_STATE;
			}
			break;

		default:
			break;
		}
	}
}

VOID LinkTestTxPowerCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, UINT8 u1BandIdx)
{
	BOOLEAN fgPowerBoost = FALSE;

	/* channel band info updated sanity check */
	if (pAd->fgChannelBandInfoUpdate[u1BandIdx])
		return;

	/* check MAC Table size to determine enabled/disabled status of Tx Power up config */
	if ((pAd->MacTab.Size == 1) && (fgCmwLinkStatus))
		fgPowerBoost = TRUE;

	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): fgCmwLinkStatus: %d, fgPowerBoost: %d\n",
															__func__, fgCmwLinkStatus, fgPowerBoost));

	/* Tx Power config for state transition */
	switch (pAd->ucTxPwrBoostState[u1BandIdx]) {
	case TX_UNDEFINED_POWER_STATE:
	case TX_DEFAULT_POWER_STATE:

		if (fgPowerBoost) {
			/* Boost Tx Power config for Link test */
			MtCmdLinkTestTxPwrCtrl(pAd, TX_BOOST_POWER_STATE, u1BandIdx, pAd->ucCmwChannelBand[u1BandIdx]);

			/* update Tx Power State */
			pAd->ucTxPwrBoostState[u1BandIdx] = TX_BOOST_POWER_STATE;
		}
		break;

	case TX_BOOST_POWER_STATE:

		if (!fgPowerBoost) {
			/* Default Tx Power config for Link test */
			MtCmdLinkTestTxPwrCtrl(pAd, TX_DEFAULT_POWER_STATE, u1BandIdx, pAd->ucCmwChannelBand[u1BandIdx]);

			/* update Tx Power State */
			pAd->ucTxPwrBoostState[u1BandIdx] = TX_DEFAULT_POWER_STATE;
		}
		break;

	default:
		break;
	}
}

VOID LinkTestRxCntCheck(RTMP_ADAPTER *pAd)
{
	INT64 c8RxCount;

	/* Read Rx Count Info */
	c8RxCount = pAd->WlanCounters[0].ReceivedFragmentCount.QuadPart;
	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): c8RxCount: %lld, c8TempRxCount: %lld\n",
															__func__, c8RxCount, pAd->c8TempRxCount));

	/* check Rx test timeout count */
	if (c8RxCount - pAd->c8TempRxCount <= pAd->c8RxCountTh)
		pAd->ucRxTestTimeoutCount++;
	else
		pAd->ucRxTestTimeoutCount = 0;

	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): nR mode Rx Count: %lld, RxTestTimeoutCount: %d\n",
															__func__, c8RxCount - pAd->c8TempRxCount, pAd->ucRxTestTimeoutCount));

	/* update Rx Count to temp buffer */
	pAd->c8TempRxCount = pAd->WlanCounters[0].ReceivedFragmentCount.QuadPart;
}

VOID LinkTestRxStreamCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, UINT8 u1WlanId)
{
	struct _MAC_TABLE_ENTRY *pEntry;
	UINT8 u1SpeRssiIdx = 0;
	UINT8 u1RssiReason;
	UINT8 u1BandIdx;
	UINT8 u1AndIdx;
	CHAR i1MaxRssi = MINIMUM_POWER_VALUE;
	CHAR i1Rssi[WF_NUM] = {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};

	/* get pointer to Entry for specific WlanId */
	pEntry = &pAd->MacTab.Content[u1WlanId];

	/* Update Band Index */
	u1BandIdx = HcGetBandByWdev(pEntry->wdev);

	/* get Rssi value in WTBL */
	LinkTestRssiGet(pAd, RSSI_CHECK_WTBL_RSSI, u1WlanId, i1Rssi);

	/* get Max Rssi value in WTBL */
	for (u1AndIdx = WF0; u1AndIdx < WF_NUM; u1AndIdx++) {
		/* Rssi sanity protection */
		if (i1Rssi[u1AndIdx] >= 0)
			i1Rssi[u1AndIdx] = MINIMUM_POWER_VALUE;
		/* update Max Rssi value */
		if (i1Rssi[u1AndIdx] > i1MaxRssi)
			i1MaxRssi = i1Rssi[u1AndIdx];
	}

	/* Rssi value sanity check */
	if (i1MaxRssi == MINIMUM_POWER_VALUE)
		return;

	/* Config RSSI Moving Average Ratio 1/2 */
	MtCmdLinkTestRcpiMACtrl(pAd, CMW_RCPI_MA_1_2);

	/* check if Rx sensitivity workaround enabled */
	if (pAd->fgRxSensitEn) {
		MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Rssi: (%d : %d : %d : %d)\n",
																__func__, i1Rssi[WF0], i1Rssi[WF1], i1Rssi[WF2], i1Rssi[WF3]));

		/* only allow potentail state transition for Max Rssi < -40dB for change channel scenario */
		switch (pAd->ucRxStreamState[u1BandIdx]) {
		case RX_UNDEFINED_RXSTREAM_STATE:
		case RX_DEFAULT_RXSTREAM_STATE:

			if ((i1MaxRssi < pAd->cNrRssiTh) && (fgCmwLinkStatus)) {

				/* Rssi significance check */
				LinkTestRssiCheck(pAd, i1Rssi, u1BandIdx, &u1SpeRssiIdx, &u1RssiReason, u1WlanId);
				MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Band: %d, Specific Rssi Rx Path BitMap Index: %d\n",
																		__func__, u1BandIdx, u1SpeRssiIdx));

				/* Significant Rssi value Antenna Index check */
				if (u1SpeRssiIdx == 0x0) {
					/* Reset Rx Stream switching confidence count */
					pAd->ucRxSenCount[u1BandIdx] = 0;
					MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Increment Rx Stream switching confidence count: %d\n", __func__, pAd->ucRxSenCount[u1BandIdx]));
				} else if (u1SpeRssiIdx != pAd->u1SpeRssiIdxPrev[u1BandIdx]) {
					/* config Rx Stream switching confidence count to unity */
					pAd->ucRxSenCount[u1BandIdx] = 1;
				} else {
					/* Increment Rx Stream switching confidence count */
					pAd->ucRxSenCount[u1BandIdx]++;
					MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Reset Rx Stream switching confidence count\n", __func__));
				}

				/* update previous significant Rssi value Antenna Index */
				pAd->u1SpeRssiIdxPrev[u1BandIdx] = u1SpeRssiIdx;

				/* Rx Stream switching confidence count check */
				if (pAd->ucRxSenCount[u1BandIdx] >= pAd->ucRxSenCountTh) {
					/* Rx Stream switching */
					LinkTestRxStreamTrans(pAd, u1BandIdx, u1SpeRssiIdx);
					/* clear Rssi value in WTBL */
					MtAsicRcpiReset(pAd, u1WlanId);
					/* update Rx Stream switching reason */
					pAd->u1RxStreamSwitchReason[u1BandIdx] = u1RssiReason;
					/* update DCRF config for Rx Blocking */
					if (u1RssiReason == RSSI_REASON_RX_BLOCKING)
						LinkTestDCRFCtrl(pAd, FALSE);
				}
			}
			break;

		case RX_RXSTREAM_WF0_STATE:
		case RX_RXSTREAM_WF1_STATE:
		case RX_RXSTREAM_WF2_STATE:
		case RX_RXSTREAM_WF3_STATE:
		case RX_RXSTREAM_WF01_STATE:
		case RX_RXSTREAM_WF02_STATE:
		case RX_RXSTREAM_WF03_STATE:
		case RX_RXSTREAM_WF12_STATE:
		case RX_RXSTREAM_WF13_STATE:
		case RX_RXSTREAM_WF23_STATE:
		case RX_RXSTREAM_WF012_STATE:
		case RX_RXSTREAM_WF013_STATE:
		case RX_RXSTREAM_WF023_STATE:
		case RX_RXSTREAM_WF123_STATE:

			if (pAd->u1RxStreamSwitchReason[u1BandIdx] == RSSI_REASON_SENSITIVITY) {
				/* check Rx count Status */
				LinkTestRxCntCheck(pAd);
				/* Restore to default Rx Stream config for Timeout condition or not Link Test scenario or change path scenario */
				if ((pAd->ucRxTestTimeoutCount > pAd->ucTimeOutTh) || (!fgCmwLinkStatus)) {
					/* clear Rssi value in WTBL */
					MtAsicRcpiReset(pAd, u1WlanId);
					/* Restore to 4R Config */
					LinkTestRxStreamTrans(pAd, u1BandIdx, BITMAP_WF_ALL);
					/* Reset Timeout Count */
					pAd->ucRxTestTimeoutCount = 0;
				}
			} else if (pAd->u1RxStreamSwitchReason[u1BandIdx] == RSSI_REASON_RX_BLOCKING) {
				/* check WB Rssi value */
				u1SpeRssiIdx = LinkTestRssiCheckItem(pAd, i1Rssi, u1BandIdx, RSSI_CHECK_BBP_WBRSSI, u1WlanId);
				/* Restore to default Rx Stream config for Timeout condition or not Link Test scenario or change path scenario */
				if ((u1SpeRssiIdx == 0x0) || (!fgCmwLinkStatus)) {
					/* clear Rssi value in WTBL */
					MtAsicRcpiReset(pAd, u1WlanId);
					/* Restore to 4R Config */
					LinkTestRxStreamTrans(pAd, u1BandIdx, BITMAP_WF_ALL);
					/* update DCRF config for Rx Blocking */
					LinkTestDCRFCtrl(pAd, TRUE);
				}
			}
			break;

		default:
			break;
		}
	}
}

VOID LinkTestDCRFCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgDCRFenable)
{
	UINT32 u4Buffer;

	/* only MT7622 need specific DCRF control */
	if (IS_MT7622(pAd)) {
		if (fgDCRFenable) {
			/* Enable DCRF tracking */
			PHY_IO_READ32(pAd->hdev_ctrl, PHY_DCRF_TRACK, &u4Buffer);
			u4Buffer &= ~(BITS(28, 29));
			u4Buffer |= ((0x3 << 28) & BITS(28, 29));
			PHY_IO_WRITE32(pAd->hdev_ctrl, PHY_DCRF_TRACK, u4Buffer);
		} else {
			/* Disable DCRF tracking */
			PHY_IO_READ32(pAd->hdev_ctrl, PHY_DCRF_TRACK, &u4Buffer);
			u4Buffer &= ~(BITS(28, 29));
			u4Buffer |= ((0x0 << 28) & BITS(28, 29));
			PHY_IO_WRITE32(pAd->hdev_ctrl, PHY_DCRF_TRACK, u4Buffer);
		}
	}
}

VOID LinkTestRxStreamTrans(RTMP_ADAPTER *pAd, UINT8 u1BandIdx, UINT8 u1SpeRssiIdx)
{
	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): nR mode after link up!!! Rx Path Bitmap Index: %d\n", __func__, u1SpeRssiIdx));

	/* Reset Rx Stream switching confidence count */
	pAd->ucRxSenCount[u1BandIdx] = 0;

	/* Enter specific nR mode */
	MtCmdLinkTestRxCtrl(pAd, u1SpeRssiIdx, u1BandIdx);

	/* Update specific nR config Status */
	pAd->ucRxStreamState[u1BandIdx] = u1SpeRssiIdx;
}

VOID LinkTestAcrCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, UINT8 u1WlanId, UINT8 u1BandIdx)
{
	UINT8 u1AndIdx;
	CHAR i1MaxRssi = MINIMUM_POWER_VALUE;
	CHAR i1Rssi[WF_NUM] = {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};

	/* get Rssi value in WTBL */
	LinkTestRssiGet(pAd, RSSI_CHECK_WTBL_RSSI, u1WlanId, i1Rssi);

	/* get Max Rssi value */
	for (u1AndIdx = WF0; u1AndIdx < WF_NUM; u1AndIdx++) {
		/* Rssi sanity protection */
		if (i1Rssi[u1AndIdx] >= 0)
			i1Rssi[u1AndIdx] = MINIMUM_POWER_VALUE;
		/* update Max Rssi value */
		if (i1Rssi[u1AndIdx] > i1MaxRssi)
			i1MaxRssi = i1Rssi[u1AndIdx];
	}

	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Rssi: (%d : %d : %d : %d)\n",
															__func__, i1Rssi[WF0], i1Rssi[WF1], i1Rssi[WF2], i1Rssi[WF3]));

	/* Instrument check */
	if (!fgCmwLinkStatus)
		return;

	/* check ACR patch enable/disable status */
	if (pAd->fgACREn) {

		switch (pAd->ucRxFilterstate[u1BandIdx]) {
		case TX_UNDEFINED_RXFILTER_STATE:
		case TX_DEFAULT_MAXIN_STATE:

			if (i1MaxRssi <= pAd->cMaxInRssiTh) {
				if (pAd->ucRxFilterConfidenceCnt[u1BandIdx] >= pAd->ucACRConfidenceCntTh) {
					/* Fw command to apply ACR patch */
					MtCmdLinkTestACRCtrl(pAd, TX_SPECIFIC_ACR_STATE, u1BandIdx);  /* ACR patch */

					/* Clear ACR confidence count */
					pAd->ucRxFilterConfidenceCnt[u1BandIdx] = 0;

					/* update Rx Filter State */
					pAd->ucRxFilterstate[u1BandIdx] = TX_SPECIFIC_ACR_STATE;

					MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): ACR patch!!!\n", __func__));
				} else {
					/* ACI confidence count increment */
					pAd->ucRxFilterConfidenceCnt[u1BandIdx]++;
				}
			}
			break;

		case TX_SPECIFIC_ACR_STATE:

			if (i1MaxRssi > pAd->cMaxInRssiTh) {
				if (pAd->ucRxFilterConfidenceCnt[u1BandIdx] >= pAd->ucMaxInConfidenceCntTh) {
					/* Fw command to apply MaxIn patch */
					MtCmdLinkTestACRCtrl(pAd, TX_DEFAULT_MAXIN_STATE, u1BandIdx); /* Max Input patch */

					/* Clear MaxIn confidence count */
					pAd->ucRxFilterConfidenceCnt[u1BandIdx] = 0;

					/* update Rx Filter State */
					pAd->ucRxFilterstate[u1BandIdx] = TX_DEFAULT_MAXIN_STATE;

					MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): MaxIn patch!!!\n", __func__));
				} else {
					/* MaxIn confidence count increment */
					pAd->ucRxFilterConfidenceCnt[u1BandIdx]++;
				}
			}
			break;

		default:
			break;
		}
	}
}

VOID LinkTestRssiGet(RTMP_ADAPTER *pAd, ENUM_RSSI_CHECK_SOURCE eRssiSrc, UINT8 u1WlanId, PCHAR pi1Rssi)
{
	UINT32 u4Buffer1, u4Buffer2;

	switch (eRssiSrc) {
	case RSSI_CHECK_BBP_WBRSSI:
		/* read Rssi value*/
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_2, &u4Buffer1);
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_AGC_DEBUG_2, &u4Buffer2);
		if (IS_MT7622(pAd)) {
			*(pi1Rssi + 0) = (CHAR)((u4Buffer1 & BITS(0, 7)) >> 0);
			*(pi1Rssi + 1) = (CHAR)((u4Buffer1 & BITS(16, 23)) >> 16);
			*(pi1Rssi + 2) = (CHAR)((u4Buffer2 & BITS(0, 7)) >> 0);
			*(pi1Rssi + 3) = (CHAR)((u4Buffer2 & BITS(16, 23)) >> 16);
		} else if (IS_MT7615(pAd)) {
			*(pi1Rssi + 0) = (CHAR)((u4Buffer1 & BITS(16, 23)) >> 16);
			*(pi1Rssi + 1) = (CHAR)((u4Buffer1 & BITS(0, 7)) >> 0);
			*(pi1Rssi + 2) = (CHAR)((u4Buffer2 & BITS(16, 23)) >> 16);
			*(pi1Rssi + 3) = (CHAR)((u4Buffer2 & BITS(0, 7)) >> 0);
		}
		break;

	case RSSI_CHECK_BBP_IBRSSI:
		/* read Rssi value*/
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_2, &u4Buffer1);
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_AGC_DEBUG_2, &u4Buffer2);
		if (IS_MT7622(pAd)) {
			*(pi1Rssi + 0) = (CHAR)((u4Buffer1 & BITS(8, 15)) >> 8);
			*(pi1Rssi + 1) = (CHAR)((u4Buffer1 & BITS(24, 31)) >> 24);
			*(pi1Rssi + 2) = (CHAR)((u4Buffer2 & BITS(8, 15)) >> 8);
			*(pi1Rssi + 3) = (CHAR)((u4Buffer2 & BITS(24, 31)) >> 24);
		} else if (IS_MT7615(pAd)) {
			*(pi1Rssi + 0) = (CHAR)((u4Buffer1 & BITS(24, 31)) >> 24);
			*(pi1Rssi + 1) = (CHAR)((u4Buffer1 & BITS(8, 15)) >> 8);
			*(pi1Rssi + 2) = (CHAR)((u4Buffer2 & BITS(24, 31)) >> 24);
			*(pi1Rssi + 3) = (CHAR)((u4Buffer2 & BITS(8, 15)) >> 8);
		}
		break;

	case RSSI_CHECK_WTBL_RSSI:
		MtRssiGet(pAd, u1WlanId, pi1Rssi);
		break;
	default:
		break;
	}
}

VOID LinkTestRssiCheck(RTMP_ADAPTER *pAd, PCHAR pi1Rssi, UINT8 u1BandIdx, PUINT8 pu1SpeRssiIdx, PUINT8 pu1RssiReason, UINT8 u1WlanId)
{
	UINT8 u1SpeRssiIdx = 0, u1SpeRssiIdx2 = 0;

	/* check Rssi value in WTBL */
	u1SpeRssiIdx = LinkTestRssiCheckItem(pAd, pi1Rssi, u1BandIdx, RSSI_CHECK_WTBL_RSSI, u1WlanId);

	/* update Rssi reason */
	*pu1RssiReason = RSSI_REASON_SENSITIVITY;

	if (u1SpeRssiIdx == 0x0) {
		/* check WB Rssi value */
		u1SpeRssiIdx = LinkTestRssiCheckItem(pAd, pi1Rssi, u1BandIdx, RSSI_CHECK_BBP_WBRSSI, u1WlanId);

		/* check IB Rssi value */
		u1SpeRssiIdx2 = LinkTestRssiCheckItem(pAd, pi1Rssi, u1BandIdx, RSSI_CHECK_BBP_IBRSSI, u1WlanId);

		/* reset specific Rx path for not satisfy condition IB Rssi not small enough for non-significance path for Rx Block scenario */
		if ((u1SpeRssiIdx & u1SpeRssiIdx2) != u1SpeRssiIdx2)
			u1SpeRssiIdx = 0;

		/* update Rssi reason */
		*pu1RssiReason = RSSI_REASON_RX_BLOCKING;
	}

	/* update Specific Rx path bitmap */
	*pu1SpeRssiIdx = u1SpeRssiIdx;
}

/*
 *  Function: check RSSI Significance path
 *
 *  Parameter:
 *
 *	  @ pAd
 *
 *	  @ pcRSSI: pointer of array of RSSI values
 *
 *	  @ ucRSSIThManual: RSSI Significance Threshold. If this value is 0xFF, program will use dynamic threshold.
 *
 *	  @ ucBandIdx: DBDC Band Index
 *
 *  Return:
 *
 *	  @ ucRxIdx: RSSI Significant path index bitmap. 0x5 means WF0 and WF2. 0x0 mean no RSSI significant path.
 */

UINT8 LinkTestRssiCheckItem(RTMP_ADAPTER *pAd, PCHAR pi1Rssi, UINT8 u1BandIdx, ENUM_RSSI_CHECK_SOURCE eRssiCheckSrc, UINT8 u1WlanId)
{
	UINT8 ucRxIdx = 0;
	UINT8 u1RssiNum = 0;
	CHAR i1RssiBuffer[WF_NUM] = {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};
	CHAR cRssiBackup[WF_NUM] = {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};

	/* Rssi source handler */
	LinkTestRssiGet(pAd, eRssiCheckSrc, u1WlanId, i1RssiBuffer);

	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): original Rssi: (%d : %d : %d : %d) \n",
															__func__, pi1Rssi[WF0], pi1Rssi[WF1], pi1Rssi[WF2], pi1Rssi[WF3]));

	if (pAd->CommonCfg.dbdc_mode) {
		/* update Rssi Num */
		u1RssiNum = RX_STREAM_PATH_DBDC_MODE;
		/* Rssi pre-prossing for comparison */
		switch (u1BandIdx) {
		case BAND0:
			os_move_mem(cRssiBackup, i1RssiBuffer, u1RssiNum);
			break;
		case BAND1:
			os_move_mem(cRssiBackup, i1RssiBuffer + 2, u1RssiNum);
			break;
		default:
			break;
		}
	} else {
		/* update Rssi Num */
		u1RssiNum = RX_STREAM_PATH_SINGLE_MODE;
		/* Rssi pre-prossing for comparison */
		os_move_mem(cRssiBackup, i1RssiBuffer, u1RssiNum);
	}

	/* Rssi Significance check */
	ucRxIdx = LinkTestRssiComp(pAd, i1RssiBuffer, u1RssiNum, eRssiCheckSrc);

	return ucRxIdx;
}

UINT8 LinkTestRssiComp(RTMP_ADAPTER *pAd, PCHAR pi1Rssi, UINT8 u1RssiNum, ENUM_RSSI_CHECK_SOURCE eRssiCheckSrc)
{
	CHAR cRssiBackup[4] = {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};
	UINT8 u1RxPathRssiOrder[4] = {0, 1, 2, 3};
	UINT8 u1SpecificRxPathBitMap = 0;
	UINT8 u1AntIdx, u1AntIdx2;

	/* sanity check */
	if (u1RssiNum < 2) {
		MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Rssi Num not enough!! ( RssiNum < 2) \n", __func__));
		return 0;
	}

	/* Backup Rssi info to buffer */
	os_move_mem(cRssiBackup, pi1Rssi, u1RssiNum);

	/* Bubble sorting for Rssi (from small to large) */
	for (u1AntIdx = 0; u1AntIdx < u1RssiNum - 1; u1AntIdx++) {
		for (u1AntIdx2 = 0; u1AntIdx2 < u1RssiNum - u1AntIdx - 1; u1AntIdx2++) {
			if (cRssiBackup[u1AntIdx2] > cRssiBackup[u1AntIdx2 + 1]) {
				/* Swap Rssi value */
				LinkTestSwap(cRssiBackup + u1AntIdx2, cRssiBackup + u1AntIdx2 + 1);
				/* Swap Rx Path order */
				LinkTestSwap(u1RxPathRssiOrder + u1AntIdx2, u1RxPathRssiOrder + u1AntIdx2 + 1);
			}
		}
	}

	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Reordered Rssi: (%d : %d : %d : %d) \n",
																__func__, cRssiBackup[0], cRssiBackup[1], cRssiBackup[2], cRssiBackup[3]));
	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Reordered Rx Path: (%d : %d : %d : %d) \n",
																__func__, u1RxPathRssiOrder[0], u1RxPathRssiOrder[1], u1RxPathRssiOrder[2], u1RxPathRssiOrder[3]));

	/* update significant Rx path Num */
	u1SpecificRxPathBitMap = LinkTestRssiSpecificRxPath(pAd, cRssiBackup, u1RxPathRssiOrder, u1RssiNum, eRssiCheckSrc);

	/* return final Significant Rx Path BitMap */
	return u1SpecificRxPathBitMap;
}

UINT8 LinkTestRssiSpecificRxPath(RTMP_ADAPTER *pAd, PCHAR pi1Rssi, PUINT8 pu1RxPathRssiOrder, UINT8 u1RssiNum, ENUM_RSSI_CHECK_SOURCE eRssiCheckSrc)
{
	UINT8 u1RssiTh;
	UINT8 u1AntIdx;
	UINT8 ucSigRxPathNum = 0;
	UINT8 ucRxPathNumCnt = 0;
	UINT8 u1SpecificRxPathBitMap = 0;

	switch (eRssiCheckSrc) {
	case RSSI_CHECK_WTBL_RSSI:
		/* update Rssi Threshold for check */
		u1RssiTh = pAd->ucRssiSigniTh;

		/* check specific Rx path Num */
		for (u1AntIdx = 0; u1AntIdx < u1RssiNum - 1; u1AntIdx++) {
			if ((pi1Rssi[u1AntIdx + 1] - pi1Rssi[u1AntIdx]) > u1RssiTh) {
				ucSigRxPathNum = u1RssiNum - u1AntIdx - 1;
				break;
			}
		}

		/* config specific Rx Path BitMap */
		if (ucSigRxPathNum != 0) {
			for (u1AntIdx = u1RssiNum - 1, ucRxPathNumCnt = 0;
				ucRxPathNumCnt < ucSigRxPathNum; u1AntIdx--, ucRxPathNumCnt++) {
				/* Enable specific Rx Path Index */
				u1SpecificRxPathBitMap |= (1 << pu1RxPathRssiOrder[u1AntIdx]);
			}
		}
		break;

	case RSSI_CHECK_BBP_WBRSSI:
		/* update Rssi Threshold for check */
		u1RssiTh = pAd->cWBRssiTh;

		/* config specific Rx Path BitMap */
		for (u1AntIdx = 0; u1AntIdx < u1RssiNum; u1AntIdx++) {
			/* Enable specific Rx Path Index (WBRssi is larger than Threshold) */
			if (pi1Rssi[u1AntIdx] > u1RssiTh)
				u1SpecificRxPathBitMap |= (1 << pu1RxPathRssiOrder[u1AntIdx]);
		}
		break;

	case RSSI_CHECK_BBP_IBRSSI:
		/* update Rssi Threshold for check */
		u1RssiTh = pAd->cIBRssiTh;

		/* config specific Rx Path BitMap */
		for (u1AntIdx = 0; u1AntIdx < u1RssiNum; u1AntIdx++) {
			/* Enable specific Rx Path Index (IBRssi is larger than Threshold) */
			if (pi1Rssi[u1AntIdx] > u1RssiTh)
				u1SpecificRxPathBitMap |= (1 << pu1RxPathRssiOrder[u1AntIdx]);
		}
		break;

	default:
		break;
	}

	return ucSigRxPathNum;
}

VOID LinkTestSwap(PCHAR pi1Value1, PCHAR pi1Value2)
{
	CHAR i1Temp;

	/* backup value */
	i1Temp = *pi1Value1;

	/* assign value1 to be value2 */
	*pi1Value1 = *pi1Value2;

	/* assign value2 to be backup value */
	*pi1Value2 = i1Temp;
}
#endif /* LINK_TEST_SUPPORT */

BOOLEAN asic_bss_beacon_exit(struct _RTMP_ADAPTER *pAd)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	/* beacon init for USB/SDIO */
	if (arch_ops->arch_bss_beacon_exit)
		arch_ops->arch_bss_beacon_exit(pAd);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

BOOLEAN asic_bss_beacon_stop(struct _RTMP_ADAPTER *pAd)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	/* beacon stop for USB/SDIO */
	if (arch_ops->arch_bss_beacon_stop)
		arch_ops->arch_bss_beacon_stop(pAd);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

BOOLEAN asic_bss_beacon_start(struct _RTMP_ADAPTER *pAd)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	/* beacon start for USB/SDIO */
	if (arch_ops->arch_bss_beacon_start)
		arch_ops->arch_bss_beacon_start(pAd);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

BOOLEAN asic_bss_beacon_init(struct _RTMP_ADAPTER *pAd)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	/* beacon init for USB/SDIO */
	if (arch_ops->arch_bss_beacon_init)
		arch_ops->arch_bss_beacon_init(pAd);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*! Key word: "RXBLOCKER", "WBRSSI", "IBRSSI"
* \Concept:  Switch 4RX to 1RX by detect WBRSSI
*1
* \Input:	 None
*
* \return:   WBRSSI[MAX] or -1
*/
/*----------------------------------------------------------------------------*/
#ifdef	ETSI_RX_BLOCKER_SUPPORT
UINT8 ETSIWbRssiCheck(
	RTMP_ADAPTER *pAd
)
{
	BOOLEAN	fg1RVaild		   = TRUE;
	UINT8	u1WfIdx;
	UINT8	u1MaxWbRssiIdx	  = 0;
	CHAR	c1MaxWbRssi		 = MINIMUM_POWER_VALUE;
	CHAR	c1WbRssi[WF_NUM]	= {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};
	CHAR	c1IbRssi[WF_NUM]	= {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};

	/* buffer to read CR */
	UINT32	u4WbRssi			= 0;

	/* Read CR (manual command) */
	if (pAd->fgFixWbIBRssiEn) {
		/* WBRSSI */
		c1WbRssi[WF0] = pAd->c1WbRssiWF0;
		c1WbRssi[WF1] = pAd->c1WbRssiWF1;
		c1WbRssi[WF2] = pAd->c1WbRssiWF2;
		c1WbRssi[WF3] = pAd->c1WbRssiWF3;
		/* IBRSSI */
		c1IbRssi[WF0] = pAd->c1IbRssiWF0;
		c1IbRssi[WF1] = pAd->c1IbRssiWF1;
		c1IbRssi[WF2] = pAd->c1IbRssiWF2;
		c1IbRssi[WF3] = pAd->c1IbRssiWF3;
	}
	/* Read CR (HW CR) */
	else {
		if (IS_MT7622(pAd)) { /* for 7622 */
			/* Read WBRSSI/IBRSSI (WF0, WF1) */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
			c1WbRssi[WF1] = ((u4WbRssi >> 16) & BITS(0, 7));/* [23:16] */
			c1WbRssi[WF0] = (u4WbRssi & BITS(0, 7));/* [7:0] */
			c1IbRssi[WF1] = ((u4WbRssi >> 24) & BITS(0, 7));/* [31:24] */
			c1IbRssi[WF0] = ((u4WbRssi >> 8) & BITS(0, 7));/* [15:8] */

			/* Read WBRSSI/IBRSSI (WF2, WF3) */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_AGC_DEBUG_2, &u4WbRssi);
			c1WbRssi[WF3] = ((u4WbRssi >> 16) & BITS(0, 7));/* [23:16] */
			c1WbRssi[WF2] = (u4WbRssi & BITS(0, 7));/* [7:0] */
			c1IbRssi[WF3] = ((u4WbRssi >> 24) & BITS(0, 7));/* [31:24] */
			c1IbRssi[WF2] = ((u4WbRssi >> 8) & BITS(0, 7));/* [15:8] */
		}
		if (IS_MT7615(pAd)) { /* for 7615 */
			/* Read WBRSSI/IBRSSI (WF0, WF1) */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
			c1WbRssi[WF0] = ((u4WbRssi >> 16) & BITS(0, 7));/* [23:16] */
			c1WbRssi[WF1] = (u4WbRssi & BITS(0, 7));/* [7:0] */
			c1IbRssi[WF0] = ((u4WbRssi >> 24) & BITS(0, 7));/* [31:24] */
			c1IbRssi[WF1] = ((u4WbRssi >> 8) & BITS(0, 7));/* [15:8] */

			/* Read WBRSSI/IBRSSI (WF2, WF3) */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_AGC_DEBUG_2, &u4WbRssi);
			c1WbRssi[WF2] = ((u4WbRssi >> 16) & BITS(0, 7));/* [23:16] */
			c1WbRssi[WF3] = (u4WbRssi & BITS(0, 7));/* [7:0] */
			c1IbRssi[WF2] = ((u4WbRssi >> 24) & BITS(0, 7));/* [31:24] */
			c1IbRssi[WF3] = ((u4WbRssi >> 8) & BITS(0, 7));/* [15:8] */
		}
	}

	/* log for check Rssi Read (WBRSSI/IBRSSI) */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" c1WbRssi: WF0: %x, WF1: %x, WF2: %x, WF3: %x\n", c1WbRssi[WF0]&0xFF, c1WbRssi[WF1]&0xFF, c1WbRssi[WF2]&0xFF, c1WbRssi[WF3]&0xFF));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" c1IbRssi: WF0: %x, WF1: %x, WF2: %x, WF3: %x\n", c1IbRssi[WF0]&0xFF, c1IbRssi[WF1]&0xFF, c1IbRssi[WF2]&0xFF, c1IbRssi[WF3]&0xFF));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------------------------------------------------\n"));
	/* ---------------- */


	/* CR risk - no expected 0x80 value on WF0/WF1 ; WF2/WF3  */
	if (((c1WbRssi[WF0]&0xFF) == 0x80) || ((c1WbRssi[WF2]&0xFF) == 0x80)) {

		fg1RVaild = TRUE;
		/* log for check Rssi Read (WBRSSI/IBRSSI) */
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------------------------------------------------\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" CR risk !!\n"));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------------------------------------------------\n"));
		/* ---------------- */
	}
	/* No CR risk */
	else {

	/* Find Max Rssi */
	for (u1WfIdx = WF0; u1WfIdx < WF_NUM; u1WfIdx++) {
		if (c1WbRssi[u1WfIdx] > c1MaxWbRssi) {
			/* update Max WBRSSI value */
			c1MaxWbRssi = c1WbRssi[u1WfIdx];
			/* update Max WBRSSI index */
			u1MaxWbRssiIdx = u1WfIdx;
		}
	}


	/* log Max Rssi Value and Max Rssi Index */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------(4R State)-------------------------------------\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" u1WfIdx: %x\n", u1WfIdx));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" c1MaxWbRssi: %x\n", c1MaxWbRssi));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------------------------------------------------\n"));
	/* ---------------- */


	/* check state transition status (4R->1R) */
	if (c1MaxWbRssi >= pAd->c1RWbRssiHTh) {
		for (u1WfIdx = WF0; u1WfIdx < WF_NUM; u1WfIdx++) {
			if ((u1WfIdx != u1MaxWbRssiIdx) && \
				((c1WbRssi[u1WfIdx] > pAd->c1RWbRssiLTh) || (c1IbRssi[u1WfIdx] > pAd->c1RIbRssiLTh))) {
				fg1RVaild = FALSE;
			} else
				fg1RVaild = TRUE;
		}
	} else
		fg1RVaild = FALSE;

	}

	/* log check flag to 1R */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" fg1RVaild: %x\n", fg1RVaild));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------------------------------------------------\n"));
	/* ---------------- */

	/* check 1R transition flag */
	if (fg1RVaild)
		return u1MaxWbRssiIdx;
	else
		return 0xFF;
}
#endif /* end ETSI_RX_BLOCKER_SUPPORT */

VOID asic_write_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->write_tmac_info)
		arch_ops->write_tmac_info(pAd, buf, pTxBlk);
	else
		AsicNotSupportFunc(pAd, __func__);
}

USHORT asic_write_tx_resource(struct _RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk, BOOLEAN bIsLast, USHORT *freeCnt)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->write_tx_resource)
		return arch_ops->write_tx_resource(pAd, pTxBlk, bIsLast, freeCnt);
	else
		AsicNotSupportFunc(pAd, __func__);
	return 0;
}

VOID asic_write_tmac_info_fixed_rate(struct _RTMP_ADAPTER *pAd,
	UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *pTransmit)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->write_tmac_info_fixed_rate)
		arch_ops->write_tmac_info_fixed_rate(pAd, tmac_info, info, pTransmit);
	else
		AsicNotSupportFunc(pAd, __func__);
	return;
}
#ifdef REDUCE_TX_OVERHEAD
inline INT32 asic_write_txp_info(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk)
#else
INT32 asic_write_txp_info(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk)
#endif
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->write_txp_info)
		return arch_ops->write_txp_info(pAd, buf, pTxBlk);
	else
		AsicNotSupportFunc(pAd, __func__);
	return NDIS_STATUS_FAILURE;
}

VOID asic_dump_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->dump_tmac_info)
		return arch_ops->dump_tmac_info(pAd, tmac_info);
}

#ifdef REDUCE_TX_OVERHEAD
inline UCHAR *asic_get_hif_buf(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk, UINT8 hif_idx, UCHAR frame_type)
#else
UCHAR *asic_get_hif_buf(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk, UINT8 hif_idx, UCHAR frame_type)
#endif
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->get_hif_buf)
		return arch_ops->get_hif_buf(pAd, tx_blk, hif_idx, frame_type);
	else
		AsicNotSupportFunc(pAd, __func__);
	return NULL;

}

#ifdef REDUCE_TX_OVERHEAD
inline UINT32 asic_get_resource_idx(struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev, enum PACKET_TYPE pkt_type, UINT8 que_idx)
#else
UINT32 asic_get_resource_idx(struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev, enum PACKET_TYPE pkt_type, UINT8 que_idx)
#endif
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->get_resource_idx)
		return arch_ops->get_resource_idx(pAd, wdev, pkt_type, que_idx);
	else
		AsicNotSupportFunc(pAd, __func__);
	return 0;

}

inline INT asic_check_hw_resource(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR resource_idx)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->check_hw_resource)
		return arch_ops->check_hw_resource(pAd, wdev, resource_idx);
	else
		AsicNotSupportFunc(pAd, __func__);
	return NDIS_STATUS_FAILURE;
}

inline VOID asic_set_resource_state(struct _RTMP_ADAPTER *pAd, UCHAR resource_idx, BOOLEAN state)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->set_resource_state)
		arch_ops->set_resource_state(pAd, resource_idx, state);
	else
		AsicNotSupportFunc(pAd, __func__);
}

UINT32 asic_get_tx_resource_free_num(struct _RTMP_ADAPTER *pAd, UINT8 que_idx)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->get_tx_resource_free_num)
		return arch_ops->get_tx_resource_free_num(pAd, que_idx);
	else
		AsicNotSupportFunc(pAd, __func__);
	return 0;
}

inline VOID *asic_get_pkt_from_rx_resource(struct _RTMP_ADAPTER *pAd, BOOLEAN *re_schedule,
						UINT32 *rx_pending, UCHAR ring_no)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->get_pkt_from_rx_resource)
		return arch_ops->get_pkt_from_rx_resource(pAd, re_schedule, rx_pending, ring_no);
	else
		AsicNotSupportFunc(pAd, __func__);
	return NULL;
}


inline UINT32 asic_rx_pkt_process(struct _RTMP_ADAPTER *pAd, UINT8 hif_idx, struct _RX_BLK *pRxBlk, VOID *pRxPacket)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->rx_pkt_process)
		return arch_ops->rx_pkt_process(pAd, hif_idx, pRxBlk, pRxPacket);
	else
		AsicNotSupportFunc(pAd, __func__);
	return NDIS_STATUS_FAILURE;
}

#ifdef MT7626_E2_SUPPORT
inline UINT32 asic_rx2_pkt_process(struct _RTMP_ADAPTER *pAd, UINT8 hif_idx, struct _RX_BLK *pRxBlk, VOID *pRxPacket)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->rx2_pkt_process)
		return arch_ops->rx2_pkt_process(pAd, hif_idx, pRxBlk, pRxPacket);
	else
		AsicNotSupportFunc(pAd, __func__);
	return NDIS_STATUS_FAILURE;
}

inline VOID *asic_get_pkt_from_rx2_resource(struct _RTMP_ADAPTER *pAd, BOOLEAN *re_schedule,
		UINT32 *rx_pending, UCHAR ring_no)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->get_pkt_from_rx2_resource)
		return arch_ops->get_pkt_from_rx2_resource(pAd, re_schedule, rx_pending, ring_no);
	else
		AsicNotSupportFunc(pAd, __func__);
	return NULL;
}
#endif

inline UINT32 asic_get_packet_type(struct _RTMP_ADAPTER *pAd, VOID *rx_packet)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->get_packet_type)
		return arch_ops->get_packet_type(pAd, rx_packet);
	else
		AsicNotSupportFunc(pAd, __func__);
	return NDIS_STATUS_FAILURE;

}

INT32 asic_trans_rxd_into_rxblk(RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, VOID *rx_pkt)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->trans_rxd_into_rxblk)
		return arch_ops->trans_rxd_into_rxblk(pAd, rx_blk, rx_pkt);
	else
		AsicNotSupportFunc(pAd, __func__);

	/* Return RMAC Info Length */
	return 0;
}

VOID asic_kickout_data_tx(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk, UCHAR que_idx)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->kickout_data_tx)
		arch_ops->kickout_data_tx(pAd, tx_blk, que_idx);
	else
		AsicNotSupportFunc(pAd, __func__);
}

INT asic_mlme_hw_tx(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *pTransmit, struct _TX_BLK *tx_blk)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->mlme_hw_tx)
		return arch_ops->mlme_hw_tx(pAd, tmac_info, info, pTransmit, tx_blk);
	else
		AsicNotSupportFunc(pAd, __func__);
	return NDIS_STATUS_SUCCESS;
}

INT asic_hw_tx(struct _RTMP_ADAPTER *ad, struct _TX_BLK *tx_blk)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->hw_tx)
		return arch_ops->hw_tx(ad, tx_blk);
	else
		AsicNotSupportFunc(ad, __func__);
	return NDIS_STATUS_SUCCESS;
}

inline BOOLEAN asic_rx_done_handle(struct _RTMP_ADAPTER *ad)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->rx_done_handle)
		return arch_ops->rx_done_handle(ad);
	else
		AsicNotSupportFunc(ad, __func__);
	return TRUE;
}

#ifdef MT7626_E2_SUPPORT
inline BOOLEAN asic_rx2_done_handle(struct _RTMP_ADAPTER *ad)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->rx2_done_handle)
		return arch_ops->rx2_done_handle(ad);
	else
		AsicNotSupportFunc(ad, __func__);
	return TRUE;
}
#endif

BOOLEAN asic_tx_dma_done_handle(struct _RTMP_ADAPTER *ad, UINT8 hif_idx)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->tx_dma_done_handle)
		return arch_ops->tx_dma_done_handle(ad, hif_idx);
	else
		AsicNotSupportFunc(ad, __func__);
	return TRUE;
}

#if defined(NF_SUPPORT) || defined(OFFCHANNEL_SCAN_FEATURE)

BOOLEAN asic_calculate_nf(struct _RTMP_ADAPTER *ad, UCHAR band_idx)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->arch_calculate_nf)
		arch_ops->arch_calculate_nf(ad, band_idx);
	else
		AsicNotSupportFunc(ad, __func__);
	return TRUE;
}

BOOLEAN asic_reset_enable_nf_registers(struct _RTMP_ADAPTER *ad, UCHAR band_idx)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->arch_reset_enable_nf_registers)
		arch_ops->arch_reset_enable_nf_registers(ad, band_idx);
	else
		AsicNotSupportFunc(ad, __func__);
	return TRUE;
}

BOOLEAN asic_enable_nf_support(struct _RTMP_ADAPTER *ad)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->arch_enable_nf_support)
		arch_ops->arch_enable_nf_support(ad);
	else
		AsicNotSupportFunc(ad, __func__);
	return TRUE;
}
#endif

