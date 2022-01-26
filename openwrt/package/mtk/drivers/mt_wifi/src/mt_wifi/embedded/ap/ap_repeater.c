/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_repeater.c

    Abstract:
    Support MAC Repeater function.

    Revision History:
	Who		When              What
    --------------  ----------      ----------------------------------------------
    Arvin				11-16-2012      created
*/

#ifdef MAC_REPEATER_SUPPORT

#include "rt_config.h"

#define OUI_LEN	3
UCHAR VENDOR_DEFINED_OUI_ADDR[][OUI_LEN] =
	{
{0x02, 0x0C, 0x43},
{0x02, 0x0C, 0xE7},
{0x02, 0x0A, 0x00}
	};
static UCHAR  rept_vendor_def_oui_table_size = (sizeof(VENDOR_DEFINED_OUI_ADDR) / sizeof(UCHAR[OUI_LEN]));

/* IOCTL */
INT Show_ReptTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int CliIdx;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	RETURN_ZERO_IF_PAD_NULL(pAd);

	if (!pAd->ApCfg.bMACRepeaterEn)
		return TRUE;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
			("---------------------------------\n"));
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
			("--------pRepeaterCliPool --------\n"));
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
			("---------------------------------\n"));
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
			("\n%-3s%-4s%-5s%-4s%-4s%-5s%-6s%-5s%-5s%-5s%-5s%-5s%-19s%-19s%-19s%-19s\n",
			 "AP", "CLI", "WCID", "En", "Vld", "bEth", "Block", "Conn", "CTRL", "SYNC", "AUTH", "ASSO", "REAL_MAC", "FAKE_MAC", "MUAR_MAC", "MUAR_ROOT"));

	for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
		PREPEATER_CLIENT_ENTRY		pReptCliEntry;

		pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("%-3d", pReptCliEntry->MatchApCliIdx));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("%-4d", pReptCliEntry->MatchLinkIdx));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("%-5d", pReptCliEntry->MacTabWCID));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("%-4d", pReptCliEntry->CliEnable));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("%-4d", pReptCliEntry->CliValid));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("%-5d", pReptCliEntry->bEthCli));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("%-6d", pReptCliEntry->bBlockAssoc));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("%-5d", pReptCliEntry->CliConnectState));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("%-5lu", pReptCliEntry->CtrlCurrState));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("%-5lu", pReptCliEntry->SyncCurrState));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("%-5lu", pReptCliEntry->AuthCurrState));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("%-5lu", pReptCliEntry->AssocCurrState));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("%02X:%02X:%02X:%02X:%02X:%02X  ",
				 pReptCliEntry->OriginalAddress[0], pReptCliEntry->OriginalAddress[1], pReptCliEntry->OriginalAddress[2],
				 pReptCliEntry->OriginalAddress[3], pReptCliEntry->OriginalAddress[4], pReptCliEntry->OriginalAddress[5]));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("%02X:%02X:%02X:%02X:%02X:%02X  ",
				 pReptCliEntry->CurrentAddress[0], pReptCliEntry->CurrentAddress[1], pReptCliEntry->CurrentAddress[2],
				 pReptCliEntry->CurrentAddress[3], pReptCliEntry->CurrentAddress[4], pReptCliEntry->CurrentAddress[5]));
		/* read muar cr MAR0,MAR1 */
		{
			/* UINT32	mar_val; */
			RMAC_MAR0_STRUC mar0_val;
			RMAC_MAR1_STRUC mar1_val;

			memset(&mar0_val, 0x0, sizeof(mar0_val));
			memset(&mar1_val, 0x0, sizeof(mar1_val));
			mar1_val.field.access_start = 1;
			mar1_val.field.multicast_addr_index = pReptCliEntry->MatchLinkIdx*2;
			/* Issue a read command */
			HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR1, (UINT32)mar1_val.word);

			/* wait acess complete*/
			do {
				HW_IO_READ32(pAd->hdev_ctrl, RMAC_MAR1, (UINT32 *)&mar1_val);
				/* delay */
			} while (mar1_val.field.access_start == 1);

			HW_IO_READ32(pAd->hdev_ctrl, RMAC_MAR0, (UINT32 *)&mar0_val);
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
					("%02x:%02x:%02x:%02x:%02x:%02x  ",
					 (UINT8)(mar0_val.addr_31_0 & 0x000000ff),
					 (UINT8)((mar0_val.addr_31_0 & 0x0000ff00) >> 8),
					 (UINT8)((mar0_val.addr_31_0 & 0x00ff0000) >> 16),
					 (UINT8)((mar0_val.addr_31_0 & 0xff000000) >> 24),
					 (UINT8)mar1_val.field.addr_39_32,
					 (UINT8)mar1_val.field.addr_47_40
					));
			memset(&mar0_val, 0x0, sizeof(mar0_val));
			memset(&mar1_val, 0x0, sizeof(mar1_val));
			mar1_val.field.access_start = 1;
			mar1_val.field.multicast_addr_index = pReptCliEntry->MatchLinkIdx*2+1;
			/* Issue a read command */
			HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR1, (UINT32)mar1_val.word);

			/* wait acess complete*/
			do {
				HW_IO_READ32(pAd->hdev_ctrl, RMAC_MAR1, (UINT32 *)&mar1_val);
				/* delay */
			} while (mar1_val.field.access_start == 1);

			HW_IO_READ32(pAd->hdev_ctrl, RMAC_MAR0, (UINT32 *)&mar0_val);
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
					("%02x:%02x:%02x:%02x:%02x:%02x\n",
					 (UINT8)(mar0_val.addr_31_0 & 0x000000ff),
					 (UINT8)((mar0_val.addr_31_0 & 0x0000ff00) >> 8),
					 (UINT8)((mar0_val.addr_31_0 & 0x00ff0000) >> 16),
					 (UINT8)((mar0_val.addr_31_0 & 0xff000000) >> 24),
					 (UINT8)mar1_val.field.addr_39_32,
					 (UINT8)mar1_val.field.addr_47_40
					));
		}
	}

	return TRUE;
}

/* End of IOCTL */

VOID ApCliAuthTimeoutExt(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PREPEATER_CLIENT_ENTRY pRepeaterCliEntry = (PREPEATER_CLIENT_ENTRY)FunctionContext;
	PRTMP_ADAPTER pAd;
	struct wifi_dev *wdev = pRepeaterCliEntry->wdev;
	struct inter_machine_info priv_data;

	ASSERT(wdev);
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			 ("Repeater Cli AUTH - AuthTimeout\n"));
	pAd = pRepeaterCliEntry->pAd;
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
			 ("(%s) ifIndex = %d, CliIdx = %d !!!\n",
			  __func__,
			  pRepeaterCliEntry->MatchApCliIdx,
			  pRepeaterCliEntry->MatchLinkIdx));
	priv_data.rept_cli_entry = pRepeaterCliEntry;
	priv_data.rept_cli_idx = pRepeaterCliEntry->MatchLinkIdx;
	MlmeEnqueueWithWdev(pAd, AUTH_FSM, AUTH_FSM_AUTH_TIMEOUT, 0, NULL, 0, wdev, &priv_data);
	RTMP_MLME_HANDLER(pAd);
}

DECLARE_TIMER_FUNCTION(ApCliAuthTimeoutExt);
BUILD_TIMER_FUNCTION(ApCliAuthTimeoutExt);

/*
    ==========================================================================
    Description:
	Association timeout procedure. After association timeout, this function
	will be called and it will put a message into the MLME queue
    Parameters:
	Standard timer parameters
    ==========================================================================
 */
VOID ApCliAssocTimeoutExt(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PREPEATER_CLIENT_ENTRY pRepeaterCliEntry = (PREPEATER_CLIENT_ENTRY)FunctionContext;
	PRTMP_ADAPTER pAd;
	struct wifi_dev *wdev = pRepeaterCliEntry->wdev;
	struct inter_machine_info priv_data;

	ASSERT(wdev);

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Repeater Cli ASSOC - enqueue APCLI_MT2_ASSOC_TIMEOUT\n"));
	pAd = pRepeaterCliEntry->pAd;
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
			 ("(%s) ifIndex = %d, CliIdx = %d !!!\n",
			  __func__,
			  pRepeaterCliEntry->MatchApCliIdx,
			  pRepeaterCliEntry->MatchLinkIdx));
	priv_data.rept_cli_entry = pRepeaterCliEntry;
	priv_data.rept_cli_idx = pRepeaterCliEntry->MatchLinkIdx;
	MlmeEnqueueWithWdev(pAd, ASSOC_FSM, ASSOC_FSM_ASSOC_TIMEOUT, 0, NULL, 0, wdev, &priv_data);
	RTMP_MLME_HANDLER(pAd);
}


DECLARE_TIMER_FUNCTION(ApCliAssocTimeoutExt);
BUILD_TIMER_FUNCTION(ApCliAssocTimeoutExt);

static VOID ReptCompleteInit(REPEATER_CLIENT_ENTRY *pReptEntry)
{
	RTMP_OS_INIT_COMPLETION(&pReptEntry->free_ack);
}

static VOID ReptLinkDownComplete(REPEATER_CLIENT_ENTRY *pReptEntry)
{
	RTMP_OS_COMPLETE(&pReptEntry->free_ack);
}

VOID ReptWaitLinkDown(REPEATER_CLIENT_ENTRY *pReptEntry)
{
	if (pReptEntry->CliEnable && !RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pReptEntry->free_ack, REPT_WAIT_TIMEOUT)) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 ("(%s) ApCli Rept[%d] can't done.\n", __func__, pReptEntry->MatchLinkIdx));
	}
}



VOID CliLinkMapInit(RTMP_ADAPTER *pAd)
{
	UCHAR MbssIdx;
	MBSS_TO_CLI_LINK_MAP_T  *pMbssToCliLinkMap;
	struct wifi_dev *cli_link_wdev = &pAd->StaCfg[0].wdev;/* default bind to apcli0 */
	struct wifi_dev *mbss_link_wdev;
	int		apcli_idx;

	NdisAcquireSpinLock(&pAd->ApCfg.CliLinkMapLock);

	for (MbssIdx = 0; MbssIdx < HW_BEACON_MAX_NUM; MbssIdx++) {
		mbss_link_wdev = &pAd->ApCfg.MBSSID[MbssIdx].wdev;
		pMbssToCliLinkMap = &pAd->ApCfg.MbssToCliLinkMap[MbssIdx];

		if (pAd->CommonCfg.dbdc_mode == TRUE) {
			for (apcli_idx = 0; apcli_idx < pAd->ApCfg.ApCliNum; apcli_idx++) {
				cli_link_wdev = &pAd->StaCfg[apcli_idx].wdev;

				if (mbss_link_wdev->channel <= 14) { /* 2.4G */
					if (cli_link_wdev->channel <= 14) { /* 2.4G */
						pMbssToCliLinkMap->mbss_wdev = mbss_link_wdev;
						pMbssToCliLinkMap->cli_link_wdev = cli_link_wdev;
					}
				} else { /* 5G */
					if (cli_link_wdev->channel > 14) { /* 5G */
						pMbssToCliLinkMap->mbss_wdev = mbss_link_wdev;
						pMbssToCliLinkMap->cli_link_wdev = cli_link_wdev;
					}
				}
			}
		} else {
			pMbssToCliLinkMap->mbss_wdev = mbss_link_wdev;
			pMbssToCliLinkMap->cli_link_wdev = cli_link_wdev;
		}
	}

	NdisReleaseSpinLock(&pAd->ApCfg.CliLinkMapLock);
}

VOID RepeaterCtrlInit(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR MaxNumChipRept = GET_MAX_REPEATER_ENTRY_NUM(pChipCap);
	UINT32 Ret = FALSE;
	UCHAR i;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	UINT32 PoolMemSize;

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
				 ("%s, wrong State\n", __func__));
		return;
	}

	PoolMemSize = sizeof(REPEATER_CLIENT_ENTRY) * MaxNumChipRept;
	Ret = os_alloc_mem(NULL,
					   (UCHAR **)&pAd->ApCfg.pRepeaterCliPool,
					   PoolMemSize);

	if (Ret != NDIS_STATUS_SUCCESS) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 (" Alloc memory for pRepeaterCliPool failed.\n"));
		return;
	}

	os_zero_mem(pAd->ApCfg.pRepeaterCliPool, PoolMemSize);
	PoolMemSize = sizeof(REPEATER_CLIENT_ENTRY_MAP) * MaxNumChipRept;
	Ret = os_alloc_mem(NULL,
					   (UCHAR **)&pAd->ApCfg.pRepeaterCliMapPool,
					   PoolMemSize);

	if (Ret != NDIS_STATUS_SUCCESS) {
		if (pAd->ApCfg.pRepeaterCliPool)
			os_free_mem(pAd->ApCfg.pRepeaterCliPool);

		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 (" Alloc memory for pRepeaterCliMapPool failed.\n"));
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		return;
	}

	os_zero_mem(pAd->ApCfg.pRepeaterCliMapPool, PoolMemSize);

	/*initialize RepeaterEntryPool*/
	for (i = 0; i < MaxNumChipRept; i++) {
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[i];
		pReptEntry->CliConnectState = REPT_ENTRY_DISCONNT;
		pReptEntry->CliEnable = FALSE;
		pReptEntry->CliValid = FALSE;
		pReptEntry->bEthCli = FALSE;
		pReptEntry->pAd = pAd;
		pReptEntry->MatchApCliIdx = 0;
		pReptEntry->MatchLinkIdx = i;
		pReptEntry->CtrlCurrState = CNTL_IDLE;
		pReptEntry->AssocCurrState = ASSOC_IDLE;
		pReptEntry->bss_info_argument.ucBssIndex = 0xff;
		pReptEntry->AuthCurrState = AUTH_FSM_IDLE;
		ReptCompleteInit(pReptEntry);
		/* RTMPInitTimer(pAd, */
		/* &pReptEntry->ApCliAssocTimer, */
		/* GET_TIMER_FUNCTION(ApCliAssocTimeoutExt), */
		/* pReptEntry, FALSE); */
		/*  */
		/* RTMPInitTimer(pAd, &pReptEntry->ApCliAuthTimer, */
		/* GET_TIMER_FUNCTION(ApCliAuthTimeoutExt), pReptEntry, FALSE); */
	}

	pAd->ApCfg.RepeaterCliSize = 0;
	os_zero_mem(&pAd->ApCfg.ReptControl, sizeof(REPEATER_CTRL_STRUCT));
	pAd->ApCfg.bMACRepeaterEn = TRUE;
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
}

VOID RepeaterCtrlExit(RTMP_ADAPTER *pAd)
{
	/* TODO: check whole repeater control release. */
	int wait_cnt = 0;
	/*
		Add MacRepeater Entry De-Init Here, and let "iwpriv ra0 set MACRepeaterEn=0"
		can do this instead of "iwpriv apcli0 set ApCliEnable=0"
	*/
	UCHAR CliIdx;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pAd->ApCfg.bMACRepeaterEn) {
		for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
			pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

			/*disconnect the ReptEntry which is bind on the CliLink*/
			if (pReptEntry->CliEnable) {
				struct inter_machine_info priv_data;

				RTMP_OS_INIT_COMPLETION(&pReptEntry->free_ack);
				pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_IF_DOWN;

				priv_data.rept_cli_entry = pReptEntry;
				priv_data.rept_cli_idx = pReptEntry->MatchLinkIdx;
				cntl_disconnect_request(pReptEntry->wdev,
										CNTL_DISASSOC,
										pReptEntry->wdev->bssid,
										REASON_DISASSOC_STA_LEAVING,
										&priv_data);

				ReptWaitLinkDown(pReptEntry);
			}
		}
	}

	while (pAd->ApCfg.RepeaterCliSize > 0) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_OFF,
				 ("%s, wait entry to be deleted\n", __func__));
		OS_WAIT(10);
		wait_cnt++;

		if (wait_cnt > 1000)
			break;
	}

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	if (pAd->ApCfg.bMACRepeaterEn == FALSE) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
				 ("%s, wrong State\n", __func__));
		return;
	}

	pAd->ApCfg.bMACRepeaterEn = FALSE;

	if (pAd->ApCfg.pRepeaterCliMapPool != NULL) {
		os_free_mem(pAd->ApCfg.pRepeaterCliMapPool);
		pAd->ApCfg.pRepeaterCliMapPool = NULL;
	}

	if (pAd->ApCfg.pRepeaterCliPool != NULL) {
		os_free_mem(pAd->ApCfg.pRepeaterCliPool);
		pAd->ApCfg.pRepeaterCliPool = NULL;
	}

	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
}

REPEATER_CLIENT_ENTRY *RTMPLookupRepeaterCliEntry(
	IN PVOID pData,
	IN BOOLEAN bRealMAC,
	IN PUCHAR pAddr,
	IN BOOLEAN bIsPad)
{
	ULONG HashIdx;
	UCHAR tempMAC[6];
	REPEATER_CLIENT_ENTRY *pEntry = NULL;
	REPEATER_CLIENT_ENTRY_MAP *pMapEntry = NULL;

	COPY_MAC_ADDR(tempMAC, pAddr);
	HashIdx = MAC_ADDR_HASH_INDEX(tempMAC);

	/* NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock); */
	if (bIsPad == TRUE)
		NdisAcquireSpinLock(&((PRTMP_ADAPTER)pData)->ApCfg.ReptCliEntryLock);
	else
		NdisAcquireSpinLock(((REPEATER_ADAPTER_DATA_TABLE *)pData)->EntryLock);

	if (bRealMAC == TRUE) {
		if (bIsPad == TRUE)
			pMapEntry = ((PRTMP_ADAPTER)pData)->ApCfg.ReptMapHash[HashIdx];
		else
			pMapEntry = (REPEATER_CLIENT_ENTRY_MAP *)((((REPEATER_ADAPTER_DATA_TABLE *)pData)->MapHash)
						+ HashIdx);

		while (pMapEntry) {
			pEntry = pMapEntry->pReptCliEntry;

			if (pEntry) {
				if (pEntry->CliEnable && MAC_ADDR_EQUAL(pEntry->OriginalAddress, tempMAC))
					break;
				pEntry = NULL;
				pMapEntry = pMapEntry->pNext;
			} else
				pMapEntry = pMapEntry->pNext;
		}
	} else {
		if (bIsPad == TRUE)
			pEntry = ((PRTMP_ADAPTER)pData)->ApCfg.ReptCliHash[HashIdx];
		else
			pEntry = (REPEATER_CLIENT_ENTRY *)((((REPEATER_ADAPTER_DATA_TABLE *)pData)->CliHash) + HashIdx);

		while (pEntry) {
			if (pEntry->CliEnable && MAC_ADDR_EQUAL(pEntry->CurrentAddress, tempMAC))
				break;
			pEntry = pEntry->pNext;
		}
	}

	/* NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock); */
	if (bIsPad == TRUE)
		NdisReleaseSpinLock(&((PRTMP_ADAPTER)pData)->ApCfg.ReptCliEntryLock);
	else
		NdisReleaseSpinLock(((REPEATER_ADAPTER_DATA_TABLE *)pData)->EntryLock);

	return pEntry;
}

BOOLEAN RTMPQueryLookupRepeaterCliEntryMT(
	IN PVOID pData,
	IN PUCHAR pAddr,
	IN BOOLEAN bIsPad)
{
	REPEATER_CLIENT_ENTRY *pEntry = NULL;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			 ("%s:: %02x:%02x:%02x:%02x:%02x:%02x\n",
			  __func__,
			  pAddr[0],
			  pAddr[1],
			  pAddr[2],
			  pAddr[3],
			  pAddr[4],
			  pAddr[5]));
	pEntry = RTMPLookupRepeaterCliEntry(pData, FALSE, pAddr, bIsPad);

	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				 ("%s:: not the repeater client\n", __func__));
		return FALSE;
	}
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			("%s:: is the repeater client\n", __func__));
	return TRUE;
}

REPEATER_CLIENT_ENTRY *lookup_rept_entry(RTMP_ADAPTER *pAd, PUCHAR address)
{
	REPEATER_CLIENT_ENTRY *rept_entry = NULL;

	rept_entry = RTMPLookupRepeaterCliEntry(
					 pAd,
					 FALSE,
					 address,
					 TRUE);

	if (!rept_entry)
		rept_entry = RTMPLookupRepeaterCliEntry(
						 pAd,
						 TRUE,
						 address,
						 TRUE);

	if (rept_entry)
		return rept_entry;

	return NULL;
}

UINT32 ReptTxPktCheckHandler(
	RTMP_ADAPTER *pAd,
	IN struct wifi_dev *cli_link_wdev,
	IN PNDIS_PACKET pPacket,
	OUT UCHAR *pWcid)
{
	PUCHAR pSrcBufVA = NULL;
	STA_TR_ENTRY *tr_entry;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	STA_ADMIN_CONFIG *pApCliEntry = cli_link_wdev->func_dev;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev *mbss_wdev = NULL;
	MBSS_TO_CLI_LINK_MAP_T  *pMbssToCliLinkMap = NULL;
	UINT16 eth_type;

	pSrcBufVA = RTMP_GET_PKT_SRC_VA(pPacket);

	eth_type = (pSrcBufVA[12] << 8) | pSrcBufVA[13];


#ifdef VLAN_SUPPORT
	if (eth_type == ETH_TYPE_VLAN)
		eth_type = (pSrcBufVA[16] << 8) | pSrcBufVA[17];
#endif

	pReptEntry = RTMPLookupRepeaterCliEntry(
					 pAd,
					 TRUE,
					 (pSrcBufVA + MAC_ADDR_LEN),
					 TRUE);

	if (pReptEntry  && pReptEntry->CliValid) {
/*#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)*/

		if ((pReptEntry->MatchApCliIdx != pApCliEntry->wdev.func_idx) /*&&
			(wf_drv_tbl.wf_fwd_check_active_hook && wf_drv_tbl.wf_fwd_check_active_hook())*/) {
			UCHAR apCliIdx, CliIdx;
			struct inter_machine_info priv_data;

			apCliIdx = pReptEntry->MatchApCliIdx;
			CliIdx = pReptEntry->MatchLinkIdx;
			pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_CHANGE_APCLI_IF;

			priv_data.rept_cli_entry = pReptEntry;
			priv_data.rept_cli_idx = pReptEntry->MatchLinkIdx;
			cntl_disconnect_request(pReptEntry->wdev,
									CNTL_DISASSOC,
									pReptEntry->wdev->bssid,
									REASON_DISASSOC_STA_LEAVING,
									&priv_data);


			return INSERT_REPT_ENTRY;
		}

/*#endif CONFIG_WIFI_PKT_FWD */
		*pWcid = pReptEntry->MacTabWCID;
		return REPEATER_ENTRY_EXIST;
	}
	/* check SA valid. */
	if (RTMPRepeaterVaildMacEntry(pAd, pSrcBufVA + MAC_ADDR_LEN)) {
		tr_entry = &pAd->MacTab.tr_entry[pApCliEntry->MacTabWCID];

		if ((tr_entry) &&
				(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
			pMacEntry = MacTableLookup(pAd, (pSrcBufVA + MAC_ADDR_LEN));

			if (eth_type == ETH_TYPE_EAPOL)
				return INSERT_REPT_ENTRY_AND_ALLOW;

			if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry)) {
				STA_TR_ENTRY *sta_tr_entry;

				sta_tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];

				if ((sta_tr_entry) &&
						(sta_tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
							(" wireless client is not ready !!!\n"));
					return INSERT_REPT_ENTRY;
				}

				mbss_wdev = pMacEntry->wdev;
				pMbssToCliLinkMap = &pAd->ApCfg.MbssToCliLinkMap[mbss_wdev->func_idx];

				if (pMbssToCliLinkMap->cli_link_wdev->PortSecured != WPA_802_1X_PORT_SECURED
					|| pMbssToCliLinkMap->cli_link_wdev == cli_link_wdev) {
					HW_ADD_REPT_ENTRY(pAd, cli_link_wdev, (pSrcBufVA + MAC_ADDR_LEN));
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
							("pMacEntry connect to mbss idx:%d, use CliLink:%d to RootAP\n",
							 mbss_wdev->func_idx, cli_link_wdev->func_idx));
					return INSERT_REPT_ENTRY;
				}
			} else
				/*SA is not in mac table, pkt should from upper layer or eth.*/
			{
				/*
TODO: Carter, if more than one apcli/sta,
the eth pkt or upper layer pkt connecting rule should be refined.
*/
				HW_ADD_REPT_ENTRY(pAd, cli_link_wdev, (pSrcBufVA + MAC_ADDR_LEN));
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
						("pAddr %x %x %x %x %x %x: use CliLink:%d to RootAP\n",
						 PRINT_MAC((pSrcBufVA + MAC_ADDR_LEN)),
						 cli_link_wdev->func_idx));

				return INSERT_REPT_ENTRY;
			}
		}
	}

	return USE_CLI_LINK_INFO;
}

REPEATER_CLIENT_ENTRY *RTMPLookupRepeaterCliEntry_NoLock(
	IN PVOID pData,
	IN BOOLEAN bRealMAC,
	IN PUCHAR pAddr,
	IN BOOLEAN bIsPad)
{
	ULONG HashIdx;
	UCHAR tempMAC[6];
	REPEATER_CLIENT_ENTRY *pEntry = NULL;
	REPEATER_CLIENT_ENTRY_MAP *pMapEntry = NULL;

	COPY_MAC_ADDR(tempMAC, pAddr);
	HashIdx = MAC_ADDR_HASH_INDEX(tempMAC);

	/* NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock); */

	if (bRealMAC == TRUE) {
		if (bIsPad == TRUE)
			pMapEntry = ((PRTMP_ADAPTER)pData)->ApCfg.ReptMapHash[HashIdx];
		else
			pMapEntry = *((((REPEATER_ADAPTER_DATA_TABLE *)pData)->MapHash) + HashIdx);

		while (pMapEntry) {
			pEntry = pMapEntry->pReptCliEntry;

			if (pEntry) {
				if (pEntry->CliEnable && MAC_ADDR_EQUAL(pEntry->OriginalAddress, tempMAC))
					break;
				pEntry = NULL;
				pMapEntry = pMapEntry->pNext;
			} else
				pMapEntry = pMapEntry->pNext;
		}
	} else {
		if (bIsPad == TRUE)
			pEntry = ((PRTMP_ADAPTER)pData)->ApCfg.ReptCliHash[HashIdx];
		else
			pEntry = *((((REPEATER_ADAPTER_DATA_TABLE *)pData)->CliHash) + HashIdx);

		while (pEntry) {
			if (pEntry->CliEnable && MAC_ADDR_EQUAL(pEntry->CurrentAddress, tempMAC))
				break;
			pEntry = pEntry->pNext;
		}
	}

	return pEntry;
}

VOID RTMPInsertRepeaterEntry(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	PUCHAR pAddr)
{
	INT CliIdx, idx;
	UCHAR HashIdx;
	/* BOOLEAN Cancelled; */
	UCHAR tempMAC[MAC_ADDR_LEN];
	PREPEATER_CLIENT_ENTRY pReptCliEntry = NULL, pCurrEntry = NULL;
	INT pValid_ReptCliIdx;
	PREPEATER_CLIENT_ENTRY_MAP pReptCliMap;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct inter_machine_info priv_data;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif


	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			 ("%s.\n", __func__));
	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	if (pAd->ApCfg.RepeaterCliSize >= GET_MAX_REPEATER_ENTRY_NUM(cap)) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 (" Repeater Client Full !!!\n"));
		return;
	}

#ifdef CONFIG_STA_SUPPORT
	if (pStaCfg && STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_APCLI_MAIN_LINK_DOWN_IN_PROGRESS)) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
			(" ApCli main linkdown is in progress can't add Repeater entry !!!\n"));
		return;
	}
#endif
	pValid_ReptCliIdx = GET_MAX_REPEATER_ENTRY_NUM(cap);

	for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
		pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

		if ((pReptCliEntry->CliEnable) &&
			(MAC_ADDR_EQUAL(pReptCliEntry->OriginalAddress, pAddr) ||
			(pAd->ApCfg.MACRepeaterOuiMode != VENDOR_DEFINED_MAC_ADDR_OUI
			&& MAC_ADDR_EQUAL(pReptCliEntry->CurrentAddress, pAddr)))
		) {
			NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 ("\n  receive mac :%02x:%02x:%02x:%02x:%02x:%02x !!!\n",
					  PRINT_MAC(pAddr)));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 (" duplicate Insert !!!\n"));
			return;
		}

		if ((pReptCliEntry->CliEnable == FALSE) && (pValid_ReptCliIdx == GET_MAX_REPEATER_ENTRY_NUM(cap)))
			pValid_ReptCliIdx = CliIdx;
	}

	CliIdx = pValid_ReptCliIdx;

	if (CliIdx >= GET_MAX_REPEATER_ENTRY_NUM(cap)) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 ("Repeater Pool Full !!!\n"));
		return;
	}

	pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
	pReptCliMap = &pAd->ApCfg.pRepeaterCliMapPool[CliIdx];
	/* ENTRY PREEMPTION: initialize the entry */
	/* timer init */
	RTMPInitTimer(pAd,
				  &pReptCliEntry->ApCliAssocTimer,
				  GET_TIMER_FUNCTION(ApCliAssocTimeoutExt),
				  pReptCliEntry, FALSE);
	/* timer init */
	RTMPInitTimer(pAd, &pReptCliEntry->ApCliAuthTimer,
				  GET_TIMER_FUNCTION(ApCliAuthTimeoutExt), pReptCliEntry, FALSE);
	pReptCliEntry->CtrlCurrState = CNTL_IDLE;
	pReptCliEntry->AuthCurrState = AUTH_FSM_IDLE;
	pReptCliEntry->AssocCurrState = ASSOC_IDLE;
	pReptCliEntry->CliConnectState = REPT_ENTRY_DISCONNT;
	pReptCliEntry->LinkDownReason = APCLI_LINKDOWN_NONE;
	pReptCliEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_NONE;
	pReptCliEntry->CliValid = FALSE;
	pReptCliEntry->bEthCli = FALSE;
	pReptCliEntry->MacTabWCID = 0xFF;
#ifdef FAST_EAPOL_WAR

	if (pReptCliEntry->pre_entry_alloc == TRUE)
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Unexpected condition,check it (pReptCliEntry->pre_entry_alloc=%d)\n", __func__, pReptCliEntry->pre_entry_alloc));

	pReptCliEntry->pre_entry_alloc = FALSE;
#endif /* FAST_EAPOL_WAR */
	pReptCliEntry->AuthReqCnt = 0;
	pReptCliEntry->AssocReqCnt = 0;
	pReptCliEntry->CliTriggerTime = 0;
	pReptCliEntry->pNext = NULL;
	pReptCliEntry->wdev = wdev;
	pReptCliEntry->MatchApCliIdx = wdev->func_idx;
	pReptCliEntry->BandIdx = HcGetBandByWdev(wdev);
	pReptCliMap->pReptCliEntry = pReptCliEntry;
#ifdef DOT11_SAE_SUPPORT
	pReptCliEntry->sae_cfg_group = pAd->StaCfg[pReptCliEntry->MatchApCliIdx].sae_cfg_group;
#endif
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	NdisAllocateSpinLock(pAd, &pReptCliEntry->SavedPMK_lock);
#endif

	pReptCliMap->pNext = NULL;
	COPY_MAC_ADDR(pReptCliEntry->OriginalAddress, pAddr);
	COPY_MAC_ADDR(tempMAC, pAddr);

	if (pAd->ApCfg.MACRepeaterOuiMode == CASUALLY_DEFINE_MAC_ADDR) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 ("todo !!!\n"));
	} else if (pAd->ApCfg.MACRepeaterOuiMode == VENDOR_DEFINED_MAC_ADDR_OUI) {
		INT IdxToUse = 0, i;
		UCHAR flag = 0;
		UCHAR checkMAC[MAC_ADDR_LEN];

		COPY_MAC_ADDR(checkMAC, pAddr);

		for (idx = 0; idx < rept_vendor_def_oui_table_size; idx++) {
			if (RTMPEqualMemory(VENDOR_DEFINED_OUI_ADDR[idx], pAddr, OUI_LEN)) {
				if (idx < rept_vendor_def_oui_table_size - 1) {
					NdisCopyMemory(checkMAC,
						VENDOR_DEFINED_OUI_ADDR[idx+1], OUI_LEN);
					for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					if (MAC_ADDR_EQUAL(
						pAd->ApCfg.MBSSID[i].wdev.if_addr,
						checkMAC)) {
						flag = 1;
						break;
					}
					}
					if (i >= pAd->ApCfg.BssidNum) {
						IdxToUse = idx+1;
						break;
					}
				}
			} else if (flag == 1) {
				NdisCopyMemory(checkMAC, VENDOR_DEFINED_OUI_ADDR[idx], OUI_LEN);

				for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					if (MAC_ADDR_EQUAL(pAd->ApCfg.MBSSID[i].wdev.if_addr, checkMAC))
						break;
				}
				if (i >= pAd->ApCfg.BssidNum) {
					IdxToUse = idx;
					break;
				}
			}
		}
		NdisCopyMemory(tempMAC, VENDOR_DEFINED_OUI_ADDR[IdxToUse], OUI_LEN);
	} else
		NdisCopyMemory(tempMAC, wdev->if_addr, OUI_LEN);

	if (RTMPLookupRepeaterCliEntry_NoLock(pAd, FALSE, tempMAC, TRUE) != NULL) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("ReptCLI duplicate Insert %02x:%02x:%02x:%02x:%02x:%02x !\n",
				PRINT_MAC(tempMAC)));
		return;
	}

	COPY_MAC_ADDR(pReptCliEntry->CurrentAddress, tempMAC);
	pReptCliEntry->CliEnable = TRUE;
	pReptCliEntry->CliConnectState = REPT_ENTRY_CONNTING;
	pReptCliEntry->pNext = NULL;
	NdisGetSystemUpTime(&pReptCliEntry->CliTriggerTime);
	pAd->ApCfg.RepeaterCliSize++;
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	/* Before add into muar table, config Band binding. */
	HcAddRepeaterEntry(wdev, CliIdx);
	AsicInsertRepeaterEntry(pAd, CliIdx, tempMAC);
	HashIdx = MAC_ADDR_HASH_INDEX(tempMAC);

	if (pAd->ApCfg.ReptCliHash[HashIdx] == NULL)
		pAd->ApCfg.ReptCliHash[HashIdx] = pReptCliEntry;
	else {
		pCurrEntry = pAd->ApCfg.ReptCliHash[HashIdx];

		while (pCurrEntry->pNext != NULL)
			pCurrEntry = pCurrEntry->pNext;

		pCurrEntry->pNext = pReptCliEntry;
	}

	HashIdx = MAC_ADDR_HASH_INDEX(pReptCliEntry->OriginalAddress);

	if (pAd->ApCfg.ReptMapHash[HashIdx] == NULL)
		pAd->ApCfg.ReptMapHash[HashIdx] = pReptCliMap;
	else {
		PREPEATER_CLIENT_ENTRY_MAP pCurrMapEntry;

		pCurrMapEntry = pAd->ApCfg.ReptMapHash[HashIdx];

		while (pCurrMapEntry->pNext != NULL)
			pCurrMapEntry = pCurrMapEntry->pNext;

		pCurrMapEntry->pNext = pReptCliMap;
	}

	/* rept auth start */
	priv_data.rept_cli_entry = pReptCliEntry;
	priv_data.rept_cli_idx = CliIdx;
	cntl_join_start_conf(wdev, MLME_SUCCESS, &priv_data);
#ifdef MTFWD
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,  ("Insert MacRep Sta:%pM, orig MAC:%pM, %s\n",
		tempMAC, pReptCliEntry->OriginalAddress, wdev->if_dev->name));

	RtmpOSWrielessEventSend(pAd->net_dev,
				RT_WLAN_EVENT_CUSTOM,
				FWD_CMD_ADD_TX_SRC,
				NULL,
				tempMAC,
				MAC_ADDR_LEN);
#endif
}

VOID RTMPRemoveRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR func_tb_idx,
	IN UCHAR CliIdx)
{
	USHORT HashIdx;
	REPEATER_CLIENT_ENTRY *pEntry, *pPrevEntry, *pProbeEntry;
	REPEATER_CLIENT_ENTRY_MAP *pMapEntry, *pPrevMapEntry, *pProbeMapEntry;
	BOOLEAN bVaild = TRUE;
	BOOLEAN Cancelled;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, (" %s.CliIdx=%d\n", __func__, CliIdx));
	AsicRemoveRepeaterEntry(pAd, CliIdx);
	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	pEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

	/* move NULL check here, to prevent pEntry NULL dereference */
	if (pEntry == NULL) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 ("%s - pEntry is NULL !!!\n", __func__));
		return;
	}

	if (pEntry->CliEnable == FALSE) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 ("%s - CliIdx:%d Enable is FALSE already\n",
				  __func__, CliIdx));
		return;
	}

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	NdisFreeSpinLock(&pEntry->SavedPMK_lock);
#endif


	/*Release OMAC Idx*/
	HcDelRepeaterEntry(pEntry->wdev, CliIdx);
	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->CurrentAddress);
	pPrevEntry = NULL;
	pProbeEntry = pAd->ApCfg.ReptCliHash[HashIdx];
	ASSERT(pProbeEntry);

	if (pProbeEntry == NULL) {
		bVaild = FALSE;
		goto done;
	}

	if (pProbeEntry != NULL) {
		/* update Hash list*/
		do {
			if (pProbeEntry == pEntry) {
				if (pPrevEntry == NULL)
					pAd->ApCfg.ReptCliHash[HashIdx] = pEntry->pNext;
				else
					pPrevEntry->pNext = pEntry->pNext;

				break;
			}

			pPrevEntry = pProbeEntry;
			pProbeEntry = pProbeEntry->pNext;
		} while (pProbeEntry);
	}

	/* not found !!!*/
	ASSERT(pProbeEntry != NULL);

	if (pProbeEntry == NULL) {
		bVaild = FALSE;
		goto done;
	}

	pMapEntry = &pAd->ApCfg.pRepeaterCliMapPool[CliIdx];
	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->OriginalAddress);
	pPrevMapEntry = NULL;
	pProbeMapEntry = pAd->ApCfg.ReptMapHash[HashIdx];
	ASSERT(pProbeMapEntry);

	if (pProbeMapEntry != NULL) {
		/* update Hash list*/
		do {
			if (pProbeMapEntry == pMapEntry) {
				if (pPrevMapEntry == NULL)
					pAd->ApCfg.ReptMapHash[HashIdx] = pMapEntry->pNext;
				else
					pPrevMapEntry->pNext = pMapEntry->pNext;

				break;
			}

			pPrevMapEntry = pProbeMapEntry;
			pProbeMapEntry = pProbeMapEntry->pNext;
		} while (pProbeMapEntry);
	}

	/* not found !!!*/
	ASSERT(pProbeMapEntry != NULL);
done:
	RTMPReleaseTimer(&pEntry->ApCliAuthTimer, &Cancelled);
	RTMPReleaseTimer(&pEntry->ApCliAssocTimer, &Cancelled);
#ifdef FAST_EAPOL_WAR

	if (pEntry->pre_entry_alloc == TRUE)
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Unexpected condition,check it (pEntry->pre_entry_alloc=%d)\n", __func__, pEntry->pre_entry_alloc));

	pEntry->pre_entry_alloc = FALSE;
#endif /* FAST_EAPOL_WAR */
	pEntry->CliConnectState = REPT_ENTRY_DISCONNT;
	pEntry->CliValid = FALSE;
	pEntry->CliEnable = FALSE;

	if (bVaild == TRUE)
		pAd->ApCfg.RepeaterCliSize--;

	ReptLinkDownComplete(pEntry);
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
#ifdef MTFWD
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Remove MacRep Sta:%pM\n", pEntry->CurrentAddress));
	RtmpOSWrielessEventSend(pEntry->wdev->if_dev,
				RT_WLAN_EVENT_CUSTOM,
				FWD_CMD_DEL_TX_SRC,
				NULL,
				pEntry->CurrentAddress,
				MAC_ADDR_LEN);
#endif
}

VOID RTMPRepeaterReconnectionCheck(
	IN PRTMP_ADAPTER pAd)
{
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	INT i;
	PCHAR	pApCliSsid, pApCliCfgSsid;
	UCHAR	CfgSsidLen;
	NDIS_802_11_SSID Ssid;
	ULONG timeDiff[MAX_APCLI_NUM];
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl = NULL;

	if (pAd->ApCfg.bMACRepeaterEn &&
		pAd->ApCfg.MACRepeaterOuiMode == VENDOR_DEFINED_MAC_ADDR_OUI) {
		for (i = 0; i < MAX_APCLI_NUM; i++) {
			wdev = &pAd->StaCfg[i].wdev;

			if (!wdev->DevInfo.WdevActive)
				continue;

			ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

			if (!APCLI_IF_UP_CHECK(pAd, i) ||
					(pAd->StaCfg[i].ApcliInfStat.Enable == FALSE))
				continue;

			if (ScanCtrl->PartialScan.bScanning == TRUE)
				continue;

			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, (" %s(): i=%d,%d,%d,%d,%d,%d\n",
					 __func__, (int)i,
					 (int)scan_in_run_state(pAd, wdev),
					 (int)pAd->StaCfg[i].ApCliAutoConnectRunning,
					 (int)pAd->StaCfg[i].ApCliAutoConnectType,
					 (int)pAd->ApCfg.bPartialScanEnable[i],
					 (int)(pAd->Mlme.OneSecPeriodicRound%23)));

			if (scan_in_run_state(pAd, wdev))
				continue;
			if (pAd->StaCfg[i].ApCliAutoConnectRunning != FALSE)
				continue;
			if (pAd->StaCfg[i].ApcliInfStat.AutoConnectFlag == FALSE)
				continue;
			pApCliSsid = pAd->StaCfg[i].Ssid;
			pApCliCfgSsid = pAd->StaCfg[i].CfgSsid;
			CfgSsidLen = pAd->StaCfg[i].CfgSsidLen;

			if (((GetAssociatedAPByWdev(pAd, wdev) == NULL) ||
				 !NdisEqualMemory(pApCliSsid, pApCliCfgSsid, CfgSsidLen)) &&
					pAd->StaCfg[i].CfgSsidLen > 0) {
				if (RTMP_TIME_AFTER(pAd->Mlme.Now32, pAd->ApCfg.ApCliIssueScanTime[i]))
					timeDiff[i] = (pAd->Mlme.Now32 - pAd->ApCfg.ApCliIssueScanTime[i]);
				else
					timeDiff[i] = (pAd->ApCfg.ApCliIssueScanTime[i] - pAd->Mlme.Now32);
				/* will trigger scan after 23 sec */
				if (timeDiff[i] <= RTMPMsecsToJiffies(23000))
					continue;

				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
						(" %s(): Scan channels for AP (%s), pApEntry(%p)\n",
						 __func__, pApCliCfgSsid, GetAssociatedAPByWdev(pAd, wdev)));
				pAd->StaCfg[i].ApCliAutoConnectRunning = TRUE;
				if (pAd->ApCfg.bPartialScanEnable[i]) {
					pAd->ApCfg.bPartialScanning[i] = TRUE;
					ScanCtrl->PartialScan.pwdev = wdev;
					ScanCtrl->PartialScan.bScanning = TRUE;
				}
				Ssid.SsidLength = CfgSsidLen;
				NdisCopyMemory(Ssid.Ssid, pApCliCfgSsid, CfgSsidLen);
				NdisGetSystemUpTime(&pAd->ApCfg.ApCliIssueScanTime[i]);
				ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, &pAd->StaCfg[i].wdev);
			}
		}
	}

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
}

BOOLEAN RTMPRepeaterVaildMacEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pAd->ApCfg.RepeaterCliSize >= GET_MAX_REPEATER_ENTRY_NUM(cap))
		return FALSE;

	if (IS_MULTICAST_MAC_ADDR(pAddr))
		return FALSE;

	if (IS_BROADCAST_MAC_ADDR(pAddr))
		return FALSE;

	pEntry = RepeaterInvaildMacLookup(pAd, pAddr);

	if (pEntry)
		return FALSE;
	else
		return TRUE;
}

INVAILD_TRIGGER_MAC_ENTRY *RepeaterInvaildMacLookup(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	ULONG HashIdx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx];

	while (pEntry) {
		if (MAC_ADDR_EQUAL(pEntry->MacAddr, pAddr))
			break;
		pEntry = pEntry->pNext;
	}

	if (pEntry && pEntry->bInsert)
		return pEntry;
	else
		return NULL;
}

VOID InsertIgnoreAsRepeaterEntryTable(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	UCHAR HashIdx, idx = 0;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pCurrEntry = NULL;

	if (pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntrySize >= MAX_IGNORE_AS_REPEATER_ENTRY_NUM)
		return;

	if (MAC_ADDR_EQUAL(pAddr, ZERO_MAC_ADDR))
		return;

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	for (idx = 0; idx < MAX_IGNORE_AS_REPEATER_ENTRY_NUM; idx++) {
		pEntry = &pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntry[idx];

		if (MAC_ADDR_EQUAL(pEntry->MacAddr, pAddr)) {
			if (pEntry->bInsert) {
				NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
				return;
			}
		}

		/* pick up the first available vacancy*/
		if (pEntry->bInsert == FALSE) {
			NdisZeroMemory(pEntry->MacAddr, MAC_ADDR_LEN);
			COPY_MAC_ADDR(pEntry->MacAddr, pAddr);
			pEntry->entry_idx = idx;
			pEntry->bInsert = TRUE;
			break;
		}
	}

	/* add this entry into HASH table */
	if (pEntry) {
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		pEntry->pNext = NULL;

		if (pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx] == NULL)
			pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx] = pEntry;
		else {
			pCurrEntry = pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx];

			while (pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;

			pCurrEntry->pNext = pEntry;
		}
	}

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, (" Store Invaild MacAddr = %02x:%02x:%02x:%02x:%02x:%02x. !!!\n",
			 PRINT_MAC(pEntry->MacAddr)));
	pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntrySize++;
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
}

BOOLEAN RepeaterRemoveIngoreEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR idx,
	IN PUCHAR pAddr)
{
	USHORT HashIdx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pPrevEntry, *pProbeEntry;

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = &pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntry[idx];

	if (pEntry && pEntry->bInsert) {
		pPrevEntry = NULL;
		pProbeEntry = pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx];
		ASSERT(pProbeEntry);

		if (pProbeEntry != NULL) {
			/* update Hash list*/
			do {
				if (pProbeEntry == pEntry) {
					if (pPrevEntry == NULL)
						pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx] = pEntry->pNext;
					else
						pPrevEntry->pNext = pEntry->pNext;

					break;
				}

				pPrevEntry = pProbeEntry;
				pProbeEntry = pProbeEntry->pNext;
			} while (pProbeEntry);
		}

		/* not found !!!*/
		ASSERT(pProbeEntry != NULL);
		pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntrySize--;
	}

	NdisZeroMemory(pEntry->MacAddr, MAC_ADDR_LEN);
	pEntry->bInsert = FALSE;
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	return TRUE;
}

VOID RepeaterLinkMonitor(RTMP_ADAPTER *pAd)
{
	REPEATER_CLIENT_ENTRY *ReptPool = pAd->ApCfg.pRepeaterCliPool;
	REPEATER_CLIENT_ENTRY *pReptCliEntry = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR Wcid = 0;
	STA_TR_ENTRY *tr_entry = NULL;
	APCLI_CTRL_MSG_STRUCT msg;
	UCHAR CliIdx;
	UCHAR TimeoutVal = 5;
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	STA_ADMIN_CONFIG *pApCliEntry = NULL;
	TimeoutVal = 30;
#endif


	if ((pAd->ApCfg.bMACRepeaterEn) && (ReptPool != NULL)) {
		for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
			pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
			pApCliEntry = &pAd->StaCfg[pReptCliEntry->MatchApCliIdx];
#endif
			if (pReptCliEntry->CliEnable) {
				Wcid = pReptCliEntry->MacTabWCID;
				tr_entry = &pAd->MacTab.tr_entry[Wcid];

				if ((tr_entry->PortSecured != WPA_802_1X_PORT_SECURED) &&
					RTMP_TIME_AFTER(pAd->Mlme.Now32, (pReptCliEntry->CliTriggerTime + (TimeoutVal * OS_HZ)))) {
					if (pReptCliEntry->CtrlCurrState == APCLI_CTRL_DISCONNECTED)
						HW_REMOVE_REPT_ENTRY(pAd, pReptCliEntry->MatchApCliIdx, CliIdx);
					else {
						struct inter_machine_info priv_data;

						if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
							continue;



#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
						if (IS_AKM_SAE_SHA256(pApCliEntry->MlmeAux.AKMMap) || IS_AKM_OWE(pApCliEntry->MlmeAux.AKMMap)) {
							UCHAR pmkid[80];
							UCHAR pmk[LEN_PMK];
							INT CachedIdx;
							UCHAR ifIndex = pApCliEntry->wdev.func_idx;
							UCHAR CliIdx = pReptCliEntry->MatchLinkIdx;

							/*Update PMK cache and delete sae instance*/
							if (
#ifdef DOT11_SAE_SUPPORT

								(IS_AKM_SAE_SHA256(pApCliEntry->MlmeAux.AKMMap) &&
									sae_get_pmk_cache(&pAd->SaeCfg, pReptCliEntry->CurrentAddress, pApCliEntry->MlmeAux.Bssid, pmkid, pmk))
#endif
							) {

								CachedIdx = sta_search_pmkid_cache(pAd, pApCliEntry->MlmeAux.Bssid, ifIndex, CliIdx);

								if (CachedIdx != INVALID_PMKID_IDX) {
#ifdef DOT11_SAE_SUPPORT
									SAE_INSTANCE *pSaeIns = search_sae_instance(&pAd->SaeCfg, pReptCliEntry->CurrentAddress, pApCliEntry->MlmeAux.Bssid);

									MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
												("%s:Connection falied with pmkid ,delete cache entry and sae instance \n", __func__));
									if (pSaeIns != NULL) {
										delete_sae_instance(pSaeIns);
										pSaeIns = NULL;
									}
#endif
									sta_delete_pmkid_cache(pAd, pApCliEntry->MlmeAux.Bssid, ifIndex, CliIdx);
								}
							}
						}
#endif


						pReptCliEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_REPTLM_TRIGGER_TOO_LONG;
						NdisZeroMemory(&msg, sizeof(APCLI_CTRL_MSG_STRUCT));
						msg.BssIdx = pReptCliEntry->MatchApCliIdx;
						msg.CliIdx = CliIdx;

						priv_data.rept_cli_entry = pReptCliEntry;
						priv_data.rept_cli_idx = pReptCliEntry->MatchLinkIdx;
						cntl_disconnect_request(pReptCliEntry->wdev,
												CNTL_DEAUTH,
												pReptCliEntry->wdev->bssid,
												REASON_DEAUTH_STA_LEAVING,
												&priv_data);


					}
				}
			}
		}
	}
}

INT Show_Repeater_Cli_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	ULONG DataRate = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	ADD_HT_INFO_IE *addht;

	if (!wdev)
		return FALSE;

	addht = wlan_operate_get_addht(wdev);

	if (!pAd->ApCfg.bMACRepeaterEn)
		return TRUE;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			("\n"));
#ifdef DOT11_N_SUPPORT
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			("HT Operating Mode : %d\n", addht->AddHtInfo2.OperaionMode));
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			("\n"));
#endif /* DOT11_N_SUPPORT */
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			("\n%-19s%-4s%-4s%-4s%-4s%-8s%-7s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s%-7s%-7s\n",
			 "MAC", "AID", "BSS", "PSM", "WMM", "MIMOPS", "RSSI0", "RSSI1",
			 "RSSI2", "RSSI3", "PhMd", "BW", "MCS", "SGI", "STBC", "Idle", "Rate"));

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (pEntry &&
			(IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (pEntry->Sst == SST_ASSOC) && (pEntry->bReptCli)) {
			DataRate = 0;
			getRate(pEntry->HTPhyMode, &DataRate);
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%02X:%02X:%02X:%02X:%02X:%02X  ",
					 pEntry->ReptCliAddr[0], pEntry->ReptCliAddr[1], pEntry->ReptCliAddr[2],
					 pEntry->ReptCliAddr[3], pEntry->ReptCliAddr[4], pEntry->ReptCliAddr[5]));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-4d", (int)pEntry->Aid));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-4d-%d", (int)pEntry->apidx, pEntry->func_tb_idx));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-4d", (int)pEntry->PsMode));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)));
#ifdef DOT11_N_SUPPORT
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-8d", (int)pEntry->MmpsMode));
#endif /* DOT11_N_SUPPORT */
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-7d", pEntry->RssiSample.AvgRssi[0]));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-7d", pEntry->RssiSample.AvgRssi[1]));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-7d", pEntry->RssiSample.AvgRssi[2]));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-7d", pEntry->RssiSample.AvgRssi[3]));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE)));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-6s", get_bw_str(pEntry->HTPhyMode.field.BW)));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-6d", pEntry->HTPhyMode.field.MCS));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-6d", pEntry->HTPhyMode.field.ShortGI));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-6d", pEntry->HTPhyMode.field.STBC));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-7d", (int)(pEntry->StaIdleTimeout - pEntry->NoDataIdleCount)));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-7d", (int)DataRate));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount,
					 (pEntry->DebugTxCount) ? ((pEntry->DebugTxCount-pEntry->DebugFIFOCount)*100/pEntry->DebugTxCount) : 0));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("\n"));
		}
	}

	return TRUE;
}

VOID UpdateMbssCliLinkMap(
	RTMP_ADAPTER *pAd,
	UCHAR MbssIdx,
	struct wifi_dev *cli_link_wdev,
	struct wifi_dev *mbss_link_wdev)
{
	MBSS_TO_CLI_LINK_MAP_T  *pMbssToCliLinkMap = NULL;

	NdisAcquireSpinLock(&pAd->ApCfg.CliLinkMapLock);
	pMbssToCliLinkMap = &pAd->ApCfg.MbssToCliLinkMap[MbssIdx];
	pMbssToCliLinkMap->mbss_wdev = mbss_link_wdev;
	pMbssToCliLinkMap->cli_link_wdev = cli_link_wdev;
	NdisReleaseSpinLock(&pAd->ApCfg.CliLinkMapLock);
}

#ifdef REPEATER_TX_RX_STATISTIC
BOOLEAN MtRepeaterGetTxRxInfo(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR CliIfIndex, RETRTXRXINFO *pReptStatis)
{
	RETRTXRXINFO RptrTRInfo[MAX_APCLI_NUM];
	UCHAR Index, IfIndex;
	MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	if ((wcid != 0xff) && (!VALID_TR_WCID(wcid))) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Invalid wcid!\n", __func__));
		return FALSE;
	}
	if ((CliIfIndex > MAX_APCLI_NUM) && (CliIfIndex != 0xf)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Invalid Client Index!\n", __func__));
		return FALSE;
	}
	if (pReptStatis == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:NULL Pointer!\n", __func__));
		return FALSE;
	}
	NdisZeroMemory(RptrTRInfo, sizeof(RptrTRInfo));
	for (Index = 1; VALID_TR_WCID(Index); Index++) {
		if ((wcid != 0xff) && (wcid != Index))
			continue;
		pEntry = &pAd->MacTab.Content[Index];
		if (!IS_VALID_ENTRY(pEntry))
			continue;
		if ((CliIfIndex != 0xf) && (pEntry->func_tb_idx != CliIfIndex))
			continue;
		if (IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry)) {
			RptrTRInfo[pEntry->func_tb_idx].RxBytes += pEntry->RxBytes;
			RptrTRInfo[pEntry->func_tb_idx].TxBytes += pEntry->TxBytes;
			RptrTRInfo[pEntry->func_tb_idx].RxPackets += pEntry->RxPackets.QuadPart;
			RptrTRInfo[pEntry->func_tb_idx].TxPackets += pEntry->TxPackets.QuadPart;
			if (pEntry->pApCli) {
				STA_ADMIN_CONFIG *pAPCliEntry;
				pAPCliEntry = (STA_ADMIN_CONFIG *)(pEntry->pApCli);
				RptrTRInfo[pEntry->func_tb_idx].TxDropPackets = pAPCliEntry->TxDropCount;
			}
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
			if (cap->fgRateAdaptFWOffload == TRUE) {
				EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
				MtCmdGetTxStatistic(pAd, GET_TX_STAT_TOTAL_TX_CNT, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("%s(%d):wcid(%d)failfw(%u),totalfw(%u)-If(%d)\"%s\".\n", __func__, __LINE__,
				pEntry->wcid, rTxStatResult.u4TotalTxFailCount, rTxStatResult.u4TotalTxCount, pEntry->func_tb_idx,
				(IS_ENTRY_APCLI(pEntry)?"APCLI":(IS_ENTRY_REPEATER(pEntry)?"REPTR":"INVADER"))));
				pEntry->TxFailCount += rTxStatResult.u4TotalTxFailCount;
			} else
#endif
			{
				MT_TX_COUNTER TxInfo;
				MtAsicTxCntUpdate(pAd, pEntry->wcid, &TxInfo);
				pEntry->TxFailCount += TxInfo.TxFailCount;
			}
			RptrTRInfo[pEntry->func_tb_idx].TxFailPackets += pEntry->TxFailCount;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("%s(%d):IfIdx(%d),wcid(%d)TXInfo:RxBytes:%llu,RxPackets:%llu,TxBytes:%llu,TxDropPackets:%llu,TxFailPackets:%llu,TxPackets:%llu\n",
				__func__, __LINE__, pEntry->func_tb_idx, pEntry->wcid, RptrTRInfo[pEntry->func_tb_idx].RxBytes,
				RptrTRInfo[pEntry->func_tb_idx].RxPackets, RptrTRInfo[pEntry->func_tb_idx].TxBytes,
				RptrTRInfo[pEntry->func_tb_idx].TxDropPackets, RptrTRInfo[pEntry->func_tb_idx].TxFailPackets,
				RptrTRInfo[pEntry->func_tb_idx].TxPackets));
			} else {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d):It's not Repeater Entry!\n", __func__, __LINE__));
		}
	}
	if ((wcid != 0xff)) {
		CliIfIndex = pEntry->func_tb_idx;
	}
	for (IfIndex = 0; IfIndex < MAX_APCLI_NUM; IfIndex++) {
		if ((CliIfIndex != 0xf) && (IfIndex != CliIfIndex))
			continue;
		pReptStatis->RxBytes += RptrTRInfo[IfIndex].RxBytes;
		pReptStatis->RxPackets += RptrTRInfo[IfIndex].RxPackets;
		pReptStatis->TxBytes += RptrTRInfo[IfIndex].TxBytes;
		pReptStatis->TxPackets += RptrTRInfo[IfIndex].TxPackets;
		pReptStatis->TxFailPackets += RptrTRInfo[IfIndex].TxFailPackets;
		pReptStatis->TxDropPackets += RptrTRInfo[IfIndex].TxDropPackets;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	("%s(%d):RxBytes:%llu,RxPackets:%llu,TxBytes:%llu,TxPackets:%llu,TxFailPackets:%llu,TxDropPackets:%llu\n",
		__func__, __LINE__, pReptStatis->RxBytes, pReptStatis->RxPackets, pReptStatis->TxBytes,
		pReptStatis->TxPackets, pReptStatis->TxFailPackets, pReptStatis->TxDropPackets));
	return TRUE;
}
#endif /* REPEATER_TX_RX_STATISTIC */
#endif /* MAC_REPEATER_SUPPORT */

