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
	sta_mgmt_assoc.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/
#include "rt_config.h"


struct _assoc_api_ops sta_assoc_api;

static VOID set_mlme_rsn_ie(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, PMAC_TABLE_ENTRY pEntry)
{
	ULONG Idx = 0;
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	/* Set New WPA information */
	Idx = BssTableSearch(ScanTab, pEntry->Addr, wdev->channel);
	if (Idx == BSS_NOT_FOUND) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ASSOC - Can't find BSS after receiving Assoc response\n"));
	} else {
		/* Init variable */
		pEntry->RSNIE_Len = 0;
		NdisZeroMemory(pEntry->RSN_IE, MAX_LEN_OF_RSNIE);

		/* Store appropriate RSN_IE for WPA SM negotiation later */
		if (IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap)
			&& (Idx < MAX_LEN_OF_BSS_TABLE)
			&& (ScanTab->BssEntry[Idx].VarIELen != 0)) {
			PUCHAR pVIE;
			USHORT len;
			PEID_STRUCT pEid;
			USHORT tmp_len = 0;

			pVIE = ScanTab->BssEntry[Idx].VarIEs;
			len = ScanTab->BssEntry[Idx].VarIELen;

			while (len > 0) {
				pEid = (PEID_STRUCT) pVIE;
				/* For WPA/WPAPSK */
				if ((pEid->Eid == IE_WPA)
					&& (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))
					&& (IS_AKM_WPA1(wdev->SecConfig.AKMMap)
					|| IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap))) {
					if ((tmp_len + pEid->Len + 2) > MAX_LEN_OF_RSNIE)
						break;
					NdisMoveMemory(&pEntry->RSN_IE[tmp_len], pVIE, (pEid->Len + 2));
					tmp_len += (pEid->Len + 2);
					pEntry->RSNIE_Len = tmp_len;
					MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("%s():=> Store RSN_IE for WPA SM negotiation\n", __func__));
				}
				/* For WPA2/WPA2PSK/WPA3/WPA3PSK */
				else if ((pEid->Eid == IE_RSN)
					 && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
					 && (IS_AKM_WPA2(wdev->SecConfig.AKMMap)
						 || IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap)
						 || IS_AKM_WPA3_192BIT(wdev->SecConfig.AKMMap)
						 || IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap)
						 || IS_AKM_OWE(wdev->SecConfig.AKMMap))) {
					if ((tmp_len + pEid->Len + 2) > MAX_LEN_OF_RSNIE)
						break;
					NdisMoveMemory(&pEntry->RSN_IE[tmp_len], pVIE, (pEid->Len + 2));
					tmp_len += (pEid->Len + 2);
					pEntry->RSNIE_Len = tmp_len;
					MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("%s():=> Store RSN_IE for WPA2 SM negotiation\n", __func__));
				}


				pVIE += (pEid->Len + 2);
				len -= (pEid->Len + 2);
			}

		}

		if (pEntry->RSNIE_Len == 0) {
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():=> no RSN_IE\n", __func__));
		} else {
			hex_dump("RSN_IE", pEntry->RSN_IE, pEntry->RSNIE_Len);
		}
	}

}

/*
	==========================================================================
	Description:
		Association timeout procedure. After association timeout, this function
		will be called and it will put a message into the MLME queue
	Parameters:
		Standard timer parameters

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_assoc_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PTIMER_FUNC_CONTEXT pContext = (PTIMER_FUNC_CONTEXT)FunctionContext;
	RTMP_ADAPTER *pAd = pContext->pAd;

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ASSOC - enqueue ASSOC_FSM_ASSOC_TIMEOUT\n"));

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

	MlmeEnqueueWithWdev(pAd, ASSOC_FSM, ASSOC_FSM_ASSOC_TIMEOUT, 0, NULL, 0, pContext->wdev, NULL);
	RTMP_MLME_HANDLER(pAd);
}
DECLARE_TIMER_FUNCTION(sta_assoc_timeout);
BUILD_TIMER_FUNCTION(sta_assoc_timeout);


/*
	==========================================================================
	Description:
		Reassociation timeout procedure. After reassociation timeout, this
		function will be called and put a message into the MLME queue
	Parameters:
		Standard timer parameters

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_reassoc_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PTIMER_FUNC_CONTEXT pContext = (PTIMER_FUNC_CONTEXT)FunctionContext;
	RTMP_ADAPTER *pAd = pContext->pAd;

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

	MlmeEnqueueWithWdev(pAd, ASSOC_FSM, ASSOC_FSM_REASSOC_TIMEOUT, 0, NULL, 0, pContext->wdev, NULL);
	RTMP_MLME_HANDLER(pAd);
}
DECLARE_TIMER_FUNCTION(sta_reassoc_timeout);
BUILD_TIMER_FUNCTION(sta_reassoc_timeout);

/*
	==========================================================================
	Description:
		Disassociation timeout procedure. After disassociation timeout, this
		function will be called and put a message into the MLME queue
	Parameters:
		Standard timer parameters

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_disassoc_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PTIMER_FUNC_CONTEXT pContext = (PTIMER_FUNC_CONTEXT)FunctionContext;
	RTMP_ADAPTER *pAd = pContext->pAd;

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

	MlmeEnqueueWithWdev(pAd, ASSOC_FSM, ASSOC_FSM_DISASSOC_TIMEOUT, 0, NULL, 0, pContext->wdev, NULL);
	RTMP_MLME_HANDLER(pAd);
}
DECLARE_TIMER_FUNCTION(sta_disassoc_timeout);
BUILD_TIMER_FUNCTION(sta_disassoc_timeout);

/* Link down report*/
static VOID sta_link_down_exec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct wifi_dev *wdev = (struct wifi_dev *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	if (pAd != NULL) {

		if ((pStaCfg->wdev.PortSecured == WPA_802_1X_PORT_NOT_SECURED) &&
			(INFRA_ON(pStaCfg))) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("LinkDownExec(): disassociate with current AP...\n"));
			cntl_disconnect_request(wdev, CNTL_DISASSOC, pStaCfg->Bssid, REASON_DISASSOC_STA_LEAVING, NULL);
			RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
			pAd->ExtraInfo = GENERAL_LINK_DOWN;
		}
	}
}
DECLARE_TIMER_FUNCTION(sta_link_down_exec);
BUILD_TIMER_FUNCTION(sta_link_down_exec);



static VOID ApCliAssocPostProc(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN PUCHAR pAddr2,
	IN USHORT CapabilityInfo,
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen,
	IN ADD_HT_INFO_IE *pAddHtInfo,
	IN MAC_TABLE_ENTRY *pEntry)
{
	PSTA_ADMIN_CONFIG pApCliEntry = GetStaCfgByWdev(pAd, pEntry->wdev);
	UINT_8 OmacIdx;

	ASSERT(pApCliEntry);
	pApCliEntry->MlmeAux.BssType = BSS_INFRA;
	pApCliEntry->MlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
	NdisMoveMemory(&pApCliEntry->MlmeAux.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));
	/* filter out un-supported rates */
	pApCliEntry->MlmeAux.SupRateLen = SupRateLen;
	NdisMoveMemory(pApCliEntry->MlmeAux.SupRate, SupRate, SupRateLen);
	RTMPCheckRates(pAd, pApCliEntry->MlmeAux.SupRate, &(pApCliEntry->MlmeAux.SupRateLen), pApCliEntry->wdev.PhyMode);
	/* filter out un-supported rates */
	pApCliEntry->MlmeAux.ExtRateLen = ExtRateLen;
	NdisMoveMemory(pApCliEntry->MlmeAux.ExtRate, ExtRate, ExtRateLen);
	RTMPCheckRates(pAd, pApCliEntry->MlmeAux.ExtRate, &(pApCliEntry->MlmeAux.ExtRateLen), pApCliEntry->wdev.PhyMode);
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, (HtCapabilityLen ? "%s===> 11n HT STA\n" : "%s===> legacy STA\n", __func__));
#ifdef DOT11_N_SUPPORT

	if (HtCapabilityLen > 0 && WMODE_CAP_N(pApCliEntry->wdev.PhyMode))
		RTMPCheckHt(pAd, pEntry->wcid, pHtCapability, pAddHtInfo);

#endif /* DOT11_N_SUPPORT */

	OmacIdx  = HcGetOmacIdx(pAd, &pApCliEntry->wdev);
	chip_arch_set_aid(pAd, pEntry->Aid, OmacIdx);

}

/*
	==========================================================================
	Description:
		procedures on IEEE 802.11/1999 p.376
	Parametrs:

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_assoc_post_proc(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr2,
	IN USHORT CapabilityInfo,
	IN USHORT Aid,
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	IN IE_LISTS *ie_list,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen,
	IN ADD_HT_INFO_IE *pAddHtInfo,
	IN MAC_TABLE_ENTRY *pEntry)
{
	/* AP might use this additional ht info IE */
	ULONG Idx;
	struct wifi_dev *wdev = NULL;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	BSS_TABLE *ScanTab = NULL;

	if (!pEntry)
		return;

	wdev = pEntry->wdev;
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	pStaCfg->MlmeAux.BssType = BSS_INFRA;
	COPY_MAC_ADDR(pStaCfg->MlmeAux.Bssid, pAddr2);
	pStaCfg->MlmeAux.Aid = Aid;
	pStaCfg->MlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
	chip_arch_set_aid(pAd, Aid, 0);

#ifdef DOT11_N_SUPPORT

	/* Some HT AP might lost WMM IE. We add WMM ourselves. beacuase HT requires QoS on. */
	if ((HtCapabilityLen > 0) && (pEdcaParm->bValid == FALSE)) {
		pEdcaParm->bValid = TRUE;
		pEdcaParm->Aifsn[0] = 3;
		pEdcaParm->Aifsn[1] = 7;
		pEdcaParm->Aifsn[2] = 2;
		pEdcaParm->Aifsn[3] = 2;
		pEdcaParm->Cwmin[0] = 4;
		pEdcaParm->Cwmin[1] = 4;
		pEdcaParm->Cwmin[2] = 3;
		pEdcaParm->Cwmin[3] = 2;
		pEdcaParm->Cwmax[0] = 10;
		pEdcaParm->Cwmax[1] = 10;
		pEdcaParm->Cwmax[2] = 4;
		pEdcaParm->Cwmax[3] = 3;
		pEdcaParm->Txop[0] = 0;
		pEdcaParm->Txop[1] = 0;
		pEdcaParm->Txop[2] = 94;
		pEdcaParm->Txop[3] = 47;
	}

#endif /* DOT11_N_SUPPORT */

	if (pEdcaParm->bValid == TRUE)
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

	NdisMoveMemory(&pStaCfg->MlmeAux.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));
	/* filter out un-supported rates */
	pStaCfg->MlmeAux.SupRateLen = SupRateLen;
	NdisMoveMemory(pStaCfg->MlmeAux.SupRate, SupRate, SupRateLen);
	RTMPCheckRates(pAd, pStaCfg->MlmeAux.SupRate, &pStaCfg->MlmeAux.SupRateLen, wdev->PhyMode);
	/* filter out un-supported rates */
	pStaCfg->MlmeAux.ExtRateLen = ExtRateLen;
	NdisMoveMemory(pStaCfg->MlmeAux.ExtRate, ExtRate, ExtRateLen);
	RTMPCheckRates(pAd, pStaCfg->MlmeAux.ExtRate, &pStaCfg->MlmeAux.ExtRateLen, wdev->PhyMode);
#ifdef DOT11_N_SUPPORT

	if (HtCapabilityLen > 0) {
		RTMPCheckHt(pAd, pEntry->wcid, pHtCapability, pAddHtInfo);
	}

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s():=>AP.AMsduSize = %d. ClientStatusFlags = 0x%lx\n",
			  __func__, pEntry->AMsduSize, pEntry->ClientStatusFlags));
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s():=>(Mmps=%d, AmsduSize=%d, )\n",
			  __func__, pEntry->MmpsMode, pEntry->AMsduSize));
#ifdef DOT11_VHT_AC

	if (ie_list->vht_cap_len > 0 && ie_list->vht_op_len > 0)
		RTMPCheckVht(pAd, pEntry->wcid, &ie_list->vht_cap, &ie_list->vht_op);

#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	/* Set New WPA information */
	Idx = BssTableSearch(ScanTab, pAddr2, pStaCfg->MlmeAux.Channel);

	if (Idx == BSS_NOT_FOUND)
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ASSOC - Can't find BSS after receiving Assoc response\n"));
	else if (ie_list->RSNIE_Len >= MIN_LEN_OF_RSNIE) {
		PUCHAR rsnie_from_resp = ie_list->RSN_IE;
		USHORT len = ie_list->RSNIE_Len;

		/*
		 * for OWE, AP also takes RSNE in assoc resp for take out PMKID and ECDH IE to show AP's public key,
		 * we need to check RSN_IE ie list here.
		 */
		/* Init variable */
		pEntry->RSNIE_Len = 0;
		NdisZeroMemory(pEntry->RSN_IE, MAX_LEN_OF_RSNIE);

		/* For WPA2/WPA2PSK/WPA3/WPA3PSK */
		if (IS_AKM_WPA2(wdev->SecConfig.AKMMap) ||
		    IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap) ||
		    IS_AKM_WPA3_192BIT(wdev->SecConfig.AKMMap) ||
		    IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap) ||
		    IS_AKM_OWE(wdev->SecConfig.AKMMap)) {
			NdisMoveMemory(pEntry->RSN_IE, rsnie_from_resp, len);
			pEntry->RSNIE_Len = len;
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("%s():=> Store Assoc Resp RSN_IE for WPA2 SM negotiation\n", __func__));
		}

		if (pEntry->RSNIE_Len == 0)
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():=> no RSN_IE\n", __func__));
		else
			hex_dump("RSN_IE", pEntry->RSN_IE, pEntry->RSNIE_Len);

#ifdef CONFIG_OWE_SUPPORT
		if (IS_AKM_OWE(wdev->SecConfig.AKMMap)) {
			BOOLEAN need_process_ecdh_ie = FALSE;
			UINT8 *pmkid = NULL;
			UINT8 pmkid_count = 0;

			pmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE,
								ie_list->RSNIE_Len,
								PMKID_LIST,
								&pmkid_count);
			if (pmkid != NULL) {
				INT idx;
				BOOLEAN FoundPMK = FALSE;

				/*	Search chched PMKID, append it if existed */
				for (idx = 0; idx < PMKID_NO; idx++) {
					if (NdisEqualMemory(pAddr2, &pStaCfg->SavedPMK[idx].BSSID, 6)) {
						FoundPMK = TRUE;
						break;
					}
				}

				if (FoundPMK == FALSE) {
					need_process_ecdh_ie = TRUE;
					MTWF_LOG(DBG_CAT_AP,
						 DBG_SUBCAT_ALL,
						 DBG_LVL_ERROR,
						 ("%s: cannot find match PMKID\n", __func__));
				} else if ((pEntry->SecConfig.pmkid) &&
					  ((RTMPEqualMemory(pmkid, pEntry->SecConfig.pmkid, LEN_PMKID)) != 0)) {
					/*
					 * if STA would like to use PMK CACHE,
					 * it stored the PMKID in assoc req stage already.
					 * no need to restore it again here.
					 */
					MTWF_LOG(DBG_CAT_AP,
						 DBG_SUBCAT_ALL,
						 DBG_LVL_ERROR,
						 ("%s: PMKID doesn't match STA sent\n", __func__));
					need_process_ecdh_ie = TRUE;
				}
			} else
				need_process_ecdh_ie = TRUE;

			if (need_process_ecdh_ie == TRUE) {
				MTWF_LOG(DBG_CAT_AP,
					 DBG_SUBCAT_ALL,
					 DBG_LVL_TRACE,
					("%s: do normal ECDH procedure\n", __func__));
				process_ecdh_element(pAd,
						pEntry,
						(EXT_ECDH_PARAMETER_IE *)&ie_list->ecdh_ie,
						ie_list->ecdh_ie.length,
						SUBTYPE_ASSOC_RSP);
			}
		}
#endif /*CONFIG_OWE_SUPPORT*/
	} else {
		/* Init variable */
		pEntry->RSNIE_Len = 0;
		NdisZeroMemory(pEntry->RSN_IE, MAX_LEN_OF_RSNIE);

		/* Store appropriate RSN_IE for WPA SM negotiation later */
		if (IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap)
			&& (Idx < MAX_LEN_OF_BSS_TABLE)
			&& (ScanTab->BssEntry[Idx].VarIELen != 0)) {
			PUCHAR pVIE;
			USHORT len;
			PEID_STRUCT pEid;

			pVIE = ScanTab->BssEntry[Idx].VarIEs;
			len = ScanTab->BssEntry[Idx].VarIELen;

			while (len > 0) {
				pEid = (PEID_STRUCT) pVIE;

				/* For WPA/WPAPSK */
				if ((pEid->Eid == IE_WPA)
					&& (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))
					&& (IS_AKM_WPA1(wdev->SecConfig.AKMMap)
						|| IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap))) {
					NdisMoveMemory(pEntry->RSN_IE, pVIE, (pEid->Len + 2));
					pEntry->RSNIE_Len = (pEid->Len + 2);
					MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("%s():=> Store RSN_IE for WPA SM negotiation\n", __func__));
				}
				/* For WPA2/WPA2PSK */
				else if ((pEid->Eid == IE_RSN)
						 && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
						 && (IS_AKM_WPA2(wdev->SecConfig.AKMMap) ||
						     IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap) ||
						     IS_AKM_WPA3_192BIT(wdev->SecConfig.AKMMap) ||
						     IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap) ||
						     IS_AKM_OWE(wdev->SecConfig.AKMMap))) {
					NdisMoveMemory(pEntry->RSN_IE, pVIE, (pEid->Len + 2));
					pEntry->RSNIE_Len = (pEid->Len + 2);
					MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("%s():=> Store RSN_IE for WPA2 SM negotiation\n", __func__));
				}

				pVIE += (pEid->Len + 2);
				len -= (pEid->Len + 2);
			}

#ifdef DOT11R_FT_SUPPORT

			if (pStaCfg->Dot11RCommInfo.bFtSupport &&
				pStaCfg->Dot11RCommInfo.bInMobilityDomain &&
				(pStaCfg->MlmeAux.FtIeInfo.GtkLen != 0)) {
				/* extract GTK related information */
				FT_ExtractGTKSubIe(pAd,
								   &pAd->MacTab.Content[MCAST_WCID],
								   &pStaCfg->MlmeAux.FtIeInfo);
			}

#endif /* DOT11R_FT_SUPPORT */
		}

		if (pEntry->RSNIE_Len == 0)
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():=> no RSN_IE\n", __func__));
		else
			hex_dump("RSN_IE", pEntry->RSN_IE, pEntry->RSNIE_Len);
	}
}

static BOOLEAN sta_block_checker(struct wifi_dev *wdev, struct inter_machine_info *priv_data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)wdev->sys_handle;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	USHORT Status = MLME_FAIL_NO_RESOURCE;

	/* Block all authentication request durning WPA block period */
	if (pStaCfg->bBlockAssoc == TRUE) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("ASSOC - Block ReAssoc request durning WPA block period!\n"));
		Status = MLME_STATE_MACHINE_REJECT;
		cntl_auth_assoc_conf(wdev, CNTL_MLME_REASSOC_CONF, Status, priv_data); /* CNTL_MLME_ASSOC_CONF */
		return TRUE;
	}

	return FALSE;
}

/*
	==========================================================================
	Description:
		mlme assoc req handling procedure
	Parameters:
		Adapter - Adapter pointer
		Elem - MLME Queue Element
	Pre:
		the station has been authenticated and the following information is stored in the config
			-# SSID
			-# supported rates and their length
			-# listen interval (Adapter->StaCfg[0].default_listen_count)
			-# Transmit power  (Adapter->StaCfg[0].tx_power)
	Post  :
		-# An association request frame is generated and sent to the air
		-# Association timer starts
		-# Association state -> ASSOC_WAIT_RSP

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_mlme_assoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR ApAddr[6];
	HEADER_802_11 AssocHdr;
	USHORT ListenIntv;
	ULONG Timeout;
	USHORT CapabilityInfo;
	BOOLEAN TimerCancelled;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	ULONG tmp;
	USHORT VarIesOffset = 0;
	USHORT Status;
#ifdef APCLI_CONNECTION_TRIAL
	ULONG temp;
#endif
	struct wifi_dev *wdev = Elem->wdev;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	MAC_TABLE_ENTRY *pAPEntry = GetAssociatedAPByWdev(pAd, Elem->wdev);
	RALINK_TIMER_STRUCT *assoc_timer = NULL;
	UCHAR SsidIe    = IE_SSID;
	UCHAR SupRateIe = IE_SUPP_RATES;
	UCHAR ExtRateIe = IE_EXT_SUPP_RATES;
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;
#ifdef MAC_REPEATER_SUPPORT
	USHORT ifIndex = wdev->func_idx;
	REPEATER_CLIENT_ENTRY *pReptEntry = Elem->priv_data.rept_cli_entry;
#endif /* MAC_REPEATER_SUPPORT */
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	ASSERT(pStaCfg);
	ASSERT(pAPEntry);
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != NON_REPT_ENTRY) {
		assoc_timer = &pReptEntry->ApCliAssocTimer;
		ASSERT(pReptEntry->wdev == wdev);
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		assoc_timer = &pStaCfg->MlmeAux.AssocTimer;
	}

	assoc_fsm_state_transition(wdev, CliIdx, ASSOC_IDLE);

	if (sta_block_checker(wdev, &Elem->priv_data) == TRUE)
		return;

	/* check sanity first */
	if (MlmeAssocReqSanity(pAd, Elem->Msg, Elem->MsgLen, ApAddr, &CapabilityInfo, &Timeout, &ListenIntv) == FALSE) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("ASSOC - MlmeAssocReqAction() sanity check failed. BUG!!!!!!\n"));
		Status = MLME_INVALID_FORMAT;
		cntl_auth_assoc_conf(wdev, CNTL_MLME_ASSOC_CONF, Status, &Elem->priv_data);
		return;
	}

	/* insert MacRepeater Mac Entry here */
#ifdef MAC_REPEATER_SUPPORT

	if ((pAd->ApCfg.bMACRepeaterEn) &&
		(IS_HIF_TYPE(pAd, HIF_MT)) &&
		(CliIdx != NON_REPT_ENTRY)) {
		pAPEntry = MacTableInsertEntry(
					   pAd,
					   (PUCHAR)(pStaCfg->MlmeAux.Bssid),
					   pReptEntry->wdev,
					   ENTRY_REPEATER,
					   OPMODE_AP,
					   TRUE);


		if (pAPEntry)
			pReptEntry->MacTabWCID = pAPEntry->wcid;
		else {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("repeater pEntry insert fail"));
			return;
		}
	}

#endif /* MAC_REPEATER_SUPPORT */
	{
		struct _build_ie_info ie_info = {0};

		ie_info.frame_subtype = SUBTYPE_ASSOC_REQ;
		ie_info.channel = pStaCfg->MlmeAux.Channel;
		ie_info.phy_mode = wdev->PhyMode;
		ie_info.wdev = wdev;
		RTMPCancelTimer(assoc_timer, &TimerCancelled);
		COPY_MAC_ADDR(pStaCfg->MlmeAux.Bssid, ApAddr);
		/* Get an unused nonpaged memory */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("ASSOC - MlmeAssocReqAction() allocate memory failed\n"));
			Status = MLME_FAIL_NO_RESOURCE;
			cntl_auth_assoc_conf(wdev, CNTL_MLME_ASSOC_CONF, Status, &Elem->priv_data);
			/*
						 ApCliCtrlMsg.Status = MLME_FAIL_NO_RESOURCE;
						 MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
								sizeof(CTRL_JOIN_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

			 */
			return;
		}

		/* Add by James 03/06/27 */
		pStaCfg->AssocInfo.Length =
			sizeof(NDIS_802_11_ASSOCIATION_INFORMATION);
		/* Association don't need to report MAC address */
		pStaCfg->AssocInfo.AvailableRequestFixedIEs =
			NDIS_802_11_AI_REQFI_CAPABILITIES | NDIS_802_11_AI_REQFI_LISTENINTERVAL;
		pStaCfg->AssocInfo.RequestFixedIEs.Capabilities = CapabilityInfo;
		pStaCfg->AssocInfo.RequestFixedIEs.ListenInterval = ListenIntv;
		/* Only reassociate need this */
		/*COPY_MAC_ADDR(pStaCfg->AssocInfo.RequestFixedIEs.CurrentAPAddress, ApAddr); */
		pStaCfg->AssocInfo.OffsetRequestIEs = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION);
		NdisZeroMemory(pStaCfg->ReqVarIEs, MAX_VIE_LEN);
		/* First add SSID */
		VarIesOffset = 0;
		NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &SsidIe, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &pStaCfg->MlmeAux.SsidLen, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
		VarIesOffset += pStaCfg->MlmeAux.SsidLen;
		/* Second add Supported rates */
		NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &SupRateIe, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &pStaCfg->MlmeAux.SupRateLen, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, pStaCfg->MlmeAux.SupRate, pStaCfg->MlmeAux.SupRateLen);
		VarIesOffset += pStaCfg->MlmeAux.SupRateLen;
		/* End Add by James */

		/*
		   CapabilityInfo already sync value with AP in PeerBeaconAtJoinAction.
		   But we need to clean Spectrum Management bit here, if we disable bIEEE80211H in infra sta
		 */
		if (!((wdev->channel > 14) &&
			  (pAd->CommonCfg.bIEEE80211H == TRUE)))
			CapabilityInfo &= (~0x0100);

		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ASSOC - Send ASSOC request...\n"));
		MgtMacHeaderInitExt(pAd, &AssocHdr, SUBTYPE_ASSOC_REQ, 0, ApAddr,
							pStaCfg->wdev.if_addr, ApAddr);
#ifdef MAC_REPEATER_SUPPORT

		if (CliIdx != 0xFF)
			COPY_MAC_ADDR(AssocHdr.Addr2, pReptEntry->CurrentAddress);

#endif /* MAC_REPEATER_SUPPORT */
		/* Build basic frame first */
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(HEADER_802_11), &AssocHdr,
						  2, &CapabilityInfo,
						  2, &ListenIntv,
						  1, &SsidIe,
						  1, &pStaCfg->MlmeAux.SsidLen,
						  pStaCfg->MlmeAux.SsidLen, pStaCfg->MlmeAux.Ssid,
						  1, &SupRateIe,
						  1, &pStaCfg->MlmeAux.SupRateLen,
						  pStaCfg->MlmeAux.SupRateLen, pStaCfg->MlmeAux.SupRate,
						  END_OF_ARGS);

		if (pStaCfg->MlmeAux.ExtRateLen != 0) {
			MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
							  1, &ExtRateIe,
							  1, &pStaCfg->MlmeAux.ExtRateLen,
							  pStaCfg->MlmeAux.ExtRateLen,
							  pStaCfg->MlmeAux.ExtRate, END_OF_ARGS);
			FrameLen += tmp;
		}

#ifdef DOT11R_FT_SUPPORT

		/* Add MDIE if we are connection to DOT11R AP */
		if (pStaCfg->Dot11RCommInfo.bFtSupport &&
			pStaCfg->MlmeAux.MdIeInfo.Len) {
			/* MDIE */
			FT_InsertMdIE(pAd, pOutBuffer + FrameLen, &FrameLen,
						  pStaCfg->MlmeAux.MdIeInfo.MdId,
						  pStaCfg->MlmeAux.MdIeInfo.FtCapPlc);
		}

#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11_N_SUPPORT

		/*
			WFA recommend to restrict the encryption type in 11n-HT mode.
			So, the WEP and TKIP are not allowed in HT rate.
		*/
		if (pAd->CommonCfg.HT_DisallowTKIP &&
			IS_INVALID_HT_SECURITY(pAPEntry->SecConfig.PairwiseCipher)) {
			/* Force to None-HT mode due to WiFi 11n policy */
			pStaCfg->MlmeAux.HtCapabilityLen = 0;
#ifdef DOT11_VHT_AC
			pStaCfg->MlmeAux.vht_cap_len = 0;
#endif /* DOT11_VHT_AC */
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s : Force STA as Non-HT mode\n", __func__));
		}

		/* HT */
		if ((pStaCfg->MlmeAux.HtCapabilityLen > 0)
			&& WMODE_CAP_N(wdev->PhyMode)) {
#ifdef MAC_REPEATER_SUPPORT
			if (pAd->ApCfg.bMACRepeaterEn && (pReptEntry != NULL))
				ie_info.ReptMacTabWCID = pReptEntry->MacTabWCID;
#endif /* MAC_REPEATER_SUPPORT */

			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
			FrameLen += build_ht_ies(pAd, &ie_info);

#ifdef DOT11_VHT_AC
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			ucETxBfCap = wlan_config_get_etxbf(wdev);

			if (HcIsBfCapSupport(wdev) == FALSE)
				wlan_config_set_etxbf(wdev, SUBF_OFF);

#ifdef MAC_REPEATER_SUPPORT
			else if (pAd->ApCfg.bMACRepeaterEn) {
				struct _RTMP_CHIP_CAP *cap;
				cap = hc_get_chip_cap(pAd->hdev_ctrl);

				/* BFee function is limited if there is AID HW limitation*/
				if (cap->FlgHwTxBfCap & TXBF_AID_HW_LIMIT) {
					wlan_config_set_etxbf(wdev, SUBF_BFER);

					/* Just first cloned STA has full BF capability */
					if ((pReptEntry != NULL) && (pAd->fgClonedStaWithBfeeSelected == FALSE)) {
						MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)NULL;

						MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s : OriginalAddress[0~5] = %x:%x:%x:%x:%x:%x\n",
								 __func__, PRINT_MAC(pReptEntry->OriginalAddress)));
						pEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];

						if ((pEntry) && (HcIsBfCapSupport(pEntry->wdev) == TRUE)) {
							wlan_config_set_etxbf(wdev, SUBF_ALL);
							pAd->fgClonedStaWithBfeeSelected = TRUE;
							pAd->ReptClonedStaEntry_CliIdx   = CliIdx;
						}
					}
				}
			}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#ifdef APCLI_CONNECTION_TRIAL
			if (CliIdx != 0xFF) {
				/*Disable the Mu beamforming in repeater connection if Trial connection is enabled*/
				temp = pAd->CommonCfg.MUTxRxEnable;
				pAd->CommonCfg.MUTxRxEnable = MUBF_OFF;
			}
#endif /*APCLI_CONNECTION_TRIAL*/
			FrameLen += build_vht_ies(pAd, &ie_info);

#ifdef APCLI_CONNECTION_TRIAL
			if (CliIdx != 0xFF)
				pAd->CommonCfg.MUTxRxEnable = temp;
#endif /*APCLI_CONNECTION_TRIAL*/

#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#endif /* DOT11_VHT_AC */
		}

#endif /* DOT11_N_SUPPORT */

		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_extended_cap_ie(pAd, &ie_info);


		FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen), VIE_ASSOC_REQ
		);

		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_wmm_cap_ie(pAd, &ie_info);
		/*
			Let WPA(#221) Element ID on the end of this association frame.
			Otherwise some AP will fail on parsing Element ID and set status fail on Assoc Rsp.
			For example: Put Vendor Specific IE on the front of WPA IE.
			This happens on AP (Model No:Linksys WRK54G)
		*/
		printk("======================================> check WPA2PSK :%d\n", IS_AKM_PSK(pAPEntry->SecConfig.AKMMap));

#ifdef CONFIG_OWE_SUPPORT
	/* Allow OWE STA to connect to OPEN AP */
	if ((IS_AKM_OWE(wdev->SecConfig.AKMMap)) && (IS_AKM_OPEN_ONLY(pAPEntry->SecConfig.AKMMap))) {
		pStaCfg->AKMMap = pAPEntry->SecConfig.AKMMap;
		pStaCfg->PairwiseCipher = pAPEntry->SecConfig.PairwiseCipher;
	}
#endif

#ifdef APCLI_CFG80211_SUPPORT
		/*pStaCfg->ReqVarIELen = 0;*/
		/*NdisZeroMemory(pStaCfg->ReqVarIEs, MAX_VIE_LEN);*/
		/*shailesh: commenting out this as making variable part of ies in request variable ies*/
		if ((pStaCfg->wpa_supplicant_info.WpaSupplicantUP & 0x7F) ==  WPA_SUPPLICANT_ENABLE) {
			ULONG TmpWpaAssocIeLen = 0;

			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s:: APCLI WPA_ASSOC_IE FROM SUPPLICANT\n", __func__));
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpWpaAssocIeLen,
							pStaCfg->wpa_supplicant_info.WpaAssocIeLen, pStaCfg->wpa_supplicant_info.pWpaAssocIe,
							END_OF_ARGS);
			FrameLen += TmpWpaAssocIeLen;
			/*VarIesOffset = 0;*/
			NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset,
							pStaCfg->wpa_supplicant_info.pWpaAssocIe, pStaCfg->wpa_supplicant_info.WpaAssocIeLen);
			VarIesOffset += pStaCfg->wpa_supplicant_info.WpaAssocIeLen;
			/* Set Variable IEs Length */
			pStaCfg->ReqVarIELen = VarIesOffset;
		} else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */

		/* Append RSN_IE when WPAPSK OR WPA2PSK, */
		if ((IS_AKM_PSK(pAPEntry->SecConfig.AKMMap)
			 || IS_AKM_OPEN(pAPEntry->SecConfig.AKMMap)
			 || IS_AKM_SHARED(pAPEntry->SecConfig.AKMMap)
			 || IS_AKM_AUTOSWITCH(pAPEntry->SecConfig.AKMMap)
			)
#ifdef WSC_STA_SUPPORT
			&& ((wdev->WscControl.WscConfMode == WSC_DISABLE) ||
				((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
				 !(wdev->WscControl.bWscTrigger)))
#endif /* WSC_STA_SUPPORT */
		   ) {

/*Shifted to Sta_mlme_assoc_req_action because 1st Eapol is received before linkup_infra completes ,so we need*
	fill pEntry->seconfig before the 1st Eapol is received*/

	{
	struct _SECURITY_CONFIG *pProfile_SecConfig = &wdev->SecConfig;
	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)NULL;
	struct _SECURITY_CONFIG *pEntry_SecConfig = NULL;
	BOOLEAN do_wsc_now = FALSE;

#ifdef WSC_INCLUDED
			if ((pStaCfg->wdev.WscControl.WscConfMode != WSC_DISABLE)
				&& pStaCfg->wdev.WscControl.bWscTrigger)
				do_wsc_now = TRUE;

#endif /* WSC_INCLUDED */


#ifdef MAC_REPEATER_SUPPORT
			if ((pAd->ApCfg.bMACRepeaterEn) &&
				(IS_HIF_TYPE(pAd, HIF_MT)) &&
				(CliIdx != NON_REPT_ENTRY)) {
				pEntry = pAPEntry;
			} else
				pEntry = GetAssociatedAPByWdev(pAd, wdev);
#endif /* MAC_REPEATER_SUPPORT */

			if (pEntry) {

				pEntry_SecConfig = &pEntry->SecConfig;

				if (IS_CIPHER_WEP(pEntry_SecConfig->PairwiseCipher)) {
					os_move_mem(pEntry_SecConfig->WepKey, pProfile_SecConfig->WepKey,
							sizeof(SEC_KEY_INFO)*SEC_KEY_NUM);
					pProfile_SecConfig->GroupKeyId = pProfile_SecConfig->PairwiseKeyId;
					pEntry_SecConfig->PairwiseKeyId = pProfile_SecConfig->PairwiseKeyId;
				} else {
					/* Calculate PMK */
#ifdef WSC_INCLUDED
					if (!do_wsc_now)
#endif /* WSC_INCLUDED */
#ifndef CONFIG_STA_ADHOC_SUPPORT
#ifdef DOT11_SAE_SUPPORT
					if (!((IS_AKM_SAE_SHA256(pEntry_SecConfig->AKMMap) || IS_AKM_OWE(pEntry_SecConfig->AKMMap)
							|| IS_AKM_OPEN(pEntry_SecConfig->AKMMap))))
#endif
						NdisCopyMemory(pEntry_SecConfig->PMK, pProfile_SecConfig->PMK, LEN_PMK);
#else
					SetWPAPSKKey(pAd, pProfile_SecConfig->PSK, strlen(pProfile_SecConfig->PSK),
									pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen, pEntry_SecConfig->PMK);
#endif
#ifdef MAC_REPEATER_SUPPORT

					if ((pAd->ApCfg.bMACRepeaterEn) && (IS_HIF_TYPE(pAd, HIF_MT)) && (CliIdx != 0xFF)

					) {
						os_move_mem(pEntry_SecConfig->Handshake.AAddr, pEntry->Addr, MAC_ADDR_LEN);
						os_move_mem(pEntry_SecConfig->Handshake.SAddr,
								pReptEntry->CurrentAddress, MAC_ADDR_LEN);
					} else
#endif /* MAC_REPEATER_SUPPORT */
					{
						os_move_mem(pEntry_SecConfig->Handshake.AAddr, pEntry->Addr, MAC_ADDR_LEN);
						os_move_mem(pEntry_SecConfig->Handshake.SAddr, wdev->if_addr, MAC_ADDR_LEN);
					}

					os_zero_mem(pEntry_SecConfig->Handshake.ReplayCounter, LEN_KEY_DESC_REPLAY);
					pEntry->SecConfig.Handshake.WpaState = AS_INITPSK;
				}

				pEntry_SecConfig->GroupKeyId = pProfile_SecConfig->GroupKeyId;
				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("%s: (apcli%d) connect AKM(0x%x)=%s, PairwiseCipher(0x%x)=%s, GroupCipher(0x%x)=%s\n",
						  __func__, ifIndex,
						  pEntry_SecConfig->AKMMap, GetAuthModeStr(pEntry_SecConfig->AKMMap),
						  pEntry_SecConfig->PairwiseCipher, GetEncryModeStr(pEntry_SecConfig->PairwiseCipher),
						  pEntry_SecConfig->GroupCipher, GetEncryModeStr(pEntry_SecConfig->GroupCipher)));
				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("%s(): PairwiseKeyId=%d, GroupKeyId=%d\n",
						  __func__, pEntry_SecConfig->PairwiseKeyId, pEntry_SecConfig->GroupKeyId));
			}
}

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
			if (IS_AKM_WPA3PSK(pStaCfg->wdev.SecConfig.AKMMap) ||
				IS_AKM_OWE(pStaCfg->wdev.SecConfig.AKMMap)) {
				INT idx;
				MAC_TABLE_ENTRY *pentry = (MAC_TABLE_ENTRY *)NULL;
				PBSSID_INFO psaved_pmk = NULL;
				VOID *psaved_pmk_lock = NULL;
#ifdef MAC_REPEATER_SUPPORT
				if ((pAd->ApCfg.bMACRepeaterEn) &&
					(CliIdx != NON_REPT_ENTRY)) {
					pentry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
					psaved_pmk = &pReptEntry->SavedPMK[0];
					psaved_pmk_lock = (void *)&pReptEntry->SavedPMK_lock;
				} else
#endif /* MAC_REPEATER_SUPPORT */
				{
					pentry = &pAd->MacTab.Content[pStaCfg->MacTabWCID];
					psaved_pmk = &pStaCfg->SavedPMK[0];
					psaved_pmk_lock = (VOID *)&pStaCfg->SavedPMK_lock;
				}

				idx = sta_search_pmkid_cache(pAd, ApAddr, ifIndex, CliIdx);

				if (idx != INVALID_PMKID_IDX) {
					if (psaved_pmk_lock)
						NdisAcquireSpinLock(psaved_pmk_lock);

					/*Update the pentry->pmkcache from the Saved PMK cache */
					pentry->SecConfig.pmkid = psaved_pmk[idx].PMKID;
					pentry->SecConfig.pmk_cache = psaved_pmk[idx].PMK;

					if (psaved_pmk_lock)
						NdisReleaseSpinLock(psaved_pmk_lock);
				} else {
					printk("PMKID not found in cache: Normal Assoc\n");
					pentry->SecConfig.pmkid = NULL;
					pentry->SecConfig.pmk_cache = NULL;
				}
			}
#endif

			{
				ULONG TempLen = 0;
				CHAR rsne_idx = 0;
				{ /* Todo by Eddy: It's not good code */
					struct _SECURITY_CONFIG *pSecConfig = &pAPEntry->SecConfig;
					UINT32 AKMMap = pSecConfig->AKMMap;
					UINT32 PairwiseCipher = pSecConfig->PairwiseCipher;
					UINT32 GroupCipher = pSecConfig->GroupCipher;
#ifdef DOT11W_PMF_SUPPORT
					/* Need to fill the pSecConfig->PmfCfg to let WPAMakeRSNIE() generate correct RSNIE*/
					{
						RSN_CAPABILITIES RsnCap;

						NdisMoveMemory(&RsnCap, &pStaCfg->MlmeAux.RsnCap, sizeof(RSN_CAPABILITIES));
						RsnCap.word = cpu2le16(RsnCap.word);
						/* init to FALSE */
						pSecConfig->PmfCfg.UsePMFConnect = FALSE;
						pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA1;

						/*mismatch case*/
						if (((pSecConfig->PmfCfg.MFPR) && (RsnCap.field.MFPC == FALSE))
							|| ((pSecConfig->PmfCfg.MFPC == FALSE) && (RsnCap.field.MFPR))) {
							pSecConfig->PmfCfg.UsePMFConnect = FALSE;
							pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA256;
						}

						if ((pSecConfig->PmfCfg.MFPC) && (RsnCap.field.MFPC)) {
							pSecConfig->PmfCfg.UsePMFConnect = TRUE;

							if ((pStaCfg->MlmeAux.IsSupportSHA256KeyDerivation) || (RsnCap.field.MFPR))
								pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA256;
						}
					}
#endif /* DOT11W_PMF_SUPPORT */
					WPAMakeRSNIE(pStaCfg->wdev.wdev_type, pSecConfig, pAPEntry);
					pStaCfg->AKMMap = AKMMap;
					pStaCfg->PairwiseCipher = PairwiseCipher;
					pStaCfg->GroupCipher = GroupCipher;
				}

				for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
					if (pAPEntry->SecConfig.RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
						continue;

					MakeOutgoingFrame(pOutBuffer + FrameLen, &TempLen,
									  1, &pAPEntry->SecConfig.RSNE_EID[rsne_idx][0],
									  1, &pAPEntry->SecConfig.RSNE_Len[rsne_idx],
									  pAPEntry->SecConfig.RSNE_Len[rsne_idx], &pAPEntry->SecConfig.RSNE_Content[rsne_idx][0],
									  END_OF_ARGS);
					FrameLen += TempLen;
				}
			}

			{
				CHAR rsne_idx = 0;

				for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
					if (pStaCfg->wdev.SecConfig.RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
						continue;

					/* Append Variable IE */
					NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &pStaCfg->wdev.SecConfig.RSNE_EID[rsne_idx][0], 1);
					VarIesOffset += 1;
					NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &pStaCfg->wdev.SecConfig.RSNE_Len[rsne_idx], 1);
					VarIesOffset += 1;
					NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &pStaCfg->wdev.SecConfig.RSNE_Content[rsne_idx][0],
								   pStaCfg->wdev.SecConfig.RSNE_Len[rsne_idx]);
					VarIesOffset += pStaCfg->RSNIE_Len;
					/* Set Variable IEs Length */
					pStaCfg->ReqVarIELen = VarIesOffset;
					break;
				}
			}
		}

		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen +=  build_extra_ie(pAd, &ie_info);
#ifdef WSC_INCLUDED
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_wsc_ie(pAd, &ie_info);
#endif /* WSC_INCLUDED */
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd))
			MAP_InsertMapCapIE(pAd, wdev, pOutBuffer+FrameLen, &FrameLen);
#endif /* CONFIG_MAP_SUPPORT */

#ifdef CONFIG_OWE_SUPPORT
		if (IS_AKM_OWE(pAPEntry->SecConfig.AKMMap)) {
			OWE_INFO *owe = &pAPEntry->SecConfig.owe;
			UCHAR group = pStaCfg->curr_owe_group;
			owe->last_try_group = group;
			if (init_owe_group(owe, group) == 0) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
					("==> %s(), init_owe_group failed. shall not happen!\n", __func__));
				return;
			}

			FrameLen +=  build_owe_dh_ie(pAd, pAPEntry, (UCHAR *)(pOutBuffer + FrameLen), group);
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		}
#endif /*CONFIG_OWE_SUPPORT*/


		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pOutBuffer);
		RTMPSetTimer(assoc_timer, Timeout);
		assoc_fsm_state_transition(wdev, CliIdx, ASSOC_WAIT_RSP);
	}
}

/*
	==========================================================================
	Description:
		mlme reassoc req handling procedure
	Parameters:
		Elem -
	Pre:
		-# SSID  (Adapter->StaCfg[0].ssid[])
		-# BSSID (AP address, Adapter->StaCfg[0].bssid)
		-# Supported rates (Adapter->StaCfg[0].supported_rates[])
		-# Supported rates length (Adapter->StaCfg[0].supported_rates_len)
		-# Tx power (Adapter->StaCfg[0].tx_power)

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_mlme_reassoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR ApAddr[6];
	HEADER_802_11 ReassocHdr;
	USHORT CapabilityInfo, ListenIntv;
	ULONG Timeout;
	ULONG FrameLen = 0;
	BOOLEAN TimerCancelled;
	NDIS_STATUS NStatus;
	ULONG tmp;
	PUCHAR pOutBuffer = NULL;
	USHORT Status;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	struct wifi_dev *wdev = Elem->wdev;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	UCHAR CliIdx = 0xFF;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	assoc_fsm_state_transition(wdev, CliIdx, ASSOC_IDLE);

	if (sta_block_checker(Elem->wdev, &Elem->priv_data) == TRUE)
		return;

	/* the parameters are the same as the association */
	if (MlmeAssocReqSanity(pAd, Elem->Msg, Elem->MsgLen, ApAddr, &CapabilityInfo, &Timeout, &ListenIntv) == FALSE) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("ASSOC - MlmeReassocReqAction() sanity check failed. BUG!!!!\n"));
		Status = MLME_INVALID_FORMAT;
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_REASSOC_CONF, Status, &Elem->priv_data);
	}

	{
		struct _build_ie_info ie_info = {0};

		ie_info.frame_subtype = SUBTYPE_ASSOC_REQ;
		ie_info.channel = pStaCfg->MlmeAux.Channel;
		ie_info.phy_mode = wdev->PhyMode;
		ie_info.wdev = wdev;
		/*for dhcp,issue ,wpa_supplicant ioctl too fast , at link_up, it will add key before driver remove key  */
		RTMPWPARemoveAllKeys(pAd, wdev);
		RTMPCancelTimer(&pStaCfg->MlmeAux.ReassocTimer, &TimerCancelled);
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	/*Get an unused nonpaged memory */

		if (NStatus != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("ASSOC - MlmeReassocReqAction() allocate memory failed\n"));
			Status = MLME_FAIL_NO_RESOURCE;
			cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_REASSOC_CONF, Status, &Elem->priv_data);
			return;
		}

		COPY_MAC_ADDR(pStaCfg->MlmeAux.Bssid, ApAddr);
		/* make frame, use bssid as the AP address?? */
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("ASSOC - Send RE-ASSOC request...\n"));
		MgtMacHeaderInitExt(pAd, &ReassocHdr, SUBTYPE_REASSOC_REQ, 0, ApAddr,
							pStaCfg->wdev.if_addr, ApAddr);
		MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11),
						  &ReassocHdr, 2, &CapabilityInfo, 2,
						  &ListenIntv, MAC_ADDR_LEN, ApAddr, 1, &SsidIe,
						  1, &pStaCfg->MlmeAux.SsidLen,
						  pStaCfg->MlmeAux.SsidLen, pStaCfg->MlmeAux.Ssid, 1,
						  &SupRateIe, 1, &pStaCfg->MlmeAux.SupRateLen,
						  pStaCfg->MlmeAux.SupRateLen, pStaCfg->MlmeAux.SupRate,
						  END_OF_ARGS);

		if (pStaCfg->MlmeAux.ExtRateLen != 0) {
			MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
							  1, &ExtRateIe,
							  1, &pStaCfg->MlmeAux.ExtRateLen,
							  pStaCfg->MlmeAux.ExtRateLen,
							  pStaCfg->MlmeAux.ExtRate, END_OF_ARGS);
			FrameLen += tmp;
		}

#ifdef DOT11R_FT_SUPPORT

		/* Add MDIE if we are connection to DOT11R AP */
		if (pStaCfg->Dot11RCommInfo.bFtSupport &&
			pStaCfg->MlmeAux.MdIeInfo.Len) {
			PUINT8 mdie_ptr;
			UINT mdie_len = 0;
			/* MDIE */
			mdie_ptr = pOutBuffer + FrameLen;
			mdie_len = 5;
			FT_InsertMdIE(pAd, pOutBuffer + FrameLen, &FrameLen,
						  pStaCfg->MlmeAux.MdIeInfo.MdId,
						  pStaCfg->MlmeAux.MdIeInfo.FtCapPlc);

			/* Indicate the FT procedure */
			if (pStaCfg->Dot11RCommInfo.bInMobilityDomain &&
				!IS_CIPHER_NONE(pStaCfg->wdev.SecConfig.PairwiseCipher)) {
				UINT8 FtIeLen = 0;
				PMAC_TABLE_ENTRY pEntry;
				FT_MIC_CTR_FIELD mic_ctr;
				PUINT8 rsnie_ptr;
				UINT rsnie_len = 0;
				PUINT8 ftie_ptr;
				UINT ftie_len = 0;
				UINT8 ft_mic[16];

				pEntry = &pAd->MacTab.Content[MCAST_WCID];
				/* Insert RSNIE[PMK-R1-NAME] */
				rsnie_ptr = pOutBuffer + FrameLen;
				rsnie_len = 2 + pStaCfg->RSNIE_Len + 2 + LEN_PMK_NAME;
				WPAInsertRSNIE(pOutBuffer + FrameLen,
								&FrameLen,
								pStaCfg->RSN_IE,
								pStaCfg->RSNIE_Len,
								pEntry->FT_PMK_R1_NAME,
								LEN_PMK_NAME);
				/* Insert FTIE[MIC, ANONCE, SNONCE, R1KH-ID, R0KH-ID] */
				FtIeLen = sizeof(FT_FTIE) +
						  (2 + MAC_ADDR_LEN) +
						  (2 + pStaCfg->Dot11RCommInfo.R0khIdLen);
				ftie_ptr = pOutBuffer + FrameLen;
				ftie_len = (2 + FtIeLen);
				mic_ctr.field.IECnt = 3;
				NdisZeroMemory(ft_mic, 16);
				FT_InsertFTIE(pAd,
							  pOutBuffer + FrameLen,
							  &FrameLen,
							  FtIeLen,
							  mic_ctr,
							  ft_mic,
							  pEntry->FtIeInfo.ANonce, pEntry->FtIeInfo.SNonce);
				FT_FTIE_InsertKhIdSubIE(pAd,
										pOutBuffer + FrameLen,
										&FrameLen,
										FT_R1KH_ID,
										pStaCfg->MlmeAux.Bssid,
										MAC_ADDR_LEN);
				FT_FTIE_InsertKhIdSubIE(pAd,
										pOutBuffer + FrameLen,
										&FrameLen,
										FT_R0KH_ID,
										&pStaCfg->Dot11RCommInfo.R0khId[0],
										pStaCfg->Dot11RCommInfo.R0khIdLen);

				/* RIC-Request */
				if (pStaCfg->MlmeAux.MdIeInfo.FtCapPlc.field.RsrReqCap
					&& pStaCfg->Dot11RCommInfo.bSupportResource) {
				}

				/* Calculate MIC */
				if (mic_ctr.field.IECnt) {
					UINT8 ft_mic[16];
					PFT_FTIE pFtIe;

					FT_CalculateMIC(pAd->CurrentAddress,
									pStaCfg->MlmeAux.Bssid,
									pEntry->SecConfig.PTK,
									5,
									rsnie_ptr,
									rsnie_len,
									mdie_ptr,
									mdie_len,
									ftie_ptr,
									ftie_len,
									NULL, 0, ft_mic);
					/* Update the MIC field of FTIE */
					pFtIe = (PFT_FTIE) (ftie_ptr + 2);
					NdisMoveMemory(pFtIe->MIC, ft_mic, 16);
				}
			}
		}

#endif /* DOT11R_FT_SUPPORT */
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_wmm_cap_ie(pAd, &ie_info);
#ifdef DOT11_N_SUPPORT

		/* HT */
		if ((pStaCfg->MlmeAux.HtCapabilityLen > 0)
			&& WMODE_CAP_N(wdev->PhyMode)) {
			ULONG TmpLen;
			UCHAR HtLen;
			UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
			PHT_CAPABILITY_IE pHtCapability;
#ifdef RT_BIG_ENDIAN
			HT_CAPABILITY_IE HtCapabilityTmp;

			NdisZeroMemory(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
			NdisMoveMemory(&HtCapabilityTmp, &pStaCfg->MlmeAux.HtCapability, pStaCfg->MlmeAux.HtCapabilityLen);
			*(USHORT *) (&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *) (&HtCapabilityTmp.HtCapInfo));
			*(USHORT *) (&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(USHORT *) (&HtCapabilityTmp.ExtHtCapInfo));
			pHtCapability = &HtCapabilityTmp;
#else
			pHtCapability = &pStaCfg->MlmeAux.HtCapability;
#endif

			if (pStaCfg->StaActive.SupportedPhyInfo.bPreNHt == TRUE) {
				HtLen = SIZE_HT_CAP_IE + 4;
				MakeOutgoingFrame(pOutBuffer + FrameLen,
								  &TmpLen, 1, &WpaIe, 1, &HtLen,
								  4, &BROADCOM[0],
								  pStaCfg->MlmeAux.HtCapabilityLen,
								  pHtCapability, END_OF_ARGS);
			} else {
				MakeOutgoingFrame(pOutBuffer + FrameLen,
								  &TmpLen, 1, &HtCapIe, 1,
								  &pStaCfg->MlmeAux.HtCapabilityLen,
								  pStaCfg->MlmeAux.HtCapabilityLen,
								  pHtCapability, END_OF_ARGS);
			}

			FrameLen += TmpLen;
#ifdef DOT11_VHT_AC
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			ucETxBfCap = wlan_config_get_etxbf(wdev);

			if (HcIsBfCapSupport(wdev) == FALSE)
				wlan_config_set_etxbf(wdev, SUBF_OFF);

#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
			FrameLen += build_vht_ies(pAd, &ie_info);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#endif /* DOT11_VHT_AC */
		}

#endif /* DOT11_N_SUPPORT */
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_extended_cap_ie(pAd, &ie_info);
		/* add Ralink proprietary IE to inform AP this STA is going to use AGGREGATION or PIGGY-BACK+AGGREGATION */
		/* Case I: (Aggregation + Piggy-Back) */
		/* 1. user enable aggregation, AND */
		/* 2. Mac support piggy-back */
		/* 3. AP annouces it's PIGGY-BACK+AGGREGATION-capable in BEACON */
		/* Case II: (Aggregation) */
		/* 1. user enable aggregation, AND */
		/* 2. AP annouces it's AGGREGATION-capable in BEACON */
		FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen), VIE_ASSOC_REQ);
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pOutBuffer);
		RTMPSetTimer(&pStaCfg->MlmeAux.ReassocTimer, Timeout * 2);	/* in mSec */
		assoc_fsm_state_transition(Elem->wdev, CliIdx, REASSOC_WAIT_RSP);
	}
}

/*
	==========================================================================
	Description:
		Upper layer issues disassoc request
	Parameters:
		Elem -

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
static VOID sta_mlme_disassoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	MLME_DISCONNECT_STRUCT *pDisassocReq; /* snowpin for cntl mgmt */
	HEADER_802_11 DisassocHdr;
	PHEADER_802_11 pDisassocHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS NStatus;
	BOOLEAN TimerCancelled;
	ULONG Timeout = 500;
	USHORT Status = MLME_SUCCESS;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;
	struct wifi_dev *wdev = Elem->wdev;
	RALINK_TIMER_STRUCT *disassoc_timer = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = Elem->priv_data.rept_cli_entry;
	/* USHORT ifIndex = wdev->func_idx; */
#endif /* MAC_REPEATER_SUPPORT */
	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	if (CliIdx == NON_REPT_ENTRY)
		STA_STATUS_SET_FLAG(pStaCfg, fSTA_STATUS_APCLI_MAIN_LINK_DOWN_IN_PROGRESS);


#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != NON_REPT_ENTRY) {
		disassoc_timer = &pReptEntry->ApCliAssocTimer;
		ASSERT(pReptEntry->wdev == wdev);
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		disassoc_timer = &pStaCfg->MlmeAux.DisassocTimer;
	}

	/* skip sanity check */
	pDisassocReq = (MLME_DISCONNECT_STRUCT *) (Elem->Msg); /* snowpin for cntl mgmt */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	/*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("ASSOC - MlmeDisassocReqAction() allocate memory failed\n"));
		Status = MLME_FAIL_NO_RESOURCE;
		goto SEND_EVENT_TO_CNTL;
	}

#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx == NON_REPT_ENTRY)
#endif /* MAC_REPEATER_SUPPORT */
		RTMPCancelTimer(&pStaCfg->MlmeAux.DisassocTimer, &TimerCancelled);
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("ASSOC - Send DISASSOC request[BSSID::%02x:%02x:%02x:%02x:%02x:%02x (Reason=%d)\n",
			  PRINT_MAC(pDisassocReq->addr), pDisassocReq->reason)); /* snowpin for cntl mgmt */
	MgtMacHeaderInitExt(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pDisassocReq->addr, /* snowpin for cntl mgmt */
						pStaCfg->wdev.if_addr,
						pDisassocReq->addr);	/* patch peap ttls switching issue */ /* snowpin for cntl mgmt */
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF)
		COPY_MAC_ADDR(DisassocHdr.Addr2, pAd->ApCfg.pRepeaterCliPool[CliIdx].CurrentAddress);

#endif /* MAC_REPEATER_SUPPORT */
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(HEADER_802_11), &DisassocHdr,
					  2, &pDisassocReq->reason, END_OF_ARGS); /* snowpin for cntl mgmt */
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	/* To patch Instance and Buffalo(N) AP */
	/* Driver has to send deauth to Instance AP, but Buffalo(N) needs to send disassoc to reset Authenticator's state machine */
	/* Therefore, we send both of them. */
	pDisassocHdr = (PHEADER_802_11) pOutBuffer;
	pDisassocHdr->FC.SubType = SUBTYPE_DEAUTH;
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	pStaCfg->DisassocReason = REASON_DISASSOC_STA_LEAVING;
	COPY_MAC_ADDR(pStaCfg->DisassocSta, pDisassocReq->addr); /* snowpin for cntl mgmt */
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx == NON_REPT_ENTRY)
#endif /* MAC_REPEATER_SUPPORT */
	{
		RTMPSetTimer(&pStaCfg->MlmeAux.DisassocTimer, Timeout);	/* in mSec */
		assoc_fsm_state_transition(wdev, CliIdx, DISASSOC_WAIT_RSP);
	}

SEND_EVENT_TO_CNTL:
	/* linkdown should be done after DisAssoc frame is sent */
	cntl_auth_assoc_conf(wdev, CNTL_MLME_DISASSOC_CONF, Status, &Elem->priv_data);
	RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, NULL, BSS0, 0);
}

/*
	==========================================================================
	Description:
		peer sends assoc rsp back
	Parameters:
		Elme - MLME message containing the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_peer_assoc_rsp_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT CapabilityInfo, Status, Aid;
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES], SupRateLen;
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRateLen;
	UCHAR Addr2[MAC_ADDR_LEN];
	BOOLEAN TimerCancelled;
	UCHAR CkipFlag;
	EDCA_PARM EdcaParm;
	HT_CAPABILITY_IE HtCapability;
	ADD_HT_INFO_IE AddHtInfo;	/* AP might use this additional ht info IE */
	UCHAR HtCapabilityLen = 0;
	UCHAR AddHtInfoLen;
	UCHAR NewExtChannelOffset = 0xff;
	EXT_CAP_INFO_ELEMENT ExtCapInfo;
	MAC_TABLE_ENTRY *pEntry;
	IE_LISTS *ie_list = NULL;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA) || defined(APCLI_CFG80211_SUPPORT)
	PFRAME_802_11 pFrame =	(PFRAME_802_11) (Elem->Msg);
#endif
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = Elem->priv_data.rept_cli_entry;
#endif /* MAC_REPEATER_SUPPORT */
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	os_alloc_mem(pAd, (UCHAR **)&ie_list, sizeof(IE_LISTS));

	if (ie_list == NULL) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():mem alloc failed!\n", __func__));
		return;
	}

	NdisZeroMemory((UCHAR *)ie_list, sizeof(IE_LISTS));
	os_zero_mem(&EdcaParm, sizeof(EDCA_PARM));

	if (PeerAssocRspSanity(&pStaCfg->wdev, Elem->Msg, Elem->MsgLen,
						   Addr2, &CapabilityInfo, &Status, &Aid, SupRate,
						   &SupRateLen, ExtRate, &ExtRateLen, &HtCapability,
						   &AddHtInfo, &HtCapabilityLen, &AddHtInfoLen,
						   &NewExtChannelOffset, &EdcaParm, &ExtCapInfo,
						   &CkipFlag, ie_list)) {
		/* The frame is for me ? */
		if (MAC_ADDR_EQUAL(Addr2, pStaCfg->MlmeAux.Bssid)) {
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s():ASSOC - receive ASSOC_RSP to me (status=%d)\n", __func__, Status));
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA) || defined(APCLI_CFG80211_SUPPORT)
			pFrame =	(PFRAME_802_11) (Elem->Msg);
			/* Store the AssocRsp Frame to wpa_supplicant via CFG80211 */
			NdisZeroMemory(pAd->StaCfg[ifIndex].ResVarIEs, MAX_VIE_LEN);
			pAd->StaCfg[ifIndex].ResVarIELen = 0;
			pAd->StaCfg[ifIndex].ResVarIELen = Elem->MsgLen - 6 - sizeof(HEADER_802_11);
			NdisCopyMemory(pAd->StaCfg[ifIndex].ResVarIEs, &pFrame->Octet[6], pAd->StaCfg[ifIndex].ResVarIELen);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
#ifdef MAC_REPEATER_SUPPORT

			if (CliIdx != NON_REPT_ENTRY)
				RTMPCancelTimer(&pReptEntry->ApCliAssocTimer, &TimerCancelled);
			else
#endif /* MAC_REPEATER_SUPPORT */
				RTMPCancelTimer(&pStaCfg->MlmeAux.AssocTimer, &TimerCancelled);

#ifdef DOT11R_FT_SUPPORT

			if (pStaCfg->Dot11RCommInfo.bFtSupport && pStaCfg->MlmeAux.FtIeInfo.Len) {
				FT_FTIE_INFO *pFtInfo = &pStaCfg->MlmeAux.FtIeInfo;

				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():ASSOC - FTIE\n", __func__));
				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("MIC Countrol IECnt: %x\n", pFtInfo->MICCtr.field.IECnt));
				hex_dump("ANonce", pFtInfo->ANonce, 32);
				hex_dump("SNonce", pFtInfo->SNonce, 32);

				if (pFtInfo->R1khIdLen)
					hex_dump("R1KH-ID", pFtInfo->R1khId, pFtInfo->R1khIdLen);

				if (pFtInfo->R0khIdLen)
					hex_dump("R0KH-ID", pFtInfo->R0khId, pFtInfo->R0khIdLen);

				if ((pStaCfg->Dot11RCommInfo.R0khIdLen != pFtInfo->R0khIdLen)
					|| (!NdisEqualMemory(pFtInfo->R0khId, pFtInfo->R0khId,
										 pStaCfg->Dot11RCommInfo.R0khIdLen))) {
					if (pStaCfg->Dot11RCommInfo.bInMobilityDomain)
						Status = MLME_INVALID_FORMAT;
				}
			}

#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11V_WNM_SUPPORT

			if (ExtCapInfo.BssTransitionManmt == 1)
				pStaCfg->bBSSMantAPSupport = TRUE;

			if (ExtCapInfo.DMSSupport == 1)
				pStaCfg->bDMSAPSupport = TRUE;

#endif /* DOT11V_WNM_SUPPORT */


			if (Status == MLME_SUCCESS) {
				UCHAR MaxSupportedRateIn500Kbps = 0;
				UCHAR op_mode = OPMODE_AP;

				MaxSupportedRateIn500Kbps = dot11_max_sup_rate(SupRateLen, &SupRate[0],
											ExtRateLen, &ExtRate[0]);
#ifdef MAC_REPEATER_SUPPORT

				if (CliIdx != NON_REPT_ENTRY)
					pEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
				else
#endif /* MAC_REPEATER_SUPPORT */
					pEntry = MacTableLookup2(pAd, Addr2, &pStaCfg->wdev);

				ASSERT(pEntry);

				if (!pEntry)
					return;

				set_mlme_rsn_ie(pAd, wdev, pEntry);
				if (IF_COMBO_HAVE_AP_STA(pAd) && wdev->wdev_type == WDEV_TYPE_STA) {
					op_mode = OPMODE_AP;
#ifdef CONFIG_MAP_SUPPORT
					if (IS_MAP_ENABLE(pAd)) {
						pEntry->DevPeerRole = ie_list->MAP_AttriValue;
					}
#endif /* CONFIG_MAP_SUPPORT */
					if (CliIdx == NON_REPT_ENTRY) {
						ApCliAssocPostProc(pAd, wdev, Addr2, CapabilityInfo, SupRate, SupRateLen,
										   ExtRate, ExtRateLen, &EdcaParm, &HtCapability, HtCapabilityLen, &AddHtInfo, pEntry);
						pStaCfg->MlmeAux.Aid = Aid;
#ifdef CONFIG_OWE_SUPPORT
						if (IS_AKM_OWE(wdev->SecConfig.AKMMap)) {
							BOOLEAN need_process_ecdh_ie = FALSE;
							UINT8 *pmkid = NULL;
							UINT8 pmkid_count = 0;

							pmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE,
											ie_list->RSNIE_Len,
											PMKID_LIST,
											&pmkid_count);
								if (pmkid != NULL) {
									INT idx;
									BOOLEAN FoundPMK = FALSE;

									/*  Search chched PMKID, append it if existed */
									idx = sta_search_pmkid_cache(pAd, Addr2, wdev->func_idx, CliIdx);
									if (idx != INVALID_PMKID_IDX)
										FoundPMK = TRUE;

									if (FoundPMK == FALSE) {
										need_process_ecdh_ie = TRUE;
										MTWF_LOG(DBG_CAT_AP,
														DBG_SUBCAT_ALL,
														DBG_LVL_ERROR,
														("%s: cannot find match PMKID\n", __func__));
									} else if ((pEntry->SecConfig.pmkid) &&
												((RTMPEqualMemory(pmkid, pEntry->SecConfig.pmkid, LEN_PMKID)) != 0)) {
											/*
											 * if STA would like to use PMK CACHE,
											 * it stored the PMKID in assoc req stage already.
											 * no need to restore it again here.
											 */
											MTWF_LOG(DBG_CAT_AP,
															DBG_SUBCAT_ALL,
															DBG_LVL_ERROR,
															("%s: PMKID doesn't match STA sent\n", __func__));
											need_process_ecdh_ie = TRUE;
									}
								} else
										need_process_ecdh_ie = TRUE;

								if (need_process_ecdh_ie == TRUE) {
									MTWF_LOG(DBG_CAT_AP,
													DBG_SUBCAT_ALL,
													DBG_LVL_TRACE,
													("%s: do normal ECDH procedure\n", __func__));
/* XXX: TRACE_NM: Should we process this during EAPOL-1_of_4 handling */
									process_ecdh_element(pAd,
													pEntry,
													(EXT_ECDH_PARAMETER_IE *)&ie_list->ecdh_ie,
													ie_list->ecdh_ie.length,
													SUBTYPE_ASSOC_RSP);
								}
						}
#endif /*CONFIG_OWE_SUPPORT*/
#ifdef DOT11_VHT_AC
						RTMPZeroMemory(&pStaCfg->MlmeAux.vht_cap, sizeof(VHT_CAP_IE));
						RTMPZeroMemory(&pStaCfg->MlmeAux.vht_op, sizeof(VHT_OP_IE));
						pStaCfg->MlmeAux.vht_cap_len = 0;
						pStaCfg->MlmeAux.vht_op_len = 0;

						if (WMODE_CAP_AC(pStaCfg->wdev.PhyMode) && ie_list->vht_cap_len && ie_list->vht_op_len) {
							MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("There is vht le at Assoc Rsp ifIndex=%d vht_cap_len=%d\n", ifIndex, ie_list->vht_cap_len));
							NdisMoveMemory(&pStaCfg->MlmeAux.vht_cap, &(ie_list->vht_cap), ie_list->vht_cap_len);
							pStaCfg->MlmeAux.vht_cap_len = ie_list->vht_cap_len;
							NdisMoveMemory(&pStaCfg->MlmeAux.vht_op, &(ie_list->vht_op), ie_list->vht_op_len);
							pStaCfg->MlmeAux.vht_op_len = ie_list->vht_op_len;
						}

#endif /* DOT11_VHT_AC */
					}

					/* For Repeater get correct wmm valid setting */
					pStaCfg->MlmeAux.APEdcaParm.bValid = EdcaParm.bValid;

#ifdef APCLI_AS_WDS_STA_SUPPORT
				{
					PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[pStaCfg->MacTabWCID];

					if (!(IS_AKM_WPA_CAPABILITY_Entry(pEntry)
#ifdef DOT1X_SUPPORT
					|| IS_IEEE8021X(&pEntry->SecConfig)
#endif /* DOT1X_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
					|| pEntry->wdev->IsCFG1xWdev
#endif /* RT_CFG80211_SUPPORT */
					|| pEntry->bWscCapable)) {
					pEntry->bEnable4Addr = TRUE;
					if (pStaCfg->wdev.wds_enable)
						HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pStaCfg->MacTabWCID, TRUE);
					}
				}
#endif /* APCLI_AS_WDS_STA_SUPPORT */
					/*
						In roaming case, LinkDown wouldn't be invoked.
						For preventing finding MacTable Hash index malfunction,
						we need to do MacTableDeleteEntry here.
					*/
				} else if (wdev->wdev_type == WDEV_TYPE_STA) {
					op_mode = OPMODE_STA;
					/* go to procedure listed on page 376 */
					sta_assoc_post_proc(pAd, Addr2, CapabilityInfo, Aid,
										SupRate, SupRateLen, ExtRate,
										ExtRateLen, &EdcaParm,
										ie_list,
										&HtCapability, HtCapabilityLen,
										&AddHtInfo,
										pEntry);
				}

				if (EdcaParm.bValid && wdev->bWmmCapable)
					CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
				else
					CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

				StaUpdateMacTableEntry(pAd,
									   pEntry,
									   MaxSupportedRateIn500Kbps,
									   &HtCapability,
									   HtCapabilityLen, &AddHtInfo,
									   AddHtInfoLen,
									   ie_list,
									   CapabilityInfo);
					TRTableInsertEntry(pAd, pEntry->wcid, pEntry);

				/* TRTableEntryDump(pAd, pEntry->wcid, __FUNCTION__, __LINE__); */
				/* ---Add by shiang for debug */

				RTMPSetSupportMCS(pAd, op_mode, pEntry,
								  SupRate, SupRateLen,
								  ExtRate, ExtRateLen,
#ifdef DOT11_VHT_AC
								  ie_list->vht_cap_len,
								  &ie_list->vht_cap,
#endif /* DOT11_VHT_AC */
								  &HtCapability,
								  HtCapabilityLen);
#ifdef APCLI_CFG80211_SUPPORT
				CFG80211_checkScanTable(pAd);

				RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, pStaCfg->MlmeAux.Bssid, ifIndex,
					pStaCfg->ReqVarIEs, pStaCfg->ReqVarIELen,
					pStaCfg->ResVarIEs, pStaCfg->ResVarIELen, TRUE);

#endif
			} else {
#ifdef FAST_EAPOL_WAR
				ApCliAssocDeleteMacEntry(pAd, ifIndex, CliIdx);
#endif /* FAST_EAPOL_WAR */

#ifdef DOT11_SAE_SUPPORT
				/* TRACE_NM: Should we use pEntry ? */
				if ((IS_AKM_SAE(pStaCfg->AKMMap)) && (Status == MLME_INVALID_PMKID || Status == 40)) {
					UCHAR if_addr[6];
					INT CachedIdx;
					SAE_INSTANCE *pSaeIns = NULL;

#ifdef MAC_REPEATER_SUPPORT
					if (CliIdx != NON_REPT_ENTRY)
						NdisCopyMemory(if_addr, pAd->ApCfg.pRepeaterCliPool[CliIdx].CurrentAddress, MAC_ADDR_LEN);
					else
#endif /* MAC_REPEATER_SUPPORT */
						NdisCopyMemory(if_addr, pStaCfg->wdev.if_addr, MAC_ADDR_LEN);

					CachedIdx = sta_search_pmkid_cache(pAd, Addr2, ifIndex, CliIdx);
					if (CachedIdx != INVALID_PMKID_IDX) {
						MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF,
										("%s: Delete pmkid on assoc fail(incorrect pmkid)\n", __func__));
						sta_delete_pmkid_cache(pAd, Addr2, ifIndex, CliIdx);
					}
					pSaeIns = search_sae_instance(&pAd->SaeCfg, if_addr, Addr2);
					if (pSaeIns != NULL) {
						MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF,
										("%s: Delete Existing sae instance on assoc fail(incorrect pmkid)\n", __func__));
						delete_sae_instance(pSaeIns);
					}
				}
#endif
#ifdef APCLI_CFG80211_SUPPORT

					CFG80211_checkScanTable(pAd);
					RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, pStaCfg->MlmeAux.Bssid, ifIndex, NULL, 0, NULL, 0, 0);
#endif

			}

			assoc_fsm_state_transition(Elem->wdev, CliIdx, ASSOC_IDLE);
			cntl_auth_assoc_conf(Elem->wdev,  CNTL_MLME_ASSOC_CONF, Status, &Elem->priv_data);
#ifdef LINUX
#ifndef APCLI_CFG80211_SUPPORT
#ifdef RT_CFG80211_SUPPORT

			if (Status == MLME_SUCCESS) {
				PFRAME_802_11 pFrame =  (PFRAME_802_11) (Elem->Msg);

				RTEnqueueInternalCmd(pAd, CMDTHREAD_CONNECT_RESULT_INFORM,
									 &pFrame->Octet[6], Elem->MsgLen - 6 - sizeof(HEADER_802_11));
			}

#endif /* RT_CFG80211_SUPPORT */
#endif
#endif /* LINUX */
		}
	} else
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ASSOC - %s() sanity check fail\n", __func__));

	if (ie_list != NULL)
		os_free_mem(ie_list);
}


/*
	==========================================================================
	Description:
		peer sends reassoc rsp
	Parametrs:
		Elem - MLME message cntaining the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_peer_reassoc_rsp_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT CapabilityInfo;
	USHORT Status;
	USHORT Aid;
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES], SupRateLen;
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRateLen;
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR CkipFlag;
	BOOLEAN TimerCancelled;
	EDCA_PARM EdcaParm;
	HT_CAPABILITY_IE HtCapability;
	ADD_HT_INFO_IE AddHtInfo;	/* AP might use this additional ht info IE */
	UCHAR HtCapabilityLen;
	UCHAR AddHtInfoLen;
	UCHAR NewExtChannelOffset = 0xff;
	EXT_CAP_INFO_ELEMENT ExtCapInfo;
	IE_LISTS *ie_list = NULL;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	UCHAR CliIdx = 0xFF;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	os_alloc_mem(pAd, (UCHAR **)&ie_list, sizeof(IE_LISTS));

	if (ie_list == NULL) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():mem alloc failed!\n", __func__));
		return;
	}

	NdisZeroMemory((UCHAR *)ie_list, sizeof(IE_LISTS));

	if (PeerAssocRspSanity(&pStaCfg->wdev, Elem->Msg, Elem->MsgLen, Addr2,
						   &CapabilityInfo, &Status, &Aid, SupRate,
						   &SupRateLen, ExtRate, &ExtRateLen, &HtCapability,
						   &AddHtInfo, &HtCapabilityLen, &AddHtInfoLen,
						   &NewExtChannelOffset, &EdcaParm, &ExtCapInfo,
						   &CkipFlag, ie_list)) {
		if (MAC_ADDR_EQUAL(Addr2, pStaCfg->MlmeAux.Bssid)) {	/* The frame is for me ? */
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("REASSOC - receive REASSOC_RSP to me (status=%d)\n", Status));
			RTMPCancelTimer(&pStaCfg->MlmeAux.ReassocTimer,
							&TimerCancelled);

			if (Status == MLME_SUCCESS) {
				UCHAR MaxSupportedRateIn500Kbps = 0;
				PMAC_TABLE_ENTRY pEntry = NULL;
				/*
					In roaming case, LinkDown wouldn't be invoked.
					For preventing finding MacTable Hash index malfunction,
					we need to do MacTableDeleteEntry here.
				*/
				pEntry = MacTableLookup2(pAd, pStaCfg->Bssid, &pStaCfg->wdev);

				if (pEntry) {
					MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
					pEntry = NULL;
				}

				pEntry = MacTableLookup2(pAd, Addr2, &pStaCfg->wdev);

				if (pEntry) {
					MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
					pEntry = NULL;
				}

				pEntry = MacTableInsertEntry(pAd, Addr2, &pStaCfg->wdev, ENTRY_INFRA, OPMODE_STA, TRUE);
				ASSERT(pEntry);
				MaxSupportedRateIn500Kbps = dot11_max_sup_rate(SupRateLen, &SupRate[0], ExtRateLen, &ExtRate[0]);
				/* go to procedure listed on page 376 */
				sta_assoc_post_proc(pAd, Addr2, CapabilityInfo, Aid,
									SupRate, SupRateLen, ExtRate,
									ExtRateLen, &EdcaParm,
									ie_list,
									&HtCapability, HtCapabilityLen,
									&AddHtInfo,
									pEntry);
				StaUpdateMacTableEntry(pAd,
									   pEntry,
									   MaxSupportedRateIn500Kbps,
									   &HtCapability,
									   HtCapabilityLen, &AddHtInfo,
									   AddHtInfoLen,
									   ie_list,
									   CapabilityInfo);
				RTMPSetSupportMCS(pAd,
								  OPMODE_STA,
								  pEntry,
								  SupRate,
								  SupRateLen,
								  ExtRate,
								  ExtRateLen,
#ifdef DOT11_VHT_AC
								  ie_list->vht_cap_len,
								  &ie_list->vht_cap,
#endif /* DOT11_VHT_AC */
								  &HtCapability,
								  HtCapabilityLen);
			}

#ifdef DOT11V_WNM_SUPPORT

			if (ExtCapInfo.BssTransitionManmt == 1)
				pStaCfg->bBSSMantAPSupport = TRUE;

			if (ExtCapInfo.DMSSupport == 1)
				pStaCfg->bDMSAPSupport = TRUE;

#endif /* DOT11V_WNM_SUPPORT */
			/* CkipFlag is no use for reassociate */
			assoc_fsm_state_transition(Elem->wdev, CliIdx, ASSOC_IDLE);
			cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_REASSOC_CONF, Status, &Elem->priv_data);
		}
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("REASSOC - %s() sanity check fail\n", __func__));
	}

	if (ie_list)
		os_free_mem(ie_list);
}


/*
	==========================================================================
	Description:
		left part of IEEE 802.11/1999 p.374
	Parameters:
		Elem - MLME message containing the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_peer_disassoc_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	UCHAR Addr2[MAC_ADDR_LEN];
	USHORT Reason;
	PSTA_ADMIN_CONFIG pStaCfg;
	ULONG *pDisconnect_Sub_Reason = NULL;
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = Elem->priv_data.rept_cli_entry;
#endif /* MAC_REPEATER_SUPPORT */
#ifdef FAST_EAPOL_WAR
	USHORT ifIndex = wdev->func_idx;
#endif /* FAST_EAPOL_WAR */
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	{
		struct wifi_dev *wdev = &pStaCfg->wdev;
		WSC_CTRL *wsc_ctrl = &wdev->WscControl;

		if (wsc_ctrl->WscState == WSC_STATE_WAIT_EAPFAIL) {
			RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&wsc_ctrl->WscEAPHandshakeCompleted,
								DISASSOC_WAIT_EAP_SUCCESS);
		}
	}

	if (CliIdx == NON_REPT_ENTRY)
		STA_STATUS_SET_FLAG(pStaCfg, fSTA_STATUS_APCLI_MAIN_LINK_DOWN_IN_PROGRESS);


#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != NON_REPT_ENTRY)
		pDisconnect_Sub_Reason = &pReptEntry->Disconnect_Sub_Reason;
	else
#endif /* MAC_REPEATER_SUPPORT */
		pDisconnect_Sub_Reason = &pStaCfg->ApcliInfStat.Disconnect_Sub_Reason;

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ASSOC - PeerDisassocAction()\n"));

	if (PeerDisassocSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Reason)) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("ASSOC - PeerDisassocAction() Reason = %d\n",
				  Reason));
#ifdef FAST_EAPOL_WAR
		ApCliAssocDeleteMacEntry(pAd, ifIndex, CliIdx);
#endif /* FAST_EAPOL_WAR */

		if (INFRA_ON(pStaCfg)
			&& MAC_ADDR_EQUAL(pStaCfg->Bssid, Addr2)) {
			RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, NULL,
								  BSS0, 0);
			/*
			   It is possible that AP sends dis-assoc frame(PeerDisassocAction) to STA
			   after driver enqueue ASSOC_FSM_MLME_DISASSOC_REQ (MlmeDisassocReqAction)
			   and set CntlMachine.CurrState = CNTL_WAIT_DISASSOC.
			   DisassocTimer is useless because AssocMachine.CurrState will set to ASSOC_IDLE here.
			   Therefore, we need to check CntlMachine.CurrState here and enqueue MT2_DISASSOC_CONF to
			   reset CntlMachine.CurrState to CNTL_IDLE state again.
			 */
			cntl_fsm_state_transition(wdev, CliIdx, CNTL_WAIT_DISASSOC, __func__);

			if (cntl_auth_assoc_conf(wdev, CNTL_MLME_DISASSOC_CONF, Reason, &Elem->priv_data) == FALSE) {
				UINT link_down_type = 0;

				link_down_type |= LINK_REQ_FROM_AP;
				link_down_type |= LINK_HAVE_INTER_SM_DATA;
				LinkDown(pAd, link_down_type, wdev, Elem);
			}

			assoc_fsm_state_transition(Elem->wdev, CliIdx, ASSOC_IDLE);
		} else if (STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_APCLI_MAIN_LINK_DOWN_IN_PROGRESS)) {
			STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_APCLI_MAIN_LINK_DOWN_IN_PROGRESS);

		}
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("ASSOC - PeerDisassocAction() sanity check fail\n"));
		if (STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_APCLI_MAIN_LINK_DOWN_IN_PROGRESS)) {
			STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_APCLI_MAIN_LINK_DOWN_IN_PROGRESS);

		}
	}
}

/*
	==========================================================================
	Description:
		what the state machine will do after assoc timeout
	Parameters:
		Elme -

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_mlme_assoc_req_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct wifi_dev *wdev = Elem->wdev;
#ifdef FAST_EAPOL_WAR
	USHORT ifIndex = wdev->func_idx;
#endif /* FAST_EAPOL_WAR */
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;
	/* rept use different timer assigned in InsertRepeaterEntry */
	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ASSOC - AssocTimeoutAction\n"));
	/* rept use different timer assigned in InsertRepeaterEntry */
#ifdef FAST_EAPOL_WAR
	ApCliAssocDeleteMacEntry(pAd, ifIndex, CliIdx);
#endif /* FAST_EAPOL_WAR */
	assoc_fsm_state_transition(wdev, CliIdx, ASSOC_IDLE);
	Status = MLME_REJ_TIMEOUT;
	cntl_auth_assoc_conf(wdev, CNTL_MLME_ASSOC_CONF, Status, &Elem->priv_data);
}

/*
	==========================================================================
	Description:
		what the state machine will do after reassoc timeout

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_mlme_reassoc_req_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ASSOC - ReassocTimeoutAction\n"));
	assoc_fsm_state_transition(Elem->wdev, CliIdx, ASSOC_IDLE);
	Status = MLME_REJ_TIMEOUT;
	cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_REASSOC_CONF, Status, &Elem->priv_data);
}

/*
	==========================================================================
	Description:
		what the state machine will do after disassoc timeout

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_mlme_disassoc_req_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;
	/* rept doesn't have disassoc_req_timeout? */
	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ASSOC - DisassocTimeoutAction\n"));
	assoc_fsm_state_transition(Elem->wdev, CliIdx, ASSOC_IDLE);
	Status = MLME_SUCCESS;
	cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_DISASSOC_CONF, Status, &Elem->priv_data);
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
	RT_CFG80211_LOST_GO_INFORM(pAd);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
}

VOID sta_assoc_init(struct wifi_dev *wdev)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	sta_assoc_api.mlme_assoc_req_action = sta_mlme_assoc_req_action;
	sta_assoc_api.peer_assoc_rsp_action = sta_peer_assoc_rsp_action;
	sta_assoc_api.mlme_assoc_req_timeout_action = sta_mlme_assoc_req_timeout_action;
	sta_assoc_api.mlme_reassoc_req_action = sta_mlme_reassoc_req_action;
	sta_assoc_api.peer_reassoc_rsp_action = sta_peer_reassoc_rsp_action;
	sta_assoc_api.mlme_reassoc_req_timeout_action = sta_mlme_reassoc_req_timeout_action;
	sta_assoc_api.mlme_disassoc_req_action = sta_mlme_disassoc_req_action;
	sta_assoc_api.peer_disassoc_action =     sta_peer_disassoc_action;
	sta_assoc_api.mlme_disassoc_req_timeout_action = sta_mlme_disassoc_req_timeout_action;
	wdev->assoc_api = &sta_assoc_api;
	wdev->assoc_machine.CurrState = ASSOC_IDLE;

	/* initialize the timer */
	if (!pStaCfg->MlmeAux.AssocTimer.Valid) {
		pStaCfg->MlmeAux.AssocTimerFuncContext.pAd = pAd;
		pStaCfg->MlmeAux.AssocTimerFuncContext.wdev = wdev;
		RTMPInitTimer(pAd, &pStaCfg->MlmeAux.AssocTimer,
					  GET_TIMER_FUNCTION(sta_assoc_timeout), &pStaCfg->MlmeAux.AssocTimerFuncContext, FALSE);
	}

	if (!pStaCfg->MlmeAux.ReassocTimer.Valid) {
		pStaCfg->MlmeAux.ReassocTimerFuncContext.pAd = pAd;
		pStaCfg->MlmeAux.ReassocTimerFuncContext.wdev = wdev;
		RTMPInitTimer(pAd, &pStaCfg->MlmeAux.ReassocTimer,
					  GET_TIMER_FUNCTION(sta_reassoc_timeout), &pStaCfg->MlmeAux.ReassocTimerFuncContext, FALSE);
	}

	if (!pStaCfg->MlmeAux.DisassocTimer.Valid) {
		pStaCfg->MlmeAux.DisassocTimerFuncContext.pAd = pAd;
		pStaCfg->MlmeAux.DisassocTimerFuncContext.wdev = wdev;
		RTMPInitTimer(pAd, &pStaCfg->MlmeAux.DisassocTimer,
					  GET_TIMER_FUNCTION(sta_disassoc_timeout), &pStaCfg->MlmeAux.DisassocTimerFuncContext, FALSE);
	}

	if (!pStaCfg->LinkDownTimer.Valid)
		RTMPInitTimer(pAd, &pStaCfg->LinkDownTimer,
					  GET_TIMER_FUNCTION(sta_link_down_exec), wdev, FALSE);

	if (!pStaCfg->MlmeAux.WpaDisassocAndBlockAssocTimer.Valid)
		RTMPInitTimer(pAd, &pStaCfg->MlmeAux.WpaDisassocAndBlockAssocTimer,
					  GET_TIMER_FUNCTION(WpaDisassocApAndBlockAssoc), wdev, FALSE);
}

