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
	rtmp_init.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include	"rt_config.h"

#ifdef MT_DFS_SUPPORT
#include "mt_rdm.h"
#endif /*MT_DFS_SUPPORT*/

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
#include "phy/rlm_cal_cache.h"
#endif /* RLM_CAL_CACHE_SUPPORT */


#ifdef OS_ABL_FUNC_SUPPORT
/* Os utility link: printk, scanf */
RTMP_OS_ABL_OPS RaOsOps, *pRaOsOps = &RaOsOps;
#endif /* OS_ABL_FUNC_SUPPORT */

UCHAR NUM_BIT8[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
#ifdef DBG
char *CipherName[] = {"none", "wep64", "wep128", "TKIP", "AES", "CKIP64", "CKIP128", "CKIP152", "SMS4", "WEP152"};
#endif


NDIS_STATUS RTMPAllocGlobalUtility(VOID)
{
#ifdef OS_ABL_FUNC_SUPPORT
	/* must put the function before any print message */
	/* init OS utilities provided from UTIL module */
	RtmpOsOpsInit(&RaOsOps);
#endif /* OS_ABL_FUNC_SUPPORT */
	/* init UTIL module */
	RtmpUtilInit();
	return NDIS_STATUS_SUCCESS;
}

/*
 * SpaceX: Generates output to procfile to provide info about each wifi band (2.4 and 5 GHz).
 */
static INT band_info_show(struct seq_file *m, void *v) {
	RTMP_ADAPTER *pAd = m->private;
    INT i;
    BOOL isFirstEntry = TRUE;

    seq_printf(m, "{\n");
    UCHAR concurrent_bands = HcGetAmountOfBand(pAd);
    for (i = 0; i < concurrent_bands; i++) {
        if (!(isFirstEntry))
            seq_printf(m, ",\n");
        isFirstEntry = FALSE;

        if(i == 0) {
            seq_printf(m, "\t\"rf_2ghz\": {\n");
        } else {
            seq_printf(m, "\t\"rf_5ghz\": {\n");
        }

        seq_printf(m, "\t\t\"chan_busy_time_us_last_s\":%d,\n", pAd->OneSecMibBucket.ChannelBusyTime[i]);
        seq_printf(m, "\t\t\"tx_air_time_us_last_s\":%d,\n", pAd->OneSecMibBucket.MyTxAirtime[i]);
        seq_printf(m, "\t\t\"rx_air_time_us_last_s\":%d,\n", pAd->OneSecMibBucket.MyRxAirtime[i]);
        seq_printf(m, "\t\t\"obss_air_time_us_last_s\":%d,\n", pAd->OneSecMibBucket.OBSSAirtime[i]);
		seq_printf(m, "\t\t\"edcca_air_time_us_last_s\":%d,\n", pAd->OneSecMibBucket.EDCCAtime[i]);
		seq_printf(m, "\t\t\"pd_count_last_s\":%d,\n", pAd->OneSecMibBucket.PdCount[i]);
		seq_printf(m, "\t\t\"mdrdy_count_last_s\":%d,\n", pAd->OneSecMibBucket.MdrdyCount[i]);
		seq_printf(m, "\t\t\"tx_fail\":%lld,\n", pAd->WlanCounters[i].FailedCount);
		seq_printf(m, "\t\t\"tx_success\":%lld,\n", pAd->WlanCounters[i].TransmittedFragmentCount);
		seq_printf(m, "\t\t\"rx_fail\":%lld,\n", pAd->WlanCounters[i].FCSErrorCount);
		seq_printf(m, "\t\t\"rx_success\":%lld\n", pAd->WlanCounters[i].ReceivedFragmentCount);
        seq_printf(m, "\t}\n");
    }
    seq_printf(m, "}\n");
	return 0;
}

/*
 * SpaceX: Open callback that is called when the band_info_open proc file is opened.
 */
static INT band_info_open(struct inode *inode, struct file *file) {
	RTMP_ADAPTER *pAd = PDE_DATA(file_inode(file));
  	return single_open(file, band_info_show, pAd);
}

/*
 * SpaceX: File operations structure for band_info file manipulation callbacks.
 */
static const struct file_operations band_info_fops = {
	.open = band_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release, // Must use single_release when using single_open
};

// Output is normally around 40k for 128 clients, so making this 128k should be more than enough
// and future proofs against adding fields
#define SX_CLIENT_PROC_STR_LEN 128 * 1024

/*
 * SpaceX: Generates the output of assoc_clients proc file when opened.
 */
INT assoc_clients_show(struct seq_file *m, void *v) {
	RTMP_ADAPTER *pAd = m->private;
	INT i;
	BOOL isFirstEntry = TRUE;
    INT bytesLeft = SX_CLIENT_PROC_STR_LEN - 1;
    char* msg;
    
    os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR) * SX_CLIENT_PROC_STR_LEN);
	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Alloc memory failed\n", __func__));
        return 1;
	}
    memset(msg, 0, sizeof(CHAR) * SX_CLIENT_PROC_STR_LEN);

    bytesLeft -= snprintf(msg, bytesLeft,
                        "{\n"
                        "\t\"clients\": {\n");
        
	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
        PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
        if (IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
            ULONG txRate, rxRate;
            UINT8 txAnt, rxAnt;
            UINT8 txBW, rxBW;
            UINT8 txMCS, rxMCS;
            UINT16 txGuardNS, rxGuardNS;
            HTTRANSMIT_SETTING LastTxRate;
            HTTRANSMIT_SETTING LastRxRate;
            INT bw;
            INT snr;
            INT signalStrength;
            char* clientStr;

            signalStrength = (pEntry->RssiSample.AvgRssi[0] + pEntry->RssiSample.AvgRssi[1] + pEntry->RssiSample.AvgRssi[2])/3;
        
            /*
            * SpaceX: Noise offset for GetClients' SNR.
            * Obtained from [https://stash/projects/SAT/repos/starlink-wifi-mtk/pull-requests/106/diff#openwrt/package/mtk/applications/mapd/src/mapd\_lib/apEstimator/ap\_est_lib.c](https://stash/projects/SAT/repos/starlink-wifi-mtk/pull-requests/106/diff#openwrt/package/mtk/applications/mapd/src/mapd_lib/apEstimator/ap_est_lib.c)
            */
            switch (pEntry->HTPhyMode.field.BW) {
            case BW_20:
                bw = 20;
                snr = signalStrength + 94;
                break;
            case BW_40:
                bw = 40;
                snr = signalStrength + 91;
                break;
            case BW_80:
                bw = 80;
                snr = signalStrength + 88;
                break;
            case BW_160:
                bw = 160;
                snr = signalStrength + 85;
                break;
            default:
                bw = -1;
                snr = -1;
            }

            LastTxRate.word = RTMPGetLastTxRate(pAd, pEntry);
            LastRxRate.word = (USHORT)pEntry->LastRxRate;
            getRateEtc(LastTxRate, &txRate, &txAnt, &txBW, &txMCS, &txGuardNS);
            getRateEtc(LastRxRate, &rxRate, &rxAnt, &rxBW, &rxMCS, &rxGuardNS);

            clientStr = "%s"
                        "\t\t\"%02x:%02x:%02x:%02x:%02x:%02x\": {\n"
                        "\t\t\t\"mac_address\": \"%02x:%02x:%02x:%02x:%02x:%02x\",\n"
                        "\t\t\t\"signal_strength\": %d,\n"
                        "\t\t\t\"channel_width\": %d,\n"
                        "\t\t\t\"snr\": %d,\n"
                        "\t\t\t\"associated_time_s\": %d,\n"
                        "\t\t\t\"iface\": \"%s\",\n"
                        "\t\t\t\"psmode\": %d,\n"
						"\t\t\t\"swq_checks\": %d,\n"
                        "\t\t\t\"swq_checks_non_empty\": %d,\n"
                        "\t\t\t\"rx_stats\": {\n"
                        "\t\t\t\t\"nss\": %d,\n"
                        "\t\t\t\t\"rate_mbps\": %lu,\n"
                        "\t\t\t\t\"bytes\": %lld,\n"
                        "\t\t\t\t\"mcs\": %d,\n"
                        "\t\t\t\t\"guard_ns\": %d,\n"
                        "\t\t\t\t\"bandwidth\": %d\n"
                        "\t\t\t},\n"
                        "\t\t\t\"tx_stats\": {\n"
                        "\t\t\t\t\"nss\": %d,\n"
                        "\t\t\t\t\"rate_mbps\": %lu,\n"
                        "\t\t\t\t\"bytes\": %lld,\n"
                        "\t\t\t\t\"mcs\": %d,\n"
                        "\t\t\t\t\"guard_ns\": %u,\n"
                        "\t\t\t\t\"bandwidth\": %d\n"
                        "\t\t\t}\n"
                        "\t\t}\n";
            bytesLeft -= snprintf(msg + SX_CLIENT_PROC_STR_LEN - 1 - bytesLeft, bytesLeft,
                clientStr, 
                isFirstEntry ? "" : ",\n",
                PRINT_MAC(pEntry->Addr),
                PRINT_MAC(pEntry->Addr),
                signalStrength,
                bw,
                snr,
                pEntry->StaConnectTime,
                pEntry->wdev->channel > 14 ? "RF_5GHZ" : "RF_2GHZ",
                (int)pEntry->PsMode,
				pEntry->SwqChecks,
				pEntry->SwqChecksNonEmpty,

                /*
                * RX stats
                * NOTE: Using TX stats because they are from the perspective of the AP (bytes transmitted by AP to station).
                */
                txAnt,
                txRate,
                pEntry->TxBytes,
                txMCS,
                txGuardNS,
                txBW,

                /*
                * TX stats
                * Note: using RX stats because they are from the perspective of the AP (bytes received at AP from station).
                */
                rxAnt,
                rxRate,
                pEntry->RxBytes,
                rxMCS,
                rxGuardNS,
                rxBW
            );

            isFirstEntry = FALSE;
        }
    }

	snprintf(msg + SX_CLIENT_PROC_STR_LEN - 1 - bytesLeft, bytesLeft,
            "\t}\n"
            "}\n");

    seq_printf(m, msg);
    os_free_mem(msg);
	return 0;
}

/*
 * SpaceX: Open callback that is called when the assoc_clients proc file is opened.
 */
static INT assoc_clients_open(struct inode *inode, struct  file *file) {
	RTMP_ADAPTER *pAd = PDE_DATA(file_inode(file));
  	return single_open(file, assoc_clients_show, pAd);
}

/*
 * SpaceX: File operations structure for assoc_clients file manipulation callbacks.
 */
static const struct file_operations assoc_clients_fops = {
	.open = assoc_clients_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release, // Must use single_release when using single_open
};

/*
 * SpaceX: Generates the output of assoc_connections proc file when opened,
 * which contains the upstream router this device connects to over apcli0/apclix0.
 * See RTMPIoctlConnStatus() in cmm_info.c for reference.
 * In the future, you can dereference pAd and pEntry for more information about the connection.
 * See assoc_clients_show() above for reference.
 */
static INT assoc_connections_show(struct seq_file *m, void *v) {
	RTMP_ADAPTER *pAd = m->private;
	int i;
	struct wifi_dev *wdev = NULL;
	BOOLEAN firstEntry = TRUE;

	seq_printf(m, "{\n");
	seq_printf(m, "\t\"connections\": {\n");

	// Iterate over apcli0 and apclix0, indexed 0 and 1, respectively, to find any upstream connections.
	int ifIndex;
	for (ifIndex = 0; ifIndex < 2; ifIndex++) {
		wdev = &pAd->StaCfg[ifIndex].wdev;

		if (((GetAssociatedAPByWdev(pAd, wdev)) != NULL) && (pAd->StaCfg[ifIndex].SsidLen != 0)) {
			for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
				PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
				STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[i];

				if (IS_ENTRY_PEER_AP(pEntry)
					&& (pEntry->Sst == SST_ASSOC)
					&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
					if (pEntry->wdev == &pAd->StaCfg[ifIndex].wdev) {
						if (!(firstEntry))
							seq_printf(m, ",\n");
						seq_printf(m, "\t\t\"%02x:%02x:%02x:%02x:%02x:%02x\": {\n", PRINT_MAC(pEntry->Addr));
						seq_printf(m, "\t\t\t\"mac_address\": \"%02x:%02x:%02x:%02x:%02x:%02x\",\n", PRINT_MAC(pEntry->Addr));
						seq_printf(m, "\t\t\t\"ssid\": \"%s\",\n", pAd->StaCfg[ifIndex].Ssid);
						if (pEntry->wdev->channel > 14)
							seq_printf(m, "\t\t\t\"iface\": \"RF_5GHZ\"\n");
						else
							seq_printf(m, "\t\t\t\"iface\": \"RF_2GHZ\"\n");
						seq_printf(m, "\t\t}\n");
						firstEntry = FALSE;
					}
				}
			}
		}
	}

	seq_printf(m, "\t}\n");
	seq_printf(m, "}\n");
	return 0;
}

/*
 * SpaceX: Open callback that is called when the assoc_connections proc file is opened.
 */
static INT assoc_connections_open(struct inode *inode, struct  file *file) {
	RTMP_ADAPTER *pAd = PDE_DATA(file_inode(file));
  	return single_open(file, assoc_connections_show, pAd);
}

/*
 * SpaceX: File operations structure for assoc_connections file manipulation callbacks.
 */
static const struct file_operations assoc_connections_fops = {
	.open = assoc_connections_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release, // Must use single_release when using single_open
};

/*
	========================================================================

	Routine Description:
		Allocate RTMP_ADAPTER data block and do some initialization

	Arguments:
		Adapter		Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS RTMPAllocAdapterBlock(VOID *handle, VOID **ppAdapter, INT type)
{
	RTMP_ADAPTER *pAd = NULL;
	NDIS_STATUS	 Status;
	INT index;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> RTMPAllocAdapterBlock\n"));
	RTMPAllocGlobalUtility();
	*ppAdapter = NULL;

	do {
		/* Allocate RTMP_ADAPTER memory block*/
		Status = AdapterBlockAllocateMemory(handle, (PVOID *)&pAd, sizeof(RTMP_ADAPTER));

		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Failed to allocate memory - ADAPTER\n"));
			break;
		} else {
			/* init resource list (must be after pAd allocation) */
			initList(&pAd->RscTimerMemList);
			initList(&pAd->RscTaskMemList);
			initList(&pAd->RscLockMemList);
			initList(&pAd->RscTaskletMemList);
			initList(&pAd->RscSemMemList);
			initList(&pAd->RscAtomicMemList);
			initList(&pAd->RscTimerCreateList);
			pAd->OS_Cookie = handle;
			((POS_COOKIE)(handle))->pAd_va = (LONG)pAd;
		}

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\n=== pAd = %p, size = %zu ===\n\n", pAd,
				 sizeof(RTMP_ADAPTER)));

		if (RtmpOsStatsAlloc(&pAd->stats, &pAd->iw_stats) == FALSE) {
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		/*Allocate Timer Lock*/
		NdisAllocateSpinLock(pAd, &pAd->TimerSemLock);
		/* Init spin locks*/
		NdisAllocateSpinLock(pAd, &pAd->WdevListLock);
		NdisAllocateSpinLock(pAd, &pAd->BssInfoIdxBitMapLock);
		NdisAllocateSpinLock(pAd, &pAd->irq_lock);
#ifdef CONFIG_FWOWN_SUPPORT
		NdisAllocateSpinLock(pAd, &pAd->DriverOwnLock);
#endif /* CONFIG_FWOWN_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
		NdisAllocateSpinLock(pAd, &pAd->ApCfg.ReptCliEntryLock);
		NdisAllocateSpinLock(pAd, &pAd->ApCfg.CliLinkMapLock);
#endif
#endif
#endif
#ifdef GREENAP_SUPPORT
		NdisAllocateSpinLock(pAd, &pAd->ApCfg.greenap.lock);
#endif /* GREENAP_SUPPORT */
		NdisAllocateSpinLock(pAd, &pAd->tssi_lock);
		NdisAllocateSpinLock(pAd, &pAd->LockInterrupt);
#ifdef MT7626_E2_SUPPORT
		NdisAllocateSpinLock(pAd, &pAd->LockInterrupt_md);
#endif
		/*Allocate interface lock*/
		NdisAllocateSpinLock(pAd, &pAd->VirtualIfLock);
		/*HIF related initial for pAd*/
		RTMPInitHifAdapterBlock(pAd);
#ifdef RLM_CAL_CACHE_SUPPORT
		rlmCalCacheInit(pAd, &pAd->rlmCalCache);
#endif /* RLM_CAL_CACHE_SUPPORT */
		*ppAdapter = (VOID *)pAd;
	} while (FALSE);

	if (Status != NDIS_STATUS_SUCCESS) {
		if (pAd) {
			if (pAd->stats) {
				os_free_mem(pAd->stats);
				pAd->stats = NULL;
			}

			if (pAd->iw_stats) {
				os_free_mem(pAd->iw_stats);
				pAd->iw_stats = NULL;
			}

			RtmpOsVfree(pAd);
		}

		return Status;
	}

	/* Init ProbeRespIE Table */
	for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++) {
		if (os_alloc_mem(pAd, &pAd->ProbeRespIE[index].pIe, MAX_VIE_LEN) == NDIS_STATUS_SUCCESS)
			RTMPZeroMemory(pAd->ProbeRespIE[index].pIe, MAX_VIE_LEN);
		else
			pAd->ProbeRespIE[index].pIe = NULL;
	}
	/*allocate hdev_ctrl struct for prepare chip_cap & chip_ops */
	hdev_ctrl_init(pAd, type);
	/*init WifiSys information structure*/
	wifi_sys_init(pAd);
	/*allocate wpf related memory*/
	wpf_config_init(pAd);
#ifdef MULTI_INF_SUPPORT
	Status = multi_inf_adapt_reg((VOID *) pAd);
#endif /* MULTI_INF_SUPPORT */
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<-- RTMPAllocAdapterBlock, Status=%x\n", Status));

	/* SpaceX: Create proc endpoints. */
	proc_create_data("assoc_clients", 0, NULL, &assoc_clients_fops, pAd);
	proc_create_data("assoc_connections", 0, NULL, &assoc_connections_fops, pAd);
    proc_create_data("band_info", 0, NULL, &band_info_fops, pAd);

    return Status;
}

NDIS_STATUS alloc_chip_cap_dep_data(RTMP_ADAPTER *pAd)
{
#ifdef RTMP_MAC_PCI

	if (IS_RBUS_INF(pAd) || IS_PCI_INF(pAd)) {
		if (alloc_chip_cap_dep_data_pci(pAd->hdev_ctrl) != NDIS_STATUS_SUCCESS)
			return NDIS_STATUS_FAILURE;
	}

#endif

	if (alloc_chip_cap_dep_data_cmm(pAd) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	return NDIS_STATUS_SUCCESS;
}

VOID free_chip_cap_dep_data(RTMP_ADAPTER *pAd)
{
#ifdef RTMP_MAC_PCI

	if (IS_RBUS_INF(pAd) || IS_PCI_INF(pAd))
		free_chip_cap_dep_data_pci(pAd->hdev_ctrl);

#endif
	free_chip_cap_dep_data_cmm(pAd);
}

NDIS_STATUS alloc_chip_cap_dep_data_cmm(RTMP_ADAPTER *pAd)
{
	INT index;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	BOOLEAN *pDeQueueRunning = NULL;
	NDIS_SPIN_LOCK *pDeQueueLock = NULL;
	QUEUE_HEADER *pTxSwQueue = NULL;
	NDIS_SPIN_LOCK *pTxSwQueueLock = NULL;
#if defined(MT_MAC) && defined(IP_ASSEMBLY)
	DL_LIST *pAssebQueue = NULL;
#endif
#ifdef BLOCK_NET_IF
	BLOCK_QUEUE_ENTRY *pBlockQueueTab;
#endif /* BLOCK_NET_IF */
#ifdef COMPOS_TESTMODE_WIN
	ULONG *pOneSecOsTxCount;
	ULONG *pOneSecDmaDoneCount;
#endif
	os_alloc_mem(pAd, (UCHAR **)&pDeQueueRunning, (num_of_tx_ring)*sizeof(BOOLEAN));

	if (pDeQueueRunning == NULL)
		return NDIS_STATUS_FAILURE;

	NdisZeroMemory(pDeQueueRunning, (num_of_tx_ring)*sizeof(BOOLEAN));
	pAd->DeQueueRunning = pDeQueueRunning;

	for (index = 0; index < num_of_tx_ring; index++)
		pAd->DeQueueRunning[index] = FALSE;

	os_alloc_mem(pAd, (UCHAR **)&pDeQueueLock, (num_of_tx_ring)*sizeof(NDIS_SPIN_LOCK));

	if (pDeQueueLock == NULL)
		return NDIS_STATUS_FAILURE;

	NdisZeroMemory(pDeQueueLock, (num_of_tx_ring)*sizeof(NDIS_SPIN_LOCK));
	pAd->DeQueueLock = pDeQueueLock;

	for (index = 0; index < num_of_tx_ring; index++)
		NdisAllocateSpinLock(pAd, &pAd->DeQueueLock[index]);

	os_alloc_mem(pAd, (UCHAR **)&pTxSwQueue, (num_of_tx_ring)*sizeof(QUEUE_HEADER));

	if (pTxSwQueue == NULL)
		return NDIS_STATUS_FAILURE;

	NdisZeroMemory(pTxSwQueue, (num_of_tx_ring)*sizeof(QUEUE_HEADER));
	pAd->TxSwQueue = pTxSwQueue;
	os_alloc_mem(pAd, (UCHAR **)&pTxSwQueueLock, (num_of_tx_ring)*sizeof(NDIS_SPIN_LOCK));

	if (pTxSwQueueLock == NULL)
		return NDIS_STATUS_FAILURE;

	NdisZeroMemory(pTxSwQueueLock, (num_of_tx_ring)*sizeof(NDIS_SPIN_LOCK));
	pAd->TxSwQueueLock = pTxSwQueueLock;

	for (index = 0; index < num_of_tx_ring; index++)
		NdisAllocateSpinLock(pAd, &pAd->TxSwQueueLock[index]);

#if defined(MT_MAC) && defined(IP_ASSEMBLY)
	os_alloc_mem(pAd, (UCHAR **)&pAssebQueue, (num_of_tx_ring)*sizeof(DL_LIST));

	if (pAssebQueue == NULL)
		return NDIS_STATUS_FAILURE;

	NdisZeroMemory(pAssebQueue, (num_of_tx_ring)*sizeof(DL_LIST));
	pAd->assebQueue = pAssebQueue;
	os_alloc_mem(
		pAd,
		(UCHAR **)&pAd->cur_ip_assem_data,
		(num_of_tx_ring)*sizeof(struct ip_assemble_data *));

	if (pAd->cur_ip_assem_data == NULL)
		return NDIS_STATUS_FAILURE;

	for (index = 0; index < num_of_tx_ring; index++)
		DlListInit(&pAd->assebQueue[index]);

#endif
#ifdef BLOCK_NET_IF
	os_alloc_mem(pAd, (UCHAR **)&pBlockQueueTab, (num_of_tx_ring)*sizeof(BLOCK_QUEUE_ENTRY));

	if (pBlockQueueTab == NULL)
		return NDIS_STATUS_FAILURE;

	NdisZeroMemory(pBlockQueueTab, (num_of_tx_ring)*sizeof(BLOCK_QUEUE_ENTRY));
	pAd->blockQueueTab = pBlockQueueTab;
#endif /* BLOCK_NET_IF */
#ifdef COMPOS_TESTMODE_WIN
	os_alloc_mem(pAd, (UCHAR **)&pOneSecOsTxCount, (num_of_tx_ring)*sizeof(ULONG));

	if (pOneSecOsTxCount == NULL)
		return NDIS_STATUS_FAILURE;

	NdisZeroMemory(pOneSecOsTxCount, (num_of_tx_ring)*sizeof(ULONG));
	pAd->RalinkCounters.OneSecOsTxCount = pOneSecOsTxCount;
	os_alloc_mem(pAd, (UCHAR **)&pOneSecDmaDoneCount, (num_of_tx_ring)*sizeof(ULONG));

	if (pOneSecDmaDoneCount == NULL)
		return NDIS_STATUS_FAILURE;

	NdisZeroMemory(pOneSecDmaDoneCount, (num_of_tx_ring)*sizeof(ULONG));
	pAd->RalinkCounters.OneSecDmaDoneCount = pOneSecDmaDoneCount;
#endif
	return NDIS_STATUS_SUCCESS;
}

VOID free_chip_cap_dep_data_cmm(RTMP_ADAPTER *pAd)
{
	INT index;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);

	/* spinlock must be freed before its data structure is freed */
	for (index = 0; index < num_of_tx_ring; index++)
		NdisFreeSpinLock(&pAd->DeQueueLock[index]);

	for (index = 0; index < num_of_tx_ring; index++)
		NdisFreeSpinLock(&pAd->TxSwQueueLock[index]);

	os_free_mem(pAd->DeQueueRunning);
	os_free_mem(pAd->DeQueueLock);
	os_free_mem(pAd->TxSwQueue);
	os_free_mem(pAd->TxSwQueueLock);
#if defined(MT_MAC) && defined(IP_ASSEMBLY)
	os_free_mem(pAd->assebQueue);
	os_free_mem(pAd->cur_ip_assem_data);
#endif
#ifdef BLOCK_NET_IF
	os_free_mem(pAd->blockQueueTab);
#endif
#ifdef COMPOS_TESTMODE_WIN
	os_free_mem(pAd->RalinkCounters.OneSecOsTxCount);
	os_free_mem(pAd->RalinkCounters.OneSecDmaDoneCount);
#endif
}

BOOLEAN RTMPCheckPhyMode(RTMP_ADAPTER *pAd, UINT8 band_cap, UCHAR *pPhyMode)
{
	BOOLEAN RetVal = TRUE;

	if (band_cap == RFIC_24GHZ) {
		if (!WMODE_2G_ONLY(*pPhyMode)) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s(): Warning! The board type is 2.4G only!\n",
					  __func__));
			RetVal =  FALSE;
		}
	} else if (band_cap == RFIC_5GHZ) {
		if (!WMODE_5G_ONLY(*pPhyMode)) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s(): Warning! The board type is 5G only!\n",
					  __func__));
			RetVal =  FALSE;
		}
	} else if (band_cap == RFIC_DUAL_BAND)
		RetVal = TRUE;
	else {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s(): Unknown supported band (%u), assume dual band used.\n",
				  __func__, band_cap));
		RetVal = TRUE;
	}

	if (RetVal == FALSE) {
#ifdef DOT11_N_SUPPORT

		if (band_cap == RFIC_5GHZ) /*5G ony: change to A/N mode */
			*pPhyMode = PHY_11AN_MIXED;
		else /* 2.4G only or Unknown supported band: change to B/G/N mode */
			*pPhyMode = PHY_11BGN_MIXED;

#else

		if (band_cap == RFIC_5GHZ) /*5G ony: change to A mode */
			*pPhyMode = PHY_11A;
		else /* 2.4G only or Unknown supported band: change to B/G mode */
			*pPhyMode = PHY_11BG_MIXED;

#endif /* !DOT11_N_SUPPORT */
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s(): Changed PhyMode to %u\n",
				  __func__, *pPhyMode));
	}

	return RetVal;
}


/*
	========================================================================

	Routine Description:
		Set default value from EEPROM

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID NICInitAsicFromEEPROM(RTMP_ADAPTER *pAd)
{
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> NICInitAsicFromEEPROM\n"));
	NicConfig2.word = pAd->NicConfig2.word;
	/* finally set primary ant */
	AntCfgInit(pAd);
	RTMP_CHIP_ASIC_INIT_TEMPERATURE_COMPENSATION(pAd);
#ifdef RTMP_RF_RW_SUPPORT
	/*Init RFRegisters after read RFIC type from EEPROM*/
	InitRFRegisters(pAd);
#endif /* RTMP_RF_RW_SUPPORT */
#ifdef CONFIG_ATE
	RTMPCfgTssiGainFromEEPROM(pAd);
#endif /* CONFIG_ATE */
#ifndef MAC_INIT_OFFLOAD
	AsicSetRxStream(pAd, pAd->Antenna.field.RxPath, 0);
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		UCHAR bandIdx = HcGetBandByWdev(&pAd->StaCfg[MAIN_MSTA_ID].wdev);

		RTMPStaCfgRadioCtrlFromEEPROM(pAd, NicConfig2);
		AsicSetTxStream(pAd, pAd->Antenna.field.TxPath, OPMODE_STA, FALSE, bandIdx);
	}
#endif /* CONFIG_STA_SUPPORT */
	RTMP_EEPROM_ASIC_INIT(pAd);
	AsicBbpInitFromEEPROM(pAd);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TxPath = %d, RxPath = %d, RFIC=%d\n",
			 pAd->Antenna.field.TxPath, pAd->Antenna.field.RxPath, pAd->RfIcType));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<-- NICInitAsicFromEEPROM\n"));
}



INT32 WfHifSysInit(RTMP_ADAPTER *pAd, HIF_INFO_T *pHifInfo)
{
	NDIS_STATUS status;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()-->\n", __func__));
	RT28XXDMADisable(pAd);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Disable WPDMA\n", __func__));
#ifdef CONFIG_WIFI_PAGE_ALLOC_SKB
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use dev_alloc_skb\n"));
#else
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use alloc_skb\n"));
#endif
#ifdef RESOURCE_PRE_ALLOC
	status = RTMPInitTxRxRingMemory(pAd);
#else
	status = RTMPAllocTxRxRingMemory(pAd);
#endif /* RESOURCE_PRE_ALLOC */

	if (cut_through_init(&pAd->PktTokenCb, pAd) != TRUE) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): cut_through_init failed\n", __func__));
		status = NDIS_STATUS_FAILURE;
	}
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPAllocTxRxMemory failed, Status[=0x%08x]\n", status));
		goto err;
	}

#ifdef WLAN_SKB_RECYCLE
	skb_queue_head_init(&pAd->rx0_recycle);
#endif /* WLAN_SKB_RECYCLE */
err:
	return status;
}


INT32 WfHifSysExit(RTMP_ADAPTER *pAd)
{
	WLAN_HOOK_CALL(WLAN_HOOK_HIF_EXIT, pAd, NULL);
#ifdef RESOURCE_PRE_ALLOC
	RTMPResetTxRxRingMemory(pAd);
#else
	RTMPFreeTxRxRingMemory(pAd);
#endif /* RESOURCE_PRE_ALLOC */

	cut_through_deinit((PKT_TOKEN_CB **)&pAd->PktTokenCb);

	RT28XXDMAReset(pAd);
#ifdef CONFIG_FWOWN_SUPPORT
	FwOwn(pAd);
#endif
	return 0;
}


INT32 WfMcuSysInit(RTMP_ADAPTER *pAd)
{
	MCU_CTRL_INIT(pAd);
	chip_fw_init(pAd);

	return NDIS_STATUS_SUCCESS;
}

INT32 WfMcuSysExit(RTMP_ADAPTER *pAd)
{
	MCUSysExit(pAd);
	return 0;
}


extern RTMP_STRING *mac;

INT32 WfEPROMSysInit(RTMP_ADAPTER *pAd)
{
	UCHAR RfIC;
	/* hook e2p operation */
	RtmpChipOpsEepromHook(pAd, pAd->infType, E2P_NONE);
	/* We should read EEPROM for all cases */
	/* TODO: shiang-7603, revise this! */
	NICReadEEPROMParameters(pAd, (RTMP_STRING *)mac);
	HcRadioInit(pAd, pAd->RfIcType, pAd->CommonCfg.dbdc_mode);
	/* +++Add by shiang for debug */
	RfIC = HcGetRadioRfIC(pAd);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s():PhyCtrl=>RfIcType/rf_band_cap = 0x%x/0x%x\n",
			  __func__, pAd->RfIcType, RfIC));
	RTMP_NET_DEV_NICKNAME_INIT(pAd);
	return NDIS_STATUS_SUCCESS;
}


INT32 WfEPROMSysExit(RTMP_ADAPTER *pAd)
{
	return NDIS_STATUS_SUCCESS;
}

/*
	========================================================================

	Routine Description:
		Initialize NIC hardware

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS	NICInitializeAdapter(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;

	ret = WfMacInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS) {
		ret = NDIS_STATUS_FAILURE;
		goto err;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MAC Init Done!\n"));
	ret = WfPhyInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS) {
		ret = NDIS_STATUS_FAILURE;
		goto err;
	}

	ret = NICInitializeAsic(pAd);
err:
	return ret;
}


INT rtmp_hif_cyc_init(RTMP_ADAPTER *pAd, UINT8 val)
{
	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				 __func__, __LINE__));
		return FALSE;
	}

	return TRUE;
}



/*
	========================================================================

	Routine Description:
		Initialize ASIC

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS NICInitializeAsic(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> NICInitializeAsic\n"));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<-- NICInitializeAsic\n"));
	return NDIS_STATUS_SUCCESS;
}


/*
	========================================================================

	Routine Description:
		Reset NIC from error

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		Reset NIC from error state

	========================================================================
*/
static VOID NICResetFromErrorForRf(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	INT i;
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		for (i = 0; i < MAX_MULTI_STA; i++) {
			pStaCfg = &pAd->StaCfg[i];
			AsicStaBbpTuning(pAd, pStaCfg);
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	hc_reset_radio(pAd);
}

VOID NICResetFromError(RTMP_ADAPTER *pAd)
{
	/* Reset BBP (according to alex, reset ASIC will force reset BBP*/
	/* Therefore, skip the reset BBP*/
	/* RTMP_IO_WRITE32(pAd->hdev_ctrl, MAC_CSR1, 0x2);*/
	/* TODO: shaing-7603 */
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT76x6(pAd) || IS_MT7637(pAd)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): for MT7603\n", __func__));
		NICInitializeAdapter(pAd);
		NICInitAsicFromEEPROM(pAd);
		RTMPEnableRxTx(pAd);
		return;
	}

#ifndef MT_MAC
	RTMP_IO_WRITE32(pAd->hdev_ctrl, MAC_SYS_CTRL, 0x1);
	/* Remove ASIC from reset state*/
	RTMP_IO_WRITE32(pAd->hdev_ctrl, MAC_SYS_CTRL, 0x0);
#endif /*ndef MT_MAC */
	NICInitializeAdapter(pAd);
	NICInitAsicFromEEPROM(pAd);
	NICResetFromErrorForRf(pAd);
}


VOID NICUpdateFifoStaCounters(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_ATE

	/* Nothing to do in ATE mode */
	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		/* MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n", */
		/* __func__, __LINE__)); */
		return;
	}
}


/*
	========================================================================

	Routine Description:
		Compare two memory block

	Arguments:
		pSrc1		Pointer to first memory address
		pSrc2		Pointer to second memory address

	Return Value:
		0:			memory is equal
		1:			pSrc1 memory is larger
		2:			pSrc2 memory is larger

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
ULONG RTMPCompareMemory(VOID *pSrc1, VOID *pSrc2, ULONG Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	ULONG	Index = 0;

	pMem1 = (PUCHAR) pSrc1;
	pMem2 = (PUCHAR) pSrc2;

	for (Index = 0; Index < Length; Index++) {
		if (pMem1[Index] > pMem2[Index])
			return 1;
		else if (pMem1[Index] < pMem2[Index])
			return 2;
	}

	/* Equal*/
	return 0;
}


/*
	========================================================================

	Routine Description:
		Zero out memory block

	Arguments:
		pSrc1		Pointer to memory address
		Length		Size

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPZeroMemory(VOID *pSrc, ULONG Length)
{
	PUCHAR	pMem;
	ULONG	Index = 0;

	pMem = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++)
		pMem[Index] = 0x00;
}


/*
	========================================================================

	Routine Description:
		Copy data from memory block 1 to memory block 2

	Arguments:
		pDest		Pointer to destination memory address
		pSrc		Pointer to source memory address
		Length		Copy size

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPMoveMemory(VOID *pDest, VOID *pSrc, ULONG Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	UINT	Index;

	ASSERT((Length == 0) || (pDest && pSrc));
	pMem1 = (PUCHAR) pDest;
	pMem2 = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++) {
		if (pMem1 && pMem2)
			pMem1[Index] = pMem2[Index];
	}
}


VOID UserCfgExit(RTMP_ADAPTER *pAd)
{
#ifdef RATE_PRIOR_SUPPORT
	PBLACK_STA pBlackSta = NULL, tmp;
#endif /*RATE_PRIOR_SUPPORT*/
#ifdef RT_CFG80211_SUPPORT
	/* Reset the CFG80211 Internal Flag */
	RTMP_DRIVER_80211_RESET(pAd);
#endif /* RT_CFG80211_SUPPORT */
#ifdef DOT11_N_SUPPORT
	BATableExit(pAd);
#endif /* DOT11_N_SUPPORT */
#ifdef MT_DFS_SUPPORT
	os_free_mem(pAd->CommonCfg.DfsParameter.prRadarDetectionParam);
#endif /*MT_FDS_SUPPORT*/
	NdisFreeSpinLock(&pAd->MacTabLock);
	RTMP_SEM_EVENT_DESTORY(&pAd->IndirectUpdateLock);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef BAND_STEERING
	if (pAd->ApCfg.BandSteering) {
		BndStrg_Release(pAd);
	}
#endif /* BAND_STEERING */
#ifdef RADIUS_MAC_ACL_SUPPORT
		{
			PLIST_HEADER pListHeader = NULL;
			RT_LIST_ENTRY *pListEntry = NULL;
			UCHAR apidx = 0;

			for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
				pListHeader = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList;

				if (pListHeader->size == 0)
					continue;

				pListEntry = pListHeader->pHead;

				while (pListEntry != NULL) {
					removeHeadList(pListHeader);
					os_free_mem(pListEntry);
					pListEntry = pListHeader->pHead;
				}

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Clean [%d] Radius ACL Cache.\n", apidx));
			}
		}
#endif /* RADIUS_MAC_ACL_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	do {
		INT i;
		PSTA_ADMIN_CONFIG	pStaCfg = NULL;

		for (i = 0; i < MAX_MULTI_STA; i++) {
			pStaCfg = &pAd->StaCfg[i];

			if (pStaCfg->wdev.pEapolPktFromAP) {
				os_free_mem(pStaCfg->wdev.pEapolPktFromAP);
				pStaCfg->wdev.pEapolPktFromAP = NULL;
			}

#ifdef	WSC_STA_SUPPORT
			{
				PWSC_CTRL		pWscControl = NULL;

				pWscControl = &pStaCfg->wdev.WscControl;

				if (pWscControl->pWscRxBuf)
					os_free_mem(pWscControl->pWscRxBuf);

				pWscControl->pWscRxBuf = NULL;

				if (pWscControl->pWscTxBuf)
					os_free_mem(pWscControl->pWscTxBuf);

				pWscControl->pWscTxBuf = NULL;
			}
#endif /*WSC_STA_SUPPORT*/
		}
	} while (0);

#endif /* CONFIG_STA_SUPPORT */
	wdev_config_init(pAd);
#ifdef DOT11_SAE_SUPPORT
	sae_cfg_deinit(pAd, &pAd->SaeCfg);
#endif /* DOT11_SAE_SUPPORT */
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	group_info_bi_deinit();
#endif
	scan_release_mem(pAd);

#ifdef RATE_PRIOR_SUPPORT
	RTMP_SEM_LOCK(&pAd->LowRateCtrl.BlackListLock);
		DlListForEach(pBlackSta, &pAd->LowRateCtrl.BlackList, BLACK_STA, List) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("Remove from blklist, %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pBlackSta->Addr)));
			tmp = pBlackSta;
			pBlackSta = DlListEntry(pBlackSta->List.Prev, BLACK_STA, List);
			DlListDel(&(tmp->List));
			os_free_mem(tmp);
	}
	RTMP_SEM_UNLOCK(&pAd->LowRateCtrl.BlackListLock);

	NdisFreeSpinLock(&pAd->LowRateCtrl.BlackListLock);
#endif/*RATE_PRIOR_SUPPORT*/
}


/*
	========================================================================

	Routine Description:
		Initialize port configuration structure

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID UserCfgInit(RTMP_ADAPTER *pAd)
{
	UINT i;
	UINT key_index, bss_index;
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> UserCfgInit\n"));
	/*init wifi profile*/
	wpf_init(pAd);
	pAd->IndicateMediaState = NdisMediaStateDisconnected;
	/* part I. intialize common configuration */
	pAd->CommonCfg.BasicRateBitmap = 0xF;
	pAd->CommonCfg.BasicRateBitmapOld = 0xF;

#if defined(BB_SOC) && defined(TCSUPPORT_WLAN_SW_RPS)
	pAd->rxThreshold = 400;
	pAd->rxPassThresholdCnt = 2;
#endif

	pAd->ucBFBackOffMode = BF_BACKOFF_4T_MODE;

#ifdef SINGLE_SKU_V2
	pAd->fgPwrLimitRead[POWER_LIMIT_TABLE_TYPE_SKU] = FALSE;
	pAd->fgPwrLimitRead[POWER_LIMIT_TABLE_TYPE_BACKOFF] = FALSE;

	pAd->u1SkuParamLen[0] = SINGLE_SKU_PARSE_TABLE_CCK_LENGTH;
	pAd->u1SkuParamLen[1] = SINGLE_SKU_PARSE_TABLE_OFDM_LENGTH;
	pAd->u1SkuParamLen[2] = SINGLE_SKU_PARSE_TABLE_HTVHT20_LENGTH;
	pAd->u1SkuParamLen[3] = SINGLE_SKU_PARSE_TABLE_HTVHT40_LENGTH;
	pAd->u1SkuParamLen[4] = SINGLE_SKU_PARSE_TABLE_VHT80_LENGTH;
	pAd->u1SkuParamLen[5] = SINGLE_SKU_PARSE_TABLE_VHT160_LENGTH;

	/* cck */
	pAd->u1SkuParamTransOffset[0] = 0;
	/* ofdm */
	pAd->u1SkuParamTransOffset[1] = SINGLE_SKU_PARSE_TABLE_CCK_LENGTH;
	/* ht20 */
	pAd->u1SkuParamTransOffset[2] = SINGLE_SKU_PARSE_TABLE_CCK_LENGTH
									+ SINGLE_SKU_PARSE_TABLE_OFDM_LENGTH;
	/* ht40 */
	pAd->u1SkuParamTransOffset[3] = SINGLE_SKU_PARSE_TABLE_CCK_LENGTH
									+ SINGLE_SKU_PARSE_TABLE_OFDM_LENGTH
									+ SINGLE_SKU_PARSE_TABLE_HTVHT20_LENGTH;
	/* vht20 */
	pAd->u1SkuParamTransOffset[4] = SINGLE_SKU_PARSE_TABLE_CCK_LENGTH
									+ SINGLE_SKU_PARSE_TABLE_OFDM_LENGTH;
	/* vht40 */
	pAd->u1SkuParamTransOffset[5] = SINGLE_SKU_PARSE_TABLE_CCK_LENGTH
									+ SINGLE_SKU_PARSE_TABLE_OFDM_LENGTH
									+ SINGLE_SKU_PARSE_TABLE_HTVHT20_LENGTH;
	/* vht80 */
	pAd->u1SkuParamTransOffset[6] = SINGLE_SKU_PARSE_TABLE_CCK_LENGTH
									+ SINGLE_SKU_PARSE_TABLE_OFDM_LENGTH
									+ SINGLE_SKU_PARSE_TABLE_HTVHT20_LENGTH
									+ SINGLE_SKU_PARSE_TABLE_HTVHT40_LENGTH;
	/* vht160 */
	pAd->u1SkuParamTransOffset[7] = SINGLE_SKU_PARSE_TABLE_CCK_LENGTH
									+ SINGLE_SKU_PARSE_TABLE_OFDM_LENGTH
									+ SINGLE_SKU_PARSE_TABLE_HTVHT20_LENGTH
									+ SINGLE_SKU_PARSE_TABLE_HTVHT40_LENGTH
									+ SINGLE_SKU_PARSE_TABLE_VHT80_LENGTH;

	pAd->u1SkuChBandNeedParse[0] = TABLE_PARSE_G_A_BAND;
	pAd->u1SkuChBandNeedParse[1] = TABLE_PARSE_G_A_BAND;
	pAd->u1SkuChBandNeedParse[2] = TABLE_PARSE_G_A_BAND;
	pAd->u1SkuChBandNeedParse[3] = TABLE_PARSE_G_A_BAND;
	pAd->u1SkuChBandNeedParse[4] = TABLE_PARSE_G_A_BAND;
	pAd->u1SkuChBandNeedParse[5] = TABLE_PARSE_G_A_BAND;

	pAd->u1SkuFillParamLen[0] = SINGLE_SKU_FILL_TABLE_CCK_LENGTH;
	pAd->u1SkuFillParamLen[1] = SINGLE_SKU_FILL_TABLE_OFDM_LENGTH;
	pAd->u1SkuFillParamLen[2] = SINGLE_SKU_FILL_TABLE_HT20_LENGTH;
	pAd->u1SkuFillParamLen[3] = SINGLE_SKU_FILL_TABLE_HT40_LENGTH;
	pAd->u1SkuFillParamLen[4] = SINGLE_SKU_FILL_TABLE_VHT20_LENGTH;
	pAd->u1SkuFillParamLen[5] = SINGLE_SKU_FILL_TABLE_VHT40_LENGTH;
	pAd->u1SkuFillParamLen[6] = SINGLE_SKU_FILL_TABLE_VHT80_LENGTH;
	pAd->u1SkuFillParamLen[7] = SINGLE_SKU_FILL_TABLE_VHT160_LENGTH;

	pAd->u1BackoffParamLen[0] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH;
	pAd->u1BackoffParamLen[1] = BACKOFF_TABLE_BF_OFF_OFDM_LENGTH;
	pAd->u1BackoffParamLen[2] = BACKOFF_TABLE_BF_ON_OFDM_LENGTH;
	pAd->u1BackoffParamLen[3] = BACKOFF_TABLE_BF_OFF_VHT20_LENGTH;
	pAd->u1BackoffParamLen[4] = BACKOFF_TABLE_BF_ON_VHT20_LENGTH;
	pAd->u1BackoffParamLen[5] = BACKOFF_TABLE_BF_OFF_VHT40_LENGTH;
	pAd->u1BackoffParamLen[6] = BACKOFF_TABLE_BF_ON_VHT40_LENGTH;
	pAd->u1BackoffParamLen[7] = BACKOFF_TABLE_BF_OFF_VHT80_LENGTH;
	pAd->u1BackoffParamLen[8] = BACKOFF_TABLE_BF_ON_VHT80_LENGTH;
	pAd->u1BackoffParamLen[9] = BACKOFF_TABLE_BF_OFF_VHT160_LENGTH;
	pAd->u1BackoffParamLen[10] = BACKOFF_TABLE_BF_ON_VHT160_LENGTH;

	/* cck */
	pAd->u1BackoffParamTransOffset[0] = 0;
	/* ofdm off */
	pAd->u1BackoffParamTransOffset[1] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH;
	/* ofdm on */
	pAd->u1BackoffParamTransOffset[2] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH
										+ BACKOFF_TABLE_BF_OFF_OFDM_LENGTH;
	/* ht20 off */
	pAd->u1BackoffParamTransOffset[3] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH
										+ BACKOFF_TABLE_BF_OFF_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_ON_OFDM_LENGTH;
	/* ht20 on */
	pAd->u1BackoffParamTransOffset[4] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH
										+ BACKOFF_TABLE_BF_OFF_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_ON_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT20_LENGTH;
	/* ht40 off */
	pAd->u1BackoffParamTransOffset[5] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH
										+ BACKOFF_TABLE_BF_OFF_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_ON_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT20_LENGTH
										+ BACKOFF_TABLE_BF_ON_VHT20_LENGTH;
	/* ht40 on */
	pAd->u1BackoffParamTransOffset[6] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH
										+ BACKOFF_TABLE_BF_OFF_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_ON_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT20_LENGTH
										+ BACKOFF_TABLE_BF_ON_VHT20_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT40_LENGTH;
	/* vht20 off */
	pAd->u1BackoffParamTransOffset[7] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH
										+ BACKOFF_TABLE_BF_OFF_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_ON_OFDM_LENGTH;
	/* vht20 on */
	pAd->u1BackoffParamTransOffset[8] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH
										+ BACKOFF_TABLE_BF_OFF_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_ON_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT20_LENGTH;
	/* vht40 off */
	pAd->u1BackoffParamTransOffset[9] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH
										+ BACKOFF_TABLE_BF_OFF_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_ON_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT20_LENGTH
										+ BACKOFF_TABLE_BF_ON_VHT20_LENGTH;
	/* vht40 on */
	pAd->u1BackoffParamTransOffset[10] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH
										+ BACKOFF_TABLE_BF_OFF_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_ON_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT20_LENGTH
										+ BACKOFF_TABLE_BF_ON_VHT20_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT40_LENGTH;
	/* vht80 off */
	pAd->u1BackoffParamTransOffset[11] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH
										+ BACKOFF_TABLE_BF_OFF_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_ON_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT20_LENGTH
										+ BACKOFF_TABLE_BF_ON_VHT20_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT40_LENGTH
										+ BACKOFF_TABLE_BF_ON_VHT40_LENGTH;
	/* vht80 on */
	pAd->u1BackoffParamTransOffset[12] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH
										+ BACKOFF_TABLE_BF_OFF_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_ON_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT20_LENGTH
										+ BACKOFF_TABLE_BF_ON_VHT20_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT40_LENGTH
										+ BACKOFF_TABLE_BF_ON_VHT40_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT80_LENGTH;
	/* vht160 off */
	pAd->u1BackoffParamTransOffset[13] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH
										+ BACKOFF_TABLE_BF_OFF_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_ON_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT20_LENGTH
										+ BACKOFF_TABLE_BF_ON_VHT20_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT40_LENGTH
										+ BACKOFF_TABLE_BF_ON_VHT40_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT80_LENGTH
										+ BACKOFF_TABLE_BF_ON_VHT80_LENGTH;
	/* vht160 on */
	pAd->u1BackoffParamTransOffset[14] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH
										+ BACKOFF_TABLE_BF_OFF_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_ON_OFDM_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT20_LENGTH
										+ BACKOFF_TABLE_BF_ON_VHT20_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT40_LENGTH
										+ BACKOFF_TABLE_BF_ON_VHT40_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT80_LENGTH
										+ BACKOFF_TABLE_BF_ON_VHT80_LENGTH
										+ BACKOFF_TABLE_BF_OFF_VHT160_LENGTH;

	pAd->u1BackoffChBandNeedParse[0] = TABLE_PARSE_G_A_BAND;
	pAd->u1BackoffChBandNeedParse[1] = TABLE_PARSE_G_A_BAND;
	pAd->u1BackoffChBandNeedParse[2] = TABLE_PARSE_G_A_BAND;
	pAd->u1BackoffChBandNeedParse[3] = TABLE_PARSE_G_A_BAND;
	pAd->u1BackoffChBandNeedParse[4] = TABLE_PARSE_G_A_BAND;
	pAd->u1BackoffChBandNeedParse[5] = TABLE_PARSE_G_A_BAND;
	pAd->u1BackoffChBandNeedParse[6] = TABLE_PARSE_G_A_BAND;
	pAd->u1BackoffChBandNeedParse[7] = TABLE_PARSE_G_A_BAND;
	pAd->u1BackoffChBandNeedParse[8] = TABLE_PARSE_G_A_BAND;
	pAd->u1BackoffChBandNeedParse[9] = TABLE_PARSE_G_A_BAND;
	pAd->u1BackoffChBandNeedParse[10] = TABLE_PARSE_G_A_BAND;

	pAd->u1BackoffFillParamLen[0] = BACKOFF_TABLE_BF_OFF_CCK_LENGTH;
	pAd->u1BackoffFillParamLen[1] = BACKOFF_TABLE_BF_OFF_OFDM_LENGTH;
	pAd->u1BackoffFillParamLen[2] = BACKOFF_TABLE_BF_ON_OFDM_LENGTH;
	pAd->u1BackoffFillParamLen[3] = BACKOFF_TABLE_BF_OFF_VHT20_LENGTH;
	pAd->u1BackoffFillParamLen[4] = BACKOFF_TABLE_BF_ON_VHT20_LENGTH;
	pAd->u1BackoffFillParamLen[5] = BACKOFF_TABLE_BF_OFF_VHT40_LENGTH;
	pAd->u1BackoffFillParamLen[6] = BACKOFF_TABLE_BF_ON_VHT40_LENGTH;
	pAd->u1BackoffFillParamLen[7] = BACKOFF_TABLE_BF_OFF_VHT20_LENGTH;
	pAd->u1BackoffFillParamLen[8] = BACKOFF_TABLE_BF_ON_VHT20_LENGTH;
	pAd->u1BackoffFillParamLen[9] = BACKOFF_TABLE_BF_OFF_VHT40_LENGTH;
	pAd->u1BackoffFillParamLen[10] = BACKOFF_TABLE_BF_ON_VHT40_LENGTH;
	pAd->u1BackoffFillParamLen[11] = BACKOFF_TABLE_BF_OFF_VHT80_LENGTH;
	pAd->u1BackoffFillParamLen[12] = BACKOFF_TABLE_BF_ON_VHT80_LENGTH;
	pAd->u1BackoffFillParamLen[13] = BACKOFF_TABLE_BF_OFF_VHT160_LENGTH;
	pAd->u1BackoffFillParamLen[14] = BACKOFF_TABLE_BF_ON_VHT160_LENGTH;
#endif /* SINGLE_SKU_V2 */

#ifdef TX_POWER_CONTROL_SUPPORT
#if defined(MT7626) || defined(AXE) || defined(MT7915)
#else
	os_zero_mem(pAd->CommonCfg.cPowerUpCckOfdm,
		sizeof(CHAR) * DBDC_BAND_NUM * POWER_UP_CATEGORY_RATE_NUM);
	os_zero_mem(pAd->CommonCfg.cPowerUpHt20,
		sizeof(CHAR) * DBDC_BAND_NUM * POWER_UP_CATEGORY_RATE_NUM);
	os_zero_mem(pAd->CommonCfg.cPowerUpHt40,
		sizeof(CHAR) * DBDC_BAND_NUM * POWER_UP_CATEGORY_RATE_NUM);
	os_zero_mem(pAd->CommonCfg.cPowerUpVht20,
		sizeof(CHAR) * DBDC_BAND_NUM * POWER_UP_CATEGORY_RATE_NUM);
	os_zero_mem(pAd->CommonCfg.cPowerUpVht40,
		sizeof(CHAR) * DBDC_BAND_NUM * POWER_UP_CATEGORY_RATE_NUM);
	os_zero_mem(pAd->CommonCfg.cPowerUpVht80,
		sizeof(CHAR) * DBDC_BAND_NUM * POWER_UP_CATEGORY_RATE_NUM);
	os_zero_mem(pAd->CommonCfg.cPowerUpVht160,
		sizeof(CHAR) * DBDC_BAND_NUM * POWER_UP_CATEGORY_RATE_NUM);
#endif
#endif /* TX_POWER_CONTROL_SUPPORT */

#ifdef LINK_TEST_SUPPORT
	/* state machine state flag */
	os_fill_mem(pAd->ucLinkBwState, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_BW_STATE);
	os_fill_mem(pAd->ucRxStreamState, sizeof(UINT8) * BAND_NUM, RX_UNDEFINED_RXSTREAM_STATE);
	os_fill_mem(pAd->ucRxStreamStatePrev, sizeof(UINT8) * BAND_NUM, RX_UNDEFINED_RXSTREAM_STATE);
	os_fill_mem(pAd->ucRxFilterstate, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_RXFILTER_STATE);
	os_fill_mem(pAd->ucTxCsdState, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_CSD_STATE);
	os_fill_mem(pAd->ucTxPwrBoostState, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_POWER_STATE);
	os_fill_mem(pAd->ucLinkRcpiState, sizeof(UINT8) * BAND_NUM, RX_UNDEFINED_RCPI_STATE);
	os_fill_mem(pAd->ucLinkSpeState, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_SPEIDX_STATE);
    os_fill_mem(pAd->ucLinkSpeStatePrev, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_SPEIDX_STATE);
    os_fill_mem(pAd->ucLinkBwStatePrev, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_BW_STATE);
    os_fill_mem(pAd->ucTxCsdStatePrev, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_CSD_STATE);

	/* BW Control Paramter */
	os_fill_mem(pAd->fgBwInfoUpdate, sizeof(BOOLEAN) * BAND_NUM, FALSE);

	/* Rx Control Parameter */
	pAd->ucRxTestTimeoutCount			   =	   0;
	pAd->c8TempRxCount					   =	   0;
	pAd->ucRssiTh						   =	  10;
	pAd->ucRssiSigniTh					   =	  15;
	pAd->c8RxCountTh					   =	   5;
	pAd->ucTimeOutTh					   =	 200;  /* 20s */
	pAd->ucPerTh						   =	  50;
	pAd->cNrRssiTh						   =	 -40;
	pAd->cChgTestPathTh					   =	 -30;
	pAd->ucRxSenCountTh					   =	   3;
	pAd->cWBRssiTh						   =	 -70;
	pAd->cIBRssiTh						   =	 -80;
	os_fill_mem(pAd->u1RxStreamSwitchReason, sizeof(UINT8) * BAND_NUM, 0);
	os_fill_mem(pAd->u1RxSenCount, sizeof(UINT8) * BAND_NUM, 0);
	os_fill_mem(pAd->u1SpeRssiIdxPrev, sizeof(UINT8) * BAND_NUM, 0);
    os_fill_mem(pAd->ucRxSenCount, sizeof(UINT8) * BAND_NUM, 0);

	/* ACR Control Parameter */
	pAd->ucACRConfidenceCntTh			   =	  10;
	pAd->ucMaxInConfidenceCntTh			   =	  10;
	pAd->cMaxInRssiTh					   =	 -40;
	os_fill_mem(pAd->ucRxFilterConfidenceCnt, sizeof(UINT8) * BAND_NUM, 0);

	/* Tx Control Parameter */
	pAd->ucCmwCheckCount				   =	   0;
	pAd->ucCmwCheckCountTh				   =	  20;  /* 2s */
	pAd->fgCmwInstrumBack4T				   =   FALSE;
	pAd->ucRssiBalanceCount				   =	   0;
	pAd->ucRssiIBalanceCountTh			   =	 100;  /* 10s */
	pAd->fgRssiBack4T					   =   FALSE;
	pAd->ucCableRssiTh					   =	  25;
	pAd->fgCmwLinkDone					   =   FALSE;
	pAd->fgApclientLinkUp				   =   FALSE;
	pAd->ucLinkCount					   =	   0;
	pAd->ucLinkCountTh					   =	  30;
	pAd->fgLinkRSSICheck				   =   FALSE;
	os_fill_mem(pAd->ucCmwChannelBand, sizeof(UINT8) * BAND_NUM, CHANNEL_BAND_2G);

	/* channel band Control Paramter */
	os_fill_mem(pAd->fgChannelBandInfoUpdate, sizeof(BOOLEAN) * BAND_NUM, FALSE);

	/* Tx Power Control Paramter */
	os_zero_mem(pAd->ucTxPwrUpTbl, sizeof(UINT8)*CMW_POWER_UP_RATE_NUM*4);

	/* manual command control function enable/disable flag */
	pAd->fgTxSpeEn						   = TRUE;
	pAd->fgRxRcpiEn						   = TRUE;
	pAd->fgTxSpurEn						   = TRUE;
	pAd->fgRxSensitEn					   = TRUE;
	pAd->fgACREn						   = TRUE;
#endif /* LINK_TEST_SUPPORT */

#ifdef	ETSI_RX_BLOCKER_SUPPORT
	pAd->c1RWbRssiHTh	= -70;
	pAd->c1RWbRssiLTh	= -70;
	pAd->c1RIbRssiLTh	= -80;
	pAd->c1WBRssiTh4R	= -75;

	pAd->fgFixWbIBRssiEn = FALSE;
	pAd->c1WbRssiWF0 = 0xFF;
	pAd->c1WbRssiWF1 = 0xFF;
	pAd->c1WbRssiWF2 = 0xFF;
	pAd->c1WbRssiWF3 = 0xFF;
	pAd->c1IbRssiWF0 = 0xFF;
	pAd->c1IbRssiWF1 = 0xFF;
	pAd->c1IbRssiWF2 = 0xFF;
	pAd->c1IbRssiWF3 = 0xFF;

	pAd->u1RxBlockerState = ETSI_RXBLOCKER4R;
	pAd->u1To1RCheckCnt  = 10;
	pAd->u2To1RvaildCntTH = 100;
	pAd->u2To4RvaildCntTH = 3;
	pAd->u1ValidCnt		 = 0;
	pAd->u14RValidCnt	= 0;

	pAd->u1CheckTime	 = 1;
	pAd->u1TimeCnt		 = 0;

	pAd->i1MaxWRssiIdxPrev  = 0xFF;
	pAd->fgAdaptRxBlock  = 0;
#endif /* end ETSI_RX_BLOCKER_SUPPORT */


#ifdef RF_LOCKDOWN
	/* disable QA Effuse Write back status by default */
	pAd->fgQAEffuseWriteBack = FALSE;
#endif /* RF_LOCKDOWN */
	/* Disable EPA flag */
	pAd->fgEPA = FALSE;
#ifdef RF_LOCKDOWN
	/* Apply Cal-Free Effuse value by default */
	pAd->fgCalFreeApply = TRUE;
	pAd->RFlockTempIdx  =    0;
	pAd->CalFreeTempIdx =    0;
#endif /* RF_LOCKDOWN */

	for (key_index = 0; key_index < SHARE_KEY_NUM; key_index++) {
		for (bss_index = 0; bss_index < MAX_MBSSID_NUM(pAd) + MAX_P2P_NUM; bss_index++) {
			pAd->SharedKey[bss_index][key_index].KeyLen = 0;
			pAd->SharedKey[bss_index][key_index].CipherAlg = CIPHER_NONE;
		}
	}

	pAd->bLocalAdminMAC = FALSE;
	pAd->EepromAccess = FALSE;
	pAd->Antenna.word = 0;
#ifdef RTMP_MAC_PCI
#ifdef LED_CONTROL_SUPPORT
	pAd->LedCntl.LedIndicatorStrength = 0;
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_MAC_PCI */
#ifdef THERMAL_PROTECT_SUPPORT
	pAd->force_one_tx_stream = FALSE;
#endif /* THERMAL_PROTECT_SUPPORT */
	pAd->RfIcType = RFIC_2820;
	/* Init timer for reset complete event*/
	pAd->bForcePrintTX = FALSE;
	pAd->bForcePrintRX = FALSE;
	pAd->bStaFifoTest = FALSE;
	pAd->bProtectionTest = FALSE;
	pAd->bHCCATest = FALSE;
	pAd->bGenOneHCCA = FALSE;
	pAd->CommonCfg.Dsifs = 10;      /* in units of usec */
	pAd->CommonCfg.TxPower = 100; /* mW*/
	pAd->CommonCfg.ucTxPowerPercentage[BAND0] = 100; /* AUTO*/
#ifdef DBDC_MODE
	pAd->CommonCfg.ucTxPowerPercentage[BAND1] = 100; /* AUTO*/
#endif /* DBDC_MODE */
	pAd->CommonCfg.ucTxPowerDefault[BAND0] = 100; /* AUTO*/
#ifdef DBDC_MODE
	pAd->CommonCfg.ucTxPowerDefault[BAND1] = 100; /* AUTO*/
#endif /* DBDC_MODE */
	pAd->CommonCfg.TxPreamble = Rt802_11PreambleAuto; /* use Long preamble on TX by defaut*/
	pAd->CommonCfg.bUseZeroToDisableFragment = FALSE;
	pAd->bDisableRtsProtect = FALSE;
	pAd->CommonCfg.UseBGProtection = 0;    /* 0: AUTO*/
	pAd->CommonCfg.bEnableTxBurst = TRUE; /* 0;	*/
	pAd->CommonCfg.SavedPhyMode = 0xff;
	pAd->CommonCfg.BandState = UNKNOWN_BAND;
	pAd->wmm_cw_min = 4;

	switch (pAd->OpMode) {
	case OPMODE_AP:
		pAd->wmm_cw_max = 6;
		break;

	case OPMODE_STA:
		pAd->wmm_cw_max = 10;
		break;
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
	os_zero_mem(&pAd->ApCfg.ACSCheckTime, sizeof(UINT32) * DBDC_BAND_NUM);
	os_zero_mem(&pAd->ApCfg.ACSCheckCount, sizeof(UINT32) * DBDC_BAND_NUM);
#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

	/* If profile parameter is set, ACS parameters of HW bands need to be reset*/
	hc_init_ACSChCtrl(pAd);
#endif/* CONFIG_AP_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
	pAd->CommonCfg.CarrierDetect.delta = CARRIER_DETECT_DELTA;
	pAd->CommonCfg.CarrierDetect.div_flag = CARRIER_DETECT_DIV_FLAG;
	pAd->CommonCfg.CarrierDetect.criteria = CARRIER_DETECT_CRITIRIA(pAd);
	pAd->CommonCfg.CarrierDetect.threshold = CARRIER_DETECT_THRESHOLD;
	pAd->CommonCfg.CarrierDetect.recheck1 = CARRIER_DETECT_RECHECK_TIME;
	pAd->CommonCfg.CarrierDetect.CarrierGoneThreshold = CARRIER_GONE_TRESHOLD;
	pAd->CommonCfg.CarrierDetect.VGA_Mask = CARRIER_DETECT_DEFAULT_MASK;
	pAd->CommonCfg.CarrierDetect.Packet_End_Mask = CARRIER_DETECT_DEFAULT_MASK;
	pAd->CommonCfg.CarrierDetect.Rx_PE_Mask = CARRIER_DETECT_DEFAULT_MASK;
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef UAPSD_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	{
		UINT32 IdMbss;

		for (IdMbss = 0; IdMbss < HW_BEACON_MAX_NUM; IdMbss++)
			UAPSD_INFO_INIT(&pAd->ApCfg.MBSSID[IdMbss].wdev.UapsdInfo);
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* UAPSD_SUPPORT */
	pAd->CommonCfg.bNeedSendTriggerFrame = FALSE;
	pAd->CommonCfg.TriggerTimerCount = 0;
	pAd->CommonCfg.bAPSDForcePowerSave = FALSE;
	/*pAd->CommonCfg.bCountryFlag = FALSE;*/
	/*pAd->CommonCfg.TxStream = 0;*/
	/*pAd->CommonCfg.RxStream = 0;*/
#ifdef DOT11_N_SUPPORT
	pAd->bBroadComHT = FALSE;
	pAd->CommonCfg.bRdg = FALSE;
#ifdef DOT11N_DRAFT3
	pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	/* Unit : TU. 5~1000*/
	pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	/* Unit : TU. 10~1000*/
	pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	/* Unit : Second	*/
	pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000*/
	pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	/* Unit : TU. 20~10000*/
	pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
	pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	/* Unit : percentage*/
	pAd->CommonCfg.Dot11BssWidthChanTranDelay = (ULONG)(pAd->CommonCfg.Dot11BssWidthTriggerScanInt *
			pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
	pAd->CommonCfg.bBssCoexEnable =
		TRUE; /* by default, we enable this feature, you can disable it via the profile or ioctl command*/
	pAd->CommonCfg.BssCoexApCntThr = 0;
	pAd->CommonCfg.Bss2040NeedFallBack = 0;
#endif  /* DOT11N_DRAFT3 */
	pAd->CommonCfg.bRcvBSSWidthTriggerEvents = FALSE;
	pAd->CommonCfg.BACapability.field.Policy = IMMED_BA;
	pAd->CommonCfg.BACapability.field.RxBAWinLimit = 64; /*32;*/
	pAd->CommonCfg.BACapability.field.TxBAWinLimit = 64; /*32;*/
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> UserCfgInit. BACapability = 0x%x\n",
			 pAd->CommonCfg.BACapability.word));
	pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
	BATableInit(pAd, &pAd->BATable);
	pAd->CommonCfg.bExtChannelSwitchAnnouncement = 1;
	pAd->CommonCfg.bMIMOPSEnable = TRUE;
	pAd->CommonCfg.bBADecline = FALSE;
	pAd->CommonCfg.bDisableReordering = FALSE;

	if (pAd->MACVersion == 0x28720200)
		pAd->CommonCfg.TxBASize = 13; /*by Jerry recommend*/
	else
		pAd->CommonCfg.TxBASize = 7;

	pAd->CommonCfg.REGBACapability.word = pAd->CommonCfg.BACapability.word;
#endif /* DOT11_N_SUPPORT */
	pAd->CommonCfg.TxRate = RATE_6;
	pAd->CommonCfg.BeaconPeriod = 100;     /* in mSec*/
#ifdef STREAM_MODE_SUPPORT

	if (cap->FlgHwStreamMode) {
		pAd->CommonCfg.StreamMode = 3;
		pAd->CommonCfg.StreamModeMCS = 0x0B0B;
		NdisMoveMemory(&pAd->CommonCfg.StreamModeMac[0][0],
					   BROADCAST_ADDR, MAC_ADDR_LEN);
	}

#endif /* STREAM_MODE_SUPPORT */
#ifdef TXBF_SUPPORT
	pAd->CommonCfg.ETxBfNoncompress = 0;
	pAd->CommonCfg.ETxBfIncapable = 0;
#endif /* TXBF_SUPPORT */
#if defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT)
	pAd->CommonCfg.lowTrafficThrd = 2;
	pAd->CommonCfg.TrainUpRule = 2; /* 1; */
	pAd->CommonCfg.TrainUpRuleRSSI = -70; /* 0; */
	pAd->CommonCfg.TrainUpLowThrd = 90;
	pAd->CommonCfg.TrainUpHighThrd = 110;
#endif /* defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT) */
	/* WFA policy - disallow TH rate in WEP or TKIP cipher */
	pAd->CommonCfg.HT_DisallowTKIP = TRUE;
	/* Frequency for rate adaptation */
	pAd->ra_interval = DEF_RA_TIME_INTRVAL;
	pAd->ra_fast_interval = DEF_QUICK_RA_TIME_INTERVAL;
#ifdef AGS_SUPPORT

	if (pAd->rateAlg == RATE_ALG_AGS)
		pAd->ra_fast_interval = AGS_QUICK_RA_TIME_INTERVAL;

#endif /* AGS_SUPPORT */
	pAd->CommonCfg.bRalinkBurstMode = FALSE;
	/* global variables mXXXX used in MAC protocol state machines*/
	OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);
	/* PHY specification*/
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);  /* CCK use LONG preamble*/
	/* Default for extra information is not valid*/
	pAd->ExtraInfo = EXTRA_INFO_CLEAR;
#ifdef CONFIG_AP_SUPPORT
	/* Default Config change flag*/
	pAd->bConfigChanged = FALSE;
#ifdef GREENAP_SUPPORT
	greenap_init(greenap);
#endif /* GREENAP_SUPPORT */
#endif
	/*
		part III. AP configurations
	*/
#ifdef CONFIG_AP_SUPPORT
#if defined(P2P_APCLI_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
#else
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_APCLI_SUPPORT || RT_CFG80211_P2P_SUPPORT */
	{
		/* Set MBSS Default Configurations*/
		UCHAR j;

		pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);

		for (j = BSS0; j < pAd->ApCfg.BssidNum; j++) {
			BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[j];
			struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[j].wdev;
#ifdef DOT11V_WNM_SUPPORT
			WNM_CONFIG *wnm_cfg = &pAd->ApCfg.MBSSID[j].WnmCfg;

			wnm_cfg->bDot11vWNM_BSSEnable = 1;
#endif /* DOT11V_WNM_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
		MAP_Init(pAd, wdev, WDEV_TYPE_AP);
#endif /* CONFIG_MAP_SUPPORT */
#ifdef DOT1X_SUPPORT
			/* PMK cache setting*/

#ifdef R1KH_HARD_RETRY /* yiwei no give up! */
			/* profile already set to 120to prevent PMK is delete on DUT */
			mbss->PMKCachePeriod = (120 * 60 * OS_HZ); /* unit : tick(default: 120 minute)*/
#else /* R1KH_HARD_RETRY */
			mbss->PMKCachePeriod = (10 * 60 * OS_HZ); /* unit : tick(default: 10 minute)*/
#endif /* !R1KH_HARD_RETRY */

			/* dot1x related per BSS */
			mbss->wdev.SecConfig.radius_srv_num = 0;
			mbss->wdev.SecConfig.NasIdLen = 0;
			mbss->wdev.SecConfig.IEEE8021X = FALSE;
			mbss->wdev.SecConfig.PreAuth = FALSE;
#ifdef RADIUS_MAC_ACL_SUPPORT
			NdisZeroMemory(&mbss->wdev.SecConfig.RadiusMacAuthCache, sizeof(RT_802_11_RADIUS_ACL));
			/* Default Timeout Value for 1xDaemon Radius ACL Cache */
			mbss->wdev.SecConfig.RadiusMacAuthCacheTimeout = 30;
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */
			/* VLAN related */
			mbss->wdev.VLAN_VID = 0;
			/* Default MCS as AUTO*/
			wdev->bAutoTxRateSwitch = TRUE;
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			/* Default is zero. It means no limit.*/
			mbss->MaxStaNum = 0;
			mbss->StaCount = 0;

#ifdef WSC_AP_SUPPORT
			wdev->WscSecurityMode = 0xff;
			{
				PWSC_CTRL pWscControl;
				INT idx;
#ifdef WSC_V2_SUPPORT
				PWSC_V2_INFO	pWscV2Info;
#endif /* WSC_V2_SUPPORT */
				/*
					WscControl cannot be zero here, because WscControl timers are initial in MLME Initialize
					and MLME Initialize is called before UserCfgInit.
				*/
				pWscControl = &wdev->WscControl;
				NdisZeroMemory(&pWscControl->RegData, sizeof(WSC_REG_DATA));
				NdisZeroMemory(&pAd->CommonCfg.WscStaPbcProbeInfo, sizeof(WSC_STA_PBC_PROBE_INFO));
				pWscControl->WscMode = 1;
				pWscControl->WscConfStatus = 1;
#ifdef WSC_V2_SUPPORT
				pWscControl->WscConfigMethods = 0x238C;
#else
				pWscControl->WscConfigMethods = 0x0084;
#endif /* WSC_V2_SUPPORT */
				pWscControl->RegData.ReComputePke = 1;
				pWscControl->lastId = 1;
				/* pWscControl->EntryIfIdx = (MIN_NET_DEVICE_FOR_MBSSID | j); */
				pWscControl->WscRejectSamePinFromEnrollee = FALSE;
				pAd->CommonCfg.WscPBCOverlap = FALSE;
				pWscControl->WscConfMode = 0;
				pWscControl->WscStatus = 0;
				pWscControl->WscState = 0;
				pWscControl->WscPinCode = 0;
				pWscControl->WscLastPinFromEnrollee = 0;
				pWscControl->WscEnrollee4digitPinCode = FALSE;
				pWscControl->WscEnrolleePinCode = 0;
				pWscControl->WscSelReg = 0;
				pWscControl->WscUseUPnP = 0;
				pWscControl->bWCNTest = FALSE;
				pWscControl->WscKeyASCII = 0; /* default, 0 (64 Hex) */

				/*
					Enrollee 192 random bytes for DH key generation
				*/
				for (idx = 0; idx < 192; idx++)
					pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAd);

				/* Enrollee Nonce, first generate and save to Wsc Control Block*/
				for (idx = 0; idx < 16; idx++)
					pWscControl->RegData.SelfNonce[idx] = RandomByte(pAd);

				NdisZeroMemory(&pWscControl->WscDefaultSsid, sizeof(NDIS_802_11_SSID));
				NdisZeroMemory(&pWscControl->Wsc_Uuid_Str[0], UUID_LEN_STR);
				NdisZeroMemory(&pWscControl->Wsc_Uuid_E[0], UUID_LEN_HEX);
				pWscControl->bCheckMultiByte = FALSE;
				pWscControl->bWscAutoTigeer = FALSE;
				pWscControl->bWscFragment = FALSE;
				pWscControl->WscFragSize = 128;
				initList(&pWscControl->WscPeerList);
				NdisAllocateSpinLock(pAd, &pWscControl->WscPeerListSemLock);
				pWscControl->PinAttackCount = 0;
				pWscControl->bSetupLock = FALSE;
#ifdef WSC_V2_SUPPORT
				pWscV2Info = &pWscControl->WscV2Info;
				pWscV2Info->bWpsEnable = TRUE;
				pWscV2Info->ExtraTlv.TlvLen = 0;
				pWscV2Info->ExtraTlv.TlvTag = 0;
				pWscV2Info->ExtraTlv.pTlvData = NULL;
				pWscV2Info->ExtraTlv.TlvType = TLV_ASCII;
				pWscV2Info->bEnableWpsV2 = TRUE;
				pWscControl->SetupLockTime = WSC_WPS_AP_SETUP_LOCK_TIME;
				pWscControl->MaxPinAttack = WSC_WPS_AP_MAX_PIN_ATTACK;
#endif /* WSC_V2_SUPPORT */
			}
#endif /* WSC_AP_SUPPORT */
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
			pAd->ApCfg.MBSSID[j].ap_vendor_ie.length = 0;
			pAd->ApCfg.MBSSID[j].ap_vendor_ie.pointer = NULL;
			NdisAllocateSpinLock(pAd, &pAd->ApCfg.MBSSID[j].ap_vendor_ie.vendor_ie_lock);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
			for (i = 0; i < WLAN_MAX_NUM_OF_TIM; i++)
				mbss->wdev.bcn_buf.TimBitmaps[i] = 0;
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
			wdev->rate.BcnPhyMode.field.MODE = MODE_CCK;
			wdev->rate.BcnPhyMode.field.MCS = RATE_1;
			wdev->rate.BcnPhyMode.field.BW = BW_20;
			wdev->rate.BcnPhyMode_5G.field.MODE = MODE_OFDM;
			wdev->rate.BcnPhyMode_5G.field.MCS = MCS_RATE_6;
			wdev->rate.BcnPhyMode_5G.field.BW = BW_20;
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
#ifdef MCAST_RATE_SPECIFIC
			wdev->rate.MCastPhyMode.word = pAd->MacTab.Content[MCAST_WCID].HTPhyMode.word;
			wdev->rate.MCastPhyMode_5G.word = pAd->MacTab.Content[MCAST_WCID].HTPhyMode.word;
#endif /* MCAST_RATE_SPECIFIC */
			wdev->bUseShortSlotTime = TRUE;
			wdev->SlotTimeValue = 9;
		}

#ifdef DOT1X_SUPPORT
		/* PMK cache setting*/
		NdisZeroMemory(&pAd->ApCfg.PMKIDCache, sizeof(NDIS_AP_802_11_PMKID));
#endif /* DOT1X_SUPPORT */
		pAd->ApCfg.DtimCount  = 0;
		pAd->ApCfg.DtimPeriod = DEFAULT_DTIM_PERIOD;
		pAd->ApCfg.ErpIeContent = 0;
		pAd->ApCfg.StaIdleTimeout = MAC_TABLE_AGEOUT_TIME;
		pAd->ApCfg.BANClass3Data = FALSE;
#ifdef IDS_SUPPORT
		/* Default disable IDS threshold and reset all IDS counters*/
		pAd->ApCfg.IdsEnable = FALSE;
		pAd->ApCfg.AuthFloodThreshold = 0;
		pAd->ApCfg.AssocReqFloodThreshold = 0;
		pAd->ApCfg.ReassocReqFloodThreshold = 0;
		pAd->ApCfg.ProbeReqFloodThreshold = 0;
		pAd->ApCfg.DisassocFloodThreshold = 0;
		pAd->ApCfg.DeauthFloodThreshold = 0;
		pAd->ApCfg.EapReqFloodThreshold = 0;
		RTMPClearAllIdsCounter(pAd);
#endif /* IDS_SUPPORT */
#ifdef WDS_SUPPORT
		APWdsInitialize(pAd);
#endif /* WDS_SUPPORT*/
#ifdef WSC_INCLUDED
		pAd->WriteWscCfgToDatFile = 0xFF;
		pAd->WriteWscCfgToAr9DatFile = FALSE;
#ifdef CONFIG_AP_SUPPORT
#if defined(RTMP_PCI_SUPPORT) && defined(RTMP_RBUS_SUPPORT)
		pAd->bWscDriverAutoUpdateCfg = (IS_RBUS_INF(pAd)) ? FALSE : TRUE;
#else
#ifdef RTMP_RBUS_SUPPORT
		pAd->bWscDriverAutoUpdateCfg = FALSE;
#else
		pAd->bWscDriverAutoUpdateCfg = TRUE;
#endif
#endif /* defined(RTMP_PCI_SUPPORT) && defined (RTMP_RBUS_SUPPORT) */
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef APCLI_SUPPORT
		pAd->ApCfg.FlgApCliIsUapsdInfoUpdated = FALSE;
		pAd->ApCfg.ApCliNum = MAX_APCLI_NUM;
#ifdef BT_APCLI_SUPPORT
		pAd->ApCfg.ApCliAutoBWBTSupport = FALSE;
#endif
		for (j = 0; j < MAX_APCLI_NUM; j++) {
			STA_ADMIN_CONFIG *apcli_entry = &pAd->StaCfg[j];
			struct wifi_dev *wdev = &apcli_entry->wdev;
#ifdef CONFIG_MAP_SUPPORT
			MAP_Init(pAd, wdev, WDEV_TYPE_STA);
#endif /* CONFIG_MAP_SUPPORT */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			apcli_entry->ApcliInfStat.AutoConnectFlag = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
			wdev->bAutoTxRateSwitch = TRUE;
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			apcli_entry->wdev.UapsdInfo.bAPSDCapable = FALSE;
			apcli_entry->bBlockAssoc = FALSE;
#if defined(APCLI_CFG80211_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT)
#if defined(DOT1X_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT)
			apcli_entry->wdev.SecConfig.IEEE8021X = FALSE;
#endif
			apcli_entry->wpa_supplicant_info.IEEE8021x_required_keys = FALSE;
			apcli_entry->wpa_supplicant_info.bRSN_IE_FromWpaSupplicant = FALSE;
			apcli_entry->wpa_supplicant_info.bLostAp = FALSE;
			apcli_entry->bConfigChanged = FALSE;
			apcli_entry->wpa_supplicant_info.DesireSharedKeyId = 0;
			apcli_entry->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;
			apcli_entry->wpa_supplicant_info.WpaSupplicantScanCount = 0;
			apcli_entry->wpa_supplicant_info.pWpsProbeReqIe = NULL;
			apcli_entry->wpa_supplicant_info.WpsProbeReqIeLen = 0;
			apcli_entry->wpa_supplicant_info.pWpaAssocIe = NULL;
			apcli_entry->wpa_supplicant_info.WpaAssocIeLen = 0;
			apcli_entry->SavedPMKNum = 0;
			RTMPZeroMemory(apcli_entry->SavedPMK, (PMKID_NO * sizeof(BSSID_INFO)));
#endif/*WPA_SUPPLICANT_SUPPORT*/
#ifdef APCLI_CONNECTION_TRIAL
			apcli_entry->TrialCh = 0;/* if the channel is 0, AP will connect the rootap is in the same channel with ra0. */
#endif /* APCLI_CONNECTION_TRIAL */
#ifdef WSC_AP_SUPPORT
			apcli_entry->wdev.WscControl.WscApCliScanMode = TRIGGER_FULL_SCAN;
#endif /* WSC_AP_SUPPORT */
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
			/* for vendor ie */
			pAd->StaCfg[j].apcli_vendor_ie.length = 0;
			pAd->StaCfg[j].apcli_vendor_ie.pointer = NULL;
			NdisAllocateSpinLock(pAd, &pAd->StaCfg[j].apcli_vendor_ie.vendor_ie_lock);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
		}
#endif /* APCLI_SUPPORT */

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
		for (i = 0; i < DBDC_BAND_NUM; i++)
			NdisAllocateSpinLock(pAd, &pAd->ScanCtrl[i].ScanTab.event_bss_entry_lock);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

		pAd->ApCfg.EntryClientCount = 0;
	}
#endif /* CONFIG_AP_SUPPORT */
	/*
		part IV. others
	*/
	/* dynamic BBP R66:sensibity tuning to overcome background noise*/
	pAd->BbpTuning.bEnable = TRUE;
	pAd->BbpTuning.FalseCcaLowerThreshold = 100;
	pAd->BbpTuning.FalseCcaUpperThreshold = 512;
	pAd->BbpTuning.R66Delta = 4;
	pAd->Mlme.bEnableAutoAntennaCheck = TRUE;
	/* Also initial R66CurrentValue, RTUSBResumeMsduTransmission might use this value.*/
	/* if not initial this value, the default value will be 0.*/
	pAd->BbpTuning.R66CurrentValue = 0x38;
	/* initialize MAC table and allocate spin lock*/
	NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));
	InitializeQueueHeader(&pAd->MacTab.McastPsQueue);
	NdisAllocateSpinLock(pAd, &pAd->MacTabLock);
	/*RTMPInitTimer(pAd, &pAd->RECBATimer, RECBATimerTimeout, pAd, TRUE);*/
	/*RTMPSetTimer(&pAd->RECBATimer, REORDER_EXEC_INTV);*/
	pAd->CommonCfg.bWiFiTest = FALSE;
#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.EntryLifeCheck = MAC_ENTRY_LIFE_CHECK_CNT;
#ifdef DOT11R_FT_SUPPORT
	FT_CfgInitial(pAd);
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef DOT11_SAE_SUPPORT
	sae_cfg_init(pAd, &pAd->SaeCfg);
#endif /* DOT11_SAE_SUPPORT */
	pAd->RxAnt.Pair1PrimaryRxAnt = 0;
	pAd->RxAnt.Pair1SecondaryRxAnt = 1;
	pAd->RxAnt.EvaluatePeriod = 0;
	pAd->RxAnt.RcvPktNumWhenEvaluate = 0;
	pAd->MaxTxPwr = 27;

#ifdef CONFIG_AP_SUPPORT
	pAd->RxAnt.Pair1AvgRssiGroup1[0] = pAd->RxAnt.Pair1AvgRssiGroup1[1] = 0;
	pAd->RxAnt.Pair1AvgRssiGroup2[0] = pAd->RxAnt.Pair1AvgRssiGroup2[1] = 0;
#endif /* CONFIG_AP_SUPPORT */
#ifdef TXRX_SW_ANTDIV_SUPPORT
	cap->bTxRxSwAntDiv = FALSE;
#endif /* TXRX_SW_ANTDIV_SUPPORT */
#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	{
		UCHAR BandIdx = 0;

		for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
			BSS_TABLE *ScanTab = &pAd->ScanCtrl[BandIdx].ScanTab;
	for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) {
				BSS_ENTRY *pBssEntry = &ScanTab->BssEntry[i];

		if (pAd->ProbeRespIE[i].pIe)
			pBssEntry->pVarIeFromProbRsp = pAd->ProbeRespIE[i].pIe;
		else
			pBssEntry->pVarIeFromProbRsp = NULL;
	}
		}
	}
#endif /* defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT) */
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		pAd->ScanCtrl[i].dfs_ch_utilization = TRUE;
	}
#ifdef WSC_INCLUDED
	NdisZeroMemory(&pAd->CommonCfg.WscStaPbcProbeInfo, sizeof(WSC_STA_PBC_PROBE_INFO));
	pAd->CommonCfg.WscPBCOverlap = FALSE;
#endif /* WSC_INCLUDED */
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

	if (wf_drv_tbl.wf_fwd_set_cb_num != NULL)
		wf_drv_tbl.wf_fwd_set_cb_num(PACKET_BAND_CB, RECV_FROM_CB);

#endif /* CONFIG_WIFI_PKT_FWD */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
	pAd->WOW_Cfg.bEnable = FALSE;
	pAd->WOW_Cfg.bWOWFirmware = FALSE;	/* load normal firmware */
	pAd->WOW_Cfg.bInBand = TRUE;		/* use in-band signal */
	pAd->WOW_Cfg.nSelectedGPIO = 2;
	pAd->WOW_Cfg.nDelay = 3; /* (3+1)*3 = 12 sec */
	pAd->WOW_Cfg.nHoldTime = 1000;	/* unit is us */
	pAd->WOW_Cfg.nWakeupInterface = cap->nWakeupInterface; /* WOW_WAKEUP_BY_USB; */
	pAd->WOW_Cfg.bGPIOHighLow = WOW_GPIO_LOW_TO_HIGH;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WOW Enable %d, WOWFirmware %d\n", pAd->WOW_Cfg.bEnable,
			 pAd->WOW_Cfg.bWOWFirmware));
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT) */

	/* Clear channel ctrl buffer */
	hc_init_ChCtrl(pAd);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\x1b[1;33m [UserCfgInit] - Clear channel ctrl buffer \x1b[m \n"));
	pAd->CommonCfg.ChGrpEn = 0;
	NdisZeroMemory(pAd->CommonCfg.ChGrpChannelList, (MAX_NUM_OF_CHANNELS)*sizeof(UCHAR));
	pAd->CommonCfg.ChGrpChannelNum = 0;

#ifdef MT_DFS_SUPPORT
	DfsParamInit(pAd);/* Jelly20150311 */
#endif
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	for (i = 0; i < DBDC_BAND_NUM; i++)
		pAd->Dot11_H[i].RDMode = RD_SILENCE_MODE;
#endif
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		pAd->Dot11_H[i].CSCount = 0;
		pAd->Dot11_H[i].CSPeriod = 10;
		pAd->Dot11_H[i].wdev_count = 0;
		pAd->Dot11_H[i].ChMovingTime = 65;
		pAd->Dot11_H[i].bDFSIndoor = 1;
	}
	scan_partial_init(pAd);
#ifdef APCLI_SUPPORT
#ifdef APCLI_AUTO_CONNECT_SUPPORT

	for (i = 0; i < MAX_APCLI_NUM; i++) {
		pAd->StaCfg[i].ApCliAutoConnectType = TRIGGER_SCAN_BY_USER; /* User Trigger SCAN by default */
		pAd->StaCfg[i].ApCliAutoConnectRunning = FALSE;
	}

	pAd->ApCfg.ApCliAutoConnectChannelSwitching = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	for (i = 0; i < MAX_APCLI_NUM; i++) {
		pAd->ApCfg.bPartialScanEnable[i] = FALSE;
		pAd->ApCfg.bPartialScanning[i] = FALSE;
	}
#endif /* APCLI_SUPPORT */
#ifdef DOT11_VHT_AC
#ifdef WFA_VHT_PF
	pAd->force_amsdu = FALSE;
	pAd->force_noack = FALSE;
	pAd->force_vht_op_mode = FALSE;
	pAd->vht_force_sgi = FALSE;
	pAd->vht_force_tx_stbc = FALSE;
	pAd->CommonCfg.vht_nss_cap = cap->mcs_nss.max_nss;
	pAd->CommonCfg.vht_mcs_cap = cap->mcs_nss.max_vht_mcs;
#endif /* WFA_VHT_PF */
#endif /* DOT11_VHT_AC */
#ifdef MICROWAVE_OVEN_SUPPORT

	if (pAd->OpMode == OPMODE_AP)
		pAd->CommonCfg.MO_Cfg.bEnable = TRUE;
	else
		pAd->CommonCfg.MO_Cfg.bEnable = FALSE;

	pAd->CommonCfg.MO_Cfg.nFalseCCATh = MO_FALSE_CCA_TH;
#endif /* MICROWAVE_OVEN_SUPPORT */
#ifdef DYNAMIC_VGA_SUPPORT
	pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable = TRUE;
	pAd->CommonCfg.lna_vga_ctl.nFalseCCATh = 600;
	pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh = 100;
#endif /* DYNAMIC_VGA_SUPPORT */
#ifdef DOT11_VHT_AC
	pAd->CommonCfg.bNonVhtDisallow = FALSE;
#endif /* DOT11_VHT_AC */
#ifdef MT_MAC
	cap->TmrEnable = 0;
	RTMP_SEM_EVENT_INIT(&(pAd->IndirectUpdateLock), &pAd->RscSemMemList);
	pAd->PSEWatchDogEn = 0;
	pAd->RxPseCheckTimes = 0;
	pAd->PSEResetCount = 0;
	pAd->PSETriggerType1Count = 0;
	pAd->PSETriggerType1Count = 0;
	pAd->PSEResetFailCount = 0;
#endif
#ifdef RTMP_MAC_PCI
	pAd->PDMAWatchDogEn = 0;
	pAd->TxDMAResetCount = 0;
	pAd->RxDMAResetCount = 0;
	pAd->TxDMACheckTimes = 0;
	pAd->RxDMACheckTimes = 0;
	pAd->PDMAResetFailCount = 0;
#endif
#ifdef CONFIG_MULTI_CHANNEL
	pAd->Mlme.bStartMcc = FALSE;
	pAd->Mlme.bStartScc = FALSE;
	pAd->Mlme.channel_1st_staytime = 40;
	pAd->Mlme.channel_2nd_staytime = 40;
	pAd->Mlme.switch_idle_time = 10;
	pAd->Mlme.null_frame_count = 1;
	pAd->Mlme.channel_1st_bw = 0;
	pAd->Mlme.channel_2nd_bw = 0;
#endif /* CONFIG_MULTI_CHANNEL */
#ifdef SNIFFER_SUPPORT
	pAd->monitor_ctrl.CurrentMonitorMode = 0;
	pAd->monitor_ctrl.FrameType = FC_TYPE_RSVED;
	pAd->monitor_ctrl.FilterSize = RX_DATA_BUFFER_SIZE + sizeof(struct mtk_radiotap_header);
#endif /* SNIFFER_SUPPORT */
	pAd->bPS_Retrieve = 1;
	pAd->CommonCfg.bTXRX_RXV_ON = 0;
	pAd->parse_rxv_stat_enable = 0;
	pAd->AccuOneSecRxBand0FcsErrCnt = 0;
	pAd->AccuOneSecRxBand0MdrdyCnt = 0;
	pAd->AccuOneSecRxBand1FcsErrCnt = 0;
	pAd->AccuOneSecRxBand1MdrdyCnt = 0;
	pAd->CommonCfg.ManualTxop = 0;
	pAd->CommonCfg.ManualTxopThreshold = 10; /* Mbps */
	pAd->CommonCfg.ManualTxopUpBound = 20; /* Ratio */
	pAd->CommonCfg.ManualTxopLowBound = 5; /* Ratio */
#ifdef CONFIG_AP_SUPPORT
#ifdef VOW_SUPPORT
	vow_variable_reset(pAd);
#endif /* VOW_SUPPORT */
#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
	pAd->ApCfg.bRoamingEnhance = FALSE;
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef RED_SUPPORT
	pAd->red_en = TRUE;
	pAd->red_targetdelay = 20000;
	pAd->red_atm_on_targetdelay = 15000;
	pAd->red_atm_off_targetdelay = 20000;
	pAd->red_sta_num = 0;
	pAd->red_in_use_sta = 0;
#endif /* RED_SUPPORT */
	pAd->cp_support = 2;
#ifdef FQ_SCH_SUPPORT
	if ((!IS_MT7615(pAd) && (!(pAd->fq_ctrl.enable & FQ_READY)))) {
		pAd->fq_ctrl.enable = FQ_NEED_ON | FQ_NO_PKT_STA_KEEP_IN_LIST | FQ_ARRAY_SCH;
		pAd->fq_ctrl.factor = 2;
	}
#endif
#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
	pAd->ICapMode = 0;
	pAd->SpectrumEventCnt = 0;
	pAd->ICapStatus = 0;
	pAd->ICapEventCnt = 0;
	pAd->ICapDataCnt = 0;
	pAd->ICapIdx = 0;
	pAd->ICapCapLen = 0;
	pAd->ICapL32Cnt = 0;
	pAd->ICapM32Cnt = 0;
	pAd->ICapH32Cnt = 0;
	pAd->pL32Bit = NULL;
	pAd->pM32Bit = NULL;
	pAd->pH32Bit = NULL;
	pAd->pSrc_IQ = "/tmp/WifiSpectrum_IQ.txt";
	pAd->pSrc_Gain = "/tmp/WifiSpectrum_LNA_LPF.txt";
	/* Dynamic allocate memory for pIQ_Array buffer */
	{
		UINT32 retval, Len;
		RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

		Len = pChipCap->ICapMaxIQCnt * sizeof(RBIST_IQ_DATA_T);
		retval = os_alloc_mem(pAd, (UCHAR **)&pAd->pIQ_Array, Len);
		if (retval != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : Not enough memory for dynamic allocating !!\n", __func__));
		}
		os_zero_mem(pAd->pIQ_Array, Len);
	}
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
	pAd->CalFileOffset = 0;
#endif /* CAL_BIN_FILE_SUPPORT */

	/* ===================================================== */
#ifdef CONFIG_STA_SUPPORT
	/* Following code is needed for both STA mode and ApCli Mode */
	if ((IF_COMBO_HAVE_AP_STA(pAd)) || (IF_COMBO_HAVE_STA(pAd))) {
		pAd->MSTANum = 1;
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#ifdef RTMP_MAC_PCI
			pAd->RLnkCtrlOffset = 0;
			pAd->HostLnkCtrlOffset = 0;
			pAd->brt30xxBanMcuCmd = FALSE;
#endif
			RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_DIRECT);
			RX_FILTER_CLEAR_FLAG(pAd, fRX_FILTER_ACCEPT_MULTICAST);
			RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_BROADCAST);
			RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_ALL_MULTICAST);
			pAd->CommonCfg.NdisRadioStateOff = FALSE;
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WAKEUP_NOW);
#ifdef PROFILE_STORE
			pAd->bWriteDat = FALSE;
#endif /* PROFILE_STORE */
			pAd->RxAnt.Pair1AvgRssi[0] = pAd->RxAnt.Pair1AvgRssi[1] = 0;

			for (i = 0; i < DBDC_BAND_NUM; i++)
				pAd->Dot11_H[i].RDMode = RD_NORMAL_MODE;
		}

		for (i = 0; i < MAX_MULTI_STA; i++) {
			PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[i];
			struct adhoc_info *adhocInfo = &pStaCfg->adhocInfo;
			struct wifi_dev *wdev = &pStaCfg->wdev;
			SCAN_INFO *ScanInfo = &wdev->ScanInfo;

			pStaCfg->PwrMgmt.bDoze = FALSE;
			pStaCfg->CountDowntoPsm = 0;
#ifdef UAPSD_SUPPORT
			wdev->UapsdInfo.bAPSDCapable = FALSE;
#endif
			pStaCfg->PwrMgmt.Psm = PWR_ACTIVE;
			CLEAR_CIPHER(pStaCfg->PairwiseCipher);
			CLEAR_CIPHER(pStaCfg->GroupCipher);
			/* 802.1x port control*/
			pStaCfg->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
			wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			pStaCfg->LastMicErrorTime = 0;
			pStaCfg->MicErrCnt        = 0;
			pStaCfg->bBlockAssoc      = FALSE;
			pStaCfg->WpaState         = SS_NOTUSE;
			pStaCfg->RssiTrigger = 0;
			NdisZeroMemory(&pStaCfg->RssiSample, sizeof(RSSI_SAMPLE));
			pStaCfg->RssiTriggerMode = RSSI_TRIGGERED_UPON_BELOW_THRESHOLD;
			adhocInfo->AtimWin = 0;
			pStaCfg->DefaultListenCount = 3;/*default listen count;*/
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			pStaCfg->DefaultListenCount = 1;
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			pStaCfg->BssType = BSS_INFRA;  /* BSS_INFRA or BSS_ADHOC or BSS_MONITOR*/
			pStaCfg->bSkipAutoScanConn = FALSE;
			wdev->bAutoTxRateSwitch = TRUE;
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			pStaCfg->bAutoConnectIfNoSSID = FALSE;
			wdev->bLinkUpDone = FALSE;

			if (!pStaCfg->wdev.pEapolPktFromAP)
				os_alloc_mem(NULL,
							 (UCHAR **)&pStaCfg->wdev.pEapolPktFromAP,
							 sizeof(*pStaCfg->wdev.pEapolPktFromAP));
			else
				MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 ("non-NULL pEapolPktFromAP 0x%p\n",
						  pStaCfg->wdev.pEapolPktFromAP));

#ifdef EXT_BUILD_CHANNEL_LIST
			pStaCfg->IEEE80211dClientMode = Rt802_11_D_None;
#endif /* EXT_BUILD_CHANNEL_LIST */
			STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_INFRA_ON);
			/* user desired power mode*/
			pStaCfg->WindowsPowerMode = Ndis802_11PowerModeCAM;
			pStaCfg->WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
			pStaCfg->bWindowsACCAMEnable = FALSE;
			pStaCfg->bHwRadio = TRUE; /* Default Hardware Radio status is On*/
			pStaCfg->bSwRadio = TRUE; /* Default Software Radio status is On*/
			pStaCfg->bRadio = TRUE; /* bHwRadio && bSwRadio*/
			pStaCfg->bHardwareRadio = FALSE;        /* Default is OFF*/
			pStaCfg->bShowHiddenSSID = FALSE;       /* Default no show*/
			/* Nitro mode control*/
#if defined(NATIVE_WPA_SUPPLICANT_SUPPORT) || defined(RT_CFG80211_SUPPORT)
			pStaCfg->bAutoReconnect = FALSE;
#else
			pStaCfg->bAutoReconnect = TRUE;
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT || RT_CFG80211_SUPPORT*/
			/* Save the init time as last scan time, the system should do scan after 2 seconds.*/
			/* This patch is for driver wake up from standby mode, system will do scan right away.*/
			NdisGetSystemUpTime(&pStaCfg->LastScanTime);

			if (pStaCfg->LastScanTime > 10 * OS_HZ)
				pStaCfg->LastScanTime -= (10 * OS_HZ);

			NdisZeroMemory(pAd->nickname, IW_ESSID_MAX_SIZE + 1);
#ifdef WSC_STA_SUPPORT
			{
				INT                 idx;
				PWSC_CTRL           pWscControl;
#ifdef WSC_V2_SUPPORT
				PWSC_V2_INFO    pWscV2Info;
#endif /* WSC_V2_SUPPORT */
				/*
				 WscControl cannot be zero here, because WscControl timers are initial in MLME Initialize
				 and MLME Initialize is called before UserCfgInit.
				*/
				pWscControl = &pStaCfg->wdev.WscControl;
				pWscControl->WscConfMode = WSC_DISABLE;
				pWscControl->WscMode = WSC_PIN_MODE;
				pWscControl->WscConfStatus = WSC_SCSTATE_UNCONFIGURED;
#ifdef WSC_V2_SUPPORT
				pWscControl->WscConfigMethods = 0x238C;
#else
				pWscControl->WscConfigMethods = 0x008C;
#endif /* WSC_V2_SUPPORT */
				pWscControl->WscState = WSC_STATE_OFF;
				pWscControl->WscStatus = STATUS_WSC_NOTUSED;
				pWscControl->WscPinCode = 0;
				pWscControl->WscLastPinFromEnrollee = 0;
				pWscControl->WscEnrollee4digitPinCode = FALSE;
				pWscControl->WscEnrolleePinCode = 0;
				pWscControl->WscSelReg = 0;
				NdisZeroMemory(&pWscControl->RegData, sizeof(WSC_REG_DATA));
				NdisZeroMemory(&pWscControl->WscProfile, sizeof(WSC_PROFILE));
				pWscControl->WscUseUPnP = 0;
				pWscControl->WscEnAssociateIE = TRUE;
				pWscControl->WscEnProbeReqIE = TRUE;
				pWscControl->RegData.ReComputePke = 1;
				pWscControl->lastId = 1;
				pWscControl->EntryIfIdx = BSS0;
				pWscControl->WscDriverAutoConnect = 0x02;
				pAd->WriteWscCfgToDatFile = 0xFF;
				pWscControl->WscRejectSamePinFromEnrollee = FALSE;
				pWscControl->WpsApBand = PREFERRED_WPS_AP_PHY_TYPE_AUTO_SELECTION;
				pWscControl->bCheckMultiByte = FALSE;
				pWscControl->bWscAutoTigeer = FALSE;

				/* Enrollee Nonce, first generate and save to Wsc Control Block*/
				for (idx = 0; idx < 16; idx++)
					pWscControl->RegData.SelfNonce[idx] = RandomByte(pAd);

				pWscControl->WscRxBufLen = 0;
				pWscControl->pWscRxBuf = NULL;
				os_alloc_mem(pAd, &pWscControl->pWscRxBuf, MAX_MGMT_PKT_LEN);

				if (pWscControl->pWscRxBuf)
					NdisZeroMemory(pWscControl->pWscRxBuf, MAX_MGMT_PKT_LEN);

				pWscControl->WscTxBufLen = 0;
				pWscControl->pWscTxBuf = NULL;
				os_alloc_mem(pAd, &pWscControl->pWscTxBuf, MAX_MGMT_PKT_LEN);

				if (pWscControl->pWscTxBuf)
					NdisZeroMemory(pWscControl->pWscTxBuf, MAX_MGMT_PKT_LEN);

				pWscControl->bWscFragment = FALSE;
				pWscControl->WscFragSize = 128;
				initList(&pWscControl->WscPeerList);
				NdisAllocateSpinLock(pAd, &pWscControl->WscPeerListSemLock);
#ifdef WSC_V2_SUPPORT
				pWscV2Info = &pWscControl->WscV2Info;
				pWscV2Info->bWpsEnable = TRUE;
				pWscV2Info->ExtraTlv.TlvLen = 0;
				pWscV2Info->ExtraTlv.TlvTag = 0;
				pWscV2Info->ExtraTlv.pTlvData = NULL;
				pWscV2Info->ExtraTlv.TlvType = TLV_ASCII;
				pWscV2Info->bEnableWpsV2 = TRUE;
				pWscV2Info->bForceSetAP = FALSE;
#endif /* WSC_V2_SUPPORT */
			}
#endif /* WSC_STA_SUPPORT */
			NdisZeroMemory(pStaCfg->ReplayCounter, 8);
#ifdef DOT11R_FT_SUPPORT
			NdisZeroMemory(&pStaCfg->Dot11RCommInfo, sizeof(DOT11R_CMN_STRUC));
#endif /* DOT11R_FT_SUPPORT */
			pStaCfg->bAutoConnectByBssid = FALSE;
			pStaCfg->BeaconLostTime = BEACON_LOST_TIME;
			NdisZeroMemory(pStaCfg->WpaPassPhrase, 64);
			pStaCfg->WpaPassPhraseLen = 0;
			pStaCfg->bAutoRoaming = FALSE;
			pStaCfg->bForceTxBurst = FALSE;
			pStaCfg->bNotFirstScan = FALSE;
			ScanInfo->bImprovedScan = FALSE;
#ifdef DOT11_N_SUPPORT
			adhocInfo->bAdhocN = TRUE;
#endif /* DOT11_N_SUPPORT */
			pStaCfg->bFastConnect = FALSE;
			adhocInfo->bAdhocCreator = FALSE;
			pStaCfg->MlmeAux.OldChannel = 0;
#ifdef IP_ASSEMBLY
			pStaCfg->bFragFlag = TRUE;
#endif /* IP_ASSEMBLY */
		}
	}
#endif

#ifdef SMART_CARRIER_SENSE_SUPPORT
	if (IS_MT7615(pAd))
		pAd->SCSCtrl.SCSGeneration = SCS_Gen2;
	else if (IS_MT7622(pAd))
		pAd->SCSCtrl.SCSGeneration = SCS_Gen3;
#ifdef SCS_FW_OFFLOAD
	else if (IS_MT7626(pAd))
		pAd->SCSCtrl.SCSGeneration = SCS_Gen5;
#else
	else if (IS_MT7663(pAd) || IS_MT7626(pAd))
		pAd->SCSCtrl.SCSGeneration = SCS_Gen4;
#endif /* SCS_FW_OFFLOAD */
	else
		pAd->SCSCtrl.SCSGeneration = SCS_Gen1;

	/* SCS Variable initialization */
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		pAd->SCSCtrl.SCSEnable[i] = SCS_DISABLE;
		pAd->SCSCtrl.SCSTrafficThreshold[i] = TriggerTrafficeTh; /* 2M */
		pAd->SCSCtrl.SCSStatus[i] = PD_BLOCKING_OFF;
		pAd->SCSCtrl.OneSecTxByteCount[i] = 0;
		pAd->SCSCtrl.OneSecRxByteCount[i] = 0;
		pAd->SCSCtrl.CckPdBlkTh[i] = PdBlkCckThDefault;
		pAd->SCSCtrl.OfdmPdBlkTh[i] = PdBlkOfmdThDefault;
		pAd->SCSCtrl.SCSThTolerance[i] = ThTolerance;
		pAd->SCSCtrl.SCSMinRssiTolerance[i] = MinRssiTolerance;
		pAd->SCSCtrl.OfdmPdSupport[i] = TRUE;
		pAd->SCSCtrl.CckFalseCcaUpBond[i] = FalseCcaUpBondDefault;
		pAd->SCSCtrl.CckFalseCcaLowBond[i] = FalseCcaLowBondDefault;
		pAd->SCSCtrl.OfdmFalseCcaUpBond[i] = FalseCcaUpBondDefault;
		pAd->SCSCtrl.OfdmFalseCcaLowBond[i] = FalseCcaLowBondDefault;
		pAd->SCSCtrl.CckFixedRssiBond[i] = CckFixedRssiBondDefault;
		pAd->SCSCtrl.OfdmFixedRssiBond[i] = OfdmFixedRssiBondDefault;
		/*SCSGen4_for_MT7663*/
#if defined(MT7663) || defined(MT7626)
		if (pAd->SCSCtrl.SCSGeneration == SCS_Gen4) {
			pAd->SCSCtrl.PHY_MIN_PRI_PWR_OFFSET = MT7663_PHY_MIN_PRI_PWR_OFFSET;
			pAd->SCSCtrl.PHY_RXTD_CCKPD_7_OFFSET = MT7663_PHY_RXTD_CCKPD_7_OFFSET;
			pAd->SCSCtrl.PHY_RXTD_CCKPD_8_OFFSET = MT7663_PHY_RXTD_CCKPD_8_OFFSET;
		} else
#endif
		{
			pAd->SCSCtrl.PHY_MIN_PRI_PWR_OFFSET = 0;
			pAd->SCSCtrl.PHY_RXTD_CCKPD_7_OFFSET = 0;
			pAd->SCSCtrl.PHY_RXTD_CCKPD_8_OFFSET = 0;
		}
		/*SCSGen4_for_MT7663*/
	}
	pAd->SCSCtrl.SCSEnable[DBDC_BAND0] = SCS_ENABLE;
#endif /* SMART_CARRIER_SENSE_SUPPORT */

	for (i = 0; i < DBDC_BAND_NUM; i++)
		pAd->CCI_ACI_TxOP_Value[i] = 0;

	pAd->g_mode_txop_wdev = NULL;
	pAd->G_MODE_INFRA_TXOP_RUNNING = FALSE;
	pAd->MUMIMO_TxOP_Value = 0;
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		pAd->txop_ctl[i].multi_client_nums = 0;
		pAd->txop_ctl[i].multi_tcp_nums = 0;
		pAd->txop_ctl[i].cur_wdev = NULL;
		pAd->txop_ctl[i].multi_cli_txop_running = FALSE;
		pAd->txop_ctl[i].near_far_txop_running = FALSE;
	}

#ifdef PKT_BUDGET_CTRL_SUPPORT
	pAd->pbc_bound[PBC_AC_BE] = PBC_WMM_UP_DEFAULT_BE;
	pAd->pbc_bound[PBC_AC_BK] = PBC_WMM_UP_DEFAULT_BK;
	pAd->pbc_bound[PBC_AC_VO] = PBC_WMM_UP_DEFAULT_VO;
	pAd->pbc_bound[PBC_AC_VI] = PBC_WMM_UP_DEFAULT_VI;
	pAd->pbc_bound[PBC_AC_MGMT] = PBC_WMM_UP_DEFAULT_MGMT;
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
#ifdef TX_AGG_ADJUST_WKR
	pAd->TxAggAdjsut = TRUE;
#endif /* TX_AGG_ADJUST_WKR */
#ifdef HTC_DECRYPT_IOT
	pAd->HTC_ICV_Err_TH = 5;
#endif /* HTC_DECRYPT_IOT */
#ifdef DHCP_UC_SUPPORT
	pAd->DhcpUcEnable = FALSE;
#endif /* DHCP_UC_SUPPORT */

	for (i = 0; i < DBDC_BAND_NUM; i++) {
		pAd->CommonCfg.ucEDCCACtrl[i] = TRUE; /* EDCCA default is ON. */
	}
#ifdef DSCP_PRI_SUPPORT
	for (i = 0; i < 64; i++) {
		pAd->dscp_pri_map[DSCP_PRI_2G_MAP][i] = -1;
		pAd->dscp_pri_map[DSCP_PRI_5G_MAP][i] = -1;
	}
#endif
#ifdef AIR_MONITOR
	pAd->MntRuleBitMap = DEFAULT_MNTR_RULE;
#endif /* AIR_MONITOR */
#ifdef MBO_SUPPORT
	pAd->reg_domain = REG_GLOBAL;
#endif /* MBO_SUPPORT */
#ifdef WAPP_SUPPORT
	for (i = 0; i < DBDC_BAND_NUM; i++)
		pAd->bss_load_info.high_thrd[i] = MAX_BSSLOAD_THRD;
#endif /* WAPP_SUPPORT */
#ifdef FW_LOG_DUMP
	sprintf(pAd->fw_log_ctrl.fw_log_dest_dir, DEFAULT_FW_LOG_DESTINATION);
#endif /* FW_LOG_DUMP */
	/* CCSA performance enhancement */
	pAd->ccsa_overlapping = FALSE;
	pAd->ccsa_more_than_1bss = FALSE;
	pAd->ccsa_last_bssid_time = 0;
	pAd->ccsa_bw80_cnt = 0;
	NdisZeroMemory(pAd->ccsa_last_bssid, MAC_ADDR_LEN);
#ifdef RATE_PRIOR_SUPPORT
	DlListInit(&pAd->LowRateCtrl.BlackList);
	NdisAllocateSpinLock(pAd, &pAd->LowRateCtrl.BlackListLock);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n<--RATE_PRIOR AllocateSpinLock\n"));
#endif/*RATE_PRIOR_SUPPORT*/
#ifdef IXIA_SUPPORT
	pAd->chkTmr = 1;
	pAd->pktthld = 50;
	pAd->DeltaRssiTh = 10;/* initial as 10dBm*/
	pAd->MinRssiTh = -65;
	memset(&pAd->ixiaCtrl, 0, sizeof(IXIA_CTL));
	pAd->masktxop = FALSE;
#endif /*IXIA_SUPPORT*/

#ifdef MT_DFS_SUPPORT
	os_alloc_mem(pAd, (UCHAR **)&pAd->CommonCfg.DfsParameter.prRadarDetectionParam, sizeof(DFS_RADAR_DETECTION_PARAM));
	NdisZeroMemory(pAd->CommonCfg.DfsParameter.prRadarDetectionParam, sizeof(DFS_RADAR_DETECTION_PARAM));
#endif /*MT_DFS_SUPPORT*/
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<-- UserCfgInit\n"));
}


/* IRQL = PASSIVE_LEVEL*/
UCHAR BtoH(RTMP_STRING ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';        /* Handle numerals*/

	if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 0xA;  /* Handle capitol hex digits*/

	if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 0xA;  /* Handle small hex digits*/

	return 255;
}


/*
	FUNCTION: AtoH(char *, UCHAR *, int)

	PURPOSE:  Converts ascii string to network order hex

	PARAMETERS:
		src    - pointer to input ascii string
		dest   - pointer to output hex
		destlen - size of dest

	COMMENTS:

		2 ascii bytes make a hex byte so must put 1st ascii byte of pair
		into upper nibble and 2nd ascii byte of pair into lower nibble.

	IRQL = PASSIVE_LEVEL
*/
void AtoH(RTMP_STRING *src, PUCHAR dest, int destlen)
{
	RTMP_STRING *srcptr;
	PUCHAR destTemp;

	srcptr = src;
	destTemp = (PUCHAR) dest;

	while (destlen--) {
		*destTemp = BtoH(*srcptr++) << 4;    /* Put 1st ascii byte in upper nibble.*/
		*destTemp += BtoH(*srcptr++);      /* Add 2nd ascii byte to above.*/
		destTemp++;
	}
}


/*
========================================================================
Routine Description:
	Add a timer to the timer list.

Arguments:
	pAd				- WLAN control block pointer
	pRsc			- the OS resource

Return Value:
	None

Note:
========================================================================
*/
VOID RTMP_TimerListAdd(RTMP_ADAPTER *pAd, VOID *pRsc)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);

	while (1) {
		if (pObj == NULL)
			break;

		if ((ULONG)(pObj->pRscObj) == (ULONG)pRsc)
			return;

		pObj = pObj->pNext;
	}

	/* allocate a timer record entry */
	os_alloc_mem(NULL, (UCHAR **) &(pObj), sizeof(LIST_RESOURCE_OBJ_ENTRY));

	if (pObj == NULL) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: alloc timer obj fail!\n", __func__));
		return;
	} else {
		pObj->pRscObj = pRsc;
		insertTailList(pRscList, (RT_LIST_ENTRY *)pObj);
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: add timer obj %lx!\n", __func__, (ULONG)pRsc));
	}
}


VOID RTMP_TimerListRelease(RTMP_ADAPTER *pAd, VOID *pRsc)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	RT_LIST_ENTRY *pListEntry;

	pListEntry = pRscList->pHead;
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)pListEntry;

	while (pObj) {
		if ((ULONG)(pObj->pRscObj) == (ULONG)pRsc) {
			pListEntry = (RT_LIST_ENTRY *)pObj;
			break;
		}

		pListEntry = pListEntry->pNext;
		pObj = (LIST_RESOURCE_OBJ_ENTRY *)pListEntry;
	}

	if (pListEntry) {
		delEntryList(pRscList, pListEntry);
		/* free a timer record entry */
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: release timer obj %lx!\n", __func__, (ULONG)pRsc));
		os_free_mem(pObj);
	}
}



/*
* Show Timer Information
*/
VOID RTMPShowTimerList(RTMP_ADAPTER *pAd)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	RALINK_TIMER_STRUCT *pTimer;
	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Timer List Size:%d\n", pRscList->size));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=====================================\n"));

	while (1) {
		if (pObj == NULL)
			break;

		pTimer = (RALINK_TIMER_STRUCT *)pObj->pRscObj;
		pObj = pObj->pNext;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Valid:%d\n", pTimer->Valid));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pObj:%lx\n", (ULONG)pTimer));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PeriodicType:%d\n", pTimer->PeriodicType));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Repeat:%d\n", pTimer->Repeat));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("State:%d\n", pTimer->State));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TimerValue:%ld\n", pTimer->TimerValue));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("timer_lock:%lx\n", (ULONG)pTimer->timer_lock));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pCaller:%pS\n", pTimer->pCaller));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=====================================\n"));
	}
}



/*
========================================================================
Routine Description:
	Cancel all timers in the timer list.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RTMP_AllTimerListRelease(RTMP_ADAPTER *pAd)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj, *pObjOld;
	BOOLEAN Cancel;
	RALINK_TIMER_STRUCT *pTimer;
	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Size=%d\n", __func__, pRscList->size));

	while (1) {
		if (pObj == NULL)
			break;

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Cancel timer obj %lx!\n", __func__,
				 (ULONG)(pObj->pRscObj)));
		pObjOld = pObj;
		pObj = pObj->pNext;
		pTimer = (RALINK_TIMER_STRUCT *)pObjOld->pRscObj;
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Timer is allocated by %pS,Valid:%d,Lock:%lx,State:%d\n",
				 __func__,
				 pTimer->pCaller, pTimer->Valid, (ULONG)pTimer->timer_lock, pTimer->State));
		RTMPReleaseTimer(pObjOld->pRscObj, &Cancel);
	}

	/* reset TimerList */
	initList(&pAd->RscTimerCreateList);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pAd			Pointer to our adapter
		pTimer				Timer structure
		pTimerFunc			Function to execute when timer expired
		Repeat				Ture for period timer

	Return Value:
		None

	Note:

	========================================================================
*/
VOID RTMPInitTimer(
	IN	RTMP_ADAPTER *pAd,
	IN	RALINK_TIMER_STRUCT *pTimer,
	IN	VOID *pTimerFunc,
	IN	VOID *pData,
	IN	BOOLEAN	 Repeat)
{
	pTimer->timer_lock = &pAd->TimerSemLock;
	RTMP_SEM_LOCK(pTimer->timer_lock);
	RTMP_TimerListAdd(pAd, pTimer);
	/* Set Valid to TRUE for later used.*/
	/* It will crash if we cancel a timer or set a timer */
	/* that we haven't initialize before.*/
	/* */
	pTimer->Valid      = TRUE;
	pTimer->PeriodicType = Repeat;
	pTimer->State      = FALSE;
	pTimer->cookie = (ULONG) pData;
	pTimer->pAd = pAd;
	pTimer->pCaller = (VOID *)OS_TRACE;
	RTMP_OS_Init_Timer(pAd, &pTimer->TimerObj,	pTimerFunc, (PVOID) pTimer, &pAd->RscTimerMemList);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: %lx\n", __func__, (ULONG)pTimer));
	RTMP_SEM_UNLOCK(pTimer->timer_lock);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pTimer				Timer structure
		Value				Timer value in milliseconds

	Return Value:
		None

	Note:
		To use this routine, must call RTMPInitTimer before.

	========================================================================
*/
VOID RTMPSetTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value)
{
	if (!pTimer->timer_lock)
		return;

	RTMP_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid) {
		RTMP_ADAPTER *pAd;

		pAd = (RTMP_ADAPTER *)pTimer->pAd;

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPSetTimer failed, Halt in Progress!\n"));
			RTMP_SEM_UNLOCK(pTimer->timer_lock);
			return;
		}

		pTimer->TimerValue = Value;
		pTimer->State      = FALSE;

		if (pTimer->PeriodicType == TRUE) {
			pTimer->Repeat = TRUE;
			RTMP_SetPeriodicTimer(&pTimer->TimerObj, Value);
		} else {
			pTimer->Repeat = FALSE;
			RTMP_OS_Add_Timer(&pTimer->TimerObj, Value);
		}

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: %lx\n", __func__, (ULONG)pTimer));
	} else
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPSetTimer failed, Timer hasn't been initialize! <caller: %pS>\n", __builtin_return_address(0)));

	RTMP_SEM_UNLOCK(pTimer->timer_lock);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pTimer				Timer structure
		Value				Timer value in milliseconds

	Return Value:
		None

	Note:
		To use this routine, must call RTMPInitTimer before.

	========================================================================
*/
VOID RTMPModTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value)
{
	BOOLEAN	Cancel;

	if (!pTimer->timer_lock)
		return;

	RTMP_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid) {
		pTimer->TimerValue = Value;
		pTimer->State      = FALSE;

		if (pTimer->PeriodicType == TRUE) {
			RTMP_SEM_UNLOCK(pTimer->timer_lock);
			RTMPCancelTimer(pTimer, &Cancel);
			RTMPSetTimer(pTimer, Value);
		} else {
			RTMP_OS_Mod_Timer(&pTimer->TimerObj, Value);
			RTMP_SEM_UNLOCK(pTimer->timer_lock);
		}

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: %lx\n", __func__, (ULONG)pTimer));
	} else {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPModTimer failed, Timer hasn't been initialize!\n"));
		RTMP_SEM_UNLOCK(pTimer->timer_lock);
	}
}


/*
	========================================================================

	Routine Description:
		Cancel timer objects

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:
		1.) To use this routine, must call RTMPInitTimer before.
		2.) Reset NIC to initial state AS IS system boot up time.

	========================================================================
*/
VOID RTMPCancelTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled)
{
	if (!pTimer->timer_lock)
		goto end;

	RTMP_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid) {
		if (pTimer->State == FALSE)
			pTimer->Repeat = FALSE;

		RTMP_SEM_UNLOCK(pTimer->timer_lock);
		RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);
		RTMP_SEM_LOCK(pTimer->timer_lock);

		if (*pCancelled == TRUE)
			pTimer->State = TRUE;

#ifdef RTMP_TIMER_TASK_SUPPORT
		/* We need to go-through the TimerQ to findout this timer handler and remove it if */
		/*		it's still waiting for execution.*/
		RtmpTimerQRemove(pTimer->pAd, pTimer);
#endif /* RTMP_TIMER_TASK_SUPPORT */
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: %lx\n", __func__, (ULONG)pTimer));
	}

	RTMP_SEM_UNLOCK(pTimer->timer_lock);
	return;
end:
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("RTMPCancelTimer failed, Timer hasn't been initialize!\n"));
}


VOID RTMPReleaseTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled)
{
	if (!pTimer->timer_lock)
		goto end;

	RTMP_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid) {
		if (pTimer->State == FALSE)
			pTimer->Repeat = FALSE;

		RTMP_SEM_UNLOCK(pTimer->timer_lock);
		RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);
		RTMP_SEM_LOCK(pTimer->timer_lock);

		if (*pCancelled == TRUE)
			pTimer->State = TRUE;

#ifdef RTMP_TIMER_TASK_SUPPORT
		/* We need to go-through the TimerQ to findout this timer handler and remove it if */
		/*		it's still waiting for execution.*/
		RtmpTimerQRemove(pTimer->pAd, pTimer);
#endif /* RTMP_TIMER_TASK_SUPPORT */
		/* release timer */
		RTMP_OS_Release_Timer(&pTimer->TimerObj);
		pTimer->Valid = FALSE;
		/* TODO: shiang-usw, merge this from NXTC, make sure if that's necessary here!! */
		RTMP_TimerListRelease(pTimer->pAd, pTimer);
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: %lx\n", __func__, (ULONG)pTimer));
	}

	RTMP_SEM_UNLOCK(pTimer->timer_lock);
end:
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("RTMPReleasefailed, Timer hasn't been initialize!\n"));
}


/*
	========================================================================

	Routine Description:
		Enable RX

	Arguments:
		pAd						Pointer to our adapter

	Return Value:
		None

	IRQL <= DISPATCH_LEVEL

	Note:
		Before Enable RX, make sure you have enabled Interrupt.
	========================================================================
*/
VOID RTMPEnableRxTx(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==> RTMPEnableRxTx\n"));
	RT28XXDMAEnable(pAd);
	AsicSetRxFilter(pAd);

	if (pAd->CommonCfg.bTXRX_RXV_ON)
		AsicSetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, TRUE);
	else
		AsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<== RTMPEnableRxTx\n"));
}


void CfgInitHook(RTMP_ADAPTER *pAd)
{
	/*pAd->bBroadComHT = TRUE;*/
}

static INT RtmpChipOpsRegister(RTMP_ADAPTER *pAd, INT infType)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	int ret = 0;

	NdisZeroMemory(cap, sizeof(RTMP_CHIP_CAP));
	ret = RtmpChipOpsHook(pAd);

	if (ret) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("chip ops hook error\n"));
		return ret;
	}

	/* MCU related */
	ChipOpsMCUHook(pAd, cap->MCUType);
	get_dev_config_idx(pAd);
	return ret;
}




PNET_DEV get_netdev_from_bssid(RTMP_ADAPTER *pAd, UCHAR wdev_idx)
{
	PNET_DEV dev_p = NULL;

	if (wdev_idx < WDEV_NUM_MAX)
		dev_p = pAd->wdev_list[wdev_idx]->if_dev;

	ASSERT((dev_p != NULL));
	return dev_p;
}


INT RtmpRaDevCtrlInit(VOID *pAdSrc, RTMP_INF_TYPE infType)
{
	RTMP_ADAPTER *pAd = (PRTMP_ADAPTER)pAdSrc;
#ifdef FW_DUMP_SUPPORT
	pAd->fw_dump_max_size = MAX_FW_DUMP_SIZE;
	RTMP_OS_FWDUMP_PROCINIT(pAd);
#endif
	/* Assign the interface type. We need use it when do register/EEPROM access.*/
	pAd->infType = infType;
#ifdef CONFIG_STA_SUPPORT
	pAd->OpMode = OPMODE_STA;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("STA Driver version-%s\n", STA_DRIVER_VERSION));
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	pAd->OpMode = OPMODE_AP;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AP Driver version-%s\n", AP_DRIVER_VERSION));
#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pAd->infType=%d\n", pAd->infType));
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
	pAd->OpMode = OPMODE_STA;
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT || CFG80211_MULTI_STA */

	pAd->iface_combinations = 0;
#ifdef CONFIG_STA_SUPPORT
	pAd->iface_combinations |= HAVE_STA_INF;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	pAd->iface_combinations |= HAVE_AP_INF;
#endif /* CONFIG_AP_SUPPORT */

	RTMP_SEM_EVENT_INIT(&(pAd->AutoRateLock), &pAd->RscSemMemList);

	if (RtmpChipOpsRegister(pAd, infType))
		return FALSE;

	/*prepeare hw resource depend on chipcap*/
	hdev_resource_init(pAd->hdev_ctrl);

	AsicGetMacInfo(pAd, &pAd->ChipID, &pAd->HWVersion, &pAd->FWVersion);
	/*initial wlan hook module*/
	WLAN_HOOK_INIT();

	/* dynamic allocation of ChipCap-dependent data structure */
	if (alloc_chip_cap_dep_data(pAd) != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Failed to allocate memory - ChipCap-dependent data structure\n"));
		return FALSE;
	}

	if (hif_data_init(pAd) != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Hif data struct init fail \n"));
		return FALSE;
	}

#ifdef RESOURCE_PRE_ALLOC

	/*
		move this function from mt_wifi_init() to here. now this function only allocate memory and
		leave the initialization job to RTMPInitTxRxRingMemory() which called in mt_wifi_init().
	*/
	if (RTMPAllocTxRxRingMemory(pAd) != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Failed to allocate memory - TxRxRing\n"));
		return FALSE;
	}

#endif /* RESOURCE_PRE_ALLOC */
#ifdef MCS_LUT_SUPPORT

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCS_LUT)) {
		if (MAX_LEN_OF_MAC_TABLE < 128)
			RTMP_SET_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT);
		else {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(): MCS_LUT not used becasue MacTb size(%d) > 128!\n",
					 __func__, MAX_LEN_OF_MAC_TABLE));
		}
	}

#endif /* MCS_LUT_SUPPORT */

	return 0;
}


BOOLEAN RtmpRaDevCtrlExit(IN VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	INT index;
#ifdef CONFIG_STA_SUPPORT
#ifdef CREDENTIAL_STORE
	NdisFreeSpinLock(&pAd->StaCtIf.Lock);
#endif /* CREDENTIAL_STORE */
#endif /* CONFIG_STA_SUPPORT */
	RTMP_SEM_EVENT_DESTORY(&(pAd->AutoRateLock));

	/*
		Free ProbeRespIE Table
	*/
	for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++) {
		if (pAd->ProbeRespIE[index].pIe)
			os_free_mem(pAd->ProbeRespIE[index].pIe);
	}

#ifdef RESOURCE_PRE_ALLOC
	RTMPFreeTxRxRingMemory(pAd);
#endif /* RESOURCE_PRE_ALLOC */
#ifdef FW_DUMP_SUPPORT
	RTMP_OS_FWDUMP_PROCREMOVE(pAd);

	if (pAd->fw_dump_buffer) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Free FW dump buffer\n", __func__));
		os_free_mem(pAd->fw_dump_buffer);
		pAd->fw_dump_buffer = 0;
		pAd->fw_dump_size = 0;
		pAd->fw_dump_read = 0;
	}

#endif
	free_chip_cap_dep_data(pAd);
	wpf_config_exit(pAd);
	RTMPFreeAdapter(pAd);
	return TRUE;
}


#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID RTMP_11N_D3_TimerInit(RTMP_ADAPTER *pAd)
{
	RTMPInitTimer(pAd, &pAd->CommonCfg.Bss2040CoexistTimer, GET_TIMER_FUNCTION(Bss2040CoexistTimeOut), pAd, FALSE);
}

VOID RTMP_11N_D3_TimerRelease(RTMP_ADAPTER *pAd)
{
	BOOLEAN Cancel;

	RTMPReleaseTimer(&pAd->CommonCfg.Bss2040CoexistTimer, &Cancel);
}

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

VOID AntCfgInit(RTMP_ADAPTER *pAd)
{
#if defined(ANT_DIVERSITY_SUPPORT) || defined(TXRX_SW_ANTDIV_SUPPORT)
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				 __func__, __LINE__));
		return;
	}

#ifdef TXRX_SW_ANTDIV_SUPPORT
	/* EEPROM 0x34[15:12] = 0xF is invalid, 0x2~0x3 is TX/RX SW AntDiv */
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: bTxRxSwAntDiv %d\n", __func__, cap->bTxRxSwAntDiv));

	if (cap->bTxRxSwAntDiv) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Antenna word %X/%d, AntDiv %d\n",
				 pAd->Antenna.word, pAd->Antenna.field.BoardType, pAd->NicConfig2.field.AntDiversity));
	}

#endif /* TXRX_SW_ANTDIV_SUPPORT */
	{
		if (pAd->NicConfig2.field.AntOpt == 1) { /* ant selected by efuse */
			if (pAd->NicConfig2.field.AntDiversity == 0) { /* main */
				pAd->RxAnt.Pair1PrimaryRxAnt = 0;
				pAd->RxAnt.Pair1SecondaryRxAnt = 1;
			} else { /* aux */
				pAd->RxAnt.Pair1PrimaryRxAnt = 1;
				pAd->RxAnt.Pair1SecondaryRxAnt = 0;
			}
		} else if (pAd->NicConfig2.field.AntDiversity == 0) { /* Ant div off: default ant is main */
			pAd->RxAnt.Pair1PrimaryRxAnt = 0;
			pAd->RxAnt.Pair1SecondaryRxAnt = 1;
		} else if (pAd->NicConfig2.field.AntDiversity == 1) /* Ant div on */
		{/* eeprom on, but sw ant div support is not enabled: default ant is main */
			pAd->RxAnt.Pair1PrimaryRxAnt = 0;
			pAd->RxAnt.Pair1SecondaryRxAnt = 1;
		}
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: primary/secondary ant %d/%d\n",
			 __func__,
			 pAd->RxAnt.Pair1PrimaryRxAnt,
			 pAd->RxAnt.Pair1SecondaryRxAnt));
}


