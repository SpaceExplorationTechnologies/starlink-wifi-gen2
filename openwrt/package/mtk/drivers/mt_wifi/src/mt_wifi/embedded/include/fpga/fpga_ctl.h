
#ifndef __FPGA_CTL_H__
#define __FPGA_CTL_H__


#ifdef MT_MAC
typedef enum _MT_MAC_TXS_TYPE {
	TXS_NONE = 0x0,

	TXS_DATA = 0x1ff,
	TXS_QDATA = 0x101,
	TXS_NON_QDATA = 0x102,

	TXS_MGMT = 0x2ff,
	TXS_BCN = 0x201,
	TXS_MGMT_OTHER = 0x202,

	TXS_CTRL = 0x4ff,
	TXS_ALL = 0xfff,
} MT_MAC_TXS_TYPE;
#endif /* MT_MAC */


struct manual_conn {
	UINT8 peer_mac[MAC_ADDR_LEN];
	UINT8 peer_band;		/* band 0/1 */
	UINT8 peer_op_type;		/* ap/sta for "OPMODE_AP"/"OPMODE_STA" */
	UINT8 ownmac_idx;
	UINT8 wtbl_idx;
	UINT8 peer_phy_mode;	/* a/b/g/gn/an/ac for "WMODE_A/B/G/GN/AN/AC" */
	UINT8 peer_bw;			/* 20/40/80/160 for "BW_20/40/80/160" */
	UINT8 peer_nss;			/* 1 ~ 4 */
	UINT16 pfmuId;
	UINT16 aid;
	UINT8 peer_maxrate_mode;	/* cck/ofdm/htmix/htgf/vht for "MODE_CCK/OFDM/HTMIX/HTGF/VHT" */
	UINT32 peer_maxrate_mcs;	/* MODE_CCK: 0~3, MODE_OFDM: 0~7, MODE_HTMIX/GF: 0~32, MODE_VHT:0~9 */
	UINT8 ba_info[WMM_NUM_OF_AC];

	/* protocol wise */
	/* HT_CAP_INFO ht_cap_info; */	/* HT capability information */
	/* VHT_CAP_INFO vht_cap_info; */	/* VHT capability information */
	/* VHT_MCS_SET vht_mcs_set; */
};


struct fpga_ctrl {
	int tx_kick_cnt;
	int phy_rates;
	int tx_data_phy;
	UINT8 tx_data_bw;
	UINT8 tx_data_ldpc;
	UINT8 tx_data_mcs;
	UINT8 tx_data_nss;
	UINT8 tx_data_gi;
	UINT8 tx_data_stbc;
	int rx_data_phy;
	UINT8 rx_data_bw;
	UINT8 rx_data_ldpc;
	UINT8 rx_data_mcs;
	UINT8 rx_data_gi;
	UINT8 rx_data_stbc;
	UINT8 data_basize;
	UINT8 fpga_on;
	UINT8 fpga_tr_stop;
	UINT8 vco_cal;

	UINT8 dma_mode;

#ifdef MT_MAC
	MT_MAC_TXS_TYPE txs_type;
	UCHAR no_bcn;
#ifdef AUTOMATION
	UINT8 txrx_dbg_type;
#endif /* AUTOMATION */
	struct manual_conn manual_conn_info;
	UCHAR assoc_type;
#endif /* MT_MAC */

#ifdef CAPTURE_MODE
	BOOLEAN cap_support;	/* 0: no cap mode; 1: cap mode enable */
	UCHAR cap_type;			/* 1: ADC6, 2: ADC8, 3: FEQ */
	UCHAR cap_trigger;		/* 1: manual trigger, 2: auto trigger */
	BOOLEAN do_cap;			/* 1: start to do cap, if auto, will triggered depends on trigger condition, if manual, start immediately */
	BOOLEAN cap_done;		/* 1: capture done, 0: capture not finish yet */
	UINT32 trigger_offset;	/* in unit of bytes */
	UCHAR *cap_buf;
#endif /* CAPTURE_MODE */
};

#endif

