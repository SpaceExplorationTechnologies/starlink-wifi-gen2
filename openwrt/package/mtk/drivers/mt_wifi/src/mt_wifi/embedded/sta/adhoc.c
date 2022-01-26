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
#ifdef CONFIG_STA_ADHOC_SUPPORT
#include "rt_config.h"

#define ADHOC_ENTRY_BEACON_LOST_TIME    (2*OS_HZ)       /* 2 sec */ /* we re-add the ad-hoc peer into our mac table */
#define ADHOC_BEACON_LOST_TIME          (8*OS_HZ)       /* 8 sec */ /* we deauth the ad-hoc peer */

BOOLEAN adhoc_add_peer_from_beacon(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BCN_IE_LIST *bcn_ie_list,
								   NDIS_802_11_VARIABLE_IEs *pVIE, USHORT LenVIE)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	if (ADHOC_ON(pAd) && (CAP_IS_IBSS_ON(bcn_ie_list->CapabilityInfo))) {
		UCHAR MaxSupportedRateIn500Kbps = 0;
		UCHAR idx;
		MAC_TABLE_ENTRY *pEntry;
		ULONG Now;

		NdisGetSystemUpTime(&Now);
		MaxSupportedRateIn500Kbps = dot11_max_sup_rate(bcn_ie_list->SupRateLen, &bcn_ie_list->SupRate[0],
									bcn_ie_list->ExtRateLen, &bcn_ie_list->ExtRate[0]);
		/* look up the existing table */
		pEntry = MacTableLookup2(pAd, bcn_ie_list->Addr2, wdev);

		/*
		   Ad-hoc mode is using MAC address as BA session. So we need to continuously find newly joined adhoc station by receiving beacon.
		   To prevent always check this, we use wcid == RESERVED_WCID to recognize it as newly joined adhoc station.
		*/
		if ((ADHOC_ON(pAd) && ((!pEntry) || (pEntry && IS_ENTRY_NONE(pEntry)))) ||
			(pEntry && RTMP_TIME_AFTER(Now, pEntry->LastBeaconRxTime + ADHOC_ENTRY_BEACON_LOST_TIME))) {
			/* Another adhoc joining, add to our MAC table. */
			if (pEntry == NULL) {
				pEntry = MacTableInsertEntry(pAd, bcn_ie_list->Addr2, wdev, ENTRY_ADHOC, OPMODE_STA, FALSE);
#ifdef RT_CFG80211_SUPPORT
				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Another adhoc joining, add to our MAC table ==> %02x:%02x:%02x:%02x:%02x:%02x\n",
						 PRINT_MAC(bcn_ie_list->Addr2)));
				RT_CFG80211_JOIN_IBSS(pAd, pStaCfg->MlmeAux.Bssid);
				CFG80211OS_NewSta(pAd->net_dev, bcn_ie_list->Addr2, NULL, 0, FALSE);
#endif /* RT_CFG80211_SUPPORT */
			}

			if (pEntry == NULL)
				return FALSE;

			SET_ENTRY_CLIENT(pEntry);
			{
				BOOLEAN result;
				IE_LISTS *ielist = NULL;
#ifdef DOT11_VHT_AC
				os_alloc_mem(NULL, (UCHAR **)&ielist, sizeof(IE_LISTS));

				if (!ielist)
					return FALSE;

				NdisZeroMemory((UCHAR *)ielist, sizeof(IE_LISTS));

				if (bcn_ie_list->vht_cap_len && bcn_ie_list->vht_op_len) {
					NdisMoveMemory(&ielist->vht_cap, &bcn_ie_list->vht_cap_ie, sizeof(VHT_CAP_IE));
					NdisMoveMemory(&ielist->vht_op, &bcn_ie_list->vht_op_ie, sizeof(VHT_OP_IE));
					ielist->vht_cap_len = bcn_ie_list->vht_cap_len;
					ielist->vht_op_len = bcn_ie_list->vht_op_len;
				}

#endif /* DOT11_VHT_AC */
				result = StaUpdateMacTableEntry(pAd, pEntry, MaxSupportedRateIn500Kbps,
												&bcn_ie_list->HtCapability, bcn_ie_list->HtCapabilityLen,
												&bcn_ie_list->AddHtInfo, bcn_ie_list->AddHtInfoLen,
												ielist, bcn_ie_list->CapabilityInfo);

				if (ielist != NULL)
					os_free_mem(ielist);

				if (result == FALSE) {
					MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ADHOC - Add Entry failed.\n"));
					return FALSE;
				}

			}
			RTMPSetSupportMCS(pAd, OPMODE_STA, pEntry,
							  bcn_ie_list->SupRate, bcn_ie_list->SupRateLen,
							  bcn_ie_list->ExtRate, bcn_ie_list->ExtRateLen,
#ifdef DOT11_VHT_AC
							  bcn_ie_list->vht_cap_len, &bcn_ie_list->vht_cap_ie,
#endif /* DOT11_VHT_AC */
							  &bcn_ie_list->HtCapability, bcn_ie_list->HtCapabilityLen);
			pEntry->LastBeaconRxTime = 0;

			if (pEntry /*&& (Elem->Wcid == RESERVED_WCID)*/) {
				ASIC_SEC_INFO Info = {0};
				/* Set key material to Asic */
				os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
				Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
				Info.Direction = SEC_ASIC_KEY_BOTH;
				Info.Wcid = pEntry->wcid;
				Info.BssIndex = wdev->func_idx;
				Info.Cipher = wdev->SecConfig.GroupCipher;
				Info.KeyIdx = wdev->SecConfig.GroupKeyId;
				os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
				HW_ADDREMOVE_KEYTABLE(pAd, &Info);
				idx = wdev->SecConfig.GroupKeyId;
				/* HW_SET_WCID_SEC_INFO(pAd, BSS0, idx, */
				/* pEntry->SecConfig.GroupCipher, */
				/* pEntry->wcid, */
				/* SHAREDKEYTABLE); */
			}
		}

		if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
			pEntry->LastBeaconRxTime = Now;
		}

		/* At least another peer in this IBSS, declare MediaState as CONNECTED */
		if (!STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) {
			STA_STATUS_SET_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
			RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);
			pAd->ExtraInfo = GENERAL_LINK_UP;
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ADHOC  fSTA_STATUS_MEDIA_STATE_CONNECTED.\n"));
		}

	}

	return TRUE;
}

VOID Adhoc_checkPeerBeaconLost(RTMP_ADAPTER *pAd)
{
}

#endif /* CONFIG_STA_ADHOC_SUPPORT */
