/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2017, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/

#include "rt_config.h"

UCHAR tmi_rate_map_cck_lp[] = {
	TMI_TX_RATE_CCK_1M_LP,
	TMI_TX_RATE_CCK_2M_LP,
	TMI_TX_RATE_CCK_5M_LP,
	TMI_TX_RATE_CCK_11M_LP,
};

UCHAR tmi_rate_map_cck_lp_size = ARRAY_SIZE(tmi_rate_map_cck_lp);

UCHAR tmi_rate_map_cck_sp[] = {
	TMI_TX_RATE_CCK_2M_SP,
	TMI_TX_RATE_CCK_5M_SP,
	TMI_TX_RATE_CCK_11M_SP,
};

UCHAR tmi_rate_map_cck_sp_size = ARRAY_SIZE(tmi_rate_map_cck_sp);

UCHAR tmi_rate_map_ofdm[] = {
	TMI_TX_RATE_OFDM_6M,
	TMI_TX_RATE_OFDM_9M,
	TMI_TX_RATE_OFDM_12M,
	TMI_TX_RATE_OFDM_18M,
	TMI_TX_RATE_OFDM_24M,
	TMI_TX_RATE_OFDM_36M,
	TMI_TX_RATE_OFDM_48M,
	TMI_TX_RATE_OFDM_54M,
};

UCHAR tmi_rate_map_ofdm_size = ARRAY_SIZE(tmi_rate_map_ofdm);

const UCHAR altx_filter_list[] = {
	SUBTYPE_ASSOC_REQ,
	SUBTYPE_ASSOC_RSP,
	SUBTYPE_REASSOC_REQ,
	SUBTYPE_REASSOC_RSP,
	SUBTYPE_PROBE_REQ,
	SUBTYPE_PROBE_RSP,
	SUBTYPE_ATIM,
	SUBTYPE_DISASSOC,
	SUBTYPE_AUTH,
};

char *pkt_ft_str[] = {"cut_through", "store_forward", "cmd", "PDA_FW_Download"};
char *hdr_fmt_str[] = {
	"Non-80211-Frame",
	"Command-Frame",
	"Normal-80211-Frame",
	"enhanced-80211-Frame",
};

BOOLEAN in_altx_filter_list(UCHAR sub_type)
{
	UCHAR i;

	for (i = 0; i < (ARRAY_SIZE(altx_filter_list)); i++) {
		if (sub_type == altx_filter_list[i])
			return TRUE;
	}

	return FALSE;
}

INT mt_ct_check_hw_resource(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR resource_idx)
{
	UINT free_num;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = &hif->TxRing[resource_idx];
	PKT_TOKEN_CB *pktTokenCb;

	free_num = pci_get_tx_resource_free_num_nolock(pAd, resource_idx);

	if (free_num < tx_ring->tx_ring_low_water_mark) {
		pci_inc_resource_full_cnt(pAd, resource_idx);
		pci_set_resource_state(pAd, resource_idx, TX_RING_LOW);
		return NDIS_STATUS_RESOURCES;
	}

	pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
	free_num = pktTokenCb->tx_id_list.list->FreeTokenCnt;

	if (free_num < pktTokenCb->TxTokenLowWaterMark) {
		cut_through_inc_token_full_cnt(pAd);
		cut_through_set_token_state(pAd, TX_TOKEN_LOW);
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

INT mt_ct_get_hw_resource_free_num(RTMP_ADAPTER *pAd, UCHAR resource_idx, UINT32 *free_num, UINT32 *free_token)
{
	UINT free_ring_num, free_token_num;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = &hif->TxRing[resource_idx];
	PKT_TOKEN_CB *pktTokenCb;

	free_ring_num = pci_get_tx_resource_free_num_nolock(pAd, resource_idx);

	if (free_ring_num < tx_ring->tx_ring_low_water_mark) {
		pci_inc_resource_full_cnt(pAd, resource_idx);
		pci_set_resource_state(pAd, resource_idx, TX_RING_LOW);
		return NDIS_STATUS_RESOURCES;
	}
	free_ring_num = free_ring_num - tx_ring->tx_ring_low_water_mark + 1;


	pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
	free_token_num = pktTokenCb->tx_id_list.list->FreeTokenCnt;

	if (free_token_num < pktTokenCb->TxTokenLowWaterMark) {
		cut_through_inc_token_full_cnt(pAd);
		cut_through_set_token_state(pAd, TX_TOKEN_LOW);
		return NDIS_STATUS_FAILURE;
	}

	free_token_num = free_token_num - pktTokenCb->TxTokenLowWaterMark + 1;

	/* return free num which can be used */
	*free_num = (free_ring_num < free_token_num) ? free_ring_num:free_token_num;
	*free_token = free_token_num;
	return NDIS_STATUS_SUCCESS;
}

INT mt_ct_hw_tx(RTMP_ADAPTER *pAd, TX_BLK *tx_blk)
{
	USHORT free_cnt = 1;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (!TX_BLK_TEST_FLAG(tx_blk, fTX_MCU_OFFLOAD))
		tx_bytes_calculate(pAd, tx_blk);

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS)) {

		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			asic_write_tmac_info(pAd, &tx_blk->HeaderBuf[0], tx_blk);

		ret = asic_write_txp_info(pAd, &tx_blk->HeaderBuf[cap->tx_hw_hdr_len], tx_blk);

		if (ret != NDIS_STATUS_SUCCESS)
			return ret;

#ifdef RED_SUPPORT_BY_HOST
		if (!pAd->red_have_cr4)
			red_record_data(pAd, tx_blk->Wcid, tx_blk->pPacket);
#endif

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		/* if needr_to_drop_counter > 0 then skip to update PDMA descritpor */
		if (pAd->vow_need_drop_cnt[tx_blk->Wcid]) {
			return ret;
		}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */

		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			asic_write_tx_resource(pAd, tx_blk, TRUE, &free_cnt);
	} else {
		INT dot11_meta_hdr_len, tx_hw_hdr_len;
		PNDIS_PACKET pkt;
		NDIS_STATUS status;
		UCHAR *dot11_hdr;
		TX_BLK new_tx_blk, *p_new_tx_blk = &new_tx_blk;

		NdisCopyMemory(p_new_tx_blk, tx_blk, sizeof(*tx_blk));
		dot11_meta_hdr_len = tx_blk->MpduHeaderLen + tx_blk->HdrPadLen;
		tx_hw_hdr_len = cap->tx_hw_hdr_len;
		dot11_hdr = &tx_blk->HeaderBuf[tx_hw_hdr_len];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s():DataFrm, MpduHdrL=%d,WFHdrL=%d,HdrPadL=%d,HwRsvL=%d, NeedCopyHdrLen=%d\n",
				  __func__, tx_blk->MpduHeaderLen, tx_blk->wifi_hdr_len, tx_blk->HdrPadLen,
				  tx_blk->hw_rsv_len, dot11_meta_hdr_len));
		/* original packet only 802.3 payload, so create a new packet including 802.11 header for cut-through transfer */
		status = RTMPAllocateNdisPacket(pAd, &pkt,
										dot11_hdr, dot11_meta_hdr_len,
										tx_blk->pSrcBufData, tx_blk->SrcBufLen);

		if (status != NDIS_STATUS_SUCCESS)
			return NDIS_STATUS_FAILURE;

		if ((tx_blk->FragIdx == TX_FRAG_ID_NO) || (tx_blk->FragIdx == TX_FRAG_ID_LAST)) {
#ifdef IXIA_SUPPORT
			if (IS_EXPECTED_LENGTH(pAd, RTPKT_TO_OSPKT(tx_blk->pPacket)->len))
				pAd->tr_ctl.tp_dbg.TxDropPacket[FRAG_ID_ERROR]++;
#endif /*IXIA_SUPPORT*/
			RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_SUCCESS);
		}
		p_new_tx_blk->HeaderBuf = asic_get_hif_buf(pAd, p_new_tx_blk, tx_blk->resource_idx, tx_blk->TxFrameType);
		p_new_tx_blk->pPacket = pkt;
		p_new_tx_blk->pSrcBufData = GET_OS_PKT_DATAPTR(pkt);
		p_new_tx_blk->SrcBufLen = GET_OS_PKT_LEN(pkt);
		TX_BLK_SET_FLAG(p_new_tx_blk, fTX_CT_WithTxD);

		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			asic_write_tmac_info(pAd, &p_new_tx_blk->HeaderBuf[0], p_new_tx_blk);

		/* because 802.11 header already in new packet, not in HeaderBuf, so set below parameters to 0 */
		p_new_tx_blk->MpduHeaderLen = 0;
		p_new_tx_blk->wifi_hdr_len = 0;
		p_new_tx_blk->HdrPadLen = 0;
		p_new_tx_blk->hw_rsv_len = 0;

		tx_blk->resource_idx = p_new_tx_blk->resource_idx;
		ret = asic_write_txp_info(pAd, &p_new_tx_blk->HeaderBuf[cap->tx_hw_hdr_len], p_new_tx_blk);

		if (ret != NDIS_STATUS_SUCCESS)
			return ret;

		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			asic_write_tx_resource(pAd, p_new_tx_blk, TRUE, &free_cnt);
	}

	return NDIS_STATUS_SUCCESS;
}

INT mt_sf_hw_tx(RTMP_ADAPTER *pAd, TX_BLK *tx_blk)
{
	USHORT free_cnt;

	asic_write_tmac_info(pAd, &tx_blk->HeaderBuf[0], tx_blk);
	asic_write_tx_resource(pAd, tx_blk, TRUE, &free_cnt);
	return NDIS_STATUS_SUCCESS;
}
INT mt_ct_mlme_hw_tx(RTMP_ADAPTER *pAd, UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *transmit, TX_BLK *tx_blk)
{
	UINT16 free_cnt = 1;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	asic_write_tmac_info_fixed_rate(pAd, tmac_info, info, transmit);
	tx_blk->pSrcBufData = tx_blk->pSrcBufHeader;
	NdisCopyMemory(&tx_blk->HeaderBuf[0], tx_blk->pSrcBufData, cap->tx_hw_hdr_len);
	tx_blk->pSrcBufData += cap->tx_hw_hdr_len;
	tx_blk->SrcBufLen -= cap->tx_hw_hdr_len;
	tx_blk->MpduHeaderLen = 0;
	tx_blk->wifi_hdr_len = 0;
	tx_blk->HdrPadLen = 0;
	tx_blk->hw_rsv_len = 0;

	ret = asic_write_txp_info(pAd, &tx_blk->HeaderBuf[cap->tx_hw_hdr_len], tx_blk);

	if (ret != NDIS_STATUS_SUCCESS)
		return ret;

	asic_write_tx_resource(pAd, tx_blk, TRUE, &free_cnt);
	return NDIS_STATUS_SUCCESS;
}

#ifdef CONFIG_ATE
INT32 mt_ct_ate_hw_tx(RTMP_ADAPTER *pAd, TMAC_INFO *info, TX_BLK *tx_blk)
{
	UINT16 free_cnt = 1;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MtWriteTMacInfo(pAd, tx_blk->pSrcBufHeader, info);
	tx_blk->pSrcBufData = tx_blk->pSrcBufHeader;
	NdisCopyMemory(&tx_blk->HeaderBuf[0], tx_blk->pSrcBufData, cap->tx_hw_hdr_len);
	tx_blk->pSrcBufData += cap->tx_hw_hdr_len;
	tx_blk->SrcBufLen -= cap->tx_hw_hdr_len;
	tx_blk->MpduHeaderLen = 0;
	tx_blk->wifi_hdr_len = 0;
	tx_blk->HdrPadLen = 0;
	tx_blk->hw_rsv_len = 0;
	/* Indicate which Tx ring to be sent by band_idx */
	ret = asic_write_txp_info(pAd, &tx_blk->HeaderBuf[cap->tx_hw_hdr_len], tx_blk);

	if (ret != NDIS_STATUS_SUCCESS)
		return ret;

	asic_write_tx_resource(pAd, tx_blk, TRUE, &free_cnt);
	return NDIS_STATUS_SUCCESS;
}
#endif
