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
	cmm_data.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */


#include "rt_config.h"
#ifdef TXRX_STAT_SUPPORT
#include "hdev/hdev_basic.h"
#endif

UCHAR	SNAP_802_1H[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
UCHAR	SNAP_BRIDGE_TUNNEL[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8};
UCHAR	EAPOL[] = {0x88, 0x8e};
UCHAR   TPID[] = {0x81, 0x00}; /* VLAN related */

UCHAR	IPX[] = {0x81, 0x37};
UCHAR	APPLE_TALK[] = {0x80, 0xf3};

/* UserPriority To AccessCategory mapping */
UCHAR WMM_UP2AC_MAP[8] = {QID_AC_BE, QID_AC_BK,
						  QID_AC_BK, QID_AC_BE,
						  QID_AC_VI, QID_AC_VI,
						  QID_AC_VO, QID_AC_VO
						 };

#ifdef VLAN_SUPPORT
VOID update_rxblk_addr(RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *) pRxBlk->FC;

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
		if ((FC->ToDs == 0) && (FC->FrDs == 0)) {
			pRxBlk->Addr1 = pRxBlk->pData;
			pRxBlk->Addr2 = pRxBlk->pData + 6;
		} else if ((FC->ToDs == 0) && (FC->FrDs == 1)) {
			pRxBlk->Addr1 = pRxBlk->pData;
			pRxBlk->Addr3 = pRxBlk->pData + 6;
		} else if ((FC->ToDs == 1) && (FC->FrDs == 0)) {
			pRxBlk->Addr2 = pRxBlk->pData + 6;
			pRxBlk->Addr3 = pRxBlk->pData;
		} else {
			pRxBlk->Addr2 = pRxBlk->FC + 2;
			pRxBlk->Addr3 = pRxBlk->pData;
			pRxBlk->Addr4 = pRxBlk->pData + 6;
		}
	} else {
		pRxBlk->Addr1 = pRxBlk->FC + 4; /*4 byte Frame Control*/
		pRxBlk->Addr2 = pRxBlk->FC + 4 + MAC_ADDR_LEN;
		pRxBlk->Addr3 = pRxBlk->FC + 4 + MAC_ADDR_LEN * 2;
		if ((FC->ToDs == 1) && (FC->FrDs == 1))
			pRxBlk->Addr4 = pRxBlk->FC + 6 + MAC_ADDR_LEN * 3;
	}

}
VOID remove_vlan_hw_padding(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	/*For IOT, remove unused fields*/
	if (pRxBlk->pRxPacket) {
		UCHAR byte0, byte1, extra_field_offset;

		extra_field_offset = IS_VLAN_PACKET(GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket)) ? (LENGTH_802_3_NO_TYPE+LENGTH_802_1Q) : LENGTH_802_3_NO_TYPE;
		/*Remove the extra field (2 Bytes) which is added by hardware*/
		/*The added info is the length of actual data (without overhead)*/
		byte0 = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket)[extra_field_offset];
		byte1 = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket)[extra_field_offset+1];
		/*If there is the extra field, remove it*/
		if (((byte0<<8) | byte1) == GET_OS_PKT_LEN(pRxBlk->pRxPacket) - extra_field_offset - 2) { /*2 : len of extra field*/
			NdisMoveMemory(GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket) + 2, GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket), extra_field_offset);
			RtmpOsSkbPullRcsum(pRxBlk->pRxPacket, 2);
			RtmpOsSkbResetNetworkHeader(pRxBlk->pRxPacket);
			RtmpOsSkbResetTransportHeader(pRxBlk->pRxPacket);
			RtmpOsSkbResetMacLen(pRxBlk->pRxPacket);
			pRxBlk->pData = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket);
			pRxBlk->DataSize -= 2;
			update_rxblk_addr(pRxBlk);
		}
		/*End of remove extra field*/
	}
}

VOID remove_vlan_tag(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	NdisMoveMemory(GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket) + LENGTH_802_1Q, GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket), LENGTH_802_3_NO_TYPE);
	RtmpOsSkbPullRcsum(pRxBlk->pRxPacket, LENGTH_802_1Q);
	RtmpOsSkbResetNetworkHeader(pRxBlk->pRxPacket);
	RtmpOsSkbResetTransportHeader(pRxBlk->pRxPacket);
	RtmpOsSkbResetMacLen(pRxBlk->pRxPacket);
	pRxBlk->pData = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket);
	pRxBlk->DataSize -= LENGTH_802_1Q;
	update_rxblk_addr(pRxBlk);
}

VOID rebuild_802_11_eapol_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{

	HEADER_802_11 buf;
	HEADER_802_11 *pHhdr80211 = &buf;
	UCHAR hdr_len;
	UCHAR *pData;
	char addr4[6];

	NdisZeroMemory(pHhdr80211, sizeof(HEADER_802_11));
	pHhdr80211->FC = *((FRAME_CONTROL *) pRxBlk->FC);
	COPY_MAC_ADDR(pHhdr80211->Addr1, pRxBlk->Addr1);
	COPY_MAC_ADDR(pHhdr80211->Addr2, pRxBlk->Addr2);
	COPY_MAC_ADDR(pHhdr80211->Addr3, pRxBlk->Addr3);

	if (pHhdr80211->FC.FrDs == 1 && pHhdr80211->FC.ToDs == 1) {
		COPY_MAC_ADDR(addr4, pRxBlk->Addr4);
		hdr_len = LENGTH_802_11_WITH_ADDR4;
	} else
		hdr_len = LENGTH_802_11;

	if (pHhdr80211->FC.SubType & 0x08) /*Is QoS Packet*/
		hdr_len += LENGTH_WMMQOS_H + 6; /*QoS Control & SNAP*/
	else
		hdr_len += 6; /*SNAP*/

	skb_push(pRxBlk->pRxPacket, hdr_len - LENGTH_802_3_NO_TYPE);
	RtmpOsSkbResetNetworkHeader(pRxBlk->pRxPacket);
	RtmpOsSkbResetTransportHeader(pRxBlk->pRxPacket);
	RtmpOsSkbResetMacLen(pRxBlk->pRxPacket);

	pRxBlk->pData = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket);
	pRxBlk->DataSize += (hdr_len - LENGTH_802_3_NO_TYPE);
	pRxBlk->FC = pRxBlk->pData;

	NdisZeroMemory(GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket), hdr_len);
	pData = pRxBlk->pData;
	NdisMoveMemory(pData, pHhdr80211, sizeof(HEADER_802_11));
	pData += sizeof(HEADER_802_11);

	if (pHhdr80211->FC.FrDs == 1 && pHhdr80211->FC.ToDs == 1) {
		NdisMoveMemory(pData, addr4, 6);
		pData += 6; /*addr4*/
	}

	/*QoS Control : 2 byte*/
	if (pHhdr80211->FC.SubType & 0x08) {
		*pData = pRxBlk->UserPriority;
		pData += LENGTH_WMMQOS_H;
	}
	NdisMoveMemory(pData, SNAP_802_1H, 6);
	RX_BLK_CLEAR_FLAG(pRxBlk, fRX_HDR_TRANS);
	update_rxblk_addr(pRxBlk);
}

#endif /*VLAN_SUPPORT*/



#ifdef DBG_DIAGNOSE
VOID dbg_diag_deque_log(RTMP_ADAPTER *pAd)
{
	struct dbg_diag_info *diag_info;
#ifdef DBG_TXQ_DEPTH
	UCHAR QueIdx = 0;
#endif
	diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];
#ifdef DBG_TXQ_DEPTH
	if ((pAd->DiagStruct.diag_cond & DIAG_COND_TXQ_DEPTH) != DIAG_COND_TXQ_DEPTH) {
		if ((pAd->DiagStruct.wcid > 0) &&  (pAd->DiagStruct.wcid < MAX_LEN_OF_TR_TABLE)) {
			STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pAd->DiagStruct.wcid];
			UCHAR swq_cnt;

			for (QueIdx = 0; QueIdx < 4; QueIdx++) {
				if (tr_entry->tx_queue[QueIdx].Number <= 7)
					swq_cnt = tr_entry->tx_queue[QueIdx].Number;
				else
					swq_cnt = 8;

				diag_info->TxSWQueCnt[QueIdx][swq_cnt]++;
			}
		}
	}

#endif /* DBG_TXQ_DEPTH */
}
#endif /* DBG_DIAGNOSE */

VOID dump_rxblk(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Dump RX_BLK Structure:\n"));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tHW rx info:\n"));
	hex_dump("RawData", &pRxBlk->hw_rx_info[0], RXD_SIZE);
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tData Pointer info:\n"));

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\trmac_info=0x%p\n", pRxBlk->rmac_info));
		dump_rmac_info(pAd, pRxBlk->rmac_info);
	}

	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tpRxPacket=0x%p, MPDUtotalByteCnt=%d\n", pRxBlk->pRxPacket, pRxBlk->MPDUtotalByteCnt));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tpData=0x%p\n", pRxBlk->pData));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tDataSize=%d\n", pRxBlk->DataSize));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tFlags=0x%x\n", pRxBlk->Flags));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tUserPriority=%d\n", pRxBlk->UserPriority));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tOpMode=%d\n", pRxBlk->OpMode));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tMirror Info from RMAC Info:\n"));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tWCID=%d\n", pRxBlk->wcid));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tTID=%d\n", pRxBlk->TID));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tKey_idx=%d\n", pRxBlk->key_idx));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tBSS_IDX=%d\n", pRxBlk->bss_idx));
	/* hex_dump("Dump RxPacket in dump_rxblk", (UCHAR *)pRxBlk->pHeader, pRxBlk->MPDUtotalByteCnt > 512 ? 512 : pRxBlk->MPDUtotalByteCnt); */
}


VOID dump_txblk(RTMP_ADAPTER *pAd, TX_BLK *txblk)
{
	/* hex_dump("TxBlk Raw Data", (UCHAR *)txblk, sizeof(TX_BLK)); */
	printk("TxBlk Info\n");
	printk("\twdev=%p\n", txblk->wdev);
	printk("\tWCID=%d\n", txblk->Wcid);
	printk("\tWMM_Idx=%d\n", txblk->WmmIdx);
	printk("\tQueIdx=%d\n", txblk->QueIdx);
	printk("\tWMM_Set=%d\n", txblk->wmm_set);
	printk("\tpMacEntry=%p\n", txblk->pMacEntry);

	if (txblk->pMacEntry) {
		printk("\t\tpMacEntry->wcid=%d\n", txblk->pMacEntry->wcid);
		printk("\t\tpMacEntry->tr_tb_idx=%d\n", txblk->pMacEntry->tr_tb_idx);
	}

	printk("\tTR_Entry=%p\n", txblk->tr_entry);

	if (txblk->tr_entry)
		printk("\t\tTR_Entry->wcid=%d\n", txblk->tr_entry->wcid);

	printk("\tOpMode=%d\n", txblk->OpMode);
	printk("\tTxFrameType=%d\n", txblk->TxFrameType);
	printk("\tTotalFragNum=%d\n", txblk->TotalFragNum);
	printk("\tUserPriority=%d\n", txblk->UserPriority);
}


#ifdef MT_MAC

static VOID ParseRxVStat_v2(RTMP_ADAPTER *pAd, RX_STATISTIC_RXV *rx_stat, UCHAR *Data)
{
	RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = NULL;
	RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = NULL;
	RX_VECTOR1_3TH_CYCLE *RXV1_3TH_CYCLE = NULL;
	RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = NULL;
	RX_VECTOR1_5TH_CYCLE *RXV1_5TH_CYCLE = NULL;
	RX_VECTOR1_6TH_CYCLE *RXV1_6TH_CYCLE = NULL;
	RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = NULL;
	RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = NULL;
	RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = NULL;
	INT16 foe = 0;
	UINT32 i = 0;
	UINT8 cbw = 0;
	UINT32 foe_const = 0;

	RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(Data + 8);
	RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 12);
	RXV1_3TH_CYCLE = (RX_VECTOR1_3TH_CYCLE *)(Data + 16);
	RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(Data + 20);
	RXV1_5TH_CYCLE = (RX_VECTOR1_5TH_CYCLE *)(Data + 24);
	RXV1_6TH_CYCLE = (RX_VECTOR1_6TH_CYCLE *)(Data + 28);
	RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(Data + 32);
	RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(Data + 36);
	RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(Data + 40);

	if (RXV1_1ST_CYCLE->TxMode == MODE_CCK) {
		foe = (RXV1_5TH_CYCLE->MISC1 & 0x7ff);
		foe = (foe * 1000) >> 11;
	} else {
		cbw = RXV1_1ST_CYCLE->FrMode;
		foe_const = ((1 << (cbw + 1)) & 0xf) * 10000;
		foe = (RXV1_5TH_CYCLE->MISC1 & 0xfff);

		if (foe >= 2048)
			foe = foe - 4096;

		foe = (foe * foe_const) >> 15;
	}

	rx_stat->FreqOffsetFromRx = foe;
	rx_stat->RCPI[0] = RXV1_4TH_CYCLE->RCPI0;
	rx_stat->RCPI[1] = RXV1_4TH_CYCLE->RCPI1;
	rx_stat->RCPI[2] = RXV1_4TH_CYCLE->RCPI2;
	rx_stat->RCPI[3] = RXV1_4TH_CYCLE->RCPI3;
	rx_stat->FAGC_RSSI_IB[0] = RXV1_3TH_CYCLE->IBRssiRx;
	rx_stat->FAGC_RSSI_WB[0] = RXV1_3TH_CYCLE->WBRssiRx;
	rx_stat->FAGC_RSSI_IB[1] = RXV1_3TH_CYCLE->IBRssiRx;
	rx_stat->FAGC_RSSI_WB[1] = RXV1_3TH_CYCLE->WBRssiRx;
	rx_stat->FAGC_RSSI_IB[2] = RXV1_3TH_CYCLE->IBRssiRx;
	rx_stat->FAGC_RSSI_WB[2] = RXV1_3TH_CYCLE->WBRssiRx;
	rx_stat->FAGC_RSSI_IB[3] = RXV1_3TH_CYCLE->IBRssiRx;
	rx_stat->FAGC_RSSI_WB[3] = RXV1_3TH_CYCLE->WBRssiRx;
	rx_stat->SNR[0] = (RXV1_5TH_CYCLE->MISC1 >> 19) - 16;

	for (i = 0; i < 4; i++) {
		if (rx_stat->FAGC_RSSI_IB[i] >= 128)
			rx_stat->FAGC_RSSI_IB[i] -= 256;

		if (rx_stat->FAGC_RSSI_WB[i] >= 128)
			rx_stat->FAGC_RSSI_WB[i] -= 256;
	}
}

VOID parse_RXV_packet_v2(RTMP_ADAPTER *pAd, UINT32 Type, RX_BLK *RxBlk, UCHAR *Data)
{
	RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = NULL;
	RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = NULL;
	RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = NULL;
	BOOLEAN parse_rssi = TRUE;
#ifdef LTF_SNR_SUPPORT
	RX_VECTOR1_5TH_CYCLE *RXV1_5TH_CYCLE = NULL;
	INT32 SNR = 0;
#endif
	if (Type == RMAC_RX_PKT_TYPE_RX_NORMAL) {
		RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)Data;
		RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 4);

#ifdef WIFI_DIAG
		/* get SNR from RXV6 */
		if (VALID_UCAST_ENTRY_WCID(pAd, RxBlk->wcid))
			DiagGetSnr(pAd, RxBlk->wcid, Data + 20);
#endif
		if (((FRAME_CONTROL *)RxBlk->FC)->Type == FC_TYPE_DATA) {
			parse_rssi = FALSE;
#ifdef AIR_MONITOR
			if (pAd->MntEnable && VALID_WCID(RxBlk->wcid)) {
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[RxBlk->wcid];

				if (IS_ENTRY_MONITOR(pEntry))
					parse_rssi = TRUE;
			}
#endif /* AIR_MONITOR */
		}
	} else if (Type == RMAC_RX_PKT_TYPE_RX_TXRXV) {
		RXV_DWORD0 *DW0 = NULL;
		RXV_DWORD1 *DW1 = NULL;
		RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = NULL;
		RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = NULL;
		RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = NULL;
		RX_STATISTIC_RXV *rx_stat = &pAd->rx_stat_rxv;

		DW0 = (RXV_DWORD0 *)Data;
		DW1 = (RXV_DWORD1 *)(Data + 4);
		RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(Data + 8);
		RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 12);
		RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(Data + 32);
		RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(Data + 36);
		RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(Data + 40);
		RxBlk->rxv2_cyc1 = *(UINT32 *)RXV2_1ST_CYCLE;
		RxBlk->rxv2_cyc2 = *(UINT32 *)RXV2_2ND_CYCLE;
		RxBlk->rxv2_cyc3 = *(UINT32 *)RXV2_3TH_CYCLE;
		pAd->rxv2_cyc3[(DW1->RxvSn % 10)] = RxBlk->rxv2_cyc3;

		if (pAd->parse_rxv_stat_enable)
			ParseRxVStat_v2(pAd, rx_stat, Data);

#ifdef CONFIG_ATE

		if (ATE_ON(pAd))
			MT_ATEUpdateRxStatistic(pAd, TESTMODE_RXV, Data);

#endif /* CONFIG_ATE */
	} else {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s(): invalid Type %u\n", __func__, Type));
		return; /* return here to avoid dereferencing NULL pointer below */
	}

#ifdef TXRX_STAT_SUPPORT
	if ((parse_rssi == TRUE) || (pAd->TXRX_EnableReadRssi && (parse_rssi == FALSE)))
#else
	if (parse_rssi)
#endif
	{
		RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(Data + 12);
		RxBlk->rx_signal.raw_rssi[0] = (RXV1_4TH_CYCLE->RCPI0 - 220) / 2;
		RxBlk->rx_signal.raw_rssi[1] = (RXV1_4TH_CYCLE->RCPI1 - 220) / 2;
		RxBlk->rx_signal.raw_rssi[2] = (RXV1_4TH_CYCLE->RCPI2 - 220) / 2;
		RxBlk->rx_signal.raw_rssi[3] = (RXV1_4TH_CYCLE->RCPI3 - 220) / 2;
	}

#ifdef AUTOMATION
	if (pAd->auto_dvt &&
		pAd->auto_dvt->rxv.enable &&
		(Type == RMAC_RX_PKT_TYPE_RX_NORMAL) &&
		(((FRAME_CONTROL *)RxBlk->FC)->Type == FC_TYPE_DATA) &&
		(((FRAME_CONTROL *)RxBlk->FC)->SubType != SUBTYPE_DATA_NULL) &&
		OS_NTOHS(*((UINT16 *)((PUCHAR)(RxBlk->pData + 12)))) != ETH_P_ARP)
		rxv_correct_test(Data);
#endif /* AUTOMATION */

	RxBlk->rx_rate.field.MODE = RXV1_1ST_CYCLE->TxMode;
	RxBlk->rx_rate.field.MCS = RXV1_1ST_CYCLE->TxRate;
	RxBlk->rx_rate.field.ldpc = RXV1_1ST_CYCLE->HtAdCode;
	RxBlk->rx_rate.field.BW = RXV1_1ST_CYCLE->FrMode;
	RxBlk->rx_rate.field.STBC = RXV1_1ST_CYCLE->HtStbc ? 1 : 0;
	RxBlk->rx_rate.field.ShortGI = RXV1_1ST_CYCLE->HtShortGi;

	if ((RxBlk->rx_rate.field.MODE == MODE_VHT) && (RxBlk->rx_rate.field.STBC == 0))
		RxBlk->rx_rate.field.MCS |= ((RXV1_2ND_CYCLE->NstsField & 0x3) << 4);

#if defined(MT7615) || defined(MT7622) || defined(MT7663) || defined(MT7626)
	if (pAd->ucRxRateRecordEn) {
		UINT8 MCS, Steam;
#ifdef DOT11_VHT_AC
			if (RxBlk->rx_rate.field.MODE >= MODE_VHT) {
				MCS = (RxBlk->rx_rate.field.MCS & 0xf);
				Steam  = (RxBlk->rx_rate.field.MCS >> 4) + 1;
				pAd->arRateVHT[Steam][MCS]++;
			} else
#endif /* DOT11_VHT_AC */
				if (RxBlk->rx_rate.field.MODE == MODE_OFDM) {
					MCS = getLegacyOFDMMCSIndex(RxBlk->rx_rate.field.MCS);
					Steam  = (RxBlk->rx_rate.field.MCS >> 4) + 1;
					pAd->arRateOFDM[MCS]++;
				} else {
					MCS = (RxBlk->rx_rate.field.MCS % 8);
					Steam  = (RxBlk->rx_rate.field.MCS >> 3) + 1;
					if (RxBlk->rx_rate.field.MODE == MODE_CCK)
						pAd->arRateCCK[MCS]++;
					else
						pAd->arRateHT[Steam][MCS]++;
				}
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: Rx Rate record enable %u\n", __func__, pAd->ucRxRateRecordEn));
	}

	if (pAd->ucRxvRecordEn) {
		RecordRxVB(pAd, RxBlk, Data, Type);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: RXV record enable %u, Type %u\n", __func__, pAd->ucRxvRecordEn, Type));
	}
#endif /* #if defined(MT7615) || defined(MT7622) || defined(MT7663) defined(MT7626) */
/* Calculate SNR */

}

#if defined(MT7615) || defined(MT7622) || defined(MT7663) || defined(MT7626)
VOID RecordRxVB(
	RTMP_ADAPTER *pAd,
	RX_BLK *RxBlk,
	UCHAR *Data,
	UINT32 Type)
{
	PRxVBQElmt prxvbqelmt = NULL;

	if (Type == RMAC_RX_PKT_TYPE_RX_NORMAL) {
		RXD_BASE_STRUCT *rx_base = (RXD_BASE_STRUCT *)RxBlk->rmac_info;

		if (rx_base->RxD3.RxVSeq > 0) {
			if (pAd->ucRxvCurSN != 0) {
				prxvbqelmt = RxVBQueueSearch(pAd, pAd->ucRxvCurSN, Type);
				if (prxvbqelmt->valid != 0) {
					prxvbqelmt->rxblk_setting_done = TRUE;
				}
			}
			pAd->ucRxvCurSN = rx_base->RxD3.RxVSeq;
			prxvbqelmt = RxVBQueueSearch(pAd, pAd->ucRxvCurSN, Type);
			prxvbqelmt->wcid = RxBlk->wcid;
			prxvbqelmt->rxv_sn = rx_base->RxD3.RxVSeq;
			prxvbqelmt->timestamp = RxBlk->TimeStamp;
			prxvbqelmt->aggcnt = 1;
			prxvbqelmt->valid = 1;
		} else {
			prxvbqelmt = RxVBQueueSearch(pAd, pAd->ucRxvCurSN, Type);
			if (prxvbqelmt->valid != 0)
				prxvbqelmt->aggcnt++;
		}

		if (rx_base->RxD2.FcsErr && prxvbqelmt->valid != 0) {
			UINT8 i;
			for (i = 0; i < 8; i++)
				if (prxvbqelmt->aggcnt > 32*i && prxvbqelmt->aggcnt <= 32*(i+1))
					prxvbqelmt->arFCScheckBitmap[i] |= (rx_base->RxD2.FcsErr << (prxvbqelmt->aggcnt-1-(32*i)));
		}
	} else if (Type == RMAC_RX_PKT_TYPE_RX_TXRXV) {
		RXV_DWORD1 *DW1 = (RXV_DWORD1 *)(Data + 4);
		UINT8 i;

		prxvbqelmt = RxVBQueueSearch(pAd, DW1->RxvSn, Type);
		for (i = 0; i < 9; i++)
			prxvbqelmt->RXV_CYCLE[i] = *(UINT32 *)(Data + 8 + 4*i);
		prxvbqelmt->rxv_sn = DW1->RxvSn;
		prxvbqelmt->rxv_setting_done = TRUE;
		prxvbqelmt->valid = 1;
	} else
		return;

	/* Sync Rxblk & RxV and dequeue to file. */
	while (TRUE) {
		prxvbqelmt = RxVBDequeue(pAd);
		if (prxvbqelmt == NULL)
			break;

		if (pAd->ucRecordStart == TRUE) {
			pAd->ucRecordStart = FALSE;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RXV record start...\n"));
		}

		if ((prxvbqelmt->wcid == pAd->ucRxvWcid) || (pAd->ucRxvWcid == 0)) {
			if (pAd->ucRxvMode == 0)
				RxVB_Report_Action(pAd, RxBlk->wcid, prxvbqelmt);
			else
				RTEnqueueInternalCmd(pAd, CMDTHRED_RXV_WRITE_IN_FILE, (VOID *)prxvbqelmt, sizeof(RxVBQElmt));

			NdisZeroMemory(prxvbqelmt, sizeof(RxVBQElmt));
			pAd->prxvbcurelmt++;
			pAd->u4RxvCurCnt++;

			if (pAd->prxvbcurelmt->isEnd == TRUE)
				pAd->prxvbcurelmt = pAd->prxvbstartelmt;
		}
	}
}

PRxVBQElmt RxVBQueueSearch(
	IN	PRTMP_ADAPTER pAd,
	IN	UINT8		rxv_sn,
	IN	UINT32		Type)
{
	UINT8 fgSettingDone = FALSE;
	UINT8 fgSearchDone = FALSE;
	PRxVBQElmt ptmprxvbelmt = pAd->prxvbcurelmt;

	while (ptmprxvbelmt->valid != 0) {
		if (Type == RMAC_RX_PKT_TYPE_RX_NORMAL) {
			fgSettingDone = ptmprxvbelmt->rxblk_setting_done;
			fgSearchDone = ptmprxvbelmt->rxblk_search_done;
		} else if (Type == RMAC_RX_PKT_TYPE_RX_TXRXV) {
			fgSettingDone = ptmprxvbelmt->rxv_setting_done;
			fgSearchDone = ptmprxvbelmt->rxv_search_done;
		}

		if (ptmprxvbelmt->rxv_sn == rxv_sn && fgSettingDone == FALSE && fgSearchDone == FALSE)
			break;

		if (Type == RMAC_RX_PKT_TYPE_RX_NORMAL)
			ptmprxvbelmt->rxblk_search_done = TRUE;
		else if (Type == RMAC_RX_PKT_TYPE_RX_TXRXV)
			ptmprxvbelmt->rxv_search_done = TRUE;

		ptmprxvbelmt++;

		if (ptmprxvbelmt->isEnd == TRUE)
			ptmprxvbelmt = pAd->prxvbstartelmt;
	}

	return ptmprxvbelmt;
}

PRxVBQElmt RxVBDequeue(
	IN PRTMP_ADAPTER pAd)
{
	PRxVBQElmt pcurrxvbqelmt = pAd->prxvbcurelmt;
	UINT8 fgCurSetting = FALSE;
	UINT16 u2count = 0;
	UINT16 i;

	while (TRUE) {
		if (pcurrxvbqelmt->valid == 0) {
			u2count = 0;
			break;
		}

		fgCurSetting = (pcurrxvbqelmt->rxblk_setting_done & pcurrxvbqelmt->rxv_setting_done);

		if (fgCurSetting == TRUE)
			break;

		pcurrxvbqelmt++;
		u2count++;

		if (pcurrxvbqelmt->isEnd == TRUE)
			pcurrxvbqelmt = pAd->prxvbstartelmt;
	}

	for (i = 0; i < u2count; i++) {
		NdisZeroMemory(pAd->prxvbcurelmt, sizeof(RxVBQElmt));
		pAd->prxvbcurelmt++;

		if (pAd->prxvbcurelmt->isEnd == TRUE)
			pAd->prxvbcurelmt = pAd->prxvbstartelmt;
	}
	fgCurSetting = (pAd->prxvbcurelmt->rxblk_setting_done & pAd->prxvbcurelmt->rxv_setting_done);
	if (pAd->prxvbcurelmt->valid != 0 && fgCurSetting) {
		return pAd->prxvbcurelmt;
	}
	return NULL;
}

VOID RxVB_Report_Action(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			wcid,
	IN PRxVBQElmt 		prxvbqelmt)
{
    UCHAR FrameBuf[256], udpchecksum[256];
    UCHAR s_addr[MAC_ADDR_LEN];
	UCHAR d_addr[MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    UCHAR ETH_P_AIR_MONITOR[LENGTH_802_3_TYPE] = {0x08, 0x00};
    UINT32 frame_len = 0, offset = 0, count = 0;
    NDIS_PACKET *skb = NULL;
	IP_V4_HDR ipv4_hdr = {0};
	UINT16 Flag_and_Frag_Offset = 0;
	UINT8 ttl = 64;
	UINT8 protocol = 17; /* UDP */
	UINT16 checksum = 0;
	UINT32 source_ip, destination_ip;
	UINT16 source_port, destination_port, length;


	prxvbqelmt->g0_debug_set = pAd->ucRxvG0;
	prxvbqelmt->g1_debug_set = pAd->ucRxvG1;
	prxvbqelmt->g2_debug_set = pAd->ucRxvG2;
	if (IS_MT7615(pAd))
		sprintf(prxvbqelmt->chipid, "7615");
	else if (IS_MT7622(pAd))
		sprintf(prxvbqelmt->chipid, "7622");
	else if (IS_MT7663(pAd))
		sprintf(prxvbqelmt->chipid, "7663");
	else if (IS_MT7626(pAd))
		sprintf(prxvbqelmt->chipid, "7626");
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s : This tool only support 7615/7622/7663.\n", __func__));
		return;
	}
    /* Init frame buffer */
	NdisZeroMemory(FrameBuf, sizeof(FrameBuf));

    /* Fake a Source Address for transmission */
#ifdef CONFIG_AP_SUPPORT
	COPY_MAC_ADDR(s_addr, pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.if_addr);
#else
	COPY_MAC_ADDR(s_addr, pAd->StaCfg[0].wdev.if_addr);
#endif /* CONFIG_AP_SUPPORT */
    if (s_addr[1] == 0xff)
		s_addr[1] = 0;
    else
		s_addr[1]++;

    /* Prepare the 802.3 header */
	MAKE_802_3_HEADER(FrameBuf, d_addr, s_addr, ETH_P_AIR_MONITOR);
	offset += LENGTH_802_3;

	/* Prepare IP header */
	ipv4_hdr.version = 4;
	ipv4_hdr.ihl = 5;
	ipv4_hdr.tos = 0;
	ipv4_hdr.tot_len = htons(sizeof(RxVBQElmt) + 20 + 8);
	ipv4_hdr.identifier = 0;
	source_ip = htonl(0x00000000);
	destination_ip = htonl(0xFFFFFFFF);
	Flag_and_Frag_Offset = htons(0x4000);

	NdisCopyMemory(&FrameBuf[offset], (CHAR *)&ipv4_hdr, sizeof(IP_V4_HDR));
	offset += sizeof(IP_V4_HDR);
	NdisCopyMemory(&FrameBuf[offset], (CHAR *)&Flag_and_Frag_Offset, sizeof(UINT16));
	offset += sizeof(UINT16);
	FrameBuf[offset] = ttl;
	offset += sizeof(UINT8);
	FrameBuf[offset] = protocol;
	udpchecksum[count] = 0;
	udpchecksum[count + 1] = protocol;
	offset += sizeof(UINT8);
	count += sizeof(UINT16);

	offset += sizeof(UINT16);
	NdisCopyMemory(&FrameBuf[offset], (CHAR *)&source_ip, sizeof(UINT32));
	NdisCopyMemory(&udpchecksum[count], (CHAR *)&source_ip, sizeof(UINT32));
	offset += sizeof(UINT32);
	count += sizeof(UINT32);
	NdisCopyMemory(&FrameBuf[offset], (CHAR *)&destination_ip, sizeof(UINT32));
	NdisCopyMemory(&udpchecksum[count], (CHAR *)&destination_ip, sizeof(UINT32));
	offset += sizeof(UINT32);
	count += sizeof(UINT32);

	checksum = Checksum16(&FrameBuf[LENGTH_802_3], offset - LENGTH_802_3);
	NdisCopyMemory(&FrameBuf[LENGTH_802_3 + sizeof(IP_V4_HDR) + sizeof(UINT16) + sizeof(UINT8) + sizeof(UINT8)],
		(CHAR *)&checksum, sizeof(UINT16));

	/* Prepare UDP header */
	source_port = htons(54321);
	destination_port = htons(55688);
	length = htons(8 + sizeof(RxVBQElmt));
	NdisCopyMemory(&FrameBuf[offset], (CHAR *)&source_port, sizeof(UINT16));
	NdisCopyMemory(&udpchecksum[count], (CHAR *)&source_port, sizeof(UINT16));
	offset += sizeof(UINT16);
	count += sizeof(UINT16);
	NdisCopyMemory(&FrameBuf[offset], (CHAR *)&destination_port, sizeof(UINT16));
	NdisCopyMemory(&udpchecksum[count], (CHAR *)&destination_port, sizeof(UINT16));
	offset += sizeof(UINT16);
	count += sizeof(UINT16);
	NdisCopyMemory(&FrameBuf[offset], (CHAR *)&length, sizeof(UINT16));
	NdisCopyMemory(&udpchecksum[count], (CHAR *)&length, sizeof(UINT16));
	offset += sizeof(UINT16);
	count += sizeof(UINT16);
	NdisCopyMemory(&udpchecksum[count], (CHAR *)&length, sizeof(UINT16));
	count += sizeof(UINT16);
	NdisCopyMemory(&FrameBuf[offset], (CHAR *)&checksum, sizeof(UINT16));
	offset += sizeof(UINT16);

    /* Prepare payload*/
	NdisCopyMemory(&FrameBuf[offset], (CHAR *)prxvbqelmt, sizeof(RxVBQElmt));
	offset += sizeof(RxVBQElmt);
	NdisCopyMemory(&udpchecksum[count], (CHAR *)prxvbqelmt, sizeof(RxVBQElmt));
	count += sizeof(RxVBQElmt);
	frame_len = offset;


	checksum = Checksum16(&udpchecksum[0], count);
	NdisCopyMemory(&FrameBuf[LENGTH_802_3 + sizeof(IP_V4_HDR) + sizeof(UINT16) + sizeof(UINT8) + sizeof(UINT8) + sizeof(UINT16) + sizeof(UINT32) + sizeof(UINT32) + sizeof(UINT16) * 3],
		(CHAR *)&checksum, sizeof(UINT16));

    /* Create skb */
	skb = RtmpOSNetPktAlloc(pAd, frame_len);
	if (!skb) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : Error! Can't allocate a skb.\n", __FUNCTION__));
		return;
	}
#ifdef CONFIG_AP_SUPPORT
	SET_OS_PKT_NETDEV(skb, pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.if_dev);
#else
	SET_OS_PKT_NETDEV(skb, pAd->StaCfg[0].wdev.if_dev);
#endif /* CONFIG_AP_SUPPORT */


    /* Insert the frame content */
	NdisMoveMemory(GET_OS_PKT_DATAPTR(skb), FrameBuf, frame_len);

    /* End this frame */
	OS_PKT_TAIL_BUF_EXTEND(skb, frame_len);

    /* Report to upper layer */
	RtmpOsPktProtocolAssign(skb);
	RtmpOsPktRcvHandle(skb);
}

UINT16 Checksum16(UINT8 *pData, int len)
{
	int sum = 0;

	while (len > 1) {
		sum += *((UINT16 *)pData);

		pData = pData + 2;

		if (sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);

		len -= 2;
	}

	if (len)
		sum += *((UINT8 *)pData);

    while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return ~sum;
}

UINT16 UdpChecksum16(UINT8 *pData, int len)
{
	UINT16 offset = 0;
	UCHAR tmp[512] = {0};

	return Checksum16(&tmp[0], offset);
}
#endif /* #if defined(MT7615) || defined(MT7622) || defined(MT7663) || defined(MT7626)*/

/* this function ONLY if not allow pn replay attack and drop packet */
static BOOLEAN check_rx_pkt_pn_allowed(RTMP_ADAPTER *pAd, RX_BLK *rx_blk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	BOOLEAN isAllow = TRUE;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)rx_blk->FC;

	if (pFmeCtrl->Wep == 0)
		return TRUE;

	if (!VALID_UCAST_ENTRY_WCID(pAd, rx_blk->wcid))
		return TRUE;

	pEntry = &pAd->MacTab.Content[rx_blk->wcid];

	if (!pEntry || !pEntry->wdev)
		return TRUE;

	if (rx_blk->key_idx > 3) {
		return TRUE;
	}

	if (rx_blk->pRxInfo->Mcast || rx_blk->pRxInfo->Bcast) {
		UINT32 GroupCipher = pEntry->SecConfig.GroupCipher;
		UCHAR key_idx = rx_blk->key_idx;

		if (IS_CIPHER_CCMP128(GroupCipher) || IS_CIPHER_CCMP256(GroupCipher) ||
			IS_CIPHER_GCMP128(GroupCipher) || IS_CIPHER_GCMP256(GroupCipher) ||
			IS_CIPHER_TKIP(GroupCipher)) {

			if (pEntry->Init_CCMP_BC_PN_Passed[key_idx] == FALSE) {
					if (rx_blk->CCMP_PN >= pEntry->CCMP_BC_PN[key_idx]) {
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BC, %s (%d)(%d) OK: 1st come-in the %llu and now is %llu\n",
							__FUNCTION__, pEntry->wcid, key_idx, rx_blk->CCMP_PN, pEntry->CCMP_BC_PN[key_idx]));
						pEntry->CCMP_BC_PN[key_idx] = rx_blk->CCMP_PN;
						pEntry->Init_CCMP_BC_PN_Passed[key_idx] = TRUE;
					} else {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BC, %s (%d)(%d) Reject: 1st come-in the %llu and now is %llu\n",
						__FUNCTION__, pEntry->wcid, key_idx, rx_blk->CCMP_PN, pEntry->CCMP_BC_PN[key_idx]));
						isAllow = FALSE;
					}
			} else {
				if (rx_blk->CCMP_PN <= pEntry->CCMP_BC_PN[key_idx]) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BC, %s (%d)(%d) Reject: come-in the %llu and now is %llu\n",
						__FUNCTION__, pEntry->wcid, key_idx, rx_blk->CCMP_PN, pEntry->CCMP_BC_PN[key_idx]));
					isAllow = FALSE;
				} else {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BC, %s (%d)(%d) OK: come-in the %llu and now is %llu\n",
						__FUNCTION__, pEntry->wcid, key_idx, rx_blk->CCMP_PN, pEntry->CCMP_BC_PN[key_idx]));
					pEntry->CCMP_BC_PN[key_idx] = rx_blk->CCMP_PN;
				}
			}
		}
	}
#ifdef PN_UC_REPLAY_DETECTION_SUPPORT
	else {
		UCHAR TID = rx_blk->TID;
		if (rx_blk->CCMP_PN < pEntry->CCMP_UC_PN[TID]) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("UC, %s (%d) Reject: come-in the %llu and now is %llu\n",
				__FUNCTION__, pEntry->wcid, rx_blk->CCMP_PN, pEntry->CCMP_UC_PN[TID]));
			isAllow = FALSE;
		} else {
			pEntry->CCMP_UC_PN[TID] = rx_blk->CCMP_PN;
		}
	}
#endif /* PN_UC_REPLAY_DETECTION_SUPPORT */

	return isAllow;
}

#define NUM_TYPE_STRING 8
UCHAR *rx_pkt_type_string[NUM_TYPE_STRING] = {
	"RMAC_RX_PKT_TYPE_RX_TXS", "RMAC_RX_PKT_TYPE_RX_TXRXV", "RMAC_RX_PKT_TYPE_RX_NORMAL",
	"RMAC_RX_PKT_TYPE_RX_DUP_RFB", "RMAC_RX_PKT_TYPE_RX_TMR", "Undefine Type 0x5",
	"Undefine Type 0x6", "RMAC_RX_PKT_TYPE_RX_EVENT"
};


#ifdef CUT_THROUGH
#define GET_TX_FREE_TOKEN_ID(_ptr, _idx) le2cpu16(*(UINT16 *)(((UINT8 *)(_ptr)) + 2 * (_idx)))
#define GET_RX_ORDER_TOKEN_ID(_ptr, _idx) le2cpu16(*(UINT16 *)(((UINT8 *)(_ptr)) + 4 * (_idx)))
#define GET_RX_ORDER_DROP(_ptr, _idx) ((*(UINT8 *)(((UINT8 *)(_ptr)) + 4 * (_idx) + 3)) & 0x01)

static VOID EventTxFreeNotifyHandler(RTMP_ADAPTER *pAd, UINT8 *dataPtr, UINT32 TxDCnt, UINT32 Len, UINT8 LenPerToken)
{
	UINT loop;
	PNDIS_PACKET pkt;
	UINT16 *token_ptr, token_id;
	PKT_TOKEN_CB *pktTokenCb = pAd->PktTokenCb;
	UINT8 Type;
	struct qm_ops *qm_ops = pAd->qm_ops;
	UINT32 free_num;

	if (dataPtr == NULL) {
		ASSERT(0);
		return;
	}

	pktTokenCb->tx_id_list.list->TotalTxTokenEventCnt++;
	pktTokenCb->tx_id_list.list->TotalTxTokenCnt += TxDCnt;

	for (loop = 0; loop < TxDCnt; loop++) {
		token_ptr = (UINT16 *)dataPtr;
		token_id = le2cpu16(*token_ptr);

		/* shift position according to the version of tx done event */
		dataPtr += LenPerToken;

		if (token_id > pktTokenCb->pkt_tx_tkid_max)
			continue;

#ifdef CONFIG_HOTSPOT_R2

		/* already handled , do nothing */
		if (pktTokenCb->tx_id_list.list->pkt_token[token_id].Reprocessed) {
			pktTokenCb->tx_id_list.list->pkt_token[token_id].Reprocessed = FALSE;
			continue;
		}

#endif /* CONFIG_HOTSPOT_R2 */
		pkt = cut_through_tx_deq(pAd->PktTokenCb, token_id, &Type);
#ifdef CUT_THROUGH_DBG
		/* debug only: measure the time interval between Pkt sended to free-notity comed back. */
		NdisGetSystemUpTime(&pktTokenCb->tx_id_list.list->pkt_token[token_id].endTime);
		MTWF_LOG(DBG_CAT_TOKEN, TOKEN_PROFILE, DBG_LVL_TRACE,
				 ("%s: tx time token_id[%d] = %ld %ld %ld OS_HZ=%d\n",
				  __func__, token_id,
				  pktTokenCb->tx_id_list.list->pkt_token[token_id].endTime -
				  pktTokenCb->tx_id_list.list->pkt_token[token_id].startTime,
				  pktTokenCb->tx_id_list.list->pkt_token[token_id].endTime,
				  pktTokenCb->tx_id_list.list->pkt_token[token_id].startTime,
				  OS_HZ));
		MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE,
				 ("%s: token_id[%d] = 0x%x, try to free TxPkt(%p)\n",
				  __func__, loop, token_id, pkt));
#endif
#ifdef CONFIG_ATE

		if (ATE_ON(pAd))
			MT_ATETxControl(pAd, 0xFF, pkt);

#endif

		if (pkt != NULL) {
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
			if (!RTMP_GET_PACKET_MGMT_PKT(pkt)) {
				if ((RTMP_GET_PACKET_TYPE(pkt) != TX_ALTX)
					&& (!RTMP_GET_PACKET_HIGH_PRIO(pkt))) {
				UCHAR wcid = pAd->vow_queue_map[token_id][0];
				UCHAR qidx = pAd->vow_queue_map[token_id][1];

				if ((wcid > 0 && wcid < MAX_LEN_OF_MAC_TABLE) &&
					(qidx >= 0 && qidx < WMM_NUM_OF_AC)) {
					/* Workaround for vow dvt issue */
					if (pAd->vow_queue_len[wcid][qidx] > 0)
						pAd->vow_queue_len[wcid][qidx]--;
					pAd->vow_queue_map[token_id][0] = 0xff;
					pAd->vow_queue_map[token_id][1] = 0xf;
					pAd->vow_tx_free[wcid]++;
				}
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("\x1b[31m%s: tokenid %d, wcid %d, qidx %d\x1b[m\n",
					__func__, token_id, wcid, qidx));
				}
			}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */

#ifdef RED_SUPPORT_BY_HOST
			/* maintain RA/AC Tail Drop counter */
			if (!pAd->red_have_cr4)
				red_tx_free_handle(pAd, pkt);
#endif

#ifdef CONFIG_ATE

			if (ATE_ON(pAd))
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
			else
#endif
			{
#ifdef FQ_SCH_SUPPORT
				if (!RTMP_GET_PACKET_MGMT_PKT(pkt)) {
					if ((RTMP_GET_PACKET_TYPE(pkt) != TX_ALTX)
						&& (!RTMP_GET_PACKET_HIGH_PRIO(pkt))) {
					RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

					if ((cap->qm == FAST_PATH_FAIR_QM) || (cap->qm == GENERIC_FAIR_QM))
						fq_tx_free_per_packet(pAd, RTMP_GET_PACKET_QUEIDX(pkt),
								      RTMP_GET_PACKET_WCID(pkt), pkt);
					}
				}
#endif
				if (Type == TOKEN_TX_DATA)
					RELEASE_NDIS_PACKET_IRQ(pAd, pkt, NDIS_STATUS_SUCCESS);
				else
					RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
			}
		} else {
			MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
					 ("%s: Get a token_id[%d] = 0x%x but PktPtr is NULL!\n",
					  __func__, loop, token_id));
			hex_dump("EventTxFreeNotifyHandlerNullPktPtrFrame", dataPtr, Len);
		}
	}

	free_num = pktTokenCb->tx_id_list.list->FreeTokenCnt;
	if (cut_through_get_token_state(pAd) == TX_TOKEN_LOW && free_num >= pktTokenCb->TxTokenHighWaterMark) {
		cut_through_set_token_state(pAd, TX_TOKEN_HIGH);
		qm_ops->schedule_tx_que(pAd);
	} else if (tx_flow_check_if_blocked(pAd)) {
		qm_ops->schedule_tx_que(pAd);
	}
}

static VOID EventRxOrderNotifyNotifyHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 RxDCnt, UINT32 Len)
{
	UINT loop;
	UINT16 token_id;
	UINT16 drop;
	PKT_TOKEN_CB *pktTokenCb = pAd->PktTokenCb;
#ifdef RX_CUT_THROUGH
	UINT8 Type;
#endif

	if (!CUT_THROUGH_RX_ENABL(pktTokenCb)) {
		MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR, ("%s(): CutThrough Rx is disabled but still get RxReorderNotify!\n", __func__));
		ASSERT(FALSE);
		return;
	}

	if (Data == NULL) {
		ASSERT(FALSE);
		return;
	}

#ifdef CUT_THROUGH_DBG

	if ((RxDCnt >= 0) && (RxDCnt < 32))
		pktTokenCb->rx_id_list.list->FreeAgg0_31++;
	else if ((RxDCnt >= 32) && (RxDCnt < 64))
		pktTokenCb->rx_id_list.list->FreeAgg32_63++;
	else if ((RxDCnt >= 64) && (RxDCnt < 96))
		pktTokenCb->rx_id_list.list->FreeAgg64_95++;
	else if ((RxDCnt >= 96) && (RxDCnt < 128))
		pktTokenCb->rx_id_list.list->FreeAgg96_127++;

#endif

	for (loop = 0; loop < RxDCnt; loop++) {
		if (loop * 4 > Len) {
			MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
					 ("%s: token number len mismatch RxD Cnt=%d Len=%d\n",
					  __func__, loop, Len));
			break;
		}

		token_id = GET_RX_ORDER_TOKEN_ID(Data, loop);
		drop = GET_RX_ORDER_DROP(Data, loop);
		MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE, ("%s: token_id[%d] = 0x%x\n",
				 __func__, loop, token_id));
#ifdef RX_CUT_THROUGH
		RTMP_SEM_LOCK(&((PKT_TOKEN_CB *)(pAd->PktTokenCb))->rx_order_notify_lock);

		if (cut_through_rx_mark_token_info(pAd->PktTokenCb, token_id, drop) &&
			cut_through_rx_rxdone(pAd->PktTokenCb, token_id)) {
			PNDIS_PACKET pkt;
			RX_BLK *pRxBlk;

			pkt = cut_through_rx_deq(pAd->PktTokenCb, token_id, &Type);
			RTMP_SEM_UNLOCK(&((PKT_TOKEN_CB *)(pAd->PktTokenCb))->rx_order_notify_lock);

			if (pkt) {
				if (drop) {
#ifdef CUT_THROUGH_DBG
					pktTokenCb->rx_id_list.list->DropPktCnt++;
#endif
					RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_RESOURCES);
#ifdef IXIA_SUPPORT
					pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_RX_ORDER]++;
#endif /*IXIA_SUPPORT*/
				} else {
					pRxBlk = (RX_BLK *)((UINT8 *)GET_OS_PKT_DATAPTR(pkt) - sizeof(RX_BLK) - RTMP_GET_RMACLEN(pkt));
					rx_packet_process(pAd, pkt, pRxBlk);
					MTWF_LOG(DBG_CAT_TOKEN, TOKEN_PROFILE, DBG_LVL_TRACE, ("%s: inorder time[%d] = %ld\n",
							 __func__, token_id, cut_through_inorder_time(pAd->PktTokenCb, token_id)));
				}
			}
		} else
			RTMP_SEM_UNLOCK(&((PKT_TOKEN_CB *)(pAd->PktTokenCb))->rx_order_notify_lock);

#endif /* RX_CUT_THROUGH */
	}

	return;
}
#endif /* CUT_THROUGH */

static UINT32 rx_data_handler(RTMP_ADAPTER *pAd, RX_BLK *rx_blk, VOID *rx_packet)
{
	/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
#ifdef IXIA_SUPPORT
		pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_RX_HALT]++;
#endif /*IXIA_SUPPORT*/
		RELEASE_NDIS_PACKET(pAd, rx_packet, NDIS_STATUS_SUCCESS);
		return NDIS_STATUS_FAILURE;
	}

	if (asic_trans_rxd_into_rxblk(pAd, rx_blk, rx_packet) == 0) {
		RELEASE_NDIS_PACKET(pAd, rx_packet, NDIS_STATUS_SUCCESS);
#ifdef IXIA_SUPPORT
		pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_RXMAC_LEN]++;
#endif /*IXIA_SUPPORT*/
		return NDIS_STATUS_FAILURE;
	}


	if (header_packet_process(pAd, rx_packet, rx_blk) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	rx_packet_process(pAd, rx_packet, rx_blk);
	return NDIS_STATUS_SUCCESS;
}

UINT32 rxv_handler(RTMP_ADAPTER *pAd, RX_BLK *rx_blk, VOID *rx_packet)
{
	RMAC_RXD_0_TXRXV *rxv = (RMAC_RXD_0_TXRXV *)(rx_packet);
	UCHAR *ptr;
	INT idx;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_LOUD,
			 ("RxV Report: Number=%d, ByteCnt=%d\n",
			  rxv->RxvCnt, rxv->RxByteCnt));
#if (ENABLE_RXD_LOG)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RxV Report: Number=%d, ByteCnt=%d\n",
			  rxv->RxvCnt, rxv->RxByteCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_0_RxV: 0x%x\n", *((UINT32 *)rxv + 0)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_1_RxV: 0x%x\n", *((UINT32 *)rxv + 1)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_2_RxV: 0x%x\n", *((UINT32 *)rxv + 2)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_3_RxV: 0x%x\n", *((UINT32 *)rxv + 3)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_4_RxV: 0x%x\n", *((UINT32 *)rxv + 4)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_5_RxV: 0x%x\n", *((UINT32 *)rxv + 5)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_6_RxV: 0x%x\n", *((UINT32 *)rxv + 6)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_7_RxV: 0x%x\n", *((UINT32 *)rxv + 7)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_8_RxV: 0x%x\n", *((UINT32 *)rxv + 8)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_9_RxV: 0x%x\n", *((UINT32 *)rxv + 9)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_10_RxV: 0x%x\n", *((UINT32 *)rxv + 10)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_11_RxV: 0x%x\n", *((UINT32 *)rxv + 11)));
#endif

	if (rxv->RxByteCnt != (rxv->RxvCnt * 44 + 4)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("ReceivedByteCnt not equal rxv_entry required!\n"));
	} else {
		ptr = (UCHAR *)(rx_packet + 4);
#ifdef RT_BIG_ENDIAN

		if ((rxv->RxByteCnt - 4) > 0)
			MTMacInfoEndianChange(pAd, ptr, TYPE_RMACINFO, rxv->RxByteCnt);

#endif /* RT_BIG_ENDIAN */

		for (idx = 0; idx < rxv->RxvCnt; idx++) {
			chip_parse_rxv_packet(pAd, RMAC_RX_PKT_TYPE_RX_TXRXV, rx_blk, ptr);
			ptr += 44;
		}
	}

	return TRUE;
}


INT32 txs_handler_v2(RTMP_ADAPTER *pAd, RX_BLK *rx_blk, VOID *rx_packet)
{
	RMAC_RXD_0_TXS *txs = (RMAC_RXD_0_TXS *)(rx_packet);
	UCHAR *ptr;
	INT idx, txs_entry_len = 20;
	BOOLEAN dump_all = FALSE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("TxS Report: Number=%d, ByteCnt=%d\n",
			  txs->TxSCnt, txs->TxSRxByteCnt));

	if (txs->TxSRxByteCnt == 0)
		return TRUE;

#if (ENABLE_RXD_LOG)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("TxS Report:TxSPktType =%d, Number=%d, ByteCnt=%d\n",
			  txs->TxSPktType, txs->TxSCnt, txs->TxSRxByteCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_0_TXS: 0x%x\n", *((UINT32 *)txs + 0)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_1_TXS: 0x%x\n", *((UINT32 *)txs + 1)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_2_TXS: 0x%x\n", *((UINT32 *)txs + 2)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_3_TXS: 0x%x\n", *((UINT32 *)txs + 3)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_4_TXS: 0x%x\n", *((UINT32 *)txs + 4)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_5_TXS: 0x%x\n", *((UINT32 *)txs + 5)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_6_TXS: 0x%x\n", *((UINT32 *)txs + 6)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_7_TXS: 0x%x\n", *((UINT32 *)txs + 7)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_8_TXS: 0x%x\n", *((UINT32 *)txs + 8)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_9_TXS: 0x%x\n", *((UINT32 *)txs + 9)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_10_TXS: 0x%x\n", *((UINT32 *)txs + 10)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_11_TXS: 0x%x\n", *((UINT32 *)txs + 11)));
#endif

	txs_entry_len = 28; /* sizeof(TXS_STRUC); // 28 bytes */


	if (txs->TxSRxByteCnt != (txs->TxSCnt * txs_entry_len + 4)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("ReceivedByteCnt not equal txs_entry required!\n"));
	} else {
		ptr = ((UCHAR *)(rx_packet)) + 4;

		for (idx = 0; idx < txs->TxSCnt; idx++) {
			TXS_STRUC *txs_entry = (TXS_STRUC *)ptr;
			TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
			INT32 stat;
#ifdef RT_BIG_ENDIAN
			TXS_D_1 *TxSD1 = &txs_entry->TxSD1;
			TXS_D_2 *TxSD2 = &txs_entry->TxSD2;
			TXS_D_3 *TxSD3 = &txs_entry->TxSD3;
#endif /* RT_BIG_ENDIAN */
#ifdef RT_BIG_ENDIAN
			*(((UINT32 *)TxSD0)) = SWAP32(*(((UINT32 *)TxSD0)));
			*(((UINT32 *)TxSD1)) = SWAP32(*(((UINT32 *)TxSD1)));
			*(((UINT32 *)TxSD2)) = SWAP32(*(((UINT32 *)TxSD2)));
			*(((UINT32 *)TxSD3)) = SWAP32(*(((UINT32 *)TxSD3)));
#endif /* RT_BIG_ENDIAN */
#ifdef RT_BIG_ENDIAN
			{
				TXS_D_5 *TxSD5 = &txs_entry->TxSD5;
				TXS_D_6 *TxSD6 = &txs_entry->TxSD6;
				*(((UINT32 *)TxSD5)) = SWAP32(*(((UINT32 *)TxSD5)));
				*(((UINT32 *)TxSD6)) = SWAP32(*(((UINT32 *)TxSD6)));
			}
#endif /* RT_BIG_ENDIAN */
			stat = ParseTxSPacket_v2(pAd, TxSD0->TxS_PId, TxSD0->TxSFmt, ptr);

			if (stat == -1)
				dump_all = TRUE;


			ptr += txs_entry_len;
		}

		if (dump_all == TRUE) {
			printk("%s(): Previous received TxS has error, dump raw data!\n", __func__);
			printk("\tTxS Report: Cnt=%d, ByteCnt=%d\n", txs->TxSCnt, txs->TxSRxByteCnt);
			hex_dump("Raw data", ((UCHAR *)(rx_packet)),
					 txs->TxSRxByteCnt > 400 ? 400 : txs->TxSRxByteCnt);
		}
	}

	return TRUE;
}

UINT32 rx_mcu_event_handler(RTMP_ADAPTER *ad, RX_BLK *rx_blk, VOID *rx_packet)
{
#if (ENABLE_RXD_LOG)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:\n", __func__));
#endif

	chip_rx_event_handler(ad, rx_packet);

	if (rx_blk)
		RX_BLK_SET_FLAG(rx_blk, fRX_CMD_RSP);

	return TRUE;
}




BOOLEAN tmr_handler(RTMP_ADAPTER *ad, RX_BLK *rx_blk, VOID *rx_packet)
{
	TMR_FRM_STRUC *tmr = (TMR_FRM_STRUC *)(rx_packet);
	/*Tmr pkt comes to Host directly, there are some minor calculation need to do.*/
	TmrReportParser(ad, tmr, FALSE, 0); /* TMRv1.0 TOAE calibration result need to leverage EXT_EVENT_ID_TMR_CAL */
	return TRUE;
}


#ifdef CUT_THROUGH
#define TX_FREE_NOTIFY 0
#define RX_REORDER_NOTIFY 1
#define TXRX_NOTE_GET_EVENT_TYPE(_ptr) ((UINT8)(((UINT32)(((UINT8 *)(_ptr)) + 3) & 0x1e000000) >> 25))
#define TXRX_NOTE_GET_TXDCNT(_ptr) (le2cpu16((UINT16)(((UINT32)(((UINT8 *)(_ptr)) + 2) & 0x01ff0000) >> 16)))
#define TXRX_NOTE_GET_LEN(_ptr) (le2cpu16((UINT16)(((UINT32)(((UINT8 *)(_ptr)) + 1) & 0x0000ffff))))
#define TXRX_NOTE_GET_DATA(_ptr) ((UINT8 *)(((UINT8 *)(_ptr)) + 8))
#define TXRX_NOTE_GET_TOKEN_LIST(_ptr) ((UINT8 *)(((UINT8 *)(_ptr)) + 8))

UINT32 tx_free_event_handler(RTMP_ADAPTER *ad, VOID *ptr)
{
	UINT32 dw0 = *(UINT32 *)ptr;
	UINT32 dw1 = le2cpu32(*((UINT32 *)ptr + 1));
	UINT8 *tokenList;
	UINT8 tokenCnt;
	UINT16 rxByteCnt;
	UINT8 report_type;
	UINT8 version, len_per_token;

	rxByteCnt = (dw0 & 0xffff);
	tokenCnt = ((dw0 & (0x7f << 16)) >> 16) & 0x7f;
	report_type = ((dw0 & (0x3f << 23)) >> 23) & 0x3f;
	version = ((dw1 & (0x7 << 16)) >> 16) & 0x7;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: DW0=0x%08x, rxByteCnt=%d,tokenCnt= %d, ReportType=%d\n",
			  __func__, dw0, rxByteCnt, tokenCnt, report_type));

	switch (report_type) {
	case TX_FREE_NOTIFY:
		len_per_token = (version == 0) ? 2 : 4;

		if ((tokenCnt * len_per_token + 8) != rxByteCnt) {
			printk("tokenCnt(%d) and rxByteCnt(%d) mismatch!\n", tokenCnt, rxByteCnt);
			hex_dump("TxFreeNotifyEventMisMatchFrame", ptr, rxByteCnt);
			return FALSE;
		}

		tokenList = TXRX_NOTE_GET_TOKEN_LIST(ptr);

		EventTxFreeNotifyHandler(ad, tokenList, tokenCnt, rxByteCnt - 8, len_per_token);
		break;

	case RX_REORDER_NOTIFY:
		if ((tokenCnt * 4 + 8) != rxByteCnt) {
			printk("tokenCnt(%d) and rxByteCnt(%d) mismatch!\n", tokenCnt, rxByteCnt);
			hex_dump("EventRxOrderNotifyNotifyHandler_RawData", ptr, rxByteCnt);
			return FALSE;
		}

		tokenList = TXRX_NOTE_GET_TOKEN_LIST(ptr);
		EventRxOrderNotifyNotifyHandler(ad, tokenList, tokenCnt, rxByteCnt - 8);
		break;

	default:
		printk("Invalid type(%d)!\n", report_type);
		break;
	}

	return TRUE;
}
#endif /* CUT_THROUGH */


UINT32 pkt_alloc_fail_handle(RTMP_ADAPTER *ad, PNDIS_PACKET rx_packet)
{
	UINT32 rx_pkt_type;

	rx_pkt_type = asic_get_packet_type(ad, rx_packet);

	switch (rx_pkt_type) {
#ifdef CUT_THROUGH

	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY:
		tx_free_event_handler(ad, rx_packet);
		break;
#endif

	default:
		break;
	}

	return 0;
}

UINT32 mt_rx_pkt_process(RTMP_ADAPTER *pAd, UINT8 hif_idx, RX_BLK *rx_blk, PNDIS_PACKET rx_packet)
{
	UINT32 rx_pkt_type;

	if (hif_idx == HIF_RX_IDX0)
		rx_pkt_type = asic_get_packet_type(pAd, GET_OS_PKT_DATAPTR(rx_packet));
	else
		rx_pkt_type = asic_get_packet_type(pAd, rx_packet);

	switch (rx_pkt_type) {
	case RMAC_RX_PKT_TYPE_RX_NORMAL:
	case RMAC_RX_PKT_TYPE_RX_DUP_RFB:
		rx_data_handler(pAd, rx_blk, rx_packet);
		break;

	case RMAC_RX_PKT_TYPE_RX_TXRXV:
		rxv_handler(pAd, rx_blk, rx_packet);
		free_rx_buf(pAd->hdev_ctrl, hif_idx);
		break;

	case RMAC_RX_PKT_TYPE_RX_TXS:
		chip_txs_handler(pAd, rx_blk, rx_packet);
		free_rx_buf(pAd->hdev_ctrl, hif_idx);
		break;

	case RMAC_RX_PKT_TYPE_RX_EVENT:
		rx_mcu_event_handler(pAd, rx_blk, rx_packet);
		free_rx_buf(pAd->hdev_ctrl, hif_idx);
		break;

	case RMAC_RX_PKT_TYPE_RX_TMR:
		tmr_handler(pAd, rx_blk, rx_packet);
		free_rx_buf(pAd->hdev_ctrl, hif_idx);
		break;
#ifdef CUT_THROUGH

	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY:
		tx_free_event_handler(pAd, rx_packet);
		free_rx_buf(pAd->hdev_ctrl, hif_idx);
		break;
#endif /* CUT_THROUGH */

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Invalid PktType:%d\n", __func__, rx_pkt_type));
		break;
	}

	return NDIS_STATUS_SUCCESS;
}

#ifdef MT7626_E2_SUPPORT
UINT32 mt_rx2_pkt_process(RTMP_ADAPTER *pAd, UINT8 hif_idx, RX_BLK *rx_blk, PNDIS_PACKET rx_packet)
{
	UINT32 rx_pkt_type;

	if (hif_idx == HIF_RX_IDX2)
		rx_pkt_type = asic_get_packet_type(pAd, GET_OS_PKT_DATAPTR(rx_packet));
	else
		return NDIS_STATUS_SUCCESS;
	switch (rx_pkt_type) {
	case RMAC_RX_PKT_TYPE_RX_NORMAL:
	case RMAC_RX_PKT_TYPE_RX_DUP_RFB:
		rx_data_handler(pAd, rx_blk, rx_packet);
		break;

	case RMAC_RX_PKT_TYPE_RX_TXRXV:
		rxv_handler(pAd, rx_blk, rx_packet);
		free_rx_buf(pAd->hdev_ctrl, hif_idx);
		break;

	case RMAC_RX_PKT_TYPE_RX_TXS:
		chip_txs_handler(pAd, rx_blk, rx_packet);
		free_rx_buf(pAd->hdev_ctrl, hif_idx);
		break;

	case RMAC_RX_PKT_TYPE_RX_EVENT:
		rx_mcu_event_handler(pAd, rx_blk, rx_packet);
		free_rx_buf(pAd->hdev_ctrl, hif_idx);
		break;

	case RMAC_RX_PKT_TYPE_RX_TMR:
		tmr_handler(pAd, rx_blk, rx_packet);
		free_rx_buf(pAd->hdev_ctrl, hif_idx);
		break;
#ifdef CUT_THROUGH

	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY:
		tx_free_event_handler(pAd, rx_packet);
		free_rx_buf(pAd->hdev_ctrl, hif_idx);
		break;
#endif /* CUT_THROUGH */

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Invalid PktType:%d\n", __func__, rx_pkt_type));
		break;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* MT7626_E2_SUPPORT */
#endif /* MT_MAC */

/*
	========================================================================

	Routine Description:
		API for MLME to transmit management frame to AP (BSS Mode)
	or station (IBSS Mode)

	Arguments:
		pAd Pointer to our adapter
		pData		Pointer to the outgoing 802.11 frame
		Length		Size of outgoing management frame

	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS MiniportMMRequest(RTMP_ADAPTER *pAd, UCHAR QueIdx, UCHAR *pData, UINT Length)
{
	PNDIS_PACKET pkt;
	NDIS_STATUS Status = NDIS_STATUS_FAILURE;
	BOOLEAN bUseDataQ = FALSE, FlgDataQForce = FALSE;
	HEADER_802_11 *pHead = (HEADER_802_11 *)pData;
	struct wifi_dev *wdev = NULL;
	struct wifi_dev_ops *ops = NULL;
	INT hw_len;
	UCHAR hw_hdr[40];
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	hw_len = cap->tx_hw_hdr_len;
	ASSERT((sizeof(hw_hdr) > hw_len));
	NdisZeroMemory(&hw_hdr, hw_len);

	ASSERT(Length <= MAX_MGMT_PKT_LEN);

	Status = RTMPAllocateNdisPacket(pAd, &pkt, (UCHAR *)&hw_hdr[0], hw_len, pData, Length);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("MiniportMMRequest (error:: can't allocate NDIS PACKET)\n"));
		return NDIS_STATUS_FAILURE;
	}

	wdev = wdev_search_by_address(pAd, pHead->Addr2);

	if (wdev) {
		ops = wdev->wdev_ops;
	} else {
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}


	if ((QueIdx & MGMT_USE_QUEUE_FLAG) == MGMT_USE_QUEUE_FLAG) {
		bUseDataQ = TRUE;
		QueIdx &= (~MGMT_USE_QUEUE_FLAG);
	}

	if (bUseDataQ)
		FlgDataQForce = TRUE;

#ifdef WIFI_DIAG
	DiagMiniportMMRequest(pAd, pData, Length);
#endif

	RTMP_SET_PACKET_TYPE(pkt, TX_MGMT);
	Status = send_mlme_pkt(pAd, pkt, wdev, QueIdx, bUseDataQ);

	return Status;
}

#ifdef CONFIG_AP_SUPPORT
/*
	========================================================================

	Routine Description:
		Copy frame from waiting queue into relative ring buffer and set
	appropriate ASIC register to kick hardware transmit function

	Arguments:
		pAd Pointer to our adapter
		pBuffer	Pointer to	memory of outgoing frame
		Length		Size of outgoing management frame
		FlgIsDeltsFrame 1: the frame is a DELTS frame

	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
void AP_QueuePsActionPacket(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN PNDIS_PACKET pPacket,
	IN BOOLEAN FlgIsDeltsFrame,
	IN BOOLEAN FlgIsLocked,
	IN UCHAR MgmtQid)
{
#ifdef UAPSD_SUPPORT
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
	PNDIS_PACKET DuplicatePkt = NULL;
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
#endif /* UAPSD_SUPPORT */
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];
	/* Note: for original mode of 4 AC are UAPSD, if station want to change
			the mode of a AC to legacy PS, we dont know where to put the
			response;
			1. send the response;
			2. but the station is in ps mode, so queue the response;
			3. we should queue the reponse to UAPSD queue because the station
				is not yet change its mode to legacy ps AC;
			4. so AP should change its mode to legacy ps AC only when the station
				sends a trigger frame and we send out the reponse;
			5. the mechanism is too complicate; */
#ifdef UAPSD_SUPPORT

	/*
		If the frame is action frame and the VO is UAPSD, we can not send the
		frame to VO queue, we need to send to legacy PS queue; or the frame
		maybe not got from QSTA.
	*/
	/*    if ((pMacEntry->bAPSDDeliverEnabledPerAC[MgmtQid]) &&*/
	/*		(FlgIsDeltsFrame == 0))*/
	if (pMacEntry->bAPSDDeliverEnabledPerAC[MgmtQid]) {
		/* queue the management frame to VO queue if VO is deliver-enabled */
		MTWF_LOG(DBG_CAT_PS, CATPS_UAPSD, DBG_LVL_TRACE, ("ps> mgmt to UAPSD queue %d ... (IsDelts: %d)\n",
				 MgmtQid, FlgIsDeltsFrame));
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY

		if (!pMacEntry->bAPSDAllAC) {
			/* duplicate one packet to legacy PS queue */
			RTMP_SET_PACKET_UAPSD(pPacket, 0, MgmtQid);
			DuplicatePkt = DuplicatePacket(wdev->if_dev, pPacket);
		} else
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		{
			RTMP_SET_PACKET_UAPSD(pPacket, 1, MgmtQid);
		}

		UAPSD_PacketEnqueue(pAd, pMacEntry, pPacket, MgmtQid, FALSE);

		if (pMacEntry->bAPSDAllAC) {
			/* mark corresponding TIM bit in outgoing BEACON frame*/
			WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->func_tb_idx, pMacEntry->Aid);
		} else {
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
			/* duplicate one packet to legacy PS queue */

			/*
				Sometimes AP will send DELTS frame to STA but STA will not
				send any trigger frame to get the DELTS frame.
				We must force to send it so put another one in legacy PS
				queue.
			*/
			if (DuplicatePkt != NULL) {
				pPacket = DuplicatePkt;
				goto Label_Legacy_PS;
			}

#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		}
	} else
#endif /* UAPSD_SUPPORT */
	{
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
Label_Legacy_PS:
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("ps> mgmt to legacy ps queue... (%d)\n", FlgIsDeltsFrame));

		if (tr_entry->ps_queue.Number >= MAX_PACKETS_IN_PS_QUEUE ||
			ge_enq_req(pAd, pPacket, MgmtQid, tr_entry, NULL) == FALSE) {
			MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s(%d): WLAN_TX_DROP, pPacket=%p, QueIdx=%d, ps_queue_num=%d, wcid=%d\n",
					  __func__, __LINE__, pPacket, MgmtQid, tr_entry->ps_queue.Number, tr_entry->wcid));
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_RESOURCES);
			return;
		} else {
			/* mark corresponding TIM bit in outgoing BEACON frame*/
			WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->func_tb_idx, pMacEntry->Aid);
		}
	}
}
#endif /* CONFIG_AP_SUPPORT */

#ifdef REDUCE_TX_OVERHEAD
inline UINT16 tx_pkt_classification(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, TX_BLK *tx_blk)
#else
UINT16 tx_pkt_classification(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, TX_BLK *tx_blk)
#endif
{
	UCHAR wcid;
	UCHAR TxFrameType = TX_LEGACY_FRAME;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	UCHAR up = RTMP_GET_PACKET_UP(pkt);

#ifdef CONFIG_ATE
	if (RTMP_GET_PACKET_TXTYPE(pkt) & TX_ATE_FRAME)
		return TX_ATE_FRAME;
#endif /* CONFIG_ATE */

	if (RTMP_GET_PACKET_TXTYPE(pkt) == TX_MCAST_FRAME)
		TxFrameType = TX_MCAST_FRAME;

	wcid = RTMP_GET_PACKET_WCID(pkt);
	tr_entry = &pAd->MacTab.tr_entry[wcid];

	if (VALID_UCAST_ENTRY_WCID(pAd, wcid))
		pMacEntry = &pAd->MacTab.Content[wcid];
	else {
		if (wcid > 0)
			pMacEntry = &pAd->MacTab.Content[wcid];
		else
			pMacEntry = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE];
	}

	tx_blk->pMacEntry = pMacEntry;

	if (RTMP_GET_PACKET_MGMT_PKT(pkt)) {
		if (RTMP_GET_PACKET_MGMT_PKT_DATA_QUE(pkt))
			return TX_MLME_DATAQ_FRAME;
		else
			return TX_MLME_MGMTQ_FRAME;
	}

	if ((RTMP_GET_PACKET_FRAGMENTS(pkt) > 1)
		&& ((pMacEntry->TXBAbitmap & (1 << up)) == 0))
		return TX_FRAG_FRAME;

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD) &&
		IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if (TxFrameType != TX_FRAG_FRAME)
			TX_BLK_SET_FLAG(tx_blk, fTX_MCU_OFFLOAD);
	}

	if (!TX_BLK_TEST_FLAG(tx_blk, fTX_MCU_OFFLOAD)) {
		if (pMacEntry->tx_amsdu_bitmap & (1 << up)) {
#ifdef HW_TX_AMSDU_SUPPORT
			if (IS_ASIC_CAP(pAd, fASIC_CAP_HW_TX_AMSDU))
				TX_BLK_SET_FLAG(tx_blk, fTX_HW_AMSDU);
			else
#endif /* HW_TX_AMSDU_SUPPORT */
			TxFrameType = TX_AMSDU_FRAME;
		}
	}

	return TxFrameType;
}

inline BOOLEAN fill_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	BOOLEAN ret;
	struct wifi_dev_ops *ops = wdev->wdev_ops;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_MCU_OFFLOAD))
		ret = ops->fill_offload_tx_blk(pAd, wdev, pTxBlk);
	else
		ret = ops->fill_non_offload_tx_blk(pAd, wdev, pTxBlk);

	return ret;
}

#ifdef VLAN_SUPPORT

static inline VOID Sniff2BytesFromSrcBuffer(PNDIS_BUFFER buf, UCHAR offset, UCHAR *p0, UCHAR *p1)
{
	UCHAR *ptr = (UCHAR *)(buf + offset);
	*p0 = *ptr;
	*p1 = *(ptr + 1);
}

static inline VOID RtmpOsRemoveVLANTag(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt)
{
	UCHAR *pSrcBuf;
	UINT16 VLAN_LEN = 4;
	UCHAR extra_field_offset = 2 * ETH_ALEN;

	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
	ASSERT(pSrcBuf);
	memmove(GET_OS_PKT_DATAPTR(pkt) + VLAN_LEN, GET_OS_PKT_DATAPTR(pkt), extra_field_offset);
	skb_pull_rcsum(RTPKT_TO_OSPKT(pkt), 4);
	skb_reset_mac_header(RTPKT_TO_OSPKT(pkt));
	skb_reset_network_header(RTPKT_TO_OSPKT(pkt));
	skb_reset_transport_header(RTPKT_TO_OSPKT(pkt));
	skb_reset_mac_len(RTPKT_TO_OSPKT(pkt));
}

BOOLEAN check_copy_pkt_needed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	UCHAR *pSrcBuf;
	UINT16 TypeLen;
	BOOLEAN ret = FALSE;
	USHORT vlan_id, vlan_pcp;

	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
	ASSERT(pSrcBuf);
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];

	/*skip the Ethernet Header*/
	pSrcBuf += LENGTH_802_3;

	vlan_id = *(USHORT *)pSrcBuf;

	vlan_id = cpu2be16(vlan_id);
	vlan_pcp = (vlan_id & (~MASK_CLEAR_TCI_PCP)) >> 13;
	vlan_id = vlan_id & MASK_TCI_VID; /* 12 bit */

	if (TypeLen == ETH_TYPE_VLAN) {
		if (wdev->bVLAN_Tag == FALSE) {
			ret = TRUE;
		} else if (wdev->VLAN_VID != 0) {
			if (vlan_id != wdev->VLAN_VID) {
				switch (wdev->VLAN_Policy[TX_VLAN]) {
				case VLAN_TX_REPLACE_VID:
				case VLAN_TX_REPLACE_ALL:
					ret = TRUE;
				default:
					ret = FALSE;
				}
			} else if (vlan_pcp != wdev->VLAN_Priority) {
				ret = TRUE;
			}
		}
	} else if (wdev->bVLAN_Tag && wdev->VLAN_Policy[TX_VLAN] != VLAN_TX_ALLOW) {
		ret = TRUE;
	}

	return ret;
}

PNDIS_PACKET RtmpVlanPktCopy(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	struct sk_buff *skb = NULL;
	PNDIS_PACKET pkt_copy = NULL;
	struct net_device *net_dev = wdev->if_dev;

	skb = skb_copy(RTPKT_TO_OSPKT(pkt), GFP_ATOMIC);

	if (skb) {
		skb->dev = net_dev;
		pkt_copy = OSPKT_TO_RTPKT(skb);
		RTMP_SET_PACKET_WDEV(pkt_copy, wdev->wdev_idx);
		RTMP_SET_PACKET_WCID(pkt_copy, MAX_LEN_OF_MAC_TABLE);
	}

	return pkt_copy;
}

static int ap_fp_tx_pkt_vlan_tag_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET *pPkt)
{
	UCHAR *pSrcBuf, *pkt_va;
	UINT16 TypeLen;
	UCHAR Byte0, Byte1;
	PACKET_INFO pkt_info;
	UINT pkt_len;
	PNDIS_PACKET pkt = NULL, pkt_copy = NULL;
	BOOLEAN need_copy;
	struct sk_buff *skb = (struct sk_buff *)(*pPkt);

	pkt = *pPkt;
	RTMP_QueryPacketInfo(pkt, &pkt_info, &pkt_va, &pkt_len);
	if ((!pkt_va) || (pkt_len <= 14))
		return FALSE;

	if (skb->cloned) {
		need_copy = check_copy_pkt_needed(pAd, wdev, pkt);
		if (need_copy) {
			pkt_copy = RtmpVlanPktCopy(pAd, wdev, pkt);
			if (pkt_copy == NULL) {
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s():copy packet fail!!\n", __func__));
				return FALSE;
			}
			*pPkt = pkt_copy;
			pkt = *pPkt;
			RTMP_QueryPacketInfo(pkt, &pkt_info, &pkt_va, &pkt_len);
			if ((!pkt_va) || (pkt_len <= 14))
				return FALSE;
		}
	}

	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
	ASSERT(pSrcBuf);
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
	if (TypeLen != ETH_TYPE_VLAN) {
		RTMP_SET_PACKET_VLAN(pkt, FALSE);
		RTMP_SET_PACKET_PROTOCOL(pkt, TypeLen);
	}

	/*insert 802.1Q tag if required*/
	if (wdev->bVLAN_Tag && TypeLen != ETH_TYPE_VLAN) {
		UINT16 tci = (wdev->VLAN_Priority<<(CFI_LEN + VID_LEN)) | wdev->VLAN_VID; /*CFI = 0*/

		pkt = RtmpOsVLANInsertTag(pkt, tci);
		pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
		ASSERT(pSrcBuf);
		TypeLen = ETH_TYPE_VLAN;
	}

	/*skip the Ethernet Header*/
	pSrcBuf += LENGTH_802_3;

	if (TypeLen == ETH_TYPE_VLAN) {
#ifdef CONFIG_AP_SUPPORT
		/*
			802.3 VLAN packets format:

			DstMAC(6B) + SrcMAC(6B)
			+ 802.1Q Tag Type (2B = 0x8100) + Tag Control Information (2-bytes)
			+ Length/Type(2B)
			+ data payload (42-1500 bytes)
			+ FCS(4B)

			VLAN tag: 3-bit UP + 1-bit CFI + 12-bit VLAN ID
		*/

		/* If it is not my vlan pkt, no matter unicast or multicast, deal with it according to the policy*/
		if (wdev->VLAN_VID != 0) {

			USHORT vlan_id = *(USHORT *)pSrcBuf;

			vlan_id = cpu2be16(vlan_id);
			vlan_id = vlan_id & MASK_TCI_VID; /* 12 bit */

			if (vlan_id != wdev->VLAN_VID) {
				switch (wdev->VLAN_Policy[TX_VLAN]) {
				case VLAN_TX_DROP:
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s():Drop the packet\n", __func__));
					return FALSE;
				case VLAN_TX_ALLOW:
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s():Allow the packet\n", __func__));
					break;
				case VLAN_TX_REPLACE_VID:
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s():Replace the packet VLAN ID\n", __func__));
					*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_VID);
					*(USHORT *)pSrcBuf |= be2cpu16(wdev->VLAN_VID);
					break;
				case VLAN_TX_REPLACE_ALL:
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s():Replace the packet VLAN Tag\n", __func__));
					*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_PCP);
					*(USHORT *)pSrcBuf |= be2cpu16((wdev->VLAN_Priority)<<(CFI_LEN + VID_LEN));
					*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_VID);
					*(USHORT *)pSrcBuf |= be2cpu16(wdev->VLAN_VID);
					break;
				default:
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s(): Unexpected checking policy\n", __func__));
					return FALSE;
				}
			} else {
				/* align PCP*/
				*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_PCP);
				*(USHORT *)pSrcBuf |= be2cpu16((wdev->VLAN_Priority)<<(CFI_LEN + VID_LEN));
			}
		}

		if (wdev->bVLAN_Tag == FALSE) {
			RtmpOsRemoveVLANTag(pAd, pkt);
			RTMP_SET_PACKET_VLAN(pkt, FALSE);
			pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
			ASSERT(pSrcBuf);
			TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
			RTMP_SET_PACKET_PROTOCOL(pkt, TypeLen);
		} else {
			USHORT vlan_pcp = *(USHORT *)pSrcBuf;
			vlan_pcp = cpu2be16(vlan_pcp);
			vlan_pcp = (((vlan_pcp) & (~MASK_CLEAR_TCI_PCP)) >> 13); /* 3 bit */
			RTMP_SET_VLAN_PCP(pkt, vlan_pcp);
			RTMP_SET_PACKET_VLAN(pkt, TRUE);
			Sniff2BytesFromSrcBuffer((PNDIS_BUFFER)pSrcBuf, 2, &Byte0, &Byte1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
			RTMP_SET_PACKET_PROTOCOL(pkt, TypeLen);
		}

#endif /* CONFIG_AP_SUPPORT */
	}

	return TRUE;
}

#endif /*VLAN_SUPPORT*/

INT send_data_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	INT ret = NDIS_STATUS_SUCCESS;
	BOOLEAN allowed = TRUE;
	struct wifi_dev_ops *ops = wdev->wdev_ops;


#ifdef VLAN_SUPPORT
	allowed = ap_fp_tx_pkt_vlan_tag_handle(pAd, wdev, &pkt);
	if (!allowed) {
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
		ret = NDIS_STATUS_FAILURE;
		return ret;
	}
#endif /*VLAN_SUPPORT*/

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
		allowed = ops->fp_tx_pkt_allowed(pAd, wdev, pkt);

		if (allowed) {
			ret = ops->fp_send_data_pkt(pAd, wdev, pkt);
		}

	} else {
		allowed = ops->tx_pkt_allowed(pAd, wdev, pkt);

		if (allowed) {
			ret = ops->send_data_pkt(pAd, wdev, pkt);
		}
	}

	if (!allowed) {
#ifdef IXIA_SUPPORT
		if (IS_EXPECTED_LENGTH(pAd, RTPKT_TO_OSPKT(pkt)->len))
			pAd->tr_ctl.tp_dbg.TxDropPacket[INVALID_TR_WCID]++;
#endif /*IXIA_SUPPORT*/
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
		ret = NDIS_STATUS_FAILURE;
	}
	return ret;
}

INT send_mlme_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, struct wifi_dev *wdev, UCHAR q_idx, BOOLEAN is_data_queue)
{
	INT ret;
	struct wifi_dev_ops *ops = wdev->wdev_ops;

	ret = ops->send_mlme_pkt(pAd, pkt, wdev, q_idx, is_data_queue);
	return ret;
}

/*
	========================================================================

	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.

	Arguments:
		pAd	Pointer to our adapter
		Rate			Transmit rate
		Size			Frame size in units of byte

	Return Value:
		Duration number in units of usec

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
USHORT RTMPCalcDuration(RTMP_ADAPTER *pAd, UCHAR Rate, ULONG Size)
{
	ULONG	Duration = 0;

	if (Rate < RATE_FIRST_OFDM_RATE) { /* CCK*/
		if ((Rate > RATE_1) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
			Duration = 96;	/* 72+24 preamble+plcp*/
		else
			Duration = 192; /* 144+48 preamble+plcp*/

		Duration += (USHORT)((Size << 4) / RateIdTo500Kbps[Rate]);

		if ((Size << 4) % RateIdTo500Kbps[Rate])
			Duration++;
	} else if (Rate <= RATE_LAST_OFDM_RATE) { /* OFDM rates*/
		Duration = 20 + 6;		/* 16+4 preamble+plcp + Signal Extension*/
		Duration += 4 * (USHORT)((11 + Size * 4) / RateIdTo500Kbps[Rate]);

		if ((11 + Size * 4) % RateIdTo500Kbps[Rate])
			Duration += 4;
	} else {	/*mimo rate*/
		Duration = 20 + 6;		/* 16+4 preamble+plcp + Signal Extension*/
	}

	return (USHORT)Duration;
}


/*
	NOTE: we do have an assumption here, that Byte0 and Byte1
		always reasid at the same scatter gather buffer
 */
static inline VOID Sniff2BytesFromNdisBuffer(
	IN PNDIS_BUFFER buf,
	IN UCHAR offset,
	OUT UCHAR *p0,
	OUT UCHAR *p1)
{
	UCHAR *ptr = (UCHAR *)(buf + offset);
	*p0 = *ptr;
	*p1 = *(ptr + 1);
}

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
BOOLEAN is_gratuitous_arp(UCHAR *pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
	UCHAR *SenderIP;
	UCHAR *TargetIP;

	NdisMoveMemory(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_ARP) {
		/*
		 * Check if Gratuitous ARP, Sender IP equal Target IP
		 */
		SenderIP = Pos + 14;
		TargetIP = Pos + 24;

		if (NdisCmpMemory(SenderIP, TargetIP, 4) == 0) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("The Packet is GratuitousARP\n"));
			return TRUE;
		}
	}

	return FALSE;
}

BOOLEAN is_dad_packet(RTMP_ADAPTER *pAd, UCHAR *pData)
{
	UCHAR *pSenderIP = pData + 16;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR *pSourceMac = pData + 10;
#endif /* MAC_REPEATER_SUPPORT */
	UCHAR *pDestMac = pData + 20;
	UCHAR ZERO_IP_ADDR[4] = {0x00, 0x00, 0x00, 0x00};
	UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	/*
	* Check if DAD packet
	*/
	if (
#ifdef MAC_REPEATER_SUPPORT
		(RTMPLookupRepeaterCliEntry(pAd, FALSE, pSourceMac, TRUE) != NULL) &&
#endif /* MAC_REPEATER_SUPPORT */
		((MAC_ADDR_EQUAL(pDestMac, BROADCAST_ADDR) == TRUE) ||
		 (MAC_ADDR_EQUAL(pDestMac, ZERO_MAC_ADDR) == TRUE)) &&
		(RTMPEqualMemory(pSenderIP, ZERO_IP_ADDR, 4) == TRUE)) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("DAD found, and do not send this packet\n"));
		return TRUE;
	}

	return FALSE;
}

BOOLEAN is_looping_packet(RTMP_ADAPTER *pAd, NDIS_PACKET *pPacket)
{
	UCHAR *pSrcBuf;
	UINT16 TypeLen;
	UCHAR Byte0, Byte1;

	if (!pAd)
		return FALSE;

	pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
	ASSERT(pSrcBuf);
	/* get Ethernet protocol field and skip the Ethernet Header */
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
	pSrcBuf += LENGTH_802_3;

	if (TypeLen <= 1500) {
		/*
			802.3, 802.3 LLC:
				DestMAC(6) + SrcMAC(6) + Lenght(2) +
				DSAP(1) + SSAP(1) + Control(1) +
			if the DSAP = 0xAA, SSAP=0xAA, Contorl = 0x03, it has a 5-bytes SNAP header.
				=> + SNAP (5, OriginationID(3) + etherType(2))
			else
				=> It just has 3-byte LLC header, maybe a legacy ether type frame. we didn't handle it
		*/
		if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03) {
			Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 6, &Byte0, &Byte1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
			pSrcBuf += LENGTH_802_1_H; /* Skip this LLC/SNAP header*/
		} else {
			return FALSE;
		}
	}

	if (TypeLen == ETH_TYPE_ARP) {
		/* AP's tx shall check DAD.*/
		if (is_dad_packet(pAd, pSrcBuf - 2) || is_gratuitous_arp(pSrcBuf - 2))
			return TRUE;
	}

	return FALSE;
}

#endif /* CONFIG_WIFI_PKT_FWD */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

void set_wf_fwd_cb(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket, struct wifi_dev *wdev)
{
	PNDIS_PACKET pOsRxPkt = pPacket;

	RTMP_CLEAN_PACKET_BAND(pOsRxPkt);
	RTMP_CLEAN_PACKET_RECV_FROM(pOsRxPkt);

	if (wdev->channel >= H_CHANNEL_BIGGER_THAN) {
		RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_5G_H);

		if ((wdev->wdev_type == WDEV_TYPE_AP) || (wdev->wdev_type == WDEV_TYPE_WDS))
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_H_AP);
		else if (IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA))
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_H_CLIENT);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("No Setting RTMP_SET_PACKET_RECV_FROM(5G_H) wdev->channel(%d) wdev->wdev_type(%d)\n", wdev->channel, wdev->wdev_type));
		}
	} else if (wdev->channel > 14) {
		RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_5G);

		if ((wdev->wdev_type == WDEV_TYPE_AP) || (wdev->wdev_type == WDEV_TYPE_WDS))
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_AP);
		else if (IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA))
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_CLIENT);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("No Setting RTMP_SET_PACKET_RECV_FROM(5G) wdev->channel(%d) wdev->wdev_type(%d)\n", wdev->channel, wdev->wdev_type));
		}
	} else {
		RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_2G);

		if ((wdev->wdev_type == WDEV_TYPE_AP) || (wdev->wdev_type == WDEV_TYPE_WDS))
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_2G_AP);
		else if (IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA))
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_2G_CLIENT);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("No Setting RTMP_SET_PACKET_RECV_FROM(2G) wdev->channel(%d) wdev->wdev_type(%d)\n", wdev->channel, wdev->wdev_type));
		}
	}

	/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("set_wf_fwd_cb: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x=> BandFrom:0x%x, RecvdFrom: 0x%x\n", */
	/* wdev->wdev_idx,wdev->wdev_type,wdev->func_idx, */
	/* RTMP_GET_PACKET_BAND(pOsRxPkt),RTMP_GET_PACKET_RECV_FROM(pOsRxPkt))); */

}
#endif


VOID CheckQosMapUP(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pEntry, UCHAR DSCP, PUCHAR pUserPriority)
{
#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_HOTSPOT_R2)
	UCHAR i = 0, find_up = 0, dscpL = 0, dscpH = 0;
	BSS_STRUCT *pMbss = NULL;
	STA_TR_ENTRY *tr_entry = NULL;

	if (pEntry == NULL || pEntry->wdev == NULL)
		return;
	else {
		/* pAd = (RTMP_ADAPTER *)pEntry->pAd; */
		tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	}

	if (IS_ENTRY_CLIENT(tr_entry))
		pMbss = (BSS_STRUCT *)pEntry->wdev->func_dev;
	else
		return;

	if (pEntry->QosMapSupport && pMbss->HotSpotCtrl.QosMapEnable && pMbss->HotSpotCtrl.HotSpotEnable) {
		for (i = 0; i < (pEntry->DscpExceptionCount / 2); i++) {
			if ((pEntry->DscpException[i] & 0xff) == DSCP) {
				*pUserPriority = (pEntry->DscpException[i] >> 8) & 0xff;
				find_up = 1;
				break;
			}
		}

		if (!find_up) {
			for (i = 0; i < 8; i++) {
				dscpL = pEntry->DscpRange[i] & 0xff;
				dscpH = (pEntry->DscpRange[i] >> 8) & 0xff;

				if ((dscpH >= DSCP) && (dscpL <= DSCP)) {
					*pUserPriority = i;
					break;
				}
			}
		}
	}

#endif /* defined(CONFIG_AP_SUPPORT) && defined(CONFIG_HOTSPOT_R2) */
}
/*For QOS MAP Support Cert Test in Passpoint R2*/
#ifdef CONFIG_HOTSPOT_R2
static BOOLEAN is_hotspot_disabled_for_wdev(IN RTMP_ADAPTER * pAd, IN struct wifi_dev *wdev)
{
	BSS_STRUCT *pMbss;

	/*For Non-AP devices hotspot is disabled by default*/
	if (wdev->wdev_type != WDEV_TYPE_AP)
		return TRUE;

	ASSERT(wdev->func_idx < pAd->ApCfg.BssidNum);
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];

	if (pMbss->HotSpotCtrl.HotSpotEnable)
		return false;
	else
		return true;
}
#else
static inline BOOLEAN is_hotspot_disabled_for_wdev(IN RTMP_ADAPTER * pAd, IN struct wifi_dev *wdev)
{
	return true;
}
#endif

extern UINT8 dmac_ac_queue_to_up[4];
/*
	Check the Ethernet Frame type, and set RTMP_SET_PACKET_SPECIFIC flags
	Here we set the PACKET_SPECIFIC flags(LLC, VLAN, DHCP/ARP, EAPOL).
*/
BOOLEAN RTMPCheckEtherType(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET	pPacket,
	IN STA_TR_ENTRY *tr_entry,
	IN struct wifi_dev *wdev)
{
	UINT16 TypeLen;
	UCHAR Byte0, Byte1, *pSrcBuf, up = 0;
	UCHAR q_idx = QID_AC_BE;
	UCHAR final_user_prio = 0;

#ifdef RT_CFG80211_SUPPORT
	BOOLEAN bClearFrame;
#endif
	MAC_TABLE_ENTRY *pMacEntry = &pAd->MacTab.Content[tr_entry->wcid];

	pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
	ASSERT(pSrcBuf);
	RTMP_SET_PACKET_SPECIFIC(pPacket, 0);
	/* get Ethernet protocol field */
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];

#ifdef VLAN_SUPPORT
	/*insert 802.1Q tag if required*/
	if (wdev->bVLAN_Tag && TypeLen != ETH_TYPE_VLAN && wdev->VLAN_Policy[TX_VLAN] != VLAN_TX_ALLOW) {
		UINT16 tci = (wdev->VLAN_Priority<<(CFI_LEN + VID_LEN)) | wdev->VLAN_VID; /*CFI = 0*/

		pPacket = RtmpOsVLANInsertTag(pPacket, tci);
		pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
		ASSERT(pSrcBuf);

		TypeLen = ETH_TYPE_VLAN;
	}
#endif /*VLAN_SUPPORT*/

	/*skip the Ethernet Header*/
	pSrcBuf += LENGTH_802_3;

	if (TypeLen <= 1500) {
		/*
			802.3, 802.3 LLC:
				DestMAC(6) + SrcMAC(6) + Lenght(2) +
				DSAP(1) + SSAP(1) + Control(1) +
			if the DSAP = 0xAA, SSAP=0xAA, Contorl = 0x03, it has a 5-bytes SNAP header.
				=> + SNAP (5, OriginationID(3) + etherType(2))
			else
				=> It just has 3-byte LLC header, maybe a legacy ether type frame. we didn't handle it
		*/
		if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03) {
			Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 6, &Byte0, &Byte1);
			RTMP_SET_PACKET_LLCSNAP(pPacket, 1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
			pSrcBuf += LENGTH_802_1_H; /* Skip this LLC/SNAP header*/
		} else
			return FALSE;
	}

	/* If it's a VLAN packet, get the real Type/Length field.*/
	if (TypeLen == ETH_TYPE_VLAN) {
#ifdef CONFIG_AP_SUPPORT
		/*
			802.3 VLAN packets format:

			DstMAC(6B) + SrcMAC(6B)
			+ 802.1Q Tag Type (2B = 0x8100) + Tag Control Information (2-bytes)
			+ Length/Type(2B)
			+ data payload (42-1500 bytes)
			+ FCS(4B)

			VLAN tag: 3-bit UP + 1-bit CFI + 12-bit VLAN ID
		*/

		/* If it is not my vlan pkt, no matter unicast or multicast, deal with it according to the policy*/
		if (wdev->VLAN_VID != 0) {
			USHORT vlan_id = *(USHORT *)pSrcBuf;

			vlan_id = cpu2be16(vlan_id);
			vlan_id = vlan_id & MASK_TCI_VID; /* 12 bit */

			if (vlan_id != wdev->VLAN_VID) {
#ifdef VLAN_SUPPORT
				switch (wdev->VLAN_Policy[TX_VLAN]) {
				case VLAN_TX_DROP:
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s():Drop the packet\n", __func__));
					return FALSE;
				case VLAN_TX_ALLOW:
				case VLAN_TX_KEEP_TAG:
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s():Allow the packet\n", __func__));
					break;
				case VLAN_TX_REPLACE_VID:
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s():Replace the packet VLAN ID\n", __func__));
					*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_VID);
					*(USHORT *)pSrcBuf |= be2cpu16(wdev->VLAN_VID);
					break;
				case VLAN_TX_REPLACE_ALL:
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s():Replace the packet VLAN Tag\n", __func__));
					*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_PCP);
					*(USHORT *)pSrcBuf |= be2cpu16((wdev->VLAN_Priority)<<(CFI_LEN + VID_LEN));
					*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_VID);
					*(USHORT *)pSrcBuf |= be2cpu16(wdev->VLAN_VID);
					break;
				default:
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s(): Unexpected checking policy\n", __func__));
					return FALSE;
				}
#else
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():failed for VLAN_ID(vlan_id=%d, VLAN_VID=%d)\n",
						 __func__, vlan_id, wdev->VLAN_VID));
				return FALSE;
#endif /*VLAN_SUPPORT*/
			}
#ifdef VLAN_SUPPORT
			else {
				/* align PCP*/
				*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_PCP);
				*(USHORT *)pSrcBuf |= be2cpu16((wdev->VLAN_Priority)<<(CFI_LEN + VID_LEN));
			}
#endif /*VLAN_SUPPORT*/
		}

#endif /* CONFIG_AP_SUPPORT */
		RTMP_SET_PACKET_VLAN(pPacket, 1);
		Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 2, &Byte0, &Byte1);
		TypeLen = (USHORT)((Byte0 << 8) + Byte1);
#ifdef AUTOMATION
		if ((pAd->fpga_ctl.txrx_dbg_type == 1) ||
			(pAd->fpga_ctl.txrx_dbg_type == 5)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: vlan, Eth type=0x%x\n",
					 __func__, TypeLen));
		}
#endif /* AUTOMATION */
		/* only use VLAN tag as UserPriority setting */
		up = (*pSrcBuf & 0xe0) >> 5;
		CheckQosMapUP(pAd, pMacEntry, (*pSrcBuf & 0xfc) >> 2, &up);
		pSrcBuf += LENGTH_802_1Q; /* Skip the VLAN Header.*/
	} else if (TypeLen == ETH_TYPE_IPv4) {
		/*
			0       4       8          14  15                      31(Bits)
			+---+----+-----+----+---------------+
			|Ver |  IHL |DSCP |ECN |    TotalLen           |
			+---+----+-----+----+---------------+
			Ver    - 4bit Internet Protocol version number.
			IHL    - 4bit Internet Header Length
			DSCP - 6bit Differentiated Services Code Point(TOS)
			ECN   - 2bit Explicit Congestion Notification
			TotalLen - 16bit IP entire packet length(include IP hdr)
		*/
		up = (*(pSrcBuf + 1) & 0xe0) >> 5;
		CheckQosMapUP(pAd, pMacEntry, (*(pSrcBuf + 1) & 0xfc) >> 2, &up);
	} else if (TypeLen == ETH_TYPE_IPv6) {
		/*
			0       4       8        12     16                      31(Bits)
			+---+----+----+----+---------------+
			|Ver | TrafficClas |  Flow Label                   |
			+---+----+----+--------------------+
			Ver           - 4bit Internet Protocol version number.
			TrafficClas - 8bit traffic class field, the 6 most-significant bits used for DSCP
		*/
		up = ((*pSrcBuf) & 0x0e) >> 1;
		CheckQosMapUP(pAd, pMacEntry, ((*pSrcBuf & 0x0f) << 2) | ((*(pSrcBuf + 1)) & 0xc0) >> 6, &up);
	}

	switch (TypeLen) {
	case ETH_TYPE_IPv4: {
		UINT32 pktLen = GET_OS_PKT_LEN(pPacket);
#ifdef CSO_TEST_SUPPORT
		if (CsCtrl & BIT0) {
#ifdef AUTOMATION
/* for MSP check always 0xFF for Rx log easily check PASS / FAIL */
			*(pSrcBuf + 10) = 0xFF;
			*(pSrcBuf + 11) = 0xFF;
#else /* AUTOMATION */
			*(pSrcBuf + 10) = ~(*(pSrcBuf + 10));
			*(pSrcBuf + 11) = ~(*(pSrcBuf + 11));
#endif /* !AUTOMATION */
		}
#endif

		ASSERT((pktLen > (ETH_HDR_LEN + IP_HDR_LEN)));	/* 14 for ethernet header, 20 for IP header*/
		RTMP_SET_PACKET_IPV4(pPacket, 1);

		switch (*(pSrcBuf + 9)) {
		case IP_PROTO_UDP: {
			UINT16 srcPort, dstPort;

#ifdef CSO_TEST_SUPPORT
			if (CsCtrl & BIT2) {
#ifdef AUTOMATION
/* for MSP check always 0xFF for Rx log easily check PASS / FAIL */
				*(pSrcBuf + 26) = 0xFF;
				*(pSrcBuf + 27) = 0xFF;
#else /* AUTOMATION */
				*(pSrcBuf + 26) = ~(*(pSrcBuf + 26));
				*(pSrcBuf + 27) = ~(*(pSrcBuf + 27));
#endif /* !AUTOMATION */
			}
			if (CsCtrl & BIT3)
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Ipv4 udp pkt\n"));
#endif
			pSrcBuf += IP_HDR_LEN;
			srcPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf)));
			dstPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf + 2)));

			if ((srcPort == 0x44 && dstPort == 0x43) || (srcPort == 0x43 && dstPort == 0x44)) {
				/*It's a BOOTP/DHCP packet*/
				RTMP_SET_PACKET_DHCP(pPacket, 1);
				RTMP_SET_PACKET_HIGH_PRIO(pPacket, 1);
				RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
			}

#ifdef CONFIG_AP_SUPPORT
#ifdef CONFIG_DOT11V_WNM
			WNMIPv4ProxyARPCheck(pAd, pPacket, srcPort, dstPort, pSrcBuf, 0);
#endif
#ifdef CONFIG_HOTSPOT
			{
				USHORT Wcid = RTMP_GET_PACKET_WCID(pPacket);

				if (!HSIPv4Check(pAd, &Wcid, pPacket, pSrcBuf, srcPort, dstPort))
					return FALSE;
			}
#endif
#endif
			}
			break;
		case IP_PROTOCOL_ICMP:
#ifdef AUTOMATION
		if (pAd->auto_dvt && pAd->auto_dvt->apps.block_packet & ICMP_PACKET) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Automation Block ICMP Packet\n"));
			return FALSE;
		}
#endif /* !AUTOMATION */
			pSrcBuf += IP_HDR_LEN;
			if ((*pSrcBuf == ICMP_TYPE_ECHO_RSP) ||
				(*pSrcBuf == ICMP_TYPE_ECHO_REQ)) {
				RTMP_SET_PACKET_PING(pPacket, 1);
				RTMP_SET_PACKET_HIGH_PRIO(pPacket, 1);
				RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
			}

			break;
#ifdef CSO_TEST_SUPPORT
		case IP_PROTOCOL_TCP:
			if (CsCtrl & BIT1) {
#ifdef AUTOMATION
/* for MSP check always 0xFF for Rx log easily check PASS / FAIL */
				*(pSrcBuf + 36) = 0xFF;
				*(pSrcBuf + 37) = 0xFF;
#else /* AUTOMATION */
				*(pSrcBuf + 36) = ~(*(pSrcBuf + 36));
				*(pSrcBuf + 37) = ~(*(pSrcBuf + 37));
#endif /* !AUTOMATION */
			}
			if (CsCtrl & BIT3)
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Ipv4 tcp pkt\n"));
			break;
#endif
		}
	}
	break;

	case ETH_TYPE_ARP: {
#ifdef CONFIG_AP_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_AP) {
#ifdef CONFIG_DOT11V_WNM
			BSS_STRUCT *pMbss = (BSS_STRUCT *)wdev->func_dev;

			if (pMbss->WNMCtrl.ProxyARPEnable) {
				/* Check if IPv4 Proxy ARP Candidate from DS */
				if (IsIPv4ProxyARPCandidate(pAd, pSrcBuf - 2)) {
					BOOLEAN FoundProxyARPEntry;

					FoundProxyARPEntry = IPv4ProxyARP(pAd, pMbss, pSrcBuf - 2, TRUE, 0);

					if (!FoundProxyARPEntry)
						MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Can not find proxy entry\n"));

					return FALSE;
				}
			}

#endif

#ifdef CONFIG_HOTSPOT

			if (pMbss->HotSpotCtrl.HotSpotEnable) {
				if (!pMbss->HotSpotCtrl.DGAFDisable) {
					if (IsGratuitousARP(pAd, pSrcBuf - 2, pSrcBuf - 14, pMbss, 0))
						return FALSE;
				}
			}
#endif

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
			/* AP's tx shall check DAD.*/
			if (is_dad_packet(pAd, pSrcBuf - 2) || is_gratuitous_arp(pSrcBuf - 2))
				return FALSE;
#endif /* CONFIG_WIFI_PKT_FWD */
		}
#endif
#ifdef AUTOMATION
		if (pAd->auto_dvt && pAd->auto_dvt->apps.block_packet & ARP_PACKET) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Automation Block ARP Packet\n"));
			return FALSE;
		}
#endif /* !AUTOMATION */

		RTMP_SET_PACKET_ARP(pPacket, 1);
		RTMP_SET_PACKET_HIGH_PRIO(pPacket, 1);
		RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
	}
	break;

	case ETH_P_IPV6: {
#ifdef CONFIG_AP_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_AP) {
#ifdef CONFIG_DOT11V_WNM
			BSS_STRUCT *pMbss = (BSS_STRUCT *)wdev->func_dev;

			WNMIPv6ProxyARPCheck(pAd, pPacket, pSrcBuf, 0);

			if (pMbss->WNMCtrl.ProxyARPEnable) {
				/* Check if IPv6 Proxy ARP Candidate from DS */
				if (IsIPv6ProxyARPCandidate(pAd, pSrcBuf - 2)) {
					BOOLEAN FoundProxyARPEntry;

					FoundProxyARPEntry = IPv6ProxyARP(pAd, pMbss, pSrcBuf - 2, TRUE, 0);

					if (!FoundProxyARPEntry)
						MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Can not find IPv6 proxy entry\n"));

					return FALSE;
				}
			}

#endif
#ifdef CONFIG_HOTSPOT
			if (pMbss->HotSpotCtrl.HotSpotEnable) {
				if (!pMbss->HotSpotCtrl.DGAFDisable) {
					if (IsUnsolicitedNeighborAdver(pAd, pSrcBuf - 2))
						return FALSE;
				}
			}
#endif
		}
		/*
			Check if DHCPv6 Packet, and Convert group-address DHCP
			packets to individually-addressed 802.11 frames
		 */
#endif

		/* return AC_BE if packet is not IPv6 */
		if ((*pSrcBuf & 0xf0) != 0x60)
			up = 0;

#ifdef CSO_TEST_SUPPORT
		switch (*(pSrcBuf + 6)) {
		case IP_PROTO_UDP:
			if (CsCtrl & BIT2) {
#ifdef AUTOMATION
/* for MSP check always 0xFF for Rx log easily check PASS / FAIL */
				*(pSrcBuf + 46) = 0xFF;
				*(pSrcBuf + 47) = 0xFF;
#else /* AUTOMATION */
				*(pSrcBuf + 46) = ~(*(pSrcBuf + 46));
				*(pSrcBuf + 47) = ~(*(pSrcBuf + 47));
#endif /* !AUTOMATION */
			}
			if (CsCtrl & BIT3)
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Ipv6 udp pkt\n"));
			break;
		case IP_PROTOCOL_TCP:
			if (CsCtrl & BIT1) {
#ifdef AUTOMATION
/* for MSP check always 0xFF for Rx log easily check PASS / FAIL */
				*(pSrcBuf + 56) = 0xFF;
				*(pSrcBuf + 57) = 0xFF;
#else /* AUTOMATION */
				*(pSrcBuf + 56) = ~(*(pSrcBuf + 56));
				*(pSrcBuf + 57) = ~(*(pSrcBuf + 57));
#endif /* !AUTOMATION */
			}
			if (CsCtrl & BIT3)
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Ipv6 tcp pkt\n"));
			break;
		}
#endif
	}
	break;

	case ETH_TYPE_EAPOL:
		RTMP_SET_PACKET_EAPOL(pPacket, 1);
		RTMP_SET_PACKET_HIGH_PRIO(pPacket, 1);
		RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
#ifdef RT_CFG80211_SUPPORT
		bClearFrame = (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE;
		RTMP_SET_PACKET_CLEAR_EAP_FRAME(pPacket, (bClearFrame ? 1 : 0));
#endif
		break;
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

	case ETHER_TYPE_TDLS_MMPDU: {
		RTMP_SET_PACKET_TDLS_MMPDU(pPacket, 1);
		RTMP_SET_PACKET_HIGH_PRIO(pPacket, 1);
		RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
		up = 5;
	}
	break;
#endif /* DOT11Z_TDLS_SUPPORT */
	case ETH_TYPE_1905: {
		RTMP_SET_PACKET_HIGH_PRIO(pPacket, 1);
		RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
	}
	break;

	default:
		break;
	}

#ifdef VENDOR_FEATURE1_SUPPORT
	RTMP_SET_PACKET_PROTOCOL(pPacket, TypeLen);
#endif /* VENDOR_FEATURE1_SUPPORT */

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (pAd->vow_dvt_en) {
		UINT8 band = 0;
		UINT8 wcid = tr_entry->wcid;
		if (pAd->CommonCfg.dbdc_mode)
			band = WMODE_CAP_5G(wdev->PhyMode) ? 1 : 0;

		if ((!RTMP_GET_PACKET_MGMT_PKT(pPacket)) &&
			(RTMP_GET_PACKET_TYPE(pPacket) != TX_ALTX)) {
			if ((wcid == pAd->vow_cloned_wtbl_from[band]) ||
				((wcid >= pAd->vow_cloned_wtbl_start[band]) &&
				(wcid <= pAd->vow_cloned_wtbl_max[band]))) {
				q_idx = pAd->vow_sta_ac[tr_entry->wcid];
				up = dmac_ac_queue_to_up[pAd->vow_sta_ac[tr_entry->wcid]];
				final_user_prio = up;
			}
		}
	}
#endif
	/*
	 * Set WMM when
	 * 1. wdev->bWmmCapable == TRUE
	 * 2. Receiver's capability
	 *	a). bc/mc packets
	 *		->Need to get UP for IGMP use
	 *	b). unicast packets
	 *		-> CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE)
	 *	3. has VLAN tag or DSCP fields in IPv4/IPv6 hdr
	 */
	if ((wdev->bWmmCapable == TRUE) && (up <= 7)) {
		if (RTMP_GET_PACKET_HIGH_PRIO(pPacket)) {
			if ((up == 0) || (up == 3)) {
				if (is_hotspot_disabled_for_wdev(pAd, wdev))
					up = dmac_ac_queue_to_up[pAd->cp_support];
			}
		}
	}

	/* have to check ACM bit. downgrade UP & QueIdx before passing ACM*/
	/* NOTE: AP doesn't have to negotiate TSPEC. ACM is controlled purely via user setup, not protocol handshaking*/
	/*
		Under WMM ACM control, we dont need to check the bit;
		Or when a TSPEC is built for VO but we will change priority to
		BE here and when we issue a BA session, the BA session will
		be BE session, not VO session.
	*/
	if ((pAd->CommonCfg.APEdcaParm[0].bACM[WMM_UP2AC_MAP[up]])
		|| (((wdev->wdev_type == WDEV_TYPE_REPEATER)
			|| (wdev->wdev_type == WDEV_TYPE_STA))
			&& (pMacEntry->bACMBit[WMM_UP2AC_MAP[up]])))

		up = 0;



	if ((wdev->bWmmCapable == TRUE) && (up <= 7)) {
		final_user_prio = up;
		q_idx = WMM_UP2AC_MAP[up];
	}

	RTMP_SET_PACKET_UP(pPacket, final_user_prio);
	RTMP_SET_PACKET_QUEIDX(pPacket, q_idx);
	return TRUE;
}

#ifdef SOFT_ENCRYPT
BOOLEAN RTMPExpandPacketForSwEncrypt(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk)
{
	UINT32	ex_head = 0, ex_tail = 0;
	UCHAR	NumberOfFrag = RTMP_GET_PACKET_FRAGMENTS(pTxBlk->pPacket);
		if (pTxBlk->CipherAlg == CIPHER_AES)
			ex_tail = LEN_CCMP_MIC;

	ex_tail = (NumberOfFrag * ex_tail);
	pTxBlk->pPacket = ExpandPacket(pAd, pTxBlk->pPacket, ex_head, ex_tail);

	if (pTxBlk->pPacket == NULL) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: out of resource.\n", __func__));
		return FALSE;
	}

	pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pTxBlk->pPacket);
	pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pTxBlk->pPacket);
	return TRUE;
}


VOID RTMPUpdateSwCacheCipherInfo(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR *pHdr)
{
	HEADER_802_11 *pHeader_802_11;
	MAC_TABLE_ENTRY *pMacEntry;

	pHeader_802_11 = (HEADER_802_11 *) pHdr;
	pMacEntry = pTxBlk->pMacEntry;

	if (pMacEntry && pHeader_802_11->FC.Wep &&
		CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT)) {
		PCIPHER_KEY pKey = &pMacEntry->PairwiseKey;

		TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);
		pTxBlk->CipherAlg = pKey->CipherAlg;
		pTxBlk->pKey = pKey;
			if ((pKey->CipherAlg == CIPHER_WEP64) || (pKey->CipherAlg == CIPHER_WEP128))
				inc_iv_byte(pKey->TxTsc, LEN_WEP_TSC, 1);
			else if ((pKey->CipherAlg == CIPHER_TKIP) || (pKey->CipherAlg == CIPHER_AES))
				inc_iv_byte(pKey->TxTsc, LEN_WPA_TSC, 1);
	}
}


INT tx_sw_encrypt(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk, UCHAR *pHeaderBufPtr, HEADER_802_11 *wifi_hdr)
{
	UCHAR iv_offset = 0, ext_offset = 0;
	/*
		If original Ethernet frame contains no LLC/SNAP,
		then an extra LLC/SNAP encap is required
	*/
	EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2,
										pTxBlk->pExtraLlcSnapEncap);

	/* Insert LLC-SNAP encapsulation (8 octets) to MPDU data buffer */
	if (pTxBlk->pExtraLlcSnapEncap) {
		/* Reserve the front 8 bytes of data for LLC header */
		pTxBlk->pSrcBufData -= LENGTH_802_1_H;
		pTxBlk->SrcBufLen += LENGTH_802_1_H;
		NdisMoveMemory(pTxBlk->pSrcBufData, pTxBlk->pExtraLlcSnapEncap, 6);
	}

	/* Construct and insert specific IV header to MPDU header */
	RTMPSoftConstructIVHdr(pTxBlk->CipherAlg,
						   pTxBlk->KeyIdx,
						   pTxBlk->pKey->TxTsc,
						   pHeaderBufPtr, &iv_offset);
	pHeaderBufPtr += iv_offset;
	/* TODO: shiang-MT7603, for header Len, shall we take care that?? */
	pTxBlk->MpduHeaderLen += iv_offset;
	/* Encrypt the MPDU data by software */
	RTMPSoftEncryptionAction(pAd,
							 pTxBlk->CipherAlg,
							 (UCHAR *)wifi_hdr,
							 pTxBlk->pSrcBufData,
							 pTxBlk->SrcBufLen,
							 pTxBlk->KeyIdx,
							 pTxBlk->pKey, &ext_offset);
	pTxBlk->SrcBufLen += ext_offset;
	pTxBlk->TotalFrameLen += ext_offset;
	return TRUE;
}
#endif /* SOFT_ENCRYPT */

#ifdef IP_ASSEMBLY
/*for cache usage to improve throughput*/
static INT rtmp_IpAssembleDataCreate(RTMP_ADAPTER *pAd, UCHAR queId, IP_ASSEMBLE_DATA **ppIpAsmbData, UINT id, UINT fragSize)
{
	ULONG now = 0;
	IP_ASSEMBLE_DATA *pIpAsmbData = NULL;
	DL_LIST *pAssHead = &pAd->assebQueue[queId];

	os_alloc_mem(NULL, (UCHAR **)&pIpAsmbData, sizeof(IP_ASSEMBLE_DATA));
	*ppIpAsmbData = pIpAsmbData;

	if (pIpAsmbData == NULL)
		return NDIS_STATUS_FAILURE;

	InitializeQueueHeader(&pIpAsmbData->queue);
	NdisGetSystemUpTime(&now);
	pIpAsmbData->identify = id;
	pIpAsmbData->fragSize = fragSize;
	pIpAsmbData->createTime = now;
	DlListAdd(pAssHead, &pIpAsmbData->list);
	return NDIS_STATUS_SUCCESS;
}


static VOID rtmp_IpAssembleDataDestory(RTMP_ADAPTER *pAd, IP_ASSEMBLE_DATA *pIpAsmbData)
{
	PQUEUE_ENTRY pPktEntry;
	PNDIS_PACKET pPkt;

	/*free queue packet*/
	while (1) {
		pPktEntry = RemoveHeadQueue(&pIpAsmbData->queue);

		if (pPktEntry == NULL)
			break;

		pPkt = QUEUE_ENTRY_TO_PACKET(pPktEntry);
		RELEASE_NDIS_PACKET(pAd, pPkt, NDIS_STATUS_FAILURE);
	}

	/*remove from list*/
	DlListDel(&pIpAsmbData->list);
	/*free data*/
	os_free_mem(pIpAsmbData);
}


static IP_ASSEMBLE_DATA *rtmp_IpAssembleDataSearch(RTMP_ADAPTER *pAd, UCHAR queIdx, UINT identify)
{
	DL_LIST *pAssHead = &pAd->assebQueue[queIdx];
	IP_ASSEMBLE_DATA *pAssData = NULL;

	DlListForEach(pAssData, pAssHead, struct ip_assemble_data, list) {
		if (pAssData->identify == identify)
			return pAssData;
	}
	return NULL;
}


static VOID rtmp_IpAssembleDataUpdate(RTMP_ADAPTER *pAd)
{
	DL_LIST *pAssHead = NULL;
	IP_ASSEMBLE_DATA *pAssData = NULL, *pNextAssData = NULL;
	INT i = 0;
	ULONG now = 0;
	QUEUE_HEADER *pAcQueue = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	struct ip_assemble_data **cur_ip_assem_data = (struct ip_assemble_data **) pAd->cur_ip_assem_data;

	NdisGetSystemUpTime(&now);

	for (i = 0; i < num_of_tx_ring; i++) {
		pAssHead = &pAd->assebQueue[i];
		DlListForEachSafe(pAssData, pNextAssData, pAssHead, struct ip_assemble_data, list) {
			pAcQueue = &pAssData->queue;

			if ((pAcQueue->Number != 0) && (RTMP_TIME_AFTER(now, (pAssData->createTime) + (1000 * OS_HZ)))) {
				if (cur_ip_assem_data[i] == pAssData)
					cur_ip_assem_data[i] = NULL;

				rtmp_IpAssembleDataDestory(pAd, pAssData);
			}
		}
	}
}


INT rtmp_IpAssembleHandle(RTMP_ADAPTER *pAd, STA_TR_ENTRY *pTrEntry, PNDIS_PACKET pPacket, UCHAR queIdx, PACKET_INFO packetInfo)
{
	IP_ASSEMBLE_DATA *pIpAsmData = NULL;
	/*define local variable*/
	IP_V4_HDR *pIpv4Hdr, Ipv4Hdr;
	IP_FLAGS_FRAG_OFFSET *pFlagsFragOffset, flagsFragOffset;
	UINT fragSize = 0;
	QUEUE_HEADER *pAcQueue = NULL;
	UINT32 fragCount = 0;
	struct ip_assemble_data **cur_ip_assem_data = (struct ip_assemble_data **) pAd->cur_ip_assem_data;
	/*check all timer of assemble for ageout */
	rtmp_IpAssembleDataUpdate(pAd);

	/*is not ipv4 packet*/
	if (!RTMP_GET_PACKET_IPV4(pPacket)) {
		/*continue to do normal path*/
		return NDIS_STATUS_INVALID_DATA;
	}

	pFlagsFragOffset = (IP_FLAGS_FRAG_OFFSET *) (packetInfo.pFirstBuffer + LENGTH_802_3 + 6);
	flagsFragOffset.word = ntohs(pFlagsFragOffset->word);

	/*is not fragment packet*/
	if (flagsFragOffset.field.flags_more_frag == 0 && flagsFragOffset.field.frag_offset == 0) {
		/*continue to do normal path*/
		return NDIS_STATUS_INVALID_DATA;
	}

	/*get ipv4 */
	pIpv4Hdr = (IP_V4_HDR *) (packetInfo.pFirstBuffer + LENGTH_802_3);
	Ipv4Hdr.identifier = ntohs(pIpv4Hdr->identifier);
	Ipv4Hdr.tot_len = ntohs(pIpv4Hdr->tot_len);
	Ipv4Hdr.ihl = pIpv4Hdr->ihl;
	fragSize = Ipv4Hdr.tot_len - (Ipv4Hdr.ihl * 4);

	/* check if 1st fragment */
	if ((flagsFragOffset.field.flags_more_frag == 1) && (flagsFragOffset.field.frag_offset == 0)) {
		/*check current queue is exist this id packet or not*/
		pIpAsmData = rtmp_IpAssembleDataSearch(pAd, queIdx, Ipv4Hdr.identifier);

		/*if not exist, create it*/
		if (!pIpAsmData) {
			rtmp_IpAssembleDataCreate(pAd, queIdx, &pIpAsmData, Ipv4Hdr.identifier, fragSize);

			if (!pIpAsmData) {
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				return NDIS_STATUS_FAILURE;
			}
		}

		/*store to  cache */
		cur_ip_assem_data[queIdx] = pIpAsmData;
		/*insert packet*/
		pAcQueue = &pIpAsmData->queue;
		InsertTailQueue(pAcQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
	} else {
		/*search assemble data from identify and cache first*/
		if (cur_ip_assem_data[queIdx] && (cur_ip_assem_data[queIdx]->identify == Ipv4Hdr.identifier))
			pIpAsmData = cur_ip_assem_data[queIdx];
		else {
			pIpAsmData = rtmp_IpAssembleDataSearch(pAd, queIdx, Ipv4Hdr.identifier);

			/*not create assemble, should drop*/
			if (!pIpAsmData) {
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				return NDIS_STATUS_FAILURE;
			}

			/*update cache*/
			cur_ip_assem_data[queIdx] = pIpAsmData;
		}

		pAcQueue = &pIpAsmData->queue;
		InsertTailQueue(pAcQueue, PACKET_TO_QUEUE_ENTRY(pPacket));

		/* check if last fragment */
		if (pIpAsmData && (flagsFragOffset.field.flags_more_frag == 0) && (flagsFragOffset.field.frag_offset != 0)) {
			/*fragment packets gatter and check*/
			fragCount = ((flagsFragOffset.field.frag_offset * 8) / (pIpAsmData->fragSize)) + 1;

			if (pAcQueue->Number != fragCount) {
				rtmp_IpAssembleDataDestory(pAd, pIpAsmData);
				cur_ip_assem_data[queIdx] = NULL;
				return NDIS_STATUS_FAILURE;
			}

			/* move backup fragments to software queue */
			if (ge_enq_req(pAd, NULL, queIdx, pTrEntry, pAcQueue) == FALSE) {
				rtmp_IpAssembleDataDestory(pAd, pIpAsmData);
				cur_ip_assem_data[queIdx] = NULL;
				return NDIS_STATUS_FAILURE;
			}

			rtmp_IpAssembleDataDestory(pAd, pIpAsmData);
			cur_ip_assem_data[queIdx] = NULL;
		}
	}

	return NDIS_STATUS_SUCCESS;
}
#endif


VOID enable_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					 UINT8 ac_type, UINT8 prio, UINT16 level)
{
	if (wdev == NULL)
		return;
#ifdef IXIA_SUPPORT
	if (pAd->masktxop == TRUE)
		return;
#endif /*IXIA_SUPPORT*/
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s, prio=%d, level=0x%x, <caller: %pS>\n",
				  __func__, prio, level,
				  __builtin_return_address(0)));
		HW_SET_TX_BURST(pAd, wdev, ac_type, prio, level, 1);
	}

#endif /* MT_MAC */
}

VOID disable_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					  UINT8 ac_type, UINT8 prio, UINT16 level)
{
	if (wdev == NULL)
		return;
#ifdef IXIA_SUPPORT
	if (pAd->masktxop == TRUE)
		return;
#endif /*IXIA_SUPPORT*/
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s, prio=%d, level=0x%x, <caller: %pS>\n",
				  __func__, prio, level,
				  __builtin_return_address(0)));
		HW_SET_TX_BURST(pAd, wdev, ac_type, prio, level, 0);
	}

#endif /* MT_MAC */
}

UINT8 query_tx_burst_prio(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UINT8 i, prio = PRIO_DEFAULT;
	UINT32 prio_bitmap = 0;

	if (wdev == NULL)
		return prio;
#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		prio_bitmap = wdev->bss_info_argument.prio_bitmap;
		for (i = 0; i < MAX_PRIO_NUM; i++) {
			if (prio_bitmap & (1 << i))
				prio = i;
		}
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s, curr: prio=%d, txop=0x%x, <caller: %pS>\n",
				__func__, prio, wdev->bss_info_argument.txop_level[prio],
				__builtin_return_address(0)));
	}
#endif /* MT_MAC */
	return prio;
}

INT TxOPUpdatingAlgo(RTMP_ADAPTER *pAd)
{
	UCHAR UpdateTxOP = 0xFF;
	UINT32 TxTotalByteCnt = pAd->TxTotalByteCnt;
	UINT32 RxTotalByteCnt = pAd->RxTotalByteCnt;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if ((TxTotalByteCnt == 0) || (RxTotalByteCnt == 0)) {
		/* Avoid to divide 0, when doing the traffic calculating */
		/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Not expected one of them is 0, TxTotalByteCnt = %lu, RxTotalByteCnt = %lu\n", (ULONG)TxTotalByteCnt, (ULONG)RxTotalByteCnt)); */
	} else if ((pAd->MacTab.Size == 1)
			   && (pAd->CommonCfg.ManualTxop == 0)
			   && pAd->CommonCfg.bEnableTxBurst
			   && (cap->qos.TxOPScenario == 1)) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE) == TRUE) {
			UpdateTxOP = 0x80; /* Tx/Rx/Bi */
		} else if ((((TxTotalByteCnt + RxTotalByteCnt) << 3) >> 20) > pAd->CommonCfg.ManualTxopThreshold) {
			/* TxopThreshold unit is Mbps */
			if (TxTotalByteCnt > RxTotalByteCnt) {
				if ((TxTotalByteCnt / RxTotalByteCnt) >= pAd->CommonCfg.ManualTxopUpBound) {
					/* Boundary unit is Ratio */
					UpdateTxOP = 0x60; /* Tx */
				} else if ((TxTotalByteCnt / RxTotalByteCnt) <= pAd->CommonCfg.ManualTxopLowBound) {
					/* UpdateTxOP = 0x0; // Bi */
					UpdateTxOP = (pAd->MacTab.fCurrentStaBw40) ? 0x0 : 0x60;
				} else {
					/* No change TxOP */
				}
			} else {
				if ((RxTotalByteCnt / TxTotalByteCnt) >= pAd->CommonCfg.ManualTxopUpBound) {
					/* Boundary unit is Ratio */
					UpdateTxOP = 0x0; /* Rx */
				} else if ((RxTotalByteCnt / TxTotalByteCnt) <= pAd->CommonCfg.ManualTxopLowBound) {
					/* UpdateTxOP = 0x0; // Bi */
					UpdateTxOP = (pAd->MacTab.fCurrentStaBw40) ? 0x0 : 0x60;
				} else {
					/* No change TxOP */
				}
			}
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Current TP=%lu < Threshold(%lu), turn-off TxOP\n",
					 (ULONG)(((TxTotalByteCnt + RxTotalByteCnt) << 3) >> 20), (ULONG)pAd->CommonCfg.ManualTxopThreshold));
			UpdateTxOP = 0x0;
		}
	} else if (pAd->MacTab.Size > 1)
		UpdateTxOP = 0x0;

	if (UpdateTxOP != 0xFF &&
		UpdateTxOP != pAd->CurrEdcaParam[WMM_PARAM_AC_1].u2Txop)
		AsicUpdateTxOP(pAd, WMM_PARAM_AC_1, UpdateTxOP);

	pAd->TxTotalByteCnt = 0;
	pAd->RxTotalByteCnt = 0;
	return TRUE;
}


VOID ComposeNullFrame(
	RTMP_ADAPTER *pAd,
	PHEADER_802_11 pNullFrame,
	UCHAR *pAddr1,
	UCHAR *pAddr2,
	UCHAR *pAddr3)
{
	NdisZeroMemory(pNullFrame, sizeof(HEADER_802_11));
	pNullFrame->FC.Type = FC_TYPE_DATA;
	pNullFrame->FC.SubType = SUBTYPE_DATA_NULL;
	pNullFrame->FC.ToDs = 1;
	COPY_MAC_ADDR(pNullFrame->Addr1, pAddr1);
	COPY_MAC_ADDR(pNullFrame->Addr2, pAddr2);
	COPY_MAC_ADDR(pNullFrame->Addr3, pAddr3);
}

inline VOID mt_detect_wmm_traffic(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR usr_prior, UCHAR FlgIsOutput)
{
	/*Check WMM Rx traffic : Count Non-BE pkt in 1 sec*/
	if (WMM_UP2AC_MAP[usr_prior] != QID_AC_BE) {
		if (FlgIsOutput == FLG_IS_OUTPUT)
			pAd->tx_OneSecondnonBEpackets++;
		else
			pAd->rx_OneSecondnonBEpackets++;
	}
}

VOID mt_dynamic_wmm_be_tx_op(
	IN RTMP_ADAPTER *pAd,
	IN ULONG nonBEpackets)
{
	struct wifi_dev *wdev;
#if defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	INT idx;
#endif /* APCLI_SUPPORT */

	if (pAd->tx_OneSecondnonBEpackets > nonBEpackets ||
		pAd->rx_OneSecondnonBEpackets > nonBEpackets) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE)) {

#ifdef APCLI_SUPPORT
			for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
				wdev = &pAd->StaCfg[idx].wdev;

					if ((wdev) && (pAd->StaCfg[idx].ApcliInfStat.Valid == TRUE))
					enable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);
			}

#endif /* APCLI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

			if (wdev)
				enable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);

#ifdef PKT_BUDGET_CTRL_SUPPORT
			if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
				HW_SET_PBC_CTRL(pAd, NULL, NULL, PBC_TYPE_WMM);
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT

			for (idx = 0; idx < pAd->MSTANum; idx++) {
				wdev = &pAd->StaCfg[idx].wdev;

				if ((wdev) && (pAd->StaCfg[idx].wdev.DevInfo.WdevActive))
					enable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);
			}

#endif /* CONFIG_STA_SUPPORT */
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE);
		}
	} else {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) == 0) {

#ifdef APCLI_SUPPORT
			for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
				wdev = &pAd->StaCfg[idx].wdev;

					if ((wdev) && (pAd->StaCfg[idx].ApcliInfStat.Valid == TRUE))
					disable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);
			}

#endif /* APCLI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

			if (wdev)
				disable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);

#ifdef PKT_BUDGET_CTRL_SUPPORT
			if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
				HW_SET_PBC_CTRL(pAd, NULL, NULL, PBC_TYPE_NORMAL);
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
			for (idx = 0; idx < pAd->MSTANum; idx++) {
				wdev = &pAd->StaCfg[idx].wdev;

				if ((wdev) && (pAd->StaCfg[idx].wdev.DevInfo.WdevActive))
					disable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);
			}
#endif /* CONFIG_STA_SUPPORT */
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE);
		}
	}

	pAd->tx_OneSecondnonBEpackets = 0;
	pAd->rx_OneSecondnonBEpackets = 0;
}

UINT32 Get_OBSS_AirTime(
	IN	PRTMP_ADAPTER	pAd,
	IN      UCHAR                   BandIdx)
{
	return pAd->OneSecMibBucket.OBSSAirtime[BandIdx];
}

VOID Reset_OBSS_AirTime(
	IN	PRTMP_ADAPTER	pAd,
	IN      UCHAR                   BandIdx)
{
	/* Policy. CR access MUST implement at FW.
	     It will move to FW in the future. */
	UINT32  CrValue = 0;

	HW_IO_READ32(pAd->hdev_ctrl, RMAC_MIBTIME0, &CrValue);
	CrValue |= 1 << RX_MIBTIME_CLR_OFFSET;
	CrValue |= 1 << RX_MIBTIME_EN_OFFSET;
	HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MIBTIME0, CrValue);
}

UINT32 Get_My_Tx_AirTime(
	IN	PRTMP_ADAPTER	pAd,
	IN      UCHAR                 BandIdx)
{
	return pAd->OneSecMibBucket.MyTxAirtime[BandIdx];
}

UINT32 Get_My_Rx_AirTime(
	IN	PRTMP_ADAPTER	pAd,
	IN      UCHAR                   BandIdx)
{
	return pAd->OneSecMibBucket.MyRxAirtime[BandIdx];
}

UINT32 Get_EDCCA_Time(
	IN	PRTMP_ADAPTER	pAd,
	IN      UCHAR                   BandIdx)
{
	return pAd->OneSecMibBucket.EDCCAtime[BandIdx];
}


VOID CCI_ACI_scenario_maintain(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR   i = 0;
	UCHAR   j = 0;
	UINT32  ObssAirTime[DBDC_BAND_NUM] = {0};
	UINT32  MyTxAirTime[DBDC_BAND_NUM] = {0};
	UINT32  MyRxAirTime[DBDC_BAND_NUM] = {0};
	UINT32  EDCCATime[DBDC_BAND_NUM] = {0};
	UCHAR   ObssAirOccupyPercentage[DBDC_BAND_NUM] = {0};
	UCHAR   MyAirOccupyPercentage[DBDC_BAND_NUM] = {0};
	UCHAR   EdccaOccupyPercentage[DBDC_BAND_NUM] = {0};
	UCHAR   MyTxAirOccupyPercentage[DBDC_BAND_NUM] = {0};
	UCHAR concurrent_bands = HcGetAmountOfBand(pAd);
	struct wifi_dev *pWdev = NULL;
	UCHAR BandIdx;

#ifdef DYNAMIC_STEERING_LOADING
	if (pAd->disable_auto_txop == 1)
		return;
#endif
	if (concurrent_bands > DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("It should not happen!!!!!"));
		concurrent_bands = DBDC_BAND_NUM;
	}

	for (i = 0; i < concurrent_bands; i++) {
		ObssAirTime[i] = Get_OBSS_AirTime(pAd, i);
		MyTxAirTime[i] = Get_My_Tx_AirTime(pAd, i);
		MyRxAirTime[i] = Get_My_Rx_AirTime(pAd, i);
		EDCCATime[i] = Get_EDCCA_Time(pAd, i);

		if (ObssAirTime[i] != 0)
			ObssAirOccupyPercentage[i] = (ObssAirTime[i] * 100) / ONE_SEC_2_US;

		if (MyTxAirTime[i] != 0 || MyRxAirTime[i] != 0)
			MyAirOccupyPercentage[i] = ((MyTxAirTime[i] + MyRxAirTime[i]) * 100) / ONE_SEC_2_US;

		if (EDCCATime[i] != 0)
			EdccaOccupyPercentage[i] = (EDCCATime[i] * 100) / ONE_SEC_2_US;

		if (MyTxAirTime[i] + MyRxAirTime[i] != 0)
			MyTxAirOccupyPercentage[i] = (MyTxAirTime[i] * 100) / (MyTxAirTime[i] + MyRxAirTime[i]);

		/* MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, */
		/* ("Band%d OBSSAirtime=%d, MyAirtime=%d, EdccaTime=%d, MyTxAirtime=%d, MyRxAirtime=%d\n", */
		/* i, ObssAirOccupyPercentage[i], MyAirOccupyPercentage[i], EdccaOccupyPercentage[i], MyTxAirTime[i], MyRxAirTime[i])); */

		if ((ObssAirOccupyPercentage[i] > OBSS_OCCUPY_PERCENT_HIGH_TH)
			&& (ObssAirOccupyPercentage[i] + MyAirOccupyPercentage[i] > ALL_AIR_OCCUPY_PERCENT)
			&& MyTxAirOccupyPercentage[i] > TX_RATIO_TH
			&& MyAirOccupyPercentage[i] > My_OCCUPY_PERCENT) {
			if (pAd->CCI_ACI_TxOP_Value[i] == 0) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("CCI detected !!!!! Apply TxOP=FE\n"));

				for (j = 0; j < WDEV_NUM_MAX; j++) {
					pWdev = pAd->wdev_list[j];

					if (!pWdev)
						continue;

					BandIdx = HcGetBandByWdev(pWdev);

					if (BandIdx == i) {
						if (pAd->ccsa_overlapping == TRUE) {
							MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("CCSA case !!!!! Apply TxOP=0x80\n"));
							pAd->CCI_ACI_TxOP_Value[i] = TXOP_80;
							enable_tx_burst(pAd, pWdev, AC_BE, PRIO_CCI, TXOP_80);
						} else {
						pAd->CCI_ACI_TxOP_Value[i] = TXOP_FE;
						enable_tx_burst(pAd, pWdev, AC_BE, PRIO_CCI, TXOP_FE);
						}
						break;
					}
				}
			}
		} else if ((EdccaOccupyPercentage[i] > OBSS_OCCUPY_PERCENT_HIGH_TH)
				   && (EdccaOccupyPercentage[i] + MyAirOccupyPercentage[i] > ALL_AIR_OCCUPY_PERCENT)
				   && MyTxAirOccupyPercentage[i] > TX_RATIO_TH
				   && MyAirOccupyPercentage[i] > My_OCCUPY_PERCENT) {
			if (pAd->CCI_ACI_TxOP_Value[i] == 0) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("ACI detected !!!!! Apply TxOP=FE\n"));

				for (j = 0; j < WDEV_NUM_MAX; j++) {
					pWdev = pAd->wdev_list[j];

					if (!pWdev)
						continue;

					BandIdx = HcGetBandByWdev(pWdev);

					if (BandIdx == i) {
						if (pAd->ccsa_overlapping == TRUE) {
							MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("CCSA case !!!!! Apply TxOP=0x80\n"));
							pAd->CCI_ACI_TxOP_Value[i] = TXOP_80;
							enable_tx_burst(pAd, pWdev, AC_BE, PRIO_CCI, TXOP_80);
						} else {
						pAd->CCI_ACI_TxOP_Value[i] = TXOP_FE;
						enable_tx_burst(pAd, pWdev, AC_BE, PRIO_CCI, TXOP_FE);
						}
						break;
					}
				}
			}
		} else {
			if (pAd->CCI_ACI_TxOP_Value[i] != 0) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("NO CCI /ACI detected !!!!! Apply TxOP=0\n"));

				for (j = 0; j < WDEV_NUM_MAX; j++) {
					pWdev = pAd->wdev_list[j];

					if (!pWdev)
						continue;

					BandIdx = HcGetBandByWdev(pWdev);

					if (BandIdx == i) {
						pAd->CCI_ACI_TxOP_Value[i] = TXOP_0;
						disable_tx_burst(pAd, pWdev, AC_BE, PRIO_CCI, TXOP_0);
						break;
					}
				}
			}
		}

		/* Reset_OBSS_AirTime(pAd,i); */
	}
}

/*
	detect AC Category of trasmitting packets
	to turn AC0(BE) TX_OP (MAC reg 0x1300)
*/
/* TODO: shiang-usw, this function should move to other place!! */
VOID detect_wmm_traffic(
	IN RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	IN UCHAR UserPriority,
	IN UCHAR FlgIsOutput)
{
	if (!pAd)
		return;

	/* For BE & BK case and TxBurst function is disabled */
	if ((pAd->CommonCfg.bEnableTxBurst == FALSE)
#ifdef DOT11_N_SUPPORT
		&& (pAd->CommonCfg.bRdg == FALSE)
		&& (pAd->CommonCfg.bRalinkBurstMode == FALSE)
#endif /* DOT11_N_SUPPORT */
		&& (FlgIsOutput == 1)
	   ) {
		if (WMM_UP2AC_MAP[UserPriority] == QID_AC_BK) {
			/* has any BK traffic */
			if (pAd->flg_be_adjust == 0) {
				/* yet adjust */
				/* TODO: here need YF check! */
				/* RTMP_SET_TX_BURST(pAd, wdev); */
				pAd->flg_be_adjust = 1;
				NdisGetSystemUpTime(&pAd->be_adjust_last_time);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("wmm> adjust be!\n"));
			}
		} else {
			if (pAd->flg_be_adjust != 0) {
				QUEUE_HEADER *pQueue;
				/* has adjusted */
				pQueue = &pAd->TxSwQueue[QID_AC_BK];

				if ((pQueue == NULL) ||
					((pQueue != NULL) && (pQueue->Head == NULL))) {
					ULONG	now;

					NdisGetSystemUpTime(&now);

					if ((now - pAd->be_adjust_last_time) > TIME_ONE_SECOND) {
						/* no any BK traffic */
						/* TODO: here need YF check! */
						/* RTMP_SET_TX_BURST(pAd, wdev); */
						pAd->flg_be_adjust = 0;
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("wmm> recover be!\n"));
					}
				} else
					NdisGetSystemUpTime(&pAd->be_adjust_last_time);
			}
		}
	}

	/* count packets which priority is more than BE */
	if (UserPriority > 3) {
		pAd->tx_OneSecondnonBEpackets++;

		if (pAd->tx_OneSecondnonBEpackets > 100
#ifdef DOT11_N_SUPPORT
			&& pAd->MacTab.fAnyStationMIMOPSDynamic
#endif /* DOT11_N_SUPPORT */
		   ) {
			if (!pAd->is_on) {
				RTMP_AP_ADJUST_EXP_ACK_TIME(pAd);
				pAd->is_on = 1;
			}
		} else {
			if (pAd->is_on) {
				RTMP_AP_RECOVER_EXP_ACK_TIME(pAd);
				pAd->is_on = 0;
			}
		}
	}
}

#ifdef CONFIG_HOTSPOT
extern BOOLEAN hotspot_rx_handler(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk);
#endif /* CONFIG_HOTSPOT */

#ifdef AIR_MONITOR
extern VOID Air_Monitor_Pkt_Report_Action(PRTMP_ADAPTER pAd, UCHAR wcid, RX_BLK *pRxBlk);
#endif /* AIR_MONITOR */

/* TODO: shiang-usw, temporary put this function here, should remove to other place or re-write! */
VOID Update_Rssi_Sample(
	IN RTMP_ADAPTER *pAd,
	IN RSSI_SAMPLE * pRssi,
	IN struct rx_signal_info *signal,
	IN UCHAR phy_mode,
	IN UCHAR bw)
{
	BOOLEAN bInitial = FALSE;
	INT ant_idx, ant_max = 4;

	if (!(pRssi->AvgRssi[0] | pRssi->AvgRssiX8[0] | pRssi->LastRssi[0]))
		bInitial = TRUE;

	/* TODO: shiang-usw, shall we check this here to reduce the for loop count? */
	if (ant_max > pAd->Antenna.field.RxPath)
		ant_max = pAd->Antenna.field.RxPath;

	for (ant_idx = 0; ant_idx < ant_max; ant_idx++) {
		if (signal->raw_snr[ant_idx] != 0 && phy_mode != MODE_CCK) {
			pRssi->LastSnr[ant_idx] = ConvertToSnr(pAd, signal->raw_snr[ant_idx]);

			if (bInitial) {
				pRssi->AvgSnrX8[ant_idx] = pRssi->LastSnr[ant_idx] << 3;
				pRssi->AvgSnr[ant_idx] = pRssi->LastSnr[ant_idx];
			} else
				pRssi->AvgSnrX8[ant_idx] = (pRssi->AvgSnrX8[ant_idx] - pRssi->AvgSnr[ant_idx]) + pRssi->LastSnr[ant_idx];

			pRssi->AvgSnr[ant_idx] = pRssi->AvgSnrX8[ant_idx] >> 3;
		}

		if (signal->raw_rssi[ant_idx] != 0) {
			pRssi->LastRssi[ant_idx] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&signal->raw_rssi[0]), ant_idx);

			if (bInitial) {
				pRssi->AvgRssiX8[ant_idx] = pRssi->LastRssi[ant_idx] << 3;
				pRssi->AvgRssi[ant_idx] = pRssi->LastRssi[ant_idx];
			} else
				pRssi->AvgRssiX8[ant_idx] = (pRssi->AvgRssiX8[ant_idx] - pRssi->AvgRssi[ant_idx]) + pRssi->LastRssi[ant_idx];

			pRssi->AvgRssi[ant_idx] = pRssi->AvgRssiX8[ant_idx] >> 3;
		}
	}
}


#ifdef DOT11_N_SUPPORT
UINT deaggregate_amsdu_announce(
	IN RTMP_ADAPTER *pAd,
	PNDIS_PACKET pPacket,
	IN UCHAR *pData,
	IN ULONG DataSize,
	IN UCHAR OpMode)
{
	USHORT PayloadSize;
	USHORT SubFrameSize;
	HEADER_802_3 *pAMSDUsubheader;
	UINT nMSDU;
	UCHAR Header802_3[14];
	UCHAR *pPayload, *pDA, *pSA, *pRemovedLLCSNAP;
	PNDIS_PACKET pClonePacket;
	struct wifi_dev *wdev;
	UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);
	UCHAR VLAN_Size;
#ifdef CONFIG_AP_SUPPORT
	USHORT VLAN_VID = 0;
	USHORT VLAN_Priority = 0;
#endif
	ASSERT(wdev_idx < WDEV_NUM_MAX);

	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():invalud wdev_idx(%d)\n", __func__, wdev_idx));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		return 0;
	}

	wdev = pAd->wdev_list[wdev_idx];
	/* only MBssid support VLAN.*/
	VLAN_Size = (wdev->VLAN_VID != 0) ? LENGTH_802_1Q : 0;
	nMSDU = 0;

	while (DataSize > LENGTH_802_3) {
		nMSDU++;
		/*hex_dump("subheader", pData, 64);*/
		pAMSDUsubheader = (PHEADER_802_3)pData;
		/*pData += LENGTH_802_3;*/
		PayloadSize = pAMSDUsubheader->Octet[1] + (pAMSDUsubheader->Octet[0] << 8);
		SubFrameSize = PayloadSize + LENGTH_802_3;

		if ((DataSize < SubFrameSize) || (PayloadSize > 1518))
			break;

		/*MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%d subframe: Size = %d\n",  nMSDU, PayloadSize));*/
		pPayload = pData + LENGTH_802_3;
		pDA = pData;
		pSA = pData + MAC_ADDR_LEN;
		/* convert to 802.3 header*/
		CONVERT_TO_802_3(Header802_3, pDA, pSA, pPayload, PayloadSize, pRemovedLLCSNAP);
#ifdef CONFIG_STA_SUPPORT

		if ((Header802_3[12] == 0x88) && (Header802_3[13] == 0x8E)
		   ) {
			MLME_QUEUE_ELEM *Elem;

			os_alloc_mem(pAd, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));

			if (Elem != NULL) {
				struct wifi_dev_ops *ops = wdev->wdev_ops;
				MAC_TABLE_ENTRY *pEntry;

				ops->mac_entry_lookup(pAd, pSA, wdev, &pEntry);

				ASSERT(pEntry);

				if (pEntry) {
					memmove(Elem->Msg + (LENGTH_802_11 + LENGTH_802_1_H), pPayload, PayloadSize);
					Elem->MsgLen = LENGTH_802_11 + LENGTH_802_1_H + PayloadSize;
					REPORT_MGMT_FRAME_TO_MLME(pAd, pEntry->wcid, Elem->Msg,
						Elem->MsgLen, 0, 0, 0, 0, 0, 0,
						OPMODE_STA, wdev, pEntry->HTPhyMode.field.MODE);
				}

				os_free_mem(Elem);
			}
		}

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (pRemovedLLCSNAP) {
				pPayload -= (LENGTH_802_3 + VLAN_Size);
				PayloadSize += (LENGTH_802_3 + VLAN_Size);
				/*NdisMoveMemory(pPayload, &Header802_3, LENGTH_802_3);*/
			} else {
				pPayload -= VLAN_Size;
				PayloadSize += VLAN_Size;
			}

			WDEV_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, wdev);
			RT_VLAN_8023_HEADER_COPY(pAd, VLAN_VID, VLAN_Priority,
									 Header802_3, LENGTH_802_3, pPayload,
									 TPID);
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (pRemovedLLCSNAP) {
				pPayload -= LENGTH_802_3;
				PayloadSize += LENGTH_802_3;
				NdisMoveMemory(pPayload, &Header802_3[0], LENGTH_802_3);
			}
		}
#endif /* CONFIG_STA_SUPPORT */
		pClonePacket = ClonePacket(MONITOR_ON(pAd), wdev->if_dev, pPacket, pPayload, PayloadSize);

		if (pClonePacket) {
			UCHAR opmode = pAd->OpMode;
			announce_or_forward_802_3_pkt(pAd, pClonePacket, wdev, opmode);
		}

		/* A-MSDU has padding to multiple of 4 including subframe header.*/
		/* align SubFrameSize up to multiple of 4*/
		SubFrameSize = (SubFrameSize + 3) & (~0x3);

		if (SubFrameSize > 1528 || SubFrameSize < 32)
			break;

		if (DataSize > SubFrameSize) {
			pData += SubFrameSize;
			DataSize -= SubFrameSize;
		} else {
			/* end of A-MSDU*/
			DataSize = 0;
		}
	}

	/* finally release original rx packet*/
	RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
	return nMSDU;
}

VOID indicate_amsdu_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	UINT nMSDU;
	struct _RXD_BASE_STRUCT *rx_base;

	if (check_rx_pkt_pn_allowed(pAd, pRxBlk) == FALSE) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s:drop packet by PN mismatch!\n", __FUNCTION__));
#ifdef IXIA_SUPPORT
		pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_PN_ERROR]++;
#endif /*IXIA_SUPPORT*/
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	RTMP_UPDATE_OS_PACKET_INFO(pAd, pRxBlk, wdev_idx);
	RTMP_SET_PACKET_WDEV(pRxBlk->pRxPacket, wdev_idx);

	rx_base = (struct _RXD_BASE_STRUCT *)pRxBlk->rmac_info;

	if ((rx_base->RxD1.HdrOffset == 1) && (rx_base->RxD1.PayloadFmt != 0) && (rx_base->RxD1.HdrTranslation == 0)) {
		pRxBlk->pData += 2;
		pRxBlk->DataSize -= 2;
	}

	nMSDU = deaggregate_amsdu_announce(pAd, pRxBlk->pRxPacket, pRxBlk->pData, pRxBlk->DataSize, pRxBlk->OpMode);
}
#endif /* DOT11_N_SUPPORT */

VOID announce_or_forward_802_3_pkt(
	RTMP_ADAPTER *pAd,
	PNDIS_PACKET pPacket,
	struct wifi_dev *wdev,
	UCHAR op_mode)
{
	BOOLEAN to_os = FALSE;
	struct wifi_dev_ops *ops = wdev->wdev_ops;
#ifdef IXIA_SUPPORT
	UCHAR *pHeader8023;
#endif /*IXIA_SUPPORT*/
#ifdef VLAN_SUPPORT
	if (wdev && IS_VLAN_PACKET(GET_OS_PKT_DATAPTR(pPacket))) {
		UINT16 tci = (GET_OS_PKT_DATAPTR(pPacket)[14]<<8) | (GET_OS_PKT_DATAPTR(pPacket)[15]);
		UINT16 vid = tci & MASK_TCI_VID;

		if (wdev->VLAN_VID != 0) {
			if (vid != 0 && vid != wdev->VLAN_VID) {
				switch (wdev->VLAN_Policy[RX_VLAN]) {
				case VLAN_RX_DROP:
#ifdef IXIA_SUPPORT
					pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_NOT_TOOS]++;
#endif /*IXIA_SUPPORT*/
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					return;
				case VLAN_RX_ALLOW:
				default:
					break;
				}
			}
		}
	}
#endif /*VLAN_SUPPORT*/
#ifdef IXIA_SUPPORT
	pHeader8023 = (UCHAR *)GET_OS_PKT_DATAPTR(pPacket);
		/*if (pHeader8023 && (pAd->ixiaCtrl.itxCtrl == 2)) {*/
			/*MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ANpktlen:%d\n",RTPKT_TO_OSPKT(pPacket)->len));*/
				/*dumpPkt(pHeader8023, 100);*/
		/*}*/
	if ((pAd->ixiaCtrl.iMode == VERIWAVE_MODE)
		&& pHeader8023 && (RTPKT_TO_OSPKT(pPacket)->len == 248)
		&& (((pHeader8023[50] << 8) | pHeader8023[51]) == 0x8A)) {
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		pAd->NbdsDelCnt++;
		return;
	}
#endif /*IXIA_SUPPORT*/

	if (ops->rx_pkt_foward)
		to_os = ops->rx_pkt_foward(pAd, wdev, pPacket);

	if (to_os == TRUE) {
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

		if (wf_drv_tbl.wf_fwd_needed_hook != NULL && wf_drv_tbl.wf_fwd_needed_hook() == TRUE)
			set_wf_fwd_cb(pAd, pPacket, wdev);

#endif /* CONFIG_WIFI_PKT_FWD */

		announce_802_3_packet_cmm(pAd, wdev, pPacket);
	} else {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): No need to send to OS!\n", __func__));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
#ifdef IXIA_SUPPORT
		pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_NOTTO_OS]++;
#endif /*IXIA_SUPPORT*/
	}
}


VOID indicate_802_3_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;

	if (check_rx_pkt_pn_allowed(pAd, pRxBlk) == FALSE) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s:drop packet by PN mismatch!\n", __FUNCTION__));
#ifdef IXIA_SUPPORT
		pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_PN_ERROR]++;
#endif /*IXIA_SUPPORT*/
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	/* pass this 802.3 packet to upper layer or forward this packet to WM directly*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef MAC_REPEATER_SUPPORT /* This should be moved to some where else */

		if (pRxBlk->pRxInfo->Bcast && (pAd->ApCfg.bMACRepeaterEn) && (pAd->ApCfg.MACRepeaterOuiMode != CASUALLY_DEFINE_MAC_ADDR)) {
			PUCHAR pPktHdr, pLayerHdr;

			pPktHdr = GET_OS_PKT_DATAPTR(pRxPacket);
			pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);
#ifdef VLAN_SUPPORT
			if (IS_VLAN_PACKET(GET_OS_PKT_DATAPTR(pRxPacket)))
				pLayerHdr = (pPktHdr + MAT_VLAN_ETH_HDR_LEN);
#endif /*VLAN_SUPPORT*/

			/*For UDP packet, we need to check about the DHCP packet. */
			if (*(pLayerHdr + 9) == 0x11) {
				PUCHAR pUdpHdr;
				UINT16 srcPort, dstPort;
				BOOLEAN bHdrChanged = FALSE;

				pUdpHdr = pLayerHdr + 20;
				srcPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr)));
				dstPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr + 2)));

				if (srcPort == 67 && dstPort == 68) { /*It's a DHCP packet */
					PUCHAR bootpHdr, dhcpHdr, pCliHwAddr;
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

					bootpHdr = pUdpHdr + 8;
					dhcpHdr = bootpHdr + 236;
					pCliHwAddr = (bootpHdr + 28);
					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pCliHwAddr, TRUE);

					if (pReptEntry) {
						ASSERT(pReptEntry->CliValid == TRUE);
						NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
					}

					bHdrChanged = TRUE;
				}

				if (bHdrChanged == TRUE) {
					NdisZeroMemory((pUdpHdr + 6), 2); /*modify the UDP chksum as zero */
#ifdef DHCP_UC_SUPPORT
					*(UINT16 *)(pUdpHdr + 6) = RTMP_UDP_Checksum(pRxPacket);
#endif /* DHCP_UC_SUPPORT */
				}
			}
		}

#endif /* MAC_REPEATER_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
	SET_OS_PKT_NETDEV(pRxPacket, get_netdev_from_bssid(pAd, wdev_idx));
	SET_OS_PKT_DATATAIL(pRxPacket, GET_OS_PKT_LEN(pRxPacket));
	announce_or_forward_802_3_pkt(pAd, pRxBlk->pRxPacket, wdev_search_by_idx(pAd, wdev_idx), pAd->OpMode);
}

VOID indicate_802_11_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	UCHAR Header802_3[LENGTH_802_3];
	USHORT VLAN_VID = 0, VLAN_Priority = 0;
	UINT max_pkt_len = MAX_RX_PKT_LEN;
	UCHAR *pData = pRxBlk->pData;
	INT data_len = pRxBlk->DataSize;
	struct wifi_dev *wdev;
	UCHAR opmode = pAd->OpMode;

	ASSERT(wdev_idx < WDEV_NUM_MAX);

	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():invalid wdev_idx(%d)!\n", __func__, wdev_idx));
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
#ifdef IXIA_SUPPORT
		pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_WDEV_INDEX]++;
#endif /*IXIA_SUPPORT*/
		return;
	}

	if (check_rx_pkt_pn_allowed(pAd, pRxBlk) == FALSE) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s:drop packet by PN mismatch!\n", __FUNCTION__));
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
#ifdef IXIA_SUPPORT
		pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_PN_ERROR]++;
#endif /*IXIA_SUPPORT*/
		return;
	}

	wdev = pAd->wdev_list[wdev_idx];

	if (0)
		hex_dump("indicate_802_11_pkt", pData, data_len);

	/*
		1. get 802.3 Header
		2. remove LLC
			a. pointer pRxBlk->pData to payload
			b. modify pRxBlk->DataSize
	*/
#ifdef HDR_TRANS_SUPPORT

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
		max_pkt_len = 1514;
		pData = pRxBlk->pTransData;
		data_len = pRxBlk->TransDataSize;
	} else
#endif /* HDR_TRANS_SUPPORT */
	{
		if (pRxBlk->AmsduState == 0) {
			RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);

		} else {
			/* Update the DA and SA from AMSDU Header instead of 80211 HDR */
			PUCHAR _pRemovedLLCSNAP = NULL, _pDA = NULL, _pSA = NULL;

			_pDA = pRxBlk->pData - LENGTH_802_3;
			_pSA = pRxBlk->pData - 8; /* MAC_ADDR_LEN + PAD */

			CONVERT_TO_802_3(Header802_3, _pDA, _pSA, pRxBlk->pData,
				pRxBlk->DataSize, _pRemovedLLCSNAP);
		}
	}
	/* hex_dump("802_3_hdr", (UCHAR *)Header802_3, LENGTH_802_3); */
	pData = pRxBlk->pData;
	data_len = pRxBlk->DataSize;

	if (data_len > max_pkt_len) {
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():data_len(%d) > max_pkt_len(%d)!\n",
				 __func__, data_len, max_pkt_len));
#ifdef IXIA_SUPPORT
		pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_RXLEN_DISMATCH]++;
#endif /*IXIA_SUPPORT*/
		return;
	}

	STATS_INC_RX_PACKETS(pAd, wdev_idx);
#ifdef CONFIG_AP_SUPPORT
	WDEV_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, wdev);
#endif /* CONFIG_AP_SUPPORT */

	/* +++Add by shiang for debug */
	if (0) {
		hex_dump("Before80211_2_8023", pData, data_len);
		hex_dump("header802_3", &Header802_3[0], LENGTH_802_3);
	}

	/* ---Add by shiang for debug */
#ifdef HDR_TRANS_SUPPORT

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
		struct sk_buff *pOSPkt = RTPKT_TO_OSPKT(pRxPacket);

		pOSPkt->dev = get_netdev_from_bssid(pAd, wdev_idx);
		pOSPkt->data = pRxBlk->pTransData;
		pOSPkt->len = pRxBlk->TransDataSize;
		SET_OS_PKT_DATATAIL(pOSPkt, pOSPkt->len);
		/* printk("%s: rx trans ...%d\n", __func__, __LINE__); */
	} else
#endif /* HDR_TRANS_SUPPORT */
	{
		RT_80211_TO_8023_PACKET(pAd, VLAN_VID, VLAN_Priority,
								pRxBlk, Header802_3, wdev_idx, TPID);
	}
#ifndef APCLI_AS_WDS_STA_SUPPORT
	/* pass this 802.3 packet to upper layer or forward this packet to WM directly*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef MAC_REPEATER_SUPPORT /* This should be moved to some where else */

		if (pRxBlk->pRxInfo->Bcast && (pAd->ApCfg.bMACRepeaterEn) && (pAd->ApCfg.MACRepeaterOuiMode != CASUALLY_DEFINE_MAC_ADDR)) {
			PUCHAR pPktHdr, pLayerHdr;

			pPktHdr = GET_OS_PKT_DATAPTR(pRxPacket);
			pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);

			/*For UDP packet, we need to check about the DHCP packet. */
			if (*(pLayerHdr + 9) == 0x11) {
				PUCHAR pUdpHdr;
				UINT16 srcPort, dstPort;
				BOOLEAN bHdrChanged = FALSE;

				pUdpHdr = pLayerHdr + 20;
				srcPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr)));
				dstPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr + 2)));

				if (srcPort == 67 && dstPort == 68) { /*It's a DHCP packet */
					PUCHAR bootpHdr, dhcpHdr, pCliHwAddr;
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

					bootpHdr = pUdpHdr + 8;
					dhcpHdr = bootpHdr + 236;
					pCliHwAddr = (bootpHdr + 28);
					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pCliHwAddr, TRUE);

					if (pReptEntry) {
						ASSERT(pReptEntry->CliValid == TRUE);
						NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
					}

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
					else {
						VOID *opp_band_tbl = NULL;
						VOID *band_tbl = NULL;
						VOID *other_band_tbl = NULL;

						if (wf_drv_tbl.wf_fwd_feedback_map_table)
							wf_drv_tbl.wf_fwd_feedback_map_table(pAd,
												&band_tbl,
												&opp_band_tbl,
												&other_band_tbl);

						if (opp_band_tbl != NULL) {
							/*
								check the ReptTable of the opposite band due to dhcp packet (BC)
									may come-in 2/5G band when STA send dhcp broadcast to Root AP
							*/
							pReptEntry = RTMPLookupRepeaterCliEntry(opp_band_tbl, FALSE, pCliHwAddr, FALSE);

							if (pReptEntry)
								NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
						} else
							MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("cannot find the adapter of the oppsite band\n"));

						if (other_band_tbl != NULL) {
							pReptEntry = RTMPLookupRepeaterCliEntry(other_band_tbl, FALSE, pCliHwAddr, FALSE);

							if (pReptEntry)
								NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
						} else
							MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("cannot find the adapter of the othersite band\n"));
					}

#endif /* CONFIG_WIFI_PKT_FWD */
					bHdrChanged = TRUE;
				}

				if (bHdrChanged == TRUE)
					NdisZeroMemory((pUdpHdr + 6), 2); /*modify the UDP chksum as zero */
			}
		}

#endif /* MAC_REPEATER_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* APCLI_AS_WDS_STA_SUPPORT */


	if (0)
		hex_dump("After80211_2_8023", GET_OS_PKT_DATAPTR(pRxPacket), GET_OS_PKT_LEN(pRxPacket));

	announce_or_forward_802_3_pkt(pAd, pRxPacket, wdev, opmode);
}

VOID indicate_agg_ralink_pkt(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx)
{
	UCHAR Header802_3[LENGTH_802_3];
	UINT16 Msdu2Size;
	UINT16 Payload1Size, Payload2Size;
	PUCHAR pData2;
	PNDIS_PACKET pPacket2 = NULL;
	USHORT VLAN_VID = 0, VLAN_Priority = 0;
	UCHAR opmode = pAd->OpMode;
	struct wifi_dev *wdev;

	ASSERT(wdev_idx < WDEV_NUM_MAX);

	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid wdev_idx(%d)\n", __func__, wdev_idx));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	wdev = pAd->wdev_list[wdev_idx];
	Msdu2Size = *(pRxBlk->pData) + (*(pRxBlk->pData + 1) << 8);

	if ((Msdu2Size <= 1536) && (Msdu2Size < pRxBlk->DataSize)) {
		/* skip two byte MSDU2 len */
		pRxBlk->pData += 2;
		pRxBlk->DataSize -= 2;
	} else {
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	/* get 802.3 Header and  remove LLC*/
	RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
	ASSERT(pRxBlk->pRxPacket);

	if (!pRxBlk->pRxPacket) {
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	pAd->RalinkCounters.OneSecRxARalinkCnt++;
	Payload1Size = pRxBlk->DataSize - Msdu2Size;
	Payload2Size = Msdu2Size - LENGTH_802_3;
	pData2 = pRxBlk->pData + Payload1Size + LENGTH_802_3;
	pPacket2 = duplicate_pkt_vlan(wdev->if_dev,
								  wdev->VLAN_VID, wdev->VLAN_Priority,
								  (pData2 - LENGTH_802_3), LENGTH_802_3,
								  pData2, Payload2Size, TPID);

	if (!pPacket2) {
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	/* update payload size of 1st packet*/
	pRxBlk->DataSize = Payload1Size;
	RT_80211_TO_8023_PACKET(pAd, VLAN_VID, VLAN_Priority,
							pRxBlk, Header802_3, wdev_idx, TPID);
	announce_or_forward_802_3_pkt(pAd, pRxBlk->pRxPacket, wdev, opmode);

	if (pPacket2)
		announce_or_forward_802_3_pkt(pAd, pPacket2, wdev, opmode);
}

VOID indicate_ampdu_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	INT max_pkt_len, data_len;

	max_pkt_len = MAX_RX_PKT_LEN;
	data_len = pRxBlk->DataSize;

	if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU) &&  (data_len > max_pkt_len)) {
		static int err_size;

		err_size++;

		if (err_size > 20) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("AMPDU DataSize = %d\n", pRxBlk->DataSize));
			hex_dump("802.11 Header", pRxBlk->FC, 24);
			hex_dump("Payload", pRxBlk->pData, 64);
			err_size = 0;
		}
#ifdef IXIA_SUPPORT
		pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_RXLEN_DISMATCH1]++;
#endif /*IXIA_SUPPORT*/
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	ba_reorder(pAd, pRxBlk, wdev_idx);
}

#define RESET_FRAGFRAME(_fragFrame) \
	{								\
		_fragFrame.RxSize = 0;		\
		_fragFrame.Sequence = 0;	\
		_fragFrame.LastFrag = 0;	\
		_fragFrame.Flags = 0;		\
	}

VOID de_fragment_data_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	UCHAR *pData = pRxBlk->pData;
	USHORT DataSize = pRxBlk->DataSize;
	PNDIS_PACKET pRetPacket = NULL;
	UCHAR *pFragBuffer = NULL;
	BOOLEAN bReassDone = FALSE;
	UCHAR HeaderRoom = 0;
	RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 RXWISize = cap->RXWISize;
	/* TODO: shiang-MT7603, fix me for this function work in MT series chips */
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		RXWISize = 0;

#endif /* MT_MAC */
	HeaderRoom = pData - pRxBlk->FC;

	/* Re-assemble the fragmented packets*/
	if (pRxBlk->FN == 0) {
		/* Frag. Number is 0 : First frag or only one pkt*/
		/* the first pkt of fragment, record it.*/
		if (FC->MoreFrag && pAd->FragFrame.pFragPacket) {
			pFragBuffer = GET_OS_PKT_DATAPTR(pAd->FragFrame.pFragPacket);
			/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items.
			    Copy RxWI content to pFragBuffer.
			*/
			/* pAd->FragFrame.RxSize = DataSize + HeaderRoom; */
			/* NdisMoveMemory(pFragBuffer, pHeader, pAd->FragFrame.RxSize); */
#ifdef HDR_TRANS_RX_SUPPORT

			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
				pAd->FragFrame.RxSize = DataSize + RXWISize;
				NdisMoveMemory(pFragBuffer, pRxWI, RXWISize);
				NdisMoveMemory(pFragBuffer + RXWISize, pData, pAd->FragFrame.RxSize);
			} else
#endif /* HDR_TRANS_RX_SUPPORT */
			{
				pAd->FragFrame.RxSize = DataSize + HeaderRoom + RXWISize;
				NdisMoveMemory(pFragBuffer, pRxWI, RXWISize);
				NdisMoveMemory(pFragBuffer + RXWISize, FC, pAd->FragFrame.RxSize - RXWISize);
			}

			pAd->FragFrame.Sequence = pRxBlk->SN;
			pAd->FragFrame.LastFrag = pRxBlk->FN;	   /* Should be 0*/
#ifdef HDR_TRANS_RX_SUPPORT

			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
				pAd->FragFrame.Header_802_3 = TRUE;
			else
#endif /* HDR_TRANS_RX_SUPPORT */
				pAd->FragFrame.Header_802_3 = FALSE;

			ASSERT(pAd->FragFrame.LastFrag == 0);
			goto done;	/* end of processing this frame*/
		} else if (!pAd->FragFrame.pFragPacket)
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ERR: pAd->FragFrame.pFragPacket is NULL.\n"));
	} else {
		/*Middle & End of fragment*/
		if ((pRxBlk->SN != pAd->FragFrame.Sequence) ||
			(pRxBlk->FN != (pAd->FragFrame.LastFrag + 1))) {
			/* Fragment is not the same sequence or out of fragment number order*/
			/* Reset Fragment control blk*/
			if (pRxBlk->SN != pAd->FragFrame.Sequence)
				RESET_FRAGFRAME(pAd->FragFrame);

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Fragment is not the same sequence or out of fragment number order.\n"));
			goto done;
		}
		/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items. */
		/* else if ((pAd->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE) */
		else if ((pAd->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE + RXWISize) {
			/* Fragment frame is too large, it exeeds the maximum frame size.*/
			/* Reset Fragment control blk*/
			RESET_FRAGFRAME(pAd->FragFrame);
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Fragment frame is too large, it exeeds the maximum frame size.\n"));
			goto done;
		}

		/* Broadcom AP(BCM94704AGR) will send out LLC in fragment's packet, LLC only can accpet at first fragment.*/
		/* In this case, we will drop it.*/
		if (NdisEqualMemory(pData, SNAP_802_1H, sizeof(SNAP_802_1H))) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Find another LLC at Middle or End fragment(SN=%d, Frag=%d)\n", pRxBlk->SN, pRxBlk->FN));
			goto done;
		}

		pFragBuffer = GET_OS_PKT_DATAPTR(pAd->FragFrame.pFragPacket);
		/* concatenate this fragment into the re-assembly buffer*/
		NdisMoveMemory((pFragBuffer + pAd->FragFrame.RxSize), pData, DataSize);
		pAd->FragFrame.RxSize  += DataSize;
		pAd->FragFrame.LastFrag = pRxBlk->FN;	   /* Update fragment number*/

		/* Last fragment*/
		if (FC->MoreFrag == FALSE)
			bReassDone = TRUE;
	}

done:
	/* always release rx fragmented packet*/
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	pRxBlk->pRxPacket = NULL;

	/* return defragmented packet if packet is reassembled completely*/
	/* otherwise return NULL*/
	if (bReassDone) {
		PNDIS_PACKET pNewFragPacket;
		/* allocate a new packet buffer for fragment*/
		pNewFragPacket = RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);

		if (pNewFragPacket) {
			/* update RxBlk*/
			pRetPacket = pAd->FragFrame.pFragPacket;
			/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items. */
			/* pRxBlk->pHeader = (PHEADER_802_11) GET_OS_PKT_DATAPTR(pRetPacket); */
			/* pRxBlk->pData = (UCHAR *)pRxBlk->pHeader + HeaderRoom; */
			/* pRxBlk->DataSize = pAd->FragFrame.RxSize - HeaderRoom; */
			/* pRxBlk->pRxPacket = pRetPacket; */
			pRxBlk->pRxWI = (RXWI_STRUC *) GET_OS_PKT_DATAPTR(pRetPacket);
#ifdef HDR_TRANS_RX_SUPPORT

			if (pAd->FragFrame.Header_802_3) {
				pRxBlk->pData =	GET_OS_PKT_DATAPTR(pRetPacket) + RXWISize;
				pRxBlk->DataSize = pAd->FragFrame.RxSize - RXWISize;
			} else
#endif /* HDR_TRANS_RX_SUPPORT */
			{
				pRxBlk->pData =	GET_OS_PKT_DATAPTR(pRetPacket) + HeaderRoom + RXWISize;
				pRxBlk->DataSize = pAd->FragFrame.RxSize - HeaderRoom - RXWISize;
			}

			pRxBlk->pRxPacket = pRetPacket;
#ifdef HDR_TRANS_RX_SUPPORT

			if (pAd->FragFrame.Header_802_3) {
				SET_OS_PKT_DATAPTR(pRxBlk->pRxPacket, pRxBlk->pData);
				SET_OS_PKT_LEN(pRxBlk->pRxPacket, pRxBlk->DataSize);
				RX_BLK_SET_FLAG(pRxBlk, fRX_HDR_TRANS);
				pAd->FragFrame.Header_802_3 = FALSE;
			}

#endif /* HDR_TRANS_RX_SUPPORT */
			pAd->FragFrame.pFragPacket = pNewFragPacket;
		} else
			RESET_FRAGFRAME(pAd->FragFrame);
	}
}


VOID rx_eapol_frm_handle(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx)
{
	UCHAR *pTmpBuf;
	BOOLEAN to_mlme = TRUE, to_daemon = FALSE;
	struct wifi_dev *wdev;
	unsigned char hdr_len = LENGTH_802_11;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;

	if (pEntry->wdev->wdev_type == WDEV_TYPE_STA) {
		pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);
		ASSERT(pStaCfg);
	}
#endif
	ASSERT(wdev_idx < WDEV_NUM_MAX);

	if (wdev_idx >= WDEV_NUM_MAX)
		goto done;

	wdev = pAd->wdev_list[wdev_idx];

	if (pRxBlk->DataSize < (LENGTH_802_1_H + LENGTH_EAPOL_H)) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pkts size too small\n"));
		goto done;
	} else if (!RTMPEqualMemory(SNAP_802_1H, pRxBlk->pData, 6)) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("no SNAP_802_1H parameter\n"));
		goto done;
	} else if (!RTMPEqualMemory(EAPOL, pRxBlk->pData + 6, 2)) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("no EAPOL parameter\n"));
		goto done;
	} else if (*(pRxBlk->pData + 9) > EAPOLASFAlert) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknown EAP type(%d)\n", *(pRxBlk->pData + 9)));
		goto done;
	}

#ifdef A4_CONN
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS))
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

#ifdef CONFIG_AP_SUPPORT

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef RT_CFG80211_SUPPORT

	if (pEntry) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CFG80211 EAPOL indicate_802_11_pkt\n"));
#ifdef CONFIG_STA_SUPPORT

		/*
			Only decide to en-queue EAP-Request packet or not for Infra STA. @20140610
		*/
		if (pStaCfg && (pEntry->wdev == &pStaCfg->wdev)) {
			if (pStaCfg->wdev.bLinkUpDone)
				indicate_802_11_pkt(pAd, pRxBlk, wdev_idx);
			else {
				RX_BLK *pEapBlk = pStaCfg->wdev.pEapolPktFromAP;

				if (pEapBlk) {
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CFG80211(%s) bLinkUpDone = %d, Q this eapol packet\n",
							 __func__, pStaCfg->wdev.bLinkUpDone));
					pStaCfg->wdev.bGotEapolPkt = TRUE;
					NdisZeroMemory(pEapBlk, sizeof(RX_BLK));
					NdisMoveMemory(pEapBlk, pRxBlk, sizeof(RX_BLK));
					pEapBlk->pData = pRxBlk->pData;
					pEapBlk->FC = pRxBlk->FC;
					pEapBlk->pRxInfo = pRxBlk->pRxInfo;
					pEapBlk->pRxPacket = pRxBlk->pRxPacket;
					pEapBlk->pRxWI = pRxBlk->pRxWI;
					pEapBlk->rmac_info = pRxBlk->rmac_info;
#ifdef HDR_TRANS_SUPPORT
					pEapBlk->pTransData = pRxBlk->pTransData;
#endif /* HDR_TRANS_SUPPORT */
				} else
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211 pEapolPktFromAP is NULL\n", __func__));
			}
		} else
#endif /* CONFIG_STA_SUPPORT */
			indicate_802_11_pkt(pAd, pRxBlk, wdev_idx);

		return;
	}

#endif /*RT_CFG80211_SUPPORT*/

	if (pEntry && IS_ENTRY_PEER_AP(pEntry)) {
		{
			to_mlme = TRUE;
			to_daemon = FALSE;
		}
	}

#ifdef CONFIG_AP_SUPPORT

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
#ifdef DOT1X_SUPPORT

		/* sent this frame to upper layer TCPIP */
		if ((pEntry->SecConfig.Handshake.WpaState < AS_INITPMK) &&
			(IS_AKM_WPA1(pEntry->SecConfig.AKMMap) ||
			 (IS_AKM_WPA2(pEntry->SecConfig.AKMMap) && (!is_pmkid_cache_in_sec_config(&pEntry->SecConfig))) ||
			 (IS_AKM_WPA3_192BIT(pEntry->SecConfig.AKMMap) && (!is_pmkid_cache_in_sec_config(&pEntry->SecConfig))) ||
			 IS_IEEE8021X(&pEntry->SecConfig))) {
			to_daemon = TRUE;
			to_mlme = FALSE;
#ifdef WSC_AP_SUPPORT

			/* report EAP packets to MLME to check this packet is WPS packet or not */
			if ((pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.WscConfMode != WSC_DISABLE) &&
				(!MAC_ADDR_EQUAL(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.EntryAddr, ZERO_MAC_ADDR))) {
				to_mlme = TRUE;
				pTmpBuf = pRxBlk->pData - hdr_len;
				/* TODO: shiang-usw, why we need to change pHeader here?? */
				pRxBlk->FC = pTmpBuf;
			}

#endif /* WSC_AP_SUPPORT */
		} else
#endif /* DOT1X_SUPPORT */
		{
			/* sent this frame to WPA state machine */

			/*
				Check Addr3 (DA) is AP or not.
				If Addr3 is AP, forward this EAP packets to MLME
				If Addr3 is NOT AP, forward this EAP packets to upper layer or STA.
			*/
			if (wdev->wdev_type == WDEV_TYPE_AP || wdev->wdev_type == WDEV_TYPE_GO) {
				ASSERT(wdev->func_idx < HW_BEACON_MAX_NUM);

				if (wdev->func_idx < HW_BEACON_MAX_NUM)
					ASSERT(wdev == (&pAd->ApCfg.MBSSID[wdev->func_idx].wdev));
			}

			/* TODO: shiang-usw, why we check this here?? */
			if ((wdev->wdev_type == WDEV_TYPE_AP || wdev->wdev_type == WDEV_TYPE_GO) &&
				(NdisEqualMemory(pRxBlk->Addr3, pAd->ApCfg.MBSSID[wdev->func_idx].wdev.bssid, MAC_ADDR_LEN) == FALSE))
				to_daemon = TRUE;
			else
				to_mlme = TRUE;
		}
	}

#endif /* CONFIG_AP_SUPPORT */

	/*
	   Special DATA frame that has to pass to MLME
	   1. Cisco Aironet frames for CCX2. We need pass it to MLME for special process
	   2. EAPOL handshaking frames when driver supplicant enabled, pass to MLME for special process
	 */
	if (to_mlme) {
		pTmpBuf = pRxBlk->pData - hdr_len;
		NdisMoveMemory(pTmpBuf, pRxBlk->FC, hdr_len);
		{
			RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)pRxBlk->rmac_info;
			REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
						  pTmpBuf,
						  pRxBlk->DataSize + hdr_len,
						  pRxBlk->rx_signal.raw_rssi[0],
						  pRxBlk->rx_signal.raw_rssi[1],
						  pRxBlk->rx_signal.raw_rssi[2],
						  pRxBlk->rx_signal.raw_rssi[3],
						  0,
						  (rxd_base != NULL) ? rxd_base->RxD1.ChFreq : 0,
						  pRxBlk->OpMode,
						  wdev,
						  pRxBlk->rx_rate.field.MODE);
		}
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("!!! report EAPOL DATA to MLME (len=%d) !!!\n",
				  pRxBlk->DataSize));
	}

	if (to_daemon == TRUE) {
		indicate_802_11_pkt(pAd, pRxBlk, wdev_idx);
		return;
	}

done:
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	return;
}

VOID indicate_eapol_pkt(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (pAd == NULL) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("indicate_eapol_pkt: invalid pAd.\n"));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	if (!VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("indicate_eapol_pkt: invalid wcid.\n"));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	if (check_rx_pkt_pn_allowed(pAd, pRxBlk) == FALSE) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s:drop packet by PN mismatch!\n", __FUNCTION__));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("indicate_eapol_pkt: drop and release the invalid packet.\n"));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	rx_eapol_frm_handle(pAd, pEntry, pRxBlk, wdev_idx);
	return;
}

VOID indicate_rx_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU) && (!(IS_ASIC_CAP(pAd, fASIC_CAP_HW_DAMSDU))))
		indicate_amsdu_pkt(pAd, pRxBlk, wdev_idx);
	else if (RX_BLK_TEST_FLAG(pRxBlk, fRX_EAP))
		indicate_eapol_pkt(pAd, pRxBlk, wdev_idx);
	else if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
		indicate_802_3_pkt(pAd, pRxBlk, wdev_idx);
	else
		indicate_802_11_pkt(pAd, pRxBlk, wdev_idx);
}

static MAC_TABLE_ENTRY *research_entry(RTMP_ADAPTER *pAd, RX_BLK *rx_blk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	pEntry = MacTableLookup(pAd, rx_blk->Addr2);
#endif

#ifdef MAC_REPEATER_SUPPORT

	if (pEntry) {
		if ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
			(IS_ENTRY_REPEATER(pEntry) || IS_ENTRY_PEER_AP(pEntry))) {
			REPEATER_CLIENT_ENTRY	*pReptEntry = NULL;

			pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, &rx_blk->Addr1[0], TRUE);

			if (pReptEntry) {
				if (VALID_WCID(pReptEntry->MacTabWCID)) {
					rx_blk->wcid = pReptEntry->MacTabWCID;
					pEntry = &pAd->MacTab.Content[rx_blk->wcid];
				}
			}
		}
	}

#endif

#ifdef CONFIG_STA_SUPPORT
	/* TODO: broadcast frame match 1st network interface only */
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	pEntry = MacTableLookup2(pAd, &rx_blk->Addr2[0], NULL);
#endif

	if (pEntry)
		rx_blk->wcid = pEntry->wcid;

	return pEntry;
}

VOID dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
#ifdef CONFIG_AP_SUPPORT
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
#endif
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef RT_CFG80211_SUPPORT
	INT op_mode = pRxBlk->OpMode;
#endif
	MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("-->%s()\n", __func__));

	if (pRxBlk->DataSize > MAX_MGMT_PKT_LEN) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DataSize=%d\n", pRxBlk->DataSize));
		hex_dump("MGMT ???", pRxBlk->FC, pRxBlk->DataSize);
		goto done;
	}

#ifdef RT_CFG80211_SUPPORT
#ifdef CFG_TDLS_SUPPORT

	if (CFG80211_HandleTdlsDiscoverRespFrame(pAd, pRxBlk, op_mode))
		goto done;

#endif /* CFG_TDLS_SUPPORT */

	if (CFG80211_HandleP2pMgmtFrame(pAd, pRxBlk, op_mode))
		goto done;

#endif /* RT_CFG80211_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT
	if (PMF_PerformRxFrameAction(pAd, pRxBlk) == FALSE)
		goto done;
#endif /* DOT11W_PMF_SUPPORT */

	if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
		pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	else
		pEntry = research_entry(pAd, pRxBlk);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (!ap_dev_rx_mgmt_frm(pAd, pRxBlk, pEntry))
			goto done;

		/* BssInfo not ready, drop frame */
		if ((pEntry) && (pEntry->wdev) && ((FC->SubType == SUBTYPE_AUTH) || (FC->SubType == SUBTYPE_ASSOC_REQ) ||
						 (FC->SubType == SUBTYPE_DEAUTH) || (FC->SubType == SUBTYPE_DISASSOC))) {
			if (WDEV_BSS_STATE(pEntry->wdev) != BSS_READY && pEntry->wdev->wdev_type != WDEV_TYPE_STA) {
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR: BSS idx (%d) wdev %s state %d not ready. (subtype %d)\n",
						 pEntry->wdev->bss_info_argument.ucBssIndex,
						 pEntry->wdev->if_dev->name,
						 WDEV_BSS_STATE(pEntry->wdev),
						 FC->SubType));
				goto done;
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (!sta_dev_rx_mgmt_frm(pAd, pRxBlk, pEntry))
			goto done;
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef WIFI_DIAG
	DiagDevRxMgmtFrm(pAd, pRxBlk);
#endif
done:
	MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s()\n", __func__));

	if (pRxPacket)
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
}


VOID dev_rx_ctrl_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;

	switch (FC->SubType) {
#ifdef DOT11_N_SUPPORT

	case SUBTYPE_BLOCK_ACK_REQ: {
		FRAME_BA_REQ *bar = (FRAME_BA_REQ *)FC;
#ifdef MT_MAC

		if ((IS_HIF_TYPE(pAd, HIF_MT)) &&
			(pRxBlk->wcid == RESERVED_WCID)) {
#ifdef MAC_REPEATER_SUPPORT
			REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */

			MAC_TABLE_ENTRY *pEntry;

			pEntry = MacTableLookup(pAd, &pRxBlk->Addr2[0]);

			if (pEntry) {
				pRxBlk->wcid = pEntry->wcid;
#ifdef MAC_REPEATER_SUPPORT

				if ((pAd->ApCfg.bMACRepeaterEn == TRUE) && (IS_ENTRY_REPEATER(pEntry) || IS_ENTRY_PEER_AP(pEntry))) {
					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, &pRxBlk->Addr1[0], TRUE);

					if (pReptEntry && (pReptEntry->CliValid == TRUE)) {
						pEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
						pRxBlk->wcid = pEntry->wcid;
					} else if ((pReptEntry == NULL) && IS_ENTRY_PEER_AP(pEntry)) {/* this packet is for APCLI */
						INT apcli_idx = pEntry->func_tb_idx;

						pEntry = &pAd->MacTab.Content[pAd->StaCfg[apcli_idx].MacTabWCID];
						pRxBlk->wcid = pEntry->wcid;
					} else {
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WCID of BAR packet!. A1:%02x:%02x:%02x:%02x:%02x:%02x,A2:%02x:%02x:%02x:%02x:%02x:%02x\n\r", __func__,
								 PRINT_MAC(pRxBlk->Addr1), PRINT_MAC(pRxBlk->Addr2)));
						break;
					}

					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():%02x:%02x:%02x:%02x:%02x:%02x recv BAR\n\r", __func__,
							 PRINT_MAC(pRxBlk->Addr1)));
				}

#endif
			} else {
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Cannot found WCID of BAR packet!\n",
						 __func__));
			}
		}

#endif /* MT_MAC */
		bar_process(pAd, pRxBlk->wcid, (pRxBlk->MPDUtotalByteCnt),
					(PFRAME_BA_REQ)FC);

		if (bar->BARControl.Compressed == 0) {
			UCHAR tid = bar->BARControl.TID;

			ba_rec_session_tear_down(pAd, pRxBlk->wcid, tid, FALSE);
		}
	}
	break;
#endif /* DOT11_N_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	case SUBTYPE_PS_POLL:
		/*
			    This marco is not suitable for P2P GO.
			    It is OK to remove this marco here. @20140728
					IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		*/
	{
		USHORT Aid = pRxBlk->Duration & 0x3fff;
		PUCHAR pAddr = pRxBlk->Addr2;
		MAC_TABLE_ENTRY *pEntry;
#ifdef MT_MAC

		if ((IS_HIF_TYPE(pAd, HIF_MT)) &&
			(pRxBlk->wcid == RESERVED_WCID)) {

			UCHAR wdev_idx;
			struct wifi_dev *wdev;
			struct wifi_dev_ops *ops;

			wdev_idx = RTMP_GET_PACKET_WDEV(pRxPacket);
			wdev = pAd->wdev_list[wdev_idx];
			ops = wdev->wdev_ops;
			ops->mac_entry_lookup(pAd, &pRxBlk->Addr2[0], wdev, &pEntry);

			if (pEntry)
				pRxBlk->wcid = pEntry->wcid;
			else {
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Cannot found WCID of PS-Poll packet!\n",
						 __func__));
			}
		}

#endif /* MT_MAC */

		/* printk("dev_rx_ctrl_frm0 SUBTYPE_PS_POLL pRxBlk->wcid: %x pEntry->wcid:%x\n",pRxBlk->wcid,pEntry->wcid); */
		if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
			/* printk("dev_rx_ctrl_frm1 SUBTYPE_PS_POLL\n"); */
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

			if (pEntry->Aid == Aid)
				RtmpHandleRxPsPoll(pAd, pAddr, pRxBlk->wcid, FALSE);
			else {
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Aid mismatch(pkt:%d, Entry:%d)!\n",
						 __func__, Aid, pEntry->Aid));
			}
		}
	}
	break;
#endif /* CONFIG_AP_SUPPORT */
#ifdef WFA_VHT_PF

	case SUBTYPE_RTS:
		if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
			PLCP_SERVICE_FIELD *srv_field;
			RTS_FRAME *rts = (RTS_FRAME *)pRxBlk->FC;

			if ((rts->Addr1[0] & 0x1) == 0x1) {
				srv_field = (PLCP_SERVICE_FIELD *)&pRxBlk->pRxWI->RXWI_N.bbp_rxinfo[15];

				if (srv_field->dyn_bw == 1) {
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x, WCID:%d, DYN,BW=%d\n",
							 PRINT_MAC(rts->Addr1), pRxBlk->wcid, srv_field->cbw_in_non_ht));
				}
			}
		}

		break;

	case SUBTYPE_CTS:
		break;
#endif /* WFA_VHT_PF */
#ifdef DOT11_N_SUPPORT

	case SUBTYPE_BLOCK_ACK:
		/* +++Add by shiang for debug */
		/* TODO: shiang-MT7603, remove this! */
	{
		UCHAR *ptr, *ra, *ta;
		BA_CONTROL *ba_ctrl;

#ifdef SNIFFER_SUPPORT
		if (!MONITOR_ON(pAd))
#endif /* SNIFFER_SUPPORT */
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s():BlockAck From WCID:%d\n", __func__, pRxBlk->wcid));

		ptr = (UCHAR *)pRxBlk->FC;
		ptr += 4;
		ra = ptr;
		ptr += 6;
		ta = ptr;
		ptr += 6;
		ba_ctrl = (BA_CONTROL *)ptr;
		ptr += sizeof(BA_CONTROL);

#ifdef SNIFFER_SUPPORT
		if (!MONITOR_ON(pAd))
#endif /* SNIFFER_SUPPORT */
		{
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\tRA=%02x:%02x:%02x:%02x:%02x:%02x, TA=%02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(ra), PRINT_MAC(ta)));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\tBA Control: AckPolicy=%d, MTID=%d, Compressed=%d, TID_INFO=0x%x\n",
					ba_ctrl->ACKPolicy, ba_ctrl->MTID, ba_ctrl->Compressed, ba_ctrl->TID));
		}

		if (ba_ctrl->ACKPolicy == 0 && ba_ctrl->Compressed == 1) {
			BASEQ_CONTROL *ba_seq;

			ba_seq = (BASEQ_CONTROL *)ptr;

#ifdef SNIFFER_SUPPORT
			if (!MONITOR_ON(pAd))
#endif /* SNIFFER_SUPPORT */
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("\tBA StartingSeqCtrl:StartSeq=%d, FragNum=%d\n",
						ba_seq->field.StartSeq, ba_seq->field.FragNum));

			ptr += sizeof(BASEQ_CONTROL);

#ifdef SNIFFER_SUPPORT
			if (!MONITOR_ON(pAd))
#endif /* SNIFFER_SUPPORT */
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("\tBA Bitmap:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
						*ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3),
						*(ptr + 4), *(ptr + 5), *(ptr + 6), *(ptr + 7)));

		}
	}

		/* ---Add by shiang for debug */
#endif /* DOT11_N_SUPPORT */

	case SUBTYPE_ACK:
	default:
		break;
	}

#ifdef WIFI_DIAG
	DiagDevRxCtrlFrm(pAd, pRxBlk);
#endif

	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
}

VOID rtmp_handle_mic_error_event(
	RTMP_ADAPTER *ad,
	RX_BLK *rx_blk)
{
	MAC_TABLE_ENTRY *entry;
	RXINFO_STRUC *rx_info = rx_blk->pRxInfo;

	entry = MacTableLookup(ad, rx_blk->Addr2);

	if (!entry || RX_BLK_TEST_FLAG(rx_blk, fRX_WCID_MISMATCH))
		return;

	if ((rx_info->U2M) && VALID_UCAST_ENTRY_WCID(ad, rx_blk->wcid) && (IS_CIPHER_TKIP_Entry(entry))) {
		/* unicast case */
		if (IS_ENTRY_PEER_AP(entry) || IS_ENTRY_REPEATER(entry)) {
			/* sta rx mic error */
#ifdef CONFIG_STA_SUPPORT
			sta_handle_mic_error_event(ad, entry, rx_blk);
#endif /* CONFIG_STA_SUPPORT */
		} else {
			/* ap rx mic error */
#ifdef CONFIG_AP_SUPPORT
			ap_handle_mic_error_event(ad, entry, rx_blk);
#endif /* CONFIG_AP_SUPPORT */
		}
	} else if (rx_info->Mcast || rx_info->Bcast) {
		/* bmc case */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rx bc/mc Cipher Err(MPDUsize=%d, WCID=%d, CipherErr=%d)\n",
				 rx_blk->MPDUtotalByteCnt, rx_blk->wcid, rx_info->CipherErr));

		if (IS_ENTRY_PEER_AP(entry) || IS_ENTRY_REPEATER(entry)) {
#ifdef CONFIG_STA_SUPPORT
			sta_handle_mic_error_event(ad, entry, rx_blk);

#endif /* CONFIG_STA_SUPPORT */
		}
	}
}

/*
	========================================================================
	Routine Description:
		Check Rx descriptor, return NDIS_STATUS_FAILURE if any error found
	========================================================================
*/
static INT rtmp_chk_rx_err(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo;
	FRAME_CONTROL *FC;
#if defined(OUI_CHECK_SUPPORT) || defined(HTC_DECRYPT_IOT) || defined(MWDS)
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif /* defined (OUI_CHECK_SUPPORT) || defined (HTC_DECRYPT_IOT)  || defined (MWDS) */
	int LogDbgLvl = DBG_LVL_WARN;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	if ((pRxBlk == NULL) || (pRxBlk->pRxInfo == NULL)) {
		/* +++Add by shiang for debug */
		if (pRxBlk == NULL)
			printk("%s(): pRxBlk is NULL\n", __func__);
		else
			printk("%s(): pRxBlk->pRxInfo is NULL\n", __func__);

		/* ---Add by shiang for debug */
		return NDIS_STATUS_FAILURE;
	}

	FC = (FRAME_CONTROL *)pRxBlk->FC;
	pRxInfo = pRxBlk->pRxInfo;


#ifdef MT_MAC

	/* TODO: shiang-MT7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		/* +++Add by shiang for work-around, should remove it once we correctly configure the BSSID! */
		/* TODO: shiang-MT7603 work around!! */
		RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)pRxBlk->rmac_info;

		if (rxd_base->RxD2.IcvErr) {
#ifdef WIFI_DIAG
			/* WEP + open, wrong passowrd can association success, but rx data error */
			if ((pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE) && (pRxBlk->pRxInfo->U2M)) {
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
				if (pEntry && IS_ENTRY_CLIENT(pEntry) && (IS_CIPHER_WEP(pEntry->SecConfig.PairwiseCipher)))
					DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_AUTH_FAIL, 0);
			}
#endif
#ifdef CONN_FAIL_EVENT
			if ((pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE) && (pRxBlk->pRxInfo->U2M)) {
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
				if (pEntry && IS_ENTRY_CLIENT(pEntry) && (IS_CIPHER_WEP(pEntry->SecConfig.PairwiseCipher)))
					ApSendConnFailMsg(pAd, pEntry->func_tb_idx, pEntry->Addr, REASON_MIC_FAILURE);
			}
#endif
#ifdef OUI_CHECK_SUPPORT
			UCHAR band_idx = (rxd_base->RxD1.ChFreq > 14) ? 1 : 0;
			if (band_idx >= DBDC_BAND_NUM)
				return NDIS_STATUS_FAILURE;

			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

			if ((pAd->MacTab.oui_mgroup_cnt > 0)
#ifdef MAC_REPEATER_SUPPORT
				|| (pAd->ApCfg.RepeaterCliSize != 0)
#endif
			   ) {
				if (NdisCmpMemory(pEntry->Addr, pRxBlk->Addr2, MAC_ADDR_LEN)) {
					INC_COUNTER64(pAd->WlanCounters[band_idx].RxHWLookupWcidErrCount);

					if (DebugLevel < DBG_LVL_TRACE)
						return NDIS_STATUS_FAILURE;
				}

#ifdef MAC_REPEATER_SUPPORT

				if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
					BOOLEAN bwcid_error = FALSE;

					if (IS_ENTRY_REPEATER(pEntry)) {
						UCHAR	orig_wcid = pRxBlk->wcid;
						REPEATER_CLIENT_ENTRY	*pReptEntry = NULL;

						pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, &pRxBlk->Addr1[0], TRUE);

						if (pReptEntry && (pReptEntry->CliValid == TRUE)) {
							if (orig_wcid != pReptEntry->MacTabWCID)
								bwcid_error = TRUE;
						} else
							bwcid_error = TRUE;
					} else if (IS_ENTRY_PEER_AP(pEntry)) {
						INT apcli_idx = pEntry->func_tb_idx;

						if (NdisCmpMemory(pAd->StaCfg[apcli_idx].wdev.if_addr, pRxBlk->Addr1, MAC_ADDR_LEN))
							bwcid_error = TRUE;
					}

					if (bwcid_error) {
							INC_COUNTER64(pAd->WlanCounters[band_idx].RxHWLookupWcidErrCount);

						if (DebugLevel < DBG_LVL_TRACE)
							return NDIS_STATUS_FAILURE;
					}
				}

#endif /* MAC_REPEATER_SUPPORT */
			}

#endif

#ifdef A4_CONN
			if (IS_BM_MAC_ADDR(pRxBlk->Addr1)) {
				pEntry = MacTableLookup(pAd, pRxBlk->Addr2);

				if (pEntry && IS_ENTRY_PEER_AP(pEntry) && IS_ENTRY_A4(pEntry))
					return NDIS_STATUS_FAILURE;
			}
#endif /* A4_CONN */


#ifdef HTC_DECRYPT_IOT

			if (rxd_base->RxD1.HTC == 1) {
				/* if ((rxd_base->RxD2.SecMode != 0)) */
				{
					if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
						pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

					if (pEntry && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))) {
						/* Rx HTC and FAIL decryp case! */
						/* if (rxd_base->RxD2.IcvErr == 1) */
						if ((pEntry->HTC_AAD_OM_CountDown == 0) &&
							(pEntry->HTC_AAD_OM_Freeze == 0)) {
							if (pEntry->HTC_ICVErrCnt++ > pAd->HTC_ICV_Err_TH) {
								pEntry->HTC_ICVErrCnt = 0; /* reset the history */
								pEntry->HTC_AAD_OM_CountDown = 3;

								if (pEntry->HTC_AAD_OM_Force == 0) {
									pEntry->HTC_AAD_OM_Force = 1;
									MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("@AAD_OM Trigger ! wcid=%u\n", pRxBlk->wcid));
									HW_SET_ASIC_WCID_AAD_OM(pAd, pRxBlk->wcid, 1);
								}
							}
						}
					}

					if (pEntry->HTC_AAD_OM_Freeze) {
						/*
							Wokradound if the ICV Error happened again!
						*/
						if (pEntry->HTC_AAD_OM_Force) {
							HW_SET_ASIC_WCID_AAD_OM(pAd, pRxBlk->wcid, 1);
							MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, ("@ICV Error, HTC_AAD_OM_Force already 1, wcid=%u", pRxBlk->wcid));
						} else
							MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, ("ICV Error"));

						if (pRxBlk->Addr1 != NULL)
							MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
									 (", Addr1 = %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(pRxBlk->Addr1)));

						if (pRxBlk->Addr2 != NULL)
							MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
									 (", Addr2 = %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(pRxBlk->Addr2)));

						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, ("\n"));
						dump_rmac_info_for_ICVERR(pAd, pRxBlk->rmac_info);
					} else
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("AAD_OM detection in progress!\n"));
				}
			} else
#endif /* HTC_DECRYPT_IOT */
			{
#if defined(TXRX_STAT_SUPPORT) || defined(CUSTOMER_DCC_FEATURE)
				{
					if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
#ifdef TXRX_STAT_SUPPORT
						struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
						UCHAR bandidx = HcGetBandByWdev(pEntry->wdev);
#endif
						MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
#ifdef TXRX_STAT_SUPPORT
						if ((pEntry != NULL) && IS_ENTRY_CLIENT(pEntry)) {
							INC_COUNTER64(pEntry->RxDecryptionErrorCount);
							INC_COUNTER64(pEntry->pMbss->stat_bss.RxDecryptionErrorCount);
							INC_COUNTER64(ctrl->rdev[bandidx].pRadioCtrl->RxDecryptionErrorCount);
							}
#endif
#ifdef CUSTOMER_DCC_FEATURE
							if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
							pEntry->RxErrorCount++;
#endif
					}
				}
#endif

				/* If receive ICV error packet, add counter and show in tpinfo */
				tr_ctl->rx_icv_err_cnt++;
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, ("ICV Error"));

				if (pRxBlk->Addr1 != NULL)
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
							 (", Addr1 = %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(pRxBlk->Addr1)));

				if (pRxBlk->Addr2 != NULL)
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
							 (", Addr2 = %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(pRxBlk->Addr2)));

				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, ("\n"));
				dump_rmac_info_for_ICVERR(pAd, pRxBlk->rmac_info);
			}

			if (DebugLevel >= DBG_LVL_TRACE)
				dump_rxblk(pAd, pRxBlk);

			return NDIS_STATUS_FAILURE;
		}

		if (rxd_base->RxD2.CipherLenMis) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("CM Length Error\n, WlanIndex = %d\n", rxd_base->RxD2.RxDWlanIdx));
			return NDIS_STATUS_FAILURE;
		}

		if (pRxBlk->DeAmsduFail) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("Deammsdu Fail\n, WlanIndex = %d\n", rxd_base->RxD2.RxDWlanIdx));
			return NDIS_STATUS_FAILURE;
		}

		if (rxd_base->RxD2.TkipMicErr) {
#ifdef OUI_CHECK_SUPPORT
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

			if (pAd->MacTab.oui_mgroup_cnt > 0) {
				if (NdisCmpMemory(pEntry->Addr, pRxBlk->Addr2, MAC_ADDR_LEN))
					RX_BLK_SET_FLAG(pRxBlk, fRX_WCID_MISMATCH);
			}

#endif
#if defined(TXRX_STAT_SUPPORT) || defined(CUSTOMER_DCC_FEATURE)
			{
				if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
#ifdef TXRX_STAT_SUPPORT
					struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
					UCHAR bandidx = HcGetBandByWdev(pEntry->wdev);
#endif
					MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
#ifdef TXRX_STAT_SUPPORT
					if ((pEntry != NULL) && IS_ENTRY_CLIENT(pEntry)) {
						INC_COUNTER64(pEntry->RxMICErrorCount);
						INC_COUNTER64(pEntry->pMbss->stat_bss.RxMICErrorCount);
						INC_COUNTER64(ctrl->rdev[bandidx].pRadioCtrl->RxMICErrorCount);
					}
#endif
#ifdef CUSTOMER_DCC_FEATURE
					if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
						pEntry->RxErrorCount++;
#endif
				}
			}
#endif

			if (!(RX_BLK_TEST_FLAG(pRxBlk, fRX_WCID_MISMATCH)))
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("TKIP MIC Error\n, WlanIndex = %d\n", rxd_base->RxD2.RxDWlanIdx));

#ifdef CONFIG_STA_SUPPORT
			rtmp_handle_mic_error_event(pAd, pRxBlk);
			return NDIS_STATUS_FAILURE;
#endif /* CONFIG_STA_SUPPORT */
		}

		pRxBlk->CipherMis = rxd_base->RxD2.CipherMis;
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pRxBlk->pRxInfo->MyBss = 1;
#endif /* CONFIG_STA_SUPPORT */
#ifdef HTC_DECRYPT_IOT

		if (rxd_base->RxD1.HTC == 1) { /* focus HTC pkt only! */
			if (rxd_base->RxD2.SecMode != 0) {
				if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
					pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

				if (pEntry && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))) {
					if ((pEntry->HTC_AAD_OM_CountDown == 0) &&
						(pEntry->HTC_AAD_OM_Freeze == 0)) {
						/* Rx decrypt OK  of HTC */
						if (rxd_base->RxD2.IcvErr == 0) {
							if ((rxd_base->RxD2.CipherMis == 0) &&
								(rxd_base->RxD2.CipherLenMis == 0) &&
								(rxd_base->RxD2.TkipMicErr == 0)) {
								pEntry->HTC_ICVErrCnt = 0; /* reset counter! */
								pEntry->HTC_AAD_OM_Freeze = 1; /* decode ok, we treat the entry don't need to count pEntry->HTC_ICVErrCnt any more! */
							}
						}
					}
				}
			}
		}

#endif /* HTC_DECRYPT_IOT */
	}

#endif /* MT_MAC */

	/* Phy errors & CRC errors*/
	if (pRxInfo->Crc) {
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			INT dBm = (pRxBlk->rx_signal.raw_rssi[0]) - pAd->BbpRssiToDbmDelta;

			/* Check RSSI for Noise Hist statistic collection.*/
			if (dBm <= -87)
				pAd->StaCfg[0].RPIDensity[0] += 1;
			else if (dBm <= -82)
				pAd->StaCfg[0].RPIDensity[1] += 1;
			else if (dBm <= -77)
				pAd->StaCfg[0].RPIDensity[2] += 1;
			else if (dBm <= -72)
				pAd->StaCfg[0].RPIDensity[3] += 1;
			else if (dBm <= -67)
				pAd->StaCfg[0].RPIDensity[4] += 1;
			else if (dBm <= -62)
				pAd->StaCfg[0].RPIDensity[5] += 1;
			else if (dBm <= -57)
				pAd->StaCfg[0].RPIDensity[6] += 1;
			else if (dBm > -57)
				pAd->StaCfg[0].RPIDensity[7] += 1;
		}
#endif /* CONFIG_STA_SUPPORT */
		return NDIS_STATUS_FAILURE;
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		/* Drop ToDs promiscous frame, it is opened due to CCX 2 channel load statistics*/
		if ((pRxBlk->DataSize < 14) || (pRxBlk->MPDUtotalByteCnt > MAX_AGGREGATION_SIZE)) {
			/*
				min_len: CTS/ACK frame len = 10, but usually we filter it in HW,
						so here we drop packet with length < 14 Bytes.

				max_len:  Paul 04-03 for OFDM Rx length issue
			*/
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("rx pkt len err(%d, %d)\n",
					 pRxBlk->DataSize, pRxBlk->MPDUtotalByteCnt));
			return NDIS_STATUS_FAILURE;
		}

		if (pRxBlk->FC) {
#ifndef CLIENT_WDS

			if (FC->ToDs
			   ) {
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Line(%d) (ToDs Packet not allow in STA Mode)\n", __func__, __LINE__));
				return NDIS_STATUS_FAILURE;
			}

#endif /* !CLIENT_WDS */
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	/* drop decyption fail frame*/
	if (pRxInfo->Decrypted && pRxInfo->CipherErr) {
		if (pRxInfo->CipherErr == 2)
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RxErr: ICV ok but MICErr"));
		else if (pRxInfo->CipherErr == 1)
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RxErr: ICV Err"));
		else if (pRxInfo->CipherErr == 3)
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RxErr: Key not valid"));
		else
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RxErr: CipherErr 0x%x", pRxInfo->CipherErr));

		INC_COUNTER64(pAd->WlanCounters[0].WEPUndecryptableCount);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/*
				WCID equal to 255 mean MAC couldn't find any matched entry in Asic-MAC table.
				The incoming packet mays come from WDS or AP-Client link.
				We need them for further process. Can't drop the packet here.
			*/
			if ((pRxInfo->U2M) && (pRxBlk->wcid == 255)
#ifdef WDS_SUPPORT
				&& (pAd->WdsTab.Mode == WDS_LAZY_MODE)
#endif /* WDS_SUPPORT */
			   )
				return NDIS_STATUS_SUCCESS;

			/* Increase received error packet counter per BSS */
			if (FC->FrDs == 0 &&
				pRxInfo->U2M &&
				pRxBlk->bss_idx < pAd->ApCfg.BssidNum) {
				BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pRxBlk->bss_idx];

				pMbss->RxDropCount++;
				pMbss->RxErrorCount++;
			}
#if defined(TXRX_STAT_SUPPORT) || defined(CUSTOMER_DCC_FEATURE)
			if (FC->FrDs == 0 && pRxInfo->U2M && (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))) {
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
#ifdef TXRX_STAT_SUPPORT
				if ((pEntry != NULL) && IS_ENTRY_CLIENT(pEntry)) {
					INC_COUNTER64(pEntry->pMbss->stat_bss.RxPacketDroppedCount);
					}
#endif
#ifdef CUSTOMER_DCC_FEATURE
					if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
						pEntry->RxDropCount++;
					}
#endif

			}
#endif

#ifdef WDS_SUPPORT
#ifdef STATS_COUNT_SUPPORT

			if ((FC->FrDs == 1) && (FC->ToDs == 1) &&
				(VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))) {
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

				if (IS_ENTRY_WDS(pEntry) && (pEntry->func_tb_idx < MAX_WDS_ENTRY))
					pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].WdsCounter.RxErrorCount++;
			}

#endif /* STATS_COUNT_SUPPORT */
#endif /* WDS_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (INFRA_ON(&pAd->StaCfg[0]) && pRxInfo->MyBss) {
				if ((pRxInfo->CipherErr & 1) == 1) {
					RTMPSendWirelessEvent(pAd, IW_ICV_ERROR_EVENT_FLAG,
										  pRxBlk->Addr2,
										  BSS0, 0);
				}
			}
		}
#endif /* CONFIG_STA_SUPPORT */
		rtmp_handle_mic_error_event(pAd, pRxBlk);
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): %d (len=%d, Mcast=%d, MyBss=%d, Wcid=%d, KeyId=%d)\n",
				 __func__, pRxInfo->CipherErr, pRxBlk->MPDUtotalByteCnt,
				 pRxInfo->Mcast | pRxInfo->Bcast, pRxInfo->MyBss, pRxBlk->wcid,
				 pRxBlk->key_idx));
#ifdef DBG
		dump_rxinfo(pAd, pRxInfo);
		dump_rmac_info(pAd, (UCHAR *)pRxBlk->pRxWI);
		hex_dump("ErrorPkt",  (UCHAR *)pRxBlk->FC, pRxBlk->MPDUtotalByteCnt);
#endif /* DBG */

		if (pRxBlk->FC == NULL)
			return NDIS_STATUS_SUCCESS;

		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}


BOOLEAN dev_rx_no_foward(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	return TRUE;
}


#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
struct net_device *rlt_dev_get_by_name(const char *name)
{
#if (KERNEL_VERSION(2, 6, 24) <= LINUX_VERSION_CODE)
	return dev_get_by_name(&init_net, name);
#else
	return dev_get_by_name(name);
#endif
}

VOID ApCliLinkCoverRxPolicy(
	IN RTMP_ADAPTER * pAd,
	IN PNDIS_PACKET pPacket,
	OUT BOOLEAN * DropPacket)
{
#ifdef MAC_REPEATER_SUPPORT
	void *opp_band_tbl = NULL;
	void *band_tbl = NULL;
	void *other_band_tbl = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pInvalidEntry = NULL;
	REPEATER_CLIENT_ENTRY *pOtherBandReptEntry = NULL;
	REPEATER_CLIENT_ENTRY *pAnotherBandReptEntry = NULL;
	PNDIS_PACKET pRxPkt = pPacket;
	UCHAR *pPktHdr = NULL;

	pPktHdr = GET_OS_PKT_DATAPTR(pRxPkt);

	if (wf_drv_tbl.wf_fwd_feedback_map_table)
		wf_drv_tbl.wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl, &other_band_tbl);

	if ((opp_band_tbl == NULL) && (other_band_tbl == NULL))
		return;

	if (IS_GROUP_MAC(pPktHdr)) {
		pInvalidEntry = RepeaterInvaildMacLookup(pAd, pPktHdr + 6);

		if (opp_band_tbl != NULL)
			pOtherBandReptEntry = RTMPLookupRepeaterCliEntry(opp_band_tbl, FALSE, pPktHdr + 6, FALSE);

		if (other_band_tbl != NULL)
			pAnotherBandReptEntry = RTMPLookupRepeaterCliEntry(other_band_tbl, FALSE, pPktHdr + 6, FALSE);

		if ((pInvalidEntry != NULL) || (pOtherBandReptEntry != NULL) || (pAnotherBandReptEntry != NULL)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s, recv broadcast from InvalidRept Entry, drop this packet\n", __func__));
			*DropPacket = TRUE;
		}
	}

#endif /* MAC_REPEATER_SUPPORT */
}
#endif /* CONFIG_WIFI_PKT_FWD */

INT sta_rx_pkt_allow(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT hdr_len = LENGTH_802_11;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("-->%s():pRxBlk->wcid=%d\n", __func__, pRxBlk->wcid));

#ifdef AUTOMATION
	sta_rx_packet_check(pAd, pRxBlk);
#endif /* AUTOMATION */

	if (!pAd) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): pAd is null", __func__));
		return FALSE;
	}

	pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): pEntry is null", __func__));
		return FALSE;
	}

#ifdef CONFIG_STA_SUPPORT
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif /* CONFIG_STA_SUPPORT */


	if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1)) {
#if defined(CLIENT_WDS) || defined(APCLI_AS_WDS_STA_SUPPORT)
#ifdef APCLI_AS_WDS_STA_SUPPORT
		if (IS_ENTRY_PEER_AP(pEntry)) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
			hdr_len = LENGTH_802_11_WITH_ADDR4;
		}
#endif /* APCLI_AS_WDS_STA_SUPPORT */
#ifdef CLIENT_WDS
		if ((VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
			&& IS_ENTRY_CLIENT(pEntry))
#endif /* CLIENT_WDS */
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
			hdr_len = LENGTH_802_11_WITH_ADDR4;
#ifdef CLIENT_WDS
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
#endif /* CLIENT_WDS */
		}
#endif /* CLIENT_WDS || APCLI_AS_WDS_STA_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef A4_CONN
		if (IS_ENTRY_A4(pEntry)) {
			PSTA_ADMIN_CONFIG pApCliEntry = NULL;

			/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("sta_rx_pkt_allow: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x\n ApCli recvd a MWDS pkt\n", */
			/* wdev->wdev_idx,wdev->wdev_type,wdev->func_idx)); */

			if (pEntry->func_tb_idx < MAX_APCLI_NUM)
				pApCliEntry = &pAd->StaCfg[pEntry->func_tb_idx];
			if (pApCliEntry) {
				NdisGetSystemUpTime(&pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime);
				if (MAC_ADDR_EQUAL(pRxBlk->Addr4, pApCliEntry->wdev.if_addr)) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("ApCli receive a looping packet!\n"));
					return FALSE;
				}
			}
			if ((pAd->ApCfg.ApCliInfRunned == 1)) {
				BOOLEAN bTAMatchSA = MAC_ADDR_EQUAL(pEntry->Addr, pRxBlk->Addr4);

				/*
				For ApCli, we have to avoid to delete the bridge MAC(SA) and AP MAC(TA) is the same case.
				*/

				if (!bTAMatchSA) {
						MAC_TABLE_ENTRY *pMovedEntry = MacTableLookup(pAd, pRxBlk->Addr4);

						if (pMovedEntry
#ifdef AIR_MONITOR
							&& !IS_ENTRY_MONITOR(pMovedEntry)
#endif /* AIR_MONITOR */
							&& IS_ENTRY_CLIENT(pMovedEntry)
						) {

						/*
						It means this source entry has moved to another one and hidden behind it.
						So delete this source entry!
						*/
						/*
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("ApCli found a entry(%02X:%02X:%02X:%02X:%02X:%02X) who has moved to another side! Delete it from MAC table.\n",
						PRINT_MAC(pMovedEntry->Addr)));
						*/
#ifdef WH_EVENT_NOTIFIER
						{
							EventHdlr pEventHdlrHook = NULL;

							pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_LEAVE);
							if (pEventHdlrHook && pMovedEntry->wdev)
								pEventHdlrHook(pAd, pMovedEntry->wdev,
									pMovedEntry->Addr, pMovedEntry->wdev->channel);
						}
#endif /* WH_EVENT_NOTIFIER */
						mac_entry_delete(pAd, pMovedEntry);
					}
				}
			}
		}
#endif /* A4_CONN */
#endif /* APCLI_SUPPORT */

		RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
		hdr_len = LENGTH_802_11_WITH_ADDR4;
	}


	/* Drop not my BSS frames */
	if (pRxInfo->MyBss == 0) {
		/* CFG_TODO: NEED CHECK for MT_MAC */
		{
#ifdef A4_CONN
			if (IS_ENTRY_A4(pEntry))
		pRxInfo->MyBss = 1;
	    else
#endif /* A4_CONN */


#ifdef APCLI_SUPPORT

			if (pEntry &&
				(IS_ENTRY_PEER_AP(pEntry) || (IS_ENTRY_REPEATER(pEntry)))
			   ) {
				/* Focus the EAP PKT only! */
				if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 0) &&
					(pFmeCtrl->Type == FC_TYPE_DATA) && (MAC_ADDR_EQUAL(pEntry->wdev->bssid, pRxBlk->Addr3))) {
					/*
						update RxBlk->pData, DataSize, 802.11 Header, QOS, HTC, Hw Padding
					*/
					UCHAR *pData = (UCHAR *)pFmeCtrl;
					/* 1. skip 802.11 HEADER */
					pData += LENGTH_802_11;
					/* pRxBlk->DataSize -= hdr_len; */

					/* 2. QOS */
					if (pFmeCtrl->SubType & 0x08) {
						/* skip QOS contorl field */
						pData += 2;
					}

					/* 3. Order bit: A-Ralink or HTC+ */
					if (pFmeCtrl->Order) {
#ifdef AGGREGATION_SUPPORT

						/* TODO: shiang-MT7603, fix me, because now we don't have rx_rate.field.MODE can refer */
						if ((pRxBlk->rx_rate.field.MODE <= MODE_OFDM) &&
							(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE)))
							RX_BLK_SET_FLAG(pRxBlk, fRX_ARALINK);
						else
#endif /* AGGREGATION_SUPPORT */
						{
#ifdef DOT11_N_SUPPORT
							/* skip HTC control field */
							pData += 4;
#endif /* DOT11_N_SUPPORT */
						}
					}

					/* 4. skip HW padding */
					if (pRxInfo->L2PAD)
						pData += 2;

					if (NdisEqualMemory(SNAP_802_1H, pData, 6) ||
						/* Cisco 1200 AP may send packet with SNAP_BRIDGE_TUNNEL*/
						NdisEqualMemory(SNAP_BRIDGE_TUNNEL, pData, 6))
						pData += 6;

					if (NdisEqualMemory(EAPOL, pData, 2)) {
						pRxInfo->MyBss = 1; /* correct this */
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Hit EAP!\n", __func__));
					} else {
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():  Not my bss! pRxInfo->MyBss=%d\n", __func__, pRxInfo->MyBss));
						return FALSE;
					}
				}
			} else
#endif /* APCLI_SUPPORT */

			{
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():  Not my bss! pRxInfo->MyBss=%d\n", __func__, pRxInfo->MyBss));
				return FALSE;
			}
		}
	}

	pAd->RalinkCounters.RxCountSinceLastNULL++;
#ifdef UAPSD_SUPPORT

	if (wdev->UapsdInfo.bAPSDCapable
		&& pAd->CommonCfg.APEdcaParm[0].bAPSDCapable
		&& (pFmeCtrl->SubType & 0x08)) {
		UCHAR *pData;
#ifdef CONFIG_STA_SUPPORT
		PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif /* CONFIG_STA_SUPPORT */
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("bAPSDCapable\n"));
		/* Qos bit 4 */
		pData = pRxBlk->FC + LENGTH_802_11;

		if ((*pData >> 4) & 0x01) {
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

			/* ccv EOSP frame so the peer can sleep */
			if (pEntry != NULL)
				RTMP_PS_VIRTUAL_SLEEP(pEntry);

#ifdef CONFIG_STA_SUPPORT

			if (pStaCfg->FlgPsmCanNotSleep == TRUE)
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tdls uapsd> Rcv EOSP frame but we can not sleep!\n"));
			else
#endif /* CONFIG_STA_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			{
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 ("RxDone- Rcv EOSP frame, driver may fall into sleep\n"));
				pAd->CommonCfg.bInServicePeriod = FALSE;
#ifdef CONFIG_STA_SUPPORT

				/* Force driver to fall into sleep mode when rcv EOSP frame */
				if (!pStaCfg->PwrMgmt.bDoze)
					RTMP_SLEEP_FORCE_AUTO_WAKEUP(pAd, pStaCfg);

#endif /* CONFIG_STA_SUPPORT */
			}
		}

		if ((pFmeCtrl->MoreData) && (pAd->CommonCfg.bInServicePeriod))
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MoreData bit=1, Sending trigger frm again\n"));
	}

#endif /* UAPSD_SUPPORT */
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

	/* 1: PWR_SAVE, 0: PWR_ACTIVE */
	if (pEntry != NULL) {
		UCHAR OldPwrMgmt;

		OldPwrMgmt = RtmpPsIndicate(pAd, pRxBlk->Addr2, pEntry->wcid, pFmeCtrl->PwrMgmt);
#ifdef UAPSD_SUPPORT
		RTMP_PS_VIRTUAL_TIMEOUT_RESET(pEntry);

		if (pFmeCtrl->PwrMgmt) {
			if ((CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_APSD_CAPABLE)) &&
				(pFmeCtrl->SubType & 0x08)) {
				/*
					In IEEE802.11e, 11.2.1.4 Power management with APSD,
					If there is no unscheduled SP in progress, the unscheduled SP begins
					when the QAP receives a trigger frame from a non-AP QSTA, which is a
					QoS data or QoS Null frame associated with an AC the STA has
					configured to be trigger-enabled.

					In WMM v1.1, A QoS Data or QoS Null frame that indicates transition
					to/from Power Save Mode is not considered to be a Trigger Frame and
					the AP shall not respond with a QoS Null frame.
				*/
				/* Trigger frame must be QoS data or QoS Null frame */
				UCHAR  OldUP;

				if ((*(pRxBlk->pData + LENGTH_802_11) & 0x10) == 0) {
					/* this is not a EOSP frame */
					OldUP = (*(pRxBlk->pData + LENGTH_802_11) & 0x07);

					if (OldPwrMgmt == PWR_SAVE) {
						/* hex_dump("trigger frame", pRxBlk->pData, 26); */
						UAPSD_TriggerFrameHandle(pAd, pEntry, OldUP);
					}
				} else
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("This is a EOSP frame, not a trigger frame!\n"));
			}
		}

#endif /* UAPSD_SUPPORT */
	}

#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

	/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
	if ((pFmeCtrl->SubType & 0x04)) { /* bit 2 : no DATA */
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():  No DATA!\n", __func__));
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT

	if (pEntry &&
		(IS_ENTRY_PEER_AP(pEntry) || (IS_ENTRY_REPEATER(pEntry)))
	   ) {


		if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 0)) {
			ULONG Now32;
			PSTA_ADMIN_CONFIG pApCliEntry = NULL;

			if (!(pEntry && APCLI_IF_UP_CHECK(pAd, pEntry->wdev->func_idx)))
				return FALSE;

			pApCliEntry = &pAd->StaCfg[pEntry->wdev->func_idx];

			if (pApCliEntry) {
				NdisGetSystemUpTime(&Now32);
				pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime = Now32;
			}


#ifdef CONFIG_MAP_SUPPORT
			/* do not receive 3-address broadcast/multicast packet, */
			/* because the broadcast/multicast packet woulld be send using 4-address, */
			/* 1905 message is an exception, need to receive 3-address 1905 multicast, */
			/* because some vendor send only one 3-address 1905 multicast packet */
			/* 1905 daemon would filter and drop duplicate packet */
			if (GET_ENTRY_A4(pEntry) == A4_TYPE_MAP &&
				(pRxInfo->Mcast || pRxInfo->Bcast) &&
				(memcmp(pRxBlk->Addr1, multicast_mac_1905, MAC_ADDR_LEN) != 0))
				return FALSE;
#endif /* CONFIG_MAP_SUPPORT */

			if (pApCliEntry != NULL) {
				pApCliEntry->StaStatistic.ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
				pApCliEntry->StaStatistic.RxCount++;
			}

			/* Process broadcast packets */
			if (pRxInfo->Mcast || pRxInfo->Bcast) {
				/* Process the received broadcast frame for AP-Client. */
				if (!ApCliHandleRxBroadcastFrame(pAd, pRxBlk, pEntry))
					return FALSE;
			}

			/* drop received packet which come from apcli */
			/* check if sa is to apcli */
			if (MAC_ADDR_EQUAL(pEntry->wdev->if_addr, pRxBlk->Addr3)) {
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s[%d]SA is from APCLI=%d\n\r", __func__, __LINE__, pEntry->wdev->func_idx));
				return FALSE;	/* give up this frame */
			}
		}
				RX_BLK_SET_FLAG(pRxBlk, fRX_AP);
			goto ret;
		}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (pStaCfg->BssType == BSS_INFRA) {
		/* Infrastructure mode, check address 2 for BSSID */
		if (1
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			&& (!IS_TDLS_SUPPORT(pAd))
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
		   ) {
			if (!RTMPEqualMemory(pRxBlk->Addr2, pStaCfg->MlmeAux.Bssid, 6)) {
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():  Infra-No my BSSID(Peer=>%02x:%02x:%02x:%02x:%02x:%02x, My=>%02x:%02x:%02x:%02x:%02x:%02x)!\n",
						 __func__, PRINT_MAC(pRxBlk->Addr2), PRINT_MAC(pStaCfg->MlmeAux.Bssid)));
				return FALSE; /* Receive frame not my BSSID */
			}
		}
	} else {
		/* Ad-Hoc mode or Not associated */

		/* Ad-Hoc mode, check address 3 for BSSID */
		if (!RTMPEqualMemory(pRxBlk->Addr3, pAd->StaCfg[0].Bssid, 6)) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():  AdHoc-No my BSSID(Peer=>%02x:%02x:%02x:%02x:%02x:%02x, My=>%02x:%02x:%02x:%02x:%02x:%02x)!\n",
					 __func__, PRINT_MAC(pRxBlk->Addr3), PRINT_MAC(pStaCfg->Bssid)));
			return FALSE; /* Receive frame not my BSSID */
		}
	}

#endif /*CONFIG_STA_SUPPORT */

	if (pEntry) {
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (pStaCfg->BssType == BSS_INFRA) {
			/* infra mode */
			RX_BLK_SET_FLAG(pRxBlk, fRX_AP);
#if defined(DOT11Z_TDLS_SUPPORT)  || defined(CFG_TDLS_SUPPORT)

			if ((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 0))
				RX_BLK_SET_FLAG(pRxBlk, fRX_DLS);
			else
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
				ASSERT(pRxBlk->wcid == pEntry->wcid);
		} else {
			/* ad-hoc mode */
			if ((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 0))
				RX_BLK_SET_FLAG(pRxBlk, fRX_ADHOC);
		}
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
ret:
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef APCLI_AS_WDS_STA_SUPPORT
	if (hdr_len != LENGTH_802_11_WITH_ADDR4)
		hdr_len = LENGTH_802_11;
#else
	hdr_len = LENGTH_802_11;
#endif /* APCLI_AS_WDS_STA_SUPPORT */


	return hdr_len;
}

INT sta_rx_fwd_hnd(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{
	/*
		For STA, direct to OS and no need to forwad the packet to WM
	*/
	return TRUE; /* need annouce to upper layer */
}

INT sta_rx_announce_802_3_pkt(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{

	POS_COOKIE  pObj;
	UCHAR       apidx;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;

#ifdef DOT11V_WNM_SUPPORT
#ifdef CONFIG_STA_SUPPORT

	/*drop the match DMS flow data*/
	if (RxDMSHandle(pAd, pPacket) == NDIS_STATUS_FAILURE) {
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return FALSE;
	}

#endif /* CONFIG_STA_SUPPORT */
#endif /* DOT11V_WNM_SUPPORT */
#ifdef APCLI_AS_WDS_STA_SUPPORT
	if (pAd->StaCfg[apidx].wdev.wds_enable == 0) {
#endif /*APCLI_AS_WDS_STA_SUPPORT*/

#ifdef APCLI_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAT_SUPPORT

		if (RTMP_MATPktRxNeedConvert(pAd, RtmpOsPktNetDevGet(pPacket))) {
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

			if ((wf_drv_tbl.wf_fwd_needed_hook != NULL) && (wf_drv_tbl.wf_fwd_needed_hook() == TRUE)) {
				BOOLEAN  need_drop = FALSE;

				ApCliLinkCoverRxPolicy(pAd, pPacket, &need_drop);

				if (need_drop == TRUE) {
					/*MEM_DBG_PKT_ALLOC_INC(pPacket);*/
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					return FALSE;
				}
			}

#endif /* CONFIG_WIFI_PKT_FWD */
			RTMP_MATEngineRxHandle(pAd, pPacket, 0);
		}

#endif /* MAT_SUPPORT */
	}
#endif /* APCLI_SUPPORT */
#ifdef APCLI_AS_WDS_STA_SUPPORT
	}
#endif /*APCLI_AS_WDS_STA_SUPPORT*/


#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

	return TRUE;
}

#endif /* defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT) */



VOID rx_data_frm_announce(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN struct wifi_dev *wdev)
{
	BOOLEAN eth_frame = FALSE;
	UCHAR *pData = pRxBlk->pData;
	UINT data_len = pRxBlk->DataSize;
	UCHAR wdev_idx = wdev->wdev_idx;
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;

	ASSERT(wdev_idx < WDEV_NUM_MAX);

	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid wdev_idx(%d)\n", __func__, wdev_idx));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
#ifdef IXIA_SUPPORT
		pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_WDEV_INDEX1]++;
#endif /*IXIA_SUPPORT*/
		return;
	}

#ifdef HDR_TRANS_SUPPORT

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
		eth_frame = TRUE;
		pData = pRxBlk->pTransData;
		data_len = pRxBlk->TransDataSize;
	}

#endif /* HDR_TRANS_SUPPORT */


	/* non-EAP frame */
	if (!RTMPCheckWPAframe(pAd, pEntry, pData, data_len, wdev_idx, eth_frame)) {
		if ((pRxBlk->CipherMis && FC && (FC->Type == FC_TYPE_DATA) && (pRxBlk->DataSize) && (FC->Wep))
#ifdef APCLI_CONNECTION_TRIAL
			|| (wdev->func_idx == (pAd->ApCfg.ApCliNum-1))/*Drop any non-EAP data packet in trial connection*/
#endif
			){
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: CM, wcid=%d\n",
				__func__, pRxBlk->wcid));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Addr1=%02x:%02x:%02x:%02x:%02x:%02x\t",
				PRINT_MAC(pRxBlk->Addr1)));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Addr2=%02x:%02x:%02x:%02x:%02x:%02x\n",
				PRINT_MAC(pRxBlk->Addr2)));
#ifdef IXIA_SUPPORT
			pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_NO_EAP]++;
#endif /*IXIA_SUPPORT*/
			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

#ifdef CONFIG_STA_SUPPORT

		/* TODO: revise for APCLI about this checking */
		if (pEntry->wdev->wdev_type == WDEV_TYPE_STA
			|| pEntry->wdev->wdev_type == WDEV_TYPE_GC) {
			PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);

			if (IS_ENTRY_PEER_AP(pEntry)) {
				/* printk("%s: RX\n", __func__); */
			} else /* drop all non-EAP DATA frame before peer's Port-Access-Control is secured */
				if (!STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) {
					RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
					return;
				}

#ifdef CFG_TDLS_SUPPORT
			cfg_tdls_rx_parsing(pAd, pRxBlk);
			/* return; */
#endif /* CFG_TDLS_SUPPORT */
		}

#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

		/* drop all non-EAP DATA frame before peer's Port-Access-Control is secured */
		if ((pEntry->wdev->wdev_type == WDEV_TYPE_AP || pEntry->wdev->wdev_type == WDEV_TYPE_GO) &&
			IS_ENTRY_CLIENT(pEntry) && (pEntry->PrivacyFilter == Ndis802_11PrivFilter8021xWEP)) {
			/*
				If	1) no any EAP frame is received within 5 sec and
					2) an encrypted non-EAP frame from peer associated STA is received,
				AP would send de-authentication to this STA.
			 */
			if (FC != NULL && FC->Wep &&
				pEntry->StaConnectTime > 5 && pEntry->SecConfig.Handshake.WpaState < AS_AUTHENTICATION2) {
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("==> De-Auth this STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
						 PRINT_MAC(pEntry->Addr)));
				MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
			}
#ifdef IXIA_SUPPORT
			pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_NOT_SEC]++;
#endif /*IXIA_SUPPORT*/
			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			BOOLEAN PortSecured = wdev->PortSecured;
			/* CFG TODO: ApCli on OPMODE_STA */
			/* ASSERT(wdev == &pAd->StaCfg[0].wdev); */

			/* drop all non-EAP DATA frame before peer's Port-Access-Control is secured */
			if (FC && FC->Wep) {
				/* unsupported cipher suite */
				if (IS_NO_SECURITY_Entry(wdev)) {
					RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
					return;
				}
			} else {
				/* encryption in-use but receive a non-EAPOL clear text frame, drop it */
				if (IS_SECURITY_Entry(wdev)
					&& (PortSecured == WPA_802_1X_PORT_NOT_SECURED)) {
					RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
					return;
				}
			}
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_HOTSPOT

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->pMbss) && pEntry->pMbss->HotSpotCtrl.HotSpotEnable) {
			if (hotspot_rx_handler(pAd, pEntry, pRxBlk) == TRUE)
				return;
		}

#endif /* CONFIG_HOTSPOT */

#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
	APCLI_ROAMING_ENHANCE_CHECK(pAd, pEntry, pRxBlk, wdev);
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef STATS_COUNT_SUPPORT

		if ((IS_ENTRY_CLIENT(pEntry)) && (pEntry->pMbss)) {
			BSS_STRUCT *pMbss = pEntry->pMbss;
			UCHAR *pDA = pRxBlk->Addr3;

			if (((*pDA) & 0x1) == 0x01) {
				if (IS_BROADCAST_MAC_ADDR(pDA))
					pMbss->bcPktsRx++;
				else
					pMbss->mcPktsRx++;
			} else
				pMbss->ucPktsRx++;
		}

#endif /* STATS_COUNT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef DOT11_N_SUPPORT

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU) /*&& (pAd->CommonCfg.bDisableReordering == 0)*/)
			indicate_ampdu_pkt(pAd, pRxBlk, wdev_idx);
		else if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU))
			indicate_amsdu_pkt(pAd, pRxBlk, wdev_idx);
		else
#endif /* DOT11_N_SUPPORT */
			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_ARALINK))
				indicate_agg_ralink_pkt(pAd, pEntry, pRxBlk, wdev->wdev_idx);
			else
				indicate_802_11_pkt(pAd, pRxBlk, wdev_idx);
	} else {
		RX_BLK_SET_FLAG(pRxBlk, fRX_EAP);
#ifdef CONFIG_AP_SUPPORT

		/* Update the WPA STATE to indicate the EAP handshaking is started */
		if (IS_ENTRY_CLIENT(pEntry)) {
			if (pEntry->SecConfig.Handshake.WpaState == AS_AUTHENTICATION)
				pEntry->SecConfig.Handshake.WpaState = AS_AUTHENTICATION2;
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef DOT11_N_SUPPORT

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU)
			/*&& (pAd->CommonCfg.bDisableReordering == 0)*/)
			indicate_ampdu_pkt(pAd, pRxBlk, wdev_idx);
		else
#endif /* DOT11_N_SUPPORT */
		{
#ifdef CONFIG_HOTSPOT_R2
			UCHAR *pData = (UCHAR *)pRxBlk->pData;

			if (pEntry) {
				BSS_STRUCT *pMbss = pEntry->pMbss;

				if (NdisEqualMemory(SNAP_802_1H, pData, 6) ||
					/* Cisco 1200 AP may send packet with SNAP_BRIDGE_TUNNEL*/
					NdisEqualMemory(SNAP_BRIDGE_TUNNEL, pData, 6))
					pData += 6;

				if (NdisEqualMemory(EAPOL, pData, 2))
					pData += 2;

				if ((*(pData + 1) == EAPOLStart) && (pMbss->HotSpotCtrl.HotSpotEnable == 1) &&  IS_AKM_WPA2(pMbss->wdev.SecConfig.AKMMap) && (pEntry->hs_info.ppsmo_exist == 1)) {
					UCHAR HS2_Header[4] = {0x50, 0x6f, 0x9a, 0x12};

					memcpy(&pRxBlk->pData[pRxBlk->DataSize], HS2_Header, 4);
					memcpy(&pRxBlk->pData[pRxBlk->DataSize + 4], &pEntry->hs_info, sizeof(struct _sta_hs_info));
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: hotspot rcv eapol start, %x:%x:%x:%x\n",
							 __func__, pRxBlk->pData[pRxBlk->DataSize + 4], pRxBlk->pData[pRxBlk->DataSize + 5],
							 pRxBlk->pData[pRxBlk->DataSize + 6], pRxBlk->pData[pRxBlk->DataSize + 7]));
					pRxBlk->DataSize += 8;
				}
			}

#endif
			/* Determin the destination of the EAP frame */
			/*  to WPA state machine or upper layer */
			rx_eapol_frm_handle(pAd, pEntry, pRxBlk, wdev_idx);
		}
	}
}

static BOOLEAN amsdu_non_ampdu_sanity(RTMP_ADAPTER *pAd, UINT16 cur_sn, UINT8 cur_amsdu_state,
					UINT16 previous_sn, UINT8 previous_amsdu_state)
{
	BOOLEAN amsdu_miss = FALSE;

	if (cur_sn != previous_sn) {
		if ((previous_amsdu_state == FIRST_AMSDU_FORMAT) ||
				(previous_amsdu_state == MIDDLE_AMSDU_FORMAT)) {
			amsdu_miss = TRUE;
		}
	} else {
		if (((previous_amsdu_state == FIRST_AMSDU_FORMAT) ||
			(previous_amsdu_state == MIDDLE_AMSDU_FORMAT)) &&
				(cur_amsdu_state == FIRST_AMSDU_FORMAT)) {
			amsdu_miss = TRUE;
		}


	}

	return amsdu_miss;
}

INT rx_chk_duplicate_frame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, struct wifi_dev *wdev)
{
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	UCHAR wcid = pRxBlk->wcid;
	STA_TR_ENTRY *trEntry = NULL;
	INT sn = pRxBlk->SN;
	UINT32 WmmIndex = HcGetWmmIdx(pAd, wdev);
	UCHAR up = 0;

	/*check if AMPDU frame ignore it, since AMPDU wil handle reorder */
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU))
		return NDIS_STATUS_SUCCESS;

	/*check is vaild sta entry*/
	if (wcid >= MAX_LEN_OF_TR_TABLE)
		return NDIS_STATUS_SUCCESS;

	/*check sta tr entry is exist*/
	trEntry = &pAd->MacTab.tr_entry[wcid];

	if (!trEntry)
		return NDIS_STATUS_SUCCESS;

	if ((pFmeCtrl->Type == FC_TYPE_DATA && pFmeCtrl->SubType == SUBTYPE_DATA_NULL) ||
		(pFmeCtrl->Type == FC_TYPE_DATA && pFmeCtrl->SubType == SUBTYPE_QOS_NULL))
		return NDIS_STATUS_SUCCESS;

	up = (WmmIndex * 4) + pRxBlk->UserPriority;

	/*check frame is QoS or Non-QoS frame*/
	if (!(pFmeCtrl->SubType & 0x08) || up >= NUM_OF_UP)
		up = (NUM_OF_UP - 1);

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU)) {
		if ((amsdu_non_ampdu_sanity(pAd, sn, pRxBlk->AmsduState,
			trEntry->previous_sn[up], trEntry->previous_amsdu_state[up]))
			|| (pRxBlk->AmsduState == FINAL_AMSDU_FORMAT)) {
			trEntry->cacheSn[up] =  trEntry->previous_sn[up];
		}

		trEntry->previous_amsdu_state[up] = pRxBlk->AmsduState;
		trEntry->previous_sn[up] = sn;

		if (!pFmeCtrl->Retry || trEntry->cacheSn[up] != sn)
			return NDIS_STATUS_SUCCESS;
	} else {
		if (!pFmeCtrl->Retry || trEntry->cacheSn[up] != sn) {
			trEntry->cacheSn[up] = sn;
			return NDIS_STATUS_SUCCESS;
		}
	}

	/* Middle/End of fragment */
	if (pRxBlk->FN && pRxBlk->FN != pAd->FragFrame.LastFrag)
		return NDIS_STATUS_SUCCESS;

	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): pFrameCtrl->Retry=%d, trEntry->cacheSn[%d]=%d, pkt->sn=%d\n",
			 __func__, pFmeCtrl->Retry, up, trEntry->cacheSn[up], sn));
	/*is duplicate frame, should return failed*/
	return NDIS_STATUS_FAILURE;
}

VOID rx_802_3_data_frm_announce(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk, struct wifi_dev *wdev)
{
#ifdef CONFIG_HOTSPOT

	if (IS_ENTRY_CLIENT(pEntry) && (pEntry->pMbss) && pEntry->pMbss->HotSpotCtrl.HotSpotEnable) {
		if (hotspot_rx_handler(pAd, pEntry, pRxBlk) == TRUE)
			return;
	}

#endif /* CONFIG_HOTSPOT */

#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
	APCLI_ROAMING_ENHANCE_CHECK(pAd, pEntry, pRxBlk, wdev);
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU))
		indicate_ampdu_pkt(pAd, pRxBlk, wdev->wdev_idx);
	else
		indicate_802_3_pkt(pAd, pRxBlk, wdev->wdev_idx);
}

static MAC_TABLE_ENTRY *check_rx_pkt_allowed(RTMP_ADAPTER *pAd, RX_BLK *rx_blk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef CONFIG_AP_SUPPORT
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)rx_blk->FC;
#endif

	if (VALID_UCAST_ENTRY_WCID(pAd, rx_blk->wcid)) {
#if defined(OUI_CHECK_SUPPORT) && defined(MAC_REPEATER_SUPPORT)
		BOOLEAN	bCheck_ApCli = FALSE;
		UCHAR	orig_wcid = rx_blk->wcid;
#endif
		pEntry = &pAd->MacTab.Content[rx_blk->wcid];
#ifdef OUI_CHECK_SUPPORT

		if (
			(pAd->MacTab.oui_mgroup_cnt > 0)
#ifdef MAC_REPEATER_SUPPORT
			|| (pAd->ApCfg.RepeaterCliSize != 0)
#endif
		) {
			if (NdisCmpMemory(pEntry->Addr, rx_blk->Addr2, MAC_ADDR_LEN))
				pEntry = research_entry(pAd, rx_blk);

#ifdef MAC_REPEATER_SUPPORT

			if (pEntry) {
				if (IS_ENTRY_REPEATER(pEntry) || IS_ENTRY_PEER_AP(pEntry))
					bCheck_ApCli = TRUE;
			}

#endif
		}

#ifdef MAC_REPEATER_SUPPORT

		if (bCheck_ApCli == TRUE) {
			if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
				REPEATER_CLIENT_ENTRY	*pReptEntry = NULL;
				BOOLEAN					entry_found = FALSE;

				pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, &rx_blk->Addr1[0], TRUE);

				if (pReptEntry && (pReptEntry->CliValid == TRUE)) {
					pEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
					rx_blk->wcid = pEntry->wcid;
					entry_found = TRUE;
				} else if ((pReptEntry == NULL) && IS_ENTRY_PEER_AP(pEntry)) {/* this packet is for APCLI */
					INT apcli_idx = pEntry->func_tb_idx;

					pEntry = &pAd->MacTab.Content[pAd->StaCfg[apcli_idx].MacTabWCID];
					rx_blk->wcid = pEntry->wcid;
					entry_found = TRUE;
				}

				if (entry_found) {
					if (orig_wcid != rx_blk->wcid) {
						pAd->MacTab.repeater_wcid_error_cnt++;
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							("orig_wcid=%d,pRxBlk->wcid=%d\n\r", orig_wcid, rx_blk->wcid));
					}

					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("%s()[%d]:%02x:%02x:%02x:%02x:%02x:%02x recv data\n\r",
						__func__, __LINE__, PRINT_MAC(rx_blk->Addr1)));
				} else {
					if ((rx_blk->Addr1[0] & 0x01) == 0x01) {/* BM packet,find apcli entry */
						if (IS_ENTRY_PEER_AP(pEntry)) {
							rx_blk->wcid = pEntry->wcid;
							pAd->MacTab.repeater_bm_wcid_error_cnt++;
						} else if (IS_ENTRY_REPEATER(pEntry)) {
							UCHAR apcli_wcid = 0;

							if (pEntry->wdev && (pEntry->wdev->func_idx < pAd->ApCfg.ApCliNum))
								apcli_wcid = pAd->StaCfg[pEntry->wdev->func_idx].MacTabWCID;
							else   /* use default apcli0 */
								apcli_wcid = pAd->StaCfg[0].MacTabWCID;

							pEntry = &pAd->MacTab.Content[apcli_wcid];
							rx_blk->wcid = pEntry->wcid;
							pAd->MacTab.repeater_bm_wcid_error_cnt++;
						}
					} else {
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("%s()[%d]:Cannot found WCID of data packet!\n",
							__func__, __LINE__));
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("A1:%02x:%02x:%02x:%02x:%02x:%02x, A2:%02x:%02x:%02x:%02x:%02x:%02x\n",
							PRINT_MAC(rx_blk->Addr1),
							PRINT_MAC(rx_blk->Addr2)));
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("wcid=%d,orig_wcid=%d,M/B/U=%d/%d/%d\r\n",
							pEntry->wcid, orig_wcid,
							rx_blk->pRxInfo->Mcast,
							rx_blk->pRxInfo->Bcast,
							rx_blk->pRxInfo->U2M));
					}
				}
			}
		}

#endif /* MAC_REPEATER_SUPPORT */
#endif /*OUI_CHECK_SUPPORT*/
	} else {
#ifdef AIR_MONITOR
		if (pAd->MntEnable) {
			UCHAR count;
			MNT_STA_ENTRY *pMntTable;

			for (count = 0; count < MAX_NUM_OF_MONITOR_STA; count++) {
				pMntTable = &pAd->MntTable[count];

				if (pMntTable->bValid &&
					 (MAC_ADDR_EQUAL(rx_blk->Addr1, pMntTable->addr) ||
					 MAC_ADDR_EQUAL(rx_blk->Addr2, pMntTable->addr))) {
					pEntry = pMntTable->pMacEntry;
					break;
				}
			}
		}
#endif
		if (!pEntry)
			pEntry = research_entry(pAd, rx_blk);

		if (pEntry) {
			rx_blk->wcid = pEntry->wcid;
#ifdef MAC_REPEATER_SUPPORT

			if (IS_ENTRY_PEER_AP(pEntry))
				rx_blk->wcid = pEntry->wcid;
			else if (IS_ENTRY_REPEATER(pEntry)) {
				UCHAR apcli_wcid = 0;

				if (pEntry->wdev && (pEntry->wdev->func_idx < pAd->ApCfg.ApCliNum))
					apcli_wcid = pAd->StaCfg[pEntry->wdev->func_idx].MacTabWCID;
				else   /* use default apcli0 */
					apcli_wcid = pAd->StaCfg[0].MacTabWCID;

				pEntry = &pAd->MacTab.Content[apcli_wcid];
				rx_blk->wcid = pEntry->wcid;
			}

#endif
		}
	}

	if (pEntry && pEntry->wdev) {
		struct wifi_dev_ops *ops = pEntry->wdev->wdev_ops;

		if (ops->rx_pkt_allowed) {
			if (!ops->rx_pkt_allowed(pAd, pEntry->wdev, rx_blk)) {
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s(): rx_pkt_allowed drop this paket!\n", __func__));
				return NULL;
			}
		}
	} else {
		if (pEntry) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("invalid hdr_len, wdev=%p! ", pEntry->wdev));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		} else {
#ifdef CONFIG_AP_SUPPORT
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)

			if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1)) {
				if (MAC_ADDR_EQUAL(rx_blk->Addr1, pAd->CurrentAddress))
					pEntry = FindWdsEntry(pAd, rx_blk);
			}

#endif /* defined(WDS_SUPPORT) || defined(CLIENT_WDS) */

			/* check if Class2 or 3 error */
			if ((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 1))
				ap_chk_cl2_cl3_err(pAd, rx_blk);

#endif /* CONFIG_AP_SUPPORT */
		}
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): drop this packet as pEntry NULL OR !rx_pkt_allowed !!\n", __func__));

		return NULL;
	}

	return pEntry;
}


VOID dev_rx_802_3_data_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
#endif
	struct wifi_dev_ops *ops;

	pEntry = check_rx_pkt_allowed(pAd, pRxBlk);
	if (!pEntry)
		goto drop;
	wdev = pEntry->wdev;
	ops = wdev->wdev_ops;
#ifdef APCLI_CONNECTION_TRIAL
	if (wdev->func_idx == (pAd->ApCfg.ApCliNum-1)) /*Drop 802_3 packet for trial connection*/
		goto drop;
#endif /*APCLI_CONNECTION_TRIAL*/
	if (ops->ieee_802_3_data_rx(pAd, wdev, pRxBlk, pEntry))
		return;
drop:
#ifdef CONFIG_AP_SUPPORT

	/* Increase received error packet counter per BSS */
	if (pFmeCtrl->FrDs == 0 &&
		pRxInfo->U2M &&
		pRxBlk->bss_idx < pAd->ApCfg.BssidNum) {
		pAd->ApCfg.MBSSID[pRxBlk->bss_idx].RxDropCount++;
		pAd->ApCfg.MBSSID[pRxBlk->bss_idx].RxErrorCount++;
	}
#if defined(TXRX_STAT_SUPPORT) || defined(CUSTOMER_DCC_FEATURE)
	if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
#ifdef TXRX_STAT_SUPPORT
		INC_COUNTER64(pEntry->pMbss->stat_bss.RxPacketDroppedCount);
#endif
#ifdef CUSTOMER_DCC_FEATURE
		pEntry->RxDropCount++;
#endif
	}
#endif
#endif
#ifdef IXIA_SUPPORT
	pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_RX_NOT_ALLOW8023]++;
#endif /*IXIA_SUPPORT*/
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
}


/*
 All Rx routines use RX_BLK structure to hande rx events
 It is very important to build pRxBlk attributes
  1. pHeader pointer to 802.11 Header
  2. pData pointer to payload including LLC (just skip Header)
  3. set payload size including LLC to DataSize
  4. set some flags with RX_BLK_SET_FLAG()
*/
VOID dev_rx_802_11_data_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
#ifdef CONFIG_AP_SUPPORT
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
#endif
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct wifi_dev *wdev;
	struct wifi_dev_ops *ops;

	pEntry = check_rx_pkt_allowed(pAd, pRxBlk);

	if (!pEntry)
		goto drop;

	wdev = pEntry->wdev;
	ops = wdev->wdev_ops;
	if (ops->ieee_802_11_data_rx(pAd, wdev, pRxBlk, pEntry))
		return;

drop:
#ifdef CONFIG_AP_SUPPORT

	/* Increase received error packet counter per BSS */
	if (pFmeCtrl->FrDs == 0 &&
		pRxInfo->U2M &&
		pRxBlk->bss_idx < pAd->ApCfg.BssidNum) {
		pAd->ApCfg.MBSSID[pRxBlk->bss_idx].RxDropCount++;
		pAd->ApCfg.MBSSID[pRxBlk->bss_idx].RxErrorCount++;
	}
#if defined(TXRX_STAT_SUPPORT) || defined(CUSTOMER_DCC_FEATURE)
	if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
#ifdef TXRX_STAT_SUPPORT
		INC_COUNTER64(pEntry->pMbss->stat_bss.RxPacketDroppedCount);
#endif
#ifdef CUSTOMER_DCC_FEATURE
		pEntry->RxDropCount++;
#endif
	}
#endif
#endif /* CONFIG_AP_SUPPORT */
#ifdef IXIA_SUPPORT
	pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_RX_NOT_ALLOW80211]++;
#endif /*IXIA_SUPPORT*/
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	return;
}

/*
		========================================================================
		Routine Description:
			Process RxDone interrupt, running in DPC level

		Arguments:
			pAd    Pointer to our adapter

		Return Value:
			None

		Note:
			This routine has to maintain Rx ring read pointer.
	========================================================================
*/
NDIS_STATUS header_packet_process(
	RTMP_ADAPTER *pAd,
	PNDIS_PACKET pRxPacket,
	RX_BLK *pRxBlk)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if ((pRxBlk->DataSize == 0) && (pRxPacket)) {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Packet Length is zero!\n", __func__));
#ifdef CUT_THROUGH_DBG
			pAd->RxDropPacket++;
#endif
#ifdef IXIA_SUPPORT
			pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_DATA_SIZE]++;
#endif /*IXIA_SUPPORT*/
			return NDIS_STATUS_INVALID_DATA;
		}
	}

#endif /* MT_MAC */
#ifdef CONFIG_STA_SUPPORT
#ifdef SNIFFER_SUPPORT

	if (MONITOR_ON(pAd) && pAd->monitor_ctrl.CurrentMonitorMode == MONITOR_MODE_REGULAR_RX)
		return NDIS_STATUS_SUCCESS;

#endif /* SNIFFER_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE

	if (pAd->CommonCfg.DebugFlags & DBF_DBQ_RXWI) {
		dbQueueEnqueueRxFrame(GET_OS_PKT_DATAPTR(pRxPacket),
							  (UCHAR *)pHeader,
							  pAd->CommonCfg.DebugFlags);
	}

#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

#ifdef AIR_MONITOR
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (pAd->MntEnable && !pRxBlk->pRxInfo->CipherErr && !pRxBlk->pRxInfo->Crc) {
			MAC_TABLE_ENTRY *pEntry = NULL;

			if (VALID_WCID(pRxBlk->wcid))
				pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

			if (pEntry && IS_ENTRY_MONITOR(pEntry)) {
				FRAME_CONTROL *fc = (FRAME_CONTROL *)pRxBlk->FC;

				Air_Monitor_Pkt_Report_Action(pAd, pRxBlk->wcid, pRxBlk);

				if (!((fc->Type == FC_TYPE_MGMT) && (fc->SubType == SUBTYPE_PROBE_REQ))) {
					RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
					return NDIS_STATUS_INVALID_DATA;
				}
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* AIR_MONITOR */

#ifdef RT_CFG80211_SUPPORT
	{
		/* todo: Bind to pAd->net_dev */
		if (RTMP_CFG80211_HOSTAPD_ON(pAd))
			SET_PKT_OPMODE_AP(pRxBlk);
		else
			SET_PKT_OPMODE_STA(pRxBlk);
	}

#endif /* RT_CFG80211_SUPPORT */
#ifdef CFG80211_MULTI_STA

	if (RTMP_CFG80211_MULTI_STA_ON(pAd, pAd->cfg80211_ctrl.multi_sta_net_dev) &&
		(((FC->SubType == SUBTYPE_BEACON || FC->SubType == SUBTYPE_PROBE_RSP) &&
		  NdisEqualMemory(pAd->StaCfg[MAIN_MBSSID].CfgApCliBssid, pRxBlk->Addr2, MAC_ADDR_LEN)) ||
		 (pHeader->FC.SubType == SUBTYPE_PROBE_REQ) ||
		 NdisEqualMemory(pAd->StaCfg[MAIN_MBSSID].MlmeAux.Bssid, pRxBlk->Addr2, MAC_ADDR_LEN)))
		SET_PKT_OPMODE_AP(pRxBlk);
	else
		SET_PKT_OPMODE_STA(pRxBlk);

#endif /* CFG80211_MULTI_STA */
	/* Increase Total receive byte counter after real data received no mater any error or not */
	pAd->RalinkCounters.ReceivedByteCount += pRxBlk->DataSize;
	pAd->RalinkCounters.OneSecReceivedByteCount += pRxBlk->DataSize;
#ifdef IXIA_SUPPORT
	if (IS_EXPECTED_LENGTH(pAd, GET_OS_PKT_LEN(pRxPacket))) {
		pAd->RalinkCounters.RxCount++;
		pAd->tr_ctl.tp_dbg.rx_pkt_len = GET_OS_PKT_LEN(pRxPacket);
		pAd->tr_ctl.tp_dbg.rxpkt_dct_period++;
	}
#else
	pAd->RalinkCounters.RxCount++;
#endif /*IXIA_SUPPORT*/
	pAd->RalinkCounters.OneSecRxCount++;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd)) {
		MT_ATERxDoneHandle(pAd, pRxBlk);
		return NDIS_STATUS_SUCCESS;
	}

#endif
#ifdef STATS_COUNT_SUPPORT
#if defined(MT7626)
	if (IS_ATE_DBDC(pAd)) {
		RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)pRxBlk->rmac_info;
		UCHAR BandIdx = 0, Channel = 0;

		if (!rxd_base) {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_INVALID_DATA;
		}

		Channel = rxd_base->RxD1.ChFreq;

		if (Channel == 0) {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_INVALID_DATA;
		}

		BandIdx = HcGetBandByChannel(pAd, Channel);
		INC_COUNTER64(pAd->WlanCounters[BandIdx].ReceivedFragmentCount);
	}
#else
	INC_COUNTER64(pAd->WlanCounters[0].ReceivedFragmentCount);
#endif /* defined(MT7626) */
#endif /* STATS_COUNT_SUPPORT */

	/* Check for all RxD errors */
	if (rtmp_chk_rx_err(pAd, pRxBlk) != NDIS_STATUS_SUCCESS) {
		pAd->Counters8023.RxErrors++;
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): CheckRxError!\n", __func__));
#ifdef CUT_THROUGH_DBG
		pAd->RxDropPacket++;
#endif
#ifdef IXIA_SUPPORT
		pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_RXD_ERROR]++;
#endif /*IXIA_SUPPORT*/
		return NDIS_STATUS_INVALID_DATA;
	}

	/* TODO: shiang-usw, for P2P, we original has following code, need to check it and merge to correct place!!! */
	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS rx_packet_process(
	RTMP_ADAPTER *pAd,
	PNDIS_PACKET pRxPacket,
	RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
#ifdef SNIFFER_SUPPORT
	struct wifi_dev *wdev;
	UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pRxPacket);
#endif /* SNIFFER_SUPPORT */
#ifdef RATE_PRIOR_SUPPORT
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif/*RATE_PRIOR_SUPPORT*/
#ifdef CONFIG_ATE

	if (ATE_ON(pAd)) {
		/* TODO::Check if Rx cutthrough stable */
		if (pRxPacket)
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);

		return NDIS_STATUS_SUCCESS;
	}

#endif /* CONFIG_ATE */

#ifdef SNIFFER_SUPPORT

		if (MONITOR_ON(pAd) && pAd->monitor_ctrl.CurrentMonitorMode == MONITOR_MODE_FULL) {
			PNDIS_PACKET	pClonePacket;
			PNDIS_PACKET    pTmpRxPacket;

			if (pAd->monitor_ctrl.FrameType == FC_TYPE_RSVED ||
				pAd->monitor_ctrl.FrameType == FC->Type) {
				if (!pAd->monitor_ctrl.MacFilterOn || (pAd->monitor_ctrl.MacFilterOn &&
					((pRxBlk->Addr1 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr1)) ||
					(pRxBlk->Addr2 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr2)) ||
					(pRxBlk->Addr3 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr3)) ||
					(pRxBlk->Addr4 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr4))))) {
					wdev = pAd->wdev_list[wdev_idx];
					pTmpRxPacket = pRxBlk->pRxPacket;
					pClonePacket = ClonePacket(MONITOR_ON(pAd), wdev->if_dev, pRxBlk->pRxPacket,
								pRxBlk->pData, pRxBlk->DataSize);
					pRxBlk->pRxPacket = pClonePacket;
					STA_MonPktSend(pAd, pRxBlk);
					pRxBlk->pRxPacket = pTmpRxPacket;
				}
			}
		}
	if (MONITOR_ON(pAd) && pAd->monitor_ctrl.CurrentMonitorMode == MONITOR_MODE_REGULAR_RX) {
		if (pAd->monitor_ctrl.FrameType == FC_TYPE_RSVED ||
				pAd->monitor_ctrl.FrameType == FC->Type) {
			if (!pAd->monitor_ctrl.MacFilterOn || (pAd->monitor_ctrl.MacFilterOn &&
				((pRxBlk->Addr1 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr1)) ||
				(pRxBlk->Addr2 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr2)) ||
				(pRxBlk->Addr3 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr3)) ||
				(pRxBlk->Addr4 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr4))))) {
				STA_MonPktSend(pAd, pRxBlk);
				return NDIS_STATUS_SUCCESS;
			}
		}
	}

#endif /* SNIFFER_SUPPORT */
#ifdef RATE_PRIOR_SUPPORT
		pEntry = MacTableLookup(pAd, pRxBlk->Addr2);
		if (pEntry && FC->SubType != SUBTYPE_DATA_NULL && FC->SubType != SUBTYPE_QOS_NULL) {
			pEntry->McsTotalRxCount++;
			if (pRxBlk->rx_rate.field.MCS <= 3)
				pEntry->McsLowRateRxCount++;
		}
#endif/*RATE_PRIOR_SUPPORT*/

	if (FC == NULL) {
#ifdef IXIA_SUPPORT
		pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_FC_NULL]++;
#endif /*IXIA_SUPPORT*/
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_SUCCESS;
	}
#ifdef LTF_SNR_SUPPORT
	if (MacTableLookup(pAd, pRxBlk->Addr2)) {
		if (pRxBlk->rx_rate.field.MODE > MODE_CCK) {
			pAd->Avg_LTFSNRx16 = (pAd->Avg_LTFSNR != 0) ?
				(pRxBlk->Ofdm_SNR - pAd->Avg_LTFSNR + pAd->Avg_LTFSNRx16) : pRxBlk->Ofdm_SNR << 4;
			pAd->Avg_LTFSNR = pAd->Avg_LTFSNRx16 >> 4;
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s(): OFDM Pkt_SNR %d Avg_LTFSNR = %d \t Avg_LTFSNRx16 = %d\n",
					__func__, pRxBlk->Ofdm_SNR, pAd->Avg_LTFSNR, pAd->Avg_LTFSNRx16));
		}
	}
#endif
	switch (FC->Type) {
	case FC_TYPE_DATA:
		/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("rx_packet_process: wcid=0x%x\n",pRxBlk->wcid)); */
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
#ifdef VLAN_SUPPORT
			/*Check HW-padded field*/
			remove_vlan_hw_padding(pAd, pRxBlk);

			if (RTMPEqualMemory(TPID, pRxBlk->pData + LENGTH_802_3_NO_TYPE, 2) &&
				 RTMPEqualMemory(EAPOL, pRxBlk->pData + LENGTH_802_3_NO_TYPE + LENGTH_802_1Q, 2)) {
				/* It is VLAN EAPOL Packet*/
				remove_vlan_tag(pAd, pRxBlk);
				rebuild_802_11_eapol_frm(pAd, pRxBlk);
				dev_rx_802_11_data_frm(pAd, pRxBlk);
			} else if (RTMPEqualMemory(EAPOL, pRxBlk->pData + LENGTH_802_3_NO_TYPE, 2)) {
				/* It is a HW-VLAN-untagged EAPOL Packet*/
				rebuild_802_11_eapol_frm(pAd, pRxBlk);
				dev_rx_802_11_data_frm(pAd, pRxBlk);
			} else
#endif /*VLAN_SUPPORT*/
				dev_rx_802_3_data_frm(pAd, pRxBlk);
		}
		else
			dev_rx_802_11_data_frm(pAd, pRxBlk);

		break;

	case FC_TYPE_MGMT:
		dev_rx_mgmt_frm(pAd, pRxBlk);
		break;

	case FC_TYPE_CNTL:
		dev_rx_ctrl_frm(pAd, pRxBlk);
		break;

	default:
#ifdef IXIA_SUPPORT
		pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_RX_TYPE_ERR]++;
#endif /*IXIA_SUPPORT*/
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		break;
	}

	return NDIS_STATUS_SUCCESS;
}

#ifdef BB_SOC
#ifdef TCSUPPORT_WLAN_SW_RPS
extern int (*get_WifitolanRps_hook)(void);
extern int (*fromWlan5GPktRpsHandle_hook)(void *pRxPacket);
#endif
#endif

BOOLEAN mtd_rx_done_handle(RTMP_ADAPTER *pAd)
{
	UINT32 RxProcessed, RxPending;
	BOOLEAN bReschedule = FALSE;
	PNDIS_PACKET pkt = NULL;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *pRxRing = &hif->RxRing[HIF_RX_IDX0];
	RX_BLK rx_blk, *p_rx_blk = NULL;
	UINT16 max_rx_process_cnt = pRxRing->max_rx_process_cnt;

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	RxProcessed = RxPending = 0;

	while (1) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)
			&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE)))
			break;

#ifdef ERR_RECOVERY
		if (IsStopingPdma(&pAd->ErrRecoveryCtl))
			break;
#endif /* ERR_RECOVERY */

#ifdef RTMP_MAC_PCI
		if (RxProcessed++ > max_rx_process_cnt) {
			bReschedule = TRUE;
			break;
		}
#endif /* RTMP_MAC_PCI */

		pkt = asic_get_pkt_from_rx_resource(pAd, &bReschedule, &RxPending, HIF_RX_IDX0);

		if (pkt) {
			os_zero_mem(&rx_blk, sizeof(RX_BLK));
			p_rx_blk = &rx_blk;
#ifdef BB_SOC
#ifdef TCSUPPORT_WLAN_SW_RPS
		if (get_WifitolanRps_hook) {
			if (get_WifitolanRps_hook() == 1) {
				struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);

				if (skb) {
					p_rx_blk = (RX_BLK *)(skb->rxBlk);
					memset(p_rx_blk, 0, sizeof(RX_BLK));
					skb->pAd = (void *) pAd;
				}
			} else {
				struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);

				if (skb)
					skb->pAd = NULL;
			}
		}

		if (fromWlan5GPktRpsHandle_hook) {
			struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);

			if (skb->pAd) {
				if (fromWlan5GPktRpsHandle_hook(pkt) == 0)
					continue;
			}
		}
#endif
#endif
			asic_rx_pkt_process(pAd, HIF_RX_IDX0, p_rx_blk, pkt);
		} else
			break;
	}

	if (pRxRing->sw_read_idx_inc > 0) {
		HIF_IO_WRITE32(pAd->hdev_ctrl, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);
		pRxRing->sw_read_idx_inc = 0;
#ifdef CONFIG_TP_DBG
		tp_dbg->IoWriteRx++;
#endif
	}


#ifdef CONFIG_TP_DBG
	RxProcessed--;
	/* 0 <= RxMaxProcessCntA <= 1/4 <= RxMaxProcessCntB <= 1/2 <= RxMaxProcessCntC < 1 == RxMaxProcessCntD */
	if (RxProcessed <= (max_rx_process_cnt >> 2))
		tp_dbg->RxMaxProcessCntA++;
	else if (RxProcessed <= (max_rx_process_cnt >> 1))
		tp_dbg->RxMaxProcessCntB++;
	else if (RxProcessed < max_rx_process_cnt)
		tp_dbg->RxMaxProcessCntC++;
	else
		tp_dbg->RxMaxProcessCntD++;
#endif

	return bReschedule;
}

#ifdef MT7626_E2_SUPPORT
BOOLEAN mtd_rx2_done_handle(RTMP_ADAPTER *pAd)
{
	UINT32 RxProcessed, RxPending;
	BOOLEAN bReschedule = FALSE;
	PNDIS_PACKET pkt = NULL;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *pRxRing = &hif->RxRing[HIF_RX_IDX2];
	RX_BLK rx_blk, *p_rx_blk = NULL;
	UINT16 max_rx_process_cnt = pRxRing->max_rx_process_cnt;

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	RxProcessed = RxPending = 0;

	while (1) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)
				&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE)))
			break;

#ifdef ERR_RECOVERY
		if (IsStopingPdma(&pAd->ErrRecoveryCtl))
			break;
#endif /* ERR_RECOVERY */

#ifdef RTMP_MAC_PCI
		if (RxProcessed++ > max_rx_process_cnt) {
			bReschedule = TRUE;
			break;
		}
#endif /* RTMP_MAC_PCI */

		pkt = asic_get_pkt_from_rx2_resource(pAd, &bReschedule, &RxPending, HIF_RX_IDX2);

		if (pkt) {
			os_zero_mem(&rx_blk, sizeof(RX_BLK));
			p_rx_blk = &rx_blk;
#ifdef BB_SOC
#ifdef TCSUPPORT_WLAN_SW_RPS
			if (get_WifitolanRps_hook) {
				if (get_WifitolanRps_hook() == 1) {
					struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);

					if (skb) {
						p_rx_blk = (RX_BLK *)(skb->rxBlk);
						memset(p_rx_blk, 0, sizeof(RX_BLK));
						skb->pAd = (void *) pAd;
					}
				} else {
					struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);

					if (skb)
						skb->pAd = NULL;
				}
			}

			if (fromWlan5GPktRpsHandle_hook) {
				struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);

				if (skb->pAd) {
					if (fromWlan5GPktRpsHandle_hook(pkt) == 0)
						continue;
				}
			}
#endif
#endif
			asic_rx2_pkt_process(pAd, HIF_RX_IDX2, p_rx_blk, pkt);
		} else
			break;
	}

	if (pRxRing->sw_read_idx_inc > 0) {
		HIF_IO_WRITE32(pAd->hdev_ctrl, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);
		pRxRing->sw_read_idx_inc = 0;
#ifdef CONFIG_TP_DBG
		tp_dbg->IoWriteRx++;
#endif
	}


#ifdef CONFIG_TP_DBG
	RxProcessed--;
	/* 0 <= RxMaxProcessCntA <= 1/4 <= RxMaxProcessCntB <= 1/2 <= RxMaxProcessCntC < 1 == RxMaxProcessCntD */
	if (RxProcessed <= (max_rx_process_cnt >> 2))
		tp_dbg->RxMaxProcessCntA++;
	else if (RxProcessed <= (max_rx_process_cnt >> 1))
		tp_dbg->RxMaxProcessCntB++;
	else if (RxProcessed < max_rx_process_cnt)
		tp_dbg->RxMaxProcessCntC++;
	else
		tp_dbg->RxMaxProcessCntD++;
#endif

	return bReschedule;
}
#endif

BOOLEAN rtmp_rx_done_handle(RTMP_ADAPTER *pAd)
{
	UINT32 RxProcessed, RxPending;
	BOOLEAN bReschedule = FALSE;
	PNDIS_PACKET pRxPacket = NULL;
	RX_BLK rxblk, *pRxBlk;
#ifdef LINUX
#ifdef RTMP_RBUS_SUPPORT

	if (pAd->infType == RTMP_DEV_INF_RBUS) {
#if defined(CONFIG_RA_CLASSIFIER)  || defined(CONFIG_RA_CLASSIFIER_MODULE)
#if defined(CONFIG_RALINK_EXTERNAL_TIMER)
		classifier_cur_cycle = (*((UINT32 *)(0xB0000D08)) & 0x0FFFF);
#else
		classifier_cur_cycle = read_c0_count();
#endif /* CONFIG_RALINK_EXTERNAL_TIMER */
#endif /* CONFIG_RA_CLASSIFIER */
	}

#endif /* RTMP_RBUS_SUPPORT */
#endif /* LINUX */
	RxProcessed = RxPending = 0;

	while (1) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)
			&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE)))
			break;

#ifdef ERR_RECOVERY

		if (IsStopingPdma(&pAd->ErrRecoveryCtl))
			break;

#endif /* ERR_RECOVERY */
#ifdef RTMP_MAC_PCI
#ifdef UAPSD_SUPPORT
		UAPSD_TIMING_RECORD_INDEX(RxProcessed);
#endif /* UAPSD_SUPPORT */

		if (RxProcessed++ > MAX_RX_PROCESS_CNT) {
			bReschedule = TRUE;
			break;
		}

#ifdef UAPSD_SUPPORT
		/*if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)) */
		UAPSD_MR_SP_SUSPEND(pAd);
#endif /* UAPSD_SUPPORT */

		/*
			Note:

			Can not take off the NICUpdateFifoStaCounters(); Or the
			FIFO overflow rate will be high, i.e. > 3%
			(see the rate by "iwpriv ra0 show stainfo")

			Based on different platform, try to find the best value to
			replace '4' here (overflow rate target is about 0%).
		*/
		if (++pAd->FifoUpdateDone >= FIFO_STAT_READ_PERIOD) {
			NICUpdateFifoStaCounters(pAd);
			pAd->FifoUpdateDone = 0;
		}

#endif /* RTMP_MAC_PCI */
		os_zero_mem(&rxblk, sizeof(RX_BLK));
		pRxBlk = &rxblk;
		pRxPacket = asic_get_pkt_from_rx_resource(pAd, &bReschedule, &RxPending, HIF_RX_IDX0);

		if (pRxPacket == NULL)
			break;

		/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
			if (pRxPacket) {
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
#ifdef IXIA_SUPPORT
			pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_RX_DMA_BZ]++;
#endif /*IXIA_SUPPORT*/
				continue;
			}
		}

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_CMD_RSP)) {
			RX_BLK_CLEAR_FLAG(pRxBlk, fRX_CMD_RSP);
			continue;
		}

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_RETRIEVE)) {
			RX_BLK_CLEAR_FLAG(pRxBlk, fRX_RETRIEVE);
			continue;
		}

		if (header_packet_process(pAd, pRxPacket, pRxBlk) != NDIS_STATUS_SUCCESS)
			continue;

#ifdef CUT_THROUGH

		if (CUT_THROUGH_RX_ENABL(pAd->PktTokenCb)) {
			PKT_TOKEN_CB *PktTokenCB = (PKT_TOKEN_CB *)pAd->PktTokenCb;
			UINT8 Type;
			UINT32 Drop;

			RTMP_SEM_LOCK(&PktTokenCB->rx_order_notify_lock);
			Drop = cut_through_rx_drop(pAd->PktTokenCb, pRxBlk->token_id);

			if (cut_through_rx_mark_rxdone(PktTokenCB, pRxBlk->token_id) &&
				(cut_through_rx_in_order(PktTokenCB, pRxBlk->token_id) ||
				 Drop)) {
				pRxPacket = cut_through_rx_deq(PktTokenCB, pRxBlk->token_id, &Type);
				RTMP_SEM_UNLOCK(&PktTokenCB->rx_order_notify_lock);

				if (pRxPacket) {
					if (Drop) {
#ifdef IXIA_SUPPORT
						pAd->tr_ctl.tp_dbg.RxDropPacketCnt[DROP_RX_CUT]++;
#endif /*IXIA_SUPPORT*/
						RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_RESOURCES);
						continue;
					} else
						rx_packet_process(pAd, pRxPacket, pRxBlk);
				} else
					continue;
			} else {
				RTMP_SEM_UNLOCK(&PktTokenCB->rx_order_notify_lock);
				continue;
			}
		} else
#endif /* CUT_THROUGH */
		{
			rx_packet_process(pAd, pRxPacket, pRxBlk);
		}
	}

#ifdef UAPSD_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* dont remove the function or UAPSD will fail */
		UAPSD_MR_SP_RESUME(pAd);
		UAPSD_SP_CloseInRVDone(pAd);
	}

#endif /* CONFIG_AP_SUPPORT */
#endif /* UAPSD_SUPPORT */
	return bReschedule;
}

/*
	STEP 1. Decide number of fragments required to deliver this MSDU.
		The estimation here is not very accurate because difficult to
		take encryption overhead into consideration here. The result
		"NumberOfFrag" is then just used to pre-check if enough free
		TXD are available to hold this MSDU.

		The calculated "NumberOfFrag" is a rough estimation because of various
		encryption/encapsulation overhead not taken into consideration. This number is just
		used to make sure enough free TXD are available before fragmentation takes place.
		In case the actual required number of fragments of an NDIS packet
		excceeds "NumberOfFrag"caculated here and not enough free TXD available, the
		last fragment (i.e. last MPDU) will be dropped in RTMPHardTransmit() due to out of
		resource, and the NDIS packet will be indicated NDIS_STATUS_FAILURE. This should
		rarely happen and the penalty is just like a TX RETRY fail. Affordable.

		exception:
			a). fragmentation not allowed on multicast & broadcast
			b). Aggregation overwhelms fragmentation (fCLIENT_STATUS_AGGREGATION_CAPABLE)
			c). TSO/CSO not do fragmentation
*/

/* TODO: shiang-usw. we need to modify the TxPktClassificatio to adjust the NumberOfFrag! */
UCHAR get_frag_num(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{
	PACKET_INFO pkt_info;
	UCHAR *src_buf_va;
	UINT32 src_buf_len, frag_sz, pkt_len;
	UCHAR frag_num;

	RTMP_QueryPacketInfo(pPacket, &pkt_info, &src_buf_va, &src_buf_len);
	pkt_len = pkt_info.TotalPacketLength - LENGTH_802_3 + LENGTH_802_1_H;
	frag_sz = wlan_operate_get_frag_thld(wdev);
	frag_sz = frag_sz - LENGTH_802_11 - LENGTH_CRC;

	if (*src_buf_va & 0x01) {
		/* fragmentation not allowed on multicast & broadcast */
		frag_num = 1;
	} else {
		if (pkt_len <= frag_sz)
			frag_num = 1;
		else {
			frag_num = (pkt_len / frag_sz) + 1;

			/* To get accurate number of fragmentation */
			/* Minus 1 if the size just match to allowable fragment size */
			if ((pkt_len % frag_sz) == 0)
				frag_num--;
		}
	}
#ifdef AUTOMATION
	if (pAd->fpga_ctl.txrx_dbg_type == 3) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s[%d]pkt_len=%d,frag_sz=%d,frag_num=%d\n\r",
				__func__, __LINE__, pkt_len, frag_sz, frag_num));
	}
#endif /* AUTOMATION */
	return frag_num;
}

INT32 fp_send_data_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	UCHAR q_idx = QID_AC_BE;
	UCHAR wcid = RTMP_GET_PACKET_WCID(pkt);
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[wcid];
	struct qm_ops *ops = pAd->qm_ops;

	if (wcid != 0)

	if (tr_entry->EntryType == ENTRY_CAT_MCAST)
		RTMP_SET_PACKET_TXTYPE(pkt, TX_MCAST_FRAME);

	q_idx = RTMP_GET_PACKET_QUEIDX(pkt);
	ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
	return NDIS_STATUS_SUCCESS;
}

INT wdev_tx_pkts(NDIS_HANDLE dev_hnd, PPNDIS_PACKET pkt_list, UINT pkt_cnt, struct wifi_dev *wdev)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)dev_hnd;
	PNDIS_PACKET pPacket;
	UCHAR wcid = MAX_LEN_OF_MAC_TABLE;
	UINT Index;
#ifdef REDUCE_TCP_ACK_SUPPORT
	PMAC_TABLE_ENTRY pEntry = NULL;
	PUCHAR pDA = NULL;
#endif

	for (Index = 0; Index < pkt_cnt; Index++) {
		pPacket = pkt_list[Index];

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)
			|| !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY)) {
			/* Drop send request since hardware is in reset state */
#ifdef IXIA_SUPPORT
			if (IS_EXPECTED_LENGTH(pAd, RTPKT_TO_OSPKT(pPacket)->len))
				pAd->tr_ctl.tp_dbg.TxDropPacket[DROP_HW_RESET]++;
#endif /*IXIA_SUPPORT*/
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			continue;
		}

#ifdef ERR_RECOVERY

		if (IsStopingPdma(&pAd->ErrRecoveryCtl)) {
			/* Drop send request since hardware is in reset state */
#ifdef IXIA_SUPPORT
			if (IS_EXPECTED_LENGTH(pAd, RTPKT_TO_OSPKT(pPacket)->len))
				pAd->tr_ctl.tp_dbg.TxDropPacket[PDMA_STOPPING]++;
#endif /*IXIA_SUPPORT*/
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			continue;
		}

#endif /* ERR_RECOVERY */

		if (wdev->forbid_data_tx) {
#ifdef IXIA_SUPPORT
			if (IS_EXPECTED_LENGTH(pAd, RTPKT_TO_OSPKT(pPacket)->len))
				pAd->tr_ctl.tp_dbg.TxDropPacket[FORBID_DATA_TX]++;
#endif /*IXIA_SUPPORT*/
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			continue;
		}
		RTMP_SET_PACKET_WDEV(pPacket, wdev->wdev_idx);
		/*
			WIFI HNAT need to learn packets going to which interface from skb cb setting.
			@20150325
		*/
#if defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI)

		if (ra_sw_nat_hook_tx != NULL) {
#ifdef TCSUPPORT_MT7510_FE

			if (ra_sw_nat_hook_tx(pPacket, NULL, FOE_MAGIC_WLAN) == 0)
#else
			if (ra_sw_nat_hook_tx(pPacket, 1) == 0)
#endif
			{
#ifdef IXIA_SUPPORT
				if (IS_EXPECTED_LENGTH(pAd, RTPKT_TO_OSPKT(pPacket)->len))
					pAd->tr_ctl.tp_dbg.TxDropPacket[BB_SW_NAT]++;
#endif /*IXIA_SUPPORT*/
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				continue;
			}
		}

#endif
#ifdef CONFIG_FAST_NAT_SUPPORT
	if ((ra_sw_nat_hook_tx != NULL)
		) {
			ra_sw_nat_hook_tx(pPacket, 0);
		}


#endif /*CONFIG_FAST_NAT_SUPPORT*/
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		RTMPWakeUpWdev(pAd, wdev);
#endif /* CONFIG_STA_SUPPORT */

    RTMP_SET_PACKET_WCID(pPacket, wcid);

#ifdef REDUCE_TCP_ACK_SUPPORT
		pDA = GET_OS_PKT_DATAPTR(pPacket);
		pEntry = MacTableLookup(pAd, pDA);
		if (pEntry)
			RTMP_SET_PACKET_WCID(pPacket, pEntry->wcid);

		if (!ReduceTcpAck(pAd, pPacket))
#endif /* REDUCE_TCP_ACK_SUPPORT */
		{
#ifndef A4_CONN
#ifdef RT_CFG80211_SUPPORT
			if (RTMP_CFG80211_HOSTAPD_ON(pAd)) {
				UCHAR *pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
				UINT16 TypeLen = 0;
				if (pSrcBuf) {
					TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
					if (TypeLen == ETH_TYPE_EAPOL) {
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL,
						DBG_LVL_ERROR,
						("%s, send EAPOL of length %d from hostapd\n", __func__,
						GET_OS_PKT_LEN(pPacket)));
					}
				}
			}
#endif /* RT_CFG80211_SUPPORT */
			send_data_pkt(pAd, wdev, pPacket);
#else
			if (wdev->wdev_type == WDEV_TYPE_AP) {
				UCHAR *pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);

				if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
					if (MAC_ADDR_IS_GROUP(pSrcBufVA))
						a4_send_clone_pkt(pAd, wdev->func_idx, pPacket, NULL);
				} else {
		/*If IGMP snooping enabled , igmp_pkt_clone will be used for cloning multicast packets on A4 link*/
					if (!wdev->IgmpSnoopEnable && MAC_ADDR_IS_GROUP(pSrcBufVA))
						a4_send_clone_pkt(pAd, wdev->func_idx, pPacket, NULL);
				}
			}
			send_data_pkt(pAd, wdev, pPacket);
#endif /*A4_CONN*/
		}
	}

	return 0;
}

#ifdef TX_AGG_ADJUST_WKR
BOOLEAN tx_check_for_agg_adjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN check_result = FALSE;
	BOOLEAN support_four_stream = FALSE;

	if (pAd->TxAggAdjsut == FALSE)
		return FALSE;

	if (!pEntry)
		return FALSE;

	if (pEntry->vendor_ie.is_rlt == TRUE ||
		pEntry->vendor_ie.is_mtk == TRUE)
		return FALSE;

	if ((pEntry->SupportHTMCS & 0xff000000) != 0)
		support_four_stream = TRUE;

	if (pEntry->SupportVHTMCS4SS != 0)
		support_four_stream = TRUE;

	if (support_four_stream == FALSE)
		check_result = TRUE;

	return check_result;
}
#endif /* TX_AGG_ADJUST_WKR */

VOID tx_bytes_calculate(RTMP_ADAPTER *pAd, TX_BLK *tx_blk)
{
	if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
		(tx_blk->amsdu_state == TX_AMSDU_ID_FIRST))
		tx_blk->tx_bytes_len = sizeof(TMAC_TXD_L);

	tx_blk->tx_bytes_len += tx_blk->MpduHeaderLen +
					tx_blk->HdrPadLen +
					tx_blk->SrcBufLen;

	if (tx_blk->amsdu_state != TX_AMSDU_ID_NO) {

		if (((RTMP_GET_PACKET_PROTOCOL(tx_blk->pPacket) <= 1500) ? 0 : 1))
			tx_blk->tx_bytes_len += 8;

		/* TODO: need to profile to decide if remove vlan or not  */
		if (RTMP_GET_PACKET_VLAN(tx_blk->pPacket)) {
			if (!(tx_blk->wdev->bVLAN_Tag))/*is_rmvl */
				tx_blk->tx_bytes_len -= 4;
			else
				tx_blk->tx_bytes_len += 6;
		}

		if (tx_blk->amsdu_state != TX_AMSDU_ID_LAST) {
			if (tx_blk->tx_bytes_len & 3)
				tx_blk->tx_bytes_len += (4 - (tx_blk->tx_bytes_len & 3));
		}
	}
}

#ifdef REDUCE_TX_OVERHEAD
VOID tx_bytes_calculate2(RTMP_ADAPTER *pAd, TX_BLK *tx_blk)
{
	tx_blk->tx_bytes_len = sizeof(TMAC_TXD_L);

	tx_blk->tx_bytes_len += tx_blk->MpduHeaderLen +
							tx_blk->HdrPadLen +
							tx_blk->SrcBufLen;
}
#endif

BOOLEAN is_udp_packet(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt)
{
	UINT16 TypeLen;
	UCHAR Byte0, Byte1, *pSrcBuf;

	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
	ASSERT(pSrcBuf);

	/* get Ethernet protocol field and skip the Ethernet Header */
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];

	pSrcBuf += LENGTH_802_3;

	if (TypeLen <= 1500) {
		if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03) {
			Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 6, &Byte0, &Byte1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
			pSrcBuf += LENGTH_802_1_H;
		} else {
			return FALSE;
		}
	}

	if (TypeLen == ETH_TYPE_IPv4) {
		UINT32 pktLen = GET_OS_PKT_LEN(pkt);

		ASSERT((pktLen > (ETH_HDR_LEN + IP_HDR_LEN)));

		if (*(pSrcBuf + 9) == IP_PROTO_UDP)
			return TRUE;
		else
			return FALSE;
	} else  {
		return FALSE;
	}
}

#ifdef TXRX_STAT_SUPPORT
BOOLEAN RTMPGetUserPriority(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET	pPacket,
	IN struct wifi_dev *wdev,
	OUT UCHAR *pUserPriority,
	OUT UCHAR *pQueIdx)
{
		UINT16 TypeLen;
		UCHAR Byte0, Byte1, *pSrcBuf, up = 0;
		MAC_TABLE_ENTRY *pMacEntry = NULL;

		*pUserPriority = 0;
		*pQueIdx = 0;

		pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
		ASSERT(pSrcBuf);

		pMacEntry = MacTableLookup(pAd, pSrcBuf);

		/* get Ethernet protocol field and skip the Ethernet Header */
		TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
		pSrcBuf += LENGTH_802_3;

		if (TypeLen <= 1500) {
			/*
				802.3, 802.3 LLC:
					DestMAC(6) + SrcMAC(6) + Lenght(2) +
					DSAP(1) + SSAP(1) + Control(1) +
				if the DSAP = 0xAA, SSAP=0xAA, Contorl = 0x03, it has a 5-bytes SNAP header.
					=> + SNAP (5, OriginationID(3) + etherType(2))
				else
					=> It just has 3-byte LLC header, maybe a legacy ether type frame. we didn't handle it
			*/
			if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03) {
				Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 6, &Byte0, &Byte1);

				TypeLen = (USHORT)((Byte0 << 8) + Byte1);
				pSrcBuf += LENGTH_802_1_H; /* Skip this LLC/SNAP header*/
			} else
				return FALSE;
		}


		switch (TypeLen) {
		case ETH_TYPE_VLAN: {
			Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 2, &Byte0, &Byte1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
#ifdef RTMP_UDMA_SUPPORT
			/*patch for vlan_udma TOS*/

			if (pAd->CommonCfg.bUdmaFlag  == TRUE) {
				if (TypeLen == ETH_TYPE_IPv4) {
					/*For IPV4 packet*/
					up = (*(pSrcBuf + 5) & 0xe0) >> 5;
				} else if (TypeLen == ETH_TYPE_IPv6) {
					/*For IPV6 Packet*/
					up = ((*(pSrcBuf + 4)) & 0x0e) >> 1;
				}
			} else
#endif	/* RTMP_UDMA_SUPPORT */
			{
				/* only use VLAN tag as UserPriority setting */
				up = (*pSrcBuf & 0xe0) >> 5;
			}
			pSrcBuf += LENGTH_802_1Q; /* Skip the VLAN Header.*/
		}
		break;

		case ETH_TYPE_IPv4: {
			/* If it's a VLAN packet, get the real Type/Length field.*/
			/*
			0   4	   8		  14  15		  31(Bits)
			+---+----+-----+----+---------------+
			|Ver |  IHL |DSCP |ECN |    TotalLen 		  |
			+---+----+-----+----+---------------+
			Ver	  - 4bit Internet Protocol version number.
			IHL	  - 4bit Internet Header Length
			DSCP - 6bit Differentiated Services Code Point(TOS)
			ECN	 - 2bit Explicit Congestion Notification
			TotalLen - 16bit IP entire packet length(include IP hdr)
			*/
			up = (*(pSrcBuf + 1) & 0xe0) >> 5;

		}
		break;

		case ETH_TYPE_IPv6: {
			/*
			0	4	8	 12 	16		31(Bits)
			+---+----+----+----+---------------+
			|Ver | TrafficClas |  Flow Label		   |
			+---+----+----+--------------------+
			Ver   - 4bit Internet Protocol version number.
			TrafficClas - 8bit traffic class field, the 6 most-significant bits used for DSCP
			*/
			up = ((*pSrcBuf) & 0x0e) >> 1;

			/* return AC_BE if packet is not IPv6 */
			if ((*pSrcBuf & 0xf0) != 0x60)
				up = 0;
		}
		break;

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		case ETHER_TYPE_TDLS_MMPDU: {
			up = 5;
		}
		break;
#endif /* DOT11Z_TDLS_SUPPORT */

		default:
			break;
		}

		if (up > 7)
			return FALSE;

		if ((pAd->CommonCfg.APEdcaParm[0].bACM[WMM_UP2AC_MAP[up]])
			|| (((wdev->wdev_type == WDEV_TYPE_REPEATER)
				|| (wdev->wdev_type == WDEV_TYPE_STA))
				&& (pMacEntry && pMacEntry->bACMBit[WMM_UP2AC_MAP[up]])))

			up = 0;

		/*
			Set WMM when
			1. wdev->bWmmCapable == TRUE
			2. Receiver's capability
				a). bc/mc packets
					->Need to get UP for IGMP use
				b). unicast packets
					-> CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE)
			3. has VLAN tag or DSCP fields in IPv4/IPv6 hdr
		*/
		if ((wdev->bWmmCapable == TRUE) && (up <= 7)) {
			*pUserPriority = up;
			*pQueIdx = WMM_UP2AC_MAP[up];
		}

		return TRUE;
}


/* Get the status of the current Streaming stataus ( BE, BK, VI, VO) which is getting transmitted throught the AP */
#endif
DECLARE_TIMER_FUNCTION(rx_delay_ctl_algo);

VOID rx_delay_ctl_algo(PVOID SystemSpecific1, PVOID FunctionContext, PVOID SystemSpecific2, PVOID SystemSpecific3)
{
}

BUILD_TIMER_FUNCTION(rx_delay_ctl_algo);

#ifdef CONFIG_TP_DBG
DECLARE_TIMER_FUNCTION(tp_dbg_history);

VOID tp_dbg_history(PVOID SystemSpecific1, PVOID FunctionContext, PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	struct tp_debug *tp_dbg = (struct tp_debug *)FunctionContext;

	tp_dbg->IsrTxCntRec[tp_dbg->time_slot] = tp_dbg->IsrTxCnt;
	tp_dbg->IsrTxCnt = 0;
	tp_dbg->IsrRxCntRec[tp_dbg->time_slot] = tp_dbg->IsrRxCnt;
	tp_dbg->IsrRxCnt = 0;
	tp_dbg->IsrRx1CntRec[tp_dbg->time_slot] = tp_dbg->IsrRx1Cnt;
	tp_dbg->IsrRx1Cnt = 0;
	tp_dbg->IsrRxDlyCntRec[tp_dbg->time_slot] = tp_dbg->IsrRxDlyCnt;
	tp_dbg->IsrRxDlyCnt = 0;
	tp_dbg->IoReadTxRec[tp_dbg->time_slot] = tp_dbg->IoReadTx;
	tp_dbg->IoReadTx = 0;
	tp_dbg->IoWriteTxRec[tp_dbg->time_slot] = tp_dbg->IoWriteTx;
	tp_dbg->IoWriteTx = 0;
	tp_dbg->IoReadRxRec[tp_dbg->time_slot] = tp_dbg->IoReadRx;
	tp_dbg->IoReadRx = 0;
	tp_dbg->IoWriteRxRec[tp_dbg->time_slot] = tp_dbg->IoWriteRx;
	tp_dbg->IoWriteRx = 0;
	tp_dbg->IoReadRx1Rec[tp_dbg->time_slot] = tp_dbg->IoReadRx1;
	tp_dbg->IoReadRx1 = 0;
	tp_dbg->IoWriteRx1Rec[tp_dbg->time_slot] = tp_dbg->IoWriteRx1;
	tp_dbg->IoWriteRx1 = 0;
	tp_dbg->MaxProcessCntRxRecA[tp_dbg->time_slot] = tp_dbg->RxMaxProcessCntA;
	tp_dbg->RxMaxProcessCntA = 0;
	tp_dbg->MaxProcessCntRxRecB[tp_dbg->time_slot] = tp_dbg->RxMaxProcessCntB;
	tp_dbg->RxMaxProcessCntB = 0;
	tp_dbg->MaxProcessCntRxRecC[tp_dbg->time_slot] = tp_dbg->RxMaxProcessCntC;
	tp_dbg->RxMaxProcessCntC = 0;
	tp_dbg->MaxProcessCntRxRecD[tp_dbg->time_slot] = tp_dbg->RxMaxProcessCntD;
	tp_dbg->RxMaxProcessCntD = 0;
	tp_dbg->MaxProcessCntRx1RecA[tp_dbg->time_slot] = tp_dbg->Rx1MaxProcessCntA;
	tp_dbg->Rx1MaxProcessCntA = 0;
	tp_dbg->MaxProcessCntRx1RecB[tp_dbg->time_slot] = tp_dbg->Rx1MaxProcessCntB;
	tp_dbg->Rx1MaxProcessCntB = 0;
	tp_dbg->MaxProcessCntRx1RecC[tp_dbg->time_slot] = tp_dbg->Rx1MaxProcessCntC;
	tp_dbg->Rx1MaxProcessCntC = 0;
	tp_dbg->MaxProcessCntRx1RecD[tp_dbg->time_slot] = tp_dbg->Rx1MaxProcessCntD;
	tp_dbg->Rx1MaxProcessCntD = 0;
	tp_dbg->TRDoneTimesRec[tp_dbg->time_slot] = tp_dbg->TRDoneTimes;
	tp_dbg->TRDoneTimes = 0;
	tp_dbg->time_slot++;
	tp_dbg->time_slot = tp_dbg->time_slot % TP_DBG_TIME_SLOT_NUMS;
}
BUILD_TIMER_FUNCTION(tp_dbg_history);
#endif /* CONFIG_TP_DBG */

INT32 tr_ctl_init(RTMP_ADAPTER *pAd)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tr_flow_control *tr_flow_ctl = &tr_ctl->tr_flow_ctl;
	UINT8 *pTxFlowBlockState = NULL;
	NDIS_SPIN_LOCK *pTxBlockLock = NULL;
	DL_LIST *pTxBlockDevList = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	UINT32 Index;
	ULONG Flags;

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &tr_ctl->tp_dbg;
#endif
	struct tr_delay_control *tr_delay_ctl = &tr_ctl->tr_delay_ctl;

	/* please do not clean tr_ctl data struct.
	   It will disable rx delay interrupt.
	*/
	tr_ctl->rx_icv_err_cnt = 0;

	os_alloc_mem(pAd, (UCHAR **)&pTxFlowBlockState, (num_of_tx_ring) * sizeof(UINT8));

	if (pTxFlowBlockState == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s os_alloc_mem fail\n",
				  __func__));
		return FALSE;
	}

	NdisZeroMemory(pTxFlowBlockState, (num_of_tx_ring) * sizeof(UINT8));
	tr_flow_ctl->TxFlowBlockState = pTxFlowBlockState;
	os_alloc_mem(pAd, (UCHAR **)&pTxBlockLock, (num_of_tx_ring) * sizeof(NDIS_SPIN_LOCK));

	if (pTxBlockLock == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s os_alloc_mem fail\n",
				  __func__));
		return FALSE;
	}

	NdisZeroMemory(pTxBlockLock, (num_of_tx_ring) * sizeof(NDIS_SPIN_LOCK));
	tr_flow_ctl->TxBlockLock = pTxBlockLock;
	os_alloc_mem(pAd, (UCHAR **)&pTxBlockDevList, (num_of_tx_ring) * sizeof(DL_LIST));

	if (pTxBlockDevList == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s os_alloc_mem fail\n",
				  __func__));
		return FALSE;
	}

	NdisZeroMemory(pTxBlockDevList, (num_of_tx_ring) * sizeof(DL_LIST));
	tr_flow_ctl->TxBlockDevList = pTxBlockDevList;

	tr_flow_ctl->RxFlowBlockState = 0;

	for (Index = 0; Index < num_of_tx_ring; Index++) {
		tr_flow_ctl->TxFlowBlockState[Index] = 0;
		NdisAllocateSpinLock(pAd, &tr_flow_ctl->TxBlockLock[Index]);
		RTMP_SPIN_LOCK_IRQSAVE(&tr_flow_ctl->TxBlockLock[Index], &Flags);
		DlListInit(&tr_flow_ctl->TxBlockDevList[Index]);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&tr_flow_ctl->TxBlockLock[Index], &Flags);
	}

	tr_flow_ctl->IsTxBlocked = FALSE;

#ifdef CONFIG_TP_DBG
	NdisZeroMemory(tp_dbg, sizeof(struct tp_debug));
	tp_dbg->debug_flag = (TP_DEBUG_ISR);
	RTMPInitTimer(pAd, &tp_dbg->tp_dbg_history_timer, GET_TIMER_FUNCTION(tp_dbg_history), tp_dbg, TRUE);
	RTMPSetTimer(&tp_dbg->tp_dbg_history_timer, 1000);
#endif
	RTMPInitTimer(pAd, &tr_delay_ctl->rx_delay_timer, GET_TIMER_FUNCTION(rx_delay_ctl_algo), tr_delay_ctl, TRUE);
	RTMPSetTimer(&tr_delay_ctl->rx_delay_timer, 1000);
	return 0;
}

INT32 tr_ctl_exit(RTMP_ADAPTER *pAd)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tr_delay_control *tr_delay_ctl = &tr_ctl->tr_delay_ctl;
	struct tr_flow_control *tr_flow_ctl = &tr_ctl->tr_flow_ctl;
	TX_BLOCK_DEV *TxBlockDev = NULL;
	UINT32 Index;
	BOOLEAN cancelled;
#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &tr_ctl->tp_dbg;
#endif
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	for (Index = 0; Index < GET_NUM_OF_TX_RING(cap); Index++) {
		RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[Index]);

		while (1) {
			TxBlockDev = DlListFirst(&tr_flow_ctl->TxBlockDevList[Index], TX_BLOCK_DEV, list);

			if (!TxBlockDev)
				break;

			DlListDel(&TxBlockDev->list);
			RTMP_OS_NETDEV_WAKE_QUEUE(TxBlockDev->NetDev);
			os_free_mem(TxBlockDev);
		}

		RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[Index]);
		NdisFreeSpinLock(&tr_flow_ctl->TxBlockLock[Index]);
	}

	os_free_mem(tr_flow_ctl->TxFlowBlockState);
	os_free_mem(tr_flow_ctl->TxBlockLock);
	os_free_mem(tr_flow_ctl->TxBlockDevList);

	RTMPReleaseTimer(&tr_delay_ctl->rx_delay_timer, &cancelled);

#ifdef CONFIG_TP_DBG
	RTMPReleaseTimer(&tp_dbg->tp_dbg_history_timer, &cancelled);
#endif

	return 0;
}

INT32 tx_flow_block(RTMP_ADAPTER *pAd, PNET_DEV NetDev, UINT8 State, BOOLEAN Block, UINT8 RingIdx)
{
	INT32 Ret = 0;
	struct tr_flow_control *tr_flow_ctl = &pAd->tr_ctl.tr_flow_ctl;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);

	if (Block == TRUE) {
		TX_BLOCK_DEV *TxBlockDev = NULL;

		RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
		tr_flow_ctl->TxFlowBlockState[RingIdx] |= State;
		DlListForEach(TxBlockDev, &tr_flow_ctl->TxBlockDevList[RingIdx], TX_BLOCK_DEV, list) {
			if (TxBlockDev->NetDev == NetDev) {
				RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
				return Ret;
			}
		}

		os_alloc_mem(NULL, (PUCHAR *)&TxBlockDev, sizeof(*TxBlockDev));

		if (!TxBlockDev) {
			MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
					 ("can not allocate TX_BLOCK_DEV\n"));
			RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
			return -1;
		}

		TxBlockDev->NetDev = NetDev;
		DlListAddTail(&tr_flow_ctl->TxBlockDevList[RingIdx], &TxBlockDev->list);
		RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
		RTMP_OS_NETDEV_STOP_QUEUE(NetDev);
	} else {
		TX_BLOCK_DEV *TxBlockDev = NULL;

		if (RingIdx == num_of_tx_ring) {
			UINT8 Idx = 0;

			for (Idx = 0; Idx < num_of_tx_ring; Idx++) {
				RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[Idx]);
				tr_flow_ctl->TxFlowBlockState[Idx] &= ~State;

				if (tr_flow_ctl->TxFlowBlockState[Idx] != 0) {
					RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[Idx]);
					continue;
				}

				while (1) {
					TxBlockDev = DlListFirst(&tr_flow_ctl->TxBlockDevList[Idx], TX_BLOCK_DEV, list);

					if (!TxBlockDev)
						break;

					DlListDel(&TxBlockDev->list);
					RTMP_OS_NETDEV_WAKE_QUEUE(TxBlockDev->NetDev);
					os_free_mem(TxBlockDev);
				}

				RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[Idx]);
			}
		} else {
			RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
			tr_flow_ctl->TxFlowBlockState[RingIdx] &= ~State;

			if (tr_flow_ctl->TxFlowBlockState[RingIdx] != 0) {
				RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
				return Ret;
			}

			while (1) {
				TxBlockDev = DlListFirst(&tr_flow_ctl->TxBlockDevList[RingIdx], TX_BLOCK_DEV, list);

				if (!TxBlockDev)
					break;

				DlListDel(&TxBlockDev->list);
				RTMP_OS_NETDEV_WAKE_QUEUE(TxBlockDev->NetDev);
				os_free_mem(TxBlockDev);
			}

			RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
		}
	}

	return Ret;
}

static inline void tx_flow_block_all_netdev(RTMP_ADAPTER *pAd, BOOLEAN Block)
{
	INT32 wdev_idx = 0;
	struct wifi_dev *wdev = NULL;

	if (Block) {
		for (wdev_idx = 0; wdev_idx < WDEV_NUM_MAX; wdev_idx++) {
			wdev = pAd->wdev_list[wdev_idx];
			if (wdev && (wdev->if_dev))
				RTMP_OS_NETDEV_STOP_QUEUE(wdev->if_dev);
		}
	} else {
		for (wdev_idx = 0; wdev_idx < WDEV_NUM_MAX; wdev_idx++) {
			wdev = pAd->wdev_list[wdev_idx];
			if (wdev && (wdev->if_dev))
				RTMP_OS_NETDEV_WAKE_QUEUE(wdev->if_dev);
		}
	}
}

INT32 tx_flow_set_state_block(RTMP_ADAPTER *pAd, PNET_DEV NetDev, UINT8 State, BOOLEAN Block, UINT8 RingIdx)
{
	INT32 Ret = 0;
	struct tr_flow_control *tr_flow_ctl = &pAd->tr_ctl.tr_flow_ctl;

	if (Block == TRUE) {
		RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
		tr_flow_ctl->TxFlowBlockState[RingIdx] |= State;
		RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
		/* Stop device because this queue got congestion */
		/* stop all bss if no Netdev input */
		if (NetDev)
			RTMP_OS_NETDEV_STOP_QUEUE(NetDev);
		else
			tx_flow_block_all_netdev(pAd, TRUE);

		tr_flow_ctl->IsTxBlocked = TRUE;
		pAd->tr_ctl.net_if_stop_cnt++;
	} else {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
		UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);

		RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
		tr_flow_ctl->TxFlowBlockState[RingIdx] &= ~State;
		RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
		/* Wake up device if all queue are available */
		if (!tx_flow_check_state(pAd, State, num_of_tx_ring)) {
			/* Wake up all bss if no Netdev input */
			if (NetDev)
				RTMP_OS_NETDEV_WAKE_QUEUE(NetDev);
			else
				tx_flow_block_all_netdev(pAd, FALSE);
		} /* !tx_flow_check_state */

		tr_flow_ctl->IsTxBlocked = FALSE;
	}

	return Ret;
}

inline BOOLEAN tx_flow_check_if_blocked(RTMP_ADAPTER *pAd)
{
	return pAd->tr_ctl.tr_flow_ctl.IsTxBlocked;
}

BOOLEAN tx_flow_check_state(RTMP_ADAPTER *pAd, UINT8 State, UINT8 RingIdx)
{
	UINT8 Idx;
	struct tr_flow_control *tr_flow_ctl = &pAd->tr_ctl.tr_flow_ctl;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);

	if (RingIdx == num_of_tx_ring) {
		for (Idx = 0; Idx < num_of_tx_ring; Idx++) {
			RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[Idx]);

			if ((tr_flow_ctl->TxFlowBlockState[Idx] & State) == State) {
				RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[Idx]);
				return TRUE;
			}

			RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[Idx]);
		}
	} else {
		RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);

		if ((tr_flow_ctl->TxFlowBlockState[RingIdx] & State) == State) {
			RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
			return TRUE;
		}

		RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
	}

	return FALSE;
}


#if defined(MT_MAC) && defined(VHT_TXBF_SUPPORT)
VOID Mumimo_scenario_maintain(
	IN PRTMP_ADAPTER pAd)
{
#ifdef CFG_SUPPORT_MU_MIMO
	UCHAR               ucWlanIdx = 0, ucWdevice = 0;
	UCHAR               ucMUNum = 0;
	struct wifi_dev     *pWdev = NULL;
	PMAC_TABLE_ENTRY    pEntry;
	BOOL                fgHitMUTxOPCondition = FALSE;
#ifdef DYNAMIC_STEERING_LOADING
	if (pAd->disable_auto_txop == 1)
		return;
#endif
	for (ucWlanIdx = 1; ucWlanIdx <= pAd->MacTab.Size; ucWlanIdx++) {

	pEntry = &pAd->MacTab.Content[ucWlanIdx];

	if (pEntry->rStaRecBf.fgSU_MU) {
		ucMUNum++;
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("at 1\n"));
	}

	if (ucMUNum >= 2) {
		fgHitMUTxOPCondition = TRUE;
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("at 2\n"));
		break;
		}
	}

	/* If Hit Trigger Condition, assign TxOP=0xC0 for 3/4 MU-MIMO Peak */
	if (fgHitMUTxOPCondition) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("value of pAd->MUMIMO_TxOP_Value = %d\n", pAd->MUMIMO_TxOP_Value));
		if (pAd->MUMIMO_TxOP_Value != TXOP_C0) {
		for (ucWdevice = 0; ucWdevice < WDEV_NUM_MAX; ucWdevice++) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("at 3\n"));
			pWdev = pAd->wdev_list[ucWdevice];
			if (!pWdev)
				continue;
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("at 4\n"));
			pAd->MUMIMO_TxOP_Value = TXOP_C0;
			enable_tx_burst(pAd, pWdev, AC_BE, PRIO_MU_MIMO, TXOP_C0);
			}
		}
	} else {
		if (pAd->MUMIMO_TxOP_Value != TXOP_0) {
			for (ucWdevice = 0; ucWdevice < WDEV_NUM_MAX; ucWdevice++) {
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("at 5\n"));
				pWdev = pAd->wdev_list[ucWdevice];
				if (!pWdev)
					continue;
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("at 6\n"));
				pAd->MUMIMO_TxOP_Value = TXOP_0;
				disable_tx_burst(pAd, pWdev, AC_BE, PRIO_MU_MIMO, TXOP_0);
			}
		}
	}
/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,*/
/*("MU TxOP = %d, MU STA = %d, MU CW Min %d\n", pAd->MUMIMO_TxOP_Value, ucMUNum, u4EdcaCWmin));*/
	return;
#else
	return;
#endif
}

#endif /* MT_MAC && VHT_TXBF_SUPPORT */
#ifdef IXIA_SUPPORT
char *rdrop_reason[MAX_RDROP_RESON] = {
	"RPKT_SUCCESS",
	"ALREADY_IN_ORDER",
	"DUP_SEQ_PKT_EQ",
	"DUP_SEQ_PKT_SMALL",
	"DROP_DATA_SIZE",
	"DROP_RXD_ERROR",
	"DROP_BA_STAT_ERROR",
	"DROP_NOT_SEC",
	"DROP_NO_EAP",
	"DROP_MWDS",
	"DROP_WDEV_INDEX",
	"DROP_WDEV_INDEX1",
	"DROP_FC_NULL",
	"DROP_RX_ORDER",
	"DROP_RX_HALT",
	"DROP_NOTTO_OS",
	"DROP_PN_ERROR",
	"DROP_NOT_TOOS",
	"DROP_MSDU_REL",
	"DROP_MPDU_REL",
	"DROP_RXLEN_DISMATCH",
	"DROP_RXLEN_DISMATCH1",
	"DROP_RX_NOT_ALLOW",
	"DROP_RX_NOT_ALLOW8023",
	"DROP_RX_NOT_ALLOW80211",
	"DROP_RX_TYPE_ERR",
	"DROP_RX_DMA_BZ",
	"DROP_RX_CUT",
	"DROP_RXSWENC_FAIL",
	"DROP_RXMAC_LEN",
};
char *tdrop_reason[MAX_TDROP_RESON] = {
	"PKT_SUCCESS",
	"INVALID_TR_WCID",
	"INVALID_OP_STAT",
	"INVALID_WDEV",
	"INVALID_PKT_TYPE",
	"DROP_PSQ_FULL",
	"DROP_TXQ_FULL",
	"DROP_TXQ_ENQ_FAIL",
	"DROP_TXQ_ENQ_PS",
	"DROP_HW_RESET",
	"DROP_80211H_MODE",
	"DROP_TXBLK_FAIL",
	"DROP_TxSWEncrypt_FAIL",
	"DROP_TXVOWTXD_FAIL",
	"DROP_TXVOWDROP",
	"DROP_TXRED",
	"DROP_TXHIQ_EXCD",
	"DROP_RRM_QUIET",
	"INVALID_WDEV2",
	"DROP_ENQ_FAIL",
	"BB_SW_NAT",
	"FORBID_DATA_TX",
	"PDMA_STOPPING",
	"WIFI_PKT_FWD",
	"DROP_TXP_FAIL",
	"FRAG_ID_ERROR",
	"NO_HW_RESOURCE",
	"NO_HW_RESOURCE_HI",
};
char *txType[TX_TYPE_MAX] = {
	"APD",
	"ASD",
	"LGY",
	"FRG",
};

void wifi_txrx_parmtrs_dump(RTMP_ADAPTER *pAd)
{
	UINT8 idx = 0;
	/******************** TX ******************************/
	if (pAd->tr_ctl.tp_dbg.tx_pkt_from_os >= 500) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("************ 7613TX ************\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("tx_pkt_len    : %d .\n", pAd->tr_ctl.tp_dbg.tx_pkt_len));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("tx_pkt_from_os: %ld .\n", pAd->tr_ctl.tp_dbg.tx_pkt_from_os));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("tx_pkt_enq_cnt: %ld .\n", pAd->tr_ctl.tp_dbg.tx_pkt_enq_cnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("tx_pkt_deq_cnt: %ld .\n", pAd->tr_ctl.tp_dbg.tx_pkt_deq_cnt));
		for (idx = 0; idx < TX_TYPE_MAX; idx++) {
			if (pAd->tr_ctl.tp_dbg.tx_pkt_to_hw[idx] != 0)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s-tx_pkt_to_hw  : %d .\n",
				txType[idx], pAd->tr_ctl.tp_dbg.tx_pkt_to_hw[idx]));
		}
		for (idx = 1; idx < MAX_TDROP_RESON; idx++) {
			if (pAd->tr_ctl.tp_dbg.TxDropPacket[idx] != 0)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("TX Drop Reason:%s,Drop Cnt:%d\n",
				tdrop_reason[idx], pAd->tr_ctl.tp_dbg.TxDropPacket[idx]));
		}
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\tsw_q_drop cnt = %d\n", pAd->tr_ctl.tx_sw_q_drop));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\tnet_if_stop_cnt = %d\n", pAd->tr_ctl.net_if_stop_cnt));
#ifdef FQ_SCH_SUPPORT
		for (idx = 0; idx < WMM_NUM_OF_AC; idx++) {
			if (pAd->fq_ctrl.drop_cnt[idx] != 0)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC[%d] drop cnt = %d\n", idx, pAd->fq_ctrl.drop_cnt[idx]));
			pAd->fq_ctrl.drop_cnt[idx] = 0;
		}
#endif
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\tTxSwQMaxLen = %d\n", pAd->TxSwQMaxLen));
#ifdef REDUCE_TX_OVERHEAD
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\tcachedInCnt = %d\n", pAd->cached_inCnt));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\tcachedOutCnt = %d\n", pAd->cached_outCnt));
		pAd->cached_inCnt = 0;
		pAd->cached_outCnt = 0;
#endif
		pAd->tr_ctl.tp_dbg.tx_pkt_len = 0;
		pAd->tr_ctl.tp_dbg.tx_pkt_from_os = 0;
		pAd->tr_ctl.tp_dbg.tx_pkt_enq_cnt = 0;
		pAd->tr_ctl.tp_dbg.tx_pkt_deq_cnt = 0;
		NdisZeroMemory(pAd->tr_ctl.tp_dbg.tx_pkt_to_hw,
			(TX_TYPE_MAX * sizeof(UINT)));
		pAd->tr_ctl.tx_sw_q_drop = 0;
		pAd->tr_ctl.net_if_stop_cnt = 0;
		NdisZeroMemory(pAd->tr_ctl.tp_dbg.TxDropPacket,
			(MAX_TDROP_RESON * sizeof(UINT32)));
		ge_bss_clean_queue_ixia(pAd);
	}
	/******************** RX ******************************/
	if (pAd->RalinkCounters.RxCount >= 500) {
		ba_reordering_list_pkts_release(pAd);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("************ 7613RX ************\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("rx_pkt_len    : %d .\n", pAd->tr_ctl.tp_dbg.rx_pkt_len));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("rx_pkt_from_hw: %d .\n", pAd->RalinkCounters.RxCount));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("rx_pkt_to_os  : %d .\n", rx_pkt_to_os));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("rxStepOneCnt: %d .\n", pAd->tr_ctl.tp_dbg.rxStepOneCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("rxWithinCnt: %d .\n", pAd->tr_ctl.tp_dbg.rxWithinCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("rxSurpassCnt: %d .\n", pAd->tr_ctl.tp_dbg.rxSurpassCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("rxListRlsCnt: %d .\n", pAd->tr_ctl.tp_dbg.rxListRlsCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("IoWriteRx	: %d .\n", pAd->tr_ctl.tp_dbg.IoWriteRx));
		if (pAd->Counters8023.RxErrors)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("rxd error drop: %d .\n", pAd->Counters8023.RxErrors));
		for (idx = 1; idx < MAX_RDROP_RESON; idx++) {
			if (pAd->tr_ctl.tp_dbg.RxDropPacketCnt[idx] != 0)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("RX Drop Reason:%s,Drop Cnt:%d\n",
				rdrop_reason[idx], pAd->tr_ctl.tp_dbg.RxDropPacketCnt[idx]));
		}
		pAd->tr_ctl.tp_dbg.IoWriteRx = 0;
		NdisZeroMemory(pAd->tr_ctl.tp_dbg.RxDropPacketCnt,
			(MAX_RDROP_RESON * sizeof(UINT32)));
		pAd->tr_ctl.tp_dbg.rx_pkt_len = 0;
		rx_pkt_to_os = 0;
		pAd->Counters8023.RxErrors = 0;
		pAd->RalinkCounters.RxCount = 0;
		pAd->tr_ctl.tp_dbg.rxListRlsCnt = 0;
		pAd->tr_ctl.tp_dbg.rxStepOneCnt = 0;
		pAd->tr_ctl.tp_dbg.rxWithinCnt = 0;
		pAd->tr_ctl.tp_dbg.rxSurpassCnt = 0;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("rxNBDS Dect Pkts  : %d .\n", pAd->NbdsDctCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("rxNBDS Drop Pkts  : %d .\n", pAd->NbdsDelCnt));
	pAd->NbdsDelCnt = 0;
	pAd->NbdsDctCnt = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("TxSch:%d\n", pAd->tx_dequeue_scheduable));
}
#endif /*IXIA_SUPPORT*/
