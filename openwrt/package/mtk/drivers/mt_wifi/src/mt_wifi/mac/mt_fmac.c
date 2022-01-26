/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2017, MediaTek, Inc.
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
#include "mac/mac_mt/fmac/mt_fmac.h"

extern UCHAR tmi_rate_map_cck_lp[];
extern UCHAR tmi_rate_map_cck_lp_size;
extern UCHAR tmi_rate_map_cck_sp[];
extern UCHAR tmi_rate_map_cck_sp_size;
extern UCHAR tmi_rate_map_ofdm[];
extern UCHAR tmi_rate_map_ofdm_size;
extern char *pkt_ft_str[];
extern char *hdr_fmt_str[];

const UCHAR wmm_aci_2_hw_ac_queue[] = {
	TxQ_IDX_AC1, /* ACI:0 AC_BE */
	TxQ_IDX_AC0, /* ACI:1 AC_BK */
	TxQ_IDX_AC2, /* ACI:2 AC_VI */
	TxQ_IDX_AC3, /* ACI:3 AC_VO */
	TxQ_IDX_AC11,
	TxQ_IDX_AC10,
	TxQ_IDX_AC12,
	TxQ_IDX_AC13,
	TxQ_IDX_AC21,
	TxQ_IDX_AC20,
	TxQ_IDX_AC22,
	TxQ_IDX_AC23,
	TxQ_IDX_AC31,
	TxQ_IDX_AC30,
	TxQ_IDX_AC32,
	TxQ_IDX_AC33,
	TxQ_IDX_ALTX0, /*16*/
	TxQ_IDX_BMC0,
	TxQ_IDX_BCN0,
	TxQ_IDX_PSMP0,
	TxQ_IDX_ALTX1, /*20*/
	TxQ_IDX_BMC1,
	TxQ_IDX_BCN1,
	TxQ_IDX_PSMP1,
};


VOID dump_rxinfo(RTMP_ADAPTER *pAd, RXINFO_STRUC *pRxInfo)
{
	hex_dump("RxInfo Raw Data", (UCHAR *)pRxInfo, sizeof(RXINFO_STRUC));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxInfo Fields:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBA=%d\n", pRxInfo->BA));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDATA=%d\n", pRxInfo->DATA));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tNULLDATA=%d\n", pRxInfo->NULLDATA));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFRAG=%d\n", pRxInfo->FRAG));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tU2M=%d\n", pRxInfo->U2M));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMcast=%d\n", pRxInfo->Mcast));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBcast=%d\n", pRxInfo->Bcast));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMyBss=%d\n", pRxInfo->MyBss));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCrc=%d\n", pRxInfo->Crc));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCipherErr=%d\n", pRxInfo->CipherErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAMSDU=%d\n", pRxInfo->AMSDU));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tHTC=%d\n", pRxInfo->HTC));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRSSI=%d\n", pRxInfo->RSSI));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tL2PAD=%d\n", pRxInfo->L2PAD));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAMPDU=%d\n", pRxInfo->AMPDU));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDecrypted=%d\n", pRxInfo->Decrypted));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBssIdx3=%d\n", pRxInfo->BssIdx3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twapi_kidx=%d\n", pRxInfo->wapi_kidx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpn_len=%d\n", pRxInfo->pn_len));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsw_fc_type0=%d\n", pRxInfo->sw_fc_type0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsw_fc_type1=%d\n", pRxInfo->sw_fc_type1));
}

static char *rmac_info_type_str[] = {
	"TXS",
	"RXV",
	"RxNormal",
	"DupRFB",
	"TMR",
	"Unknown",
};

static inline char *rxd_pkt_type_str(INT pkt_type)
{
	if (pkt_type <= 0x04)
		return rmac_info_type_str[pkt_type];
	else
		return rmac_info_type_str[5];
}


VOID dump_rmac_info_normal(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)rmac_info;

	hex_dump("RMAC_Info Raw Data: ", rmac_info, sizeof(RXD_BASE_STRUCT));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxData_BASE:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPktType=%d(%s)\n",
			 rxd_base->RxD0.PktType,
			 rxd_pkt_type_str(rxd_base->RxD0.PktType)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tGroupValid=%x\n", rxd_base->RxD0.RfbGroupVld));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tRxByteCnt=%d\n", rxd_base->RxD0.RxByteCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tIP/UT=%d/%d\n", rxd_base->RxD0.IpChkSumOffload, rxd_base->RxD0.UdpTcpChkSumOffload));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEtherTypeOffset=%d(WORD)\n", rxd_base->RxD0.EthTypeOffset));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHTC/UC2ME/MC/BC=%d/%d/%d/%d\n",
			 rxd_base->RxD1.HTC,
			 (rxd_base->RxD1.a1_type == 0x1 ? 1 : 0),
			 (rxd_base->RxD1.a1_type == 0x2 ? 1 : 0),
			 rxd_base->RxD1.a1_type == 0x3 ? 1 : 0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBeacon with BMCast/Ucast=%d/%d\n",
			 rxd_base->RxD1.BcnWithBMcst, rxd_base->RxD1.BcnWithUCast));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tKeyID=%d\n", rxd_base->RxD1.KeyId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tChFrequency=%x\n", rxd_base->RxD1.ChFreq));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdearLength(MAC)=%d\n", rxd_base->RxD1.MacHdrLen));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHeaerOffset(HO)=%d\n", rxd_base->RxD1.HdrOffset));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHeaerTrans(H)=%d\n", rxd_base->RxD1.HdrTranslation));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPayloadFormat(PF)=%d\n", rxd_base->RxD1.PayloadFmt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBSSID=%d\n", rxd_base->RxD1.RxDBssidIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tWlanIndex=%d\n", rxd_base->RxD2.RxDWlanIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTID=%d\n", rxd_base->RxD2.RxDTid));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSEC Mode=%d\n", rxd_base->RxD2.SecMode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSW BIT=%d\n", rxd_base->RxD2.SwBit));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFCE Error(FC)=%d\n", rxd_base->RxD2.FcsErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tCipher Mismatch(CM)=%d\n", rxd_base->RxD2.CipherMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tCipher Length Mismatch(CLM)=%d\n", rxd_base->RxD2.CipherLenMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tICV Err(I)=%d\n", rxd_base->RxD2.IcvErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTKIP MIC Err(T)=%d\n", rxd_base->RxD2.TkipMicErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tLength Mismatch(LM)=%d\n", rxd_base->RxD2.LenMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tDeAMSDU Fail(DAF)=%d\n", rxd_base->RxD2.DeAmsduFail));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tExceedMax Rx Length(EL)=%d\n", rxd_base->RxD2.ExMaxRxLen));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrTransFail(HTF)=%d\n", rxd_base->RxD2.HdrTransFail));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tInterested Frame(INTF)=%d\n", rxd_base->RxD2.InterestedFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFragment Frame(FRAG)=%d\n", rxd_base->RxD2.FragFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNull Frame(NULL)=%d\n", rxd_base->RxD2.NullFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon Data Frame(NDATA)=%d\n", rxd_base->RxD2.NonDataFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon-AMPDU Subframe(NASF)=%d\n", rxd_base->RxD2.NonAmpduSfrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon AMPDU(NAMP)=%d\n", rxd_base->RxD2.NonAmpduFrm));
}

VOID dump_rmac_info_for_ICVERR(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)rmac_info;
	union _RMAC_RXD_0_UNION *rxd_0;
	UINT32 pkt_type;
	int LogDbgLvl = DBG_LVL_ERROR;

	if (!IS_HIF_TYPE(pAd, HIF_MT))
		return;

	rxd_0 = (union _RMAC_RXD_0_UNION *)rmac_info;
	pkt_type = RMAC_RX_PKT_TYPE(rxd_0->word);

	if (pkt_type != RMAC_RX_PKT_TYPE_RX_NORMAL)
		return;


	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ("\tHTC/UC2ME/MC/BC=%d/%d/%d/%d",
				rxd_base->RxD1.HTC,
				(rxd_base->RxD1.a1_type == 0x1 ? 1 : 0),
				(rxd_base->RxD1.a1_type == 0x2 ? 1 : 0),
				rxd_base->RxD1.a1_type == 0x3 ? 1 : 0));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", WlanIndex=%d", rxd_base->RxD2.RxDWlanIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", SEC Mode=%d\n", rxd_base->RxD2.SecMode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ("\tFCE Error(FC)=%d", rxd_base->RxD2.FcsErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", CM=%d", rxd_base->RxD2.CipherMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", CLM=%d", rxd_base->RxD2.CipherLenMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", I=%d", rxd_base->RxD2.IcvErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", T=%d", rxd_base->RxD2.TkipMicErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", LM=%d\n", rxd_base->RxD2.LenMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ("\tFragment Frame(FRAG)=%d\n", rxd_base->RxD2.FragFrm));

}


VOID dump_rmac_info_txs(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* TXS_FRM_STRUC *txs_frm = (TXS_FRM_STRUC *)rmac_info; */
}


VOID dump_rmac_info_rxv(RTMP_ADAPTER *pAd, UCHAR *Data)
{
	RXV_DWORD0 *DW0 = NULL;
	RXV_DWORD1 *DW1 = NULL;

	RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = NULL;
	RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = NULL;
	RX_VECTOR1_3TH_CYCLE *RXV1_3TH_CYCLE = NULL;
	RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = NULL;
	RX_VECTOR1_5TH_CYCLE *RXV1_5TH_CYCLE = NULL;
	RX_VECTOR1_6TH_CYCLE *RXV1_6TH_CYCLE = NULL;

	RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = NULL;
	RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = NULL;
	RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = NULL;

	DW0 = (RXV_DWORD0 *)Data;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TA_0_31=%d\n", DW0->TA_0_31));

	DW1 = (RXV_DWORD1 *)(Data + 4);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TA_32_47=%d", DW1->TA_32_47));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxvSn=%d", DW1->RxvSn));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TR=%d\n", DW1->TR));


	RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(Data + 8);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TxRate=%d", RXV1_1ST_CYCLE->TxRate));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtStbc=%d", RXV1_1ST_CYCLE->HtStbc));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtAdCode=%d", RXV1_1ST_CYCLE->HtAdCode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtExtltf=%d", RXV1_1ST_CYCLE->HtExtltf));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TxMode=%d", RXV1_1ST_CYCLE->TxMode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FrMode=%d\n", RXV1_1ST_CYCLE->FrMode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", VHTA1_B22=%d", RXV1_1ST_CYCLE->VHTA1_B22));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtAggregation=%d", RXV1_1ST_CYCLE->HtAggregation));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtShortGi=%d", RXV1_1ST_CYCLE->HtShortGi));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtSmooth=%d", RXV1_1ST_CYCLE->HtSmooth));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtNoSound=%d", RXV1_1ST_CYCLE->HtNoSound));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", NumRx=%d\n", RXV1_1ST_CYCLE->NumRx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", VHTA2_B8_B3=%d", RXV1_1ST_CYCLE->VHTA2_B8_B3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", ACID_DET_LOWER=%d", RXV1_1ST_CYCLE->ACID_DET_LOWER));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", ACID_DET_UPPER=%d\n", RXV1_1ST_CYCLE->ACID_DET_UPPER));

	RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 12);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Length=%d", RXV1_2ND_CYCLE->Length));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", GroupId=%d", RXV1_2ND_CYCLE->GroupId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", NstsField=%d", RXV1_2ND_CYCLE->NstsField));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxValidIndicator=%d", RXV1_2ND_CYCLE->RxValidIndicator));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", SelAnt=%d\n", RXV1_2ND_CYCLE->SelAnt));

	RXV1_3TH_CYCLE = (RX_VECTOR1_3TH_CYCLE *)(Data + 16);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", VHTA1_B21_B10=%d", RXV1_3TH_CYCLE->VHTA1_B21_B10));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", BFAgcApply=%d", RXV1_3TH_CYCLE->BFAgcApply));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", POPEverTrig=%d", RXV1_3TH_CYCLE->POPEverTrig));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FgacCalLnaRx=%d", RXV1_3TH_CYCLE->FgacCalLnaRx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", IBRssiRx=%d", RXV1_3TH_CYCLE->IBRssiRx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", WBRssiRx=%d\n", RXV1_3TH_CYCLE->WBRssiRx));

	RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(Data + 20);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RCPI0=%d", RXV1_4TH_CYCLE->RCPI0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RCPI1=%d", RXV1_4TH_CYCLE->RCPI1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RCPI2=%d", RXV1_4TH_CYCLE->RCPI2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RCPI3=%d\n", RXV1_4TH_CYCLE->RCPI3));

	RXV1_5TH_CYCLE = (RX_VECTOR1_5TH_CYCLE *)(Data + 24);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FagcLnaGainx=%d", RXV1_5TH_CYCLE->FagcLnaGainx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FagcLpfGainx=%d", RXV1_5TH_CYCLE->FagcLpfGainx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", NoiseBalanceEnable=%d", RXV1_5TH_CYCLE->NoiseBalanceEnable));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", MISC1=%d\n", RXV1_5TH_CYCLE->MISC1));

	RXV1_6TH_CYCLE = (RX_VECTOR1_6TH_CYCLE *)(Data + 28);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Nf0=%d", RXV1_6TH_CYCLE->Nf0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Nf1=%d", RXV1_6TH_CYCLE->Nf1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Nf2=%d", RXV1_6TH_CYCLE->Nf2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Nf3=%d\n", RXV1_6TH_CYCLE->Nf3));

	RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(Data + 32);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", PrimItfrEnv=%d", RXV2_1ST_CYCLE->PrimItfrEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", SecItfrEnv=%d", RXV2_1ST_CYCLE->SecItfrEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Sec40ItfrEnv=%d", RXV2_1ST_CYCLE->Sec40ItfrEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Sec80ItfrEnv=%d", RXV2_1ST_CYCLE->Sec80ItfrEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxLQ=%d", RXV2_1ST_CYCLE->RxLQ));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", BtEnv=%d", RXV2_1ST_CYCLE->BtEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", ScrambleSeed=%d\n", RXV2_1ST_CYCLE->ScrambleSeed));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", VHT_A2=%d", RXV2_1ST_CYCLE->VHT_A2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxCERmsd=%d", RXV2_1ST_CYCLE->RxCERmsd));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FCSErr=%d\n", RXV2_1ST_CYCLE->FCSErr));

	RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(Data + 36);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", PostTMD=%d", RXV2_2ND_CYCLE->PostTMD));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxBW=%d", RXV2_2ND_CYCLE->RxBW));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", PostBWDSecCh=%d", RXV2_2ND_CYCLE->PostBWDSecCh));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", PostDewSecCh=%d", RXV2_2ND_CYCLE->PostDewSecCh));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtSTFDet=%d", RXV2_2ND_CYCLE->HtSTFDet));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", CagcSTFDet=%d", RXV2_2ND_CYCLE->CagcSTFDet));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", IBRssi0=%d", RXV2_2ND_CYCLE->IBRssi0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", WBRssi0=%d", RXV2_2ND_CYCLE->WBRssi0));

	RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(Data + 40);
}


VOID dump_rmac_info_rfb(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* RXD_BASE_STRUCT *rfb_frm = (RXD_BASE_STRUCT *)rmac_info; */
}


VOID dump_rmac_info_tmr(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* TMR_FRM_STRUC *rxd_base = (TMR_FRM_STRUC *)rmac_info; */
}


VOID dump_rmac_info(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	union _RMAC_RXD_0_UNION *rxd_0;
	UINT32 pkt_type;

	rxd_0 = (union _RMAC_RXD_0_UNION *)rmac_info;
	pkt_type = RMAC_RX_PKT_TYPE(rxd_0->word);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RMAC_RXD Header Format :%s\n",
			 rxd_pkt_type_str(pkt_type)));

	switch (pkt_type) {
	case RMAC_RX_PKT_TYPE_RX_TXS:
		dump_rmac_info_txs(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_TXRXV:
		dump_rmac_info_rxv(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_NORMAL:
		dump_rmac_info_normal(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_DUP_RFB:
		dump_rmac_info_rfb(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_TMR:
		dump_rmac_info_tmr(pAd, rmac_info);
		break;

	default:
		break;
	}
}


VOID DumpTxSFormat(RTMP_ADAPTER *pAd, UINT8 Format, CHAR *Data)
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
	TXS_D_1 *TxSD1 = &txs_entry->TxSD1;
	TXS_D_2 *TxSD2 = &txs_entry->TxSD2;
	TXS_D_3 *TxSD3 = &txs_entry->TxSD3;
	/* TXS_D_4 *TxSD4 = &txs_entry->TxSD4; */
	/* TXS_D_5 *TxSD5 = &txs_entry->TxSD5; */
	/* TXS_D_6 *TxSD6 = &txs_entry->TxSD6; */

	if (Format == TXS_FORMAT0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tType=TimeStamp/FrontTime Mode\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tTXSFM=%d, TXS2M/H=%d/%d, FixRate=%d, TxRate/BW=0x%x/%d\n",
				  TxSD0->TxSFmt, TxSD0->TxS2M, TxSD0->TxS2H,
				  TxSD0->TxS_FR, TxSD0->TxRate, TxSD1->TxS_TxBW));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tME/RE/LE/BE/TxOPLimitErr/BA-Fail=%d/%d/%d/%d/%d/%d, PS=%d, Pid=%d\n",
				  TxSD0->ME, TxSD0->RE, TxSD0->LE, TxSD0->BE, TxSD0->TxOp,
				  TxSD0->BAFail, TxSD0->PSBit, TxSD0->TxS_PId));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tTid=%d, AntId=%d, ETxBF/ITxBf=%d/%d\n",
				  TxSD1->TxS_Tid, TxSD1->TxS_AntId,
				  TxSD1->TxS_ETxBF, TxSD1->TxS_ITxBF));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tTxPwrdBm=0x%x, FinalMPDU=0x%x, AMPDU=0x%x\n",
				  TxSD1->TxPwrdBm, TxSD1->TxS_FianlMPDU, TxSD1->TxS_AMPDU));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tTxDelay=0x%x, RxVSeqNum=0x%x, Wlan Idx=0x%x\n",
				  TxSD2->TxS_TxDelay, TxSD2->TxS_RxVSN, TxSD2->TxS_WlanIdx));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tSN=0x%x, MPDU TxCnt=%d, LastTxRateIdx=%d\n",
				  TxSD3->type_0.TxS_SN, TxSD3->type_0.TxS_MpduTxCnt,
				  TxSD3->type_0.TxS_LastTxRateIdx));
	} else if (Format == TXS_FORMAT1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tType=Noisy/RCPI Mode\n"));
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Unknown TxSFormat(%d)\n", __func__, Format));
	}
}

INT dump_dmac_amsdu_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#define MSDU_CNT_CR_NUMBER 8
	UINT32 msdu_cnt[MSDU_CNT_CR_NUMBER] = {0};
	UINT idx = 0;
	PMAC_TABLE_ENTRY pEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;

	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_1_MSDU_CNT, &msdu_cnt[0]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_2_MSDU_CNT, &msdu_cnt[1]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_3_MSDU_CNT, &msdu_cnt[2]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_4_MSDU_CNT, &msdu_cnt[3]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_5_MSDU_CNT, &msdu_cnt[4]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_6_MSDU_CNT, &msdu_cnt[5]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_7_MSDU_CNT, &msdu_cnt[6]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_8_MSDU_CNT, &msdu_cnt[7]);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=== HW_AMSDU INFO.===\n"));
	for (idx = 0; idx < MSDU_CNT_CR_NUMBER; idx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PACK_%d_MSDU_CNT=%d\n", (idx+1), msdu_cnt[idx]));
	}
	for (idx = 0; VALID_UCAST_ENTRY_WCID(pAd, idx); idx++) {
		pEntry = &pAd->MacTab.Content[idx];
		tr_entry = &pAd->MacTab.tr_entry[idx];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		if ((pEntry->Sst == SST_ASSOC) &&
			(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) &&
			(pEntry->tx_amsdu_bitmap != 0)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Wcid%03d: %02x", idx, pEntry->tx_amsdu_bitmap));
		}
	}
	return TRUE;
}

VOID Update_Mib_Bucket_One_Sec(RTMP_ADAPTER *pAd)
{
	UCHAR   i = 0, j = 0;
	UCHAR concurrent_bands = HcGetAmountOfBand(pAd);

	for (i = 0 ; i < concurrent_bands ; i++) {
		if (pAd->OneSecMibBucket.Enabled[i] == TRUE) {
			pAd->OneSecMibBucket.ChannelBusyTimeCcaNavTx[i] = 0;
			pAd->OneSecMibBucket.ChannelBusyTime[i] = 0;
			pAd->OneSecMibBucket.OBSSAirtime[i] = 0;
			pAd->OneSecMibBucket.MyTxAirtime[i] = 0;
			pAd->OneSecMibBucket.MyRxAirtime[i] = 0;
			pAd->OneSecMibBucket.EDCCAtime[i] =  0;
			pAd->OneSecMibBucket.MdrdyCount[i] = 0;
			pAd->OneSecMibBucket.PdCount[i] = 0;
			for (j = 0 ; j < 2 ; j++) {
				pAd->OneSecMibBucket.ChannelBusyTime[i] +=
					pAd->MsMibBucket.ChannelBusyTime[i][j];
				pAd->OneSecMibBucket.ChannelBusyTimeCcaNavTx[i] += pAd->MsMibBucket.ChannelBusyTimeCcaNavTx[i][j];
				pAd->OneSecMibBucket.OBSSAirtime[i] += pAd->MsMibBucket.OBSSAirtime[i][j];
				pAd->OneSecMibBucket.MyTxAirtime[i] += pAd->MsMibBucket.MyTxAirtime[i][j];
				pAd->OneSecMibBucket.MyRxAirtime[i] += pAd->MsMibBucket.MyRxAirtime[i][j];
				pAd->OneSecMibBucket.EDCCAtime[i] += pAd->MsMibBucket.EDCCAtime[i][j];
				pAd->OneSecMibBucket.MdrdyCount[i] += pAd->MsMibBucket.MdrdyCount[i][j];
				pAd->OneSecMibBucket.PdCount[i] += pAd->MsMibBucket.PdCount[i][j];
			}
		}
	}
}

VOID Update_Mib_Bucket_500Ms(RTMP_ADAPTER *pAd)
{
	UINT32  CrValue;
	UCHAR   i = 0;
	UCHAR   CurrIdx = 0;
	UCHAR concurrent_bands = HcGetAmountOfBand(pAd);

	pAd->MsMibBucket.CurIdx++;
	if (pAd->MsMibBucket.CurIdx >= 2)
		pAd->MsMibBucket.CurIdx = 0;
	CurrIdx = pAd->MsMibBucket.CurIdx;
	for (i = 0; i < concurrent_bands; i++) {
		if (pAd->MsMibBucket.Enabled == TRUE) {
			/* Channel Busy Time */
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR9 + (i * BandOffset), &CrValue);
			pAd->MsMibBucket.ChannelBusyTimeCcaNavTx[i][CurrIdx] = CrValue;
			/* Primary Channel Busy Time */
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR16 + (i * BandOffset), &CrValue);
			pAd->MsMibBucket.ChannelBusyTime[i][CurrIdx] = CrValue;
			/* OBSS Air time */
			HW_IO_READ32(pAd->hdev_ctrl, RMAC_MIBTIME5 + i * 4, &CrValue);
			pAd->MsMibBucket.OBSSAirtime[i][CurrIdx] = CrValue;
			/* My Tx Air time */
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR36 + (i * BandOffset), &CrValue);
			pAd->MsMibBucket.MyTxAirtime[i][CurrIdx] = CrValue;
			/* My Rx Air time */
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR37 + (i * BandOffset), &CrValue);
			pAd->MsMibBucket.MyRxAirtime[i][CurrIdx] = CrValue;
			/* EDCCA time */
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR18 + (i * BandOffset), &CrValue);
			pAd->MsMibBucket.EDCCAtime[i][CurrIdx] = CrValue;
			/* Reset OBSS Air time */
			HW_IO_READ32(pAd->hdev_ctrl, RMAC_MIBTIME0, &CrValue);
			CrValue |= 1 << RX_MIBTIME_CLR_OFFSET;
			CrValue |= 1 << RX_MIBTIME_EN_OFFSET;
			HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MIBTIME0, CrValue);
			HW_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS0 + (i * BandOffset), &CrValue); /* PD count */
			pAd->MsMibBucket.PdCount[i][CurrIdx] = CrValue;
			HW_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS5 + (i * BandOffset), &CrValue); /* MDRDY count */
			pAd->MsMibBucket.MdrdyCount[i][CurrIdx] = CrValue;
			HW_IO_READ32(pAd->hdev_ctrl, PHY_BAND0_PHYMUX_5 + (i * BandOffset), &CrValue);
			CrValue &= 0xff8fffff;
			HW_IO_WRITE32(pAd->hdev_ctrl, PHY_BAND0_PHYMUX_5 + (i * BandOffset), CrValue); /* Reset */
			CrValue |= 0x500000;
			HW_IO_WRITE32(pAd->hdev_ctrl, PHY_BAND0_PHYMUX_5 + (i * BandOffset), CrValue); /* Enable */
		}
	}
}

static RTMP_REG_PAIR mac_cr_seg[] = {
	{0x20000, 0x20010}, /* WF_CFG */
	{WF_TRB_BASE, 0x21040}, /* WF_CFG */
	{WF_AGG_BASE, 0x21240}, /* WF_CFG */
	{WF_ARB_BASE, 0x21440}, /* WF_CFG */
	{0, 0},
};


VOID dump_mt_mac_cr(RTMP_ADAPTER *pAd)
{
	INT index = 0;
	UINT32 mac_val, mac_addr, seg_s, seg_e;

	while (mac_cr_seg[index].Register != 0) {
		seg_s = mac_cr_seg[index].Register;
		seg_e = mac_cr_seg[index].Value;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WF_CFG Segment(Start=0x%x, End=0x%x)\n",
				 seg_s, seg_e));

		for (mac_addr = seg_s; mac_addr < seg_e; mac_addr += 4) {
			MAC_IO_READ32(pAd->hdev_ctrl, mac_addr, &mac_val);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MAC[0x%x] = 0x%x\n", mac_addr, mac_val));
		}

		index++;
	};
}


INT mt_mac_fifo_stat_update(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
	return FALSE;
}


VOID NicGetTxRawCounters(
	IN RTMP_ADAPTER *pAd,
	IN TX_STA_CNT0_STRUC * pStaTxCnt0,
	IN TX_STA_CNT1_STRUC * pStaTxCnt1)
{
	/* TODO: shiang-7603 */
	return;
}


/*
	========================================================================

	Routine Description:
		Read statistical counters from hardware registers and record them
		in software variables for later on query

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	========================================================================
*/
static VOID NICUpdateAmpduRawCounters(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	/* for PER debug */
	UINT32 AmpduTxCount = 0;
	UINT32 AmpduTxSuccessCount = 0;
	COUNTER_802_11 *wlanCounter;
	UINT32 mac_val, ampdu_range_cnt[4];
	UINT32 Offset = 0x200 * BandIdx;

	wlanCounter = &pAd->WlanCounters[BandIdx];
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR14 + Offset, &AmpduTxCount);
	AmpduTxCount &= 0xFFFFFF;
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR15 + Offset, &AmpduTxSuccessCount);
	AmpduTxSuccessCount &= 0xFFFFFF;
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0DR2 + Offset, &mac_val);
	ampdu_range_cnt[0] = mac_val & 0xffff;
	ampdu_range_cnt[1] =  (mac_val >> 16) & 0xffff;
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0DR3 + Offset, &mac_val);
	ampdu_range_cnt[2] = mac_val & 0xffff;
	ampdu_range_cnt[3] =  (mac_val >> 16) & 0xffff;
#ifdef STATS_COUNT_SUPPORT
	wlanCounter->AmpduSuccessCount.u.LowPart += AmpduTxSuccessCount;
	wlanCounter->AmpduFailCount.u.LowPart += (AmpduTxCount - AmpduTxSuccessCount);
#endif /* STATS_COUNT_SUPPORT */
	wlanCounter->TxAggRange1Count.u.LowPart += ampdu_range_cnt[0];
	wlanCounter->TxAggRange2Count.u.LowPart += ampdu_range_cnt[1];
	wlanCounter->TxAggRange3Count.u.LowPart += ampdu_range_cnt[2];
	wlanCounter->TxAggRange4Count.u.LowPart += ampdu_range_cnt[3];
}

VOID NICUpdateRawCounters(RTMP_ADAPTER *pAd)
{
	UINT32 OldValue, i;
	UINT32 rx_err_cnt, fcs_err_cnt, mdrdy_cnt = 0, fcs_err_cnt_band1 = 0, mdrdy_cnt_band1 = 0;
	/* UINT32 TxSuccessCount = 0, TxRetryCount = 0; */
#ifdef COMPOS_WIN
	COUNTER_MTK *pPrivCounters;
#else
	COUNTER_RALINK *pPrivCounters;
#endif
	COUNTER_802_11 *wlanCounter;
	COUNTER_802_3 *dot3Counters;
#ifdef ERR_RECOVERY

	if (IsStopingPdma(&pAd->ErrRecoveryCtl))
		return;

#endif /* ERR_RECOVERY */
#ifdef COMPOS_WIN
	wlanCounter = &pAd->Counter.WlanCounters;
	pPrivCounters = &pAd->Counter.MTKCounters;
	dot3Counters = &pAd->Counter.Counters8023;
#else
	pPrivCounters = &pAd->RalinkCounters;
	wlanCounter = &pAd->WlanCounters[0];
	dot3Counters = &pAd->Counters8023;
#endif /* COMPOS_WIN */
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR3, &rx_err_cnt);
	fcs_err_cnt = rx_err_cnt & 0xffff;
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR4, &rx_err_cnt);

	if (pAd->parse_rxv_stat_enable) {
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR10, &mdrdy_cnt);
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR3, &fcs_err_cnt_band1);
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR10, &mdrdy_cnt_band1);
	}

	pPrivCounters->OneSecRxFcsErrCnt += fcs_err_cnt;

	if (pAd->parse_rxv_stat_enable) {
		pAd->AccuOneSecRxBand0FcsErrCnt += fcs_err_cnt; /*Used for rx_statistic*/
		pAd->AccuOneSecRxBand0MdrdyCnt += mdrdy_cnt; /*Used for rx_statistic*/
		pAd->AccuOneSecRxBand1FcsErrCnt += fcs_err_cnt_band1; /*Used for rx_statistic*/
		pAd->AccuOneSecRxBand1MdrdyCnt += mdrdy_cnt_band1; /*Used for rx_statistic*/
	}

#ifdef STATS_COUNT_SUPPORT
	/* Update FCS counters*/
	OldValue = pAd->WlanCounters[0].FCSErrorCount.u.LowPart;
	pAd->WlanCounters[0].FCSErrorCount.u.LowPart += fcs_err_cnt; /* >> 7);*/

	if (pAd->WlanCounters[0].FCSErrorCount.u.LowPart < OldValue)
		pAd->WlanCounters[0].FCSErrorCount.u.HighPart++;

#endif /* STATS_COUNT_SUPPORT */
	/* Add FCS error count to private counters*/
	pPrivCounters->OneSecRxFcsErrCnt += fcs_err_cnt;
	OldValue = pPrivCounters->RealFcsErrCount.u.LowPart;
	pPrivCounters->RealFcsErrCount.u.LowPart += fcs_err_cnt;

	if (pPrivCounters->RealFcsErrCount.u.LowPart < OldValue)
		pPrivCounters->RealFcsErrCount.u.HighPart++;

	dot3Counters->RxNoBuffer += (rx_err_cnt & 0xffff);

	for (i = 0; i < DBDC_BAND_NUM; i++)
		NICUpdateAmpduRawCounters(pAd, i);

#ifdef CONFIG_QA

	if (pAd->ATECtrl.bQAEnabled == TRUE) {
		/* Modify Rx stat structure */
		/* pAd->ATECtrl.rx_stat.RxMacFCSErrCount += fcs_err_cnt; */
		MT_ATEUpdateRxStatistic(pAd, 3, wlanCounter);
	}

#endif
	return;
}


/*
	========================================================================

	Routine Description:
		Clean all Tx/Rx statistic raw counters from hardware registers

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	========================================================================
*/
VOID NicResetRawCounters(RTMP_ADAPTER *pAd)
{
	return;
}

#define TMI_TX_RATE_CCK_VAL(_mcs) \
	((TMI_TX_RATE_MODE_CCK << TXD_TX_RATE_BIT_MODE) | (_mcs))

#define TMI_TX_RATE_OFDM_VAL(_mcs) \
	((TMI_TX_RATE_MODE_OFDM << TXD_TX_RATE_BIT_MODE) | (_mcs))

#define TMI_TX_RATE_HT_VAL(_mode, _mcs, _stbc) \
	(((_stbc) << TXD_TX_RATE_BIT_STBC) |\
	 ((_mode) << TXD_TX_RATE_BIT_MODE) | \
	 (_mcs))

#define TMI_TX_RATE_VHT_VAL(_nss, _mcs, _stbc) \
	(((_stbc) << TXD_TX_RATE_BIT_STBC) |\
	 (((_nss - 1) & (TXD_TX_RATE_MASK_NSS)) << TXD_TX_RATE_BIT_NSS) | \
	 (TMI_TX_RATE_MODE_VHT << TXD_TX_RATE_BIT_MODE) | \
	 (_mcs))

UCHAR get_nsts_by_mcs(UCHAR phy_mode, UCHAR mcs, BOOLEAN stbc, UCHAR vht_nss)
{
	UINT8 nsts = 1;

	switch (phy_mode) {
	case MODE_VHT:
		if (stbc && (vht_nss == 1))
			nsts++;
		else
			nsts = vht_nss;

		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		if (mcs != 32) {
			nsts += (mcs >> 3);

			if (stbc && (nsts == 1))
				nsts++;
		}

		break;

	case MODE_CCK:
	case MODE_OFDM:
	default:
		break;
	}

	return nsts;
}

UCHAR dmac_wmm_swq_2_hw_ac_que[4][4] = {
	{
		TxQ_IDX_AC0, /* 0: QID_AC_BK */
		TxQ_IDX_AC1, /* 1: QID_AC_BE */
		TxQ_IDX_AC2, /* 2: QID_AC_VI */
		TxQ_IDX_AC3, /* 3: QID_AC_VO */
	},
	{
		TxQ_IDX_AC10,
		TxQ_IDX_AC11,
		TxQ_IDX_AC12,
		TxQ_IDX_AC13,
	},
	{
		TxQ_IDX_AC20,
		TxQ_IDX_AC21,
		TxQ_IDX_AC22,
		TxQ_IDX_AC23,
	},
	{
		TxQ_IDX_AC30,
		TxQ_IDX_AC31,
		TxQ_IDX_AC32,
		TxQ_IDX_AC33,
	}
};

UINT8  dmac_ac_queue_to_up[4] = {
    1 /* AC_BK */,   0 /* AC_BE */,   5 /* AC_VI */,   7 /* AC_VO */
};

VOID MtWriteTMacInfo(RTMP_ADAPTER *pAd, UCHAR *buf, TMAC_INFO *TxInfo)
{
}

VOID mtf_write_tmac_info_fixed_rate(
	RTMP_ADAPTER *pAd,
	UCHAR *tmac_info,
	MAC_TX_INFO *info,
	HTTRANSMIT_SETTING *transmit)
{
	MAC_TABLE_ENTRY *mac_entry = NULL;
	CHAR stbc, bw, mcs, nss = 1, sgi, phy_mode, ldpc = 0, preamble = LONG_PREAMBLE;
	UCHAR q_idx = info->q_idx;
	STA_TR_ENTRY *tr_entry = NULL;
	struct txd_l *txd = (struct txd_l *)tmac_info;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (VALID_UCAST_ENTRY_WCID(pAd, info->WCID))
		mac_entry = &pAd->MacTab.Content[info->WCID];

	/*  DW0 */

	/* TX Byte Count [15:0] */
	txd->txd_0 |= ((sizeof(struct txd_l) + info->Length)
				<< TXD_TX_BYTE_COUNT_SHIFT);

	/* PKT_FT [24:23] */
	txd->txd_0 |= (FT_CTD << TXD_PKT_FT_SHIFT);

	/* Q_IDX [31:25] */
	if (q_idx < 4) {
		txd->txd_0 |= (dmac_wmm_swq_2_hw_ac_que[info->wmm_set][q_idx] << TXD_Q_IDX_SHIFT);
	} else {
		 txd->txd_0 |= (q_idx << TXD_Q_IDX_SHIFT);
	}

	/* DW1 */

	/* WLAN Index [9:0] */
	txd->txd_1 |= (info->WCID << TXD_WLAN_IDX_SHIFT);

	/* HEADER_LENGTH [15:11] (HF=2'b10) */
	/* MRD [11] (HF=2'b00) */
	/* EOSP [12] (HF=2'b00) */
	/* RMVL [13] (HF=2'b00) */
	/* VLAN [14] (HF=2'b00) */
	/* ETYP [15] (HF=2'b00) */
	/* HF [17:16] */
	/* Header Padding [19:18] */
	txd->txd_1 |= ((info->hdr_len >> 1) << TXD_HDR_LEN_SHIFT);

	txd->txd_1 |= (HF_802_11_FRAME << TXD_HF_SHIFT);

	if (info->hdr_pad) {
		txd->txd_1 |= (((TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | 0x1)
					<< TXD_HDR_PAD_SHIFT);
	}

	/* TID [22:20]  */
	txd->txd_1 |= (info->TID << TXD_TID_SHIFT);

	/* OM [29:24] */
	if (mac_entry && IS_ENTRY_REPEATER(mac_entry)) {
		tr_entry = &pAd->MacTab.tr_entry[mac_entry->wcid];
		txd->txd_1 |= (tr_entry->OmacIdx << TXD_OM_SHIFT);
	} else if (mac_entry && !IS_ENTRY_NONE(mac_entry) && !IS_ENTRY_MCAST(mac_entry)) {
		txd->txd_1 |= (mac_entry->wdev->OmacIdx << TXD_OM_SHIFT);
	} else {
		txd->txd_1 |= (info->OmacIdx << TXD_OM_SHIFT);
	}

	/* FT [31] */
	txd->txd_1 |= TXD_FT;

	/* DW2 */
	/* BM [10]  */
	if (info->BM)
		txd->txd_2 |= TXD_BM;

	/* B [11] */
	if (info->prot == 2)
		txd->txd_2 |= TXD_B;

	/* FR [31] */
	if (!info->IsAutoRate)
		txd->txd_2 |= TXD_FR;


	/* DW3 */
	/* NA [0] */
	/* Remaining TX Count [15:11] */

	if (info->Ack) {
		txd->txd_3 |= (MT_TX_LONG_RETRY << TXD_REMAIN_TX_CNT_SHIFT);
	} else {
		txd->txd_3 |= TXD_NA;
		txd->txd_3 |= (MT_TX_RETRY_UNLIMIT << TXD_REMAIN_TX_CNT_SHIFT);
	}


	/* PF [1] */
	if (info->prot == 1)
		txd->txd_3 |= TXD_PF;


	/* TM [5] */
	if (cap->TmrEnable == 1) {
		if (info->IsTmr)
			txd->txd_3 |= TXD_TM;
	}

	/* Power Management [29] */
	if (info->PsmBySw)
		txd->txd_3 |= TXD_PM;

	/* DW5 */
	/* LDPC [11] */
	/* GI [15:14] */
	/* DW6 */
	/* BW [2:0]  */
	/* Rate to be Fixed [29:16] */

	if (!info->IsAutoRate) {
		ldpc = transmit->field.ldpc;

		if (ldpc)
			txd->txd_5 |= TXD_LDPC;

		sgi = transmit->field.ShortGI;
		txd->txd_5 = (sgi << TXD_GI_SHIFT);

		phy_mode = transmit->field.MODE;
		bw = (phy_mode <= MODE_OFDM) ? (BW_20) : (transmit->field.BW);

		txd->txd_6 |= (((1 << 2) | bw) << TXD_BW_SHIFT);

		mcs = transmit->field.MCS;
		stbc = transmit->field.STBC;

		if (phy_mode == MODE_CCK)
			preamble = info->Preamble;

		nss = 1;

		txd->txd_6 |= (tx_rate_to_tmi_rate(phy_mode, mcs, 1, stbc, preamble) << TXD_FR_RATE_SHIFT) ;
	}

	/* DW7 */
	/* SPE_IDX [15:11] */
	/* Subtype [19:16] */
	/* Type [21:20] */
	txd->txd_7 |= (info->AntPri << TXD_SPE_IDX_SHIFT);
	txd->txd_7 |= (info->SubType  << TXD_PP_SUBTYPE_SHIFT);
	txd->txd_7 |= (info->Type << TXD_PP_TYPE_SHIFT);

	if (0)
		mtd_dump_tmac_info(pAd, tmac_info);
}

VOID mtf_write_tmac_info(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	mtf_write_tmac_info_by_host(pAd, buf, pTxBlk);
}

UINT16 tx_rate_to_tmi_rate(UINT8 mode, UINT8 mcs, UINT8 nss, BOOLEAN stbc, UINT8 preamble)
{
	UINT16 tmi_rate = 0, mcs_id = 0;

	stbc = (stbc == TRUE) ? 1 : 0;

	switch (mode) {
	case MODE_CCK:
		if (preamble) {
			if (mcs < tmi_rate_map_cck_lp_size)
				mcs_id = tmi_rate_map_cck_lp[mcs];
		} else {
			if (mcs < tmi_rate_map_cck_sp_size)
				mcs_id = tmi_rate_map_cck_sp[mcs];
		}

		tmi_rate = (TMI_TX_RATE_MODE_CCK << TXD_TX_RATE_BIT_MODE) | (mcs_id);
		break;

	case MODE_OFDM:
		if (mcs < tmi_rate_map_ofdm_size) {
			mcs_id = tmi_rate_map_ofdm[mcs];
			tmi_rate = (TMI_TX_RATE_MODE_OFDM << TXD_TX_RATE_BIT_MODE) | (mcs_id);
		}

		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		tmi_rate = ((USHORT)(stbc << TXD_TX_RATE_BIT_STBC)) |
				   (((nss - 1) & TXD_TX_RATE_MASK_NSS) << TXD_TX_RATE_BIT_NSS) |
				   ((USHORT)(mode << TXD_TX_RATE_BIT_MODE)) |
				   ((USHORT)(mcs));
		break;

	case MODE_VHT:
		tmi_rate = TMI_TX_RATE_VHT_VAL(nss, mcs, stbc);
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid mode(mode=%d)\n",
				 __func__, mode));
		break;
	}

	return tmi_rate;
}

VOID mtf_write_tmac_info_by_host(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *tx_blk)
{
	struct txd_l *txd = (struct txd_l *)buf;
	MAC_TABLE_ENTRY *mac_entry = tx_blk->pMacEntry;
	struct wifi_dev *wdev  = tx_blk->wdev;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	HTTRANSMIT_SETTING *transmit = tx_blk->pTransmit;
	UCHAR stbc = 0, bw = BW_20, mcs = 0, nss = 1, sgi = 0, phy_mode = 0, preamble = 1, ldpc = 0;
	STA_TR_ENTRY *tr_entry = NULL;

	/*  DW0 */

	/* TX Byte Count [15:0] */
	txd->txd_0 |= (tx_blk->tx_bytes_len << TXD_TX_BYTE_COUNT_SHIFT);

	/* PKT_FT [24:23] */
	txd->txd_0 |= (FT_CTD << TXD_PKT_FT_SHIFT);

	/* Q_IDX [31:25] */
	if (tx_blk->QueIdx < 4) {
		txd->txd_0 |= (dmac_wmm_swq_2_hw_ac_que[tx_blk->wmm_set][tx_blk->QueIdx] << TXD_Q_IDX_SHIFT);
	} else {
		 txd->txd_0 |= (tx_blk->QueIdx << TXD_Q_IDX_SHIFT);
	}

	/* DW1 */

	/* WLAN Index [9:0] */
	txd->txd_1 |= (tx_blk->Wcid << TXD_WLAN_IDX_SHIFT);

	/* HEADER_LENGTH [15:11] (HF=2'b10) */
	/* MRD [11] (HF=2'b00) */
	/* EOSP [12] (HF=2'b00) */
	/* RMVL [13] (HF=2'b00) */
	/* VLAN [14] (HF=2'b00) */
	/* ETYP [15] (HF=2'b00) */
	/* HF [17:16] */
	/* Header Padding [19:18] */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS)) {

		if (TX_BLK_TEST_FLAG(tx_blk, fTX_bMoreData))
			txd->txd_1 |= TXD_MRD;

		if (TX_BLK_TEST_FLAG(tx_blk, fTX_bWMM_UAPSD_EOSP))
			txd->txd_1 |= TXD_EOSP;

		txd->txd_1 |= TXD_RMVL;

		if (RTMP_GET_PACKET_VLAN(tx_blk->pPacket))
			txd->txd_1 |= TXD_VLAN;

		if (RTMP_GET_PACKET_PROTOCOL(tx_blk->pPacket) > 1500)
			txd->txd_1 |= TXD_ETYP;

		txd->txd_1 |= (HF_802_3_FRAME << TXD_HF_SHIFT);

		if (tx_blk->HdrPadLen) {
			txd->txd_1 |= (((TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | 0x1)
						<< TXD_HDR_PAD_SHIFT);
		}
	} else {
		txd->txd_1 |= ((tx_blk->wifi_hdr_len >> 1) << TXD_HDR_LEN_SHIFT);

		txd->txd_1 |= (HF_802_11_FRAME << TXD_HF_SHIFT);

		if (tx_blk->HdrPadLen) {
			txd->txd_1 |= (((TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | 0x1)
						<< TXD_HDR_PAD_SHIFT);

		}

	}

	/* TID [22:20]  */
	txd->txd_1 |= (tx_blk->UserPriority << TXD_TID_SHIFT);

	/* AMSDU [23] */
	if (tx_blk->TxFrameType & TX_AMSDU_FRAME)
		txd->txd_1 |= TXD_AMSDU;

	/* OM [29:24] */
	if (mac_entry && IS_ENTRY_REPEATER(mac_entry)) {
		if (tx_blk->tr_entry) {
			tr_entry = tx_blk->tr_entry;
			txd->txd_1 |= (tr_entry->OmacIdx << TXD_OM_SHIFT);
		}
	} else {
		txd->txd_1 |= (wdev->OmacIdx << TXD_OM_SHIFT);
	}

	/* FT [31] */
	txd->txd_1 |= TXD_FT;

	/* DW2 */
	/* BM [10]  */
	if (tx_blk->TxFrameType == TX_MCAST_FRAME)
		txd->txd_2 |= TXD_BM;

	/* FRAG [15:14] */
	txd->txd_2 |= (tx_blk->FragIdx << TXD_FRAG_SHIFT);

	/* FR [31] */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_ForceRate))
		txd->txd_2 |= TXD_FR;


	/* DW3 */
	/* NA [0] */
	/* Remaining TX Count [15:11] */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bAckRequired)) {
		txd->txd_3 |= (MT_TX_LONG_RETRY << TXD_REMAIN_TX_CNT_SHIFT);
	} else {
		txd->txd_3 |= TXD_NA;
		txd->txd_3 |= (MT_TX_RETRY_UNLIMIT << TXD_REMAIN_TX_CNT_SHIFT);
	}

	/* PF [1] */
	if (!IS_CIPHER_NONE(tx_blk->CipherAlg))
		txd->txd_3 |= TXD_PF;

	/* DAS [4] */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS)) {
		if (TX_BLK_TEST_FLAG(tx_blk, fTX_MCAST_CLONE))
			txd->txd_3 |= TXD_DAS;
	}

	/* TM [5] */
	if (cap->TmrEnable == 1) {
		if (TX_BLK_TEST_FLAG(tx_blk, fTX_bAckRequired))
			txd->txd_3 |= TXD_TM;
	}

	/* Power Management [29] */
#if defined(CONFIG_STA_SUPPORT) && defined(CONFIG_PM_BIT_HW_MODE)
	txd->txd_3 &= ~TXD_PM;
#else
	txd->txd_3 |= TXD_PM;
#endif

	/* DW5 */
	/* LDPC [11] */
	/* GI [15:14] */
	/* DW6 */
	/* BW [2:0]  */
	/* Rate to be Fixed [29:16] */

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_ForceRate) && transmit) {
		ldpc = transmit->field.ldpc;

		if (ldpc)
			txd->txd_5 |= TXD_LDPC;

		sgi = transmit->field.ShortGI;

		txd->txd_5 = (sgi << TXD_GI_SHIFT);

		phy_mode = transmit->field.MODE;
		bw = (phy_mode <= MODE_OFDM) ? (BW_20) : (transmit->field.BW);

		txd->txd_6 |= (((1 << 2) | bw) << TXD_BW_SHIFT);

		stbc = transmit->field.STBC;
		mcs = transmit->field.MCS;
		nss = get_nsts_by_mcs(phy_mode, mcs, stbc, phy_mode == MODE_VHT ?
					((transmit->field.MCS & (0x3 << 4)) >> 4) + 1 : 0);

		if (phy_mode == MODE_CCK) {
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
				preamble = SHORT_PREAMBLE;
			else
				preamble = LONG_PREAMBLE;

		}

		txd->txd_6 |= (tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble) << TXD_FR_RATE_SHIFT) ;
	}

	/* DW7 */
	/* HW_AMSDU_CAP [10] */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_HW_AMSDU)) {
		txd->txd_7 |= TXD_HW_AMSDU_CAP;
	}

	if (0)
		mtd_dump_tmac_info(pAd, buf);
}

VOID mtf_dump_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info)
{
	struct txd_l *txd = (struct txd_l *)tmac_info;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	hex_dump("TMAC_Info Raw Data: ", (UCHAR *)tmac_info, cap->TXWISize);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_TXD Fields:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_0:\n"));


	/* DW0 */
	/* TX Byte Count [15:0]  */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxByteCnt = %d\n",
				((txd->txd_0 & TXD_TX_BYTE_COUNT_MASK) >> TXD_TX_BYTE_COUNT_SHIFT)));

	/* PKT_FT: Packet Format [24:23] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpkt_ft = %d(%s)\n",
				((txd->txd_0 & TXD_PKT_FT_MASK) >> TXD_PKT_FT_SHIFT),
				pkt_ft_str[((txd->txd_0 & TXD_PKT_FT_MASK) >> TXD_PKT_FT_SHIFT)]));

	/* Q_IDX [31:25]  */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tQueID =0x%x\n",
				((txd->txd_0 & TXD_Q_IDX_MASK) >> TXD_Q_IDX_SHIFT)));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_1:\n"));

	/* DW1 */
	/* WLAN Indec [9:0] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tWlan Index = %d\n",
				((txd->txd_1 & TXD_WLAN_IDX_MASK) >> TXD_WLAN_IDX_SHIFT)));


	/* HF: Header Format [17:16] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrFmt = %d(%s)\n",
				((txd->txd_1 & TXD_HF_MASK) >> TXD_HF_SHIFT),
				((txd->txd_1 & TXD_HF_MASK) >> TXD_HF_SHIFT) < 4 ?
				 hdr_fmt_str[((txd->txd_1 & TXD_HF_MASK) >> TXD_HF_SHIFT)] : "N/A"));


	switch ((txd->txd_1 & TXD_HF_MASK) >> TXD_HF_SHIFT) {
	case TMI_HDR_FT_NON_80211:

		/* MRD [11], EOSP [12], RMVL [13], VLAN [14], ETYPE [15] */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tMRD = %d, EOSP = %d,\
				RMVL = %d, VLAN = %d, ETYP = %d\n",
				(txd->txd_1 & TXD_MRD) ? 1 : 0,
				(txd->txd_1 & TXD_EOSP) ? 1 : 0,
				(txd->txd_1 & TXD_RMVL) ? 1 : 0,
				(txd->txd_1 & TXD_VLAN) ? 1 : 0,
				(txd->txd_1 & TXD_ETYP) ? 1 : 0));
		break;
	case TMI_HDR_FT_NOR_80211:
		/* HEADER_LENGTH [15:11] */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tHeader Len = %d(WORD)\n",
				((txd->txd_1 & TXD_HDR_LEN_MASK) >> TXD_HDR_LEN_SHIFT)));
		break;

	case TMI_HDR_FT_ENH_80211:
		/* EOSP [12], AMS [13]  */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tEOSP = %d, AMS = %d\n",
				(txd->txd_1 & TXD_EOSP) ? 1 : 0,
				(txd->txd_1 & TXD_AMS) ? 1 : 0));
		break;
	}

	/* Header Padding [19:18] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrPad = %d\n",
				((txd->txd_1 & TXD_HDR_PAD_MASK) >> TXD_HDR_PAD_SHIFT)));

	/* TID [22:20] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTID = %d\n",
				((txd->txd_1 & TXD_TID_MASK) >> TXD_TID_SHIFT)));

	/* UtxB/AMSDU_C/AMSDU [23] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tamsdu = %d\n",
				((txd->txd_1 & TXD_AMSDU) ? 1 : 0)));

	/* OM [29:24] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\town_mac = %d\n",
				((txd->txd_1 & TXD_OM_MASK) >> TXD_OM_SHIFT)));

	/* FT [31] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxDFormatType = %d\n",
				(txd->txd_1 & TXD_FT) ? 1 : 0));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_2:\n"));

	/* DW2 */
	/* Subtype [3:0] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsub_type = %d\n",
				((txd->txd_2 & TXD_SUBTYPE_MASK) >> TXD_SUBTYPE_SHIFT)));

	/* Type[5:4] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tfrm_type = %d\n",
				((txd->txd_2 & TXD_TYPE_MASK) >> TXD_TYPE_SHIFT)));

	/* NDP [6] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNDP = %d\n",
				((txd->txd_2 & TXD_NDP) ? 1 : 0)));

	/* NDPA [7] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNDPA = %d\n",
				((txd->txd_2 & TXD_NDPA) ? 1 : 0)));

	/* SD [8] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSounding = %d\n",
				((txd->txd_2 & TXD_SD) ? 1 : 0)));

	/* RTS [9] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tRTS = %d\n",
				((txd->txd_2 & TXD_RTS) ? 1 : 0)));

	/* BM [10] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tbc_mc_pkt = %d\n",
				((txd->txd_2 & TXD_BM) ? 1 : 0)));

	/* B [11]  */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBIP = %d\n",
				((txd->txd_2 & TXD_B) ? 1 : 0)));

	/* DU [12] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tDuration = %d\n",
				((txd->txd_2 & TXD_DU) ? 1 : 0)));

	/* HE [13] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHE(HTC Exist) = %d\n",
				((txd->txd_2 & TXD_HE) ? 1 : 0)));

	/* FRAG [15:14] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFRAG = %d\n",
				((txd->txd_2 & TXD_FRAG_MASK) >> TXD_FRAG_SHIFT)));

	/* Remaining Life Time [23:16]*/
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tReamingLife/MaxTx time = %d\n",
				((txd->txd_2 & TXD_REMAIN_TIME_MASK) >> TXD_REMAIN_TIME_SHIFT)));

	/* Power Offset [29:24] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpwr_offset = %d\n",
				((txd->txd_2 & TXD_PWR_OFFESET_MASK) >> TXD_PWR_OFFESET_SHIFT)));

	/* FRM [30] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tfix rate mode = %d\n",
				(txd->txd_2 & TXD_FRM) ? 1 : 0));

	/* FR[31] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tfix rate = %d\n",
				(txd->txd_2 & TXD_FR) ? 1 : 0));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_3:\n"));

	/* DW3 */
	/* NA [0] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNoAck = %d\n",
				(txd->txd_3 & TXD_NA) ? 1 : 0));

	/* PF [1] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPF = %d\n",
				(txd->txd_3 & TXD_PF) ? 1 : 0));

	/* EMRD [2] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEMRD = %d\n",
				(txd->txd_3 & TXD_EMRD) ? 1 : 0));

	/* EEOSP [3] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEEOSP = %d\n",
				(txd->txd_3 & TXD_EEOSP) ? 1 : 0));

	/* DAS [4] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tda_select = %d\n",
				(txd->txd_3 & TXD_DAS) ? 1 : 0));

	/* TM [5] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttm = %d\n",
				(txd->txd_3 & TXD_TM) ? 1 : 0));

	/* TX Count [10:6] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_cnt = %d\n",
				((txd->txd_3 & TXD_TX_CNT_MASK) >> TXD_TX_CNT_SHIFT)));

	/* Remaining TX Count [15:11] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tremain_tx_cnt = %d\n",
				((txd->txd_3 & TXD_REMAIN_TX_CNT_MASK) >> TXD_REMAIN_TX_CNT_SHIFT)));

	/* SN [27:16] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsn = %d\n",
				((txd->txd_3 & TXD_SN_MASK) >> TXD_SN_SHIFT)));

	/* BA_DIS [28] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tba dis = %d\n",
				(txd->txd_3 & TXD_BA_DIS) ? 1 : 0));

	/* Power Management [29] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpwr_mgmt = 0x%x\n",
				(txd->txd_3 & TXD_PM) ? 1 : 0));

	/* PN_VLD [30] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_vld = %d\n",
				(txd->txd_3 & TXD_PN_VLD) ? 1 : 0));

	/* SN_VLD [31] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsn_vld = %d\n",
				(txd->txd_3 & TXD_SN_VLD) ? 1 : 0));


	/* DW4 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_4:\n"));

	/* PN_LOW [31:0] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_low = 0x%x\n",
				((txd->txd_4 & TXD_PN1_MASK) >> TXD_PN1_SHIFT)));


	/* DW5 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_5:\n"));

	/* PID [7:0] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpid = %d\n",
				((txd->txd_5 & TXD_PID_MASK) >> TXD_PID_SHIFT)));

	/* TXSFM [8] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_fmt = %d\n",
				(txd->txd_5 & TXD_TXSFM) ? 1 : 0));

	/* TXS2M [9] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_2_mcu = %d\n",
				(txd->txd_5 & TXD_TXS2M) ? 1 : 0));

	/* TXS2H [10] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_2_host = %d\n",
				(txd->txd_5 & TXD_TXS2H) ? 1 : 0));

	if (txd->txd_2 & TXD_FR) {
		/* LDPC [11] */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tldpc = %d\n",
				(txd->txd_5 & TXD_LDPC) ? 1 : 0));

		/* HELTF Type[13:12] */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHELTF Type = %d\n",
				((txd->txd_5 & TXD_HELTF_TYPE_MASK) >> TXD_HELTF_TYPE_SHIFT)));

		/* GI Type [15:14] */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tGI = %d\n",
				((txd->txd_5 & TXD_GI_MASK) >> TXD_GI_SHIFT)));
	}

	/* PN_HIGH [31:16]  */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_high = 0x%x\n",
				((txd->txd_5 & TXD_PN2_MASK) >> TXD_PN2_SHIFT)));

	/* DW6 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_6:\n"));

	if (txd->txd_2 & TXD_FR) {
		/* Fixed BandWidth mode [2:0] */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tbw = %d\n",
				((txd->txd_6 & TXD_BW_MASK) >> TXD_BW_SHIFT)));

		/* DYN_BW [3] */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tdyn_bw = %d\n",
				(txd->txd_6 & TXD_DYN_BW) ? 1 : 0));

		/* ANT_ID [15:4] */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tant_id = %d\n",
				((txd->txd_6 & TXD_ANT_ID_MASK) >> TXD_ANT_ID_SHIFT)));

		/* Rate to be Fixed [29:16] */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_rate = 0x%x\n",
				((txd->txd_6 & TXD_FR_RATE_MASK) >> TXD_FR_RATE_SHIFT)));
	}

	/* TXEBF [30] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttxebf = %d\n",
				(txd->txd_6 & TXD_TXEBF)  ? 1 : 0));

	/* TXIBF [31] */
	 MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttxibf = %d\n",
				(txd->txd_6 & TXD_TXIBF) ? 1 : 0));

	/* DW7 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_7:\n"));

	/* SW Tx Time [9:0] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsw_tx_time = %d\n",
				((txd->txd_7 & TXD_SW_TX_TIME_MASK) >> TXD_SW_TX_TIME_SHIFT)));

	/* HW_AMSDU_CAP [10] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\thw amsdu cap = %d\n",
				(txd->txd_7 & TXD_HW_AMSDU_CAP) ? 1 : 0));

	/* SPE_IDX [15:11] */
	if (txd->txd_2 & TXD_FR) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tspe_idx = 0x%x\n",
				((txd->txd_7 & TXD_SPE_IDX_MASK) >> TXD_SPE_IDX_SHIFT)));
	}

	/* PSE_FID [27:16] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpse_fid = 0x%x\n",
				((txd->txd_7 & TXD_PSE_FID_MASK) >> TXD_PSE_FID_SHIFT)));

	/* Subtype [19:16] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpp_sub_type=%d\n",
				((txd->txd_7 & TXD_PP_SUBTYPE_MASK) >> TXD_PP_SUBTYPE_SHIFT)));

	/* Type [21:20] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpp_type=%d\n",
				((txd->txd_7 & TXD_PP_TYPE_MASK) >> TXD_PP_TYPE_SHIFT)));

	/* CTXD_CNT [25:23] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tctxd cnt=0x%x\n",
				((txd->txd_7 & TXD_CTXD_CNT_MASK) >> TXD_CTXD_CNT_SHIFT)));

	/* CTXD [26] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tctxd = %d\n",
				(txd->txd_7 & TXD_CTXD) ? 1 : 0));

	/* I [28]  */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ti = %d\n",
				(txd->txd_7 & TXD_IP_CHKSUM) ? 1 : 0));

	/* UT [29] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tUT = %d\n",
				(txd->txd_7 & TXD_UT) ? 1 : 0));

	/* TXDLEN [31:30] */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t txd len= %d\n",
				((txd->txd_7 & TXD_TXD_LEN_MASK) >> TXD_TXD_LEN_SHIFT)));
}

#ifdef RANDOM_PKT_GEN
VOID random_write_qidx(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
	TMAC_TXD_0 *txd_0 = &txd_l->TxD0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (RandomTxCtrl != 0 && txd_0->q_idx < (cap->WmmHwNum * 4))
		txd_0->q_idx = pTxBlk->lmac_qidx;
}
#endif

INT32 mtf_write_txp_info_by_host(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *tx_blk)
{
	MAC_TX_PKT_T *txp = (MAC_TX_PKT_T *)buf;
	TXD_PTR_LEN_T *txp_ptr_len;
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
	UINT16 token;

	if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
		(tx_blk->amsdu_state == TX_AMSDU_ID_FIRST))
		NdisZeroMemory(txp, sizeof(MAC_TX_PKT_T));

	txp_ptr_len = &txp->arPtrLen[tx_blk->frame_idx / 2];

	if ((tx_blk->frame_idx & 0x1) == 0x0) {
		txp_ptr_len->u4Ptr0 = cpu2le32(PCI_MAP_SINGLE(pAd, tx_blk, 0, 1, RTMP_PCI_DMA_TODEVICE));

		if (RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) {
			token = cut_through_tx_enq(pktTokenCb, tx_blk->pPacket, TOKEN_TX_MGT, tx_blk->Wcid,
							txp_ptr_len->u4Ptr0, GET_OS_PKT_LEN(tx_blk->pPacket));
		} else {
			token = cut_through_tx_enq(pktTokenCb, tx_blk->pPacket, TOKEN_TX_DATA, tx_blk->Wcid,
							txp_ptr_len->u4Ptr0, GET_OS_PKT_LEN(tx_blk->pPacket));
		}

		txp_ptr_len->u2Len0 = cpu2le16((tx_blk->SrcBufLen & TXD_LEN_MASK_V2) | TXD_LEN_ML_V2);
	} else {
		txp_ptr_len->u4Ptr1 = cpu2le32(PCI_MAP_SINGLE(pAd, tx_blk, 0, 1, RTMP_PCI_DMA_TODEVICE));

		if (RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) {
			token = cut_through_tx_enq(pktTokenCb, tx_blk->pPacket, TOKEN_TX_MGT, tx_blk->Wcid,
							txp_ptr_len->u4Ptr1, GET_OS_PKT_LEN(tx_blk->pPacket));
		} else {
			token = cut_through_tx_enq(pktTokenCb, tx_blk->pPacket, TOKEN_TX_DATA, tx_blk->Wcid,
							txp_ptr_len->u4Ptr1, GET_OS_PKT_LEN(tx_blk->pPacket));
		}

		txp_ptr_len->u2Len1 = cpu2le16((tx_blk->SrcBufLen & TXD_LEN_MASK_V2) | TXD_LEN_ML_V2);
	}

	txp->au2MsduId[tx_blk->frame_idx] = cpu2le16(token | TXD_MSDU_ID_VLD);
	tx_blk->txp_len = sizeof(MAC_TX_PKT_T);
	return NDIS_STATUS_SUCCESS;
}

INT dump_txp_info(RTMP_ADAPTER *pAd, CR4_TXP_MSDU_INFO *txp_info)
{
	INT cnt;

	hex_dump("CT_TxP_Info Raw Data: ", (UCHAR *)txp_info, sizeof(CR4_TXP_MSDU_INFO));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXP_Fields:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\ttype_and_flags=0x%x\n", txp_info->type_and_flags));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tmsdu_token=%d\n", txp_info->msdu_token));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tbss_index=%d\n", txp_info->bss_index));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tbuf_num=%d\n", txp_info->buf_num));

	for (cnt = 0; cnt < txp_info->buf_num; cnt++) {
		if (cnt >= MAX_BUF_NUM_PER_PKT)
			break;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tbuf_ptr=0x%x\n", txp_info->buf_ptr[cnt]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tbuf_len=%d(0x%x)\n", txp_info->buf_len[cnt], txp_info->buf_len[cnt]));
	}

	return 0;
}

VOID mt_write_tmac_info_beacon(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *tmac_buf, HTTRANSMIT_SETTING *BeaconTransmit, ULONG frmLen)
{
	MAC_TX_INFO mac_info;

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));
	mac_info.Type = FC_TYPE_MGMT;
	mac_info.SubType = SUBTYPE_BEACON;
	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = TRUE;
	mac_info.AMPDU = FALSE;
	mac_info.BM = 1;
	mac_info.Ack = FALSE;
	mac_info.NSeq = TRUE;
	mac_info.BASize = 0;
	mac_info.WCID = 0;
	mac_info.Length = frmLen;
	mac_info.TID = 0;
	mac_info.TxRate = 0;
	mac_info.Txopmode = IFS_HTTXOP;
	mac_info.hdr_len = 24;
	mac_info.bss_idx = wdev->func_idx;
	mac_info.SpeEn = 1;
	mac_info.q_idx = HcGetBcnQueueIdx(pAd, wdev);
	mac_info.TxSPriv = wdev->func_idx;
	mac_info.OmacIdx = wdev->OmacIdx;

	if (wdev->bcn_buf.BcnUpdateMethod == BCN_GEN_BY_FW)
		mac_info.IsOffloadPkt = TRUE;
	else
		mac_info.IsOffloadPkt = FALSE;

	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;

	if (pAd->CommonCfg.bSeOff != TRUE) {
		if (HcGetBandByWdev(wdev) == BAND0)
			mac_info.AntPri = BAND0_SPE_IDX;
		else if (HcGetBandByWdev(wdev) == BAND1)
			mac_info.AntPri = BAND1_SPE_IDX;
	}
	NdisZeroMemory(tmac_buf, sizeof(TMAC_TXD_L));
	mtf_write_tmac_info_fixed_rate(pAd, tmac_buf, &mac_info, BeaconTransmit);
#ifdef RT_BIG_ENDIAN

	if (IS_HIF_TYPE(pAd, HIF_MT))
		MTMacInfoEndianChange(pAd, tmac_buf, TYPE_TXWI, sizeof(TMAC_TXD_L));

#endif
}


INT rtmp_mac_set_band(RTMP_ADAPTER *pAd, int  band)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
	return FALSE;
}


INT mt_mac_set_ctrlch(RTMP_ADAPTER *pAd, UINT8 extch)
{
	/* TODO: shiang-7603 */
	return FALSE;
}


#ifdef GREENAP_SUPPORT
INT rtmp_mac_set_mmps(RTMP_ADAPTER *pAd, INT ReduceCorePower)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
	return FALSE;
}
#endif /* GREENAP_SUPPORT */


#define BCN_TBTT_OFFSET		64	/*defer 64 us*/
VOID ReSyncBeaconTime(RTMP_ADAPTER *pAd)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
}


#ifdef RTMP_MAC_PCI
static INT mt_asic_cfg_hif_tx_ring(RTMP_ADAPTER *pAd, struct hif_pci_tx_ring *ring, UINT32 offset, UINT32 phy_addr, UINT32 cnt)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);

	ring->TxSwFreeIdx = 0;
	ring->TxCpuIdx = 0;
	ring->hw_desc_base = MT_TX_RING_BASE + offset;
	ring->hw_cnt_addr = ring->hw_desc_base + 0x04;
	ring->hw_cidx_addr = ring->hw_desc_base + 0x08;
	ring->hw_didx_addr = ring->hw_desc_base + 0x0c;
	HIF_IO_WRITE32(pAd->hdev_ctrl, ring->hw_desc_base, phy_addr);
	HIF_IO_WRITE32(pAd->hdev_ctrl, ring->hw_cidx_addr, ring->TxCpuIdx);
	HIF_IO_WRITE32(pAd->hdev_ctrl, ring->hw_cnt_addr, tx_ring_size);
	return TRUE;
}


VOID mt_asic_init_txrx_ring(RTMP_ADAPTER *pAd)
{
	UINT32 phy_addr, offset;
	INT i, TxHwRingNum;
	struct hif_pci_tx_ring *ctrl_ring;
	struct hif_pci_tx_ring *FwDwlo_ring;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	UINT16 rx0_ring_size = GET_RX0_RING_SIZE(cap);
	UINT16 rx1_ring_size = GET_RX1_RING_SIZE(cap);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct tr_delay_control *tr_delay_ctl = &pAd->tr_ctl.tr_delay_ctl;
	UINT32 reg_value = 0;

	/* Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits */
	AsicSetWPDMA(pAd, PDMA_TX_RX, FALSE);
	AsicWaitPDMAIdle(pAd, 100, 1000);

	if (IS_ASIC_CAP(pAd, fASIC_CAP_RX_DLY)) {
		reg_value = RX_DLY_INT_CFG;
		tr_delay_ctl->rx_delay_en = TRUE;
	}

	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_DELAY_INT_CFG, reg_value);

	/* Reset DMA Index */
	HIF_IO_WRITE32(pAd->hdev_ctrl, WPDMA_RST_PTR, 0xFFFFFFFF);

	TxHwRingNum = GET_NUM_OF_TX_RING(cap);


	for (i = 0; i < TxHwRingNum; i++) {
		offset = i * 0x10;
		phy_addr = RTMP_GetPhysicalAddressLow(hif->TxRing[i].Cell[0].AllocPa);
		mt_asic_cfg_hif_tx_ring(pAd, &hif->TxRing[i], offset, phy_addr, tx_ring_size);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->TX_RING_%d[0x%x]: Base=0x%x, Cnt=%d!\n",
				 i, hif->TxRing[i].hw_desc_base, phy_addr, tx_ring_size));
	}

	/* init CTRL ring index pointer */
	ctrl_ring = &hif->ctrl_ring;
	phy_addr = RTMP_GetPhysicalAddressLow(ctrl_ring->Cell[0].AllocPa);

	if (IS_MT7615(pAd))
		offset = MT_RINGREG_DIFF * 2;
	else if (IS_MT7622(pAd))
		offset = MT_RINGREG_DIFF * 15;
	else if (IS_P18(pAd))
		offset = MT_RINGREG_DIFF * 15;
	else if (IS_MT7663(pAd))
		offset = MT_RINGREG_DIFF * 15;
	else if (IS_AXE(pAd))
		offset = MT_RINGREG_DIFF * 15;
	else
		offset = MT_RINGREG_DIFF * 5;

	ctrl_ring->TxSwFreeIdx = 0;
	ctrl_ring->TxCpuIdx = 0;
	ctrl_ring->hw_desc_base = (MT_TX_RING_BASE  + offset);
	ctrl_ring->hw_cnt_addr = (ctrl_ring->hw_desc_base + 0x04);
	ctrl_ring->hw_cidx_addr = (ctrl_ring->hw_desc_base + 0x08);
	ctrl_ring->hw_didx_addr = (ctrl_ring->hw_desc_base + 0x0c);
	HIF_IO_WRITE32(pAd->hdev_ctrl, ctrl_ring->hw_desc_base, phy_addr);
	HIF_IO_WRITE32(pAd->hdev_ctrl, ctrl_ring->hw_cidx_addr, ctrl_ring->TxCpuIdx);
	HIF_IO_WRITE32(pAd->hdev_ctrl, ctrl_ring->hw_cnt_addr, MGMT_RING_SIZE);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->TX_RING_CTRL: Base=0x%x, Cnt=%d!\n",
			 phy_addr, MGMT_RING_SIZE));

	/* init Firmware download ring index pointer */
	FwDwlo_ring = &hif->fwdl_ring;
	phy_addr = RTMP_GetPhysicalAddressLow(FwDwlo_ring->Cell[0].AllocPa);
	offset = MT_RINGREG_DIFF * 3;
	FwDwlo_ring->TxSwFreeIdx = 0;
	FwDwlo_ring->TxCpuIdx = 0;
	FwDwlo_ring->hw_desc_base = (MT_TX_RING_BASE  + offset);
	FwDwlo_ring->hw_cnt_addr = (FwDwlo_ring->hw_desc_base + 0x04);
	FwDwlo_ring->hw_cidx_addr = (FwDwlo_ring->hw_desc_base + 0x08);
	FwDwlo_ring->hw_didx_addr = (FwDwlo_ring->hw_desc_base + 0x0c);
	HIF_IO_WRITE32(pAd->hdev_ctrl, FwDwlo_ring->hw_desc_base, phy_addr);
	HIF_IO_WRITE32(pAd->hdev_ctrl, FwDwlo_ring->hw_cidx_addr, FwDwlo_ring->TxCpuIdx);
	HIF_IO_WRITE32(pAd->hdev_ctrl, FwDwlo_ring->hw_cnt_addr, MGMT_RING_SIZE);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("-->TX_RING_FwDwlo: Base=0x%x, Cnt=%d!\n",
			  phy_addr, MGMT_RING_SIZE));

	/* Init RX Ring0 Base/Size/Index pointer CSR */
	for (i = 0; i < GET_NUM_OF_RX_RING(cap); i++) {
		UINT16 RxRingSize = (i == 0) ? rx0_ring_size : rx1_ring_size;
		struct hif_pci_rx_ring *rx_ring;

		rx_ring = &hif->RxRing[i];
		offset = i * 0x10;
		phy_addr = RTMP_GetPhysicalAddressLow(rx_ring->Cell[0].AllocPa);
		rx_ring->RxSwReadIdx = 0;
		rx_ring->RxCpuIdx = RxRingSize - 1;
		rx_ring->hw_desc_base = MT_RX_RING_BASE + offset;
		rx_ring->hw_cidx_addr = MT_RX_RING_CIDX + offset;
		rx_ring->hw_didx_addr = MT_RX_RING_DIDX + offset;
		rx_ring->hw_cnt_addr = MT_RX_RING_CNT + offset;
		HIF_IO_WRITE32(pAd->hdev_ctrl, rx_ring->hw_desc_base, phy_addr);
		HIF_IO_WRITE32(pAd->hdev_ctrl, rx_ring->hw_cidx_addr, rx_ring->RxCpuIdx);
		HIF_IO_WRITE32(pAd->hdev_ctrl, rx_ring->hw_cnt_addr, RxRingSize);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->RX_RING%d[0x%x]: Base=0x%x, Cnt=%d\n",
				 i, rx_ring->hw_desc_base, phy_addr, RxRingSize));
	}
}
#endif /* RTMP_MAC_PCI */

INT mt_mac_pse_init(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Don't Support this now!\n", __func__, __LINE__));
	return FALSE;
}


#define HW_TX_RATE_TO_MODE(_x)			(((_x) & (0x7 << 6)) >> 6)
#define HW_TX_RATE_TO_MCS(_x, _mode)		((_x) & (0x3f))
#define HW_TX_RATE_TO_NSS(_x)				(((_x) & (0x3 << 9)) >> 9)
#define HW_TX_RATE_TO_STBC(_x)			(((_x) & (0x1 << 11)) >> 11)

#define MAX_TX_MODE 5
static char *HW_TX_MODE_STR[] = {"CCK", "OFDM", "HT-Mix", "HT-GF", "VHT", "N/A"};
static char *HW_TX_RATE_CCK_STR[] = {"1M", "2M", "5.5M", "11M", "N/A"};
static char *HW_TX_RATE_OFDM_STR[] = {"6M", "9M", "12M", "18M", "24M", "36M", "48M", "54M", "N/A"};

static char *hw_rate_ofdm_str(UINT16 ofdm_idx)
{
	switch (ofdm_idx) {
	case 11: /* 6M */
		return HW_TX_RATE_OFDM_STR[0];

	case 15: /* 9M */
		return HW_TX_RATE_OFDM_STR[1];

	case 10: /* 12M */
		return HW_TX_RATE_OFDM_STR[2];

	case 14: /* 18M */
		return HW_TX_RATE_OFDM_STR[3];

	case 9: /* 24M */
		return HW_TX_RATE_OFDM_STR[4];

	case 13: /* 36M */
		return HW_TX_RATE_OFDM_STR[5];

	case 8: /* 48M */
		return HW_TX_RATE_OFDM_STR[6];

	case 12: /* 54M */
		return HW_TX_RATE_OFDM_STR[7];

	default:
		return HW_TX_RATE_OFDM_STR[8];
	}
}

static char *hw_rate_str(UINT8 mode, UINT16 rate_idx)
{
	if (mode == 0)
		return rate_idx < 4 ? HW_TX_RATE_CCK_STR[rate_idx] : HW_TX_RATE_CCK_STR[4];
	else if (mode == 1)
		return hw_rate_ofdm_str(rate_idx);
	else
		return "MCS";
}


VOID dump_wtbl_basic_info(RTMP_ADAPTER *pAd, struct wtbl_struc *tb)
{
	struct wtbl_basic_info *basic_info = &tb->peer_basic_info;
	struct wtbl_tx_rx_cap *trx_cap = &tb->trx_cap;
	struct wtbl_rate_tb *rate_info = &tb->auto_rate_tb;
	UCHAR addr[MAC_ADDR_LEN];
	UINT16 txrate[8], rate_idx, txmode, mcs, nss, stbc;

	NdisMoveMemory(&addr[0], (UCHAR *)basic_info, 4);
	addr[0] = basic_info->wtbl_d1.field.addr_0 & 0xff;
	addr[1] = ((basic_info->wtbl_d1.field.addr_0 & 0xff00) >> 8);
	addr[2] = ((basic_info->wtbl_d1.field.addr_0 & 0xff0000) >> 16);
	addr[3] = ((basic_info->wtbl_d1.field.addr_0 & 0xff000000) >> 24);
	addr[4] = basic_info->wtbl_d0.field.addr_4 & 0xff;
	addr[5] = basic_info->wtbl_d0.field.addr_5 & 0xff;
	hex_dump("WTBL Raw Data", (UCHAR *)tb, sizeof(struct wtbl_struc));
	/* Basic Info (DW0~DW1) */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Basic Info(DW0~DW1):\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tAddr: %02x:%02x:%02x:%02x:%02x:%02x(D0[B0~15], D1[B0~31])\n",
			  PRINT_MAC(addr)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tMUAR_Idx(D0[B16~21]):%d\n",
			  basic_info->wtbl_d0.field.muar_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\trc_a1/rc_a2:%d/%d(D0[B22]/D0[B29])\n",
			  basic_info->wtbl_d0.field.rc_a1, basic_info->wtbl_d0.field.rc_a2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tKID:%d/RCID:%d/RKV:%d/RV:%d/IKV:%d/WPI_FLAG:%d(D0[B23~24], D0[B25], D0[B26], D0[B28], D0[B30])\n",
			  basic_info->wtbl_d0.field.kid, basic_info->wtbl_d0.field.rc_id,
			  basic_info->wtbl_d0.field.rkv, basic_info->wtbl_d0.field.rv,
			  basic_info->wtbl_d0.field.ikv, basic_info->wtbl_d0.field.wpi_flg));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tGID_SU:%d(D0[B31])\n", basic_info->wtbl_d0.field.gid_su));
	/* TRX Cap(DW2~5) */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("TRX Cap(DW2~5):\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tsw/DIS_RHTR:%d/%d\n",
			  trx_cap->wtbl_d2.field.SW, trx_cap->wtbl_d2.field.dis_rhtr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tHT/VHT/HT-LDPC/VHT-LDPC/DYN_BW/MMSS:%d/%d/%d/%d/%d/%d\n",
			  trx_cap->wtbl_d2.field.ht, trx_cap->wtbl_d2.field.vht,
			  trx_cap->wtbl_d5.field.ldpc, trx_cap->wtbl_d5.field.ldpc_vht,
			  trx_cap->wtbl_d5.field.dyn_bw, trx_cap->wtbl_d5.field.mm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tTx Power Offset:0x%x(%d)\n",
			  trx_cap->wtbl_d5.field.txpwr_offset, ((trx_cap->wtbl_d5.field.txpwr_offset == 0x0) ? 0 : (trx_cap->wtbl_d5.field.txpwr_offset - 0x20))));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tFCAP/G2/G4/G8/G16/CBRN:%d/%d/%d/%d/%d/%d\n",
			  trx_cap->wtbl_d5.field.fcap, trx_cap->wtbl_d5.field.g2,
			  trx_cap->wtbl_d5.field.g4, trx_cap->wtbl_d5.field.g8,
			  trx_cap->wtbl_d5.field.g16, trx_cap->wtbl_d5.field.cbrn));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tHT-TxBF(tibf/tebf):%d/%d, VHT-TxBF(tibf/tebf):%d/%d, PFMU_IDX=%d\n",
			  trx_cap->wtbl_d2.field.tibf, trx_cap->wtbl_d2.field.tebf,
			  trx_cap->wtbl_d2.field.tibf_vht, trx_cap->wtbl_d2.field.tebf_vht,
			  trx_cap->wtbl_d2.field.pfmu_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tSPE_IDX=%d\n", trx_cap->wtbl_d3.field.spe_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tBA Enable:0x%x, BAFail Enable:%d\n",
			  trx_cap->wtbl_d4.field.ba_en, trx_cap->wtbl_d3.field.baf_en));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tQoS Enable:%d\n",
			  trx_cap->wtbl_d5.field.qos));

	if (trx_cap->wtbl_d4.field.ba_en) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tBA WinSize: TID 0 - %d, TID 1 - %d\n",
				  trx_cap->wtbl_d4.field.ba_win_size_tid_0,
				  trx_cap->wtbl_d4.field.ba_win_size_tid_1));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tBA WinSize: TID 2 - %d, TID 3 - %d\n",
				  trx_cap->wtbl_d4.field.ba_win_size_tid_2,
				  trx_cap->wtbl_d4.field.ba_win_size_tid_3));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tBA WinSize: TID 4 - %d, TID 5 - %d\n",
				  trx_cap->wtbl_d4.field.ba_win_size_tid_4,
				  trx_cap->wtbl_d4.field.ba_win_size_tid_5));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tBA WinSize: TID 6 - %d, TID 7 - %d\n",
				  trx_cap->wtbl_d4.field.ba_win_size_tid_6,
				  trx_cap->wtbl_d4.field.ba_win_size_tid_7));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpartial_aid:%d\n", trx_cap->wtbl_d2.field.partial_aid));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twpi_even:%d\n", trx_cap->wtbl_d2.field.wpi_even));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAAD_OM/CipherSuit:%d/%d\n",
			 trx_cap->wtbl_d2.field.AAD_OM, trx_cap->wtbl_d2.field.cipher_suit));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\taf:%d\n", trx_cap->wtbl_d3.field.af));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\trdg_ba:%d/rdg capability:%d\n", trx_cap->wtbl_d3.field.rdg_ba, trx_cap->wtbl_d3.field.r));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tcipher_suit:%d\n", trx_cap->wtbl_d2.field.cipher_suit));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFromDS:%d\n", trx_cap->wtbl_d5.field.fd));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tToDS:%d\n", trx_cap->wtbl_d5.field.td));
	/* Rate Info (DW6~8) */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rate Info (DW6~8):\n"));
	txrate[0] = (rate_info->wtbl_d6.word & 0xfff);
	txrate[1] = (rate_info->wtbl_d6.word & (0xfff << 12)) >> 12;
	txrate[2] = ((rate_info->wtbl_d6.word & (0xff << 24)) >> 24) | ((rate_info->wtbl_d7.word & 0xf) << 8);
	txrate[3] = ((rate_info->wtbl_d7.word & (0xfff << 4)) >> 4);
	txrate[4] = ((rate_info->wtbl_d7.word & (0xfff << 16)) >> 16);
	txrate[5] = ((rate_info->wtbl_d7.word & (0xf << 28)) >> 28) | ((rate_info->wtbl_d8.word & (0xff)) << 4);
	txrate[6] = ((rate_info->wtbl_d8.word & (0xfff << 8)) >> 8);
	txrate[7] = ((rate_info->wtbl_d8.word & (0xfff << 20)) >> 20);

	for (rate_idx = 0; rate_idx < 8; rate_idx++) {
		txmode = HW_TX_RATE_TO_MODE(txrate[rate_idx]);
		mcs = HW_TX_RATE_TO_MCS(txrate[rate_idx], txmode);
		nss = HW_TX_RATE_TO_NSS(txrate[rate_idx]);
		stbc = HW_TX_RATE_TO_STBC(txrate[rate_idx]);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tRate%d(0x%x):TxMode=%d(%s), TxRate=%d(%s), Nsts=%d, STBC=%d\n",
				  rate_idx + 1, txrate[rate_idx],
				  txmode, (txmode < MAX_TX_MODE ? HW_TX_MODE_STR[txmode] : HW_TX_MODE_STR[MAX_TX_MODE]),
				  mcs, hw_rate_str(txmode, mcs), nss, stbc));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpsm:%d\n", trx_cap->wtbl_d3.field.psm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tskip_tx:%d\n", trx_cap->wtbl_d3.field.skip_tx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tdu_i_psm:%d\n", trx_cap->wtbl_d3.field.du_i_psm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\ti_psm:%d\n", trx_cap->wtbl_d3.field.i_psm));
}



VOID dump_wtbl_base_info(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWTBL Basic Info:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tMemBaseAddr:0x%x\n",
			 pAd->mac_ctrl.wtbl_base_addr[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEntrySize/Cnt:%d/%d\n",
			 pAd->mac_ctrl.wtbl_entry_size[0],
			 pAd->mac_ctrl.wtbl_entry_cnt[0]));
}


VOID dump_wtbl_info(RTMP_ADAPTER *pAd, UINT wtbl_idx)
{
	INT idx, start_idx, end_idx, wtbl_len;
	UINT32 wtbl_offset, addr;
	UCHAR *wtbl_raw_dw = NULL;
	struct wtbl_entry wtbl_ent;
	struct wtbl_struc *wtbl = &wtbl_ent.wtbl;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR wtbl_num = cap->WtblHwNum;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WTBL info of WLAN_IDX:%d\n", wtbl_idx));

	if (wtbl_idx == RESERVED_WCID) {
		start_idx = 0;
		end_idx = (wtbl_num - 1);
	} else if (wtbl_idx < wtbl_num)
		start_idx = end_idx = wtbl_idx;
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():Invalid WTBL index(%d)!\n",
				  __func__, wtbl_idx));
		return;
	}

	wtbl_len = sizeof(WTBL_STRUC);
	os_alloc_mem(pAd, (UCHAR **)&wtbl_raw_dw, wtbl_len);

	if (!wtbl_raw_dw) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():AllocMem fail(%d)!\n",
				  __func__, wtbl_idx));
		return;
	}

	for (idx = start_idx; idx <= end_idx; idx++) {
		wtbl_ent.wtbl_idx = idx;
		wtbl_ent.wtbl_addr = pAd->mac_ctrl.wtbl_base_addr[0] + idx * pAd->mac_ctrl.wtbl_entry_size[0];

		/* Read WTBL Entries */
		for (wtbl_offset = 0; wtbl_offset <= wtbl_len; wtbl_offset += 4) {
			addr = wtbl_ent.wtbl_addr + wtbl_offset;
			HW_IO_READ32(pAd->hdev_ctrl, addr, (UINT32 *)(&wtbl_raw_dw[wtbl_offset]));
		}

		NdisCopyMemory((UCHAR *)wtbl, &wtbl_raw_dw[0], sizeof(struct wtbl_struc));
		dump_wtbl_entry(pAd, &wtbl_ent);
	}

	os_free_mem(wtbl_raw_dw);
}


VOID dump_wtbl_entry(RTMP_ADAPTER *pAd, struct wtbl_entry *ent)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WTBL SW Entry[%d] info\n", ent->wtbl_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWTBL info:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tAddr=0x%x\n", ent->wtbl_addr));
	dump_wtbl_basic_info(pAd, &ent->wtbl);
}


INT mt_wtbl_get_entry234(RTMP_ADAPTER *pAd, UCHAR widx, struct wtbl_entry *ent)
{
	struct rtmp_mac_ctrl *wtbl_ctrl;
	UINT8 wtbl_idx;

	wtbl_ctrl = &pAd->mac_ctrl;

	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
		wtbl_idx = (widx < wtbl_ctrl->wtbl_entry_cnt[0] ? widx : wtbl_ctrl->wtbl_entry_cnt[0] - 1);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():pAd->mac_ctrl not init yet!\n", __func__));
		return FALSE;
	}

	ent->wtbl_idx = wtbl_idx;
	ent->wtbl_addr = wtbl_ctrl->wtbl_base_addr[0] +
					 wtbl_idx * wtbl_ctrl->wtbl_entry_size[0];
	return TRUE;
}

INT mt_wtbl_init_ByFw(struct _RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pAd->mac_ctrl.wtbl_base_addr[0] = (UINT32)WTBL_BASE_ADDR;
	pAd->mac_ctrl.wtbl_entry_size[0] = (UINT16)WTBL_PER_ENTRY_SIZE;
	pAd->mac_ctrl.wtbl_entry_cnt[0] = (UINT8)cap->WtblHwNum;
	return TRUE;
}

INT mt_wtbl_init_ByDriver(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pAd->mac_ctrl.wtbl_base_addr[0] = (UINT32)WTBL_BASE_ADDR;
	pAd->mac_ctrl.wtbl_entry_size[0] = (UINT16)WTBL_PER_ENTRY_SIZE;
	pAd->mac_ctrl.wtbl_entry_cnt[0] = (UINT8)cap->WtblHwNum;
	return TRUE;
}

INT mt_wtbl_init(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_WTBL_TLV_MODE
	/* We can use mt_wtbl_init_ByWtblTlv() to replace mt_wtbl_init_ByDriver() when there are no */
	/* function using mt_wtbl_get_entry234 and pAd->mac_ctrl */
	mt_wtbl_init_ByFw(pAd);
#else
	mt_wtbl_init_ByDriver(pAd);
#endif /* CONFIG_WTBL_TLV_MODE */
	return TRUE;
}
#define MCAST_WCID_TO_REMOVE 0
INT mt_hw_tb_init(RTMP_ADAPTER *pAd, BOOLEAN bHardReset)
{
	/* TODO: shiang-7603 */
	mt_wtbl_init(pAd);
	return TRUE;
}


/*
	ASIC register initialization sets
*/
INT mt_mac_init(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __func__));
	mt_mac_pse_init(pAd);
	MtAsicInitMac(pAd);

	/* re-set specific MAC registers for individual chip */
	/* TODO: Shiang-usw-win, here we need call "mt7603_init_mac_cr" for windows! */
	if (ops->AsicMacInit != NULL)
		ops->AsicMacInit(pAd);

	/* auto-fall back settings */
	MtAsicAutoFallbackInit(pAd);
	MtAsicSetMacMaxLen(pAd);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<--%s()\n", __func__));
	return TRUE;
}


VOID mt_chip_info_show(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("MAC[Ver:Rev/ID=0x%08x : 0x%08x]\n",
			  pAd->MACVersion, pAd->ChipID));
}

INT mt_nic_asic_init(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;
	MT_DMASCH_CTRL_T DmaSchCtrl;

	if (AsicWaitPDMAIdle(pAd, 100, 1000) != TRUE) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
			ret =  NDIS_STATUS_FAILURE;
			return ret;
		}
	}

#if  defined(COMPOS_WIN)  || defined(COMPOS_TESTMODE_WIN)
	DmaSchCtrl.bBeaconSpecificGroup = FALSE;
#else

	if (MTK_REV_GTE(pAd, MT7603, MT7603E1) && MTK_REV_LT(pAd, MT7603, MT7603E2))
		DmaSchCtrl.bBeaconSpecificGroup = FALSE;
	else
		DmaSchCtrl.bBeaconSpecificGroup = TRUE;

#endif
	DmaSchCtrl.mode = DMA_SCH_LMAC;
#ifdef DMA_SCH_SUPPORT
	MtAsicDMASchedulerInit(pAd, DmaSchCtrl);
#endif
#ifdef RTMP_PCI_SUPPORT

	if (IS_PCI_INF(pAd)) {
		pAd->CommonCfg.bPCIeBus = FALSE;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():device act as PCI%s driver\n",
				 __func__, (pAd->CommonCfg.bPCIeBus ? "-E" : "")));
	}

#endif /* RTMP_PCI_SUPPORT */
	/* TODO: shiang-7603, init MAC setting */
	/* TODO: shiang-7603, init beacon buffer */
	mt_mac_init(pAd);
	mt_hw_tb_init(pAd, TRUE);
#ifdef HDR_TRANS_RX_SUPPORT
	AsicRxHeaderTransCtl(pAd, TRUE, FALSE, FALSE, TRUE, FALSE);
	AsicRxHeaderTaranBLCtl(pAd, 0, TRUE, ETH_TYPE_EAPOL);
	AsicRxHeaderTaranBLCtl(pAd, 1, TRUE, ETH_TYPE_WAI);
	AsicRxHeaderTaranBLCtl(pAd, 2, TRUE, ETH_TYPE_FASTROAMING);
#endif

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
		MtAsicAutoBATrigger(pAd, TRUE, BA_TRIGGER_OFFLOAD_TIMEOUT);

	return ret;
}

UINT32 mtf_get_packet_type(RTMP_ADAPTER *ad, VOID *rx_packet)
{
	union _RMAC_RXD_0_UNION *rxd_0;

	rxd_0 = (union _RMAC_RXD_0_UNION *)(rx_packet);

#ifdef RT_BIG_ENDIAN
	mt_rmac_d0_endian_change(&rxd_0->word);
#endif /* RT_BIG_ENDIAN */

	return RMAC_RX_PKT_TYPE(rxd_0->word) ;
}

#define RMAC_INFO_BASE_SIZE 24
#define RMAC_INFO_GRP_1_SIZE 16
#define RMAC_INFO_GRP_2_SIZE 8
#define RMAC_INFO_GRP_3_SIZE 8
#define RMAC_INFO_GRP_4_SIZE 16
#define RMAC_INFO_GRP_5_SIZE 64
/*
    1'b0: the related GROUP is not present
    1'b1: the related GROUP is present

    bit[0]: indicates GROUP1 (DW10~DW13)
    bit[1]: indicates GROUP2 (DW14~DW15)
    bit[2]: indicates GROUP3 (DW16~DW17)
    bit[3]: indicates GROUP4 (DW6~DW9)
    bit[4]: indicates GROUP5 (DW18~DW33)
*/
INT32 mtf_trans_rxd_into_rxblk(RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, PNDIS_PACKET rx_pkt)
{

	UCHAR *rmac_info, *pos, *fc = NULL;
	struct rxd_grp_0 *rxd_grp0;
	struct rxd_grp_1 *rxd_grp1 = NULL;
	struct rxd_grp_2 *rxd_grp2 = NULL;
	struct rxd_grp_3 *rxd_grp3 = NULL;
	struct rxd_grp_4 *rxd_grp4 = NULL;
	struct rxd_grp_5 *rxd_grp5 = NULL;
	INT32 rmac_info_len = RMAC_INFO_BASE_SIZE;

	rmac_info = (UCHAR *)(GET_OS_PKT_DATAPTR(rx_pkt));
	pos = rmac_info;
	rx_blk->rmac_info = rmac_info;
	rxd_grp0 = (struct rxd_grp_0 *)rmac_info;
	pos += RMAC_INFO_BASE_SIZE;

	if (rxd_grp0->rxd_1 & RXD_GROUP4_VLD) {
		rxd_grp4  = (struct rxd_grp_4 *)pos;
		rmac_info_len += RMAC_INFO_GRP_4_SIZE;
		fc = pos;
		pos += RMAC_INFO_GRP_4_SIZE;
	}

	if (rxd_grp0->rxd_1 & RXD_GROUP1_VLD) {
		rxd_grp1 = (struct rxd_grp_1 *)pos;
		rmac_info_len += RMAC_INFO_GRP_1_SIZE;
		pos += RMAC_INFO_GRP_1_SIZE;
	}

	if (rxd_grp0->rxd_1 & RXD_GROUP2_VLD) {
		rxd_grp2 = (struct rxd_grp_2 *)pos;
		rmac_info_len += RMAC_INFO_GRP_2_SIZE;
		pos += RMAC_INFO_GRP_2_SIZE;
	}

	if (rxd_grp0->rxd_1 & RXD_GROUP3_VLD) {
		rxd_grp3 = (struct rxd_grp_3 *)pos;
		rmac_info_len += RMAC_INFO_GRP_3_SIZE;
		pos += RMAC_INFO_GRP_3_SIZE;
	}

	if (rxd_grp0->rxd_1 & RXD_GROUP5_VLD) {
		rxd_grp5 = (struct rxd_grp_5 *)pos;
		rmac_info_len += RMAC_INFO_GRP_5_SIZE;
		pos += RMAC_INFO_GRP_5_SIZE;
	}

	rx_blk->MPDUtotalByteCnt = (rxd_grp0->rxd_0 & RXD_RX_BYTE_COUNT_MASK) - rmac_info_len;

	if (rxd_grp0->rxd_2 & RXD_HO_MASK) {
		rx_blk->MPDUtotalByteCnt -= 2;
		rmac_info_len += 2;
	}

	rx_blk->DataSize = rx_blk->MPDUtotalByteCnt;
	rx_blk->wcid = (rxd_grp0->rxd_1 & RXD_WLAN_IDX_MASK);
	rx_blk->bss_idx = (rxd_grp0->rxd_2 & RXD_BSSID_MASK);
	rx_blk->key_idx = ((rxd_grp0->rxd_1 & RXD_KID_MASK) >> RXD_KID_SHIFT);
	rx_blk->TID = ((rxd_grp0->rxd_2 & RXD_TID_MASK) >> RXD_TID_SHIFT);

	if (rxd_grp0->rxd_2 & RXD_H)
		RX_BLK_SET_FLAG(rx_blk, fRX_HDR_TRANS);

	switch ((rxd_grp0->rxd_3 & RXD_A1_TYPE_MASK) >> RXD_A1_TYPE_SHIFT) {
	case 0x1:
		rx_blk->pRxInfo->U2M = 1;
		break;
	case 0x2:
		rx_blk->pRxInfo->Mcast = 1;
		break;
	case 0x3:
		rx_blk->pRxInfo->Bcast = 1;
		break;
	}

	rx_blk->AmsduState = (rxd_grp0->rxd_4 & RXD_PF_MASK);

	if (rxd_grp0->rxd_2 & RXD_DAF)
		rx_blk->DeAmsduFail = 1;

	if (rxd_grp0->rxd_2 & RXD_FRAG)
		rx_blk->pRxInfo->FRAG = 1;

	if (rxd_grp0->rxd_2 & RXD_NULL)
		rx_blk->pRxInfo->NULLDATA = 1;

	if (!(rxd_grp0->rxd_2 & RXD_NDATA))
		rx_blk->pRxInfo->DATA = 1;

	if (rxd_grp0->rxd_3 & RXD_HTC)
		rx_blk->pRxInfo->HTC = 1;

	if (!(rxd_grp0->rxd_2 & RXD_NAMP))
		rx_blk->pRxInfo->AMPDU = 1;

	rx_blk->pRxInfo->L2PAD = 0;
	rx_blk->pRxInfo->AMSDU = 0;

	/* 0: decryption okay, 1:ICV error, 2:MIC error, 3:KEY not valid */
	rx_blk->pRxInfo->CipherErr = (((rxd_grp0->rxd_1 & RXD_ICV_ERR) ? 1 : 0) |
					(((rxd_grp0->rxd_1 & RXD_TKIPMIC_ERR) ? 1 : 0) << 1));

	if (rxd_grp0->rxd_1 & RXD_FCS_ERR)
		rx_blk->pRxInfo->Crc = 1;

	rx_blk->pRxInfo->MyBss = ((rx_blk->bss_idx == 0xf) ? 0 : 1);
	rx_blk->pRxInfo->Decrypted = 0;

	SET_OS_PKT_DATAPTR(rx_pkt, GET_OS_PKT_DATAPTR(rx_pkt) + rmac_info_len);
	SET_OS_PKT_LEN(rx_pkt, rx_blk->MPDUtotalByteCnt);

	rx_blk->pRxPacket = rx_pkt;
	rx_blk->pData = (UCHAR *)GET_OS_PKT_DATAPTR(rx_pkt);

	if (RX_BLK_TEST_FLAG(rx_blk, fRX_HDR_TRANS)) {
		struct wifi_dev *wdev = NULL;

		if (!fc)
			return 0;

		rx_blk->FC = fc;
		rx_blk->FN = *((UINT16 *)(fc + 8)) & 0x000f;
		rx_blk->SN = (*((UINT16 *)(fc + 8)) & 0xfff0) >> 4;
		wdev = wdev_search_by_wcid(pAd, rx_blk->wcid);

		if (!wdev)
			wdev = wdev_search_by_omac_idx(pAd, rx_blk->bss_idx);

		if (!wdev)
			return 0;

		if ((((FRAME_CONTROL *)fc)->ToDs == 0) && (((FRAME_CONTROL *)fc)->FrDs == 0)) {
			rx_blk->Addr1 = rx_blk->pData;
			rx_blk->Addr2 = rx_blk->pData + 6;
			rx_blk->Addr3 = wdev->bssid;
		} else if ((((FRAME_CONTROL *)fc)->ToDs == 0) && (((FRAME_CONTROL *)fc)->FrDs == 1)) {
			rx_blk->Addr1 = rx_blk->pData;
			rx_blk->Addr2 = wdev->bssid;
			rx_blk->Addr3 = rx_blk->pData + 6;
		} else if ((((FRAME_CONTROL *)fc)->ToDs == 1) && (((FRAME_CONTROL *)fc)->FrDs == 0)) {
			rx_blk->Addr1 = wdev->bssid;
			rx_blk->Addr2 = rx_blk->pData + 6;
			rx_blk->Addr3 = rx_blk->pData;
		} else {
			rx_blk->Addr1 = wdev->if_addr;
			rx_blk->Addr2 = fc + 2;
			rx_blk->Addr3 = rx_blk->pData;
			rx_blk->Addr4 = rx_blk->pData + 6;
		}
	} else {
		rx_blk->FC = rx_blk->pData;
		rx_blk->Duration = *((UINT16 *)(rx_blk->pData + 2));

		if ((((FRAME_CONTROL *)rx_blk->FC)->Type == FC_TYPE_MGMT)
			|| (((FRAME_CONTROL *)rx_blk->FC)->Type == FC_TYPE_DATA)) {
			rx_blk->FN = *((UINT16 *)(rx_blk->pData + 22)) & 0x000f;
			rx_blk->SN = (*((UINT16 *)(rx_blk->pData + 22)) & 0xfff0) >> 4;


		}

		rx_blk->Addr1 = rx_blk->pData + 4;
		rx_blk->Addr2 = rx_blk->pData + 10;
		rx_blk->Addr3 = rx_blk->pData + 16;


		if ((((FRAME_CONTROL *)rx_blk->FC)->ToDs == 1) && (((FRAME_CONTROL *)rx_blk->FC)->FrDs == 1))
			rx_blk->Addr4 = rx_blk->pData + 24;
	}

	if (((FRAME_CONTROL *)rx_blk->FC)->SubType == SUBTYPE_AUTH) {

		if ((rxd_grp0->rxd_1 & RXD_SEC_MODE_MASK) &&
			(!(rxd_grp0->rxd_1 & RXD_CM)) &&
			(!(rxd_grp0->rxd_1 & RXD_CLM)))
			rx_blk->pRxInfo->Decrypted = 1;
	}

	return rmac_info_len;
}
