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
#include "wlan_config/config_internal.h"

/*
 * LOADER function
 */

/*
 * SET function
 */
VOID wlan_config_set_he_bw(struct wifi_dev *wdev, UINT8 he_bw)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->he_conf.bw = he_bw;
}

VOID wlan_config_set_he_vhtop_present(struct wifi_dev *wdev, UINT8 vhtop_en)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->he_conf.he_vhtop = vhtop_en;
}

/*
 * GET function
 */
UINT8 wlan_config_get_he_bw(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	UINT8 he_bw = BW_80;
	UINT8 ht_bw = wlan_config_get_ht_bw(wdev);

	if (cfg->he_conf.bw == HE_BW_2040) {
		if (ht_bw == HT_BW_20)
			he_bw = BW_20;
		else
			he_bw = BW_40;
	} else if (cfg->he_conf.bw == HE_BW_80) {
		he_bw = BW_80;
	} else if (cfg->he_conf.bw == HE_BW_160) {
		he_bw = BW_160;
	} else if (cfg->he_conf.bw == HE_BW_8080) {
		he_bw = BW_8080;
	} else {
		he_bw = BW_80;
	}

	return he_bw;
}

UINT8 wlan_config_get_he_vhtop_present(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	UINT8 vhtop_present;

	vhtop_present = cfg->he_conf.he_vhtop;

	return vhtop_present;
}

UINT8 wlan_config_get_he_tx_stbc(struct wifi_dev *wdev)
{
	UINT8 tx_stbc;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	tx_stbc = cfg->he_conf.tx_stbc;

	return tx_stbc;
}

UINT8 wlan_config_get_he_rx_stbc(struct wifi_dev *wdev)
{
	UINT8 rx_stbc;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	rx_stbc = cfg->he_conf.rx_stbc;

	return rx_stbc;
}

UINT8 wlan_config_get_he_ldpc(struct wifi_dev *wdev)
{
	UINT8 ldpc;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	ldpc = cfg->he_conf.ldpc;

	return ldpc;
}

UINT8 wlan_config_get_he_tx_nss(struct wifi_dev *wdev)
{
	UINT8 tx_nss;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	tx_nss = cfg->he_conf.tx_nss;

	return tx_nss;
}

UINT8 wlan_config_get_he_rx_nss(struct wifi_dev *wdev)
{
	UINT8 rx_nss;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	rx_nss = cfg->he_conf.tx_nss;

	return rx_nss;
}

UINT8 wlan_config_get_he_txop_rts_thld(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	UINT8 txop_duration = 1;

	/* unit 32us */
	txop_duration = cfg->he_conf.txop_duration;

	return txop_duration;
}
