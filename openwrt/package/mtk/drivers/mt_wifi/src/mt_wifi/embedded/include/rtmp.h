/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004,2018 Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtmp.h

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who		 When		  What
	--------	----------	----------------------------------------------

*/
#ifndef __RTMP_H__
#define __RTMP_H__
#include "common/link_list.h"
#include "common/module.h"
#include "spectrum_def.h"

#include "rtmp_dot11.h"

#include "security/sec_cmm.h"

#ifdef DOT11_SAE_SUPPORT
#include "security/sae_cmm.h"
#include "security/crypt_biginteger.h"
#endif /* DOT11_SAE_SUPPORT */

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
#include "icap.h"
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#ifdef CONFIG_AP_SUPPORT
#include "ap_autoChSel_cmm.h"
#endif /* CONFIG_AP_SUPPORT */

#include "wsc.h"
#ifdef MAT_SUPPORT
#include "mat.h"
#endif /* MAT_SUPPORT */



#include "rtmp_chip.h"

#ifdef DOT11R_FT_SUPPORT
#include "ft_cmm.h"
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
#include "rrm_cmm.h"
#endif /* DOT11K_RRM_SUPPORT */


#ifdef CLIENT_WDS
#include "client_wds_cmm.h"
#endif /* CLIENT_WDS */

#ifdef DOT11V_WNM_SUPPORT
#include "wnm_cmm.h"
#endif /* DOT11V_WNM_SUPPORT */



#ifdef RT_CFG80211_SUPPORT
#include "cfg80211_cmm.h"
#endif /* RT_CFG80211_SUPPORT */


#include "rate_ctrl/ra_ctrl.h"

#ifdef MT_MAC
#ifdef TXBF_SUPPORT
#include "txbf/mt_txbf.h"
#include "txbf/mt_txbf_cal.h"
#endif
#endif

#ifdef ROUTING_TAB_SUPPORT
#include "routing_tab.h"
#endif /* ROUTING_TAB_SUPPORT */

#include "tr.h"
#include "ht.h"
#include "vht.h"
#include "he.h"

/* vendor specific ie */
#include "vendor.h"

struct _RTMP_RA_LEGACY_TB;

typedef struct _RTMP_ADAPTER RTMP_ADAPTER;
typedef struct _RTMP_ADAPTER *PRTMP_ADAPTER;

typedef struct wifi_dev RTMP_WDEV;
typedef struct wifi_dev *PRTMP_WDEV;

typedef struct _STA_ADMIN_CONFIG STA_ADMIN_CONFIG;
typedef struct _STA_ADMIN_CONFIG *PSTA_ADMIN_CONFIG;
/* typedef struct _RTMP_CHIP_OP RTMP_CHIP_OP; */
/* typedef struct _RTMP_CHIP_CAP RTMP_CHIP_CAP; */

typedef struct _PEER_PROBE_REQ_PARAM PEER_PROBE_REQ_PARAM;

#ifdef BB_SOC
#include "os/bb_soc.h"
#endif

#include "mcu/mcu.h"

#ifdef CONFIG_ANDES_SUPPORT
#include "mcu/andes_core.h"
#endif

#include "radar.h"

#ifdef CARRIER_DETECTION_SUPPORT
#include "cs.h"
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef MT_DFS_SUPPORT
#include "mt_rdm.h" /* Jelly20150322 */
#endif

#ifdef LED_CONTROL_SUPPORT
#include "rt_led.h"
#endif /* LED_CONTROL_SUPPORT */

#ifdef CONFIG_ATE
#include "ate.h"
#endif

#ifdef CONFIG_DOT11U_INTERWORKING
#include "dot11u_interworking.h"
#include "gas.h"
#endif

#ifdef CONFIG_DOT11V_WNM
#include "dot11v_wnm.h"
#include "wnm.h"
#endif

#ifdef CONFIG_HOTSPOT
#include "hotspot.h"
#endif

#ifdef MBO_SUPPORT
#include "mbo.h"
#endif

#ifdef OCE_SUPPORT
#include "oce.h"
#endif /* OCE_SUPPORT */

#ifdef SNIFFER_SUPPORT
#include "sniffer/sniffer.h"
#endif

#ifdef CONFIG_MAP_SUPPORT
#include "map.h"
#endif /* CONFIG_MAP_SUPPORT */

#ifdef COEX_SUPPORT
#include "mcu/btcoex.h"
#endif /* COEX_SUPPORT */

#include "hw_ctrl/cmm_asic.h"

#include "hw_ctrl/cmm_chip.h"

#include "rtmp_dmacb.h"

#include "common/wifi_sys_info.h"
#include "wifi_sys_notify.h"

#ifdef BACKGROUND_SCAN_SUPPORT
#include "bgnd_scan.h"
#endif /* BACKGROUND_SCAN_SUPPORT */

#ifdef SMART_CARRIER_SENSE_SUPPORT
#include "scs.h"
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#include "cmm_rvr_dbg.h"
#ifdef REDUCE_TCP_ACK_SUPPORT
#include "cmm_tcprack.h"
#endif

#ifdef WH_EVENT_NOTIFIER
#include "event_notifier.h"
#endif /* WH_EVENT_NOTIFIER */

#include "wlan_config/config_export.h"
#include "mgmt/be_export.h"
#include "fsm/fsm_sync.h"

#ifdef RED_SUPPORT
#include "ra_ac_q_mgmt.h"
#endif
#ifdef FQ_SCH_SUPPORT
#include "fq_qm.h"
#endif

#ifdef AUTOMATION
#include "automation.h"
#endif /* AUTOMATION */

#ifdef LINUX_NET_TXQ_SUPPORT
#define LINUX_DEF_TX_QUEUE_LENGTH	1000
#endif /* LINUX_NET_TXQ_SUPPORT */

#include "protocol/protection.h"

#include "txpwr/single_sku.h"

#include "dbg_ctrl.h"

/* TODO: shiang-6590, remove it after ATE fully re-organized! copy from rtmp_bbp.h */

/* Debug log color */
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#ifndef MAX_BBP_ID
#ifdef RTMP_RBUS_SUPPORT
/* TODO: for this definition, need to modify it!! */
/*#define MAX_BBP_ID	255 */
#define MAX_BBP_ID	200

#else
#define MAX_BBP_ID	136

#endif /* RTMP_RBUS_SUPPORT */

#endif
/* TODO: ---End */

#define	FW_OWN_POLLING_COUNTER	3000

#define NON_DEFINED_BSSINFO_IDX 0xff


/*+++Used for merge MiniportMMRequest() and MiniportDataMMRequest() into one function */
#define MGMT_USE_QUEUE_FLAG	0x80
#define MGMT_USE_PS_FLAG	0x40
/*---Used for merge MiniportMMRequest() and MiniportDataMMRequest() into one function */
/* The number of channels for per-channel Tx power offset */

#define	MAXSEQ		(0xFFF)

#define MAX_MCS_SET 16		/* From MCS 0 ~ MCS 15 */

#ifdef AIR_MONITOR
#define MAX_NUM_OF_MONITOR_STA		16
#define MAX_NUM_OF_MONITOR_GROUP	8 /* (MAX_NUM_OF_MONITOR_STA/2) */
#define MONITOR_MUAR_BASE_INDEX	 32
#define MAX_NUM_PER_GROUP		   2
#endif /* AIR_MONITOR */

#define MAX_TXPOWER_ARRAY_SIZE	5

#define MAX_EEPROM_BUFFER_SIZE	1024
#define PS_RETRIEVE_TOKEN		0x76

#ifdef TXBF_SUPPORT
#define NSTS_2          2
#define NSTS_3          3
#define NSTS_4          4
#endif

#ifdef DSCP_PRI_SUPPORT
#define DSCP_PRI_2G_MAP	0
#define DSCP_PRI_5G_MAP	1
#endif


#ifdef WIFI_EAP_FEATURE
#ifdef ACK_CTS_TIMEOUT_SUPPORT
#define CCK_OFDM_MAX_ACK_TIMEOUT 0xFFFF
#define SPEED_OF_LIGHT_IN_METER  300 /*speed = 277792458, converted in us*/
#endif /* ACK_CTS_TIMEOUT_SUPPORT */
#endif /* WIFI_EAP_FEATURE */



#define MAX_RXVB_BUFFER_IN_NORMAL 1024
#ifdef IXIA_SUPPORT
typedef struct IXIA_CTL {
	UCHAR iMode:1;
	UCHAR iRssiflag:1;
	UCHAR iMacflag:1;
	UCHAR iForceMTO:1;/*force multi-client MAC to one Mac.*/
	UCHAR iForceAge:1;
	UCHAR itxCtrl:3;
	UCHAR OnLineStaCntChk;
} IXIA_CTL;
#define VERIWAVE_MODE 1
#define NORMAL_MODE 0
#define DUMP_LIMIT 1
typedef enum {
	IXIA_CTL_ENQ_DBG = 1,
	IXIA_CTL_DEQ_DBG,
	IXIA_CTL_MDDCT_DBG,
	IXIA_CTL_FORCE_MAX = 7,/*Force veriwave Mode.*/
} IXIA_CTL_MODE;
#endif /*IXIA_SUPPORT*/
extern unsigned char CISCO_OUI[];
extern UCHAR BaSizeArray[4];

extern UCHAR BROADCAST_ADDR[MAC_ADDR_LEN];
extern UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN];
extern char *CipherName[];
extern UCHAR SNAP_802_1H[6];
extern UCHAR SNAP_BRIDGE_TUNNEL[6];
extern UCHAR EAPOL[2];
extern UCHAR IPX[2];
extern UCHAR TPID[];
extern UCHAR APPLE_TALK[2];
extern UCHAR OfdmRateToRxwiMCS[];
extern UCHAR WMM_UP2AC_MAP[8];

extern unsigned char RateIdToMbps[];
extern USHORT RateIdTo500Kbps[];

extern UCHAR CipherSuiteWpaNoneTkip[];
extern UCHAR CipherSuiteWpaNoneTkipLen;

extern UCHAR CipherSuiteWpaNoneAes[];
extern UCHAR CipherSuiteWpaNoneAesLen;

extern UCHAR SsidIe;
extern UCHAR SupRateIe;
extern UCHAR ExtRateIe;

#ifdef DOT11_N_SUPPORT
extern UCHAR HtCapIe;
extern UCHAR AddHtInfoIe;
extern UCHAR NewExtChanIe;
extern UCHAR BssCoexistIe;
extern UCHAR ExtHtCapIe;
#endif /* DOT11_N_SUPPORT */
extern UCHAR ExtCapIe;

extern UCHAR ErpIe;
extern UCHAR DsIe;
extern UCHAR TimIe;
extern UCHAR WpaIe;
extern UCHAR Wpa2Ie;
extern UCHAR IbssIe;
extern UCHAR WapiIe;

extern UCHAR WPA_OUI[];
extern UCHAR RSN_OUI[];
extern UCHAR WAPI_OUI[];
extern UCHAR WME_INFO_ELEM[];
extern UCHAR WME_PARM_ELEM[];
extern UCHAR RALINK_OUI[];
#if (defined(WH_EZ_SETUP) || defined(MWDS) || defined(WAPP_SUPPORT))
extern UCHAR MTK_OUI[];
#endif
extern UCHAR PowerConstraintIE[];

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

extern struct wifi_fwd_func_table wf_drv_tbl;

extern UCHAR wf_apcli_active_links;
#endif

#ifdef RANDOM_PKT_GEN
extern INT32 RandomTxCtrl;
extern UINT32 Qidmapping[16];
#endif

#ifdef CSO_TEST_SUPPORT
extern INT32 CsCtrl;
#endif

/*VLAN (802.1Q) Related*/
#define TX_VLAN 0
#define RX_VLAN 1
#define MASK_CLEAR_PCP 0xFFFF1FFF
#define MASK_CLEAR_VID 0xFFFFF000
#define MASK_CLEAR_TCI_VID 0xF000
#define MASK_CLEAR_TCI_PCP 0x1FFF
#define MASK_TCI_VID 0x0FFF
#define MAX_VID 0x0FFF
#define MAX_PCP 7
#define PCP_LEN 3
#define CFI_LEN 1
#define VID_LEN 12

#ifdef VLAN_SUPPORT
/*
	VLAN TX Checking Policy :
	If the VLAN ID of the pkt is different from the wdev's VLANID
	0 : Drop this pkt (Drop)
	1 : Do nothing (Allow)
	2 : Replace the VLAN ID by the wdev's VLAN ID.(REPLACE VID )
	3 : Replace the VLAN tag by the wdev's VLAN tag.(REPLACE ALL)
*/
enum VLAN_TX_Policy{
	VLAN_TX_DROP = 0,
	VLAN_TX_KEEP_TAG,
	VLAN_TX_REPLACE_VID,
	VLAN_TX_REPLACE_ALL,
	VLAN_TX_ALLOW,
	VLAN_TX_POLICY_NUM,
};
/*
	VLAN RX Checking Policy :
	If the VLAN ID of the pkt is different from the wdev's VLANID
	0 : Drop this pkt (Drop)
	1 : Remove VLAN Tag (Untag)
	2 : Do nothing (Allow)
	3 : Replace the VID only. (Replace VID)
	     Insert the VLAN Tag if the ingress pkt is non-vlan
	4 : Replace the VLAN tag by the wdev's VLAN tag (Replace ALL)
	     Insert the VLAN Tag if the ingress pkt is non-vlan
*/
enum VLAN_RX_Policy{
	VLAN_RX_DROP = 0,
	VLAN_RX_UNTAG,
	VLAN_RX_ALLOW,
	VLAN_RX_REPLACE_VID,
	VLAN_RX_REPLACE_ALL,
	VLAN_RX_POLICY_NUM,
};

#endif /*VLAN_SUPPORT*/

struct _RX_BLK;
struct raw_rssi_info;

typedef struct _UAPSD_INFO {
	BOOLEAN bAPSDCapable;
} UAPSD_INFO;

typedef union _CAPTURE_MODE_PACKET_BUFFER {
	struct {
		UINT32	   BYTE0:8;
		UINT32	   BYTE1:8;
		UINT32	   BYTE2:8;
		UINT32	   BYTE3:8;
	} field;
	UINT32				   Value;
} CAPTURE_MODE_PACKET_BUFFER;

#ifdef APCLI_SUPPORT
#ifdef APCLI_AUTO_CONNECT_SUPPORT
typedef enum _APCLI_CONNECT_SCAN_TYPE {
	TRIGGER_SCAN_BY_USER = 0,
	TRIGGER_SCAN_BY_DRIVER = 1,
} APCLI_CONNECT_SCAN_TYPE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#ifdef WSC_AP_SUPPORT
typedef enum _APCLI_WSC_SCAN_TYPE {
	TRIGGER_FULL_SCAN = 0,
	TRIGGER_PARTIAL_SCAN = 1
} APCLI_WSC_SCAN_TYPE;
#endif /* WSC_AP_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifdef AIR_MONITOR
#define RULE_CTL							BIT(0)
#define RULE_CTL_OFFSET						0
#define RULE_MGT							BIT(1)
#define RULE_MGT_OFFSET						1
#define RULE_DATA							BIT(2)
#define RULE_DATA_OFFSET					2
#define RULE_A1								BIT(3)
#define RULE_A1_OFFSET						3
#define RULE_A2								BIT(4)
#define RULE_A2_OFFSET						4
#if defined(MT7663) || defined(MT7626)
#define DEFAULT_MNTR_RULE	(RULE_MGT | RULE_DATA | RULE_A1 | RULE_A2)
#else
#define DEFAULT_MNTR_RULE	(RULE_MGT | RULE_CTL | RULE_DATA | RULE_A1 | RULE_A2)
#endif /* defined(MT7663) || defined(MT7626) */

typedef enum _MNT_BAND_TYPE {
	MNT_BAND0 = 1,
	MNT_BAND1 = 2
} MNT_BAND_TYPE;

typedef struct	_MNT_MUAR_GROUP {
	BOOLEAN bValid;
	UCHAR	Count;
	UCHAR   Band;
	UCHAR	MuarGroupBase;
} MNT_MUAR_GROUP, *PMNT_MUAR_GROUP;

typedef struct	_MNT_STA_ENTRY {
	BOOLEAN bValid;
	UCHAR Band;
	UCHAR muar_idx;
	UCHAR muar_group_idx;
	ULONG Count;
	ULONG data_cnt;
	ULONG mgmt_cnt;
	ULONG cntl_cnt;
	UCHAR addr[MAC_ADDR_LEN];
	RSSI_SAMPLE RssiSample;
	VOID *pMacEntry;
}	MNT_STA_ENTRY, *PMNT_STA_ENTRY;

typedef struct _HEADER_802_11_4_ADDR {
	FRAME_CONTROL FC;
	USHORT Duration;
	USHORT SN;
	UCHAR FN;
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR Addr3[MAC_ADDR_LEN];
	UCHAR Addr4[MAC_ADDR_LEN];
} HEADER_802_11_4_ADDR, *PHEADER_802_11_4_ADDR;

typedef struct _AIR_RADIO_INFO {
	CHAR PHYMODE;
	CHAR STREAM;
	CHAR MCS;
	CHAR BW;
	CHAR ShortGI;
	ULONG RATE;
	CHAR RSSI[4];
	UCHAR Channel;
} AIR_RADIO_INFO, *PAIR_RADIO_INFO;

typedef struct _AIR_RAW {
	AIR_RADIO_INFO wlan_radio_tap;
	HEADER_802_11_4_ADDR wlan_header;
} AIR_RAW, *PAIR_RAW;
#endif /* AIR_MONITOR */

#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
enum CH_SWITCH_STATE {
	CH_SWITCH_STATE_BASE = 0,
	CH_SWITCH_STATE_INIT = CH_SWITCH_STATE_BASE,
	CH_SWITCH_STATE_RUNNING,
	CH_SWITCH_STATE_MAX
};
enum CH_SWITCH_MSG {
	CH_SWITCH_MSG_LISTEN = 0,
	CH_SWITCH_MSG_CANCLE,
	CH_SWITCH_MSG_TIMEOUT,
	CH_SWITCH_MSG_MAX
};
#define CH_SWITCH_DFT_LISTEN_TIME 200
#define CH_SWITCH_FUNC_SIZE 6/*CH_SWITCH_STATE_MAX * CH_SWITCH_MSG_MAX*/
struct ch_switch_cfg {
	UCHAR channel;
	USHORT duration;
	BOOLEAN ch_sw_on_going;
	INT	ioctl_if;
	struct wifi_dev *wdev;
	RALINK_TIMER_STRUCT	ch_sw_timer;
	STATE_MACHINE ch_switch_sm;
	STATE_MACHINE_FUNC ch_switch_state_func[CH_SWITCH_FUNC_SIZE];
};
struct ch_switch_user_cfg {
	UCHAR channel;
	USHORT duration;
};
#endif
typedef enum _ENUM_DBDC_MODE_T {
	ENUM_SingleBand = 0,
	ENUM_DBDC_2G5G = 1,
	ENUM_DBDC_5G5G = 2
} ENUM_DBDC_MODE;

/* */
/*  Macros for flag and ref count operations */
/* */
#define RTMP_SET_FLAG(_M, _F)	   ((_M)->Flags |= (_F))
#define RTMP_CLEAR_FLAG(_M, _F)	 ((_M)->Flags &= ~(_F))
#define RTMP_CLEAR_FLAGS(_M)		((_M)->Flags = 0)
#define RTMP_TEST_FLAG(_M, _F)	  (((_M)->Flags & (_F)) != 0)
#define RTMP_TEST_FLAGS(_M, _F)	 (((_M)->Flags & (_F)) == (_F))
/* Macro for power save flag. */
#define RTMP_SET_PSFLAG(_M, _F)	   ((_M)->PSFlags |= (_F))
#define RTMP_CLEAR_PSFLAG(_M, _F)	 ((_M)->PSFlags &= ~(_F))
#define RTMP_CLEAR_PSFLAGS(_M)		((_M)->PSFlags = 0)
#define RTMP_TEST_PSFLAG(_M, _F)	  (((_M)->PSFlags & (_F)) != 0)
#define RTMP_TEST_PSFLAGS(_M, _F)	 (((_M)->PSFlags & (_F)) == (_F))

#define OPSTATUS_SET_FLAG(_pAd, _F)	 ((_pAd)->CommonCfg.OpStatusFlags |= (_F))
#define OPSTATUS_CLEAR_FLAG(_pAd, _F)   ((_pAd)->CommonCfg.OpStatusFlags &= ~(_F))
#define OPSTATUS_TEST_FLAG(_pAd, _F)	(((_pAd)->CommonCfg.OpStatusFlags & (_F)) != 0)

#define OPSTATUS_SET_FLAG_WDEV(_Wdev, _F)	 ((_Wdev)->OpStatusFlags |= (_F))
#define OPSTATUS_CLEAR_FLAG_WDEV(_Wdev, _F)   ((_Wdev)->OpStatusFlags &= ~(_F))
#define OPSTATUS_TEST_FLAG_WDEV(_Wdev, _F)	(((_Wdev)->OpStatusFlags & (_F)) != 0)
#define OPSTATUS_EQUAL_FLAG_WDEV(_Wdev, _F)	(((_Wdev)->OpStatusFlags & (_F)) == (_F))

#ifdef CONFIG_STA_SUPPORT
#define STA_STATUS_SET_FLAG(_pStaCfg, _F)	 ((_pStaCfg)->StaStatusFlags |= (_F))
#define STA_STATUS_CLEAR_FLAG(_pStaCfg, _F)   ((_pStaCfg)->StaStatusFlags &= ~(_F))
#define STA_STATUS_TEST_FLAG(_pStaCfg, _F)	(((_pStaCfg)->StaStatusFlags & (_F)) != 0)
#endif

#define WIFI_TEST_SET_FLAG(_pAd, _F)	 ((_pAd)->CommonCfg.WiFiTestFlags |= (_F))
#define WIFI_TEST_CLEAR_FLAG(_pAd, _F)   ((_pAd)->CommonCfg.WiFiTestFlags &= ~(_F))
#define WIFI_TEST_CHECK_FLAG(_pAd, _F)	(((_pAd)->CommonCfg.WiFiTestFlags & (_F)) != 0)

#define CLIENT_STATUS_SET_FLAG(_pEntry, _F)	  ((_pEntry)->ClientStatusFlags |= (_F))
#define CLIENT_STATUS_CLEAR_FLAG(_pEntry, _F)	((_pEntry)->ClientStatusFlags &= ~(_F))
#define CLIENT_STATUS_TEST_FLAG(_pEntry, _F)	 (((_pEntry)->ClientStatusFlags & (_F)) != 0)

#define CLIENT_CAP_SET_FLAG(_pEntry, _F)	  ((_pEntry)->cli_cap_flags |= (_F))
#define CLIENT_CAP_CLEAR_FLAG(_pEntry, _F)	((_pEntry)->cli_cap_flags &= ~(_F))
#define CLIENT_CAP_TEST_FLAG(_pEntry, _F)	 (((_pEntry)->cli_cap_flags & (_F)) != 0)

#define RX_FILTER_SET_FLAG(_pAd, _F)	((_pAd)->CommonCfg.PacketFilter |= (_F))
#define RX_FILTER_CLEAR_FLAG(_pAd, _F)  ((_pAd)->CommonCfg.PacketFilter &= ~(_F))
#define RX_FILTER_TEST_FLAG(_pAd, _F)   (((_pAd)->CommonCfg.PacketFilter & (_F)) != 0)

#define RTMP_SET_MORE_FLAG(_M, _F)	   ((_M)->MoreFlags |= (_F))
#define RTMP_TEST_MORE_FLAG(_M, _F)	  (((_M)->MoreFlags & (_F)) != 0)
#define RTMP_CLEAR_MORE_FLAG(_M, _F)	 ((_M)->MoreFlags &= ~(_F))

#define SET_ASIC_CAP(_pAd, _caps)		(hc_set_asic_cap(_pAd->hdev_ctrl, _caps))
#define IS_ASIC_CAP(_pAd, _caps)			((hc_get_asic_cap(_pAd->hdev_ctrl) & (_caps)) != 0)
#define CLR_ASIC_CAP(_pAd, _caps)		(hc_clear_asic_cap(_pAd->hdev_ctrl, _caps))
#define IS_HIF_TYPE(_pAd, _type) (hc_get_hif_type(_pAd->hdev_ctrl) == _type)
#define GET_HIF_TYPE(_pAd) (hc_get_hif_type(_pAd->hdev_ctrl))

/*
 * Replace pAd in the future, new definition API parameters
 */
/* MAC CAPS(old ASIC_CAPS) */
#define IS_MAC_CAPS(mac_caps, cap) (((mac_caps) & (cap)) != 0)
#define SET_MAC_CAPS(hdev_ctrl, caps)\
		hc_set_mac_cap(hdev_ctrl, caps)
#define GET_MAC_CAPS(hdev_ctrl)\
		hc_get_mac_cap(hdev_ctrl)
/* PHY CAPS */
#define IS_PHY_CAPS(phy_caps, cap) (((phy_caps) & (cap)) != 0)
#define SET_PHY_CAPS(hdev_ctrl, caps)\
		hc_set_phy_cap(hdev_ctrl, caps)
#define GET_PHY_CAPS(hdev_ctrl)\
		hc_get_phy_cap(hdev_ctrl)

#define TX_FLAG_STOP_DEQUEUE	(fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS | \
								 fRTMP_ADAPTER_HALT_IN_PROGRESS | \
								 fRTMP_ADAPTER_NIC_NOT_EXIST | \
								 fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET)

#define INC_RING_INDEX(_idx, _RingSize)	\
	{										  \
		(_idx) = (_idx+1) % (_RingSize);	   \
	}

#define TR_ENQ_COUNT_INC(tr) \
	{								\
		tr->enqCount++;				\
	}

#define TR_ENQ_COUNT_DEC(tr) \
	{								\
		tr->enqCount--;				\
	}

#define TR_TOKEN_COUNT_INC(tr, qid) \
	{								\
		tr->TokenCount[qid]++;				\
	}

#define TR_TOKEN_COUNT_DEC(tr, qid) \
	{								\
		tr->TokenCount[qid]--;				\
	}

#define SQ_ENQ_PS_MAX 32
#define SQ_ENQ_PSQ_MAX 32
#ifdef MEMORY_OPTIMIZATION
#define SQ_ENQ_PSQ_TOTAL_MAX 128
#define SQ_ENQ_RESERVE_PERAC	(1024)
#else
#define SQ_ENQ_PSQ_TOTAL_MAX 1024
#ifdef IXIA_SUPPORT
#define SQ_ENQ_RESERVE_PERAC	(8192)
#else
#define SQ_ENQ_RESERVE_PERAC	(4096)
#endif /*IXIA_SUPPORT*/
#endif
#define SQ_ENQ_NORMAL_MAX	(SQ_ENQ_RESERVE_PERAC * 4)

#ifdef USB_BULK_BUF_ALIGMENT
#define CUR_WRITE_IDX_INC(_idx, _RingSize)	\
	{										  \
		(_idx) = (_idx+1) % (_RingSize);	   \
	}
#endif /* USB_BULK_BUF_ALIGMENT */

/*
	Common fragment list structure -  Identical to the scatter gather frag list structure
*/
#define NIC_MAX_PHYS_BUF_COUNT			  8

typedef struct _RTMP_SCATTER_GATHER_ELEMENT {
	PVOID Address;
	ULONG Length;
	PULONG Reserved;
} RTMP_SCATTER_GATHER_ELEMENT, *PRTMP_SCATTER_GATHER_ELEMENT;

typedef struct _RTMP_SCATTER_GATHER_LIST {
	ULONG NumberOfElements;
	PULONG Reserved;
	RTMP_SCATTER_GATHER_ELEMENT Elements[NIC_MAX_PHYS_BUF_COUNT];
} RTMP_SCATTER_GATHER_LIST, *PRTMP_SCATTER_GATHER_LIST;

typedef struct _MT_MIB_BUCKET_ONE_SEC {
	UCHAR	Enabled[DBDC_BAND_NUM];
	UINT32 ChannelBusyTime[DBDC_BAND_NUM]; /* Every second update once. primary channel only */
	UINT32 ChannelBusyTimeCcaNavTx[DBDC_BAND_NUM]; /* Every second update once. including CCA NAV TX time*/
	UINT32 OBSSAirtime[DBDC_BAND_NUM];
	UINT32 MyTxAirtime[DBDC_BAND_NUM];
	UINT32 MyRxAirtime[DBDC_BAND_NUM];
	UINT32 EDCCAtime[DBDC_BAND_NUM];
	UINT32 PdCount[DBDC_BAND_NUM];
	UINT32 MdrdyCount[DBDC_BAND_NUM];
} MT_MIB_BUCKET_ONE_SEC, *PMT_MIB_BUCKET_ONE_SEC;

typedef struct _MT_MIB_BUCKET_MS {
	UCHAR	CurIdx;
	UCHAR	Enabled;
	UINT32 ChannelBusyTime[DBDC_BAND_NUM][2];
	UINT32 ChannelBusyTimeCcaNavTx[DBDC_BAND_NUM][2];
	UINT32 OBSSAirtime[DBDC_BAND_NUM][2];
	UINT32 MyTxAirtime[DBDC_BAND_NUM][2];
	UINT32 MyRxAirtime[DBDC_BAND_NUM][2];
	UINT32 EDCCAtime[DBDC_BAND_NUM][2];
	UINT32 PdCount[DBDC_BAND_NUM][2];
	UINT32 MdrdyCount[DBDC_BAND_NUM][2];
	UINT32 RO_BAND0_PHYCTRL_STS0_OFFSET;
	UINT32 RO_BAND0_PHYCTRL_STS5_OFFSET;
	UINT32 PHY_BAND0_PHYMUX_5_OFFSET;
} MT_MIB_BUCKET_MS, *PMT_MIB_BUCKET_MS;

/*
	Some utility macros
*/
#define GET_LNA_GAIN(_pAd)	((_pAd->LatchRfRegs.Channel <= 14) ? (_pAd->BLNAGain) : ((_pAd->LatchRfRegs.Channel <= 64) ? (_pAd->ALNAGain0) : ((_pAd->LatchRfRegs.Channel <= 128) ? (_pAd->ALNAGain1) : (_pAd->ALNAGain2))))

#define INC_COUNTER64(Val)		  (Val.QuadPart++)

#define INFRA_ON(_p)				(STA_STATUS_TEST_FLAG(_p, fSTA_STATUS_INFRA_ON))
#define ADHOC_ON(_p)				(OPSTATUS_TEST_FLAG(_p, fOP_STATUS_ADHOC_ON))
#define MONITOR_ON(_p)			  (((_p)->monitor_ctrl.bMonitorOn) == TRUE)
#define IDLE_ON(_pAd, _pStaCfg)				 (!INFRA_ON(_pStaCfg) && !ADHOC_ON(_pAd))

/* Check LEAP & CCKM flags */
#define LEAP_ON(_p)				 (((_p)->StaCfg[0].LeapAuthMode) == CISCO_AuthModeLEAP)
#define LEAP_CCKM_ON(_p)			((((_p)->StaCfg[0].LeapAuthMode) == CISCO_AuthModeLEAP) && ((_p)->StaCfg[0].LeapAuthInfo.CCKM == TRUE))

/* if orginal Ethernet frame contains no LLC/SNAP, then an extra LLC/SNAP encap is required */
#define EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(_pBufVA, _pExtraLlcSnapEncap)		\
	{																\
		if (((*(_pBufVA + 12) << 8) + *(_pBufVA + 13)) > 1500) {		\
			_pExtraLlcSnapEncap = SNAP_802_1H;						\
			if (NdisEqualMemory(IPX, _pBufVA + 12, 2) ||			\
				NdisEqualMemory(APPLE_TALK, _pBufVA + 12, 2))		\
				_pExtraLlcSnapEncap = SNAP_BRIDGE_TUNNEL;			\
		} else														\
			_pExtraLlcSnapEncap = NULL;								\
	}

/* New Define for new Tx Path. */
#define EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(_pBufVA, _pExtraLlcSnapEncap)	\
	{																\
		if (((*(_pBufVA) << 8) + *(_pBufVA + 1)) > 1500) {			\
			_pExtraLlcSnapEncap = SNAP_802_1H;						\
			if (NdisEqualMemory(IPX, _pBufVA, 2) ||				\
				NdisEqualMemory(APPLE_TALK, _pBufVA, 2))			\
				_pExtraLlcSnapEncap = SNAP_BRIDGE_TUNNEL;			\
		} else														\
			_pExtraLlcSnapEncap = NULL;								\
	}

#define MAKE_802_3_HEADER(_buf, _pMac1, _pMac2, _pType)				   \
	{																	   \
		NdisMoveMemory(_buf, _pMac1, MAC_ADDR_LEN);						   \
		NdisMoveMemory((_buf + MAC_ADDR_LEN), _pMac2, MAC_ADDR_LEN);		  \
		NdisMoveMemory((_buf + MAC_ADDR_LEN * 2), _pType, LENGTH_802_3_TYPE); \
	}

/*
	if pData has no LLC/SNAP (neither RFC1042 nor Bridge tunnel),
		keep it that way.
	else if the received frame is LLC/SNAP-encaped IPX or APPLETALK,
		preserve the LLC/SNAP field
	else remove the LLC/SNAP field from the result Ethernet frame

	Patch for WHQL only, which did not turn on Netbios but use IPX within its payload
	Note:
		_pData & _DataSize may be altered (remove 8-byte LLC/SNAP) by this MACRO
		_pRemovedLLCSNAP: pointer to removed LLC/SNAP; NULL is not removed
*/
#define CONVERT_TO_802_3(_p8023hdr, _pDA, _pSA, _pData, _DataSize, _pRemovedLLCSNAP)	  \
	{																	   \
		char LLC_Len[2];													\
		\
		_pRemovedLLCSNAP = NULL;											\
		if (NdisEqualMemory(SNAP_802_1H, _pData, 6)  ||					 \
			NdisEqualMemory(SNAP_BRIDGE_TUNNEL, _pData, 6)) {				 \
			PUCHAR pProto = _pData + 6;									 \
			\
			if ((NdisEqualMemory(IPX, pProto, 2) || NdisEqualMemory(APPLE_TALK, pProto, 2)) &&  \
				NdisEqualMemory(SNAP_802_1H, _pData, 6)) {					\
				LLC_Len[0] = (UCHAR)(_DataSize >> 8);					   \
				LLC_Len[1] = (UCHAR)(_DataSize & (256 - 1));				\
				MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);		  \
			} else {															\
				MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, pProto);		   \
				_pRemovedLLCSNAP = _pData;								  \
				_DataSize -= LENGTH_802_1_H;								\
				_pData += LENGTH_802_1_H;								   \
			}															   \
		} else {																\
			LLC_Len[0] = (UCHAR)(_DataSize >> 8);						   \
			LLC_Len[1] = (UCHAR)(_DataSize & (256 - 1));					\
			MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);			  \
		}																   \
	}

/*
	Enqueue this frame to MLME engine
	We need to enqueue the whole frame because MLME need to pass data type
	information from 802.11 header
*/
#define REPORT_MGMT_FRAME_TO_MLME(_pAd, Wcid, _pFrame, _FrameSize, _Rssi0, _Rssi1, _Rssi2, _Rssi3, \
	_MinSNR, _channel, _OpMode, _wdev, _RxPhyMode)\
	do {																					   \
		struct raw_rssi_info _rssi_info;\
		_rssi_info.raw_rssi[0] = _Rssi0;\
		_rssi_info.raw_rssi[1] = _Rssi1;\
		_rssi_info.raw_rssi[2] = _Rssi2;\
		_rssi_info.raw_rssi[3] = _Rssi3;\
		_rssi_info.raw_snr = _MinSNR;\
		_rssi_info.Channel = _channel;\
		MlmeEnqueueForRecv(_pAd, Wcid, &_rssi_info, _FrameSize, _pFrame, _OpMode, _wdev, _RxPhyMode);   \
	} while (0)

#ifdef OUI_CHECK_SUPPORT
enum {
	OUI_MGROUP_ACT_JOIN = 0,
	OUI_MGROUP_ACT_LEAVE = 1
};
#define MAC_OUI_EQUAL(pAddr1, pAddr2)		   RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), 4)
#endif /*OUI_CHECK_SUPPORT*/
#define IPV4_ADDR_EQUAL(pAddr1, pAddr2)		 RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), 4)
#define IPV6_ADDR_EQUAL(pAddr1, pAddr2)		 RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), 16)
#define MAC_ADDR_EQUAL(pAddr1, pAddr2)		   RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), MAC_ADDR_LEN)
#define SSID_EQUAL(ssid1, len1, ssid2, len2)	((len1 == len2) && (RTMPEqualMemory(ssid1, ssid2, len1)))

#define ONE_SEC_2_US			0xF4240
#define OBSSAIRTIME_TH		  60
#define RX_MIBTIME_CLR_OFFSET   31
#define RX_MIBTIME_EN_OFFSET	30
#define OBSS_OCCUPY_PERCENT_HIGH_TH 40
#define OBSS_OCCUPY_PERCENT_LOW_TH 40
#define EDCCA_OCCUPY_PERCENT_TH 40
#define My_OCCUPY_PERCENT	   15
#define ALL_AIR_OCCUPY_PERCENT 90
#define TX_RATIO_TH			 90

#define BAND0_SPE_IDX 0x18
#if defined(MT7626)
#define BAND1_SPE_IDX 0x18
#else
#define BAND1_SPE_IDX 0x19
#endif

BOOLEAN MonitorRxRing(struct _RTMP_ADAPTER *pAd);
BOOLEAN MonitorTxRing(struct _RTMP_ADAPTER *pAd);
BOOLEAN MonitorRxPse(struct _RTMP_ADAPTER *pAd);

/*
	Statistic counter structure
*/
typedef struct _COUNTER_802_3 {
	/* General Stats */
	ULONG GoodTransmits;
	ULONG GoodReceives;
	ULONG TxErrors;
	ULONG RxErrors;
	ULONG RxNoBuffer;
} COUNTER_802_3, *PCOUNTER_802_3;

typedef struct _COUNTER_802_11 {
	ULONG Length;
	/*	LARGE_INTEGER   LastTransmittedFragmentCount; */
	LARGE_INTEGER TransmittedFragmentCount;
	LARGE_INTEGER MulticastTransmittedFrameCount;
	LARGE_INTEGER FailedCount;
	LARGE_INTEGER RetryCount;
	LARGE_INTEGER MultipleRetryCount;
	LARGE_INTEGER RTSSuccessCount;
	LARGE_INTEGER RTSFailureCount;
	LARGE_INTEGER ACKFailureCount;
	LARGE_INTEGER FrameDuplicateCount;
	LARGE_INTEGER ReceivedFragmentCount;
	LARGE_INTEGER MulticastReceivedFrameCount;
	LARGE_INTEGER FCSErrorCount;
	LARGE_INTEGER TransmittedFrameCount;
	LARGE_INTEGER WEPUndecryptableCount;
	LARGE_INTEGER TransmitCountFrmOs;
#ifdef OUI_CHECK_SUPPORT
	LARGE_INTEGER RxHWLookupWcidErrCount;
#endif
#ifdef MT_MAC
	LARGE_INTEGER TxAggRange1Count;
	LARGE_INTEGER TxAggRange2Count;
	LARGE_INTEGER TxAggRange3Count;
	LARGE_INTEGER TxAggRange4Count;
#if defined(MT7626)
	LARGE_INTEGER TxAggRange5Count;
	LARGE_INTEGER TxAggRange6Count;
	LARGE_INTEGER TxAggRange7Count;
	LARGE_INTEGER TxAggRange8Count;
	LARGE_INTEGER TxAMSDU1Count;
	LARGE_INTEGER TxAMSDU2Count;
	LARGE_INTEGER TxAMSDU3Count;
	LARGE_INTEGER TxAMSDU4Count;
#endif
	/* for PER debug */
	LARGE_INTEGER AmpduFailCount;
	LARGE_INTEGER AmpduSuccessCount;
	/* for PER debug */
	LARGE_INTEGER CurrentBwTxCount;
	LARGE_INTEGER OtherBwTxCount;

#endif /* MT_MAC */
} COUNTER_802_11, *PCOUNTER_802_11;

typedef struct _COUNTER_RALINK {
	UINT32 OneSecStart;	/* for one sec count clear use */
	UINT32 OneSecBeaconSentCnt;
	UINT32 OneSecFalseCCACnt;	/* CCA error count, for debug purpose, might move to global counter */
	UINT32 OneSecRxFcsErrCnt;	/* CRC error */
	UINT32 OneSecRxOkCnt;	/* RX without error */
	UINT32 OneSecTxFailCount;
	UINT32 OneSecTxNoRetryOkCount;
	UINT32 OneSecTxRetryOkCount;
	UINT32 OneSecRxOkDataCnt;	/* unicast-to-me DATA frame count */
	UINT32 OneSecTransmittedByteCount;	/* both successful and failure, used to calculate TX throughput */

	ULONG *OneSecOsTxCount;
	ULONG *OneSecDmaDoneCount;
	UINT32 OneSecTxDoneCount;
	ULONG OneSecRxCount;
	UINT32 OneSecReceivedByteCount;
	UINT32 OneSecTxARalinkCnt;	/* Tx Ralink Aggregation frame cnt */
	UINT32 OneSecRxARalinkCnt;	/* Rx Ralink Aggregation frame cnt */
	UINT32 OneSecEnd;	/* for one sec count clear use */

	ULONG TransmittedByteCount;	/* both successful and failure, used to calculate TX throughput */
	ULONG ReceivedByteCount;	/* both CRC okay and CRC error, used to calculate RX throughput */
	ULONG BadCQIAutoRecoveryCount;
	ULONG PoorCQIRoamingCount;
	ULONG MgmtRingFullCount;
	ULONG RxCountSinceLastNULL;
	ULONG RxCount;
	ULONG KickTxCount;
	LARGE_INTEGER RealFcsErrCount;
	ULONG PendingNdisPacketCount;
	ULONG FalseCCACnt;					/* CCA error count */

	UINT32 LastOneSecTotalTxCount;	/* OneSecTxNoRetryOkCount + OneSecTxRetryOkCount + OneSecTxFailCount */
	UINT32 LastOneSecRxOkDataCnt;	/* OneSecRxOkDataCnt */
	ULONG DuplicateRcv;
	ULONG TxAggCount;
	ULONG TxNonAggCount;
	ULONG TxAgg1MPDUCount;
	ULONG TxAgg2MPDUCount;
	ULONG TxAgg3MPDUCount;
	ULONG TxAgg4MPDUCount;
	ULONG TxAgg5MPDUCount;
	ULONG TxAgg6MPDUCount;
	ULONG TxAgg7MPDUCount;
	ULONG TxAgg8MPDUCount;
	ULONG TxAgg9MPDUCount;
	ULONG TxAgg10MPDUCount;
	ULONG TxAgg11MPDUCount;
	ULONG TxAgg12MPDUCount;
	ULONG TxAgg13MPDUCount;
	ULONG TxAgg14MPDUCount;
	ULONG TxAgg15MPDUCount;
	ULONG TxAgg16MPDUCount;

	LARGE_INTEGER TxAMSDUCount;
	LARGE_INTEGER RxAMSDUCount;
	LARGE_INTEGER TransmittedAMPDUCount;
	LARGE_INTEGER TransmittedMPDUsInAMPDUCount;
	LARGE_INTEGER TransmittedOctetsInAMPDUCount;
	LARGE_INTEGER MPDUInReceivedAMPDUCount;

	ULONG PhyErrCnt;
	ULONG PlcpErrCnt;
} COUNTER_RALINK, *PCOUNTER_RALINK;

typedef struct _COUNTER_DRS {
	/* to record the each TX rate's quality. 0 is best, the bigger the worse. */
	USHORT TxQuality[MAX_TX_RATE_INDEX + 1];
	UCHAR PER[MAX_TX_RATE_INDEX + 1];
	UCHAR TxRateUpPenalty;	/* extra # of second penalty due to last unstable condition */
	/*BOOLEAN		 fNoisyEnvironment; */
	BOOLEAN fLastSecAccordingRSSI;
	UCHAR LastSecTxRateChangeAction;	/* 0: no change, 1:rate UP, 2:rate down */
	UCHAR LastTimeTxRateChangeAction;	/*Keep last time value of LastSecTxRateChangeAction */
	ULONG LastTxOkCount;
} COUNTER_DRS, *PCOUNTER_DRS;

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
typedef struct _COUNTER_TXBF {
	ULONG TxSuccessCount;
	ULONG TxRetryCount;
	ULONG TxFailCount;
	ULONG ETxSuccessCount;
	ULONG ETxRetryCount;
	ULONG ETxFailCount;
	ULONG ITxSuccessCount;
	ULONG ITxRetryCount;
	ULONG ITxFailCount;
} COUNTER_TXBF;
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
typedef struct _STREAM_MODE_ENTRY_ {
#define STREAM_MODE_STATIC		1
	USHORT flag;
	UCHAR macAddr[MAC_ADDR_LEN];
} STREAM_MODE_ENTRY;
#endif /* STREAM_MODE_SUPPORT */

/* for Microwave oven */
#ifdef MICROWAVE_OVEN_SUPPORT
typedef struct _MO_CFG_STRUCT {
	BOOLEAN		bEnable;
	UINT8		nPeriod_Cnt;	/* measurement period 100ms, mitigate the interference period 900 ms */
	UINT16		nFalseCCACnt;
	UINT16		nFalseCCATh;	/* default is 100 */
} MO_CFG_STRUCT, *PMO_CFG_STRUCT;
#endif /* MICROWAVE_OVEN_SUPPORT */

/* TODO: need to integrate with MICROWAVE_OVEN_SUPPORT */
#ifdef DYNAMIC_VGA_SUPPORT
/* for dynamic vga */
typedef struct _LNA_VGA_CTL_STRUCT {
	BOOLEAN		bEnable;
	BOOLEAN		bDyncVgaEnable;
	UINT8		nPeriod_Cnt;	/* measurement period 100ms, mitigate the interference period 900 ms */
	UINT16		nFalseCCACnt;
	UINT16		nFalseCCATh;	/* default is 100 */
	UINT16		nLowFalseCCATh;
	UCHAR		agc_vga_init_0;
	UCHAR		agc_vga_ori_0; /* the original vga gain initialized by firmware at start up */
	UINT16		agc_0_vga_set1_2;
	UCHAR		agc_vga_init_1;
	UCHAR		agc_vga_ori_1; /* the original vga gain initialized by firmware at start up */
	UINT16		agc_1_vga_set1_2;
} LNA_VGA_CTL_STRUCT, *PLNA_VGA_CTL_STRUCT;
#endif /* DYNAMIC_VGA_SUPPORT */

/***************************************************************************
  *	security key related data structure
  **************************************************************************/

/* structure to define WPA Group Key Rekey Interval */
typedef struct GNU_PACKED _RT_802_11_WPA_REKEY {
	ULONG ReKeyMethod;	/* mechanism for rekeying: 0:disable, 1: time-based, 2: packet-based */
	ULONG ReKeyInterval;	/* time-based: seconds, packet-based: kilo-packets */
} RT_WPA_REKEY, *PRT_WPA_REKEY, RT_802_11_WPA_REKEY, *PRT_802_11_WPA_REKEY;


typedef struct {
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR ErrorCode[2];	/*00 01-Invalid authentication type */
	/*00 02-Authentication timeout */
	/*00 03-Challenge from AP failed */
	/*00 04-Challenge to AP failed */
	BOOLEAN Reported;
} ROGUEAP_ENTRY, *PROGUEAP_ENTRY;

typedef struct {
	UCHAR RogueApNr;
	ROGUEAP_ENTRY RogueApEntry[MAX_LEN_OF_BSS_TABLE];
} ROGUEAP_TABLE, *PROGUEAP_TABLE;

/*
  *	Fragment Frame structure
  */
typedef struct _FRAGMENT_FRAME {
	PNDIS_PACKET pFragPacket;
	ULONG RxSize;
	USHORT Sequence;
	USHORT LastFrag;
	ULONG Flags;		/* Some extra frame information. bit 0: LLC presented */
	BOOLEAN Header_802_3;
} FRAGMENT_FRAME, *PFRAGMENT_FRAME;

/*
	Tkip Key structure which RC4 key & MIC calculation
*/
typedef struct _TKIP_KEY_INFO {
	UINT nBytesInM;		/* # bytes in M for MICKEY */
	UINT32 IV16;
	UINT32 IV32;
	UINT32 K0;		/* for MICKEY Low */
	UINT32 K1;		/* for MICKEY Hig */
	UINT32 L;		/* Current state for MICKEY */
	UINT32 R;		/* Current state for MICKEY */
	UINT32 M;		/* Message accumulator for MICKEY */
	UCHAR RC4KEY[16];
	UCHAR MIC[8];
} TKIP_KEY_INFO, *PTKIP_KEY_INFO;

/*
	Private / Misc data, counters for driver internal use
*/
typedef struct __PRIVATE_STRUC {
	/* Tx ring full occurrance number */
	UINT TxRingFullCnt;
	/* Tkip stuff */
	TKIP_KEY_INFO Tx;
	TKIP_KEY_INFO Rx;
} PRIVATE_STRUC, *PPRIVATE_STRUC;

/***************************************************************************
  *	Channel and BBP related data structures
  **************************************************************************/
/* structure to tune BBP R66 (BBP TUNING) */
typedef struct _BBP_R66_TUNING {
	BOOLEAN bEnable;
	USHORT FalseCcaLowerThreshold;	/* default 100 */
	USHORT FalseCcaUpperThreshold;	/* default 512 */
	UCHAR R66Delta;
	UCHAR R66CurrentValue;
	BOOLEAN R66LowerUpperSelect;	/*Before LinkUp, Used LowerBound or UpperBound as R66 value. */
} BBP_R66_TUNING, *PBBP_R66_TUNING;

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
#define EFFECTED_CH_SECONDARY 0x1
#define EFFECTED_CH_PRIMARY	0x2
#define EFFECTED_CH_LEGACY		0x4
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

/* structure to store channel TX power */
typedef struct _CHANNEL_TX_POWER {
	USHORT RemainingTimeForUse;	/*unit: sec */
	UCHAR Channel;
#ifdef DOT11N_DRAFT3
	BOOLEAN bEffectedChannel;	/* For BW 40 operating in 2.4GHz , the "effected channel" is the channel that is covered in 40Mhz. */
#endif /* DOT11N_DRAFT3 */
	CHAR Power;
	CHAR Power2;
	UCHAR MaxTxPwr;
	UCHAR DfsReq;
	UCHAR RegulatoryDomain;

	/*
		Channel property:

		CHANNEL_DISABLED: The channel is disabled.
		CHANNEL_PASSIVE_SCAN: Only passive scanning is allowed.
		CHANNEL_NO_IBSS: IBSS is not allowed.
		CHANNEL_RADAR: Radar detection is required.
		CHANNEL_NO_FAT_ABOVE: Extension channel above this channel is not allowed.
		CHANNEL_NO_FAT_BELOW: Extension channel below this channel is not allowed.
		CHANNEL_40M_CAP: 40 BW channel group
		CHANNEL_80M_CAP: 800 BW channel group
	 */
#define CHANNEL_DEFAULT_PROP		 0x00
#define CHANNEL_DISABLED			 0x01	/* no use */
#define CHANNEL_PASSIVE_SCAN		 0x02
#define CHANNEL_NO_IBSS				 0x04
#define CHANNEL_RADAR				 0x08
#define CHANNEL_NO_FAT_ABOVE		 0x10
#define CHANNEL_NO_FAT_BELOW		 0x20
#define CHANNEL_40M_CAP				 0x40
#define CHANNEL_80M_CAP				 0x80
#define CHANNEL_160M_CAP			0x100

	UINT Flags;
} CHANNEL_TX_POWER, *PCHANNEL_TX_POWER;

typedef enum  {
	CH_LIST_STATE_NONE = 0,
	CH_LIST_STATE_DONE,
} CH_LIST_STATE;

typedef struct _CHANNEL_CTRL {
	CHANNEL_TX_POWER ChList[MAX_NUM_OF_CHANNELS];	/* list all supported channels for site survey */
	UCHAR ChListNum; /* number of channel in ChannelList[] */
	/* Channel Group*/
	UCHAR ChGrpABandEn;
	UCHAR ChGrpABandChList[MAX_NUM_OF_CHANNELS];
	UCHAR ChGrpABandChNum;
	CH_LIST_STATE ChListStat; /* State of channel list, 0: None, 1: Done */
} CHANNEL_CTRL, *PCHANNEL_CTRL;
/* Channel list subset */
typedef struct _CHANNEL_LIST_SUB {
	UCHAR	Channel;
	UCHAR	IdxMap; /* Index mapping to original channel list */
} CHANNEL_LIST_SUB, *PCHANNEL_LIST_SUB;

typedef struct _SOFT_RX_ANT_DIVERSITY_STRUCT {
	UCHAR EvaluatePeriod;	/* 0:not evalute status, 1: evaluate status, 2: switching status */
	UCHAR EvaluateStableCnt;
	UCHAR Pair1PrimaryRxAnt;	/* 0:Ant-E1, 1:Ant-E2 */
	UCHAR Pair1SecondaryRxAnt;	/* 0:Ant-E1, 1:Ant-E2 */
#ifdef CONFIG_STA_SUPPORT
	SHORT Pair1AvgRssi[2];	/* AvgRssi[0]:E1, AvgRssi[1]:E2 */
	SHORT Pair2AvgRssi[2];	/* AvgRssi[0]:E3, AvgRssi[1]:E4 */
#endif /* CONFIG_STA_SUPPORT */
	SHORT Pair1LastAvgRssi;	/* */
	SHORT Pair2LastAvgRssi;	/* */
	ULONG RcvPktNumWhenEvaluate;
	BOOLEAN FirstPktArrivedWhenEvaluate;
#ifdef CONFIG_AP_SUPPORT
	LONG Pair1AvgRssiGroup1[2];
	LONG Pair1AvgRssiGroup2[2];
	ULONG RcvPktNum[2];
#endif /* CONFIG_AP_SUPPORT */
} SOFT_RX_ANT_DIVERSITY, *PSOFT_RX_ANT_DIVERSITY;

typedef enum _ABGBAND_STATE_ {
	UNKNOWN_BAND,
	BG_BAND,
	A_BAND,
} ABGBAND_STATE;

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
#ifdef RTMP_MAC_PCI
/* Power save method control */
typedef union _PS_CONTROL {
	struct {
		ULONG EnablePSinIdle:1;	/* Enable radio off when not connect to AP. radio on only when sitesurvey, */
		ULONG EnableNewPS:1;	/* Enable new  Chip power save fucntion . New method can only be applied in chip version after 2872. and PCIe. */
		ULONG rt30xxPowerMode:2;	/* Power Level Mode for rt30xx chip */
		ULONG rt30xxFollowHostASPM:1;	/* Card Follows Host's setting for rt30xx chip. */
		ULONG rt30xxForceASPMTest:1;	/* Force enable L1 for rt30xx chip. This has higher priority than rt30xxFollowHostASPM Mode. */
		/*		ULONG		rsv:26;			// Radio Measurement Enable */
		ULONG AMDNewPSOn:1;	/* Enable for AMD L1 (toggle) */
		ULONG LedMode:2;	/* 0: Blink normal.  1: Slow blink not normal. */
		ULONG rt30xxForceL0:1;	/* Force only use L0 for rt30xx */

		/* PCIe config space [Completion TimeOut Disable], compatible issue with Intel HM55 */
		ULONG CTO:1;	/* 0: default, update the CTO bit to disable; 1: Keep BIOS config value */
		ULONG PM4PCIeCLKOn:1;	/* 0: default, turn off PCIE CLk at PM4; 1: FW MCU cmd arg1 as "0x5a" which will not turn off PCIE CLK */

		ULONG rsv:20;	/* Rsvd */
	} field;
	ULONG word;
} PS_CONTROL, *PPS_CONTROL;
#endif /* RTMP_MAC_PCI */
#endif /* CONFIG_STA_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef REPEATER_TX_RX_STATISTIC
typedef struct _RETR_TX_RX_INFO {
	UINT64 TxPackets;
	UINT64 TxBytes;
	UINT64 TxFailPackets;
	UINT64 TxDropPackets;
	UINT64 RxPackets;
	UINT64 RxBytes;
} RETRTXRXINFO, *pRETRTXRXINFO;
#endif /* REPEATER_TX_RX_STATISTIC */
#endif /* APCLI_SUPPORT */

/***************************************************************************
  *	structure for MLME state machine
  **************************************************************************/
typedef struct _MLME_STRUCT {
#ifdef CONFIG_STA_SUPPORT
	/* STA state machines */
#ifdef DOT11R_FT_SUPPORT
	STATE_MACHINE FtOtaAuthMachine;
	STATE_MACHINE_FUNC FtOtaAuthFunc[FT_OTA_AUTH_FUNC_SIZE];
	STATE_MACHINE FtOtdActMachine;
	STATE_MACHINE_FUNC FtOtdActFunc[FT_OTD_FUNC_SIZE];
#endif /* DOT11R_FT_SUPPORT */

#endif /* CONFIG_STA_SUPPORT */
	STATE_MACHINE_FUNC ActFunc[ACT_FUNC_SIZE];
	/* Action */
	STATE_MACHINE ActMachine;

#ifdef WSC_INCLUDED
	STATE_MACHINE WscMachine;
	STATE_MACHINE_FUNC WscFunc[WSC_FUNC_SIZE];

#endif /* WSC_INCLUDED */


#ifdef CONFIG_HOTSPOT
	STATE_MACHINE HSCtrlMachine;
	STATE_MACHINE_FUNC HSCtrlFunc[GAS_FUNC_SIZE];
#endif

#ifdef CONFIG_DOT11U_INTERWORKING
	STATE_MACHINE GASMachine;
	STATE_MACHINE_FUNC GASFunc[GAS_FUNC_SIZE];
#endif

#ifdef DOT11K_RRM_SUPPORT
	STATE_MACHINE BCNMachine;
	STATE_MACHINE_FUNC BCNFunc[BCN_FUNC_SIZE];
	STATE_MACHINE NRMachine;
	STATE_MACHINE_FUNC NRFunc[NR_FUNC_SIZE];
#endif

#ifdef CONFIG_DOT11V_WNM
	STATE_MACHINE BTMMachine;
	STATE_MACHINE_FUNC BTMFunc[BTM_FUNC_SIZE];
	STATE_MACHINE WNMNotifyMachine;
	STATE_MACHINE_FUNC WNMNotifyFunc[WNM_NOTIFY_FUNC_SIZE];
#endif

#ifdef CONFIG_AP_SUPPORT
	/* AP state machines */
#ifdef APCLI_SUPPORT
	STATE_MACHINE ApCliCtrlMachine;
	STATE_MACHINE ApCliWpaPskMachine;

	STATE_MACHINE_FUNC ApCliCtrlFunc[APCLI_CTRL_FUNC_SIZE];
#endif /* APCLI_SUPPORT */
	ULONG ChannelQuality;   /* 0..100, Channel Quality Indication for Roaming */
#endif /* CONFIG_AP_SUPPORT */
#ifdef WDS_SUPPORT
	STATE_MACHINE WdsMachine;
	STATE_MACHINE_FUNC WdsFunc[WDS_FUNC_SIZE];
#endif
	/* common WPA state machine */
	STATE_MACHINE WpaMachine;
	STATE_MACHINE_FUNC WpaFunc[WPA_FUNC_SIZE];


	ULONG Now32;		/* latch the value of NdisGetSystemUpTime() */
	ULONG LastSendNULLpsmTime;

	BOOLEAN bRunning;
	BOOLEAN suspend;
	NDIS_SPIN_LOCK TaskLock;
	MLME_QUEUE Queue;
	/*used for record jiffies than define as ULONG*/
	ULONG ShiftReg;

#ifdef BT_COEXISTENCE_SUPPORT
	RALINK_TIMER_STRUCT MiscDetectTimer;
#endif /* BT_COEXISTENCE_SUPPORT */
	RALINK_TIMER_STRUCT PeriodicTimer;
	RALINK_TIMER_STRUCT APSDPeriodicTimer;

	RALINK_TIMER_STRUCT LinkUpTimer;
#ifdef RTMP_MAC_PCI
	UCHAR bPsPollTimerRunning;
	RALINK_TIMER_STRUCT PsPollTimer;
	RALINK_TIMER_STRUCT RadioOnOffTimer;
#endif /* RTMP_MAC_PCI */
	ULONG PeriodicRound;
	ULONG GPIORound;
	ULONG OneSecPeriodicRound;

	UCHAR RealRxPath;
	BOOLEAN bLowThroughput;
	BOOLEAN bEnableAutoAntennaCheck;
	RALINK_TIMER_STRUCT RxAntEvalTimer;


#ifdef CONFIG_MULTI_CHANNEL

	BOOLEAN bStartMcc;
	BOOLEAN bpkt_dbg;

	UINT32 channel_1st_staytime;
	UINT32 channel_2nd_staytime;
	UINT32 switch_idle_time;
	UINT32 null_frame_count;

	UINT32 channel_1st_bw;
	UINT32 channel_2nd_bw;
	UINT32 channel_1st_primary_ch;
	UINT32 channel_2nd_primary_ch;
	UINT32 channel_1st_center_ch;
	UINT32 channel_2nd_center_ch;

	ULONG BeaconNow32;		/* latch the value of NdisGetSystemUpTime() */
#endif /* CONFIG_MULTI_CHANNEL */
	BOOLEAN bStartScc;
	struct notify_entry wsys_ne;
} MLME_STRUCT, *PMLME_STRUCT;

#ifdef DOT11_N_SUPPORT
/***************************************************************************
  *	802.11 N related data structures
  **************************************************************************/
struct reordering_mpdu;

struct reordering_list {
	struct reordering_mpdu *next;
	struct reordering_mpdu *tail;
	int qlen;
};

struct reordering_mpdu {
	struct reordering_mpdu *next;
	struct reordering_list AmsduList;
	PNDIS_PACKET pPacket;	/* coverted to 802.3 frame */
	int Sequence;		/* sequence number of MPDU */
	BOOLEAN bAMSDU;
	UCHAR					OpMode;
};

struct reordering_mpdu_pool {
	PVOID mem;
	NDIS_SPIN_LOCK lock;
	struct reordering_list freelist;
};

typedef enum _REC_BLOCKACK_STATUS {
	Recipient_NONE = 0,
	Recipient_USED,
	Recipient_HandleRes,
	Recipient_Initialization,
	Recipient_Established
} REC_BLOCKACK_STATUS, *PREC_BLOCKACK_STATUS;

typedef enum _ORI_BLOCKACK_STATUS {
	Originator_NONE = 0,
	Originator_USED,
	Originator_WaitRes,
	Originator_Done
} ORI_BLOCKACK_STATUS, *PORI_BLOCKACK_STATUS;

typedef struct _BA_ORI_ENTRY {
	UCHAR Wcid;
	UCHAR TID;
	UCHAR BAWinSize;
	UCHAR Token;
	UCHAR amsdu_cap;
	/* Sequence is to fill every outgoing QoS DATA frame's sequence field in 802.11 header. */
	USHORT Sequence;
	USHORT TimeOutValue;
	ORI_BLOCKACK_STATUS ORI_BA_Status;
	RALINK_TIMER_STRUCT ORIBATimer;
	PVOID pAdapter;
} BA_ORI_ENTRY, *PBA_ORI_ENTRY;

typedef struct _BA_REC_ENTRY {
	BOOLEAN check_amsdu_miss  ____cacheline_aligned;
	UINT8 PreviousAmsduState;
	UINT16 PreviousSN;
	UINT16 PreviousReorderCase;
	REC_BLOCKACK_STATUS REC_BA_Status;
	USHORT LastIndSeq;
	NDIS_SPIN_LOCK RxReRingLock;	/* Rx Ring spinlock */
	UCHAR BAWinSize;	/* 7.3.1.14. each buffer is capable of holding a max AMSDU or MSDU. */
	ULONG LastIndSeqAtTimer;
	ULONG nDropPacket;
	struct reordering_list list;
	struct reordering_mpdu *CurMpdu;
#define STEP_ONE 0
#define REPEAT 1
#define OLDPKT 2
#define WITHIN 3
#define SURPASS 4
	UCHAR Wcid;
	UCHAR TID;
	USHORT TimeOutValue;
	RALINK_TIMER_STRUCT RECBATimer;
	PVOID pAdapter;
} BA_REC_ENTRY, *PBA_REC_ENTRY;

typedef struct {
	ULONG numAsRecipient;	/* I am recipient of numAsRecipient clients. These client are in the BARecEntry[] */
	ULONG numAsOriginator;	/* I am originator of   numAsOriginator clients. These clients are in the BAOriEntry[] */
	ULONG numDoneOriginator;	/* count Done Originator sessions */
	BA_ORI_ENTRY BAOriEntry[MAX_LEN_OF_BA_ORI_TABLE];
	BA_REC_ENTRY BARecEntry[MAX_LEN_OF_BA_REC_TABLE];
	BOOLEAN ba_timeout_check;
#define BA_TIMEOUT_BITMAP_LEN (MAX_LEN_OF_BA_REC_TABLE/32)
	UINT32 ba_timeout_bitmap[BA_TIMEOUT_BITMAP_LEN];
	UINT16 ba_flush_one_time;
	UINT16 ba_flush_all_time;
} BA_TABLE, *PBA_TABLE;

/*For QureyBATableOID use; */
typedef struct GNU_PACKED _OID_BA_REC_ENTRY {
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR BaBitmap;		/* if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize */
	UCHAR rsv;
	UCHAR BufSize[8];
	REC_BLOCKACK_STATUS REC_BA_Status[8];
} OID_BA_REC_ENTRY, *POID_BA_REC_ENTRY;

/*For QureyBATableOID use; */
typedef struct GNU_PACKED _OID_BA_ORI_ENTRY {
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR BaBitmap;		/* if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize, read ORI_BA_Status[TID] for status */
	UCHAR rsv;
	UCHAR BufSize[8];
	ORI_BLOCKACK_STATUS ORI_BA_Status[8];
} OID_BA_ORI_ENTRY, *POID_BA_ORI_ENTRY;

typedef struct _QUERYBA_TABLE {
	OID_BA_ORI_ENTRY BAOriEntry[32];
	OID_BA_REC_ENTRY BARecEntry[32];
	UCHAR OriNum;		/* Number of below BAOriEntry */
	UCHAR RecNum;		/* Number of below BARecEntry */
} QUERYBA_TABLE, *PQUERYBA_TABLE;

typedef union _BACAP_STRUC {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32:4;
		UINT32 b2040CoexistScanSup:1;	/*As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz. */
		UINT32 bHtAdhoc:1;	/* adhoc can use ht rate. */
		UINT32 MMPSmode:2;	/* MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable */
		UINT32 AmsduSize:1;	/* 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]		= { 3839, 7935}; */
		UINT32 AmsduEnable:1;	/*Enable AMSDU transmisstion */
#ifdef RT_CFG80211_SUPPORT
		 UINT32 MpduDensity:3;
#else
		 UINT32 rsv1:3;
#endif
		UINT32 Policy:2;	/* 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use */
		UINT32 AutoBA:1;	/* automatically BA */
		UINT32 TxBAWinLimit:8;
		UINT32 RxBAWinLimit:8;
	} field;
#else
	struct {
		UINT32 RxBAWinLimit:8;
		UINT32 TxBAWinLimit:8;
		UINT32 AutoBA:1;	/* automatically BA */
		UINT32 Policy:2;	/* 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use */
#ifdef RT_CFG80211_SUPPORT
		 UINT32 MpduDensity:3;
#else
		 UINT32 rsv1:3;
#endif
		UINT32 AmsduEnable:1;	/*Enable AMSDU transmisstion */
		UINT32 AmsduSize:1;	/* 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]		= { 3839, 7935}; */
		UINT32 MMPSmode:2;	/* MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable */
		UINT32 bHtAdhoc:1;	/* adhoc can use ht rate. */
		UINT32 b2040CoexistScanSup:1;	/*As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz. */
		UINT32:4;
	} field;
#endif
	UINT32 word;
} BACAP_STRUC, *PBACAP_STRUC;

typedef struct {
	BOOLEAN IsRecipient;
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR TID;
	UCHAR nMSDU;
	USHORT TimeOut;
	BOOLEAN bAllTid;	/* If True, delete all TID for BA sessions with this MACaddr. */
} OID_ADD_BA_ENTRY, *POID_ADD_BA_ENTRY;

#define WLAN_MAX_NUM_OF_TIM		 ((MAX_LEN_OF_MAC_TABLE >> 3) + 1)   /* /8 + 1 */

enum BCN_TX_STATE {
	BCN_TX_UNINIT = 0,
	BCN_TX_IDLE = 1,
	BCN_TX_WRITE_TO_DMA = 2,
	BCN_TX_DMA_DONE = 3
};

typedef enum {
	PHY_IDLE = 0,
	PHY_INUSE = 1,
	PHY_RADIOOFF = 2,
} PHY_STATUS;

#ifdef MT_MAC
typedef struct _TIM_BUF_STRUCT {
	BOOLEAN bTimSntReq; /* used in if beacon send or stop */
	UCHAR TimBufIdx;
	PNDIS_PACKET TimPkt;
	/* PNDIS_PACKET TimPkt2; */
} TIM_BUF_STRUC;

INT wdev_tim_buf_init(RTMP_ADAPTER *pAd, TIM_BUF_STRUC *tim_info);
#endif

typedef struct _BCN_BUF_STRUCT {
	BOOLEAN bBcnSntReq; /* used in if beacon send or stop */
#ifdef RT_CFG80211_SUPPORT
    UCHAR BcnBufIdx; /* this value with meaning for RT chip only */
#endif
	enum BCN_TX_STATE bcn_state;	/* Make sure if no packet pending in the Hardware */
	PNDIS_PACKET BeaconPkt;
	UCHAR cap_ie_pos;
	struct wifi_dev *pWdev;/* point to associated wdev.*/

	UCHAR TimBitmaps[WLAN_MAX_NUM_OF_TIM];
	UINT16 TimIELocationInBeacon;
#ifdef MT_MAC
	TIM_BUF_STRUC tim_buf;
	UCHAR TimIELocationInTim;
#endif /* MT_MAC */

	UINT16 CsaIELocationInBeacon;

	NDIS_SPIN_LOCK BcnContentLock;
	UCHAR BcnUpdateMethod;
} BCN_BUF_STRUC;

#ifdef DOT11N_DRAFT3
typedef enum _BSS2040COEXIST_FLAG {
	BSS_2040_COEXIST_DISABLE = 0,
	BSS_2040_COEXIST_TIMER_FIRED = 1,
	BSS_2040_COEXIST_INFO_SYNC = 2,
	BSS_2040_COEXIST_INFO_NOTIFY = 4,
} BSS2040COEXIST_FLAG;

typedef struct _BssCoexChRange_ {
	UCHAR primaryCh;
	UCHAR secondaryCh;
	UCHAR effectChStart;
	UCHAR effectChEnd;
} BSS_COEX_CH_RANGE;
#endif /* DOT11N_DRAFT3 */

#define IS_VHT_STA(_pMacEntry)	(_pMacEntry->MaxHTPhyMode.field.MODE == MODE_VHT)
#define IS_HT_STA(_pMacEntry)	\
	(_pMacEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX)

#define IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#ifdef DOT11_VHT_AC
#define IS_VHT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE == MODE_VHT)
#endif
#define PEER_IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#endif /* DOT11_N_SUPPORT */

/*This structure is for all 802.11n card InterOptibilityTest action. Reset all Num every n second.  (Details see MLMEPeriodic) */
typedef struct _IOT_STRUC {
	BOOLEAN bRTSLongProtOn;
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN bLastAtheros;
	/* BOOLEAN bCurrentAtheros; */
	/* BOOLEAN bNowAtherosBurstOn; */
	/* BOOLEAN bNextDisableRxBA; */
	BOOLEAN bToggle;
#endif /* CONFIG_STA_SUPPORT */
} IOT_STRUC;

/* This is the registry setting for 802.11n transmit setting.  Used in advanced page. */
typedef union _REG_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 rsv:13;
		UINT32 EXTCHA:2;
		UINT32 HTMODE:1;
		UINT32 TRANSNO:2;
		UINT32 STBC:1;	/*SPACE */
		UINT32 ShortGI:1;
		UINT32 TxBF:1;	/* 3*3 */
		UINT32 ITxBfEn:1;
		UINT32 rsv0:10;
		/*UINT32  MCS:7;				 // MCS */
		/*UINT32  PhyMode:4; */
	} field;
#else
	struct {
		/*UINT32  PhyMode:4; */
		/*UINT32  MCS:7;				 // MCS */
		UINT32 rsv0:10;
		UINT32 ITxBfEn:1;
		UINT32 TxBF:1;
		UINT32 ShortGI:1;
		UINT32 STBC:1;	/*SPACE */
		UINT32 TRANSNO:2;
		UINT32 HTMODE:1;
		UINT32 EXTCHA:2;
		UINT32 rsv:13;
	} field;
#endif
	UINT32 word;
} REG_TRANSMIT_SETTING;

typedef union _DESIRED_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct {
		USHORT rsv:2;
		USHORT FixedTxMode:3;	/* If MCS isn't AUTO, fix rate in CCK, OFDM, HT or VHT mode. */
		USHORT PhyMode:4;
		USHORT MCS:7;	/* MCS */
	} field;
#else
	struct {
		USHORT MCS:7;
		USHORT PhyMode:4;
		USHORT FixedTxMode:3;
		USHORT rsv:2;
	} field;
#endif
	USHORT word;
} DESIRED_TRANSMIT_SETTING;

struct hw_setting {
	CHAR lan_gain;
};

/** @ingroup wifi_dev_system */
enum WDEV_TYPE {
	WDEV_TYPE_AP = (1 << 0),
	WDEV_TYPE_STA = (1 << 1),
	WDEV_TYPE_ADHOC = (1 << 2),
	WDEV_TYPE_WDS = (1 << 3),
	WDEV_TYPE_MESH = (1 << 4),
	WDEV_TYPE_GO = (1 << 5),
	WDEV_TYPE_GC = (1 << 6),
	/* WDEV_TYPE_APCLI = (1 << 7), */
	WDEV_TYPE_REPEATER = (1 << 8),
	WDEV_TYPE_P2P_DEVICE = (1 << 9),
	WDEV_TYPE_TDLS = (1 << 10),
};

#define WDEV_WITH_BCN_ABILITY(_wdev)	 (((_wdev)->wdev_type == WDEV_TYPE_AP) || \
		((_wdev)->wdev_type == WDEV_TYPE_GO) || \
		((_wdev)->wdev_type == WDEV_TYPE_MESH) || \
		((_wdev)->wdev_type == WDEV_TYPE_ADHOC))

enum BSS_INFO_DRIVER_MAINTIAN_STATE {
	BSS_INFO_INIT = 0,
	BSS_INFO_SEND_ENABLE = 1,
	BSS_INFO_SEND_DISABLE = 2,
};

enum MSDU_FORBID_REASON {
	MSDU_FORBID_CONNECTION_NOT_READY = 0,
	MSDU_FORBID_CHANNEL_MISMATCH = 1,
};

#define FLG_IS_OUTPUT 1
#define FLAG_IS_INPUT 0
#define MSDU_FORBID_SET(_wdev, _reason)	   (OS_SET_BIT(_reason, &((_wdev)->forbid_data_tx)))
#define MSDU_FORBID_CLEAR(_wdev, _reason)	 (OS_CLEAR_BIT(_reason, &((_wdev)->forbid_data_tx)))

typedef struct _RX_TA_TID_SEQ_MAPPING {
	UINT8   RxDWlanIdx;
	UINT8   MuarIdx;
	UINT16  TID_SEQ[8];
	UINT8   LatestTID;
} RX_TA_TID_SEQ_MAPPING, *PRX_TA_TID_SEQ_MAPPING;

#ifdef MAC_REPEATER_SUPPORT
typedef struct _RX_TRACKING_T {
	RX_TA_TID_SEQ_MAPPING   LastRxWlanIdx;
	UINT32  TriggerNum;
} RX_TRACKING_T, *PRX_TRACKING_T;
#endif /* MAC_REPEATER_SUPPORT */

struct _TX_BLK;

/**
 * @send_mlme_pkt: TX mlme pakcet en-queue pre handle that will call enq_data_pkt/enq_mgmt_pkt depend on which queue type that enqueue to
 * @send_data_pkt: TX data packet en-queue (TX sw queue) pre handle
 * @fp_send_data_pkt: TX data packet en-queue (TX sw queue) pre handle for fast path
 * @tx_pkt_allowed: early check for allow unicast/multicast packet to send or not
 * @fp_tx_pkt_allowed:  early check for allow unicast/multicast packet to send or not for fast path
 * @tx_pkt_handle: TX data per packet handle
 * @fill_offload_tx_blk: fill tx_blk information to build 802.11 header and hw descriptor header for txd offload frame
 * @fill_non_offload_tx_blk: fill tx_blk information to build 802.11 header and hw descriptor header for non-txd offload frame
 * @legacy_tx: build tx_blk, 802.11 header if not enable header translation and txd header then kick out to HIF
 * @ampdu_tx: same as legacy_tx except that can do cache operation (802.11 header cache)
 * @frag_tx: fragment TX data packet according to fragment threshold, fragment do not support header translation, so need to build 802.11 header
 * @amsdu_tx: aggrgation msdu packet into a-msdu packet
 * @mlme_mgmtq_tx: build tx_blk, and txd header and kick out to HIF for MLME frame that dequeue from management sw queue
 * @mlme_dataq_tx: build tx_blk, and txd header and kick out to HIF for MLME frame that dequeue from data sw queue
 * @ate_tx: build tx_blk, and txd header and kick out to HIF for ATE frames
 * @ieee_802_11_data_tx: build 802.11 header
 * @ieee_802_3_data_tx: for header translation disable case, do not need build 802.11 header any more
 * @rx_pkt_allowed: early check for allow unicast/multicast packet to receive or not
 * @ieee_802_11_data_rx: per 802.11 data packet handle (build RX_BLK, process 802.11 header nad  indicate to TCP layer or forward to other STA)
 * @ieee_802_3_data_rx: per 802.3 data packet handle (build RX_BLK and indicate to TCP layer or forward to other STA)
 * @ieee_802_11_ctl_rx: per 802.11 control packet handle (receive control frame and en-queue to MLME state machine)
 * @rx_pkt_forward: per 802.3 data packet forward handing
 * @find_cipher_algorithm: find encrypt key and decide cipher alogorithm
 * @detect_wmm_traffic: detect wmm traffic for purpose that determine if turn off txop or not
 */

struct wifi_dev_ops {
	/* TX */
	INT (*send_data_pkt)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt) ____cacheline_aligned;
	INT (*fp_send_data_pkt)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);
	INT (*send_mlme_pkt)(struct _RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, struct wifi_dev *wdev, UCHAR q_idx, BOOLEAN is_data_queue);
	INT (*tx_pkt_allowed)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);
	INT (*fp_tx_pkt_allowed)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);
	INT (*tx_pkt_handle)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*legacy_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*ampdu_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*amsdu_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*frag_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*mlme_mgmtq_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*mlme_dataq_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*ate_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	BOOLEAN (*fill_offload_tx_blk)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	BOOLEAN (*fill_non_offload_tx_blk)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	VOID (*ieee_802_11_data_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	VOID (*ieee_802_3_data_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);

	/* RX */
	INT (*rx_pkt_allowed)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *rx_blk) ____cacheline_aligned;
	INT (*rx_pkt_hdr_chk)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *rx_blk);
	INT (*rx_ps_handle)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *rx_blk);
	INT (*ieee_802_11_data_rx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
							   struct _RX_BLK *rx_blk, struct _MAC_TABLE_ENTRY *entry);
	INT (*ieee_802_3_data_rx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
							  struct _RX_BLK *rx_blk, struct _MAC_TABLE_ENTRY *entry);
	INT (*ieee_802_11_mgmt_rx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *rx_blk);
	INT (*ieee_802_11_ctl_rx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *rx_blk);
	INT (*rx_pkt_foward)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);
	VOID (*find_cipher_algorithm)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	VOID (*detect_wmm_traffic)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR up, UCHAR flg_is_output);
	INT (*rx_announce_802_3_pkt)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);
	INT (*rx_looping_detect)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);

	/* General */
	INT (*open)(struct wifi_dev *wdev) ____cacheline_aligned;
	INT (*close)(struct wifi_dev *wdev);
	INT (*linkup)(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
	INT (*linkdown)(struct wifi_dev *wdev);
	INT (*conn_act)(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
	INT (*disconn_act)(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
	VOID (*mac_entry_lookup)(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY **entry);

	BOOLEAN (*media_state_connected)(struct wifi_dev *wdev);
	INT (*ioctl)(void *net_dev_obj, void *rq_obj, INT cmd);

};

struct conn_sta_info {
	UINT16 nonerp_sta_cnt;
	UINT16 nongf_sta_cnt;
	UINT16 ht_bw20_sta_cnt;
};

/**
 * @wdev_idx: index refer from pAd->wdev_list[]
 * @func_idx: index refer to func_dev which pointer to
 * @tr_tb_idx: index refer to BSS which this device belong to
 * @wdev_ops: wifi device operation
 */

struct wifi_dev {
	PNET_DEV if_dev;
	VOID *func_dev;
	VOID *sys_handle;
	CHAR wdev_idx;	/* index refer from pAd->wdev_list[] */
	CHAR BssIdx;
	CHAR func_idx; /* index refer to func_dev which pointer to */
	CHAR func_type; /*Indicator to func type*/
	UCHAR tr_tb_idx; /* index refer to BSS which this device belong */
	UINT32 wdev_type;
	UCHAR PhyMode;
	UCHAR channel;
	UCHAR if_addr[MAC_ADDR_LEN];
	UCHAR bssid[MAC_ADDR_LEN];
	UCHAR hw_bssid_idx;
	BOOLEAN if_up_down_state;
	UINT8 OmacIdx;

	/* security segment */
	struct _SECURITY_CONFIG SecConfig;

	UCHAR PortSecured;

#ifdef DOT11R_FT_SUPPORT
	FT_CFG FtCfg;
#endif /* DOT11R_FT_SUPPORT */

	/* transmit segment */
	ULONG forbid_data_tx;  /* Use long, becasue we want to do atomic bit operation */
	BOOLEAN IgmpSnoopEnable; /* Only enabled for AP/WDS mode */
	RT_PHY_INFO DesiredHtPhyInfo;
	DESIRED_TRANSMIT_SETTING DesiredTransmitSetting;	/* Desired transmit setting. this is for reading registry setting only. not useful. */
	BOOLEAN bAutoTxRateSwitch;
	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode, MinHTPhyMode;	/* For transmit phy setting in TXWI. */

	/* 802.11 protocol related characters */
	BOOLEAN bWmmCapable;	/* 0:disable WMM, 1:enable WMM */
	UCHAR EdcaIdx;	/*mapping edca parameter to CommonCfg.APEdca[EdcaIdx]*/
	/* UAPSD information: such as enable or disable, do not remove */
	UAPSD_INFO UapsdInfo;

	/* for protocol layer using */
	UINT32 protection;

	/* tx burst */
	UINT32 prio_bitmap;
	UINT16 txop_level[MAX_PRIO_NUM];

	/* VLAN related */
	BOOLEAN bVLAN_Tag;
	USHORT VLAN_VID;
	USHORT VLAN_Priority;
	USHORT VLAN_Policy[2]; /*[0] for Tx , [1] for Rx*/

	struct wifi_dev_ops *wdev_ops;

	/* last received packet's SNR for each antenna */
	UCHAR LastSNR0;
	UCHAR LastSNR1;
	RSSI_SAMPLE RssiSample;
	ULONG NumOfAvgRssiSample;
	BOOLEAN bLinkUpDone;
	BOOLEAN bGotEapolPkt;
	struct _RX_BLK *pEapolPktFromAP;

#ifdef TXD_MGMT_TXPWR_CTRL
	UCHAR txd_txpwr_offset;
#endif /* TXD_MGMT_TXPWR_CTRL */

	/* 7636 psm */
	USHORT Psm;		/* power management mode   (PWR_ACTIVE|PWR_SAVE), Please use this value to replace  pAd->StaCfg[0].Psm in the future*/
	BOOLEAN bBeaconLost;
	BOOLEAN bTriggerRoaming;
	UINT8	ucDtimPeriod;
	UINT8	ucBeaconPeriod;
#if defined(RT_CFG80211_SUPPORT) || defined(HOSTAPD_SUPPORT)
	NDIS_HOSTAPD_STATUS Hostapd;
	BOOLEAN IsCFG1xWdev;
#endif
#ifdef MBSS_AS_WDS_AP_SUPPORT
	BOOLEAN wds_enable;
#endif

#ifdef RADIUS_MAC_AUTH_SUPPORT
	BOOLEAN radius_mac_auth_enable;
#endif /* RADIUS_MAC_AUTH_SUPPORT */

	UINT8   csa_count;
	BCN_BUF_STRUC bcn_buf;
	struct _BSS_INFO_ARGUMENT_T bss_info_argument;
	struct _DEV_INFO_CTRL_T DevInfo;
	VOID *pHObj;

	BOOLEAN fgRadioOnRequest;
#ifdef MAC_REPEATER_SUPPORT
	RX_TRACKING_T rx_tracking;
#endif /* MAC_REPEATER_SUPPORT */

	BOOLEAN fAnyStationPeekTpBound;
	struct dev_rate_info rate;
	/*wlan profile, use for configuration part.*/
	void *wpf_cfg;
	/*wlan profile, use for operating configurion, update by wcfg & mlme*/
	void *wpf_op;
	/* struct conn_sta_info conn_sta; */
	/* struct protection_cfg prot_cfg; */
	/* Flag migrate from pAd partially */
	UINT32 OpStatusFlags;
	BOOLEAN bAllowBeaconing; /* Device opened and ready for beaconing */
#ifdef DOT11K_RRM_SUPPORT
	RRM_CONFIG RrmCfg;
#endif /* DOT11K_RRM_SUPPORT */

#ifdef MBO_SUPPORT
	MBO_CTRL MboCtrl;
#endif /* MBO_SUPPORT */
#ifdef OCE_SUPPORT
	OCE_CTRL OceCtrl;
#endif /* OCE_SUPPORT */
	/* NEW FSM Segment
	 */
	SCAN_INFO ScanInfo;
	VOID *sync_fsm_ops;

	STATE_MACHINE assoc_machine;
	STATE_MACHINE_FUNC assoc_func[ASSOC_FUNC_SIZE];
	VOID *assoc_api;

	STATE_MACHINE cntl_machine;
	STATE_MACHINE_FUNC cntl_func[CNTL_FUNC_SIZE];
	VOID *cntl_api;

	STATE_MACHINE auth_machine;
	STATE_MACHINE_FUNC auth_func[AUTH_FSM_FUNC_SIZE];
	VOID *auth_api;

#ifdef WSC_INCLUDED
	/*
			WPS segment
	*/
	WSC_LV_INFO WscIEBeacon;
	WSC_LV_INFO WscIEProbeResp;
	WSC_CTRL WscControl;
	WSC_SECURITY_MODE WscSecurityMode;
#endif /* WSC_INCLUDED */
#ifdef BAND_STEERING
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN bInfReady;
#endif /* CONFIG_AP_SUPPORT */
#endif /* BAND_STEERING */
#ifdef WH_EVENT_NOTIFIER
	struct Custom_VIE custom_vie;
#endif /* WH_EVENT_NOTIFIER */
#ifdef CONFIG_MAP_SUPPORT
	MAP_CONFIG MAPCfg;
	BOOLEAN quick_ch_change;
#endif
	BOOLEAN is_marvell_ap;
	VOID *pDot11_H;
#ifdef REDUCE_TX_OVERHEAD
	UCHAR WmmIdx;
#endif
	VIE_CTRL vie_ctrl[VIE_FRM_TYPE_MAX];
#ifdef WAPP_SUPPORT
	BOOLEAN avoid_apcli_linkdown;
#endif
#ifdef MGMT_TXPWR_CTRL
	BOOLEAN bPwrCtrlEn;
	CHAR	TxPwrDelta;
	CHAR	MgmtTxPwr;
#endif
#ifdef WIFI_EAP_FEATURE
#ifdef AMPDU_CONF_SUPPORT
	UINT8 bRetryEnable;
	UINT8 bAMPDURetrynum;
#endif /* AMPDU_CONF_SUPPORT */
#endif /* WIFI_EAP_FEATURE */
	BOOLEAN bUseShortSlotTime;	/* 0: disable, 1 - use short slot (9us) */
	UINT8 SlotTimeValue;
};

struct greenap_ctrl {
	/* capability of rreenap */
	BOOLEAN cap;
	/* greenap_allow=TRUE only when AP or AP+AP case */
	BOOLEAN allow;
	/* suspend greenap operation ex. when do AP backdround scan */
	UINT32 suspend;
	NDIS_SPIN_LOCK lock;
};

struct greenap_on_off_ctrl {
	UINT8 band_idx;
	BOOLEAN reduce_core_power;
};

typedef struct _PWR_MGMT_STRUCT_ {
	USHORT		Psm;		/* power management mode   (PWR_ACTIVE|PWR_SAVE), Please use this value to replace  pAd->StaCfg[0].Psm in the future*/
	BOOLEAN	bBeaconLost;
	BOOLEAN	bTriggerRoaming;
	BOOLEAN	bEnterPsmNull;
	UINT8		ucDtimPeriod;
	UINT8		ucBeaconPeriod;
	/* Usign this wcid instead of pEntry->wcid for race condition. ex STA in PS --> BCN lost -->
	Linkdown --> Exit h/w LP with pEntry->wcid (pEntry might be NULL at this moment)*/
	UCHAR		ucWcid;
	BOOLEAN		 bDoze;
} PWR_MGMT_STRUCT, *PPWR_MGMT_STRUCT;


/***************************************************************************
  *	Multiple SSID related data structures
  **************************************************************************/
#define WLAN_CT_TIM_BCMC_OFFSET		0	/* unit: 32B */

/* clear bcmc TIM bit */
#define WLAN_MR_TIM_BCMC_CLEAR(apidx) \
	(pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] &= ~NUM_BIT8[0])

/* set bcmc TIM bit */
#define WLAN_MR_TIM_BCMC_SET(apidx) \
	(pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] |= NUM_BIT8[0])

#define WLAN_MR_TIM_BCMC_GET(apidx) \
	(pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] & NUM_BIT8[0])

/* clear a station PS TIM bit */
#define WLAN_MR_TIM_BIT_CLEAR(ad_p, apidx, _aid) \
	{	UCHAR tim_offset = _aid >> 3; \
		UCHAR bit_offset = _aid & 0x7; \
		ad_p->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimBitmaps[tim_offset] &= (~NUM_BIT8[bit_offset]); }

/* set a station PS TIM bit */
#define WLAN_MR_TIM_BIT_SET(ad_p, apidx, _aid) \
	{	UCHAR tim_offset = _aid >> 3; \
		UCHAR bit_offset = _aid & 0x7; \
		ad_p->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimBitmaps[tim_offset] |= NUM_BIT8[bit_offset]; }


#ifdef CONFIG_AP_SUPPORT
#ifdef MT_MAC
#define MAX_TIME_RECORD 5
#endif

#ifdef TXRX_STAT_SUPPORT

typedef struct _TXRX_STAT_BSS {
	LARGE_INTEGER TxDataPacketCount;
	LARGE_INTEGER TxDataPacketByte;
	LARGE_INTEGER TxDataPayloadByte;
	LARGE_INTEGER RxDataPacketCount;
	LARGE_INTEGER RxDataPacketByte;
	LARGE_INTEGER RxDataPayloadByte;
	LARGE_INTEGER TxDataPacketCountPerAC[4];	/*per access category*/
	LARGE_INTEGER RxDataPacketCountPerAC[4];	/*per access category*/
	LARGE_INTEGER TxUnicastDataPacket;
	LARGE_INTEGER TxMulticastDataPacket;
	LARGE_INTEGER TxBroadcastDataPacket;
	LARGE_INTEGER RxUnicastDataPacket;
	LARGE_INTEGER TxPacketDroppedCount;
	LARGE_INTEGER RxPacketDroppedCount;
	LARGE_INTEGER TxRetriedPacketCount;
	LARGE_INTEGER RxMICErrorCount;
	LARGE_INTEGER RxDecryptionErrorCount;
	LARGE_INTEGER TxMgmtPacketCount;
	LARGE_INTEGER TxMgmtOffChPktCount;
	LARGE_INTEGER RxMgmtPacketCount;
	LARGE_INTEGER LastSecTxBytes;
	LARGE_INTEGER LastSecRxBytes;
	HTTRANSMIT_SETTING LastMulticastTxRate;
	UINT32 LastPktStaWcid;
	UINT32 Last1SecPER;
	UINT32 Last1TxFailCnt;
	UINT32 Last1TxCnt;
} TXRX_STAT_BSS, *PTXRX_STAT_BSS;

#endif

typedef struct _BSS_STRUCT {
	struct wifi_dev wdev;

	INT mbss_idx;


#ifdef OCE_SUPPORT
	BOOLEAN ReducedNRListExist;
    REDUCED_NR_LIST_INFO   ReducedNRListInfo;
#endif /* OCE_SUPPORT */

	CHAR Ssid[MAX_LEN_OF_SSID+1];
	UCHAR SsidLen;
	BOOLEAN bHideSsid;

	USHORT CapabilityInfo;


	UCHAR MaxStaNum;	/* Limit the STA connection number per BSS */
	UCHAR StaCount;
	UINT16 StationKeepAliveTime;	/* unit: second */

	/*
		Security segment
	*/
#ifdef DISABLE_HOSTAPD_BEACON
	UINT8 RSNIE_ID[2];
#endif
	UCHAR RSNIE_Len[2];
	UCHAR RSN_IE[2][MAX_LEN_OF_RSNIE];

#ifdef HOSTAPD_OWE_SUPPORT
	UCHAR TRANSIE_Len;
	UCHAR TRANS_IE[MAX_LEN_OF_TRANS_IE];
#endif

	/* WPA */
	UCHAR GMK[32];
	UCHAR PSK[65];
	UCHAR PMK[LEN_MAX_PMK];
	UCHAR GTK[32];
	UCHAR GNonce[32];
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;

	/* for Group Rekey, AP ONLY */
	RT_WPA_REKEY WPAREKEY;
	ULONG REKEYCOUNTER;
	RALINK_TIMER_STRUCT REKEYTimer;
	UCHAR REKEYTimerRunning;
	UINT8 RekeyCountDown;

	/* For PMK Cache using, AP ONLY */
	ULONG PMKCachePeriod;	/* unit : jiffies */


	/*
		Transmitting segment
	*/
	UCHAR TxRate; /* RATE_1, RATE_2, RATE_5_5, RATE_11, ... */
	UCHAR DesiredRates[MAX_LEN_OF_SUPPORTED_RATES];	/* OID_802_11_DESIRED_RATES */
	UCHAR DesiredRatesIndex;
	UCHAR MaxTxRate; /* RATE_1, RATE_2, RATE_5_5, RATE_11 */

	/*
		Statistics segment
	*/
	/*MBSS_STATISTICS MbssStat;*/
	ULONG TxCount;
	ULONG RxCount;
	ULONG ReceivedByteCount;
	ULONG TransmittedByteCount;
	ULONG RxErrorCount;
	ULONG RxDropCount;

	ULONG TxErrorCount;
	ULONG TxDropCount;
	ULONG ucPktsTx;
	ULONG ucPktsRx;
	ULONG mcPktsTx;
	ULONG mcPktsRx;
	ULONG bcPktsTx;
	ULONG bcPktsRx;

	UCHAR BANClass3Data;
	ULONG IsolateInterStaTraffic;

#ifdef DOT11V_WNM_SUPPORT
	UCHAR DMSEntrycount;
	UCHAR totalDMScount;
#endif /* DOT11V_WNM_SUPPORT */

	RT_802_11_ACL AccessControlList;


	/* EDCA QoS */
	/*BOOLEAN bWmmCapable;*/	/* 0:disable WMM, 1:enable WMM */
	BOOLEAN bDLSCapable;	/* 0:disable DLS, 1:enable DLS */

	/*
	   Why need the parameter: 2009/09/22

	   1. iwpriv ra0 set WmmCapable=0
	   2. iwpriv ra0 set WirelessMode=9
	   3. iwpriv ra0 set WirelessMode=0
	   4. iwpriv ra0 set SSID=SampleAP

	   After the 4 commands, WMM is still enabled.
	   So we need the parameter to recover WMM Capable flag.

	   No the problem in station mode.
	 */
	BOOLEAN bWmmCapableOrg;	/* origin Wmm Capable in non-11n mode */
	/*
`		WPS segment
	*/
	WSC_LV_INFO WscIEBeacon;
	WSC_LV_INFO WscIEProbeResp;
#ifdef WSC_AP_SUPPORT
#ifdef RT_CFG80211_SUPPORT
	WSC_CTRL WscControl;
#endif
	WSC_SECURITY_MODE WscSecurityMode;
#endif /* WSC_AP_SUPPORT */

#ifdef IDS_SUPPORT
	UINT32 RcvdConflictSsidCount;
	UINT32 RcvdSpoofedAssocRespCount;
	UINT32 RcvdSpoofedReassocRespCount;
	UINT32 RcvdSpoofedProbeRespCount;
	UINT32 RcvdSpoofedBeaconCount;
	UINT32 RcvdSpoofedDisassocCount;
	UINT32 RcvdSpoofedAuthCount;
	UINT32 RcvdSpoofedDeauthCount;
	UINT32 RcvdSpoofedUnknownMgmtCount;
	UINT32 RcvdReplayAttackCount;

	CHAR RssiOfRcvdConflictSsid;
	CHAR RssiOfRcvdSpoofedAssocResp;
	CHAR RssiOfRcvdSpoofedReassocResp;
	CHAR RssiOfRcvdSpoofedProbeResp;
	CHAR RssiOfRcvdSpoofedBeacon;
	CHAR RssiOfRcvdSpoofedDisassoc;
	CHAR RssiOfRcvdSpoofedAuth;
	CHAR RssiOfRcvdSpoofedDeauth;
	CHAR RssiOfRcvdSpoofedUnknownMgmt;
	CHAR RssiOfRcvdReplayAttack;
#endif /* IDS_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
	WNM_CONFIG WnmCfg;
#endif /* DOT11V_WNM_SUPPORT */

	/* YF@20120417: Avoid connecting to AP in Poor Env, value 0 fOr disable. */
	CHAR AssocReqRssiThreshold;
	CHAR RssiLowForStaKickOut;

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	/*For AP vendor ie*/
	struct customer_vendor_ie ap_vendor_ie;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

#ifdef CONFIG_DOT11U_INTERWORKING
	GAS_CTRL GASCtrl;
#endif

#ifdef CONFIG_HOTSPOT
	HOTSPOT_CTRL HotSpotCtrl;
#endif

#ifdef CONFIG_DOT11V_WNM
	WNM_CTRL WNMCtrl;
#endif

#ifdef SPECIFIC_TX_POWER_SUPPORT
	CHAR TxPwrAdj;
#endif /* SPECIFIC_TX_POWER_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
	/* Extra IEs for (Re)Association Response provided by wpa_supplicant. E.g, WPS & P2P & WFD...etc */
	UCHAR AssocRespExtraIe[512];
	UINT32 AssocRespExtraIeLen;
#endif /* RT_CFG80211_SUPPORT */

#ifdef MT_MAC
	ULONG WriteBcnDoneTime[MAX_TIME_RECORD];
	ULONG BcnDmaDoneTime[MAX_TIME_RECORD];
	UCHAR bcn_not_idle_time;
	UINT32 bcn_recovery_num;
	ULONG TXS_TSF[MAX_TIME_RECORD];
	ULONG TXS_SN[MAX_TIME_RECORD];
	UCHAR timer_loop;
#endif /* MT_MAC */

#ifdef ROUTING_TAB_SUPPORT
	BOOLEAN bRoutingTabInit;
	UINT32 RoutingTabFlag;
	NDIS_SPIN_LOCK RoutingTabLock;
	ROUTING_ENTRY *pRoutingEntryPool;
	LIST_HEADER RoutingEntryFreeList;
	LIST_HEADER RoutingTab[ROUTING_HASH_TAB_SIZE];
#endif /* ROUTING_TAB_SUPPORT */

#ifdef A4_CONN
	UCHAR a4_init;
	NDIS_SPIN_LOCK a4_entry_lock;
	DL_LIST a4_entry_list;
#endif /* A4_CONN */
#ifdef WAPP_SUPPORT
	UCHAR ESPI_AC_BE[3];
	UCHAR ESPI_AC_BK[3];
	UCHAR ESPI_AC_VO[3];
	UCHAR ESPI_AC_VI[3];
#endif
#ifdef TXRX_STAT_SUPPORT
	TXRX_STAT_BSS stat_bss;
#endif

	struct conn_sta_info conn_sta;
} BSS_STRUCT;

#endif /* CONFIG_AP_SUPPORT */

#ifdef WAPP_SUPPORT
struct BSS_LOAD_INFO {
	UINT8 current_status[DBDC_BAND_NUM];
	UINT8 current_load[DBDC_BAND_NUM];
	UINT8 high_thrd[DBDC_BAND_NUM];
	UINT8 low_thrd[DBDC_BAND_NUM];
};
#endif /* WAPP_SUPPORT */


/* configuration common to OPMODE_AP as well as OPMODE_STA */
typedef struct _COMMON_CONFIG {
	BOOLEAN bCountryFlag;
	UCHAR CountryCode[4];
#ifdef EXT_BUILD_CHANNEL_LIST
	UCHAR Geography;
	UCHAR DfsType;
	PUCHAR pChDesp;
#endif /* EXT_BUILD_CHANNEL_LIST */
	PUCHAR pChDesc2G;
	PUCHAR pChDesc5G;
	UCHAR CountryRegion;	/* Enum of country region, 0:FCC, 1:IC, 2:ETSI, 3:SPAIN, 4:France, 5:MKK, 6:MKK1, 7:Israel */
	UCHAR CountryRegionForABand;	/* Enum of country region for A band */
	UCHAR cfg_wmode;
	UCHAR SavedPhyMode;
	USHORT Dsifs;		/* in units of usec */
	ULONG PacketFilter;	/* Packet filter for receiving */
	/* UINT8 RegulatoryClass[MAX_NUM_OF_REGULATORY_CLASS]; *//* unify to using get_regulatory_class from driver table */

	USHORT BeaconPeriod;
	/* Channel Group*/
	UCHAR ChGrpEn;
	UCHAR ChGrpChannelList[MAX_NUM_OF_CHANNELS];
	UCHAR ChGrpChannelNum;

	UCHAR ExpectedACKRate[MAX_LEN_OF_SUPPORTED_RATES];

	ULONG BasicRateBitmap;	/* backup basic ratebitmap */
	ULONG BasicRateBitmapOld;	/* backup basic ratebitmap */
#ifdef GN_MIXMODE_SUPPORT
	BOOLEAN GNMixMode;
#endif /*GN_MIXMODE_SUPPORT*/

	BOOLEAN bInServicePeriod;

	UCHAR EtherTrafficBand;
	UCHAR WfFwdDisabled;


	BOOLEAN bAPSDAC_BE;
	BOOLEAN bAPSDAC_BK;
	BOOLEAN bAPSDAC_VI;
	BOOLEAN bAPSDAC_VO;

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	BOOLEAN	 TDLS_bAPSDAC_BE;
	BOOLEAN	 TDLS_bAPSDAC_BK;
	BOOLEAN	 TDLS_bAPSDAC_VI;
	BOOLEAN	 TDLS_bAPSDAC_VO;
	UCHAR TDLS_MaxSPLength;
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

	/* because TSPEC can modify the APSD flag, we need to keep the APSD flag
	   requested in association stage from the station;
	   we need to recover the APSD flag after the TSPEC is deleted. */
	BOOLEAN bACMAPSDBackup[4];	/* for delivery-enabled & trigger-enabled both */
	BOOLEAN bACMAPSDTr[4];	/* no use */
	UCHAR MaxSPLength;

	BOOLEAN bNeedSendTriggerFrame;
	BOOLEAN bAPSDForcePowerSave;	/* Force power save mode, should only use in APSD-STAUT */
	ULONG TriggerTimerCount;
	REG_TRANSMIT_SETTING RegTransmitSetting;	/*registry transmit setting. this is for reading registry setting only. not useful. */

	UCHAR TxRate;		/* Same value to fill in TXD. TxRate is 6-bit */
	UCHAR MaxTxRate;	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR TxRateIndex;	/* Tx rate index in Rate Switch Table */
	UCHAR MinTxRate;	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR RtsRate;		/* RATE_xxx */
	UCHAR MlmeRate;		/* RATE_xxx, used to send MLME frames */
	UCHAR BasicMlmeRate;	/* Default Rate for sending MLME frames */

	UCHAR TxPower;		/* in unit of mW */
	UINT8 ucTxPowerPercentage[DBDC_BAND_NUM];	/* 0~100 % */

	UCHAR PowerUpenable[DBDC_BAND_NUM];
	CHAR cPowerUpHt20[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
	CHAR cPowerUpHt40[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
	CHAR cPowerUpVht20[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
	CHAR cPowerUpVht40[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
	CHAR cPowerUpVht80[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
	CHAR cPowerUpVht160[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];

#if defined(MT7626) || defined(AXE) || defined(MT7915)
	CHAR cPowerUpCck[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
	CHAR cPowerUpOfdm[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
	CHAR cPowerUpHe26[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
	CHAR cPowerUpHe52[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
	CHAR cPowerUpHe106[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
	CHAR cPowerUpHe242[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
	CHAR cPowerUpHe484[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
	CHAR cPowerUpHe996[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
	CHAR cPowerUpHe996X2[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
#else
	CHAR cPowerUpCckOfdm[DBDC_BAND_NUM][POWER_UP_CATEGORY_RATE_NUM];
#endif

	UCHAR SKUenable[DBDC_BAND_NUM];
	UCHAR SKUTableIdx;
	CHAR  cTxPowerCompBackup[DBDC_BAND_NUM][SKU_TABLE_SIZE][SKU_TX_SPATIAL_STREAM_NUM];
	UCHAR PERCENTAGEenable[DBDC_BAND_NUM];
	UCHAR BFBACKOFFenable[DBDC_BAND_NUM];
	UINT8 CCKTxStream[DBDC_BAND_NUM];

#ifdef LINK_TEST_SUPPORT
	UCHAR LinkTestSupport;
	UCHAR LinkTestSupportTemp[DBDC_BAND_NUM];
#endif /* LINK_TEST_SUPPORT */

	UCHAR ucEDCCACtrl[DBDC_BAND_NUM];

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
	UCHAR CalCacheApply;
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

	UINT8 ThermalRecalMode;
	UINT8 ucTxPowerDefault[DBDC_BAND_NUM];	/* keep for TxPowerPercentage */
	UINT8 PwrConstraint;

#ifdef DOT11_N_SUPPORT
	BACAP_STRUC BACapability;	/*   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0 */
	BACAP_STRUC REGBACapability;	/*   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0 */
#endif /* DOT11_N_SUPPORT */

	BOOLEAN dbdc_mode;
	ENUM_DBDC_MODE eDBDC_mode;

#ifdef DOT11_VHT_AC
	BOOLEAN force_vht;
	UCHAR vht_cent_ch;
	UCHAR vht_cent_ch2;
	UCHAR vht_mcs_cap;
	UCHAR vht_nss_cap;
	USHORT vht_tx_hrate;
	USHORT vht_rx_hrate;
	BOOLEAN ht20_forbid;
	BOOLEAN g_band_256_qam;
	BOOLEAN bUseVhtRateFor2g;
#ifdef VHT_MU_MIMO
	BOOLEAN vht_mu_mimo;
#endif /* VHT_MU_MIMO */
#endif /* DOT11_VHT_AC */

	IOT_STRUC IOTestParm;	/* 802.11n InterOpbility Test Parameter; */
	ULONG TxPreamble;	/* Rt802_11PreambleLong, Rt802_11PreambleShort, Rt802_11PreambleAuto */
	BOOLEAN bUseZeroToDisableFragment;	/* Microsoft use 0 as disable */
	ULONG UseBGProtection;	/* 0: auto, 1: always use, 2: always not use */
	BOOLEAN bUseShortSlotTime;	/* 0: disable, 1 - use short slot (9us) */
	BOOLEAN bEnableTxBurst;	/* 1: enble TX PACKET BURST (when BA is established or AP is not a legacy WMM AP), 0: disable TX PACKET BURST */
	BOOLEAN bAggregationCapable;	/* 1: enable TX aggregation when the peer supports it */
	BOOLEAN bPiggyBackCapable;	/* 1: enable TX piggy-back according MAC's version */
	BOOLEAN bIEEE80211H;	/* 1: enable IEEE802.11h spec. */
	UCHAR RDDurRegion; /* Region of radar detection */
	ULONG DisableOLBCDetect;	/* 0: enable OLBC detect; 1 disable OLBC detect */

#ifdef TPC_SUPPORT
	BOOLEAN b80211TPC;
#endif /* TPC_SUPPORT */

#ifdef DOT11_N_SUPPORT
	BOOLEAN bRdg;
#endif /* DOT11_N_SUPPORT */
	QOS_CAPABILITY_PARM APQosCapability;	/* QOS capability of the current associated AP */
	EDCA_PARM APEdcaParm[WMM_NUM];	/* EDCA parameters of the current associated AP */
	QBSS_LOAD_PARM APQbssLoad;	/* QBSS load of the current associated AP */
	UCHAR AckPolicy[WMM_NUM_OF_AC];	/* ACK policy of the specified AC. see ACK_xxx */
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN bDLSCapable;	/* 0:disable DLS, 1:enable DLS */
#endif /* CONFIG_STA_SUPPORT */
	/* a bitmap of BOOLEAN flags. each bit represent an operation status of a particular */
	/* BOOLEAN control, either ON or OFF. These flags should always be accessed via */
	/* OPSTATUS_TEST_FLAG(), OPSTATUS_SET_FLAG(), OP_STATUS_CLEAR_FLAG() macros. */
	/* see fOP_STATUS_xxx in RTMP_DEF.C for detail bit definition */
	ULONG OpStatusFlags;

	BOOLEAN NdisRadioStateOff;	/*For HCT 12.0, set this flag to TRUE instead of called MlmeRadioOff. */
	ABGBAND_STATE BandState;		/* For setting BBP used on B/G or A mode. */


#ifdef MT_DFS_SUPPORT
	DFS_PARAM DfsParameter;
#endif

#ifdef CARRIER_DETECTION_SUPPORT
	CARRIER_DETECTION_STRUCT CarrierDetect;
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef DOT11_N_SUPPORT
	/* HT */
	HT_CAPABILITY_IE HtCapability;
	/*This IE is used with channel switch announcement element when changing to a new 40MHz. */
	/*This IE is included in channel switch ammouncement frames 7.4.1.5, beacons, probe Rsp. */
	NEW_EXT_CHAN_IE NewExtChanOffset;	/*7.3.2.20A, 1 if extension channel is above the control channel, 3 if below, 0 if not present */

	EXT_CAP_INFO_ELEMENT ExtCapIE;	/* this is the extened capibility IE appreed in MGMT frames. Doesn't need to update once set in Init. */

#ifdef DOT11N_DRAFT3
	BOOLEAN bBssCoexEnable;
	/*
	   Following two paramters now only used for the initial scan operation. the AP only do
	   bandwidth fallback when BssCoexApCnt > BssCoexApCntThr
	   By default, the "BssCoexApCntThr" is set as 0 in "UserCfgInit()".
	 */
	UCHAR BssCoexApCntThr;
	UCHAR BssCoexApCnt;

	UCHAR Bss2040CoexistFlag;	/* bit 0: bBssCoexistTimerRunning, bit 1: NeedSyncAddHtInfo. */
	RALINK_TIMER_STRUCT Bss2040CoexistTimer;
	UCHAR Bss2040NeedFallBack;	/* 1: Need Fall back to 20MHz */

	/*This IE is used for 20/40 BSS Coexistence. */
	BSS_2040_COEXIST_IE BSS2040CoexistInfo;

	USHORT Dot11OBssScanPassiveDwell;	/* Unit : TU. 5~1000 */
	USHORT Dot11OBssScanActiveDwell;	/* Unit : TU. 10~1000 */
	USHORT Dot11BssWidthTriggerScanInt;	/* Unit : Second */
	USHORT Dot11OBssScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000 */
	USHORT Dot11OBssScanActiveTotalPerChannel;	/* Unit : TU. 20~10000 */
	USHORT Dot11BssWidthChanTranDelayFactor;
	USHORT Dot11OBssScanActivityThre;	/* Unit : percentage */

	ULONG Dot11BssWidthChanTranDelay;	/* multiple of (Dot11BssWidthTriggerScanInt * Dot11BssWidthChanTranDelayFactor) */
	ULONG CountDownCtr;	/* CountDown Counter from (Dot11BssWidthTriggerScanInt * Dot11BssWidthChanTranDelayFactor) */

	BSS_2040_COEXIST_IE LastBSSCoexist2040;
	BSS_2040_COEXIST_IE BSSCoexist2040;
	TRIGGER_EVENT_TAB TriggerEventTab;
	UCHAR ChannelListIdx;

	BOOLEAN bOverlapScanning;
	BOOLEAN bBssCoexNotify;
#endif /* DOT11N_DRAFT3 */

	BOOLEAN bSeOff;
	UINT8   ucAntennaIndex;

	BOOLEAN bMIMOPSEnable;
	BOOLEAN bBADecline;
	BOOLEAN bDisableReordering;
	BOOLEAN bForty_Mhz_Intolerant;
	BOOLEAN bExtChannelSwitchAnnouncement;
	BOOLEAN bRcvBSSWidthTriggerEvents;
	ULONG LastRcvBSSWidthTriggerEventsTime;

	UCHAR TxBASize;

	BOOLEAN bRalinkBurstMode;
	UINT32 RestoreBurstMode;
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
	UINT32 cfg_vht;
	VHT_CAP_INFO vht_info;
	VHT_CAP_IE vht_cap_ie;
	BOOLEAN bNonVhtDisallow; /* Disallow non-VHT connection */
#endif /* DOT11_VHT_AC */

#ifdef SYSTEM_LOG_SUPPORT
	/* Enable wireless event */
	BOOLEAN bWirelessEvent;
#endif /* SYSTEM_LOG_SUPPORT */

	BOOLEAN bWiFiTest;	/* Enable this parameter for WiFi test */

	/* Tx & Rx Stream number selection */
	UCHAR TxStream;
	UCHAR RxStream;

	/* transmit phy mode, trasmit rate for Multicast. */
	/*
	#ifdef MCAST_RATE_SPECIFIC
		UCHAR McastTransmitMcs;
		UCHAR McastTransmitPhyMode;
	#endif // MCAST_RATE_SPECIFIC
	*/
	BOOLEAN bHardwareRadio;	/* Hardware controlled Radio enabled */


#ifdef WSC_INCLUDED
	/* WSC hardware push button function 0811 */
	UINT8 WscHdrPshBtnCheckCount;
#endif /* WSC_INCLUDED */


	NDIS_SPIN_LOCK MeasureReqTabLock;
	PMEASURE_REQ_TAB pMeasureReqTab;

	NDIS_SPIN_LOCK TpcReqTabLock;
	PTPC_REQ_TAB pTpcReqTab;

	/* transmit phy mode, trasmit rate for Multicast. */

#ifdef SINGLE_SKU
	UINT16 DefineMaxTxPwr;
	BOOLEAN bSKUMode;
	UINT16 AntGain;
	UINT16 BandedgeDelta;
	UINT16 ModuleTxpower;
#endif /* SINGLE_SKU */

#ifdef RTMP_RBUS_SUPPORT
	ULONG CID;
	ULONG CN;
#endif /* RTMP_RBUS_SUPPORT */

	BOOLEAN HT_DisallowTKIP;	/* Restrict the encryption type in 11n HT mode */
#ifdef DOT11K_RRM_SUPPORT
	BOOLEAN VoPwrConstraintTest;
#endif /* DOT11K_RRM_SUPPORT */

	BOOLEAN HT_Disable;	/* 1: disable HT function; 0: enable HT function */

#if defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT)
	USHORT	lowTrafficThrd;		/* Threshold for reverting to default MCS when traffic is low */
	SHORT	TrainUpRuleRSSI;	/* If TrainUpRule=2 then use Hybrid rule when RSSI < TrainUpRuleRSSI */
	USHORT	TrainUpLowThrd;		/* QuickDRS Hybrid train up low threshold */
	USHORT	TrainUpHighThrd;	/* QuickDRS Hybrid train up high threshold */
	BOOLEAN	TrainUpRule;		/* QuickDRS train up criterion: 0=>Throughput, 1=>PER, 2=> Throughput & PER */
#endif /* defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT) */

#ifdef STREAM_MODE_SUPPORT
#define		STREAM_MODE_STA_NUM		4

	UCHAR	StreamMode; /* 0=disabled, 1=enable for 1SS, 2=enable for 2SS, 3=enable for 1,2SS */
	UCHAR	StreamModeMac[STREAM_MODE_STA_NUM][MAC_ADDR_LEN];
	UINT16	StreamModeMCS;	/* Bit map for enabling Stream Mode based on MCS */
#endif /* STREAM_MODE_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	ULONG ITxBfTimeout;
	ULONG ETxBfTimeout;
	ULONG	ETxBfEnCond;		/* Enable sending of sounding and beamforming */
	ULONG	MUTxRxEnable;		/* Enable MUTxRxEnable */
	BOOLEAN	ETxBfNoncompress;	/* Force non-compressed Sounding Response */
	BOOLEAN	ETxBfIncapable;		/* Report Incapable of BF in TX BF Capabilities */
#ifdef TXBF_DYNAMIC_DISABLE
	UINT_8  ucAutoSoundingCtrl; /* Initial AutoSoundingCtrl for rStaRecBf */
#endif /* TXBF_DYNAMIC_DISABLE */
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */

#ifdef RTMP_MAC_PCI
	BOOLEAN bPCIeBus;	/* The adapter runs over PCIe bus */
#endif /* RTMP_MAC_PCI */

#ifdef WSC_INCLUDED
	BOOLEAN WscPBCOverlap;
	WSC_STA_PBC_PROBE_INFO WscStaPbcProbeInfo;
#endif /* WSC_INCLUDED */

#ifdef CONFIG_ZTE_RADIO_ONOFF
	BOOLEAN bRadioEnable;
#endif /* CONFIG_ZTE_RADIO_ONOFF */

#ifdef MICROWAVE_OVEN_SUPPORT
	MO_CFG_STRUCT MO_Cfg;	/* data structure for mitigating microwave interference */
#endif /* MICROWAVE_OVEN_SUPPORT */


	/* TODO: need to integrate with MICROWAVE_OVEN_SUPPORT */
#ifdef DYNAMIC_VGA_SUPPORT
	LNA_VGA_CTL_STRUCT lna_vga_ctl;
#endif /* DYNAMIC_VGA_SUPPORT */

	BOOLEAN bStopReadTemperature; /* avoid race condition between FW/driver */
	BOOLEAN bTXRX_RXV_ON;
	BOOLEAN ManualTxop;
	ULONG ManualTxopThreshold;
	UCHAR ManualTxopUpBound;
	UCHAR ManualTxopLowBound;
#ifdef REDUCE_TCP_ACK_SUPPORT
	UINT32 ReduceAckEnable;
	UINT32 ReduceAckProbability;
	UINT32 ReduceAckTimeout;
	UINT32 ReduceAckCnxTimeout;
#endif
#ifdef WIFI_EAP_FEATURE
#ifdef ACK_CTS_TIMEOUT_SUPPORT
	UINT32 bDistance;
#endif /* AMPDU_CONF_SUPPORT */
#endif /* WIFI_EAP_FEATURE */

} COMMON_CONFIG, *PCOMMON_CONFIG;

#ifdef CFG_TDLS_SUPPORT
typedef struct _CFG_TDLS_STRUCT {
	/* For TPK handshake */
	UCHAR			ANonce[32];	/* Generated in Message 1, random variable */
	UCHAR			SNonce[32];	/* Generated in Message 2, random variable */
	ULONG			KeyLifetime;	/*  Use type= 'Key Lifetime Interval' unit: Seconds, min lifetime = 300 seconds */
	UCHAR			TPK[LEN_PMK];	/* TPK-KCK(16 bytes) for MIC + TPK-TP (16 bytes) for data */
	UCHAR			TPKName[LEN_PMK_NAME];
	BOOLEAN			IneedKey;
	BOOLEAN			bCfgTDLSCapable; /* 0:disable TDLS, 1:enable TDLS  ; using supplicant sm */
	BOOLEAN			TdlsChSwitchSupp;
	BOOLEAN		TdlsPsmSupp;

	UINT8			TdlsLinkCount;
	UINT8			TdlsDialogToken;
	CFG_TDLS_ENTRY		TDLSEntry[MAX_NUM_OF_CFG_TDLS_ENTRY];
	/* Channel Switch */
	UCHAR			CHSWPeerMacAddr[MAC_ADDR_LEN];
	BOOLEAN				bDoingPeriodChannelSwitch;
	BOOLEAN					IamInOffChannel;
	USHORT					ChSwitchTime;
	USHORT					ChSwitchTimeout;
	UINT					BaseChannelStayTime;
	UINT					OffChannelStayTime;
	RALINK_TIMER_STRUCT	BaseChannelSwitchTimer;		/* Use to channel switch */
	USHORT					BaseChannel;
	USHORT					BaseChannelBW;
	USHORT					OrigTargetOffChannel;
	USHORT					TargetOffChannel;
	USHORT					TargetOffChannelBW;
	USHORT					TargetOffChannelExt;
	BOOLEAN					bChannelSwitchInitiator;
	BOOLEAN					IsentCHSW;
} CFG_TDLS_STRUCT, *PCFG_TDLS_STRUCT;
#endif /* CFG_TDLS_SUPPORT */

#if defined(APCLI_CFG80211_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT)
typedef struct _WPA_SUPPLICANT_INFO {
	/*
		802.1x WEP + MD5 will set key to driver before assoc, but we need to apply the key to
		ASIC after get EAPOL-Success frame, so we use this flag to indicate that
	*/
	BOOLEAN IEEE8021x_required_keys;
	CIPHER_KEY DesireSharedKey[4];	/* Record user desired WEP keys */
	UCHAR DesireSharedKeyId;

	/* 0x00: driver ignores wpa_supplicant */
	/* 0x01: wpa_supplicant initiates scanning and AP selection */
	/* 0x02: driver takes care of scanning, AP selection, and IEEE 802.11 association parameters */
	/* 0x80: wpa_supplicant trigger driver to do WPS */
	UCHAR WpaSupplicantUP;
	UCHAR WpaSupplicantScanCount;
	BOOLEAN bRSN_IE_FromWpaSupplicant;
	BOOLEAN bLostAp;
	UCHAR *pWpsProbeReqIe;
	UINT WpsProbeReqIeLen;
	UCHAR *pWpaAssocIe;
	UINT WpaAssocIeLen;
#ifdef CFG_TDLS_SUPPORT
	CFG_TDLS_STRUCT CFG_Tdls_info;
#endif
} WPA_SUPPLICANT_INFO;
#endif /* WPA_SUPPLICANT_SUPPORT  || APCLI_CFG80211_SUPPORT */

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)

#ifdef CREDENTIAL_STORE
typedef struct _STA_CONNECT_INFO {
	BOOLEAN Changeable;
	BOOLEAN IEEE8021X;
	CHAR Ssid[MAX_LEN_OF_SSID]; /* NOT NULL-terminated */
	UCHAR SsidLen; /* the actual ssid length in used */
	NDIS_802_11_AUTHENTICATION_MODE AuthMode; /* This should match to whatever microsoft defined */
	NDIS_802_11_WEP_STATUS WepStatus;
	UCHAR DefaultKeyId;
	UCHAR PMK[LEN_MAX_PMK]; /* WPA PSK mode PMK */
	UCHAR WpaPassPhrase[64]; /* WPA PSK pass phrase */
	UINT WpaPassPhraseLen; /* the length of WPA PSK pass phrase */
	UINT8 WpaState;
	CIPHER_KEY SharedKey[1][4]; /* STA always use SharedKey[BSS0][0..3] */
	NDIS_SPIN_LOCK Lock;
} STA_CONNECT_INFO, *P_STA_CONNECT_INFO;
#endif /* CREDENTIAL_STORE */


typedef struct _apcli_inf_stat {
	BOOLEAN ApCliInit;	/* Set it as 1 if ApCli is initialized */
	BOOLEAN Enable;		/* Set it as 1 if the apcli interface was configured to "1"  or by iwpriv cmd "ApCliEnable" */
	BOOLEAN Valid;		/* Set it as 1 if the apcli interface associated success to remote AP. */
	/* recv beacon part..... */
	ULONG ApCliRcvBeaconTime_MlmeEnqueueForRecv;
	ULONG ApCliRcvBeaconTime_MlmeEnqueueForRecv_2;
	ULONG ApCliRcvBeaconTime;
	BOOLEAN bPeerExist; /* TRUE if we hear Root AP's beacon */

	ULONG ApCliLinkUpTime;
	ULONG LinkDownReason;
	ULONG Disconnect_Sub_Reason;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	USHORT	ProbeReqCnt;
	BOOLEAN AutoConnectFlag;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	USHORT AuthReqCnt;
	USHORT AssocReqCnt;
	RTMP_OS_COMPLETION ifdown_complete;
	RTMP_OS_COMPLETION linkdown_complete;
} APCLI_INF_STAT, *PAPCLI_INF_STAT;

typedef struct _stat_counters {
	ULONG TxCount;
	ULONG RxCount;
	ULONG ReceivedByteCount;
	ULONG TransmittedByteCount;
	ULONG RxErrorCount;
	ULONG RxDropCount;

	ULONG TxErrorCount;
	ULONG TxDropCount;
	ULONG ucPktsTx;
	ULONG ucPktsRx;
	ULONG mcPktsTx;
	ULONG mcPktsRx;
	ULONG bcPktsTx;
	ULONG bcPktsRx;

	ULONG OneSecTxBytes;
	ULONG OneSecRxBytes;
} STAT_COUNTERS, *PSTAT_COUNTERS;

/*
	This data structure keep the current active BSS/IBSS's configuration that
	this STA had agreed upon joining the network. Which means these parameters
	are usually decided by the BSS/IBSS creator instead of user configuration.
	Data in this data structurre is valid only when either ADHOC_ON()/INFRA_ON()
	is TRUE. Normally, after SCAN or failed roaming attempts, we need to
	recover back to the current active settings
*/
typedef struct _STA_ACTIVE_CONFIG {
	USHORT Aid;
	USHORT AtimWin;		/* in kusec; IBSS parameter set element */
	USHORT CapabilityInfo;
	EXT_CAP_INFO_ELEMENT ExtCapInfo;
	USHORT CfpMaxDuration;
	USHORT CfpPeriod;

	/* Copy supported rate from desired AP's beacon. We are trying to match */
	/* AP's supported and extended rate settings. */
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen;
	UCHAR ExtRateLen;
	/* Copy supported ht from desired AP's beacon. We are trying to match */
	RT_PHY_INFO SupportedPhyInfo;
	RT_HT_CAPABILITY SupportedHtPhy;
#ifdef DOT11_VHT_AC
	RT_VHT_CAP	SupVhtCap;
#endif /* DOT11_VHT_AC */
	UCHAR RxMcsSet[16];
} STA_ACTIVE_CONFIG;

struct adhoc_info {
	USHORT AtimWin;		/* used when starting a new IBSS */
	BCN_BUF_STRUC bcn_buf;
#ifdef DOT11_N_SUPPORT
	BOOLEAN bAdhocN;
#endif /* DOT11_N_SUPPORT */
	BOOLEAN bAdhocCreator;	/*TRUE indicates divice is Creator. */

#ifdef XLINK_SUPPORT
	BOOLEAN PSPXlink;	/* 0: Disable. 1: Enable */
#endif /* XLINK_SUPPORT */
};

#define NET_DEV_NAME_MAX_LENGTH	16

/* Modified by Wu Xi-Kun 4/21/2006 */
/* STA configuration and status */
struct _STA_ADMIN_CONFIG {
	struct wifi_dev wdev;

	/*
		GROUP 1 -
		User configuration loaded from Registry, E2PROM or OID_xxx. These settings describe
		the user intended configuration, but not necessary fully equal to the final
		settings in ACTIVE BSS after negotiation/compromize with the BSS holder (either
		AP or IBSS holder).
		Once initialized, user configuration can only be changed via OID_xxx
	*/
	UCHAR BssType;		/* BSS_INFRA or BSS_ADHOC */

	/*APPS DVT MSP*/
	BOOLEAN PwrSaveSet;

	struct adhoc_info adhocInfo;
	STA_ACTIVE_CONFIG StaActive;	/* valid only when ADHOC_ON(pAd) || INFRA_ON(pAd) */
	MLME_AUX MlmeAux;	/* temporary settings used during MLME state machine */

	CHAR dev_name[NET_DEV_NAME_MAX_LENGTH];
	USHORT OriDevType;
	CHAR Ssid[MAX_LEN_OF_SSID]; /* NOT NULL-terminated */
	UCHAR SsidLen;		/* the actual ssid length in used */
	UCHAR LastSsidLen;	/* the actual ssid length in used */
	UCHAR Bssid[MAC_ADDR_LEN];
	CHAR LastSsid[MAX_LEN_OF_SSID]; /* NOT NULL-terminated */
	UCHAR LastBssid[MAC_ADDR_LEN];

	UCHAR DtimCount;	/* 0.. DtimPeriod-1 */
	UCHAR DtimPeriod;	/* default = 3 */

	ULONG BeaconLostTime;	/* seconds */
	ULONG LastBeaconRxTime;	/* OS's timestamp of the last BEACON RX time */
	ULONG ChannelQuality;	/* 0..100, Channel Quality Indication for Roaming */
	ULONG StaStatusFlags;
	BOOLEAN bConfigChanged; /* Config Change flag for the same SSID setting */
	BOOLEAN bShowHiddenSSID;	/* Show all known SSID in SSID list get operation */
	BOOLEAN bForceTxBurst;	/* 1: force enble TX PACKET BURST, 0: disable */
	BOOLEAN bTGnWifiTest;

	PVOID pAssociatedAPEntry;
	RALINK_TIMER_STRUCT LinkDownTimer;

	/*
		GROUP 2 -
		User configuration loaded from Registry, E2PROM or OID_xxx. These settings describe
		the user intended configuration, and should be always applied to the final
		settings in ACTIVE BSS without compromising with the BSS holder.
		Once initialized, user configuration can only be changed via OID_xxx
	*/
	USHORT CountDowntoPsm;
	USHORT DefaultListenCount;	/* default listen count; */
	USHORT ThisTbttNumToNextWakeUp;
	ULONG WindowsPowerMode;	/* Power mode for AC power */
	ULONG WindowsBatteryPowerMode;	/* Power mode for battery if exists */
	BOOLEAN bWindowsACCAMEnable;	/* Enable CAM power mode when AC on */
	BOOLEAN	 FlgPsmCanNotSleep; /* TRUE: can not switch ASIC to sleep */
	/* MIB:ieee802dot11.dot11smt(1).dot11StationConfigTable(1) */
	USHORT Psm;		/* power management mode   (PWR_ACTIVE|PWR_SAVE) */
	PWR_MGMT_STRUCT	PwrMgmt;

	/* remove YF fsm */
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;	/* PrivacyFilter enum for 802.1X */
	UCHAR RssiTrigger;
	UCHAR RssiTriggerMode;	/* RSSI_TRIGGERED_UPON_BELOW_THRESHOLD or RSSI_TRIGGERED_UPON_EXCCEED_THRESHOLD */

	/*
		Security segment
	*/
	/* Add to support different cipher suite for WPA2/WPA mode */
	UINT32 PairwiseCipher;
	UINT32 GroupCipher;

	/*Add to support Mix Mode (WPA/WPA2) AP */
	UINT32 AKMMap;

	USHORT RsnCapability;

	UCHAR WpaPassPhrase[64];	/* WPA PSK pass phrase */
	UINT WpaPassPhraseLen;	/* the length of WPA PSK pass phrase */
	UCHAR PMK[LEN_MAX_PMK];	/* WPA PSK mode PMK */
	UCHAR PTK[LEN_PTK];	/* WPA PSK mode PTK */
	UCHAR GMK[LEN_GMK];	/* WPA PSK mode GMK */
	UCHAR GTK[MAX_LEN_GTK];	/* GTK from authenticator */
	UCHAR GNonce[32];	/* GNonce for WPA2PSK from authenticator */
	CIPHER_KEY TxGTK;
	BSSID_INFO SavedPMK[PMKID_NO];
	UINT SavedPMKNum;	/* Saved PMKID number */
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	NDIS_SPIN_LOCK SavedPMK_lock;
#endif


	/* For WPA countermeasures */
	ULONG LastMicErrorTime;	/* record last MIC error time */
	ULONG MicErrCnt;	/* Should be 0, 1, 2, then reset to zero (after disassoiciation). */
	BOOLEAN bBlockAssoc;	/* Block associate attempt for 60 seconds after counter measure occurred. */
	/* For WPA-PSK supplicant state */
	UINT8 WpaState;		/* Default is SS_NOTUSE and handled by microsoft 802.1x */
	UCHAR ReplayCounter[8];
	UCHAR ANonce[32];	/* ANonce for WPA-PSK from aurhenticator */
	UCHAR SNonce[32];	/* SNonce for WPA-PSK */
	ULONG LastScanTime;	 /* remove YF fsm */
	BOOLEAN bNotFirstScan;	/* remove YF fsm */
	/* New for WPA, windows want us to to keep association information and */
	/* Fixed IEs from last association response */
	NDIS_802_11_ASSOCIATION_INFORMATION AssocInfo;
	USHORT ReqVarIELen;	/* Length of next VIE include EID & Length */
	UCHAR ReqVarIEs[MAX_VIE_LEN];	/* The content saved here should be little-endian format. */
	USHORT ResVarIELen;	/* Length of next VIE include EID & Length */
	UCHAR ResVarIEs[MAX_VIE_LEN];

	UCHAR RSNIE_Len;
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];	/* The content saved here should be little-endian format. */

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	/*for probe request vendor ie*/
	struct customer_vendor_ie apcli_vendor_ie;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT S*/
/* --- */

	BOOLEAN bSwRadio;	/* Software controlled Radio On/Off, TRUE: On */
	BOOLEAN bHwRadio;	/* Hardware controlled Radio On/Off, TRUE: On */
	BOOLEAN bRadio;		/* Radio state, And of Sw & Hw radio state */
	BOOLEAN bHardwareRadio;	/* Hardware controlled Radio enabled */

	RALINK_TIMER_STRUCT StaQuickResponeForRateUpTimer;
	BOOLEAN StaQuickResponeForRateUpTimerRunning;

	BOOLEAN bAutoReconnect;	/* Set to TRUE when setting OID_802_11_SSID with no matching BSSID */
	BOOLEAN bAutoConnectByBssid;
	BOOLEAN bAutoConnectIfNoSSID;
	BOOLEAN bFastConnect;
	BOOLEAN bSkipAutoScanConn;

	/* Fast Roaming */
	BOOLEAN bAutoRoaming;	/* 0:disable auto roaming by RSSI, 1:enable auto roaming by RSSI */
	CHAR dBmToRoam;		/* the condition to roam when receiving Rssi less than this value. It's negative value. */

	/*
		Statistics segment
	*/
	STAT_COUNTERS StaStatistic;
#ifdef REPEATER_TX_RX_STATISTIC
	UINT64 TxDropCount;
#endif

	USHORT RPIDensity[8];	/* Array for RPI density collection */
	RSSI_SAMPLE RssiSample;
	ULONG NumOfAvgRssiSample;

	USHORT DisassocReason;
	UCHAR DisassocSta[MAC_ADDR_LEN];
	USHORT DeauthReason;
	UCHAR DeauthSta[MAC_ADDR_LEN];
	USHORT AuthFailReason;
	UCHAR AuthFailSta[MAC_ADDR_LEN];

	/*connectinfo  for tmp store connect info from UI*/
	BOOLEAN Connectinfoflag;
	UCHAR	ConnectinfoBssid[MAC_ADDR_LEN];
	UCHAR	ConnectinfoChannel;
	UCHAR	ConnectinfoSsidLen;
	CHAR	ConnectinfoSsid[MAX_LEN_OF_SSID];
	UCHAR ConnectinfoBssType;

	/*
		OS_COMPLETION segment
	*/
	RTMP_OS_COMPLETION ifdown_fsm_reset_complete;
	RTMP_OS_COMPLETION linkdown_complete;
#ifdef APCLI_CFG80211_SUPPORT
	RTMP_OS_COMPLETION scan_complete;
	BOOLEAN MarkToClose;
#endif /* APCLI_CFG80211_SUPPORT */

#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
#define MONITOR_FLAG_11N_SNIFFER		0x01
	UCHAR BssMonitorFlag;	/* Specific flag for monitor */
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */

#if defined(APCLI_CFG80211_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT)
	WPA_SUPPLICANT_INFO wpa_supplicant_info;
#endif /* WPA_SUPPLICANT_SUPPORT */


#ifdef EXT_BUILD_CHANNEL_LIST
	UCHAR IEEE80211dClientMode;
	UCHAR StaOriCountryCode[3];
	UCHAR StaOriGeography;
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef IP_ASSEMBLY
	BOOLEAN bFragFlag;
#endif /* IP_ASSEMBLY */

#ifdef DOT11R_FT_SUPPORT
	DOT11R_CMN_STRUC Dot11RCommInfo;
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
	WNM_CONFIG WnmCfg;
	BOOLEAN bBSSMantAPSupport;
	BOOLEAN bDMSAPSupport;
	BOOLEAN BTMQuerySend;
	BOOLEAN bDMSSetUp;
	UCHAR BTMQueryDialogToken;
	UCHAR DMSREQDialogToken;
#endif				/* DOT11V_WNM_SUPPORT */

#ifdef DOT11N_DRAFT3
	UCHAR RegClass;		/*IE_SUPP_REG_CLASS: 2009 PF#3: For 20/40 Intolerant Channel Report */
#endif /* DOT11N_DRAFT3 */




#ifdef CONFIG_DOT11U_INTERWORKING
	GAS_CTRL GASCtrl;
#endif

#ifdef CONFIG_HOTSPOT
	HOTSPOT_CTRL HotSpotCtrl;
#endif

#ifdef CONFIG_DOT11V_WNM
	WNM_CTRL WNMCtrl;
#endif



/* apcli stuff */

	UCHAR MacTabWCID;	/*WCID value, which point to the entry of ASIC Mac table. */

	CHAR CfgSsid[MAX_LEN_OF_SSID];
	UCHAR CfgSsidLen;
	UCHAR CfgApCliBssid[MAC_ADDR_LEN];

	PSPOLL_FRAME PsPollFrame;
	HEADER_802_11 NullFrame;

	APCLI_INF_STAT ApcliInfStat;

#ifdef APCLI_CONNECTION_TRIAL
	UCHAR	TrialCh;/* the channel that Apcli interface will try to connect the rootap locates */
	UCHAR 	PrevCh; /* the Previous operating channel*/
	RALINK_TIMER_STRUCT TrialConnectTimer;
	RALINK_TIMER_STRUCT TrialConnectPhase2Timer;
	RALINK_TIMER_STRUCT TrialConnectRetryTimer;

	USHORT NewRootApRetryCnt;
#endif /* APCLI_CONNECTION_TRIAL */
#ifdef FAST_EAPOL_WAR
	BOOLEAN	pre_entry_alloc;
#endif /* FAST_EAPOL_WAR */
#ifdef CON_WPS
	UINT ConWpsApCliModeScanDoneStatus;
#endif /* CON_WPS */

	UINT8 dync_txop_histogram[5];
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	APCLI_CONNECT_SCAN_TYPE ApCliAutoConnectType; /* 0 : User Trigger SCAN Mode, 1 :  Driver Trigger SCAN Mode, this is for Sigma DUT test , Peer AP may change BSSID, but SSID is the same */
	BOOLEAN		ApCliAutoConnectRunning;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#ifdef A4_CONN
	UCHAR a4_init;
	UCHAR a4_apcli;
#endif

#ifdef CONFIG_MAP_SUPPORT
	UCHAR last_controller_connectivity;
#endif
#ifdef DOT11_SAE_SUPPORT
	UCHAR sae_cfg_group;
#endif
#ifdef CONFIG_OWE_SUPPORT
	UCHAR curr_owe_group;
	UCHAR owe_trans_ssid_len;
	CHAR owe_trans_ssid[MAX_LEN_OF_SSID];
	CHAR owe_trans_bssid[MAC_ADDR_LEN];
	UCHAR owe_trans_open_ssid_len;
	CHAR owe_trans_open_ssid[MAX_LEN_OF_SSID];
	CHAR owe_trans_open_bssid[MAC_ADDR_LEN];
#endif
	BOOLEAN apcliNeedEnable;
};


#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
/***************************************************************************
  *	AP related data structures
  **************************************************************************/
/* AUTH-RSP State Machine Aux data structure */
typedef struct _AP_MLME_AUX {
	UCHAR Addr[MAC_ADDR_LEN];
	USHORT Alg;
	CHAR Challenge[CIPHER_TEXT_LEN];
} AP_MLME_AUX, *PAP_MLME_AUX;
#endif /* CONFIG_AP_SUPPORT */



typedef struct _STA_TR_ENTRY {
	UINT32 EntryType;
	struct wifi_dev *wdev;

	UCHAR wcid;
	/*
		func_tb_idx used to indicate following index:
			in StaCfg
			in pAd->MeshTab
			in WdsTab.MacTab
	*/

	UCHAR func_tb_idx;
	UCHAR Addr[MAC_ADDR_LEN];
	/*
		Tx Info
	*/
	USHORT NonQosDataSeq;
	USHORT TxSeq[NUM_OF_TID];

	QUEUE_HEADER tx_queue[WMM_QUE_NUM];
	QUEUE_HEADER ps_queue;
	UINT enqCount;
	UINT TokenCount[WMM_QUE_NUM];
	INT ps_qbitmap;
	UCHAR ps_state;
	UCHAR retrieve_start_state;
	UCHAR token_enq_all_fail;

	BOOLEAN tx_pend_for_agg[WMM_QUE_NUM];
	NDIS_SPIN_LOCK txq_lock[WMM_QUE_NUM];
	NDIS_SPIN_LOCK ps_queue_lock;
	NDIS_SPIN_LOCK ps_sync_lock;

	UINT deq_cnt;
	UINT deq_bytes;
	ULONG PsQIdleCount;

	BOOLEAN enq_cap;
	BOOLEAN deq_cap;

	/*
		STA status
	*/

	UCHAR bssid[MAC_ADDR_LEN];
	BOOLEAN bIAmBadAtheros;	/* Flag if this is Atheros chip that has IOT problem.  We need to turn on RTS/CTS protection. */
	BOOLEAN isCached;
	UCHAR PortSecured;
	UCHAR PsMode;
	UCHAR FlgPsModeIsWakeForAWhile; /* wake up for a while until a condition */
	BOOLEAN LockEntryTx;	/* TRUE = block to WDS Entry traffic, FALSE = not. */

	UCHAR CurrTxRate;

#ifdef VENDOR_FEATURE1_SUPPORT
	/* total 128B, use UINT32 to avoid alignment problem */
	UINT32 HeaderBuf[32];	/* (total 128B) TempBuffer for TX_INFO + TX_WI + 802.11 Header + padding + AMSDU SubHeader + LLC/SNAP */
	UCHAR HdrPadLen;	/* padding length*/
	UCHAR MpduHeaderLen; /* 802.11 header + LLC/SNAP not including padding */
	UCHAR wifi_hdr_len; /* 802.11 header */
	UINT16 Protocol;
#endif /* VENDOR_FEATURE1_SUPPORT */

#ifdef DOT11_N_SUPPORT
	UINT32 CachedBuf[16];	/* UINT (4 bytes) for alignment */

	USHORT RXBAbitmap;	/* fill to on-chip  RXWI_BA_BITMASK in 8.1.3RX attribute entry format */
	USHORT TXBAbitmap;	/* This bitmap as originator, only keep in software used to mark AMPDU bit in TXWI */
	USHORT tx_amsdu_bitmap;
	USHORT TXAutoBAbitmap;
	USHORT BADeclineBitmap;
	USHORT BARecWcidArray[NUM_OF_TID];	/* The mapping wcid of recipient session. if RXBAbitmap bit is masked */
	USHORT BAOriWcidArray[NUM_OF_TID];	/* The mapping wcid of originator session. if TXBAbitmap bit is masked */
	USHORT BAOriSequence[NUM_OF_TID];	/* The mapping wcid of originator session. if TXBAbitmap bit is masked */

	UCHAR MpduDensity;
	UCHAR MaxRAmpduFactor;
	UCHAR AMsduSize;
	UCHAR MmpsMode;		/* MIMO power save mode. */
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	BOOLEAN bReptCli;
	BOOLEAN bReptEthCli;
	UCHAR MatchReptCliIdx;
	UCHAR ReptCliAddr[MAC_ADDR_LEN];
	ULONG ReptCliIdleCount;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef VHT_TXBF_SUPPORT
	UINT8 snd_dialog_token;
#ifdef SOFT_SOUNDING
	BOOLEAN snd_reqired;
	HTTRANSMIT_SETTING snd_rate;
#endif /* SOFT_SOUNDING */
#endif /* VHT_TXBF_SUPPORT */

	/*
		Statistics related parameters
	*/
	UINT32 ContinueTxFailCnt;
	ULONG TimeStamp_toTxRing;
	ULONG NoDataIdleCount;

#ifdef CONFIG_AP_SUPPORT
	LARGE_INTEGER TxPackets;
	LARGE_INTEGER RxPackets;
	ULONG TxBytes;
	ULONG RxBytes;
#endif /* CONFIG_AP_SUPPORT */
	ULONG one_sec_tx_pkts;
	ULONG avg_tx_pkts;
	UINT8 previous_amsdu_state[NUM_OF_UP];
	INT previous_sn[NUM_OF_UP];
	INT cacheSn[NUM_OF_UP];
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
#if TC_PAGE_BASED_DEMAND
	INT32 TotalPageCount[WMM_QUE_NUM];
#endif
#endif
	/*
		Used to ignore consecutive PS poll.
		set: when we get a PS poll.
		clear: when a PS data is sent or two period passed.
	*/
	UINT8 PsDeQWaitCnt;

	UINT8 OmacIdx;
	struct _STA_REC_CTRL_T StaRec;

#ifdef DBG_AMSDU
#define TIME_SLOT_NUMS 10
	UINT32 amsdu_1;
	UINT32 amsdu_1_rec[TIME_SLOT_NUMS];
	UINT32 amsdu_2;
	UINT32 amsdu_2_rec[TIME_SLOT_NUMS];
	UINT32 amsdu_3;
	UINT32 amsdu_3_rec[TIME_SLOT_NUMS];
	UINT32 amsdu_4;
	UINT32 amsdu_4_rec[TIME_SLOT_NUMS];
#endif
	UINT16 token_cnt;
#ifdef FQ_SCH_SUPPORT
	struct fq_stainfo_type	fq_sta_rec;
#endif
} STA_TR_ENTRY;

/* sub catalog of MAC_TABLE ENTRY */
struct ampdu_caps {
	/* ht */
	UINT8 max_ht_ampdu_len_exp;
	UINT8 min_mpdu_start_spacing;
	/* vht */
	UINT8 max_mpdu_len;
	UINT8 max_vht_ampdu_len_exp;
	/* he */
	UINT8 max_he_ampdu_len_exp;
	UINT8 multi_tid_agg;
};

struct caps_info {
	/* ht */
	enum ht_caps ht_cap;
	UINT8 ht_rx_stbc;
	UINT8 max_amsdu_len;
	struct ampdu_caps ampdu;
	UINT8 pco_tx_time;
	UINT8 mfb;
	/* vht */
	enum vht_caps vht_cap;
	UINT8 vht_support_ch_width_set;
	UINT8 vht_rx_stbc;
	UINT8 bfee_sts_cap;
	UINT8 num_sound_dimen;
	UINT8 vht_link_adapt;
	UINT8 ext_nss_bw;
	struct oper_mode op;
	/* he */
	enum he_mac_caps he_mac_cap;
	enum he_phy_caps he_phy_cap;
	enum he_gi_caps he_gi;
	struct he_bf_info he_bf;
	UINT8 he_frag_level;
	UINT8 max_num_msdu_frag;
	UINT8 min_frag_size;
	UINT8 tf_mac_pad_duration;
	UINT8 he_link_adapt;
	UINT8 he_ch_width_set;
	UINT8 punc_preamble_rx;
	UINT8 midamble_rx_max_nsts;
	UINT8 dcm_max_constellation_tx;
	UINT8 dcm_max_constellation_rx;
};

#ifdef OCE_SUPPORT
#ifdef OCE_FILS_SUPPORT
struct fils_info {
	UINT16 last_pending_id;
	UCHAR *pending_ie;
	UINT pending_ie_len;
	UCHAR *extra_ie;
	UINT extra_ie_len;
	struct raw_rssi_info rssi_info;

	UCHAR auth_algo;

	UCHAR PTK[80];
	UCHAR PTK_len;

	INT16 status;
	BOOLEAN is_pending_auth;
	BOOLEAN is_pending_assoc;
	BOOLEAN is_pending_decrypt;
	BOOLEAN is_pending_encrypt;
	BOOLEAN is_post_assoc;

	VOID (*pending_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*pending_decrypt)(PRTMP_ADAPTER pAd, struct _MAC_TABLE_ENTRY *pEntry,
		struct _SECURITY_CONFIG *pSecConfig, MLME_QUEUE_ELEM *Elem);
};
#endif /* OCE_FILS_SUPPORT */
struct oce_info {
	INT8	DeltaAssocRSSI;
};

typedef union GNU_PACKED _ESP_INFO {
struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
	UINT32 None:8;                  /* bit 24 ~ 31 */
	UINT32 DataPPDUDurationTarget:8;                        /* bit 16 ~ 23 */
	UINT32 EstimatedAirTimeFraction:8;              /* bit 8 ~ 15 */
	UINT32 BAWinSize:3;                                             /* bit 5 ~ 7 */
	UINT32 DataFormat:2;                                            /* bit 3 ~ 4 */
	UINT32 Reserved:1;                                              /* bit 2 */
	UINT32 ACI:2;                                                   /* bit 0 ~ 1 */
	/* -------------------------------------        */
	/* ACI  AC              Access Category                 */
	/* -------------------------------------        */
	/* 00   AC_BE   Best Effort                                     */
	/* 01   AC_BK   Background                                      */
	/* 10   AC_VI   Video                                           */
	/* 11   AC_VO   Voice                                           */
	/* -------------------------------------        */
#else
	UINT32 ACI:2;                                                   /* bit 0 ~ 1 */
	/* -------------------------------------        */
	/* ACI  AC              Access Category                 */
	/* -------------------------------------        */
	/* 00   AC_BE   Best Effort                                     */
	/* 01   AC_BK   Background                                      */
	/* 10   AC_VI   Video                                           */
	/* 11   AC_VO   Voice                                           */
	/* -------------------------------------        */
	UINT32 Reserved:1;                                              /* bit 2 */
	UINT32 DataFormat:2;                                            /* bit 3 ~ 4 */
	UINT32 BAWinSize:3;                                             /* bit 5 ~ 7 */
	UINT32 EstimatedAirTimeFraction:8;              /* bit 8 ~ 15 */
	UINT32 DataPPDUDurationTarget:8;                        /* bit 16 ~ 23 */
	UINT32 None:8;                  /* bit 24 ~ 31 */
#endif /* RT_BIG_ENDIAN */
	} field;
	UINT32 word;
} ESP_INFO, *PESP_INFO;

#endif /* OCE_SUPPORT */

#ifdef HOSTAPD_11R_SUPPORT
typedef struct _AUTH_FRAME_INFO{
	UCHAR addr1[MAC_ADDR_LEN];
	UCHAR addr2[MAC_ADDR_LEN];
	USHORT auth_alg;
	USHORT auth_seq;
	USHORT auth_status;
	CHAR Chtxt[CIPHER_TEXT_LEN];
#ifdef DOT11R_FT_SUPPORT
	FT_INFO FtInfo;
#endif /* DOT11R_FT_SUPPORT */
} AUTH_FRAME_INFO;
#endif /* HOSTAPD_11R_SUPPORT */

typedef struct _RADIUS_ACCOUNT_ENTRY {
	BOOLEAN occupied;
	UCHAR Addr[MAC_ADDR_LEN];
	struct wifi_dev *wdev;
	HTTRANSMIT_SETTING HTPhyMode;
	RSSI_SAMPLE RssiSample;
	ULONG TxBytes;
	ULONG RxBytes;
	LARGE_INTEGER TxPackets;
	LARGE_INTEGER RxPackets;
	ULONG NoDataIdleCount;
} RADIUS_ACCOUNT_ENTRY, *PRADIUS_ACCOUNT_ENTRY;


typedef struct _MAC_TABLE_ENTRY {
	UINT32 EntryType;
	UINT32 EntryState;
	struct wifi_dev *wdev;
	PVOID pAd;
	struct _MAC_TABLE_ENTRY *pNext;

	/* SpaceX ********************/

	/// Number of times this client's software queue has been checked.
    UINT32 SwqChecks;

	/// Number of checks where this client's software queue was not empty.
    UINT32 SwqChecksNonEmpty;
	
	/*****************************/

	struct caps_info cap;
	/*
		A bitmap of BOOLEAN flags. each bit represent an operation status of a particular
		BOOLEAN control, either ON or OFF. These flags should always be accessed via
		CLIENT_STATUS_TEST_FLAG(), CLIENT_STATUS_SET_FLAG(), CLIENT_STATUS_CLEAR_FLAG() macros.
		see fOP_STATUS_xxx in RTMP_DEF.C for detail bit definition. fCLIENT_STATUS_AMSDU_INUSED
	*/
	ULONG ClientStatusFlags;
	ULONG cli_cap_flags;

	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode;	/* For transmit phy setting in TXWI. */
	HTTRANSMIT_SETTING MinHTPhyMode;

	/*
		wcid:

		tr_tb_idx:

		func_tb_idx used to indicate following index:
			in StaCfg
			in pAd->MeshTab
			in WdsTab.MacTab

		apidx: should remove this
	*/
#ifdef OCE_SUPPORT
#ifdef OCE_FILS_SUPPORT
	struct fils_info filsInfo;
#endif /* OCE_FILS_SUPPORT */
	struct oce_info oceInfo;
#endif /* OCE_SUPPORT */

	UCHAR wcid;
	UCHAR tr_tb_idx;
	UCHAR func_tb_idx;
	UCHAR apidx;		/* MBSS number */

	BOOLEAN isRalink;
	BOOLEAN bIAmBadAtheros;	/* Flag if this is Atheros chip that has IOT problem.  We need to turn on RTS/CTS protection. */

#ifdef MBO_SUPPORT
	BOOLEAN bIndicateNPC;
	BOOLEAN bIndicateCDC;
	MBO_STA_CH_PREF_CDC_INFO MboStaInfoNPC;
	MBO_STA_CH_PREF_CDC_INFO MboStaInfoCDC;
#endif /* MBO_SUPPORT */
#ifdef A4_CONN
	UCHAR	a4_entry;		/* Determine if this entry act which A4 role */
#endif /* A4_CONN */
#ifdef DYNAMIC_VLAN_SUPPORT
	UINT32 vlan_id;
#endif

#ifdef MBSS_AS_WDS_AP_SUPPORT
    BOOLEAN bEnable4Addr;
#endif

#ifdef RADIUS_MAC_AUTH_SUPPORT
	BOOLEAN	bAllowTraffic;
#endif /* RADIUS_MAC_AUTH_SUPPORT */



	UCHAR Addr[MAC_ADDR_LEN];
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss;
#ifdef APCLI_SUPPORT
	PVOID pApCli;
#ifdef ROAMING_ENHANCE_SUPPORT
	BOOLEAN bRoamingRefreshDone;
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/*
		STATE MACHINE Status
	*/
	USHORT Aid;	/* in range 1~2007, with bit 14~15 = b'11, e.g., 0xc001~0xc7d7 */
	SST Sst;
	AUTH_STATE AuthState;	/* for SHARED KEY authentication state machine used only */


	/* Rx status related parameters */
	RSSI_SAMPLE RssiSample;
	UINT32 LastTxRate;
	UINT32 LastRxRate;
	SHORT freqOffset;		/* Last RXWI FOFFSET */
	SHORT freqOffsetValid;	/* Set when freqOffset field has been updated */
#ifdef WIFI_DIAG
	CHAR DiagAvgSnr;
#endif
#ifdef AIR_MONITOR
	UCHAR mnt_idx[BAND_NUM];
	UCHAR mnt_band;
#endif /* AIR_MONITOR */


	/* WPA/WPA2 4-way database */
	UCHAR EnqueueEapolStartTimerRunning;	/* Enqueue EAPoL-Start for triggering EAP SM */


	struct _SECURITY_CONFIG SecConfig;
	UCHAR RSNIE_Len;
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];
	UCHAR CMTimerRunning;
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;	/* PrivacyFilter enum for 802.1X */

	UCHAR bssid[MAC_ADDR_LEN];
	BOOLEAN IsReassocSta;	/* Indicate whether this is a reassociation procedure */
	ULONG NoDataIdleCount;
	ULONG AssocDeadLine;
	UINT16 StationKeepAliveCount;	/* unit: second */
	USHORT CapabilityInfo;
	UCHAR PsMode;
	UCHAR FlgPsModeIsWakeForAWhile; /* wake up for a while until a condition */
	UCHAR VirtualTimeout; /* peer power save virtual timeout */

#ifdef WDS_SUPPORT
	BOOLEAN LockEntryTx;	/* TRUE = block to WDS Entry traffic, FALSE = not. */
#endif /* WDS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	BOOLEAN bReptCli;
	BOOLEAN bReptEthCli;
	BOOLEAN bReptEthBridgeCli;
	UCHAR MatchReptCliIdx;
	UCHAR ReptCliAddr[MAC_ADDR_LEN];
	ULONG ReptCliIdleCount;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	UINT32 StaConnectTime;	/* the live time of this station since associated with AP */
	UINT32 StaIdleTimeout;	/* idle timeout per entry */

#ifdef UAPSD_SUPPORT
	/* these UAPSD states are used on the fly */
	/* 0:AC_BK, 1:AC_BE, 2:AC_VI, 3:AC_VO */
	BOOLEAN bAPSDCapablePerAC[4];	/* for trigger-enabled */
	BOOLEAN bAPSDDeliverEnabledPerAC[4];	/* for delivery-enabled */


	UCHAR MaxSPLength;

	BOOLEAN bAPSDAllAC;	/* 1: all AC are delivery-enabled U-APSD */

	QUEUE_HEADER UAPSDQueue[WMM_NUM_OF_AC];	/* queue for each U-APSD */
	USHORT UAPSDQIdleCount;	/* U-APSD queue timeout */

	PQUEUE_ENTRY pUAPSDEOSPFrame;	/* the last U-APSD frame */
	USHORT UAPSDTxNum;	/* total U-APSD frame number */
	BOOLEAN bAPSDFlagEOSPOK;	/* 1: EOSP frame is tx by ASIC */
	BOOLEAN bAPSDFlagSPStart;	/* 1: SP is started */

	/* need to use unsigned long, because time parameters in OS is defined as
	   unsigned long */
	unsigned long UAPSDTimeStampLast;	/* unit: 1000000/OS_HZ */
	BOOLEAN bAPSDFlagSpRoughUse;	/* 1: use rough SP (default: accurate) */

	/* we will set the flag when PS-poll frame is received and
	   clear it when statistics handle.
	   if the flag is set when PS-poll frame is received then calling
	   statistics handler to clear it. */
	BOOLEAN bAPSDFlagLegacySent;	/* 1: Legacy PS sent but yet statistics handle */

#endif /* UAPSD_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
	UINT32 StreamModeMACReg;	/* MAC reg used to control stream mode for this client. 0=>No stream mode */
#endif /* STREAM_MODE_SUPPORT */

	UINT FIFOCount;
	UINT DebugFIFOCount;
	UINT DebugTxCount;

#ifdef DOT11_N_SUPPORT
	USHORT NoBADataCountDown;
#endif /* DOT11_N_SUPPORT */

	/* ==================================================== */
	enum RATE_ADAPT_ALG rateAlg;
	/* TODO: shiang-usw, use following parameters to replace "RateLen/MaxSupportedRate" */
	UCHAR RateLen;
	UCHAR MaxSupportedRate;

	BOOLEAN bAutoTxRateSwitch;
	UCHAR CurrTxRate;
	UCHAR CurrTxRateIndex;
	UCHAR lastRateIdx;
	UCHAR *pTable;	/* Pointer to this entry's Tx Rate Table */

#ifdef NEW_RATE_ADAPT_SUPPORT
	UCHAR lowTrafficCount;
	UCHAR fewPktsCnt;
	BOOLEAN perThrdAdj;
	UCHAR mcsGroup;/* the mcs group to be tried */
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef AGS_SUPPORT
	AGS_CONTROL AGSCtrl;	/* AGS control */
#endif /* AGS_SUPPORT */

	/* to record the each TX rate's quality. 0 is best, the bigger the worse. */
	USHORT TxQuality[MAX_TX_RATE_INDEX + 1];
	BOOLEAN fLastSecAccordingRSSI;
	UCHAR LastSecTxRateChangeAction;	/* 0: no change, 1:rate UP, 2:rate down */
	CHAR LastTimeTxRateChangeAction;	/* Keep last time value of LastSecTxRateChangeAction */
	ULONG LastTxOkCount; /* TxSuccess count in last Rate Adaptation interval */
	UCHAR LastTxPER;	/* Tx PER in last Rate Adaptation interval */
	UCHAR PER[MAX_TX_RATE_INDEX + 1];
	UINT32 CurrTxRateStableTime;	/* # of second in current TX rate */
	UCHAR TxRateUpPenalty;	/* extra # of second penalty due to last unstable condition */

	BOOLEAN fgGband256QAMSupport;
	UCHAR SupportRateMode; /* 1: CCK 2:OFDM 4: HT, 8:VHT */
	UINT8 SupportCCKMCS;
	UINT8 SupportOFDMMCS;
#ifdef DOT11_N_SUPPORT
	UINT32 SupportHTMCS;
#ifdef DOT11_VHT_AC
	UINT16 SupportVHTMCS1SS;
	UINT16 SupportVHTMCS2SS;
	UINT16 SupportVHTMCS3SS;
	UINT16 SupportVHTMCS4SS;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

#ifdef PEER_DELBA_TX_ADAPT
	BOOLEAN bPeerDelBaTxAdaptEn;
#endif /* PEER_DELBA_TX_ADAPT */

#ifdef MFB_SUPPORT
	UCHAR lastLegalMfb;	/* last legal mfb which is used to set rate */
	BOOLEAN isMfbChanged;	/* purpose: true when mfb has changed but the new mfb is not adopted for Tx */
	struct _RTMP_RA_LEGACY_TB *LegalMfbRS;
	BOOLEAN fLastChangeAccordingMfb;
	NDIS_SPIN_LOCK fLastChangeAccordingMfbLock;
	/* Tx MRQ */
	BOOLEAN toTxMrq;
	UCHAR msiToTx, mrqCnt;	/*mrqCnt is used to count down the inverted-BF mrq to be sent */
	/* Rx mfb */
	UCHAR pendingMfsi;
	/* Tx MFB */
	BOOLEAN toTxMfb;
	UCHAR	mfbToTx;
	UCHAR	mfb0, mfb1;
#endif	/* MFB_SUPPORT */
#ifdef TXBF_SUPPORT
	UCHAR		eTxBfEnCond;
	UCHAR		iTxBfEn;
	COUNTER_TXBF TxBFCounters;		/* TxBF Statistics */
#ifdef MT_MAC
	VENDOR_BF_SETTING rStaBfRecVendorUpdate;
	TXBF_PFMU_STA_INFO rStaRecBf;
	UCHAR	has_oui; /* Indication of STA's OUI when association */
#endif
#endif /* TXBF_SUPPORT */

#ifdef VHT_TXBF_SUPPORT
	UINT8 snd_dialog_token;
#ifdef SOFT_SOUNDING
	BOOLEAN snd_reqired;
	HTTRANSMIT_SETTING snd_rate;
#endif /* SOFT_SOUNDING */
#endif /* VHT_TXBF_SUPPORT */

	UINT32 OneSecTxNoRetryOkCount;
	UINT32 OneSecTxRetryOkCount;
	UINT32 OneSecTxFailCount;
	UINT32 OneSecRxLGICount;		/* unicast-to-me Long GI count */
	UINT32 OneSecRxSGICount;		/* unicast-to-me Short GI count */
	UINT32 ContinueTxFailCnt;
	ULONG TimeStamp_toTxRing;

	/*==================================================== */
	EXT_CAP_INFO_ELEMENT ext_cap;
	struct _vendor_ie_cap vendor_ie;
#ifdef DOT11_N_SUPPORT
	HT_CAPABILITY_IE HTCapability;

	USHORT RXBAbitmap;	/* fill to on-chip  RXWI_BA_BITMASK in 8.1.3RX attribute entry format */
	USHORT TXBAbitmap;	/* This bitmap as originator, only keep in software used to mark AMPDU bit in TXWI */
	USHORT tx_amsdu_bitmap;
	USHORT TXAutoBAbitmap;
	USHORT BADeclineBitmap;
	USHORT BARecWcidArray[NUM_OF_TID];	/* The mapping wcid of recipient session. if RXBAbitmap bit is masked */
	USHORT BAOriWcidArray[NUM_OF_TID];	/* The mapping wcid of originator session. if TXBAbitmap bit is masked */
	USHORT BAOriSequence[NUM_OF_TID];	/* The mapping wcid of originator session. if TXBAbitmap bit is masked */

	UCHAR MpduDensity;
	UCHAR MaxRAmpduFactor;
	UCHAR AMsduSize;
	UINT32 amsdu_limit_len;
	UINT32 amsdu_limit_len_adjust;
	UCHAR MmpsMode;		/* MIMO power save mode. */

#ifdef DOT11N_DRAFT3
	UCHAR BSS2040CoexistenceMgmtSupport;
	BOOLEAN bForty_Mhz_Intolerant;
#endif /* DOT11N_DRAFT3 */

#ifdef DOT11_VHT_AC
	VHT_CAP_IE vht_cap_ie;

	/* only take effect if ext_cap.operating_mode_notification = 1 */
	BOOLEAN force_op_mode;
	OPERATING_MODE operating_mode;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_DOT11V_WNM
	UCHAR BssTransitionManmtSupport;
#endif /* CONFIG_DOT11V_WNM */

#ifdef DOT11V_WNM_SUPPORT
	UCHAR DMSSupport;
	BOOLEAN Beclone;
#endif /* DOT11V_WNM_SUPPORT */

	BOOLEAN bWscCapable;
	UCHAR Receive_EapolStart_EapRspId;

	UINT32 TXMCSExpected[MAX_MCS_SET];
	UINT32 TXMCSSuccessful[MAX_MCS_SET];
	UINT32 TXMCSFailed[MAX_MCS_SET];
	UINT32 TXMCSAutoFallBack[MAX_MCS_SET][MAX_MCS_SET];

#ifdef CONFIG_STA_SUPPORT
	ULONG LastBeaconRxTime;
#endif /* CONFIG_STA_SUPPORT */


#ifdef DOT11R_FT_SUPPORT
	FT_MDIE_INFO MdIeInfo;
	FT_FTIE_INFO FtIeInfo;

	UINT8 InitialMDIE[5];
	UINT8 InitialFTIE[256];
	UINT InitialFTIE_Len;

	UCHAR FT_PMK_R0[32];
	UCHAR FT_PMK_R0_NAME[16];
	UCHAR FT_PMK_R1[32];
	UCHAR FT_PMK_R1_NAME[16];
	UCHAR PTK_NAME[16];

	UCHAR FT_UCipher[4];
	UCHAR FT_Akm[4];
	UCHAR FT_PTK[LEN_MAX_PTK]; /* 512 bits max, KCK(16)+KEK(16)+TK(32) */
	UCHAR FT_Status;
	UCHAR FT_R1kh_CacheMiss_Times;

#ifdef R1KH_HARD_RETRY
	UCHAR FT_R1kh_CacheMiss_Hard;
	RTMP_OS_COMPLETION ack_r1kh;
#endif /* R1KH_HARD_RETRY */

#ifdef HOSTAPD_11R_SUPPORT
	AUTH_FRAME_INFO auth_info_resp;
#endif


#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
	RRM_EN_CAP_IE RrmEnCap;
#endif /* DOT11K_RRM_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN bBSSMantSTASupport;
	BOOLEAN bDMSSTASupport;
	UCHAR BTMQueryDialogToken;
	BOOLEAN DisassociationImminent;
	BOOLEAN BSSTerminationIncluded;
	RALINK_TIMER_STRUCT DisassocTimer;
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11V_WNM_SUPPORT */

#ifdef CONFIG_MAP_SUPPORT
	UCHAR assoc_req_frame[ASSOC_REQ_LEN];
	UCHAR assoc_req_len;
	UCHAR DevPeerRole;
	UCHAR cur_rssi_status;
	UCHAR pre_rssi_status;
	BOOLEAN isTriggerSteering;
	UCHAR pre_traffic_mode;
#endif

#ifdef CONFIG_AP_SUPPORT
	LARGE_INTEGER TxPackets;
	LARGE_INTEGER RxPackets;
	UINT64 TxBytes;
	UINT64 RxBytes;
	UINT64 TxFailCount;
#endif /* CONFIG_AP_SUPPORT */
	ULONG OneSecTxBytes;
	ULONG OneSecRxBytes;
	ULONG AvgTxBytes;
	ULONG AvgRxBytes;
	ULONG one_sec_tx_pkts;
	ULONG avg_tx_pkts;
	ULONG one_sec_tx_succ_pkts;

#ifdef RX_COUNT_DETECT
	ULONG avg_rx_pkts;
	ULONG one_sec_rx_pkts;
#endif /* RX_COUNT_DETECT */

#ifdef TXRX_STAT_SUPPORT
	LARGE_INTEGER TxDataPacketCount;
	LARGE_INTEGER TxDataPacketByte;
	LARGE_INTEGER TxUnicastPktCount;
	LARGE_INTEGER TxDataPacketCount1SecValue;
	LARGE_INTEGER TxDataPacketByte1SecValue;
	LARGE_INTEGER LastTxDataPacketCountValue;
	LARGE_INTEGER LastTxDataPacketByteValue;
	LARGE_INTEGER TxDataPacketCountPerAC[4];	/*per access category*/
	LARGE_INTEGER TxMgmtPacketCount;
	LARGE_INTEGER RxDataPacketCount;
	LARGE_INTEGER RxDataPacketByte;
	LARGE_INTEGER RxUnicastPktCount;
	LARGE_INTEGER RxUnicastByteCount;
	LARGE_INTEGER RxDataPacketCount1SecValue;
	LARGE_INTEGER RxDataPacketByte1SecValue;
	LARGE_INTEGER LastRxDataPacketCountValue;
	LARGE_INTEGER LastRxDataPacketByteValue;
	LARGE_INTEGER RxDataPacketCountPerAC[4];	/*per access category*/
	LARGE_INTEGER RxMgmtPacketCount;
	LARGE_INTEGER RxDecryptionErrorCount;
	LARGE_INTEGER RxMICErrorCount;
	ULONG RxLastMgmtPktRate;
	CHAR LastDataPktRssi[4];
	CHAR LastMgmtPktRssi[4];
	UINT32 LastOneSecTxTotalCountByWtbl;
	UINT32 LastOneSecTxFailCountByWtbl;
	UINT32 LastOneSecPER;
	UINT32 TxSuccessByWtbl; 					/*data/unicast same variable, updated per sec*/

#ifdef VENDOR_FEATURE11_SUPPORT
	LARGE_INTEGER mpdu_attempts;
	LARGE_INTEGER mpdu_retries;
	LARGE_INTEGER mpdu_low_rate_fail_cnt;
#endif
#endif
#ifdef CUSTOMER_DCC_FEATURE
	UINT32 RxCount;
	UINT32 TxCount;
	UINT32 ReceivedByteCount;
	UINT32 TransmittedByteCount;
	UINT32 RxErrorCount;
	UINT32 RxDropCount;
	UINT32 TxErrorCount;
	UINT32 TxDropCount;
	UINT32 TxRetriedPktCount;
	UINT64 ChannelUseTime;
#endif

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	UINT16			TdlsTxFailCount;
	UINT32			TdlsKeyLifeTimeCount;
	UCHAR			MatchTdlsEntryIdx; /* indicate the index in pAd->StaCfg[0].DLSEntry */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) // */


	ULONG ChannelQuality;	/* 0..100, Channel Quality Indication for Roaming */
#ifdef CONFIG_HOTSPOT_R2
	UCHAR				IsWNMReqValid;
	UCHAR				QosMapSupport;
	UCHAR				DscpExceptionCount;
	USHORT				DscpRange[8];
	USHORT				DscpException[21];
	struct wnm_req_data	*ReqData;
	struct _sta_hs_info hs_info;
	UCHAR OSEN_IE_Len;
	UCHAR OSEN_IE[MAX_LEN_OF_RSNIE];
#endif /* CONFIG_HOTSPOT_R2 */
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)
	UCHAR				IsBTMReqValid;
	UCHAR				IsKeep;
	UINT16				BTMDisassocCount;
	BOOLEAN				bBSSMantSTASupport;
	struct btm_req_data	*ReqbtmData;
#endif


	BOOLEAN bACMBit[WMM_NUM_OF_AC];

	RA_ENTRY_INFO_T RaEntry;
	RA_INTERNAL_INFO_T	RaInternal;
	UINT32	ConnectionType;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	BOOLEAN bTxPktChk;
	UINT8 TxStatRspCnt;
	UINT32 TotalTxSuccessCnt;	/* Accumulated Tx success count from N9 (WTBL) */
#endif

#ifdef HTC_DECRYPT_IOT
	UINT32 HTC_ICVErrCnt; /* keep the ICV Error cnt of HTC Rx Cnt */
	UCHAR HTC_AAD_OM_Force; /* when reach the threshold, force set the WTBL.DW2.AAD_OM to 1 */
	UINT32 HTC_AAD_OM_CountDown; /* settling time (1 count 1 second) for start count HTC_ICVErrCnt */
	UCHAR HTC_AAD_OM_Freeze; /* Treat the entry's AAD_OM setting is correct now */
#endif /* HTC_DECRYPT_IOT */

#ifdef WH_EVENT_NOTIFIER
	UCHAR custom_ie_len;			   /* Length of Vendor Information Element */
	UCHAR custom_ie[CUSTOM_IE_TOTAL_LEN];  /* Vendor Information Element  */
	StaActivityItem tx_state;			  /* Station's tx state record */
	StaActivityItem rx_state;			  /* Station's rx state record */
#endif /* WH_EVENT_NOTIFIER */
#ifdef GN_MIXMODE_SUPPORT
	BOOLEAN FlgIs11bSta;
#endif /*GN_MIXMODE_SUPPORT*/
#ifdef HOSTAPD_11R_SUPPORT
	IE_LISTS *ie_list;
#endif

#ifdef PN_UC_REPLAY_DETECTION_SUPPORT
	UINT64 CCMP_UC_PN[NUM_OF_TID];
#endif /* PN_UC_REPLAY_DETECTION_SUPPORT */
	UINT64 CCMP_BC_PN[4];
	BOOLEAN Init_CCMP_BC_PN_Passed[4];
	BOOLEAN AllowUpdateRSC;
#ifdef REDUCE_TCP_ACK_SUPPORT
	BOOLEAN RACKEnalbedSta;
#endif
	BOOLEAN WtblSetFlag; /* This will reflect whetherWtbl entry is created or not */
	RTMP_OS_COMPLETION WtblSetDone; /* To intimate status when Wtbl entry is created */

#ifdef RATE_PRIOR_SUPPORT
	ULONG McsTotalRxCount;
	ULONG McsLowRateRxCount;
#endif /*RATE_PRIOR_SUPPORT*/
} MAC_TABLE_ENTRY, *PMAC_TABLE_ENTRY;

typedef enum _MAC_ENT_STATUS_ {
	/* fAnyStationInPsm */
	MAC_TB_ANY_PSM = 0x1,
	/*
		fAnyStationBadAtheros
		Check if any Station is atheros 802.11n Chip.  We need to use RTS/CTS with Atheros 802,.11n chip.
	*/
	MAC_TB_ANY_ATH = 0x2,
	/*
		fAnyTxOPForceDisable
		Check if it is necessary to disable BE TxOP
	*/
	MAC_TB_FORCE_TxOP = 0x4,
	/*
		fAllStationAsRalink
		Check if all stations are ralink-chipset
	*/
	MAC_TB_ALL_RALINK = 0x8,
	/*
		fAnyStationIsLegacy
		Check if I use legacy rate to transmit to my BSS Station
	*/
	MAC_TB_ANY_LEGACY = 0x10,
	/*
		fAnyStationNonGF
		Check if any Station can't support GF
	*/
	MAC_TB_ANY_NON_GF = 0x20,
	/* fAnyStation20Only */
	MAC_TB_ANY_HT20 = 0x40,
	/*
		fAnyStationMIMOPSDynamic
		Check if any Station is MIMO Dynamic
	*/
	MAC_TB_ANY_MIMO_DYNAMIC = 0x80,
	/*
		fAnyBASession
		Check if there is BA session.  Force turn on RTS/CTS
	*/
	MAC_TB_ANY_BA = 0x100,
	/* fAnyStaFortyIntolerant */
	MAC_TB_ANY_40_INTOlERANT = 0x200,
	/*
		fAllStationGainGoodMCS
		Check if all stations more than MCS threshold
	*/
	MAC_TB_ALL_GOOD_MCS = 0x400,
	/*
		fAnyStationIsHT
		Check if still has any station set the Intolerant bit on!
	*/
	MAC_TB_ANY_HT = 0x800,
	/* fAnyWapiStation */
	MAC_TB_ANY_WAPI = 0x1000,
} MAC_ENT_STATUS;

#define BAND_NUM_MAX 2
typedef struct _MAC_TABLE {
	MAC_TABLE_ENTRY * Hash[HASH_TABLE_SIZE];
	MAC_TABLE_ENTRY Content[MAX_LEN_OF_MAC_TABLE];
	STA_TR_ENTRY tr_entry[MAX_LEN_OF_TR_TABLE];
	/*
		Be care in mgmt_entrytb.c  MacTableReset() will NdisZeroMemory(&pAd->MacTab.Size, sizeof(MAC_TABLE)-offsetof(MAC_TABLE, Size));
		above need to be backup, klock's warnnig @118489 should be mark as not an issue.
	*/
	UINT16 Size;
	QUEUE_HEADER McastPsQueue;
	ULONG PsQIdleCount;
	MAC_ENT_STATUS sta_status;

	BOOLEAN fAnyStationInPsm;
	BOOLEAN fAnyStationBadAtheros;	/* Check if any Station is atheros 802.11n Chip.  We need to use RTS/CTS with Atheros 802,.11n chip. */
	BOOLEAN fAnyTxOPForceDisable;	/* Check if it is necessary to disable BE TxOP */
	BOOLEAN fAllStationAsRalink[2];	/* Check if all stations are ralink-chipset */
	BOOLEAN fCurrentStaBw40;		/* Check if only one STA w/ BW40 */
#ifdef DOT11_N_SUPPORT
	BOOLEAN fAnyStationIsLegacy;	/* Check if I use legacy rate to transmit to my BSS Station/ */
	BOOLEAN fAnyStationNonGF[BAND_NUM_MAX];	/* Check if any Station can't support GF. */
	BOOLEAN fAnyStation20Only;	/* Check if any Station can't support GF. */
	BOOLEAN fAnyStationMIMOPSDynamic;	/* Check if any Station is MIMO Dynamic */
	BOOLEAN fAnyBASession;	/* Check if there is BA session.  Force turn on RTS/CTS */
	BOOLEAN fAnyStaFortyIntolerant;	/* Check if still has any station set the Intolerant bit on! */
	BOOLEAN fAllStationGainGoodMCS; /* Check if all stations more than MCS threshold */

#endif /* DOT11_N_SUPPORT */


	USHORT MsduLifeTime; /* life time for PS packet */
#ifdef OUI_CHECK_SUPPORT
	UCHAR oui_mgroup_cnt;
	UINT32 repeater_wcid_error_cnt;
	UINT32 repeater_bm_wcid_error_cnt;
#endif /*OUI_CHECK_SUPPORT*/
} MAC_TABLE, *PMAC_TABLE;

#ifdef SNIFFER_SUPPORT
#define MONITOR_MODE_OFF  0
#define MONITOR_MODE_REGULAR_RX  1
#define MONITOR_MODE_FULL 2
#endif /*SNIFFER_SUPPORT*/

typedef struct _MONITOR_STRUCT {
	struct wifi_dev wdev;
	INT CurrentMonitorMode;
	UINT FilterSize;
	UINT FrameType;
	UCHAR MacFilterAddr[MAC_ADDR_LEN];
	BOOLEAN	MacFilterOn;
	BOOLEAN	bMonitorInitiated;
	BOOLEAN bMonitorOn;
} MONITOR_STRUCT;

#ifdef CONFIG_AP_SUPPORT
/***************************************************************************
  *	AP WDS related data structures
  **************************************************************************/
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
typedef struct _WDS_COUNTER {
	LARGE_INTEGER ReceivedFragmentCount;
	LARGE_INTEGER TransmittedFragmentCount;
	ULONG ReceivedByteCount;
	ULONG TransmittedByteCount;
	ULONG RxErrorCount;
	ULONG TxErrors;
	LARGE_INTEGER MulticastReceivedFrameCount;
	ULONG RxNoBuffer;
} WDS_COUNTER, *PWDS_COUNTER;

typedef struct _WDS_ENTRY {
	BOOLEAN Valid;
	UCHAR Addr[MAC_ADDR_LEN];
	ULONG NoDataIdleCount;
	struct _WDS_ENTRY *pNext;
} WDS_ENTRY, *PWDS_ENTRY;

typedef struct _RT_802_11_WDS_ENTRY {
	struct wifi_dev wdev;
	UCHAR Valid;
	UCHAR PeerWdsAddr[MAC_ADDR_LEN];
	UCHAR MacTabMatchWCID;	/* ASIC */
	UCHAR KeyIdx;
	CIPHER_KEY WdsKey;
	UCHAR PhyOpMode;

	WDS_COUNTER WdsCounter;
} RT_802_11_WDS_ENTRY, *PRT_802_11_WDS_ENTRY;

typedef struct _WDS_TABLE {
	UCHAR Mode;
	UINT Size;
	NDIS_SPIN_LOCK WdsTabLock;
	BOOLEAN flg_wds_init;
	RT_802_11_WDS_ENTRY WdsEntry[MAX_WDS_ENTRY];
} WDS_TABLE, *PWDS_TABLE;
#endif /* WDS_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
#define MAX_IGNORE_AS_REPEATER_ENTRY_NUM	32

typedef struct _MBSS_TO_CLI_LINK_MAP_T {
	struct wifi_dev *mbss_wdev;
	struct wifi_dev *cli_link_wdev;
} MBSS_TO_CLI_LINK_MAP_T;

typedef struct _REPEATER_CLIENT_ENTRY {
	/* BOOLEAN bValid; */
	BOOLEAN CliEnable;
	BOOLEAN CliValid;
	BOOLEAN bEthCli;
	UCHAR MatchApCliIdx;
	UCHAR MatchLinkIdx;
	UCHAR MacTabWCID;
	UCHAR CliConnectState; /* 0: disconnect 1: connecting 2: connected */

	ULONG CtrlCurrState;
	ULONG SyncCurrState;
	ULONG AuthCurrState;
	ULONG AssocCurrState;

	RALINK_TIMER_STRUCT ApCliAssocTimer, ApCliAuthTimer;

	USHORT AuthReqCnt;
	USHORT AssocReqCnt;
	ULONG CliTriggerTime;
		/* For WPA countermeasures */
	ULONG LastMicErrorTime; /* record last MIC error time */
	BOOLEAN bBlockAssoc;	/* Block associate attempt for 60 seconds after counter measure occurred. */

	UCHAR OriginalAddress[MAC_ADDR_LEN];
	UCHAR CurrentAddress[MAC_ADDR_LEN];
	PVOID pAd;
	struct _REPEATER_CLIENT_ENTRY *pNext;

	ULONG Disconnect_Sub_Reason;
	ULONG LinkDownReason;

	BSS_INFO_ARGUMENT_T bss_info_argument;
	RTMP_OS_COMPLETION free_ack;

	UCHAR BandIdx;	/*link on which Apcli link bandidx.*/
	RALINK_TIMER_STRUCT ReptCliResetTimer;
	struct wifi_dev *wdev;	/*pointer to the linking Apcli interface wdev. */

#ifdef FAST_EAPOL_WAR
	BOOLEAN pre_entry_alloc;
#endif /* FAST_EAPOL_WAR */
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	BSSID_INFO SavedPMK[PMKID_NO];
	UINT SavedPMKNum; /* Saved PMKID number */
#endif
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	NDIS_SPIN_LOCK SavedPMK_lock;
#endif
#ifdef DOT11_SAE_SUPPORT
	UCHAR sae_cfg_group;
#endif
#ifdef CONFIG_OWE_SUPPORT
	UCHAR curr_owe_group;
#endif
} REPEATER_CLIENT_ENTRY, *PREPEATER_CLIENT_ENTRY;

typedef struct _REPEATER_CLIENT_ENTRY_MAP {
	PREPEATER_CLIENT_ENTRY pReptCliEntry;
	struct _REPEATER_CLIENT_ENTRY_MAP *pNext;
} REPEATER_CLIENT_ENTRY_MAP, *PREPEATER_CLIENT_ENTRY_MAP;

typedef struct _INVAILD_TRIGGER_MAC_ENTRY {
	UCHAR MacAddr[MAC_ADDR_LEN];
	UCHAR entry_idx;
	BOOLEAN bInsert;
	struct _INVAILD_TRIGGER_MAC_ENTRY *pNext;
} INVAILD_TRIGGER_MAC_ENTRY, *PINVAILD_TRIGGER_MAC_ENTRY;

typedef struct _REPEATER_CTRL_STRUCT {
	INVAILD_TRIGGER_MAC_ENTRY IgnoreAsRepeaterEntry[MAX_IGNORE_AS_REPEATER_ENTRY_NUM];
	INVAILD_TRIGGER_MAC_ENTRY * IgnoreAsRepeaterHash[HASH_TABLE_SIZE];
	UCHAR IgnoreAsRepeaterEntrySize;
} REPEATER_CTRL_STRUCT, *PREPEATER_CTRL_STRUCT;

typedef struct _REPEATER_ADAPTER_DATA_TABLE {
	bool Enabled;
	void *EntryLock;
	void **CliHash;
	void **MapHash;
	void *Wdev_ifAddr;
	void *Wdev_ifAddr_DBDC;
} REPEATER_ADAPTER_DATA_TABLE;
#endif /* MAC_REPEATER_SUPPORT */

typedef struct _AP_ADMIN_CONFIG {
	USHORT CapabilityInfo;
	/* Multiple SSID */
	UCHAR BssidNum;
	UCHAR MacMask;
	BSS_STRUCT MBSSID[HW_BEACON_MAX_NUM];
	ULONG IsolateInterStaTrafficBTNBSSID;

#ifdef APCLI_SUPPORT
	UCHAR ApCliInfRunned;	/* Number of  ApClient interface which was running. value from 0 to MAX_APCLI_INTERFACE */
	UINT8 ApCliNum;
	BOOLEAN FlgApCliIsUapsdInfoUpdated;
	/*STA_ADMIN_CONFIG ApCliTab[MAX_APCLI_NUM];	AP-client */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	BOOLEAN		ApCliAutoConnectChannelSwitching;
#ifdef BT_APCLI_SUPPORT
	BOOLEAN	ApCliAutoBWBTSupport;
#endif
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	BOOLEAN		bPartialScanEnable[MAX_APCLI_NUM];
	BOOLEAN		bPartialScanning[MAX_APCLI_NUM];
	ULONG		ApCliIssueScanTime[MAX_APCLI_NUM];
#ifdef ROAMING_ENHANCE_SUPPORT
	BOOLEAN bRoamingEnhance;
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
	BOOLEAN bMACRepeaterEn_precfg;
	BOOLEAN bMACRepeaterEn;
	UCHAR MACRepeaterOuiMode;
	UINT8 EthApCliIdx;
	UCHAR RepeaterCliSize;
	NDIS_SPIN_LOCK ReptCliEntryLock;
	REPEATER_CLIENT_ENTRY * ReptCliHash[HASH_TABLE_SIZE];
	REPEATER_CLIENT_ENTRY_MAP * ReptMapHash[HASH_TABLE_SIZE];
	UCHAR BridgeAddress[MAC_ADDR_LEN];
	REPEATER_CTRL_STRUCT ReptControl;

	NDIS_SPIN_LOCK CliLinkMapLock;
	MBSS_TO_CLI_LINK_MAP_T  MbssToCliLinkMap[HW_BEACON_MAX_NUM];
	REPEATER_CLIENT_ENTRY *pRepeaterCliPool;
	REPEATER_CLIENT_ENTRY_MAP *pRepeaterCliMapPool;
#endif /* MAC_REPEATER_SUPPORT */

	/* for wpa */
	RALINK_TIMER_STRUCT CounterMeasureTimer;

	UCHAR CMTimerRunning;
	UCHAR BANClass3Data;
	LARGE_INTEGER aMICFailTime;
	LARGE_INTEGER PrevaMICFailTime;
	ULONG MICFailureCounter;

	NDIS_AP_802_11_PMKID PMKIDCache;

	RSSI_SAMPLE RssiSample;
	ULONG NumOfAvgRssiSample;

	BOOLEAN bAutoChannelAtBootup[DBDC_BAND_NUM];	/* 0: disable, 1: enable */
	ChannelSel_Alg AutoChannelAlg[DBDC_BAND_NUM];	/* Alg for selecting Channel */
#ifdef ACS_CTCC_SUPPORT
	BOOLEAN auto_ch_score_flag;	/* score for Channel, and don't switch channel */
#endif
#ifdef AP_SCAN_SUPPORT
	UINT32  ACSCheckTime[DBDC_BAND_NUM]; /* Periodic timer to trigger Auto Channel Selection (unit: second) */
	UINT32  ACSCheckCount[DBDC_BAND_NUM]; /* if  ACSCheckCount > ACSCheckTime, then do ACS check */
#endif /* AP_SCAN_SUPPORT */
	BOOLEAN bAvoidDfsChannel;	/* 0: disable, 1: enable */
	BOOLEAN bIsolateInterStaTraffic;
	BOOLEAN bHideSsid;

	/* temporary latch for Auto channel selection */
	ULONG ApCnt;		/* max RSSI during Auto Channel Selection period */
	UCHAR AutoChannel_Channel;	/* channel number during Auto Channel Selection */
	UCHAR current_channel_index;	/* current index of channel list */
	UCHAR AutoChannelSkipListNum;	/* number of rejected channel list */
	UCHAR AutoChannelSkipList[MAX_NUM_OF_CHANNELS + 1];
	UCHAR DtimCount;	/* 0.. DtimPeriod-1 */
	UCHAR DtimPeriod;	/* default = 3 */
	UCHAR ErpIeContent;
	ULONG LastOLBCDetectTime;
	ULONG LastNoneHTOLBCDetectTime;
	ULONG LastScanTime;	/* Record last scan time for issue BSSID_SCAN_LIST */

	/* EDCA parameters to be announced to its local BSS */
	EDCA_PARM BssEdcaParm;

	RALINK_TIMER_STRUCT ApQuickResponeForRateUpTimer;
	BOOLEAN ApQuickResponeForRateUpTimerRunning;

#ifdef IDS_SUPPORT
	/* intrusion detection parameter */
	BOOLEAN IdsEnable;
	UINT32 AuthFloodThreshold;	/* Authentication frame flood threshold */
	UINT32 AssocReqFloodThreshold;	/* Association request frame flood threshold */
	UINT32 ReassocReqFloodThreshold;	/* Re-association request frame flood threshold */
	UINT32 ProbeReqFloodThreshold;	/* Probe request frame flood threshold */
	UINT32 DisassocFloodThreshold;	/* Disassociation frame flood threshold */
	UINT32 DeauthFloodThreshold;	/* Deauthentication frame flood threshold */
	UINT32 EapReqFloodThreshold;	/* EAP request frame flood threshold */
	UINT32 DataFloodThreshold;		/* Malicious data frame flood threshold */

	UINT32 RcvdAuthCount;
	UINT32 RcvdAssocReqCount;
	UINT32 RcvdReassocReqCount;
	UINT32 RcvdProbeReqCount;
	UINT32 RcvdDisassocCount;
	UINT32 RcvdDeauthCount;
	UINT32 RcvdEapReqCount;
	UINT32 RcvdMaliciousDataCount;	/* Data Frame DDOS */

	RALINK_TIMER_STRUCT IDSTimer;
	BOOLEAN IDSTimerRunning;
#endif /* IDS_SUPPORT */

	/* Indicate the maximum idle timeout */
	UINT32 StaIdleTimeout;

	ULONG EntryLifeCheck;

#ifdef IGMP_SNOOP_SUPPORT
	BOOLEAN IgmpSnoopEnable[DBDC_BAND_NUM];	/* 0: disable, 1: enable. */
#endif /* IGMP_SNOOP_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
	FT_TAB FtTab;
#endif /* DOT11R_FT_SUPPORT */

#ifdef CLIENT_WDS
	NDIS_SPIN_LOCK CliWdsTabLock;
	PCLIWDS_PROXY_ENTRY pCliWdsEntryPool;
	LIST_HEADER CliWdsEntryFreeList;
	LIST_HEADER CliWdsProxyTb[CLIWDS_HASH_TAB_SIZE];
#endif /* CLIENT_WDS */
	UCHAR EntryClientCount;
#ifdef MBSS_AS_WDS_AP_SUPPORT
	UCHAR wds_mac[MAC_ADDR_LEN];
#endif

#ifdef MT_MAC
	UINT32 ext_mbss_enable_bitmap;
	UINT32 ext_mbss_tttt_enable_bitmap;
#endif /*MT_MAC*/
#ifdef BAND_STEERING
	BOOLEAN BandSteering;
	UINT8	BndStrgBssIdx[HW_BEACON_MAX_NUM];
	BND_STRG_CLI_TABLE BndStrgTable[DBDC_BAND_NUM];
	UINT32	BndStrgHeartbeatCount;
	UINT32	BndStrgHeartbeatMonitor;
	UINT32	BndStrgHeartbeatNoChange;
#endif /* BAND_STEERING */

#ifdef CONFIG_HOTSPOT_R2
	QOS_MAP_TABLE_T HsQosMapTable[MAX_QOS_MAP_TABLE_SIZE];
#endif /* CONFIG_HOTSPOT_R2 */

#ifdef CON_WPS
	UINT ConWpsApCliMode;  /* means get profile from rootAp by 2G, 5G perferred or AUTO */
	BOOLEAN ConWpsApCliStatus; /* status of Received the EAPOL-FAIL */
	BOOLEAN ConWpsApCliDisableSetting;
	BOOLEAN ConWpsApDisableSetting;
	BOOLEAN ConWpsApCliDisabled;
	RALINK_TIMER_STRUCT ConWpsApCliBandMonitorTimer;
	BOOLEAN	ConWpsMonitorTimerRunning;
	UINT ConWpsApcliAutoPreferIface;
#endif /* CON_WPS */
#ifdef CONFIG_MAP_SUPPORT
	struct map_policy_setting SteerPolicy;
#endif
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl greenap;
#endif /* GREENAP_SUPPORT */

#ifdef WH_EVENT_NOTIFIER
	struct EventNotifierCfg EventNotifyCfg;
#endif /* WH_EVENT_NOTIFIER */

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	struct customer_oui_filter ap_customer_oui;
#endif

#ifdef DOT11K_RRM_SUPPORT
	BOOLEAN HandleNRReqbyUplayer;
#endif

#ifdef OCE_SUPPORT
	RALINK_TIMER_STRUCT FdFrameTimer;
	BOOLEAN FdFrameTimerRunning;
	BOOLEAN FdFrameTxsEnabled;
	BOOLEAN FdFrameCurNum;
	RALINK_TIMER_STRUCT APAutoScanNeighborTimer;
#endif /* OCE_SUPPORT */
#ifdef MGMT_TXPWR_CTRL
	UINT8 MgmtTxPwr[DBDC_BAND_NUM]; /* for 5G : 6M (OFDM) pwr  2.4G: 1M (CCK) pwr */
#endif
} AP_ADMIN_CONFIG;

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || defined(AXE) || defined(MT7626)
#ifdef CONFIG_AP_SUPPORT
typedef struct _BCN_CHECK_INFO_STRUC {
	UINT32 nobcncnt0; /* nobcn accumulative cnt for band 0 */
	UINT32 prebcncnt0; /* previous 2.5s bcn cnt for band 0 */
	UINT32 totalbcncnt0; /* total bcn cnt for band 0 */
	UINT32 nobcncnt1; /* nobcn accumulative cnt for band 1 */
	UINT32 prebcncnt1; /* previous 2.5s bcn cnt for band 1 */
	UINT32 totalbcncnt1; /* total bcn cnt for band 1 */
} BCN_CHECK_INFO_STRUC, *PBCN_CHECK_INFO_STRUC;
#endif
#endif

#ifdef IGMP_SNOOP_SUPPORT
typedef enum _IGMP_GROUP_TYPE {
	MODE_IS_INCLUDE = 1,
	MODE_IS_EXCLUDE,
	CHANGE_TO_INCLUDE_MODE,
	CHANGE_TO_EXCLUDE_MODE,
	ALLOW_NEW_SOURCES,
	BLOCK_OLD_SOURCES
} IgmpGroupType;

typedef enum _MULTICAST_FILTER_ENTRY_TYPE {
	MCAT_FILTER_STATIC = 0,
	MCAT_FILTER_DYNAMIC,
} MulticastFilterEntryType;

typedef struct _MEMBER_ENTRY {
	struct _MEMBER_ENTRY *pNext;
	UCHAR Addr[MAC_ADDR_LEN];
	/*	USHORT Aid; */
} MEMBER_ENTRY, *PMEMBER_ENTRY;

typedef struct _MULTICAST_FILTER_TABLE_ENTRY {
	BOOLEAN Valid;
	MulticastFilterEntryType type;	/* 0: static, 1: dynamic. */
	ULONG lastTime;
	PNET_DEV net_dev;
	UCHAR Addr[MAC_ADDR_LEN];
	LIST_HEADER MemberList;
	struct _MULTICAST_FILTER_TABLE_ENTRY *pNext;
} MULTICAST_FILTER_TABLE_ENTRY, *PMULTICAST_FILTER_TABLE_ENTRY;

typedef struct _MULTICAST_FILTER_TABLE {
	UCHAR Size;

	PMULTICAST_FILTER_TABLE_ENTRY
	Hash[MAX_LEN_OF_MULTICAST_FILTER_HASH_TABLE];
	MULTICAST_FILTER_TABLE_ENTRY Content[MAX_LEN_OF_MULTICAST_FILTER_TABLE];
	NDIS_SPIN_LOCK MulticastFilterTabLock;
	NDIS_SPIN_LOCK FreeMemberPoolTabLock;
	MEMBER_ENTRY freeMemberPool[FREE_MEMBER_POOL_SIZE];
	LIST_HEADER freeEntryList;
} MULTICAST_FILTER_TABLE, *PMULTICAST_FILTER_TABLE;
#endif /* IGMP_SNOOP_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT

typedef struct _DMSID_TABLE {
	struct _DMSID_TABLE *pNext;
	UCHAR DMSIDs;
	UINT32 DMSID_DestIP;
} DMSID_TABLE, PDMSID_TABLE;
typedef struct _DMS_ENTRY {
	struct _DMS_ENTRY *pNext;
	LIST_HEADER DMSIDList;
	PMAC_TABLE_ENTRY pEntry;
	UCHAR apidx;
} DMS_ENTRY, *PDMS_ENTRY;

#endif /* DOT11V_WNM_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
typedef enum _RT_GREEN_AP_LEVEL {
	GREENAP_11BGN_STAS = 0,
	GREENAP_ONLY_11BG_STAS,
	GREENAP_WITHOUT_ANY_STAS_CONNECT
} RT_GREEN_AP_LEVEL;

typedef enum _GREEN_AP_SUSPEND_REASON {
	AP_BACKGROUND_SCAN = (1 << 0),
} GREEN_AP_SUSPEND_REASON;
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */

/* ----------- end of AP ---------------------------- */
#endif /* CONFIG_AP_SUPPORT */

#ifdef BLOCK_NET_IF
typedef struct _BLOCK_QUEUE_ENTRY {
	BOOLEAN SwTxQueueBlockFlag;
	LIST_HEADER NetIfList;
} BLOCK_QUEUE_ENTRY, *PBLOCK_QUEUE_ENTRY;
#endif /* BLOCK_NET_IF */


struct wificonf {
	BOOLEAN bShortGI;
	BOOLEAN bGreenField;
};

typedef struct _RTMP_DEV_INFO_ {
	UCHAR chipName[16];
	RTMP_INF_TYPE infType;
} RTMP_DEV_INFO;

#ifdef DBG_DIAGNOSE
#define MAX_VHT_MCS_SET	20 /* for 1ss~ 2ss with MCS0~9 */

#define DIAGNOSE_TIME	10	/* 10 sec */

struct dbg_diag_info {
	USHORT TxDataCnt[WMM_NUM_OF_AC];	/* Tx total data count */
	USHORT TxFailCnt;
	USHORT RxDataCnt;	/* Rx Total Data count. */
	USHORT RxCrcErrCnt;

#ifdef DBG_TXQ_DEPTH
	/* TxSwQ length in scale of 0, 1, 2, 3, 4, 5, 6, 7, >=8 */
	USHORT TxSWQueCnt[WMM_NUM_OF_AC][9];
	UINT32 enq_fall_cnt[WMM_NUM_OF_AC];
	UINT32 deq_fail_no_resource_cnt[WMM_NUM_OF_AC];
	UINT32 deq_called;
	UINT32 deq_round;
	UINT32 deq_cnt[9];
#endif /* DBG_TXQ_DEPTH */

#ifdef DBG_TX_AGG_CNT
	USHORT TxAggCnt;
	USHORT TxNonAggCnt;
	/* TxDMA APMDU Aggregation count in range from 0 to 15, in setp of 1. */
	USHORT TxAMPDUCnt[16];
#endif /* DBG_TX_AGG_CNT */
};

typedef enum {
	DIAG_COND_ALL = 0,
	DIAG_COND_TXQ_DEPTH = 4,
} DIAG_COND_STATUS;

typedef struct _RtmpDiagStrcut_ {	/* Diagnosis Related element */
	BOOLEAN inited;
	UCHAR wcid;
	UCHAR qIdx;
	UCHAR ArrayStartIdx;
	UCHAR ArrayCurIdx;
	UINT32 diag_cond;

	struct dbg_diag_info diag_info[DIAGNOSE_TIME];
} RtmpDiagStruct;
#endif /* DBG_DIAGNOSE */

/* */
/* The entry of transmit power control over MAC */
/* */
typedef struct _TX_POWER_CONTROL_OVER_MAC_ENTRY {
	USHORT MACRegisterOffset;	/* MAC register offset */
	ULONG RegisterValue;	/* Register value */
} TX_POWER_CONTROL_OVER_MAC_ENTRY, *PTX_POWER_CONTROL_OVER_MAC_ENTRY;

/* */
/* The maximum registers of transmit power control */
/* */
#define MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS 5

/* */
/* The configuration of the transmit power control over MAC */
/* */
typedef struct _CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC {
	UCHAR NumOfEntries;	/* Number of entries */
	TX_POWER_CONTROL_OVER_MAC_ENTRY TxPwrCtrlOverMAC[MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS];
} CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC, *PCONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC;

/* */
/* The extension of the transmit power control over MAC */
/* */
typedef struct _TX_POWER_CONTROL_EXT_OVER_MAC {
	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW20Over2Dot4G;

	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW40Over2Dot4G;

	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW20Over5G;

	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW40Over5G;
} TX_POWER_CONTROL_EXT_OVER_MAC, *PTX_POWER_CONTROL_EXT_OVER_MAC;

/* For Wake on Wireless LAN */
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
typedef struct _WOW_CFG_STRUCT {
	BOOLEAN			bEnable;		/* Enable WOW function*/
	BOOLEAN			bWOWFirmware;	/* Enable WOW function, trigger to reload WOW-support firmware */
	BOOLEAN			bInBand;		/* use in-band signal to wakeup system */
	BOOLEAN			bWowIfDownSupport;
	UINT8			nSelectedGPIO;	/* Side band signal to wake up system */
	UINT8			nDelay;			/* Delay number is multiple of 3 secs, and it used to postpone the WOW function */
	UINT32		  nHoldTime;	  /* GPIO pulse hold time, unit: 1us, 0 means hold forever.*/
	BOOLEAN			bWoWRunning;	/* WOW function is working */
	UINT8			nWakeupInterface; /* PCI:0 USB:1 GPIO:2 */
	UINT8			IPAddress[16];	/* Used for ARP response */
	UINT8			bGPIOHighLow;	/* 0: low to high, 1: high to low */
} WOW_CFG_STRUCT, *PWOW_CFG_STRUCT;

typedef enum {
	WOW_GPIO_LOW_TO_HIGH,
	WOW_GPIO_HIGH_TO_LOW
} WOW_GPIO_HIGH_LOW_T;

typedef enum {
	WOW_GPIO_OOTPUT_DISABLE = 0,
	WOW_GPIO_OUTPUT_ENABLE = 1,
} WOW_GPIO_OUTPUT_ENABLE_T;

typedef enum {
	WOW_GPIO_OUTPUT_LEVEL_LOW = 0,
	WOW_GPIO_OUTPUT_LEVEL_HIGH = 1,
} WOW_GPIO_OUTPUT_LEVEL_T;

typedef enum {
	WOW_GPIO_WAKEUP_LEVEL_LOW = 0,
	WOW_GPIO_WAKEUP_LEVEL_HIGH = 1,
} WOW_GPIO_WAKEUP_LEVEL_T;

#define WOW_GPIO_LOW_TO_HIGH_PARAMETER ((WOW_GPIO_OUTPUT_ENABLE << 0) | \
										(WOW_GPIO_OUTPUT_LEVEL_LOW << 1) | (WOW_GPIO_WAKEUP_LEVEL_HIGH << 2))

#define WOW_GPIO_HIGH_TO_LOW_PARAMETER ((WOW_GPIO_OUTPUT_ENABLE << 0) | \
										(WOW_GPIO_OUTPUT_LEVEL_HIGH << 1) | (WOW_GPIO_WAKEUP_LEVEL_LOW << 2));

typedef enum {
	WOW_PKT_TO_HOST,
	WOW_PKT_TO_ANDES
} WOW_PKT_FLOW_T;

typedef enum {
	WOW_WAKEUP_BY_PCIE,
	WOW_WAKEUP_BY_USB,
	WOW_WAKEUP_BY_GPIO
} WOW_WAKEUP_METHOD_T;

typedef enum {
	WOW_ENABLE = 1,
	WOW_TRAFFIC = 3,
	WOW_WAKEUP = 4
} WOW_FEATURE_T;

typedef enum {
	WOW_MASK_CFG = 1,
	WOW_SEC_CFG,
	WOW_INFRA_CFG,
	WOW_P2P_CFG,
} WOW_CONFIG_T;

enum {
	WOW_MAGIC_PKT,
	WOW_BITMAP,
	WOW_IPV4_TCP_SYNC,
	WOW_IPV6_TCP_SYNC
};

typedef struct NEW_WOW_MASK_CFG_STRUCT {
	UINT32	Config_Type;
	UINT32	Function_Enable;
	UINT32	Detect_Mask;
	UINT32	Event_Mask;
} NEW_WOW_MASK_CFG_STRUCT, PNEW_WOW_MASK_CFG_STRUCT;

typedef struct NEW_WOW_SEC_CFG_STRUCT {
	UINT32	Config_Type;
	UINT32	WPA_Ver;
	UCHAR	PTK[64];
	UCHAR	R_COUNTER[8];
	UCHAR	Key_Id;
	UCHAR	Cipher_Alg;
	UCHAR	WCID;
	UCHAR	Group_Cipher;
} NEW_WOW_SEC_CFG_STRUCT, PNEW_WOW_SEC_CFG_STRUCT;

typedef struct NEW_WOW_INFRA_CFG_STRUCT {
	UINT32	Config_Type;
	UCHAR	STA_MAC[6];
	UCHAR	AP_MAC[6];
	UINT32	AP_Status;
} NEW_WOW_INFRA_CFG_STRUCT, PNEW_WOW_INFRA_CFG_STRUCT;

typedef struct _NEW_WOW_P2P_CFG_STRUCT {
	UINT32	Config_Type;
	UCHAR	GO_MAC[6];
	UCHAR	CLI_MAC[6];
	UINT32	P2P_Status;
} NEW_WOW_P2P_CFG_STRUCT, *PNEW_WOW_P2P_CFG_STRUCT;

typedef struct _NEW_WOW_PARAM_STRUCT {
	UINT32	Parameter;
	UINT32	Value;
} NEW_WOW_PARAM_STRUCT, *PNEW_WOW_PARAM_STRUCT;

#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)  || defined(MT_WOW_SUPPORT) */

/*
	Packet drop reason code
*/
typedef enum {
	PKT_ATE_ON = 1 << 8,
	PKT_RADAR_ON = 2 << 8,
	PKT_RRM_QUIET = 3 << 8,
	PKT_TX_STOP = 4 << 8,
	PKT_TX_JAM = 5 << 8,

	PKT_NETDEV_DOWN = 6 < 8,
	PKT_NETDEV_NO_MATCH = 7 << 8,
	PKT_NOT_ALLOW_SEND = 8 << 8,

	PKT_INVALID_DST = 9 << 8,
	PKT_INVALID_SRC = 10 << 8,
	PKT_INVALID_PKT_DATA = 11 << 8,
	PKT_INVALID_PKT_LEN = 12 << 8,
	PKT_INVALID_ETH_TYPE = 13 << 8,
	PKT_INVALID_TXBLK_INFO = 14 << 8,
	PKT_INVALID_SW_ENCRYPT = 15 << 8,
	PKT_INVALID_PKT_TYPE = 16 << 8,
	PKT_INVALID_PKT_MIC = 17 << 8,

	PKT_PORT_NOT_SECURE = 18 << 8,
	PKT_TSPEC_NO_MATCH  = 19 << 8,
	PKT_NO_ASSOCED_STA = 20 << 8,
	PKT_INVALID_MAC_ENTRY = 21 << 8,

	PKT_TX_QUE_FULL = 22 << 8,
	PKT_TX_QUE_ADJUST = 23 << 8,

	PKT_PS_QUE_TIMEOUT = 24 << 8,
	PKT_PS_QUE_CLEAN = 25 << 8,
	PKT_MCAST_PS_QUE_FULL = 26 << 8,
	PKT_UCAST_PS_QUE_FULL = 27 << 8,

	PKT_RX_EAPOL_SANITY_FAIL = 28 << 8,
	PKT_RX_NOT_TO_KERNEL = 29 << 8,
	PKT_RX_MESH_SIG_FAIL = 30 << 8,
	PKT_APCLI_FAIL = 31 << 8,
	PKT_ZERO_DATA = 32 << 8,
	PKT_SW_DECRYPT_FAIL = 33 << 8,
	PKT_TX_SW_ENC_FAIL = 34 << 8,

	PKT_ACM_FAIL = 35 << 8,
	PKT_IGMP_GRP_FAIL = 36 << 8,
	PKT_MGMT_FAIL = 37 << 8,
	PKT_AMPDU_OUT_ORDER = 38 << 8,
	PKT_UAPSD_EOSP = 39 << 8,
	PKT_UAPSD_Q_FULL = 40 << 8,

	PKT_DRO_REASON_MAX = 41,
} PKT_DROP_REASON;

/* Packet drop Direction code */
typedef enum {
	PKT_TX = 0,
	PKT_RX = 1 << 31,
} PKT_DROP_DIECTION;


typedef struct _BBP_RESET_CTL {
#define BBP_RECORD_NUM	49
	REG_PAIR BBPRegDB[BBP_RECORD_NUM];
	BOOLEAN	AsicCheckEn;
} BBP_RESET_CTL, *PBBP_RESET_CTL;

#define DEFLAUT_PARTIAL_SCAN_CH_NUM		1
#define DEFLAUT_PARTIAL_SCAN_BREAK_TIME	4  /* Period of partial scaning: unit: 100ms */
typedef struct _PARTIAL_SCAN_ {
	BOOLEAN bScanning;			/* Doing partial scan or not */
	UINT8	 NumOfChannels;			/* How many channels to scan each time */
	UINT8	 LastScanChannel;		/* last scaned channel */
	UINT32	 BreakTime;			/* Period of partial scanning: unit: 100ms */
	UINT32	 TimerInterval;		/* The time interval between scan epoches: unit 1 ms*/
	RALINK_TIMER_STRUCT PartialScanTimer; /*Count the time interval between scan epoch*/
	struct	 wifi_dev *pwdev;
} PARTIAL_SCAN;

#ifdef OFFCHANNEL_SCAN_FEATURE

typedef enum {
	OFFCHANNEL_SCAN_INVALID = 0,
	OFFCHANNEL_SCAN_START,
	OFFCHANNEL_SCAN_COMPLETE,
	OFFCHANNEL_SCAN_MAX
} RTMP_OFFCHANNEL_SCAN_STAGE;

#endif


typedef struct _SCAN_CHANNEL_SKIPLIST_ {
	UINT8 Channel;
} SCAN_CHANNEL_SKIPLIST;

/*dwell time defined by user*/
typedef struct _USR_DEF_DWELL_ {
	BOOLEAN isActive;
	UINT16 dwell_t_5g;
	UINT16 dwell_t_2g;
} USR_DEF_DWELL;

typedef struct _SCAN_CTRL_ {
	UCHAR ScanType;
	UCHAR BssType;
	UCHAR Channel;
	UCHAR SsidLen;
	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR Bssid[MAC_ADDR_LEN];
	UINT8 SkipCh_Num;
	SCAN_CHANNEL_SKIPLIST *SkipList;
	BOOLEAN dfs_ch_utilization;

	STATE_MACHINE	  SyncFsm;
	STATE_MACHINE_FUNC SyncFsmFun[SYNC_FSM_FUNC_SIZE];

	TIMER_FUNC_CONTEXT SyncTimerFuncContex;
	RALINK_TIMER_STRUCT ScanTimer;

	PARTIAL_SCAN PartialScan;
	USR_DEF_DWELL Usr_dwell;
	UCHAR BandIdx;
	BSS_TABLE ScanTab;
	struct wifi_dev *ScanReqwdev;
	struct wifi_dev *ImprovedScanWdev;
#ifdef OFFCHANNEL_SCAN_FEATURE
	UCHAR			if_name[32];
	UCHAR			ScanGivenChannel[MAX_AWAY_CHANNEL];
	UCHAR			ScanTime[MAX_AWAY_CHANNEL];
	UCHAR			CurrentGivenChan_Index;
	UCHAR			Num_Of_Channels;
	UCHAR			Offchan_Scan_Type[MAX_AWAY_CHANNEL];
	RTMP_OFFCHANNEL_SCAN_STAGE					 state;
	BOOLEAN 		OffChScan;
	BOOLEAN 		OffChScan_Ongoing;
#endif
	/*In OFFCHANNEL_SCAN_FEATURE also actual time is desired */
#ifdef OFFCHANNEL_SCAN_FEATURE
	ktime_t 		ScanTimeActualStart;
	ktime_t 		ScanTimeActualEnd;
	UCHAR			ScanTimeActualDiff;
#endif

} SCAN_CTRL;

#ifdef MEMORY_OPTIMIZATION
#define TX_SWQ_FIFO_LEN	2048
#else
#ifdef IXIA_SUPPORT
#define TX_SWQ_FIFO_LEN	(8192*2)
#else
#define TX_SWQ_FIFO_LEN	8192
#endif /*IXIA_SUPPORT*/
#endif
typedef struct tx_swq_fifo {
	UCHAR swq[TX_SWQ_FIFO_LEN]; /* value 0 is used to indicate free to insert, value 1~127 used to incidate the WCID entry */
	UINT enqIdx;
	UINT deqIdx;
	UINT low_water_mark;
	UINT high_water_mark;
	BOOLEAN q_state;
	NDIS_SPIN_LOCK swq_lock;	/* spinlock for swq */
} TX_SWQ_FIFO;

#ifdef RT_CFG80211_SUPPORT
typedef struct _CFG80211_VIF_DEV {
	struct _CFG80211_VIF_DEV *pNext;
	BOOLEAN isMainDev;
	UINT32 devType;
	PNET_DEV net_dev;
	UCHAR CUR_MAC[MAC_ADDR_LEN];

	/* ProbeReq Frame */
	BOOLEAN Cfg80211RegisterProbeReqFrame;
	CHAR Cfg80211ProbeReqCount;

	/* Action Frame */
	BOOLEAN Cfg80211RegisterActionFrame;
	CHAR Cfg80211ActionCount;
} CFG80211_VIF_DEV, *PCFG80211_VIF_DEV;

typedef struct _CFG80211_VIF_DEV_SET {
#define MAX_CFG80211_VIF_DEV_NUM  2

	BOOLEAN inUsed;
	UINT32 vifDevNum;
	LIST_HEADER vifDevList;
	BOOLEAN isGoingOn; /* To check any vif in list */
} CFG80211_VIF_DEV_SET;

/* TxMmgt Related */
typedef struct _CFG80211_TX_PACKET {
	struct _CFG80211_TX_PACKET *pNext;
	UINT32 TxStatusSeq;			  /* TxMgmt Packet ID from sequence */
	UCHAR *pTxStatusBuf;		  /* TxMgmt Packet buffer content */
	UINT32 TxStatusBufLen;		  /* TxMgmt Packet buffer Length */

} CFG80211_TX_PACKET, *PCFG80211_TX_PACKET;

/* CFG80211 Total CTRL Point */
typedef struct _CFG80211_CONTROL {
	BOOLEAN FlgCfg8021Disable2040Scan;
	BOOLEAN FlgCfg80211Scanning;   /* Record it When scanReq from wpa_supplicant */
	BOOLEAN FlgCfg80211Connecting; /* Record it When ConnectReq from wpa_supplicant*/

	/* Scan Related */
	UINT32 *pCfg80211ChanList;	/* the channel list from from wpa_supplicant */
	UCHAR Cfg80211ChanListLen;	/* channel list length */
	UCHAR Cfg80211CurChanIndex;   /* current index in channel list when driver in scanning */

	UCHAR *pExtraIe;  /* Carry on Scan action from supplicant */
	UINT   ExtraIeLen;

	UCHAR Cfg_pending_Ssid[MAX_LEN_OF_SSID + 1]; /* Record the ssid, When ScanTable Full */
	UCHAR Cfg_pending_SsidLen;

	/* ROC Related */
	RALINK_TIMER_STRUCT Cfg80211RocTimer;
	CMD_RTPRIV_IOCTL_80211_CHAN Cfg80211ChanInfo;
	BOOLEAN Cfg80211RocTimerInit;
	BOOLEAN Cfg80211RocTimerRunning;

	/* Tx_Mmgt Related */
	UINT32 TxStatusSeq;			  /* TxMgmt Packet ID from sequence */
	UCHAR *pTxStatusBuf;		  /* TxMgmt Packet buffer content */
	UINT32 TxStatusBufLen;		  /* TxMgmt Packet buffer Length */
	BOOLEAN TxStatusInUsed;
	LIST_HEADER cfg80211TxPacketList;

	/* P2P Releated*/
	UCHAR P2PCurrentAddress[MAC_ADDR_LEN];	  /* User changed MAC address */
	BOOLEAN isCfgDeviceInP2p;				  /* For BaseRate 6 */

	/* MainDevice Info. */
	CFG80211_VIF_DEV cfg80211MainDev;
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
	/* For add_virtual_intf */
	CFG80211_VIF_DEV_SET Cfg80211VifDevSet;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */

#ifdef CFG80211_MULTI_STA
	PNET_DEV multi_sta_net_dev;
	BOOLEAN flg_cfg_multi_sta_init;
#endif /* CFG80211_MULTI_STA */


	/* In AP Mode */
	UINT8 isCfgInApMode;	/* Is any one Device in AP Mode */
	UCHAR *beacon_tail_buf; /* Beacon buf from upper layer */
	UINT32 beacon_tail_len;
	BOOLEAN beaconIsSetFromHostapd; /* set true after BeaconAdd */

	UCHAR *pCfg80211ExtraIeAssocRsp;
	UINT32 Cfg80211ExtraIeAssocRspLen;

	/* for AdHoc Mode */
	UCHAR *BeaconExtraIe;
	UINT  BeaconExtraIeLen;

	/* Mcc Part */
	/* BOOLEAN isMccOn; */

	/* TODO: need fix it */
	UCHAR Cfg80211_Alpha2[2];
	CMD_RTPRIV_IOCTL_80211_KEY WepKeyInfoBackup;
#ifdef HOSTAPD_11R_SUPPORT
	AUTH_FRAME_INFO auth_info;
#endif
} CFG80211_CTRL, *PCFG80211_CTRL;
#endif /* RT_CFG80211_SUPPORT */

typedef struct rtmp_mac_ctrl {
#ifdef MT_MAC
	UINT8 wtbl_entry_cnt[4];
	UINT16 wtbl_entry_size[4];
	UINT32 wtbl_base_addr[4]; /* base address for WTBL2/3/4 */
	UINT32 wtbl_base_fid[4];
	UINT32 page_size;
#endif /* MT_MAC */

} RTMP_MAC_CTRL;

typedef struct _RADIO_CTRL {
	UCHAR BandIdx;
	UCHAR CurStat;
	UCHAR PhyMode;
	UCHAR Channel;
	UCHAR Channel2;
	UCHAR CentralCh;
	UCHAR Bw;
	UCHAR ExtCha;
	/*check first radio update is for scan or not*/
	BOOLEAN scan_state;
	BOOLEAN IsBfBand;
#ifdef GREENAP_SUPPORT
	BOOLEAN bGreenAPActive;
#endif /* GREENAP_SUPPORT */
#ifdef TXRX_STAT_SUPPORT
	LARGE_INTEGER TxDataPacketCount;
	LARGE_INTEGER TxDataPacketByte;
	LARGE_INTEGER RxDataPacketCount;
	LARGE_INTEGER RxDataPacketByte;
	LARGE_INTEGER TxUnicastDataPacket;
	LARGE_INTEGER TxMulticastDataPacket;
	LARGE_INTEGER TxBroadcastDataPacket;
	LARGE_INTEGER TxMgmtPacketCount;
	LARGE_INTEGER RxMgmtPacketCount;
	LARGE_INTEGER TxBeaconPacketCount;
	LARGE_INTEGER TxDataPacketCountPerAC[4];	/*per access category*/
	LARGE_INTEGER RxDataPacketCountPerAC[4];	/*per access category*/
	CHAR LastDataPktRssi[4];
	LARGE_INTEGER TxPacketDroppedCount;
	LARGE_INTEGER RxDecryptionErrorCount;
	LARGE_INTEGER RxCRCErrorCount;
	LARGE_INTEGER RxMICErrorCount;
	LARGE_INTEGER LastSecTxByte;
	LARGE_INTEGER LastSecRxByte;
	UINT32 TotalPER;
	UINT32 TotalTxFailCnt;
	UINT32 TotalTxCnt;
	UINT32 Last1SecPER;
	UINT32 Last1TxFailCnt;
	UINT32 Last1TxCnt;
#endif
} RADIO_CTRL;

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
typedef struct _QLOAD_CTRL {
	UINT8 FlgQloadEnable;	/* 1: any BSS WMM is enabled */
	ULONG QloadUpTimeLast;	/* last up time */
	UINT8 QloadChanUtil;	/* last QBSS Load, unit: us, primary channel only */
	UINT8 QloadChanUtilCcaNavTx;	/* last QBSS Load, unit: us, including CCA NAV TX time */
	UINT32 QloadChanUtilTotal;	/* current QBSS Load Total, primary channel only */
	UINT32 QloadChanUtilTotalCcaNavTx;	/* current QBSS Load Total, including CCA NAV TX time */
	UINT8 QloadChanUtilBeaconCnt;	/* 1~100, default: 50 */
	UINT8 QloadChanUtilBeaconInt;	/* 1~100, default: 50 */
	UINT32 QloadLatestChannelBusyTimePri;
	UINT32 QloadLatestChannelBusyTimeSec;

	/*
	   ex: For 100ms beacon interval,
	   if the busy time in last TBTT is smaller than 5ms, QloadBusyCount[0] ++;
	   if the busy time in last TBTT is between 5 and 10ms, QloadBusyCount[1] ++;
	   ......
	   if the busy time in last TBTT is larger than 95ms, QloadBusyCount[19] ++;

	   Command: "iwpriv ra0 qload show".
	 */

	/* provide busy time statistics for every TBTT */
#define QLOAD_FUNC_BUSY_TIME_STATS

	/* provide busy time alarm mechanism */
	/* use the function to avoid to locate in some noise environments */
#define QLOAD_FUNC_BUSY_TIME_ALARM

#ifdef QLOAD_FUNC_BUSY_TIME_STATS
#define QLOAD_BUSY_INTERVALS	20	/* partition TBTT to QLOAD_BUSY_INTERVALS */
	/* for primary channel & secondary channel */
	UINT32 QloadBusyCountPri[QLOAD_BUSY_INTERVALS];
	UINT32 QloadBusyCountSec[QLOAD_BUSY_INTERVALS];
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
#define QLOAD_DOES_ALARM_OCCUR(pAd)	(HcGetQloadCtrl(pAd)->FlgQloadAlarmIsSuspended == TRUE)
#define QLOAD_ALARM_EVER_OCCUR(pAd) (HcGetQloadCtrl(pAd)->QloadAlarmNumber > 0)
	BOOLEAN FlgQloadAlarmIsSuspended;	/* 1: suspend */

	UINT8 QloadAlarmBusyTimeThreshold;	/* unit: 1/100 */
	UINT8 QloadAlarmBusyNumThreshold;	/* unit: 1 */
	UINT8 QloadAlarmBusyNum;
	UINT8 QloadAlarmDuration;	/* unit: TBTT */

	UINT32 QloadAlarmNumber;	/* total alarm times */
	BOOLEAN FlgQloadAlarm;	/* 1: alarm occurs */

	/* speed up use */
	UINT32 QloadTimePeriodLast;
	UINT32 QloadBusyTimeThreshold;
#else

#define QLOAD_DOES_ALARM_OCCUR(pAd)	0
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
} QLOAD_CTRL;
#endif /* AP_QLOAD_SUPPORT */

typedef struct  _AUTO_CH_CTRL {
	PCHANNELINFO pChannelInfo;
	PBSSINFO pBssInfoTab;
#ifdef CONFIG_AP_SUPPORT
	AUTOCH_SEL_CTRL AutoChSelCtrl;
#endif
} AUTO_CH_CTRL;

#endif /*CONFIG_AP_SUPPORT*/

typedef struct rtmp_phy_ctrl {
	UINT8 rf_band_cap;
#ifdef CONFIG_AP_SUPPORT
	AUTO_CH_CTRL AutoChCtrl;
#ifdef AP_QLOAD_SUPPORT
	QLOAD_CTRL QloadCtrl;
#endif /*AP_QLOAD_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */
	RADIO_CTRL RadioCtrl;
} RTMP_PHY_CTRL;


typedef struct _MANUAL_CONN {
	UINT8 peer_mac[MAC_ADDR_LEN];
	UINT8 peer_band;		/* band 0/1 */
	UINT8 peer_op_type;		/* ap/sta for "OPMODE_AP"/"OPMODE_STA" */
	UINT8 ownmac_idx;
	UINT8 wtbl_idx;
	UINT8 peer_phy_mode;	/* a/b/g/gn/an/ac for "WMODE_A/B/G/GN/AN/AC" */
	UINT8 peer_bw;			/* 20/40/80/160 for "BW_20/40/80/160" */
	UINT8 peer_nss;			/* 1 ~ 4 */
	UINT16 pfmuId;
	UINT8  spe_idx;
	UCHAR  gid;
	UINT16 aid;
	UINT8  rca2;
	UINT8  rv;
	UINT8 peer_maxrate_mode;	/* cck/ofdm/htmix/htgf/vht for "MODE_CCK/OFDM/HTMIX/HTGF/VHT" */
	UINT32 peer_maxrate_mcs;	/* MODE_CCK: 0~3, MODE_OFDM: 0~7, MODE_HTMIX/GF: 0~32, MODE_VHT:0~9 */
	UINT8 ba_info[WMM_NUM_OF_AC];

	/* protocol wise */
	HT_CAP_INFO ht_cap_info;	/* HT capability information */
#ifdef DOT11_VHT_AC
	VHT_CAP_INFO vht_cap_info;	/* VHT capability information */
	VHT_MCS_SET vht_mcs_set;
#endif
} MANUAL_CONN, *P_MANUAL_CONN;

#ifdef WFA_VHT_R2_PF
typedef struct _MANUAL_DUMP {
	UINT8 VhtBwSignal;
	UINT8 VhtCapIE;
	UINT8 VhtOpNotify;
} MANUAL_DUMP, *P_MANUAL_DUMP;
#endif /* WFA_VHT_R2_PF  */

/* Rx Statistic */
#define MAX_ANT_NUM 4
typedef struct _RX_STATISTIC_RXV {
	INT32 FreqOffsetFromRx;
	UINT32 RSSI[MAX_ANT_NUM];
	UINT32 SNR[MAX_ANT_NUM];
	UINT32 RCPI[MAX_ANT_NUM];
	UINT32 FAGC_RSSI_IB[MAX_ANT_NUM];
	UINT32 FAGC_RSSI_WB[MAX_ANT_NUM];
} RX_STATISTIC_RXV;

typedef struct _RX_STATISTIC_CR {
	UINT32 RxMacFCSErrCount;
	UINT32 RxMacMdrdyCount;
	UINT32 RxMacFCSOKCount;
	UINT32 PhyMdrdyOFDM;
	UINT32 PhyMdrdyCCK;
	UINT32 FCSErr_OFDM;
	UINT32 FCSErr_CCK;
	UINT32 RxLenMismatch;
	UINT32 OFDM_PD;
	UINT32 CCK_PD;
	UINT32 CCK_SIG_Err;
	UINT32 CCK_SFD_Err;
	UINT32 OFDM_SIG_Err;
	UINT32 OFDM_TAG_Err;
	UINT32 ACIHitLow;
	UINT32 ACIHitHigh;
	UINT32 Inst_IB_RSSSI[MAX_ANT_NUM];
	UINT32 Inst_WB_RSSSI[MAX_ANT_NUM];
	UINT32 RxMacFCSErrCount_band1;
	UINT32 RxMacMdrdyCount_band1;
	UINT32 RxMacFCSOKCount_band1;
	UINT32 PhyMdrdyOFDM_band1;
	UINT32 PhyMdrdyCCK_band1;
	UINT32 RxLenMismatch_band1;
	UINT32 OFDM_PD_band1;
	UINT32 CCK_PD_band1;
	UINT32 CCK_SIG_Err_band1;
	UINT32 CCK_SFD_Err_band1;
	UINT32 OFDM_SIG_Err_band1;
	UINT32 OFDM_TAG_Err_band1;
} RX_STATISTIC_CR;

struct peak_tp_ctl {
	UCHAR client_nums;
	/* handle main STA's TP, maximum TP of (TX+RX) */
	struct wifi_dev *main_wdev;
	MAC_TABLE_ENTRY *main_entry;
	BOOLEAN	cli_peak_tp_running;
	UINT32	main_tx_tp;
	UINT32	main_rx_tp;
	UINT8	main_traffc_mode;
	BOOLEAN	cli_peak_tp_txop_enable;
	UINT16	cli_peak_tp_txop_level;
	BOOLEAN	cli_ampdu_efficiency_running;
};

struct txop_ctl {
	UCHAR multi_client_nums;
	UCHAR multi_tcp_nums;
	struct wifi_dev *cur_wdev;
	BOOLEAN multi_cli_txop_running;
	BOOLEAN near_far_txop_running;
#ifdef RX_COUNT_DETECT
	BOOLEAN limit_ampdu_size_running;
#endif /* RX_COUNT_DETECT */
};

struct multi_cli_ctl {
	UINT32 pkt_avg_len;
	UCHAR sta_nums;
	UCHAR amsdu_cnt;
	UINT32 rts_length;
	UINT32 rts_pkt;
	BOOLEAN c2s_only;
	ULONG last_tx_cnt;
	ULONG last_tx_fail_cnt;
};

typedef struct _RxVBQElmt {
	UINT8 valid;
	UINT8 isEnd;
	UINT8 rxblk_setting_done;
	UINT8 rxv_setting_done;
	UINT8 rxblk_search_done;
	UINT8 rxv_search_done;
	UINT8 rxv_sn;
	UINT8 wcid;
	UINT8 aggcnt;
	UINT8 g0_debug_set;
	UINT8 g1_debug_set;
	UINT8 g2_debug_set;
	UCHAR chipid[5];
	UINT32 MPDUbytecnt;
	UINT32 timestamp;
	UINT32 arFCScheckBitmap[8];
	UINT32 RXV_CYCLE[9];
} RxVBQElmt, *PRxVBQElmt;

#ifdef RATE_PRIOR_SUPPORT
typedef struct _LOWRATE_CTRL{
	BOOLEAN RatePrior;/*enable flage*/
	UINT LowRateRatioThreshold;/*percent of total packet in low rate*/
	UINT LowRateCountPeriod;/* low rate detect  perid*/
	UINT TotalCntThreshold;/*threshold of total pack in detected time*/
	DL_LIST BlackList;/*avoid black list of sta in a blk timeout time */
	UINT BlackListTimeout;/*blk time to avoid sta assocated with ap*/
	NDIS_SPIN_LOCK BlackListLock;/*spinlock for ops the BlackList*/
} LOWRATE_CTRL, *PLOWRATE_CTRL;
typedef struct _BLACK_STA {
	UCHAR Addr[MAC_ADDR_LEN];
	DL_LIST List;
	ULONG Jiff;
} BLACK_STA, *PBLACK_STA;
#endif /*RATE_PRIOR_SUPPORT*/

#ifdef WAPP_SUPPORT
struct scan_req {
	INT32 scan_id;
	INT32 last_bss_cnt;
	INT32 if_index;
};
#endif

#if defined(TXRX_STAT_SUPPORT) || defined(OFFCHANNEL_SCAN_FEATURE) || defined(CUSTOMER_RSG_FEATURE)
typedef struct _CHANNEL_STATS {
		UINT32		MibUpdateOBSSAirtime[2];
		UINT32		MibUpdateMyTxAirtime[2];
		UINT32		MibUpdateMyRxAirtime[2];
		UINT32		TotalDuration;
		UINT32		MeasurementDuration;
		UINT32		LastReadTime;
		UINT32		msec100counts;
		UINT32		ChBusytime;
		UINT32		ChBusyTimeAvg;
		UINT32		ChBusyTime1secValue;
		UINT32		CCABusytime;
		UINT32		CCABusyTimeAvg;
		UINT32		CCABusyTime1secValue;
		UINT32		FalseCCACount;
		UINT32		FalseCCACountAvg;
		UINT32		FalseCCACount1secValue;
		UINT32		ChannelApActivity;
		UINT32		ChannelApActivityAvg;
		UINT32		ChannelApActivity1secValue;
		UINT32		MibUpdateEDCCAtime[2];
		UINT32		MibUpdatePdCount[2];
		UINT32		MibUpdateMdrdyCount[2];
		UINT32 		Radio100msecCounts;
} CHANNEL_STATS, *PCHANNEL_STATS;
#endif

/*
	The miniport adapter structure
*/
struct _RTMP_ADAPTER {
	VOID *OS_Cookie;	/* save specific structure relative to OS */
	PNET_DEV net_dev;
	NDIS_SPIN_LOCK WdevListLock;
	struct wifi_dev *wdev_list[WDEV_NUM_MAX];

	/*About MacTab, the sta driver will use #0 and #1 for multicast and AP. */
	MAC_TABLE MacTab;	/* ASIC on-chip WCID entry table.  At TX, ASIC always use key according to this on-chip table. */
	NDIS_SPIN_LOCK MacTabLock;

#ifdef DOT11_N_SUPPORT
	BA_TABLE BATable;
	NDIS_SPIN_LOCK BATabLock;
	RALINK_TIMER_STRUCT RECBATimer;
#endif /* DOT11_N_SUPPORT */

	UINT8	ucBFBackOffMode;

#ifdef RF_LOCKDOWN
	BOOLEAN fgQAEffuseWriteBack;
#endif /* RF_LOCKDOWN */
	INT32 Avg_NF[DBDC_BAND_NUM];
	INT32 Avg_NFx16[DBDC_BAND_NUM];
#if defined(OFFCHANNEL_SCAN_FEATURE) || defined(CUSTOMER_RSG_FEATURE)
	CHANNEL_BUSY_TIME Ch_Stats[DBDC_BAND_NUM];
	UINT32 Ch_BusyTime[DBDC_BAND_NUM];
	CHANNEL_STATS	ChannelStats;
	BOOLEAN EnableChannelStatsCheck;
#endif
#ifdef OFFCHANNEL_SCAN_FEATURE
	SORTED_CHANNEL_LIST  sorted_list;
	UINT8 last_selected_channel;
	BOOLEAN radar_hit;
#endif

#ifdef TXRX_STAT_SUPPORT
	CHANNEL_BUSY_TIME Ch_Stats[DBDC_BAND_NUM];
	UINT32 Ch_BusyTime[DBDC_BAND_NUM];
	UINT32 Ch_BusyTime_11k[DBDC_BAND_NUM];
	CHANNEL_STATS ChannelStats;
#endif

#ifdef LINK_TEST_SUPPORT
	/* state machine state flag */
	UINT8 ucLinkBwState[BAND_NUM];
	UINT8 ucRxStreamState[BAND_NUM];
	UINT8 ucRxStreamStatePrev[BAND_NUM];
	UINT8 ucRxFilterstate[BAND_NUM];
	UINT8 ucTxCsdState[BAND_NUM];
	UINT8 ucTxPwrBoostState[BAND_NUM];
	UINT8 ucLinkRcpiState[BAND_NUM];
	UINT8 ucLinkSpeState[BAND_NUM];
    UINT8 ucLinkSpeStatePrev[BAND_NUM];
    UINT8 ucLinkBwStatePrev[BAND_NUM];
    UINT8 ucTxCsdStatePrev[BAND_NUM];

	/* BW Control Paramter */
	UINT8   ucPrimChannel[BAND_NUM];
	UINT8	ucCentralChannel[BAND_NUM];
	UINT8	ucCentralChannel2[BAND_NUM];
	UINT8	ucExtendChannel[BAND_NUM];
	UINT8	ucHtBw[BAND_NUM];
	UINT8	ucVhtBw[BAND_NUM];
	BOOLEAN fgBwInfoUpdate[BAND_NUM];

	/* Rx Control parameter */
	UINT8	ucRxTestTimeoutCount;
	INT64	c8TempRxCount;
	UINT8	ucRssiTh;
	UINT8	ucRssiSigniTh;
	INT64	c8RxCountTh;
	UINT8	ucTimeOutTh;
	UINT8	ucPerTh;
	CHAR	cNrRssiTh;
	CHAR	cChgTestPathTh;
	UINT8	u1RxSenCount[BAND_NUM];
	UINT8	ucRxSenCountTh;
	UINT8	u1SpeRssiIdxPrev[BAND_NUM];
	CHAR	cWBRssiTh;
	CHAR	cIBRssiTh;
	UINT8	u1RxStreamSwitchReason[BAND_NUM];
    UINT8	ucRxSenCount[BAND_NUM];

	/* ACR Control Parameter */
	UINT8	ucRxFilterConfidenceCnt[BAND_NUM];
	UINT8	ucACRConfidenceCntTh;
	UINT8	ucMaxInConfidenceCntTh;
	CHAR	cMaxInRssiTh;

	/* Tx Control Parameter */
	UINT8	ucCmwCheckCount;
	UINT8	ucCmwCheckCountTh;
	BOOLEAN	fgCmwInstrumBack4T;
	UINT8	ucRssiBalanceCount;
	UINT8	ucRssiIBalanceCountTh;
	BOOLEAN	fgRssiBack4T;
	UINT8	ucCableRssiTh;
	BOOLEAN	fgCmwLinkDone;
	BOOLEAN	fgApclientLinkUp;
	UINT8	ucLinkCount;
	UINT8	ucLinkCountTh;
	BOOLEAN	fgLinkRSSICheck;
	UINT8	ucCmwChannelBand[BAND_NUM];

	/* channel band Control Paramter */
	BOOLEAN fgChannelBandInfoUpdate[BAND_NUM];

	/* Tx Power Control Paramter */
	UINT8	ucTxPwrUpTbl[CMW_POWER_UP_CATEGORY_NUM][CMW_POWER_UP_RATE_NUM];

	/* manual command control function enable/disable flag */
	BOOLEAN fgTxSpeEn;
	BOOLEAN fgRxRcpiEn;
	BOOLEAN fgTxSpurEn;
	BOOLEAN fgRxSensitEn;
	BOOLEAN fgACREn;
#endif /* LINK_TEST_SUPPORT */

#ifdef	ETSI_RX_BLOCKER_SUPPORT
	/* Set fix WBRSSI/IBRSSI pattern */
	BOOLEAN	 fgFixWbIBRssiEn;

	CHAR	 c1WbRssiWF0;
	CHAR	 c1WbRssiWF1;
	CHAR	 c1WbRssiWF2;
	CHAR	 c1WbRssiWF3;
	CHAR	 c1IbRssiWF0;
	CHAR	 c1IbRssiWF1;
	CHAR	 c1IbRssiWF2;
	CHAR	 c1IbRssiWF3;

	/* set RSSI threshold */
	CHAR	 c1RWbRssiHTh;
	CHAR	 c1RWbRssiLTh;
	CHAR	 c1RIbRssiLTh;
	CHAR	 c1WBRssiTh4R;
	/* set RX BLOCKER check para. */
	UINT8	 u1CheckTime;
	UINT8	 u1To1RCheckCnt;
	UINT16	 u2To1RvaildCntTH;
	UINT16   u2To4RvaildCntTH;

	/* for verification */
	UINT8	 u1RxBlockerState;
	UINT8	 u1ValidCnt;
	UINT8	u14RValidCnt;

	UINT8	i1MaxWRssiIdxPrev;
	UINT8	 u1TimeCnt;
	BOOLEAN  fgAdaptRxBlock;
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */

	BOOLEAN fgEPA;

	UINT8 TxPowerSKU[SKU_SIZE];
	UINT8 u1SkuParamLen[SINGLE_SKU_TYPE_PARSE_NUM];
	UINT8 u1SkuParamTransOffset[SINGLE_SKU_TYPE_NUM];
	UINT8 u1SkuChBandNeedParse[SINGLE_SKU_TYPE_PARSE_NUM];
	UINT8 u1SkuFillParamLen[SINGLE_SKU_TYPE_NUM];
	UINT8 u1BackoffParamLen[BACKOFF_TYPE_PARSE_NUM];
	UINT8 u1BackoffParamTransOffset[BACKOFF_TYPE_NUM];
	UINT8 u1BackoffChBandNeedParse[BACKOFF_TYPE_PARSE_NUM];
	UINT8 u1BackoffFillParamLen[BACKOFF_TYPE_NUM];

#ifdef RTMP_MAC_PCI
	/*****************************************************************************************/
	/*	  PCI related parameters	*/
	/*****************************************************************************************/
	PUCHAR CSRBaseAddress;	/* PCI MMIO Base Address, all access will use */
	unsigned int irq_num;
	PCI_HIF_T PciHif;
	USHORT RLnkCtrlConfiguration;
	USHORT RLnkCtrlOffset;
	USHORT HostLnkCtrlConfiguration;
	USHORT HostLnkCtrlOffset;
	USHORT PCIePowerSaveLevel;
#ifdef CONFIG_STA_SUPPORT
	USHORT LnkCtrlBitMask;
	ULONG Rt3xxHostLinkCtrl;	/* USed for 3090F chip */
	ULONG Rt3xxRalinkLinkCtrl;	/* USed for 3090F chip */
	ULONG HostVendor;
#endif /* CONFIG_STA_SUPPORT */
	ULONG AccessBBPFailCount;
	BOOLEAN brt30xxBanMcuCmd;	/* when = 0xff means all commands are ok to set . */
	BOOLEAN b3090ESpecialChip;	/* 3090E special chip that write EEPROM 0x24=0x9280. */
	/* ULONG CheckDmaBusyCount;  // Check Interrupt Status Register Count. */

	NDIS_SPIN_LOCK LockInterrupt;
#ifdef MT7626_E2_SUPPORT
	NDIS_SPIN_LOCK LockInterrupt_md;
#endif
	NDIS_SPIN_LOCK tssi_lock;

	BOOLEAN reschedule_rx0;
	BOOLEAN reschedule_rx1;
#ifdef MT7626_E2_SUPPORT
	BOOLEAN reschedule_rx2;
	BOOLEAN reschedule_rx3;
#endif
	UINT32 wrong_wlan_idx_num;

#ifdef CUT_THROUGH
	VOID *PktTokenCb;
#endif /* CUT_THROUGH */

	NDIS_SPIN_LOCK fp_que_lock;
	QUEUE_HEADER fp_que;
	NDIS_SPIN_LOCK mgmt_que_lock;
	QUEUE_HEADER mgmt_que;
	NDIS_SPIN_LOCK high_prio_que_lock;
	QUEUE_HEADER high_prio_que;
	BOOLEAN tx_dequeue_scheduable;
	RTMP_NET_TASK_STRUCT tx_deque_tasklet;

#ifdef DYNAMIC_STEERING_LOADING
	RTMP_NET_TASK_STRUCT tx_combo_deque_tasklet;
	BOOLEAN combo_tx_dequeue_scheduable;
	NDIS_SPIN_LOCK tx_combo_que_lock;
	QUEUE_HEADER tx_combo_que;
#endif

#ifndef CONFIG_PROPRIETARY_DRIVER
	struct workqueue_struct *qm_wq;
	struct work_struct tx_deq_work;
	struct work_struct cmd_msg_work;

#endif
#ifdef DBG_AMSDU
	UINT32 dbg_time_slot;
	RALINK_TIMER_STRUCT amsdu_history_timer;
#endif
	BOOLEAN amsdu_fix;
	UCHAR amsdu_fix_num;
	UCHAR amsdu_max_num;

#ifdef INT_STATISTIC
	ULONG INTCNT;
#ifdef MT_MAC
	ULONG	INTWFMACINT0CNT;
	ULONG	INTWFMACINT1CNT;
	ULONG	INTWFMACINT2CNT;
	ULONG	INTWFMACINT3CNT;
	ULONG	INTWFMACINT4CNT;
	ULONG	INTBCNDLY;
	ULONG	INTBMCDLY;
#endif
	ULONG INTTxCoherentCNT;
	ULONG INTRxCoherentCNT;
	ULONG INTTxRxCoherentCNT;
	ULONG INTFifoStaFullIntCNT;
	ULONG INTMGMTDLYCNT;
	ULONG INTRXDATACNT;
	ULONG INTRXCMDCNT;
	ULONG INTHCCACNT;
	ULONG INTAC3CNT;
	ULONG INTAC2CNT;
	ULONG INTAC1CNT;
	ULONG INTAC0CNT;

	ULONG INTPreTBTTCNT;
	ULONG INTTBTTIntCNT;
	ULONG INTGPTimeOutCNT;
	ULONG INTRadarCNT;
	ULONG INTAutoWakeupIntCNT;
#endif

#endif /* RTMP_MAC_PCI */

	NDIS_SPIN_LOCK irq_lock;

	/*======Cmd Thread in PCI/RBUS/USB */
	CmdQ CmdQ;
	NDIS_SPIN_LOCK CmdQLock;	/* CmdQLock spinlock */
	RTMP_OS_TASK cmdQTask;
	HW_CTRL_T HwCtrl;
	RTMP_OS_SEM AutoRateLock;



#if defined(CONFIG_AP_SUPPORT) && defined(AP_SCAN_SUPPORT) && defined(OFFCHANNEL_SCAN_FEATURE)
	CHANNELINFO ChannelInfo;
#endif

#ifdef CONFIG_ATE
	/* lock for ATE */
	NDIS_SPIN_LOCK GenericLock;	/* ATE Tx/Rx generic spinlock */
#endif /* CONFIG_ATE */


	/*********************************************************/
	/*	  Both PCI/USB related parameters										 */
	/*********************************************************/
	/*RTMP_DEV_INFO				 chipInfo; */
	RTMP_INF_TYPE infType;
	UCHAR			AntMode;

	/*********************************************************/
	/*	  Driver Mgmt related parameters											*/
	/*********************************************************/
	/* OP mode: either AP or STA */
	UCHAR OpMode;		/* OPMODE_STA, OPMODE_AP */

	UINT32 BssInfoIdxBitMap0;/* mapping BssInfoIdx inside wdev struct with FW BssInfoIdx */
	UINT32 BssInfoIdxBitMap1;/* mapping BssInfoIdx inside wdev struct with FW BssInfoIdx */
	NDIS_SPIN_LOCK BssInfoIdxBitMapLock;

	UCHAR iface_combinations; /* AP , STA , AP+STA */

	RTMP_OS_TASK mlmeTask;
#ifdef RTMP_TIMER_TASK_SUPPORT
	/* If you want use timer task to handle the timer related jobs, enable this. */
	RTMP_TIMER_TASK_QUEUE TimerQ;
	NDIS_SPIN_LOCK TimerQLock;
	RTMP_OS_TASK timerTask;
#endif /* RTMP_TIMER_TASK_SUPPORT */

	/*********************************************************/
	/*	  Tx related parameters														   */
	/*********************************************************/
	BOOLEAN *DeQueueRunning;	/* for ensuring RTUSBDeQueuePacket get call once */
	NDIS_SPIN_LOCK *DeQueueLock;


	/* resource for software backlog queues */
	QUEUE_HEADER *TxSwQueue;	/* 4 AC + 1 HCCA */
	NDIS_SPIN_LOCK *TxSwQueueLock;	/* TxSwQueue spinlock */
#if defined(MT_MAC) && defined(IP_ASSEMBLY)
	DL_LIST *assebQueue;
	VOID *cur_ip_assem_data;
#endif

	/* Maximum allowed tx software Queue length */
	UINT TxSwQMaxLen;
	NDIS_SPIN_LOCK tx_swq_lock[WMM_NUM_OF_AC];
	struct tx_swq_fifo tx_swq[WMM_NUM_OF_AC];


	UCHAR LastMCUCmd;

#ifdef REDUCE_TCP_ACK_SUPPORT
	struct hlist_head ackCnxHashTbl[REDUCE_ACK_MAX_HASH_BUCKETS];
	struct list_head ackCnxList;
	UINT32 ReduceAckConnections;
	struct delayed_work ackFlushWork;
	struct delayed_work cnxFlushWork;
	NDIS_SPIN_LOCK ReduceAckLock;
#endif

#ifdef RACTRL_LIMIT_MAX_PHY_RATE
	BOOLEAN fgRaLimitPhyRate;
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */

	/*********************************************************/
	/*	  Rx related parameters														  */
	/*********************************************************/

#ifdef RTMP_MAC_PCI
	/* TODO: shiang, check the purpose of following lock "McuCmdLock" */
	NDIS_SPIN_LOCK McuCmdLock;	/*MCU Command Queue spinlock for RT3090/3592/3390/3593/5390/5392/5592/3290 */
#endif /* RTMP_MAC_PCI */


	/* RX re-assembly buffer for fragmentation */
	FRAGMENT_FRAME FragFrame;	/* Frame storage for fragment frame */

#ifdef MT_MAC
	TXS_CTL TxSCtl;
	TMR_CTRL_STRUCT *pTmrCtrlStruct;
#endif



	/***********************************************************/
	/*	  ASIC related parameters														  */
	/***********************************************************/
	struct qm_ctl qm_ctl;
	struct qm_ops *qm_ops;
	struct notify_entry qm_wsys_ne;
	struct tm_ops *tm_qm_ops;
	struct tm_ops *tm_hif_ops;
	struct phy_ops *phy_op;
	struct hw_setting hw_cfg;

	UINT32 MACVersion;	/* MAC version. Record rt2860C(0x28600100) or rt2860D (0x28600101).. */
	UINT32 ChipID;
	UINT32 HWVersion;
	UINT32 FWVersion;
	UINT16 ee_chipId;		/* Chip version. Read from EEPROM 0x00 to identify RT5390H */
	INT dev_idx;

#ifdef MT_MAC
	struct rtmp_mac_ctrl mac_ctrl;
	USHORT rx_pspoll_filter;
#endif /* MT_MAC */

	/* --------------------------- */
	/* E2PROM									 */
	/* --------------------------- */
	enum EEPROM_STORAGE_TYPE eeprom_type;

	ULONG EepromVersion;	/* byte 0: version, byte 1: revision, byte 2~3: unused */
	ULONG FirmwareVersion;	/* byte 0: Minor version, byte 1: Major version, otherwise unused. */
	USHORT EEPROMDefaultValue[NUM_EEPROM_BBP_PARMS];
	UCHAR EEPROMAddressNum;	/* 93c46=6  93c66=8 */
	BOOLEAN EepromAccess;
	UCHAR EFuseTag;

#ifdef RTMP_EFUSE_SUPPORT
	BOOLEAN bUseEfuse;
#endif /* RTMP_EFUSE_SUPPORT */

	UCHAR *EEPROMImage;

#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT
		EFUSE_INFO_ALL_T EfuseInfoAll;
#endif /*#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT*/

	UCHAR E2pAccessMode; /* Used to identify flash, efuse, eeprom or bin from start-up */
	struct _EEPROM_CONTROL E2pCtrl;

#ifdef RF_LOCKDOWN
	BOOLEAN fgCalFreeApply;
	UINT16  RFlockTempIdx;
	UINT16  CalFreeTempIdx;
#endif /* RF_LOCKDOWN */

#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
	UINT32 CalFileOffset;
#endif /* CAL_BIN_FILE_SUPPORT */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	BOOLEAN bDCOCReloaded;
	BOOLEAN bDPDReloaded;
	UCHAR *CalDCOCImage;
	UCHAR *CalDPDAPart1Image;
	UCHAR *CalDPDAPart2Image;
#endif /* PRE_CAL_TRX_SET1_SUPPORT */

#ifdef PRE_CAL_MT7622_SUPPORT
#define CAL_LOG_SIZE            5000
#define CAL_TXLPFG_SIZE		(4  * sizeof(UINT32))
#define CAL_TXDCIQ_SIZE		(48 * sizeof(UINT32))
#define CAL_TXDPD_PERCHAN_SIZE  (304 * sizeof(UINT32))
#define CAL_TXDPD_SIZE          (CAL_TXDPD_PERCHAN_SIZE * 14)
	BOOLEAN bPreCalMode;
	UCHAR *CalTXLPFGImage;
	UCHAR *CalTXDCIQImage;
	UCHAR *CalTXDPDImage;
	UINT16 TxDpdCalOfst;
#endif /*PRE_CAL_MT7622_SUPPORT*/

#if defined(PRE_CAL_MT7663_SUPPORT) || defined(PRE_CAL_MT7626_SUPPORT)
#define CAL_LOG_SIZE            6000
	BOOLEAN bPreCalMode;
	UCHAR   *PreCalImageInfo;
	UCHAR   *PreCalImage;
	UINT32  PreCalOfst;
	UCHAR   *TxDPDImage;
	UINT32  TxDPDOfst;
#endif /* PRE_CAL_MT7663_SUPPORT || PRE_CAL_MT7626_SUPPORT */

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
	UCHAR *PreCalReStoreBuffer;
	UCHAR *PreCalStoreBuffer;
	UINT16 PreCalWriteOffSet;
	UINT16 ChGrpMap;
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
	VOID *rlmCalCache;
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

	EEPROM_ANTENNA_STRUC Antenna;	/* Since ANtenna definition is different for a & g. We need to save it for future reference. */
#ifdef DBDC_MODE
	UINT8 dbdc_band0_tx_path;
	UINT8 dbdc_band0_rx_path;
	UINT8 dbdc_band1_tx_path;
	UINT8 dbdc_band1_rx_path;
#endif
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;

	/* --------------------------- */
	/* BBP Control							   */
	/* --------------------------- */
#if defined(RTMP_BBP) || defined(CONFIG_ATE)
	/* TODO: shiang-6590, remove "defined(CONFIG_ATE)" after ATE has fully re-organized! */
	UCHAR BbpWriteLatch[MAX_BBP_ID + 1];	/* record last BBP register value written via BBP_IO_WRITE/BBP_IO_WRITE_VY_REG_ID */
	UCHAR Bbp94;
#endif /* defined(RTMP_BBP) || defined(CONFIG_ATE) */

	CHAR BbpRssiToDbmDelta;	/* change from UCHAR to CHAR for high power */
	BBP_R66_TUNING BbpTuning;

	/* ---------------------------- */
	/* RFIC control								 */
	/* ---------------------------- */
	UCHAR RfIcType;		/* RFIC_xxx */
	UCHAR RfFreqOffset;	/* Frequency offset for channel switching */

	RTMP_RF_REGS LatchRfRegs;	/* latch th latest RF programming value since RF IC doesn't support READ */

#ifdef RTMP_RBUS_SUPPORT
	UCHAR RfFreqDelta;	/* Frequency Delta */
#endif /* RTMP_RBUS_SUPPORT */
#ifdef RTMP_MAC_PCI
	UCHAR ShowRf;		/* Show RF register write for 2880 */
#endif /* RTMP_MAC_PCI */

	/* This soft Rx Antenna Diversity mechanism is used only when user set */
	/* RX Antenna = DIVERSITY ON */
	SOFT_RX_ANT_DIVERSITY RxAnt;

	/* ---------------------------- */
	/* TxPower control						   */
	/* ---------------------------- */
	CHANNEL_TX_POWER TxPower[MAX_NUM_OF_CHANNELS];	/* Store Tx power value for all channels. */
	CHANNEL_TX_POWER ChannelList[MAX_NUM_OF_CHANNELS];	/* list all supported channels for site survey */

	UCHAR MaxTxPwr;/*announced in beacon*/

	UCHAR ChannelListNum;	/* number of channel in ChannelList[] */
	ULONG Tx20MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx20MPwrCfgGBand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx40MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx40MPwrCfgGBand[MAX_TXPOWER_ARRAY_SIZE];
#ifdef DOT11_VHT_AC
	ULONG Tx80MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE]; /* Per-rate Tx power control for VHT BW80 (5GHz only) */
	BOOLEAN force_vht_op_mode;
#endif /* DOT11_VHT_AC */

#ifdef THERMAL_PROTECT_SUPPORT
	BOOLEAN force_one_tx_stream;
	BOOLEAN fgThermalProtectToggle;
#endif /* THERMAL_PROTECT_SUPPORT */

	signed char BGRssiOffset[4]; /* Store B/G RSSI #0/1/2 Offset value on EEPROM 0x46h */
	signed char ARssiOffset[4]; /* Store A RSSI 0/1/2 Offset value on EEPROM 0x4Ah */

	CHAR BLNAGain;		/* Store B/G external LNA#0 value on EEPROM 0x44h */
	CHAR ALNAGain0;		/* Store A external LNA#0 value for ch36~64 */
	CHAR ALNAGain1;		/* Store A external LNA#1 value for ch100~128 */
	CHAR ALNAGain2;		/* Store A external LNA#2 value for ch132~165 */

#ifdef LED_CONTROL_SUPPORT
	/* LED control */
	LED_CONTROL LedCntl;
#endif /* LED_CONTROL_SUPPORT */

	/* ---------------------------- */
	/* MAC control								 */
	/* ---------------------------- */
	UCHAR wmm_cw_min; /* CW_MIN_IN_BITS, actual CwMin = 2^CW_MIN_IN_BITS - 1 */
	UCHAR wmm_cw_max; /* CW_MAX_IN_BITS, actual CwMax = 2^CW_MAX_IN_BITS - 1 */

	/*****************************************************************************************/
	/*	  802.11 related parameters														*/
	/*****************************************************************************************/
	/* outgoing BEACON frame buffer and corresponding TXD */
	USHORT BeaconOffset[HW_BEACON_MAX_NUM];

	/* pre-build PS-POLL and NULL frame upon link up. for efficiency purpose. */
#ifdef CONFIG_STA_SUPPORT
	PSPOLL_FRAME PsPollFrame;
#endif /* CONFIG_STA_SUPPORT */
	HEADER_802_11 NullFrame;



#ifdef UAPSD_SUPPORT
	NDIS_SPIN_LOCK UAPSDEOSPLock;	/* EOSP frame access lock use */
	BOOLEAN bAPSDFlagSPSuspend;	/* 1: SP is suspended; 0: SP is not */
#endif /* UAPSD_SUPPORT */

	MONITOR_STRUCT monitor_ctrl;
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
	struct ch_switch_cfg ch_sw_cfg;
#endif

#ifdef DOT11_SAE_SUPPORT
	SAE_CFG SaeCfg;
#endif /* DOT11_SAE_SUPPORT */

	/*=========AP=========== */
#ifdef CONFIG_AP_SUPPORT
	/* ----------------------------------------------- */
	/* AP specific configuration & operation status */
	/* used only when pAd->OpMode == OPMODE_AP */
	/* ----------------------------------------------- */
	AP_ADMIN_CONFIG ApCfg;	/* user configuration when in AP mode */
	AP_MLME_AUX ApMlmeAux;

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || defined(AXE) || defined(MT7626)
	BCN_CHECK_INFO_STRUC BcnCheckInfo;
#endif

#ifdef RT_CFG80211_SUPPORT
	RADIUS_ACCOUNT_ENTRY  radius_tbl[MAX_LEN_OF_MAC_TABLE];
#endif


#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
	WDS_TABLE WdsTab;	/* WDS table when working as an AP */
#endif /* WDS_SUPPORT */

#ifdef MBSS_SUPPORT
	BOOLEAN FlgMbssInit;
#endif /* MBSS_SUPPORT */

#ifdef APCLI_SUPPORT
	BOOLEAN flg_apcli_init;
#endif /* APCLI_SUPPORT */

	/*#ifdef AUTO_CH_SELECT_ENHANCE */
	PBSSINFO pBssInfoTab;
	/*#endif // AUTO_CH_SELECT_ENHANCE */

#endif /* CONFIG_AP_SUPPORT */

	/*=======STA=========== */
#ifdef CONFIG_STA_SUPPORT
	/* ----------------------------------------------- */
	/* STA specific configuration & operation status */
	/* used only when pAd->OpMode == OPMODE_STA */
	/* ----------------------------------------------- */
	STA_ADMIN_CONFIG	StaCfg[MAX_MULTI_STA];	/* user desired settings */
	UCHAR			   MSTANum;				/* Actual working MSTA number */
	UCHAR			   MaxMSTANum;			 /* Mini of Max MSTA number from profile & MAX_MULTI_STA*/

	BOOLEAN			flg_msta_init;

	CHAR nickname[IW_ESSID_MAX_SIZE + 1];	/* nickname, only used in the iwconfig i/f */
	NDIS_MEDIA_STATE PreMediaState;
#endif /* CONFIG_STA_SUPPORT */

	/*=======Common=========== */
	enum RATE_ADAPT_ALG rateAlg;		/* Rate adaptation algorithm */

#ifdef PROFILE_PATH_DYNAMIC
	CHAR *profilePath;
#endif /* PROFILE_PATH_DYNAMIC */

	NDIS_MEDIA_STATE IndicateMediaState;	/* Base on Indication state, default is NdisMediaStateDisConnected */

#ifdef PROFILE_STORE
	RTMP_OS_TASK	WriteDatTask;
	BOOLEAN			bWriteDat;
#endif /* PROFILE_STORE */

#ifdef CREDENTIAL_STORE
	STA_CONNECT_INFO StaCtIf;
#endif /* CREDENTIAL_STORE */

#ifdef WSC_INCLUDED
	RTMP_OS_TASK wscTask;
	UCHAR WriteWscCfgToDatFile;
	BOOLEAN WriteWscCfgToAr9DatFile;
	NDIS_SPIN_LOCK WscElmeLock;
	MLME_QUEUE_ELEM *pWscElme;

	/* WSC hardware push button function 0811 */
	BOOLEAN WscHdrPshBtnFlag;	/* 1: support, read from EEPROM */
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN bWscDriverAutoUpdateCfg;
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */

	/* MAT related parameters */
#ifdef MAT_SUPPORT
	MAT_STRUCT MatCfg;
#endif /* MAT_SUPPORT */

	/*
		Frequency setting for rate adaptation
			@ra_interval:		for baseline time interval
			@ra_fast_interval:	for quick response time interval
	*/
	UINT32 ra_interval;
	UINT32 ra_fast_interval;

	/* configuration: read from Registry & E2PROM */
	BOOLEAN bLocalAdminMAC;	/* Use user changed MAC */
	UCHAR PermanentAddress[MAC_ADDR_LEN];	/* Factory default MAC address */
	UCHAR CurrentAddress[MAC_ADDR_LEN];	/* User changed MAC address */

#ifdef MT_MAC
	UCHAR ExtendMBssAddr[EXTEND_MBSS_MAC_MAX][MAC_ADDR_LEN]; /* User defined MAC address for MBSSID*/
	BOOLEAN bLocalAdminExtendMBssMAC; /* Use user changed MAC */
#endif

	/* ------------------------------------------------------ */
	/* common configuration to both OPMODE_STA and OPMODE_AP */
	/* ------------------------------------------------------ */
	UINT VirtualIfCnt;
	NDIS_SPIN_LOCK VirtualIfLock;

	COMMON_CONFIG CommonCfg;
#ifdef COEX_SUPPORT
	BOOLEAN BtCoexBeaconLimit;
	UINT32 BtWlanStatus;
	UINT32 BtCoexSupportMode;
	UINT32 BtCoexMode;
	UINT32 BtCoexLiveTime;
	UINT32 BtCoexBASize;
	UINT32 BtCoexOriBASize;
	UCHAR BtProtectionMode;
	UCHAR BtProtectionRate;
	UCHAR BtSkipFDDFix20MH;
#endif /* COEX_SUPPORT */
	MLME_STRUCT Mlme;

	/* AP needs those vaiables for site survey feature. */

#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	SCAN_CTRL ScanCtrl[DBDC_BAND_NUM];
#endif /* defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT) */
#ifdef TXRX_STAT_SUPPORT
	BOOLEAN TXRX_EnableReadRssi;
	BOOLEAN EnableTxRxStats;
#endif

	EXT_CAP_INFO_ELEMENT ExtCapInfo;

#ifdef AIR_MONITOR
	UCHAR	MntEnable;
	MNT_STA_ENTRY MntTable[MAX_NUM_OF_MONITOR_STA];
	MNT_MUAR_GROUP MntGroupTable[MAX_NUM_OF_MONITOR_GROUP];
	UCHAR	MntIdx;
	UCHAR	curMntAddr[MAC_ADDR_LEN];
	UINT32 MonitrCnt[BAND_NUM];
	UCHAR	MntRuleBitMap;
#endif /* AIR_MONITOR */

#ifdef DSCP_PRI_SUPPORT
	INT8 dscp_pri_map[2][64];  /*priority mapping for dscp values */
#endif

	/* DOT11_H */
	struct DOT11_H Dot11_H[DBDC_BAND_NUM];

	/* encryption/decryption KEY tables */
	CIPHER_KEY SharedKey[HW_BEACON_MAX_NUM + MAX_P2P_NUM][4];	/* STA always use SharedKey[BSS0][0..3] */

	/* various Counters */
	COUNTER_802_3 Counters8023;	/* 802.3 counters */
	COUNTER_802_11 WlanCounters[DBDC_BAND_NUM];	/* 802.11 MIB counters */
	COUNTER_RALINK RalinkCounters;	/* Ralink propriety counters */
	/* COUNTER_DRS DrsCounters;	*/ /* counters for Dynamic TX Rate Switching */
	PRIVATE_STRUC PrivateInfo;	/* Private information & counters */

	/* flags, see fRTMP_ADAPTER_xxx flags */
	ULONG Flags;		/* Represent current device status */
	ULONG PSFlags;		/* Power Save operation flag. */
	ULONG MoreFlags;	/* Represent specific requirement */

	/* current TX sequence # */
	USHORT Sequence;

	/* Control disconnect / connect event generation */
	/*+++Didn't used anymore */
	ULONG LinkDownTime;
	/*--- */
	ULONG LastRxRate;
	ULONG LastTxRate;

#ifdef CONFIG_AP_SUPPORT
	BOOLEAN bConfigChanged;	/* Config Change flag for the same SSID setting */
#endif
	/*+++Used only for Station */
	BOOLEAN bDisableRtsProtect;
	/*--- */

	ULONG ExtraInfo;	/* Extra information for displaying status of UI */
	ULONG SystemErrorBitmap;	/* b0: E2PROM version error */

#ifdef SYSTEM_LOG_SUPPORT
	/* --------------------------- */
	/* System event log					   */
	/* --------------------------- */
	RT_802_11_EVENT_TABLE EventTab;
#endif /* SYSTEM_LOG_SUPPORT */

#ifdef INF_PPA_SUPPORT
	UINT32 g_if_id;
	BOOLEAN PPAEnable;
	PPA_DIRECTPATH_CB *pDirectpathCb;
#endif /* INF_PPA_SUPPORT */

	/**********************************************************/
	/*	  Statistic related parameters													*/
	/**********************************************************/

	BOOLEAN bUpdateBcnCntDone;

	ULONG macwd;
	/* ---------------------------- */
	/* DEBUG paramerts */
	/* ---------------------------- */

	BOOLEAN bLinkAdapt;
	BOOLEAN bForcePrintTX;
	BOOLEAN bForcePrintRX;
	BOOLEAN bStaFifoTest;
	BOOLEAN bProtectionTest;
	BOOLEAN bHCCATest;
	BOOLEAN bGenOneHCCA;
	BOOLEAN bBroadComHT;
	ULONG BulkOutReq;
	ULONG BulkOutComplete;
	ULONG BulkOutCompleteOther;
	ULONG BulkOutCompleteCancel;	/* seems not use now? */
	struct wificonf WIFItestbed;

	UCHAR TssiGain;


#ifdef CONFIG_ATE
	struct _ATE_CTRL ATECtrl;
	struct _LOOPBACK_CTRL LbCtrl;
#endif
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	BOOLEAN KtoFlashDebug;
#endif

#ifdef DOT11_N_SUPPORT
	struct reordering_mpdu_pool mpdu_blk_pool;
#endif /* DOT11_N_SUPPORT */

	/* statistics count */

	VOID *iw_stats;
	VOID *stats;

#ifdef BLOCK_NET_IF
	BLOCK_QUEUE_ENTRY *blockQueueTab;
#endif /* BLOCK_NET_IF */

#ifdef CONFIG_AP_SUPPORT
#ifdef IGMP_SNOOP_SUPPORT
	PMULTICAST_FILTER_TABLE pMulticastFilterTable;
	UCHAR IgmpGroupTxRate;
#endif /* IGMP_SNOOP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	ULONG TbttTickCount;	/* beacon timestamp work-around */

#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT PeriodicTimer;
#endif /* CONFIG_AP_SUPPORT */

	/* for detect_wmm_traffic() BE TXOP use */
	ULONG tx_OneSecondnonBEpackets;	/* record non BE packets per second */
	ULONG rx_OneSecondnonBEpackets;
#define ONE_SECOND_NON_BE_PACKETS_THRESHOLD	(50)
	UCHAR is_on;

	/* for detect_wmm_traffic() BE/BK TXOP use */
#define TIME_BASE			(1000000/OS_HZ)
#define TIME_ONE_SECOND		(1000000/TIME_BASE)
	UCHAR flg_be_adjust;
	ULONG be_adjust_last_time;
#ifdef WSC_INCLUDED
	/* for multiple card */
	UCHAR *pHmacData;
#endif /* WSC_INCLUDED */

#ifdef IKANOS_VX_1X0
	struct IKANOS_TX_INFO IkanosTxInfo;
	struct IKANOS_TX_INFO IkanosRxInfo[HW_BEACON_MAX_NUM + MAX_WDS_ENTRY +
										   MAX_APCLI_NUM + MAX_MESH_NUM];
#endif /* IKANOS_VX_1X0 */



	UINT8 FlgCtsEnabled;
	UINT8 PM_FlgSuspend;

#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	/* RIC */
#define FT_RIC_CB		((FT_RIC_CTRL_BLOCK *)(pAd->pFT_RIC_Ctrl_BK))
	VOID *pFT_RIC_Ctrl_BK;
	NDIS_SPIN_LOCK FT_RicLock;
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
	LIST_HEADER DMSEntryList;
	LIST_HEADER FMSEntryList;
#endif /* DOT11V_WNM_SUPPORT */

	UCHAR FifoUpdateDone, FifoUpdateRx;

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
	CFG80211_CTRL cfg80211_ctrl;
	VOID *pCfg80211_CB;
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */

#ifdef OS_ABL_SUPPORT
#ifdef MAT_SUPPORT
	/* used in OS_ABL */
	BOOLEAN (*MATPktRxNeedConvert)(RTMP_ADAPTER *pAd, PNET_DEV net_dev);

	PUCHAR (*MATEngineRxHandle)(RTMP_ADAPTER *pAd, PNDIS_PACKET pPkt, UINT infIdx);
#endif /* MAT_SUPPORT */
#endif /* OS_ABL_SUPPORT */

	UINT32 ContinueMemAllocFailCount;

	struct {
		INT IeLen;
		UCHAR *pIe;
	} ProbeRespIE[MAX_LEN_OF_BSS_TABLE];

	/* purpose: We free all kernel resources when module is removed */
	LIST_HEADER RscTimerMemList;	/* resource timers memory */
	LIST_HEADER RscTaskMemList;	/* resource tasks memory */
	LIST_HEADER RscLockMemList;	/* resource locks memory */
	LIST_HEADER RscTaskletMemList;	/* resource tasklets memory */
	LIST_HEADER RscSemMemList;	/* resource semaphore memory */
	LIST_HEADER RscAtomicMemList;	/* resource atomic memory */

	/* purpose: Cancel all timers when module is removed */
	LIST_HEADER RscTimerCreateList;	/* timers list */

#ifdef OS_ABL_SUPPORT
#ifdef RTMP_PCI_SUPPORT
	RTMP_PCI_CONFIG PciConfig;
#endif /* RTMP_PCI_SUPPORT */
#endif /* OS_ABL_SUPPORT */




#ifdef CONFIG_MULTI_CHANNEL
	CHAR NullFrBuf[256];
	UINT32 MultiChannelFlowCtl;
	RTMP_OS_TASK MultiChannelTask;
	UCHAR MultiChannelAction;
#endif /* CONFIG_MULTI_CHANNEL */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
	WOW_CFG_STRUCT WOW_Cfg; /* data structure for wake on wireless */
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT) */

#ifdef MULTI_MAC_ADDR_EXT_SUPPORT
	BOOLEAN bUseMultiMacAddrExt;
#endif /* MULTI_MAC_ADDR_EXT_SUPPORT */

#ifdef HW_TX_RATE_LOOKUP_SUPPORT
	BOOLEAN bUseHwTxLURate;
#endif /* HW_TX_RATE_LOOKUP_SUPPORT */

#ifdef CONFIG_ANDES_SUPPORT
	struct MCU_CTRL MCUCtrl;
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef WLAN_SKB_RECYCLE
	struct sk_buff_head rx0_recycle;
#endif /* WLAN_SKB_RECYCLE */

#ifdef CONFIG_MULTI_CHANNEL
	/* TODO: shiang-usw, duplicate definition with */
	USHORT NullBufOffset[2];
	CHAR NullFrBuf[100];
	UINT32 NullFrLen;
	/*
	UINT32 MultiChannelFlowCtl;
	RTMP_OS_TASK MultiChannelTask;
	UCHAR MultiChannelAction;
	*/
#endif /* CONFIG_MULTI_CHANNEL */

	DL_LIST PwrLimitSkuList;
	DL_LIST PwrLimitBackoffList;
	BOOLEAN fgPwrLimitRead[POWER_LIMIT_TABLE_TYPE_NUM];
	UCHAR DefaultTargetPwr;
	CHAR SingleSkuRatePwrDiff[19];
	BOOLEAN bOpenFileSuccess;

#if defined(WFA_VHT_PF) || defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA) || defined(MT7637_FPGA)
	BOOLEAN force_amsdu;
#endif
#ifdef WFA_VHT_PF
	BOOLEAN force_noack;
	BOOLEAN vht_force_sgi;
	BOOLEAN vht_force_tx_stbc;

	BOOLEAN force_vht_op_mode;
	UCHAR vht_pf_op_ss;
	UCHAR vht_pf_op_bw;
#endif /* WFA_VHT_PF */


#ifdef DBG_DIAGNOSE
	RtmpDiagStruct DiagStruct;
#endif /* DBG_DIAGNOSE */

#ifdef SNIFFER_SUPPORT
	struct sniffer_control sniffer_ctl;
#endif


#ifdef TXBF_SUPPORT
	MANUAL_CONN	AteManualConnInfo;
	PUCHAR		piBfPhaseG0;
	PUCHAR		piBfPhaseGx;
	BOOLEAN		fgCalibrationFail;
	BOOLEAN		fgGroupIdPassFailStatus[9];
	BOOLEAN		fgAutoStart;
#ifdef VHT_TXBF_SUPPORT
	BOOLEAN		NDPA_Request;
#endif
	TXBF_PFMU_STA_INFO rStaRecBf;
	PFMU_PROFILE_TAG1  rPfmuTag1;
	PFMU_PROFILE_TAG2  rPfmuTag2;
	PFMU_DATA	prof;
	UCHAR		ApCli_idx;
	UCHAR		ApCli_CmmWlanId;
	BOOLEAN		fgApCliBfStaRecRegister;
	BOOLEAN		fgApClientMode;
	BOOLEAN		fgClonedStaWithBfeeSelected;
	UCHAR		ReptClonedStaEntry_CliIdx;
	UCHAR		u1IbfCalPhase2G5GE2pClean;
	BF_DYNAMIC_MECHANISM bfdm;
	UCHAR       profile_data_cnt;
	PFMU_HALF_DATA     profile_data[64];
#endif /* TXBF_SUPPORT */

#ifdef CFG_SUPPORT_MU_MIMO
	UINT32		u4RxMuPktCount;
	UINT32		u4TxMuPktCount;
#endif /* CFG_SUPPORT_MU_MIMO  */

	BOOLEAN		bPS_Retrieve;

	UINT32 rxv2_cyc3[10];

#ifdef DBG
#ifdef MT_MAC
	UCHAR BcnCnt; /* Carter debug */
	ULONG HandleInterruptTime;
	ULONG HandlePreInterruptTime;
#endif
#endif


#ifdef RTMP_MAC_PCI
	UINT32 RxResetDropCount;
	BOOLEAN RxRest;
	BOOLEAN PDMAWatchDogEn;
	BOOLEAN PDMAWatchDogDbg;
	UINT8 TxDMACheckTimes;
	UINT8 RxDMACheckTimes;
	ULONG TxDMAResetCount;
	ULONG RxDMAResetCount;
	ULONG PDMAResetFailCount;
#endif
	BOOLEAN PSEWatchDogEn;
	UINT8 RxPseCheckTimes;
	RTMP_OS_SEM IndirectUpdateLock;
	ULONG PSEResetCount;
	ULONG PSETriggerType1Count;
	ULONG PSETriggerType2Count;
	ULONG PSEResetFailCount;

	ULONG   TxTotalByteCnt;
	ULONG   RxTotalByteCnt;
#ifdef MT_MAC
	TX_AC_PARAM_T   CurrEdcaParam[CMD_EDCA_AC_MAX];
#endif /* MT_MAC */

#ifdef CONFIG_DVT_MODE
	DVTCTRL_T rDvtCtrl;
#endif /* CONFIG_DVT_MODE */

	/* ------------------- */
	/* For heart beat detection*/
	/* ------------------- */
	UINT32 pre_cr4_heart_beat_cnt;
	UINT32 pre_n9_heart_beat_cnt;
	UINT8 heart_beat_stop;

#ifdef FW_DUMP_SUPPORT
	/* ---------------------------- */
	/* For FW crash dump			*/
	/* ---------------------------- */
#define MAX_FW_DUMP_SIZE 1024000
	UCHAR *fw_dump_buffer;
	UINT32 fw_dump_max_size;
	UINT32 fw_dump_size;
	UINT32 fw_dump_read;
#endif

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
	INT32 SpectrumStatus;
	INT32 ICapStatus;
	UINT8 ICapMode;
	UINT32 SpectrumEventCnt;
	UINT32 SpectrumIdx;
	UINT32 ICapEventCnt;
	UINT32 ICapDataCnt;
	UINT32 ICapL32Cnt;
	UINT32 ICapM32Cnt;
	UINT32 ICapH32Cnt;
	UINT32 ICapIdx;
	UINT32 ICapCapLen;
	PUINT32 pL32Bit;
	PUINT32 pM32Bit;
	PUINT32 pH32Bit;
	RTMP_OS_FD pSrcf_IQ;
	RTMP_OS_FD pSrcf_Gain;
	RTMP_STRING *pSrc_IQ;
	RTMP_STRING *pSrc_Gain;
	RTMP_OS_COMPLETION SpectrumDumpDataDone;
	RTMP_OS_COMPLETION ICapDumpDataDone;
	P_RBIST_IQ_DATA_T pIQ_Array;
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#ifdef BACKGROUND_SCAN_SUPPORT
	BACKGROUND_SCAN_CTRL BgndScanCtrl;
#endif /* BACKGROUND_SCAN_SUPPORT */
	VOID *hdev_ctrl;
#ifdef CONFIG_AP_SUPPORT
	/*for VOW HW CR address change.*/
	VOW_CR_OFFSET_FOR_GEN_T vow_gen;
#endif/* CONFIG_AP_SUPPORT */
#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	VOW_RX_TIME_CFG_T  vow_rx_time_cfg;
	VOW_BSS_USER_CFG_T vow_bss_cfg[VOW_MAX_GROUP_NUM];
	VOW_STA_USER_CFG_T vow_sta_cfg[MAX_LEN_OF_MAC_TABLE];
	VOW_CFG_T		  vow_cfg;
	VOW_AT_ESTIMATOR_T vow_at_est;
	VOW_BAD_NODE_T	 vow_badnode;
	VOW_MISC_CFG_T	   vow_misc_cfg;
	UINT8  vow_dvt_en;
	UINT8  vow_show_en;
	UINT8  vow_monitor_sta;
	UINT8  vow_monitor_bss;
	UINT8  vow_monitor_mbss;
	UINT16 vow_avg_num;
	UINT32 vow_show_sta;
	UINT32 vow_show_mbss;

	UINT8	vow_watf_en;
	UINT8	vow_watf_q_lv0;
	UINT8	vow_watf_q_lv1;
	UINT8	vow_watf_q_lv2;
	UINT8	vow_watf_q_lv3;

	VOW_WATF vow_watf_mac[VOW_WATF_LEVEL_NUM];

	UINT8	vow_sta_frr_quantum; /* for fast round robin */
	BOOLEAN	vow_sta_frr_flag;
	struct multi_cli_ctl vow_mcli_ctl;
#endif /* CONFIG_AP_SUPPORT */
#endif /* VOW_SUPPORT */
#ifdef RED_SUPPORT
	UINT8  red_en;
	UINT8  red_have_cr4;
	UINT8  red_debug_en;
	RED_STA_T red_sta[MAX_LEN_OF_MAC_TABLE];
	UINT16 red_targetdelay;
	UINT16 red_atm_on_targetdelay;
	UINT16 red_atm_off_targetdelay;
	UINT8 red_sta_num;
	UINT8 red_in_use_sta;
	RALINK_TIMER_STRUCT red_badnode_timer;
#endif /* RED_SUPPORT */
#ifdef FQ_SCH_SUPPORT
	struct fq_ctrl_type fq_ctrl;
#endif /* FQ_SCH_SUPPORT */
	UINT8  cp_support;
	UINT8  cp_have_cr4;
#if defined(MT7615) || defined(MT7622) || defined(MT7663) || defined(MT7626)
	UINT8  ucRxRateRecordEn;
	UINT32 arRateCCK[8];
	UINT32 arRateOFDM[8];
	UINT32 arRateHT[4][10];
	UINT32 arRateVHT[4][10];

	UINT8  ucRxvRecordEn;
	UINT8  ucRxvMode;
	UINT8  ucRxvWcid;
	UINT8  ucRxvCurSN;
	UINT8  ucFirstWrite;
	UINT8  ucRecordStart;
	UINT8  ucRxvBandIdx;
	UINT8  ucRxvG0;
	UINT8  ucRxvG1;
	UINT8  ucRxvG2;
	UINT32 u4RFCRBand0ori;
	UINT32 u4RFCRBand1ori;
	UINT32 u4BN0RFCR0ori;
	UINT32 u4BN1RFCR0ori;
	UINT32 u4RQCRori;
	UINT32 u4RMACAirtime0ori;
	UINT32 u4RxvCurCnt;
	CHAR RxvFilePath[256];
	/* Start pointer of RxVB element */
	PRxVBQElmt prxvbstartelmt;
	/* Current pointer of RxVB element */
	PRxVBQElmt prxvbcurelmt;
#endif /* #if defined(MT7615) || defined(MT7622) || defined(MT7663) || defined(MT7626) */
#ifdef CONFIG_FWOWN_SUPPORT
#ifdef RTMP_MAC_PCI
	UINT8 bDrvOwn;
	UINT8 bCRAccessing;
	UINT8 bSetFWOwnRunning;
	NDIS_SPIN_LOCK DriverOwnLock;
	/* NDIS_SPIN_LOCK PowerLock; */
#endif
#endif /* CONFIG_FWOWN_SUPPORT */

	WIFI_SYS_INFO_T WifiSysInfo;

	struct tx_rx_ctl tr_ctl;

#ifdef ERR_RECOVERY
	ERR_RECOVERY_CTRL_T ErrRecoveryCtl;
#endif
#ifdef TX_AGG_ADJUST_WKR
	BOOLEAN TxAggAdjsut;
#endif /* TX_AGG_ADJUST_WKR */
	NDIS_SPIN_LOCK TimerSemLock;

	RX_STATISTIC_RXV rx_stat_rxv;
	UINT32 parse_rxv_stat_enable;
	UINT32 AccuOneSecRxBand0FcsErrCnt;
	UINT32 AccuOneSecRxBand0MdrdyCnt;
	UINT32 AccuOneSecRxBand1FcsErrCnt;
	UINT32 AccuOneSecRxBand1MdrdyCnt;

#ifdef TRACELOG_TCP_PKT
	UINT32 u4TcpRxAckCnt;
	UINT32 u4TcpTxDataCnt;
#endif

#ifdef AUTOMATION
	UCHAR assignWcid;
	BSS_CHECK_CTRL BssChkCtrl[DBDC_BAND_NUM];
	AUTOMATION_DVT *auto_dvt;
#endif

#ifdef SMART_CARRIER_SENSE_SUPPORT
	SMART_CARRIER_SENSE_CTRL	SCSCtrl;
#endif /* SMART_CARRIER_SENSE_SUPPORT */
	RvR_Debug_CTRL RVRDBGCtrl;

	UINT16						   CCI_ACI_TxOP_Value[DBDC_BAND_NUM];
	UINT16						   MUMIMO_TxOP_Value;
	BOOL							 G_MODE_INFRA_TXOP_RUNNING;
	struct wifi_dev				  *g_mode_txop_wdev;

	MT_MIB_BUCKET_ONE_SEC   OneSecMibBucket;
	MT_MIB_BUCKET_MS		MsMibBucket;
	struct peak_tp_ctl peak_tp_ctl[DBDC_BAND_NUM];
	struct txop_ctl txop_ctl[DBDC_BAND_NUM];
	struct multi_cli_ctl mcli_ctl;
#ifdef PKT_BUDGET_CTRL_SUPPORT
	UINT16 pbc_bound[PBC_AC_NUM];
#endif /*PKT_BUDGET_CTRL_SUPPORT*/

#ifdef HTC_DECRYPT_IOT
	UINT32 HTC_ICV_Err_TH; /* threshold */
#endif /* HTC_DECRYPT_IOT */

#if defined(BB_SOC) && defined(TCSUPPORT_WLAN_SW_RPS)
	UINT32 rxThreshold;
	UINT32 rxPassThresholdCnt;
#endif

#ifdef DHCP_UC_SUPPORT
	BOOLEAN DhcpUcEnable;
#endif /* DHCP_UC_SUPPORT */
	struct wpf_ctrl wpf;
#ifdef LTF_SNR_SUPPORT
	INT16 Avg_LTFSNR;
	INT16 Avg_LTFSNRx16;
#endif
	/* For QAtool log buffer limitation. */
	UINT16  u2LogEntryIdx;
	UINT8	fgDumpStart;
	UINT8	fgQAtoolBatchDumpSupport;
#ifdef MULTI_PROFILE
	VOID *multi_pf_ctrl;
#endif /*MULTI_PROFILE*/
#ifdef A4_CONN
	UINT32 a4_interface_count;
	UINT32 a4_need_refresh;
#endif
	UINT8 nearfar_far_client_num; /* far client number in near/far condition */
#ifdef DBG_STARVATION
	struct starv_log starv_log_ctrl;
#endif /*DBG_STARVATION*/
#ifdef MBO_SUPPORT
	UINT8 MboBssTermCountDown;
	UINT8 Mbo_Bss_Terminate_Index;
#endif /* MBO_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
	struct wifi_dev *bh_bss_wdev[DBDC_BAND_NUM];
	UCHAR MAPMode;
	UCHAR bMAPQuickChChangeEn;
	UCHAR bMAPAvoidScanDuringCac;
#endif /* CONFIG_MAP_SUPPORT */
#ifdef WAPP_SUPPORT
	struct BSS_LOAD_INFO bss_load_info;
#endif /* WAPP_SUPPORT */

	UCHAR reg_domain;

#ifdef FW_LOG_DUMP
	FW_LOG_CTRL fw_log_ctrl;
#endif /* FW_LOG_DUMP */
#ifdef DYNAMIC_STEERING_LOADING
	UINT32 con_tx_tp_running;
	UINT32 con_rx_tp_running;

	UINT32 tp_2g_tx;
	UINT32 tp_2g_rx;
	UINT32 tp_5g_tx;
	UINT32 tp_5g_rx;
	UINT32 disable_auto_txop;
	UINT32 disable_auto_rps;
	UINT32 disable_auto_ratio;
	UINT32 disable_amsdu_read;
	UINT32 delta_tx_cnt;
	UINT32 delta_rx_cnt;
	UINT32 delta_tx_cnt_avg;
	UINT32 delta_rx_cnt_avg;
	UINT32 tr_ratio;
	UINT32 rt_ratio;
	UINT32 tr_ratio_avg;
	UINT32 rt_ratio_avg;
	UINT32 pre_tr_ratio_avg;
	UINT32 pre_rt_ratio_avg;

	UINT32 First_combo_TxRpsRatio;
	UINT32 First_combo_TxRpsRatiobaseF;
	UINT32 First_combo_cnt_core0;
	UINT32 First_combo_cnt_core1;
	UINT32 First_combo_cnt_core0_old;
	UINT32 First_combo_cnt_core1_old;
	UINT32 First_combo_tx_cpu;

	UINT32 Second_combo_cnt_core0;
	UINT32 Second_combo_cnt_core1;

	UINT32 First_combo_tx_tasklet_schedule;
	UINT32 Second_combo_tx_tasklet_schedule;
	UINT32 First_combo_rx_cnt_core0;
	UINT32 First_combo_rx_cnt_core1;
	UINT32 First_combo_rx_cnt_core0_old;
	UINT32 First_combo_rx_cnt_core1_old;
	UINT32 First_combo_buf_underrun;

	UINT32 First_combo_buf_max_processed_cnt;
	UINT32 Second_combo_buf_max_processed_cnt;

	UINT32 First_combo_buf_max_cnt;
	UINT32 Second_combo_buf_max_cnt;

	UINT32 tcp_small_packet_combo_buf_max_cnt;
	UINT32 one_sec_rx_max_pkt_size;
	UINT32 one_sec_tx_max_pkt_size;

	UINT32 rx_intr_cpu;
	UINT32 total_tx_process_cnt_for_specific_cpu;
	UINT32 tasklet_schdule_lo_flag;
	UINT32 concurrent_tput_flag;
	UINT32 single_5g_tput_flag;
	UINT32 single_2g_tput_flag;
	UINT32 single_5g_one_pair_tput_flag;
	UINT32 single_2g_one_pair_tput_flag;
	UINT32 bidi_tput_flag;
	UINT32 tx_tput_flag;
	UINT32 rx_tput_flag;
	UINT32 rps_apply_idx;
	ULONG rps_update_tick;
#endif /* DYNAMIC_STEERING_LOADING */

	UINT32 truncate_sum_tx_rx_time[DBDC_BAND_NUM];
	UINT32 truncate_enable;
	UINT32 truncate_force_disable;
	UINT32 air_time_tx_ratio[DBDC_BAND_NUM];
	UINT32 air_time_rx_ratio[DBDC_BAND_NUM];

#ifdef MT7626
	/* count the error Fixed CCK Rate on 5G cases */
	UINT32 FixRateErr1;
	UINT32 FixRateErr2;
	UINT32 FixRateErr3;
	UINT32 FixRateErr4;
	UINT32 FixRateErr5;
	UINT32 FixRateErr6;
#endif
#ifdef WIFI_DIAG
	void *pDiagCtrl;
#endif
#ifdef WAPP_SUPPORT
	struct scan_req last_scan_req;
#endif
#ifdef APCLI_SUPPORT
	BOOLEAN bApCliCertTest;
#endif
	/* CCSA performance enhancement */
	BOOLEAN ccsa_overlapping;
	BOOLEAN ccsa_more_than_1bss;
	UCHAR ccsa_last_bssid[MAC_ADDR_LEN];
	UCHAR ccsa_bw80_cnt;
	ULONG ccsa_last_bssid_time;
#ifdef MT7626_E2_SUPPORT
	unsigned int dev_irq_md;
#endif
#ifdef RATE_PRIOR_SUPPORT
	LOWRATE_CTRL LowRateCtrl;
#endif/*RATE_PRIOR_SUPPORT*/
#ifdef IXIA_SUPPORT
	int NbdsDctCnt;
	int NbdsDelCnt;
	UCHAR chkTmr;
	UCHAR DumpTimeCtl;
	UINT16 pktthld;
	UINT8 DeltaRssiTh;
	IXIA_CTL ixiaCtrl;
	CHAR MinRssiTh;
	UINT8 masktxop;
#ifdef REDUCE_TX_OVERHEAD
	UINT cached_inCnt;
	UINT cached_outCnt;
#endif
#endif /*IXIA_SUPPORT*/
#ifdef ANTENNA_CONTROL_SUPPORT
	UINT8 TxStream[DBDC_BAND_NUM];
	UINT8 RxStream[DBDC_BAND_NUM];
	UINT8 bAntennaSetAPEnable[DBDC_BAND_NUM];
#endif
};

typedef struct _PEER_PROBE_REQ_PARAM {
#ifdef OCE_SUPPORT
	UCHAR Addr1[MAC_ADDR_LEN];
#endif /* OCE_SUPPORT */

	UCHAR Addr2[MAC_ADDR_LEN];
	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR SsidLen;
	BOOLEAN bRequestRssi;
	BOOLEAN IsFromIos; /* For IOS immediately connect */
#ifdef CONFIG_HOTSPOT
	BOOLEAN IsIWIE;
	BOOLEAN IsIWCapability;
	UCHAR Hessid[MAC_ADDR_LEN];
	BOOLEAN IsHessid;
	UINT8 AccessNetWorkType;
#endif /* CONFIG_HOTSPOT */
#ifdef BAND_STEERING
	BOOLEAN IsHtSupport;
	BOOLEAN IsVhtSupport;
	UINT32 RxMCSBitmask;
/* WPS_BandSteering Support */
	BOOLEAN bWpsCapable;
#endif
#ifdef WH_EVENT_NOTIFIER
	IE_LISTS ie_list;
#endif /* WH_EVENT_NOTIFIER */
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	struct probe_req_report report_param;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
#ifdef OCE_SUPPORT
	BOOLEAN bProbeSupp[WDEV_NUM_MAX];
	BOOLEAN IsOceCapability;
	UINT32 MaxChannelTime;
#endif
} PEER_PROBE_REQ_PARAM, *PPEER_PROBE_REQ_PARAM;

/***************************************************************************
  *	Rx Path software control block related data structures
  **************************************************************************/
typedef enum RX_BLK_FLAGS {
	fRX_AMPDU = (1 << 0),
	fRX_AMSDU = (1 << 1),
	fRX_ARALINK = (1 << 2),
	fRX_HTC = (1 << 3),
	fRX_PAD = (1 << 4),
	fRX_QOS = (1 << 5),
	fRX_EAP = (1 << 6),
	fRX_WPI = (1 << 7),
	fRX_AP = (1 << 8),			/* Packet received from AP */
	fRX_STA = (1 << 9),			/* Packet received from Client(Infra mode) */
	fRX_ADHOC = (1 << 10),		/* packet received from AdHoc peer */
	fRX_WDS = (1 << 11),		/* Packet received from WDS */
	fRX_MESH = (1 << 12),		/* Packet received from MESH */
	fRX_DLS = (1 << 13),		/* Packet received from DLS peer */
	fRX_TDLS = (1 << 14),		/* Packet received from TDLS peer */
	fRX_RETRIEVE = (1 << 15),	/* Packet received from mcu */
	fRX_CMD_RSP = (1 << 16),	/* Pakket received from mcu command response */
	fRX_TXRX_RXV = (1 << 17),   /* RxV received from Rx Ring1 */
	fRX_HDR_TRANS = (1 << 18),
	fRX_WCID_MISMATCH = (1 << 19), /* for HW Lookup Wcid Mismatch */
} RX_BLK_FLAGS;

#define QOS_USER_PRIORITY_MASK 0x000f
enum Qos_User_Priority {
	AC_BE_be = 0,
	AC_BK_bk,
	AC_BK_,
	AC_BE_ee,
	AC_VI_cl,
	AC_VI_vi,
	AC_VO_vo,
	AC_VO_nc,
};

typedef struct _RX_BLK {
	UCHAR hw_rx_info[RXD_SIZE]; /* include "RXD_STRUC RxD" and "RXINFO_STRUC rx_info " */
	RXINFO_STRUC *pRxInfo; /* for RLT, in head of frame buffer, for RTMP, in hw_rx_info[RXINFO_OFFSET] */
	/* TODO: shiang-usw, revise this! */
	RXWI_STRUC *pRxWI; /*in frame buffer and after "rx_info" fields */
	UCHAR *rmac_info;
	UCHAR *FC;
	UINT16 Duration;
	UCHAR FN;
	UINT16 SN;
	UCHAR *Addr1;
	UCHAR *Addr2;
	UCHAR *Addr3;
	UCHAR *Addr4;
	PNDIS_PACKET pRxPacket; /* os_packet pointer, shall not change */
	UCHAR *pData; /* init to pRxPacket->data, refer to frame buffer, may changed depends on processing */
	USHORT DataSize; /* init to  RXWI->MPDUtotalByteCnt, and may changes depends on processing */
	RX_BLK_FLAGS Flags;

	/* Mirror info of partial fields of RxWI and RxInfo */
	USHORT MPDUtotalByteCnt; /* Refer to RXWI->MPDUtotalByteCnt */
	UCHAR UserPriority;	/* for calculate TKIP MIC using */
	UCHAR OpMode;	/* 0:OPMODE_STA 1:OPMODE_AP */
	UCHAR wcid;		/* copy of pRxWI->wcid */
	UCHAR U2M;
	UCHAR key_idx;
	UCHAR bss_idx;
	UCHAR TID;
	UINT32 TimeStamp;
	struct rx_signal_info rx_signal;
	CHAR ldpc_ex_sym;
	HTTRANSMIT_SETTING rx_rate;
	UINT32 rxv2_cyc1;
	UINT32 rxv2_cyc2;
	UINT32 rxv2_cyc3;
#ifdef HDR_TRANS_SUPPORT
	BOOLEAN bHdrVlanTaged;	/* VLAN tag is added to this header */
	UCHAR *pTransData;
	USHORT TransDataSize;
#endif /* HDR_TRANS_SUPPORT */
#ifdef CUT_THROUGH
	UINT16 token_id;
#endif /* CUT_THROUGH */
	BOOLEAN CipherMis;
	UINT8 AmsduState;
	BOOLEAN DeAmsduFail;
	UINT64 CCMP_PN;
#ifdef LTF_SNR_SUPPORT
	INT16 Ofdm_SNR;
#endif
#ifdef IXIA_SUPPORT
	UCHAR isNBDSPacket;
#endif /*IXIA_SUPPORT*/
} RX_BLK ____cacheline_aligned;

#define RX_BLK_SET_FLAG(_pRxBlk, _flag)		(_pRxBlk->Flags |= _flag)
#define RX_BLK_TEST_FLAG(_pRxBlk, _flag)		(_pRxBlk->Flags & _flag)
#define RX_BLK_CLEAR_FLAG(_pRxBlk, _flag)	(_pRxBlk->Flags &= ~(_flag))

#define AMSDU_SUBHEAD_LEN	14
#define ARALINK_SUBHEAD_LEN	14
#define ARALINK_HEADER_LEN	 2

typedef enum TX_FRAME_TYPE {
	TX_UNKOWN_FRAME,
	TX_MCAST_FRAME = (1 << 0),
	TX_LEGACY_FRAME = (1 << 1),
	TX_AMPDU_FRAME = (1 << 2),
	TX_AMSDU_FRAME = (1 << 3),
	TX_FRAG_FRAME = (1 << 4),
	TX_MLME_MGMTQ_FRAME = (1 << 5),
	TX_MLME_DATAQ_FRAME = (1 << 6),
	TX_ATE_FRAME = (1 << 7),
} TX_FRAME_TYPE;

#define TX_FRAG_ID_NO			0x0
#define TX_FRAG_ID_FIRST		0x1
#define TX_FRAG_ID_MIDDLE		0x2
#define TX_FRAG_ID_LAST			0x3

#define TX_AMSDU_ID_NO 0x0
#define TX_AMSDU_ID_FIRST 0x1
#define TX_AMSDU_ID_MIDDLE 0x2
#define TX_AMSDU_ID_LAST 0x3

typedef enum TX_BLK_FLAGS {
	fTX_bRtsRequired = (1 << 0),
	fTX_bAckRequired = (1 << 1),
	fTX_bPiggyBack = (1 << 2),
	fTX_bHTRate = (1 << 3),
	fTX_bWMM = (1 << 4),
	fTX_bAllowFrag = (1 << 5),
	fTX_bMoreData = (1 << 6),
	fTX_bClearEAPFrame = (1 << 8),
	fTX_bApCliPacket = (1 << 9),
	fTX_bSwEncrypt = (1 << 10),
	fTX_bWMM_UAPSD_EOSP = (1 << 11),
	fTX_bWDSEntry = (1 << 12),
	fTX_bDonglePkt = (1 << 13),
	fTX_bMeshEntry = (1 << 14),
	fTX_bWPIDataFrame = (1 << 15),
	fTX_bClientWDSFrame = (1 << 16),
	fTX_bTdlsEntry = (1 << 17),
	fTX_AmsduInAmpdu = (1 << 18),
	fTX_ForceRate = (1 << 19),
	fTX_CT_WithTxD = (1 << 20),
	fTX_CT_WithoutTxD = (1 << 21),
	fTX_DumpPkt = (1 << 22),
	fTX_HDR_TRANS = (1 << 23),
	fTX_MCU_OFFLOAD = (1 << 24),
	fTX_MCAST_CLONE = (1 << 25),
	fTX_HIGH_PRIO = (1 << 26),
#ifdef A4_CONN
	fTX_bA4Frame = (1 << 27),
#endif
	fTX_HW_AMSDU = (1 << 28),
} TX_BLK_FLAGS;

typedef struct _TX_BLK {
	UCHAR resource_idx ____cacheline_aligned;
	UCHAR QueIdx;
	UCHAR WmmIdx;
	UCHAR TotalFrameNum;				/* Total frame number want to send-out in one batch */
	UCHAR TotalFragNum;				/* Total frame fragments required in one batch */
#ifdef DSCP_PRI_SUPPORT
	INT8				DscpMappedPri;
#endif
	UCHAR TxFrameType;				/* Indicate the Transmission type of the all frames in one batch */
	USHORT TotalFrameLen;				/* Total length of all frames want to send-out in one batch */
	STA_TR_ENTRY *tr_entry;
	PNDIS_PACKET pPacket;
	struct wifi_dev *wdev;
	MAC_TABLE_ENTRY	*pMacEntry;			/* NULL: packet with 802.11 RA field is multicast/broadcast address */
	TX_BLK_FLAGS Flags;
	UCHAR UserPriority;				/* priority class of packet */
	UCHAR Wcid;						/* The MAC entry associated to this packet */
	UINT8 wmm_set;
	UINT8 dbdc_band;
	UCHAR *pSrcBufHeader;				/* Reference to the head of sk_buff->data */
	UINT SrcBufLen;					/* Length of packet payload which not including Layer 2 header */
	UCHAR wifi_hdr_len; /* 802.11 header */
	UCHAR frame_idx;
	UCHAR amsdu_state;
	UCHAR FragIdx;					/* refer to TX_FRAG_ID_xxxx */
	UCHAR *pSrcBufData;				/* Reference to the sk_buff->data, will changed depends on hanlding progresss */
	UCHAR HdrPadLen; /* padding length*/
	UCHAR MpduHeaderLen; /* 802.11 header + LLC/SNAP not including padding */
	INT16 tx_bytes_len; /* txd + packet length not including txp */
	UCHAR txp_len;
	UCHAR hw_rsv_len;
	UCHAR first_buf_len;
	ra_dma_addr_t SrcBufPA;
	QUEUE_HEADER TxPacketList;
	HTTRANSMIT_SETTING *pTransmit;
	UCHAR *pExtraLlcSnapEncap;			/* NULL means no extra LLC/SNAP is required */
	UCHAR *HeaderBuf;
	UCHAR *wifi_hdr;
	UCHAR FrameGap;					/* what kind of IFS this packet use */
	UCHAR MpduReqNum;					/* number of fragments of this frame */
	UCHAR TxRate;						/* TODO: Obsoleted? Should change to MCS? */
	UINT32 CipherAlg;						/* cipher alogrithm */
	PCHAR pKey;
	UCHAR KeyIdx;						/* Indicate the transmit key index */
	UCHAR OpMode;
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	UINT ApCliIfidx;
	PSTA_ADMIN_CONFIG		pApCliEntry;
#endif /* APCLI_SUPPORT */
	BSS_STRUCT *pMbss;
#endif /* CONFIG_AP_SUPPORT */
	ULONG Priv;						/* Hardware specific value saved in here. */


#ifdef TXBF_SUPPORT
	UCHAR TxSndgPkt; /* 1: sounding 2: NDP sounding */
	UCHAR TxNDPSndgBW;
	UCHAR TxNDPSndgMcs;
#endif /* TXBF_SUPPORT */
	UINT8 dot11_type;
	UINT8 dot11_subtype;
	UINT8 TxSFormat;
	UINT8 Pid;
	UINT8 TxS2Host;
	UINT8 TxS2Mcu;
	UINT32 TxSPriv;
#ifdef RANDOM_PKT_GEN
	UINT8 lmac_qidx;
#endif
	UINT32 HeaderBuffer[32] ____cacheline_aligned;
} TX_BLK;

#define TX_BLK_SET_FLAG(_pTxBlk, _flag)		(_pTxBlk->Flags |= _flag)
#define TX_BLK_TEST_FLAG(_pTxBlk, _flag)	(((_pTxBlk->Flags & _flag) == _flag) ? 1 : 0)
#define TX_BLK_CLEAR_FLAG(_pTxBlk, _flag)	(_pTxBlk->Flags &= ~(_flag))

#ifdef DBG_DEQUE
struct deq_log_struct {
	UCHAR que_depth[WMM_NUM_OF_AC];
	UCHAR deq_cnt[WMM_NUM_OF_AC];
	UCHAR deq_round;
};
#endif /* DBG_DEQUE */

typedef struct dequeue_info {
	BOOLEAN inited;
	UCHAR start_q;
	UCHAR end_q;
	CHAR cur_q;
	UCHAR target_wcid;
	UCHAR target_que;
	UCHAR cur_wcid;
	USHORT q_max_cnt[WMM_QUE_NUM];
	INT pkt_bytes;
	INT pkt_cnt;
	INT deq_pkt_bytes;
	INT deq_pkt_cnt;
	INT status;
	BOOLEAN full_qid[WMM_QUE_NUM];
#ifdef DBG_DEQUE
	deq_log_struct deq_log;
#endif /* DBG_DEQUE */
} DEQUE_INFO;

#ifdef TRACELOG_TCP_PKT
#define TCP_TRAFFIC_DATAPKT_MIN_SIZE	1000
static inline BOOLEAN RTMPIsTcpDataPkt(
	IN PNDIS_PACKET pPacket)
{
	UINT32 pktlen;

	pktlen = GET_OS_PKT_LEN(pPacket);

	if (pktlen > TCP_TRAFFIC_DATAPKT_MIN_SIZE)
		return TRUE;
	else
		return FALSE;
}
#define TCP_TRAFFIC_ACKPKT_SIZE		 54
static inline BOOLEAN RTMPIsTcpAckPkt(
	IN PNDIS_PACKET pPacket)
{
	UINT32 pktlen;

	pktlen = GET_OS_PKT_LEN(pPacket);

	if (pktlen == TCP_TRAFFIC_ACKPKT_SIZE)
		return TRUE;
	else
		return FALSE;
}
#endif /* TRACELOG_TCP_PKT */

#ifdef RT_BIG_ENDIAN
#include "hdev_ctrl.h"

/***************************************************************************
  *	Endian conversion related functions
  **************************************************************************/

#ifdef MT_MAC
static inline VOID mt_rmac_d0_endian_change(UINT32 *rxinfo)
{
	(*rxinfo) = SWAP32(*rxinfo);
}

static inline VOID mt_rmac_base_info_endian_change(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *rxinfo)
{
	int i;

	for (i = 1; i < 4; i++)
		*(((UINT32 *)rxinfo) + i) = SWAP32(*(((UINT32 *)rxinfo) + i));
}
#endif /* MT_MAC */

/*
	========================================================================

	Routine Description:
		Endian conversion of Tx/Rx descriptor .

	Arguments:
		pAd	Pointer to our adapter
		pData			Pointer to Tx/Rx descriptor
		DescriptorType	Direction of the frame

	Return Value:
		None

	Note:
		Call this function when read or update descriptor
	========================================================================
*/
static inline VOID RTMPWIEndianChange(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pData,
	IN ULONG DescriptorType)
{
	int size;
	int i;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 TXWISize = cap->TXWISize;
	UINT8 RXWISize = cap->RXWISize;

	size = ((DescriptorType == TYPE_TXWI) ? TXWISize : RXWISize);

	if (DescriptorType == TYPE_TXWI) {
		*((UINT32 *)(pData)) = SWAP32(*((UINT32 *)(pData)));		/* Byte 0~3 */
		*((UINT32 *)(pData + 4)) = SWAP32(*((UINT32 *)(pData + 4)));	/* Byte 4~7 */

		if (size > 16)
			*((UINT32 *)(pData + 16)) = SWAP32(*((UINT32 *)(pData + 16)));	/* Byte 16~19 */
	} else {
		for (i = 0; i < size / 4; i++)
			*(((UINT32 *)pData) + i) = SWAP32(*(((UINT32 *)pData) + i));
	}
}

#ifdef MT_MAC
/*
	========================================================================

	Routine Description:
		Endian conversion of MacTxInfo/MacRxInfo descriptor .

	Arguments:
		pAd	Pointer to our adapter
		pData			Pointer to Tx/Rx descriptor
		DescriptorType	Direction of the frame
		Length		Length of MacTxInfo/MacRxInfo

	Return Value:
		None

	Note:
		Call this function when read or update descriptor
	========================================================================
*/
static inline VOID MTMacInfoEndianChange(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pData,
	IN ULONG DescriptorType,
	IN UINT16 Length)
{
	int i;

	if (DescriptorType == TYPE_TMACINFO) {
		for (i = 0; i < Length / 4; i++)
			*(((UINT32 *)pData) + i) = SWAP32(*(((UINT32 *)pData) + i));
	} else { /* TYPE_RMACINFO */
		for (i = 1; i < Length / 4; i++) /* i from 1, due to 1st DW had endia change already, so skip it here. */
			*(((UINT32 *)pData) + i) = SWAP32(*(((UINT32 *)pData) + i));
	}
}
#endif /* MT_MAC */

#ifdef RTMP_MAC_PCI
static inline VOID WriteBackToDescriptor(
	IN UCHAR *Dest,
	IN UCHAR *Src,
	IN BOOLEAN DoEncrypt,
	IN ULONG DescriptorType)
{
	UINT32 *p1, *p2;

	p1 = ((UINT32 *)Dest);
	p2 = ((UINT32 *)Src);
	*p1 = *p2;
	*(p1 + 2) = *(p2 + 2);
	/*new txd & rxd just have 3 word*/
	*(p1 + 3) = *(p2 + 3);
	/* +++Add by shiang for jeffrey debug */
#if defined(LINUX) || defined(VXWORKS)
	wmb();
#endif /* LINUX */
	/* ---Add by shiang for jeffrey debug */
	*(p1 + 1) = *(p2 + 1); /* Word 1; this must be written back last */
}
#endif /* RTMP_MAC_PCI */

/*
	========================================================================

	Routine Description:
		Endian conversion of Tx/Rx descriptor .

	Arguments:
		pAd	Pointer to our adapter
		pData			Pointer to Tx/Rx descriptor
		DescriptorType	Direction of the frame

	Return Value:
		None

	Note:
		Call this function when read or update descriptor
	========================================================================
*/
#ifdef RTMP_MAC_PCI
static inline VOID RTMPDescriptorEndianChange(UCHAR *pData, ULONG DescType)
{
	*((UINT32 *)(pData)) = SWAP32(*((UINT32 *)(pData)));			/* Byte 0~3 */
	*((UINT32 *)(pData + 8)) = SWAP32(*((UINT32 *)(pData + 8)));		/* Byte 8~11 */
	/*new txd & rxd just have 3 word*/
	*((UINT32 *)(pData + 12)) = SWAP32(*((UINT32 *)(pData + 12)));	/* Byte 12~15 */
	*((UINT32 *)(pData + 4)) = SWAP32(*((UINT32 *)(pData + 4)));		/* Byte 4~7, this must be swapped last */
}
#endif /* RTMP_MAC_PCI */

/*
	========================================================================

	Routine Description:
		Endian conversion of all kinds of 802.11 frames .

	Arguments:
		pAd	Pointer to our adapter
		pData			Pointer to the 802.11 frame structure
		Dir			Direction of the frame
		FromRxDoneInt	Caller is from RxDone interrupt

	Return Value:
		None

	Note:
		Call this function when read or update buffer data
	========================================================================
*/
static inline VOID	RTMPFrameEndianChange(
	IN	RTMP_ADAPTER *pAd,
	IN	PUCHAR			pData,
	IN	ULONG			Dir,
	IN	BOOLEAN		FromRxDoneInt)
{
	PHEADER_802_11 pFrame;
	PUCHAR	pMacHdr;

	/* swab 16 bit fields - Frame Control field */
	if (Dir == DIR_READ)
		*(USHORT *)pData = SWAP16(*(USHORT *)pData);

	pFrame = (PHEADER_802_11) pData;
	pMacHdr = (PUCHAR) pFrame;
	/* swab 16 bit fields - Duration/ID field */
	*(USHORT *)(pMacHdr + 2) = SWAP16(*(USHORT *)(pMacHdr + 2));

	if (pFrame->FC.Type != FC_TYPE_CNTL) {
		/* swab 16 bit fields - Sequence Control field */
		*(USHORT *)(pMacHdr + 22) = SWAP16(*(USHORT *)(pMacHdr + 22));
	}

	if (pFrame->FC.Type == FC_TYPE_MGMT) {
		switch (pFrame->FC.SubType) {
		case SUBTYPE_ASSOC_REQ:
		case SUBTYPE_REASSOC_REQ:
			/* swab 16 bit fields - CapabilityInfo field */
			pMacHdr += sizeof(HEADER_802_11);
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			/* swab 16 bit fields - Listen Interval field */
			pMacHdr += 2;
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			break;

		case SUBTYPE_ASSOC_RSP:
		case SUBTYPE_REASSOC_RSP:
			/* swab 16 bit fields - CapabilityInfo field */
			pMacHdr += sizeof(HEADER_802_11);
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			/* swab 16 bit fields - Status Code field */
			pMacHdr += 2;
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			/* swab 16 bit fields - AID field */
			pMacHdr += 2;
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			break;

		case SUBTYPE_AUTH:

			/* When the WEP bit is on, don't do the conversion here.
			This is only shared WEP can hit this condition.
			For AP, it shall do conversion after decryption.
			For STA, it shall do conversion before encryption. */
			if (pFrame->FC.Wep == 1)
				break;
			else {
				/* swab 16 bit fields - Auth Alg No. field */
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				/* swab 16 bit fields - Auth Seq No. field */
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				/* swab 16 bit fields - Status Code field */
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			}

			break;

		case SUBTYPE_BEACON:
		case SUBTYPE_PROBE_RSP:
			/* swab 16 bit fields - BeaconInterval field */
			pMacHdr += (sizeof(HEADER_802_11) + TIMESTAMP_LEN);
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			/* swab 16 bit fields - CapabilityInfo field */
			pMacHdr += sizeof(USHORT);
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			break;

		case SUBTYPE_DEAUTH:
		case SUBTYPE_DISASSOC:

			/* If the PMF is negotiated, those frames shall be encrypted */
			if (!FromRxDoneInt && pFrame->FC.Wep == 1)
				break;
			else {
				/* swab 16 bit fields - Reason code field */
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			}

			break;
		}

	} else if (pFrame->FC.Type == FC_TYPE_DATA) {
	} else if (pFrame->FC.Type == FC_TYPE_CNTL) {
		switch (pFrame->FC.SubType) {
		case SUBTYPE_BLOCK_ACK_REQ: {
			PFRAME_BA_REQ pBAReq = (PFRAME_BA_REQ)pFrame;
			*(USHORT *)(&pBAReq->BARControl) = SWAP16(*(USHORT *)(&pBAReq->BARControl));
			pBAReq->BAStartingSeq.word = SWAP16(pBAReq->BAStartingSeq.word);
		}
		break;

		case SUBTYPE_BLOCK_ACK:
			/* For Block Ack packet, the HT_CONTROL field is in the same offset with Addr3 */
			*(UINT32 *)(&pFrame->Addr3[0]) = SWAP32(*(UINT32 *)(&pFrame->Addr3[0]));
			break;

		case SUBTYPE_ACK:
			/*For ACK packet, the HT_CONTROL field is in the same offset with Addr2 */
			*(UINT32 *)(&pFrame->Addr2[0]) =	SWAP32(*(UINT32 *)(&pFrame->Addr2[0]));
			break;
		}
	} else
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid Frame Type!!!\n"));

	/* swab 16 bit fields - Frame Control */
	if (Dir == DIR_WRITE)
		*(USHORT *)pData = SWAP16(*(USHORT *)pData);
}
/*
 ========================================================================

 Routine Description:
	 Endian conversion of normal data ,data type should be int or uint.

 Arguments:
	 pAd	 Pointer to our adapter
	 pData	 Pointer to data
	 size		 length of data

 Return Value:
	 None

========================================================================
*/

static inline VOID RTMPEndianChange(
	IN UCHAR *pData,
	IN UINT size)
{
	int i;

	if (size % 4)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid DATA Type!!!\n"));
	else {
		for (i = 0; i < size / 4; i++)
			*(((UINT32 *)pData) + i) = SWAP32(*(((UINT32 *)pData) + i));
	}
}
#endif /* RT_BIG_ENDIAN */

/***************************************************************************
  *	Other static inline function definitions
  **************************************************************************/
static inline VOID ConvertMulticastIP2MAC(
	IN PUCHAR pIpAddr,
	IN PUCHAR *ppMacAddr,
	IN UINT16 ProtoType)
{
	if (pIpAddr == NULL)
		return;

	if (ppMacAddr == NULL || *ppMacAddr == NULL)
		return;

	switch (ProtoType) {
	case ETH_P_IPV6:
		/*			memset(*ppMacAddr, 0, MAC_ADDR_LEN); */
		*(*ppMacAddr) = 0x33;
		*(*ppMacAddr + 1) = 0x33;
		*(*ppMacAddr + 2) = pIpAddr[12];
		*(*ppMacAddr + 3) = pIpAddr[13];
		*(*ppMacAddr + 4) = pIpAddr[14];
		*(*ppMacAddr + 5) = pIpAddr[15];
		break;

	case ETH_P_IP:
	default:
		/*			memset(*ppMacAddr, 0, MAC_ADDR_LEN); */
		*(*ppMacAddr) = 0x01;
		*(*ppMacAddr + 1) = 0x00;
		*(*ppMacAddr + 2) = 0x5e;
		*(*ppMacAddr + 3) = pIpAddr[1] & 0x7f;
		*(*ppMacAddr + 4) = pIpAddr[2];
		*(*ppMacAddr + 5) = pIpAddr[3];
		break;
	}

	return;
}

char *get_phymode_str(int phy_mode);

NDIS_STATUS alloc_chip_cap_dep_data(RTMP_ADAPTER *pAd);
VOID free_chip_cap_dep_data(RTMP_ADAPTER *pAd);
NDIS_STATUS alloc_chip_cap_dep_data_cmm(RTMP_ADAPTER *pAd);
VOID free_chip_cap_dep_data_cmm(RTMP_ADAPTER *pAd);
NDIS_STATUS alloc_chip_cap_dep_data_pci(void *hdev_ctrl);
VOID free_chip_cap_dep_data_pci(void *hdev_ctrl);
NDIS_STATUS alloc_chip_cap_dep_data_usb(RTMP_ADAPTER *pAd);
VOID free_chip_cap_dep_data_usb(RTMP_ADAPTER *pAd);

NDIS_STATUS RTMPAllocTxRxRingMemory(RTMP_ADAPTER *pAd);

#ifdef RESOURCE_PRE_ALLOC
NDIS_STATUS RTMPInitTxRxRingMemory(RTMP_ADAPTER *pAd);
#endif /* RESOURCE_PRE_ALLOC */

struct dev_type_name_map_t {
	INT type;
	RTMP_STRING prefix[IFNAMSIZ];
};

void rtmp_eeprom_of_platform(RTMP_ADAPTER *pAd);
NDIS_STATUS load_dev_l1profile(RTMP_ADAPTER *pAd);
UCHAR *get_dev_l2profile(RTMP_ADAPTER *pAd);
INT get_dev_config_idx(RTMP_ADAPTER *pAd);
UCHAR *get_dev_name_prefix(RTMP_ADAPTER *pAd, INT dev_type);
UCHAR *get_single_sku_path(RTMP_ADAPTER *pAd);
UCHAR *get_bf_sku_path(RTMP_ADAPTER *pAd);
int ShowL1profile(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
NDIS_STATUS	RTMPReadParametersHook(RTMP_ADAPTER *pAd);
NDIS_STATUS	RTMPSetProfileParameters(RTMP_ADAPTER *pAd, RTMP_STRING *pBuffer);
NDIS_STATUS	RTMPPreReadProfile(RTMP_ADAPTER *pAd);
NDIS_STATUS	RTMPSetPreProfileParameters(RTMP_ADAPTER *pAd, RTMP_STRING *pBuffer);

INT RTMPGetKeyParameter(
	IN RTMP_STRING *key,
	OUT RTMP_STRING *dest,
	IN INT destsize,
	IN RTMP_STRING *buffer,
	IN BOOLEAN bTrimSpace);

INT RTMPSetKeyParameter(
    IN RTMP_STRING *key,
    OUT CHAR *value,
    IN INT destsize,
    IN RTMP_STRING *buffer,
    IN BOOLEAN bTrimSpace);

INT RTMPAddKeyParameter(
    IN RTMP_STRING *key,
    OUT CHAR *value,
    IN INT destsize,
    IN RTMP_STRING *buffer);

INT RTMPGetKeyParameterWithOffset(
	IN  RTMP_STRING *key,
	OUT RTMP_STRING *dest,
	OUT	USHORT	*end_offset,
	IN  INT	 destsize,
	IN  RTMP_STRING *buffer,
	IN	BOOLEAN	bTrimSpace);

#ifdef MIN_PHY_RATE_SUPPORT
VOID RTMPMinPhyDataRateCfg(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *Buffer);

VOID RTMPMinPhyBeaconRateCfg(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *Buffer);

VOID RTMPMinPhyMgmtRateCfg(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *Buffer);

VOID RTMPMinPhyBcMcRateCfg(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *Buffer);

VOID RTMPLimitClientSupportRateCfg(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *Buffer);

VOID RTMPDisableCCKRateCfg(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *Buffer);
#endif /* MIN_PHY_RATE_SUPPORT */

#ifdef WSC_INCLUDED
VOID rtmp_read_wsc_user_parms_from_file(
	IN	RTMP_ADAPTER *pAd,
	IN	char *tmpbuf,
	IN	char *buffer);
#endif/*WSC_INCLUDED*/

INT rtmp_band_index_get_by_order(struct _RTMP_ADAPTER *pAd, UCHAR order);

#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
void rtmp_read_vow_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer);
#endif /* CONFIG_AP_SUPPORT */
#endif /* VOW_SUPPORT */

#ifdef GN_MIXMODE_SUPPORT
VOID gn_mixmode_is_enable(
	IN	PRTMP_ADAPTER pAd);
#endif /* GN_MIXMODE_SUPPORT */

#ifdef RED_SUPPORT
void rtmp_read_red_parms_from_file(
	IN	PRTMP_ADAPTER pAd,
	char *tmpbuf,
	char *buffer);
#endif /* RED_SUPPORT */
#ifdef FQ_SCH_SUPPORT
void rtmp_read_fq_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer);
#endif /* FQ_SCH_SUPPORT */

void rtmp_read_cp_parms_from_file(
	IN	PRTMP_ADAPTER pAd,
	char *tmpbuf,
	char *buffer);

VOID cp_support_is_enabled(
	PRTMP_ADAPTER pAd);

INT set_cp_support_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg);

NDIS_STATUS RTMPSetSkuParam(RTMP_ADAPTER *pAd);
NDIS_STATUS RTMPSetBackOffParam(RTMP_ADAPTER *pAd);
NDIS_STATUS RTMPResetSkuParam(RTMP_ADAPTER *pAd);
NDIS_STATUS RTMPResetBackOffParam(RTMP_ADAPTER *pAd);

VOID InitSkuRateDiffTable(RTMP_ADAPTER *pAd);
UCHAR GetSkuChannelBasePwr(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR channel);
VOID UpdateSkuRatePwr(RTMP_ADAPTER *pAd, UCHAR ch, UCHAR bw, CHAR base_pwr);

#ifdef RTMP_RF_RW_SUPPORT
VOID RTMP_ReadRF(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR			RegID,
	OUT	PUCHAR			pValue1,
	OUT PUCHAR			pValue2,
	IN	UCHAR			BitMask);

VOID RTMP_WriteRF(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR			RegID,
	IN	UCHAR			Value,
	IN	UCHAR			BitMask);

NDIS_STATUS	RT30xxWriteRFRegister(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR			regID,
	IN	UCHAR			value);

NDIS_STATUS	RT30xxReadRFRegister(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR			regID,
	IN	PUCHAR			pValue);

NDIS_STATUS	RT635xWriteRFRegister(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR			bank,
	IN	UCHAR			regID,
	IN	UCHAR			value);

NDIS_STATUS	RT635xReadRFRegister(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR			bank,
	IN	UCHAR			regID,
	IN	PUCHAR			pValue);

BOOLEAN RTMPAdjustFrequencyOffset(
	IN RTMP_ADAPTER *pAd,
	INOUT PUCHAR pRefFreqOffset);
#endif /* RTMP_RF_RW_SUPPORT */

BOOLEAN RTMPCheckPhyMode(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 BandSupported,
	INOUT UCHAR *pPhyMode);

#ifdef RTMP_MAC_PCI
INT NICInitPwrPinCfg(RTMP_ADAPTER *pAd);
#endif /* RTMP_MAC_PCI */

VOID NICInitAsicFromEEPROM(RTMP_ADAPTER *pAd);

NDIS_STATUS NICInitializeAdapter(RTMP_ADAPTER *pAd);
NDIS_STATUS NICInitializeAsic(RTMP_ADAPTER *pAd);

VOID NICResetFromError(RTMP_ADAPTER *pAd);

VOID RTMPRingCleanUp(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR		   RingType);

VOID UserCfgExit(RTMP_ADAPTER *pAd);
VOID UserCfgInit(RTMP_ADAPTER *pAd);

VOID NICUpdateFifoStaCounters(RTMP_ADAPTER *pAd);
VOID NICUpdateRawCounters(RTMP_ADAPTER *pAd);

UINT32 AsicGetCrcErrCnt(RTMP_ADAPTER *pAd);
UINT32 AsicGetCCACnt(RTMP_ADAPTER *pAd);
UINT32 AsicGetChBusyCnt(RTMP_ADAPTER *pAd, UCHAR BandIdx);

VOID AsicDMASchedulerInit(RTMP_ADAPTER *pAd, INT mode);
VOID AsicTxCntUpdate(RTMP_ADAPTER *pAd, UCHAR Wcid, MT_TX_COUNTER *pTxInfo);

VOID NicResetRawCounters(RTMP_ADAPTER *pAd);
VOID NicGetTxRawCounters(
	IN RTMP_ADAPTER *pAd,
	IN TX_STA_CNT0_STRUC * pStaTxCnt0,
	IN TX_STA_CNT1_STRUC * pStaTxCnt1);

VOID RTMPZeroMemory(VOID *pSrc, ULONG Length);
ULONG RTMPCompareMemory(VOID *pSrc1, VOID *pSrc2, ULONG Length);
VOID RTMPMoveMemory(VOID *pDest, VOID *pSrc, ULONG Length);

VOID AtoH(RTMP_STRING *src, UCHAR *dest, int destlen);
UCHAR BtoH(char ch);

VOID RTMP_TimerListAdd(RTMP_ADAPTER *pAd, VOID *pRsc);
VOID RTMP_TimerListRelease(RTMP_ADAPTER *pAd, VOID *pRsc);
VOID RTMP_AllTimerListRelease(RTMP_ADAPTER *pAd);

VOID RTMPInitTimer(
	IN RTMP_ADAPTER *pAd,
	IN RALINK_TIMER_STRUCT *pTimer,
	IN VOID *pTimerFunc,
	IN VOID *pData,
	IN BOOLEAN Repeat);

VOID RTMPSetTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value);
VOID RTMPModTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value);
VOID RTMPCancelTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled);
VOID RTMPReleaseTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled);
VOID RTMPShowTimerList(RTMP_ADAPTER *pAd);

VOID RTMPEnableRxTx(RTMP_ADAPTER *pAd);
VOID RTMPDisableRxTx(RTMP_ADAPTER *pAd);

VOID AntCfgInit(RTMP_ADAPTER *pAd);

VOID rtmp_init_hook_set(RTMP_ADAPTER *pAd);

/* */
/* prototype in action.c */
/* */
VOID ActHeaderInit(
	IN RTMP_ADAPTER *pAd,
	IN OUT HEADER_802_11 *pHdr80211,
	IN UCHAR *da,
	IN UCHAR *sa,
	IN UCHAR *bssid);

VOID ActionStateMachineInit(
	IN	RTMP_ADAPTER *pAd,
	IN  STATE_MACHINE *S,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeADDBAAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID MlmeDELBAAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID SendSMPSAction(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	IN UCHAR smps);

#ifdef CONFIG_AP_SUPPORT
VOID SendBeaconRequest(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR				Wcid);
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
VOID RECBATimerTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID ORIBATimerTimeout(
	IN	RTMP_ADAPTER *pAd);

VOID SendRefreshBAR(
	IN	RTMP_ADAPTER *pAd,
	IN	MAC_TABLE_ENTRY	*pEntry);

#ifdef DOT11N_DRAFT3
VOID RTMP_11N_D3_TimerInit(
	IN RTMP_ADAPTER *pAd);

VOID RTMP_11N_D3_TimerRelease(
	IN RTMP_ADAPTER *pAd);

VOID SendBSS2040CoexistMgmtAction(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR	Wcid,
	IN	UCHAR	apidx,
	IN	UCHAR	InfoReq);

VOID SendNotifyBWActionFrame(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR  Wcid,
	IN UCHAR apidx);

BOOLEAN ChannelSwitchSanityCheck(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR  Wcid,
	IN	UCHAR  NewChannel,
	IN	UCHAR  Secondary);

VOID ChannelSwitchAction(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR  Wcid,
	IN	UCHAR  Channel,
	IN	UCHAR  Secondary);

ULONG BuildIntolerantChannelRep(
	IN	RTMP_ADAPTER *pAd,
	IN	PUCHAR  pDest);

VOID Update2040CoexistFrameAndNotify(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR  Wcid,
	IN	BOOLEAN	bAddIntolerantCha);

VOID Send2040CoexistAction(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR  Wcid,
	IN	BOOLEAN	bAddIntolerantCha);

VOID UpdateBssScanParm(
	IN RTMP_ADAPTER *pAd,
	IN OVERLAP_BSS_SCAN_IE APBssScan);
#endif /* DOT11N_DRAFT3 */

INT AsicSetRalinkBurstMode(RTMP_ADAPTER *pAd, BOOLEAN enable);
#endif /* DOT11_N_SUPPORT */
UCHAR get_regulatory_class(RTMP_ADAPTER *pAd, UCHAR Channel, UCHAR PhyMode, struct wifi_dev *wdev);

UCHAR get_channel_set_num(UCHAR *ChannelSet);

PUCHAR get_channelset_by_reg_class(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 RegulatoryClass,
	IN UCHAR PhyMode);

INT Set_Reg_Domain_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);


VOID BarHeaderInit(
	IN	RTMP_ADAPTER *pAd,
	IN OUT PFRAME_BAR pCntlBar,
	IN PUCHAR pDA,
	IN PUCHAR pSA);

VOID InsertActField(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Category,
	IN UINT8 ActCode);

BOOLEAN QosBADataParse(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN bAMSDU,
	IN PUCHAR p8023Header,
	IN UCHAR	WCID,
	IN UCHAR	TID,
	IN USHORT Sequence,
	IN UCHAR DataOffset,
	IN USHORT Datasize,
	IN UINT   CurRxIndex);

BOOLEAN bar_process(
	RTMP_ADAPTER *pAd,
	ULONG Wcid,
	ULONG MsgLen,
	PFRAME_BA_REQ pMsg);

VOID BaAutoManSwitch(
	IN	RTMP_ADAPTER *pAd);

VOID HTIOTCheck(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR	 BatRecIdx);

INT32 wdev_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum WDEV_TYPE wdev_type, PNET_DEV if_dev,
				INT8 func_idx, VOID *func_dev, VOID *sys_handle);

INT32 wdev_deinit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

enum queue_mm;
INT wdev_ops_register(struct wifi_dev *wdev, enum WDEV_TYPE wdev_type,
					  struct wifi_dev_ops *ops, UCHAR wmm_detect_method);
INT32 wdev_attr_update(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT wdev_tx_pkts(NDIS_HANDLE dev_hnd, PPNDIS_PACKET pkt_list, UINT pkt_cnt, struct wifi_dev *wdev);
UCHAR get_frag_num(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);
BOOLEAN check_if_fragment(struct wifi_dev *wdev, PNDIS_PACKET pPacket);
#ifdef TX_AGG_ADJUST_WKR
BOOLEAN tx_check_for_agg_adjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);
#endif /* TX_AGG_ADJUST_WKR */
VOID tx_bytes_calculate(RTMP_ADAPTER *pAd, TX_BLK *tx_blk);
#ifdef REDUCE_TX_OVERHEAD
VOID tx_bytes_calculate2(RTMP_ADAPTER *pAd, TX_BLK *tx_blk);
#endif
struct wifi_dev *wdev_search_by_address(RTMP_ADAPTER *pAd, UCHAR *Address);
#ifdef RT_CFG80211_SUPPORT
struct wifi_dev *WdevSearchByBssid(RTMP_ADAPTER *pAd, UCHAR *Address);
#endif
struct wifi_dev *wdev_search_by_omac_idx(RTMP_ADAPTER *pAd, UINT8 BssIndex);
struct wifi_dev *wdev_search_by_wcid(RTMP_ADAPTER *pAd, UINT8 wcid);
struct wifi_dev *wdev_search_by_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt);
struct wifi_dev *wdev_search_by_idx(RTMP_ADAPTER *pAd, UINT32 idx);
struct wifi_dev *wdev_search_by_netdev(RTMP_ADAPTER *pAd, VOID *pDev);
VOID BssInfoArgumentLinker(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT32 GetBssInfoIdx(RTMP_ADAPTER *pAd);
INT32 wdev_idx_reg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT32 wdev_idx_unreg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID ReleaseBssIdx(RTMP_ADAPTER *pAd, UINT32 BssIdx);
VOID BssInfoArgumentUnLink(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID BssInfoArgumentLink(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _BSS_INFO_ARGUMENT_T *bssinfo);
VOID wdev_if_up_down(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN if_up_down_state);
INT32 wdev_config_init(struct _RTMP_ADAPTER *pAd);
void wdev_sync_ch_by_rfic(struct _RTMP_ADAPTER *ad, UCHAR rfic, UCHAR ch);
void wdev_sync_prim_ch(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);

#ifdef IP_ASSEMBLY

typedef union ip_flags_frag_offset {
	struct {
#ifdef RT_BIG_ENDIAN
		USHORT flags_reserved:1;
		USHORT flags_may_frag:1;
		USHORT flags_more_frag:1;
		USHORT frag_offset:13;
#else
		USHORT frag_offset:13;
		USHORT flags_more_frag:1;
		USHORT flags_may_frag:1;
		USHORT flags_reserved:1;
#endif
	} field;
	USHORT word;
} IP_FLAGS_FRAG_OFFSET;

typedef struct ip_v4_hdr {
#ifdef RT_BIG_ENDIAN
	UCHAR version:4, ihl:4;
#else
	UCHAR ihl:4, version:4;
#endif
	UCHAR tos;
	USHORT tot_len;
	USHORT identifier;
} IP_V4_HDR;

typedef struct ip_assemble_data {
	DL_LIST list;
	QUEUE_HEADER queue;
	INT32 identify;
	INT32 fragSize;
	ULONG createTime;
} IP_ASSEMBLE_DATA;

INT rtmp_IpAssembleHandle(RTMP_ADAPTER *pAd, STA_TR_ENTRY *pTrEntry, PNDIS_PACKET pPacket, UCHAR queIdx, PACKET_INFO packetInfo);
#endif

VOID ba_ori_session_start(
	IN RTMP_ADAPTER *pAd,
	IN STA_TR_ENTRY *tr_entry,
	IN UINT8 UserPriority);


INT rtmp_tx_burst_set(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID enable_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					 UINT8 ac_type, UINT8 prio, UINT16 level);
VOID disable_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					  UINT8 ac_type, UINT8 prio, UINT16 level);
UINT8 query_tx_burst_prio(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

INT TxOPUpdatingAlgo(RTMP_ADAPTER *pAd);

VOID mt_detect_wmm_traffic(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR UserPriority, UCHAR FlgIsOutput);
VOID rx_802_3_data_frm_announce(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk, struct wifi_dev *wdev);

VOID rx_data_frm_announce(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN struct wifi_dev *wdev);

INT sta_rx_pkt_allow(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk);
INT sta_rx_announce_802_3_pkt(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);

INT rx_chk_duplicate_frame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, struct wifi_dev *wdev);
VOID mt_dynamic_wmm_be_tx_op(
	IN RTMP_ADAPTER *pAd,
	IN ULONG nonBEpackets);

NDIS_STATUS RTMPFreeTXDRequest(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR		   RingType,
	IN  UCHAR		   NumberRequired,
	IN	PUCHAR		  FreeNumberIs);

NDIS_STATUS MlmeHardTransmit(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR	QueIdx,
	IN  PNDIS_PACKET	pPacket,
	IN	BOOLEAN			FlgDataQForce,
	IN	BOOLEAN			FlgIsLocked,
	IN	BOOLEAN			FlgIsCheckPS);

NDIS_STATUS MlmeHardTransmitMgmtRing(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR	QueIdx,
	IN  PNDIS_PACKET	pPacket);

#ifdef RTMP_MAC_PCI
NDIS_STATUS MlmeHardTransmitTxRing(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket);

NDIS_STATUS MlmeDataHardTransmit(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket);

#endif /* RTMP_MAC_PCI */

USHORT RTMPCalcDuration(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Rate,
	IN ULONG Size);

VOID mtd_write_tmac_info_fixed_rate(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *tmac_info,
	IN MAC_TX_INFO * info,
	IN HTTRANSMIT_SETTING *pTransmit);

VOID mt_write_tmac_info_fixed_rate(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *tmac_info,
	IN MAC_TX_INFO * info,
	IN HTTRANSMIT_SETTING *pTransmit);

VOID rtmp_write_tmac_info_fixed_rate(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *tmac_info,
	IN MAC_TX_INFO * info,
	IN HTTRANSMIT_SETTING *pTransmit);

#ifdef RANDOM_PKT_GEN
VOID random_write_qidx(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk);
#endif

#ifdef MT_MAC
VOID mt_write_tmac_info_beacon(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *tmac_buf, HTTRANSMIT_SETTING *BeaconTransmit, ULONG frmLen);
#endif /* MT_MAC */

VOID RTMPSuspendMsduTransmission(
	IN RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

VOID RTMPResumeMsduTransmission(
	IN RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

NDIS_STATUS MiniportMMRequest(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN UCHAR *pData,
	IN UINT Length);

VOID LowPowerDebug(
	PRTMP_ADAPTER pAd,
	PSTA_ADMIN_CONFIG pStaCfg);

VOID RTMPSendNullFrame(
	IN RTMP_ADAPTER *pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN UCHAR TxRate,
	IN BOOLEAN bQosNull,
	IN USHORT PwrMgmt);

#ifdef CONFIG_STA_SUPPORT
VOID RTMPOffloadPm(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg, UINT8 ucPmNumber, UINT8 ucPmState);
VOID RTMPWakeUpWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif /* #ifdef CONFIG_STA_SUPPORT */


#ifdef MT_MAC
UINT32 pkt_alloc_fail_handle(RTMP_ADAPTER *ad, PNDIS_PACKET rx_packet);
UINT32 mt_rx_pkt_process(RTMP_ADAPTER *, UINT8 hif_idx, RX_BLK *pRxBlk, PNDIS_PACKET pRxPacket);
#endif /* MT_MAC */

#ifdef MT7626_E2_SUPPORT
UINT32 mt_rx2_pkt_process(RTMP_ADAPTER *, UINT8 hif_idx, RX_BLK *pRxBlk, PNDIS_PACKET pRxPacket);
#endif

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
BOOLEAN is_looping_packet(
	IN RTMP_ADAPTER *pAd,
	IN NDIS_PACKET * pPacket);

VOID set_wf_fwd_cb(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket,
	IN struct wifi_dev *wdev);
#endif /* CONFIG_WIFI_PKT_FWD */

BOOLEAN is_udp_packet(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt);

#ifdef TXRX_STAT_SUPPORT
BOOLEAN RTMPGetUserPriority(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET	pPacket,
	IN struct wifi_dev *wdev,
	OUT UCHAR *pUserPriority,
	OUT UCHAR *pQueIdx);

#endif
BOOLEAN RTMPCheckEtherType(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET	pPacket,
	IN STA_TR_ENTRY *tr_entry,
	IN struct wifi_dev *wdev);

VOID RTMPCckBbpTuning(
	IN	RTMP_ADAPTER *pAd,
	IN	UINT			TxRate);

/*
	MLME routines
*/

/* Asic/RF/BBP related functions */
VOID AsicGetTxPowerOffset(
	IN PRTMP_ADAPTER			pAd,
	IN PULONG					TxPwr);

VOID AsicExtraPowerOverMAC(RTMP_ADAPTER *pAd);

#ifdef SINGLE_SKU
VOID GetSingleSkuDeltaPower(
	IN		RTMP_ADAPTER *pAd,
	IN		PCHAR			pTotalDeltaPower,
	INOUT	PULONG			pSingleSKUTotalDeltaPwr,
	INOUT	PUCHAR			pSingleSKUBbpR1Offset);
#endif /* SINGLE_SKU*/

VOID AsicPercentageDeltaPower(
	IN		PRTMP_ADAPTER		pAd,
	IN		CHAR				Rssi,
	INOUT	PCHAR				pDeltaPwr,
	INOUT	PCHAR				pDeltaPowerByBbpR1);

VOID AsicCompensatePowerViaBBP(
	IN RTMP_ADAPTER *pAd,
	INOUT CHAR *pTotalDeltaPower);

VOID AsicAdjustTxPower(RTMP_ADAPTER *pAd);

#define WMM_PARAM_TXOP	0
#define WMM_PARAM_AIFSN	1
#define WMM_PARAM_CWMIN	2
#define WMM_PARAM_CWMAX	3
#define WMM_PARAM_ALL		4

#define WMM_PARAM_AC_0		0
#define WMM_PARAM_AC_1		1
#define WMM_PARAM_AC_2		2
#define WMM_PARAM_AC_3		3

#ifdef RTMP_RBUS_SUPPORT
int RtmpAsicSendCommandToSwMcu(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN FlgIsNeedLocked);
#endif /* RTMP_RBUS_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
UINT32 StreamModeRegVal(
	IN RTMP_ADAPTER *pAd);

/*
	Update the Tx chain address
	Parameters
		pAd: The adapter data structure
		pMacAddress: The MAC address of the peer STA

	Return Value:
		None
*/
VOID AsicUpdateTxChainAddress(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddress);

INT Set_StreamMode_Proc(
	IN  RTMP_ADAPTER *pAd,
	IN  RTMP_STRING *arg);

INT Set_StreamModeMac_Proc(
	IN  RTMP_ADAPTER *pAd,
	IN  RTMP_STRING *arg);

INT Set_StreamModeMCS_Proc(
	IN  RTMP_ADAPTER *pAd,
	IN  RTMP_STRING *arg);
#endif /* STREAM_MODE_SUPPORT */

VOID MacAddrRandomBssid(
	IN  RTMP_ADAPTER *pAd,
	OUT PUCHAR pAddr);

VOID MgtMacHeaderInit(
	IN  RTMP_ADAPTER *pAd,
	INOUT HEADER_802_11 *pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN UCHAR *pDA,
	IN UCHAR *pSA,
	IN UCHAR *pBssid);

VOID MgtMacHeaderInitExt(
	IN  RTMP_ADAPTER *pAd,
	IN OUT PHEADER_802_11 pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN PUCHAR pDA,
	IN PUCHAR pSA,
	IN PUCHAR pBssid);

VOID MlmeRadioOff(
	IN RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

VOID MlmeRadioOn(
	IN RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

VOID MlmeLpEnter(
	IN RTMP_ADAPTER *pAd);

VOID MlmeLpExit(
	IN RTMP_ADAPTER *pAd);

VOID BssTableInit(
	IN BSS_TABLE *Tab);

#ifdef DOT11_N_SUPPORT
VOID BATableInit(
	IN RTMP_ADAPTER *pAd,
	IN BA_TABLE *Tab);

VOID BATableExit(
	IN RTMP_ADAPTER *pAd);
#endif /* DOT11_N_SUPPORT */

ULONG BssTableSearch(
	IN BSS_TABLE *Tab,
	IN PUCHAR pBssid,
	IN UCHAR Channel);

ULONG BssSsidTableSearch(
	IN BSS_TABLE *Tab,
	IN PUCHAR	pBssid,
	IN PUCHAR	pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel);

ULONG BssTableSearchWithSSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR	Bssid,
	IN PUCHAR	pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel);

ULONG BssSsidTableSearchBySSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen);

VOID BssTableDeleteEntry(
	IN OUT  PBSS_TABLE pTab,
	IN	  PUCHAR pBssid,
	IN	  UCHAR Channel);

ULONG BssTableSetEntry(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	OUT BSS_TABLE *Tab,
	IN BCN_IE_LIST * ie_list,
	IN CHAR Rssi,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE);

#ifdef DOT11_N_SUPPORT
VOID BATableInsertEntry(
	IN	RTMP_ADAPTER *pAd,
	IN USHORT Aid,
	IN USHORT		TimeOutValue,
	IN USHORT		StartingSeq,
	IN UCHAR TID,
	IN UCHAR BAWinSize,
	IN UCHAR OriginatorStatus,
	IN BOOLEAN IsRecipient);

BOOLEAN bss_coex_insert_effected_ch_list(
	RTMP_ADAPTER *pAd,
	UCHAR Channel,
	BCN_IE_LIST *ie_list,
	struct wifi_dev *wdev);

#ifdef DOT11N_DRAFT3
BOOLEAN build_trigger_event_table(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN BCN_IE_LIST *ie_list);

VOID Bss2040CoexistTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID  TriEventInit(
	IN	RTMP_ADAPTER *pAd);

INT TriEventTableSetEntry(
	IN	RTMP_ADAPTER *pAd,
	OUT TRIGGER_EVENT_TAB *Tab,
	IN PUCHAR pBssid,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR			HtCapabilityLen,
	IN UCHAR			RegClass,
	IN UCHAR ChannelNo);

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

VOID BssTableSsidSort(
	IN  RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	OUT BSS_TABLE *OutTab,
	IN  CHAR Ssid[],
	IN  UCHAR SsidLen);

VOID  BssTableSortByRssi(
	IN OUT BSS_TABLE *OutTab,
	IN BOOLEAN isInverseOrder);

NDIS_STATUS  MlmeQueueInit(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE *Queue);

VOID  MlmeQueueDestroy(
	IN MLME_QUEUE *Queue);

BOOLEAN MlmeEnqueue(
	IN RTMP_ADAPTER *pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN ULONG Priv);

BOOLEAN MlmeEnqueueWithWdev(
	IN RTMP_ADAPTER *pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN ULONG Priv,
	IN struct wifi_dev *wdev,
	struct inter_machine_info *priv_data);

BOOLEAN MlmeEnqueueForRecv(
	IN  RTMP_ADAPTER *pAd,
	IN ULONG Wcid,
	IN struct raw_rssi_info *rssi_info,
	IN ULONG MsgLen,
	IN PVOID Msg,
	IN UCHAR OpMode,
	IN struct wifi_dev *wdev,
	IN UCHAR RxPhyMode);

#ifdef WSC_INCLUDED
BOOLEAN MlmeEnqueueForWsc(
	IN RTMP_ADAPTER *pAd,
	IN ULONG eventID,
	IN LONG senderID,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN struct wifi_dev *wdev);
#endif /* WSC_INCLUDED */

BOOLEAN MlmeDequeue(
	IN MLME_QUEUE *Queue,
	OUT MLME_QUEUE_ELEM **Elem);

VOID	MlmeRestartStateMachine(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev);

BOOLEAN  MlmeQueueEmpty(
	IN MLME_QUEUE *Queue);

BOOLEAN  MlmeQueueFull(
	IN MLME_QUEUE *Queue,
	IN UCHAR SendId);

BOOLEAN  MsgTypeSubst(
	IN RTMP_ADAPTER *pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType);

VOID StateMachineInit(
	IN STATE_MACHINE *Sm,
	IN STATE_MACHINE_FUNC Trans[],
	IN ULONG StNr,
	IN ULONG MsgNr,
	IN STATE_MACHINE_FUNC DefFunc,
	IN ULONG InitState,
	IN ULONG Base);

VOID StateMachineSetAction(
	IN STATE_MACHINE *S,
	IN ULONG St,
	ULONG Msg,
	IN STATE_MACHINE_FUNC F);

VOID StateMachineSetMsgChecker(
	IN STATE_MACHINE *S,
	IN STATE_MACHINE_MSG_CHECKER MsgCheckFun);

VOID StateMachinePerformAction(
	IN  RTMP_ADAPTER *pAd,
	IN STATE_MACHINE *S,
	IN MLME_QUEUE_ELEM *Elem,
	IN ULONG CurrState);

VOID Drop(
	IN  RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID AssocStateMachineInit(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev,
	IN  STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID ReassocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID AssocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID DisassocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

/*---------------------------------------------- */
VOID MlmeDisassocReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeAssocReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeReassocReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeDisassocReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAssocRspAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerReassocRspAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerDisassocAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID DisassocTimeoutAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID AssocTimeoutAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID  ReassocTimeoutAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID  Cls3errAction(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR pAddr);

VOID  InvalidStateWhenAssoc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID  InvalidStateWhenReassoc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenDisassociate(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);


VOID  ComposePsPoll(
	IN	RTMP_ADAPTER *pAd,
	IN	PPSPOLL_FRAME pPsPollFrame,
	IN	USHORT	Aid,
	IN	UCHAR *pBssid,
	IN	UCHAR *pTa);

VOID ComposeNullFrame(
	RTMP_ADAPTER *pAd,
	PHEADER_802_11 pNullFrame,
	UCHAR *pAddr1,
	UCHAR *pAddr2,
	UCHAR *pAddr3);

VOID  AssocPostProc(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR pAddr2,
	IN  USHORT CapabilityInfo,
	IN  USHORT Aid,
	IN  UCHAR SupRate[],
	IN  UCHAR SupRateLen,
	IN  UCHAR ExtRate[],
	IN  UCHAR ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	struct _IE_lists *ie_list,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN  UCHAR HtCapabilityLen,
	IN ADD_HT_INFO_IE *pAddHtInfo,
	IN MAC_TABLE_ENTRY *pEntry);

VOID AuthStateMachineInit(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev,
	IN PSTATE_MACHINE sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID AuthTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID MlmeAuthReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAuthRspAtSeq2Action(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAuthRspAtSeq4Action(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID AuthTimeoutAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID Cls2errAction(
	IN  struct wifi_dev *wdev,
	IN  PUCHAR pAddr);

VOID MlmeDeauthReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenAuth(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

/*============================================= */

VOID AuthRspStateMachineInit(
	IN  RTMP_ADAPTER *pAd,
	IN  PSTATE_MACHINE Sm,
	IN  STATE_MACHINE_FUNC Trans[]);

VOID PeerDeauthAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerAuthSimpleRspGenAndSend(
	IN  RTMP_ADAPTER *pAd,
	IN  PHEADER_802_11  pHdr80211,
	IN  USHORT Alg,
	IN  USHORT Seq,
	IN  USHORT Reason,
	IN  USHORT Status);

BOOLEAN PeerProbeReqSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	OUT PEER_PROBE_REQ_PARAM * Param);

/*======================================== */

VOID SyncStateMachineInit(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev,
	IN  STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeScanReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenScan(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenJoin(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenStart(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeacon(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID EnqueueProbeRequest(
	IN RTMP_ADAPTER *pAd);

BOOLEAN ScanRunning(
	IN RTMP_ADAPTER *pAd);
/*========================================= */


VOID MlmeCntlMachinePerformAction(
	IN  RTMP_ADAPTER *pAd,
	IN  STATE_MACHINE *S,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlIdleProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlOidScanProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlOidSsidProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlOidRTBssidProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlMlmeRoamingProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitDisassocProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitJoinProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitReassocProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitStartProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAuthProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAuthProc2(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAssocProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

#ifdef DOT11V_WNM_SUPPORT
#ifdef CONFIG_AP_SUPPORT
VOID APPeerWNMBTMQueryAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID APPeerWNMBTMRspAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID APPeerWNMDMSReqAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
VOID MlmeWNMBTMQueryAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID MlmeWNMDMSRspAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerWNMBTMReqAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

#endif /* CONFIG_STA_SUPPORT */
#endif /* DOT11V_WNM_SUPPORT */

VOID LinkUp(
	RTMP_ADAPTER *pAd,
	UCHAR BssType,
	struct wifi_dev *wdev,
	UINT link_up_type,
	MLME_QUEUE_ELEM *Elem);

VOID LinkDown(
	RTMP_ADAPTER *pAd,
	UINT linkdown_type,
	struct wifi_dev *wdev,
	MLME_QUEUE_ELEM *Elem);

VOID IterateOnBssTab(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev);

VOID IterateOnBssTab2(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev);

VOID AssocParmFill(
	IN  RTMP_ADAPTER *pAd,
	IN OUT MLME_ASSOC_REQ_STRUCT * AssocReq,
	IN  PUCHAR pAddr,
	IN  USHORT CapabilityInfo,
	IN  ULONG Timeout,
	IN  USHORT ListenIntv);

VOID DisassocParmFill(
	IN  RTMP_ADAPTER *pAd,
	IN  OUT VOID *DisassocReq, /* snowpin for cntl mgmt */
	IN  PUCHAR pAddr,
	IN  USHORT Reason);

VOID EnqueuePsPoll(
	IN  RTMP_ADAPTER *pAd,
	PSTA_ADMIN_CONFIG pStaCfg);

VOID EnqueueBeaconFrame(
	IN  RTMP_ADAPTER *pAd);

VOID MlmeJoinReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeScanReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeStartReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeForceJoinReqAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID MlmeForceScanReqAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID ScanTimeoutAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID BeaconTimeoutAtJoinAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeaconAtScanAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeaconAtJoinAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeacon(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerProbeReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);


BOOLEAN MlmeScanReqSanity(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT UCHAR *BssType,
	OUT CHAR ssid[],
	OUT UCHAR *SsidLen,
	OUT UCHAR *ScanType);

BOOLEAN PeerBeaconAndProbeRspSanity_Old(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	IN  UCHAR MsgChannel,
	OUT PUCHAR pAddr2,
	OUT PUCHAR pBssid,
	OUT CHAR Ssid[],
	OUT UCHAR *pSsidLen,
	OUT UCHAR *pBssType,
	OUT USHORT *pBeaconPeriod,
	OUT UCHAR *pChannel,
	OUT UCHAR *pNewChannel,
	OUT LARGE_INTEGER *pTimestamp,
	OUT CF_PARM * pCfParm,
	OUT USHORT *pAtimWin,
	OUT USHORT *pCapabilityInfo,
	OUT UCHAR *pErp,
	OUT UCHAR *pDtimCount,
	OUT UCHAR *pDtimPeriod,
	OUT UCHAR *pBcastFlag,
	OUT UCHAR *pMessageToMe,
	OUT UCHAR SupRate[],
	OUT UCHAR *pSupRateLen,
	OUT UCHAR ExtRate[],
	OUT UCHAR *pExtRateLen,
	OUT	UCHAR *pCkipFlag,
	OUT	UCHAR *pAironetCellPowerLimit,
	OUT PEDCA_PARM	   pEdcaParm,
	OUT PQBSS_LOAD_PARM  pQbssLoad,
	OUT PQOS_CAPABILITY_PARM pQosCapability,
	OUT ULONG *pRalinkIe,
	OUT UCHAR		 *pHtCapabilityLen,
#ifdef CONFIG_STA_SUPPORT
	OUT UCHAR		 *pPreNHtCapabilityLen,
#endif /* CONFIG_STA_SUPPORT */
	OUT HT_CAPABILITY_IE *pHtCapability,
	OUT EXT_CAP_INFO_ELEMENT *pExtCapInfo,
	OUT UCHAR		 *AddHtInfoLen,
	OUT ADD_HT_INFO_IE *AddHtInfo,
	OUT UCHAR *NewExtChannel,
	OUT USHORT *LengthVIE,
	OUT PNDIS_802_11_VARIABLE_IEs pVIE);

BOOLEAN PeerBeaconAndProbeRspSanity(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN VOID *Msg,
	IN ULONG MsgLen,
	IN UCHAR  MsgChannel,
	OUT BCN_IE_LIST *ie_list,
	OUT USHORT *LengthVIE,
	OUT PNDIS_802_11_VARIABLE_IEs pVIE,
	IN BOOLEAN bGetDtim,

	IN BOOLEAN bFromBeaconReport
);

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
BOOLEAN PeerBeaconAndProbeRspSanity2(
	IN RTMP_ADAPTER *pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	IN OVERLAP_BSS_SCAN_IE * BssScan,
	OUT UCHAR	*RegClass);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

BOOLEAN PeerAddBAReqActionSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr2);

BOOLEAN PeerAddBARspActionSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen);

BOOLEAN PeerDelBAActionSanity(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	IN VOID *pMsg,
	IN ULONG MsgLen);

BOOLEAN MlmeAssocReqSanity(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pApAddr,
	OUT USHORT *CapabilityInfo,
	OUT ULONG *Timeout,
	OUT USHORT *ListenIntv);

BOOLEAN MlmeAuthReqSanity(
	IN  RTMP_ADAPTER *pAd,
#ifdef CONFIG_STA_SUPPORT
	IN struct wifi_dev *wdev,
#endif
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr,
	OUT ULONG *Timeout,
	OUT USHORT *Alg);

BOOLEAN MlmeStartReqSanity(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT CHAR Ssid[],
	OUT UCHAR *Ssidlen);

BOOLEAN PeerAuthSanity(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr,
	OUT USHORT *Alg,
	OUT USHORT *Seq,
	OUT USHORT *Status,
	OUT CHAR ChlgText[]);

BOOLEAN PeerAssocRspSanity(
	IN  struct wifi_dev *wdev,
	IN  VOID *pMsg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT USHORT *pCapabilityInfo,
	OUT USHORT *pStatus,
	OUT USHORT *pAid,
	OUT UCHAR SupRate[],
	OUT UCHAR *pSupRateLen,
	OUT UCHAR ExtRate[],
	OUT UCHAR *pExtRateLen,
	OUT HT_CAPABILITY_IE		*pHtCapability,
	OUT ADD_HT_INFO_IE *pAddHtInfo,	/* AP might use this additional ht info IE */
	OUT UCHAR			*pHtCapabilityLen,
	OUT UCHAR			*pAddHtInfoLen,
	OUT UCHAR			*pNewExtChannelOffset,
	OUT PEDCA_PARM pEdcaParm,
	OUT EXT_CAP_INFO_ELEMENT *pExtCapInfo,
	OUT UCHAR *pCkipFlag,
	struct _IE_lists *ie_list);

BOOLEAN PeerDisassocSanity(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT USHORT *Reason);

BOOLEAN PeerDeauthSanity(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr1,
	OUT PUCHAR pAddr2,
	OUT PUCHAR pAddr3,
	OUT USHORT *Reason);

BOOLEAN GetTimBit(
	IN  CHAR *Ptr,
	IN  USHORT Aid,
	OUT UCHAR *TimLen,
	OUT UCHAR *BcastFlag,
	OUT UCHAR *DtimCount,
	OUT UCHAR *DtimPeriod,
	OUT UCHAR *MessageToMe);

UCHAR ChannelSanity(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR channel);

UCHAR ChannelSanityByWdev(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR channel);

NDIS_802_11_NETWORK_TYPE NetworkTypeInUseSanity(
	IN BSS_ENTRY *pBss);

BOOLEAN MlmeDelBAReqSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *Msg,
	IN ULONG MsgLen);

BOOLEAN MlmeAddBAReqSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr2);

ULONG MakeOutgoingFrame(
	OUT UCHAR *Buffer,
	OUT ULONG *Length, ...);

UCHAR RandomByte(
	IN  RTMP_ADAPTER *pAd);

UCHAR RandomByte2(
	IN  RTMP_ADAPTER *pAd);

VOID MlmePeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID  MlmePeriodicExecTimer(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID LinkDownExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID LinkUpExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID STAMlmePeriodicExec(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

VOID sta_2040_coex_scan_check(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

VOID MlmeAutoScan(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev);

VOID MlmeAutoReconnectLastSSID(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev);

BOOLEAN MlmeValidateSSID(
	IN PUCHAR pSsid,
	IN UCHAR  SsidLen);

VOID MlmeCheckForRoaming(
	IN RTMP_ADAPTER *pAd,
	IN ULONG	Now32);

BOOLEAN MlmeCheckForFastRoaming(
	IN  RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev);

VOID MlmeCalculateChannelQuality(
	IN RTMP_ADAPTER *pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN ULONG Now);

VOID MlmeCheckPsmChange(
	IN RTMP_ADAPTER *pAd,
	IN ULONG	Now32,
	IN struct wifi_dev *wdev);

VOID MlmeSetPsmBit(
	IN RTMP_ADAPTER *pAd,
	IN PSTA_ADMIN_CONFIG pStaCfg,
	IN USHORT psm);

#ifdef STA_LP_PHASE_1_SUPPORT
VOID RTMPSetEnterPsmNullBit(
	IN PPWR_MGMT_STRUCT pPwrMgmt);

VOID RTMPClearEnterPsmNullBit(
	IN PPWR_MGMT_STRUCT pPwrMgmt);

BOOLEAN RTMPEnterPsmNullBitStatus(
	IN PPWR_MGMT_STRUCT pPwrMgmt);
#endif /* STA_LP_PHASE_1_SUPPORT */

VOID MlmeSetTxPreamble(
	IN RTMP_ADAPTER *pAd,
	IN USHORT TxPreamble);

VOID UpdateBasicRateBitmap(
	IN	RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev);

VOID MlmeUpdateTxRates(
	IN RTMP_ADAPTER *pAd,
	IN	BOOLEAN			bLinkUp,
	IN	UCHAR			apidx);

VOID MlmeUpdateTxRatesWdev(RTMP_ADAPTER *pAd, BOOLEAN bLinkUp, struct wifi_dev *wdev);

#ifdef DOT11_N_SUPPORT
VOID MlmeUpdateHtTxRates(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#ifdef DOT11_VHT_AC
VOID MlmeUpdateVhtTxRates(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pEntry, struct wifi_dev *wdev);
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

VOID	RTMPCheckRates(
	IN	  RTMP_ADAPTER *pAd,
	IN OUT  UCHAR		   SupRate[],
	IN OUT  UCHAR		   *SupRateLen,
	IN	  UCHAR PhyMode);

BOOLEAN RTMPCheckHt(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	INOUT HT_CAPABILITY_IE *pHtCapability,
	INOUT ADD_HT_INFO_IE *pAddHtInfo);

#ifdef DOT11_VHT_AC
BOOLEAN RTMPCheckVht(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	IN VHT_CAP_IE *vht_cap,
	IN VHT_OP_IE * vht_op);
#endif /* DOT11_VHT_AC */

VOID RTMPUpdateMlmeRate(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

CHAR RTMPMaxRssi(
	IN RTMP_ADAPTER *pAd,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2);

CHAR RTMPMinRssi(
	IN RTMP_ADAPTER *pAd,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2,
	IN CHAR				Rssi3);

CHAR RTMPAvgRssi(
	IN RTMP_ADAPTER *pAd,
	IN RSSI_SAMPLE		*pRssi);

CHAR RTMPMinSnr(
	IN RTMP_ADAPTER *pAd,
	IN CHAR				Snr0,
	IN CHAR				Snr1);

#ifdef MICROWAVE_OVEN_SUPPORT
INT Set_MO_FalseCCATh_Proc(
	IN	RTMP_ADAPTER *pAd,
	IN	RTMP_STRING *arg);

#endif /* MICROWAVE_OVEN_SUPPORT */

#ifdef RTMP_EFUSE_SUPPORT
INT set_eFuseGetFreeBlockCount_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_eFusedump_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_eFuseLoadFromBin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

UCHAR eFuseReadRegisters(RTMP_ADAPTER *pAd, USHORT Offset, UINT16 Length, UINT16 *pData);
VOID EfusePhysicalReadRegisters(RTMP_ADAPTER *pAd, USHORT Offset, USHORT Length, USHORT *pData);

#ifdef CONFIG_ATE
INT Set_LoadEepromBufferFromEfuse_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_eFuseBufferModeWriteBack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_BinModeWriteBack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CONFIG_ATE */

VOID  rtmp_ee_load_from_efuse(RTMP_ADAPTER *pAd);

#endif /* RTMP_EFUSE_SUPPORT */

VOID AsicEvaluateRxAnt(RTMP_ADAPTER *pAd);

VOID AsicRxAntEvalTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID APSDPeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

BOOLEAN RTMPCheckEntryEnableAutoRateSwitch(
	IN RTMP_ADAPTER *pAd,
	IN PMAC_TABLE_ENTRY	pEntry);

UCHAR RTMPStaFixedTxMode(
	IN RTMP_ADAPTER *pAd,
	IN PMAC_TABLE_ENTRY	pEntry);

VOID RTMPUpdateLegacyTxSetting(
	IN UCHAR fixed_tx_mode,
	IN PMAC_TABLE_ENTRY	pEntry);

BOOLEAN RTMPAutoRateSwitchCheck(RTMP_ADAPTER *pAd);

VOID MlmeHalt(RTMP_ADAPTER *pAd);
NDIS_STATUS MlmeInit(RTMP_ADAPTER *pAd);
VOID MlmeResetRalinkCounters(RTMP_ADAPTER *pAd);

VOID BuildChannelList(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UCHAR ApAutoChannelAtBootUp(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

UCHAR FirstChannel(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UCHAR NextChannel(
	RTMP_ADAPTER *pAd,
	SCAN_CTRL *ScanCtrl,
	UCHAR channel, struct wifi_dev *wdev);
UCHAR FindScanChannel(RTMP_ADAPTER *pAd, UINT8 LastScanChannel, struct wifi_dev *wdev);
UCHAR scan_find_next_channel(RTMP_ADAPTER *pAd, SCAN_CTRL *ScanCtrl, UINT8 LastScanChannel);
INT scan_partial_init(RTMP_ADAPTER *pAd);
VOID scan_partial_trigger_checker(RTMP_ADAPTER *pAd);
INT scan_release_mem(struct _RTMP_ADAPTER *ad);

VOID ChangeToCellPowerLimit(RTMP_ADAPTER *pAd, UCHAR AironetCellPowerLimit);

VOID	RTMPInitMICEngine(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR		  pKey,
	IN  PUCHAR		  pDA,
	IN  PUCHAR		  pSA,
	IN  UCHAR		   UserPriority,
	IN  PUCHAR		  pMICKey);

BOOLEAN RTMPTkipCompareMICValue(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR		  pSrc,
	IN  PUCHAR		  pDA,
	IN  PUCHAR		  pSA,
	IN  PUCHAR		  pMICKey,
	IN	UCHAR			UserPriority,
	IN  UINT			Len);

VOID	RTMPCalculateMICValue(
	IN  RTMP_ADAPTER *pAd,
	IN  PNDIS_PACKET	pPacket,
	IN  PUCHAR pEncap,
	IN	PUCHAR	pKey,
	IN	PUCHAR	pMIC,
	IN	UCHAR apidx);

VOID	RTMPTkipAppendByte(TKIP_KEY_INFO *pTkip, UCHAR uChar);
VOID	RTMPTkipAppend(TKIP_KEY_INFO *pTkip, UCHAR *pSrc, UINT nBytes);
VOID RTMPTkipGetMIC(TKIP_KEY_INFO *pTkip);


INT RT_CfgSetCountryRegion(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg,
	IN INT band);

INT RT_CfgSetWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg, struct wifi_dev *wdev);

/*update phy mode for all of wdev*/
VOID RtmpUpdatePhyMode(RTMP_ADAPTER *pAd, UCHAR *pWmode);
RT_802_11_PHY_MODE wmode_2_cfgmode(UCHAR wmode);
UCHAR cfgmode_2_wmode(UCHAR cfg_mode);
#ifdef MIN_PHY_RATE_SUPPORT
HTTRANSMIT_SETTING MinPhyRate_2_HtTransmit(UCHAR MinPhyRate);
#endif /* MIN_PHY_RATE_SUPPORT */

BOOLEAN wmode_valid_and_correct(RTMP_ADAPTER *pAd, UCHAR *wmode);
UCHAR *wmode_2_str(UCHAR wmode);

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
INT RT_CfgSetMbssWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

INT RT_CfgSetShortSlot(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	RT_CfgSetWepKey(
	IN	RTMP_ADAPTER *pAd,
	IN	RTMP_STRING *keyString,
	IN	CIPHER_KEY		*pSharedKey,
	IN	INT				keyIdx);

INT	RT_CfgSetFixedTxPhyMode(RTMP_STRING *arg);
INT	RT_CfgSetMacAddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg, UCHAR idx);
INT	RT_CfgSetTxMCSProc(RTMP_STRING *arg, BOOLEAN *pAutoRate);
INT	RT_CfgSetAutoFallBack(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef WSC_INCLUDED
INT	RT_CfgSetWscPinCode(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *pPinCodeStr,
	OUT PWSC_CTRL   pWscControl);
#endif /* WSC_INCLUDED */

INT	Set_Antenna_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef HW_TX_RATE_LOOKUP_SUPPORT
INT Set_HwTxRateLookUp_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* HW_TX_RATE_LOOKUP_SUPPORT */

#ifdef MULTI_MAC_ADDR_EXT_SUPPORT
INT Set_EnMultiMacAddrExt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_MultiMacAddrExt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MULTI_MAC_ADDR_EXT_SUPPORT */

INT set_tssi_enable(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef MT_MAC

INT set_cr4_query(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_cr4_set(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_cr4_capability(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_cr4_debug(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT dump_cr4_pdma_debug_probe(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT dump_remap_cr_content(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef MT7626
INT set_cal_control(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT set_re_calibration(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_thermal_recal_mode(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_get_fid(RTMP_ADAPTER *pAd, char *arg);
INT set_fw_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_fw_dbg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_isr_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txop_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_rts_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ser(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 set_fw_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 get_fw_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT SetManualTxOP(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_themal_sensor(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetManualTxOPThreshold(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetManualTxOPUpBound(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetManualTxOPLowBound(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetPSEWatchDog_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT setTmrEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT setTmrVerProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT SetTmrCalProc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

#ifdef FW_DUMP_SUPPORT
INT set_fwdump_path(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT fwdump_print(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_fwdump_max_size(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#endif /* MT_MAC */

#ifdef RTMP_MAC_PCI
INT Set_PDMAWatchDog_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT	Set_RadioOn_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);


#ifdef CUSTOMER_RSG_FEATURE
INT Set_ApEnableRadioChStats_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);
VOID ReadChannelStats(IN PRTMP_ADAPTER pAd);
#endif

#ifdef CUSTOMER_DCC_FEATURE
VOID RTMPIoctlQuerySTAStat(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	* wrq);
VOID dump_STA_Assoc_Cap(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);

#endif
#ifdef NEW_SET_RX_STREAM
INT	Set_RxStream_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif

INT	Set_Lp_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);

INT set_fix_amsdu(
	PRTMP_ADAPTER pAd,
	char *arg);

INT set_rx_max_cnt(
	PRTMP_ADAPTER pAd,
	char *arg);

INT set_rx1_max_cnt(
	PRTMP_ADAPTER pAd,
	char *arg);

#ifdef MT7626_E2_SUPPORT
INT set_rx2_max_cnt(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif

INT set_rx_dly_ctl(
	PRTMP_ADAPTER pAd,
	char *arg);

INT set_tx_dly_ctl(
	PRTMP_ADAPTER pAd,
	char *arg);

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
void set_pcie_aspm_dym_ctrl_cap(
	PRTMP_ADAPTER pAd,
	BOOLEAN flag_pcie_aspm_dym_ctrl);

BOOLEAN get_pcie_aspm_dym_ctrl_cap(
	PRTMP_ADAPTER pAd);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef CONFIG_DVT_MODE
INT16 i2SetDvt(RTMP_ADAPTER *pAd, RTMP_STRING *pArg);
#endif /* CONFIG_DVT_MODE */

NDIS_STATUS RTMPWPARemoveKeyProc(
	IN  RTMP_ADAPTER *pAd,
	IN  PVOID		   pBuf);

VOID RTMPWPARemoveAllKeys(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev);

BOOLEAN RTMPCheckStrPrintAble(
	IN  CHAR *pInPutStr,
	IN  UCHAR strLen);

VOID RTMPSetDefaultChannel(
	IN	PRTMP_ADAPTER	pAd);

VOID RTMPUpdateRateInfo(
	UCHAR phymode,
	struct dev_rate_info *rate
);

#ifdef GN_MIXMODE_SUPPORT
VOID RTMPUpdateGNRateInfo(
	UCHAR phymode,
	struct dev_rate_info *rate
);
#endif /*GN_MIXMODE_SUPPORT*/

VOID RTMPSetPhyMode(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev,
	IN  UCHAR phymode);

VOID RTMPUpdateHTIE(
	IN		UCHAR				*pMcsSet,
	IN struct wifi_dev *wdev,
	OUT		HT_CAPABILITY_IE *pHtCapability,
	OUT		ADD_HT_INFO_IE *pAddHtInfo);

VOID RTMPAddWcidAttributeEntry(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR			BssIdx,
	IN	UCHAR			KeyIdx,
	IN	UCHAR			CipherAlg,
	IN	MAC_TABLE_ENTRY *pEntry);

RTMP_STRING *GetEncryptType(CHAR enc);
RTMP_STRING *GetAuthMode(CHAR auth);

VOID MacTableSetEntryRaCap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *ent, struct _vendor_ie_cap *vendor_ie);
#ifdef DOT11_VHT_AC
VOID MacTableEntryCheck2GVHT(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *ent, UCHAR vht_cap_len);
#endif /* DOT11_VHT_AC */

VOID RTMPDisableDesiredHtInfo(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

#ifdef SYSTEM_LOG_SUPPORT
VOID RtmpDrvSendWirelessEvent(
	IN	VOID			*pAdSrc,
	IN	USHORT			Event_flag,
	IN	PUCHAR			pAddr,
	IN  UCHAR			wdev_idx,
	IN	CHAR			Rssi);
#else
#define RtmpDrvSendWirelessEvent(_pAd, _Event_flag, _pAddr, wdev_idx, _Rssi)
#endif /* SYSTEM_LOG_SUPPORT */

CHAR ConvertToRssi(
	IN RTMP_ADAPTER *pAd,
	IN struct raw_rssi_info *rssi_info,
	IN UCHAR rssi_idx);

CHAR ConvertToSnr(RTMP_ADAPTER *pAd, UCHAR Snr);

#ifdef CONFIG_STA_SUPPORT
BOOLEAN AdjustBwToSyncAp(RTMP_ADAPTER *pAd, BCN_IE_LIST *ie_list, UCHAR  Wcid, UCHAR fun_tb_idx);
#endif

#ifdef DOT11N_DRAFT3
VOID BuildEffectedChannelList(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev);


VOID CntlChannelWidth(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR			PrimaryChannel,
	IN UCHAR			CentralChannel,
	IN UCHAR			ChannelWidth,
	IN UCHAR			SecondaryChannelOffset);

#endif /* DOT11N_DRAFT3 */

VOID APAsicEvaluateRxAnt(
	IN RTMP_ADAPTER *pAd);


VOID APAsicRxAntEvalTimeout(RTMP_ADAPTER *pAd);

MAC_TABLE_ENTRY *PACInquiry(RTMP_ADAPTER *pAd, UCHAR Wcid);

VOID HandleCounterMeasure(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY  *pEntry);

VOID WPAStart4WayHS(
	IN PRTMP_ADAPTER	pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN ULONG			TimeInterval);

VOID WPAStart2WayGroupHS(
	IN  RTMP_ADAPTER *pAd,
	IN  MAC_TABLE_ENTRY *pEntry);

VOID CMTimerExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID RTMPHandleSTAKey(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM  *Elem);

VOID MlmeDeAuthAction(
	IN  RTMP_ADAPTER *pAd,
	IN  PMAC_TABLE_ENTRY pEntry,
	IN  USHORT		   Reason,
	IN  BOOLEAN		  bDataFrameFirst);

#ifdef DOT11W_PMF_SUPPORT
VOID PMF_SAQueryTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID PMF_SAQueryConfirmTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif /* DOT11W_PMF_SUPPORT */

VOID AES_128_CMAC(
	IN	PUCHAR	key,
	IN	PUCHAR	input,
	IN	INT		len,
	OUT	PUCHAR	mac);

#ifdef DOT1X_SUPPORT
VOID	WpaSend(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR		  pPacket,
	IN  ULONG		   Len);

INT RTMPAddPMKIDCache(
	IN NDIS_AP_802_11_PMKID *pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr,
	IN UCHAR *PMKID,
	IN UCHAR *PMK,
	IN UINT8 pmk_len);

INT RTMPSearchPMKIDCache(
	IN NDIS_AP_802_11_PMKID *pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr);

INT RTMPSearchPMKIDCacheByPmkId(
	IN NDIS_AP_802_11_PMKID * pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr,
	IN UCHAR *pPmkId);

INT RTMPValidatePMKIDCache(
	IN NDIS_AP_802_11_PMKID *pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr,
	IN UCHAR *pPMKID);

VOID RTMPDeletePMKIDCache(
	IN NDIS_AP_802_11_PMKID *pPMKIDCache,
	IN INT apidx,
	IN INT idx);

VOID RTMPMaintainPMKIDCache(
	IN RTMP_ADAPTER *pAd);

UCHAR is_rsne_pmkid_cache_match(
	IN UINT8 *rsnie,
	IN UINT	rsnie_len,
	IN NDIS_AP_802_11_PMKID * pmkid_cache,
	IN INT apidx,
	IN UCHAR *addr,
	OUT INT *cacheidx);
#else
#define RTMPMaintainPMKIDCache(_pAd)
#endif /* DOT1X_SUPPORT */

#ifdef RESOURCE_PRE_ALLOC
VOID RTMPResetTxRxRingMemory(
	IN  RTMP_ADAPTER   *pAd);
#endif /* RESOURCE_PRE_ALLOC */

VOID *alloc_rx_buf_1k(void *hif_resource);
VOID free_rx_buf_1k(void *hif_resource);
VOID *alloc_rx_buf_64k(void *hif_resource);
VOID free_rx_buf_64k(void *hif_resource);
VOID free_rx_buf(void *hdev_ctrl, UCHAR hif_idx);
VOID RTMPFreeTxRxRingMemory(RTMP_ADAPTER *pAd);
BOOLEAN fill_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
UINT16 tx_pkt_classification(RTMP_ADAPTER *pAd, PNDIS_PACKET  pPacket, TX_BLK *pTxBlk);
INT send_data_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);
INT send_mlme_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, struct wifi_dev *wdev, UCHAR q_idx, BOOLEAN is_data_queue);
INT32 fp_send_data_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);

VOID announce_802_3_packet_cmm(
	IN VOID *pAdSrc,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET	pPacket);

UINT announce_amsdu_pkt(
	RTMP_ADAPTER *pAd,
	PNDIS_PACKET pPacket,
	UCHAR OpMode);

PNET_DEV get_netdev_from_bssid(RTMP_ADAPTER *pAd, UCHAR FromWhichBSSID);

#ifdef DOT11_N_SUPPORT
void ba_flush_reordering_timeout_mpdus(
	IN RTMP_ADAPTER *pAd,
	IN PBA_REC_ENTRY	pBAEntry,
	IN ULONG			Now32);

void ba_timeout_flush(RTMP_ADAPTER *pAd);
void ba_timeout_monitor(RTMP_ADAPTER *pAd);

VOID ba_ori_session_setup(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY	*pEntry,
	UCHAR TID,
	USHORT TimeOut,
	ULONG DelayTime,
	BOOLEAN isForced);

BOOLEAN ba_rec_session_add(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry,
	FRAME_ADDBA_REQ *pFrame);

VOID ba_session_tear_down_all(
	RTMP_ADAPTER *pAd,
	UCHAR Wcid);

VOID ba_ori_session_tear_down(
	RTMP_ADAPTER *pAd,
	UCHAR Wcid,
	UCHAR TID,
	BOOLEAN bPassive,
	BOOLEAN	bForceSend);

VOID ba_resource_dump_all(RTMP_ADAPTER *pAd);
VOID ba_reordering_resource_dump_all(RTMP_ADAPTER *pAd);
VOID ba_reodering_resource_dump(RTMP_ADAPTER *pAd, UCHAR wcid);
VOID ba_rec_session_tear_down(RTMP_ADAPTER *pAd, UCHAR Wcid,
				UCHAR TID, BOOLEAN bPassive);
#endif /* DOT11_N_SUPPORT */

BOOLEAN ba_reordering_resource_init(RTMP_ADAPTER *pAd, int num);
void ba_reordering_resource_release(RTMP_ADAPTER *pAd);
struct reordering_mpdu *ba_reordering_mpdu_probe(struct reordering_list *list);

INT ComputeChecksum(
	IN UINT PIN);

UINT GenerateWpsPinCode(
	IN	RTMP_ADAPTER *pAd,
	IN  BOOLEAN		 bFromApcli,
	IN	UCHAR	apidx);

#ifdef WSC_INCLUDED
INT	Set_WscGenPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef BB_SOC
INT	Set_WscResetPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT Set_WscVendorPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef WSC_AP_SUPPORT
VOID RTMPIoctlSetWSCOOB(IN RTMP_ADAPTER *pAd);
#endif

/* */
/* prototype in wsc.c */
/* */
BOOLEAN	WscMsgTypeSubst(
	IN	UCHAR	EAPType,
	IN	UCHAR	EAPCode,
	OUT	INT	    *MsgType);

VOID	WscStateMachineInit(
	IN	RTMP_ADAPTER *pAd,
	IN	STATE_MACHINE		*S,
	OUT STATE_MACHINE_FUNC Trans[]);


VOID	WscEAPOLStartAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM  *Elem);

VOID	WscEAPAction(
	IN	RTMP_ADAPTER *pAd,
	IN	MLME_QUEUE_ELEM *Elem);

VOID	WscEapEnrolleeAction(
	IN	RTMP_ADAPTER *pAd,
	IN	MLME_QUEUE_ELEM	*Elem,
	IN  UCHAR			MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL	   pWscControl);

#ifdef CONFIG_AP_SUPPORT
VOID	WscEapApProxyAction(
	IN	RTMP_ADAPTER *pAd,
	IN	MLME_QUEUE_ELEM	*Elem,
	IN  UCHAR			MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL	   pWscControl);
#endif /* CONFIG_AP_SUPPORT */

VOID	WscEapRegistrarAction(
	IN	RTMP_ADAPTER *pAd,
	IN	MLME_QUEUE_ELEM	*Elem,
	IN  UCHAR			MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL	   pWscControl);

VOID	WscEAPOLTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

VOID	Wsc2MinsTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

UCHAR	WscRxMsgType(
	IN	RTMP_ADAPTER *pAd,
	IN	PMLME_QUEUE_ELEM	pElem);

VOID	WscInitRegistrarPair(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	IN  UCHAR				apidx);

VOID	WscSendEapReqId(
	IN	RTMP_ADAPTER *pAd,
	IN	PMAC_TABLE_ENTRY	pEntry,
	IN  UCHAR				CurOpMode);

VOID	WscSendEapolStart(
	IN	RTMP_ADAPTER *pAd,
	IN  PUCHAR		pBssid,
	IN  UCHAR		 CurOpMode,
	IN  VOID		  *wdev_obj);

VOID	WscSendEapRspId(
	IN	RTMP_ADAPTER *pAd,
	IN  PMAC_TABLE_ENTRY	pEntry,
	IN  PWSC_CTRL		   pWscControl);

VOID	WscMacHeaderInit(
	IN	RTMP_ADAPTER *pAd,
	IN OUT	PHEADER_802_11	Hdr,
	IN	PUCHAR			pAddr1,
	IN  PUCHAR		  pBSSID,
	IN  BOOLEAN		 bFromApCli);

VOID	WscSendMessage(
	IN	RTMP_ADAPTER *pAd,
	IN  UCHAR			   OpCode,
	IN  PUCHAR				pData,
	IN  INT					Len,
	IN  PWSC_CTRL		   pWscControl,
	IN  UCHAR			   OpMode,	/* 0: AP Mode, 1: AP Client Mode, 2: STA Mode */
	IN  UCHAR			   EapType);

VOID	WscSendEapReqAck(
	IN	RTMP_ADAPTER *pAd,
	IN	PMAC_TABLE_ENTRY	pEntry);

VOID	WscSendEapReqDone(
	IN	RTMP_ADAPTER *pAd,
	IN	PMLME_QUEUE_ELEM	pElem);

VOID	WscSendEapFail(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	IN  BOOLEAN				bSendDeAuth);

VOID WscM2DTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

VOID WscUPnPMsgTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

int WscSendUPnPConfReqMsg(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR apIdx,
	IN PUCHAR ssidStr,
	IN PUCHAR macAddr,
	IN INT	  Status,
	IN UINT   eventID,
	IN UCHAR  CurOpMode);

int WscSendUPnPMessage(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR				apIdx,
	IN	USHORT				msgType,
	IN	USHORT				msgSubType,
	IN	PUCHAR				pData,
	IN	INT					dataLen,
	IN	UINT				eventID,
	IN	UINT				toIPAddr,
	IN	PUCHAR				pMACAddr,
	IN  UCHAR				CurOpMode);

VOID WscUPnPErrHandle(
	IN RTMP_ADAPTER *pAd,
	IN PWSC_CTRL		pWscControl,
	IN UINT			eventID);

VOID	WscBuildBeaconIE(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR b_configured,
	IN	BOOLEAN b_selRegistrar,
	IN	USHORT devPwdId,
	IN	USHORT selRegCfgMethods,
	IN  UCHAR apidx,
	IN  UCHAR *pAuthorizedMACs,
	IN  UCHAR	AuthorizedMACsLen,
	IN  UCHAR	CurOpMode);

VOID	WscBuildProbeRespIE(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR respType,
	IN	UCHAR scState,
	IN	BOOLEAN b_selRegistrar,
	IN	USHORT devPwdId,
	IN	USHORT selRegCfgMethods,
	IN  UCHAR apidx,
	IN  UCHAR *pAuthorizedMACs,
	IN  INT   AuthorizedMACsLen,
	IN  UCHAR	CurOpMode);


#ifdef CONFIG_AP_SUPPORT
VOID WscBuildAssocRespIE(
	IN	RTMP_ADAPTER *pAd,
	IN  UCHAR			ApIdx,
	IN  UCHAR			Reason,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pIeLen);

VOID	WscSelectedRegistrar(
	IN	RTMP_ADAPTER *pAd,
	IN	PUCHAR	RegInfo,
	IN	UINT	length,
	IN  UCHAR	apidx);

VOID	WscInformFromWPA(
	IN  PMAC_TABLE_ENTRY	pEntry);

#ifdef WSC_AP_SUPPORT
#ifdef APCLI_SUPPORT
VOID  WscApCliLinkDownById(
	IN	PRTMP_ADAPTER	pAd,
	IN  UCHAR	apidx);
#endif /* APCLI_SUPPORT */
#endif /* WSC_AP_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */

VOID WscBuildProbeReqIE(
	IN	RTMP_ADAPTER *pAd,
	IN  VOID		  *wdev_obj,
	OUT	PUCHAR		pOutBuf,
	OUT	PUCHAR		pIeLen);

VOID WscBuildAssocReqIE(
	IN  PWSC_CTRL		pWscControl,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pIeLen);

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

VOID	WscProfileRetryTimeout(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

VOID	WscPBCTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);
#ifdef CON_WPS
VOID	WscScanDoneCheckTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);
#endif /*CON_WPS*/

#ifdef WSC_STA_SUPPORT
VOID	WscPINTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);
#endif

VOID	WscScanTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

INT WscGenerateUUID(
	RTMP_ADAPTER	*pAd,
	UCHAR			*uuidHexStr,
	UCHAR			*uuidAscStr,
	int			apIdx,
	BOOLEAN			bUseCurrentTime,
	BOOLEAN			from_apcli);

VOID WscStop(
	IN	RTMP_ADAPTER *pAd,
#ifdef CONFIG_AP_SUPPORT
	IN  BOOLEAN		 bFromApcli,
#endif /* CONFIG_AP_SUPPORT */
	IN  PWSC_CTRL	   pWscControl);

VOID WscInit(
	IN	RTMP_ADAPTER *pAd,
	IN  BOOLEAN		 bFromApcli,
	IN  UCHAR			BssIndex);
#ifdef CON_WPS
VOID WscConWpsStop(
	IN  PRTMP_ADAPTER   pAd,
	IN  BOOLEAN		 bFromApCli,
	IN  PWSC_CTRL	   pWscControl);
#endif /* CON_WPS */

BOOLEAN	ValidateChecksum(UINT PIN);

UINT WscRandomGen4digitPinCode(RTMP_ADAPTER *pAd);

UINT WscRandomGeneratePinCode(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR	apidx);

int BuildMessageM1(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM2(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM2D(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM3(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM4(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM5(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM6(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM7(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM8(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageDONE(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageACK(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageNACK(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int ProcessMessageM1(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM2(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	IN  UCHAR			apidx,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM2D(
	IN	RTMP_ADAPTER *pAd,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM3(
	IN	RTMP_ADAPTER *pAd,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM4(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM5(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM6(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM7(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM8(
	IN	RTMP_ADAPTER *pAd,
	IN	VOID *precv,
	IN	INT Length,
	IN  PWSC_CTRL	   pWscControl);

USHORT  WscGetAuthType(
	IN  UINT32 authType);

USHORT  WscGetEncryType(
	IN  UINT32 encryType);

NDIS_STATUS WscThreadInit(RTMP_ADAPTER *pAd);

BOOLEAN WscThreadExit(RTMP_ADAPTER *pAd);

int	 AppendWSCTLV(
	IN  USHORT index,
	OUT UCHAR *obuf,
	IN  UCHAR *ibuf,
	IN  USHORT varlen);

VOID	WscGetRegDataPIN(
	IN  RTMP_ADAPTER *pAd,
	IN  UINT			PinCode,
	IN  PWSC_CTRL	   pWscControl);

VOID	WscPushPBCAction(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		pWscControl);

#ifdef WSC_STA_SUPPORT
VOID	WscPINAction(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		pWscControl);

BOOLEAN WscPINExec(
	IN	RTMP_ADAPTER *pAd,
	IN  BOOLEAN			bFromM2,
	IN  PWSC_CTRL	   pWscControl);

VOID	WscPINBssTableSort(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL	   pWscControl);
#endif

VOID	WscScanExec(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		pWscControl);

BOOLEAN WscPBCExec(
	IN	RTMP_ADAPTER *pAd,
	IN  BOOLEAN			bFromM2,
	IN  PWSC_CTRL	   pWscControl);

VOID	WscPBCBssTableSort(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL	   pWscControl);

VOID	WscGenRandomKey(
	IN	RTMP_ADAPTER *pAd,
	IN	PWSC_CTRL	   pWscControl,
	INOUT	PUCHAR			pKey,
	INOUT	PUSHORT			pKeyLen);

VOID	WscCreateProfileFromCfg(
	IN	RTMP_ADAPTER *pAd,
	IN  UCHAR			   OpMode,		 /* 0: AP Mode, 1: AP Client Mode, 2: STA Mode */
	IN  PWSC_CTRL		   pWscControl,
	OUT PWSC_PROFILE		pWscProfile);

void	WscWriteConfToPortCfg(
	IN  RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL	   pWscControl,
	IN  PWSC_CREDENTIAL pCredential,
	IN  BOOLEAN		 bEnrollee);

#ifdef APCLI_SUPPORT
void	WscWriteConfToApCliCfg(
	IN  RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL	   pWscControl,
	IN  PWSC_CREDENTIAL pCredential,
	IN  BOOLEAN		 bEnrollee);
#endif /* APCLI_SUPPORT */

VOID   WpsSmProcess(
	IN PRTMP_ADAPTER		pAd,
	IN MLME_QUEUE_ELEM	   *Elem);

VOID WscPBCSessionOverlapCheck(
	IN	RTMP_ADAPTER *pAd);

VOID WscPBC_DPID_FromSTA(
	IN	RTMP_ADAPTER *pAd,
	IN	PUCHAR				pMacAddr);

#ifdef CONFIG_AP_SUPPORT
INT	WscGetConfWithoutTrigger(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL	   pWscControl,
	IN  BOOLEAN		 bFromUPnP);

BOOLEAN	WscReadProfileFromUfdFile(
	IN	RTMP_ADAPTER *pAd,
	IN  UCHAR			   ApIdx,
	IN  RTMP_STRING *pUfdFileName);

BOOLEAN	WscWriteProfileToUfdFile(
	IN	RTMP_ADAPTER *pAd,
	IN  UCHAR			   ApIdx,
	IN  RTMP_STRING *pUfdFileName);
#endif /* CONFIG_AP_SUPPORT */

VOID WscCheckWpsIeFromWpsAP(
	IN  RTMP_ADAPTER *pAd,
	IN  PEID_STRUCT		pEid,
	OUT PUSHORT			pDPIDFromAP);

#ifdef CONFIG_STA_SUPPORT
VOID PeerProbeRespAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

ULONG WscSearchWpsApBySSID(
	IN  RTMP_ADAPTER *pAd,
	IN PUCHAR			pSsid,
	IN UCHAR			SsidLen,
	IN INT				WscMode,
	IN VOID			 *wdev_obj);
#endif /* CONFIG_STA_SUPPORT */

/* WSC hardware push button function 0811 */
VOID WSC_HDR_BTN_Init(RTMP_ADAPTER *pAd);
VOID WSC_HDR_BTN_Stop(RTMP_ADAPTER *pAd);
VOID WSC_HDR_BTN_CheckHandler(RTMP_ADAPTER *pAd);
#ifdef WSC_LED_SUPPORT
BOOLEAN WscSupportWPSLEDMode(RTMP_ADAPTER *pAd);
BOOLEAN WscSupportWPSLEDMode10(RTMP_ADAPTER *pAd);

BOOLEAN WscAPHasSecuritySetting(RTMP_ADAPTER *pAd, PWSC_CTRL pWscControl);

VOID WscLEDTimer(
	IN PVOID	SystemSpecific1,
	IN PVOID	FunctionContext,
	IN PVOID	SystemSpecific2,
	IN PVOID	SystemSpecific3);

VOID WscSkipTurnOffLEDTimer(
	IN PVOID	SystemSpecific1,
	IN PVOID	FunctionContext,
	IN PVOID	SystemSpecific2,
	IN PVOID	SystemSpecific3);
#endif /* WSC_LED_SUPPORT */



#ifdef CONFIG_AP_SUPPORT
VOID WscUpdatePortCfgTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif /* CONFIG_AP_SUPPORT */

VOID WscCheckPeerDPID(
	IN  RTMP_ADAPTER *pAd,
	IN  PFRAME_802_11	Fr,
	IN  PUCHAR			eid_data,
	IN  INT				eid_len);

VOID WscClearPeerList(PLIST_HEADER pWscEnList);

PWSC_PEER_ENTRY WscFindPeerEntry(PLIST_HEADER	pWscEnList, UCHAR *pMacAddr);
VOID WscDelListEntryByMAC(PLIST_HEADER pWscEnList, UCHAR *pMacAddr);
VOID WscInsertPeerEntryByMAC(PLIST_HEADER	pWscEnList, UCHAR *pMacAddr);

#ifdef CONFIG_AP_SUPPORT
INT WscApShowPeerList(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT WscApShowPin(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
INT WscStaShowPeerList(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CONFIG_STA_SUPPORT */

VOID WscMaintainPeerList(RTMP_ADAPTER *pAd, PWSC_CTRL pWpsCtrl);

VOID WscAssignEntryMAC(RTMP_ADAPTER *pAd, PWSC_CTRL pWpsCtrl);

#ifdef WSC_V2_SUPPORT
#ifdef CONFIG_AP_SUPPORT
VOID WscOnOff(RTMP_ADAPTER *pAd, INT ApIdx, BOOLEAN bOff);

VOID WscAddEntryToAclList(RTMP_ADAPTER *pAd, INT ApIdx, UCHAR *pMacAddr);

VOID WscSetupLockTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID WscCheckPinAttackCount(RTMP_ADAPTER *pAd, PWSC_CTRL pWscControl);
#endif /* CONFIG_AP_SUPPORT */

BOOLEAN	WscGenV2Msg(
	IN  PWSC_CTRL		pWpsCtrl,
	IN  BOOLEAN			bSelRegistrar,
	IN	PUCHAR			pAuthorizedMACs,
	IN  INT				AuthorizedMACsLen,
	OUT	UCHAR			**pOutBuf,
	OUT	INT				*pOutBufLen);

BOOLEAN	WscParseV2SubItem(
	IN	UCHAR			SubID,
	IN	PUCHAR			pData,
	IN	USHORT			DataLen,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pOutBufLen);

VOID	WscSendEapFragAck(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL			pWscControl,
	IN	PMAC_TABLE_ENTRY	pEntry);

VOID	WscSendEapFragData(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL			pWscControl,
	IN	PMAC_TABLE_ENTRY	pEntry);
#endif /* WSC_V2_SUPPORT */

BOOLEAN WscGetDataFromPeerByTag(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR			pIeData,
	IN  INT				IeDataLen,
	IN  USHORT			WscTag,
	OUT PUCHAR			pWscBuf,
	OUT PUSHORT			pWscBufLen);

VOID WscUUIDInit(
	IN  PRTMP_ADAPTER pAd,
	IN  INT inf_idx,
	IN  UCHAR from_apcli);

#ifdef CONFIG_AP_SUPPORT
INT Set_AP_WscMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_AP_WscGetConf_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */

INT32 ShowRFInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowBBPInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 show_redirect_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowWifiInterruptCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);


BOOLEAN rtstrmactohex(RTMP_STRING *s1, RTMP_STRING *s2);
BOOLEAN rtstrcasecmp(RTMP_STRING *s1, RTMP_STRING *s2);
RTMP_STRING *rtstrstruncasecmp(RTMP_STRING *s1, RTMP_STRING *s2);

RTMP_STRING *rtstrstr(const RTMP_STRING *s1, const RTMP_STRING *s2);
RTMP_STRING *rstrtok(RTMP_STRING *s, const RTMP_STRING *ct);
int rtinet_aton(const RTMP_STRING *cp, unsigned int *addr);

/*//////// common ioctl functions ////////*/
INT show_driverinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_CountryRegion_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_CountryRegionABand_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_WirelessMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef RT_CFG80211_SUPPORT
INT Set_DisableCfg2040Scan_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT Set_MBSS_WirelessMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Probe_Rsp_Times_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Channel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT rtmp_set_channel(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel);
#ifdef CONFIG_MT7663_DPD_RE_CAL_SUPPORT
INT Set_DPDReCal_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT MT7663DPDRecal(RTMP_ADAPTER *pAd, UCHAR Channel);
#endif

INT	Set_ShortSlot_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MaxTxPwr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_TxPower_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_BGProtection_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxPreamble_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_RTSThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_FragThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBurst_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef RTMP_MAC_PCI
INT Set_ShowRF_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* RTMP_MAC_PCI */

#ifdef AGGREGATION_SUPPORT
INT	Set_PktAggregate_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* AGGREGATION_SUPPORT */

INT	Set_IEEE80211H_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_ExtCountryCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ExtDfsType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ChannelListAdd_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ChannelListShow_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ChannelListDel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef DBG
INT	Set_Debug_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_DebugCategory_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID RTMPIoctlMAC(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif
#if defined(BB_SOC) && defined(TCSUPPORT_WLAN_SW_RPS)
INT	Set_RxMaxTraffic_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_rx_detect_flag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#ifdef RATE_PRIOR_SUPPORT
INT Set_RatePrior_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_BlackListTimeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_LowRateRatio_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_LowRateCountPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TotalCntThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif/*RATE_PRIOR_SUPPORT*/

#ifdef RANDOM_PKT_GEN
INT Set_TxCtrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID regular_pause_umac(RTMP_ADAPTER *pAd);
#endif

#ifdef CSO_TEST_SUPPORT
INT Set_CsCtrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

#ifdef MT_MAC
INT Show_TxVinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

#ifdef THERMAL_PROTECT_SUPPORT
INT set_thermal_protection_criteria_proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);

INT set_thermal_protection_admin_ctrl_duty_proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);

INT get_thermal_protection_admin_ctrl_duty_proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);
#endif /* THERMAL_PROTECT_SUPPORT */
#ifdef TXBF_SUPPORT
INT	Set_InvTxBfTag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_ITxBfDivCal_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_ETxBfEnCond_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ETxBfCodebook_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ETxBfCoefficient_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ETxBfGrouping_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ETxBfNoncompress_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ETxBfIncapable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_Trigger_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_Stop_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_ITxBfEn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_StaITxBfEnCond_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef MT_MAC
INT Set_TxBfProfileTag_Help(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_PfmuIdx(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_BfType(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_DBW(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_SuMu(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_InValid(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_Mem(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_Matrix(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_SNR(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_SmartAnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_SeIdx(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_RmsdThrd(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_McsThrd(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_TimeOut(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_DesiredBW(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_DesiredNc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_DesiredNr(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTagRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTagWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileDataRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileDataWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileData20MAllWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfilePnRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfilePnWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfAidUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfQdRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfFbRptDbgInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txbf_stop_report_poll_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfTxApply(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_TxBfPfmuMemAlloc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_TxBfPfmuMemRelease(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_TxBfPfmuMemAllocMapRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_StaRecCmmUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_StaRecBfUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_StaRecBfRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfAwareCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_dynsnd_en_intr(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CFG_SUPPORT_MU_MIMO
INT set_dynsnd_cfg_dmcs(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_dynsnd_en_mu_intr(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT Set_BssInfoUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_DevInfoUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef TXBF_DYNAMIC_DISABLE
INT Set_TxBfDynamicDisable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* TXBF_DYNAMIC_DISABLE */
INT set_txbf_dynamic_mechanism_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */

#ifdef VHT_TXBF_SUPPORT
INT Set_VhtNDPA_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* VHT_TXBF_SUPPORT */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
INT WifiFwdSet(IN int disabled);
INT Set_WifiFwd_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_WifiFwd_Down(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
INT Set_WifiFwdAccessSchedule_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_WifiFwdHijack_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_WifiFwdBpdu_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_WifiFwdRepDevice(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_WifiFwdShowEntry(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_WifiFwdDeleteEntry(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_PacketSourceShowEntry(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_PacketSourceDeleteEntry(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_WifiFwdBridge_Proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);

#endif /* CONFIG_WIFI_PKT_FWD */

INT Set_RateAdaptInterval(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_DescInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_MacTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef ACL_BLK_COUNT_SUPPORT
INT Show_ACLRejectCount_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif/*ACL_BLK_COUNT_SUPPORT*/

#ifdef DOT11_N_SUPPORT
INT Show_BaTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_ChannelSet_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DOT11_N_SUPPORT */

#ifdef MT_MAC
INT Show_PSTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_wtbl_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_wtbltlv_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_mib_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_amsdu_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 show_wifi_sys(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef DBDC_MODE
INT32 ShowDbdcProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT32 ShowChCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef GREENAP_SUPPORT
INT32 ShowGreenAPProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT32 show_pcie_aspm_dym_ctrl_cap_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

INT32 show_tx_burst_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowTmacInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowAggInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowArbInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowManualTxOP(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_dmasch_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowPseInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowPseData(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT ShowPLEInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_TXD_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowTXDInfo(RTMP_ADAPTER *pAd, UINT fid);
INT show_mem_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_protect_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_cca_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef CUT_THROUGH
INT ShowCutThroughInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CUT_THROUGH */

#endif /* MT_MAC */
INT Show_sta_tr_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_stainfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_devinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_sysinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_trinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CONFIG_TP_DBG
INT Set_TPDbg_Level(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_TPDbg_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CONFIG_TP_DBG */
INT show_tpinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txqinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_swqinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_efuseinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#if defined(MT_MAC) && defined(TXBF_SUPPORT)
INT Show_AteIbfPhaseCalStatus(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT	Set_ResetStatCounter_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	SetCommonHtVht(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

INT	Set_AP_SlotTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef DOT11_N_SUPPORT
INT	Set_BASetup_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_BADecline_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_BAOriTearDown_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_BARecTearDown_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtBw_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtMcs_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtGi_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtOpMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtLdpc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtStbc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtExtcha_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	set_extcha_for_wdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR value);
INT	Set_HtMpduDensity_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtBaWinSize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtRdg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtLinkAdapt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtAmsdu_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtAutoBa_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtProtect_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtMimoPs_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef EEPROM_RETRIEVE_SUPPORT
INT32 show_e2p_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* EEPROM_RETRIEVE_SUPPORT */
#ifdef DOT11N_DRAFT3
INT Set_HT_BssCoex_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_HT_BssCoexApCntThr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DOT11N_DRAFT3 */

#ifdef CONFIG_AP_SUPPORT
INT	Set_HtTxStream_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtRxStream_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
INT	Set_GreenAP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT	set_pcie_aspm_dym_ctrl_cap_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

INT	SetCommonHT(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

INT	Set_ForceShortGI_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_ForceGF_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_SendSMPSAction_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

void convert_reordering_packet_to_preAMSDU_or_802_3_packet(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx);

INT	Set_HtMIMOPSmode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtTxBASize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtDisallowTKIP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_BurstMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
INT Set_VhtBw_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_VhtLdpc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_VhtStbc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_VhtBwSignal_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_VhtDisallowNonVHT_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DOT11_VHT_AC */

#ifdef APCLI_SUPPORT
INT RTMPIoctlConnStatus(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /*APCLI_SUPPORT*/



#ifdef MEM_ALLOC_INFO_SUPPORT
INT Show_MemInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_PktInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MEM_ALLOC_INFO_SUPPORT */

RTMP_STRING *GetAuthModeStr(
	IN UINT32 authMode);

RTMP_STRING *GetEncryModeStr(
	IN UINT32 encryMode);

UINT32 SecAuthModeOldToNew(
	IN USHORT authMode);

UINT32 SecEncryModeOldToNew(
	IN USHORT encryMode);

USHORT SecAuthModeNewToOld(
	IN UINT32 authMode);

USHORT SecEncryModeNewToOld(
	IN UINT32 encryMode);

#ifdef CONFIG_STA_SUPPORT
VOID RTMPSendDLSTearDownFrame(RTMP_ADAPTER *pAd, UCHAR *pDA, struct wifi_dev *wdev);
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

VOID detect_wmm_traffic(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR up, UCHAR bOutput);
#ifdef CONFIG_AP_SUPPORT
VOID dynamic_tune_be_tx_op(RTMP_ADAPTER *pAd, ULONG nonBEpackets);
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
VOID Handle_BSS_Width_Trigger_Events(RTMP_ADAPTER *pAd, UCHAR Channel);

void build_ext_channel_switch_ie(
	IN RTMP_ADAPTER *pAd,
	IN HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE * pIE,
	IN UCHAR Channel,
	IN UCHAR PhyMode,
	IN struct wifi_dev *wdev);

void assoc_ht_info_debugshow(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN UCHAR ht_cap_len,
	IN HT_CAPABILITY_IE *pHTCapability);
#endif /* DOT11_N_SUPPORT */

INT header_packet_process(
	RTMP_ADAPTER *pAd,
	PNDIS_PACKET pRxPacket,
	RX_BLK *pRxBlk);

NDIS_STATUS rx_packet_process(
	RTMP_ADAPTER *pAd,
	PNDIS_PACKET pRxPacket,
	RX_BLK *pRxBlk);

VOID ap_ieee802_3_data_rx(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
VOID sta_ieee802_3_data_rx(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

BOOLEAN rtmp_rx_done_handle(RTMP_ADAPTER *pAd);
BOOLEAN mtd_rx_done_handle(RTMP_ADAPTER *pAd);
#ifdef MT7626_E2_SUPPORT
BOOLEAN mtd_rx2_done_handle(RTMP_ADAPTER *pAd);
#endif
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
#ifdef CONFIG_ANDES_SUPPORT
BOOLEAN RxRing1DoneInterruptHandle(RTMP_ADAPTER *pAd);
VOID RTMPHandleTxRing8DmaDoneInterrupt(RTMP_ADAPTER *pAd);
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || defined(AXE) || defined(MT7626)
BOOLEAN RTMPHandleFwDwloCmdRingDmaDoneInterrupt(RTMP_ADAPTER *pAd);
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
VOID RTMPHandleBcnDmaDoneInterrupt(RTMP_ADAPTER *pAd);
#endif /* MT_MAC */
#endif /* RTMP_PCI_SUPPORT */

VOID indicate_rx_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx);
VOID indicate_ampdu_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx);
VOID indicate_amsdu_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx);
VOID ba_reorder(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx);
VOID ba_reorder_buf_maintain(RTMP_ADAPTER *pAd);

VOID indicate_802_3_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx);
VOID indicate_802_11_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx);
VOID indicate_eapol_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx);

UINT deaggregate_amsdu_announce(
	IN	RTMP_ADAPTER *pAd,
	PNDIS_PACKET		pPacket,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			OpMode);

/* remove LLC and get 802_3 Header */
#define  RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(_pRxBlk, _pHeader802_3)	\
	{																			\
		PUCHAR _pRemovedLLCSNAP = NULL, _pDA, _pSA;								\
		\
		if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_WDS) || RX_BLK_TEST_FLAG(_pRxBlk, fRX_MESH)) {	\
			_pDA = _pRxBlk->Addr3;															\
			_pSA = _pRxBlk->Addr4;															\
		} else if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_AP)) {								\
			_pDA = _pRxBlk->Addr1;													\
			if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_DLS))									\
				_pSA = _pRxBlk->Addr2;												\
			else																	\
				_pSA = _pRxBlk->Addr3;		\
		} else if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_STA)) {							\
			_pDA = _pRxBlk->Addr3;									 \
			_pSA = _pRxBlk->Addr2;									 \
		} else if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_ADHOC)) {							  \
			_pDA = _pRxBlk->Addr1;									 \
			_pSA = _pRxBlk->Addr2;									 \
		} else {														\
			/* TODO: shiang-usw, where shall we go here?? */			\
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Un-assigned Peer's Role!\n", __func__));\
			_pDA = _pRxBlk->Addr3;										\
			_pSA = _pRxBlk->Addr2;										\
		}																\
		\
		CONVERT_TO_802_3(_pHeader802_3, _pDA, _pSA, _pRxBlk->pData,		\
						 _pRxBlk->DataSize, _pRemovedLLCSNAP);								  \
	}

VOID announce_or_forward_802_3_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket,
								   struct wifi_dev *wdev, UCHAR op_mode);

VOID indicate_agg_ralink_pkt(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx);

VOID Update_Rssi_Sample(
	IN RTMP_ADAPTER *pAd,
	IN RSSI_SAMPLE *pRssi,
	IN struct rx_signal_info *signal,
	IN UCHAR phymode,
	IN UCHAR bw);

UINT32 mtd_pci_get_resource_idx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum PACKET_TYPE, UCHAR q_idx);
NDIS_STATUS mtd_pci_build_resource_idx(RTMP_ADAPTER *pAd);

PNDIS_PACKET mtd_pci_get_pkt_from_rx_resource(
	RTMP_ADAPTER *pAd,
	BOOLEAN	*pbReschedule,
	UINT32 *pRxPending,
	UCHAR RxRingNo);

#ifdef MT7626_E2_SUPPORT
PNDIS_PACKET mtd_pci_get_pkt_from_rx2_resource(
	RTMP_ADAPTER *pAd,
	BOOLEAN	*pbReschedule,
	UINT32 *pRxPending,
	UCHAR RxRingNo);
#endif

PNDIS_PACKET mtd_pci_get_pkt_from_rx_resource_io(
	RTMP_ADAPTER *pAd,
	BOOLEAN	*pbReschedule,
	UINT32 *pRxPending,
	UCHAR RxRingNo);

PNDIS_PACKET pci_get_pkt_from_rx_resource(
	RTMP_ADAPTER *pAd,
	BOOLEAN	*pbReschedule,
	UINT32 *pRxPending,
	UCHAR RxRingNo);

PNDIS_PACKET usb_get_pkt_from_rx_resource(
	RTMP_ADAPTER *pAd,
	BOOLEAN	*pbReschedule,
	UINT32 *pRxPending,
	UCHAR RxRingNo);

PNDIS_PACKET sdio_get_pkt_from_rx_resource(
	RTMP_ADAPTER *pAd,
	BOOLEAN	*pbReschedule,
	UINT32 *pRxPending,
	UCHAR RxRingNo);

UCHAR *mt_pci_get_hif_buf(RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk, UCHAR hif_idx, UCHAR frame_type);

VOID de_fragment_data_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
VOID RTMPIoctlGetSiteSurvey(
	IN	RTMP_ADAPTER *pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
INT Set_ApCli_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_Ssid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_PwrSet_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_SendPsPoll_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef APCLI_CFG80211_SUPPORT
INT Set_ApCli_EncrypType(RTMP_ADAPTER *pAd, INT staidx, RTMP_STRING *arg);
INT Set_ApCli_AuthMode(RTMP_ADAPTER *pAd, INT staidx, RTMP_STRING *arg);
INT Set_ApCli_Enable(RTMP_ADAPTER *pAd, INT staidx, RTMP_STRING *arg);
INT Set_ApCli_Ssid(RTMP_ADAPTER *pAd, INT staidx, RTMP_STRING *arg);

#endif /* APCLI_CFG80211_SUPPORT */

#ifdef ROAMING_ENHANCE_SUPPORT
BOOLEAN IsApCliLinkUp(IN PRTMP_ADAPTER pAd);
BOOLEAN ApCliDoRoamingRefresh(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PNDIS_PACKET pRxPacket,
	IN struct wifi_dev *wdev);
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef MAT_SUPPORT

VOID getIPv6MacTbInfo(MAT_STRUCT *, char *, ULONG);
VOID getIPMacTbInfo(MAT_STRUCT *pMatCfg, char *pOutBuf, ULONG BufLen);

NDIS_STATUS MATEngineInit(RTMP_ADAPTER *pAd);
NDIS_STATUS MATEngineExit(RTMP_ADAPTER *pAd);

PUCHAR MATEngineRxHandle(RTMP_ADAPTER *pAd, PNDIS_PACKET pPkt, UINT infIdx);
PUCHAR MATEngineTxHandle(RTMP_ADAPTER *pAd, PNDIS_PACKET pPkt, UINT infIdx, UCHAR OpMode);

BOOLEAN MATPktRxNeedConvert(RTMP_ADAPTER *pAd, PNET_DEV net_dev);

#endif /* MAT_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
typedef struct CountryCodeToCountryRegion {
	USHORT		CountryNum;
	UCHAR		IsoName[3];
	/*UCHAR		CountryName[40]; */
	RTMP_STRING *pCountryName;
	BOOLEAN		SupportABand;
	/*ULONG		RegDomainNum11A; */
	UCHAR		RegDomainNum11A;
	BOOLEAN	SupportGBand;
	/*ULONG		RegDomainNum11G; */
	UCHAR		RegDomainNum11G;
} COUNTRY_CODE_TO_COUNTRY_REGION;
#endif /* CONFIG_AP_SUPPORT */

#ifdef SNMP_SUPPORT
/*for snmp */
typedef struct _DefaultKeyIdxValue {
	UCHAR	KeyIdx;
	UCHAR	Value[16];
} DefaultKeyIdxValue, *PDefaultKeyIdxValue;
#endif

void STA_MonPktSend(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

#ifdef CONFIG_STA_SUPPORT
VOID RTMPSetDesiredRates(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, LONG Rates);
#endif /* CONFIG_STA_SUPPORT */

INT	Set_FixedTxMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef CONFIG_APSTA_MIXED_SUPPORT
INT	Set_OpMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

INT Set_LongRetryLimit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ShortRetryLimit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_AutoFallBack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID RT28XXDMADisable(RTMP_ADAPTER *pAd);
VOID RT28XXDMAEnable(RTMP_ADAPTER *pAd);
VOID RT28XXDMAReset(RTMP_ADAPTER *pAd);

#ifdef RTMP_MAC_PCI
VOID PDMAWatchDog(RTMP_ADAPTER *pAd);
VOID PDMAResetAndRecovery(RTMP_ADAPTER *pAd);
#endif

#ifdef MT_MAC
VOID DumpPseInfo(RTMP_ADAPTER *pAd);
VOID PSEWatchDog(RTMP_ADAPTER *pAd);
VOID PSEResetAndRecovery(RTMP_ADAPTER *pAd);
#endif

VOID MtUpdateBeaconToAsic(
	IN RTMP_ADAPTER *pAd,
	VOID *wdev_void,
	IN UINT16 FrameLen,
	IN UCHAR UpdatePktType);

void CfgInitHook(RTMP_ADAPTER *pAd);

NDIS_STATUS RtmpMgmtTaskInit(RTMP_ADAPTER *pAd);
VOID mtd_net_tasklet_exit(RTMP_ADAPTER *pAd);
VOID RtmpMgmtTaskExit(RTMP_ADAPTER *pAd);

void tbtt_tasklet(unsigned long data);

#ifdef MT_MAC
void mt_mac_int_0_tasklet(unsigned long data);
void mt_mac_int_1_tasklet(unsigned long data);
void mt_mac_int_2_tasklet(unsigned long data);
void mt_mac_int_3_tasklet(unsigned long data);
void mt_mac_int_4_tasklet(unsigned long data);
#endif /* MT_MAC */

#ifdef RTMP_MAC_PCI
BOOLEAN RT28xxPciAsicRadioOff(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR			Level,
	IN USHORT		   TbttNumToNextWakeUp);

BOOLEAN RT28xxPciAsicRadioOn(RTMP_ADAPTER *pAd, UCHAR Level);
VOID RTMPInitPCIeDevice(RT_CMD_PCIE_INIT *pConfig, VOID *pAd);

#ifdef CONFIG_STA_SUPPORT
VOID RT28xxPciStaAsicWakeup(
	RTMP_ADAPTER *pAd,
	BOOLEAN bFromTx,
	PSTA_ADMIN_CONFIG pStaCfg);

VOID RT28xxPciStaAsicSleepAutoWakeup(
	RTMP_ADAPTER *pAd,
	PSTA_ADMIN_CONFIG pStaCfg);

#endif /* CONFIG_STA_SUPPORT */

VOID PciMlmeRadioOn(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID PciMlmeRadioOFF(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

ra_dma_addr_t RtmpDrvPciMapSingle(
	IN RTMP_ADAPTER *pAd,
	IN VOID *ptr,
	IN size_t size,
	IN INT sd_idx,
	IN INT direction);

#endif /* RTMP_MAC_PCI */

INT irq_init(RTMP_ADAPTER *pAd);

#ifdef CONFIG_STA_SUPPORT
#ifdef CREDENTIAL_STORE
NDIS_STATUS RecoverConnectInfo(RTMP_ADAPTER *pAd);
NDIS_STATUS StoreConnectInfo(RTMP_ADAPTER *pAd);
#endif /* CREDENTIAL_STORE */
#endif /* CONFIG_STA_SUPPORT */

#ifdef NEW_WOW_SUPPORT
VOID RT28xxAndesWOWEnable(RTMP_ADAPTER *pAd);
VOID RT28xxAndesWOWDisable(RTMP_ADAPTER *pAd);
#endif /* NEW_WOW_SUPPORT */

#ifdef MT_WOW_SUPPORT
VOID MT76xxAndesWOWEnable(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg);
VOID MT76xxAndesWOWDisable(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg);
VOID MT76xxAndesWOWInit(RTMP_ADAPTER *pAd);
#endif

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
VOID RT28xxAsicWOWEnable(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg);
VOID RT28xxAsicWOWDisable(RTMP_ADAPTER *pAd);
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

/*////////////////////////////////////*/

/*///////////////////////////////////*/
INT RTMPShowCfgValue(RTMP_ADAPTER *pAd, RTMP_STRING *name, RTMP_STRING *buf, UINT32 MaxLen);
/*//////////////////////////////////*/

#ifdef CONFIG_STA_SUPPORT
VOID AsicStaBbpTuning(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg);

BOOLEAN StaUpdateMacTableEntry(
	IN  RTMP_ADAPTER *pAd,
	IN  PMAC_TABLE_ENTRY	pEntry,
	IN  UCHAR				MaxSupportedRateIn500Kbps,
	IN  HT_CAPABILITY_IE	*pHtCapability,
	IN  UCHAR				HtCapabilityLen,
	IN  ADD_HT_INFO_IE		*pAddHtInfo,
	IN  UCHAR				AddHtInfoLen,
	IN IE_LISTS *ie_list,
	IN  USHORT				CapabilityInfo);

#ifdef DOT11R_FT_SUPPORT
VOID FT_OTA_AuthTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID FT_OTD_TimeoutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif /* DOT11R_FT_SUPPORT */

BOOLEAN	AUTH_ReqSend(
	IN  PRTMP_ADAPTER		pAd,
	IN  PMLME_QUEUE_ELEM	pElem,
	IN  PRALINK_TIMER_STRUCT pAuthTimer,
	IN  RTMP_STRING *pSMName,
	IN  USHORT				SeqNo,
	IN  PUCHAR				pNewElement,
	IN  ULONG				ElementLen);
#endif /* CONFIG_STA_SUPPORT */

UINT VIRTUAL_IF_INC(RTMP_ADAPTER *pAd);
UINT VIRTUAL_IF_DEC(RTMP_ADAPTER *pAd);
UINT VIRTUAL_IF_NUM(RTMP_ADAPTER *pAd);



#ifdef SOFT_ENCRYPT
BOOLEAN RTMPExpandPacketForSwEncrypt(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk);
VOID RTMPUpdateSwCacheCipherInfo(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk, UCHAR *pHdr);
INT tx_sw_encrypt(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk, UCHAR *buf_ptr, HEADER_802_11 *wifi_hdr);
#endif /* SOFT_ENCRYPT */

/*
	OS Related funciton prototype definitions.
	TODO: Maybe we need to move these function prototypes to other proper place.
*/
VOID RTInitializeCmdQ(PCmdQ cmdq);

INT RTPCICmdThread(ULONG Context);

VOID CMDHandler(RTMP_ADAPTER *pAd);

VOID RTThreadDequeueCmd(PCmdQ cmdq, PCmdQElmt *pcmdqelmt);

NDIS_STATUS RTEnqueueInternalCmd(
	IN RTMP_ADAPTER *pAd,
	IN NDIS_OID			Oid,
	IN PVOID			pInformationBuffer,
	IN UINT32			InformationBufferLength);

void RtmpCmdQExit(RTMP_ADAPTER *pAd);
void RtmpCmdQInit(RTMP_ADAPTER *pAd);


BOOLEAN CHAN_PropertyCheck(RTMP_ADAPTER *pAd, UINT32 ChanNum, UCHAR Property);

#ifdef CONFIG_STA_SUPPORT

/* command */
INT Set_SSID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_WmmCapable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_APSDCapable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_APSDAC_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MaxSPLength_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_NetworkType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AuthMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_EncrypType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DefaultKeyID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Wep_Key_Proc(RTMP_ADAPTER  *pAd, RTMP_STRING *Key, INT KeyLen, INT KeyId);
INT Set_Key1_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Key2_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Key3_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Key4_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_WPAPSK_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_PSMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CFG_TDLS_SUPPORT
INT Set_CfgTdlsChannelSwitchRequest_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT	Set_CfgTdlsChannelSwitchBaseStayTime_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT	Set_CfgTdlsChannelSwitchBW_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT	Set_TdlsDisableChannelSwitchProc(PRTMP_ADAPTER	pAd, RTMP_STRING *arg);

#endif /* CFG_TDLS_SUPPORT */

#ifdef DBG
VOID RTMPIoctlE2PROM(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif /* DBG */

#ifdef WSC_STA_SUPPORT
VOID CntlWscIterate(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg);

USHORT WscGetAuthTypeFromStr(RTMP_STRING *arg);
USHORT WscGetEncrypTypeFromStr(RTMP_STRING *arg);

INT	Set_WscConfMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_WscConfStatus_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_WscSsid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_WscBssid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_WscMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_WscUUIDE_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_WscGetConf_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_WscStop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_WscPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef WSC_V2_SUPPORT
INT Set_WscForceSetAP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WSC_V2_SUPPORT */
#endif /* WSC_STA_SUPPORT */

NDIS_STATUS RTMPWPANoneAddKeyProc(RTMP_ADAPTER *pAd, VOID *pBuf);


#ifdef DOT11_N_SUPPORT
INT Set_TGnWifiTest_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DOT11_N_SUPPORT */

INT Set_LongRetryLimit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ShortRetryLimit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_Ieee80211dClientMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* EXT_BUILD_CHANNEL_LIST */

INT	Show_Adhoc_MacTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *extra, UINT32 size);

#ifdef RTMP_RF_RW_SUPPORT
VOID RTMPIoctlRF(
	IN	RTMP_ADAPTER *pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif /* RTMP_RF_RW_SUPPORT */


INT Set_BeaconLostTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_AutoRoaming_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_SiteSurvey_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ForceTxBurst_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID RTMPAddKey(RTMP_ADAPTER *pAd, PNDIS_802_11_KEY pKey, struct wifi_dev *wdev);
VOID StaSiteSurvey(
	IN	PRTMP_ADAPTER		pAd,
	IN	PNDIS_802_11_SSID	pSsid,
	IN	UCHAR				ScanType,
	IN	struct wifi_dev *wdev);

VOID bss_table_maintenance(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev,
	IN OUT	BSS_TABLE *Tab,
	IN  ULONG	MaxRxTimeDiff,
	IN  UCHAR	MaxSameRxTimeCount);
#endif /* CONFIG_STA_SUPPORT */
INT32 getLegacyOFDMMCSIndex(UINT8 MCS);
void  getRate(HTTRANSMIT_SETTING HTSetting, ULONG *fLastTxRxRate);
void  getRateEtc(HTTRANSMIT_SETTING HTSetting, ULONG *Rate, UINT8 *Antenna, UINT8 *Bandwidth, UINT8 *MCS, UINT16 *GuardNS);

#ifdef APCLI_SUPPORT

VOID ApCliRTMPSendNullFrame(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR			TxRate,
	IN BOOLEAN		bQosNull,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN	  USHORT		  PwrMgmt);

/*APPS DVT MSP*/
VOID AppsApCliRTMPSendNullFrame(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR			TxRate,
	IN BOOLEAN		bQosNull,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN      USHORT          PwrMgmt);

VOID ApCliRTMPSendPsPollFrame(
	IN	PRTMP_ADAPTER    pAd,
	IN	UINT_8  index);
#endif/*APCLI_SUPPORT*/

void RTMP_IndicateMediaState(
	IN	RTMP_ADAPTER *pAd,
	IN  NDIS_MEDIA_STATE	media_state);

INT RTMPSetInformation(
	IN RTMP_ADAPTER *pAd,
	IN OUT RTMP_IOCTL_INPUT_STRUCT *rq,
	IN INT cmd,
	IN struct wifi_dev *wdev);

INT RTMPQueryInformation(
	IN RTMP_ADAPTER *pAd,
	INOUT RTMP_IOCTL_INPUT_STRUCT *rq,
	IN INT cmd,
	IN struct wifi_dev *wdev);

VOID RTMPIoctlShow(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *rq,
	IN UINT32 subcmd,
	IN VOID *pData,
	IN ULONG Data);

INT RTMP_COM_IoctlHandle(
	IN VOID *pAdSrc,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq,
	IN INT cmd,
	IN USHORT subcmd,
	IN VOID *pData,
	IN ULONG Data);

#ifdef CONFIG_STA_SUPPORT
INT RTMP_STA_IoctlPrepare(RTMP_ADAPTER *pAd, VOID *pParm);
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
INT RTMP_AP_IoctlPrepare(RTMP_ADAPTER *pAd, VOID *pCB);
#endif /* CONFIG_AP_SUPPORT */
#ifdef SNIFFER_SUPPORT
INT Set_MonitorMode_Proc(RTMP_ADAPTER	*pAd, RTMP_STRING *arg);
INT Set_MonitorFilterSize_Proc(RTMP_ADAPTER	*pAd, RTMP_STRING *arg);
INT Set_MonitorFrameType_Proc(RTMP_ADAPTER	*pAd, RTMP_STRING *arg);
INT	Set_MonitorMacFilter_Proc(RTMP_ADAPTER	*pAd, RTMP_STRING *arg);
INT	Set_MonitorMacFilterOff_Proc(RTMP_ADAPTER	*pAd, RTMP_STRING *arg);

#endif /* SNIFFER_SUPPORT */

INT Set_VcoPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_RateAlg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef SINGLE_SKU
INT Set_ModuleTxpower_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* SINGLE_SKU */

VOID RtmpEnqueueNullFrame(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN UCHAR TxRate,
	IN UCHAR AID,
	IN UCHAR apidx,
	IN BOOLEAN bQosNull,
	IN BOOLEAN bEOSP,
	IN UCHAR OldUP);

VOID RtmpCleanupPsQueue(
	IN  RTMP_ADAPTER *pAd,
	IN  PQUEUE_HEADER   pQueue);

NDIS_STATUS RtmpInsertPsQueue(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN UCHAR QueIdx);

VOID RtmpHandleRxPsPoll(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT Aid, BOOLEAN isActive);
BOOLEAN RtmpPsIndicate(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR wcid, UCHAR Psm);

#ifdef MT_MAC
VOID MtHandleRxPsPoll(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT wcid, BOOLEAN isActive);
BOOLEAN MtPsIndicate(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR wcid, UCHAR Psm);
VOID MtPsRedirectDisableCheck(RTMP_ADAPTER *pAd, UCHAR wcid);
VOID MtPsSendToken(RTMP_ADAPTER *pAd, UINT32 WlanIdx);
VOID MtPsRecovery(RTMP_ADAPTER *pAd);
VOID MtSetIgnorePsm(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, UCHAR value);
VOID MtEnqTxSwqFromPsQueue(RTMP_ADAPTER *pAd, UCHAR qidx, STA_TR_ENTRY *tr_entry);
VOID CheckSkipTX(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);
NDIS_STATUS RtmpEnqueueTokenFrame(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN UCHAR TxRate,
	IN UCHAR AID,
	IN UCHAR apidx,
	IN UCHAR OldUP);
#endif /* MT_MAC */

#ifdef CONFIG_MULTI_CHANNEL
VOID RtmpEnqueueLastNullFrame(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pAddr,
	IN UCHAR TxRate,
	IN UCHAR PID,
	IN UCHAR apidx,
	IN BOOLEAN bQosNull,
	IN BOOLEAN bEOSP,
	IN UCHAR OldUP,
	IN UCHAR PwrMgmt,
	IN UCHAR OpMode);

VOID EnableMACTxPacket(
	IN RTMP_ADAPTER *pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR PwrMgmt,
	IN BOOLEAN bTxNullFramei,
	IN UCHAR QSel);

VOID DisableMACTxPacket(
	IN RTMP_ADAPTER *pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR PwrMgmt,
	IN BOOLEAN bWaitACK,
	IN UCHAR QSel);

VOID InitMultiChannelRelatedValue(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Channel,
	IN UCHAR CentralChannel);

VOID EDCA_ActionTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID HCCA_ActionTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

NDIS_STATUS MultiChannelThreadInit(RTMP_ADAPTER *pAd);
BOOLEAN MultiChannelThreadExit(RTMP_ADAPTER *pAd);
VOID MultiChannelTimerStop(RTMP_ADAPTER *pAd);
VOID MultiChannelTimerStart(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY  *pEntry);
#endif /* CONFIG_MULTI_CHANNEL */

VOID dev_rx_802_11_data_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
VOID dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
VOID dev_rx_ctrl_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

#ifdef CONFIG_STA_SUPPORT
BOOLEAN RtmpPktPmBitCheck(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg);
VOID RtmpPsActiveExtendCheck(RTMP_ADAPTER *pAd);
VOID RtmpPsModeChange(RTMP_ADAPTER *pAd, UINT32 PsMode);
#endif /* CONFIG_STA_SUPPORT */

#ifdef DOT11_N_SUPPORT
void DisplayTxAgg(RTMP_ADAPTER *pAd);
#endif /* DOT11_N_SUPPORT */

VOID set_default_ap_edca_param(EDCA_PARM *pEdca);
VOID set_default_sta_edca_param(EDCA_PARM *pEdca);

UCHAR dot11_2_ra_rate(UCHAR MaxSupportedRateIn500Kbps);
UCHAR dot11_max_sup_rate(INT SupRateLen, UCHAR *SupRate, INT ExtRateLen, UCHAR *ExtRate);

VOID TRTableResetEntry(RTMP_ADAPTER *pAd, UCHAR tr_tb_idx);
VOID TRTableInsertEntry(RTMP_ADAPTER *pAd, UCHAR tr_tb_idx, MAC_TABLE_ENTRY *pEntry);
VOID TRTableInsertMcastEntry(RTMP_ADAPTER *pAd, UCHAR tr_tb_idx, struct wifi_dev *wdev);
VOID TRTableEntryDump(RTMP_ADAPTER *pAd, INT tr_idx, const RTMP_STRING *caller, INT line);
VOID MgmtTableSetMcastEntry(RTMP_ADAPTER *pAd, UCHAR wcid);
VOID DataTableSetMcastEntry(RTMP_ADAPTER *pAd, UCHAR wcid);
VOID MacTableSetEntryPhyCfg(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);
VOID MacTableReset(RTMP_ADAPTER *pAd);
VOID MacTableResetWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
MAC_TABLE_ENTRY *MacTableLookup(RTMP_ADAPTER *pAd, UCHAR *pAddr);
MAC_TABLE_ENTRY *MacTableLookup2(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev);
BOOLEAN MacTableDeleteEntry(RTMP_ADAPTER *pAd, USHORT wcid, UCHAR *pAddr);
MAC_TABLE_ENTRY *MacTableInsertEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN struct wifi_dev *wdev,
	IN UINT32 ent_type,
	IN UCHAR OpMode,
	IN BOOLEAN CleanAll);

VOID dump_rxblk(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

#ifdef RTMP_MAC_PCI
VOID dump_rxd(RTMP_ADAPTER *pAd, RXD_STRUC *pRxD);
#endif /* RTMP_MAC_PCI */

INT set_no_bcn(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#if defined(WFA_VHT_PF) || defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA) || defined(MT7637_FPGA)
INT set_force_amsdu(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* defined(WFA_VHT_PF) || defined(MT7603_FPGA) */

#ifdef WFA_VHT_PF
/* for SIGMA */
INT set_vht_nss_mcs_cap(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_vht_nss_mcs_opt(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_vht_opmode_notify_ie(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT set_force_operating_mode(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_force_noack(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_force_vht_sgi(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_force_vht_tx_stbc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_force_ext_cca(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WFA_VHT_PF */


INT SetRF(RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
int write_reg(RTMP_ADAPTER *ad, UINT32 base, UINT16 offset, UINT32 value);
int read_reg(struct _RTMP_ADAPTER *ad, UINT32 base, UINT16 offset, UINT32 *value);


#ifdef MT_MAC
VOID StatRateToString(RTMP_ADAPTER *pAd, CHAR *Output, UCHAR TxRx, UINT32 RawData);
#endif /* MT_MAC */

#ifdef CONFIG_MULTI_CHANNEL
VOID Start_MCC(RTMP_ADAPTER *pAd);
VOID Stop_MCC(RTMP_ADAPTER *pAd, INT channel);
#endif /* CONFIG_MULTI_CHANNEL */

NDIS_STATUS RTMPInitHifAdapterBlock(RTMP_ADAPTER *pAd);
NDIS_STATUS RTMPFreeHifAdapterBlock(RTMP_ADAPTER *pAd);
NDIS_STATUS hif_data_init(RTMP_ADAPTER *pAd);
BOOLEAN wmode_band_equal(UCHAR smode, UCHAR tmode);
UCHAR wmode_2_rfic(UCHAR PhyMode);

struct wifi_dev *get_wdev_by_ioctl_idx_and_iftype(RTMP_ADAPTER *pAd, INT idx, INT iftype);
struct wifi_dev *get_wdev_by_idx(RTMP_ADAPTER *pAd, INT idx);

#ifdef SNIFFER_SUPPORT
VOID Monitor_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps);
VOID Monitor_Remove(RTMP_ADAPTER *pAd);
BOOLEAN Monitor_Open(RTMP_ADAPTER *pAd, PNET_DEV dev_p);
BOOLEAN Monitor_Close(RTMP_ADAPTER *pAd, PNET_DEV dev_p);
#endif /* SNIFFER_SUPPORT */

#ifdef CONFIG_FWOWN_SUPPORT
VOID FwOwn(RTMP_ADAPTER *pAd);
INT32 DriverOwn(RTMP_ADAPTER *pAd);
#endif

#ifdef ERR_RECOVERY
INT	Set_ErrDetectOn_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

INT	Set_ErrDetectMode_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);
#endif /* ERR_RECOVERY */

#ifdef CONFIG_STA_SUPPORT
PSTA_ADMIN_CONFIG GetStaCfgByWdev(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev);
MAC_TABLE_ENTRY *GetAssociatedAPByWdev(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev);
VOID sta_mac_entry_lookup(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev, MAC_TABLE_ENTRY **entry);
#endif /* CONFIG_STA_SUPPORT */

VOID mac_entry_lookup(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev, MAC_TABLE_ENTRY **entry);
VOID mac_entry_delete(struct _RTMP_ADAPTER	*ad, struct _MAC_TABLE_ENTRY *entry);

INT32 MtAcquirePowerControl(RTMP_ADAPTER *pAd, UINT32 Offset);
void MtReleasePowerControl(RTMP_ADAPTER *pAd, UINT32 Offset);

#ifdef BACKGROUND_SCAN_SUPPORT
INT set_background_scan(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_background_scan_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_background_scan_test(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_background_scan_notify(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_background_scan_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* BACKGROUND_SCAN_SUPPORT */

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
INT32 Set_RBIST_Switch_Mode(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);
INT32 Set_RBIST_Capture_Start(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);
INT32 Get_RBIST_Capture_Status(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);
INT32 Get_RBIST_Raw_Data_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);
INT32 Get_RBIST_IQ_Data(
	IN RTMP_ADAPTER *pAd,
	IN PINT32 pData,
	IN PINT32 pDataLen,
	IN UINT32 IQ_Type,
	IN UINT32 WF_Num);
INT32 Get_RBIST_IQ_Data_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);
UINT32 Get_System_CapNode_Info(
	IN RTMP_ADAPTER *pAd);
UINT32 Get_System_CenFreq_Info(
	IN RTMP_ADAPTER *pAd,
	IN UINT32 CapNode);
UINT8 Get_System_Bw_Info(
	IN RTMP_ADAPTER *pAd,
	IN UINT32 CapNode);
INT32 Get_System_Wireless_Info(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#define QA_IRR_WF0 1
#define QA_IRR_WF1 2
#define QA_IRR_WF2 4
#define QA_IRR_WF3 8
#define WF0 0
#define WF1 1
#define WF2 2
#define WF3 3
#define WF_NUM 4
#define BITMAP_WF0 1
#define BITMAP_WF1 2
#define BITMAP_WF2 4
#define BITMAP_WF3 8
#define BITMAP_WF_ALL 15
INT Set_IRR_ADC(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_IRR_RxGain(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_IRR_TTG(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_IRR_TTGOnOff(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT set_manual_protect(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_manual_rdg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_cca_en(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || defined(AXE) || defined(MT7626)
typedef enum _Stat_Action {
	RESET_COUNTER = 0,
	SHOW_RX_STATISTIC
} Stat_Action;

INT Set_Rx_Vector_Control(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq);

INT Show_Rx_Statistic(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif /* defined(MT7615) || defined(MT7622) */

#ifdef CUT_THROUGH
INT Set_CtLowWaterMark_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /*CUT_THROUGH*/

#ifdef SMART_CARRIER_SENSE_SUPPORT
INT Set_SCSEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_SCSCfg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_SCSPd_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_SCSinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef SCS_FW_OFFLOAD
INT SendSCSDataProc(RTMP_ADAPTER *pAd);
INT Show_SCS_FW_Offload_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT GetScsFwCounterProc(RTMP_ADAPTER *pAd);
#endif
#endif /* SMART_CARRIER_SENSE_SUPPORT */
INT Set_MibBucket_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef WIFI_EAP_FEATURE
#ifdef CHUTIL_SUPPORT
INT Show_ChUtil_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#endif
INT Show_MibBucket_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef DHCP_UC_SUPPORT
UINT16 RTMP_UDP_Checksum(IN PNDIS_PACKET pSkb);
#endif /* DHCP_UC_SUPPORT */
#ifdef ANTENNA_CONTROL_SUPPORT
INT Set_Antenna_Control_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_Antenna_Control_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Antenna_Control_Init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif
#ifdef NF_SUPPORT
INT Show_noise_floor_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT SetSKUCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetPercentageCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetPowerDropCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetBfBackoffCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetThermoCompCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetCCKTxStream(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetRfTxAnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTxPowerInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTOAECtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetEDCCACtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowEDCCAStatus(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetSKUInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetBFBackoffInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef WIFI_EAP_FEATURE
INT SetEDCCAThresholdCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetInitIPICtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowIPIValue(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_data_txpwr_offset(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_fw_ratbl_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_ratbl_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WIFI_EAP_FEATURE */

#ifdef TXD_MGMT_TXPWR_CTRL
INT set_mgmt_txpwr_offset(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* TXD_MGMT_TXPWR_CTRL */

#ifdef WIFI_GPIO_CTRL
INT set_gpio_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_gpio_value(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WIFI_GPIO_CTRL */
#ifdef LINK_TEST_SUPPORT
INT SetLinkTestRxParamCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetLinkTestModeCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetLinkTestPowerUpTblCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetLinkTestPowerUpTblInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetLinkTestInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* LINK_TEST_SUPPORT */
INT SetMuTxPower(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetBFNDPATxDCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTxPowerCompInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetThermalManualCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetThermalItemInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef TX_POWER_CONTROL_SUPPORT
INT SetTxPowerBoostCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* TX_POWER_CONTROL_SUPPORT */
#ifdef RF_LOCKDOWN
INT SetCalFreeApply(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetWriteEffuseRFpara(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetRFBackup(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* RF_LOCKDOWN */

#ifdef	ETSI_RX_BLOCKER_SUPPORT
INT SetFixWbIbRssiCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetRssiThCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetCheckThCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetAdaptRxBlockCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetWbRssiDirectCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetIbRssiDirectCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT ShowRssiThInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */

#ifdef TXPWRMANUAL
INT TxPowerManualCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgPwrManCtrl, UINT8 u1TxPwrModeManual, UINT8 u1TxPwrBwManual, UINT8 u1TxPwrRateManual, INT8 i1TxPwrValueManual, UCHAR ucBandIdx);
INT SetTxPwrManualCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* TXPWRMANUAL */

INT TxPowerPercentCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgTxPowerPercentEn, UCHAR ucBandIdx);
INT TxPowerDropCtrl(PRTMP_ADAPTER pAd, UINT8 ucPowerDrop, UCHAR ucBandIdx);
INT TxPowerSKUCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgTxPowerSKUEn, UCHAR ucBandIdx);
INT TxPowerBfBackoffCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgTxBFBackoffEn, UCHAR ucBandIdx);
INT TxCCKStreamCtrl(PRTMP_ADAPTER pAd, UINT8 u1CCKTxStream, UCHAR ucBandIdx);
INT ThermoCompCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgThermoCompEn, UCHAR ucBandIdx);
INT TxPowerRfTxAnt(PRTMP_ADAPTER pAd, UINT8 ucTxAntIdx);
INT TxPowerShowInfo(PRTMP_ADAPTER pAd, UCHAR ucTxPowerInfoCatg, UINT8 ucBandIdx);
INT show_BSSEdca_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_APEdca_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef WIFI_EAP_FEATURE
INT SetEdccaThreshold(PRTMP_ADAPTER pAd, UINT32 edcca_threshold, UINT8 BandIdx);
INT InitIPICtrl(PRTMP_ADAPTER pAd, UINT8 BandIdx);
INT GetIPIValue(PRTMP_ADAPTER pAd, UINT8 BandIdx);
INT SetDataTxPwrOffset(PRTMP_ADAPTER pAd, UINT8 WlanIdx, INT8 TxPwr_Offset,
		UINT8 BandIdx);
INT SetFwRaTable(PRTMP_ADAPTER pAd, UINT8 BandIdx, UINT8 TblType,
		UINT8 TblIndex, UINT16 TblLength, PUCHAR Buffer);
INT GetRaTblInfo(PRTMP_ADAPTER pAd, UINT8 BandIdx, UINT8 TblType,
		UINT8 TblIndex, UINT8 ReadnWrite);
#endif /* WIFI_EAP_FEATURE */
#ifdef WIFI_GPIO_CTRL
INT SetGpioCtrl(PRTMP_ADAPTER pAd, UINT8 GpioIdx, BOOLEAN GpioEn);
INT SetGpioValue(PRTMP_ADAPTER pAd, UINT8 GpioIdx, UINT8 GpioVal);
#endif /* WIFI_GPIO_CTRL */
INT TOAECtrlCmd(PRTMP_ADAPTER pAd, UCHAR TOAECtrl);
INT EDCCACtrlCmd(PRTMP_ADAPTER pAd, UCHAR ucBandIdx, UCHAR EDCCACtrl);
INT MuPwrCtrlCmd(PRTMP_ADAPTER pAd, BOOLEAN fgMuTxPwrManEn, CHAR cMuTxPwr, UINT8 u1BandIdx);
INT BFNDPATxDCtrlCmd(PRTMP_ADAPTER pAd, BOOLEAN fgNDPA_ManualMode, UINT8 ucNDPA_TxMode, UINT8 ucNDPA_Rate, UINT8 ucNDPA_BW, UINT8 ucNDPA_PowerOffset);
INT TemperatureCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgManualMode, CHAR cTemperature);
INT ThermalItemInfo(PRTMP_ADAPTER pAd);

#ifdef TX_POWER_CONTROL_SUPPORT
INT TxPwrUpCtrl(PRTMP_ADAPTER pAd, UINT8 ucBandIdx, CHAR cPwrUpCat,
		CHAR cPwrUpValue[POWER_UP_CATEGORY_RATE_NUM]);
#endif /* TX_POWER_CONTROL_SUPPORT */

UINT8 TxPowerGetChBand(UINT8 ucBandIdx, UINT8 CentralCh);
#ifdef TPC_SUPPORT
INT TxPowerTpcFeatureCtrl(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, INT8 TpcPowerValue);
INT TxPowerTpcFeatureForceCtrl(PRTMP_ADAPTER pAd, INT8 TpcPowerValue, UINT8 ucBandIdx, UINT8 CentralChannel);
#endif /* TPC_SUPPORT */

#define RETURN_STATUS_TRUE	   0

INT set_hnat_register(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

enum {
	MSDU_FORMAT,
	FINAL_AMSDU_FORMAT,
	MIDDLE_AMSDU_FORMAT,
	FIRST_AMSDU_FORMAT,
};

#define RETURN_IF_PAD_NULL(_pAd)	\
	{								   \
		if (_pAd == NULL) {			\
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Unexpected pAd NULL!\n", __func__));	   \
			return;						\
		}								\
	}
#define RETURN_ZERO_IF_PAD_NULL(_pAd)	\
	{								   \
		if (_pAd == NULL) {				\
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Unexpected pAd NULL!\n", __func__));	   \
			return 0;					\
		}								\
	}

UINT32 Get_OBSS_AirTime(PRTMP_ADAPTER pAd, UCHAR BandIdx);
VOID Reset_OBSS_AirTime(PRTMP_ADAPTER pAd, UCHAR BandIdx);
UINT32 Get_My_Tx_AirTime(PRTMP_ADAPTER pAd, UCHAR BandIdx);
UINT32 Get_My_Rx_AirTime(PRTMP_ADAPTER pAd, UCHAR BandIdx);
UINT32 Get_EDCCA_Time(PRTMP_ADAPTER pAd, UCHAR BandIdx);
VOID CCI_ACI_scenario_maintain(PRTMP_ADAPTER pAd);
#if defined(MT_MAC) && defined(VHT_TXBF_SUPPORT)
VOID Mumimo_scenario_maintain(PRTMP_ADAPTER	pAd);
#endif

#ifdef LED_CONTROL_SUPPORT
INT	Set_Led_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT wdev_edca_acquire(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);
struct wifi_dev *get_default_wdev(struct _RTMP_ADAPTER *ad);
UCHAR decide_phy_bw_by_channel(struct _RTMP_ADAPTER *ad, UCHAR channel);
void update_att_from_wdev(struct wifi_dev *dev1, struct wifi_dev *dev2);
BOOLEAN IsPublicActionFrame(PRTMP_ADAPTER pAd, VOID *pbuf);

#ifdef ERR_RECOVERY
INT32 ShowSerProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowSerProc2(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

#ifdef MT7626
INT32 ShowCCKCountProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /*MT7626*/

INT show_radar_threshold_param_proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 ShowBcnProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef TX_POWER_CONTROL_SUPPORT
INT32 ShowTxPowerBoostInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* TX_POWER_CONTROL_SUPPORT */

#ifdef MT_FDB
void fdb_enable(struct _RTMP_ADAPTER *pAd);
INT show_fdb_n9_log(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_fdb_cr4_log(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MT_FDB */

INT wdev_do_open(struct wifi_dev *wdev);
INT wdev_do_close(struct wifi_dev *wdev);
INT wdev_do_linkup(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT wdev_do_linkdown(struct wifi_dev *wdev);
INT wdev_do_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT wdev_do_disconn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
#ifdef CON_WPS
VOID APMlmeScanCompleteAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
#endif /* CON_WPS */

#if defined(WH_EZ_SETUP) || defined(CONFIG_MAP_SUPPORT)
VOID APMlmeDeauthReqAction(
	IN PRTMP_ADAPTER pAd,
	IN PMLME_QUEUE_ELEM Elem);
#endif

#ifdef OFFCHANNEL_SCAN_FEATURE

INT Set_ScanResults_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT Set_ApScan_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT Channel_Info_MsgHandle(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, POS_COOKIE pObj);

VOID ReadChannelStats(IN PRTMP_ADAPTER   pAd);
#endif

#ifdef TXRX_STAT_SUPPORT
VOID ReadChannelStats(IN PRTMP_ADAPTER   pAd);
#endif

#ifdef MULTI_PROFILE
typedef enum MTB_PROFILE_ID_T {
	MTB_2G_PROFILE,
	MTB_5G_PROFILE,
	MTB_MERGE_PROFILE,
	MTB_DEV_PREFIX
} MTB_PROFILE_ID;

NDIS_STATUS update_mtb_value(RTMP_ADAPTER *pAd, UCHAR profile_id, UINT_32 extra, RTMP_STRING *value);
INT	multi_profile_apcli_devname_req(struct _RTMP_ADAPTER *ad, UCHAR *final_name,
									INT *ifidx);
UCHAR is_multi_profile_enable(struct _RTMP_ADAPTER *ad);
UCHAR multi_profile_get_pf1_num(struct _RTMP_ADAPTER *ad);
UCHAR multi_profile_get_pf2_num(struct _RTMP_ADAPTER *ad);
#endif /*MULTI_PROFILE*/

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
typedef enum _ENUM_PCIE_ASPM_TYPE_T {
	PCIE_ASPM_TYPE_L0S,
	PCIE_ASPM_TYPE_L1,
	PCIE_ASPM_TYPE_NUM,
} ENUM_PCIE_ASPM_TYPE_T, *P_ENUM_PCIE_ASPM_TYPE_T;
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

INT build_extra_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info);
INT build_extended_cap_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info);
INT build_rsn_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf);
INT build_wsc_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info);
INT build_wmm_cap_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info);
VOID parse_RXV_packet_v1(RTMP_ADAPTER *pAd, UINT32 Type, RX_BLK *RxBlk, UCHAR *Data);
VOID parse_RXV_packet_v2(RTMP_ADAPTER *pAd, UINT32 Type, RX_BLK *RxBlk, UCHAR *Data);
INT32 txs_handler_v1(RTMP_ADAPTER *pAd, RX_BLK *rx_blk, VOID *rx_packet);
INT32 txs_handler_v2(RTMP_ADAPTER *pAd, RX_BLK *rx_blk, VOID *rx_packet);
SCAN_CTRL *get_scan_ctrl_by_wdev(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
PBSS_TABLE get_scan_tab_by_wdev(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

BOOLEAN sync_fsm_ops_init(struct wifi_dev *wdev);
VOID wdev_fsm_init(struct wifi_dev *wdev);

#if defined(MT7615) || defined(MT7622) || defined(MT7663) || defined(MT7626)
INT SetRxRateRecordEn(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetRxvRecordEn(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
BOOLEAN SetCsdDebugSet(RTMP_ADAPTER *pAd, UINT8 band_idx, UINT8 g0_value, UINT8 g1_value, UINT8 g2_value);
INT ShowRxRateHistogram(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

PRxVBQElmt RxVBDequeue(
	IN PRTMP_ADAPTER pAd);

PRxVBQElmt RxVBQueueSearch(
	IN	PRTMP_ADAPTER pAd,
	IN	UINT8		rxv_sn,
	IN	UINT32		Type);

VOID RecordRxVB(
	RTMP_ADAPTER *pAd,
	RX_BLK *RxBlk,
	UCHAR *Data,
	UINT32 Type);

VOID RxVB_Report_Action(
	PRTMP_ADAPTER pAd,
	UCHAR wcid,
	PRxVBQElmt prxvbqelmt);

UINT16 Checksum16(UINT8 *pData, int len);
UINT16 UdpChecksum16(UINT8 *pData, int len);
#endif /* #if defined(MT7615) || defined(MT7622) || defined(MT7663) || defined(MT7626) */
#ifdef MAC_REPEATER_SUPPORT
VOID RxTrackingInit(struct wifi_dev *wdev);
#endif /* MAC_REPEATER_SUPPORT */
INT Set_BSSAifsn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_BSSCwmin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_BSSCwmax_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_BSSTxop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_APAifsn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_APCwmin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_APCwmax_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_APTxop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef IXIA_SUPPORT
#define IS_EXPECTED_LENGTH(pAd, len) (((len >= (dectlen_l - 4)) && (len <= dectlen_l))\
		|| ((len >= (dectlen_m - 8)) && (len <= (dectlen_m + 8)))\
		|| ((len >= (dectlen_h - 8)) && (len <= (dectlen_h + 8))))
INT Set_pkt_threshld_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_chkT_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_statistic_pktlen_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
void wifi_txrx_parmtrs_dump(RTMP_ADAPTER *pAd);
extern UINT rx_pkt_to_os;
extern char *rdrop_reason[MAX_RDROP_RESON];
extern unsigned short dectlen_l;
extern unsigned short dectlen_m;
extern unsigned short dectlen_h;
VOID periodic_detect_ixia_mode(RTMP_ADAPTER *pAd);
INT	Set_Rssi_Threshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_MinRssi_Threshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxSwqCtrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_IXIA_TX_MODE_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_vowMode_Manual_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT vow_set_feature_all(struct _RTMP_ADAPTER *pad);
INT ge_bss_clean_queue_ixia(struct _RTMP_ADAPTER *pAd);
#endif /*IXIA_SUPPORT*/
void ba_reordering_list_pkts_release(RTMP_ADAPTER *pAd);
#ifdef MGMT_TXPWR_CTRL
INT set_mgmt_frame_power(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_mgmt_frame_power(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT update_mgmt_frame_power(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif

USHORT RTMPGetLastTxRate(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *pEntry);

#endif  /* __RTMP_H__ */
