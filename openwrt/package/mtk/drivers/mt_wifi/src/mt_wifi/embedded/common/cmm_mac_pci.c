/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/



#ifdef RTMP_MAC_PCI
#include	"rt_config.h"


static INT desc_ring_alloc(RTMP_ADAPTER *pAd, RTMP_DMABUF *pDescRing, INT size)
{
	VOID *pdev = ((POS_COOKIE)(pAd->OS_Cookie))->pDev;

	pDescRing->AllocSize = size;
	RtmpAllocDescBuf(pdev,
					 0,
					 pDescRing->AllocSize,
					 FALSE,
					 &pDescRing->AllocVa,
					 &pDescRing->AllocPa);

	if (pDescRing->AllocVa == NULL) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Failed to allocate a big buffer\n"));
		return ERRLOG_OUT_OF_SHARED_MEMORY;
	}

	/* Zero init this memory block*/
	NdisZeroMemory(pDescRing->AllocVa, size);
	return 0;
}

NDIS_STATUS alloc_chip_cap_dep_data_pci(void *hdev_ctrl)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(hdev_ctrl);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	UINT8 num_of_rx_ring = GET_NUM_OF_RX_RING(cap);
	INT index;
	struct hif_pci_rx_ring *pRxRing = NULL;
	struct hif_pci_tx_ring *pTxRing = NULL;

	/* allocate pci hif data structure which depends on ChipCap */
	os_alloc_mem(NULL, (UCHAR **)&pTxRing, (num_of_tx_ring)*sizeof(struct hif_pci_tx_ring));

	if (pTxRing == NULL)
		return NDIS_STATUS_FAILURE;

	NdisZeroMemory(pTxRing, (num_of_tx_ring)*sizeof(struct hif_pci_tx_ring));
	hif->TxRing = pTxRing;

	os_alloc_mem(NULL, (UCHAR **)&pRxRing, (num_of_rx_ring)*sizeof(struct hif_pci_rx_ring));

	if (pRxRing == NULL)
		return NDIS_STATUS_FAILURE;

	NdisZeroMemory(pRxRing, (num_of_rx_ring)*sizeof(struct hif_pci_rx_ring));
	hif->RxRing = pRxRing;

	/* spinlock is allocated after data structure is ready */
	for (index = 0; index < num_of_tx_ring; index++)
		OS_NdisAllocateSpinLock(&hif->TxRing[index].ring_lock);

	for (index = 0; index < num_of_rx_ring; index++)
		OS_NdisAllocateSpinLock(&hif->RxRing[index].ring_lock);

	return NDIS_STATUS_SUCCESS;
}

VOID free_chip_cap_dep_data_pci(void *hdev_ctrl)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(hdev_ctrl);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	UINT8 num_of_rx_ring = GET_NUM_OF_RX_RING(cap);
	INT index;

	/* free pci hif data structure which depends on ChipCap */

	/* spinlock must be freed before its data structure is freed */
	for (index = 0; index < num_of_tx_ring; index++)
		OS_NdisFreeSpinLock(&hif->TxRing[index].ring_lock);

	for (index = 0; index < num_of_rx_ring; index++)
		OS_NdisFreeSpinLock(&hif->RxRing[index].ring_lock);

	os_free_mem(hif->TxRing);
	os_free_mem(hif->RxRing);
}

inline VOID *alloc_rx_buf_1k(void *hif_resource)
{
	struct hif_pci_rx_ring *rx_ring = (struct hif_pci_rx_ring *)hif_resource;
	RTMP_DMABUF *dma_buf = &rx_ring->free_buf[rx_ring->free_buf_head];

	INC_INDEX(rx_ring->free_buf_head, rx_ring->free_buf_size);

	return dma_buf->AllocVa;
}

inline VOID free_rx_buf_1k(void *hif_resource)
{
	struct hif_pci_rx_ring *rx_ring = (struct hif_pci_rx_ring *)hif_resource;

	INC_INDEX(rx_ring->free_buf_tail, rx_ring->free_buf_size);
}

inline VOID *alloc_rx_buf_64k(void *hif_resource)
{
	struct hif_pci_rx_ring *rx_ring = (struct hif_pci_rx_ring *)hif_resource;
	RTMP_DMABUF *dma_buf = &rx_ring->free_buf_64k[rx_ring->free_buf_64k_head];

	INC_INDEX(rx_ring->free_buf_64k_head, rx_ring->free_buf_64k_size);

	return dma_buf->AllocVa;
}

inline VOID free_rx_buf_64k(void *hif_resource)
{
	struct hif_pci_rx_ring *rx_ring = (struct hif_pci_rx_ring *)hif_resource;

	INC_INDEX(rx_ring->free_buf_64k_tail, rx_ring->free_buf_64k_size);
}

VOID free_rx_buf(void *hdev_ctrl, UCHAR hif_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = &hif->RxRing[hif_idx];

	if (rx_ring->cur_free_buf_len == FREE_BUF_1k) {
		free_rx_buf_1k(rx_ring);
	} else if (rx_ring->cur_free_buf_len == FREE_BUF_64k) {
		free_rx_buf_64k(rx_ring);
	} else {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
			("%s: fail, rx_ring->cur_free_buf_len = %d\n", __func__, rx_ring->cur_free_buf_len));
	}
}

static UINT32 alloc_rx_free_buffer(RTMP_ADAPTER *pAd, UCHAR hif_idx, UINT32 num)
{
	UINT32 i;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = &hif->RxRing[hif_idx];
	RTMP_DMABUF *dma_buf;

	rx_ring->free_buf_head = 0;
	rx_ring->free_buf_tail = 0;
	rx_ring->free_buf_size = (num + 1);

	os_alloc_mem(pAd, (UCHAR **)&rx_ring->free_buf, rx_ring->free_buf_size * sizeof(struct _RTMP_DMABUF));
	NdisZeroMemory(rx_ring->free_buf, rx_ring->free_buf_size * sizeof(struct _RTMP_DMABUF));

	for (i = 0; i < rx_ring->free_buf_size; i++) {
		dma_buf = &rx_ring->free_buf[i];
		dma_buf->AllocSize = RX1_BUFFER_SIZE;
		os_alloc_mem(pAd, (UCHAR **)&dma_buf->AllocVa, dma_buf->AllocSize);
		NdisZeroMemory(dma_buf->AllocVa, dma_buf->AllocSize);
	}

	rx_ring->free_buf_64k_head = 0;
	rx_ring->free_buf_64k_tail = 0;
	rx_ring->free_buf_64k_size = 1;

	os_alloc_mem(pAd, (UCHAR **)&rx_ring->free_buf_64k, rx_ring->free_buf_64k_size * sizeof(struct _RTMP_DMABUF));
	NdisZeroMemory(rx_ring->free_buf_64k, rx_ring->free_buf_64k_size * sizeof(struct _RTMP_DMABUF));

	for (i = 0; i < rx_ring->free_buf_64k_size; i++) {
		dma_buf = &rx_ring->free_buf_64k[i];
		dma_buf->AllocSize = 65536;
		os_alloc_mem(pAd, (UCHAR **)&dma_buf->AllocVa, dma_buf->AllocSize);
		NdisZeroMemory(dma_buf->AllocVa, dma_buf->AllocSize);
	}

	return 0;
}

#ifdef RESOURCE_PRE_ALLOC
static UINT32 reset_rx_free_buffer(void *hdev_ctrl, UINT8 hif_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = &hif->RxRing[hif_idx];
	UINT32 num = rx_ring->ring_size;
	UINT32 i;
	RTMP_DMABUF *dma_buf;

	rx_ring->free_buf_head = 0;
	rx_ring->free_buf_tail = 0;
	rx_ring->free_buf_size = (num + 1);

	for (i = 0; i < rx_ring->free_buf_size; i++) {
		dma_buf = &rx_ring->free_buf[i];
		dma_buf->AllocSize = RX1_BUFFER_SIZE;
	}

	rx_ring->free_buf_64k_head = 0;
	rx_ring->free_buf_64k_tail = 0;
	rx_ring->free_buf_64k_size = 1;

	for (i = 0; i < rx_ring->free_buf_64k_size; i++) {
		dma_buf = &rx_ring->free_buf_64k[i];
		dma_buf->AllocSize = 65536;
	}

	return 0;
}
#endif

static UINT32 release_rx_free_buffer(void *hdev_ctrl, UINT8 hif_idx)
{
	UINT32 i;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = &hif->RxRing[hif_idx];
	RTMP_DMABUF *dma_buf;

	for (i = 0; i < rx_ring->free_buf_size; i++) {
		dma_buf = &rx_ring->free_buf[i];
		os_free_mem(dma_buf->AllocVa);
	}

	os_free_mem(rx_ring->free_buf);

	for (i = 0; i < rx_ring->free_buf_64k_size; i++) {
		dma_buf = &rx_ring->free_buf_64k[i];
		os_free_mem(dma_buf->AllocVa);
	}

	os_free_mem(rx_ring->free_buf_64k);

	return 0;
}


#ifdef RESOURCE_PRE_ALLOC
static INT desc_ring_free(void *hdev_ctrl, RTMP_DMABUF *pDescRing)
{
	struct os_cookie *cookie = hc_get_os_cookie(hdev_ctrl);

	if (pDescRing->AllocVa) {
		RtmpFreeDescBuf(cookie->pDev,
						pDescRing->AllocSize,
						pDescRing->AllocVa,
						pDescRing->AllocPa);
	}

	NdisZeroMemory(pDescRing, sizeof(RTMP_DMABUF));
	return TRUE;
}


VOID RTMPResetTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	int index, j;
	struct hif_pci_tx_ring *pTxRing;
	TXD_STRUC *pTxD;
	RTMP_DMACB *dma_cb;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif /* RT_BIG_ENDIAN */
	PNDIS_PACKET pPacket;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	UINT8 num_of_rx_ring = GET_NUM_OF_RX_RING(cap);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	UINT16 rx0_ring_size = GET_RX0_RING_SIZE(cap);
	UINT16 rx1_ring_size = GET_RX1_RING_SIZE(cap);
#ifdef MT7626_E2_SUPPORT
	UINT16 rx2_ring_size = GET_RX2_RING_SIZE(cap);
	UINT16 rx3_ring_size = GET_RX3_RING_SIZE(cap);
#endif
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	/* Free Tx Ring Packet*/
	for (index = 0; index < num_of_tx_ring; index++) {
		pTxRing = &hif->TxRing[index];

		for (j = 0; j < tx_ring_size; j++) {
			dma_cb = &pTxRing->Cell[j];
#ifdef RT_BIG_ENDIAN
			pDestTxD = (TXD_STRUC *)(dma_cb->AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (dma_cb->AllocVa);
#endif /* RT_BIG_ENDIAN */

			if (dma_cb->DmaBuf.AllocSize > 0)
				PCI_UNMAP_SINGLE(pAd, dma_cb->DmaBuf.AllocPa, dma_cb->DmaBuf.AllocSize, RTMP_PCI_DMA_TODEVICE);

			pPacket = dma_cb->pNdisPacket;

			if (pPacket) {
				PCI_UNMAP_SINGLE(pAd, dma_cb->PacketPa, GET_OS_PKT_LEN(pPacket), RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/*Always assign pNdisPacket as NULL after clear*/
			dma_cb->pNdisPacket = NULL;
			pPacket = dma_cb->pNextNdisPacket;

			if (pPacket) {
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/*Always assign pNextNdisPacket as NULL after clear*/
			dma_cb->pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}

		NdisZeroMemory(pTxRing->Cell, tx_ring_size * sizeof(RTMP_DMACB));
	}

	{
		struct hif_pci_tx_ring *pCtrlRing = &hif->ctrl_ring;

		HIF_IO_READ32(pAd->hdev_ctrl, pCtrlRing->hw_didx_addr, &pCtrlRing->TxDmaIdx);

		while (pCtrlRing->TxSwFreeIdx != pCtrlRing->TxCpuIdx) {
			if (pCtrlRing->TxSwFreeIdx >= CTL_RING_SIZE) {
				/*should not happen*/
				MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("--> TxSwFreeIdx is out of range! %d\n", pCtrlRing->TxSwFreeIdx));
				break;
			}

#ifdef RT_BIG_ENDIAN
			pDestTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
#endif
			pTxD->DMADONE = 0;
			pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket;

			if (pPacket == NULL) {
				INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, CTL_RING_SIZE);
				continue;
			}

			if (pPacket) {
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket = NULL;
			pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNextNdisPacket;

			if (pPacket) {
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNextNdisPacket = NULL;
			INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, CTL_RING_SIZE);
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
		}
	}

	for (index = 0; index < num_of_rx_ring; index++) {
		UINT16 RxRingSize;
#ifdef CONFIG_WIFI_BUILD_SKB
		UINT skb_data_size = 0;
#endif

		if (index == HIF_RX_IDX0)
			RxRingSize = rx0_ring_size;
		else
			RxRingSize = rx1_ring_size;
#ifdef MT7626_E2_SUPPORT
		if (index == HIF_RX_IDX2)
			RxRingSize = rx2_ring_size;
		else if (index == HIF_RX_IDX3)
			RxRingSize = rx3_ring_size;
#endif
		for (j = RxRingSize - 1; j >= 0; j--) {
			dma_cb = &hif->RxRing[index].Cell[j];

			if ((dma_cb->DmaBuf.AllocVa) && (dma_cb->pNdisPacket)) {
				PCI_UNMAP_SINGLE(pAd, dma_cb->DmaBuf.AllocPa, dma_cb->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);

				if (index == HIF_RX_IDX0) {
#ifdef CONFIG_WIFI_BUILD_SKB
					skb_data_size = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + dma_cb->DmaBuf.AllocSize) +
							SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV);

					if (skb_data_size <= PAGE_SIZE) {
						DEV_FREE_FRAG_BUF(dma_cb->pNdisPacket);
					} else {
						os_free_mem(dma_cb->pNdisPacket);
					}
#else /* CONFIG_WIFI_BUILD_SKB */
					RELEASE_NDIS_PACKET(pAd,
							dma_cb->pNdisPacket,
							NDIS_STATUS_SUCCESS);
#endif /* CONFIG_WIFI_BUILD_SKB */
				}
			}
		}

		if (index == HIF_RX_IDX1)
			reset_rx_free_buffer(pAd->hdev_ctrl, HIF_RX_IDX1);

		NdisZeroMemory(hif->RxRing[index].Cell, RxRingSize * sizeof(RTMP_DMACB));
	}

	if (pAd->FragFrame.pFragPacket) {
		RELEASE_NDIS_PACKET(pAd, pAd->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);
		pAd->FragFrame.pFragPacket = NULL;
	}

	NdisFreeSpinLock(&pAd->CmdQLock);
}

VOID RTMPFreeTxRxRingMemory(struct _RTMP_ADAPTER *pAd)
{
	INT num;
	struct os_cookie *cookie = hc_get_os_cookie(pAd->hdev_ctrl);
	VOID *pdev = cookie->pDev;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	UINT8 num_of_rx_ring = GET_NUM_OF_RX_RING(cap);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);


	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("--> RTMPFreeTxRxRingMemory\n"));

	for (num = 0; num < num_of_rx_ring; num++) {
		desc_ring_free(pAd->hdev_ctrl, &hif->RxRing[num].desc_ring);

		if (num == HIF_RX_IDX1)
			release_rx_free_buffer(pAd->hdev_ctrl, num);

		os_free_mem(hif->RxRing[num].Cell);
	}

	os_free_mem(hif->ctrl_ring.Cell);
	desc_ring_free(pAd->hdev_ctrl, &hif->ctrl_ring.desc_ring);
	os_free_mem(hif->fwdl_ring.Cell);
	desc_ring_free(pAd->hdev_ctrl, &hif->fwdl_ring.desc_ring);

	/* Free 1st TxBufSpace and TxDesc buffer*/
	for (num = 0; num < num_of_tx_ring; num++) {
		if (hif->TxRing[num].buf_space.AllocVa) {
			RTMP_FreeFirstTxBuffer(pdev,
						hif->TxRing[num].buf_space.AllocSize,
						FALSE, hif->TxRing[num].buf_space.AllocVa,
						hif->TxRing[num].buf_space.AllocPa);
		}

		NdisZeroMemory(&hif->TxRing[num].buf_space, sizeof(RTMP_DMABUF));
		desc_ring_free(pAd->hdev_ctrl, &hif->TxRing[num].desc_ring);
		os_free_mem(hif->TxRing[num].Cell);

	}

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<-- RTMPFreeTxRxRingMemory\n"));
}

NDIS_STATUS RTMPInitTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	INT num, index;
	ULONG RingBasePaHigh, RingBasePaLow;
	VOID *RingBaseVa;
	struct hif_pci_rx_ring *pRxRing;
	struct hif_pci_tx_ring *pTxRing;
	RTMP_DMABUF *pDmaBuf, *pDescRing;
	RTMP_DMACB *dma_cb;
	PNDIS_PACKET pPacket = NULL;
	TXD_STRUC *pTxD;
	RXD_STRUC *pRxD;
	ULONG ErrorValue = 0;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	UINT8 num_of_rx_ring = GET_NUM_OF_RX_RING(cap);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	UINT16 rx0_ring_size = GET_RX0_RING_SIZE(cap);
	UINT16 rx1_ring_size = GET_RX1_RING_SIZE(cap);
#ifdef MT7626_E2_SUPPORT
	UINT16 rx2_ring_size = GET_RX2_RING_SIZE(cap);
	UINT16 rx3_ring_size = GET_RX3_RING_SIZE(cap);
#endif
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	/* Initialize All Tx Ring Descriptors and associated buffer memory*/
	for (num = 0; num < num_of_tx_ring; num++) {
		VOID *BufBaseVa;
		/* memory zero the  Tx ring descriptor's memory */
		pDescRing = &hif->TxRing[num].desc_ring;
		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
		/* Save PA & VA for further operation*/
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow(pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;
		/* Zero init all 1st TXBuf's memory for this TxRing*/
		NdisZeroMemory(hif->TxRing[num].buf_space.AllocVa, hif->TxRing[num].buf_space.AllocSize);
		
		/* Save PA & VA for further operation */
		BufBaseVa = hif->TxRing[num].buf_space.AllocVa;
		
		/* linking Tx Ring Descriptor and associated buffer memory */
		pTxRing = &hif->TxRing[num];

		for (index = 0; index < tx_ring_size; index++) {
			dma_cb = &pTxRing->Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init Tx Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);
			/* Setup Tx Buffer size & address. only 802.11 header will store in this space */
			pDmaBuf = &dma_cb->DmaBuf;
			pDmaBuf->AllocSize = TX_DMA_1ST_BUFFER_SIZE;
			pDmaBuf->AllocVa = BufBaseVa;
			/* link the pre-allocated TxBuf to TXD */
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
			pTxD->DMADONE = 0;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);
			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR)RingBaseVa + TXD_SIZE;
			/* advance to next TxBuf address */
			BufBaseVa = (PUCHAR)BufBaseVa + TX_DMA_1ST_BUFFER_SIZE;
		}

		pTxRing->tx_ring_state = TX_RING_HIGH;
		pTxRing->tx_ring_low_water_mark = 5;
		pTxRing->tx_ring_high_water_mark = pTxRing->tx_ring_low_water_mark + 10;
		pTxRing->tx_ring_full_cnt = 0;
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("TxRing[%d]: total %d entry initialized\n", num, index));
	}

	/* Initialize CTRL Ring and associated buffer memory */
	pDescRing = &hif->ctrl_ring.desc_ring;
	RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
	RingBasePaLow = RTMP_GetPhysicalAddressLow(pDescRing->AllocPa);
	RingBaseVa = pDescRing->AllocVa;
	NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);

	for (index = 0; index < CTL_RING_SIZE; index++) {
		dma_cb = &hif->ctrl_ring.Cell[index];
		dma_cb->pNdisPacket = NULL;
		dma_cb->pNextNdisPacket = NULL;
		/* Init Ctrl Ring Size, Va, Pa variables */
		dma_cb->AllocSize = TXD_SIZE;
		dma_cb->AllocVa = RingBaseVa;
		RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
		RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);
		/* Offset to next ring descriptor address */
		RingBasePaLow += TXD_SIZE;
		RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;
		/* link the pre-allocated TxBuf to TXD */
		pTxD = (TXD_STRUC *)dma_cb->AllocVa;
		pTxD->DMADONE = 1;
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);
		/* no pre-allocated buffer required in CtrlRing for scatter-gather case */
	}

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("CtrlRing: total %d entry initialized\n", index));
		
	/* Initialize firmware download Ring */
	pDescRing = &hif->fwdl_ring.desc_ring;
	RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
	RingBasePaLow = RTMP_GetPhysicalAddressLow(pDescRing->AllocPa);
	RingBaseVa = pDescRing->AllocVa;
	NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);

	for (index = 0; index < CTL_RING_SIZE; index++) {
		dma_cb = &hif->fwdl_ring.Cell[index];
		dma_cb->pNdisPacket = NULL;
		dma_cb->pNextNdisPacket = NULL;
		
		/* Init firmware download ring Size, Va, Pa variables */
		dma_cb->AllocSize = TXD_SIZE;
		dma_cb->AllocVa = RingBaseVa;
		RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
		RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);
		/* Offset to next ring descriptor address */
		RingBasePaLow += TXD_SIZE;
		RingBaseVa = (PUCHAR)RingBaseVa + TXD_SIZE;
		/* link the pre-allocated TxBuf to TXD */
		pTxD = (TXD_STRUC *)dma_cb->AllocVa;
		pTxD->DMADONE = 1;
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);
		/* no pre-allocated buffer required in FwDwloRing for scatter-gather case */
	}

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("FwDwloRing: total %d entry initialized\n", index));

	/* Initialize Rx Ring and associated buffer memory */
	for (num = 0; num < num_of_rx_ring; num++) {
		UINT16 RxRingSize;
		UINT16 RxBufferSize;

		pDescRing = &hif->RxRing[num].desc_ring;
		pRxRing = &hif->RxRing[num];
		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,  ("RX[%d] DESC %p size = %lu\n",
				 num, pDescRing->AllocVa, pDescRing->AllocSize));
		/* Save PA & VA for further operation */
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow(pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;

		if (num == HIF_RX_IDX0) {
			RxRingSize = rx0_ring_size;
			RxBufferSize = RX_BUFFER_AGGRESIZE;
		} else {
			RxRingSize = rx1_ring_size;
			RxBufferSize = RX1_BUFFER_SIZE;
		}
#ifdef MT7626_E2_SUPPORT
		if (num == HIF_RX_IDX2) {
			RxRingSize = rx2_ring_size;
			RxBufferSize = RX_BUFFER_AGGRESIZE;
		} else if (num == HIF_RX_IDX3) {
			RxRingSize = rx3_ring_size;
			RxBufferSize = RX1_BUFFER_SIZE;
		}
#endif
		/* Linking Rx Ring and associated buffer memory */
		for (index = 0; index < RxRingSize; index++) {
			dma_cb = &pRxRing->Cell[index];
			/* Init RX Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = RXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);
			/* Offset to next ring descriptor address */
			RingBasePaLow += RXD_SIZE;
			RingBaseVa = (PUCHAR)RingBaseVa + RXD_SIZE;
			/* Setup Rx associated Buffer size & allocate share memory */
			pDmaBuf = &dma_cb->DmaBuf;
			pDmaBuf->AllocSize = RxBufferSize;


			if (num == HIF_RX_IDX0) {
				pPacket = RTMP_AllocateRxPacketBuffer(
						pRxRing,
						((POS_COOKIE)(pAd->OS_Cookie))->pDev,
						DYNAMIC_PAGE_ALLOC,
						pDmaBuf->AllocSize,
						&pDmaBuf->AllocVa,
						&pDmaBuf->AllocPa);
			} else if (num == HIF_RX_IDX1) {
				pPacket = RTMP_AllocateRxPacketBuffer(
						pRxRing,
						((POS_COOKIE)(pAd->OS_Cookie))->pDev,
						PRE_SLAB_ALLOC,
						pDmaBuf->AllocSize,
						&pDmaBuf->AllocVa,
						&pDmaBuf->AllocPa);
			}
#ifdef MT7626_E2_SUPPORT
			if (num == HIF_RX_IDX2) {
				pPacket = RTMP_AllocateRxPacketBuffer(
							  pRxRing,
							  ((POS_COOKIE)(pAd->OS_Cookie))->pDev,
							  DYNAMIC_PAGE_ALLOC,
							  pDmaBuf->AllocSize,
							  &pDmaBuf->AllocVa,
							  &pDmaBuf->AllocPa);
			} else if (num == HIF_RX_IDX3) {
				pPacket = RTMP_AllocateRxPacketBuffer(
							  pRxRing,
							  ((POS_COOKIE)(pAd->OS_Cookie))->pDev,
							  PRE_SLAB_ALLOC,
							  pDmaBuf->AllocSize,
							  &pDmaBuf->AllocVa,
							  &pDmaBuf->AllocPa);
			}
#endif
			/* keep allocated rx packet */
			dma_cb->pNdisPacket = pPacket;

			/* Error handling*/
			if (pDmaBuf->AllocVa == NULL) {
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Failed to allocate RxRing's 1st buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Write RxD buffer address & allocated buffer length */
			pRxD = (RXD_STRUC *)dma_cb->AllocVa;
			pRxD->SDP0 = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
			pRxD->DDONE = 0;
			pRxD->SDL0 = RxBufferSize;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#endif
			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pRxD, dma_cb->AllocSize);
		}
	}

	NdisZeroMemory(&pAd->FragFrame, sizeof(FRAGMENT_FRAME));
	pAd->FragFrame.pFragPacket = RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);

	if (pAd->FragFrame.pFragPacket == NULL)
		Status = NDIS_STATUS_RESOURCES;

	for (index = 0; index < num_of_tx_ring; index++) {
		/* Init TX rings index pointer */
		hif->TxRing[index].TxSwFreeIdx = 0;
		hif->TxRing[index].TxCpuIdx = 0;
	}

	/* Init RX Ring index pointer */
	for (index = 0; index < num_of_rx_ring; index++) {
		UINT16 RxRingSize;
		UINT16 RxBufferSize;
		UINT16 max_rx_process_cnt;

		if (index == HIF_RX_IDX0) {
			RxRingSize = rx0_ring_size;
			RxBufferSize = RX_BUFFER_AGGRESIZE;
			max_rx_process_cnt = MAX_RX_PROCESS_CNT;
		} else {
			RxRingSize = rx1_ring_size;
			RxBufferSize = RX1_BUFFER_SIZE;
			max_rx_process_cnt = MAX_RX1_PROCESS_CNT;
		}
#ifdef MT7626_E2_SUPPORT
		if (index == HIF_RX_IDX2) {
			RxRingSize = rx2_ring_size;
			RxBufferSize = RX_BUFFER_AGGRESIZE;
			max_rx_process_cnt = MAX_RX_PROCESS_CNT;
		} else if (index == HIF_RX_IDX3) {
			RxRingSize = rx3_ring_size;
			RxBufferSize = RX1_BUFFER_SIZE;
			max_rx_process_cnt = MAX_RX1_PROCESS_CNT;
		}
#endif
		hif->RxRing[index].RxSwReadIdx = 0;
		hif->RxRing[index].RxCpuIdx = RxRingSize - 1;
		hif->RxRing[index].ring_size = RxRingSize;
		hif->RxRing[index].RxBufferSize = RxBufferSize;
		hif->RxRing[index].max_rx_process_cnt = max_rx_process_cnt;
	}

#ifdef CONFIG_ANDES_SUPPORT
	/* init CTRL ring index pointer */
	hif->ctrl_ring.TxSwFreeIdx = 0;
	hif->ctrl_ring.TxCpuIdx = 0;
#endif /* CONFIG_ANDES_SUPPORT */
	pAd->PrivateInfo.TxRingFullCnt = 0;
	return Status;
}

NDIS_STATUS RTMPAllocTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	INT num;
	ULONG ErrorValue = 0;
	VOID *pdev = ((POS_COOKIE)(pAd->OS_Cookie))->pDev;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	UINT8 num_of_rx_ring = GET_NUM_OF_RX_RING(cap);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	UINT16 rx0_ring_size = GET_RX0_RING_SIZE(cap);
	UINT16 rx1_ring_size = GET_RX1_RING_SIZE(cap);
#ifdef MT7626_E2_SUPPORT
	UINT16 rx2_ring_size = GET_RX2_RING_SIZE(cap);
	UINT16 rx3_ring_size = GET_RX3_RING_SIZE(cap);
#endif
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("-->RTMPAllocTxRxRingMemory\n"));

	do {
		for (num = 0; num < num_of_tx_ring; num++) {
			struct hif_pci_tx_ring *pTxRing = &hif->TxRing[num];

			/* Allocate Tx ring descriptor's memory (5 TX rings = 4 ACs + 1 HCCA)*/
			desc_ring_alloc(pAd, &hif->TxRing[num].desc_ring, tx_ring_size * TXD_SIZE);

			if (hif->TxRing[num].desc_ring.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("TxRing[%d]: total %d bytes allocated\n",
					 num, (INT)hif->TxRing[num].desc_ring.AllocSize));
			/* Allocate all 1st TXBuf's memory for this TxRing */
			hif->TxRing[num].buf_space.AllocSize = tx_ring_size * TX_DMA_1ST_BUFFER_SIZE;

			RTMP_AllocateFirstTxBuffer(
				pdev,
				num,
				hif->TxRing[num].buf_space.AllocSize,
				FALSE,
				&hif->TxRing[num].buf_space.AllocVa,
				&hif->TxRing[num].buf_space.AllocPa);

			if (hif->TxRing[num].buf_space.AllocVa == NULL) {
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			os_alloc_mem(pAd, (UCHAR **)&pTxRing->Cell, tx_ring_size * sizeof(struct _RTMP_DMACB));
			NdisZeroMemory(pTxRing->Cell, tx_ring_size * sizeof(struct _RTMP_DMACB));
		}

		if (Status == NDIS_STATUS_RESOURCES)
			break;

		/* Alloc CTRL ring desc buffer except Tx ring allocated eariler */
		desc_ring_alloc(pAd, &hif->ctrl_ring.desc_ring, CTL_RING_SIZE * TXD_SIZE);

		if (hif->ctrl_ring.desc_ring.AllocVa == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		os_alloc_mem(pAd, (UCHAR **)&hif->ctrl_ring.Cell, CTL_RING_SIZE * sizeof(struct _RTMP_DMACB));
		if (hif->ctrl_ring.Cell == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		NdisZeroMemory(hif->ctrl_ring.Cell, CTL_RING_SIZE * sizeof(struct _RTMP_DMACB));

		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE,
				 ("CTRL Ring: total %d bytes allocated\n",
				  (INT)hif->ctrl_ring.desc_ring.AllocSize));
			
		desc_ring_alloc(pAd, &hif->fwdl_ring.desc_ring, hif->fwdl_ring.ring_size * TXD_SIZE);

		if (hif->fwdl_ring.desc_ring.AllocVa == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		os_alloc_mem(pAd,
			     (UCHAR **)&hif->fwdl_ring.Cell,
			     hif->fwdl_ring.ring_size * sizeof(struct _RTMP_DMACB));
		if (hif->fwdl_ring.Cell == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		NdisZeroMemory(hif->fwdl_ring.Cell, hif->fwdl_ring.ring_size * sizeof(struct _RTMP_DMACB));

		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE,
				("FwDwlo Ring: total %d bytes allocated\n",
				(INT)hif->fwdl_ring.desc_ring.AllocSize));

		/* Alloc RX ring desc memory */
		for (num = 0; num < num_of_rx_ring; num++) {
			UINT16 RxRingSize;
			struct hif_pci_rx_ring *pRxRing = &hif->RxRing[num];

			if (num == HIF_RX_IDX0)
				RxRingSize = rx0_ring_size;
			else
				RxRingSize = rx1_ring_size;
#ifdef MT7626_E2_SUPPORT
			if (num == HIF_RX_IDX2)
				RxRingSize = rx2_ring_size;
			else if (num == HIF_RX_IDX3)
				RxRingSize = rx3_ring_size;
#endif
			desc_ring_alloc(pAd,
					&hif->RxRing[num].desc_ring,
					RxRingSize * RXD_SIZE);

			if (hif->RxRing[num].desc_ring.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Allocate RX Free Buf */
			if (num == HIF_RX_IDX1) {
				alloc_rx_free_buffer(pAd, num, RxRingSize);
			}
#ifdef MT7626_E2_SUPPORT
			if (num == HIF_RX_IDX3) {
				alloc_rx_free_buffer(pAd, num, RxRingSize);
			}
#endif
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("Rx[%d] Ring: total %d bytes allocated\n",
				num, (INT)hif->RxRing[num].desc_ring.AllocSize));


			os_alloc_mem(pAd, (UCHAR **)&pRxRing->Cell, RxRingSize * sizeof(struct _RTMP_DMACB));
			NdisZeroMemory(pRxRing->Cell, RxRingSize * sizeof(struct _RTMP_DMACB));
		}
	} while (FALSE);

	if (Status != NDIS_STATUS_SUCCESS) {
		/* Log error inforamtion*/
		NdisWriteErrorLogEntry(
			pAd->AdapterHandle,
			NDIS_ERROR_CODE_OUT_OF_RESOURCES,
			1,
			ErrorValue);
	}

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,
			 ("<-- RTMPAllocTxRxRingMemory, Status=%x\n", Status));
	return Status;
}
#else
/*
	========================================================================

	Routine Description:
		Allocate DMA memory blocks for send, receive

	Arguments:
		Adapter		Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE
		NDIS_STATUS_RESOURCES

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS RTMPAllocTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	ULONG RingBasePaHigh, RingBasePaLow;
	PVOID RingBaseVa;
	INT index, num;
	TXD_STRUC *pTxD;
	RXD_STRUC *pRxD;
	ULONG ErrorValue = 0;
	struct hif_pci_rx_ring *rx_ring;
	struct hif_pci_tx_ring *pTxRing;
	RTMP_DMABUF *pDmaBuf;
	RTMP_DMACB *dma_cb;
	PNDIS_PACKET pPacket = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	UINT8 num_of_rx_ring = GET_NUM_OF_RX_RING(cap);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	UINT16 rx0_ring_size = GET_RX0_RING_SIZE(cap);
	UINT16 rx1_ring_size = GET_RX1_RING_SIZE(cap);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("--> RTMPAllocTxRxRingMemory\n"));
	/* Init the CmdQ and CmdQLock*/
	NdisAllocateSpinLock(pAd, &pAd->CmdQLock);

	do {
		/*
			Allocate all ring descriptors, include TxD, RxD, MgmtD.
			Although each size is different, to prevent cacheline and alignment
			issue, I intentional set them all to 64 bytes
		*/
		for (num = 0; num < num_of_tx_ring; num++) {
			ULONG  BufBasePaHigh;
			ULONG  BufBasePaLow;
			PVOID  BufBaseVa;
			/*
				Allocate Tx ring descriptor's memory (5 TX rings = 4 ACs + 1 HCCA)
			*/
			desc_ring_alloc(pAd, &hif->TxRing[num].desc_ring, tx_ring_size * TXD_SIZE);

			if (hif->TxRing[num].desc_ring.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			pDmaBuf = &hif->TxRing[num].desc_ring;
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("TxDescRing[%p]: total %d bytes allocated\n",
					 pDmaBuf->AllocVa, (INT)pDmaBuf->AllocSize));
			/* Save PA & VA for further operation*/
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDmaBuf->AllocPa);
			RingBasePaLow = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
			RingBaseVa = pDmaBuf->AllocVa;
			/*
				Allocate all 1st TXBuf's memory for this TxRing
			*/
			hif->TxRing[num].buf_space.AllocSize = tx_ring_size * TX_DMA_1ST_BUFFER_SIZE;

			RTMP_AllocateFirstTxBuffer(
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				num,
				hif->TxRing[num].buf_space.AllocSize,
				FALSE,
				&hif->TxRing[num].buf_space.AllocVa,
				&hif->TxRing[num].buf_space.AllocPa);

			if (hif->TxRing[num].buf_space.AllocVa == NULL) {
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Zero init this memory block*/
			NdisZeroMemory(hif->TxRing[num].buf_space.AllocVa, hif->TxRing[num].buf_space.AllocSize);
			/* Save PA & VA for further operation*/
			BufBasePaHigh = RTMP_GetPhysicalAddressHigh(hif->TxRing[num].buf_space.AllocPa);
			BufBasePaLow = RTMP_GetPhysicalAddressLow(hif->TxRing[num].buf_space.AllocPa);
			BufBaseVa = hif->TxRing[num].buf_space.AllocVa;
			/*
				Initialize Tx Ring Descriptor and associated buffer memory
			*/
			pTxRing = &hif->TxRing[num];

			os_alloc_mem(pAd, (UCHAR **)&pTxRing->Cell, tx_ring_size * sizeof(struct _RTMP_DMACB));
			NdisZeroMemory(pTxRing->Cell, tx_ring_size * sizeof(struct _RTMP_DMACB));

			for (index = 0; index < tx_ring_size; index++) {
				dma_cb = &pTxRing->Cell[index];
				dma_cb->pNdisPacket = NULL;
				dma_cb->pNextNdisPacket = NULL;
				/* Init Tx Ring Size, Va, Pa variables */
				dma_cb->AllocSize = TXD_SIZE;
				dma_cb->AllocVa = RingBaseVa;
				RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
				RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);
				/* Setup Tx Buffer size & address. only 802.11 header will store in this space*/
				pDmaBuf = &dma_cb->DmaBuf;
				pDmaBuf->AllocSize = TX_DMA_1ST_BUFFER_SIZE;
				pDmaBuf->AllocVa = BufBaseVa;
				RTMP_SetPhysicalAddressHigh(pDmaBuf->AllocPa, BufBasePaHigh);
				RTMP_SetPhysicalAddressLow(pDmaBuf->AllocPa, BufBasePaLow);
				/* link the pre-allocated TxBuf to TXD */
				pTxD = (TXD_STRUC *)dma_cb->AllocVa;
				pTxD->SDPtr0 = BufBasePaLow;
				pTxD->DMADONE = 0;
#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);
				RingBasePaLow += TXD_SIZE;
				RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;
				/* advance to next TxBuf address */
				BufBasePaLow += TX_DMA_1ST_BUFFER_SIZE;
				BufBaseVa = (PUCHAR) BufBaseVa + TX_DMA_1ST_BUFFER_SIZE;
			}

			pTxRing->tx_ring_state = TX_RING_HIGH;
			pTxRing->tx_ring_low_water_mark = 5;
			pTxRing->tx_ring_high_water_mark = pTxRing->tx_ring_low_water_mark + 10;
			pTxRing->tx_ring_full_cnt = 0;

			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("TxRing[%d]: total %d entry allocated\n", num, index));
		}

		if (Status == NDIS_STATUS_RESOURCES)
			break;

		/*
			Allocate CTRL ring descriptor's memory except Tx ring which allocated eariler
		*/
		desc_ring_alloc(pAd, &hif->ctrl_ring.desc_ring, CTL_RING_SIZE * TXD_SIZE);

		if (hif->ctrl_ring.desc_ring.AllocVa == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("CtrlDescRing[%p]: total %d bytes allocated\n",
				 hif->ctrl_ring.desc_ring.AllocVa, (INT)hif->ctrl_ring.desc_ring.AllocSize));
		/* Save PA & VA for further operation*/
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(hif->ctrl_ring.desc_ring.AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow(hif->ctrl_ring.desc_ring.AllocPa);
		RingBaseVa = hif->ctrl_ring.desc_ring.AllocVa;

		/*
			Initialize CTRL Ring and associated buffer memory
		*/
		for (index = 0; index < CTL_RING_SIZE; index++) {
			dma_cb = &hif->ctrl_ring.Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init CTRL Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);
			/* Offset to next ring descriptor address*/
			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;
			/* link the pre-allocated TxBuf to TXD*/
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
			pTxD->DMADONE = 1;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);
			/* no pre-allocated buffer required in CtrlRing for scatter-gather case*/
		}

		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("CTRL Ring: total %d entry allocated\n", index));

		/*
		 * Allocate firmware download ring descriptor's memory except Tx ring which allocated eariler
		 */
		desc_ring_alloc(pAd, &hif->fwdl_ring.desc_ring, hif->fwdl_ring.ring_size * TXD_SIZE);

		if (hif->fwdl_ring.desc_ring.AllocVa == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("FwDwloRing.DescRing[%p]: total %d bytes allocated\n",
				 hif->fwdl_ring.desc_ring.AllocVa, (INT)hif->fwdl_ring.desc_ring.AllocSize));

		/* Save PA & VA for further operation*/
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(hif->fwdl_ring.desc_ring.AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow(hif->fwdl_ring.desc_ring.AllocPa);
		RingBaseVa = hif->fwdl_ring.desc_ring.AllocVa;

		/*
		 * Initialize firmware download ring and associated buffer memory
		 */
		for (index = 0; index < CTL_RING_SIZE; index++) {
			dma_cb = &hif->fwdl_ring.Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init firmware download Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);
			/* Offset to next ring descriptor address*/
			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;
			/* link the pre-allocated TxBuf to TXD*/
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
			pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);
			/* no pre-allocated buffer required in CtrlRing for scatter-gather case*/
		}

		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("FwDwlo Ring: total %d entry allocated\n", index));

		for (num = 0; num < num_of_rx_ring; num++) {
			UINT16 RxRingSize;
			UINT16 RxBufferSize;
			rx_ring = &hif->RxRing[num];

			if (num == 0) {
				RxRingSize = rx0_ring_size;
				RxBufferSize = RX_BUFFER_AGGRESIZE;
			} else {
				RxRingSize = rx1_ring_size;
				RxBufferSize = RX1_BUFFER_SIZE;
			}

			/* Alloc RxRingDesc memory except Tx ring allocated eariler */
			desc_ring_alloc(pAd, &hif->RxRing[num].desc_ring, RxRingSize * RXD_SIZE);

			if (hif->RxRing[num].desc_ring.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,
				("desc_ring[%p]: total %d bytes allocated\n",
				hif->RxRing[num].desc_ring.AllocVa, (INT)hif->RxRing[num].desc_ring.AllocSize));
			/* Initialize Rx Ring and associated buffer memory */
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(hif->RxRing[num].desc_ring.AllocPa);
			RingBasePaLow = RTMP_GetPhysicalAddressLow(hif->RxRing[num].desc_ring.AllocPa);
			RingBaseVa = hif->RxRing[num].desc_ring.AllocVa;

			/* Allocate RX Free Buf */
			if (num == HIF_RX_IDX1) {
				alloc_rx_free_buffer(pAd, num, RxRingSize);
			}

			os_alloc_mem(pAd, (UCHAR **)&rx_ring->Cell, RxRingSize * sizeof(struct _RTMP_DMACB));
			NdisZeroMemory(rx_ring->Cell, RxRingSize * sizeof(struct _RTMP_DMACB));

			for (index = 0; index < RxRingSize; index++) {
				dma_cb = &rx_ring->Cell[index];
				/* Init RX Ring Size, Va, Pa variables*/
				dma_cb->AllocSize = RXD_SIZE;
				dma_cb->AllocVa = RingBaseVa;
				RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
				RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);
				/* Offset to next ring descriptor address */
				RingBasePaLow += RXD_SIZE;
				RingBaseVa = (PUCHAR) RingBaseVa + RXD_SIZE;
				/* Setup Rx associated Buffer size & allocate share memory*/
				pDmaBuf = &dma_cb->DmaBuf;
				pDmaBuf->AllocSize = RxBufferSize;

				if (num == HIF_RX_IDX0) {
					pPacket = RTMP_AllocateRxPacketBuffer(
						rx_ring,
						((POS_COOKIE)(pAd->OS_Cookie))->pDev,
						DYNAMIC_PAGE_ALLOC,
						pDmaBuf->AllocSize,
						&pDmaBuf->AllocVa,
						&pDmaBuf->AllocPa);
				} else if (num == HIF_RX_IDX1) {
					pPacket = RTMP_AllocateRxPacketBuffer(
						rx_ring,
						((POS_COOKIE)(pAd->OS_Cookie))->pDev,
						PRE_SLAB_ALLOC,
						pDmaBuf->AllocSize,
						&pDmaBuf->AllocVa,
						&pDmaBuf->AllocPa);
				}

				/* keep allocated rx packet */
				dma_cb->pNdisPacket = pPacket;

				if (pDmaBuf->AllocVa == NULL) {
					ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
					MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Failed to allocate RxRing's 1st buffer\n"));
					Status = NDIS_STATUS_RESOURCES;
					break;
				}

				/* Zero init this memory block*/
				NdisZeroMemory(pDmaBuf->AllocVa, pDmaBuf->AllocSize);

				/* Write RxD buffer address & allocated buffer length*/
				pRxD = (RXD_STRUC *)dma_cb->AllocVa;
				pRxD->SDP0 = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
				pRxD->SDL0 = RxBufferSize;
				pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#endif
				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pRxD, dma_cb->AllocSize);
			}

			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("Rx[%d] Ring: total %d entry allocated\n", num, index));
		}
	}	while (FALSE);

	NdisZeroMemory(&pAd->FragFrame, sizeof(FRAGMENT_FRAME));
	pAd->FragFrame.pFragPacket = RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);

	if (pAd->FragFrame.pFragPacket == NULL)
		Status = NDIS_STATUS_RESOURCES;

	if (Status != NDIS_STATUS_SUCCESS) {
		/* Log error inforamtion*/
		NdisWriteErrorLogEntry(
			pAd->AdapterHandle,
			NDIS_ERROR_CODE_OUT_OF_RESOURCES,
			1,
			ErrorValue);
	}

	/*
		Initialize all transmit related software queues
	*/

	for (index = 0; index < num_of_tx_ring; index++) {
		hif->TxRing[index].TxSwFreeIdx = 0;
		hif->TxRing[index].TxCpuIdx = 0;
	}

#ifdef CONFIG_ANDES_SUPPORT
	/* init CTRL ring index pointer*/
	hif->ctrl_ring.TxSwFreeIdx = 0;
	hif->ctrl_ring.TxCpuIdx = 0;
#endif /* CONFIG_ANDES_SUPPORT */

	/* Init RX Ring index pointer*/
	for (index = 0; index < num_of_rx_ring; index++) {
		UINT16 RxRingSize;
		UINT16 RxBufferSize;
		UINT16 max_rx_process_cnt;

		if (index == HIF_RX_IDX0) {
			RxRingSize = rx0_ring_size;
			RxBufferSize = RX_BUFFER_AGGRESIZE;
			max_rx_process_cnt = MAX_RX_PROCESS_CNT;
		} else {
			RxRingSize = rx1_ring_size;
			RxBufferSize = RX1_BUFFER_SIZE;
			max_rx_process_cnt = MAX_RX1_PROCESS_CNT;
		}

		hif->RxRing[index].RxSwReadIdx = 0;
		hif->RxRing[index].RxCpuIdx = RxRingSize - 1;
		hif->RxRing[index].ring_size = RxRingSize;
		hif->RxRing[index].RxBufferSize = RxBufferSize;
		hif->RxRing[index].max_rx_process_cnt = max_rx_process_cnt;
	}

	pAd->PrivateInfo.TxRingFullCnt = 0;
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("<-- RTMPAllocTxRxRingMemory, Status=%x\n", Status));
	return Status;
}


VOID RTMPFreeTxRxRingMemory(struct _RTMP_ADAPTER *pAd)
{
	int index, num, j;
	struct hif_pci_tx_ring *pTxRing;
	TXD_STRUC *pTxD;
	RTMP_DMACB *dma_cb;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif /* RT_BIG_ENDIAN */
	PNDIS_PACKET pPacket;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	UINT8 num_of_rx_ring = GET_NUM_OF_RX_RING(cap);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	UINT16 rx0_ring_size = GET_RX0_RING_SIZE(cap);
	UINT16 rx1_ring_size = GET_RX1_RING_SIZE(cap);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("--> RTMPFreeTxRxRingMemory\n"));

	/* Free Tx Ring Packet*/
	for (index = 0; index < num_of_tx_ring; index++) {
		pTxRing = &hif->TxRing[index];

		for (j = 0; j < tx_ring_size; j++) {
			dma_cb = &pTxRing->Cell[j];
#ifdef RT_BIG_ENDIAN
			pDestTxD  = (TXD_STRUC *)(dma_cb->AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *)(dma_cb->AllocVa);
#endif /* RT_BIG_ENDIAN */
			
			
			if (dma_cb->DmaBuf.AllocSize > 0)
				PCI_UNMAP_SINGLE(pAd, dma_cb->DmaBuf.AllocPa, dma_cb->DmaBuf.AllocSize, RTMP_PCI_DMA_TODEVICE);
			
			pPacket = dma_cb->pNdisPacket;

			if (pPacket) {
#ifdef CUT_THROUGH
				UINT8 Type;

				if (CUT_THROUGH_TX_ENABL(pAd->PktTokenCb))
					cut_through_tx_deq(pAd->PktTokenCb, dma_cb->token_id, &Type);

#endif /* CUT_THROUGH */
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/*Always assign pNdisPacket as NULL after clear*/
			dma_cb->pNdisPacket = NULL;
			pPacket = dma_cb->pNextNdisPacket;

			if (pPacket) {
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/*Always assign pNextNdisPacket as NULL after clear*/
			dma_cb->pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}
		os_free_mem(pTxRing->Cell);
	}

	{
		struct hif_pci_tx_ring *pCtrlRing = &hif->ctrl_ring;

		NdisAcquireSpinLock(&hif->ctrl_ring.ring_lock);
		HIF_IO_READ32(pAd->hdev_ctrl, pCtrlRing->hw_didx_addr, &pCtrlRing->TxDmaIdx);

		while (pCtrlRing->TxSwFreeIdx != pCtrlRing->TxDmaIdx) {
#ifdef RT_BIG_ENDIAN
			pDestTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
#endif
			pTxD->DMADONE = 0;
			pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket;

			if (pPacket == NULL) {
				INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, CTL_RING_SIZE);
				continue;
			}

			if (pPacket) {
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket = NULL;
			pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNextNdisPacket;

			if (pPacket) {
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNextNdisPacket = NULL;
			INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, CTL_RING_SIZE);
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
		}

		os_free_mem(pCtrlRing->Cell);

		NdisReleaseSpinLock(&hif->ctrl_ring.ring_lock);
	}

	for (j = 0; j < num_of_rx_ring; j++) {
		UINT16 RxRingSize;
		UINT skb_data_size = 0;
		if (j == 0)
			RxRingSize = rx0_ring_size;
		else
			RxRingSize = rx1_ring_size;

		for (index = RxRingSize - 1; index >= 0; index--) {
			dma_cb = &hif->RxRing[j].Cell[index];

			if ((dma_cb->DmaBuf.AllocVa) && (dma_cb->pNdisPacket)) {
				PCI_UNMAP_SINGLE(pAd, dma_cb->DmaBuf.AllocPa,
								 dma_cb->DmaBuf.AllocSize,
								 RTMP_PCI_DMA_FROMDEVICE);

				if (j == HIF_RX_IDX0) {
#ifdef CONFIG_WIFI_BUILD_SKB
					skb_data_size = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + dma_cb->DmaBuf.AllocSize) +
							SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV);

					if (skb_data_size <= PAGE_SIZE) {
						DEV_FREE_FRAG_BUF(dma_cb->pNdisPacket);
					} else {
						os_free_mem(dma_cb->pNdisPacket);
					}
#else  /* CONFIG_WIFI_BUILD_SKB */
					RELEASE_NDIS_PACKET(pAd, dma_cb->pNdisPacket,
							NDIS_STATUS_SUCCESS);
#endif /* CONFIG_WIFI_BUILD_SKB */
				}
			}
		}

		if (j == HIF_RX_IDX1)
			release_rx_free_buffer(pAd->hdev_ctrl, j);

		os_free_mem(hif->RxRing[j].Cell);

		if (hif->RxRing[j].desc_ring.AllocVa)
			RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pDev,
					hif->RxRing[j].desc_ring.AllocSize,
					hif->RxRing[j].desc_ring.AllocVa,
					hif->RxRing[j].desc_ring.AllocPa);

		NdisZeroMemory(&hif->RxRing[j].desc_ring, sizeof(RTMP_DMABUF));
	}

	if (hif->ctrl_ring.desc_ring.AllocVa) {
		RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pDev,
						hif->ctrl_ring.desc_ring.AllocSize,
						hif->ctrl_ring.desc_ring.AllocVa,
						hif->ctrl_ring.desc_ring.AllocPa);
	}

	NdisZeroMemory(&hif->ctrl_ring.desc_ring, sizeof(RTMP_DMABUF));

	if (hif->fwdl_ring.desc_ring.AllocVa) {
		RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pDev, hif->fwdl_ring.desc_ring.AllocSize,
						hif->fwdl_ring.desc_ring.AllocVa, hif->fwdl_ring.desc_ring.AllocPa);
	}

	NdisZeroMemory(&hif->fwdl_ring.desc_ring, sizeof(RTMP_DMABUF));

	for (num = 0; num < num_of_tx_ring; num++) {
		if (hif->TxRing[num].buf_space.AllocVa)
			RTMP_FreeFirstTxBuffer(((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				hif->TxRing[num].buf_space.AllocSize, FALSE,
				hif->TxRing[num].buf_space.AllocVa, hif->TxRing[num].buf_space.AllocPa);

		NdisZeroMemory(&hif->TxRing[num].buf_space, sizeof(RTMP_DMABUF));

		if (hif->TxRing[num].desc_ring.AllocVa)
			RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				hif->TxRing[num].desc_ring.AllocSize,
				hif->TxRing[num].desc_ring.AllocVa, hif->TxRing[num].desc_ring.AllocPa);

		NdisZeroMemory(&hif->TxRing[num].desc_ring, sizeof(RTMP_DMABUF));
	}

	if (pAd->FragFrame.pFragPacket) {
		RELEASE_NDIS_PACKET(pAd, pAd->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);
		pAd->FragFrame.pFragPacket = NULL;
	}

	NdisFreeSpinLock(&pAd->CmdQLock);
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<-- RTMPFreeTxRxRingMemory\n"));
}

#endif /* RESOURCE_PRE_ALLOC */


VOID AsicInitTxRxRing(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		mt_asic_init_txrx_ring(pAd);

#endif /* MT_MAC */
}


/*
	========================================================================

	Routine Description:
		Reset NIC Asics. Call after rest DMA. So reset TX_CTX_IDX to zero.

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:
		Reset NIC to initial state AS IS system boot up time.

	========================================================================
*/
VOID RTMPRingCleanUp(RTMP_ADAPTER *pAd, UCHAR RingType)
{
	TXD_STRUC *pTxD;
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD, TxD;
	RXD_STRUC *pDestRxD, RxD;
#endif /* RT_BIG_ENDIAN */
	PNDIS_PACKET pPacket;
	struct hif_pci_tx_ring *pTxRing;
	ULONG IrqFlags;
	int i, ring_id;
	NDIS_SPIN_LOCK *lock;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	UINT8 num_of_rx_ring = GET_NUM_OF_RX_RING(cap);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	UINT16 rx0_ring_size = GET_RX0_RING_SIZE(cap);
	UINT16 rx1_ring_size = GET_RX1_RING_SIZE(cap);
#ifdef MT7626_E2_SUPPORT
	UINT16 rx2_ring_size = GET_RX2_RING_SIZE(cap);
	UINT16 rx3_ring_size = GET_RX3_RING_SIZE(cap);
#endif
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	/*
		We have to clean all descriptors in case some error happened with reset
	*/
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("RTMPRingCleanUp(RingIdx=%d, Pending-NDIS=%ld)\n", RingType, pAd->RalinkCounters.PendingNdisPacketCount));

	switch (RingType) {
	case QID_AC_BK:
	case QID_AC_BE:
	case QID_AC_VI:
	case QID_AC_VO:
	case QID_HCCA:
		if (num_of_tx_ring <= RingType)
			break;

		pTxRing = &hif->TxRing[RingType];
		lock = &hif->TxRing[RingType].ring_lock;
		RTMP_IRQ_LOCK(lock, IrqFlags);

		for (i = 0; i < tx_ring_size; i++) { /* We have to scan all TX ring*/
			pTxD  = (TXD_STRUC *)pTxRing->Cell[i].AllocVa;
			pPacket = (PNDIS_PACKET) pTxRing->Cell[i].pNdisPacket;

			/* release scatter-and-gather NDIS_PACKET*/
			if (pPacket) {
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				pTxRing->Cell[i].pNdisPacket = NULL;
			}

			pPacket = (PNDIS_PACKET) pTxRing->Cell[i].pNextNdisPacket;

			/* release scatter-and-gather NDIS_PACKET*/
			if (pPacket) {
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				pTxRing->Cell[i].pNextNdisPacket = NULL;
			}
		}

		HIF_IO_READ32(pAd->hdev_ctrl, pTxRing->hw_didx_addr, &pTxRing->TxDmaIdx);
		pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
		pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
		HIF_IO_WRITE32(pAd->hdev_ctrl, pTxRing->hw_cidx_addr, pTxRing->TxCpuIdx);
		RTMP_IRQ_UNLOCK(lock, IrqFlags);
		/* TODO: need to check */
		/* rtmp_tx_swq_exit(pAd, WCID_ALL); */
		break;

	case QID_RX:
		for (ring_id = 0; ring_id < num_of_rx_ring; ring_id++) {
			struct hif_pci_rx_ring *pRxRing;
			UINT16 RxRingSize;
			UINT16 RxBufferSize;

			pRxRing = &hif->RxRing[ring_id];
			lock = &hif->RxRing[ring_id].ring_lock;

			if (ring_id == 0) {
				RxRingSize = rx0_ring_size;
				RxBufferSize = RX_BUFFER_AGGRESIZE;
			} else {
				RxRingSize = rx1_ring_size;
				RxBufferSize = RX1_BUFFER_SIZE;
			}
#ifdef MT7626_E2_SUPPORT
			if (ring_id == 2) {
				RxRingSize = rx2_ring_size;
				RxBufferSize = RX_BUFFER_AGGRESIZE;
			} else if (ring_id == 3) {
				RxRingSize = rx3_ring_size;
				RxBufferSize = RX1_BUFFER_SIZE;
			}
#endif
			RTMP_IRQ_LOCK(lock, IrqFlags);

			for (i = 0; i < RxRingSize; i++) {
#ifdef RT_BIG_ENDIAN
				pDestRxD = (RXD_STRUC *)pRxRing->Cell[i].AllocVa;
				RxD = *pDestRxD;
				pRxD = &RxD;
				RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
				/* Point to Rx indexed rx ring descriptor*/
				pRxD  = (RXD_STRUC *)pRxRing->Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */
				pRxD->DDONE = 0;
				pRxD->SDL0 = RxBufferSize;
#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
				WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif /* RT_BIG_ENDIAN */
			}

			HIF_IO_READ32(pAd->hdev_ctrl, pRxRing->hw_didx_addr, &pRxRing->RxDmaIdx);
			pRxRing->RxSwReadIdx = pRxRing->RxDmaIdx;
			pRxRing->RxCpuIdx = ((pRxRing->RxDmaIdx == 0) ? (RxRingSize - 1) : (pRxRing->RxDmaIdx - 1));
			HIF_IO_WRITE32(pAd->hdev_ctrl, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);
			RTMP_IRQ_UNLOCK(lock, IrqFlags);
		}

		break;
#ifdef CONFIG_ANDES_SUPPORT

	case QID_CTRL:
		RTMP_IRQ_LOCK(&hif->ctrl_ring.ring_lock, IrqFlags);

		for (i = 0; i < CTL_RING_SIZE; i++) {
#ifdef RT_BIG_ENDIAN
			pDestTxD  = (TXD_STRUC *) hif->ctrl_ring.Cell[i].AllocVa;
			TxD = *pDestTxD;
			pTxD = &TxD;
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD  = (TXD_STRUC *) hif->ctrl_ring.Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */
			pPacket = (PNDIS_PACKET) hif->ctrl_ring.Cell[i].pNdisPacket;

			/* rlease scatter-and-gather NDIS_PACKET*/
			if (pPacket) {
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			}

			hif->ctrl_ring.Cell[i].pNdisPacket = NULL;
			pPacket = (PNDIS_PACKET) hif->ctrl_ring.Cell[i].pNextNdisPacket;

			/* release scatter-and-gather NDIS_PACKET*/
			if (pPacket) {
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			}

			hif->ctrl_ring.Cell[i].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}

		HIF_IO_READ32(pAd->hdev_ctrl, hif->ctrl_ring.hw_didx_addr, &hif->ctrl_ring.TxDmaIdx);
		hif->ctrl_ring.TxSwFreeIdx = hif->ctrl_ring.TxDmaIdx;
		hif->ctrl_ring.TxCpuIdx = hif->ctrl_ring.TxDmaIdx;
		HIF_IO_WRITE32(pAd->hdev_ctrl, hif->ctrl_ring.hw_cidx_addr, hif->ctrl_ring.TxCpuIdx);
		RTMP_IRQ_UNLOCK(&hif->ctrl_ring.ring_lock, IrqFlags);
		break;
#endif /* CONFIG_ANDES_SUPPORT */

	default:
		break;
	}
}


VOID DumpPseInfo(RTMP_ADAPTER *pAd)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 Value;
	UINT32 RestoreValue;
	UINT32 Index;

	RTMP_IO_READ32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, &RestoreValue);

	/* PSE Infomation */
	for (Index = 0; Index < 30720; Index++) {
		RemapBase = GET_REMAP_2_BASE(0xa5000000 + Index * 4) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa5000000 + Index * 4);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x80000 + RemapOffset, &Value);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("Offset[0x%x] = 0x%x\n", 0xa5000000 + Index * 4, Value));
	}

	/* Frame linker */
	for (Index  = 0; Index < 1280; Index++) {
		RemapBase = GET_REMAP_2_BASE(0xa00001b0) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa00001b0);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x80000 + RemapOffset, &Value);
		Value &= ~0xfff;
		Value |= (Index & 0xfff);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x80000 + RemapOffset, Value);
		RemapBase = GET_REMAP_2_BASE(0xa00001b4) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa00001b4);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x80000 + RemapOffset, &Value);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("Frame Linker(0x%x) = 0x%x\n", Index, Value));
	}

	/* Page linker */
	for (Index = 0; Index < 1280; Index++) {
		RemapBase = GET_REMAP_2_BASE(0xa00001b8) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa00001b8);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x80000 + RemapOffset, &Value);
		Value &= ~(0xfff << 16);
		Value |= ((Index & 0xfff) << 16);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x80000 + RemapOffset, Value);
		RemapBase = GET_REMAP_2_BASE(0xa00001b8) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa00001b8);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x80000 + RemapOffset, &Value);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("Page Linker(0x%x) = 0x%x\n", Index, (Value & 0xfff)));
	}

	RTMP_IO_WRITE32(pAd->hdev_ctrl, MCU_PCIE_REMAP_2, RestoreValue);
}



/***************************************************************************
  *
  *	register related procedures.
  *
  **************************************************************************/
/*
========================================================================
Routine Description:
    Disable DMA.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT28XXDMADisable(RTMP_ADAPTER *pAd)
{
	AsicSetWPDMA(pAd, PDMA_TX_RX, FALSE);
}


/*
========================================================================
Routine Description:
    Enable DMA.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT28XXDMAEnable(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	AsicWaitPDMAIdle(pAd, 200, 1000);
	RtmpusecDelay(50);
	AsicSetWPDMA(pAd, PDMA_TX_RX, TRUE);
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<== %s(): WPDMABurstSIZE = %d\n", __func__, cap->WPDMABurstSIZE));
}

VOID RT28XXDMAReset(RTMP_ADAPTER *pAd)
{
	/* disable WPDMA Tx/Rx */
	AsicSetWPDMA(pAd, PDMA_TX_RX, FALSE);

	/* wait WPDMA idle then reset*/
	if (AsicWaitPDMAIdle(pAd, 200, 1000)) {
		AsicResetWPDMA(pAd);
	} else {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<== %s(): WPDMA reset fail! \n", __func__));
	}
}

#ifdef MT_MAC
VOID MtUpdateBeaconToAsic(
	IN RTMP_ADAPTER     *pAd,
	IN VOID             *wdev_void,
	IN UINT16           FrameLen,
	IN UCHAR            UpdatePktType)
{
	struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;
	BCN_BUF_STRUC *bcn_buf = NULL;
	UCHAR *buf;
	INT len;
	PNDIS_PACKET pkt = NULL;
	UINT32 WdevIdx = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || defined(AXE) || defined(MT7626)
	PNDIS_PACKET bcn_pkt = NULL;
	UCHAR qIdx = 0;
	NDIS_STATUS Status;
#endif
	ASSERT(wdev != NULL);

	if (!wdev) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s(): wdev is NULL!\n", __func__));
		return;
	}

	bcn_buf = &wdev->bcn_buf;
	WdevIdx = wdev->wdev_idx;

	if (!bcn_buf) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s(): bcn_buf is NULL!\n", __func__));
		return;
	}

	if (bcn_buf->BeaconPkt) {
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || defined(AXE) || defined(MT7626)

		if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_P18(pAd) || IS_MT7663(pAd) || IS_AXE(pAd) || IS_MT7626(pAd)) {
			Status = RTMPAllocateNdisPacket(pAd, &bcn_pkt, NULL, 0,
											GET_OS_PKT_DATAPTR(bcn_buf->BeaconPkt),
											(UINT)(FrameLen + cap->tx_hw_hdr_len));

			if (Status != NDIS_STATUS_SUCCESS) {
				printk("%s():Cannot alloc bcn pkt buf!\n", __func__);
				return;
			}

			pkt = bcn_pkt;
		} else
#endif
		{
			pkt = bcn_buf->BeaconPkt;
		}

		buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
		len = FrameLen + cap->tx_hw_hdr_len;
		SET_OS_PKT_LEN(pkt, len);
#ifdef RT_BIG_ENDIAN
		MTMacInfoEndianChange(pAd, buf, TYPE_TMACINFO, sizeof(TMAC_TXD_L));
#endif /* RT_BIG_ENDIAN */
		RTMP_SET_PACKET_WDEV(pkt, WdevIdx);
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || defined(AXE) || defined(MT7626)

		if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_P18(pAd) || IS_MT7663(pAd) || IS_AXE(pAd) || IS_MT7626(pAd)) {
			qIdx = HcGetBcnQueueIdx(pAd, wdev);
			RTMP_SET_PACKET_TYPE(pkt, TX_MGMT);
			send_mlme_pkt(pAd, pkt, wdev, qIdx, FALSE);
		}

#else
		{
			send_mlme_pkt(pAd, pkt, wdev, Q_IDX_BCN, FALSE);
			bcn_buf->bcn_state = BCN_TX_WRITE_TO_DMA;
		}
#endif
	} else
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s(): BeaconPkt is NULL!\n", __func__));
}
#endif /* MT_MAC */

#ifdef CONFIG_STA_SUPPORT
VOID RT28xxPciStaAsicWakeup(RTMP_ADAPTER *pAd, BOOLEAN bFromTx, PSTA_ADMIN_CONFIG pStaCfg)
{
#if defined(STA_LP_PHASE_1_SUPPORT) || defined(STA_LP_PHASE_2_SUPPORT)
	ASSERT(pStaCfg);

	if (pStaCfg)
		RTMPOffloadPm(pAd, pStaCfg, PM4, EXIT_PM_STATE);

#else

	if (!pStaCfg->PwrMgmt.bDoze)
		return;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WAKEUP_NOW)) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("waking up now!\n"));
		return;
	}

	OPSTATUS_SET_FLAG(pAd, fOP_STATUS_WAKEUP_NOW);

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				 __func__, __LINE__));
		return;
	}

	pStaCfg->PwrMgmt.bDoze = FALSE;
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WAKEUP_NOW);
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<=======RT28xxPciStaAsicWakeup\n"));
#endif /* STA_LP_PHASE_1_SUPPORT || STA_LP_PHASE_2_SUPPORT */
}


VOID RT28xxPciStaAsicSleepAutoWakeup(
	RTMP_ADAPTER *pAd,
	PSTA_ADMIN_CONFIG pStaCfg)
{
#if defined(STA_LP_PHASE_1_SUPPORT) || defined(STA_LP_PHASE_2_SUPPORT)
	ASSERT(pStaCfg);

	if (pStaCfg)
		RTMPOffloadPm(pAd, pStaCfg, PM4, ENTER_PM_STATE);

#else

	if (pAd->StaCfg[0].bRadio == FALSE) {
		pStaCfg->PwrMgmt.bDoze = FALSE;
		return;
	}

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				 __func__, __LINE__));
		return;
	}

#endif /* STA_LP_PHASE_1_SUPPORT || STA_LP_PHASE_2_SUPPORT */
}


#endif /* CONFIG_STA_SUPPORT */


/*
	==========================================================================
	Description:
		This routine sends command to firmware and turn our chip to wake up mode from power save mode.
		Both RadioOn and .11 power save function needs to call this routine.
	Input:
		Level = GUIRADIO_OFF : call this function is from Radio Off to Radio On.  Need to restore PCI host value.
		Level = other value : normal wake up function.

	==========================================================================
 */
BOOLEAN RT28xxPciAsicRadioOn(RTMP_ADAPTER *pAd, UCHAR Level)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	if (pAd->OpMode == OPMODE_AP && Level == DOT11POWERSAVE)
		return FALSE;

	/* 2. Send wake up command.*/
	AsicSendCommandToMcu(pAd, 0x31, PowerWakeCID, 0x00, 0x02, FALSE);
	pci_hif->bPCIclkOff = FALSE;
	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

	if (Level == GUI_IDLE_POWER_SAVE) {
		/*2009/06/09: AP and stations need call the following function*/
		/* this if defined is equivalent to ifndef RTMP_RBUS_SUPPORT */
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT) || defined(RTMP_SDIO_SUPPORT)
		/* add by johnli, RF power sequence setup, load RF normal operation-mode setup*/
		RTMP_CHIP_OP *pChipOps = hc_get_chip_ops(pAd->hdev_ctrl);

		if (!IS_RBUS_INF(pAd) && pChipOps->AsicReverseRfFromSleepMode)
			pChipOps->AsicReverseRfFromSleepMode(pAd, FALSE);
		else
#endif /* defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT) || defined(RTMP_SDIO_SUPPORT) */
		{
			/* In Radio Off, we turn off RF clk, So now need to call ASICSwitchChannel again.*/
			hc_reset_radio(pAd);
		}
	}

	return TRUE;
}


/*
	==========================================================================
	Description:
		This routine sends command to firmware and turn our chip to power save mode.
		Both RadioOff and .11 power save function needs to call this routine.
	Input:
		Level = GUIRADIO_OFF  : GUI Radio Off mode
		Level = DOT11POWERSAVE  : 802.11 power save mode
		Level = RTMP_HALT  : When Disable device.

	==========================================================================
 */
BOOLEAN RT28xxPciAsicRadioOff(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Level,
	IN USHORT TbttNumToNextWakeUp)
{
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
				 __func__));
		return FALSE;
	}

	return TRUE;
}


/*
========================================================================
Routine Description:
	Get a pci map buffer.

Arguments:
	pAd				- WLAN control block pointer
	*ptr			- Virtual address or TX control block
	size			- buffer size
	sd_idx			- 1: the ptr is TX control block
	direction		- RTMP_PCI_DMA_TODEVICE or RTMP_PCI_DMA_FROMDEVICE

Return Value:
	the PCI map buffer

Note:
========================================================================
*/
ra_dma_addr_t RtmpDrvPciMapSingle(
	IN RTMP_ADAPTER *pAd,
	IN VOID *ptr,
	IN size_t size,
	IN INT sd_idx,
	IN INT direction)
{
	ra_dma_addr_t SrcBufPA = 0;
	UCHAR ret = 0;

	if (sd_idx == 1) {
		TX_BLK *pTxBlk = (TX_BLK *)(ptr);

		if (pTxBlk->SrcBufLen) {
			SrcBufPA = PCI_MAP_SINGLE_DEV(pAd, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen, 0, direction);
			pTxBlk->SrcBufPA = SrcBufPA;
		 } else {
			return (ra_dma_addr_t)0x0;
		}
	} else
		SrcBufPA = PCI_MAP_SINGLE_DEV(pAd, ptr, size, 0, direction);
	{
		struct device *pdev  = ((POS_COOKIE)(pAd->OS_Cookie))->pDev;
		ret = dma_mapping_error(pdev, SrcBufPA);
	}
	if (ret) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s: dma mapping error,ret=%d\n", __func__, ret));
		return (ra_dma_addr_t)0x0;
	} else
		return SrcBufPA;
}


int write_reg(RTMP_ADAPTER *ad, UINT32 base, UINT16 offset, UINT32 value)
{
	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(ad, HIF_MT)) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
				 __func__));
		return FALSE;
	}

	if (base == 0x40)
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x10000 + offset, value);
	else if (base == 0x41)
		RTMP_IO_WRITE32(ad->hdev_ctrl, offset, value);
	else
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("illegal base = %x\n", base));

	return 0;
}


int read_reg(RTMP_ADAPTER *ad, UINT32 base, UINT16 offset, UINT32 *value)
{
	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(ad, HIF_MT)) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
				 __func__));
		return FALSE;
	}

	if (base == 0x40)
		RTMP_IO_READ32(ad->hdev_ctrl, 0x10000 + offset, value);
	else if (base == 0x41)
		RTMP_IO_READ32(ad->hdev_ctrl, offset, value);
	else
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("illegal base = %x\n", base));

	return 0;
}


INT irq_init(RTMP_ADAPTER *pAd)
{
	unsigned long irqFlags;
	UINT_32 reg_mask;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	reg_mask = cap->int_enable_mask;

	if (!reg_mask) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): interrupt enable mask is not assigned\n", __func__));
		ASSERT(FALSE);
		return TRUE;
	}

	RTMP_INT_LOCK(&pAd->irq_lock, irqFlags);
	hif->IntEnableReg = reg_mask;
	hif->intDisableMask = 0;
	hif->IntPending = 0;
	RTMP_INT_UNLOCK(&pAd->irq_lock, irqFlags);

	/* init sub-layer interrupt enable mask, only for chips after P18 */
	if (ops->irq_init)
		ops->irq_init(pAd);

#ifdef MT7626_E2_SUPPORT
	if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD)) {
		reg_mask = cap->int_md_enable_mask;
		if (!reg_mask) {
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): interrupt2 enable mask is not assigned\n", __func__));
			ASSERT(FALSE);
			return TRUE;
		}
		RTMP_INT_LOCK(&pAd->irq_lock, irqFlags);
		hif->IntEnableReg_md = reg_mask;
		hif->intDisableMask_md = 0;
		hif->IntPending_md = 0;
		RTMP_INT_UNLOCK(&pAd->irq_lock, irqFlags);
	}
#endif
	return FALSE;
}

#ifdef CONFIG_FWOWN_SUPPORT
INT32 MtAcquirePowerControl(RTMP_ADAPTER *pAd, UINT32 Offset)
{
	ULONG Flags = 0;
	INT32 Ret = 0;
	UINT32 Counter = 0;

	if (Offset == HIF_FUN_CAP) {
		/* These registers are accessible when Low Power */
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Access HIF_FUN_CAP, return\n", __func__));
		return 1;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&pAd->DriverOwnLock, &Flags);
	pAd->bCRAccessing++;

	if (pAd->bDrvOwn) {
		/* all registers are accessible */
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Current is DrverOnw, return\n", __func__));
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);
		return 1;
	}

	RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);
	/* Write any value to HIF_SYS_REV clear FW own */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Wirte any value to HIF_SYS_REV to clear FW own\n", __func__));
	HIF_IO_WRITE32(pAd->hdev_ctrl, HIF_SYS_REV, 1);

	/* Poll driver own status */
	while (Counter < FW_OWN_POLLING_COUNTER) {
		RtmpusecDelay(100);

		if (pAd->bDrvOwn == TRUE)
			break;

		Counter++;
	};

	if (pAd->bDrvOwn) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Successed to clear FW own\n", __func__));
		Ret = 1;
	} else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Faile to clear FW own\n", __func__));
		Ret = 0;
		RTMP_SPIN_LOCK_IRQSAVE(&pAd->DriverOwnLock, &Flags);
		pAd->bCRAccessing--;	/* will not continue accessing HW */
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);
	}

	return Ret;
}


/* Drv give up own */
void MtReleasePowerControl(RTMP_ADAPTER *pAd, UINT32 Offset)
{
	ULONG Flags = 0;

	if (Offset == HIF_FUN_CAP) {
		/* These registers are accessible when Low Power */
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Access HIF_FUN_CAP, return\n", __func__));
		return;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&pAd->DriverOwnLock, &Flags);
	pAd->bCRAccessing--;
	RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);
	return;
}

VOID FwOwn(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops && ops->fw_own)
		ops->fw_own(pAd);
}

INT32 DriverOwn(RTMP_ADAPTER *pAd)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops && ops->driver_own)
		Ret = ops->driver_own(pAd);

	return Ret;
}

INT32 MakeFWOwn(RTMP_ADAPTER *pAd)
{
	ULONG Flags = 0;
	INT32 Ret = 0;
	UINT32 Value;
	UINT32 retry = 100;

	RTMP_SPIN_LOCK_IRQSAVE(&pAd->DriverOwnLock, &Flags);

	if (!pAd->bDrvOwn) {
		/* It is already FW own, do nothing */
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Current is FwOwn, return\n", __func__));
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);
		return 1;
	}

	if (pAd->bSetFWOwnRunning) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);
		return 1;
	}

	pAd->bSetFWOwnRunning = 1;

	/* Make sure no CR access and enter FwOwn with Sleep Notify */
	while (pAd->bCRAccessing & retry) {
		RtmpusecDelay(100);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wait CRAccessing %d\n", pAd->bCRAccessing));
		retry--;
	}

	if (retry == 0) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("**************WARNING******************\n"));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Can not set FW own! gbDrvOwn=%d\n", pAd->bDrvOwn));
		pAd->bSetFWOwnRunning = 0;
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);
		return 1;
	}

	/* Wirte any value to HIF_FUN_CAP to set FW own */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Wirte any value to HIF_FUN_CAP to set FW own\n", __func__));
	HIF_IO_WRITE32(pAd->hdev_ctrl, HIF_FUN_CAP, 1);
	pAd->bDrvOwn = FALSE;
	HIF_IO_READ32(pAd->hdev_ctrl, 0x4014, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Get 0x4014=0x%x\n", __func__, Value));
	pAd->bSetFWOwnRunning = 0;
	RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);
	return Ret;
}
#endif /* CONFIG_FWOWN_SUPPORT */


#endif /* RTMP_MAC_PCI */
