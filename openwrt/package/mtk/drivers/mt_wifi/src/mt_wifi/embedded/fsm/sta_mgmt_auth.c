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


struct _auth_api_ops sta_auth_api;

#ifdef MAC_REPEATER_SUPPORT
#define IS_APCLI_RPT_IFINDEX_INVALID(ad, wdev, idx) (\
		IS_OPMODE_AP(ad)\
		&& (wdev)->wdev_type == WDEV_TYPE_STA\
		&& (idx) >= MAX_APCLI_NUM\
		&& (idx) < REPT_MLME_START_IDX)
#else
#define IS_APCLI_RPT_IFINDEX_INVALID(ad, wdev, idx) (\
		IS_OPMODE_AP(ad)\
		&& (wdev)->wdev_type == WDEV_TYPE_STA\
		&& (idx) >= MAX_APCLI_NUM)

#endif /* MAC_REPEATER_SUPPORT */

BOOLEAN sta_send_auth_req(
	IN PRTMP_ADAPTER pAd,
	IN PMLME_QUEUE_ELEM Elem,
	IN PRALINK_TIMER_STRUCT pAuthTimer,
	IN RTMP_STRING *pSMName,
	IN USHORT SeqNo,
	IN PUCHAR pNewElement,
	IN ULONG ElementLen)
{
	USHORT Alg, Seq, Status;
	UCHAR Addr[6];
	ULONG Timeout;
	HEADER_802_11 AuthHdr;
	BOOLEAN TimerCancelled;
	NDIS_STATUS NStatus;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0, tmp = 0;
	struct wifi_dev *wdev = Elem->wdev;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	USHORT ifIndex = wdev->func_idx;
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = Elem->priv_data.rept_cli_entry;
#endif /* MAC_REPEATER_SUPPORT */
	ASSERT(pStaCfg);
	ASSERT(wdev);
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	if (!pStaCfg)
		return FALSE;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return FALSE;

	/* Block all authentication request durning WPA block period */
	if (pStaCfg->bBlockAssoc == TRUE) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s - Block Auth request durning WPA block period!\n",
				  pSMName));
		auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);
		Status = MLME_STATE_MACHINE_REJECT;
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status, &Elem->priv_data);
		return FALSE;
	} else if (MlmeAuthReqSanity(pAd, Elem->wdev, Elem->Msg, Elem->MsgLen, Addr, &Timeout, &Alg)) {
		/* reset timer */
		RTMPCancelTimer(pAuthTimer, &TimerCancelled);
		COPY_MAC_ADDR(pStaCfg->MlmeAux.Bssid, Addr);
		pStaCfg->MlmeAux.Alg = Alg;
		Seq = SeqNo;
		Status = MLME_SUCCESS;
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	/*Get an unused nonpaged memory */

		if (NStatus != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s - MlmeAuthReqAction(Alg:%d) allocate memory failed\n",
					  pSMName, Alg));
			auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);
			Status = MLME_FAIL_NO_RESOURCE;
			cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status, &Elem->priv_data);
			return FALSE;
		}


		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s - Send AUTH request seq#1 (Alg=%d)...\n",
				  pSMName, Alg));
		MgtMacHeaderInitExt(pAd, &AuthHdr, SUBTYPE_AUTH, 0, Addr, pStaCfg->wdev.if_addr,
							pStaCfg->MlmeAux.Bssid);
#ifdef MAC_REPEATER_SUPPORT

		if (CliIdx != 0xFF)
			COPY_MAC_ADDR(AuthHdr.Addr2, pReptEntry->CurrentAddress);

#endif /* MAC_REPEATER_SUPPORT */
		MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11),
						  &AuthHdr, 2, &Alg, 2, &Seq, 2, &Status,
						  END_OF_ARGS);

		if (pNewElement && ElementLen) {
			MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
							  ElementLen, pNewElement, END_OF_ARGS);
			FrameLen += tmp;
		}


		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pOutBuffer);
		RTMPSetTimer(pAuthTimer, Timeout);
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): %s sanity check fail\n", __func__, pSMName));
		return FALSE;
	}

	return TRUE;
}

VOID sta_mlme_deauth_req_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem)
{
	MLME_DISCONNECT_STRUCT *pInfo; /* snowpin for cntl mgmt */
	HEADER_802_11 DeauthHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	USHORT Status;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	ASSERT(pStaCfg);
	ASSERT(wdev);

	if (!pStaCfg)
		return;

	if (CliIdx == NON_REPT_ENTRY)
		STA_STATUS_SET_FLAG(pStaCfg, fSTA_STATUS_APCLI_MAIN_LINK_DOWN_IN_PROGRESS);


	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

	pInfo = (MLME_DISCONNECT_STRUCT *) Elem->Msg; /* snowpin for cntl mgmt */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer); /*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("AUTH - MlmeDeauthReqAction() allocate memory fail\n"));
		auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);
		Status = MLME_FAIL_NO_RESOURCE;
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_DEAUTH_CONF, Status, &Elem->priv_data);
		return;
	}

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("AUTH - Send DE-AUTH request (Reason=%d)...\n",
			  pInfo->reason));
	MgtMacHeaderInitExt(pAd, &DeauthHdr, SUBTYPE_DEAUTH, 0, pInfo->addr,
						pStaCfg->wdev.if_addr,
						pStaCfg->MlmeAux.Bssid);
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != NON_REPT_ENTRY)
		COPY_MAC_ADDR(DeauthHdr.Addr2, pAd->ApCfg.pRepeaterCliPool[CliIdx].CurrentAddress);

#endif /* MAC_REPEATER_SUPPORT */
	MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11),
					  &DeauthHdr, 2, &pInfo->reason, END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	pStaCfg->DeauthReason = pInfo->reason;
	COPY_MAC_ADDR(pStaCfg->DeauthSta, pInfo->addr);
	auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);
	Status = MLME_SUCCESS;
	cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_DEAUTH_CONF, Status, &Elem->priv_data);
	/* send wireless event - for deauthentication */
	RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, NULL, BSS0, 0);
	return;
}



VOID sta_mlme_auth_req_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;
	PRALINK_TIMER_STRUCT pAuthTimer = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY pReptEntry = Elem->priv_data.rept_cli_entry;
#endif /* MAC_REPEATER_SUPPORT */
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	ASSERT(pStaCfg);
	ASSERT(wdev);

	if (!pStaCfg)
		return;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != NON_REPT_ENTRY)
		pAuthTimer = &pReptEntry->ApCliAuthTimer;
	else
#endif /* MAC_REPEATER_SUPPORT */
		pAuthTimer = &pStaCfg->MlmeAux.AuthTimer;

	if (sta_send_auth_req(pAd, Elem, pAuthTimer, "AUTH", 1, NULL, 0))
		auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_WAIT_SEQ2, __func__);
	else {
		USHORT Status;

		auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);
		Status = MLME_INVALID_FORMAT;
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status, &Elem->priv_data);
	}

	return;
}

VOID sta_class2_error_action(struct wifi_dev *wdev, UCHAR *pAddr)
{
	HEADER_802_11 DeauthHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	USHORT Reason = REASON_CLS2ERR;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	/*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("AUTH - Class 2 error, Send DEAUTH frame...\n"));
	MgtMacHeaderInitExt(pAd, &DeauthHdr, SUBTYPE_DEAUTH, 0, pAddr,
						pStaCfg->wdev.if_addr,
						pStaCfg->MlmeAux.Bssid);
	MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11),
					  &DeauthHdr, 2, &Reason, END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	pStaCfg->DeauthReason = Reason;
	COPY_MAC_ADDR(pStaCfg->DeauthSta, pAddr);
}

static VOID sta_auth_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	ASSERT(wdev);

	if (!wdev)
		return;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AUTH - AuthTimeoutAction\n"));
	auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);
	Status = MLME_REJ_TIMEOUT;
	cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status, &Elem->priv_data);
}

VOID sta_auth_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PTIMER_FUNC_CONTEXT pContext = (PTIMER_FUNC_CONTEXT)FunctionContext;
	RTMP_ADAPTER *pAd = pContext->pAd;

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():AuthTimeout\n", __func__));
	MlmeEnqueueWithWdev(pAd, AUTH_FSM, AUTH_FSM_AUTH_TIMEOUT, 0, NULL, 0, pContext->wdev, NULL);
	RTMP_MLME_HANDLER(pAd);
}

DECLARE_TIMER_FUNCTION(sta_auth_timeout);
BUILD_TIMER_FUNCTION(sta_auth_timeout);

VOID sta_peer_deauth_action(
	IN PRTMP_ADAPTER pAd,
	IN PMLME_QUEUE_ELEM Elem)
{
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR Addr3[MAC_ADDR_LEN];
	USHORT Reason;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;
	UINT link_down_type = 0;

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	ASSERT(pStaCfg);
	ASSERT(wdev);

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


	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

	if (PeerDeauthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr1, Addr2, Addr3, &Reason)) {
		auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);

		   if (INFRA_ON(pStaCfg)
			&& MAC_ADDR_EQUAL(pStaCfg->Bssid, Addr2)) {
			/* struct wifi_dev *wdev = &pStaCfg->wdev; */
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("AUTH_RSP - receive DE-AUTH from our AP (Reason=%d)\n",
					  Reason));

			if (Reason == REASON_4_WAY_TIMEOUT)
				RTMPSendWirelessEvent(pAd,
									  IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG,
									  NULL, 0, 0);

			if (Reason == REASON_GROUP_KEY_HS_TIMEOUT)
				RTMPSendWirelessEvent(pAd,
									  IW_GROUP_HS_TIMEOUT_EVENT_FLAG,
									  NULL, 0, 0);

			/* send wireless event - for deauthentication */
			RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, NULL,
								  BSS0, 0);

#ifdef CONFIG_OWE_SUPPORT
			sta_reset_owe_parameters(pAd, ifIndex);
#endif
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
			do {
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
									("%s: Delete pmkid on de-auth\n", __func__));
					sta_delete_pmkid_cache(pAd, Addr2, ifIndex, CliIdx);
				}
#ifdef DOT11_SAE_SUPPORT
				pSaeIns = search_sae_instance(&pAd->SaeCfg, if_addr, Addr2);
				if (pSaeIns != NULL) {
					MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF,
							("%s: Delete Existing sae instance on de-auth\n", __func__));
					delete_sae_instance(pSaeIns);
				}
#endif
			} while (0);
#endif /* defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT) */
			cntl_fsm_state_transition(wdev, CliIdx, CNTL_WAIT_DEAUTH, __func__);

			if (cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_DEAUTH_CONF, Reason, &Elem->priv_data) == FALSE) {
				link_down_type |= LINK_REQ_FROM_AP;
				link_down_type |= LINK_HAVE_INTER_SM_DATA;
				LinkDown(pAd, link_down_type, wdev, Elem);
			}

		}

		else if (STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_APCLI_MAIN_LINK_DOWN_IN_PROGRESS)) {
			STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_APCLI_MAIN_LINK_DOWN_IN_PROGRESS);

		}
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("AUTH_RSP - PeerDeauthAction() sanity check fail\n"));
		if (STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_APCLI_MAIN_LINK_DOWN_IN_PROGRESS)) {
			STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_APCLI_MAIN_LINK_DOWN_IN_PROGRESS);
		}
	}
}


VOID sta_peer_auth_rsp_at_seq2_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Addr2[MAC_ADDR_LEN];
	USHORT Seq, Status, RemoteStatus, Alg;
	UCHAR iv_hdr[4];
	UCHAR *ChlgText = NULL;
	UCHAR *CyperChlgText = NULL;
	ULONG c_len = 0;
	HEADER_802_11 AuthHdr;
	BOOLEAN TimerCancelled;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	USHORT Status2;
	UCHAR ChallengeIe = IE_CHALLENGE_TEXT;
	UCHAR len_challengeText = CIPHER_TEXT_LEN;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = Elem->priv_data.rept_cli_entry;
#endif /* MAC_REPEATER_SUPPORT */
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;

	ASSERT(pStaCfg);
	ASSERT(wdev);
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	if (!pStaCfg)
		return;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

	os_alloc_mem(NULL, (UCHAR **) &ChlgText, CIPHER_TEXT_LEN);

	if (ChlgText == NULL) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: alloc mem fail\n", __func__));
		return;
	}

	os_alloc_mem(NULL, (UCHAR **) &CyperChlgText, CIPHER_TEXT_LEN + 8 + 8);

	if (CyperChlgText == NULL) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: CyperChlgText Allocate memory fail!!!\n",
				  __func__));
		os_free_mem(ChlgText);
		return;
	}

	if (PeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status, (PCHAR)ChlgText)) {
		if (MAC_ADDR_EQUAL(pStaCfg->MlmeAux.Bssid, Addr2) && Seq == 2) {
#ifdef MAC_REPEATER_SUPPORT

			if (CliIdx != 0xFF) {
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("AUTH - Repeater Cli Receive AUTH_RSP seq#2 to me (Alg=%d, Status=%d)\n", Alg, Status));
				RTMPCancelTimer(&pReptEntry->ApCliAuthTimer, &TimerCancelled);
			} else
#endif /* MAC_REPEATER_SUPPORT */
			{
				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("AUTH - Receive AUTH_RSP seq#2 to me (Alg=%d, Status=%d)\n",
						  Alg, Status));
				RTMPCancelTimer(&pStaCfg->MlmeAux.AuthTimer, &TimerCancelled);
			}

			if (Status == MLME_SUCCESS) {
				/* Authentication Mode "LEAP" has allow for CCX 1.X */
				if (pStaCfg->MlmeAux.Alg == Ndis802_11AuthModeOpen) {
					auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);
					cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status, &Elem->priv_data);
				} else {
					struct wifi_dev *wdev = Elem->wdev;
					PSEC_KEY_INFO  pKey;
					UINT default_key = wdev->SecConfig.PairwiseKeyId;

					pKey = &wdev->SecConfig.WepKey[default_key];
					/* 2. shared key, need to be challenged */
					Seq++;
					RemoteStatus = MLME_SUCCESS;
					/* Get an unused nonpaged memory */
					NStatus =
						MlmeAllocateMemory(pAd,
										   &pOutBuffer);

					if (NStatus != NDIS_STATUS_SUCCESS) {
						MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
								 ("AUTH - PeerAuthRspAtSeq2Action() allocate memory fail\n"));
						auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);
						Status2 = MLME_FAIL_NO_RESOURCE;
						cntl_auth_assoc_conf(wdev, CNTL_MLME_AUTH_CONF, Status2, &Elem->priv_data);
						goto LabelOK;
					}

					MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("AUTH - Send AUTH request seq#3...\n"));
					MgtMacHeaderInitExt(pAd, &AuthHdr,
										SUBTYPE_AUTH, 0, Addr2,
										pStaCfg->wdev.if_addr,
										pStaCfg->MlmeAux.Bssid);
					AuthHdr.FC.Wep = 1;
#ifdef MAC_REPEATER_SUPPORT
					if (CliIdx != 0xFF)
						COPY_MAC_ADDR(AuthHdr.Addr2, pReptEntry->CurrentAddress);
#endif /* MAC_REPEATER_SUPPORT */
					/* TSC increment */
					INC_TX_TSC(pKey->TxTsc, LEN_WEP_TSC);
					/* Construct the 4-bytes WEP IV header */
					RTMPConstructWEPIVHdr(default_key, pKey->TxTsc, iv_hdr);
					Alg = cpu2le16(*(USHORT *) &Alg);
					Seq = cpu2le16(*(USHORT *) &Seq);
					RemoteStatus = cpu2le16(*(USHORT *) &RemoteStatus);
					/* Construct message text */
					MakeOutgoingFrame(CyperChlgText, &c_len,
									  2, &Alg,
									  2, &Seq,
									  2, &RemoteStatus,
									  1, &ChallengeIe,
									  1, &len_challengeText,
									  len_challengeText,
									  ChlgText,
									  END_OF_ARGS);

					if (RTMPSoftEncryptWEP(iv_hdr,
										   pKey,
										   CyperChlgText, c_len) == FALSE) {
						MlmeFreeMemory(pOutBuffer);
						auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);
						Status2 = MLME_FAIL_NO_RESOURCE;
						cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status2, &Elem->priv_data);
						goto LabelOK;
					}

					/* Update the total length for 4-bytes ICV */
					c_len += LEN_ICV;
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
									  sizeof
									  (HEADER_802_11),
									  &AuthHdr,
									  LEN_WEP_IV_HDR,
									  iv_hdr, c_len,
									  CyperChlgText,
									  END_OF_ARGS);
					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
					MlmeFreeMemory(pOutBuffer);
#ifdef MAC_REPEATER_SUPPORT

					if (CliIdx != 0xFF)
						RTMPSetTimer(&pReptEntry->ApCliAuthTimer, AUTH_TIMEOUT);
					else
#endif /* MAC_REPEATER_SUPPORT */
						RTMPSetTimer(&pStaCfg->MlmeAux.AuthTimer, AUTH_TIMEOUT);

					auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_WAIT_SEQ4, __func__);
				}
			} else {
				pStaCfg->AuthFailReason = Status;
				COPY_MAC_ADDR(pStaCfg->AuthFailSta, Addr2);
				auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);
				cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status, &Elem->priv_data);
			}
		}
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("AUTH - PeerAuthSanity() sanity check fail\n"));
	}

LabelOK:

	if (ChlgText != NULL)
		os_free_mem(ChlgText);

	if (CyperChlgText != NULL)
		os_free_mem(CyperChlgText);

	return;
}


VOID sta_peer_auth_rsp_at_seq4_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Addr2[MAC_ADDR_LEN];
	USHORT Alg, Seq, Status;
	/*    CHAR          ChlgText[CIPHER_TEXT_LEN]; */
	CHAR *ChlgText = NULL;
	BOOLEAN TimerCancelled;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	ASSERT(pStaCfg);
	ASSERT(wdev);

	if (!pStaCfg)
		return;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **) &ChlgText, CIPHER_TEXT_LEN);

	if (ChlgText == NULL) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: ChlgText Allocate memory fail!!!\n",
				  __func__));
		return;
	}

	if (PeerAuthSanity
		(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status,
		 ChlgText)) {
		if (MAC_ADDR_EQUAL(pStaCfg->MlmeAux.Bssid, Addr2) && Seq == 4) {
#ifdef MAC_REPEATER_SUPPORT

			if (CliIdx != NON_REPT_ENTRY) {
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("AUTH - Repeater Cli Receive AUTH_RSP seq#4 to me\n"));
				RTMPCancelTimer(&pAd->ApCfg.pRepeaterCliPool[CliIdx].ApCliAuthTimer, &TimerCancelled);
			} else
#endif /* MAC_REPEATER_SUPPORT */
			{
				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("AUTH - Receive AUTH_RSP seq#4 to me\n"));
				RTMPCancelTimer(&pStaCfg->MlmeAux.AuthTimer,
								&TimerCancelled);
			}

			if (Status != MLME_SUCCESS) {
				pStaCfg->AuthFailReason = Status;
				COPY_MAC_ADDR(pStaCfg->AuthFailSta, Addr2);
				RTMPSendWirelessEvent(pAd, IW_SHARED_WEP_FAIL,
									  NULL, BSS0, 0);
			}

			auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);
			cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status, &Elem->priv_data);
		}
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("AUTH - PeerAuthRspAtSeq4Action() sanity check fail\n"));
	}

	if (ChlgText != NULL)
		os_free_mem(ChlgText);
}

#ifdef DOT11_SAE_SUPPORT
/*
    ==========================================================================
    Description:

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID sta_sae_auth_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	STA_ADMIN_CONFIG *pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	/* SAE_MLME_AUTH_REQ_STRUCT *AuthReq = (SAE_MLME_AUTH_REQ_STRUCT *)Elem->Msg; */
	MLME_AUTH_REQ_STRUCT *AuthReq = (MLME_AUTH_REQ_STRUCT *)Elem->Msg;
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;
	UCHAR if_addr[ETH_ALEN];
	UCHAR *pSae_cfg_group = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF, ("==>%s()\n", __func__));
	ASSERT(pStaCfg);
	ASSERT(wdev);

	if (!pStaCfg)
		return;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

	COPY_MAC_ADDR(pStaCfg->MlmeAux.Bssid, AuthReq->Addr);

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != NON_REPT_ENTRY) {
			pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
			pSae_cfg_group = &pReptEntry->sae_cfg_group;
			COPY_MAC_ADDR(if_addr, pReptEntry->CurrentAddress);
		} else
#endif /* MAC_REPEATER_SUPPORT */
		{
			pSae_cfg_group = &pStaCfg->sae_cfg_group;
			COPY_MAC_ADDR(if_addr, pStaCfg->wdev.if_addr);
		}

	if (sae_auth_init(pAd, &pAd->SaeCfg, if_addr, AuthReq->Addr,
					  pStaCfg->MlmeAux.Bssid, Elem->wdev->SecConfig.PSK, *pSae_cfg_group))
		auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_WAIT_SAE, __func__);
	else {
		USHORT Status;
		auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);
		Status = MLME_INVALID_FORMAT;
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status, &Elem->priv_data);
	}
}


/*
    ==========================================================================
    Description:

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID sta_sae_auth_rsp_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	STA_ADMIN_CONFIG *pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	FRAME_802_11 *Fr = (FRAME_802_11 *)Elem->Msg;
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;
	UCHAR CliIdx = Elem->priv_data.rept_cli_idx;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif
	USHORT seq;
	USHORT status;
	USHORT mlme_status;
	UCHAR *pmk;
	ASSERT(pStaCfg);
	ASSERT(wdev);

	if (!pStaCfg)
		return;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

	NdisMoveMemory(&seq,    &Fr->Octet[2], 2);
	NdisMoveMemory(&status, &Fr->Octet[4], 2);
	if (FALSE == sae_handle_auth(pAd, &pAd->SaeCfg, Elem->Msg, Elem->MsgLen,
						  Elem->wdev->SecConfig.PSK,
						  seq, status, &pmk)) {
		mlme_status = MLME_UNSPECIFY_FAIL;
		auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, mlme_status, &Elem->priv_data);
	} else if (pmk != NULL) {
		MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != NON_REPT_ENTRY) {
			pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
			pEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
		} else
#endif /* MAC_REPEATER_SUPPORT */
			pEntry = MacTableLookup(pAd, Fr->Hdr.Addr2);

		hex_dump_with_lvl("pmk:", (char *)pmk, LEN_PMK, DBG_LVL_TRACE);

		if (pEntry) {
			NdisMoveMemory(pEntry->SecConfig.PMK, pmk, LEN_PMK);
			mlme_status = MLME_SUCCESS;
			MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF, ("%s(): Security AKM = 0x%x, PairwiseCipher = 0x%x, GroupCipher = 0x%x\n",
					 __func__, pEntry->SecConfig.AKMMap, pEntry->SecConfig.PairwiseCipher, pEntry->SecConfig.GroupCipher));
		} else
			mlme_status = MLME_UNSPECIFY_FAIL;

		auth_fsm_state_transition(wdev, CliIdx, AUTH_FSM_IDLE, __func__);
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, mlme_status, &Elem->priv_data);
	} else {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE, ("**************Unhandled ************\n"));
	}
}
#endif /* DOT11_SAE_SUPPORT */


VOID sta_auth_init(struct wifi_dev *wdev)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	sta_auth_api.mlme_deauth_req_action		= sta_mlme_deauth_req_action;
	sta_auth_api.mlme_auth_req_action			= sta_mlme_auth_req_action;
	sta_auth_api.auth_timeout_action			= sta_auth_timeout_action;
	sta_auth_api.peer_deauth_action				= sta_peer_deauth_action;
	sta_auth_api.peer_auth_rsp_at_seq2_action	= sta_peer_auth_rsp_at_seq2_action;
	sta_auth_api.peer_auth_rsp_at_seq4_action	= sta_peer_auth_rsp_at_seq4_action;
#ifdef DOT11_SAE_SUPPORT
	sta_auth_api.sae_auth_req_action		= sta_sae_auth_req_action;
	sta_auth_api.sae_auth_rsp_action		= sta_sae_auth_rsp_action;
#endif
	wdev->auth_api = &sta_auth_api;
	wdev->auth_machine.CurrState = AUTH_FSM_IDLE;

	/* if Timer is not init yet, init it */
	if (!pStaCfg->MlmeAux.AuthTimer.Valid) {
		pStaCfg->MlmeAux.AuthTimerFuncContext.pAd = pAd;
		pStaCfg->MlmeAux.AuthTimerFuncContext.wdev = wdev;
		RTMPInitTimer(pAd, &pStaCfg->MlmeAux.AuthTimer,
					  GET_TIMER_FUNCTION(sta_auth_timeout), &pStaCfg->MlmeAux.AuthTimerFuncContext, FALSE);
	}
}


