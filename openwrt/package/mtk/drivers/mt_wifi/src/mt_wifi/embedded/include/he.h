/*
 ***************************************************************************
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
#ifndef _HE_H_
#define _HE_H_

#include "dot11ax_he.h"

enum {
	HE_BW_2040,
	HE_BW_80,
	HE_BW_160,
	HE_BW_8080
};

enum frag_num {
	FRAG_NUM_0,
	FRAG_NUM_1,
	FRAG_NUM_2,
	FRAG_NUM_3,
	FRAG_NUM_4,
	FRAG_NUM_5,
	FRAG_NUM_6,
	FRAG_NUM_NO_RESTRICT
};

enum he_mac_caps {
	HE_LDPC = 1,
	HE_TWT_REQUEST = (1 << 1),
	HE_TWT_RESPOND = (1 << 2),
	HE_ALL_ACK = (1 << 3),
	HE_UMRS = (1 << 4),
	HE_BSR = (1 << 5),
	HE_BROADCAST_TWT = (1 << 6),
	HE_32BIT_BA_BITMAP = (1 << 7),
	HE_MU_CASCADING = (1 << 8),
	HE_ACK_EN_AGG = (1 << 9),
	HE_GRP_ADDR_MULTI_STA_BA_DL_MU = (1 << 10),
	HE_OM_CTRL = (1 << 11),
	HE_OFDMA_RA = (1 << 12),
	HE_AMSDU_FRAG = (1 << 13),
	HE_FLEX_TWT_SCH = (1 << 14),
	HE_RX_CTRL_FRAME_TO_MULTIBSS = (1 << 15),
	HE_BSRP_BQRP_AMPDU_AFF = (1 << 16),
	HE_QTP = (1 << 17),
	HE_BQR = (1 << 18),
	HE_SR_RESPONDER = (1 << 19),
	HE_NDP_FEEDBACK_REPORT = (1 << 20),
	HE_OPS = (1 << 21),
	HE_AMSDU_IN_AMPDU = (1 << 22)
};

enum he_bf_caps {
	HE_SU_BFER = 1,
	HE_SU_BFEE = (1 << 1),
	HE_MU_BFER = (1 << 2),
	HE_TRIG_SU_BFEE_FEEDBACK = (1 << 3),
	HE_TRIG_MU_BFEE_FEEDBACK = (1 << 4)
};

enum he_gi_caps {
	HE_SU_PPDU_1x_LTF_DOT8US_GI = 1,
	HE_SU_PPDU_MU_PPDU_4x_LTF_DOT8US_GI = (1 << 1),
	HE_ER_SU_PPDU_1x_LTF_DOT8US_GI = (1 << 2),
	HE_ER_SU_PPDU_4x_LTF_DOT8US_GI = (1 << 3),
	HE_NDP_4x_LTF_3DOT2MS_GI = (1 << 4)
};

enum he_phy_caps {
	HE_DUAL_BAND = 1,
	HE_DEV_CLASS = (1 << 1),
	HE_PLDC = (1 << 2),
	HE_LE_EQ_80M_TX_STBC = (1 << 3),
	HE_LE_EQ_80M_RX_STBC = (1 << 4),
	HE_DOPPLER_TX = (1 << 5),
	HE_DOPPLER_Rl = (1 << 6),
	HE_FULL_BW_UL_MU_MIMO = (1 << 7),
	HE_PARTIAL_BW_UL_MU_MIMO = (1 << 8),
	HE_DCM_MAX_NSS_TX = (1 << 9),
	HE_DCM_MAX_NSS_RX = (1 << 10),
	HE_RX_MU_PPDU_FROM_STA = (1 << 11),
	HE_NG_16_SU_FEEDBACK = (1 << 12),
	HE_NG_16_MU_FEEDBACK = (1 << 13),
	HE_CODEBOOK_SU_FEEDBACK = (1 << 14),
	HE_CODEBOOK_MU_FEEDBACK = (1 << 15),
	HE_TRIG_CQI_FEEDBACK = (1 << 16),
	HE_PARTIAL_BW_DL_MU_MIMO = (1 << 17),
	HE_PPE_THRESHOLD_PRESENT = (1 << 18),
	HE_SRP_BASED_SR = (1 << 19),
	HE_PWR_BOOST_FACTOR = (1 << 20),
	HE_GT_80M_TX_STBC = (1 << 21),
	HE_GT_80M_RX_STBC = (1 << 22),
	HE_24G_20M_IN_40M_PPDU = (1 << 23),
	HE_20M_IN_160M_8080M_PPDU = (1 << 24),
	HE_80M_IN_160M_8080M_PPDU = (1 << 25),
	HE_MID_RX_2x_AND_1x_LTF = (1 << 26)
};

struct he_bf_info {
	enum he_bf_caps bf_cap;
	UINT8 bfee_sts_le_eq_bw80;
	UINT8 bfee_sts_gt_bw80;
	UINT8 num_snd_dim_le_eq_bw80;
	UINT8 num_snd_dim_gt_bw80;
};

UINT32 add_beacon_he_ies(struct wifi_dev *wdev, UINT8 *f_buf);
UINT32 add_probe_rsp_he_ies(struct wifi_dev *wdev, UINT8 *f_buf);
UINT32 add_assoc_reassoc_rsp_twt_ie(struct wifi_dev *wdev, UINT8 *f_buf);
UINT32 add_assoc_rsp_he_ies(struct wifi_dev *wdev, UINT8 *f_buf);
UINT32 add_reassoc_rsp_he_ies(struct wifi_dev *wdev, UINT8 *f_buf);
UINT32 add_probe_req_he_ies(struct wifi_dev *wdev, UINT8 *f_buf);
UINT32 add_assoc_req_he_ies(struct wifi_dev *wdev, UINT8 *f_buf);
UINT32 add_reassoc_req_he_ies(struct wifi_dev *wdev, UINT8 *f_buf);

#endif /*_HE_H_*/

