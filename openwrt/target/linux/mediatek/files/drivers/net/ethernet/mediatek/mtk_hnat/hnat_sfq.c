/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2021 MediaTek Inc.
 * Author: Henry Yen <henry.yen@mediatek.com>
 */

#include "hnat.h"
#include "../mtk_eth_soc.h"

int hnat_sfq_init(void)
{
	u32 reg_val, i, j;

	reg_val = readl(hnat_priv->fe_base + MTK_VQTX_GLO);
	reg_val = reg_val | VQTB_MIB_EN;
	writel(reg_val, hnat_priv->fe_base + MTK_VQTX_GLO);
	writel(HASH_PTB_MODE_EN_0 | HASH_PTB_MODE_EN_1 | HASH_PTB_MODE_EN_2 |
	       HASH_PTB_MODE_EN_3 | HASH_PTB_PRD,
	       hnat_priv->fe_base + MTK_VQTX_HASH_CFG);
	writel(VQTX_VLD_STRG_0 | VQTX_VLD_STRG_1 | VQTX_VLD_STRG_2 |
	       VQTX_VLD_STRG_3 | VQTX_VLD_STRG_4 | VQTX_VLD_STRG_5 |
	       VQTX_VLD_STRG_6 | VQTX_VLD_STRG_7,
	       hnat_priv->fe_base + MTK_VQTX_VLD_CFG);
	writel(HASH_SD, hnat_priv->fe_base + MTK_VQTX_HASH_SD);
	writel(RING_TH | FREE_TH | SHARE_HW_TH | SHARE_SW_TH |
	       HW_DROP_EN | HW_DROP_FFA | HW_DROP_FSTVQ | HW_DROP_MODE |
	       HW_DROP_FSTVQ_MODE | SW_DROP_EN | SW_DROP_FFA |
	       SW_DROP_FSTVQ | SW_DROP_MODE | SW_DROP_FSTVQ_MODE,
	       hnat_priv->fe_base + MTK_QDMA_FC_THRES);
	writel(0, hnat_priv->fe_base + MTK_QDMA_HRED1);
	writel(0, hnat_priv->fe_base + MTK_QDMA_HRED2);
	writel(0, hnat_priv->fe_base + MTK_QDMA_SRED1);
	writel(0, hnat_priv->fe_base + MTK_QDMA_SRED2);
	writel(VQTX_NUM_0 | VQTX_NUM_1 | VQTX_NUM_2 | VQTX_NUM_3 |
	       VQTX_NUM_4 | VQTX_NUM_5 | VQTX_NUM_6 | VQTX_NUM_7,
	       hnat_priv->fe_base + MTK_VQTX_NUM);
	writel(VQTX_0_BIND_QID(MTK_PQ0) | VQTX_1_BIND_QID(MTK_PQ1) |
	       VQTX_2_BIND_QID(MTK_PQ2) | VQTX_3_BIND_QID(MTK_PQ3),
	       hnat_priv->fe_base + MTK_VQTX_0_3_BIND_QID);
	writel(VQTX_4_BIND_QID(MTK_PQ4) | VQTX_5_BIND_QID(MTK_PQ5) |
	       VQTX_6_BIND_QID(MTK_PQ6) | VQTX_7_BIND_QID(MTK_PQ7),
	       hnat_priv->fe_base + MTK_VQTX_4_7_BIND_QID);

	for (i = 0; i < MTK_VQG_NUM; i++) {
		if (!hnat_priv->sfq_tbl_cpu[i]) {
			hnat_priv->sfq_tbl_cpu[i] =
				dma_alloc_coherent(hnat_priv->dev,
						   MTK_PER_GRP_VQ_NUM *
						   sizeof(struct mtk_sfq_table),
						   &hnat_priv->sfq_tbl_dev[i],
						   GFP_KERNEL);

			if (unlikely(!hnat_priv->sfq_tbl_cpu[i])) {
				dev_info(hnat_priv->dev,
					 "QDMA SFQ VQG:%d not available\n", i);
				return 1;
			}
		}

		memset(hnat_priv->sfq_tbl_cpu[i], 0x0,
		       MTK_PER_GRP_VQ_NUM * sizeof(struct mtk_sfq_table));

		for (j = 0; j < MTK_PER_GRP_VQ_NUM; j++) {
			hnat_priv->sfq_tbl_cpu[i][j].sfq_info1.vqhptr = 0xdeadbeef;
			hnat_priv->sfq_tbl_cpu[i][j].sfq_info2.vqtptr = 0xdeadbeef;
		}

		writel((u32)hnat_priv->sfq_tbl_dev[i],
		       hnat_priv->fe_base + MTK_VQTX_TB_BASE(i));
	}

	hnat_sfq_init_hook = hnat_sfq_init;

	return 0;
}

int hnat_sfq_deinit(void)
{
	u32 reg_val, i;

	reg_val = readl(hnat_priv->fe_base + MTK_VQTX_GLO);
	reg_val = reg_val & ~VQTB_MIB_EN;
	writel(reg_val, hnat_priv->fe_base + MTK_VQTX_GLO);
	writel(0, hnat_priv->fe_base + MTK_VQTX_HASH_CFG);
	writel(0, hnat_priv->fe_base + MTK_VQTX_VLD_CFG);
	writel(0, hnat_priv->fe_base + MTK_VQTX_HASH_SD);
	writel(FC_THRES_DROP_MODE | FC_THRES_DROP_EN | FC_THRES_MIN,
	       hnat_priv->fe_base + MTK_QDMA_FC_THRES);
	writel(0, hnat_priv->fe_base + MTK_QDMA_HRED1);
	writel(0, hnat_priv->fe_base + MTK_QDMA_HRED2);
	writel(0, hnat_priv->fe_base + MTK_QDMA_SRED1);
	writel(0, hnat_priv->fe_base + MTK_QDMA_SRED2);
	writel(0, hnat_priv->fe_base + MTK_VQTX_NUM);

	for (i = 0; i < MTK_VQG_NUM; i++) {
		if (hnat_priv->sfq_tbl_cpu[i])
			dma_free_coherent(hnat_priv->dev,
					  MTK_PER_GRP_VQ_NUM *
					  sizeof(struct mtk_sfq_table),
					  hnat_priv->sfq_tbl_cpu[i],
					  hnat_priv->sfq_tbl_dev[i]);

		writel(0, hnat_priv->fe_base + MTK_VQTX_TB_BASE(i));
	}

	hnat_sfq_init_hook = NULL;

	return 0;
}
