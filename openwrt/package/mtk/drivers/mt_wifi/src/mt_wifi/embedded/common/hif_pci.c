/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2012, MediaTek, Inc.
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

#ifdef RX_SCATTER
static INT rx_scatter_info(
	IN struct hif_pci_rx_ring *pRxRing,
	IN RXD_STRUC * pRxD,
	OUT UINT * pPktSize);

static INT rx_scatter_gather(
	RTMP_ADAPTER *pAd,
	UCHAR hif_idx,
	RTMP_DMACB *pRxCell,
	RXD_STRUC *pRxD,
	UINT scatterCnt,
	UINT pktSize,
	UINT *skb_data_len);

#ifdef MT7626_E2_SUPPORT
static INT rx2_scatter_gather(
	RTMP_ADAPTER *pAd,
	UCHAR hif_idx,
	RTMP_DMACB * pRxCell,
	RXD_STRUC * pRxD,
	UINT scatterCnt,
	UINT pktSize,
	UINT *skb_data_len);
#endif
#endif /* RX_SCATTER */

VOID dump_txd(RTMP_ADAPTER *pAd, TXD_STRUC *pTxD)
{
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("TxD:\n"));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDPtr0=0x%x\n", pTxD->SDPtr0));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDLen0=0x%x\n", pTxD->SDLen0));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tLastSec0=0x%x\n", pTxD->LastSec0));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDPtr1=0x%x\n", pTxD->SDPtr1));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDLen1=0x%x\n", pTxD->SDLen1));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tLastSec1=0x%x\n", pTxD->LastSec1));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tDMADONE=0x%x\n", pTxD->DMADONE));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tBurst=0x%x\n", pTxD->Burst));
}


VOID dump_rxd(RTMP_ADAPTER *pAd, RXD_STRUC *pRxD)
{
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("RxD:\n"));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDPtr0/SDLen0/LastSec0=0x%x/0x%x/0x%x\n",
			 pRxD->SDP0, pRxD->SDL0, pRxD->LS0));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDPtr1/SDLen1/LastSec1=0x%x/0x%x/0x%x\n",
			 pRxD->SDP1, pRxD->SDL1, pRxD->LS1));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tDDONE=0x%x\n", pRxD->DDONE));
}


VOID dumpTxRing(RTMP_ADAPTER *pAd, INT ring_idx)
{
	RTMP_DMABUF *pDescRing;
	struct hif_pci_tx_ring *pTxRing;
	TXD_STRUC *pTxD;
	int index;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	ASSERT(ring_idx < num_of_tx_ring);
	pDescRing = (RTMP_DMABUF *)hif->TxRing[ring_idx].desc_ring.AllocVa;
	pTxRing = &hif->TxRing[ring_idx];

	for (index = 0; index < tx_ring_size; index++) {
		pTxD = (TXD_STRUC *)pTxRing->Cell[index].AllocVa;
		hex_dump("Dump TxDesc", (UCHAR *)pTxD, sizeof(TXD_STRUC));
		dump_txd(pAd, pTxD);
	}
}


BOOLEAN MonitorTxRing(RTMP_ADAPTER *pAd)
{
	UINT32 Value;

	if (pAd->TxDMACheckTimes < 10) {
		/* Check if TX DMA busy */
		HIF_IO_READ32(pAd->hdev_ctrl, MT_WPDMA_GLO_CFG, &Value);

		if ((Value & TX_DMA_BUSY) == TX_DMA_BUSY) {
			/* Check TX FIFO if have space */
			HIF_IO_WRITE32(pAd->hdev_ctrl, 0x4244, 0x98000000);
			HIF_IO_READ32(pAd->hdev_ctrl, 0x4244, &Value);

			if ((Value & (1 << 8)) == 0) {
				pAd->TxDMACheckTimes = 0;
				return FALSE;
			} else {
				pAd->TxDMACheckTimes++;
				return FALSE;
			}
		} else {
			pAd->TxDMACheckTimes = 0;
			return FALSE;
		}
	} else {
		pAd->TxDMACheckTimes = 0;
		return TRUE;
	}
}


BOOLEAN MonitorRxRing(RTMP_ADAPTER *pAd)
{
	UINT32 Value;

	if (pAd->RxDMACheckTimes < 10) {
		/* Check if RX DMA busy */
		HIF_IO_READ32(pAd->hdev_ctrl, MT_WPDMA_GLO_CFG, &Value);

		if ((Value & RX_DMA_BUSY) == RX_DMA_BUSY) {
			/* Check RX FIFO if have data */
			HIF_IO_WRITE32(pAd->hdev_ctrl, 0x4244, 0x28000000);
			HIF_IO_READ32(pAd->hdev_ctrl, 0x4244, &Value);

			if ((Value & (1 << 8)) == 0) {
				pAd->RxDMACheckTimes = 0;
				return FALSE;
			} else {
				pAd->RxDMACheckTimes++;
				return FALSE;
			}
		} else {
			pAd->RxDMACheckTimes = 0;
			return FALSE;
		}
	} else {
		pAd->RxDMACheckTimes = 0;
		return TRUE;
	}
}


VOID dumpRxRing(RTMP_ADAPTER *pAd, INT ring_idx)
{
	RTMP_DMABUF *pDescRing;
	struct hif_pci_rx_ring *pRxRing;
	RXD_STRUC *pRxD;
	int index;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 rx0_ring_size = GET_RX0_RING_SIZE(cap);
	UINT16 rx1_ring_size = GET_RX1_RING_SIZE(cap);
#ifdef MT7626_E2_SUPPORT
	UINT16 rx2_ring_size = GET_RX2_RING_SIZE(cap);
	UINT16 rx3_ring_size = GET_RX3_RING_SIZE(cap);
#endif
	UINT16 RxRingSize = (ring_idx == 0) ? rx0_ring_size : rx1_ring_size;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

#ifdef MT7626_E2_SUPPORT
	if (ring_idx >= 2) {
		RxRingSize = (ring_idx == 2) ? rx2_ring_size : rx3_ring_size;
	}
#endif
	pDescRing = (RTMP_DMABUF *)hif->RxRing[0].desc_ring.AllocVa;
	pRxRing = &hif->RxRing[0];

	for (index = 0; index < RxRingSize; index++) {
		pRxD = (RXD_STRUC *)pRxRing->Cell[index].AllocVa;
		hex_dump("Dump RxDesc", (UCHAR *)pRxD, sizeof(RXD_STRUC));
		dump_rxd(pAd, pRxD);
	}
}

VOID mt_write_txd(RTMP_ADAPTER *pAd, TXD_STRUC *pTxD)
{
#if defined(LINUX) || defined(VXWORKS)
	wmb();
#endif /* LINUX */
	pTxD->DMADONE = 0;
}


VOID ComposePsPoll(
	IN	RTMP_ADAPTER *pAd,
	IN	PPSPOLL_FRAME pPsPollFrame,
	IN	USHORT	Aid,
	IN	UCHAR *pBssid,
	IN	UCHAR *pTa)
{
	NdisZeroMemory(pPsPollFrame, sizeof(PSPOLL_FRAME));
	pPsPollFrame->FC.Type = FC_TYPE_CNTL;
	pPsPollFrame->FC.SubType = SUBTYPE_PS_POLL;
	pPsPollFrame->Aid = Aid | 0xC000;
	COPY_MAC_ADDR(pPsPollFrame->Bssid, pBssid);
	COPY_MAC_ADDR(pPsPollFrame->Ta, pTa);
}

USHORT mt_pci_get_buf_len(RTMP_ADAPTER *pAd, TX_BLK *tx_blk)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT tx_hw_hdr_len = cap->tx_hw_hdr_len;

	return tx_hw_hdr_len + tx_blk->txp_len + tx_blk->MpduHeaderLen + tx_blk->HdrPadLen - tx_blk->hw_rsv_len;
}

USHORT write_first_buf(RTMP_ADAPTER *pAd, TX_BLK *tx_blk, UCHAR *dma_buf)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT tx_hw_hdr_len = cap->tx_hw_hdr_len;
	USHORT first_buf_len;

	first_buf_len =  tx_hw_hdr_len + tx_blk->txp_len + tx_blk->MpduHeaderLen + tx_blk->HdrPadLen - tx_blk->hw_rsv_len;
	NdisMoveMemory(dma_buf, (UCHAR *)(tx_blk->HeaderBuf + tx_blk->hw_rsv_len), first_buf_len);
	return first_buf_len;
}

inline UINT32 pci_get_tx_resource_free_num_nolock(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	UINT32 free_num;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	struct hif_pci_tx_ring *tx_ring = &hif->TxRing[resource_idx];

	free_num = tx_ring->TxSwFreeIdx > tx_ring->TxCpuIdx ?
			   tx_ring->TxSwFreeIdx - tx_ring->TxCpuIdx - 1 :
			   tx_ring->TxSwFreeIdx + tx_ring_size - tx_ring->TxCpuIdx - 1;

	return free_num;
}

UINT32 pci_get_tx_resource_free_num(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	UINT32 free_num;
	ULONG flags;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	struct hif_pci_tx_ring *tx_ring = &hif->TxRing[resource_idx];
	NDIS_SPIN_LOCK *lock = &tx_ring->ring_lock;

	RTMP_IRQ_LOCK(lock, flags);
	free_num = tx_ring->TxSwFreeIdx > tx_ring->TxCpuIdx ?
			   tx_ring->TxSwFreeIdx - tx_ring->TxCpuIdx - 1 :
			   tx_ring->TxSwFreeIdx + tx_ring_size - tx_ring->TxCpuIdx - 1;
	RTMP_IRQ_UNLOCK(lock, flags);
	return free_num;
}

UINT32 pci_get_tx_mgmt_free_num(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	return pci_get_tx_resource_free_num(pAd, resource_idx);
}
UINT32 pci_get_tx_bcn_free_num(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	return pci_get_tx_resource_free_num(pAd, resource_idx);
}

BOOLEAN pci_is_tx_resource_empty(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	return ((hif->TxRing[resource_idx].TxDmaIdx == hif->TxRing[resource_idx].TxCpuIdx)	? 1 : 0);
}

BOOLEAN pci_is_rx_resource_empty(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	return ((hif->RxRing[resource_idx].RxDmaIdx == hif->RxRing[resource_idx].RxCpuIdx)	? 1 : 0);
}

UINT32 pci_get_tx_ctrl_free_num(RTMP_ADAPTER *pAd)
{
	UINT32 free_num;
	ULONG flags;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	NDIS_SPIN_LOCK *lock = &hif->ctrl_ring.ring_lock;
	struct hif_pci_tx_ring *ring = &hif->ctrl_ring;

	RTMP_IRQ_LOCK(lock, flags);
	free_num = ((ring->TxSwFreeIdx > ring->TxCpuIdx) ?
		(ring->TxSwFreeIdx - ring->TxCpuIdx - 1) :
		(ring->TxSwFreeIdx + MGMT_RING_SIZE - ring->TxCpuIdx - 1));
	RTMP_IRQ_UNLOCK(lock, flags);
	return free_num;
}

UINT32 pci_get_rx_resource_pending_num(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	UINT32 num;
	ULONG flags;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	NDIS_SPIN_LOCK *lock = &hif->RxRing[resource_idx].ring_lock;
	struct hif_pci_rx_ring *rx_ring = &hif->RxRing[resource_idx];

	RTMP_IRQ_LOCK(lock, flags);
	HIF_IO_READ32(pAd->hdev_ctrl, hif->RxRing[resource_idx].hw_didx_addr, &rx_ring->RxDmaIdx);

	num = ((rx_ring->RxDmaIdx > rx_ring->RxSwReadIdx) ?
	(rx_ring->RxDmaIdx - rx_ring->RxSwReadIdx) :
	(rx_ring->RxDmaIdx + rx_ring->ring_size - rx_ring->RxSwReadIdx));

	RTMP_IRQ_UNLOCK(lock, flags);
	return num;
}

inline VOID pci_inc_resource_full_cnt(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	hif->TxRing[resource_idx].tx_ring_full_cnt++;
}

inline VOID pci_dec_resource_full_cnt(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	hif->TxRing[resource_idx].tx_ring_full_cnt--;
}

inline BOOLEAN pci_get_resource_state(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	return hif->TxRing[resource_idx].tx_ring_state;
}

inline INT pci_set_resource_state(RTMP_ADAPTER *pAd, UINT8 resource_idx, BOOLEAN state)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	hif->TxRing[resource_idx].tx_ring_state = state;
	return NDIS_STATUS_SUCCESS;
}

UINT32 pci_check_resource_state(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = &hif->TxRing[resource_idx];
	BOOLEAN resource_state;
	UINT free_num;

	resource_state = pci_get_resource_state(pAd, resource_idx);
	free_num = pci_get_tx_resource_free_num_nolock(pAd, resource_idx);

	if (resource_state == TX_RING_HIGH) {
		if (free_num >= tx_ring->tx_ring_low_water_mark)
			return TX_RING_HIGH_TO_HIGH;
		else
			return TX_RING_HIGH_TO_LOW;
	} else if (resource_state == TX_RING_LOW) {
		if (free_num > tx_ring->tx_ring_high_water_mark)
			return TX_RING_LOW_TO_HIGH;
		else
			return TX_RING_LOW_TO_LOW;
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: unknow state %d, free ring number = %d",
		__func__, resource_state, free_num));
		return TX_RING_UNKNOW_CHANGE;
	}
}

USHORT mtd_pci_write_tx_resource(
	RTMP_ADAPTER *pAd,
	TX_BLK *pTxBlk,
	BOOLEAN bIsLast,
	USHORT *FreeNumber)
{
	UCHAR *pDMAHeaderBufVA;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
	RTMP_DMACB *dma_cb;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	UINT16 TmacLen;
#endif
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *pTxRing;

	pTxRing = &hif->TxRing[pTxBlk->resource_idx];
	TxIdx = pTxRing->TxCpuIdx;
	dma_cb = &pTxRing->Cell[TxIdx];
	pDMAHeaderBufVA = (UCHAR *)dma_cb->DmaBuf.AllocVa;
	dma_cb->pNextNdisPacket = NULL;
	dma_cb->PacketPa = pTxBlk->SrcBufPA;
	if (pTxBlk->TxFrameType != TX_FRAG_FRAME)
		dma_cb->DmaBuf.AllocSize = mt_pci_get_buf_len(pAd, pTxBlk);
	else
		dma_cb->DmaBuf.AllocSize = write_first_buf(pAd, pTxBlk, pDMAHeaderBufVA);
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *)dma_cb->AllocVa;
#else
	pDestTxD = (TXD_STRUC *)dma_cb->AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	MTMacInfoEndianChange(pAd,  (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, sizeof(TMAC_TXD_L));
	if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
		ra_dma_addr_t SrcBufPA = 0;

		RTMPFrameEndianChange(pAd, (PUCHAR)(pTxBlk->pSrcBufData), DIR_WRITE, FALSE);
		SrcBufPA = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
		if (SrcBufPA != dma_cb->PacketPa)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("%s SrcBufPA invaild: %p %p", __func__, SrcBufPA, dma_cb->PacketPa));
	}
#endif

	dma_cb->DmaBuf.AllocPa = PCI_MAP_SINGLE(pAd, pDMAHeaderBufVA, dma_cb->DmaBuf.AllocSize, 0, RTMP_PCI_DMA_TODEVICE);
	pTxD->SDPtr0 = dma_cb->DmaBuf.AllocPa;
	pTxD->SDLen0 = dma_cb->DmaBuf.AllocSize;
	pTxD->SDPtr1 = dma_cb->PacketPa;
	pTxD->SDLen1 = pTxBlk->SrcBufLen < cap->CtParseLen ? pTxBlk->SrcBufLen : cap->CtParseLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (bIsLast && pTxD->SDLen1) ? 1 : 0;
	pTxD->Burst = 0;
	mt_write_txd(pAd, pTxD);

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	RetTxIdx = TxIdx;

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(dma_cb->DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(dma_cb->AllocPa, RXD_SIZE);

	INC_RING_INDEX(TxIdx, tx_ring_size);
	pTxRing->TxCpuIdx = TxIdx;
	*FreeNumber -= 1;
	return RetTxIdx;
}

USHORT mt_pci_write_tx_resource(
	RTMP_ADAPTER *pAd,
	TX_BLK *pTxBlk,
	BOOLEAN bIsLast,
	USHORT *FreeNumber)
{
	UCHAR *pDMAHeaderBufVA;
	UINT32 BufBasePaLow;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	UINT16 TmacLen;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	struct hif_pci_tx_ring *pTxRing;

	/* get Tx Ring Resource*/
	pTxRing = &hif->TxRing[pTxBlk->resource_idx];
	TxIdx = hif->TxRing[pTxBlk->resource_idx].TxCpuIdx;

	pDMAHeaderBufVA = (UCHAR *)pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);
	pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
	pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;
	pTxRing->Cell[TxIdx].PacketPa = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
	/* build Tx Descriptor*/
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *)pTxRing->Cell[TxIdx].AllocVa;
#else
	pDestTxD = (TXD_STRUC *)pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#endif
	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = write_first_buf(pAd, pTxBlk, pDMAHeaderBufVA);
	pTxD->SDPtr1 = pTxRing->Cell[TxIdx].PacketPa;
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (bIsLast && pTxD->SDLen1) ? 1 : 0;
	pTxD->Burst = 0;
	mt_write_txd(pAd, pTxD);
#ifdef RT_BIG_ENDIAN
	TmacLen = (cap->tx_hw_hdr_len - pTxBlk->hw_rsv_len);
	MTMacInfoEndianChange(pAd,  (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, TmacLen);
	RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TmacLen), DIR_WRITE, FALSE);
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
	RetTxIdx = TxIdx;
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].AllocPa, RXD_SIZE);
	/* Update Tx index*/
	pTxRing->TxCpuIdx = TxIdx;
	*FreeNumber -= 1;
	return RetTxIdx;
}

USHORT mt_pci_write_multi_tx_resource(
	RTMP_ADAPTER *pAd,
	TX_BLK *pTxBlk,
	UCHAR frameNum,
	USHORT *FreeNumber)
{
	BOOLEAN bIsLast;
	UCHAR *pDMAHeaderBufVA;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	UINT16 TmacLen;
#endif
	UINT32 BufBasePaLow;
	struct hif_pci_tx_ring *pTxRing;
	UINT32 firstDMALen = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	ASSERT((pTxBlk->TxFrameType == TX_AMSDU_FRAME));
	bIsLast = ((frameNum == (pTxBlk->TotalFrameNum - 1)) ? 1 : 0);
	/* get Tx Ring Resource */
	pTxRing = &hif->TxRing[pTxBlk->resource_idx];
	TxIdx = hif->TxRing[pTxBlk->resource_idx].TxCpuIdx;
	pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

	if (frameNum == 0) {
		/* copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer */
		firstDMALen =  cap->tx_hw_hdr_len - pTxBlk->hw_rsv_len + pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;
		NdisMoveMemory(pDMAHeaderBufVA, pTxBlk->HeaderBuf + pTxBlk->hw_rsv_len, firstDMALen);
	} else {
		firstDMALen = pTxBlk->MpduHeaderLen;
		NdisMoveMemory(pDMAHeaderBufVA, pTxBlk->HeaderBuf, firstDMALen);
	}

	pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
	pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;
	pTxRing->Cell[TxIdx].PacketPa = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
	/* build Tx Descriptor */
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *) pTxRing->Cell[TxIdx].AllocVa;
#else
	pDestTxD = (TXD_STRUC *) pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#endif
	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = firstDMALen; /* include padding*/
	pTxD->SDPtr1 = pTxRing->Cell[TxIdx].PacketPa;
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (bIsLast && pTxD->SDLen1) ? 1 : 0;
	pTxD->Burst = 0;
	mt_write_txd(pAd, pTxD);
#ifdef RT_BIG_ENDIAN
	TmacLen = (cap->tx_hw_hdr_len - pTxBlk->hw_rsv_len);

	if (frameNum == 0)
		RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TmacLen), DIR_WRITE, FALSE);

	if (frameNum != 0)
		MTMacInfoEndianChange(pAd,  (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, TmacLen);

	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
	RetTxIdx = TxIdx;
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].AllocPa, RXD_SIZE);
	/* Update Tx index*/
	INC_RING_INDEX(TxIdx, tx_ring_size);
	pTxRing->TxCpuIdx = TxIdx;
	*FreeNumber -= 1;
	return RetTxIdx;
}

VOID mt_pci_write_final_tx_resource(
	RTMP_ADAPTER *pAd,
	TX_BLK *pTxBlk,
	USHORT totalMPDUSize,
	USHORT FirstTxIdx)
{
	struct hif_pci_tx_ring *pTxRing;
	UCHAR *tmac_info;
	TMAC_TXD_S *txd_s;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	/* get Tx Ring Resource*/
	pTxRing = &hif->TxRing[pTxBlk->resource_idx];

	if (FirstTxIdx >= tx_ring_size)
		return;

	tmac_info = (UCHAR *)pTxRing->Cell[FirstTxIdx].DmaBuf.AllocVa;
	txd_s = (TMAC_TXD_S *)tmac_info;
	txd_s->TxD0.TxByteCount = totalMPDUSize;
}

USHORT	mt_pci_write_frag_tx_resource(
	RTMP_ADAPTER *pAd,
	TX_BLK *pTxBlk,
	UCHAR fragNum,
	USHORT *FreeNumber)
{
	UCHAR *pDMAHeaderBufVA;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
	RTMP_DMACB *dma_cb;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	UINT16 TmacLen;
#endif
	struct hif_pci_tx_ring *pTxRing;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	pTxRing = &hif->TxRing[pTxBlk->resource_idx];
	TxIdx = pTxRing->TxCpuIdx;
	dma_cb = &pTxRing->Cell[TxIdx];
	pDMAHeaderBufVA = (PUCHAR)dma_cb->DmaBuf.AllocVa;
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *)dma_cb->AllocVa;
#else
	pDestTxD = (TXD_STRUC *)dma_cb->AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	TmacLen = (cap->tx_hw_hdr_len - pTxBlk->hw_rsv_len);
	MTMacInfoEndianChange(pAd,  (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, TmacLen);
	RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TmacLen), DIR_WRITE, FALSE);
#endif

	if (fragNum == pTxBlk->TotalFragNum) {
		dma_cb->pNdisPacket = pTxBlk->pPacket;
		dma_cb->pNextNdisPacket = NULL;
	}

	if (IS_ASIC_CAP(pAd, fASIC_CAP_CT))
		dma_cb->PacketPa = pTxBlk->SrcBufPA;
	else
		dma_cb->PacketPa = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);

	dma_cb->DmaBuf.AllocSize = mt_pci_get_buf_len(pAd, pTxBlk);
	dma_cb->DmaBuf.AllocPa = PCI_MAP_SINGLE(pAd, pDMAHeaderBufVA, dma_cb->DmaBuf.AllocSize, 0, RTMP_PCI_DMA_TODEVICE);

	pTxD->SDPtr0 = dma_cb->DmaBuf.AllocPa;
	pTxD->SDLen0 = dma_cb->DmaBuf.AllocSize;
	pTxD->SDPtr1 = dma_cb->PacketPa;
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (pTxD->SDLen1 ? 1 : 0);
	pTxD->Burst = 0;
	mt_write_txd(pAd, pTxD);
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
	RetTxIdx = TxIdx;
	pTxBlk->Priv += pTxBlk->SrcBufLen;
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(dma_cb->DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(dma_cb->AllocPa, RXD_SIZE);
	/* Update Tx index */
	INC_RING_INDEX(TxIdx, tx_ring_size);
	pTxRing->TxCpuIdx = TxIdx;
	*FreeNumber -= 1;
	return RetTxIdx;
}

VOID pci_kickout_data_tx(RTMP_ADAPTER *pAd, TX_BLK *tx_blk, UCHAR resource_idx)
{
	ULONG flags;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	NDIS_SPIN_LOCK *lock = &hif->TxRing[resource_idx].ring_lock;

	RTMP_IRQ_LOCK(lock, flags);
	HIF_IO_WRITE32(pAd->hdev_ctrl, hif->TxRing[resource_idx].hw_cidx_addr, hif->TxRing[resource_idx].TxCpuIdx);
	RTMP_IRQ_UNLOCK(lock, flags);
}

BOOLEAN mtd_free_txd(RTMP_ADAPTER *pAd, UINT8 hif_idx)
{
	struct hif_pci_tx_ring *tx_ring;
	RTMP_DMACB *dma_cb;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
#ifdef CONFIG_TP_DBG
	/* struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;*/
#endif
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	NDIS_SPIN_LOCK *lock = &hif->TxRing[hif_idx].ring_lock;

	tx_ring = &hif->TxRing[hif_idx];

	RTMP_SEM_LOCK(lock);
	HIF_IO_READ32(pAd->hdev_ctrl, tx_ring->hw_didx_addr, &tx_ring->TxDmaIdx);

	while (tx_ring->TxSwFreeIdx != tx_ring->TxDmaIdx) {
		dma_cb = &tx_ring->Cell[tx_ring->TxSwFreeIdx];

		PCI_UNMAP_SINGLE(pAd, dma_cb->DmaBuf.AllocPa, dma_cb->DmaBuf.AllocSize, RTMP_PCI_DMA_TODEVICE);

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(dma_cb->AllocPa, TXD_SIZE);
		INC_RING_INDEX(tx_ring->TxSwFreeIdx, tx_ring_size);
	}
	RTMP_SEM_UNLOCK(lock);

	return FALSE;
}

BOOLEAN mtd_tx_dma_done_handle(RTMP_ADAPTER *pAd, UINT8 hif_idx)
{
	struct qm_ops *qm_ops = pAd->qm_ops;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = &hif->TxRing[hif_idx];
	UINT free_num;

	mtd_free_txd(pAd, hif_idx);

	free_num = pci_get_tx_resource_free_num_nolock(pAd, hif_idx);
	if (free_num >= tx_ring->tx_ring_high_water_mark && pci_get_resource_state(pAd, hif_idx) == TX_RING_LOW) {
		pci_set_resource_state(pAd, hif_idx, TX_RING_HIGH);

		qm_ops->schedule_tx_que(pAd);
	}  else if (tx_flow_check_if_blocked(pAd)) {
		qm_ops->schedule_tx_que(pAd);
	}
	return FALSE;
}

BOOLEAN mt_cmd_dma_done_handle(RTMP_ADAPTER *pAd, UINT8 hif_idx)
{
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *pCtrlRing = &hif->ctrl_ring;

	NdisAcquireSpinLock(&hif->ctrl_ring.ring_lock);
	HIF_IO_READ32(pAd->hdev_ctrl, pCtrlRing->hw_didx_addr, &pCtrlRing->TxDmaIdx);

	while (pCtrlRing->TxSwFreeIdx != pCtrlRing->TxDmaIdx) {
#ifdef RT_BIG_ENDIAN
		pDestTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
		NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&hw_hdr_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
#endif
		pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket;

		if (pPacket == NULL) {
			INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, CTL_RING_SIZE);
			continue;
		}

		if (pPacket) {
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RTMPFreeNdisPacket(pAd, pPacket);
		}

		pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket = NULL;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocPa, TXD_SIZE);
		INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, CTL_RING_SIZE);

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
	}

	NdisReleaseSpinLock(&hif->ctrl_ring.ring_lock);

	return FALSE;
}

BOOLEAN mt_fwdl_dma_done_handle(RTMP_ADAPTER *pAd, UINT8 hif_idx)
{
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
	UCHAR	FREE = 0;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *pRing = &hif->fwdl_ring;

	NdisAcquireSpinLock(&pRing->ring_lock);
	HIF_IO_READ32(pAd->hdev_ctrl, pRing->hw_didx_addr, &pRing->TxDmaIdx);

	while (pRing->TxSwFreeIdx != pRing->TxDmaIdx) {
		FREE++;
#ifdef RT_BIG_ENDIAN
		pDestTxD = (TXD_STRUC *)(pRing->Cell[pRing->TxSwFreeIdx].AllocVa);
		NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&hw_hdr_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (TXD_STRUC *)(pRing->Cell[pRing->TxSwFreeIdx].AllocVa);
#endif
		pPacket = pRing->Cell[pRing->TxSwFreeIdx].pNdisPacket;

		if (pPacket == NULL) {
			INC_RING_INDEX(pRing->TxSwFreeIdx, MGMT_RING_SIZE);
			continue;
		}

		if (pPacket) {
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RTMPFreeNdisPacket(pAd, pPacket);
		}

		pRing->Cell[pRing->TxSwFreeIdx].pNdisPacket = NULL;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRing->Cell[pRing->TxSwFreeIdx].AllocPa, TXD_SIZE);
		INC_RING_INDEX(pRing->TxSwFreeIdx, MGMT_RING_SIZE);
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
	}

	NdisReleaseSpinLock(&pRing->ring_lock);
	return FALSE;
}

VOID RTMPHandleRxCoherentInterrupt(RTMP_ADAPTER *pAd)
{
	if (pAd == NULL) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("====> pAd is NULL, return.\n"));
		return;
	}

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("RTMPHandleRxCoherentInterrupt\n"));
	return;
}


#ifdef CONFIG_AP_SUPPORT
VOID RTMPHandleMcuInterrupt(RTMP_ADAPTER *pAd)
{
	UINT32 McuIntSrc = 0;

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
				 __func__));
		return;
	}

	RTMP_IO_READ32(pAd->hdev_ctrl, 0x7024, &McuIntSrc);
	/* check mac 0x7024 */
#ifdef CARRIER_DETECTION_SUPPORT

	if (pAd->infType == RTMP_DEV_INF_PCIE &&
		(McuIntSrc & (1 << 1)) && /*bit_1: carr_status interrupt */
		(pAd->CommonCfg.CarrierDetect.Enable == TRUE))
		RTMPHandleRadarInterrupt(pAd);

#endif /* CARRIER_DETECTION_SUPPORT */
	/* clear MCU Int source register.*/
	RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x7024, 0);
}
#endif /* CONFIG_AP_SUPPORT */

#ifdef RX_SCATTER

/*
 * pkt: original pkt
 * pkt_len: original pkt len not inclduing SKB_BUF_HEADROOM_RSV
 *		and SKB_BUF_TAILROOM_RSV
 * ext_head_len: new pkt extend head room len
 * ext_tail_len: new pkt extend tail room len
*/
PNDIS_PACKET ExpandPacketBuffer(
	struct _RTMP_ADAPTER *pAd,
	void *hif_resource,
	enum  MEM_ALLOC_TYPE type,
	PNDIS_PACKET original_pkt,
	UINT32 original_pkt_len,
	UINT32 new_pkt_len,
	UINT *new_skb_len)
{
	PNDIS_PACKET *new_pkt = NULL;
	UINT32 skb_len;

	switch (type) {
#ifdef CONFIG_WIFI_BUILD_SKB
	case DYNAMIC_PAGE_ALLOC:
		/*
		 * calculate new skb data size
		 * SKB_BUF_HEADROOM_RSV = NET_SKB_PAD + NET_IP_ALIGN
		 * SKB_BUF_TAILROOM_RSV = sizeof(struct skb_shared_info)
		 * skb_size = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + ext_tail_len) +
		 *		SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV)
		 * if skb_size <= PAGE_SIZE use DEV_ALLOC_FRAG else use kmalloc
		 * to alocate skb data size
		 */
		skb_len = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + new_pkt_len)
				+ SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV);

		if (skb_len <= PAGE_SIZE)
			DEV_ALLOC_FRAG(new_pkt, skb_len);
		else
			os_alloc_mem(pAd, (PUCHAR *)&new_pkt, skb_len);

		*new_skb_len = skb_len;

		if (new_pkt && original_pkt)
			os_move_mem(((PVOID)new_pkt + SKB_BUF_HEADROOM_RSV),
				((PVOID)original_pkt + SKB_BUF_HEADROOM_RSV), original_pkt_len);

		if (original_pkt)
			DEV_FREE_FRAG_BUF(original_pkt);

		break;
#endif /* CONFIG_WIFI_BUILD_SKB */

	case PRE_SLAB_ALLOC:
		new_pkt = alloc_rx_buf_64k(hif_resource);

		*new_skb_len = new_pkt_len;

		if (new_pkt && original_pkt)
			os_move_mem((PVOID)new_pkt, ((PVOID)original_pkt), original_pkt_len);

		if (original_pkt)
			free_rx_buf_1k(hif_resource);

		break;
	default:
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: unknown mem type %d\n", __func__, type));
		break;
	}

	if (!new_pkt) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("Extend Rx buffer %d size packet failed! drop pkt.\n",
			(new_pkt_len)));
			return NULL;
	}

	return new_pkt;
}

static INT rx_scatter_info(
	IN struct hif_pci_rx_ring *pRxRing,
	IN RXD_STRUC *pRxD,
	OUT UINT *pPktSize)
{
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	UINT LoopCnt;
	INT32 RxCellIdx;
	RTMP_DMACB *pCurRxCell = NULL;
	RXD_STRUC *pCurRxD;

	LoopCnt = 0;

	*pPktSize = 0;
	RxCellIdx =  pRxRing->RxSwReadIdx;

	/* walk through rx-ring and find the rx-cell content LS0 to be 1. */
	do {
		pCurRxCell = &pRxRing->Cell[RxCellIdx];
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCurRxCell->AllocPa, RXD_SIZE);
#ifdef RT_BIG_ENDIAN
		pDestRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
		/* RxD = *pDestRxD; */
		NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
		pCurRxD = (RXD_STRUC *)&rx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
#else
		/* Point to Rx indexed rx ring descriptor */
		pCurRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
#endif

		if (pCurRxD->DDONE == 0) {
			LoopCnt = 0;
			break;
		}

		*pPktSize += pCurRxD->SDL0;
		LoopCnt++;

		/* find the last pice of rx scattering. */
		if (pCurRxD->LS0 == 1) {
			break;
		}

		INC_RING_INDEX(RxCellIdx, pRxRing->ring_size);
	} while (TRUE);

	return LoopCnt;
}

static INT rx_scatter_info_io(
	struct hif_pci_rx_ring *pRxRing,
	RXD_STRUC *pRxD,
	UINT *pPktSize)
{
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	UINT LoopCnt;
	INT32 RxCellIdx;
	RTMP_DMACB *pCurRxCell = NULL;
	RXD_STRUC *pCurRxD;
	UINT isLsPktFound = FALSE;

	LoopCnt = 0;

	*pPktSize = 0;
	RxCellIdx =  pRxRing->RxSwReadIdx;

	/* walk through rx-ring and find the rx-cell content LS0 to be 1. */
	do {
		if (RxCellIdx == pRxRing->RxDmaIdx) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: RxCellIdx == pRxRing->RxDmaIdx\n", __func__));
			if (pRxD->LS0 == 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxD->LS0 == 0\n", __func__));
				LoopCnt = 0;
			}

			break;
		}

		pCurRxCell = &pRxRing->Cell[RxCellIdx];
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCurRxCell->AllocPa, RXD_SIZE);
#ifdef RT_BIG_ENDIAN
		pDestRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
		/* RxD = *pDestRxD; */
		NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
		pCurRxD = (RXD_STRUC *)&rx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
#else
		/* Point to Rx indexed rx ring descriptor */
		pCurRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
#endif

		if (pCurRxD->DDONE == 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pCurRxD->DDONE == 0 \n", __func__));
			break;
		}

		*pPktSize += pCurRxD->SDL0;
		LoopCnt++;
		INC_RING_INDEX(RxCellIdx, pRxRing->ring_size);

		/* find the last pice of rx scattering. */
		if (pCurRxD->LS0 == 1) {
			isLsPktFound = TRUE;
			break;
		}
	} while (TRUE);

	return LoopCnt;
}

static INT rx_scatter_gather(
	RTMP_ADAPTER *pAd,
	UCHAR hif_idx,
	RTMP_DMACB *pRxCell,
	RXD_STRUC *pRxD,
	UINT scatterCnt,
	UINT pktSize,
	UINT *skb_data_len)
{
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	UINT LoopCnt;
	INT32 RxCellIdx;
	UINT32 buf_idx;
	RTMP_DMACB *pCurRxCell = NULL;
	RXD_STRUC *pCurRxD;
	PNDIS_PACKET pRxCellPacket;
	struct hif_pci_rx_ring *pRxRing = &pAd->PciHif.RxRing[hif_idx];

	if (pRxRing == NULL || pRxCell == NULL || pRxD == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxRing == NULL || pRxCell == NULL\
					|| pRxD == NULL\n", __func__));
		return FALSE;
	}

	/* keep pRxCell value and replace pRxCell->pNdisPacket with expand skb buffer. */
	pRxCellPacket = pRxCell->pNdisPacket;

	if (pRxCellPacket == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxCellPacket == NULL\n", __func__));
		return FALSE;
	}

	buf_idx = pRxD->SDL0;
#ifdef CONFIG_WIFI_BUILD_SKB
	if (hif_idx == HIF_RX_IDX0) {
		pRxCell->pNdisPacket = ExpandPacketBuffer(pAd, pRxRing, DYNAMIC_PAGE_ALLOC,
						pRxCellPacket, buf_idx, pktSize, skb_data_len);

		buf_idx += (SKB_BUF_HEADROOM_RSV);

		if (pRxCell->pNdisPacket != NULL) {
			pRxCell->DmaBuf.AllocVa = (PVOID)(pRxCell->pNdisPacket)
							+ SKB_BUF_HEADROOM_RSV;
		}
	} else if (hif_idx == HIF_RX_IDX1) {
		pRxCell->pNdisPacket = ExpandPacketBuffer(pAd, pRxRing, PRE_SLAB_ALLOC,
						pRxCellPacket, buf_idx, pktSize, skb_data_len);

		if (pRxCell->pNdisPacket != NULL) {
			pRxCell->DmaBuf.AllocVa = (PVOID)(pRxCell->pNdisPacket);
		}

		pRxRing->cur_free_buf_len = FREE_BUF_64k;
	}
#else  /*CONFIG_WIFI_BUILD_SKB*/
	if (hif_idx == HIF_RX_IDX0) {
		OS_PKT_TAIL_BUF_EXTEND(pRxCellPacket, pRxD->SDL0);
		pRxCell->pNdisPacket = ExpandPacket(NULL, pRxCellPacket, 0, pktSize);

		if (pRxCell->pNdisPacket != NULL)
			pRxCell->DmaBuf.AllocVa = GET_OS_PKT_DATAPTR(pRxCell->pNdisPacket);

	} else if (hif_idx == HIF_RX_IDX1) {
		pRxCell->pNdisPacket = ExpandPacketBuffer(pAd, pRxRing, PRE_SLAB_ALLOC,
						pRxCellPacket, buf_idx, pktSize, skb_data_len);

		if (pRxCell->pNdisPacket != NULL)
			pRxCell->DmaBuf.AllocVa = (PVOID)(pRxCell->pNdisPacket);

		pRxRing->cur_free_buf_len = FREE_BUF_64k;
	}
#endif  /*CONFIG_WIFI_BUILD_SKB*/
	else
		pRxCell->DmaBuf.AllocVa = NULL;

	if (pRxCell->pNdisPacket == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxCell->pNdisPacket == NULL\n", __func__));
		return TRUE;
	}

	RxCellIdx =  pRxRing->RxSwReadIdx;

	for (LoopCnt = 0; LoopCnt < (scatterCnt - 1); LoopCnt++) {
		INC_RING_INDEX(RxCellIdx, pRxRing->ring_size);
		pCurRxCell = &pRxRing->Cell[RxCellIdx];
		/* return tokens of all rx scatter-gather cells. */
		PCI_UNMAP_SINGLE(pAd, pCurRxCell->DmaBuf.AllocPa,
						 pCurRxCell->DmaBuf.AllocSize,
						 RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCurRxCell->DmaBuf.AllocPa, pCurRxCell->DmaBuf.AllocSize);
#ifdef RT_BIG_ENDIAN
		pDestRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
		/* RxD = *pDestRxD; */
		NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
		pCurRxD = (RXD_STRUC *)&rx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
#else
		/* Point to Rx indexed rx ring descriptor */
		pCurRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
#endif
#ifdef CONFIG_WIFI_BUILD_SKB
		memcpy((pRxCell->pNdisPacket + buf_idx),
		    (VOID *)(pCurRxCell->DmaBuf.AllocVa), pCurRxD->SDL0);

		buf_idx += pCurRxD->SDL0;
#else	/* CONFIG_WIFI_BUILD_SKB */
		if (hif_idx == HIF_RX_IDX0) {
			memcpy(OS_PKT_TAIL_BUF_EXTEND(pRxCell->pNdisPacket, pCurRxD->SDL0),
			   (VOID *)(pCurRxCell->DmaBuf.AllocVa), pCurRxD->SDL0);
		} else if (hif_idx == HIF_RX_IDX1) {
			memcpy((pRxCell->pNdisPacket + buf_idx),
				(VOID *)(pCurRxCell->DmaBuf.AllocVa), pCurRxD->SDL0);

			buf_idx += pCurRxD->SDL0;
		}
#endif  /* CONFIG_WIFI_BUILD_SKB */
		/* update done bit of all rx scatter-gather cells to zero. */

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pCurRxD->SDL0 = %d, \n", __func__, pCurRxD->SDL0));

		if (hif_idx == HIF_RX_IDX0) {
#ifdef CONFIG_WIFI_BUILD_SKB
			DEV_FREE_FRAG_BUF(pCurRxCell->pNdisPacket);
#else
			RELEASE_NDIS_PACKET(pAd, pCurRxCell->pNdisPacket, NDIS_STATUS_SUCCESS);
#endif
			pCurRxCell->pNdisPacket = RTMP_AllocateRxPacketBuffer(pRxRing,
					((POS_COOKIE)(pAd->OS_Cookie))->pDev,
					DYNAMIC_PAGE_ALLOC,
					pCurRxCell->DmaBuf.AllocSize,
					&pCurRxCell->DmaBuf.AllocVa, &pCurRxCell->DmaBuf.AllocPa);
		} else if (hif_idx == HIF_RX_IDX1) {
			free_rx_buf_1k(pRxRing);
			pCurRxCell->pNdisPacket = RTMP_AllocateRxPacketBuffer(pRxRing,
					((POS_COOKIE)(pAd->OS_Cookie))->pDev,
					PRE_SLAB_ALLOC,
					pCurRxCell->DmaBuf.AllocSize,
					&pCurRxCell->DmaBuf.AllocVa, &pCurRxCell->DmaBuf.AllocPa);
		}
		pCurRxD->SDP0 = pCurRxCell->DmaBuf.AllocPa;
		pCurRxD->SDL0 = pRxRing->RxBufferSize;
		pCurRxD->DDONE = 0;
		/* update rx descriptor */
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
		WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pCurRxD, FALSE, TYPE_RXD);
#endif
	}

	/* update pRxRing->RxSwReadIdx do last cell of rx scatter-gather. */
	pRxRing->RxSwReadIdx = RxCellIdx;
	return TRUE;
}

#ifdef MT7626_E2_SUPPORT
static INT rx2_scatter_gather(
	RTMP_ADAPTER *pAd,
	UCHAR hif_idx,
	RTMP_DMACB *pRxCell,
	RXD_STRUC *pRxD,
	UINT scatterCnt,
	UINT pktSize,
	UINT *skb_data_len)
{
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	UINT LoopCnt;
	INT32 RxCellIdx;
	UINT32 buf_idx;
	RTMP_DMACB *pCurRxCell = NULL;
	RXD_STRUC *pCurRxD;
	PNDIS_PACKET pRxCellPacket;
	struct hif_pci_rx_ring *pRxRing = &pAd->PciHif.RxRing[hif_idx];

	if (pRxRing == NULL || pRxCell == NULL || pRxD == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxRing == NULL || pRxCell == NULL\
				 || pRxD == NULL\n", __func__));
		return FALSE;
	}

	/* keep pRxCell value and replace pRxCell->pNdisPacket with expand skb buffer. */
	pRxCellPacket = pRxCell->pNdisPacket;

	if (pRxCellPacket == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxCellPacket == NULL\n", __func__));
		return FALSE;
	}

	buf_idx = pRxD->SDL0;
#ifdef CONFIG_WIFI_BUILD_SKB
	if (hif_idx == HIF_RX_IDX2) {
		pRxCell->pNdisPacket = ExpandPacketBuffer(pAd, pRxRing, DYNAMIC_PAGE_ALLOC,
							   pRxCellPacket, buf_idx, pktSize, skb_data_len);

		buf_idx += (SKB_BUF_HEADROOM_RSV);

		if (pRxCell->pNdisPacket != NULL) {
			pRxCell->DmaBuf.AllocVa = (PVOID)(pRxCell->pNdisPacket)
									  + SKB_BUF_HEADROOM_RSV;
		}
	}
#else  /*CONFIG_WIFI_BUILD_SKB*/
	if (hif_idx == HIF_RX_IDX2) {
		OS_PKT_TAIL_BUF_EXTEND(pRxCellPacket, pRxD->SDL0);
		pRxCell->pNdisPacket = ExpandPacket(NULL, pRxCellPacket, 0, pktSize);

		if (pRxCell->pNdisPacket != NULL)
			pRxCell->DmaBuf.AllocVa = GET_OS_PKT_DATAPTR(pRxCell->pNdisPacket);
	}
#endif  /*CONFIG_WIFI_BUILD_SKB*/
	else
		pRxCell->DmaBuf.AllocVa = NULL;

	if (pRxCell->pNdisPacket == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxCell->pNdisPacket == NULL\n", __func__));
		return TRUE;
	}

	RxCellIdx =  pRxRing->RxSwReadIdx;

	for (LoopCnt = 0; LoopCnt < (scatterCnt - 1); LoopCnt++) {
		INC_RING_INDEX(RxCellIdx, pRxRing->ring_size);
		pCurRxCell = &pRxRing->Cell[RxCellIdx];
		/* return tokens of all rx scatter-gather cells. */
		PCI_UNMAP_SINGLE(pAd, pCurRxCell->DmaBuf.AllocPa,
						 pCurRxCell->DmaBuf.AllocSize,
						 RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCurRxCell->DmaBuf.AllocPa, pCurRxCell->DmaBuf.AllocSize);
#ifdef RT_BIG_ENDIAN
		pDestRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
		/* RxD = *pDestRxD; */
		NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
		pCurRxD = (RXD_STRUC *)&rx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
#else
		/* Point to Rx indexed rx ring descriptor */
		pCurRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
#endif
#ifdef CONFIG_WIFI_BUILD_SKB
		memcpy((pRxCell->pNdisPacket + buf_idx),
			   (VOID *)(pCurRxCell->DmaBuf.AllocVa), pCurRxD->SDL0);

		buf_idx += pCurRxD->SDL0;
#else	/* CONFIG_WIFI_BUILD_SKB */
		if (hif_idx == HIF_RX_IDX2) {
			memcpy(OS_PKT_TAIL_BUF_EXTEND(pRxCell->pNdisPacket, pCurRxD->SDL0),
				   (VOID *)(pCurRxCell->DmaBuf.AllocVa), pCurRxD->SDL0);
		}
#endif  /* CONFIG_WIFI_BUILD_SKB */
		/* update done bit of all rx scatter-gather cells to zero. */

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pCurRxD->SDL0 = %d, \n", __func__, pCurRxD->SDL0));

		if (hif_idx == HIF_RX_IDX2) {
#ifdef CONFIG_WIFI_BUILD_SKB
			DEV_FREE_FRAG_BUF(pCurRxCell->pNdisPacket);
#else
			RELEASE_NDIS_PACKET(pAd, pRxCell->pNdisPacket, NDIS_STATUS_SUCCESS);
#endif
			pCurRxCell->pNdisPacket = RTMP_AllocateRxPacketBuffer(pRxRing,
									  ((POS_COOKIE)(pAd->OS_Cookie))->pDev,
									  DYNAMIC_PAGE_ALLOC,
									  pCurRxCell->DmaBuf.AllocSize,
									  &pCurRxCell->DmaBuf.AllocVa, &pCurRxCell->DmaBuf.AllocPa);
		}
		pCurRxD->SDP0 = pCurRxCell->DmaBuf.AllocPa;
		pCurRxD->SDL0 = pRxRing->RxBufferSize;
		pCurRxD->DDONE = 0;
		/* update rx descriptor */
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
		WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pCurRxD, FALSE, TYPE_RXD);
#endif
	}

	/* update pRxRing->RxSwReadIdx do last cell of rx scatter-gather. */
	pRxRing->RxSwReadIdx = RxCellIdx;
	return TRUE;
}
#endif /* MT7626_E2_SUPPORT */
#endif /* RX_SCATTER */

UINT8 swq_to_hif[PACKET_TYPE_NUM][WMM_QUE_NUM] = {
	{HIF_TX_IDX0, HIF_TX_IDX1, HIF_TX_IDX2, HIF_TX_IDX4}, /* TX_DATA */
	{HIF_TX_IDX0, HIF_TX_IDX1, HIF_TX_IDX2, HIF_TX_IDX4}, /* TX_DATA_HIGH_PRIO */
	{HIF_TX_IDX1, HIF_TX_IDX1, HIF_TX_IDX1, HIF_TX_IDX1}, /* TX_MGMT */
	{HIF_TX_IDX5, HIF_TX_IDX5, HIF_TX_IDX5, HIF_TX_IDX5} /*  TX_ALTX */
};

inline UINT32 mtd_pci_get_resource_idx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum PACKET_TYPE pkt_type, UCHAR q_idx)
{
	return swq_to_hif[pkt_type][q_idx];
}

NDIS_STATUS mtd_pci_build_resource_idx(RTMP_ADAPTER *pAd)
{
	UINT32 i;
	PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 data_ring_idx[5] = {0}; /* total should be 4 AC + 1 ALTX */
	UINT8 data_ring_cnt = 0;

	/* find out tx data ring's resource index */
	for (i = 0; i < GET_NUM_OF_TX_RING(cap); i++) {
		struct hif_pci_tx_ring tx_ring = pci_hif->TxRing[i];

		if (tx_ring.ring_attr == TX_RING_DATA && data_ring_cnt < 5) {
			data_ring_idx[data_ring_cnt] = tx_ring.resource_idx;
			data_ring_cnt++;
		}
	}

	/* check number of tx ring matches */
	if (data_ring_cnt != 5)
		return NDIS_STATUS_FAILURE;

	/* build swq to SW TxRing mapping */
	for (i = 0; i < PACKET_TYPE_NUM; i++) {
		switch (i) {
		case TX_DATA:
			pci_hif->swq_to_tx_ring[i][0] = data_ring_idx[0];
			pci_hif->swq_to_tx_ring[i][1] = data_ring_idx[1];
			pci_hif->swq_to_tx_ring[i][2] = data_ring_idx[2];
			pci_hif->swq_to_tx_ring[i][3] = data_ring_idx[3];
			break;
		case TX_DATA_HIGH_PRIO:
			pci_hif->swq_to_tx_ring[i][0] = data_ring_idx[0];
			pci_hif->swq_to_tx_ring[i][1] = data_ring_idx[1];
			pci_hif->swq_to_tx_ring[i][2] = data_ring_idx[2];
			pci_hif->swq_to_tx_ring[i][3] = data_ring_idx[3];
			break;
		case TX_MGMT:
			pci_hif->swq_to_tx_ring[i][0] = data_ring_idx[1];
			pci_hif->swq_to_tx_ring[i][1] = data_ring_idx[1];
			pci_hif->swq_to_tx_ring[i][2] = data_ring_idx[1];
			pci_hif->swq_to_tx_ring[i][3] = data_ring_idx[1];
			break;
		case TX_ALTX:
			pci_hif->swq_to_tx_ring[i][0] = data_ring_idx[4];
			pci_hif->swq_to_tx_ring[i][1] = data_ring_idx[4];
			pci_hif->swq_to_tx_ring[i][2] = data_ring_idx[4];
			pci_hif->swq_to_tx_ring[i][3] = data_ring_idx[4];
			break;
		default:
			pci_hif->swq_to_tx_ring[i][0] = 0;
			pci_hif->swq_to_tx_ring[i][1] = 0;
			pci_hif->swq_to_tx_ring[i][2] = 0;
			pci_hif->swq_to_tx_ring[i][3] = 0;
			break;
		}
	}

	return NDIS_STATUS_SUCCESS;
}

UCHAR *mt_pci_get_hif_buf(RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk, UCHAR hif_idx, UCHAR frame_type)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = &hif->TxRing[hif_idx];
	USHORT tx_cpu_idx = tx_ring->TxCpuIdx;

	if (frame_type != TX_FRAG_FRAME)
		return (UCHAR *)tx_ring->Cell[tx_cpu_idx].DmaBuf.AllocVa;
	else
		return (UCHAR *)tx_blk->HeaderBuffer;
}

PNDIS_PACKET mtd_pci_get_pkt_from_rx_resource(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN *pbReschedule,
	UINT32 *pRxPending,
	UCHAR RxRingNo)
{
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	struct hif_pci_rx_ring *pRxRing = &pAd->PciHif.RxRing[RxRingNo];
	NDIS_SPIN_LOCK *pRxRingLock = &pAd->PciHif.RxRing[RxRingNo].ring_lock;
	PNDIS_PACKET pRxPacket = NULL, pNewPacket = NULL;
	VOID *AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell = NULL;
#ifdef RX_SCATTER
	UINT scatterCnt = 1;
	UINT pktScatterGatherSize = 0;
#endif /* RX_SCATTER */
	UINT pkt_buf_size = 0;
#if defined(CONFIG_WIFI_BUILD_SKB) || defined(RX_SCATTER)
	UINT skb_data_len = 0;
#endif

	RTMP_SEM_LOCK(pRxRingLock);

	pRxCell = &pRxRing->Cell[pRxRing->RxSwReadIdx];

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);
#ifdef RT_BIG_ENDIAN
	pDestRxD = (RXD_STRUC *)pRxCell->AllocVa;
	/* RxD = *pDestRxD; */
	NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
	pRxD = (RXD_STRUC *)&rx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
	/* Point to Rx indexed rx ring descriptor */
	pRxD = (RXD_STRUC *)pRxCell->AllocVa;
#endif
	if (pRxD->DDONE == 0) {
		bReschedule = FALSE;
		goto done;
	}

	*pRxPending = 1;
#if defined(CONFIG_WIFI_BUILD_SKB) || defined(RX_SCATTER)
	skb_data_len = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + pRxCell->DmaBuf.AllocSize) +
				SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV);
#endif

#ifdef RX_SCATTER
	if (IS_ASIC_CAP(pAd, fASIC_CAP_RX_DMA_SCATTER)) {
		scatterCnt = rx_scatter_info(pRxRing, pRxD, &pktScatterGatherSize);

		if (scatterCnt < 1) {
			bReschedule = TRUE;
			goto done;
		}

		pkt_buf_size = pktScatterGatherSize;
	} else
#endif /* RX_SCATTER */
	{
		pkt_buf_size = pRxD->SDL0;
	}

	if (RxRingNo == HIF_RX_IDX0) {
		pNewPacket = RTMP_AllocateRxPacketBuffer(pRxRing,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				DYNAMIC_PAGE_ALLOC,
				pRxCell->DmaBuf.AllocSize,
				&AllocVa, &AllocPa);
	} else if (RxRingNo == HIF_RX_IDX1) {
		pNewPacket = RTMP_AllocateRxPacketBuffer(pRxRing,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				PRE_SLAB_ALLOC,
				pRxCell->DmaBuf.AllocSize,
				&AllocVa, &AllocPa);

		pRxRing->cur_free_buf_len = FREE_BUF_1k;
	}
	if (pNewPacket) {
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
						 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

#ifdef RX_SCATTER
		if (IS_ASIC_CAP(pAd, fASIC_CAP_RX_DMA_SCATTER)
			&& (scatterCnt > 1)) {

			if (rx_scatter_gather(pAd, RxRingNo, pRxCell,
				pRxD, scatterCnt,
				pkt_buf_size, &skb_data_len) == FALSE)	{
				RELEASE_NDIS_PACKET_IRQ(pAd, pNewPacket, NDIS_STATUS_SUCCESS);
				bReschedule = TRUE;
				goto done;
			}
		}
#endif /* RX_SCATTER */

#ifdef CONFIG_WIFI_BUILD_SKB
		{
			void *rx_data = pRxCell->pNdisPacket;

			if (RxRingNo == HIF_RX_IDX0) {
				if (rx_data) {

					/* skb_data_len > page_size case is only working when RX_SCATTER enabled */
					if (skb_data_len <= PAGE_SIZE) {
						DEV_BUILD_SKB(pRxPacket, rx_data, skb_data_len);
					} else {
						DEV_BUILD_SKB(pRxPacket, rx_data, 0);
					}

					if (!pRxPacket) {
						if (skb_data_len <= PAGE_SIZE) {
							DEV_FREE_FRAG_BUF(rx_data);
						} else {
							os_free_mem(rx_data);
						}
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL,
							DBG_LVL_WARN,
							("%s, build_skb return NULL\n",
							__func__));
					} else {
						DEV_SKB_PTR_ADJUST(pRxPacket,
							pkt_buf_size, rx_data);
					}
				} else
					pRxPacket = NULL;
			} else {
				pRxPacket = pRxCell->pNdisPacket;
			}
		}
#else /* CONFIG_WIFI_BUILD_SKB */
		pRxPacket = pRxCell->pNdisPacket;
#endif /* CONFIG_WIFI_BUILD_SKB */
		pRxCell->pNdisPacket = (PNDIS_PACKET)pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;
		pRxCell->DmaBuf.AllocSize = pRxRing->RxBufferSize;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);
		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;
		pRxD->SDL0 = pRxRing->RxBufferSize;

		/*avoid dummy packet received before System Ready*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY) && RxRingNo == 0) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): system is not ready, rx pkt drop it.\n", __func__));

			if (pRxPacket) {
				RELEASE_NDIS_PACKET_IRQ(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				pRxPacket = NULL;
				bReschedule = TRUE;
			}
		}
	} else {
		bReschedule = TRUE;
		goto done;
	}

	pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif
	INC_RING_INDEX(pRxRing->RxSwReadIdx, pRxRing->ring_size);
	pRxRing->sw_read_idx_inc++;
	pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0)
						? (pRxRing->ring_size - 1) : (pRxRing->RxSwReadIdx - 1);

#ifdef CONFIG_WIFI_PREFETCH_RXDATA
	/* prefetch to enhance throughput */
	if ((RxRingNo == 0) && *pRxPending > 0)
		prefetch(pRxRing->Cell[pRxRing->RxSwReadIdx].pNdisPacket);
#endif /* CONFIG_WIFI_PREFETCH_RXDATA */

done:
	RTMP_SEM_UNLOCK(pRxRingLock);
	*pbReschedule = bReschedule;
	return pRxPacket;
}

#ifdef MT7626_E2_SUPPORT
PNDIS_PACKET mtd_pci_get_pkt_from_rx2_resource(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN *pbReschedule,
	UINT32 *pRxPending,
	UCHAR RxRingNo)
{
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	struct hif_pci_rx_ring *pRxRing = &pAd->PciHif.RxRing[RxRingNo];
	NDIS_SPIN_LOCK *pRxRingLock = &pAd->PciHif.RxRing[RxRingNo].ring_lock;
	PNDIS_PACKET pRxPacket = NULL, pNewPacket = NULL;
	VOID *AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell = NULL;
#ifdef RX_SCATTER
	UINT scatterCnt = 1;
	UINT pktScatterGatherSize = 0;
#endif /* RX_SCATTER */
	UINT pkt_buf_size = 0;
#if defined(CONFIG_WIFI_BUILD_SKB) || defined(RX_SCATTER)
	UINT skb_data_len = 0;
#endif

	RTMP_SEM_LOCK(pRxRingLock);

	pRxCell = &pRxRing->Cell[pRxRing->RxSwReadIdx];

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);
#ifdef RT_BIG_ENDIAN
	pDestRxD = (RXD_STRUC *)pRxCell->AllocVa;
	/* RxD = *pDestRxD; */
	NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
	pRxD = (RXD_STRUC *)&rx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
	/* Point to Rx indexed rx ring descriptor */
	pRxD = (RXD_STRUC *)pRxCell->AllocVa;
#endif
	if (pRxD->DDONE == 0) {
		bReschedule = FALSE;
		goto done;
	}

	*pRxPending = 1;
#if defined(CONFIG_WIFI_BUILD_SKB) || defined(RX_SCATTER)
	skb_data_len = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + pRxCell->DmaBuf.AllocSize) +
				   SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV);
#endif

#ifdef RX_SCATTER
	if (IS_ASIC_CAP(pAd, fASIC_CAP_RX_DMA_SCATTER)) {
		scatterCnt = rx_scatter_info(pRxRing, pRxD, &pktScatterGatherSize);

		if (scatterCnt < 1) {
			bReschedule = TRUE;
			goto done;
		}

		pkt_buf_size = pktScatterGatherSize;
	} else
#endif /* RX_SCATTER */
	{
		pkt_buf_size = pRxD->SDL0;
	}

	if (RxRingNo == HIF_RX_IDX2) {
		pNewPacket = RTMP_AllocateRxPacketBuffer(pRxRing,
					 ((POS_COOKIE)(pAd->OS_Cookie))->pDev,
					 DYNAMIC_PAGE_ALLOC,
					 pRxCell->DmaBuf.AllocSize,
					 &AllocVa, &AllocPa);
	}

	if (pNewPacket) {
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
						 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

#ifdef RX_SCATTER
		if (IS_ASIC_CAP(pAd, fASIC_CAP_RX_DMA_SCATTER)
				&& (scatterCnt > 1)) {

			if (rx2_scatter_gather(pAd, RxRingNo, pRxCell,
								  pRxD, scatterCnt,
								  pkt_buf_size, &skb_data_len) == FALSE)	{
				RELEASE_NDIS_PACKET_IRQ(pAd, pNewPacket, NDIS_STATUS_SUCCESS);
				bReschedule = TRUE;
				goto done;
			}
		}
#endif /* RX_SCATTER */

#ifdef CONFIG_WIFI_BUILD_SKB
		{
			void *rx_data = pRxCell->pNdisPacket;

			if (RxRingNo == HIF_RX_IDX2) {
				if (rx_data) {

					/* skb_data_len > page_size case is only working when RX_SCATTER enabled */
					if (skb_data_len <= PAGE_SIZE) {
						DEV_BUILD_SKB(pRxPacket, rx_data, skb_data_len);
					} else {
						DEV_BUILD_SKB(pRxPacket, rx_data, 0);
					}

					if (!pRxPacket) {
						if (skb_data_len <= PAGE_SIZE) {
							DEV_FREE_FRAG_BUF(rx_data);
						} else {
							os_free_mem(rx_data);
						}
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL,
								 DBG_LVL_WARN,
								 ("%s, build_skb return NULL\n",
								  __func__));
					} else {
						DEV_SKB_PTR_ADJUST(pRxPacket,
										   pkt_buf_size, rx_data);
					}
				} else
					pRxPacket = NULL;
			} else {
				pRxPacket = pRxCell->pNdisPacket;
			}
		}
#else /* CONFIG_WIFI_BUILD_SKB */
		pRxPacket = pRxCell->pNdisPacket;
#endif /* CONFIG_WIFI_BUILD_SKB */
		pRxCell->pNdisPacket = (PNDIS_PACKET)pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;
		pRxCell->DmaBuf.AllocSize = pRxRing->RxBufferSize;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);
		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;
		pRxD->SDL0 = pRxRing->RxBufferSize;

		/*avoid dummy packet received before System Ready*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY) && RxRingNo == 2) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): system is not ready, rx pkt drop it.\n", __func__));

			if (pRxPacket) {
				RELEASE_NDIS_PACKET_IRQ(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				pRxPacket = NULL;
				bReschedule = TRUE;
			}
		}
	} else {
		bReschedule = TRUE;
		goto done;
	}

	pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif
	INC_RING_INDEX(pRxRing->RxSwReadIdx, pRxRing->ring_size);
	pRxRing->sw_read_idx_inc++;
	pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0)
						? (pRxRing->ring_size - 1) : (pRxRing->RxSwReadIdx - 1);

#ifdef CONFIG_WIFI_PREFETCH_RXDATA
	/* prefetch to enhance throughput */
	if ((RxRingNo == 2) && *pRxPending > 0)
		prefetch(pRxRing->Cell[pRxRing->RxSwReadIdx].pNdisPacket);
#endif /* CONFIG_WIFI_PREFETCH_RXDATA */

done:
	RTMP_SEM_UNLOCK(pRxRingLock);
	*pbReschedule = bReschedule;
	return pRxPacket;
}
#endif

PNDIS_PACKET mtd_pci_get_pkt_from_rx_resource_io(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN *pbReschedule,
	UINT32 *pRxPending,
	UCHAR RxRingNo)
{
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *pRxRing = &hif->RxRing[RxRingNo];
	NDIS_SPIN_LOCK *pRxRingLock = &hif->RxRing[RxRingNo].ring_lock;
	PNDIS_PACKET pRxPacket = NULL, pNewPacket = NULL;
	VOID *AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell = NULL;
#ifdef RX_SCATTER
	UINT scatterCnt = 1;
	UINT pktScatterGatherSize = 0;
#endif /* RX_SCATTER */
	UINT pkt_buf_size = 0;
#if defined(CONFIG_WIFI_BUILD_SKB) || defined(RX_SCATTER)
	UINT skb_data_len = 0;
#endif
	UINT16 rx_ring_size = pRxRing->ring_size;

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	RTMP_SEM_LOCK(pRxRingLock);

	if (*pRxPending == 0) {
		/* Get how may packets had been received */
		HIF_IO_READ32(pAd->hdev_ctrl, pRxRing->hw_didx_addr, &pRxRing->RxDmaIdx);

#ifdef CONFIG_TP_DBG
		if (RxRingNo == HIF_TX_IDX0)
			tp_dbg->IoReadRx++;
		else
			tp_dbg->IoReadRx1++;
#endif

		if (pRxRing->RxDmaIdx == pRxRing->RxSwReadIdx) {
			bReschedule = FALSE;
			goto done;
		}

		/* get rx pending count */
		if (pRxRing->RxDmaIdx > pRxRing->RxSwReadIdx)
			*pRxPending = pRxRing->RxDmaIdx - pRxRing->RxSwReadIdx;
		else
			*pRxPending = pRxRing->RxDmaIdx + rx_ring_size - pRxRing->RxSwReadIdx;
	}

	pRxCell = &pRxRing->Cell[pRxRing->RxSwReadIdx];
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);
#ifdef RT_BIG_ENDIAN
	pDestRxD = (RXD_STRUC *)pRxCell->AllocVa;
	/* RxD = *pDestRxD; */
	NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
	pRxD = (RXD_STRUC *)&rx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
	/* Point to Rx indexed rx ring descriptor */
	pRxD = (RXD_STRUC *)pRxCell->AllocVa;
#endif

	if (pRxD->DDONE == 0) {
		*pRxPending = 0;
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, ("DDONE=0!\n"));
		/* DMAIndx had done but DDONE bit not ready */
		bReschedule = TRUE;
		goto done;
	}
#if defined(CONFIG_WIFI_BUILD_SKB) || defined(RX_SCATTER)
	skb_data_len = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + pRxCell->DmaBuf.AllocSize) +
				SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV);
#endif

#ifdef RX_SCATTER
	if (IS_ASIC_CAP(pAd, fASIC_CAP_RX_DMA_SCATTER)) {
		scatterCnt = rx_scatter_info_io(pRxRing, pRxD, &pktScatterGatherSize);

		if (scatterCnt < 1) {
			bReschedule = TRUE;
			goto done;
		}

		pkt_buf_size = pktScatterGatherSize;
	} else
#endif /* RX_SCATTER */
	{
		pkt_buf_size = pRxD->SDL0;
	}

	if (RxRingNo == HIF_RX_IDX0) {
		pNewPacket = RTMP_AllocateRxPacketBuffer(pRxRing,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				DYNAMIC_PAGE_ALLOC,
				pRxCell->DmaBuf.AllocSize,
				&AllocVa, &AllocPa);
	} else if (RxRingNo == HIF_RX_IDX1) {
		pNewPacket = RTMP_AllocateRxPacketBuffer(pRxRing,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				PRE_SLAB_ALLOC,
				pRxCell->DmaBuf.AllocSize,
				&AllocVa, &AllocPa);

		pRxRing->cur_free_buf_len = FREE_BUF_1k;
	}

	if (pNewPacket) {
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
						 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

#ifdef RX_SCATTER
		if (IS_ASIC_CAP(pAd, fASIC_CAP_RX_DMA_SCATTER)
			&& (scatterCnt > 1)) {

			if (rx_scatter_gather(pAd, RxRingNo, pRxCell,
				pRxD, scatterCnt,
				pkt_buf_size, &skb_data_len) == FALSE)	{
				RELEASE_NDIS_PACKET_IRQ(pAd, pNewPacket, NDIS_STATUS_SUCCESS);
				bReschedule = TRUE;
				goto done;
			}
		}
#endif /* RX_SCATTER */

#ifdef CONFIG_WIFI_BUILD_SKB
		{
			void *rx_data = pRxCell->pNdisPacket;

			if (RxRingNo == HIF_RX_IDX0) {
				if (rx_data) {

					/* skb_data_len > page_size case is only working when RX_SCATTER enabled */
					if (skb_data_len <= PAGE_SIZE) {
						DEV_BUILD_SKB(pRxPacket, rx_data, skb_data_len);
					} else {
						DEV_BUILD_SKB(pRxPacket, rx_data, 0);
					}


					if (!pRxPacket) {
						if (skb_data_len <= PAGE_SIZE) {
							DEV_FREE_FRAG_BUF(rx_data);
						} else {
							os_free_mem(rx_data);
						}
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL,
							DBG_LVL_WARN,
							("%s, build_skb return NULL\n",
							__func__));
					} else {
						DEV_SKB_PTR_ADJUST(pRxPacket,
							pkt_buf_size, rx_data);
					}
				} else
					pRxPacket = NULL;
			} else {
				pRxPacket = pRxCell->pNdisPacket;
			}
		}
#else /* CONFIG_WIFI_BUILD_SKB */
		pRxPacket = pRxCell->pNdisPacket;
#endif /* CONFIG_WIFI_BUILD_SKB */

		pRxCell->pNdisPacket = (PNDIS_PACKET)pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;
		pRxCell->DmaBuf.AllocSize = pRxRing->RxBufferSize;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);
		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;
		pRxD->SDL0 = pRxRing->RxBufferSize;

		/*avoid dummy packet received before System Ready*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY) && RxRingNo == 0) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): system is not ready, rx pkt drop it.\n", __func__));

			if (pRxPacket) {
				RELEASE_NDIS_PACKET_IRQ(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				pRxPacket = NULL;
				bReschedule = TRUE;
			}
		}
	} else {
		bReschedule = TRUE;
		goto done;
	}

	*pRxPending = *pRxPending - 1;
	pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif
	INC_RING_INDEX(pRxRing->RxSwReadIdx, rx_ring_size);
	pRxRing->sw_read_idx_inc++;
	pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0)
						? (rx_ring_size - 1) : (pRxRing->RxSwReadIdx - 1);

#ifdef CONFIG_WIFI_PREFETCH_RXDATA
	/* prefetch to enhance throughput */
	if ((RxRingNo == 0) && *pRxPending > 0)
		prefetch(pRxRing->Cell[pRxRing->RxSwReadIdx].pNdisPacket);
#endif /* CONFIG_WIFI_PREFETCH_RXDATA */

done:
	RTMP_SEM_UNLOCK(pRxRingLock);
	*pbReschedule = bReschedule;
	return pRxPacket;

}

PNDIS_PACKET pci_get_pkt_from_rx_resource(
	IN RTMP_ADAPTER *pAd,
	OUT BOOLEAN *pbReschedule,
	INOUT UINT32 *pRxPending,
	UCHAR RxRingNo)
{

	return NULL;
}

BOOLEAN RxRing1DoneInterruptHandle(RTMP_ADAPTER *pAd)
{
	UINT32 RxProcessed, RxPending;
	BOOLEAN bReschedule = FALSE;
	RX_BLK rx_blk, *p_rx_blk = NULL;
	PNDIS_PACKET pkt = NULL;
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *pRxRing = &hif->RxRing[HIF_RX_IDX1];
	UINT16 max_rx_process_cnt = pRxRing->max_rx_process_cnt;
#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	RxProcessed = RxPending = 0;

	/* process whole rx ring */
	while (1) {
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
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

		pkt = arch_ops->get_pkt_from_rx1_resource(pAd, &bReschedule, &RxPending, HIF_RX_IDX1);

		if (pkt) {
			os_zero_mem(&rx_blk, sizeof(RX_BLK));
			p_rx_blk = &rx_blk;
			asic_rx_pkt_process(pAd, HIF_RX_IDX1, p_rx_blk, pkt);
		} else
			break;
	}

	if (pRxRing->sw_read_idx_inc > 0) {
		HIF_IO_WRITE32(pAd->hdev_ctrl, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);
		pRxRing->sw_read_idx_inc = 0;
#ifdef CONFIG_TP_DBG
		tp_dbg->IoWriteRx1++;
#endif
	}

#ifdef CONFIG_TP_DBG
	RxProcessed--;
	/* 0 <= Rx1MaxProcessCntA <= 1/4 <= Rx1MaxProcessCntB <= 1/2 <= Rx1MaxProcessCntC < 1 == Rx1MaxProcessCntD */
	if (RxProcessed <= (max_rx_process_cnt >> 2))
		tp_dbg->Rx1MaxProcessCntA++;
	else if (RxProcessed <= (max_rx_process_cnt >> 1))
		tp_dbg->Rx1MaxProcessCntB++;
	else if (RxProcessed < max_rx_process_cnt)
		tp_dbg->Rx1MaxProcessCntC++;
	else
		tp_dbg->Rx1MaxProcessCntD++;
#endif

	return bReschedule;
}

NDIS_STATUS hif_data_init(RTMP_ADAPTER *pAd)
{
	PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	RTMP_ARCH_OP *ops = hc_get_arch_ops(pAd->hdev_ctrl);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	const struct hif_pci_ring_desc *ring_layout;
	UINT32 i, j;

	pci_hif->dma_done_handle[TX_DATA] = ops->tx_dma_done_handle;
	pci_hif->dma_done_handle[TX_DATA_HIGH_PRIO] = ops->tx_dma_done_handle;
	pci_hif->dma_done_handle[TX_MGMT] = ops->tx_dma_done_handle;
	pci_hif->dma_done_handle[TX_CMD] = ops->cmd_dma_done_handle;
	pci_hif->dma_done_handle[TX_FW_DL] = ops->fwdl_dma_done_handle;
	pci_hif->dma_done_handle[TX_ALTX] = ops->tx_dma_done_handle;

	/* tx: init pci hif data struct based on hif_pci_ring_desc */
	ring_layout = cap->hif_pci_ring_layout.tx_ring_layout;

	for (i = 0; i < GET_NUM_OF_TX_RING(cap); i++) {
		struct hif_pci_ring_desc ring_desc = ring_layout[i];
		struct hif_pci_tx_ring *tx_ring = &pci_hif->TxRing[i];

		tx_ring->resource_idx = i;
		tx_ring->hw_desc_base = ring_desc.hw_desc_base;
		tx_ring->hw_dma_id = ring_desc.hw_dma_id;
		tx_ring->hw_ring_id = ring_desc.hw_ring_id;
		tx_ring->ring_attr = ring_desc.ring_attr;
		tx_ring->ring_size = ring_desc.ring_size;

		switch (tx_ring->ring_attr) {
		case TX_RING_DATA:
			break;
		case TX_RING_CMD:
			/* point pci_hif->cmd_ring to its TxRing instance */
			break;
		case TX_RING_FWDL:
			/* point pci_hif->fwdl_ring to its TxRing instance */
			break;
		default:
			break;
		}
	}

	/* init hw_to_tx_ring to provide mapping between HW Tx ring and SW TxRing resource_idx */
	for (i = 0; i < MAX_HW_DMA_NUM; i++)
		for (j = 0; j < MAX_HW_TX_RING_NUM; j++)
			pci_hif->hw_to_tx_ring[i][j] = 0;

	for (i = 0; i < GET_NUM_OF_TX_RING(cap); i++) {
		struct hif_pci_tx_ring tx_ring = pci_hif->TxRing[i];

		if (tx_ring.hw_dma_id >= MAX_HW_DMA_NUM || tx_ring.hw_ring_id >= MAX_HW_TX_RING_NUM)
			return NDIS_STATUS_FAILURE;

		pci_hif->hw_to_tx_ring[tx_ring.hw_dma_id][tx_ring.hw_ring_id] = tx_ring.resource_idx;
	}

	/* rx: init pci hif data struct based on hif_pci_ring_desc */
	ring_layout = cap->hif_pci_ring_layout.rx_ring_layout;

	for (i = 0; i < GET_NUM_OF_RX_RING(cap); i++) {
		struct hif_pci_ring_desc ring_desc = ring_layout[i];
		struct hif_pci_rx_ring *rx_ring = &pci_hif->RxRing[i];

		rx_ring->resource_idx = i;
		rx_ring->hw_desc_base = ring_desc.hw_desc_base;
		rx_ring->hw_dma_id = ring_desc.hw_dma_id;
		rx_ring->hw_ring_id = ring_desc.hw_ring_id;
		rx_ring->ring_attr = ring_desc.ring_attr;
		rx_ring->ring_size = ring_desc.ring_size;
	}

	/* init hw_to_rx_ring to provide mapping between HW Rx ring and SW RxRing resource_idx */
	for (i = 0; i < MAX_HW_DMA_NUM; i++)
		for (j = 0; j < MAX_HW_RX_RING_NUM; j++)
			pci_hif->hw_to_rx_ring[i][j] = 0;

	for (i = 0; i < GET_NUM_OF_RX_RING(cap); i++) {
		struct hif_pci_rx_ring rx_ring = pci_hif->RxRing[i];

		if (rx_ring.hw_dma_id >= MAX_HW_DMA_NUM || rx_ring.hw_ring_id >= MAX_HW_RX_RING_NUM)
			return NDIS_STATUS_FAILURE;

		pci_hif->hw_to_rx_ring[rx_ring.hw_dma_id][rx_ring.hw_ring_id] = rx_ring.resource_idx;
	}

	/* build swq to SW TxRing resource_idx mapping by arch ops */
	if (ops->build_resource_idx) {
		if (ops->build_resource_idx(pAd) != NDIS_STATUS_SUCCESS)
			return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS RTMPFreeHifAdapterBlock(RTMP_ADAPTER *pAd)
{
	struct _PCI_HIF_T *hif;

	hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	NdisFreeSpinLock(&pAd->McuCmdLock);
	NdisFreeSpinLock(&pAd->LockInterrupt);
#ifdef CONFIG_ANDES_SUPPORT
	NdisFreeSpinLock(&hif->ctrl_ring.ring_lock);
	NdisFreeSpinLock(&hif->fwdl_ring.ring_lock);
#endif
	NdisFreeSpinLock(&pAd->tssi_lock);
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS RTMPInitHifAdapterBlock(RTMP_ADAPTER *pAd)
{
	/*first hif initial part on adapter allocate*/
	struct _PCI_HIF_T *hif = &pAd->PciHif;
#ifdef CONFIG_ANDES_SUPPORT
	NdisAllocateSpinLock(pAd, &hif->ctrl_ring.ring_lock);
	NdisAllocateSpinLock(pAd, &hif->fwdl_ring.ring_lock);
	/*TODO and NOTE: this size shall reference from the value of tx_layout afterward.*/
	hif->fwdl_ring.ring_size = CTL_RING_SIZE;
#endif /* CONFIG_ANDES_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}

VOID mt_int_enable(RTMP_ADAPTER *pAd, unsigned int mode)
{
	UINT32 regValue;
	PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	pci_hif->intDisableMask &= ~(mode);
	regValue = pci_hif->IntEnableReg & ~(pci_hif->intDisableMask);
	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_MASK_CSR, regValue);

	if (regValue != 0)
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
}

VOID mt_int_disable(RTMP_ADAPTER *pAd, unsigned int mode)
{
	UINT32 regValue;
	PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	pci_hif->intDisableMask |= mode;
	regValue = pci_hif->IntEnableReg & ~(pci_hif->intDisableMask);
	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_MASK_CSR, regValue);
#ifdef RTMP_MAC_PCI
	/*  Push write command to take effect quickly (i.e. flush the write data)  */
	HIF_IO_READ32(pAd->hdev_ctrl, MT_INT_MASK_CSR, &regValue);
#endif /* RTMP_MAC_PCI */

	if (regValue == 0)
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
}

#ifdef MT7626_E2_SUPPORT
VOID mt_int_md_enable(RTMP_ADAPTER *pAd, unsigned int mode)
{
	UINT32 regValue;
	PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	pci_hif->intDisableMask_md &= ~(mode);
	regValue = pci_hif->IntEnableReg_md & ~(pci_hif->intDisableMask_md);
	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT2_MASK_CSR, regValue);

	if (regValue != 0)
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
}

VOID mt_int_md_disable(RTMP_ADAPTER *pAd, unsigned int mode)
{
	UINT32 regValue;
	PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	pci_hif->intDisableMask_md |= mode;
	regValue = pci_hif->IntEnableReg_md & ~(pci_hif->intDisableMask_md);
	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT2_MASK_CSR, regValue);
#ifdef RTMP_MAC_PCI
	/*  Push write command to take effect quickly (i.e. flush the write data)  */
	HIF_IO_READ32(pAd->hdev_ctrl, MT_INT2_MASK_CSR, &regValue);
#endif /* RTMP_MAC_PCI */
	if (regValue == 0)
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
}
#endif

VOID tx_dma_done_func(RTMP_ADAPTER *pAd)
{
	unsigned long flags;
	PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	UINT32 int_pending;
	RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct tm_ops *tm_ops = pAd->tm_hif_ops;

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pci_hif->intDisableMask &= ~(MT_INT_TX_DONE);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

#ifdef ERR_RECOVERY
	if (IsStopingPdma(&pAd->ErrRecoveryCtl)) {
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pci_hif->intDisableMask &= ~(MT_INT_TX_DONE);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
#endif /* ERR_RECOVERY */

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	int_pending = pci_hif->IntPending;
	pci_hif->IntPending &= ~(MT_INT_TX_DONE);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	if (int_pending & MT_INT_T15_DONE)
		pci_hif->dma_done_handle[chip_cap->hif_pkt_type[HIF_TX_IDX15]](pAd, HIF_TX_IDX15);

	if (int_pending & MT_INT_T5_DONE)
		pci_hif->dma_done_handle[chip_cap->hif_pkt_type[HIF_TX_IDX5]](pAd, HIF_TX_IDX5);

	if (int_pending & MT_INT_T4_DONE)
		pci_hif->dma_done_handle[chip_cap->hif_pkt_type[HIF_TX_IDX4]](pAd, HIF_TX_IDX4);

	if (int_pending & MT_INT_T3_DONE)
		pci_hif->dma_done_handle[chip_cap->hif_pkt_type[HIF_TX_IDX3]](pAd, HIF_TX_IDX3);

	if (int_pending & MT_INT_T2_DONE)
		pci_hif->dma_done_handle[chip_cap->hif_pkt_type[HIF_TX_IDX2]](pAd, HIF_TX_IDX2);

	if (int_pending & MT_INT_T1_DONE)
		pci_hif->dma_done_handle[chip_cap->hif_pkt_type[HIF_TX_IDX1]](pAd, HIF_TX_IDX1);

	if (int_pending & MT_INT_T0_DONE)
		pci_hif->dma_done_handle[chip_cap->hif_pkt_type[HIF_TX_IDX0]](pAd, HIF_TX_IDX0);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if (pci_hif->IntPending & MT_INT_TX_DONE) {
		tm_ops->schedule_task(pAd, TX_DONE_TASK);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	mt_int_enable(pAd, MT_INT_TX_DONE);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}

VOID rx_done_func(RTMP_ADAPTER *pAd)
{
	unsigned long flags;
	BOOLEAN	bReschedule = 0;
	POS_COOKIE pObj;
	struct tm_ops *tm_ops = pAd->tm_hif_ops;
	PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("-->%s():\n", __func__));

	if (pAd->BATable.ba_timeout_check) {
		ba_timeout_flush(pAd);

		if (!((pci_hif->IntPending & MT_INT_RX_DATA) ||
			(pci_hif->intDisableMask & MT_INT_RX_DATA)))
			return;
	}

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pci_hif->intDisableMask &= ~(MT_INT_RX_DATA);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE)pAd->OS_Cookie;
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pci_hif->IntPending &= ~(MT_INT_RX_DATA);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	bReschedule = asic_rx_done_handle(pAd);
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);

	if (pci_hif->IntPending & MT_INT_RX_DATA || bReschedule) {
		tm_ops->schedule_task(pAd, RX_DONE_TASK);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	mt_int_enable(pAd, MT_INT_RX_DATA);

	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}

VOID rx1_done_func(RTMP_ADAPTER *pAd)
{
	unsigned long flags;
	BOOLEAN	bReschedule = 0;
	POS_COOKIE pObj;
	PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct tm_ops *tm_ops = pAd->tm_hif_ops;

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	/* if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)) */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pci_hif->intDisableMask &= ~(MT_INT_RX_CMD);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE)pAd->OS_Cookie;
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pci_hif->IntPending &= ~(MT_INT_RX_CMD);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	bReschedule = RxRing1DoneInterruptHandle(pAd);
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);

	/* double check to avoid rotting packet  */
	if ((pci_hif->IntPending & MT_INT_RX_CMD) || bReschedule) {
		tm_ops->schedule_task(pAd, RX1_DONE_TASK);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	mt_int_enable(pAd, MT_INT_RX_CMD);

	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}

VOID tr_done_func(RTMP_ADAPTER *pAd)
{
	unsigned long flags;
	PCI_HIF_T *pci_hif = &pAd->PciHif;
	UINT32 int_pending = 0;
	BOOLEAN reschedule_rx0 = pAd->reschedule_rx0;
	BOOLEAN reschedule_rx1 = pAd->reschedule_rx1;
	struct tm_ops *tm_ops = pAd->tm_hif_ops;
#ifdef CONFIG_TP_DBG
	UINT32 timestamp;
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;

	if (tp_dbg->debug_flag & TP_DEBUG_TIMING) {
		RTMP_IO_READ32(pAd->hdev_ctrl, LPON_FRCR, &timestamp);
		tp_dbg->TRDoneInterval[tp_dbg->TRDoneTimes++ % TP_DBG_TIME_SLOT_NUMS] = (timestamp - tp_dbg->TRDoneTimeStamp);
		tp_dbg->TRDoneTimeStamp = timestamp;
	}
#endif /* CONFIG_TP_DBG */

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pci_hif->intDisableMask &= ~(MT_INT_RX | MT_INT_RX_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
	if (!(reschedule_rx0 || reschedule_rx1)) {
		/* renew IntPending if called by ISR */
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		int_pending = pci_hif->IntPending;
		pci_hif->IntPending &= ~(MT_INT_RX | MT_INT_RX_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	}

	if ((int_pending & MT_INT_RX_DATA) || reschedule_rx0) {
		reschedule_rx0 = asic_rx_done_handle(pAd);
		reschedule_rx1 = RxRing1DoneInterruptHandle(pAd);
	} else if ((int_pending & MT_INT_RX_CMD) || reschedule_rx1) {
		reschedule_rx1 = RxRing1DoneInterruptHandle(pAd);
		reschedule_rx0 = asic_rx_done_handle(pAd);
	} else {
		if (pAd->BATable.ba_timeout_check)
			ba_timeout_flush(pAd);
	}
	pAd->reschedule_rx0 = reschedule_rx0;
	pAd->reschedule_rx1 = reschedule_rx1;

	/* double check to avoid rotting packet  */
	if (reschedule_rx0 || reschedule_rx1) {
		tm_ops->schedule_task(pAd, TR_DONE_TASK);
		return;
	}

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	mt_int_enable(pAd, MT_INT_RX | MT_INT_RX_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}

#ifdef MT7626_E2_SUPPORT
VOID rx2_done_func(RTMP_ADAPTER *pAd)
{
	unsigned long flags;
	PCI_HIF_T *pci_hif = &pAd->PciHif;
	UINT32 int_pending = 0;
	BOOLEAN reschedule_rx2 = pAd->reschedule_rx2;
	struct tm_ops *tm_ops = pAd->tm_hif_ops;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		RTMP_INT_LOCK(&pAd->LockInterrupt_md, flags);
		pci_hif->intDisableMask_md &= ~(MT_INT2_RX | MT_INT2_RX_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt_md, flags);
		return;
	}
	if (!(reschedule_rx2)) {
		/* renew IntPending if called by ISR */
		RTMP_INT_LOCK(&pAd->LockInterrupt_md, flags);
		int_pending = pci_hif->IntPending_md;
		pci_hif->IntPending_md &= ~(MT_INT2_RX | MT_INT2_RX_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt_md, flags);
	}

	if ((int_pending & MT_INT2_RX_DATA) || reschedule_rx2) {
		reschedule_rx2 = asic_rx2_done_handle(pAd);
	}
	pAd->reschedule_rx2 = reschedule_rx2;

	/* double check to avoid rotting packet  */
	if (reschedule_rx2) {
		tm_ops->schedule_task(pAd, RX2_DONE_TASK);
		return;
	}

	RTMP_INT_LOCK(&pAd->LockInterrupt_md, flags);
	mt_int_md_enable(pAd, MT_INT2_RX | MT_INT2_RX_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt_md, flags);
}
#endif

inline VOID isr_handle(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);

	chip_ops->hw_isr(pAd);
}
#ifdef MT7626_E2_SUPPORT
inline VOID isr_handle_md(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);

	chip_ops->hw_isr_md(pAd);
}
#endif

#ifdef ERR_RECOVERY
VOID RTMPHandleInterruptSerDump(RTMP_ADAPTER *pAd)
{
	UINT32 reg_tmp_val;

	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_ser_info) {
		chip_dbg->show_ser_info(pAd->hdev_ctrl);
		goto mibbucket_dump;
	}

	MAC_IO_READ32(pAd->hdev_ctrl, MCU_COM_REG1, &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s,::E  R	, MCU_COM_REG1=0x%08X\n", __func__, reg_tmp_val));

	if (reg_tmp_val == MCU_COM_REG1_SER_PSE) {
		MAC_IO_READ32(pAd->hdev_ctrl, 0xe064, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206c064=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0xe068, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206c068=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0xe06C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206c06C=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x8244, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x82060244=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x8248, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x82060248=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x8258, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x82060258=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x825C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206025C=0x%08X\n", __func__, reg_tmp_val));
	} else if (reg_tmp_val == MCU_COM_REG1_SER_LMAC_TX) {
		MAC_IO_READ32(pAd->hdev_ctrl, 0xe064, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206c064=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0xe068, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206c068=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0xe06C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206c06C=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0xe070, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206c070=0x%08X\n", __func__, reg_tmp_val));
	} else if (reg_tmp_val == MCU_COM_REG1_SER_SEC_RF_RX) {
		MAC_IO_READ32(pAd->hdev_ctrl, 0x21620, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F6020=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x21624, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F6024=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x21628, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F6028=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x2162C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F602C=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x21630, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F6030=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x21634, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F6034=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x21638, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F6038=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x2163C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F603C=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20824, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1024=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20924, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1124=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20a24, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1224=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20b24, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1324=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20820, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1020=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20920, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1120=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20a20, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1220=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20b20, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1320=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20840, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1040=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20844, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1044=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20848, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1048=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x2084C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F104C=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20940, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1140=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20944, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1144=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20948, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1148=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x2094C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F114C=0x%08X\n", __func__, reg_tmp_val));
	}

	MAC_IO_READ32(pAd->hdev_ctrl, (0xc1e4), &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s,::E  R	, 0x820681e4=0x%08X\n", __func__, reg_tmp_val));
	MAC_IO_READ32(pAd->hdev_ctrl, (0xc1e8), &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s,::E  R	, 0x820681e8=0x%08X\n", __func__, reg_tmp_val));
	MAC_IO_READ32(pAd->hdev_ctrl, (0xc2e8), &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s,::E  R	, 0x820682e8=0x%08X\n", __func__, reg_tmp_val));
	MAC_IO_READ32(pAd->hdev_ctrl, (0xc2ec), &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s,::E  R	, 0x820682ec=0x%08X\n", __func__, reg_tmp_val));
	MAC_IO_READ32(pAd->hdev_ctrl, (0x20afc), &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s,::E  R	, 0x820f20fc=0x%08X\n", __func__, reg_tmp_val));
mibbucket_dump:
	/* Add the EDCCA Time for Debug */
	Show_MibBucket_Proc(pAd, "");
}

VOID mt_mac_recovery_func(RTMP_ADAPTER *pAd)
{
	unsigned long Flags;
	UINT32 status;
	UINT32 INT_MCU_CMD = MT_McuCommand;

#if defined(MT7663) || defined(AXE) || defined(MT7626)
	if (IS_MT7663(pAd) || IS_AXE(pAd) || IS_MT7626(pAd))
		INT_MCU_CMD = MT_INT_MCU2HOST_SW_INT_STS;
#endif

	RTMP_SPIN_LOCK_IRQSAVE(&pAd->LockInterrupt, &Flags);
	status = pAd->ErrRecoveryCtl.status;
	pAd->ErrRecoveryCtl.status = 0;
	RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->LockInterrupt, &Flags);
	RTMP_MAC_RECOVERY(pAd, status);
	mt_int_enable(pAd, INT_MCU_CMD);
}
#endif

#ifdef CONFIG_FWOWN_SUPPORT
VOID mt_mac_fw_own_func(RTMP_ADAPTER *pAd)
{
	UINT32 INT_FW_CLEAR_OWN = MT_FW_CLEAR_OWN_BIT;

	mt_int_enable(pAd, INT_FW_CLEAR_OWN);
}
#endif

VOID mt_subsys_int_func(RTMP_ADAPTER *pAd)
{
	UINT32 int_source;

	/* read status bit from subsys2host interrupt status register */
	RTMP_IO_READ32(pAd->hdev_ctrl, MT_SUBSYS2HOST_INT_STA, &int_source);

	if (int_source & CONN_HIF_ON_HOST_INT) {
		UINT32 conn_hif_on_host_int_sta;
		RTMP_IO_READ32(pAd->hdev_ctrl, MT_CONN_HIF_ON_IRQ_STAT, &conn_hif_on_host_int_sta);
		if (conn_hif_on_host_int_sta & HOST_OWN_INT) {
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s::DriverOwn = TRUE\n", __func__));
			pAd->bDrvOwn = TRUE;
			RTMP_IO_WRITE32(pAd->hdev_ctrl, MT_CONN_HIF_ON_IRQ_STAT, HOST_OWN_INT);
		}
	}

	mt_int_enable(pAd, MT_INT_SUBSYS_INT_STS);
}

UINT16 GET_TXRING_FREENO(RTMP_ADAPTER *pAd, UCHAR QueIdx)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	UINT16 freeno;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	if (hif->TxRing[QueIdx].TxSwFreeIdx > hif->TxRing[QueIdx].TxCpuIdx)
		freeno = hif->TxRing[QueIdx].TxSwFreeIdx - hif->TxRing[QueIdx].TxCpuIdx - 1;
	else
		freeno = hif->TxRing[QueIdx].TxSwFreeIdx + tx_ring_size - hif->TxRing[QueIdx].TxCpuIdx - 1;

	return freeno;
}

/*internal io read/write*/
/*Read Ops*/
static VOID int_pci_io_force_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	*val = readl((void *)(pci_hif->CSRBaseAddress + reg));
}

static VOID int_pci_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		*val = readl((void *)(pci_hif->CSRBaseAddress + reg));
	else
		*val = 0;
}

static VOID int_pci_io_read8(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	*val = readb((void *)(pci_hif->CSRBaseAddress + reg));
}

/*Write Ops*/
static VOID int_pci_io_force_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	writel(val, (void *)(pci_hif->CSRBaseAddress + reg));
}

static VOID int_pci_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		writel(val, (void *)(pci_hif->CSRBaseAddress + reg));
}

static VOID int_pci_io_write16(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	writew(val, (PUSHORT)(pci_hif->CSRBaseAddress + reg));
}

static VOID int_pci_io_write8(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	writeb(val, (PUCHAR)(pci_hif->CSRBaseAddress + reg));
}

/*handle specific CR and forwarding to platform handle*/

static VOID int_pci_io_ops_init(void *hdev_ctrl)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);
	/*hif*/
	ops->hif_io_forec_read32 = int_pci_io_force_read32;
	ops->hif_io_forec_write32 = int_pci_io_force_write32;
	{
		ops->hif_io_read32 = int_pci_io_read32;
		ops->hif_io_write32 = int_pci_io_write32;
	}
	/*mac*/
	ops->mac_io_read32 = int_pci_io_read32;
	ops->mac_io_write32 = int_pci_io_write32;

	ops->mac_io_read16 = NULL;
	ops->mac_io_write16 = int_pci_io_write16;
	ops->mac_io_read8 = int_pci_io_read8;
	ops->mac_io_write8 = int_pci_io_write8;
	/*phy*/
	ops->phy_io_read32 = int_pci_io_read32;
	ops->phy_io_write32 = int_pci_io_write32;
	/*hw*/
	ops->hw_io_read32 = int_pci_io_read32;
	ops->hw_io_write32 = int_pci_io_write32;
}


/*mstar Platform*/
#if defined(PLATFORM_M_STB)
static VOID mstar_pci_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct os_cookie *cookie = hc_get_os_cookie(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		pci_read_config_dword(cookie->pci_dev, reg + 0x80000000, val);
	else
		*val = 0;
}

static VOID mstar_pci_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct os_cookie *cookie = hc_get_os_cookie(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		pci_write_config_dword(cookie->pci_dev, reg + 0x80000000, val);
}

static VOID mstar_pci_io_read16(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct os_cookie *cookie = hc_get_os_cookie(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		pci_read_config_word(cookie->pci_dev, reg + 0x80000000, val);
	else
		*val = 0;
}

static VOID mstar_pci_io_write16(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct os_cookie *cookie = hc_get_os_cookie(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		pci_write_config_word(cookie->pci_dev, reg + 0x80000000, val);
}

static VOID mstar_pci_io_read8(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct os_cookie *cookie = hc_get_os_cookie(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		pci_read_config_byte(cookie->pci_dev, reg + 0x80000000, val);
	else
		*val = 0;
}

static VOID mstar_pci_io_write8(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct os_cookie *cookie = hc_get_os_cookie(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		pci_write_config_byte(cookie->pci_dev, reg + 0x80000000, val);
}

static VOID mstar_pci_io_ops_init(void *hdev_ctrl)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	/*hif*/
	ops->hif_io_read32 = mstar_pci_io_read32;
	ops->hif_io_write32 = mstar_pci_io_write32;
	ops->hif_io_forec_read32 = NULL;
	ops->hif_io_forec_write32 = NULL;
	/*mac*/
	ops->mac_io_read32 = mstar_pci_io_read32;
	ops->mac_io_write32 = mstar_pci_io_write32;
	ops->mac_io_read16 = mstar_pci_io_read16;
	ops->mac_io_write16 = mstar_pci_io_write16;
	ops->mac_io_read8 = mstar_pci_io_read8;
	ops->mac_io_write8 = mstar_pci_io_write8;
	/*phy*/
	ops->phy_io_read32 = mstar_pci_io_read32;
	ops->phy_io_write32 = mstar_pci_io_write32;

}
#endif /*PLATFORM_M_STB*/

#if defined(INF_TWINPASS) || defined(INF_DANUBE) || defined(INF_AR9) || defined(IKANOS_VX_1X0)

static VOID ext_pci_forec_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	*val = readl((void *)((pci_hif->CSRBaseAddress + reg)));
	*val = SWAP32(*((UINT32 *)(val)));
}

static VOID ext_pci_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE) {
		*val = readl((void *)((pci_hif->CSRBaseAddress + reg)));
		*val = SWAP32(*((UINT32 *)(val)));
	}
}

static VOID ext_pci_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE) {
		val = SWAP32(val);
		writel(val, (pci_hif->CSRBaseAddress + reg));
	}
}

static VOID ext_pci_io_write16(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	val = SWAP16(val);
	writew(val, (PUSHORT)(pci_hif->CSRBaseAddress + reg));
}

static VOID ext_pci_io_ops_init(void *hdev_ctrl)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	/*hif*/
	ops->hif_io_read32 = ext_pci_io_read32;
	ops->hif_io_write32 = ext_pci_io_write32;
	ops->hif_io_forec_read32 = ext_pci_forec_read32;
	ops->hif_io_forec_write32 = NULL;
	/*mac*/
	ops->mac_io_read32 = ext_pci_io_read32;
	ops->mac_io_write32 = ext_pci_io_write32;
	ops->mac_io_read16 = NULL;
	ops->mac_io_write16 = ext_pci_io_write16;
	ops->mac_io_read8 = int_pci_io_read8;
	ops->mac_io_write8 = int_pci_io_write8;
	/*phy*/
	ops->phy_io_read32 = ext_pci_io_read32;
	ops->phy_io_write32 = ext_pci_io_write32;
	/*hw*/
	ops->hw_io_read32 = ext_pci_io_read32;
	ops->hw_io_write32 = ext_pci_io_write32;
}

#endif


VOID pci_io_ops_init(void *hdev_ctrl)
{
#if defined(PLATFORM_M_STB)
	mstar_pci_io_ops_init(hdev_ctrl);
#elif defined(INF_TWINPASS) || defined(INF_DANUBE) || defined(INF_AR9) || defined(IKANOS_VX_1X0)
	ext_pci_io_ops_init(hdev_ctrl);
#else
	int_pci_io_ops_init(hdev_ctrl);
#endif
}


