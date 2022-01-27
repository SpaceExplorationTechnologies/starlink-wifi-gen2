/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_asic.h

	Abstract:
	Ralink Wireless Chip HW related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __ASIC_CTRL_H__
#define __ASIC_CTRL_H__

#ifdef MT_MAC
#include "hw_ctrl/cmm_asic_mt.h"
#include "hw_ctrl/cmm_asic_mt_fw.h"
#endif /* MT_MAC */

#include "common/wifi_sys_info.h"

struct _TX_BLK;
struct _RX_BLK;
struct freq_oper;
struct wmm_entry;

enum PACKET_TYPE;

#define TX_RTY_CFG_RTY_LIMIT_SHORT		0x1
#define TX_RTY_CFG_RTY_LIMIT_LONG		0x2

#define MINIMUM_POWER_VALUE            -127
#define TX_STREAM_PATH                    4
#define RX_STREAM_PATH_SINGLE_MODE        4
#define RX_STREAM_PATH_DBDC_MODE          2

#define CHANNEL_BAND_2G                   0
#define CHANNEL_BAND_5G                   1

#define CMW_RSSI_SOURCE_BBP               0
#define CMW_RSSI_SOURCE_WTBL              1

#define CMW_RCPI_MA_1_1                   1
#define CMW_RCPI_MA_1_2                   2
#define CMW_RCPI_MA_1_4                   4
#define CMW_RCPI_MA_1_8                   8

#define TX_DEFAULT_CSD_STATE              0
#define TX_ZERO_CSD_STATE                 1
#define TX_UNDEFINED_CSD_STATE            0xFF

#define TX_DEFAULT_POWER_STATE            0
#define TX_BOOST_POWER_STATE              1
#define TX_UNDEFINED_POWER_STATE          0xFF

#define TX_DEFAULT_BW_STATE               0
#define TX_SWITCHING_BW_STATE             1
#define TX_UNDEFINED_BW_STATE             0xFF

#define TX_DEFAULT_SPEIDX_STATE           0
#define TX_SWITCHING_SPEIDX_STATE         1
#define TX_UNDEFINED_SPEIDX_STATE         0xFF

#define TX_DEFAULT_MAXIN_STATE            0
#define TX_SPECIFIC_ACR_STATE             1
#define TX_UNDEFINED_RXFILTER_STATE       0xFF

#define RX_DEFAULT_RCPI_STATE             0
#define RX_SPECIFIC_RCPI_STATE            1
#define RX_UNDEFINED_RCPI_STATE           0xFF

#define RX_DEFAULT_RXSTREAM_STATE         15
#define RX_RXSTREAM_WF0_STATE             1
#define RX_RXSTREAM_WF1_STATE             2
#define RX_RXSTREAM_WF2_STATE             4
#define RX_RXSTREAM_WF3_STATE             8
#define RX_RXSTREAM_WF01_STATE            3
#define RX_RXSTREAM_WF02_STATE            5
#define RX_RXSTREAM_WF03_STATE            9
#define RX_RXSTREAM_WF12_STATE            6
#define RX_RXSTREAM_WF13_STATE            10
#define RX_RXSTREAM_WF23_STATE            12
#define RX_RXSTREAM_WF012_STATE           7
#define RX_RXSTREAM_WF013_STATE           11
#define RX_RXSTREAM_WF023_STATE           13
#define RX_RXSTREAM_WF123_STATE           14
#define RX_UNDEFINED_RXSTREAM_STATE       0xFF

#define CMW_POWER_UP_RATE_NUM             13
#define CMW_POWER_UP_CATEGORY_NUM         4

#define LINK_TEST_AUTO_RSSI_THRESHOLD     0xFF

#ifdef LINK_TEST_SUPPORT
typedef enum _ENUM_RSSI_CHECK_SOURCE {
	RSSI_CHECK_WTBL_RSSI = 0,
	RSSI_CHECK_BBP_WBRSSI,
	RSSI_CHECK_BBP_IBRSSI,
	RSSI_CHECK_NUM
} ENUM_RSSI_CHECK_SOURCE, *P_ENUM_RSSI_CHECK_SOURCE;

typedef enum _ENUM_RSSI_REASON {
	RSSI_REASON_SENSITIVITY = 0,
	RSSI_REASON_RX_BLOCKING,
	RSSI_REASON_NUM
} ENUM_RSSI_REASON, *P_ENUM_RSSI_REASON;
#endif /* LINK_TEST_SUPPORT */

VOID AsicNotSupportFunc(struct _RTMP_ADAPTER *pAd, const RTMP_STRING *caller);


VOID AsicUpdateRtsThld(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 PktNumThrd, UINT32 PpduLengthThrd);
VOID AsicUpdateProtect(struct _RTMP_ADAPTER *pAd);

INT AsicSetTxStream(struct _RTMP_ADAPTER *pAd, UINT32 StreamNum, UCHAR opmode, BOOLEAN up, UCHAR BandIdx);
INT AsicSetRxStream(struct _RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx);
INT AsicSetBW(struct _RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx);
INT AsicSetCtrlCh(struct _RTMP_ADAPTER *pAd, UINT8 extch);
#ifdef DOT11_VHT_AC
INT AsicSetRtsSignalTA(struct _RTMP_ADAPTER *pAd, UCHAR bw_sig);
#endif /* DOT11_VHT_AC */
VOID AsicAntennaSelect(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
VOID AsicBBPAdjust(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
VOID AsicSwitchChannel(struct _RTMP_ADAPTER *pAd, UCHAR band_idx, struct freq_oper *oper, BOOLEAN bScan);

#ifdef CONFIG_STA_SUPPORT
VOID AsicSleepAutoWakeup(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);
VOID AsicWakeup(struct _RTMP_ADAPTER *pAd, BOOLEAN bFromTx, struct _STA_ADMIN_CONFIG *pStaCfg);
#endif /* CONFIG_STA_SUPPORT */

INT AsicSetDevMac(struct _RTMP_ADAPTER *pAd, UCHAR *addr, UCHAR omac_idx);
VOID AsicSetBssid(struct _RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR curr_bssid_idx);
VOID AsicDelWcidTab(struct _RTMP_ADAPTER *pAd, UCHAR Wcid);

#ifdef HTC_DECRYPT_IOT
INT32 AsicSetWcidAAD_OM(struct _RTMP_ADAPTER *pAd, UCHAR Wcid, CHAR value);
#endif /* HTC_DECRYPT_IOT */

#ifdef MBSS_AS_WDS_AP_SUPPORT
VOID AsicSetWcid4Addr_HdrTrans(struct _RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR IsEnable);
#endif


#ifdef MAC_APCLI_SUPPORT
VOID AsicSetApCliBssid(struct _RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR index);
#endif /* MAC_APCLI_SUPPORT */

INT AsicSetRxFilter(struct _RTMP_ADAPTER *pAd);

VOID AsicSetTmrCR(struct _RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx);

#ifdef DOT11_N_SUPPORT
INT AsicSetRDG(struct _RTMP_ADAPTER *pAd,
			   UCHAR wlan_idx, UCHAR band_idx, UCHAR init, UCHAR resp);
#ifdef MT_MAC
INT AsicWtblSetRDG(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable);
INT AsicUpdateTxOP(struct _RTMP_ADAPTER *pAd, UINT32 ac_num, UINT32 txop_val);
#endif /* MT_MAC */
#endif /* DOT11_N_SUPPORT */

INT AsicSetPreTbtt(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR HwBssidIdx);
INT AsicSetGPTimer(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout);
INT AsicSetChBusyStat(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
INT AsicGetTsfTime(
	struct _RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx);

#ifdef THERMAL_PROTECT_SUPPORT
INT AsicThermalProtectAdmitDutyInfo(
	struct _RTMP_ADAPTER	*pAd
);
#endif /* THERMAL_PROTECT_SUPPORT */

VOID AsicSetSyncModeAndEnable(
	struct _RTMP_ADAPTER *pAd,
	USHORT BeaconPeriod,
	UCHAR HWBssidIdx,
	UCHAR OPMode);

VOID AsicDisableSync(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx);

#ifdef CONFIG_STA_SUPPORT
VOID AsicEnableIbssSync(
	struct _RTMP_ADAPTER *pAd,
	USHORT BeaconPeriod,
	UCHAR HWBssidIdx,
	UCHAR OPMode);
#endif

UINT32 AsicGetWmmParam(struct _RTMP_ADAPTER *pAd, UINT32 ac, UINT32 type);
INT AsicSetWmmParam(struct _RTMP_ADAPTER *pAd, UCHAR idx, UINT ac, UINT type, UINT val);
VOID AsicSetEdcaParm(struct _RTMP_ADAPTER *pAd, struct wmm_entry *entry, struct wifi_dev *wdev);
INT AsicSetRetryLimit(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit);
UINT32 AsicGetRetryLimit(struct _RTMP_ADAPTER *pAd, UINT32 type);
VOID AsicSetSlotTime(struct _RTMP_ADAPTER *pAd, BOOLEAN bUseShortSlotTime, UCHAR channel, struct wifi_dev *wdev);
INT AsicSetMacMaxLen(struct _RTMP_ADAPTER *pAd);

VOID AsicAddSharedKeyEntry(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR BssIdx,
	IN UCHAR KeyIdx,
	IN PCIPHER_KEY pCipherKey);

VOID AsicRemoveSharedKeyEntry(struct _RTMP_ADAPTER *pAd, UCHAR BssIdx, UCHAR KeyIdx);

VOID AsicUpdateRxWCIDTable(struct _RTMP_ADAPTER *pAd, USHORT WCID, UCHAR *pAddr, BOOLEAN IsBCMCWCID, BOOLEAN IsReset);
VOID AsicUpdateBASession(struct _RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid, UINT16 sn, UCHAR basize, BOOLEAN isAdd, INT ses_type);
UINT16 AsicGetTidSn(struct _RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid);

#ifdef TXBF_SUPPORT
VOID AsicUpdateClientBfCap(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif /* TXBF_SUPPORT */

#ifdef MT_MAC
VOID AsicAddRemoveKeyTab(struct _RTMP_ADAPTER *pAd, struct _ASIC_SEC_INFO *pInfo);
#endif

#ifdef CONFIG_AP_SUPPORT
VOID AsicSetMbssWdevIfAddr(struct _RTMP_ADAPTER *pAd, INT idx, UCHAR *if_addr, INT opmode);
VOID AsicSetMbssWdevIfAddrGen2(struct _RTMP_ADAPTER *pAd, VOID *wdev_void, INT opmode);
#endif /* CONFIG_AP_SUPPORT */

BOOLEAN AsicDisableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev);
BOOLEAN AsicEnableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev);
BOOLEAN AsicUpdateBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev, UINT16 FrameLen, UCHAR UpdatePktType);

INT32 AsicDevInfoUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucOwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 u4EnableFeature);

INT32 AsicStaRecUpdate(
	RTMP_ADAPTER * pAd,
	STA_REC_CTRL_T *sta_rec_ctrl);

INT32 AsicRaParamStaRecUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT8 WlanIdx,
	struct _STAREC_AUTO_RATE_UPDATE_T *prParam,
	UINT32 EnableFeature);

INT32 AsicBssInfoUpdate(
	struct _RTMP_ADAPTER *pAd,
	struct _BSS_INFO_ARGUMENT_T *bss_info_argument);

#define AsicBssInfoReNew(pAd, bss_info_argument) AsicBssInfoUpdate(pAd, bss_info_argument)

INT32 AsicExtPwrMgtBitWifi(struct _RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucPwrMgtBit);
INT32 AsicRadioOnOffCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 ucRadio);
#ifdef GREENAP_SUPPORT
INT32 AsicGreenAPOnOffCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN ucGreenAPOn);
#endif /* GREENAP_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT32 asic_pcie_aspm_dym_ctrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
INT32 AsicExtPmStateCtrl(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg, UINT8 ucPmNumber, UINT8 ucPmState);
INT32 AsicExtWifiHifCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 PmStatCtrl, VOID *pResult);

INT32 AsicMccStart(struct _RTMP_ADAPTER *ad,
				   UCHAR channel_1st,
				   UCHAR channel_2nd,
				   UINT32 bw_1st,
				   UINT32 bw_2nd,
				   UCHAR central_1st_seg0,
				   UCHAR central_1st_seg1,
				   UCHAR central_2nd_seg0,
				   UCHAR central_2nd_seg1,
				   UCHAR role_1st,
				   UCHAR role_2nd,
				   USHORT stay_time_1st,
				   USHORT stay_time_2nd,
				   USHORT idle_time,
				   USHORT null_repeat_cnt,
				   UINT32 start_tsf);

INT32 AsicBfSoundingPeriodicTriggerCtrl(struct _RTMP_ADAPTER *pAd, UINT32 WlanIdx, UINT8 On);
#ifdef THERMAL_PROTECT_SUPPORT
INT32 AsicThermalProtect(
	RTMP_ADAPTER * pAd,
	UINT8 ucBand,
	UINT8 HighEn,
	CHAR HighTempTh,
	UINT8 LowEn,
	CHAR LowTempTh,
	UINT32 RechkTimer,
	UINT8 RFOffEn,
	CHAR RFOffTh,
	UINT8 ucType);

INT32 AsicThermalProtectAdmitDuty(
	RTMP_ADAPTER * pAd,
	UINT8 ucBand,
	UINT32 u4Lv0Duty,
	UINT32 u4Lv1Duty,
	UINT32 u4Lv2Duty,
	UINT32 u4Lv3Duty
);
#endif /* THERMAL_PROTECT_SUPPORT */

INT AsicSendCommandToMcu(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR         Command,
	IN UCHAR         Token,
	IN UCHAR         Arg0,
	IN UCHAR         Arg1,
	IN BOOLEAN in_atomic);

BOOLEAN AsicSendCmdToMcuAndWait(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic);


#ifdef STREAM_MODE_SUPPORT
VOID AsicSetStreamMode(
	struct _RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddr,
	IN INT chainIdx,
	IN BOOLEAN bEnabled);

VOID AsicStreamModeInit(struct _RTMP_ADAPTER *pAd);
#endif /*STREAM_MODE_SUPPORT*/

INT AsicSetAutoFallBack(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
INT AsicAutoFallbackInit(struct _RTMP_ADAPTER *pAd);

VOID AsicSetPiggyBack(struct _RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack);
VOID AsicGetTxTsc(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pTxTsc);
VOID AsicSetSMPS(struct _RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR smps);
VOID AsicTurnOffRFClk(struct _RTMP_ADAPTER *pAd, UCHAR Channel);

#ifdef MAC_REPEATER_SUPPORT
VOID AsicInsertRepeaterEntry(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR CliIdx,
	IN PUCHAR pAddr);

VOID AsicRemoveRepeaterEntry(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR CliIdx);

#ifdef MT_MAC
VOID AsicInsertRepeaterRootEntry(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	IN  UCHAR *pAddr,
	IN UCHAR ReptCliIdx);
#endif /* MT_MAC */
#endif /* MAC_REPEATER_SUPPORT*/

INT32 AsicRxHeaderTransCtl(struct _RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP);
INT32 AsicRxHeaderTaranBLCtl(struct _RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType);
INT AsicSetRxvFilter(RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR ucBandIdx);

#ifdef CONFIG_AP_SUPPORT
VOID AsicSetMbssHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID AsicSetExtMbssEnableCR(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID AsicSetExtTTTTHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
#endif /* CONFIG_AP_SUPPORT */
INT32 AsicGetMacInfo(struct _RTMP_ADAPTER *pAd, UINT32 *ChipId, UINT32 *HwVer, UINT32 *FwVer);
INT32 AsicGetAntMode(struct _RTMP_ADAPTER *pAd, UCHAR *AntMode);
INT32 AsicSetDmaByPassMode(struct _RTMP_ADAPTER *pAd, BOOLEAN isByPass);

#ifdef DBDC_MODE
INT32 AsicSetDbdcCtrl(struct _RTMP_ADAPTER *pAd, struct _BCTRL_INFO_T *pBctrlInfo);
INT32 AsicGetDbdcCtrl(struct _RTMP_ADAPTER *pAd, struct _BCTRL_INFO_T *pBctrlInfo);
#endif /*DBDC_MODE*/

VOID AsicNotSupportFunc(struct _RTMP_ADAPTER *pAd, const RTMP_STRING *caller);

#ifdef IGMP_SNOOP_SUPPORT
BOOLEAN AsicMcastEntryInsert(struct _RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr, PNET_DEV dev, UINT8 WlanIndex);
BOOLEAN AsicMcastEntryDelete(struct _RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev, UINT8 WlanIndex);
#endif

VOID RssiUpdate(struct _RTMP_ADAPTER *pAd);

#ifdef ETSI_RX_BLOCKER_SUPPORT
VOID CheckRssi(struct _RTMP_ADAPTER *pAd);
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */

INT asic_rts_on_off(struct wifi_dev *wdev, BOOLEAN rts_en);
INT AsicAmpduEfficiencyAdjust(struct wifi_dev *wdev, UCHAR	aifs_adjust);
/**
 * arch operation HAL
 * @check_hw_resource: check hw resource if enough or not for TX.
 * @indicate_tx_resource_state: indicate hw resource in safe region or not
 * @hw_tx: fill hw descriptor and kick out to hw resource for data frame
 * @mlme_hw_tx: fill hw descriptor and kick out to hw resource for mlme frame
 * @ate_hw_tx: fill hw descriptor and kick out to hw resource for ate frame
 * @write_tx_resource:HIF resource arrangement for MSDU TX packet transfer.
 * @write_multi_tx_resource:HIF resource arrangement for A-MSDU TX packet transfer.
 * @write_final_tx_resource:HIF resource arrangement for A-MSDU TX packet transfer
 *	for total bytes	update in tx hw header.
 * @write_frag_tx_resource:HIF resource arrangement for fragment TX packet transfer
 * @kickout_data_tx: TX data packet kick-out to HW to start transfer.
 * @write_tmac_info_fixed_rate: per 802.11/802.3 frame TMAC descriptor handle that use fixed rate.
 * @write_tmac_info: per 802.11/802.3 frame TMAC descriptor handle.
 * @write txp_info: per 802.11/802.3 data frame TMAC data information (address and length) handle.
 * @dump_tmac_info: dump tmac header information
 * @rx_pkt_process: per rx packet handle (build RX_BLK and packet processing).
 * @get_packet_type: get packet
 * @trans_rxd_into_rxblk: translate rxd into rxblk
 * @get_pkt_from_rx_resource: get RX packet from HIF resource.
 * @get_tx_resource_free_num: get tx data HIF resource available numbers
 * @inc_resource_full_cnt: increase HIF resource full count
 * @get_resource_state: get HIF resource number state
 * @set_resource_state: set HIF resource number state
 * @check_resource_state: check HIF resource number state change
 * @get_mgmt_resource_free_num: get management HIF resource available numbers
 * @get_bcn_resource_free_num: get beacon HIF resource available numbers
 * @get_cmd_resource_free_num: get command HIF resource available numbers
 * @get_fw_loading_resource_free_num: get fw loading HIF resource available numbers
 * @get_hif_buf: get one available HIF pre-allocated dma region for hw header
 * @is_tx_resource_empty: check if tx data HIF resource is empty or not
 * @is_rx_resource_full: check if rx HIF resource is full or not
 * @get_rx_resource_pending_num: get rx HIF resource pending numbers
 */
typedef struct _RTMP_ARCH_OP {
	UINT32 (*archGetCrcErrCnt)(struct _RTMP_ADAPTER *pAd);
	UINT32 (*archGetCCACnt)(struct _RTMP_ADAPTER *pAd);
	UINT32 (*archGetChBusyCnt)(struct _RTMP_ADAPTER *pAd, UCHAR ch_idx);
	INT (*archSetAutoFallBack)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
	INT (*archAutoFallbackInit)(struct _RTMP_ADAPTER *pAd);
	VOID (*archUpdateProtect)(struct _RTMP_ADAPTER *pAd, MT_PROTECT_CTRL_T *Protect);
	VOID (*archUpdateRtsThld)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR pkt_thld, UINT32 len_thld);
#ifdef DOT11_N_SUPPORT
	INT (*archSetRDG)(struct _RTMP_ADAPTER *pAd, MT_RDG_CTRL_T *Rdg);
#endif /* DOT11_N_SUPPORT */
	VOID (*archSwitchChannel)(struct _RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg);
	INT32 (*archSetDevMac)(
		struct _RTMP_ADAPTER *pAd,
		UINT8 OwnMacIdx,
		UINT8 *OwnMacAddr,
		UINT8 BandIdx,
		UINT8 Active,
		UINT32 EnableFeature);

	INT32 (*archSetBssid)(
		struct _RTMP_ADAPTER *pAd,
		struct _BSS_INFO_ARGUMENT_T *bss_info_argument);

	INT32 (*archSetStaRec)(struct _RTMP_ADAPTER *pAd, STA_REC_CFG_T StaCfg);
	VOID (*archDelWcidTab)(struct _RTMP_ADAPTER *pAd, UCHAR wcid_idx);
#ifdef HTC_DECRYPT_IOT
	INT32 (*archSetWcidAAD_OM)(struct _RTMP_ADAPTER *pAd, UCHAR wcid_idx, UCHAR value);
#endif /* HTC_DECRYPT_IOT */

#ifdef MBSS_AS_WDS_AP_SUPPORT
	VOID (*archSetWcid4Addr_HdrTrans)(struct _RTMP_ADAPTER *pAd, UCHAR wcid_idx, UCHAR IsEnable, UCHAR IsApcliEntry);
#endif

	VOID (*archAddRemoveKeyTab)(struct _RTMP_ADAPTER *pAd, struct _ASIC_SEC_INFO *pInfo);

	BOOLEAN (*archEnableBeacon)(struct _RTMP_ADAPTER *pAd, VOID *wdev_void);
	BOOLEAN (*archDisableBeacon)(struct _RTMP_ADAPTER *pAd, VOID *wdev_void);
	BOOLEAN (*archUpdateBeacon)(struct _RTMP_ADAPTER *pAd, VOID *wdev_void, UINT16 FrameLen, UCHAR UpdatePktType);
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	INT (*archSetReptFuncEnable)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
	VOID (*archInsertRepeaterEntry)(struct _RTMP_ADAPTER *pAd, UCHAR CliIdx, PUCHAR pAddr);
	VOID (*archRemoveRepeaterEntry)(struct _RTMP_ADAPTER *pAd, UCHAR CliIdx);
	VOID (*archInsertRepeaterRootEntry)(struct _RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR *pAddr, UCHAR ReptCliIdx);
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */

	INT (*archSetRxFilter)(struct _RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T RxFilter);
	VOID (*archSetPiggyBack)(struct _RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack);
	INT (*archSetPreTbtt)(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable, UCHAR HwBssidIdx);
	INT (*archSetGPTimer)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout);
	INT (*archSetChBusyStat)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
	INT (*archGetTsfTime)(
		struct _RTMP_ADAPTER *pAd,
		UINT32 *high_part,
		UINT32 *low_part,
		UCHAR HwBssidIdx);

	VOID (*archDisableSync)(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx);

	VOID (*archSetSyncModeAndEnable)(
		struct _RTMP_ADAPTER *pAd,
		USHORT BeaconPeriod,
		UCHAR HWBssidIdx,
		UCHAR OPMode);

#ifdef CONFIG_STA_SUPPORT
	VOID (*archEnableIbssSync)(struct _RTMP_ADAPTER *pAd,
							   USHORT BeaconPeriod,
							   UCHAR HWBssidIdx,
							   UCHAR OPMode);
#endif /* CONFIG_STA_SUPPORT */

	INT (*archSetWmmParam)(struct _RTMP_ADAPTER *pAd, UCHAR idx, UINT ac, UINT type, UINT val);
	VOID (*archSetEdcaParm)(struct _RTMP_ADAPTER *pAd, UCHAR idx, UCHAR tx_mode, struct _EDCA_PARM *pEdcaParm);
	UINT32 (*archGetWmmParam)(struct _RTMP_ADAPTER *pAd,  UINT32 AcNum, UINT32 EdcaType);
	INT (*archSetRetryLimit)(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit);
	UINT32 (*archGetRetryLimit)(struct _RTMP_ADAPTER *pAd, UINT32 type);
	VOID (*archSetSlotTime)(struct _RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime, UCHAR BandIdx);
	INT (*archSetMacMaxLen)(struct _RTMP_ADAPTER *pAd);
	VOID (*archGetTxTsc)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pTxTsc);
	VOID (*archAddSharedKeyEntry)(struct _RTMP_ADAPTER *pAd, UCHAR BssIndex, UCHAR KeyIdx, struct _CIPHER_KEY *pCipherKey);
	VOID (*archRemoveSharedKeyEntry)(struct _RTMP_ADAPTER *pAd, UCHAR BssIndex, UCHAR KeyIdx);
	VOID (*archAddPairwiseKeyEntry)(struct _RTMP_ADAPTER *pAd, UCHAR WCID, PCIPHER_KEY pCipherKey);
	INT (*archSetRtsSignalTA)(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx, BOOLEAN Enable);
	INT (*archSetRxStream)(struct _RTMP_ADAPTER *pAd, UINT32 rx_path, UCHAR BandIdx);
	INT (*archSetTxStream)(struct _RTMP_ADAPTER *pAd, UINT32 rx_path, UCHAR BandIdx);
	INT (*archSetBW)(struct _RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx);
	INT (*archSetCtrlCh)(struct _RTMP_ADAPTER *pAd, UINT8 extch);
	VOID (*archSetApCliBssid)(struct _RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR index);
	INT (*archWaitMacTxRxIdle)(struct _RTMP_ADAPTER *pAd);
	INT (*archSetMacTxRx)(struct _RTMP_ADAPTER *pAd, INT txrx, BOOLEAN enable, UCHAR BandIdx);
	INT (*archSetRxvFilter)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR BandIdx);
	INT (*archSetMacTxQ)(struct _RTMP_ADAPTER *pAd, INT WmmSet, INT band, BOOLEAN Enable);
	INT (*archSetMacWD)(struct _RTMP_ADAPTER *pAd);
	INT (*archTOPInit)(struct _RTMP_ADAPTER *pAd);
	VOID (*archSetTmrCR)(struct _RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx);

#ifdef CONFIG_AP_SUPPORT
	VOID (*archSetMbssWdevIfAddr)(struct _RTMP_ADAPTER *pAd, INT idx, UCHAR *if_addr, INT opmode);
	VOID (*archSetMbssHwCRSetting)(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
	VOID (*archSetExtTTTTHwCRSetting)(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
	VOID (*archSetExtMbssEnableCR)(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
#endif

#ifdef DBDC_MODE
	INT (*archSetDbdcCtrl)(struct _RTMP_ADAPTER *pAd, struct _BCTRL_INFO_T *pBctrInfo);
	INT (*archGetDbdcCtrl)(struct _RTMP_ADAPTER *pAd, struct _BCTRL_INFO_T *pBctrInfo);
#endif

	VOID (*archUpdateRxWCIDTable)(struct _RTMP_ADAPTER *pAd, MT_WCID_TABLE_INFO_T WtblInfo);

#ifdef TXBF_SUPPORT
	VOID (*archUpdateClientBfCap)(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif

#ifdef MT_MAC
	INT32 (*archUpdateBASession)(struct _RTMP_ADAPTER *pAd, MT_BA_CTRL_T BaCtrl);
	UINT16 (*archGetTidSn)(struct _RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid);
#endif /*MT_MAC */
	INT32 (*archUpdateStaRecBa)(struct _RTMP_ADAPTER *pAd, STA_REC_BA_CFG_T StaRecCfg);
	VOID (*archSetSMPS)(struct _RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR smps);
	INT32 (*archRxHeaderTransCtl)(struct _RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP);
	INT32 (*archRxHeaderTaranBLCtl)(struct _RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType);

	/*  ISR */
	BOOLEAN (*tx_dma_done_handle)(struct _RTMP_ADAPTER *pAd, UINT8 hif_idx);
	BOOLEAN (*cmd_dma_done_handle)(struct _RTMP_ADAPTER *pAd, UINT8 hif_idx);
	BOOLEAN (*fwdl_dma_done_handle)(struct _RTMP_ADAPTER *pAd, UINT8 hif_idx);

	/* TX */
	UINT32 (*get_resource_idx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum PACKET_TYPE pkt_type, UINT8 que_idx) ____cacheline_aligned;
	NDIS_STATUS (*build_resource_idx)(struct _RTMP_ADAPTER *pAd);
	INT (*check_hw_resource)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR resource_idx);

	VOID (*inc_resource_full_cnt)(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
	BOOLEAN (*get_resource_state)(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
	INT (*set_resource_state)(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx, BOOLEAN state);
	UINT32 (*check_resource_state)(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
	UINT32 (*get_tx_resource_free_num)(struct _RTMP_ADAPTER *pAd, UINT8 que_idx);

	UCHAR *(*get_hif_buf)(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk, UINT8 hif_idx, UCHAR frame_type);
	INT (*hw_tx)(struct _RTMP_ADAPTER *ad, struct _TX_BLK *tx_blk);
	VOID (*write_tmac_info)(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
	INT32 (*write_txp_info)(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
	USHORT (*write_tx_resource)(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, BOOLEAN bIsLast, USHORT *FreeNumber);
	VOID (*kickout_data_tx)(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk, UCHAR que_idx);
	USHORT (*write_multi_tx_resource)(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR frameNum, USHORT *FreeNumber);
	VOID (*write_final_tx_resource)(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, USHORT totalMPDUSize, USHORT FirstTxIdx);
	USHORT (*write_frag_tx_resource)(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR fragNum, USHORT *FreeNumber);
	INT (*mlme_hw_tx)(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *pTransmit, struct _TX_BLK *tx_blk);
#ifdef CONFIG_ATE
	INT32 (*ate_hw_tx)(struct _RTMP_ADAPTER *pAd, struct _TMAC_INFO *info, struct _TX_BLK *tx_blk);
#endif
	VOID (*write_tmac_info_fixed_rate)(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *pTransmit);
	VOID (*dump_tmac_info)(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info);


	/*  RX */
	BOOLEAN (*rx_done_handle)(struct _RTMP_ADAPTER *pAd) ____cacheline_aligned;
	VOID *(*get_pkt_from_rx_resource)(struct _RTMP_ADAPTER *pAd, BOOLEAN *re_schedule,
							UINT32 *rx_pending, UCHAR ring_no);
	VOID *(*get_pkt_from_rx1_resource)(struct _RTMP_ADAPTER *pAd, BOOLEAN *re_schedule,
							UINT32 *rx_pending, UCHAR ring_no);
	UINT32 (*rx_pkt_process)(struct _RTMP_ADAPTER *pAd, UINT8 hif_idx, struct _RX_BLK *rx_blk, VOID *rx_pkt);
#ifdef MT7626_E2_SUPPORT
	UINT32 (*rx2_pkt_process)(struct _RTMP_ADAPTER *pAd, UINT8 hif_idx, struct _RX_BLK *rx_blk, VOID *rx_pkt);
	BOOLEAN (*rx2_done_handle)(struct _RTMP_ADAPTER *pAd) ____cacheline_aligned;
	VOID *(*get_pkt_from_rx2_resource)(struct _RTMP_ADAPTER *pAd, BOOLEAN *re_schedule,
									   UINT32 *rx_pending, UCHAR ring_no);
#endif
	UINT32 (*get_packet_type)(struct _RTMP_ADAPTER *pAd, VOID *rx_packet);
	INT32 (*trans_rxd_into_rxblk)(RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, VOID *rx_pkt);
	VOID (*dump_rmac_info_normal)(struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info);
	VOID (*dump_rmac_info_rxv)(RTMP_ADAPTER *pAd, UCHAR *Data);
#ifdef IGMP_SNOOP_SUPPORT
	BOOLEAN (*archMcastEntryInsert)(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr, PNET_DEV dev, UINT8 WlanIndex);
	BOOLEAN (*archMcastEntryDelete)(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev, UINT8 WlanIndex);
#endif
	INT (*asic_rts_on_off)(
		struct _RTMP_ADAPTER *ad,
		UCHAR band_idx,
		UINT32 rts_num,
		UINT32 rts_len,
		BOOLEAN rts_en);
	INT (*asic_ampdu_efficiency_on_off)(struct _RTMP_ADAPTER *ad, UCHAR	wmm_idx, UCHAR aifs_adjust);
#ifdef RED_SUPPORT
	bool (*archRedMarkPktDrop)(UINT8 ucWlanIdx, UINT8 ucQidx, struct _RTMP_ADAPTER *pAd);
#endif
	VOID (*arch_bss_beacon_exit)(struct _RTMP_ADAPTER *pAd);
	VOID (*arch_bss_beacon_stop)(struct _RTMP_ADAPTER *pAd);
	VOID (*arch_bss_beacon_start)(struct _RTMP_ADAPTER *pAd);
	VOID (*arch_bss_beacon_init)(struct _RTMP_ADAPTER *pAd);

#ifdef DYNAMIC_STEERING_LOADING
	INT (*arch_combo_tx_rps_check)(struct _RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket, struct wifi_dev *wdev);
	VOID (*arch_combo_tx_rps_algo)(struct _RTMP_ADAPTER *pAd);
	INT (*arch_combo_rx_rps_check)(struct _RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket);
#endif /* DYNAMIC_STEERING_LOADING */
	VOID (*arch_truncate_algo)(struct _RTMP_ADAPTER *pAd);

#if defined(NF_SUPPORT) || defined(OFFCHANNEL_SCAN_FEATURE)
	VOID (*arch_calculate_nf)(RTMP_ADAPTER *pAd, UCHAR bandidx);
	VOID (*arch_reset_enable_nf_registers)(RTMP_ADAPTER *pAd, UCHAR bandidx);
	VOID (*arch_enable_nf_support)(struct _RTMP_ADAPTER *pAd);
#endif


} RTMP_ARCH_OP;

#ifdef LINK_TEST_SUPPORT
VOID
LinkTestRcpiSet(
	struct _RTMP_ADAPTER *pAd,
	UCHAR u1WlanId,
	UINT8 u1AntIdx,
	CHAR i1Rcpi
	);

VOID
LinkTestPeriodHandler(
	struct _RTMP_ADAPTER *pAd
	);

VOID
LinkTestTimeSlotLinkHandler(
	struct _RTMP_ADAPTER *pAd
	);

VOID
LinkTestStaLinkUpHandler(
	struct _RTMP_ADAPTER *pAd,
	struct _MAC_TABLE_ENTRY *pEntry
	);

VOID
LinkTestApClientLinkUpHandler(
	struct _RTMP_ADAPTER *pAd
	);

BOOLEAN
LinkTestInstrumentCheck(
	struct _RTMP_ADAPTER *pAd,
	struct _MAC_TABLE_ENTRY *pEntry
	);

VOID
LinkTestChannelBandUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT8 u1ControlChannel
	);

VOID
LinkTestChannelSwitchHandler(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx
	);

VOID
LinkTestRxCntCheck(
	struct _RTMP_ADAPTER *pAd
	);

VOID
LinkTestRxStreamCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgCmwLinkStatus,
	UINT8 u1WlanId
	);

VOID
LinkTestDCRFCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgDCRFenable
	);

VOID
LinkTestRxStreamTrans(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT8 u1SpeRssiIdx
	);

VOID
LinkTestTxBwSwitch(
	struct _RTMP_ADAPTER *pAd,
	struct _MAC_TABLE_ENTRY *pEntry
	);

VOID
LinkTestTxBwRestore(
	struct _RTMP_ADAPTER *pAd,
	struct _MAC_TABLE_ENTRY *pEntry
	);

VOID
LinkTestSpeIdxCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgCmwLinkStatus,
	UINT8 u1BandIdx
	);

VOID
LinkTestTxBwCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgCmwLinkStatus,
	struct _MAC_TABLE_ENTRY *pEntry
	);

VOID
LinkTestTxCsdCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgCmwLinkStatus,
	UINT8 u1BandIdx
	);

VOID
LinkTestTxPowerCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgCmwLinkStatus,
	UINT8 u1BandIdx
	);

VOID
LinkTestRcpiCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgCmwLinkStatus,
	UINT8 u1BandIdx
	);

VOID
LinkTestAcrCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgCmwLinkStatus,
	UINT8 u1WlanId,
	UINT8 u1BandIdx
	);

VOID
LinkTestRssiGet(
	struct _RTMP_ADAPTER *pAd,
	ENUM_RSSI_CHECK_SOURCE eRssiSrc,
	UINT8 u1WlanId,
	PCHAR pi1Rssi
	);

VOID
LinkTestRssiCheck(
	struct _RTMP_ADAPTER *pAd,
	PCHAR pi1Rssi,
	UINT8 u1BandIdx,
	PUINT8 pu1SpeRssiIdx,
	PUINT8 pu1RssiReason,
	UINT8 u1WlanId
	);

UINT8
LinkTestRssiCheckItem(
	struct _RTMP_ADAPTER *pAd,
	PCHAR pi1Rssi,
	UINT8 u1BandIdx,
	ENUM_RSSI_CHECK_SOURCE eRssiCheckSrc,
	UINT8 u1WlanId
	);

UINT8
LinkTestRssiComp(
	struct _RTMP_ADAPTER *pAd,
	PCHAR pi1Rssi,
	UINT8 u1RssiNum,
	ENUM_RSSI_CHECK_SOURCE eRssiCheckSrc
	);

UINT8
LinkTestRssiSpecificRxPath(
	struct _RTMP_ADAPTER *pAd,
	PCHAR pi1Rssi,
	PUINT8 pu1RxPathRssiOrder,
	UINT8 u1RssiNum,
	ENUM_RSSI_CHECK_SOURCE eRssiCheckSrc
	);

VOID
LinkTestSwap(
	PCHAR pi1Value1,
	PCHAR pi1Value2
	);
#endif /* LINK_TEST_SUPPORT */


BOOLEAN asic_bss_beacon_exit(struct _RTMP_ADAPTER *pAd);
BOOLEAN asic_bss_beacon_stop(struct _RTMP_ADAPTER *pAd);
BOOLEAN asic_bss_beacon_start(struct _RTMP_ADAPTER *pAd);
BOOLEAN asic_bss_beacon_init(struct _RTMP_ADAPTER *pAd);


#ifdef	ETSI_RX_BLOCKER_SUPPORT
UINT8	ETSIWbRssiCheck(RTMP_ADAPTER *pAd);
#endif /* ETSI_RX_BLOCKER_SUPPORT */

VOID asic_write_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
USHORT asic_write_tx_resource(struct _RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk, BOOLEAN bIsLast, USHORT *freeCnt);
VOID asic_write_tmac_info_fixed_rate(struct _RTMP_ADAPTER *pAd,
	UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *pTransmit);
INT32 asic_write_txp_info(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
UCHAR *asic_get_hif_buf(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk, UINT8 hif_idx, UCHAR frame_type);
UINT32 asic_get_resource_idx(struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev, enum PACKET_TYPE pkt_type, UINT8 que_idx);
INT asic_check_hw_resource(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR resource_idx);
VOID asic_set_resource_state(struct _RTMP_ADAPTER *pAd, UCHAR resource_idx, BOOLEAN state);

UINT32 asic_get_tx_resource_free_num(struct _RTMP_ADAPTER *pAd, UINT8 que_idx);
VOID *asic_get_pkt_from_rx_resource(struct _RTMP_ADAPTER *pAd, BOOLEAN *re_schedule,
						UINT32 *rx_pending, UCHAR ring_no);
UINT32 asic_rx_pkt_process(struct _RTMP_ADAPTER *pAd, UINT8 hif_idx, struct _RX_BLK *pRxBlk, VOID *pRxPacket);
#ifdef MT7626_E2_SUPPORT
UINT32 asic_rx2_pkt_process(struct _RTMP_ADAPTER *pAd, UINT8 hif_idx, struct _RX_BLK *pRxBlk, VOID *pRxPacket);
VOID *asic_get_pkt_from_rx2_resource(struct _RTMP_ADAPTER *pAd, BOOLEAN *re_schedule,
									UINT32 *rx_pending, UCHAR ring_no);
#endif
UINT32 asic_get_packet_type(struct _RTMP_ADAPTER *pAd, VOID *rx_packet);
INT32 asic_trans_rxd_into_rxblk(RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, VOID *rx_pkt);
VOID asic_kickout_data_tx(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk, UCHAR que_idx);
INT asic_mlme_hw_tx(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *pTransmit, struct _TX_BLK *tx_blk);
INT asic_hw_tx(struct _RTMP_ADAPTER *ad, struct _TX_BLK *tx_blk);
VOID asic_dump_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info);

BOOLEAN asic_rx_done_handle(struct _RTMP_ADAPTER *pAd);
#ifdef MT7626_E2_SUPPORT
BOOLEAN asic_rx2_done_handle(struct _RTMP_ADAPTER *pAd);
#endif
BOOLEAN asic_tx_dma_done_handle(struct _RTMP_ADAPTER *pAd, UINT8 hif_idx);

#if defined(NF_SUPPORT) || defined(OFFCHANNEL_SCAN_FEATURE)

BOOLEAN asic_calculate_nf(struct _RTMP_ADAPTER *ad, UCHAR band_idx);

BOOLEAN asic_reset_enable_nf_registers(struct _RTMP_ADAPTER *ad, UCHAR band_idx);

BOOLEAN asic_enable_nf_support(struct _RTMP_ADAPTER *ad);
#endif


#endif /* __ASIC_CTRL_H_ */
