/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2019, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/


#ifndef _DOT11AX_HE_H_
#define _DOT11AX_HE_H_

#include "rtmp_type.h"
#include "dot11_base.h"

/*
 * HE Capabilities element
 */
/* HE MAC Capabilities Information field */
/* Fragmentation Support subfield */
enum {
	DYN_FRAG_NOT_SUPP,
	DYN_FRAG_LVL_1,
	DYN_FRAG_LVL_2,
	DYN_FRAG_LVL_3
};

/* Minimum Fragment Size subfield */
enum {
	MIN_FRAG_SZ_NO_RESTRICT,
	MIN_FRAG_SZ_128,
	MIN_FRAG_SZ_256,
	MIN_FRAG_SZ_512
};

/* Trigger Frame MAC Padding Duration subfield */
enum {
	NO_PADDING,
	PADDING_8US,
	PADDING_16US
};

/* HE Link Adaptation Capable subfield */
enum {
	NOT_SUPPORT = 0,
	NO_FEEDBACK = NOT_SUPPORT,
	UNSOLICITED,
	BOTH
};

/* mac_capinfo_1: bit0..31 */
#define IEEE80211_HE_MAC_CAP_HTC 1
#define IEEE80211_HE_MAC_CAP_TWT_REQ (1 << 1)
#define IEEE80211_HE_MAC_CAP_TWT_RSP (1 << 2)
#define IEEE80211_HE_MAC_CAP_FRAG_LVL_SHIFT 3
#define IEEE80211_HE_MAC_CAP_FRAG_LVL_MASK (3 << 3)
#define IEEE80211_HE_MAC_CAP_MAX_FRAG_MSDU_NUM_SHIFT 5
#define IEEE80211_HE_MAC_CAP_MAX_FRAG_MSDU_NUM_MASK (7 << 5)
#define IEEE80211_HE_MAC_CAP_MIN_FRAG_SIZE_SHIFT 8
#define IEEE80211_HE_MAC_CAP_MIN_FRAG_SIZE_MASK (3 << 8)
#define IEEE80211_HE_MAC_CAP_TRI_MAC_PAD_SHIFT 10
#define IEEE80211_HE_MAC_CAP_TRI_MAC_PAD_MASK (3 << 10)
#define IEEE80211_HE_MAC_CAP_MULTI_TID_AGG_SHIFT 12
#define IEEE80211_HE_MAC_CAP_MULTI_TID_AGG_MASK (7 << 12)
#define IEEE80211_HE_MAC_CAP_LINK_ADAPT_SHIFT 15
#define IEEE80211_HE_MAC_CAP_LINK_ADAPT_MASK (3 << 15)
#define IEEE80211_HE_MAC_CAP_ALL_ACK (1 << 17)
#define IEEE80211_HE_MAC_CAP_UMRS (1 << 18)
#define IEEE80211_HE_MAC_CAP_BSR (1 << 19)
#define IEEE80211_HE_MAC_CAP_BROADCAST_TWT (1 << 20)
#define IEEE80211_HE_MAC_CAP_32BA_BITMAP (1 << 21)
#define IEEE80211_HE_MAC_CAP_MU_CASCADE (1 << 22)
#define IEEE80211_HE_MAC_CAP_ACK_EN_MULTI_TID_AGG (1 << 23)
#define IEEE80211_HE_MAC_CAP_DL_MU_GRPADDR_MSTA_BA (1 << 24)
#define IEEE80211_HE_MAC_CAP_OM_CTRL (1 << 25)
#define IEEE80211_HE_MAC_CAP_OFDMA_RA (1 << 26)
#define IEEE80211_HE_MAC_CAP_MAX_AMPDU_LEN_EXP_SHIFT 27
#define IEEE80211_HE_MAC_CAP_MAX_AMPDU_LEN_EXP_MASK (3 << 27)
#define IEEE80211_HE_MAC_CAP_AMSDU_FRAG (1 << 29)
#define IEEE80211_HE_MAC_CAP_FLEX_TWT_SCH (1 << 30)
#define IEEE80211_HE_MAC_CAP_RX_CTRL_FRAME_2_MULTI_BSS (1 << 31)
/* mac_capinfo_2: bit32..39 */
#define IEEE80211_HE_MAC_CAP_BSRP_AMPDU_AGG 1
#define IEEE80211_HE_MAC_CAP_QTP (1 << 1)
#define IEEE80211_HE_MAC_CAP_BQR (1 << 2)
#define IEEE80211_HE_MAC_CAP_SR_RSP (1 << 3)
#define IEEE80211_HE_MAC_CAP_NDP_FEEDBACK_REPORT (1 << 4)
#define IEEE80211_HE_MAC_CAP_OPS (1 << 5)
#define IEEE80211_HE_MAC_CAP_AMSDU_IN_AMPDU (1 << 6)

struct GNU_PACKED he_mac_capinfo {
	UINT32 mac_capinfo_1;
	UINT8 mac_capinfo_2;
};

/* HE PHY Capabilities Information field */
/* Channel Width Set subfield */
enum {
	SUPP_40M_CW_IN_24G_BAND = 1,
	SUPP_40M_80M_CW_IN_5G_BAND = (1 << 1),
	SUPP_160M_CW_IN_5G_BAND = (1 << 2),
	SUPP_160M_8080M_CW_IN_5G_BAND = (1 << 3),
	SUPP_20MSTA_RX_242TONE_RU_IN_24G_BAND = (1 << 4),
	SUPP_20MSTA_RX_242TONE_RU_IN_5G_BAND = (1 << 5)
};

/* Punctured Preamble Rx subfield */
enum {
	RX_80M_PREAMBLE_SEC_20M_PUNC = 1,
	RX_80M_PREAMBLE_SEC_40M_PUNC = (1 << 1),
	RX_PRIM_80M_PREAMBLE_SEC_20M_PUNC = (1 << 2),
	RX_PRIM_80M_PREAMBLE_PRIM_40M_PUNC = (1 << 3)
};

/* DCM Max Constellation Tx subfield */
enum {
	DCM_NOT_SUPPORT,
	DCM_BPSK,
	DCM_QPSK,
	DCM_16QAM
};

/* phy_capinfo_1: bit0..31 */
#define IEEE80211_HE_PHY_CAP_DUAL_BAND 1
#define IEEE80211_HE_PHY_CAP_CH_WIDTH_SET_SHIFT 1
#define IEEE80211_HE_PHY_CAP_CH_WIDTH_SET_MASK (0x7F << 1)
#define IEEE80211_HE_PHY_CAP_PUNC_PREAMBLE_RX_SHIFT 8
#define IEEE80211_HE_PHY_CAP_PUNC_PREAMBLE_RX_MASK (0xF << 8)
#define IEEE80211_HE_PHY_CAP_DEVICE_CLASS (1 << 12)
#define IEEE80211_HE_PHY_CAP_LDPC (1 << 13)
#define IEEE80211_HE_PHY_CAP_SU_PPDU_1x_HE_LTF_DOT8US_GI (1 << 14)
#define IEEE80211_HE_PHY_CAP_NDP_4x_HE_LTF_3DOT2US_GI (1 << 17)
#define IEEE80211_HE_PHY_CAP_TX_STBC_LE_EQ_80M (1 << 18)
#define IEEE80211_HE_PHY_CAP_RX_STBC_LE_EQ_80M (1 << 19)
#define IEEE80211_HE_PHY_CAP_TX_DOPPLER (1 << 20)
#define IEEE80211_HE_PHY_CAP_RX_DOPPLER (1 << 21)
#define IEEE80211_HE_PHY_CAP_FULL_BW_UL_MU_MIMO (1 << 22)
#define IEEE80211_HE_PHY_CAP_PARTIAL_BW_UL_MU_MIMO (1 << 23)
#define IEEE80211_HE_PHY_CAP_TX_DCM_MAX_CONSTELLATION_SHIFT 24
#define IEEE80211_HE_PHY_CAP_TX_DCM_MAX_CONSTELLATION_MASK (0x3 << 24)
#define IEEE80211_HE_PHY_CAP_TX_DCM_MAX_NSS (1 << 26)
#define IEEE80211_HE_PHY_CAP_RX_DCM_MAX_CONSTELLATION_SHIFT 27
#define IEEE80211_HE_PHY_CAP_RX_DCM_MAX_CONSTELLATION_MASK (0x3 << 27)
#define IEEE80211_HE_PHY_CAP_RX_DCM_MAX_NSS (1 << 29)
#define IEEE80211_HE_PHY_CAP_UL_HE_MU_PPDU (1 << 30)
#define IEEE80211_HE_PHY_CAP_SU_BFER (1 << 31)

/* phy_capinfo_2: bit32..bit63 */
#define IEEE80211_HE_PHY_CAP_SU_BFEE 1
#define IEEE80211_HE_PHY_CAP_MU_BFEE (1 << 1)
#define IEEE80211_HE_PHY_CAP_BFEE_STS_LE_EQ_80M_SHIFT 2
#define IEEE80211_HE_PHY_CAP_BFEE_STS_LE_EQ_80M_MASK (0x7 << 2)
#define IEEE80211_HE_PHY_CAP_BFEE_STS_GT_80M_SHIFT 5
#define IEEE80211_HE_PHY_CAP_BFEE_STS_GT_80M_MASK (0x7 << 5)
#define IEEE80211_HE_PHY_CAP_SOUND_DIM_NUM_LE_EQ_80M_SHIFT 8
#define IEEE80211_HE_PHY_CAP_SOUND_DIM_NUM_LE_EQ_80M_MASK (0x7 << 8)
#define IEEE80211_HE_PHY_CAP_SOUND_DIM_NUM_GT_80M_SHIFT 11
#define IEEE80211_HE_PHY_CAP_SOUND_DIM_NUM_GT_80M_MASK (0x7 << 11)
#define IEEE80211_HE_PHY_CAP_NG16_SU_FEEDBACK (1 << 14)
#define IEEE80211_HE_PHY_CAP_NG16_MU_FEEDBACK (1 << 15)
#define IEEE80211_HE_PHY_CAP_CODEBOOK_SU_FEEDBACK (1 << 16)
#define IEEE80211_HE_PHY_CAP_CODEBOOK_MU_FEEDBACK (1 << 17)
#define IEEE80211_HE_PHY_CAP_TRIG_SU_BF_FEEDBACK (1 << 18)
#define IEEE80211_HE_PHY_CAP_TRIG_MU_BF_FEEDBACK (1 << 19)
#define IEEE80211_HE_PHY_CAP_TRIG_CQI_FEEDBACK (1 << 20)
#define IEEE80211_HE_PHY_CAP_PARTIAL_BW_ER (1 << 21)
#define IEEE80211_HE_PHY_CAP_PARTIAL_BW_DL_MU_MIMO (1 << 22)
#define IEEE80211_HE_PHY_CAP_PPE_THRLD_PRESENT (1 << 23)
#define IEEE80211_HE_PHY_CAP_SRP_BASE_SR (1 << 24)
#define IEEE80211_HE_PHY_CAP_PWR_BOOST_FACTOR (1 << 25)
#define IEEE80211_HE_PHY_CAP_SU_MU_PPDU_4x_HE_LTF_DOT8_US (1 << 26)
#define IEEE80211_HE_PHY_CAP_MAX_NC_SHIFT 27
#define IEEE80211_HE_PHY_CAP_MAX_NC_MASK (0x7 << 27)
#define IEEE80211_HE_PHY_CAP_TX_STBC_GT_80M (1 << 30)
#define IEEE80211_HE_PHY_CAP_RX_STBC_GT_80M (1 << 31)

/* phy_capinfo_3: bit64..bit71 */
#define IEEE80211_HE_PHY_CAP_ER_SU_PPDU_4x_HE_LTF_DOT8_US 1
#define IEEE80211_HE_PHY_CAP_20M_IN_40M_HE_PPDU_24G (1 << 1)
#define IEEE80211_HE_PHY_CAP_20M_IN_160M_8080M_HE_PPDU (1 << 2)
#define IEEE80211_HE_PHY_CAP_80M_IN_160M_8080M_HE_PPDU (1 << 3)
#define IEEE80211_HE_PHY_CAP_ER_SU_PPDU_1x_HE_LTF_DOT8_US (1 << 4)
#define IEEE80211_HE_PHY_CAP_MIDAMBLE_RX_2X_1X_HE_LTF (1 << 5)

struct GNU_PACKED he_phy_capinfo {
	UINT32 phy_capinfo_1;
	UINT32 phy_capinfo_2;
	UINT8 phy_capinfo_3;
};

/* Supported HE-MCS and Nss Set field */

/* Highest MCS Supported subfield */
enum {
	HE_MCS_0_7,
	HE_MCS_0_9,
	HE_MCS_0_11,
	HE_MCS_NOT_SUPPORT
};

/* max_mcs_nss */
#define HE_MCS_NSS_MASK 0x3
#define IEEE80211_HE_MCS_1SS_SHIFT 0
#define IEEE80211_HE_MCS_1SS_MASK HE_MCS_NSS_MASK
#define IEEE80211_HE_MCS_2SS_SHIFT 2
#define IEEE80211_HE_MCS_2SS_MASK (HE_MCS_NSS_MASK << 2)
#define IEEE80211_HE_MCS_3SS_SHIFT 4
#define IEEE80211_HE_MCS_3SS_MASK (HE_MCS_NSS_MASK << 4)
#define IEEE80211_HE_MCS_4SS_SHIFT 6
#define IEEE80211_HE_MCS_4SS_MASK (HE_MCS_NSS_MASK << 6)
#define IEEE80211_HE_MCS_5SS_SHIFT 8
#define IEEE80211_HE_MCS_5SS_MASK (HE_MCS_NSS_MASK << 8)
#define IEEE80211_HE_MCS_6SS_SHIFT 10
#define IEEE80211_HE_MCS_6SS_MASK (HE_MCS_NSS_MASK << 10)
#define IEEE80211_HE_MCS_7SS_SHIFT 12
#define IEEE80211_HE_MCS_7SS_MASK (HE_MCS_NSS_MASK << 12)
#define IEEE80211_HE_MCS_8SS_SHIFT 14
#define IEEE80211_HE_MCS_8SS_MASK (HE_MCS_NSS_MASK << 14)

#define HE_MAX_MCS_NSS(nss, mcs)\
	((mcs) << (IEEE80211_HE_MCS_ ## nss ## SS_SHIFT))

struct GNU_PACKED he_txrx_mcs_nss {
	UINT16 max_tx_mcs_nss;
	UINT16 max_rx_mcs_nss;
};

struct GNU_PACKED he_basic_mcs_nss {
	UINT16 max_mcs_nss;
};

/* PPE Threshold field */
#define IEEE80211_HE_PPE_THLD_NSS_M1_MASK 7
#define IEEE80211_HE_PPE_THLD_RU_IDX_BITMASK_SHIFT 3
#define IEEE80211_HE_PPE_THLD_RU_IDX_BITMASK_MASK (0xF << 3)

struct GNU_PACKED he_cap_ie {
	struct he_mac_capinfo mac_cap;
	struct he_phy_capinfo phy_cap;
	struct he_txrx_mcs_nss txrx_mcs_nss;
	UINT8 ppe_thld[0];
};

/*
 * HE Operation Element
 */
#define INTRA_HE_BSS_COLOR_MASK 0x3F
#define INTRA_HE_BSS_COLOR_DIS (1 << 6)
#define INTRA_HE_PARTIAL_BSS_COLOR (1 << 7)
struct GNU_PACKED intra_bss_info {
	UINT8 info;
};
#define GET_BSS_COLOR(info)\
	((info) & INTRA_HE_BSS_COLOR_MASK)
#define IS_BSS_COLOR_DIS(info)\
	(((info) & INTRA_HE_BSS_COLOR_DIS) != 0)
#define IS_PARTIAL_BSS_COLOR(info)\
	(((info) & INTRA_HE_PARTIAL_BSS_COLOR) != 0)

/* he_op_params */
#define IEEE80211_HE_OP_BSS_COLOR_MASK 0x3F
#define IEEE80211_HE_OP_DEFAULT_PE_DURATION_SHIFT 6
#define IEEE80211_HE_OP_DEFAULT_PE_DURATION_MASK (0x7 << 6)
#define IEEE80211_HE_OP_TWT_REQUIRED (1 << 9)
/* unit:32us, val:0~1023 */
#define IEEE80211_HE_OP_RTS_THLD_SHIFT 10
#define IEEE80211_HE_OP_RTS_THLD_MASK (0x3FF << 10)
#define IEEE80211_HE_OP_PARTIAL_BSS_COLOR (1 << 20)
#define IEEE80211_HE_OP_VHT_OP_INFO_PRESENT (1 << 21)
#define IEEE80211_HE_OP_MULTI_BSSID_AP (1 << 28)
#define IEEE80211_HE_OP_TX_BSSID_INDICATOR (1 << 29)
#define IEEE80211_HE_OP_BSS_COLOR_DISABLE (1 << 30)

struct GNU_PACKED he_op_ie {
	UINT32 he_op_params;
	struct he_basic_mcs_nss he_mcs_nss;
	UINT8 opt_field[0];
};

/*
 * MU EDCA Parameter Set Element
 */
struct GNU_PACKED mu_edca_param_record {
	UINT8 aci_aifsn;
	UINT8 ecwmax_cwmin;
	UINT8 mu_edca_timer;
};

/* QoS Info field for AP */
#define IEEE80211_AP_QOS_INFO_EDCA_UPDATE_CNT_MASK 0xF
#define IEEE80211_AP_QOS_INFO_QACK (1 << 4)
#define IEEE80211_AP_QOS_INFO_QUE_REQ (1 << 5)
#define IEEE80211_AP_QOS_INFO_TXOP_REQ (1 << 6)
/* QoS Info field for non-AP STA */
#define IEEE80211_NON_AP_QOS_INFO_VO_UAPSD 1
#define IEEE80211_NON_AP_QOS_INFO_VI_UAPSD (1 << 1)
#define IEEE80211_NON_AP_QOS_INFO_BK_UAPSD (1 << 2)
#define IEEE80211_NON_AP_QOS_INFO_BE_UAPSD (1 << 3)
#define IEEE80211_NON_AP_QOS_INFO_QACK (1 << 4)
#define IEEE80211_NON_AP_QOS_INFO_QUE_REQ_SHIFT 5
#define IEEE80211_NON_AP_QOS_INFO_QUE_REQ_MASK (0x3 << 5)
#define IEEE80211_NON_AP_QOS_INFO_MORE_DATA_ACK (1 << 7)

struct GNU_PACKED mu_edca_params {
	UINT8 mu_qos_info;
	struct mu_edca_param_record ac_be;
	struct mu_edca_param_record ac_bk;
	struct mu_edca_param_record ac_vi;
	struct mu_edca_param_record ac_vo;
};

/*
 * BSS Color Change Announcement Element
 */
#define IEEE80211_HE_NEW_BSS_COLOR_MASK 0x3F
struct GNU_PACKED bss_color_change {
	UINT8 color_switch_cnt_down;
	UINT8 new_bss_color_info;
};

/*
 * OFDMA-based Random Access Parameter Set (RAPS) Element
 */
#define IEEE80211_HE_RAPS_OCW_EOCWMIN_MASK 0x7
#define IEEE80211_HE_RAPS_OCW_EOCWMAX_SHIFT 3
#define IEEE80211_HE_RAPS_OCW_EOCWMAX_MASK (0x7 << 3)
struct GNU_PACKED raps {
	UINT8 ocw_range;
};

#endif /* _DOT11AX_HE_H_ */
