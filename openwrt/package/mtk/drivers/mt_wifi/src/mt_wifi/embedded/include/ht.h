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

#ifndef __HT_H__
#define __HT_H__
#include "dot11n_ht.h"

struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _ADD_HT_INFO_IE;
struct _build_ie_info;

enum ht_caps {
	HT_LDPC = 1,
	HT_CH_WIDTH_20 = (1 << 1),
	HT_GF = (1 << 2),
	HT_BW20_SGI = (1 << 3),
	HT_BW40_SGI = (1 << 4),
	HT_TX_STBC = (1 << 5),
	HT_DELAY_BA = (1 << 6),
	HT_BW40_DSSS_CCK = (1 << 7),
	HT_BW40_INTOLERANT = (1 << 8),
	HT_LSIG_TXOP = (1 << 9),
	HT_EXT_PCO = (1 << 10),
	HT_EXT_HTC = (1 << 11),
	HT_EXT_RD_RESPONDER = (1 << 12)
};

#define IS_HT_STA(_pMacEntry)	\
	(_pMacEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX)

#define IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#define PEER_IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

VOID set_sta_ht_cap(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *ent, HT_CAPABILITY_IE *ht_ie);

VOID RTMPSetHT(struct _RTMP_ADAPTER *pAd, OID_SET_HT_PHYMODE *pHTPhyMode, struct wifi_dev *wdev);
VOID RTMPSetIndividualHT(struct _RTMP_ADAPTER *pAd, UCHAR apidx);
UINT32 starec_ht_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature);

UCHAR get_cent_ch_by_htinfo(
	struct _RTMP_ADAPTER *pAd,
	struct _ADD_HT_INFO_IE *ht_op,
	HT_CAPABILITY_IE *ht_cap);

INT get_ht_cent_ch(struct _RTMP_ADAPTER *pAd, UINT8 *rf_bw, UINT8 *ext_ch, UCHAR Channel);
INT ht_mode_adjust(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry, HT_CAPABILITY_IE *peer_ht_cap);
UINT8 get_max_nss_by_htcap_ie_mcs(UCHAR *cap_mcs);
INT set_ht_fixed_mcs(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry, UCHAR fixed_mcs, UCHAR mcs_bound);
INT get_ht_max_mcs(struct _RTMP_ADAPTER *pAd, UCHAR *desire_mcs, UCHAR *cap_mcs);
UCHAR cal_ht_cent_ch(UCHAR prim_ch, UCHAR phy_bw, UCHAR ext_cha, UCHAR *cen_ch);
INT build_ht_ies(struct _RTMP_ADAPTER *pAd, struct _build_ie_info *info);

#define MAKE_IE_TO_BUF(__BUF, __CONTENT, __CONTENT_LEN, __CUR_LEN) \
{																   \
	NdisMoveMemory((__BUF+__CUR_LEN), (UCHAR *)(__CONTENT), __CONTENT_LEN);    \
	__CUR_LEN += __CONTENT_LEN;									   \
}
#endif /*__HT_H__*/
