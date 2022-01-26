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
    fsm_sync.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
				2016-08-18		AP/APCLI/STA SYNC FSM Integration
*/


#include "rt_config.h"

#ifdef CONFIG_AP_SUPPORT
static INT build_country_power_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	UCHAR Environment = 0x20;

	/* add Country IE and power-related IE */
	if (pAd->CommonCfg.bCountryFlag ||
		(wdev->channel > 14 && pAd->CommonCfg.bIEEE80211H == TRUE)
#ifdef DOT11K_RRM_SUPPORT
		|| IS_RRM_ENABLE(wdev)
#endif /* DOT11K_RRM_SUPPORT */
	   ) {
		ULONG TmpLen = 0, TmpLen2 = 0;
		UCHAR TmpLen3 = 0;
		UCHAR TmpFrame[256] = {0};
		const UCHAR CountryIe = IE_COUNTRY;
		PCH_DESC pChDesc = NULL;
		UCHAR op_ht_bw = wlan_config_get_ht_bw(wdev);

		if (WMODE_CAP_2G(wdev->PhyMode)) {
			if (pAd->CommonCfg.pChDesc2G != NULL)
				pChDesc = (PCH_DESC)pAd->CommonCfg.pChDesc2G;
			else
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: pChDesc2G is NULL !!!\n", __func__));
		} else if (WMODE_CAP_5G(wdev->PhyMode)) {
			if (pAd->CommonCfg.pChDesc5G != NULL)
				pChDesc = (PCH_DESC)pAd->CommonCfg.pChDesc5G;
			else
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: pChDesc5G is NULL !!!\n", __func__));
		}

		/*
			Only APs that comply with 802.11h or 802.11k are required to include
			the Power Constraint element (IE=32) and
			the TPC Report element (IE=35) and
			the VHT Transmit Power Envelope element (IE=195)
			in beacon frames and probe response frames
		*/
		if ((wdev->channel > 14 && pAd->CommonCfg.bIEEE80211H == TRUE)
#ifdef DOT11K_RRM_SUPPORT
			|| IS_RRM_ENABLE(wdev)
#endif /* DOT11K_RRM_SUPPORT */
		   ) {
			/* prepare power constraint IE */
			MAKE_IE_TO_BUF(buf, PowerConstraintIE, 3, len);
			/* prepare TPC Report IE */
			InsertTpcReportIE(pAd, (UCHAR *)(buf + len), (ULONG *)&len,
							  GetMaxTxPwr(pAd), 0);
#ifdef DOT11_VHT_AC

			/* prepare VHT Transmit Power Envelope IE */
			if (WMODE_CAP_AC(wdev->PhyMode)) {
				const UINT8 vht_txpwr_env_ie = IE_VHT_TXPWR_ENV;
				UINT8 ie_len;
				VHT_TXPWR_ENV_IE txpwr_env;

				ie_len = build_vht_txpwr_envelope(pAd, wdev, (UCHAR *)&txpwr_env);
				MAKE_IE_TO_BUF(buf, &vht_txpwr_env_ie, 1, len);
				MAKE_IE_TO_BUF(buf, &ie_len, 1, len);
				MAKE_IE_TO_BUF(buf, &txpwr_env, ie_len, len);
			}

#endif /* DOT11_VHT_AC */
		}

		NdisZeroMemory(TmpFrame, sizeof(TmpFrame));
#ifdef EXT_BUILD_CHANNEL_LIST
		BuildBeaconChList(pAd, wdev, TmpFrame, &TmpLen2);
#else
		{
			UINT i = 0;
			UCHAR MaxTxPower = GetCuntryMaxTxPwr(pAd, wdev->PhyMode, wdev, op_ht_bw);

			if (pChDesc) {
				for (i = 0; pChDesc[i].FirstChannel != 0; i++) {
					MakeOutgoingFrame(TmpFrame + TmpLen2, &TmpLen,
									  1, &pChDesc[i].FirstChannel,
									  1, &pChDesc[i].NumOfCh,
									  1, &MaxTxPower,
									  END_OF_ARGS);
					TmpLen2 += TmpLen;
				}
			}
		}
#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef DOT11K_RRM_SUPPORT

		if (IS_RRM_ENABLE(wdev)) {
			UCHAR reg_class = get_regulatory_class(pAd, wdev->channel, wdev->PhyMode, wdev);

			TmpLen2 = 0;
			NdisZeroMemory(TmpFrame, sizeof(TmpFrame));
			RguClass_BuildBcnChList(pAd, TmpFrame, &TmpLen2, wdev->PhyMode, reg_class);
		}

#endif /* DOT11K_RRM_SUPPORT */
#ifdef MBO_SUPPORT
					if (IS_MBO_ENABLE(wdev))
						Environment = MBO_AP_USE_GLOBAL_OPERATING_CLASS;
#endif /* MBO_SUPPORT */

		/* need to do the padding bit check, and concatenate it */
		MAKE_IE_TO_BUF(buf, &CountryIe, 1, len);

		if ((TmpLen2 % 2) == 0) {
			TmpLen3 = (UCHAR)(TmpLen2 + 4);
			TmpLen2++;
		} else
			TmpLen3 = (UCHAR)(TmpLen2 + 3);

		MAKE_IE_TO_BUF(buf, &TmpLen3, 1, len);
		MAKE_IE_TO_BUF(buf, &pAd->CommonCfg.CountryCode[0], 1, len);
		MAKE_IE_TO_BUF(buf, &pAd->CommonCfg.CountryCode[1], 1, len);
		MAKE_IE_TO_BUF(buf, &Environment, 1, len);
		MAKE_IE_TO_BUF(buf, TmpFrame, TmpLen2, len);
	} /* Country IE - */

	return len;
}

static INT build_ch_switch_announcement_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	struct DOT11_H *pDot11h = NULL;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return 0;

	if ((wdev->channel > 14)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pDot11h->RDMode == RD_SWITCHING_MODE)) {
		UCHAR CSAIe = IE_CHANNEL_SWITCH_ANNOUNCEMENT;
		UCHAR CSALen = 3;
		UCHAR CSAMode = 1;

		MAKE_IE_TO_BUF(buf, &CSAIe, 1, len);
		MAKE_IE_TO_BUF(buf, &CSALen, 1, len);
		MAKE_IE_TO_BUF(buf, &CSAMode, 1, len);
		MAKE_IE_TO_BUF(buf, &wdev->channel, 1, len);
		MAKE_IE_TO_BUF(buf, &pDot11h->CSCount, 1, len);
#ifdef DOT11_N_SUPPORT

		if (pAd->CommonCfg.bExtChannelSwitchAnnouncement) {
			HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE HtExtChannelSwitchIe = {0};

			build_ext_channel_switch_ie(pAd, &HtExtChannelSwitchIe, wdev->channel, wdev->PhyMode, wdev);
			MAKE_IE_TO_BUF(buf, &HtExtChannelSwitchIe, sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE), len);
		}

#endif /* DOT11_N_SUPPORT */
	}

	return len;
}

static BOOLEAN ap_join_peer_response_matched(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, BCN_IE_LIST *ie_list, MLME_QUEUE_ELEM *Elem)
{
	ASSERT(0);
	return FALSE;
}

static BOOLEAN ap_join_peer_response_updated(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, BCN_IE_LIST *ie_list, MLME_QUEUE_ELEM *Elem,
		NDIS_802_11_VARIABLE_IEs *pVIE, USHORT LenVIE)
{
	ASSERT(0);
	return FALSE;
}

static BOOLEAN ap_rx_peer_response_allowed(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, BCN_IE_LIST *ie_list, MLME_QUEUE_ELEM *Elem)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	UCHAR WorkChannel = Elem->Channel;
	struct freq_oper oper;

#ifdef AUTOMATION
	rx_peer_beacon_check(pAd, ie_list, Elem);
#endif /* AUTOMATION */

	if (VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid)) {
		pEntry = MacTableLookup(pAd, ie_list->Addr2); /* Found the pEntry from Peer Bcn Content */

		if (!pEntry || !pEntry->wdev)
			return FALSE;

		WorkChannel = pEntry->wdev->channel;
	}

	/* ignore BEACON not in this channel */
	if ((ie_list->Channel != WorkChannel)
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		&& (pAd->CommonCfg.bOverlapScanning == FALSE)
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef CFG80211_MULTI_STA
		&& (!RTMP_CFG80211_MULTI_STA_ON(pAd, pAd->cfg80211_ctrl.multi_sta_net_dev))
#endif /* CFG80211_MULTI_STA */
	   )
		return FALSE;

#ifdef DOT11_N_SUPPORT

	hc_radio_query_by_channel(pAd, WorkChannel, &oper);

	if (WorkChannel <= 14
		&& (oper.ht_bw == HT_BW_40)
#ifdef DOT11N_DRAFT3
		&& (pAd->CommonCfg.bOverlapScanning == FALSE)
#endif /* DOT11N_DRAFT3 */
	   ) {
		if (WorkChannel <= 14) {
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)

			if (OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED) &&
				RTMP_CFG80211_MULTI_STA_ON(pAd, pAd->cfg80211_ctrl.multi_sta_net_dev)
			   ) {
				if (ie_list->Channel != WorkChannel) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Channel=%d is not equal as  band Channel = %d.\n",
							 ie_list->Channel, WorkChannel));
				}
			} else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
				if (((oper.cen_ch_1 + 2) != ie_list->Channel) &&
					((oper.cen_ch_1 - 2) != ie_list->Channel)) {
					/*
									MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x is a legacy BSS (%d)\n",
												Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5], Channel));
					*/
					return FALSE;
				}
		}
	}

#endif /* DOT11_N_SUPPORT */
	return TRUE;
}

static BOOLEAN ap_rx_peer_response_updated(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, BCN_IE_LIST *ie_list, MLME_QUEUE_ELEM *Elem,
		NDIS_802_11_VARIABLE_IEs *pVIE, USHORT LenVIE)
{
	UCHAR WorkChannel = Elem->Channel;
	PMAC_TABLE_ENTRY pEntry = NULL;
	CHAR RealRssi = 0;
	UCHAR Rates[MAX_LEN_OF_SUPPORTED_RATES] = {0}, *pRates = NULL, RatesLen = 0;
	BOOLEAN LegacyBssExist = FALSE;
	UCHAR MaxSupportedRate = 0;
#ifdef OCE_SUPPORT
	P_OCE_CTRL	pOceCtrl = &wdev->OceCtrl;
	int OceBOnlyPresentOldValue;
#endif /* OCE_SUPPORT */

	if (VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid)) {
		pEntry = MacTableLookup(pAd, ie_list->Addr2); /* Found the pEntry from Peer Bcn Content */
		if (pEntry)
			WorkChannel = pEntry->wdev->channel;
	}

	RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
						   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
						   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));

#ifdef APCLI_SUPPORT
	/*
	* Updating the Peer AP's existence when it's beacon listened on AP's wdev
	*/
	ApCliCheckPeerExistence(pAd, ie_list->Ssid, ie_list->SsidLen, ie_list->Channel);
#endif

#ifdef IDS_SUPPORT
	/* Conflict SSID detection */
	RTMPConflictSsidDetection(pAd, (PUCHAR)ie_list->Ssid, ie_list->SsidLen,
							  (CHAR)Elem->rssi_info.raw_rssi[0],
							  (CHAR)Elem->rssi_info.raw_rssi[1],
							  (CHAR)Elem->rssi_info.raw_rssi[2]);
#endif /* IDS_SUPPORT */
#ifdef DOT11_N_SUPPORT

	/* 40Mhz BSS Width Trigger events Intolerant devices */
	if ((RealRssi > OBSS_BEACON_RSSI_THRESHOLD) &&
		(ie_list->HtCapability.HtCapInfo.Forty_Mhz_Intolerant)) /* || (HtCapabilityLen == 0))) */
		Handle_BSS_Width_Trigger_Events(pAd, WorkChannel);

#endif /* DOT11_N_SUPPORT */
	pRates = (PUCHAR)Rates;
	SupportRate(ie_list->SupRate, ie_list->SupRateLen, ie_list->ExtRate, ie_list->ExtRateLen, &pRates, &RatesLen, &MaxSupportedRate);

	if ((ie_list->Erp & 0x01) || (RatesLen <= 4))
		LegacyBssExist = TRUE;
	else
		LegacyBssExist = FALSE;

#ifdef OCE_SUPPORT
	if (!pOceCtrl->Scan11bOceAPTimerRunning) {
		if (WMODE_CAP_2G(wdev->PhyMode) && RatesLen <= 4) {
			OceBOnlyPresentOldValue = OCE_GET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
			OCE_11B_ONLY_PRESENT_MASK, OCE_11B_ONLY_PRESENT_OFFSET);
			OCE_SET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
			1, OCE_11B_ONLY_PRESENT_MASK, OCE_11B_ONLY_PRESENT_OFFSET);

			if (OceBOnlyPresentOldValue != OCE_GET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
				OCE_11B_ONLY_PRESENT_MASK, OCE_11B_ONLY_PRESENT_OFFSET))
				OceSendFilsDiscoveryAction(pAd, wdev);
		}
		RTMPSetTimer(&pOceCtrl->Scan11bOceAPTimer, OCE_SCAN_11BOCEAP_PERIOD_TIME);
					pOceCtrl->Scan11bOceAPTimerRunning = TRUE;
	}
#endif


	if (LegacyBssExist && pAd->CommonCfg.DisableOLBCDetect == 0) {
		pAd->ApCfg.LastOLBCDetectTime = pAd->Mlme.Now32;
	}

#ifdef DOT11_N_SUPPORT

	if ((ie_list->HtCapabilityLen == 0) && (RealRssi > OBSS_BEACON_RSSI_THRESHOLD)) {
		pAd->ApCfg.LastNoneHTOLBCDetectTime = pAd->Mlme.Now32;
	}

#endif /* DOT11_N_SUPPORT */


#ifdef WDS_SUPPORT

	if (pAd->WdsTab.Mode != WDS_DISABLE_MODE) {
		if (pAd->WdsTab.flg_wds_init) {
			MAC_TABLE_ENTRY *pEntry;
			BOOLEAN bWmmCapable;
			/* check BEACON does in WDS TABLE. */
			pEntry = WdsTableLookup(pAd, ie_list->Addr2, FALSE);
			bWmmCapable = ie_list->EdcaParm.bValid ? TRUE : FALSE;

			if (pEntry && WDS_IF_UP_CHECK(pAd, pEntry->func_tb_idx))
				WdsPeerBeaconProc(pAd, pEntry, MaxSupportedRate, RatesLen, bWmmCapable,	ie_list);
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(), ERROR!! Beacon comes before wds_init\n", __func__));
	}

#endif /* WDS_SUPPORT */
	bss_coex_insert_effected_ch_list(pAd, WorkChannel, ie_list, wdev);


	if (pAd->ApCfg.AutoChannel_Channel) {
		UCHAR BandIdx = HcGetBandByWdev(wdev);
		AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

		if (pAutoChCtrl->pChannelInfo &&
			AutoChBssSearchWithSSID(pAd, ie_list->Bssid, (PUCHAR)ie_list->Ssid, ie_list->SsidLen, ie_list->Channel, wdev) == BSS_NOT_FOUND)
			pAutoChCtrl->pChannelInfo->ApCnt[pAd->ApCfg.current_channel_index]++;

		AutoChBssInsertEntry(pAd, ie_list->Bssid, ie_list->Ssid, ie_list->SsidLen, ie_list->Channel, ie_list->NewExtChannelOffset, RealRssi, wdev);
	}

	return TRUE;
}

static BOOLEAN ap_probe_response_allowed(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, struct _PEER_PROBE_REQ_PARAM *ProbeReqParam, MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN isNeedRsp = TRUE;
	BSS_STRUCT *mbss;
	struct freq_oper oper;
#ifdef OCE_SUPPORT
		P_OCE_CTRL pOceCtrl = NULL;
#endif
#ifdef WSC_AP_SUPPORT
	UCHAR Addr3[MAC_ADDR_LEN] = {0};
	PFRAME_802_11 pFrame = (PFRAME_802_11)Elem->Msg;

	COPY_MAC_ADDR(Addr3, pFrame->Hdr.Addr3);
#endif /* WSC_AP_SUPPORT */
	mbss = wdev->func_dev;

	if ((wdev->if_dev == NULL) ||
		((wdev->if_dev != NULL) && !(RTMP_OS_NETDEV_STATE_RUNNING(wdev->if_dev)))) {
		/* the interface is down, so we can not send probe response */
		return FALSE;
	}

	hc_radio_query_by_channel(pAd, wdev->channel, &oper);
	if ((Elem->Channel != oper.prim_ch) && (Elem->Channel != oper.cen_ch_1))
		return FALSE;

	if (((((ProbeReqParam->SsidLen == 0) && (!mbss->bHideSsid)) ||
		  ((ProbeReqParam->SsidLen == mbss->SsidLen) && NdisEqualMemory(ProbeReqParam->Ssid, mbss->Ssid, (ULONG) ProbeReqParam->SsidLen)))
#ifdef CONFIG_HOTSPOT
		 && ProbeReqforHSAP(pAd, wdev->func_idx, ProbeReqParam)
#endif /* CONFIG_HOTSPOT */
		)
#ifdef WSC_AP_SUPPORT
		/* buffalo WPS testbed STA send ProbrRequest ssid length = 32 and ssid are not AP , but DA are AP. for WPS test send ProbeResponse */
		|| ((ProbeReqParam->SsidLen == 32) && MAC_ADDR_EQUAL(Addr3, wdev->bssid) && (mbss->bHideSsid == 0))
#endif /* WSC_AP_SUPPORT */
	   )
		;
	else {
		return FALSE; /* check next BSS */
	}
#ifdef BAND_STEERING
		if (pAd->ApCfg.BandSteering
		) {
			BOOLEAN bBndStrgCheck = TRUE;

			bBndStrgCheck = BndStrg_CheckConnectionReq(pAd, wdev, ProbeReqParam->Addr2, Elem, ProbeReqParam);
			if (bBndStrgCheck == FALSE)
				return FALSE;
		}
#endif /* BAND_STEERING */

#ifdef OCE_SUPPORT
	pOceCtrl = &wdev->OceCtrl;

	if (ProbeReqParam->MaxChannelTime && (pOceCtrl->MaxChannelTimesUp ||
	    pAd->CommonCfg.BeaconPeriod <= ProbeReqParam->MaxChannelTime)) {
		pOceCtrl->MaxChannelTimesUp = FALSE;
		pOceCtrl->MaxChannelTimerRunning = FALSE;
		return FALSE;
	}
	pOceCtrl->MaxChannelTimesUp = FALSE;
	pOceCtrl->MaxChannelTimerRunning = FALSE;
#endif /* OCE_SUPPORT */

	return isNeedRsp;
}

static BOOLEAN ap_probe_response_xmit(struct _RTMP_ADAPTER *pAd,
									  struct wifi_dev *wdev, struct _PEER_PROBE_REQ_PARAM *ProbeReqParam, MLME_QUEUE_ELEM *Elem)
{
	HEADER_802_11 ProbeRspHdr;
	NDIS_STATUS NStatus;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0, TmpLen;
	struct dev_rate_info *rate;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#ifdef AP_QLOAD_SUPPORT
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);
#endif /* AP_QLOAD_SUPPORT */
	ADD_HT_INFO_IE *addht;
	UCHAR cfg_ht_bw;
	UCHAR op_ht_bw;
	LARGE_INTEGER FakeTimestamp;
	UCHAR DsLen = 1;
	UCHAR ErpIeLen = 1;
	UCHAR PhyMode, SupRateLen;
	BSS_STRUCT *mbss;
	struct _build_ie_info ie_info = {0};
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	struct customer_vendor_ie *ap_vendor_ie;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

	mbss = wdev->func_dev;
	rate = &wdev->rate;
	addht = wlan_operate_get_addht(wdev);
	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	op_ht_bw = wlan_config_get_ht_bw(wdev);
	PhyMode = wdev->PhyMode;
	ie_info.frame_subtype = SUBTYPE_PROBE_RSP;
	ie_info.channel = wdev->channel;
	ie_info.phy_mode = PhyMode;
	ie_info.wdev = wdev;

#ifdef WAPP_SUPPORT
	wapp_send_cli_probe_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), ProbeReqParam->Addr2, Elem);
#endif
#ifdef WH_EVENT_NOTIFIER
	{
		EventHdlr pEventHdlrHook = NULL;

		pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_PROBE_REQ);

		if (pEventHdlrHook && wdev)
			pEventHdlrHook(pAd, wdev, &ProbeReqParam, Elem);
	}
#endif /* WH_EVENT_NOTIFIER */
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	if ((ProbeReqParam->report_param.vendor_ie.element_id == IE_VENDOR_SPECIFIC) &&
		(ProbeReqParam->report_param.vendor_ie.len > 0)) {
		struct probe_req_report pProbeReqReportTemp;
		memset(&pProbeReqReportTemp, 0, sizeof(struct probe_req_report));
		pProbeReqReportTemp.band = (WMODE_CAP_2G(wdev->PhyMode) && wdev->channel <= 14) ? 0 : 1;
		COPY_MAC_ADDR(pProbeReqReportTemp.sta_mac, ProbeReqParam->Addr2);
		pProbeReqReportTemp.vendor_ie.element_id = ProbeReqParam->report_param.vendor_ie.element_id;
		pProbeReqReportTemp.vendor_ie.len = ProbeReqParam->report_param.vendor_ie.len;
		NdisMoveMemory(pProbeReqReportTemp.vendor_ie.custom_ie,
				ProbeReqParam->report_param.vendor_ie.custom_ie,
				ProbeReqParam->report_param.vendor_ie.len);
		RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, RT_PROBE_REQ_REPORT_EVENT,
					NULL, (PUCHAR)&pProbeReqReportTemp,
					MAC_ADDR_LEN + 3 + ProbeReqParam->report_param.vendor_ie.len);
	}
#endif

	/* allocate and send out ProbeRsp frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return FALSE;

#ifdef OCE_SUPPORT
	if (IS_OCE_ENABLE(wdev)
		&& MAC_ADDR_EQUAL(ProbeReqParam->Addr1, BROADCAST_ADDR)
		&& ProbeReqParam->IsOceCapability) /* broadcast probe request && is OCE STA*/
		MgtMacHeaderInit(pAd, &ProbeRspHdr, SUBTYPE_PROBE_RSP, 0, BROADCAST_ADDR,
						 wdev->if_addr, wdev->bssid); /* broadcast probe response */
	else
#endif
	MgtMacHeaderInit(pAd, &ProbeRspHdr, SUBTYPE_PROBE_RSP, 0, ProbeReqParam->Addr2,
					 wdev->if_addr, wdev->bssid);
	{
		SupRateLen = rate->SupRateLen;

		if (PhyMode == WMODE_B)
			SupRateLen = 4;

		MakeOutgoingFrame(pOutBuffer,				  &FrameLen,
						  sizeof(HEADER_802_11),	  &ProbeRspHdr,
						  TIMESTAMP_LEN,			  &FakeTimestamp,
						  2,						  &pAd->CommonCfg.BeaconPeriod,
						  2,						  &mbss->CapabilityInfo,
						  1,						  &SsidIe,
						  1,						  &mbss->SsidLen,
						  mbss->SsidLen,	 mbss->Ssid,
						  1,						  &SupRateIe,
						  1,						  &SupRateLen,
						  SupRateLen,				  rate->SupRate,
						  1,						  &DsIe,
						  1,						  &DsLen,
						  1,						  &wdev->channel,
						  END_OF_ARGS);
	}

	if ((rate->ExtRateLen) && (PhyMode != WMODE_B)) {
		if (WMODE_CAP_2G(wdev->PhyMode) && (wdev->channel <= 14)) {
			MakeOutgoingFrame(pOutBuffer + FrameLen,		&TmpLen,
							  1,						&ErpIe,
							  1,						&ErpIeLen,
							  1,						&pAd->ApCfg.ErpIeContent,
							  1,						&ExtRateIe,
							  1,						&rate->ExtRateLen,
							  rate->ExtRateLen,		rate->ExtRate,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}
#ifdef CONFIG_HOTSPOT_R2
	if ((mbss->HotSpotCtrl.HotSpotEnable == 0) && (mbss->HotSpotCtrl.bASANEnable == 1) && (IS_AKM_WPA2_Entry(wdev))) {
		/* replace RSN IE with OSEN IE if it's OSEN wdev */
		UCHAR RSNIe = IE_WPA;
		extern UCHAR			OSEN_IE[];
		extern UCHAR			OSEN_IELEN;

		MakeOutgoingFrame(pOutBuffer + FrameLen,			&TmpLen,
						  1,							&RSNIe,
						  1,							&OSEN_IELEN,
						  OSEN_IELEN,					OSEN_IE,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	} else
#endif /* CONFIG_HOTSPOT_R2 */
		FrameLen += build_rsn_ie(pAd, wdev, (UCHAR *)(pOutBuffer + FrameLen));

#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(PhyMode) &&
		(wdev->DesiredHtPhyInfo.bHtEnable)) {
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_ht_ies(pAd, &ie_info);
	}

#endif /* DOT11_N_SUPPORT */
	ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
	FrameLen += build_extended_cap_ie(pAd, &ie_info);
#ifdef AP_QLOAD_SUPPORT
	if (pAd->CommonCfg.dbdc_mode == 0)
		pQloadCtrl = HcGetQloadCtrl(pAd);
	else
		pQloadCtrl = (wdev->channel > 14) ? HcGetQloadCtrlByRf(pAd, RFIC_5GHZ) : HcGetQloadCtrlByRf(pAd, RFIC_24GHZ);

	if (pQloadCtrl->FlgQloadEnable != 0) {
#ifdef CONFIG_HOTSPOT_R2

		if (mbss->HotSpotCtrl.QLoadTestEnable == 1)
			FrameLen += QBSS_LoadElementAppend_HSTEST(pAd, pOutBuffer + FrameLen, wdev->func_idx);
		else if (mbss->HotSpotCtrl.QLoadTestEnable == 0)
#endif /* CONFIG_HOTSPOT_R2 */
			FrameLen += QBSS_LoadElementAppend(pAd, pOutBuffer + FrameLen, pQloadCtrl);
	}

#endif /* AP_QLOAD_SUPPORT */
#if defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT)
	if (mbss->HotSpotCtrl.HotSpotEnable)
		MakeHotSpotIE(wdev, &FrameLen, pOutBuffer);

#endif /* defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT) */
#ifdef CONFIG_DOT11U_INTERWORKING
	if (mbss->GASCtrl.b11U_enable) {
		ULONG TmpLen;
		/* Interworking element */
		MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
						  mbss->GASCtrl.InterWorkingIELen,
						  mbss->GASCtrl.InterWorkingIE, END_OF_ARGS);
		FrameLen += TmpLen;
		/* Advertisement Protocol element */
		MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
						  mbss->GASCtrl.AdvertisementProtoIELen,
						  mbss->GASCtrl.AdvertisementProtoIE, END_OF_ARGS);
		FrameLen += TmpLen;
	}
#endif /* CONFIG_DOT11U_INTERWORKING */

	/* add WMM IE here */
	ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
	FrameLen += build_wmm_cap_ie(pAd, &ie_info);
#ifdef DOT11K_RRM_SUPPORT

	if (IS_RRM_ENABLE(wdev)) {
		RRM_InsertRRMEnCapIE(pAd, pOutBuffer + FrameLen, &FrameLen, wdev->func_idx);

		InsertChannelRepIE(pAd, pOutBuffer + FrameLen, &FrameLen,
					   (RTMP_STRING *)pAd->CommonCfg.CountryCode,
					   get_regulatory_class(pAd, mbss->wdev.channel, mbss->wdev.PhyMode, &mbss->wdev),
					   NULL, PhyMode, wdev->func_idx);

#ifndef APPLE_11K_IOT
		/* Insert BSS AC Access Delay IE. */
		RRM_InsertBssACDelayIE(pAd, pOutBuffer + FrameLen, &FrameLen);
		/* Insert BSS Available Access Capacity IE. */
		RRM_InsertBssAvailableACIE(pAd, pOutBuffer + FrameLen, &FrameLen);
#endif /* !APPLE_11K_IOT */
	}
#endif /* DOT11K_RRM_SUPPORT */
#ifdef OCE_SUPPORT
	if (IS_OCE_ENABLE(wdev)) { /* some OCE STA may only have files ie(without oce CAP.)*/
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += oce_build_ies(pAd, &ie_info, (MAC_ADDR_EQUAL(ProbeReqParam->Addr1, BROADCAST_ADDR) && ProbeReqParam->IsOceCapability));
	}
#endif /* OCE_SUPPORT */

	FrameLen += build_country_power_ie(pAd, wdev, (UCHAR *)(pOutBuffer + FrameLen));
	/* add Channel switch announcement IE */
	FrameLen += build_ch_switch_announcement_ie(pAd, wdev, (UCHAR *)(pOutBuffer + FrameLen));
#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(PhyMode) &&
		(wdev->DesiredHtPhyInfo.bHtEnable)) {
		if (pAd->bBroadComHT == TRUE) {
			ie_info.is_draft_n_type = TRUE;
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
			FrameLen += build_ht_ies(pAd, &ie_info);
		}
#ifdef DOT11_VHT_AC
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
		ucETxBfCap = wlan_config_get_etxbf(wdev);

		if (HcIsBfCapSupport(wdev) == FALSE)
			wlan_config_set_etxbf(wdev, SUBF_OFF);

#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_vht_ies(pAd, &ie_info);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
		wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#endif /* DOT11_VHT_AC */
	}

#endif /* DOT11_N_SUPPORT */
	ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
	FrameLen += build_wsc_ie(pAd, &ie_info);
#ifdef DOT11R_FT_SUPPORT

	/*
	   The Mobility Domain information element (MDIE) is present in Probe-
	   Request frame when dot11FastBssTransitionEnable is set to true.
	  */
	if (wdev->FtCfg.FtCapFlag.Dot11rFtEnable) {
		PFT_CFG pFtCfg = &wdev->FtCfg;
		FT_CAP_AND_POLICY FtCap;

		FtCap.field.FtOverDs = pFtCfg->FtCapFlag.FtOverDs;
		FtCap.field.RsrReqCap = pFtCfg->FtCapFlag.RsrReqCap;
		FT_InsertMdIE(pAd, pOutBuffer + FrameLen, &FrameLen,
					  pFtCfg->FtMdId, FtCap);
	}

#endif /* DOT11R_FT_SUPPORT */

#ifdef CONFIG_MAP_SUPPORT
#if defined(WAPP_SUPPORT)
	if (IS_MAP_ENABLE(pAd) && wdev->MAPCfg.vendor_ie_len) {
		ULONG MAPIeTmpLen = 0;

		MakeOutgoingFrame(pOutBuffer + FrameLen, &MAPIeTmpLen,
				wdev->MAPCfg.vendor_ie_len, wdev->MAPCfg.vendor_ie_buf,
				END_OF_ARGS);
		FrameLen += MAPIeTmpLen;
	}
#endif /*WAPP_SUPPORT*/
#endif /* CONFIG_MAP_SUPPORT */

	/*
		add Ralink-specific IE here - Byte0.b0=1 for aggregation,
		Byte0.b1=1 for piggy-back, Byte0.b3=1 for rssi-feedback
	*/
	FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen), VIE_PROBE_RESP
	);
#ifdef MBO_SUPPORT
	if (IS_MBO_ENABLE(wdev)
#ifdef OCE_SUPPORT
		|| IS_OCE_ENABLE(wdev)
#endif /* OCE_SUPPORT */
		)
		MakeMboOceIE(pAd, wdev, NULL, pOutBuffer+FrameLen, &FrameLen, MBO_FRAME_TYPE_PROBE_RSP);
#endif /* MBO_SUPPORT */

	{
		/* Question to Rorscha: bit4 in old chip is used? but currently is using for 2.4G 256QAM */
#ifdef RSSI_FEEDBACK
		UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
		ULONG TmpLen;

		if (ProbeReqParam->bRequestRssi == TRUE) {
			MAC_TABLE_ENTRY *pEntry = NULL;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("SYNC - Send PROBE_RSP to %02x:%02x:%02x:%02x:%02x:%02x...\n",
					 PRINT_MAC(ProbeReqParam->Addr2)));
			RalinkSpecificIe[5] |= 0x8;
			pEntry = MacTableLookup(pAd, ProbeReqParam->Addr2);

			if (pEntry != NULL) {
				RalinkSpecificIe[6] = (UCHAR)pEntry->RssiSample.AvgRssi[0];
				RalinkSpecificIe[7] = (UCHAR)pEntry->RssiSample.AvgRssi[1];
				RalinkSpecificIe[8] = (UCHAR)pEntry->RssiSample.AvgRssi[2];
			}
		}

		MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
						  9, RalinkSpecificIe,
						  END_OF_ARGS);
		FrameLen += TmpLen;
#endif /* RSSI_FEEDBACK */
	}

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	{
		MAC_TABLE_ENTRY *pEntry = NULL;

		pEntry = MacTableLookup(pAd, ProbeReqParam->Addr2);
		if (pEntry != NULL) {
			ap_vendor_ie = &pAd->ApCfg.MBSSID[pEntry->apidx].ap_vendor_ie;
			RTMP_SPIN_LOCK(&ap_vendor_ie->vendor_ie_lock);
			if (ap_vendor_ie->pointer != NULL) {
				ULONG TmpLen;

				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("SYNC - Send Probe response to %02x:%02x:%02x:%02x:%02x:%02x...and add vendor ie\n",
					PRINT_MAC(ProbeReqParam->Addr2)));
				MakeOutgoingFrame(pOutBuffer + FrameLen,
						&TmpLen,
						ap_vendor_ie->length,
						ap_vendor_ie->pointer,
						END_OF_ARGS);
				FrameLen += TmpLen;
			}
			RTMP_SPIN_UNLOCK(&ap_vendor_ie->vendor_ie_lock);
		}
	}
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

	/* 802.11n 11.1.3.2.2 active scanning. sending probe response with MCS rate is */
	/* Configure to better support Multi-Sta */
	{
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
		UINT8 idx = 0;

		for (idx = 0; idx < cap->ProbeRspTimes; idx++)
			MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	}
	MlmeFreeMemory(pOutBuffer);

	return TRUE;
}

struct sync_fsm_ops ap_fsm_ops = {
	/* when Peer ProbeReq come in */
	.tx_probe_response_allowed = ap_probe_response_allowed,
	.tx_probe_response_xmit = ap_probe_response_xmit,

	.rx_peer_response_allowed = ap_rx_peer_response_allowed,
	.rx_peer_response_updated = ap_rx_peer_response_updated,

	.join_peer_response_matched = ap_join_peer_response_matched,
	.join_peer_response_updated = ap_join_peer_response_updated,
};
#endif /* CONFIG_AP_SUPPORT */
