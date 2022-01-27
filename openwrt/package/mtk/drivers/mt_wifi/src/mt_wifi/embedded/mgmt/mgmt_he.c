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

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/


#include "rt_config.h"


/*
 * Build up common HE IEs
 */
static UINT8 *build_he_mac_cap(struct wifi_dev *wdev, struct _RTMP_CHIP_CAP *chip_cap, UINT8 *f_buf)
{
	struct he_mac_capinfo he_mac_cap;
	UINT32 cap_1 = 0;
	UINT8 cap_2 = 0;
	UINT8 *pos = f_buf;

	cap_1 |= IEEE80211_HE_MAC_CAP_HTC;
	cap_1 |= ((chip_cap->ppdu.max_agg_tid_num - 1) << IEEE80211_HE_MAC_CAP_MULTI_TID_AGG_SHIFT);
	cap_1 |= IEEE80211_HE_MAC_CAP_ALL_ACK;
	cap_1 |= IEEE80211_HE_MAC_CAP_ACK_EN_MULTI_TID_AGG;

	cap_2 |= IEEE80211_HE_MAC_CAP_AMSDU_IN_AMPDU;
	cap_2 |= IEEE80211_HE_MAC_CAP_BQR;

	he_mac_cap.mac_capinfo_1 = cpu_to_le32(cap_1);
	he_mac_cap.mac_capinfo_2 = cap_2;
	NdisMoveMemory(f_buf, (UINT8 *)&he_mac_cap, sizeof(he_mac_cap));
	pos += sizeof(he_mac_cap);

	return pos;
}

static UINT8 *build_he_phy_cap(struct wifi_dev *wdev, struct _RTMP_CHIP_CAP *chip_cap, UINT8 *f_buf)
{
	struct he_phy_capinfo he_phy_cap = {0};
	UINT32 phy_cap_1 = 0;
	UINT32 phy_cap_2 = 0;
	UINT8 phy_cap_3 = 0;
	UINT32 phy_ch_width = 0;
	UINT8 *pos = f_buf;
	enum PHY_CAP phy_caps;
	UINT8 he_bw = wlan_config_get_he_bw(wdev);

	phy_caps = wlan_config_get_phy_caps(wdev);
	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_5G)) {
		phy_ch_width |= SUPP_40M_80M_CW_IN_5G_BAND;
		if (he_bw > BW_80) {
			phy_ch_width |= SUPP_160M_CW_IN_5G_BAND;
			if (he_bw > BW_160)
				phy_ch_width |= SUPP_160M_8080M_CW_IN_5G_BAND;
		}
		if (IS_PHY_CAPS(phy_caps, fPHY_CAP_BW20_242TONE))
			phy_ch_width |= SUPP_20MSTA_RX_242TONE_RU_IN_5G_BAND;
	} else {
		phy_ch_width |= SUPP_40M_CW_IN_24G_BAND;
		if (IS_PHY_CAPS(phy_caps, fPHY_CAP_BW20_242TONE))
			phy_ch_width |= SUPP_20MSTA_RX_242TONE_RU_IN_24G_BAND;
	}
	phy_cap_1 |= (phy_ch_width << IEEE80211_HE_PHY_CAP_CH_WIDTH_SET_SHIFT);

	if (wlan_config_get_he_ldpc(wdev))
		phy_cap_1 |= IEEE80211_HE_PHY_CAP_LDPC;

	phy_cap_1 |= IEEE80211_HE_PHY_CAP_SU_PPDU_1x_HE_LTF_DOT8US_GI;

	if (wlan_config_get_he_tx_stbc(wdev)) {
		phy_cap_1 |= IEEE80211_HE_PHY_CAP_TX_STBC_LE_EQ_80M;
		if (he_bw > BW_80)
			phy_cap_2 |= IEEE80211_HE_PHY_CAP_TX_STBC_GT_80M;
	}
	if (wlan_config_get_he_rx_stbc(wdev)) {
		phy_cap_1 |= IEEE80211_HE_PHY_CAP_RX_STBC_LE_EQ_80M;
		if (he_bw > BW_80)
			phy_cap_2 |= IEEE80211_HE_PHY_CAP_RX_STBC_GT_80M;
	}

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_TX_DOPPLER))
		phy_cap_1 |= IEEE80211_HE_PHY_CAP_TX_DOPPLER;
	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_RX_DOPPLER))
		phy_cap_1 |= IEEE80211_HE_PHY_CAP_RX_DOPPLER;

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_UL_MU))
		phy_cap_1 |= IEEE80211_HE_PHY_CAP_UL_HE_MU_PPDU;

	/* TODO: BF should be adding at here!
	 *if (IS_PHY_CAPS(phy_caps, fPHY_CAP_TXBF))
	 */

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_ER_SU))
		phy_cap_3 |= (IEEE80211_HE_PHY_CAP_ER_SU_PPDU_4x_HE_LTF_DOT8_US |
				IEEE80211_HE_PHY_CAP_ER_SU_PPDU_1x_HE_LTF_DOT8_US);

	/* Reserved for an AP, nonAP HE STA only */
	if (wdev->wdev_type == WDEV_TYPE_STA) {
		if (he_bw > BW_80)
			phy_cap_3 |= (IEEE80211_HE_PHY_CAP_20M_IN_160M_8080M_HE_PPDU |
					IEEE80211_HE_PHY_CAP_80M_IN_160M_8080M_HE_PPDU);
		else
			phy_cap_3 |= IEEE80211_HE_PHY_CAP_20M_IN_40M_HE_PPDU_24G;
	}

	he_phy_cap.phy_capinfo_1 = cpu_to_le32(phy_cap_1);
	he_phy_cap.phy_capinfo_2 = cpu_to_le32(phy_cap_2);
	he_phy_cap.phy_capinfo_3 = phy_cap_3;
	NdisMoveMemory(f_buf, (UINT8 *)&he_phy_cap, sizeof(he_phy_cap));
	pos += sizeof(he_phy_cap);

	return pos;
}

static UINT16 he_mcs_map(UINT8 nss, UINT8 he_mcs)
{
	UINT16 max_mcs_nss = 0xFFFF;

	max_mcs_nss &= ~(IEEE80211_HE_MCS_1SS_MASK);
	max_mcs_nss |= HE_MAX_MCS_NSS(1, he_mcs);
	if (nss > 2) {
		max_mcs_nss &= ~(IEEE80211_HE_MCS_3SS_MASK);
		max_mcs_nss |= HE_MAX_MCS_NSS(3, he_mcs);
	}
	if (nss > 3) {
		max_mcs_nss &= ~(IEEE80211_HE_MCS_4SS_MASK);
		max_mcs_nss |= HE_MAX_MCS_NSS(4, he_mcs);
	}
	if (nss > 4) {
		max_mcs_nss &= ~(IEEE80211_HE_MCS_5SS_MASK);
		max_mcs_nss |= HE_MAX_MCS_NSS(5, he_mcs);
	}
	if (nss > 5) {
		max_mcs_nss &= ~(IEEE80211_HE_MCS_6SS_MASK);
		max_mcs_nss |= HE_MAX_MCS_NSS(6, he_mcs);
	}
	if (nss > 6) {
		max_mcs_nss &= ~(IEEE80211_HE_MCS_7SS_MASK);
		max_mcs_nss |= HE_MAX_MCS_NSS(7, he_mcs);
	}
	if (nss > 7) {
		max_mcs_nss &= ~(IEEE80211_HE_MCS_8SS_MASK);
		max_mcs_nss |= HE_MAX_MCS_NSS(8, he_mcs);
	}

	return max_mcs_nss;
}

static UINT8 *build_he_mcs_nss(struct wifi_dev *wdev, struct _RTMP_CHIP_CAP *chip_cap, UINT8 *f_buf)
{
	UINT16 max_mcs_nss = 0;
	struct he_txrx_mcs_nss he_mcs_nss = {0};
	UINT8 he_bw = wlan_config_get_he_bw(wdev);
	UINT8 tx_nss = wlan_config_get_he_tx_nss(wdev);
	UINT8 rx_nss = wlan_config_get_he_rx_nss(wdev);
	UINT8 *pos = f_buf;

	/* Tx */
	max_mcs_nss = he_mcs_map(tx_nss, HE_MCS_0_11);
	he_mcs_nss.max_tx_mcs_nss = cpu_to_le16(max_mcs_nss);
	/* Rx */
	max_mcs_nss = he_mcs_map(rx_nss, HE_MCS_0_11);
	he_mcs_nss.max_rx_mcs_nss = cpu_to_le16(max_mcs_nss);
	NdisMoveMemory(pos, (UINT8 *)&he_mcs_nss, sizeof(he_mcs_nss));
	pos += sizeof(he_mcs_nss);

	if (he_bw > BW_80) {
		if (tx_nss > chip_cap->mcs_nss.bw160_max_nss)
			tx_nss = chip_cap->mcs_nss.bw160_max_nss;
		if (rx_nss > chip_cap->mcs_nss.bw160_max_nss)
			rx_nss = chip_cap->mcs_nss.bw160_max_nss;
		max_mcs_nss = he_mcs_map(tx_nss, HE_MCS_0_11);
		he_mcs_nss.max_tx_mcs_nss = cpu_to_le16(max_mcs_nss);
		max_mcs_nss = he_mcs_map(rx_nss, HE_MCS_0_11);
		he_mcs_nss.max_rx_mcs_nss = cpu_to_le16(max_mcs_nss);
		NdisMoveMemory(pos, (UINT8 *)&he_mcs_nss, sizeof(he_mcs_nss));
		pos += sizeof(he_mcs_nss);

		if (he_bw > BW_160) {
			max_mcs_nss = he_mcs_map(tx_nss, HE_MCS_0_11);
			he_mcs_nss.max_tx_mcs_nss = cpu_to_le16(max_mcs_nss);
			max_mcs_nss = he_mcs_map(rx_nss, HE_MCS_0_11);
			he_mcs_nss.max_rx_mcs_nss = cpu_to_le16(max_mcs_nss);
			NdisMoveMemory(pos, (UINT8 *)&he_mcs_nss, sizeof(he_mcs_nss));
			pos += sizeof(he_mcs_nss);
		}
	}

	return pos;
}

static UINT8 *build_he_cap_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	VOID *hdev_ctrl = hc_get_hdev_ctrl(wdev);
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(hdev_ctrl);

	/* HE MAC Capablities Information */
	pos = build_he_mac_cap(wdev, chip_cap, pos);
	/* HE PHY Capablities Information */
	pos = build_he_phy_cap(wdev, chip_cap, pos);
	/* Tx Rx HE-MCS NSS Support */
	pos = build_he_mcs_nss(wdev, chip_cap, pos);
	/* PPE Thresholds (optional) */

	return pos;
}

static UINT8 *build_he_op_params(struct wifi_dev *wdev, struct _RTMP_CHIP_CAP *chip_cap, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	UINT8 mbss = 0;
	UINT8 vhtop_present = wlan_config_get_he_vhtop_present(wdev);
	UINT16 txop_rts_thld = wlan_config_get_he_txop_rts_thld(wdev);
	UINT32 he_op_param = 0, tmp = 0;
	struct intra_bss_info intra_bss;

	intra_bss.info = wlan_operate_get_he_intra_bss_info(wdev);
	tmp |= GET_BSS_COLOR(intra_bss.info);
	/*TBD: Hanmin, TWT*/
	/*wlan_operate_get_he_twt_require*/
	tmp |= (txop_rts_thld << IEEE80211_HE_OP_RTS_THLD_SHIFT);

	if (vhtop_present)
		tmp |= IEEE80211_HE_OP_VHT_OP_INFO_PRESENT;
	if (mbss)
		tmp |= (IEEE80211_HE_OP_MULTI_BSSID_AP | IEEE80211_HE_OP_TX_BSSID_INDICATOR);
	if (IS_PARTIAL_BSS_COLOR(intra_bss.info))
		tmp |= IEEE80211_HE_OP_PARTIAL_BSS_COLOR;
	if (IS_BSS_COLOR_DIS(intra_bss.info))
		tmp |= IEEE80211_HE_OP_BSS_COLOR_DISABLE;

	he_op_param = cpu_to_le32(tmp);
	NdisMoveMemory(pos, (UINT8 *)&he_op_param, sizeof(he_op_param));
	pos += sizeof(he_op_param);

	return pos;
}

static UINT8 *build_basic_he_mcs_nss(struct wifi_dev *wdev, struct _RTMP_CHIP_CAP *chip_cap, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;

	pos = build_he_mcs_nss(wdev, chip_cap, pos);

	return pos;
}

static UINT8 *build_he_vht_op_info(struct wifi_dev *wdev, struct _RTMP_CHIP_CAP *chip_cap, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	UINT8 vhtop_present = wlan_config_get_he_vhtop_present(wdev);

	if (!vhtop_present)
		return pos;

	return pos;
}

static UINT8 *build_max_bssid_ind(struct wifi_dev *wdev, struct _RTMP_CHIP_CAP *chip_cap, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;

	return pos;
}

static UINT8 *build_he_op_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	VOID *hdev_ctrl = hc_get_hdev_ctrl(wdev);
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(hdev_ctrl);

	/* HE Operation Parameters */
	pos = build_he_op_params(wdev, chip_cap, pos);
	/* Basic HE-MCS And Nss Set */
	pos = build_basic_he_mcs_nss(wdev, chip_cap, pos);
	/* VHT Operation Information */
	pos = build_he_vht_op_info(wdev, chip_cap, pos);
	/* MaxBSSID Indicator */
	pos = build_max_bssid_ind(wdev, chip_cap, pos);

	return pos;
}

static UINT8 *build_he_mu_edca_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;

	return pos;
}

static UINT8 *build_bss_color_change_announce_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;

	return pos;
}

static UINT8 *build_spatial_reuse_param_set_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;

	return pos;
}

static UINT8 *build_he_twt_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;

	return pos;
}

/*
 * Build up HE IEs for AP
 */
UINT32 add_beacon_he_ies(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *local_fbuf = f_buf;
	UINT32 offset = 0;

	local_fbuf = build_he_cap_ie(wdev, local_fbuf);
	local_fbuf = build_he_op_ie(wdev, local_fbuf);
	local_fbuf = build_he_twt_ie(wdev, local_fbuf);
	local_fbuf = build_bss_color_change_announce_ie(wdev, local_fbuf);
	local_fbuf = build_spatial_reuse_param_set_ie(wdev, local_fbuf);
	local_fbuf = build_he_mu_edca_ie(wdev, local_fbuf);
	offset = (UINT32)(local_fbuf - f_buf);

	return offset;
}

UINT32 add_probe_rsp_he_ies(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *local_fbuf = f_buf;
	UINT32 offset = 0;

	local_fbuf = build_he_cap_ie(wdev, local_fbuf);
	local_fbuf = build_he_op_ie(wdev, local_fbuf);
	local_fbuf = build_he_twt_ie(wdev, local_fbuf);
	local_fbuf = build_bss_color_change_announce_ie(wdev, local_fbuf);
	local_fbuf = build_spatial_reuse_param_set_ie(wdev, local_fbuf);
	local_fbuf = build_he_mu_edca_ie(wdev, local_fbuf);
	offset = (UINT32)(local_fbuf - f_buf);

	return offset;
}

UINT32 add_assoc_reassoc_rsp_twt_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *local_fbuf = f_buf;
	UINT32 offset = 0;

	local_fbuf = build_he_twt_ie(wdev, local_fbuf); /* order 42 */
	offset = (UINT32)(local_fbuf - f_buf);

	return offset;
}

UINT32 add_assoc_rsp_he_ies(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *local_fbuf = f_buf;
	UINT32 offset = 0;

	local_fbuf = build_he_cap_ie(wdev, local_fbuf);
	local_fbuf = build_he_op_ie(wdev, local_fbuf);
	local_fbuf = build_bss_color_change_announce_ie(wdev, local_fbuf);
	local_fbuf = build_spatial_reuse_param_set_ie(wdev, local_fbuf);
	local_fbuf = build_he_mu_edca_ie(wdev, local_fbuf);
	offset = (UINT32)(local_fbuf - f_buf);

	return offset;
}

UINT32 add_reassoc_rsp_he_ies(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *local_fbuf = f_buf;
	UINT32 offset = 0;

	local_fbuf = build_he_cap_ie(wdev, local_fbuf);
	local_fbuf = build_he_op_ie(wdev, local_fbuf);
	local_fbuf = build_bss_color_change_announce_ie(wdev, local_fbuf);
	local_fbuf = build_spatial_reuse_param_set_ie(wdev, local_fbuf);
	local_fbuf = build_he_mu_edca_ie(wdev, local_fbuf);
	offset = (UINT32)(local_fbuf - f_buf);

	return offset;
}

/*
 * Build up IEs for STA
 */
UINT32 add_probe_req_he_ies(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *local_fbuf = f_buf;
	UINT32 offset = 0;

	local_fbuf = build_he_cap_ie(wdev, local_fbuf);
	offset = (UINT32)(local_fbuf - f_buf);

	return offset;
}

UINT32 add_assoc_req_he_ies(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *local_fbuf = f_buf;
	UINT32 offset = 0;

	local_fbuf = build_he_cap_ie(wdev, local_fbuf);
	offset = (UINT32)(local_fbuf - f_buf);

	return offset;
}

UINT32 add_reassoc_req_he_ies(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *local_fbuf = f_buf;
	UINT32 offset = 0;

	local_fbuf = build_he_cap_ie(wdev, local_fbuf);
	offset = (UINT32)(local_fbuf - f_buf);

	return offset;
}
