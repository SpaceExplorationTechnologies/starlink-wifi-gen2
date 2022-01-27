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

#ifndef __TR_H__
#define __TR_H__

#define HW_TX_BATCH_CNT 3
#define HW_QUE_AGG_TIMEOUT 0x90
#define HW_MAX_AGG_EN_TP 300
#define HW_MIN_AGG_EN_TP 50

struct tx_delay_control {
	RALINK_TIMER_STRUCT que_agg_timer;
	BOOLEAN que_agg_timer_running;
	BOOLEAN que_agg_en;
	BOOLEAN force_deq;
#define TX_BATCH_CNT 4
	UINT32 tx_process_batch_cnt;
#define MIN_AGG_PKT_LEN 58
#define MAX_AGG_PKT_LEN 135
#define MAX_AGG_EN_TP 700
#define MIN_AGG_EN_TP 50
	UINT32 min_pkt_len;
	UINT32 max_pkt_len;
#define QUE_AGG_TIMEOUT 4000
	UINT32 que_agg_timeout_value;
	BOOLEAN hw_enabled;
};

struct dly_ctl_cfg {
	UINT32 avg_tp;
	UINT32 dly_cfg;
};

struct tr_delay_control {
	RALINK_TIMER_STRUCT rx_delay_timer;
	BOOLEAN rx_delay_timer_running;
	BOOLEAN rx_delay_en;
	struct dly_ctl_cfg *dl_rx_dly_ctl_tbl;
	UINT32 dl_rx_dly_ctl_tbl_size;
	struct dly_ctl_cfg *ul_rx_dly_ctl_tbl;
	UINT32 ul_rx_dly_ctl_tbl_size;
	BOOLEAN tx_delay_en;
};
#ifdef IXIA_SUPPORT
typedef enum _R_DROP_REASON_ {
	RPKT_SUCCESS = 0,
	ALREADY_IN_ORDER = 1,
	DUP_SEQ_PKT_EQ = 2,
	DUP_SEQ_PKT_SMALL = 3,
	DROP_DATA_SIZE = 4,
	DROP_RXD_ERROR = 5,
	DROP_BA_STAT_ERROR = 6,
	DROP_NOT_SEC = 7,
	DROP_NO_EAP = 8,
	DROP_MWDS = 9,
	DROP_WDEV_INDEX = 10,
	DROP_WDEV_INDEX1 = 11,
	DROP_FC_NULL = 12,
	DROP_RX_ORDER = 13,
	DROP_RX_HALT = 14,
	DROP_NOTTO_OS = 15,
	DROP_PN_ERROR = 16,
	DROP_NOT_TOOS = 17,
	DROP_MSDU_REL = 18,
	DROP_MPDU_REL = 19,
	DROP_RXLEN_DISMATCH = 20,
	DROP_RXLEN_DISMATCH1 = 21,
	DROP_RX_NOT_ALLOW = 22,
	DROP_RX_NOT_ALLOW8023 = 23,
	DROP_RX_NOT_ALLOW80211 = 24,
	DROP_RX_TYPE_ERR = 25,
	DROP_RX_DMA_BZ = 26,
	DROP_RX_CUT = 27,
	DROP_RXSWENC_FAIL = 28,
	DROP_RXMAC_LEN = 29,
	MAX_RDROP_RESON
} R_DROP_RESON;
typedef enum DROP_REASON {
	PKT_SUCCESS = 0,
	INVALID_TR_WCID = 1,
	INVALID_OP_STAT = 2,
	INVALID_WDEV = 3,
	INVALID_PKT_TYPE = 4,
	DROP_PSQ_FULL = 5,
	DROP_TXQ_FULL = 6,
	DROP_TXQ_ENQ_FAIL = 7,
	DROP_TXQ_ENQ_PS = 8,
	DROP_HW_RESET = 9,
	DROP_80211H_MODE = 10,
	DROP_TXBLK_FAIL = 11,
	DROP_TXSWENCPT_FAIL = 12,
	DROP_TXVOWTXD_FAIL = 13,
	DROP_TXVOWDROP = 14,
	DROP_TXRED = 15,
	DROP_TXHIQ_EXCD = 16,
	DROP_RRM_QUIET = 17,
	INVALID_WDEV2 = 18,
	DROP_ENQ_FAIL = 19,
	BB_SW_NAT = 20,
	FORBID_DATA_TX = 21,
	PDMA_STOPPING = 22,
	WIFI_PKT_FWD = 23,
	DROP_TXP_FAIL = 24,
	FRAG_ID_ERROR = 25,
	NO_HW_RESOURCE = 26,
	NO_HW_RESOURCE_HI = 27,
	MAX_TDROP_RESON
} T_DROP_RESON;
enum {
	TX_AMPDU = 0,
	TX_AMSDU,
	TX_LEGACY,
	TX_FRAG,
	TX_TYPE_MAX
};
#endif /*IXIA_SUPPORT*/
enum {
	TP_DEBUG_ISR = (1 << 0),
	TP_DEBUG_TIMING = (1 << 1),
	TP_DEBUG_IO = (1 << 2),
};
#define TP_DBG_TIME_SLOT_NUMS 10
struct tp_debug {
	UINT32 IsrTxDlyCnt;
	UINT32 IsrTxDlyCntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IsrTxCnt;
	UINT32 IsrTxCntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IsrRxDlyCnt;
	UINT32 IsrRxDlyCntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IsrRxCnt;
	UINT32 IsrRxCntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IsrRx1Cnt;
	UINT32 IsrRx1CntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoReadTx;
	UINT32 IoReadTxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoWriteTx;
	UINT32 IoWriteTxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoReadRx;
	UINT32 IoReadRxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoReadRx1;
	UINT32 IoReadRx1Rec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoWriteRx;
	UINT32 IoWriteRxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoWriteRx1;
	UINT32 IoWriteRx1Rec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 MaxProcessCntRx;
	UINT32 MaxProcessCntRxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 MaxProcessCntRx1;
	UINT32 MaxProcessCntRx1Rec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 RxMaxProcessCntA;
	UINT32 MaxProcessCntRxRecA[TP_DBG_TIME_SLOT_NUMS];
	UINT32 RxMaxProcessCntB;
	UINT32 MaxProcessCntRxRecB[TP_DBG_TIME_SLOT_NUMS];
	UINT32 RxMaxProcessCntC;
	UINT32 MaxProcessCntRxRecC[TP_DBG_TIME_SLOT_NUMS];
	UINT32 RxMaxProcessCntD;
	UINT32 MaxProcessCntRxRecD[TP_DBG_TIME_SLOT_NUMS];
	UINT32 Rx1MaxProcessCntA;
	UINT32 MaxProcessCntRx1RecA[TP_DBG_TIME_SLOT_NUMS];
	UINT32 Rx1MaxProcessCntB;
	UINT32 MaxProcessCntRx1RecB[TP_DBG_TIME_SLOT_NUMS];
	UINT32 Rx1MaxProcessCntC;
	UINT32 MaxProcessCntRx1RecC[TP_DBG_TIME_SLOT_NUMS];
	UINT32 Rx1MaxProcessCntD;
	UINT32 MaxProcessCntRx1RecD[TP_DBG_TIME_SLOT_NUMS];
	/* Timeing */
	UINT32 TRDoneTimes;
	UINT32 TRDoneTimesRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 TRDoneTimeStamp;
	UINT32 TRDoneInterval[TP_DBG_TIME_SLOT_NUMS];
	UINT32 RxDropPacket;
	UINT16 time_slot;
	UINT8  debug_flag;
	RALINK_TIMER_STRUCT tp_dbg_history_timer;
#ifdef IXIA_SUPPORT
	UINT32 TxDropPacket[MAX_TDROP_RESON];
	UINT32 RxDropPacketCnt[MAX_RDROP_RESON];
	UINT tx_pkt_from_os;
	UINT tx_pkt_deq_cnt;
	UINT tx_pkt_enq_cnt;
	UINT tx_pkt_to_hw[TX_TYPE_MAX];
	UINT tx_pkt_len;
	UINT txpkt_dct_period;
	UINT rx_pkt_len;
	UINT rxpkt_dct_period;
	UINT rxListRlsCnt;
	UINT rxStepOneCnt;
	UINT rxWithinCnt;
	UINT rxSurpassCnt;
#endif /*IXIA_SUPPORT*/
};

struct tr_flow_control {
	UINT8 *TxFlowBlockState;
	NDIS_SPIN_LOCK *TxBlockLock;
	DL_LIST *TxBlockDevList;
	BOOLEAN IsTxBlocked;
	UINT8 RxFlowBlockState;
};

struct tx_rx_ctl {
	struct tr_flow_control tr_flow_ctl;
	struct tx_delay_control tx_delay_ctl;
	UINT32 tx_sw_q_drop;
	UINT32 net_if_stop_cnt;
#if defined(CONFIG_TP_DBG) || defined(IXIA_SUPPORT)
	struct tp_debug tp_dbg;
#endif /*IXIA_SUPPORT*/
	struct tr_delay_control tr_delay_ctl;
	UINT32 rx_icv_err_cnt;
};


struct _RTMP_ADAPTER;

enum {
	NO_ENOUGH_SWQ_SPACE = (1 << 0),
};

INT32 tr_ctl_init(struct _RTMP_ADAPTER *pAd);
INT32 tr_ctl_exit(struct _RTMP_ADAPTER *pAd);
BOOLEAN tx_flow_check_state(struct _RTMP_ADAPTER *pAd, UINT8 State, UINT8 RingIdx);
INT32 tx_flow_block(struct _RTMP_ADAPTER *pAd, PNET_DEV NetDev, UINT8 State, BOOLEAN Block, UINT8 RingIdx);
INT32 tx_flow_set_state_block(struct _RTMP_ADAPTER *pAd, PNET_DEV NetDev, UINT8 State, BOOLEAN Block, UINT8 RingIdx);
BOOLEAN tx_flow_check_if_blocked(struct _RTMP_ADAPTER *pAd);
#endif
