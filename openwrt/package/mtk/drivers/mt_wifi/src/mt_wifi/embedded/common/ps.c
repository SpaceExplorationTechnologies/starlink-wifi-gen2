/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

    Abstract:

	All related POWER SAVE function body.

***************************************************************************/

#include "rt_config.h"


/*
	========================================================================
	Routine Description:
		This routine is used to do insert packet into power-saveing queue.

	Arguments:
		pAd: Pointer to our adapter
		pPacket: Pointer to send packet
		pMacEntry: portint to entry of MacTab. the pMacEntry store attribute of client (STA).
		QueIdx: Priority queue idex.

	Return Value:
		NDIS_STATUS_SUCCESS:If succes to queue the packet into TxSwQ.
		NDIS_STATUS_FAILURE: If failed to do en-queue.
========================================================================
*/
NDIS_STATUS RtmpInsertPsQueue(
	RTMP_ADAPTER * pAd,
	PNDIS_PACKET pPacket,
	MAC_TABLE_ENTRY *pMacEntry,
	UCHAR QueIdx)
{
	STA_TR_ENTRY *tr_entry;
	ULONG IrqFlags;
#ifdef UAPSD_SUPPORT
	/* put the U-APSD packet to its U-APSD queue by AC ID */
	UINT32 ac_id = QueIdx - QID_AC_BK; /* should be >= 0 */

	tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];

	if (UAPSD_MR_IS_UAPSD_AC(pMacEntry, ac_id)) {
		UAPSD_PacketEnqueue(pAd, pMacEntry, pPacket, ac_id, FALSE);
#ifdef RT_CFG80211_SUPPORT
#ifdef CFG_TDLS_SUPPORT
		cfg_tdls_send_PeerTrafficIndication(pAd, pMacEntry->Addr);
#endif /* CFG_TDLS_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */
	} else
#endif /* UAPSD_SUPPORT */
	{
		if (tr_entry->ps_queue.Number >= MAX_PACKETS_IN_PS_QUEUE) {
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		} else {
			MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("legacy ps> queue a packet!\n"));
			RTMP_IRQ_LOCK(&pAd->irq_lock /*&tr_entry->ps_queue_lock*/, IrqFlags);
			InsertTailQueue(&tr_entry->ps_queue, PACKET_TO_QUEUE_ENTRY(pPacket));
			RTMP_IRQ_UNLOCK(&pAd->irq_lock /*&tr_entry->ps_queue_lock*/, IrqFlags);
		}
	}

#ifdef CONFIG_AP_SUPPORT
	/* mark corresponding TIM bit in outgoing BEACON frame */
#ifdef UAPSD_SUPPORT

	if (UAPSD_MR_IS_NOT_TIM_BIT_NEEDED_HANDLED(pMacEntry, QueIdx)) {
		/* 1. the station is UAPSD station;
		2. one of AC is non-UAPSD (legacy) AC;
		3. the destinated AC of the packet is UAPSD AC. */
		/* So we can not set TIM bit due to one of AC is legacy AC */
	} else
#endif /* UAPSD_SUPPORT */
	{
		WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->func_tb_idx, pMacEntry->Aid);
	}

#endif /* CONFIG_AP_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}


/*
	==========================================================================
	Description:
		This routine is used to clean up a specified power-saving queue. It's
		used whenever a wireless client is deleted.
	==========================================================================
 */
VOID RtmpCleanupPsQueue(RTMP_ADAPTER *pAd, QUEUE_HEADER *pQueue)
{
	QUEUE_ENTRY *pQEntry;
	PNDIS_PACKET pPacket;

	MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RtmpCleanupPsQueue (0x%08lx)...\n", (ULONG)pQueue));

	while (pQueue->Head) {
		MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("RtmpCleanupPsQueue %d...\n", pQueue->Number));
		pQEntry = RemoveHeadQueue(pQueue);
		/*pPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReservedEx); */
		pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RtmpCleanupPsQueue pkt = %lx...\n", (ULONG)pPacket));
	}
}


/*
  ========================================================================
  Description:
	This routine frees all packets in PSQ that's destined to a specific DA.
	BCAST/MCAST in DTIMCount=0 case is also handled here, just like a PS-POLL
	is received from a WSTA which has MAC address FF:FF:FF:FF:FF:FF
  ========================================================================
*/
VOID RtmpHandleRxPsPoll(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT wcid, BOOLEAN isActive)
{
	MAC_TABLE_ENTRY *pMacEntry;
	STA_TR_ENTRY *tr_entry = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->APPSMode == APPS_MODE2)
		return;

	ASSERT(!IS_MT7615(pAd));
	ASSERT(VALID_UCAST_ENTRY_WCID(pAd, wcid));
	pMacEntry = &pAd->MacTab.Content[wcid];
	tr_entry = &pAd->MacTab.tr_entry[wcid];

	if (!RTMPEqualMemory(pMacEntry->Addr, pAddr, MAC_ADDR_LEN)) {
		MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(%d) PS-POLL (MAC addr not match) from %02x:%02x:%02x:%02x:%02x:%02x. Why???\n",
				 __func__, __LINE__, PRINT_MAC(pAddr)));
		return;
	}

#ifdef UAPSD_SUPPORT00

	if (UAPSD_MR_IS_ALL_AC_UAPSD(isActive, pMacEntry)) {
		/*
			IEEE802.11e spec.
			11.2.1.7 Receive operation for STAs in PS mode during the CP
			When a non-AP QSTA that is using U-APSD and has all ACs
			delivery-enabled detects that the bit corresponding to its AID
			is set in the TIM, the non-AP QSTA shall issue a trigger frame
			or a PS-Poll frame to retrieve the buffered MSDU or management
			frames.

			WMM Spec. v1.1a 070601
			3.6.2	U-APSD STA Operation
			3.6.2.3	In case one or more ACs are not
			delivery-enabled ACs, the WMM STA may retrieve MSDUs and
			MMPDUs belonging to those ACs by sending PS-Polls to the WMM AP.
			In case all ACs are delivery enabled ACs, WMM STA should only
			use trigger frames to retrieve MSDUs and MMPDUs belonging to
			those ACs, and it should not send PS-Poll frames.

			Different definitions in IEEE802.11e and WMM spec.
			But we follow the WiFi WMM Spec.
		*/
		MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("All AC are UAPSD, can not use PS-Poll\n"));
		return; /* all AC are U-APSD, can not use PS-Poll */
	}

#endif /* UAPSD_SUPPORT */
	/* Reset ContinueTxFailCnt */
	pMacEntry->ContinueTxFailCnt = 0;
	pAd->MacTab.tr_entry[pMacEntry->wcid].ContinueTxFailCnt = 0;

	if (isActive == FALSE) {
		if (tr_entry->PsDeQWaitCnt == 0)
			tr_entry->PsDeQWaitCnt = 1;
		else {
			MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s(): : packet not send by HW then ignore other PS-Poll Aid[%d]!\n",
					  __func__, pMacEntry->Aid));
			return;
		}
	} else
		tr_entry->PsDeQWaitCnt = 0;

#ifdef CONFIG_AP_SUPPORT
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		MtHandleRxPsPoll(pAd, pAddr, wcid, isActive);

#endif /* MT_MAC */
#endif /* CONFIG_AP_SUPPORT */
}


/*
	==========================================================================
	Description:
		Update the station current power save mode. Calling this routine also
		prove the specified client is still alive. Otherwise AP will age-out
		this client once IdleCount exceeds a threshold.
	==========================================================================
 */
BOOLEAN RtmpPsIndicate(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR wcid, UCHAR Psm)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->APPSMode == APPS_MODE2)
		return PWR_ACTIVE;

	ASSERT(!IS_MT7615(pAd));
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtPsIndicate(pAd, pAddr, wcid, Psm);
	else
#endif /* MT_MAC */
		return PWR_ACTIVE;
}


#ifdef CONFIG_STA_SUPPORT
/*
========================================================================
Routine Description:
    Check if PM of any packet is set.

Arguments:
	pAd		Pointer to our adapter

Return Value:
    TRUE	can set
	FALSE	can not set

Note:
========================================================================
*/
BOOLEAN RtmpPktPmBitCheck(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
	BOOLEAN FlgCanPmBitSet = TRUE;

	if (FlgCanPmBitSet == TRUE)
		return (pStaCfg->PwrMgmt.Psm == PWR_SAVE);

	return FALSE;
}


VOID RtmpPsActiveExtendCheck(RTMP_ADAPTER *pAd)
{
	/* count down the TDLS active counter */
}


VOID RtmpPsModeChange(RTMP_ADAPTER *pAd, UINT32 PsMode)
{
	MAC_TABLE_ENTRY *pEntry = GetAssociatedAPByWdev(pAd, &pAd->StaCfg[MAIN_MSTA_ID].wdev);

	if (pAd->StaCfg[0].BssType == BSS_INFRA) {
		/* reset ps mode */
		if (PsMode == Ndis802_11PowerModeMAX_PSP) {
			/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange() */
			/* to exclude certain situations. */
			/* MlmeSetPsm(pAd, PWR_SAVE); */
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

			if (pAd->StaCfg[0].bWindowsACCAMEnable == FALSE)
				pAd->StaCfg[0].WindowsPowerMode = Ndis802_11PowerModeMAX_PSP;

			pAd->StaCfg[0].WindowsBatteryPowerMode = Ndis802_11PowerModeMAX_PSP;
			pAd->StaCfg[0].DefaultListenCount = 5;
		} else if (PsMode == Ndis802_11PowerModeFast_PSP) {
			/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange() */
			/* to exclude certain situations. */
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

			if (pAd->StaCfg[0].bWindowsACCAMEnable == FALSE)
				pAd->StaCfg[0].WindowsPowerMode = Ndis802_11PowerModeFast_PSP;

			pAd->StaCfg[0].WindowsBatteryPowerMode = Ndis802_11PowerModeFast_PSP;
			pAd->StaCfg[0].DefaultListenCount = 3;
		} else if (PsMode == Ndis802_11PowerModeLegacy_PSP) {
			/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange() */
			/* to exclude certain situations. */
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

			if (pAd->StaCfg[0].bWindowsACCAMEnable == FALSE)
				pAd->StaCfg[0].WindowsPowerMode = Ndis802_11PowerModeLegacy_PSP;

			pAd->StaCfg[0].WindowsBatteryPowerMode = Ndis802_11PowerModeLegacy_PSP;
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			pAd->StaCfg[0].DefaultListenCount = 1;
#else
			pAd->StaCfg[0].DefaultListenCount = 3;
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) // */
		} else {
			/* Default Ndis802_11PowerModeCAM */
			/* clear PSM bit immediately */
			RTMP_SET_PSM_BIT(pAd, &pAd->StaCfg[0], PWR_ACTIVE);
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

			if (pAd->StaCfg[0].bWindowsACCAMEnable == FALSE)
				pAd->StaCfg[0].WindowsPowerMode = Ndis802_11PowerModeCAM;

			pAd->StaCfg[0].WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
		}

		/* change ps mode */
		RTMPSendNullFrame(pAd, pEntry, pAd->CommonCfg.TxRate, TRUE, FALSE);
		MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PSMode=%ld\n", pAd->StaCfg[0].WindowsPowerMode));
	}
}

VOID EnqueuePsPoll(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

	if (pStaCfg->WindowsPowerMode == Ndis802_11PowerModeLegacy_PSP)
		pAd->PsPollFrame.FC.PwrMgmt = PWR_SAVE;

	MiniportMMRequest(pAd, 0, (PUCHAR)&pAd->PsPollFrame, sizeof(PSPOLL_FRAME));
#ifdef STA_LP_PHASE_1_SUPPORT

	/* Keep Waking up */
	if (pStaCfg->CountDowntoPsm == 0)
		pStaCfg->CountDowntoPsm = STAY_2_SECONDS_AWAKE;

#else
#endif /* STA_LP_PHASE_1_SUPPORT */
}

#endif /* CONFIG_STA_SUPPORT */

