/*******************************************************************************
* Copyright (c) 2014 MediaTek Inc.
*
* All rights reserved. Copying, compilation, modification, distribution
* or any other use whatsoever of this material is strictly prohibited
* except in accordance with a Software License Agreement with
* MediaTek Inc.
********************************************************************************
*/

/*******************************************************************************
* LEGAL DISCLAIMER
*
* BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
* AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
* SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
* PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
* DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
* ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
* WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
* SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
* WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
* FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
* CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
* BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
* LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
* BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
* ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
* BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
* WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
* OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
* THEREOF AND RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN
* FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
* (ICC).
********************************************************************************
*/


#ifndef COMPOS_WIN

#include "rt_config.h"

#ifdef NEW_RATE_ADAPT_SUPPORT

INT	Set_PerThrdAdj_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	UCHAR i;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
		pAd->MacTab.Content[i].perThrdAdj = (BOOLEAN)simple_strtol(arg, 0, 10);

	return TRUE;
}


/* Set_LowTrafficThrd_Proc - set threshold for reverting to default MCS based on RSSI */
INT	Set_LowTrafficThrd_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	pAd->CommonCfg.lowTrafficThrd = (USHORT)simple_strtol(arg, 0, 10);
	return TRUE;
}


/* Set_TrainUpRule_Proc - set rule for Quick DRS train up */
INT	Set_TrainUpRule_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	pAd->CommonCfg.TrainUpRule = (BOOLEAN)simple_strtol(arg, 0, 10);
	return TRUE;
}


/* Set_TrainUpRuleRSSI_Proc - set RSSI threshold for Quick DRS Hybrid train up */
INT	Set_TrainUpRuleRSSI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	pAd->CommonCfg.TrainUpRuleRSSI = (SHORT)simple_strtol(arg, 0, 10);
	return TRUE;
}


/* Set_TrainUpLowThrd_Proc - set low threshold for Quick DRS Hybrid train up */
INT	Set_TrainUpLowThrd_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	pAd->CommonCfg.TrainUpLowThrd = (USHORT)simple_strtol(arg, 0, 10);
	return TRUE;
}


/* Set_TrainUpHighThrd_Proc - set high threshold for Quick DRS Hybrid train up */
INT	Set_TrainUpHighThrd_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	pAd->CommonCfg.TrainUpHighThrd = (USHORT)simple_strtol(arg, 0, 10);
	return TRUE;
}
#endif /* NEW_RATE_ADAPT_SUPPORT */


INT	Set_RateAlg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ra_alg;

	ra_alg = simple_strtol(arg, 0, 10);

	if ((ra_alg < RATE_ALG_MAX_NUM) && (ra_alg != pAd->rateAlg)) {
		UINT32 IdEntry;

		pAd->rateAlg = ra_alg;

		for (IdEntry = 0; VALID_UCAST_ENTRY_WCID(pAd, IdEntry); IdEntry++)
			pAd->MacTab.Content[IdEntry].rateAlg = ra_alg;
	}

	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Set Alg = %d\n", __func__, ra_alg));
	return TRUE;
}

#ifdef DYNAMIC_STEERING_LOADING
#ifdef REDUCE_TX_OVERHEAD
extern TMAC_TXD_L txd_l_cache_ar[];
extern UINT8 cur_wcid_ar[], last_wcid_ar[];
extern UINT8 cached_flag_ar[];
#endif
INT Set_Rps_Ratio(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	BOOLEAN fgStatus = TRUE;
	INT32 i4Recv = 0, band_idx;
	UINT32 deq_rps_ratio = 0;
	UINT32 disable_auto_rps = 0;
	UINT32 disable_auto_ratio = 0;
	UINT32 First_combo_buf_max_cnt_local = 1024;
	UINT32 Second_combo_buf_max_cnt_local = 512;
	UINT32 total_tx_process_cnt_for_specific_cpu_local = 128;
	UINT32 truncate_force_disable_local = 1;
	UINT32 disable_auto_txop_local = 0;
	UINT32 tcp_small_packet_combo_buf_max_cnt_local = 16;

#ifdef MT7626_E2_SUPPORT
	if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD)) {
		total_tx_process_cnt_for_specific_cpu_local = 128;
	} else if (IS_MT7626_FW_VER_E2(pAd)) {
		total_tx_process_cnt_for_specific_cpu_local = 128;
	} else
#endif
	{
		total_tx_process_cnt_for_specific_cpu_local = 1536;
	}
	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d", &deq_rps_ratio, &disable_auto_rps, &disable_auto_ratio,
							&truncate_force_disable_local, &disable_auto_txop_local, &First_combo_buf_max_cnt_local, &Second_combo_buf_max_cnt_local,
							&total_tx_process_cnt_for_specific_cpu_local, &tcp_small_packet_combo_buf_max_cnt_local);

			if (i4Recv <= 2) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error!\n"));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("iwpriv ra0 set rps_ratio=[deq_rps_ratio]-[disable_auto_rps]-[disable_auto_ratio]-[truncate_force_disable_local]-[disable_auto_txop_local]-[First_combo_buf_max_cnt_local]-[Second_combo_buf_max_cnt_local]-[min_tx_cnt_per_cpu]-[tcp_small_packet_combo_buf_max_cnt_local]\n"));
				fgStatus = FALSE;
				break;
			}
			pAd->First_combo_TxRpsRatio = deq_rps_ratio;
			pAd->First_combo_TxRpsRatiobaseF = pAd->First_combo_TxRpsRatio*0x100/10;
			pAd->disable_auto_rps = disable_auto_rps;
			pAd->disable_auto_ratio = disable_auto_ratio;
			pAd->disable_auto_txop = disable_auto_txop_local;
			pAd->First_combo_buf_max_cnt = First_combo_buf_max_cnt_local;
			pAd->Second_combo_buf_max_cnt = Second_combo_buf_max_cnt_local;
			pAd->truncate_force_disable = truncate_force_disable_local;
			pAd->First_combo_cnt_core0 = pAd->First_combo_cnt_core1 = 0;
			pAd->Second_combo_cnt_core0 = pAd->Second_combo_cnt_core1 = 0;
			pAd->First_combo_rx_cnt_core0 = pAd->First_combo_rx_cnt_core1 = 0;
			pAd->First_combo_cnt_core0_old = pAd->First_combo_cnt_core1_old = 0;
			pAd->First_combo_rx_cnt_core0_old = pAd->First_combo_rx_cnt_core1_old = 0;
			pAd->First_combo_buf_max_processed_cnt = 0;
			pAd->Second_combo_buf_max_processed_cnt = 0;
			pAd->total_tx_process_cnt_for_specific_cpu = total_tx_process_cnt_for_specific_cpu_local;
			pAd->tcp_small_packet_combo_buf_max_cnt = tcp_small_packet_combo_buf_max_cnt_local;
		} while (0);
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("First_combo_TxRpsRatio=%d\n", pAd->First_combo_TxRpsRatio));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("First_combo_TxRpsRatiobaseF=%d\n", pAd->First_combo_TxRpsRatiobaseF));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("disable_auto_rps=%d\n", pAd->disable_auto_rps));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("disable_auto_ratio=%d\n", pAd->disable_auto_ratio));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("disable_auto_txop=%d\n", pAd->disable_auto_txop));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("delta     tx/rx=%06d/%06d\n", pAd->delta_tx_cnt, pAd->delta_rx_cnt));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("delta_avg tx/rx=%06d/%06d\n", pAd->delta_tx_cnt_avg, pAd->delta_rx_cnt_avg));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ratio     tr/rt=%06d/%06d\n", pAd->tr_ratio, pAd->rt_ratio));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ratio_avg tr/rt=%06d/%06d\n", pAd->tr_ratio_avg, pAd->rt_ratio_avg));

	if ((pAd->First_combo_cnt_core0 + pAd->First_combo_cnt_core1) != 0)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("First_combo tasklet ratio=%d\n", pAd->First_combo_cnt_core1*0x100/(pAd->First_combo_cnt_core0 + pAd->First_combo_cnt_core1)));
	else
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("First_combo tasklet ratio=0\n"));

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("First_combo_cnt_core0=%d\n", pAd->First_combo_cnt_core0));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("First_combo_cnt_core1=%d\n", pAd->First_combo_cnt_core1));

	if ((pAd->Second_combo_cnt_core0 + pAd->Second_combo_cnt_core1) != 0)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Second_combo tasklet ratio=%d\n", pAd->Second_combo_cnt_core1*0x100/(pAd->Second_combo_cnt_core0 + pAd->Second_combo_cnt_core1)));
	else
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Second_combo tasklet ratio=0\n"));

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Second_combo_cnt_core0=%d\n", pAd->Second_combo_cnt_core0));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Second_combo_cnt_core1=%d\n", pAd->Second_combo_cnt_core1));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("First_combo_rx_cnt_core0=%d\n", pAd->First_combo_rx_cnt_core0));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("First_combo_rx_cnt_core1=%d\n", pAd->First_combo_rx_cnt_core1));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("First_combo_buf_underrun=%d\n", pAd->First_combo_buf_underrun));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("First_combo_buf_max_cnt=%d\n", pAd->First_combo_buf_max_cnt));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Second_combo_buf_max_cnt=%d\n", pAd->Second_combo_buf_max_cnt));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("tcp_small_packet_combo_buf_max_cnt=%d\n", pAd->tcp_small_packet_combo_buf_max_cnt));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("total_tx_process_cnt_for_specific_cpu=%d\n", pAd->total_tx_process_cnt_for_specific_cpu));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->tx_combo_que.Number=%d\n", pAd->tx_combo_que.Number));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->one_sec_rx_max_pkt_size=%d\n", pAd->one_sec_rx_max_pkt_size));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->one_sec_tx_max_pkt_size=%d\n", pAd->one_sec_tx_max_pkt_size));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("cpu   tx=%d\n", pAd->First_combo_tx_cpu));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("sched tx=%d,tx2=%d\n", pAd->First_combo_tx_tasklet_schedule, pAd->Second_combo_tx_tasklet_schedule));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Max processed tx=%d,tx2=%d\n",
			pAd->First_combo_buf_max_processed_cnt, pAd->Second_combo_buf_max_processed_cnt));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->con_tp_running=%d\n", pAd->con_tx_tp_running));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->con_rx_tp_running=%d\n", pAd->con_rx_tp_running));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("2G tx=%d,rx=%d\n", pAd->tp_2g_tx, pAd->tp_2g_rx));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("5G tx=%d,rx=%d\n", pAd->tp_5g_tx, pAd->tp_5g_rx));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Total tx=%d,rx=%d\n", pAd->tp_2g_tx+pAd->tp_5g_tx, pAd->tp_2g_rx+pAd->tp_5g_rx));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("truncate_enable=%d\n", pAd->truncate_enable));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->truncate_force_disable=%d\n", pAd->truncate_force_disable));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("rps_apply_idx=%d, update timestamp=%u\n", pAd->rps_apply_idx, pAd->rps_update_tick));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[HIF_RX_IDX0] max_rx_process_cnt=%d\n", pAd->PciHif.RxRing[HIF_RX_IDX0].max_rx_process_cnt));
#ifdef MT7626_E2_SUPPORT
	if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[HIF_RX_IDX2] max_rx_process_cnt=%d\n", pAd->PciHif.RxRing[HIF_RX_IDX2].max_rx_process_cnt));
	}
#endif /*MT7626_E2_SUPPORT*/
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("tasklet priority=%s\n", (pAd->tasklet_schdule_lo_flag == 1) ? "Low" : "High"));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Tput Peak[2G+5G/5G/2G]      %d/%d/%d\n",
			 pAd->concurrent_tput_flag, pAd->single_5g_tput_flag, pAd->single_2g_tput_flag));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Tput One pair[5G/2G]        %d/%d\n",
			 pAd->single_5g_one_pair_tput_flag, pAd->single_2g_one_pair_tput_flag));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Tput Normal[5G/2G]          %d/%d\n",
			(pAd->tp_5g_tx+pAd->tp_5g_rx) ? 1:0,
			(pAd->tp_2g_tx+pAd->tp_2g_rx) ? 1:0));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Tput direction[Bidi/Tx/Rx]  %d/%d/%d\n",
			 pAd->bidi_tput_flag, pAd->tx_tput_flag, pAd->rx_tput_flag));
	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("band[%d]Total air time =%d (us)\n", band_idx, pAd->truncate_sum_tx_rx_time[band_idx]));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("band[%d]air tx_ratio=%d, rx_ratio=%d \n", band_idx,
				 pAd->air_time_tx_ratio[band_idx], pAd->air_time_rx_ratio[band_idx]));
	}
#ifdef REDUCE_TX_OVERHEAD
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MT7626_REDUCE_TX_OVERHEAD defined\n"));
	{
		UINT32 *data_ptr, i, ii;
		for (i = 0; i <= MT7626_MT_WTBL_SIZE; i++) {
			if (cached_flag_ar[i] == 1) {
				data_ptr = (UINT32)&txd_l_cache_ar[i];
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("cached wcid=%d\n", i));
				for (ii = 0; ii < 8; ii++) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("0x%08x\n", data_ptr[ii]));
				}
			}
		}
	}
#else
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MT7626_REDUCE_TX_OVERHEAD not defined\n"));
#endif
#ifdef MT7626_E2_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MT7626_E2_SUPPORT defined\n"));
	if (IS_MT7626_FW_VER_E2(pAd) && IS_ASIC_CAP(pAd, fASIC_CAP_MD)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("7629 E2(MD)\n"));
	} else if (IS_MT7626_FW_VER_E2(pAd)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("7629 E2(no MD)\n"));
	} else
#endif
	{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
									("7629 E1\n"));
	}

	return fgStatus;
}
#endif /* DYNAMIC_STEERING_LOADING */

#ifdef DBG
#ifdef MT_MAC
INT Set_Fixed_Rate_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	BOOLEAN fgStatus = TRUE;
	RA_PHY_CFG_T TxPhyCfg;
	UINT_32 rate[8];
	UINT32 ret;
	INT32 i4Recv = 0;
	UINT32 u4WCID = 0;
	UINT32 u4Mode = 0, u4Bw = 0, u4Mcs = 0, u4VhtNss = 0;
	UINT32 u4SGI = 0, u4Preamble = 0, u4STBC = 0, u4LDPC = 0, u4SpeEn = 0;
	UCHAR ucNsts;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct _RTMP_CHIP_CAP *cap;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", &(u4WCID),
							&(u4Mode), &(u4Bw), &(u4Mcs), &(u4VhtNss),
							&(u4SGI), &(u4Preamble), &(u4STBC), &(u4LDPC), &(u4SpeEn));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():WCID = %d, Mode = %d, BW = %d, MCS = %d, VhtNss = %d\n"
					 "\t\t\t\tSGI = %d, Preamble = %d, STBC = %d, LDPC = %d, SpeEn = %d\n",
					 __func__, u4WCID, u4Mode, u4Bw, u4Mcs, u4VhtNss,
					 u4SGI, u4Preamble, u4STBC, u4LDPC, u4SpeEn));

			if (i4Recv != 10) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error!\n"));
				fgStatus = FALSE;
				break;
			}

			if (!VALID_UCAST_ENTRY_WCID(pAd, u4WCID)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("WCID exceed pAd->MaxUcastEntryNum!\n"));
				fgStatus = FALSE;
				break;
			}

			if (u4Mode > MODE_VHT) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Mode!\n"));
				fgStatus = FALSE;
				break;
			}

			if (u4Bw > 4) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow BW!\n"));
				fgStatus = FALSE;
				break;
			}

			if (((u4Mode == MODE_CCK) && (u4Mcs > 3)) ||
				((u4Mode == MODE_OFDM) && (u4Mcs > 7)) ||
				((u4Mode == MODE_HTMIX) && (u4Mcs > 32)) ||
				((u4Mode == MODE_VHT) && (u4Mcs > 9))) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow MCS!\n"));
				fgStatus = FALSE;
				break;
			}

			if ((u4Mode == MODE_VHT) && (u4VhtNss > 4)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow VhtNss!\n"));
				fgStatus = FALSE;
				break;
			}

			RTMP_SEM_EVENT_WAIT(&pAd->AutoRateLock, ret);
			pEntry = &pAd->MacTab.Content[u4WCID];

			if (IS_ENTRY_NONE(pEntry)) {
				RTMP_SEM_EVENT_UP(&pAd->AutoRateLock);
				break;
			}

			os_zero_mem(&TxPhyCfg, sizeof(TxPhyCfg));
			TxPhyCfg.BW = u4Bw;
			TxPhyCfg.ShortGI = u4SGI;

			if (u4LDPC)
				TxPhyCfg.ldpc = HT_LDPC | VHT_LDPC;
			else
				TxPhyCfg.ldpc = 0;

			if (u4Preamble == 0)
				u4Preamble = LONG_PREAMBLE;
			else
				u4Preamble = SHORT_PREAMBLE;

			u4STBC = raStbcSettingCheck(u4STBC, u4Mode, u4Mcs, u4VhtNss, 0, 0);
			pEntry->HTPhyMode.field.MODE = u4Mode;
			pEntry->HTPhyMode.field.iTxBF = 0;
			pEntry->HTPhyMode.field.eTxBF = 0;
			pEntry->HTPhyMode.field.STBC = u4STBC ? 1 : 0;
			pEntry->HTPhyMode.field.ShortGI = u4SGI ? 1 : 0;
			pEntry->HTPhyMode.field.BW = u4Bw;
			pEntry->HTPhyMode.field.ldpc = u4LDPC ? 1 : 0;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC

			if (u4Mode == MODE_VHT)
				pEntry->HTPhyMode.field.MCS = (((u4VhtNss - 1) & 0x3) << 4) + u4Mcs;
			else
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
			{
				pEntry->HTPhyMode.field.MCS = u4Mcs;
			}

			pEntry->LastTxRate = pEntry->HTPhyMode.word;
			pAd->LastTxRate = pEntry->HTPhyMode.word;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

			if (cap->fgRateAdaptFWOffload == TRUE) {
				CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;

				pEntry->bAutoTxRateSwitch = FALSE;
				NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
				rRaParam.FixedRateCfg.MODE = u4Mode;
				rRaParam.FixedRateCfg.STBC = u4STBC;
				rRaParam.FixedRateCfg.ShortGI = u4SGI;
				rRaParam.FixedRateCfg.BW = u4Bw;
				rRaParam.FixedRateCfg.ldpc = TxPhyCfg.ldpc;
				rRaParam.FixedRateCfg.MCS = u4Mcs;
				rRaParam.FixedRateCfg.VhtNss = u4VhtNss;
				rRaParam.ucShortPreamble = u4Preamble;
				rRaParam.ucSpeEn = u4SpeEn;
				rRaParam.u4Field = RA_PARAM_FIXED_RATE;
				RAParamUpdate(pAd, pEntry, &rRaParam);
			} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
			{
#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
					pAd->StaCfg[0].wdev.bAutoTxRateSwitch = FALSE;
				}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
					INT	apidx = 0;

					for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
						pAd->ApCfg.MBSSID[apidx].wdev.bAutoTxRateSwitch = FALSE;
				}
#endif /* CONFIG_AP_SUPPORT */
				ucNsts = get_nsts_by_mcs(u4Mode, u4Mcs, u4STBC, u4VhtNss);
				rate[0] = tx_rate_to_tmi_rate(u4Mode,
											  u4Mcs,
											  ucNsts,
											  u4STBC,
											  u4Preamble);
				rate[0] &= 0xfff;
				rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];
				MtAsicTxCapAndRateTableUpdate(pAd, u4WCID, &TxPhyCfg, rate, u4SpeEn);
			}

			RTMP_SEM_EVENT_UP(&pAd->AutoRateLock);
		} while (0);
	}

	if (fgStatus == FALSE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("iwpriv ra0 set FixedRate=[WCID]-[Mode]-[BW]-[MCS]-[VhtNss]-[SGI]-[Preamble]-[STBC]-[LDPC]-[SPE_EN]\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[WCID]Wireless Client ID\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[Mode]CCK=0, OFDM=1, HT=2, GF=3, VHT=4\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[BW]BW20=0, BW40=1, BW80=2,BW160=3\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[MCS]CCK=0~4, OFDM=0~7, HT=0~32, VHT=0~9\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[VhtNss]VHT=1~4, Other=ignore\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[Preamble]Long=0, Other=Short\n"));
	} else
		dump_wtbl_info(pAd, u4WCID);

	return fgStatus;
}


INT Set_Fixed_Rate_With_FallBack_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	BOOLEAN fgStatus = TRUE;
	RA_PHY_CFG_T TxPhyCfg;
	UINT_32 rate[8];
	UINT32 ret;
	INT32 i4Recv = 0;
	UINT32 u4WCID = 0;
	UINT32 u4Mode = 0, u4Bw = 0, u4Mcs = 0, u4VhtNss = 0;
	UINT32 u4SGI = 0, u4Preamble = 0, u4STBC = 0, u4LDPC = 0, u4SpeEn = 0, u4Is5G = 0;
	UCHAR ucNsts;
	MAC_TABLE_ENTRY *pEntry = NULL;
#if defined(RATE_ADAPT_AGBS_SUPPORT) && !defined(RACTRL_FW_OFFLOAD_SUPPORT)
	UINT_32 u4TableSize;
	UINT_16 *pu2FallbackTable = NULL;
	UINT_8 ucIndex;
	BOOL fgFound = FALSE;
	extern UINT_16 HwFallbackTable11B[32];
	extern UINT_16 HwFallbackTable11G[64];
	extern UINT_16 HwFallbackTable11BG[56];
	extern UINT_16 HwFallbackTable11N1SS[80];
	extern UINT_16 HwFallbackTable11N2SS[80];
	extern UINT_16 HwFallbackTable11N3SS[80];
	extern UINT_16 HwFallbackTable11N4SS[80];
	extern UINT_16 HwFallbackTableBGN1SS[80];
	extern UINT_16 HwFallbackTableBGN2SS[80];
	extern UINT_16 HwFallbackTableBGN3SS[80];
	extern UINT_16 HwFallbackTableBGN4SS[80];
	extern UINT_16 HwFallbackTableVht1SS[80];
	extern UINT_16 HwFallbackTableVht2SS[80];
	extern UINT_16 HwFallbackTableVht3SS[80];
	extern UINT_16 HwFallbackTableVht4SS[80];
#endif /* RATE_ADAPT_AGBS_SUPPORT */
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", &(u4WCID),
							&(u4Mode), &(u4Bw), &(u4Mcs), &(u4VhtNss),
							&(u4SGI), &(u4Preamble), &(u4STBC), &(u4LDPC), &(u4SpeEn), &(u4Is5G));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():WCID = %d, Mode = %d, BW = %d, MCS = %d, VhtNss = %d\n"
					 "\t\t\t\tSGI = %d, Preamble = %d, STBC = %d, LDPC = %d, SpeEn = %d, Is5G = %d\n",
					 __func__, u4WCID, u4Mode, u4Bw, u4Mcs, u4VhtNss,
					 u4SGI, u4Preamble, u4STBC, u4LDPC, u4SpeEn, u4Is5G));

			if (i4Recv != 11) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error!\n"));
				fgStatus = FALSE;
				break;
			}

			if (!VALID_UCAST_ENTRY_WCID(pAd, u4WCID)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("WCID exceed pAd->MaxUcastEntryNum!\n"));
				fgStatus = FALSE;
				break;
			}

			if (u4Mode > MODE_VHT) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Mode!\n"));
				fgStatus = FALSE;
				break;
			}

			if (u4Bw > 4) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow BW!\n"));
				fgStatus = FALSE;
				break;
			}

			if (((u4Mode == MODE_CCK) && (u4Mcs > 3)) ||
				((u4Mode == MODE_OFDM) && (u4Mcs > 7)) ||
				((u4Mode == MODE_HTMIX) && (u4Mcs > 32)) ||
				((u4Mode == MODE_VHT) && (u4Mcs > 9))) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow MCS!\n"));
				fgStatus = FALSE;
				break;
			}

			if ((u4Mode == MODE_VHT) && (u4VhtNss > 4)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow VhtNss!\n"));
				fgStatus = FALSE;
				break;
			}

			RTMP_SEM_EVENT_WAIT(&pAd->AutoRateLock, ret);
			pEntry = &pAd->MacTab.Content[u4WCID];

			if (IS_ENTRY_NONE(pEntry)) {
				RTMP_SEM_EVENT_UP(&pAd->AutoRateLock);
				break;
			}

			os_zero_mem(&TxPhyCfg, sizeof(TxPhyCfg));
			TxPhyCfg.BW = u4Bw;
			TxPhyCfg.ShortGI = u4SGI;

			if (u4LDPC)
				TxPhyCfg.ldpc = HT_LDPC | VHT_LDPC;
			else
				TxPhyCfg.ldpc = 0;

			if (u4Preamble == 0)
				u4Preamble = LONG_PREAMBLE;
			else
				u4Preamble = SHORT_PREAMBLE;

			u4STBC = raStbcSettingCheck(u4STBC, u4Mode, u4Mcs, u4VhtNss, 0, 0);
			pEntry->HTPhyMode.field.MODE = u4Mode;
			pEntry->HTPhyMode.field.iTxBF = 0;
			pEntry->HTPhyMode.field.eTxBF = 0;
			pEntry->HTPhyMode.field.STBC = u4STBC ? 1 : 0;
			pEntry->HTPhyMode.field.ShortGI = u4SGI ? 1 : 0;
			pEntry->HTPhyMode.field.BW = u4Bw;
			pEntry->HTPhyMode.field.ldpc = u4LDPC ? 1 : 0;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC

			if (u4Mode == MODE_VHT)
				pEntry->HTPhyMode.field.MCS = (((u4VhtNss - 1) & 0x3) << 4) + u4Mcs;
			else
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
			{
				pEntry->HTPhyMode.field.MCS = u4Mcs;
			}

			pEntry->LastTxRate = pEntry->HTPhyMode.word;
			pAd->LastTxRate = pEntry->HTPhyMode.word;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

			if (cap->fgRateAdaptFWOffload == TRUE) {
				CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;

				pEntry->bAutoTxRateSwitch = FALSE;
				NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
				rRaParam.FixedRateCfg.MODE = u4Mode;
				rRaParam.FixedRateCfg.STBC = u4STBC;
				rRaParam.FixedRateCfg.ShortGI = u4SGI;
				rRaParam.FixedRateCfg.BW = u4Bw;
				rRaParam.FixedRateCfg.ldpc = TxPhyCfg.ldpc;
				rRaParam.FixedRateCfg.MCS = u4Mcs;
				rRaParam.FixedRateCfg.VhtNss = u4VhtNss;
				rRaParam.ucShortPreamble = u4Preamble;
				rRaParam.ucSpeEn = u4SpeEn;
				rRaParam.fgIs5G = u4Is5G ? TRUE : FALSE;
				rRaParam.u4Field = RA_PARAM_FIXED_RATE_FALLBACK;
				RAParamUpdate(pAd, pEntry, &rRaParam);
			} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
			{
#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
					pAd->StaCfg[0].wdev.bAutoTxRateSwitch = FALSE;
				}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
					INT	apidx = 0;

					for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
						pAd->ApCfg.MBSSID[apidx].wdev.bAutoTxRateSwitch = FALSE;
				}
#endif /* CONFIG_AP_SUPPORT */
				ucNsts = get_nsts_by_mcs(u4Mode, u4Mcs, u4STBC, u4VhtNss);
				rate[0] = tx_rate_to_tmi_rate(u4Mode,
											  u4Mcs,
											  ucNsts,
											  u4STBC,
											  u4Preamble);
				rate[0] &= 0xfff;
#if defined(RATE_ADAPT_AGBS_SUPPORT) && !defined(RACTRL_FW_OFFLOAD_SUPPORT)

				if (u4Mode == MODE_CCK) {
					pu2FallbackTable = HwFallbackTable11B;
					u4TableSize = sizeof(HwFallbackTable11B) / 2;
				} else if (u4Mode == MODE_OFDM) {
					if (u4Is5G == TRUE) {
						pu2FallbackTable = HwFallbackTable11G;
						u4TableSize = sizeof(HwFallbackTable11G) / 2;
					} else {
						pu2FallbackTable = HwFallbackTable11BG;
						u4TableSize = sizeof(HwFallbackTable11BG) / 2;
					}
				} else if ((u4Mode == MODE_HTMIX) || (u4Mode == MODE_HTGREENFIELD)) {
					UINT_8 ucHtNss = 1;

					ucHtNss += (u4Mcs >> 3);

					if (u4Is5G == TRUE) {
						switch (ucHtNss) {
						case 1:
							pu2FallbackTable = HwFallbackTable11N1SS;
							u4TableSize = sizeof(HwFallbackTable11N1SS) / 2;
							break;

						case 2:
							pu2FallbackTable = HwFallbackTable11N2SS;
							u4TableSize = sizeof(HwFallbackTable11N2SS) / 2;
							break;

						case 3:
							pu2FallbackTable = HwFallbackTable11N3SS;
							u4TableSize = sizeof(HwFallbackTable11N3SS) / 2;
							break;

						case 4:
							pu2FallbackTable = HwFallbackTable11N4SS;
							u4TableSize = sizeof(HwFallbackTable11N4SS) / 2;
							break;

						default:
							MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Nss%d!\n", ucHtNss));
							break;
						}
					} else {
						switch (ucHtNss) {
						case 1:
							pu2FallbackTable = HwFallbackTableBGN1SS;
							u4TableSize = sizeof(HwFallbackTableBGN1SS) / 2;
							break;

						case 2:
							pu2FallbackTable = HwFallbackTableBGN2SS;
							u4TableSize = sizeof(HwFallbackTableBGN2SS) / 2;
							break;

						case 3:
							pu2FallbackTable = HwFallbackTableBGN3SS;
							u4TableSize = sizeof(HwFallbackTableBGN3SS) / 2;
							break;

						case 4:
							pu2FallbackTable = HwFallbackTableBGN4SS;
							u4TableSize = sizeof(HwFallbackTableBGN4SS) / 2;
							break;

						default:
							MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Nss%d!\n", ucHtNss));
							break;
						}
					}
				} else if (u4Mode == MODE_VHT) {
					switch (u4VhtNss) {
					case 1:
						pu2FallbackTable = HwFallbackTableVht1SS;
						u4TableSize = sizeof(HwFallbackTableVht1SS) / 2;
						break;

					case 2:
						pu2FallbackTable = HwFallbackTableVht2SS;
						u4TableSize = sizeof(HwFallbackTableVht2SS) / 2;
						break;

					case 3:
						{
							pu2FallbackTable = HwFallbackTableVht3SS;
							u4TableSize = sizeof(HwFallbackTableVht3SS) / 2;
						}

						break;

					case 4:
						pu2FallbackTable = HwFallbackTableVht4SS;
						u4TableSize = sizeof(HwFallbackTableVht4SS) / 2;
						break;

					default:
						MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Nss%d!\n", u4VhtNss));
						break;
					}
				}

				if (pu2FallbackTable != NULL) {
					for (ucIndex = 0; ucIndex < u4TableSize; ucIndex += 8) {
						union RA_RATE_CODE rInitialRate;

						rInitialRate.word = *(pu2FallbackTable + ucIndex);

						if (rInitialRate.field.mcs == u4Mcs) {
							fgFound = TRUE;
							break;
						}
					}

					if (fgFound) {
						UINT_8 ucIdx;
						union RA_RATE_CODE rRateCode;

						for (ucIdx = 1; ucIdx < 8; ucIdx++) {
							rRateCode.word = *(pu2FallbackTable + ucIndex + ucIdx);

							if (((u4Mode == MODE_HTMIX) || (u4Mode == MODE_VHT))
								&& u4STBC && (rRateCode.field.nsts == 0)) {
								rRateCode.field.nsts = 1;
								rRateCode.field.stbc = 1;
							}

							rate[ucIdx] = rRateCode.word;
						}
					}
				}

				if (!fgFound) {
					rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];
					MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Cannot find fallback table!\n"));
				}

#else
				rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];
#endif /* RATE_ADAPT_AGBS_SUPPORT */
				MtAsicTxCapAndRateTableUpdate(pAd, u4WCID, &TxPhyCfg, rate, u4SpeEn);
			}

			RTMP_SEM_EVENT_UP(&pAd->AutoRateLock);
		} while (0);
	}

	if (fgStatus == FALSE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("iwpriv ra0 set FixedRateFallback=[WCID]-[Mode]-[BW]-[MCS]-[VhtNss]-[SGI]-[Preamble]-[STBC]-[LDPC]-[SPE_EN]-[is5G]\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[WCID]Wireless Client ID\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[Mode]CCK=0, OFDM=1, HT=2, GF=3, VHT=4\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[BW]BW20=0, BW40=1, BW80=2,BW160=3\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[MCS]CCK=0~4, OFDM=0~7, HT=0~32, VHT=0~9\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[VhtNss]VHT=1~4, Other=ignore\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[Preamble]Long=0, Other=Short\n"));
	} else
		dump_wtbl_info(pAd, u4WCID);

	return fgStatus;
}

INT
Set_RA_Debug_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg)
{
	UINT32 u4WlanIndex = 0, u4DebugType = 0;
	RTMP_STRING *pWlanIndex  = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: arg = %s\n", __func__, arg));
	pWlanIndex = strsep(&arg, ":");

	if (pWlanIndex == NULL || arg == NULL) {
		MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	u4WlanIndex = os_str_toul(pWlanIndex, 0, 10);
	u4DebugType = os_str_toul(arg, 0, 10);

	if (!VALID_UCAST_ENTRY_WCID(pAd, u4WlanIndex)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("u4WlanIndex exceed pAd->MaxUcastEntryNum!\n"));
		return FALSE;
	}
	pEntry = &pAd->MacTab.Content[u4WlanIndex];

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	if (cap->fgRateAdaptFWOffload == TRUE) {
		CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;

		if (u4DebugType < RA_PARAM_MAX)
			return FALSE;

		NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));

		rRaParam.u4Field = u4DebugType;
		RAParamUpdate(pAd, pEntry, &rRaParam);
	} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	{
		return FALSE;
	}

	return TRUE;
}


#define MAX_VHT_NSS_FIXED_RATE                  4
#define FIXED_RATE_WO_STA_PARAM_LIST_MAX        10

INT Set_Fixed_Rate_WO_STA_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	BOOLEAN fgStatus = TRUE;
	RA_PHY_CFG_T TxPhyCfg;
	INT32 i4Recv = 0;
	UINT32 u4Wcid = 0;
	UINT32 u4Mode = 0, u4Bw = 0, u4Mcs = 0, u4VhtNss = 0;
	UINT32 u4ShortGI = 0, u4Preamble = 0, u4Stbc = 0, u4Ldpc = 0, u4SpeEn = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%u-%u-%u-%u-%u-%u-%u-%u-%u-%u", &(u4Wcid),
					&(u4Mode), &(u4Bw), &(u4Mcs), &(u4VhtNss),
					&(u4ShortGI), &(u4Preamble), &(u4Stbc), &(u4Ldpc), &(u4SpeEn));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s():WCID = %d, Mode = %d, BW = %d, MCS = %d, VhtNss = %d\n"
				"\t\t\t\tSGI = %d, Preamble = %d, STBC = %d, LDPC = %d, SpeEn = %d\n",
				__func__, u4Wcid, u4Mode, u4Bw, u4Mcs, u4VhtNss,
				u4ShortGI, u4Preamble, u4Stbc, u4Ldpc, u4SpeEn));

			if (i4Recv != FIXED_RATE_WO_STA_PARAM_LIST_MAX) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Input format Error!\n"));
				fgStatus = FALSE;
				break;
			}

			if (u4Mode > MODE_VHT) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Unknown Mode!\n"));
				fgStatus = FALSE;
				break;
			}

			if (u4Bw > BW_160) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Unknown BW!\n"));
				fgStatus = FALSE;
				break;
			}

			if (((u4Mode == MODE_CCK) && (u4Mcs > MCS_LONGP_RATE_11)) ||
				((u4Mode == MODE_OFDM) && (u4Mcs > MCS_32)) ||
				((u4Mode == MODE_HTMIX) && (u4Mcs > MCS_32)) ||
				((u4Mode == MODE_VHT) && (u4Mcs > VHT_RATE_IDX_1SS_MCS9))) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Unknown MCS!\n"));
				fgStatus = FALSE;
				break;
			}

			if ((u4Mode == MODE_VHT) && (u4VhtNss > MAX_VHT_NSS_FIXED_RATE)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Unknown VhtNss!\n"));
				fgStatus = FALSE;
				break;
			}

			os_zero_mem(&TxPhyCfg, sizeof(TxPhyCfg));

			if (u4Ldpc)
				TxPhyCfg.ldpc = 1;

			if (u4Preamble)
				u4Preamble = SHORT_PREAMBLE;
			else
				u4Preamble = LONG_PREAMBLE;

			u4Stbc = raStbcSettingCheck((UINT8)u4Stbc, (UINT8)u4Mode, (UINT8)u4Mcs,
				(UINT8)u4VhtNss, 0, 0);

			TxPhyCfg.STBC = (UINT8)u4Stbc;
			TxPhyCfg.MODE = (UINT8)u4Mode;
			TxPhyCfg.ShortGI = (UINT8)u4ShortGI;
			TxPhyCfg.BW = (UINT8)u4Bw;
			TxPhyCfg.MCS = (UINT8)u4Mcs;
			TxPhyCfg.VhtNss = (UINT8)u4VhtNss;

			CmdRaFixRateUpdateWoSta(pAd, (UINT8)u4Wcid, &TxPhyCfg, (UINT8)u4SpeEn,
				(UINT8)u4Preamble);
		} while (0);
	}


	if (fgStatus == FALSE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("iwpriv ra0 set FixedRate=[WCID]-[Mode]-[BW]-[MCS]-[VhtNss]-[SGI]-[Preamble]-[STBC]-[LDPC]-[SPE_EN]\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[WCID]Wireless Client ID\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[Mode]CCK=0, OFDM=1, HT=2, GF=3, VHT=4\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[BW]BW20=0, BW40=1, BW80=2,BW160=3\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[MCS]CCK=0~4, OFDM=0~7, HT=0~32, VHT=0~9\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[VhtNss]VHT=1~4, Other=ignore\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[Preamble]Long=0, Other=Short\n"));
	} else {
		dump_wtbl_info(pAd, (UINT8)u4Wcid);
	}

	return fgStatus;
}

INT
Set_RA_Per_Thd_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg
)
{
	UINT32 u4WlanIndex = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	u4WlanIndex = os_str_toul(arg, 0, 10);

	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: WlanIdx:%d\n", __func__, u4WlanIndex));

	if (!VALID_UCAST_ENTRY_WCID(pAd, u4WlanIndex)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("u4WlanIndex exceed pAd->MaxUcastEntryNum!\n"));
		return FALSE;
	}

	pEntry = &pAd->MacTab.Content[u4WlanIndex];

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	if (cap->fgRateAdaptFWOffload == TRUE) {
		CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;

		NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));

		rRaParam.u4Field = RA_PARAM_VERIWAVE_RVR_PER;
		RAParamUpdate(pAd, pEntry, &rRaParam);
	} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	{
		return FALSE;
	}

	return TRUE;
}

#endif /* MT_MAC */
#endif /* DBG */

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
INT
Set_SupRateSet_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR rate[] = { 0x82, 0x84, 0x8b, 0x96, 0x8C, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6c};
	INT SupRateSetBitmap = 0, i = 0;
	MAC_TABLE_ENTRY *pMacEntry;
	P_RA_ENTRY_INFO_T pRaEntry;

	SupRateSetBitmap = (INT) os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("SupRateSetBitmap %x\n", SupRateSetBitmap));
	if (SupRateSetBitmap > 4095) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("Set_SupRateSet_Proc::error ForceRateSetBitmap(%04X) > 4096\n", SupRateSetBitmap));
		return FALSE;
	}

	if (!wdev)
		return FALSE;

	wdev->rate.Eap_SupRate_En = TRUE;
	wdev->rate.EapSupRateLen = 0;
	wdev->rate.EapExtSupRateLen = 0;

	for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
		if (SupRateSetBitmap & (1 << i)) {
			if (WMODE_EQUAL(wdev->PhyMode, WMODE_B) && (wdev->channel <= 14)) {
				wdev->rate.EapSupRate[wdev->rate.EapSupRateLen] = rate[i];
				wdev->rate.EapSupRateLen++;
				wdev->rate.EapSupportCCKMCS |= (1 << i);
				wdev->rate.EapSupportRateMode |= SUPPORT_CCK_MODE;
			} else if (wdev->channel > 14 && (i > 3)) {
				wdev->rate.EapSupRate[wdev->rate.EapSupRateLen] = rate[i];
				wdev->rate.EapSupRateLen++;
				wdev->rate.EapSupportOFDMMCS |= (1 << (i - 4));
				wdev->rate.EapSupportRateMode |= SUPPORT_OFDM_MODE;
			} else {
				if ((i < 4) || (i == 5) || (i == 7) || (i == 9) || (i == 11)) {
					wdev->rate.EapSupRate[wdev->rate.EapSupRateLen] = rate[i];
					wdev->rate.EapSupRateLen++;
					if (i < 4) {
						wdev->rate.EapSupportCCKMCS |= (1 << i);
						wdev->rate.EapSupportRateMode |= SUPPORT_CCK_MODE;
					} else {
						wdev->rate.EapSupportOFDMMCS |= (1 << (i - 4));
						wdev->rate.EapSupportRateMode |= SUPPORT_OFDM_MODE;
					}
				} else {
					wdev->rate.EapExtSupRate[wdev->rate.EapExtSupRateLen] = rate[i] & 0x7f;
					wdev->rate.EapExtSupRateLen++;
					wdev->rate.EapSupportOFDMMCS |= (1 << (i - 4));
					wdev->rate.EapSupportRateMode |= SUPPORT_OFDM_MODE;
				}
			}
		}
	}
	RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);

	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pMacEntry = &pAd->MacTab.Content[i];

	    if (IS_ENTRY_CLIENT(pMacEntry) && (pMacEntry->Sst == SST_ASSOC)) {
		if (pMacEntry->wdev != wdev)
			continue;

		pRaEntry = &pMacEntry->RaEntry;
		raWrapperEntrySet(pAd, pMacEntry, pRaEntry);
		WifiSysRaInit(pAd, pMacEntry);
	    }
	}

	return TRUE;
}

INT
Set_HtSupRateSet_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 HtSupRateSetBitmap = 0, i = 0;
	MAC_TABLE_ENTRY *pMacEntry;
	P_RA_ENTRY_INFO_T pRaEntry;

	HtSupRateSetBitmap = (UINT32) os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("HtSupRateSetBitmap %x\n", HtSupRateSetBitmap));
	if (!wdev)
		return FALSE;

	wdev->rate.Eap_HtSupRate_En = TRUE;
	wdev->rate.EapSupportHTMCS = HtSupRateSetBitmap;
	wdev->rate.EapMCSSet[0] = HtSupRateSetBitmap & 0x000000ff;
	wdev->rate.EapMCSSet[1] = (HtSupRateSetBitmap & 0x0000ff00) >> 8;
	wdev->rate.EapMCSSet[2] = (HtSupRateSetBitmap & 0x00ff0000) >> 16;
	wdev->rate.EapMCSSet[3] = (HtSupRateSetBitmap & 0xff000000) >> 24;
#ifdef DOT11_N_SUPPORT
	SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);

	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pMacEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(pMacEntry) && (pMacEntry->Sst == SST_ASSOC)) {
			if (pMacEntry->wdev != wdev)
				continue;
			pRaEntry = &pMacEntry->RaEntry;
			raWrapperEntrySet(pAd, pMacEntry, pRaEntry);
			WifiSysRaInit(pAd, pMacEntry);
	    }
	}

	return TRUE;
}

INT Set_VhtSupRateSet_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 VhtSupRateSetBitmap = 0;
	UINT32 i = 0;
	MAC_TABLE_ENTRY *pMacEntry;
	P_RA_ENTRY_INFO_T pRaEntry;

	VhtSupRateSetBitmap = (UINT32) os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("VhtSupRateSetBitmap %x\n", VhtSupRateSetBitmap));
	if (!wdev)
		return FALSE;

	wdev->rate.Eap_VhtSupRate_En = TRUE;
	wdev->rate.rx_mcs_map.mcs_ss1 = VhtSupRateSetBitmap & 0x0000003;
	wdev->rate.rx_mcs_map.mcs_ss2 = (VhtSupRateSetBitmap & 0x0000000c) >> 2;
	wdev->rate.rx_mcs_map.mcs_ss3 = (VhtSupRateSetBitmap & 0x00000030) >> 4;
	wdev->rate.rx_mcs_map.mcs_ss4 = (VhtSupRateSetBitmap & 0x000000c0) >> 6;
	wdev->rate.rx_mcs_map.mcs_ss5 = (VhtSupRateSetBitmap & 0x00000300) >> 8;
	wdev->rate.rx_mcs_map.mcs_ss6 = (VhtSupRateSetBitmap & 0x00000c00) >> 10;
	wdev->rate.rx_mcs_map.mcs_ss7 = (VhtSupRateSetBitmap & 0x00003000) >> 12;
	wdev->rate.rx_mcs_map.mcs_ss8 = (VhtSupRateSetBitmap & 0x0000c000) >> 14;

	wdev->rate.tx_mcs_map.mcs_ss1 = (VhtSupRateSetBitmap & 0x00030000) >> 16;
	wdev->rate.tx_mcs_map.mcs_ss2 = (VhtSupRateSetBitmap & 0x000c0000) >> 18;
	wdev->rate.tx_mcs_map.mcs_ss3 = (VhtSupRateSetBitmap & 0x00300000) >> 20;
	wdev->rate.tx_mcs_map.mcs_ss4 = (VhtSupRateSetBitmap & 0x00c00000) >> 22;
	wdev->rate.tx_mcs_map.mcs_ss5 = (VhtSupRateSetBitmap & 0x03000000) >> 24;
	wdev->rate.tx_mcs_map.mcs_ss6 = (VhtSupRateSetBitmap & 0x0c000000) >> 26;
	wdev->rate.tx_mcs_map.mcs_ss7 = (VhtSupRateSetBitmap & 0x30000000) >> 28;
	wdev->rate.tx_mcs_map.mcs_ss8 = (VhtSupRateSetBitmap & 0xc0000000) >> 30;

#ifdef DOT11_N_SUPPORT
	SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pMacEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(pMacEntry) && (pMacEntry->Sst == SST_ASSOC)) {
			if (pMacEntry->wdev != wdev)
				continue;
		pRaEntry = &pMacEntry->RaEntry;
		raWrapperEntrySet(pAd, pMacEntry, pRaEntry);
		WifiSysRaInit(pAd, pMacEntry);
	    }
	}
	return TRUE;
}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

#endif /* COMPOS_WIN */

