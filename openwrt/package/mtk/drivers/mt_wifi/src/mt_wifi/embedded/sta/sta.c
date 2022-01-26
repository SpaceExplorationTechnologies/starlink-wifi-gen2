/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	sta.c

	Abstract:
	initialization for STA module

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"
#include "hdev/hdev.h"


NET_DEV_STATS *RT28xx_get_ether_stats(PNET_DEV net_dev);
#ifdef APCLI_CONNECTION_TRIAL
extern UINT GetBandByChannel(UCHAR ch, RTMP_ADAPTER *pAd);
#endif


VOID STARxErrorHandle(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
}

INT StaAllowToSendPacket_new(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pPacket,
	IN UCHAR * pWcid)
{
	MAC_TABLE_ENTRY *pEntry;
	UCHAR *pSrcBufVA;

	/* Drop send request since we are in monitor mode */
	/* TODO: shiang-usw, integrate this check to wdev->allow_data_tx = FALSE! */
	if (MONITOR_ON(pAd))
		return FALSE;

	pSrcBufVA = RTMP_GET_PKT_SRC_VA(pPacket);

	if (MAC_ADDR_IS_GROUP(pSrcBufVA)) {
		*pWcid = wdev->tr_tb_idx;
		return TRUE;
	}

	pEntry = MacTableLookup2(pAd, pSrcBufVA, wdev);

	if (pEntry)
		ASSERT((pEntry->wdev == wdev));

	if (pEntry && (pEntry->Sst == SST_ASSOC)) {
		*pWcid = (UCHAR)pEntry->wcid;
		return TRUE;
	}

	return FALSE;
}


INT sta_func_init(RTMP_ADAPTER *pAd)
{
	return TRUE;
}

/* Initialize STA and the MAIN STA interface. */
INT STAInitialize(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	ASSERT(pStaCfg);

	if (!pStaCfg)
		return 0;

	pStaCfg->OriDevType = RTMP_OS_NETDEV_GET_TYPE(pAd->net_dev);
#ifdef CREDENTIAL_STORE
	NdisAllocateSpinLock(pAd, &pAd->StaCtIf.Lock);
#endif /* CREDENTIAL_STORE */
	pAd->MSTANum = 1;

	if (pAd->MaxMSTANum == 0) /* This mean profile has no setting for MaxMSTANum */
		pAd->MaxMSTANum = 1;

	return 0;
}



VOID rtmp_sta_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	STAInitialize(pAd, wdev);
#ifdef CREDENTIAL_STORE
	RecoverConnectInfo(pAd);
#endif /* CREDENTIAL_STORE */
	/* Set up the Mac address*//* Here is to set MAIN_MSTA_ID STA, Extend MSTA is set in MSTA_Open */
	NdisMoveMemory(&pStaCfg->wdev.if_addr[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
	RtmpOSNetDevAddrSet(pAd->OpMode, pAd->net_dev, &pStaCfg->wdev.if_addr[0], (PUCHAR)(pStaCfg->dev_name));

	if (wdev_do_open(wdev) != TRUE)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() open fail!!!\n", __func__));

	BuildChannelList(pAd, wdev);
	RTMPSetPhyMode(pAd, &pStaCfg->wdev, pStaCfg->wdev.PhyMode);
	sta_os_completion_initialize(pStaCfg);
	sta_func_init(pAd);
}


VOID RTMPStaCfgRadioCtrlFromEEPROM(RTMP_ADAPTER *pAd, EEPROM_NIC_CONFIG2_STRUC NicConfig2)
{
#ifdef RTMP_MAC_PCI

	/* Read Hardware controlled Radio state enable bit*/
	if (NicConfig2.field.HardwareRadioControl == 1) {
	} else
#endif /* RTMP_MAC_PCI */
	{
	}

#ifdef RTMP_MAC_PCI
#endif /* RTMP_MAC_PCI */
}

VOID RT28xx_MSTA_Init(VOID *pAd, PNET_DEV main_dev_p)
{
	RTMP_OS_NETDEV_OP_HOOK netDevHook;

	NdisZeroMemory(&netDevHook, sizeof(netDevHook));
	netDevHook.open = msta_virtual_if_open;  /* device opem hook point */
	netDevHook.stop = msta_virtual_if_close; /* device close hook point */
	netDevHook.xmit = rt28xx_send_packets;  /* hard transmit hook point */
	netDevHook.ioctl = rt28xx_ioctl;    /* ioctl hook point */
	netDevHook.get_stats = RT28xx_get_ether_stats;
	RTMP_STA_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_MSTA_INIT,
						 0, &netDevHook, 0, 0);
}

VOID RT28xx_MSTA_Remove(VOID *pAd)
{
	RTMP_STA_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_MSTA_REMOVE, 0, NULL, 0, 0);
}

INT msta_virtual_if_open(PNET_DEV pDev)
{
	VOID *pAd;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(pDev), __func__));

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);

	if (VIRTUAL_IF_INIT(pAd, pDev) != 0)
		return -1;

	if (VIRTUAL_IF_UP(pAd, pDev) != 0)
		return -1;

	/* increase MODULE use count */
	RT_MOD_INC_USE_COUNT();
	RT_MOD_HNAT_REG(pDev);
	RTMP_OS_NETDEV_START_QUEUE(pDev);
#ifdef MTFWD
	RTMP_OS_NETDEV_CARRIER_OFF(pDev);
#endif

	return 0;
}

INT msta_virtual_if_close(PNET_DEV pDev)
{
	VOID *pAd;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(pDev), __func__));

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	RTMP_OS_NETDEV_STOP_QUEUE(pDev);

	VIRTUAL_IF_DOWN(pAd, pDev);

	VIRTUAL_IF_DEINIT(pAd, pDev);

	RT_MOD_HNAT_DEREG(pDev);
	RT_MOD_DEC_USE_COUNT();
	return 0;
}

INT32 MSTA_IdxGet(RTMP_ADAPTER *pAd, PNET_DEV pDev)
{
	INT32 ret = -1;
	INT32 i;

	if (!pAd || !pDev)
		return -1;

	for (i = 0; i < pAd->MSTANum; i++) {
		if (pAd->StaCfg[i].wdev.if_dev == pDev) {
			ret = i;
			break;
		}
	}

	return ret;
}

extern struct wifi_dev_ops sta_wdev_ops;

VOID MSTA_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{
	UINT32 idx = 0;
	UINT32 sta_start_id = (MAIN_MSTA_ID + 1);
	UINT32 max_num_sta = pAd->MaxMSTANum;
	UINT32 inf_type = INT_MSTA;
	struct wifi_dev_ops *wdev_ops = &sta_wdev_ops;
	PNET_DEV pDevNew;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	INT status;
	struct wifi_dev *wdev;
	UINT32 MC_RowID = 0, IoctlIF = 0;
	char *dev_name;
	INT32 Ret;
#ifdef MULTIPLE_CARD_SUPPORT
	MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef CONFIG_APSTA_MIXED_SUPPORT

	if (IF_COMBO_HAVE_AP_STA(pAd)) {
		sta_start_id = 0;
		max_num_sta = min(pAd->ApCfg.ApCliNum, (UCHAR)MAX_APCLI_NUM);
		inf_type = INT_APCLI;
		wdev_ops = &apcli_wdev_ops;
		if (pAd->flg_msta_init != FALSE) {
			for (idx = 0; idx < pAd->ApCfg.ApCliNum; idx++) {
				pStaCfg = &pAd->StaCfg[idx];
				wdev = &pStaCfg->wdev;
				wlan_config_set_ext_cha(wdev, EXTCHA_NOASSIGN);
				apcli_sync_wdev(pAd, wdev);
			}
			return;
		}
	} else
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
	{
		if (pAd->flg_msta_init != FALSE)
			return;
	}
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s (%d) ---> %s\n",
			 __func__, max_num_sta, IF_COMBO_HAVE_AP_STA(pAd) ? "ApCli" : "STA"));
	/* create virtual network interface */
	for (idx = sta_start_id; idx < max_num_sta; idx++) {
#ifdef CONFIG_APSTA_MIXED_SUPPORT
#ifdef MULTI_PROFILE
		UCHAR final_name[32] = "";
#endif
#endif /*CONFIG_APSTA_MIXED_SUPPORT*/

		pStaCfg = &pAd->StaCfg[idx];
		wdev = &pStaCfg->wdev;
		dev_name = get_dev_name_prefix(pAd, inf_type);

#ifdef CONFIG_APSTA_MIXED_SUPPORT

	if (IF_COMBO_HAVE_AP_STA(pAd)) {

#ifdef MULTI_PROFILE

		if (dev_name == NULL) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
					 ("%s(): apcli interface name is null,apcli idx=%d!\n",
					  __func__, idx));
			break;
		}

		snprintf(final_name, sizeof(final_name), "%s", dev_name);
		multi_profile_apcli_devname_req(pAd, final_name, &idx);

		if (pAd->CommonCfg.dbdc_mode == TRUE) {
			/* MULTI_PROFILE enable, apcli interface name will be apcli0,apclix0*/
			pDevNew = RtmpOSNetDevCreate(pAd, MC_RowID, &IoctlIF, inf_type, 0,
										   sizeof(struct mt_dev_priv), final_name, TRUE);
		} else {
			pDevNew = RtmpOSNetDevCreate(pAd, MC_RowID, &IoctlIF, inf_type, idx,
										   sizeof(struct mt_dev_priv), final_name, TRUE);
		}

#else
		pDevNew = RtmpOSNetDevCreate(pAd, MC_RowID, &IoctlIF, inf_type, idx,
									   sizeof(struct mt_dev_priv), dev_name, TRUE);
#endif /*MULTI_PROFILE*/
	}	else
#endif /*CONFIG_APSTA_MIXED_SUPPORT*/
		pDevNew = RtmpOSNetDevCreate(pAd, MC_RowID, &IoctlIF, inf_type, idx, sizeof(struct mt_dev_priv), dev_name, TRUE);

		if (pDevNew == NULL)
			break;
		else {
			pAd->MSTANum = idx + 1; /* re-assign new actual number of total MSTA, including MAIN STA*/
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Register MSTA IF (%s) , pAd->MSTANum = %d\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(pDevNew), pAd->MSTANum));
		}

#ifdef DOT11_SAE_SUPPORT
		pStaCfg->sae_cfg_group = SAE_DEFAULT_GROUP;
#endif
#ifdef CONFIG_OWE_SUPPORT
		pStaCfg->curr_owe_group = ECDH_GROUP_256;
#endif

		Ret = wdev_init(pAd, wdev, WDEV_TYPE_STA, pDevNew, idx,
						(VOID *)pStaCfg, (VOID *)pAd);

		if (!Ret) {
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(pDevNew)));
			RtmpOSNetDevFree(pDevNew);
			break;
		}

		Ret = wdev_ops_register(wdev, WDEV_TYPE_STA, wdev_ops,
								cap->qos.wmm_detect_method);

		if (!Ret) {
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("register wdev_ops %s failed, free net device!\n", RTMP_OS_NETDEV_GET_DEVNAME(pDevNew)));
			RtmpOSNetDevFree(pDevNew);
			break;
		}

		/* init operation functions and flags */
		NdisCopyMemory(&netDevHook, pNetDevOps, sizeof(netDevHook));
		netDevHook.priv_flags = inf_type;
		netDevHook.needProtcted = TRUE;
		netDevHook.wdev = wdev;
		COPY_MAC_ADDR(&pStaCfg->wdev.if_addr[0], pAd->CurrentAddress);
#ifdef CONFIG_APSTA_MIXED_SUPPORT

		if ((IF_COMBO_HAVE_AP_STA(pAd))) {
			apcli_sync_wdev(pAd, wdev);
			/*update rate info*/
			SetCommonHtVht(pAd, wdev);
			RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
			apcli_mac_addr_decide(pAd, wdev, idx);
		} else
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
		{
			pStaCfg->wdev.if_addr[0] |= 0x2;
			/* default choose bit[31:28], if there is no assigned mac from profile. */
			pStaCfg->wdev.if_addr[3] = pStaCfg->wdev.if_addr[3] & 0xef;
			pStaCfg->wdev.if_addr[3] = (pStaCfg->wdev.if_addr[3] | (idx << 4));
		}

		RTMP_OS_NETDEV_SET_PRIV(pDevNew, pAd);
		RTMP_OS_NETDEV_SET_WDEV(pDevNew, wdev);
		NdisMoveMemory(&netDevHook.devAddr[0], pStaCfg->wdev.if_addr, MAC_ADDR_LEN);

#ifdef APCLI_CFG80211_SUPPORT
		if (IF_COMBO_HAVE_AP_STA(pAd)) {
			struct wireless_dev *pWdev;
			CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
			UINT32 DevType = RT_CMD_80211_IFTYPE_STATION;

			pWdev = kzalloc(sizeof(*pWdev), GFP_KERNEL);
			pDevNew->ieee80211_ptr = pWdev;
			pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
			SET_NETDEV_DEV(pDevNew, wiphy_dev(pWdev->wiphy));
			pWdev->netdev = pDevNew;
			pWdev->iftype = DevType;
			pWdev->use_4addr = true;
		}
#endif /* APCLI_CFG80211_SUPPORT */


		/* register this device to OS */
		status = RtmpOSNetDevAttach(pAd->OpMode, pDevNew, &netDevHook);
		pStaCfg->ApcliInfStat.ApCliInit = TRUE;
		pAd->flg_msta_init = TRUE;
	}

#ifdef MAC_REPEATER_SUPPORT
	CliLinkMapInit(pAd);
#endif
}


VOID MSTAStop(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry;
	UCHAR CliIdx;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* MAC_REPEATER_SUPPORT */
	/* Clear PMKID cache.*/
	pStaCfg->SavedPMKNum = 0;
	RTMPZeroMemory(pStaCfg->SavedPMK, (PMKID_NO * sizeof(BSSID_INFO)));

	/* Link down first if any association exists*/
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		if (INFRA_ON(pStaCfg) || ADHOC_ON(pAd)) {
#ifdef MAC_REPEATER_SUPPORT

			if (pAd->ApCfg.bMACRepeaterEn) {
				for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
					pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

					if ((pReptEntry->CliEnable) && (pReptEntry->wdev == wdev)) {
						struct inter_machine_info priv_data;

						priv_data.rept_cli_entry = pReptEntry;
						priv_data.rept_cli_idx = CliIdx;

						RTMP_OS_INIT_COMPLETION(&pReptEntry->free_ack);

						pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_IF_DOWN;
						cntl_disconnect_request(pReptEntry->wdev,
												CNTL_DISASSOC,
												pStaCfg->Bssid,
												REASON_DISASSOC_STA_LEAVING,
												&priv_data);

						ReptWaitLinkDown(pReptEntry);
					}
				}
			}

#endif /* MAC_REPEATER_SUPPORT */
#ifdef APCLI_CFG80211_SUPPORT
			RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, RT_INTERFACE_DOWN, NULL, NULL, 0);

			if (pStaCfg->wpa_supplicant_info.pWpsProbeReqIe) {
				os_free_mem(pStaCfg->wpa_supplicant_info.pWpsProbeReqIe);
				pStaCfg->wpa_supplicant_info.pWpsProbeReqIe = NULL;
				pStaCfg->wpa_supplicant_info.WpsProbeReqIeLen = 0;
			}

			if (pStaCfg->wpa_supplicant_info.pWpaAssocIe) {
				os_free_mem(pStaCfg->wpa_supplicant_info.pWpaAssocIe);
				pStaCfg->wpa_supplicant_info.pWpaAssocIe = NULL;
				pStaCfg->wpa_supplicant_info.WpaAssocIeLen = 0;
			}
#endif /* defined(APCLI_CFG80211_SUPPORT) */


			RTMP_OS_INIT_COMPLETION(&pStaCfg->linkdown_complete);
#ifdef APCLI_SUPPORT
			pStaCfg->ApcliInfStat.Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_IF_DOWN;
#endif
			cntl_disconnect_request(wdev,
									CNTL_DISASSOC,
									pStaCfg->Bssid,
									REASON_DISASSOC_STA_LEAVING,
									NULL);
			sta_wait_link_down(pStaCfg);

		}
		RTMP_OS_INIT_COMPLETION(&pStaCfg->ifdown_fsm_reset_complete);
		cntl_reset_all_fsm_in_ifdown(wdev);
		sta_wait_ifdown(pStaCfg);
	}

	/*==========================================*/
	/* Clean up old bss table*/
#ifndef ANDROID_SUPPORT
	/* because abdroid will get scan table when interface down, so we not clean scan table */
	BssTableInit(ScanTab);
#endif /* ANDROID_SUPPORT */
}


VOID MSTA_Remove(RTMP_ADAPTER *pAd)
{
	UINT MaxNumSta = 0;
	struct wifi_dev *wdev = NULL;
	UINT IdSta = 0;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;

	if (!pAd)
		return;

	MaxNumSta = pAd->MaxMSTANum;

#ifdef CONFIG_APSTA_MIXED_SUPPORT

	if ((IF_COMBO_HAVE_AP_STA(pAd))) {
		for (IdSta = 0; IdSta < MAX_APCLI_NUM; IdSta++) {
			wdev = &pAd->StaCfg[IdSta].wdev;

			if (wdev->if_dev) {
				RtmpOSNetDevProtect(1);
				RtmpOSNetDevDetach(wdev->if_dev);
				RtmpOSNetDevProtect(0);
				wdev_deinit(pAd, wdev);
				RtmpOSNetDevFree(wdev->if_dev);
				/* Clear it as NULL to prevent latter access error. */
				pAd->StaCfg[IdSta].ApcliInfStat.ApCliInit = FALSE;
				wdev->if_dev = NULL;
			}
		}
	} else
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
	{
		for (IdSta = (MAIN_MSTA_ID + 1); IdSta < MaxNumSta; IdSta++) {
			wdev = &pAd->StaCfg[IdSta].wdev;
			pStaCfg = &pAd->StaCfg[IdSta];

			if (pStaCfg)
				return;

			if (wdev->if_dev) {
				RtmpOSNetDevProtect(1);
				RtmpOSNetDevDetach(wdev->if_dev);
				RtmpOSNetDevProtect(0);
				wdev_deinit(pAd, wdev);
				RtmpOSNetDevFree(wdev->if_dev);
				wdev->if_dev = NULL;
			}
		}
	}

}

/*
* MSTA_Open
*/
INT sta_inf_open(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#ifdef APCLI_CONNECTION_TRIAL
	UINT mbss_idx;
	UINT sync_to_band;
#endif
#ifdef CONFIG_APSTA_MIXED_SUPPORT
	if ((IF_COMBO_HAVE_AP_STA(pAd))) {
		if (wdev->func_idx >= MAX_APCLI_NUM)
			return FALSE;
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
		if (wf_drv_tbl.wf_fwd_probe_adapter)
			wf_drv_tbl.wf_fwd_probe_adapter(pAd);
#endif

#if defined(CONFIG_FAST_NAT_SUPPORT) && (defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE))
		if (wf_ra_sw_nat_hook_rx_bkup != NULL)
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s:wf_ra_sw_nat_hook_rx_bkup:%p\n",
						 __func__, wf_ra_sw_nat_hook_rx_bkup));

		wf_ra_sw_nat_hook_rx_bkup = NULL;
#endif

	}

#endif /* CONFIG_APSTA_MIXED_SUPPORT */
	pStaCfg = &pAd->StaCfg[wdev->func_idx];
	pStaCfg->apcliNeedEnable = FALSE;	/*Used while scanning*/

#ifdef APCLI_CONNECTION_TRIAL
/*For Apcli Trial connection, change phymode according to the channel selected*/
/*Change mode according to the 2G/ 5G connection*/
	if ((wdev->func_idx == (pAd->ApCfg.ApCliNum-1)) && (pStaCfg->TrialCh > 0)) {
		sync_to_band = GetBandByChannel(pStaCfg->TrialCh, pAd); /*Find band idx by checking in which channel list, trial channel is there*/
		for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
			struct wifi_dev *mbss_wdev;
			mbss_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
			if (HcGetBandByWdev(mbss_wdev) == sync_to_band) {
				wdev->channel = mbss_wdev->channel; /*Update channel and PhyMode for the band idx */
				wdev->PhyMode = mbss_wdev->PhyMode;
			}
		}
		apcli_sync_wdev(pAd, wdev);
		/*update rate info*/
		SetCommonHtVht(pAd, wdev);
		BuildChannelList(pAd, wdev);
		RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);
		RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
		wlan_operate_init(wdev);
	}
#endif	/*APCLI_CONNECTION_TRIAL*/
	if (wifi_sys_open(wdev) != TRUE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() open fail!!!\n", __func__));
		return FALSE;
	}

#ifdef CONFIG_APSTA_MIXED_SUPPORT
	if (IF_COMBO_HAVE_AP_STA(pAd))
		RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

#ifdef WSC_INCLUDED
	/* WSC parameters initialization required in case of ApCli mode as well */
	WscUUIDInit(pAd, wdev->func_idx, TRUE);
#endif /* WSC_INCLUDED */
#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd))
		map_a4_init(pAd, wdev->func_idx, FALSE);
#endif /* CONFIG_MAP_SUPPORT */

	sta_os_completion_initialize(pStaCfg);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MSTA interface up for %s%x func_idx=%d OmacIdx=%d\n",
			 (IF_COMBO_HAVE_AP_STA(pAd)) ? "apcli" : "ra",
			 wdev->func_idx,
			 pStaCfg->wdev.func_idx,
			 pStaCfg->wdev.OmacIdx));


	return TRUE;
}

/*
* MSTA_Close + RTMPInfClose
*/
INT sta_inf_close(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[wdev->func_idx];
#endif /* APCLI_AUTO_CONNECT_SUPPORT */


	/* Disconnet from AP of this interface */
	/* Pat: TODO: */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	pStaCfg->ApCliAutoConnectRunning = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

#if defined(CONFIG_FAST_NAT_SUPPORT) && (defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE))
	if (wf_ra_sw_nat_hook_rx_bkup != NULL)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%s:wf_ra_sw_nat_hook_rx_bkup:%p\n", __func__, wf_ra_sw_nat_hook_rx_bkup));

	wf_ra_sw_nat_hook_rx_bkup = NULL;

#endif

#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd))
			map_a4_deinit(pAd, wdev->func_idx, FALSE);
#endif /* CONFIG_MAP_SUPPORT */
	MSTAStop(pAd, wdev);
	pStaCfg->apcliNeedEnable = FALSE;	/*Used while scanning*/
	if (wifi_sys_close(wdev) != TRUE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() close fail!!!\n", __func__));
		return FALSE;
	}

	MacTableResetWdev(pAd, wdev);


	return TRUE;
}

/*
 * wifi system layer api for ADHOC
 */
INT adhoc_link_up(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) wdev->sys_handle;

	UpdateBeaconHandler(
		ad,
		wdev,
		BCN_UPDATE_IF_STATE_CHG);

	if ((wdev->channel > 14)
		&& (ad->CommonCfg.bIEEE80211H == 1)
		&& RadarChannelCheck(ad, wdev->channel)) {
		;	/*Do nothing */
	} else {
		AsicEnableIbssSync(
			ad,
			ad->CommonCfg.BeaconPeriod,
			HW_BSSID_0,
			OPMODE_ADHOC);
	}

	if (wifi_sys_linkup(wdev, entry) != TRUE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): linkup fail!\n", __func__));
	}

	return TRUE;
}

UINT32 bssinfo_sta_feature_decision(struct wifi_dev *wdev, UCHAR wcid, UINT32 *feature)
{
	UINT32 features = 0;

	features |= BSS_INFO_PM_FEATURE;
#ifdef UAPSD_SUPPORT
	features |= BSS_INFO_UAPSD_FEATURE;
#endif /* UAPSD_SUPPORT */
	*feature |= features;
	return TRUE;
}

VOID sta_deauth_act(struct wifi_dev *wdev)
{
	RTEnqueueInternalCmd(wdev->sys_handle, CMDTHRED_STA_DEAUTH_ACT, (VOID *) wdev, sizeof(struct wifi_dev));
}


BOOLEAN sta_media_state_connected(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad;
	STA_ADMIN_CONFIG *sta_cfg;

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	sta_cfg = GetStaCfgByWdev(ad, wdev);
	return STA_STATUS_TEST_FLAG(sta_cfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
}
VOID sta_handle_mic_error_event(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *entry, RX_BLK *pRxBlk)
{
	CIPHER_KEY *pWpaKey = &pAd->SharedKey[BSS0][pRxBlk->key_idx];
	PSTA_ADMIN_CONFIG sta_cfg = NULL;
	struct wifi_dev *wdev = entry->wdev;

	sta_cfg = GetStaCfgByWdev(pAd, wdev);
	ASSERT(sta_cfg);

	if (sta_cfg && INFRA_ON(sta_cfg)) {
			RTMPReportMicError(pAd, sta_cfg, pWpaKey);

		RTMPSendWirelessEvent(pAd, IW_MIC_ERROR_EVENT_FLAG,
							  pRxBlk->Addr2, BSS0, 0);
	}
}



/* for STA/APCLI - main thread to wait for mlme completes LinkDown */
VOID sta_os_completion_initialize(STA_ADMIN_CONFIG *pStaCfg)
{
	RTMP_OS_INIT_COMPLETION(&pStaCfg->ifdown_fsm_reset_complete);
	RTMP_OS_INIT_COMPLETION(&pStaCfg->linkdown_complete);
#ifdef APCLI_CFG80211_SUPPORT
	RTMP_OS_INIT_COMPLETION(&pStaCfg->scan_complete);
#endif /* APCLI_CFG80211_SUPPORT */

}

VOID sta_link_down_complete(STA_ADMIN_CONFIG *pStaCfg)
{
	RTMP_OS_COMPLETE(&pStaCfg->linkdown_complete);
}

VOID sta_wait_link_down(STA_ADMIN_CONFIG *pStaCfg)
{
	if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pStaCfg->linkdown_complete, STA_OS_WAIT_TIMEOUT)) {
		struct wifi_dev *wdev = &pStaCfg->wdev;
		RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
		UINT linkdown_type = 0;

		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 ("(%s) sta idx [%d] can't linkdown within 500ms, do linkdown in main thread\n"
				  , __func__, pStaCfg->wdev.func_idx));
		LinkDown(pAd, linkdown_type, wdev, NULL);
	}
}

VOID sta_ifdown_fsm_reset_complete(STA_ADMIN_CONFIG *pStaCfg)
{
	RTMP_OS_COMPLETE(&pStaCfg->ifdown_fsm_reset_complete);
}

VOID sta_wait_ifdown(STA_ADMIN_CONFIG *pStaCfg)
{
	if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pStaCfg->ifdown_fsm_reset_complete, STA_OS_WAIT_TIMEOUT)) {
		struct wifi_dev *wdev = &pStaCfg->wdev;
		RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;

		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 ("(%s) sta idx [%d] can't reset fsm within 500ms, do reset in main thread\n"
				  , __func__, pStaCfg->wdev.func_idx));
		cntl_fsm_reset(wdev);
		auth_fsm_reset(wdev);
		assoc_fsm_reset(wdev);
		sync_fsm_cancel_req_action(pAd, wdev);
	}
}


VOID sta_fsm_ops_hook(struct wifi_dev *wdev)
{
	sta_cntl_init(wdev);
	sta_auth_init(wdev);
	sta_assoc_init(wdev);
}

#ifdef CONFIG_APSTA_MIXED_SUPPORT
VOID apcli_sync_wdev(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct wifi_dev *ap_wdev = NULL;

	if (pAd->CommonCfg.dbdc_mode == TRUE) {
		int mbss_idx;

		/*for 5G+5G case choose both phymode & func_idx the same first.*/
		for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
			if (pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode == wdev->PhyMode && wdev->func_idx == mbss_idx) {
				ap_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
				update_att_from_wdev(wdev, ap_wdev);
			}
		}

		if (ap_wdev)
			return;

		/*original rule*/
		for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
			if (pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode == wdev->PhyMode) {
				update_att_from_wdev(wdev, &pAd->ApCfg.MBSSID[mbss_idx].wdev);
			}
		}
	} else {
		/* align phy mode to BSS0 by default */
		wdev->PhyMode = pAd->ApCfg.MBSSID[BSS0].wdev.PhyMode;
		update_att_from_wdev(wdev, &pAd->ApCfg.MBSSID[BSS0].wdev);
	}
}


VOID apcli_mac_addr_decide(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT idx)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef MT_MAC

	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
#endif /* MT_MAC */

		if (cap->MBSSIDMode >= MBSSID_MODE1) {
			if ((pAd->ApCfg.BssidNum > 0) || (MAX_MESH_NUM > 0)) {
				UCHAR MacMask = 0;

				if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 2)
					MacMask = 0xFE;
				else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 4)
					MacMask = 0xFC;
				else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 8)
					MacMask = 0xF8;


				/*
					Refer to HW definition -
						Bit1 of MAC address Byte0 is local administration bit
						and should be set to 1 in extended multiple BSSIDs'
						Bit3~ of MAC address Byte0 is extended multiple BSSID index.
				*/
				if (cap->MBSSIDMode == MBSSID_MODE1) {
					/*
						Refer to HW definition -
							Bit1 of MAC address Byte0 is local administration bit
							and should be set to 1 in extended multiple BSSIDs'
							Bit3~ of MAC address Byte0 is extended multiple BSSID index.
					*/
#ifdef ENHANCE_NEW_MBSSID_MODE
					wdev->if_addr[0] &= (MacMask << 2);
#endif /* ENHANCE_NEW_MBSSID_MODE */
					wdev->if_addr[0] |= 0x2;
					wdev->if_addr[0] += (((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1) << 2);
				}

#ifdef ENHANCE_NEW_MBSSID_MODE
				else {
					wdev->if_addr[0] |= 0x2;
					wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] &= (MacMask);
					wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] += ((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1);
				}

#endif /* ENHANCE_NEW_MBSSID_MODE */
			}
		} else
			wdev->if_addr[MAC_ADDR_LEN - 1] = (wdev->if_addr[MAC_ADDR_LEN - 1] + pAd->ApCfg.BssidNum + MAX_MESH_NUM) & 0xFF;

#ifdef MT_MAC
	} else {
		UCHAR MacByte = 0;
		UINT32 Value = 0;
		UCHAR MacMask = 0xef;
		MacByte = 2; /* snowpin test, TODO - check mode from MBSS to avoid using same mac address. 2016/2/5 */

		HW_IO_READ32(pAd->hdev_ctrl, LPON_BTEIR, &Value);
		MacByte = Value >> 29;
		if (pAd->ApCfg.ApCliNum <= 2) {
			MacMask = 0xef;
		} else if (pAd->ApCfg.ApCliNum <= 4) {
			MacMask = 0xcf;
		}
		/* Carter, I make apcli interface use HWBSSID1 to go. */
		/* so fill own_mac and BSSID here. */
		wdev->if_addr[0] |= 0x2;
		/* apcli can not use the same mac as MBSS,so change if_addr[0] to separate */
		if ((wdev->if_addr[0] & 0x4) == 0x4)
			wdev->if_addr[0] &= ~0x4;
		else
			wdev->if_addr[0] |= 0x4;
		/*wdev->if_addr[0] |= (idx << 4);*/

		switch (MacByte) {
		case 0x2: /* choose bit[31:28]*/
			wdev->if_addr[3] = wdev->if_addr[3] & MacMask;/*clear high 4 bits*/
			wdev->if_addr[3] = (wdev->if_addr[3] | (idx << 4));
			break;

		default: /* choose bit[15:12]*/
			wdev->if_addr[1] = wdev->if_addr[1] & MacMask;/*clear high 4 bits*/
			wdev->if_addr[1] = (wdev->if_addr[1] | (idx << 4));
			break;
		}
	}

#endif /* MT_MAC */
}

VOID ApCliPeerCsaAction(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BCN_IE_LIST *ie_list)
{
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;

	if (pAd == NULL || ie_list == NULL)
		return;

	if (pDot11h == NULL)
		return;

	if ((pAd->CommonCfg.bIEEE80211H == 1) &&
		ie_list->NewChannel != 0 &&
		wdev->channel != ie_list->NewChannel &&
		pDot11h->RDMode != RD_SWITCHING_MODE) {
#ifdef DOT11_VHT_AC
		{
			struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

			if (IS_CAP_BW160(cap)) {
				struct vht_opinfo *vht_op = &ie_list->vht_op_ie.vht_op_info;

				print_vht_op_info(vht_op);
				wlan_operate_set_cen_ch_2(wdev, vht_op->ccfs_1);
			}
		}
#endif /* DOT11_VHT_AC */
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
				 ("[APCLI]  Following root AP to switch channel to ch%u\n",
				  ie_list->NewChannel));
#if defined(WAPP_SUPPORT) && defined(CONFIG_MAP_SUPPORT)
		if (pAd->bMAPQuickChChangeEn)
			wdev->quick_ch_change = TRUE;

		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				("Channel Change due to csa\n"));
#endif

#if defined(CONFIG_MAP_SUPPORT)
		if (IS_MAP_TURNKEY_ENABLE(pAd) && (pAd->bMAPAvoidScanDuringCac == 1)) {
#if defined(DBDC_MODE)
			if (pAd->CommonCfg.dbdc_mode == TRUE) {
				UCHAR Band = 0;

				if (WMODE_CAP_2G(wdev->PhyMode))
					Band = 0;
				if (WMODE_CAP_5G(wdev->PhyMode))
					Band = 1;
				BssTableInit(get_scan_tab_by_wdev(pAd, wdev));
			} else
#endif
				BssTableInit(get_scan_tab_by_wdev(pAd, wdev));
		}
#endif
		rtmp_set_channel(pAd, wdev, ie_list->NewChannel);
#if defined(WAPP_SUPPORT) && defined(CONFIG_MAP_SUPPORT)
		if (pAd->bMAPQuickChChangeEn)
			wdev->quick_ch_change = FALSE;
		wapp_send_csa_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), ie_list->NewChannel);
#endif

	}
}

/*
	==========================================================================
	Description:
		Check the Apcli Entry is valid or not.
	==========================================================================
 */
static inline BOOLEAN ValidApCliEntry(RTMP_ADAPTER *pAd, INT apCliIdx)
{
	BOOLEAN result;
	PMAC_TABLE_ENTRY pMacEntry;
	STA_ADMIN_CONFIG *pApCliEntry;

	do {
		if ((apCliIdx < 0) || (apCliIdx >= MAX_APCLI_NUM)) {
			result = FALSE;
			break;
		}

		pApCliEntry = (STA_ADMIN_CONFIG *)&pAd->StaCfg[apCliIdx];

		if (pApCliEntry->ApcliInfStat.Valid != TRUE) {
			result = FALSE;
			break;
		}

		if (pApCliEntry->ApcliInfStat.Enable != TRUE) {
			result = FALSE;
			break;
		}

		if ((!VALID_UCAST_ENTRY_WCID(pAd, pApCliEntry->MacTabWCID))
			/* || (pApCliEntry->MacTabWCID < 0)  //MacTabWCID is UCHAR, no need to check */
		   ) {
			result = FALSE;
			break;
		}

		pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];

		if (!IS_ENTRY_PEER_AP(pMacEntry)) {
			result = FALSE;
			break;
		}

		result = TRUE;
	} while (FALSE);

	return result;
}

INT apcli_fp_tx_pkt_allowed(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pkt)
{
	UCHAR idx;
	BOOLEAN	allowed = FALSE;
	STA_ADMIN_CONFIG *apcli_entry;
#ifdef MAC_REPEATER_SUPPORT
	UINT Ret = 0;
#endif
	UCHAR wcid = RTMP_GET_PACKET_WCID(pkt);
	UCHAR frag_nums;

	for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
		apcli_entry = &pAd->StaCfg[idx];

		if (&apcli_entry->wdev == wdev) {
			if (ValidApCliEntry(pAd, idx) == FALSE)
				break;

#ifdef MAC_REPEATER_SUPPORT

			if ((pAd->ApCfg.bMACRepeaterEn == TRUE)
#ifdef A4_CONN
				&& (IS_APCLI_A4(apcli_entry) == FALSE)
#endif /* A4_CONN */
			   ) {
				Ret = ReptTxPktCheckHandler(pAd, wdev, pkt, &wcid);
				if (Ret == REPEATER_ENTRY_EXIST)
					allowed = TRUE;
				else if (Ret == INSERT_REPT_ENTRY) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 ("apcli_fp_tx_pkt_allowed: return FALSE as ReptTxPktCheckHandler indicated INSERT_REPT_ENTRY\n"));
					allowed = FALSE;
				} else if (Ret == INSERT_REPT_ENTRY_AND_ALLOW)
					allowed = TRUE;
				else if (Ret == USE_CLI_LINK_INFO) {
					wcid = apcli_entry->MacTabWCID;
					allowed = TRUE;
				}
			} else
#endif /* MAC_REPEATER_SUPPORT */
			{
				pAd->RalinkCounters.PendingNdisPacketCount++;
				RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
				wcid = apcli_entry->MacTabWCID;
				allowed = TRUE;
			}

			break;
		}
	}

	if (allowed) {
		RTMP_SET_PACKET_WCID(pkt, wcid);
		frag_nums = get_frag_num(pAd, wdev, pkt);
		RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);

		/*  ethertype check is not offload to mcu for fragment frame*/
		if (frag_nums > 1) {
			if (!RTMPCheckEtherType(pAd, pkt, &pAd->MacTab.tr_entry[wcid], wdev))
				allowed = FALSE;
		}
	}

	return allowed;
}

INT apcli_tx_pkt_allowed(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pkt)
{
	UCHAR idx;
	BOOLEAN	allowed = FALSE;
	STA_ADMIN_CONFIG *apcli_entry;
#ifdef MAC_REPEATER_SUPPORT
	UINT Ret = 0;
#endif
	STA_TR_ENTRY *tr_entry = NULL;
	UCHAR wcid = RTMP_GET_PACKET_WCID(pkt);
	UCHAR frag_nums;
#ifdef WSC_INCLUDED
	BOOLEAN do_wsc_now = FALSE;
#endif
	for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
		apcli_entry = &pAd->StaCfg[idx];

		if (&apcli_entry->wdev == wdev) {
			if (ValidApCliEntry(pAd, idx) == FALSE)
				break;

#ifdef MAC_REPEATER_SUPPORT

			if ((pAd->ApCfg.bMACRepeaterEn == TRUE)
#ifdef A4_CONN
				&& (IS_APCLI_A4(apcli_entry) == FALSE)
#endif /* A4_CONN */
#ifdef APCLI_CONNECTION_TRIAL
			   && (idx != (pAd->ApCfg.ApCliNum-1)) /*No need to add repeater entry for Trial connection*/
#endif /*APCLI_CONNECTION_TRIAL*/

			   ) {
				Ret = ReptTxPktCheckHandler(pAd, wdev, pkt, &wcid);

				if (Ret == REPEATER_ENTRY_EXIST) {
					allowed = TRUE;
				} else if (Ret == INSERT_REPT_ENTRY) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 ("apcli_tx_pkt_allowed: return FALSE as ReptTxPktCheckHandler indicated INSERT_REPT_ENTRY\n"));
					allowed = FALSE;
				} else if (Ret == INSERT_REPT_ENTRY_AND_ALLOW) {
					allowed = TRUE;
				} else if (Ret == USE_CLI_LINK_INFO) {
					wcid = apcli_entry->MacTabWCID;
					allowed = TRUE;
				}
			} else
#endif /* MAC_REPEATER_SUPPORT */
			{
				pAd->RalinkCounters.PendingNdisPacketCount++;
				RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
				wcid = apcli_entry->MacTabWCID;
				allowed = TRUE;
			}
			break;
		}
	}

	if (allowed) {
		RTMP_SET_PACKET_WCID(pkt, wcid);
		frag_nums = get_frag_num(pAd, wdev, pkt);
		RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);

#ifdef WSC_INCLUDED
		if ((wdev->WscControl.WscConfMode != WSC_DISABLE)
			&& wdev->WscControl.bWscTrigger)
			do_wsc_now = TRUE;
#endif /* WSC_INCLUDED */


		if (!RTMPCheckEtherType(pAd, pkt, &pAd->MacTab.tr_entry[wcid], wdev))
			allowed = FALSE;

		tr_entry = &pAd->MacTab.tr_entry[wcid];

		if (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED) {
			if (!((
#ifdef WSC_INCLUDED
				do_wsc_now ||
#endif
				 IS_AKM_WPA_CAPABILITY_Entry(wdev)
#ifdef DOT1X_SUPPORT
				   || (IS_IEEE8021X_Entry(wdev))
#endif /* DOT1X_SUPPORT */

				  ) && ((RTMP_GET_PACKET_EAPOL(pkt) ||
						 RTMP_GET_PACKET_WAI(pkt))))
			   )
				allowed = FALSE;

		}



	}

	return allowed;
}

INT apcli_rx_looping_detect(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pPacket)
{
#if defined(WH_EZ_SETUP) && (defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE))
	BOOLEAN bypass_rx_fwd = FALSE;
#endif

#ifdef MTFWD
#ifdef APCLI_SUPPORT
	if ((wdev->wdev_type == WDEV_TYPE_STA) && IF_COMBO_HAVE_AP_STA(pAd)) {
		struct ethhdr *mh = eth_hdr(pPacket);

		if (!mh) {
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return FALSE;
		}

		if ((mh->h_dest[0] & 0x1)) {
			UCHAR idx = 0;
			STA_ADMIN_CONFIG *apcli_entry = NULL;

#ifdef MAC_REPEATER_SUPPORT
			if ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
				(RTMPQueryLookupRepeaterCliEntryMT(pAd, mh->h_source, TRUE) == TRUE)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s------>Found SA with Repeater Fake MAC In!, Src:%02X:%02X:%02X:%02X:%02X:%02X, Dest:%02X:%02X:%02X:%02X:%02X:%02X\n",
					__func__, PRINT_MAC(mh->h_source), PRINT_MAC(mh->h_dest)));
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				return FALSE;
			}
#endif
			for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
				apcli_entry = &pAd->StaCfg[idx];

				if (!apcli_entry->ApcliInfStat.ApCliInit || (&apcli_entry->wdev == wdev))
					continue;

				if (MAC_ADDR_EQUAL(apcli_entry->wdev.if_addr, mh->h_source)) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("%s------>Found SA with ApCli MAC(%d) In!, Src:%02X:%02X:%02X:%02X:%02X:%02X, Dest:%02X:%02X:%02X:%02X:%02X:%02X\n",
						__func__, idx, PRINT_MAC(mh->h_source), PRINT_MAC(mh->h_dest)));
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					return FALSE;
				}
			}

		}
	}
#endif /* APCLI_SUPPORT */

#else /* MTFWD */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
	if ((wf_drv_tbl.wf_fwd_needed_hook != NULL) && (wf_drv_tbl.wf_fwd_needed_hook() == TRUE)) {
		if ((wdev->wdev_type == WDEV_TYPE_STA) && IF_COMBO_HAVE_AP_STA(pAd)) {
			if (wf_drv_tbl.wf_fwd_rx_hook != NULL) {
				int ret = 0;
				struct ethhdr *mh = eth_hdr(pPacket);

				if (!mh) {
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					return FALSE;
				}

				if ((mh->h_dest[0] & 0x1)) {
#ifdef MAC_REPEATER_SUPPORT
					if ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
						(RTMPQueryLookupRepeaterCliEntryMT(pAd, mh->h_source, TRUE) == TRUE)) {
						/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							("announce_802_3_packet: drop rx pkt by RTMPQueryLookupRepeaterCliEntryMT check\n"));*/
						/*MEM_DBG_PKT_ALLOC_INC(pPacket);*/
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
						return FALSE;
					}
#endif /* MAC_REPEATER_SUPPORT */
					{
						VOID *opp_band_tbl = NULL;
						VOID *band_tbl = NULL;
						VOID *other_band_tbl = NULL;

						if (wf_drv_tbl.wf_fwd_feedback_map_table)
							wf_drv_tbl.wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl, &other_band_tbl);

						if (band_tbl != NULL) {
#ifdef MAC_REPEATER_SUPPORT
							if (MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)band_tbl)->Wdev_ifAddr), mh->h_source) ||
								((((REPEATER_ADAPTER_DATA_TABLE *)band_tbl)->Wdev_ifAddr_DBDC != NULL) &&
								 MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)band_tbl)->Wdev_ifAddr_DBDC), mh->h_source))) {
								/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
									("announce_802_3_packet: drop rx pkt by wf_fwd_feedback_map_table band_tbl check of source addr against Wdev_ifAddr\n"));*/
								/*MEM_DBG_PKT_ALLOC_INC(pPacket);*/
								RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
								return FALSE;
							}
#endif /* MAC_REPEATER_SUPPORT */
						}

						if (opp_band_tbl != NULL) {
#ifdef MAC_REPEATER_SUPPORT
							if ((MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)opp_band_tbl)->Wdev_ifAddr), mh->h_source)) ||
								((((REPEATER_ADAPTER_DATA_TABLE *)opp_band_tbl)->Wdev_ifAddr_DBDC != NULL) &&
								 (MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)opp_band_tbl)->Wdev_ifAddr_DBDC), mh->h_source)))) {
								/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
								*	 ("announce_802_3_packet: drop rx pkt by wf_fwd_feedback_map_table opp_band_tbl check of source addr against Wdev_ifAddr\n")); */
								/*MEM_DBG_PKT_ALLOC_INC(pPacket);*/
								RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
								return FALSE;
							}
#endif /* MAC_REPEATER_SUPPORT */
						}

						if (other_band_tbl != NULL) {
#ifdef MAC_REPEATER_SUPPORT
							if ((MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)other_band_tbl)->Wdev_ifAddr), mh->h_source)) ||
								((((REPEATER_ADAPTER_DATA_TABLE *)other_band_tbl)->Wdev_ifAddr_DBDC != NULL) &&
								 (MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)other_band_tbl)->Wdev_ifAddr_DBDC), mh->h_source)))) {
								/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
								 ("announce_802_3_packet: drop rx pkt by wf_fwd_feedback_map_table other_band_tbl check of source addr against Wdev_ifAddr\n"));*/
								/*MEM_DBG_PKT_ALLOC_INC(pPacket);*/
								RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
								return FALSE;
							}
#endif /* MAC_REPEATER_SUPPORT */
						}
					}
				}

					ret = wf_drv_tbl.wf_fwd_rx_hook(pPacket);

					if (ret == 0) {
						/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							("announce_802_3_packet: wf_fwd_rx_hook returned 0\n"));*/
						return;
					} else if (ret == 2) {
						/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							("announce_802_3_packet: wf_fwd_rx_hook returned 2\n"));*/
						/*MEM_DBG_PKT_ALLOC_INC(pRxPkt);*/
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
						return FALSE;
					}

			}
		}
	}

#endif /* CONFIG_WIFI_PKT_FWD */
#endif /* MTFWD */

	return TRUE;
}

BOOLEAN ApCliMsgTypeSubst(
	IN PRTMP_ADAPTER pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType) {
	USHORT Seq;
	UCHAR EAPType;
	BOOLEAN Return = FALSE;
#ifdef WSC_AP_SUPPORT
	UCHAR EAPCode;
	PMAC_TABLE_ENTRY pEntry;
#endif /* WSC_AP_SUPPORT */
	unsigned char hdr_len = LENGTH_802_11;
#ifdef DOT11_SAE_SUPPORT
	USHORT Alg;
#endif
#ifdef A4_CONN
	if ((pFrame->Hdr.FC.FrDs == 1) && (pFrame->Hdr.FC.ToDs == 1))
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif


	/* only PROBE_REQ can be broadcast, all others must be unicast-to-me && is_mybssid; otherwise, */
	/* ignore this frame */

	/* WPA EAPOL PACKET */
	if (pFrame->Hdr.FC.Type == FC_TYPE_DATA) {
#ifdef WSC_AP_SUPPORT
		/*WSC EAPOL PACKET */
		pEntry = MacTableLookup(pAd, pFrame->Hdr.Addr2);

		if (pEntry && IS_ENTRY_PEER_AP(pEntry) && pAd->StaCfg[pEntry->func_tb_idx].wdev.WscControl.WscConfMode == WSC_ENROLLEE) {
			*Machine = WSC_STATE_MACHINE;
			EAPType = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 1);
			EAPCode = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 4);
			Return = WscMsgTypeSubst(EAPType, EAPCode, MsgType);
		}

		if (!Return)
#endif /* WSC_AP_SUPPORT */
		{
			*Machine = WPA_STATE_MACHINE;
			EAPType = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 1);
			Return = WpaMsgTypeSubst(EAPType, MsgType);
		}

		return Return;
	} else if (pFrame->Hdr.FC.Type == FC_TYPE_MGMT) {
		switch (pFrame->Hdr.FC.SubType) {
		case SUBTYPE_ASSOC_RSP:
			*Machine = ASSOC_FSM;
			*MsgType = ASSOC_FSM_PEER_ASSOC_RSP;
			break;

		case SUBTYPE_DISASSOC:
			*Machine = ASSOC_FSM;
			*MsgType = ASSOC_FSM_PEER_DISASSOC_REQ;
			break;

		case SUBTYPE_DEAUTH:
			*Machine = AUTH_FSM;
			*MsgType = AUTH_FSM_PEER_DEAUTH;
			break;

		case SUBTYPE_AUTH:
#ifdef DOT11_SAE_SUPPORT
			NdisMoveMemory(&Alg, &pFrame->Octet[0], sizeof(USHORT));
#endif /* DOT11_SAE_SUPPORT */
			/* get the sequence number from payload 24 Mac Header + 2 bytes algorithm */
			NdisMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));
#ifdef DOT11_SAE_SUPPORT
			if (Alg == AUTH_MODE_SAE) {
				*Machine = AUTH_FSM;
				*MsgType = AUTH_FSM_SAE_AUTH_RSP;
			} else
#endif /* DOT11_SAE_SUPPORT */
			if (Seq == 2 || Seq == 4) {
				*Machine = AUTH_FSM;
				*MsgType = AUTH_FSM_PEER_AUTH_EVEN;
			} else
				return FALSE;

			break;

		case SUBTYPE_ACTION:
			*Machine = ACTION_STATE_MACHINE;

			/*  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support */
			if ((pFrame->Octet[0] & 0x7F) > MAX_PEER_CATE_MSG)
				*MsgType = MT2_ACT_INVALID;
			else
				*MsgType = (pFrame->Octet[0] & 0x7F);

			break;

		default:
			return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}



BOOLEAN preCheckMsgTypeSubset(
	IN PRTMP_ADAPTER  pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType)
{
	if (pFrame->Hdr.FC.Type == FC_TYPE_MGMT) {
		switch (pFrame->Hdr.FC.SubType) {
		/* Beacon must be processed by AP Sync state machine. */
		case SUBTYPE_BEACON:
			*Machine = SYNC_FSM;
			*MsgType = SYNC_FSM_PEER_BEACON;
			break;

		/* Only Sta have chance to receive Probe-Rsp. */
		case SUBTYPE_PROBE_RSP:
			*Machine = SYNC_FSM;
			*MsgType = SYNC_FSM_PEER_PROBE_RSP;
			break;

		default:
			return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}
BOOLEAN  ApCliHandleRxBroadcastFrame(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN MAC_TABLE_ENTRY *pEntry) {
	STA_ADMIN_CONFIG *pApCliEntry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
	/*
		It is possible to receive the multicast packet when in AP Client mode
		ex: broadcast from remote AP to AP-client,
				addr1=ffffff, addr2=remote AP's bssid, addr3=sta4_mac_addr
	*/
	pApCliEntry = &pAd->StaCfg[pEntry->func_tb_idx];
	/* Filter out Bcast frame which AP relayed for us */
	/* Multicast packet send from AP1 , received by AP2 and send back to AP1, drop this frame */

	if (MAC_ADDR_EQUAL(pRxBlk->Addr3, pApCliEntry->wdev.if_addr))
		return FALSE;

	if (pEntry->PrivacyFilter != Ndis802_11PrivFilterAcceptAll)
		return FALSE;

#ifdef MAC_REPEATER_SUPPORT

	if (pAd->ApCfg.bMACRepeaterEn
#ifdef A4_CONN
		&& (IS_APCLI_A4(pApCliEntry) == FALSE)
#endif /* A4_CONN */
	   ) {
		pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pRxBlk->Addr3, TRUE);

		if (pReptEntry) {
			/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, */
			/* ("ApCliHandleRxBroadcastFrame: return FALSE  pReptEntry found\n")); */
			return FALSE;	/* give up this frame */
		}
	}

#endif /* MAC_REPEATER_SUPPORT */
	return TRUE;
}

BOOLEAN apcli_fill_non_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	PNDIS_PACKET pPacket;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev_ops *ops = wdev->wdev_ops;

	pPacket = pTxBlk->pPacket;
	pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
	pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	pTxBlk->wmm_set = HcGetWmmIdx(pAd, wdev);
	pTxBlk->UserPriority = RTMP_GET_PACKET_UP(pPacket);
	pTxBlk->FrameGap = IFS_HTTXOP;
	pTxBlk->pMbss = NULL;
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

	if (IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if ((pTxBlk->TxFrameType == TX_LEGACY_FRAME) ||
			(pTxBlk->TxFrameType == TX_AMSDU_FRAME) ||
			(pTxBlk->TxFrameType == TX_MCAST_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
	}

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pTxBlk->pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bClearEAPFrame);


	if (pTxBlk->tr_entry->EntryType == ENTRY_CAT_MCAST) {
		pTxBlk->pMacEntry = NULL;
		TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);
		{
			{
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode;

				if (pTxBlk->wdev->channel > 14) {
					pTxBlk->pTransmit->field.MODE = MODE_OFDM;
					pTxBlk->pTransmit->field.MCS = MCS_RATE_6;
				}
			}
		}
		/* AckRequired = FALSE, when broadcast packet in Adhoc mode.*/
		TX_BLK_CLEAR_FLAG(pTxBlk, (fTX_bAckRequired | fTX_bAllowFrag | fTX_bWMM));

		if (RTMP_GET_PACKET_MOREDATA(pPacket))
			TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
	} else {
		pTxBlk->pMacEntry = &pAd->MacTab.Content[pTxBlk->Wcid];
		pTxBlk->pTransmit = &pTxBlk->pMacEntry->HTPhyMode;
		pMacEntry = pTxBlk->pMacEntry;

		if (!pMacEntry)
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Err!! pMacEntry is NULL!!\n", __func__));
		else
			pTxBlk->pMbss = pMacEntry->pMbss;

#ifdef MULTI_WMM_SUPPORT

		if (IS_ENTRY_PEER_AP(pMacEntry))
			pTxBlk->QueIdx = EDCA_WMM1_AC0_PIPE;

#endif /* MULTI_WMM_SUPPORT */
		/* For all unicast packets, need Ack unless the Ack Policy is not set as NORMAL_ACK.*/
#ifdef MULTI_WMM_SUPPORT

		if (pTxBlk->QueIdx >= EDCA_WMM1_AC0_PIPE) {
			if (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx - EDCA_WMM1_AC0_PIPE] != NORMAL_ACK)
				TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
			else
				TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);
		} else
#endif /* MULTI_WMM_SUPPORT */
		{
			if (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx] != NORMAL_ACK)
				TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
			else
				TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);
		}

		{
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef A4_CONN
				if (IS_ENTRY_A4(pMacEntry)) {
					pTxBlk->pMacEntry = pMacEntry;
					pTxBlk->pApCliEntry = &pAd->StaCfg[pMacEntry->func_tb_idx];
					TX_BLK_SET_FLAG(pTxBlk, fTX_bA4Frame);
				} else
#endif /* A4_CONN */
				if (pMacEntry && (IS_ENTRY_PEER_AP(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry)) &&
					((pTxBlk->TxFrameType != TX_MCAST_FRAME) &&
					 (pTxBlk->TxFrameType != TX_MLME_DATAQ_FRAME) &&
					 (pTxBlk->TxFrameType != TX_MLME_MGMTQ_FRAME))) {
#ifdef MAT_SUPPORT
					PNDIS_PACKET apCliPkt = NULL;
					UCHAR *pMacAddr = NULL;
#ifdef MAC_REPEATER_SUPPORT

					if ((pMacEntry->bReptCli) && (pAd->ApCfg.bMACRepeaterEn)) {
						UCHAR tmpIdx;

						pAd->MatCfg.bMACRepeaterEn = pAd->ApCfg.bMACRepeaterEn;

						if (pAd->ApCfg.MACRepeaterOuiMode != CASUALLY_DEFINE_MAC_ADDR) {
							tmpIdx = REPT_MLME_START_IDX + pMacEntry->MatchReptCliIdx;
							/* TODO: shiang-lock, fix ME! */
							apCliPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, tmpIdx, pTxBlk->OpMode);
							pMacAddr = &pAd->ApCfg.pRepeaterCliPool[pMacEntry->MatchReptCliIdx].CurrentAddress[0];
						}
					} else
#endif /* MAC_REPEATER_SUPPORT */
					{
						/* For each tx packet, update our MAT convert engine databases.*/
						/* CFG_TODO */

#ifdef APCLI_AS_WDS_STA_SUPPORT
						if (pAd->StaCfg[pMacEntry->func_tb_idx].wdev.wds_enable == 0)
#endif /* APCLI_AS_WDS_STA_SUPPORT */
						apCliPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, pMacEntry->func_tb_idx, pTxBlk->OpMode);
						pMacAddr = &pAd->StaCfg[pMacEntry->func_tb_idx].wdev.if_addr[0];
					}

					if (apCliPkt) {
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
						pPacket = apCliPkt;
						pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
						pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
						pTxBlk->pPacket = apCliPkt;
					} else {
						if (OS_PKT_CLONED(pPacket)) {
							OS_PKT_COPY(pPacket, apCliPkt);
							RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
							pPacket = apCliPkt;
							pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
							pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
							pTxBlk->pPacket = apCliPkt;
						}
					}

					{
						PUCHAR pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);

						if (pMacAddr)

#ifdef APCLI_AS_WDS_STA_SUPPORT
						if (pAd->StaCfg[pMacEntry->func_tb_idx].wdev.wds_enable == 0)

#endif /* APCLI_AS_WDS_STA_SUPPORT */
{

							NdisMoveMemory(pSrcBufVA + 6, pMacAddr, MAC_ADDR_LEN);
}

					}

#endif /* MAT_SUPPORT */
					pTxBlk->pApCliEntry = &pAd->StaCfg[pMacEntry->func_tb_idx];
					TX_BLK_SET_FLAG(pTxBlk, fTX_bApCliPacket);
				} else if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry))
					;
				else
					return FALSE;

				/* If both of peer and us support WMM, enable it.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM);
			}
		}

		if (pTxBlk->TxFrameType == TX_LEGACY_FRAME) {
			if (((RTMP_GET_PACKET_LOWRATE(pPacket))
#ifdef UAPSD_SUPPORT
				 && (!(pMacEntry && (pMacEntry->bAPSDFlagSPStart)))
#endif /* UAPSD_SUPPORT */
				) ||
				((pAd->OpMode == OPMODE_AP) && (pMacEntry->MaxHTPhyMode.field.MODE == MODE_CCK) && (pMacEntry->MaxHTPhyMode.field.MCS == RATE_1))
			   ) {
				/* Specific packet, i.e., bDHCPFrame, bEAPOLFrame, bWAIFrame, need force low rate. */
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode;
				TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);

				/* Modify the WMM bit for ICV issue. If we have a packet with EOSP field need to set as 1, how to handle it? */
				if (!pTxBlk->pMacEntry)
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Err!! pTxBlk->pMacEntry is NULL!!\n", __func__));
				else if (IS_HT_STA(pTxBlk->pMacEntry) &&
						 (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET)) &&
						 ((pAd->CommonCfg.bRdg == TRUE) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)))
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
			}

			if (!pMacEntry)
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Err!! pMacEntry is NULL!!\n", __func__));
			else if ((IS_HT_RATE(pMacEntry) == FALSE) &&
					 (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE))) {
				/* Currently piggy-back only support when peer is operate in b/g mode.*/
				TX_BLK_SET_FLAG(pTxBlk, fTX_bPiggyBack);
			}

			if (RTMP_GET_PACKET_MOREDATA(pPacket))
				TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);

#ifdef UAPSD_SUPPORT

			if (RTMP_GET_PACKET_EOSP(pPacket))
				TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP);

#endif /* UAPSD_SUPPORT */
		} else if (pTxBlk->TxFrameType == TX_FRAG_FRAME)
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAllowFrag);

		if (!pMacEntry)
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Err!! pMacEntry is NULL!!\n", __func__));
		else {
			pMacEntry->DebugTxCount++;
#ifdef MAC_REPEATER_SUPPORT

			if (pMacEntry->bReptCli)
				pMacEntry->ReptCliIdleCount = 0;

#endif
		}
	}

	pAd->LastTxRate = (USHORT)pTxBlk->pTransmit->word;
	ops->find_cipher_algorithm(pAd, wdev, pTxBlk);
	return TRUE;
}

BOOLEAN apcli_fill_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	PNDIS_PACKET pPacket;
	UCHAR *pMacAddr = NULL;
	PMAC_TABLE_ENTRY pMacEntry = NULL;
#ifdef MAT_SUPPORT
	PUCHAR pSrcBufVA = NULL;
	PNDIS_PACKET convertPkt = NULL;
#endif
	pPacket = pTxBlk->pPacket;
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
	pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);

	if (RTMP_GET_PACKET_MGMT_PKT(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);

	if (IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if ((pTxBlk->TxFrameType == TX_LEGACY_FRAME) ||
			(pTxBlk->TxFrameType == TX_AMSDU_FRAME) ||
			(pTxBlk->TxFrameType == TX_MCAST_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
	}

	pMacEntry = &pAd->MacTab.Content[pTxBlk->Wcid];
#ifdef A4_CONN
	if (IS_ENTRY_A4(pMacEntry)) {
	pTxBlk->pMacEntry = pMacEntry;
		pTxBlk->pApCliEntry = &pAd->StaCfg[pMacEntry->func_tb_idx];
		TX_BLK_SET_FLAG(pTxBlk, fTX_bA4Frame);
	} else
#endif /* A4_CONN */

	if ((IS_ENTRY_PEER_AP(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry)) &&
		((pTxBlk->TxFrameType != TX_MCAST_FRAME) &&
		 (pTxBlk->TxFrameType != TX_MLME_DATAQ_FRAME) &&
		 (pTxBlk->TxFrameType != TX_MLME_MGMTQ_FRAME))) {
#ifdef MAT_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
			if ((pMacEntry->bReptCli) && (pAd->ApCfg.bMACRepeaterEn)) {
				UCHAR tmpIdx;

				pAd->MatCfg.bMACRepeaterEn = pAd->ApCfg.bMACRepeaterEn;

				if (pAd->ApCfg.MACRepeaterOuiMode != CASUALLY_DEFINE_MAC_ADDR) {
					tmpIdx = REPT_MLME_START_IDX + pMacEntry->MatchReptCliIdx;
					convertPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, tmpIdx, pTxBlk->OpMode);
					pMacAddr = &pAd->ApCfg.pRepeaterCliPool[pMacEntry->MatchReptCliIdx].CurrentAddress[0];
				}
			} else
#endif /* MAC_REPEATER_SUPPORT */
			{
				/* For each tx packet, update our MAT convert engine databases.*/

#ifdef APCLI_AS_WDS_STA_SUPPORT
				if (pAd->StaCfg[pMacEntry->func_tb_idx].wdev.wds_enable == 0)
#endif /* APCLI_AS_WDS_STA_SUPPORT */
				convertPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, pMacEntry->func_tb_idx, pTxBlk->OpMode);
				pMacAddr = &pAd->StaCfg[pMacEntry->func_tb_idx].wdev.if_addr[0];
			}

		if (convertPkt) {
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			pPacket = convertPkt;
			pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
			pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
			pTxBlk->pPacket = convertPkt;
		}

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
			if ((pMacAddr != NULL)
			   ) {
				if (pMacAddr) {
					pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
#ifdef APCLI_AS_WDS_STA_SUPPORT
				if (pAd->StaCfg[pMacEntry->func_tb_idx].wdev.wds_enable == 0)
#endif /* APCLI_AS_WDS_STA_SUPPORT */

						NdisMoveMemory(pSrcBufVA + 6, pMacAddr, MAC_ADDR_LEN);
				}
			}
		}

#endif /* MAT_SUPPORT */
		pTxBlk->pApCliEntry = &pAd->StaCfg[pMacEntry->func_tb_idx];
		pTxBlk->pMacEntry = pMacEntry;
		TX_BLK_SET_FLAG(pTxBlk, fTX_bApCliPacket);
#ifdef MAC_REPEATER_SUPPORT

		if (pMacEntry->bReptCli)
			pMacEntry->ReptCliIdleCount = 0;

#endif
	}

	pTxBlk->wmm_set = HcGetWmmIdx(pAd, wdev);
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	return TRUE;
}
/*
    ==========================================================================
    Description:
	APCLI Interface Up.
    ==========================================================================
 */
VOID ApCliIfUp(RTMP_ADAPTER *pAd)
{
	UCHAR ifIndex;
	STA_ADMIN_CONFIG *pApCliEntry;
#ifdef APCLI_CONNECTION_TRIAL
	PULONG pCurrState = NULL;
#endif /* APCLI_CONNECTION_TRIAL */
	struct DOT11_H *pDot11h = NULL;

	/* Reset is in progress, stop immediately */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_RADIO_OFF) ||
		(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)))
		return;

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++) {
		pApCliEntry = &pAd->StaCfg[ifIndex];


		/* sanity check whether the interface is initialized. */
		if (pApCliEntry->ApcliInfStat.ApCliInit != TRUE) {
			continue;
		}

		if (pApCliEntry->wdev.if_up_down_state == FALSE) {
			continue;
		}

#ifdef APCLI_CONNECTION_TRIAL
		pCurrState = &pAd->StaCfg[ifIndex].wdev.cntl_machine.CurrState;
#endif /* APCLI_CONNECTION_TRIAL */

		if (!pApCliEntry->wdev.DevInfo.WdevActive) {
			continue;
		}


		if (!HcIsRadioAcq(&pApCliEntry->wdev)) {
			continue;
		}


		if (APCLI_IF_UP_CHECK(pAd, ifIndex)
			&& (pApCliEntry->ApcliInfStat.Enable == TRUE)
			&& (pApCliEntry->ApcliInfStat.Valid == FALSE)
#ifdef APCLI_CONNECTION_TRIAL
			&& (ifIndex != (pAd->ApCfg.ApCliNum - 1)) /* last IF is for apcli connection trial */
#endif /* APCLI_CONNECTION_TRIAL */
		   ) {
			pDot11h = pApCliEntry->wdev.pDot11_H;

			if (pDot11h == NULL) {
				return;
			}

			if (pDot11h->RDMode == RD_SWITCHING_MODE) {
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
								("%s:Switching mode: Continue\n", __func__));
				continue;
			}
			if (IS_DOT11_H_RADAR_STATE(pAd, RD_SILENCE_MODE, pApCliEntry->wdev.channel, pDot11h)) {
				if (pApCliEntry->ApcliInfStat.bPeerExist == TRUE) {
					/* Got peer's beacon; change to normal mode */
					pDot11h->RDCount = pDot11h->ChMovingTime;
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
							 ("ApCliIfUp - PeerExist\n"));
				} else
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
							 ("ApCliIfUp - Stop probing while Radar state is silent\n"));

				continue;
			}

#ifdef WSC_INCLUDED
			if (pApCliEntry->wdev.WscControl.bWscTrigger
				&& ((pApCliEntry->wdev.WscControl.WscStatus == STATUS_WSC_SCAN_AP) ||
					(pApCliEntry->wdev.WscControl.WscStatus == STATUS_WSC_PBC_NO_AP)))
				continue;
#endif /* WSC_INCLUDED */
			MTWF_LOG(DBG_CAT_ALL, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) ApCli interface[%d] startup.\n", __func__, ifIndex));
#ifdef WSC_INCLUDED
			if ((pApCliEntry->wdev.WscControl.bWscTrigger) &&
				(pApCliEntry->wdev.WscControl.WscStatus == STATUS_WSC_START_ASSOC)) {
				MTWF_LOG(DBG_CAT_ALL, CATCLIENT_APCLI, DBG_LVL_OFF, ("%s: Enqueue connect by BSSID for WPS\n", __func__));
				cntl_connect_request(&pApCliEntry->wdev, CNTL_CONNECT_BY_BSSID, MAC_ADDR_LEN,
								(UCHAR *)&pApCliEntry->wdev.WscControl.WscBssid[0]);
			} else
#endif /* WSC_INCLUDED */
{
			cntl_connect_request(&pApCliEntry->wdev, CNTL_CONNECT_BY_CFG, 0, NULL);
}
			/* MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ, 0, NULL, ifIndex); */
			/* Reset bPeerExist each time in case we could keep old status */
			pApCliEntry->ApcliInfStat.bPeerExist = FALSE;
		}

#ifdef APCLI_CONNECTION_TRIAL
		else if (
			APCLI_IF_UP_CHECK(pAd, ifIndex)
			&& (*pCurrState == CNTL_IDLE)/* Apcli1 is not connected state. */
			&& (pApCliEntry->TrialCh != 0)
			&& (pApCliEntry->ApcliInfStat.Valid == FALSE)
			/* && NdisCmpMemory(pApCliEntry->ApCliMlmeAux.Ssid, pApCliEntry->CfgSsid, pApCliEntry->SsidLen) != 0 */
			&& (pApCliEntry->CfgSsidLen != 0)
			&& (pApCliEntry->ApcliInfStat.Enable != 0)
			/* new ap ssid shall different from the origin one. */
		) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Enqueue APCLI_CTRL_TRIAL_CONNECT\n", __func__));
			trial_cntl_connect_request(&pApCliEntry->wdev, CNTL_CONNECT_BY_CFG, 0, NULL);
			/* Reset bPeerExist each time in case we could keep old status */
			pApCliEntry->ApcliInfStat.bPeerExist = FALSE;
		}

#endif /* APCLI_CONNECTION_TRIAL */
	}

	return;
}

/*
    ==========================================================================
    Description:
	APCLI Interface Down.
    ==========================================================================
 */
VOID ApCliIfDown(RTMP_ADAPTER *pAd)
{
	UCHAR ifIndex;
	PSTA_ADMIN_CONFIG pApCliEntry;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx, idx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* MAC_REPEATER_SUPPORT */

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++) {
		pApCliEntry = &pAd->StaCfg[ifIndex];

		if (pApCliEntry->ApcliInfStat.Enable == TRUE)
			continue;

		if (pApCliEntry->ApcliInfStat.Valid == FALSE)
			continue;

		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("%s():ApCli interface[%d] start down.\n", __func__, ifIndex));
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

		if (wf_drv_tbl.wf_fwd_entry_delete_hook)
			wf_drv_tbl.wf_fwd_entry_delete_hook(pApCliEntry->wdev.if_dev, pAd->net_dev, 1);

#endif /* CONFIG_WIFI_PKT_FWD */
#ifdef MAC_REPEATER_SUPPORT

		if (pAd->ApCfg.bMACRepeaterEn) {
			for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
				pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

				/*disconnect the ReptEntry which is bind on the CliLink*/
				if ((pReptEntry->CliEnable) && (pReptEntry->wdev == &pApCliEntry->wdev)) {
					struct inter_machine_info priv_data;

					priv_data.rept_cli_entry = pReptEntry;
					priv_data.rept_cli_idx = CliIdx;
					RTMP_OS_INIT_COMPLETION(&pReptEntry->free_ack);
					pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_IF_DOWN;
					cntl_disconnect_request(pReptEntry->wdev,
											CNTL_DISASSOC,
											pApCliEntry->Bssid,
											REASON_DISASSOC_STA_LEAVING,
											&priv_data);
					ReptWaitLinkDown(pReptEntry);
				}
			}
		}

#endif /* MAC_REPEATER_SUPPORT */
		RTMP_OS_INIT_COMPLETION(&pApCliEntry->linkdown_complete);
		pApCliEntry->ApcliInfStat.Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_IF_DOWN;
		cntl_disconnect_request(&pApCliEntry->wdev,
								CNTL_DISASSOC,
								pApCliEntry->Bssid,
								REASON_DISASSOC_STA_LEAVING,
								NULL);
		sta_wait_link_down(pApCliEntry);
#ifdef AUTOMATION
		pApCliEntry->PwrSaveSet = FALSE;
#endif /* AUTOMATION */
	}

#ifdef MAC_REPEATER_SUPPORT

	for (idx = 0; idx < MAX_IGNORE_AS_REPEATER_ENTRY_NUM; idx++) {
		pEntry = &pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntry[idx];

		if (pAd->ApCfg.ApCliInfRunned == 0)
			RepeaterRemoveIngoreEntry(pAd, idx, pEntry->MacAddr);
	}

#endif /* MAC_REPEATER_SUPPORT */
	return;
}

#ifdef APCLI_AUTO_CONNECT_SUPPORT
#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
UCHAR ApCliAutoConnectBWAdjust(
	IN RTMP_ADAPTER	*pAd,
	IN struct wifi_dev	*wdev,
	IN BSS_ENTRY *bss_entry)
{
	BOOLEAN bAdjust = FALSE;
	BOOLEAN bAdjust_by_channel = FALSE;
	BOOLEAN bAdjust_by_ht = FALSE;
	BOOLEAN bAdjust_by_vht = FALSE;
	UCHAR	orig_op_ht_bw;
#ifdef DOT11_VHT_AC
	UCHAR	orig_op_vht_bw;
#endif
	UCHAR	orig_ext_cha;
	STA_ADMIN_CONFIG *pStaCfg = NULL;
	struct _RTMP_CHIP_CAP *cap = NULL;

	if (pAd == NULL || wdev == NULL || bss_entry == NULL) {
		ASSERT(pAd);
		ASSERT(wdev);
		ASSERT(bss_entry);
		return AUTO_BW_PARAM_ERROR;
	}

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	ASSERT(pStaCfg);
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			 ("BW info of root AP (%s):\n", bss_entry->Ssid));
	orig_op_ht_bw = wlan_operate_get_ht_bw(wdev);
#ifdef DOT11_VHT_AC
	orig_op_vht_bw = wlan_operate_get_vht_bw(wdev);
#endif /*DOT11_VHT_AC*/
	orig_ext_cha = wlan_operate_get_ext_cha(wdev);

	if ((wdev->channel != bss_entry->Channel)) {
		bAdjust = TRUE;
		bAdjust_by_channel = TRUE;
	}

#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode) && (bss_entry->AddHtInfoLen != 0)) {
		ADD_HTINFO *add_ht_info = &bss_entry->AddHtInfo.AddHtInfo;
		UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
		UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
		UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);

		if (!bAdjust &&
			((ext_cha != add_ht_info->ExtChanOffset) ||
			 (op_ht_bw != add_ht_info->RecomWidth)))
			bAdjust = TRUE;

		if (bAdjust) {
		switch (add_ht_info->RecomWidth) { /* peer side vht bw */
			case BW_20:
				if (op_ht_bw == BW_40) {
					wlan_operate_set_ht_bw(wdev, add_ht_info->RecomWidth, EXTCHA_NONE);
					bAdjust_by_ht = TRUE;
				}

				break;
			case BW_40:
				if (op_ht_bw == BW_20) {
#ifdef BT_APCLI_SUPPORT
					if (pAd->ApCfg.ApCliAutoBWBTSupport == TRUE) {
						/*set to config extension channel/bw to let ap use new configuration*/
						UCHAR mbss_idx = 0;
						/*Moving both AP and CLI to 40Mhz since RootAP is working in 40Mhz */
						for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
							struct wifi_dev *mbss_wdev;

							mbss_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;

							if (HcGetBandByWdev(mbss_wdev) ==
									HcGetBandByWdev(wdev)) {
								wlan_config_set_ht_bw(mbss_wdev,
										add_ht_info->RecomWidth);
								wlan_config_set_ext_cha(mbss_wdev,
										add_ht_info->ExtChanOffset);
							}
						}
						/*set Config BW of CLI to 40Mhz*/
						wlan_config_set_ht_bw(wdev, add_ht_info->RecomWidth);
						wlan_operate_set_ht_bw(wdev, add_ht_info->RecomWidth,
								add_ht_info->ExtChanOffset);
						wlan_config_set_ext_cha(wdev, add_ht_info->ExtChanOffset);
						bAdjust_by_ht = TRUE;
					}
#endif
				} else {
					if (cfg_ht_bw == BW_40) {

						UCHAR mbss_idx = 0;
						for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
							struct wifi_dev *mbss_wdev;
							mbss_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
							if (HcGetBandByWdev(mbss_wdev) == HcGetBandByWdev(wdev)) {
								wlan_config_set_ext_cha(mbss_wdev, add_ht_info->ExtChanOffset);
							}
						}
					wlan_config_set_ext_cha(wdev, add_ht_info->ExtChanOffset);
					bAdjust_by_ht = TRUE;
					}
				}
				break;
			}
		}

	}

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode) && IS_CAP_BW160(cap) && (bss_entry->vht_cap_len != 0) && (bss_entry->vht_op_len != 0)) {
		BOOLEAN bResetVHTBw = FALSE, bDownBW = FALSE;
		UCHAR bw = VHT_BW_2040;
		struct vht_opinfo *vht_op = &bss_entry->vht_op_ie.vht_op_info;
		UCHAR op_vht_bw = wlan_operate_get_vht_bw(wdev);
		UCHAR cfg_vht_bw = wlan_config_get_vht_bw(wdev);
		UCHAR BandIdx = HcGetBandByWdev(wdev);
		CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
#ifdef DOT11_VHT_R2
		bw = check_vht_op_bw(vht_op);
#else
		bw = vht_op->ch_width;
		print_vht_op_info(vht_op);
#endif /* DOT11_VHT_R2 */

		if (!bAdjust &&
			(bw != op_vht_bw))
			bAdjust = TRUE;

		if (bAdjust) {
			switch (bw) {/* peer side vht bw */
			case VHT_BW_2040:
				if (cfg_vht_bw > VHT_BW_2040) {
					bResetVHTBw = TRUE;
					bDownBW = TRUE;
					bAdjust_by_vht = TRUE;
				}

				break;

			case VHT_BW_80:
				if (cfg_vht_bw > VHT_BW_80) {
					bResetVHTBw = TRUE;
					bDownBW = TRUE;
					bAdjust_by_vht = TRUE;
				}

				break;

			case VHT_BW_160:
				if (cfg_vht_bw == VHT_BW_160) {
					bAdjust_by_vht = TRUE;
					bResetVHTBw = 1;
				}

				break;

			case VHT_BW_8080:
				if (cfg_vht_bw == VHT_BW_8080) {
					wlan_operate_set_cen_ch_2(wdev, vht_op->ccfs_1);
					bResetVHTBw = 1;
					bAdjust_by_vht = TRUE;
				}

				break;
			}
		}

		if (bResetVHTBw) {
			INT Idx = -1;
			BOOLEAN bMatch = FALSE;

			for (Idx = 0; Idx < pChCtrl->ChListNum; Idx++) {
				if (bss_entry->Channel == pChCtrl->ChList[Idx].Channel) {
					bMatch = TRUE;
					break;
				}
			}

			if (bMatch && (Idx < MAX_NUM_OF_CHANNELS)) {
				if (bDownBW) {
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_OFF,
							 ("(%s): Follow BW info of root AP (%s) from vht_bw = %d to %d. (MAX=%d)\n",
							  __func__, bss_entry->Ssid,
							  op_vht_bw, bw,
							  cfg_vht_bw));
					wlan_operate_set_vht_bw(wdev, bw);
				} else if (!bDownBW && (pChCtrl->ChList[Idx].Flags & CHANNEL_80M_CAP))
					wlan_operate_set_vht_bw(wdev, cfg_vht_bw);

				wlan_operate_set_cen_ch_2(wdev, vht_op->ccfs_1);
			}
		}
	}

#endif /* DOT11_VHT_AC */
	bAdjust = FALSE;

	if (bAdjust_by_channel == TRUE)
		bAdjust = TRUE;

	if (bAdjust_by_ht == TRUE)
		bAdjust = TRUE;

	if (bAdjust_by_vht == TRUE)
		bAdjust = TRUE;

	if (bAdjust) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s:Adjust (%d %d %d)\n\r", __func__,
				 bAdjust_by_channel, bAdjust_by_ht, bAdjust_by_vht));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s:HT BW:%d to %d. MAX(%d)\n\r", __func__,
				 orig_op_ht_bw, wlan_operate_get_ht_bw(wdev), wlan_config_get_ht_bw(wdev)));
#ifdef DOT11_VHT_AC
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s:VHT BW:%d to %d. MAX(%d)\n\r", __func__,
				 orig_op_vht_bw, wlan_operate_get_vht_bw(wdev), wlan_config_get_vht_bw(wdev)));
#endif /*DOT11_VHT_AC*/
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s:EXT CH:%d to %d\n\r", __func__,
				 orig_ext_cha, wlan_operate_get_ext_cha(wdev)));
		return AUTO_BW_NEED_TO_ADJUST;
	}

	return AUTO_BW_NO_NEED_TO_ADJUST;
}
#endif /* APCLI_AUTO_BW_TMP */
/*
	===================================================

	Description:
		Find the AP that is configured in the ApcliTab, and switch to
		the channel of that AP

	Arguments:
		pAd: pointer to our adapter

	Return Value:
		TRUE: no error occured
		FALSE: otherwise

	Note:
	===================================================
*/
BOOLEAN ApCliAutoConnectExec(
	IN  PRTMP_ADAPTER   pAd,
	IN struct wifi_dev *wdev)
{
	UCHAR			ifIdx, CfgSsidLen, entryIdx;
	RTMP_STRING *pCfgSsid;
	BSS_TABLE		*pScanTab, *pSsidBssTab;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT old_ioctl_if;
	INT old_ioctl_if_type;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#ifdef CONFIG_OWE_SUPPORT
	PSTA_ADMIN_CONFIG papcli_entry = NULL;
    BSS_TABLE   *powe_bss_tab = NULL;
#endif

	ASSERT(pStaCfg);
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("---> ApCliAutoConnectExec()\n"));

	if (wdev)
		ifIdx = wdev->func_idx;
	else
		return FALSE;

	if (ifIdx >= MAX_APCLI_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Error  ifIdx=%d\n", ifIdx));
		return FALSE;
	}

	if (pStaCfg->ApcliInfStat.AutoConnectFlag != TRUE)
		return FALSE;

	CfgSsidLen = pStaCfg->CfgSsidLen;
	pCfgSsid = pStaCfg->CfgSsid;
	pScanTab = get_scan_tab_by_wdev(pAd, wdev);
	pSsidBssTab = &pStaCfg->MlmeAux.SsidBssTab;
	pSsidBssTab->BssNr = 0;

#ifdef CONFIG_OWE_SUPPORT
    papcli_entry = &pAd->StaCfg[ifIdx];
    if (IS_AKM_OWE(wdev->SecConfig.AKMMap)) {

		powe_bss_tab = &papcli_entry->MlmeAux.owe_bss_tab;
		powe_bss_tab->BssNr = 0;

		BssTableInit(powe_bss_tab);

		sta_reset_owe_parameters(pAd, ifIdx);

		/*
		   Find out APs with the OWE transition IE and store in owe_bss_tab*
		 */
		for (entryIdx = 0; entryIdx < pScanTab->BssNr; entryIdx++) {
			BSS_ENTRY *pBssEntry = &pScanTab->BssEntry[entryIdx];

			if (pBssEntry->Channel == 0)
				continue;

			if ((pBssEntry->owe_trans_ie_len > 0) &&
				(powe_bss_tab->BssNr < MAX_LEN_OF_BSS_TABLE)) {
				NdisMoveMemory(&powe_bss_tab->BssEntry[powe_bss_tab->BssNr++],
								pBssEntry, sizeof(BSS_ENTRY));
			}
		}

		if (powe_bss_tab->BssNr < MAX_LEN_OF_BSS_TABLE)
			NdisZeroMemory(&powe_bss_tab->BssEntry[powe_bss_tab->BssNr], sizeof(BSS_ENTRY));
	}
#endif

	/*
		Find out APs with the desired SSID.
	*/
	for (entryIdx = 0; entryIdx < pScanTab->BssNr; entryIdx++) {
		BSS_ENTRY *pBssEntry = &pScanTab->BssEntry[entryIdx];

		if (pBssEntry->Channel == 0)
			break;

		if (NdisEqualMemory(pCfgSsid, pBssEntry->Ssid, CfgSsidLen) &&
			pBssEntry->SsidLen &&
			(pBssEntry->SsidLen == CfgSsidLen) &&
			(pSsidBssTab->BssNr < MAX_LEN_OF_BSS_TABLE)) {
			if (((((wdev->SecConfig.AKMMap & pBssEntry->AKMMap) != 0) ||
				(IS_AKM_AUTOSWITCH(wdev->SecConfig.AKMMap) && IS_AKM_SHARED(pBssEntry->AKMMap)) ||
				(IS_AKM_WPA2PSK_ONLY(pBssEntry->AKMMap) && IS_AKM_WPA3PSK_ONLY(wdev->SecConfig.AKMMap)))
				 && ((wdev->SecConfig.PairwiseCipher & pBssEntry->PairwiseCipher) != 0))
#ifdef CONFIG_OWE_SUPPORT
					|| (!pBssEntry->hide_open_owe_bss
						&& (IS_AKM_OPEN_ONLY(pBssEntry->AKMMap) && IS_CIPHER_NONE(pBssEntry->PairwiseCipher))
						&& (IS_AKM_OWE(wdev->SecConfig.AKMMap)))
#endif
			   ) {
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
						 ("Found desired ssid in Entry %2d:\n", entryIdx));
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
						 ("I/F(apcli%d) ApCliAutoConnectExec:(Len=%d,Ssid=%s, Channel=%d, Rssi=%d)\n",
						  ifIdx, pBssEntry->SsidLen, pBssEntry->Ssid,
						  pBssEntry->Channel, pBssEntry->Rssi));
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
						 ("I/F(apcli%d) ApCliAutoConnectExec::(AuthMode=%s, EncrypType=%s)\n", ifIdx,
						  GetAuthMode(pBssEntry->AuthMode),
						  GetEncryptType(pBssEntry->WepStatus)));
				NdisMoveMemory(&pSsidBssTab->BssEntry[pSsidBssTab->BssNr++],
							   pBssEntry, sizeof(BSS_ENTRY));
			}
		}
	}
	if (pSsidBssTab->BssNr < MAX_LEN_OF_BSS_TABLE)
		NdisZeroMemory(&pSsidBssTab->BssEntry[pSsidBssTab->BssNr], sizeof(BSS_ENTRY));
	/*
		Sort by Rssi in the increasing order, and connect to
		the last entry (strongest Rssi)
	*/
	BssTableSortByRssi(pSsidBssTab, TRUE);

	if ((pSsidBssTab->BssNr == 0)) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("No match entry.\n"));
		pStaCfg->ApCliAutoConnectRunning = FALSE;
	} else if (pSsidBssTab->BssNr > 0 &&
			   pSsidBssTab->BssNr <= MAX_LEN_OF_BSS_TABLE) {
		/*
			Switch to the channel of the candidate AP
		*/
		BSS_ENTRY *pBssEntry = &pSsidBssTab->BssEntry[pSsidBssTab->BssNr - 1];
#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
		BOOLEAN bw_adj;

		bw_adj = ApCliAutoConnectBWAdjust(pAd, wdev, pBssEntry);

		if (bw_adj == AUTO_BW_NEED_TO_ADJUST || (!IS_INVALID_HT_SECURITY(pBssEntry->PairwiseCipher)))
#endif /* APCLI_AUTO_BW_TMP */
		{
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Switch to channel :%d\n", pBssEntry->Channel));
			rtmp_set_channel(pAd, wdev, pBssEntry->Channel);
		}
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("Error! Out of table range: (BssNr=%d).\n", pSsidBssTab->BssNr));
		RtmpOSNetDevProtect(1);
		old_ioctl_if = pObj->ioctl_if;
		old_ioctl_if_type = pObj->ioctl_if_type;
		pObj->ioctl_if = ifIdx;
		pObj->ioctl_if_type = INT_APCLI;
		Set_ApCli_Enable_Proc(pAd, "1");
		pObj->ioctl_if = old_ioctl_if;
		pObj->ioctl_if_type = old_ioctl_if_type;
		RtmpOSNetDevProtect(0);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("<--- ApCliAutoConnectExec()\n"));
		return FALSE;
	}

	RtmpOSNetDevProtect(1);
	old_ioctl_if = pObj->ioctl_if;
	old_ioctl_if_type = pObj->ioctl_if_type;
	pObj->ioctl_if = ifIdx;
	pObj->ioctl_if_type = INT_APCLI;
	Set_ApCli_Enable_Proc(pAd, "1");
	pObj->ioctl_if = old_ioctl_if;
	pObj->ioctl_if_type = old_ioctl_if_type;
	RtmpOSNetDevProtect(0);
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("<--- ApCliAutoConnectExec()\n"));
	return TRUE;
}
/*
	===================================================

	Description:
		If the previous selected entry connected failed, this function will
		choose next entry to connect. The previous entry will be deleted.

	Arguments:
		pAd: pointer to our adapter

	Note:
		Note that the table is sorted by Rssi in the "increasing" order, thus
		the last entry in table has stringest Rssi.
	===================================================
*/
VOID ApCliSwitchCandidateAP(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
	BSS_TABLE		*pSsidBssTab;
	PSTA_ADMIN_CONFIG	pApCliEntry;
	UCHAR			lastEntryIdx, ifIdx;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT old_ioctl_if;
	INT old_ioctl_if_type;
	SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	if (ScanCtrl->PartialScan.bScanning == TRUE)
		return;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("---> ApCliSwitchCandidateAP()\n"));

	if (wdev)
		ifIdx = wdev->func_idx;
	else
		return;

	if (ifIdx >= MAX_APCLI_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Error  ifIdx=%d\n", ifIdx));
		return;
	}

	if (pAd->StaCfg[ifIdx].ApcliInfStat.AutoConnectFlag != TRUE)
		return;

	pApCliEntry = &pAd->StaCfg[ifIdx];
	pSsidBssTab = &pApCliEntry->MlmeAux.SsidBssTab;

	if (pSsidBssTab->BssNr == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("No Bss\n"));
		pApCliEntry->ApCliAutoConnectRunning = FALSE;
		return;
	}

	/*
		delete (zero) the previous connected-failled entry and always
		connect to the last entry in talbe until the talbe is empty.
	*/
	NdisZeroMemory(&pSsidBssTab->BssEntry[--pSsidBssTab->BssNr], sizeof(BSS_ENTRY));
	lastEntryIdx = pSsidBssTab->BssNr - 1;

	if ((pSsidBssTab->BssNr > 0) && (pSsidBssTab->BssNr < MAX_LEN_OF_BSS_TABLE)) {
		BSS_ENTRY *pBssEntry = &pSsidBssTab->BssEntry[pSsidBssTab->BssNr - 1];
#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
		BOOLEAN bw_adj;

		bw_adj = ApCliAutoConnectBWAdjust(pAd, wdev, pBssEntry);

		if (bw_adj)
#endif /* APCLI_AUTO_BW_TMP */
		{
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Switch to channel :%d\n", pBssEntry->Channel));
			rtmp_set_channel(pAd, wdev, pBssEntry->Channel);
		}
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("No candidate AP, the process is about to stop.\n"));
		pApCliEntry->ApCliAutoConnectRunning = FALSE;
	}

	RtmpOSNetDevProtect(1);
	old_ioctl_if = pObj->ioctl_if;
	old_ioctl_if_type = pObj->ioctl_if_type;
	pObj->ioctl_if = ifIdx;
	pObj->ioctl_if_type = INT_APCLI;
	Set_ApCli_Enable_Proc(pAd, "1");
	pObj->ioctl_if = old_ioctl_if;
	pObj->ioctl_if_type = old_ioctl_if_type;
	RtmpOSNetDevProtect(0);
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("---> ApCliSwitchCandidateAP()\n"));
}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
VOID apcli_dync_txop_alg(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UINT tx_tp,
	UINT rx_tp)
{
#define COND3_COOL_DOWN_TIME 240

	if (!pAd || !wdev) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s:: pAd or wdev is NULL!\n",
				  __func__));
	} else {
		INT i;
		BOOLEAN cond[NUM_OF_APCLI_TXOP_COND] = {FALSE};
		UINT16 cond_txop_level[NUM_OF_APCLI_TXOP_COND] = {0};
		UINT16 cond_thrd[NUM_OF_APCLI_TXOP_COND] = {0};
		UINT16 txop_level = TXOP_0;
		UINT16 current_txop_level = TXOP_0;
		BOOLEAN apcli_txop_en = FALSE;
		STA_ADMIN_CONFIG *apcli_entry = NULL;

		cond_txop_level[1] = TXOP_30;
		cond_txop_level[2] = TXOP_FE;
		cond_txop_level[3] = TXOP_80;
		cond_thrd[1] = TP_PEEK_BOUND_THRESHOLD;
		cond_thrd[2] = TX_MODE_TP_CHECK;
		current_txop_level = wdev->bss_info_argument.txop_level[PRIO_APCLI_REPEATER];
		apcli_entry = &pAd->StaCfg[wdev->func_idx];

		/* if cond_1 is taking effect, adjust the threshold to prevent instability */
		if (current_txop_level == cond_txop_level[1]) {
			/* Adjust cond_thrd[1] */
			if (apcli_entry->dync_txop_histogram[1] >= 4) {
				UINT32 tolerance_adjust_factor = 10;
				UINT32 tolerance_adjust_value = 0;

				tolerance_adjust_value = TOLERANCE_OF_TP_THRESHOLD +
										 (apcli_entry->dync_txop_histogram[1] * tolerance_adjust_factor);

				if (tolerance_adjust_value > 150)
					tolerance_adjust_value = 150;

				cond_thrd[1] = TP_PEEK_BOUND_THRESHOLD - tolerance_adjust_value;
			} else
				cond_thrd[1] = TP_PEEK_BOUND_THRESHOLD - TOLERANCE_OF_TP_THRESHOLD;

			/* Check if t.p. has degrade right after apply cond1 */
			if (tx_tp <= (TP_PEEK_BOUND_THRESHOLD - TOLERANCE_OF_TP_THRESHOLD) &&
				apcli_entry->dync_txop_histogram[1] < 4) {
				/* If t.p. is bad right after cond1, we trigger cond3 to recover old txop */
				cond[3] = TRUE;
			}
		} else if (current_txop_level == cond_txop_level[2]) {
			/* if cond_2 is taking effect, adjust the threshold to prevent instability */
			cond_thrd[2] = TX_MODE_TP_CHECK - TOLERANCE_OF_TP_THRESHOLD;
		}

		if (tx_tp > cond_thrd[1])
			cond[1] = TRUE;
		else if (tx_tp > cond_thrd[2]  && WMODE_CAP_2G(wdev->PhyMode)) {
			/* We don't check "divided by 0" because the "if condition" already do that */
			UINT tx_ratio = (tx_tp * 100) / (tx_tp + rx_tp);

			if (tx_ratio > TX_MODE_RATIO_THRESHOLD)
				cond[2] = TRUE;
		}

		if (apcli_entry->dync_txop_histogram[3] != 0) {
			cond[3] = TRUE;
			txop_level = cond_txop_level[3];
			apcli_txop_en = TRUE;

			if (tx_tp < TP_PEEK_BOUND_THRESHOLD) {
				/* If cond3 triggered but t.p cannot keep high, we raise the decade rate */
				UINT8 cond3_decade_factor = 0;
				UINT32 cond3_accumulate_value = 0;

				cond[4] = TRUE;
				cond3_decade_factor = (1 << apcli_entry->dync_txop_histogram[4]); /* exponential decade */
				cond3_accumulate_value = apcli_entry->dync_txop_histogram[3] + cond3_decade_factor;
				apcli_entry->dync_txop_histogram[3] =
					(cond3_accumulate_value > COND3_COOL_DOWN_TIME) ?
					(COND3_COOL_DOWN_TIME) : cond3_accumulate_value;
			}
		} else if (cond[1] == TRUE) {
			txop_level = cond_txop_level[1];
			apcli_txop_en = TRUE;
		} else if (cond[2] == TRUE) {
			txop_level = cond_txop_level[2];
			apcli_txop_en = TRUE;
		} else {
			txop_level = TXOP_0;
			apcli_txop_en = FALSE;
		}

		if (txop_level != current_txop_level) {
			if (apcli_txop_en == TRUE)
				enable_tx_burst(pAd, wdev, AC_BE, PRIO_APCLI_REPEATER, txop_level);
			else
				disable_tx_burst(pAd, wdev, AC_BE, PRIO_APCLI_REPEATER, txop_level);
		}

		/* update histogram */
		for (i = 0; i < NUM_OF_APCLI_TXOP_COND; i++) {
			if (cond[i] == TRUE)
				apcli_entry->dync_txop_histogram[i]++;
			else
				apcli_entry->dync_txop_histogram[i] = 0;
		}

		/* clear histogram */
		if (apcli_entry->dync_txop_histogram[3] > COND3_COOL_DOWN_TIME)
			apcli_entry->dync_txop_histogram[3] = 0;
	}
}

BOOLEAN isValidApCliIf(SHORT if_idx)
{
	return (((if_idx >= 0) && (if_idx < MAX_APCLI_NUM)) ? TRUE : FALSE);
}


/*! \brief init the management mac frame header
 *  \param p_hdr mac header
 *  \param subtype subtype of the frame
 *  \param p_ds destination address, don't care if it is a broadcast address
 *  \return none
 *  \pre the station has the following information in the pAd->UserCfg
 *   - bssid
 *   - station address
 *  \post
 *  \note this function initializes the following field
 */
VOID ApCliMgtMacHeaderInit(
	IN RTMP_ADAPTER *pAd,
	INOUT HEADER_802_11 *pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN UCHAR *pDA,
	IN UCHAR *pBssid,
	IN USHORT ifIndex)
{
	NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
	pHdr80211->FC.Type = FC_TYPE_MGMT;
	pHdr80211->FC.SubType = SubType;
	pHdr80211->FC.ToDs = ToDs;
	COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
	COPY_MAC_ADDR(pHdr80211->Addr2, pAd->StaCfg[ifIndex].wdev.if_addr);
	COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
}


/*
    ==========================================================================
    Description:
	APCLI Interface Monitor.
    ==========================================================================
 */


VOID ApCliCheckPeerExistence(RTMP_ADAPTER *pAd, CHAR *Ssid, UCHAR SsidLen, UCHAR Channel)
{
	UCHAR ifIndex;
	STA_ADMIN_CONFIG *pApCliEntry;
	struct wifi_dev *wdev = NULL;

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++) {
		pApCliEntry = &pAd->StaCfg[ifIndex];
		wdev = &pApCliEntry->wdev;

		if (pApCliEntry->ApcliInfStat.bPeerExist == TRUE)
			continue;
		else if (Channel == pApCliEntry->wdev.channel &&
				 ((SsidLen == pApCliEntry->CfgSsidLen && NdisEqualMemory(Ssid, pApCliEntry->CfgSsid, SsidLen)) ||
				  (SsidLen == 0 /* Hidden */)
#ifdef WSC_AP_SUPPORT
				  || ((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
						  (wdev->WscControl.bWscTrigger == TRUE) &&
						  NdisEqualMemory(Ssid, wdev->WscControl.WscSsid.Ssid, SsidLen))
#endif
				 ))
			pApCliEntry->ApcliInfStat.bPeerExist = TRUE;
		else {
			/* No Root AP match the SSID */
		}
	}
}
struct wifi_dev_ops apcli_wdev_ops = {
	.tx_pkt_allowed = apcli_tx_pkt_allowed,
	.fp_tx_pkt_allowed = apcli_fp_tx_pkt_allowed,
	.send_data_pkt = ap_send_data_pkt,
	.fp_send_data_pkt = fp_send_data_pkt,
	.send_mlme_pkt = ap_send_mlme_pkt,
	.tx_pkt_handle = ap_tx_pkt_handle,
	.fill_non_offload_tx_blk = apcli_fill_non_offload_tx_blk,
	.fill_offload_tx_blk = apcli_fill_offload_tx_blk,
	.legacy_tx = ap_legacy_tx,
	.ampdu_tx = ap_ampdu_tx,
	.frag_tx = ap_frag_tx,
	.amsdu_tx = ap_amsdu_tx,
	.mlme_mgmtq_tx = ap_mlme_mgmtq_tx,
	.mlme_dataq_tx = ap_mlme_dataq_tx,
#ifdef CONFIG_ATE
	.ate_tx = mt_ate_tx,
#endif
	.ieee_802_11_data_tx = ap_ieee_802_11_data_tx,
	.ieee_802_3_data_tx = ap_ieee_802_3_data_tx,
	.rx_pkt_allowed = sta_rx_pkt_allow,
	.rx_pkt_foward = sta_rx_fwd_hnd,
	.ieee_802_3_data_rx = ap_ieee_802_3_data_rx,
	.ieee_802_11_data_rx = ap_ieee_802_11_data_rx,
	.find_cipher_algorithm = ap_find_cipher_algorithm,
	/* snowpin for ap/sta ++ */
	.media_state_connected = NULL,/* media_state_connected, */
	.ioctl = rt28xx_ap_ioctl,
	.mac_entry_lookup = mac_entry_lookup,
	/* snowpin for ap/sta -- */
	.rx_announce_802_3_pkt = sta_rx_announce_802_3_pkt,
	.rx_looping_detect = apcli_rx_looping_detect,
};

INT ApCliIfLookUp(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	SHORT if_idx;

	for (if_idx = 0; if_idx < MAX_APCLI_NUM; if_idx++) {
		if (MAC_ADDR_EQUAL(pAd->StaCfg[if_idx].wdev.if_addr, pAddr)) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s():ApCliIfIndex=%d\n",
					 __func__, if_idx));
			return if_idx;
		}
	}

	return -1;
}

/*
    ==========================================================================
    Description:
	APCLI Interface Monitor.
	EZ_SETUP please contruct a new function for ApCliIfMonitor
    ==========================================================================
 */
VOID ApCliIfMonitor(RTMP_ADAPTER *pAd)
{
	UCHAR index;
	STA_ADMIN_CONFIG *pApCliEntry;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* Reset is in progress, stop immediately */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
		!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return;

	for (index = 0; index < MAX_APCLI_NUM; index++) {
		UCHAR Wcid;
		PMAC_TABLE_ENTRY pMacEntry;
		STA_TR_ENTRY *tr_entry;
		BOOLEAN bForceBrocken = FALSE;
		BOOLEAN bWpa_4way_too_log = FALSE;
		BOOLEAN bBeacon_miss = FALSE;

		pApCliEntry = &pAd->StaCfg[index];

		if (!pApCliEntry->wdev.DevInfo.WdevActive)
			continue;

		if (scan_in_run_state(pAd, &pApCliEntry->wdev) == TRUE)
			continue;

#ifdef APCLI_CONNECTION_TRIAL

		if (index == (pAd->ApCfg.ApCliNum - 1)) /* last IF is for apcli connection trial */
			continue;/* skip apcli1 monitor. FIXME:Carter shall find a better way. */

#endif /* APCLI_CONNECTION_TRIAL */

		/* sanity check whether the interface is initialized. */
		if (pApCliEntry->ApcliInfStat.ApCliInit != TRUE)
			continue;

#ifdef MAC_REPEATER_SUPPORT
		RepeaterLinkMonitor(pAd);
#endif /* MAC_REPEATER_SUPPORT */

		if (pApCliEntry->ApcliInfStat.Valid == TRUE) {
			BOOLEAN ApclibQosNull = FALSE;

			Wcid = pAd->StaCfg[index].MacTabWCID;

			if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
				continue;

			pMacEntry = &pAd->MacTab.Content[Wcid];
			tr_entry = &pAd->MacTab.tr_entry[Wcid];

			if ((IS_AKM_WPA_CAPABILITY(pMacEntry->SecConfig.AKMMap))
				&& (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)
				&& (RTMP_TIME_AFTER(pAd->Mlme.Now32, (pApCliEntry->ApcliInfStat.ApCliLinkUpTime + (30 * OS_HZ))))) {
				bWpa_4way_too_log = TRUE;
				bForceBrocken = TRUE;
			}

			if (RTMP_TIME_AFTER(pAd->Mlme.Now32, (pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime +
					 (12 * OS_HZ)))) {
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("STA Beacon loss condition got hit.\n"));
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
				pMacEntry->bTxPktChk = FALSE;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
				bBeacon_miss = TRUE;
				bForceBrocken = TRUE;
			}

			if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
				ApclibQosNull = TRUE;
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
			if (bWpa_4way_too_log == TRUE) {
				if (IS_AKM_SAE_SHA256(pApCliEntry->AKMMap) || IS_AKM_OWE(pApCliEntry->AKMMap)) {
					UCHAR pmkid[80];
					UCHAR pmk[LEN_PMK];
					INT cached_idx;
					UCHAR if_index = pApCliEntry->wdev.func_idx;
					UCHAR cli_idx = 0xFF;

					/* Connection taking too long update PMK cache  and delete sae instance*/
					if (
#ifdef DOT11_SAE_SUPPORT
						(IS_AKM_SAE_SHA256(pApCliEntry->AKMMap) &&
						 sae_get_pmk_cache(&pAd->SaeCfg, pApCliEntry->wdev.if_addr, pApCliEntry->MlmeAux.Bssid, pmkid, pmk))
#endif

#ifdef CONFIG_OWE_SUPPORT
						|| IS_AKM_OWE(pApCliEntry->AKMMap)
#endif
					   ) {

						cached_idx = sta_search_pmkid_cache(pAd, pApCliEntry->MlmeAux.Bssid, if_index, cli_idx);
						if (cached_idx != INVALID_PMKID_IDX) {
#ifdef DOT11_SAE_SUPPORT
							SAE_INSTANCE *pSaeIns = search_sae_instance(&pAd->SaeCfg, pApCliEntry->wdev.if_addr, pApCliEntry->MlmeAux.Bssid);
							MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
										("Reconnection falied with pmkid ,delete cache entry and sae instance \n"));

							if (pSaeIns != NULL)
								delete_sae_instance(pSaeIns);
#endif
							sta_delete_pmkid_cache(pAd, pApCliEntry->MlmeAux.Bssid, if_index, cli_idx);
						}
					}
				}
			}
#endif
			if ((bForceBrocken == FALSE)
#ifdef CONFIG_MULTI_CHANNEL
				&& (pAd->Mlme.bStartMcc == FALSE)
#endif /* CONFIG_MULTI_CHANNEL */
			   )
					ApCliRTMPSendNullFrame(pAd, pMacEntry->CurrTxRate, ApclibQosNull, pMacEntry, PWR_ACTIVE);
		} else
			continue;

		if (bForceBrocken == TRUE) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCliIfMonitor: IF(apcli%d) - no Beancon is received from root-AP.\n", index));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCliIfMonitor: Reconnect the Root-Ap again.\n"));
#ifdef CONFIG_MULTI_CHANNEL

			if (pAd->Mlme.bStartMcc == TRUE)
				return;

#endif /* CONFIG_MULTI_CHANNEL */

			if (bBeacon_miss) {
				ULONG Now32;

				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCliIfMonitor apcli%d time1: %lu\n", index, pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime_MlmeEnqueueForRecv));
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCliIfMonitor apcli%d time2: %lu\n", index, pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime_MlmeEnqueueForRecv_2));
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCliIfMonitor apcli%d time3: %lu\n", index, pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime));
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCliIfMonitor apcli%d OS_HZ: %d\n", index, OS_HZ));
				NdisGetSystemUpTime(&Now32);
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCliIfMonitor apcli%d time now: %lu\n", index, Now32));
			}

#ifdef MAC_REPEATER_SUPPORT

			if (pAd->ApCfg.bMACRepeaterEn) {
				STA_ADMIN_CONFIG *apcli_entry = pApCliEntry;
				REPEATER_CLIENT_ENTRY *pReptEntry;
				UCHAR CliIdx;

				for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
					pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

					if ((pReptEntry->CliEnable) && (pReptEntry->wdev == &apcli_entry->wdev)) {
						struct inter_machine_info priv_data;

						priv_data.rept_cli_entry = pReptEntry;
						priv_data.rept_cli_idx = CliIdx;
						cntl_disconnect_request(pReptEntry->wdev,
												CNTL_DISASSOC,
												pReptEntry->wdev->bssid,
												REASON_DISASSOC_STA_LEAVING,
												&priv_data);

						if (bBeacon_miss)
							pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_MNT_NO_BEACON;
						else
							pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_TRIGGER_TOO_LONG;
					}
				}
			}

#endif /* MAC_REPEATER_SUPPORT */

			if (bBeacon_miss)
				pApCliEntry->ApcliInfStat.Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_MNT_NO_BEACON;
			else
				pApCliEntry->ApcliInfStat.Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_TRIGGER_TOO_LONG;

			cntl_disconnect_request(&pApCliEntry->wdev,
									CNTL_DISASSOC,
									pApCliEntry->Bssid,
									REASON_DISASSOC_STA_LEAVING,
									NULL);

		}
	}

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_LOUD, ("ra offload=%d\n", cap->fgRateAdaptFWOffload));
}

#endif /*CONFIG_APSTA_MIXED_SUPPORT*/

#ifdef DOT11_SAE_SUPPORT
INT set_apcli_sae_group_proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj;
	UCHAR *pSaeCfgGroup = NULL;
	UCHAR group = 0;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pSaeCfgGroup = &pAd->StaCfg[pObj->ioctl_if].sae_cfg_group;


	group = os_str_tol(arg, 0, 10);

	if ((group == 19) || (group == 20)) {
		*pSaeCfgGroup = (UCHAR) group;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[SAE]%s:: Set group=%d \n",
					__func__, group));

	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[SAE]%s:: group=%d not supported\n",
					__func__, group));
		return FALSE;
	}

	return TRUE;
}
#endif/*DOT11_SAE_SUPPORT*/

#ifdef CONFIG_OWE_SUPPORT
INT set_apcli_owe_group_proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj;
	UCHAR group = 0;
	UCHAR *pcurr_group = NULL;

	if (0 == strlen(arg))
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pcurr_group = &pAd->StaCfg[pObj->ioctl_if].curr_owe_group;


	group = os_str_tol(arg, 0, 10);
	/*OWE-currently allowing configuration of groups 19(mandatory) and 20(optional) */
	if ((group == 19) || (group == 20)) {
		*pcurr_group = (UCHAR) group;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[OWE]%s:: Set group=%d \n",
					__func__, group));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[OWE]%s:: group=%d not supported\n",
					__func__, group));
				return FALSE;
	}
	return TRUE;
}
#endif/*CONFIG_OWE_SUPPORT*/

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
INT set_apcli_del_pmkid_list(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj;
	UCHAR action = 0;

	if (0 == strlen(arg))
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	action = os_str_tol(arg, 0, 10);

	/*Delete all pmkid list associated with this  ApCli Interface*/
	if (action == 1) {
		sta_delete_pmkid_cache_all(pAd, pObj->ioctl_if);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:: Delete PMKID list (%d)  \n",
					__func__, action));

	}
	return TRUE;
}

INT sta_add_pmkid_cache(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR *paddr,
	IN UCHAR *pmkid,
	IN UCHAR *pmk,
	IN UINT8 pmk_len,
	IN UINT8 if_index,
	IN UINT8 cli_idx)
{
	PSTA_ADMIN_CONFIG papcli_entry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY preptcli_entry = NULL;
#endif
	INT cached_idx;
	PBSSID_INFO psaved_pmk = NULL;
	PUINT psaved_pmk_num = NULL;
	UCHAR update_pmkid = FALSE;
	VOID *psaved_pmk_lock = NULL;

#ifdef MAC_REPEATER_SUPPORT
	if (cli_idx != 0xff) {
		preptcli_entry = &pAd->ApCfg.pRepeaterCliPool[cli_idx];
		papcli_entry = &pAd->StaCfg[if_index];
		psaved_pmk = (PBSSID_INFO)&preptcli_entry->SavedPMK[0];
		psaved_pmk_num = &preptcli_entry->SavedPMKNum;
		psaved_pmk_lock = (VOID *)&preptcli_entry->SavedPMK_lock;
	} else
#endif
    {
		papcli_entry = &pAd->StaCfg[if_index];
		psaved_pmk = (PBSSID_INFO)&papcli_entry->SavedPMK[0];
		psaved_pmk_num = &papcli_entry->SavedPMKNum;
		psaved_pmk_lock = (VOID *)&papcli_entry->SavedPMK_lock;
	}

	cached_idx = sta_search_pmkid_cache(pAd, paddr, if_index, cli_idx);

	if (psaved_pmk_lock)
		NdisAcquireSpinLock(psaved_pmk_lock);

	if (cached_idx != INVALID_PMKID_IDX) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF,
					("%s :PMKID found, %d\n", __func__, cached_idx));
	} else {
			/* Find free cache entry */
		for (cached_idx = 0; cached_idx < PMKID_NO; cached_idx++) {
			if (psaved_pmk[cached_idx].Valid == FALSE)
				break;
		}

		if (cached_idx < PMKID_NO) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF,
						("Free Cache entry found,cached_idx %d\n", cached_idx));
			*psaved_pmk_num = *psaved_pmk_num + 1;
		} else {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF,
						("cache full, overwrite cached_idx 0\n"));
			cached_idx = 0;
		}
		update_pmkid = TRUE;
	}

	if (update_pmkid == TRUE) {
		psaved_pmk[cached_idx].Valid = TRUE;
		COPY_MAC_ADDR(&psaved_pmk[cached_idx].BSSID, paddr);
		NdisMoveMemory(&psaved_pmk[cached_idx].PMKID, pmkid, LEN_PMKID);
		NdisMoveMemory(&psaved_pmk[cached_idx].PMK, pmk, pmk_len);
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s(): add %02x:%02x:%02x:%02x:%02x:%02x cache(%d)\n",
					 __func__, PRINT_MAC(paddr), cached_idx));
	}

	if (psaved_pmk_lock)
		NdisReleaseSpinLock(psaved_pmk_lock);

	return cached_idx;
}

INT sta_search_pmkid_cache(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR *paddr,
	IN UCHAR if_index,
	IN UCHAR cli_idx)
{
	INT i = 0;
	PBSSID_INFO psaved_pmk = NULL;
	PSTA_ADMIN_CONFIG papcli_entry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY preptcli_entry = NULL;
#endif
	VOID *psaved_pmk_lock = NULL;

#ifdef MAC_REPEATER_SUPPORT
	if (cli_idx != 0xff) {
		preptcli_entry = &pAd->ApCfg.pRepeaterCliPool[cli_idx];
		papcli_entry = &pAd->StaCfg[if_index];
		psaved_pmk = (PBSSID_INFO)&preptcli_entry->SavedPMK[0];
		psaved_pmk_lock = (VOID *)&preptcli_entry->SavedPMK_lock;
	} else
#endif
	{
		papcli_entry = &pAd->StaCfg[if_index];
		psaved_pmk = (PBSSID_INFO)&papcli_entry->SavedPMK[0];
		psaved_pmk_lock = (VOID *)&papcli_entry->SavedPMK_lock;

	}

	if (psaved_pmk_lock)
		NdisAcquireSpinLock(psaved_pmk_lock);

	for (i = 0; i < PMKID_NO; i++) {
		if ((psaved_pmk[i].Valid == TRUE)
			&& MAC_ADDR_EQUAL(&psaved_pmk[i].BSSID, paddr)) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("%s():%02x:%02x:%02x:%02x:%02x:%02x cache(%d)\n",
						 __func__, PRINT_MAC(paddr), i));
			break;
		}
	}

	if (psaved_pmk_lock)
		NdisReleaseSpinLock(psaved_pmk_lock);

	if (i >= PMKID_NO) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s():  not found\n", __func__));
		return INVALID_PMKID_IDX;
	}

	return i;
}

VOID sta_delete_pmkid_cache(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR *paddr,
	IN UCHAR if_index,
	IN UCHAR cli_idx)
{

	INT cached_idx;
	PBSSID_INFO psaved_pmk = NULL;
	PSTA_ADMIN_CONFIG papcli_entry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY preptcli_entry = NULL;
#endif
	VOID *psaved_pmk_lock = NULL;
	PUINT psaved_pmk_num = NULL;


#ifdef MAC_REPEATER_SUPPORT
	if (cli_idx != 0xff) {
		preptcli_entry = &pAd->ApCfg.pRepeaterCliPool[cli_idx];
		psaved_pmk = (PBSSID_INFO)&preptcli_entry->SavedPMK[0];
		psaved_pmk_num = &preptcli_entry->SavedPMKNum;
		psaved_pmk_lock = (VOID *)&preptcli_entry->SavedPMK_lock;


	} else
#endif
	{
		papcli_entry = &pAd->StaCfg[if_index];
		psaved_pmk = (PBSSID_INFO)&papcli_entry->SavedPMK[0];
		psaved_pmk_num = &papcli_entry->SavedPMKNum;
		psaved_pmk_lock = (VOID *)&papcli_entry->SavedPMK_lock;

	}


	cached_idx = sta_search_pmkid_cache(pAd, paddr, if_index, cli_idx);

	if (cached_idx != INVALID_PMKID_IDX) {
		if (psaved_pmk_lock)
			NdisAcquireSpinLock(psaved_pmk_lock);

		if (psaved_pmk[cached_idx].Valid == TRUE) {
			psaved_pmk[cached_idx].Valid = FALSE;

			if (*psaved_pmk_num)
				*psaved_pmk_num = *psaved_pmk_num - 1;
		}
		if (psaved_pmk_lock)
			NdisReleaseSpinLock(psaved_pmk_lock);
	}
}

VOID sta_delete_pmkid_cache_all(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR if_index)
{

	INT cli_idx = 0;
	INT cached_idx;
	PBSSID_INFO psaved_pmk = NULL;
	PSTA_ADMIN_CONFIG papcli_entry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY preptcli_entry = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
#ifdef DOT11_SAE_SUPPORT
	SAE_INSTANCE *pSaeIns = NULL;
	SAE_CFG *pSaeCfg = NULL;
	UINT32 i;
	UINT32 ins_cnt = 0;
#endif

	VOID *psaved_pmk_lock = NULL;
	PUINT psaved_pmk_num = NULL;
	papcli_entry = &pAd->StaCfg[if_index];
	psaved_pmk = (PBSSID_INFO)&papcli_entry->SavedPMK[0];
	psaved_pmk_num = &papcli_entry->SavedPMKNum;
	psaved_pmk_lock = (VOID *)&papcli_entry->SavedPMK_lock;

#ifdef DOT11_SAE_SUPPORT
	pSaeCfg = &pAd->SaeCfg;
	/*Delete all SAE instances for this ApCli Interface*/
	NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		if (pSaeCfg->sae_ins[i].valid == FALSE)
			continue;

		if (RTMPEqualMemory(pSaeCfg->sae_ins[i].own_mac, papcli_entry->wdev.if_addr, MAC_ADDR_LEN)) {
			pSaeIns = &pSaeCfg->sae_ins[i];
			if (pSaeIns != NULL && (pSaeIns->valid == TRUE)) {
				NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
				delete_sae_instance(pSaeIns);
				NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);
			}
		}
		ins_cnt++;
		if (ins_cnt == pSaeCfg->total_ins)
			break;
	}

	NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
#endif
	/*Delete ApCli PMKID list*/
	for (cached_idx = 0; cached_idx < PMKID_NO; cached_idx++) {
		if (psaved_pmk_lock)
			NdisAcquireSpinLock(psaved_pmk_lock);

		if (psaved_pmk[cached_idx].Valid == TRUE) {
			psaved_pmk[cached_idx].Valid = FALSE;

			if (*psaved_pmk_num)
				*psaved_pmk_num = *psaved_pmk_num - 1;
		}
		if (psaved_pmk_lock)
			NdisReleaseSpinLock(psaved_pmk_lock);

	}
	/* Delete  PMKID list for MacRepeater linked with ApCli */

#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
		for (cli_idx = 0; cli_idx < GET_MAX_REPEATER_ENTRY_NUM(cap); cli_idx++) {
			preptcli_entry = &pAd->ApCfg.pRepeaterCliPool[cli_idx];

			if (preptcli_entry && (preptcli_entry->CliValid == TRUE) && (preptcli_entry->MatchApCliIdx == papcli_entry->wdev.func_idx)) {

#ifdef DOT11_SAE_SUPPORT
				/* Delete all SAE instances for this Rept entry */
				NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);
				ins_cnt = 0;

				for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
					if (pSaeCfg->sae_ins[i].valid == FALSE)
						continue;

					if (RTMPEqualMemory(pSaeCfg->sae_ins[i].own_mac, preptcli_entry->CurrentAddress, MAC_ADDR_LEN)) {
						pSaeIns = &pSaeCfg->sae_ins[i];
						if (pSaeIns != NULL && (pSaeIns->valid == TRUE)) {
							NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
							delete_sae_instance(pSaeIns);
							NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

						}
					}

					ins_cnt++;

					if (ins_cnt == pSaeCfg->total_ins)
						break;
				}

				NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
#endif
				psaved_pmk = (PBSSID_INFO)&preptcli_entry->SavedPMK[0];
				psaved_pmk_num = &preptcli_entry->SavedPMKNum;
				psaved_pmk_lock = (VOID *)&preptcli_entry->SavedPMK_lock;


				for (cached_idx = 0; cached_idx < PMKID_NO; cached_idx++) {
					if (psaved_pmk_lock)
						NdisAcquireSpinLock(psaved_pmk_lock);

					if (psaved_pmk[cached_idx].Valid == TRUE) {
						psaved_pmk[cached_idx].Valid = FALSE;

						if (*psaved_pmk_num)
							*psaved_pmk_num = *psaved_pmk_num - 1;
					}

					if (psaved_pmk_lock)
						NdisReleaseSpinLock(psaved_pmk_lock);
				}
			}
		}
	}
#endif
}
#endif

#ifdef CONFIG_OWE_SUPPORT
VOID sta_reset_owe_parameters(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR if_index)
{
	PSTA_ADMIN_CONFIG papcli_entry = NULL;
	papcli_entry = &pAd->StaCfg[if_index];

	/*OWE Trans reset the OWE trans bssid and ssid*/

	if (papcli_entry
		&& IS_AKM_OWE(papcli_entry->wdev.SecConfig.AKMMap)
		&& (papcli_entry->owe_trans_ssid_len > 0)) {
		NdisZeroMemory(papcli_entry->owe_trans_bssid, MAC_ADDR_LEN);
		NdisZeroMemory(papcli_entry->owe_trans_ssid, MAX_LEN_OF_SSID);
		papcli_entry->owe_trans_ssid_len = 0;

		NdisZeroMemory(papcli_entry->owe_trans_open_bssid, MAC_ADDR_LEN);
		NdisZeroMemory(papcli_entry->owe_trans_open_ssid, MAX_LEN_OF_SSID);
		papcli_entry->owe_trans_open_ssid_len = 0;
	}
}

BOOLEAN sta_handle_owe_trans(
			IN  PRTMP_ADAPTER   pAd,
			IN struct wifi_dev *wdev,
			IN BSS_ENTRY *pInBss
			)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	if (IS_AKM_OWE(wdev->SecConfig.AKMMap) && pInBss->owe_trans_ie_len > 0) {
		UCHAR pair_ch = 0;
		UCHAR pair_bssid[MAC_ADDR_LEN] = {0};
		UCHAR pair_ssid[MAX_LEN_OF_SSID] = {0};
		UCHAR pair_band = 0;
		UCHAR pair_ssid_len = 0;
		extract_pair_owe_bss_info(pInBss->owe_trans_ie, pInBss->owe_trans_ie_len, pair_bssid, pair_ssid, &pair_ssid_len, &pair_ch);
		if (pInBss->RsnIE.IELen == 0) {
			if ((WMODE_CAP_2G(pStaCfg->wdev.PhyMode) && (pair_ch <= 14)) || (WMODE_CAP_5G(pStaCfg->wdev.PhyMode) && (pair_ch > 14))) {
				if (pair_ch != 0) {
					if (pair_ch != pStaCfg->wdev.channel) {
						wext_send_owe_trans_chan_event(pStaCfg->wdev.if_dev,
									OID_802_11_OWE_EVT_SAME_BAND_DIFF_CHANNEL,
									pair_bssid,
									pair_ssid,
									&pair_ssid_len,
									&pair_band,
									&pair_ch);
						rtmp_set_channel(pAd, wdev, pair_ch);
					}
				}
			} else {
				if (pair_ch != 0) {
					wext_send_owe_trans_chan_event(pStaCfg->wdev.if_dev,
									OID_802_11_OWE_EVT_DIFF_BAND,
									pair_bssid,
									pair_ssid,
									&pair_ssid_len,
									&pair_band,
									&pair_ch);
					rtmp_set_channel(pAd, wdev, pair_ch);
				}
			}
			NdisMoveMemory(&pStaCfg->owe_trans_bssid, pair_bssid, MAC_ADDR_LEN);
			NdisMoveMemory(&pStaCfg->owe_trans_ssid, pair_ssid, pair_ssid_len);
			pStaCfg->owe_trans_ssid_len = pair_ssid_len;
			NdisMoveMemory(&pStaCfg->owe_trans_open_bssid, pInBss->Bssid, MAC_ADDR_LEN);
			NdisMoveMemory(&pStaCfg->owe_trans_open_ssid, pInBss->Ssid, pInBss->SsidLen);
			pStaCfg->owe_trans_open_ssid_len = pInBss->SsidLen;
			CLEAR_SEC_AKM(pStaCfg->MlmeAux.AKMMap);
			CLEAR_CIPHER(pStaCfg->MlmeAux.PairwiseCipher);
			CLEAR_CIPHER(pStaCfg->MlmeAux.GroupCipher);
			return TRUE;
		} else {
			if (NdisEqualMemory(pStaCfg->owe_trans_open_bssid, pair_bssid, MAC_ADDR_LEN)) {
				if (NdisEqualMemory(pStaCfg->owe_trans_open_ssid, pair_ssid, pStaCfg->owe_trans_open_ssid_len)) {
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL,
						DBG_LVL_OFF, ("Sanity Check Pass,BSS Parameters and Current Open parameters in Owe Trans IE match\n"));
					return FALSE;
				} else {
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL,
						DBG_LVL_ERROR, ("Sanity Failed Stored Open BSS Parameters and Current Open parameters in Owe Trans IE don't match\n"));
					return TRUE;
					}
			}
		}
	}
	return FALSE;
}
#endif
