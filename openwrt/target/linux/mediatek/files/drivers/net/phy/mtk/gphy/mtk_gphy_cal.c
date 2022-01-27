
#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char


#define ANACAL_INIT             0x01
#define ANACAL_ERROR            0xFD
#define ANACAL_SATURATION       0xFE
#define ANACAL_FINISH           0xFF
#define ANACAL_PAIR_A           0
#define ANACAL_PAIR_B           1
#define ANACAL_PAIR_C           2
#define ANACAL_PAIR_D           3
#define DAC_IN_0V               0x000
#define DAC_IN_2V               0x0f0
#define TX_AMP_OFFSET_0MV       0x20
#define TX_AMP_OFFSET_VALID_BITS        6
#define FE_CAL_P0                       0
#define FE_CAL_P1                       1
#if 1//defined(CONFIG_MACH_LEOPARD)
#define FE_CAL_COMMON                   1
#else
#define FE_CAL_COMMON                   0
#endif

#define CALDLY 40

static u8 fe_cal_flag;
static u8 fe_cal_flag_mdix;
static u8 fe_cal_tx_offset_flag;
static u8 fe_cal_tx_offset_flag_mdix;
static u8 fe_cal_r50_flag;
static u8 fe_cal_vbg_flag;
static u32 iext_cal_result;
static u32 r50_p0_cal_result;
static u8 ge_cal_r50_flag;
static u8 ge_cal_tx_offset_flag;
static u8 ge_cal_flag;
static int show_time =0 ;
static u8 ephy_addr_base;

/* 50ohm_new*/
const u8 ZCAL_TO_R50OHM_TBL_100[64] = {
	127, 121, 116, 115, 111, 109, 108, 104,
	102, 99, 97, 96, 77, 76, 73, 72,
	70, 69, 67, 66, 47, 46, 45, 43,
	42, 41, 40, 38, 37, 36, 35, 34,
	32, 16, 15, 14, 13, 12, 11, 10,
	9, 8, 7, 6, 6, 5, 4, 4,
	3, 2, 2, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

const u8 ZCAL_TO_R50ohm_GE_TBL_100[64] = {
	63, 63, 63, 63, 63, 63, 63, 63,
	63, 63, 63, 63, 63, 63, 63, 60,
	57, 55, 53, 51, 48, 46, 44, 42,
	40, 38, 37, 36, 34, 32, 30, 28,
	27, 26, 25, 23, 22, 21, 19, 18,
	16, 15, 14, 13, 12, 11, 10, 9,
	8, 7, 6, 5, 4, 4, 3, 2,
	1, 0, 0, 0, 0, 0, 0, 0
};

const u8 ZCAL_TO_R50ohm_GE_TBL[64] = {
	63, 63, 63, 63, 63, 63, 63, 63,
	63, 63, 63, 63, 63, 63, 63, 60,
	57, 55, 53, 51, 48, 46, 44, 42,
	40, 38, 37, 36, 34, 32, 30, 28,
	27, 26, 25, 23, 22, 21, 19, 18,
	16, 15, 14, 13, 12, 11, 10, 9,
	8, 7, 6, 5, 4, 4, 3, 2,
	1, 0, 0, 0, 0, 0, 0, 0
};

const u8 ZCAL_TO_REXT_TBL[64] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 1, 1, 1, 1,
	1, 2, 2, 2, 2, 2, 2, 3,
	3, 3, 3, 3, 3, 4, 4, 4,
	4, 4, 4, 4, 5, 5, 5, 5,
	5, 5, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7
};

const u8 ZCAL_TO_FILTER_TBL[64] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 1,
	1, 2, 2, 2, 3, 3, 3, 4,
	4, 4, 4, 5, 5, 5, 6, 6,
	7, 7, 7, 8, 8, 8, 9, 9,
	9, 10, 10, 10, 11, 11, 11, 11,
	12, 12, 12, 12, 12, 12, 12, 12
};

static inline void tc_phy_write_g_reg(u8 port_num, u8 page_num,
			u8 reg_num, u32 reg_data)
{
	u32 r31 = 0;

	r31 |= 0 << 15;	/* global */
	r31 |= ((page_num & 0x7) << 12);	/* page no */
	mii_mgr_write(port_num, 31, r31);	/* change Global page */
	mii_mgr_write(port_num, reg_num, reg_data);
}

static inline void tc_phy_write_l_reg(u8 port_no, u8 page_no,
			u8 reg_num, u32 reg_data)
{
	u32 r31 = 0;

	r31 |= 1 << 15;	/* local */
	r31 |= ((page_no & 0x7) << 12);	/* page no */
	mii_mgr_write(port_no, 31, r31); /* select local page x */
	mii_mgr_write(port_no, reg_num, reg_data);
}

static inline u32 tc_phy_read_g_reg(u8 port_num, u8 page_num, u8 reg_num)
{
	u32 phy_val;

	u32 r31 = 0;

	r31 |= 0 << 15;	/* global */
	r31 |= ((page_num & 0x7) << 12);	/* page no */
	mii_mgr_write(port_num, 31, r31);	/* change Global page */
	mii_mgr_read(port_num, reg_num, &phy_val);
	return phy_val;
}

static inline u32 tc_phy_read_l_reg(u8 port_no, u8 page_no, u8 reg_num)
{
	u32 phy_val;
	u32 r31 = 0;

	r31 |= 1 << 15;	/* local */
	r31 |= ((page_no & 0x7) << 12);	/* page no */
	mii_mgr_write(port_no, 31, r31); /* select local page x */
	mii_mgr_read(port_no, reg_num, &phy_val);
	return phy_val;
}

static inline u32 tc_phy_read_dev_reg(u32 port_no, u32 dev_addr, u32 reg_addr)
{
	u32 phy_val;
	//mii_mgr_read_cl45(port_num, dev_addr, reg_addr, &phy_val);
	mii_mgr_write(port_no, 13, dev_addr );
	mii_mgr_write(port_no, 14, reg_addr  );
	mii_mgr_write(port_no, 13, 0x4000 | dev_addr);
	mii_mgr_read(port_no, 14, &phy_val );
	
	return phy_val;
}

static inline void tc_phy_write_dev_reg(u32 port_no, u32 dev_addr, u32 reg_addr, u32 write_data)
{
	//mii_mgr_write_cl45(port_num, dev_addr, reg_addr, write_data);
	mii_mgr_write(port_no, 13, dev_addr );
	mii_mgr_write(port_no, 14,reg_addr  );
	mii_mgr_write(port_no, 13, 0x4000 | dev_addr);
	mii_mgr_write(port_no, 14, write_data );
	
}

static inline u32 tc_mii_read(u32 phy_addr, u32 phy_register)
{
	u32 phy_val;

	mii_mgr_read(phy_addr, phy_register, &phy_val);
	return phy_val;
}

static inline void tc_mii_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
	mii_mgr_write(phy_addr, phy_register, write_data);
}

static void clear_ckinv_ana_txvos(void)
{
	u16 g7r24_tmp;
	/*clear RG_CAL_CKINV/RG_ANA_CALEN/RG_TXVOS_CALEN*/
	/*g7r24[13]:0x0, RG_ANA_CALEN_P0*/
	g7r24_tmp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, (g7r24_tmp & (~0x2000)));

	/*g7r24[14]:0x0, RG_CAL_CKINV_P0*/
	g7r24_tmp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, (g7r24_tmp & (~0x4000)));

	/*g7r24[12]:0x0, DA_TXVOS_CALEN_P0*/
	g7r24_tmp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, (g7r24_tmp & (~0x1000)));
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, 0);
}

static u8 all_fe_ana_cal_wait_txamp(u32 delay, u8 port_num)
{				/* for EN7512 FE // allen_20160616 */
	u8 all_ana_cal_status;
	u16 cnt, g7r24_temp;

	tc_phy_write_l_reg(FE_CAL_COMMON, 4, 23, (0x0000));
	g7r24_temp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, g7r24_temp & (~0x10));
	g7r24_temp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, g7r24_temp | 0x10);

	cnt = 1000;
	do {
		udelay(delay);
		cnt--;
		all_ana_cal_status =
		    ((tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) >> 1) & 0x1);
	} while ((all_ana_cal_status == 0) && (cnt != 0));

	tc_phy_write_l_reg(FE_CAL_COMMON, 4, 23, (0x0000));
	tc_phy_write_l_reg(port_num, 4, 23, (0x0000));
	g7r24_temp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, g7r24_temp & (~0x10));
	return all_ana_cal_status;
}

static u8 all_fe_ana_cal_wait(u32 delay, u8 port_num)
{
	u8 all_ana_cal_status;
	u16 cnt, g7r24_temp;

	tc_phy_write_l_reg(FE_CAL_COMMON, 4, 23, (0x0000));
	tc_phy_write_l_reg(port_num, 4, 23, (0x0000));

	g7r24_temp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, g7r24_temp & (~0x10));
	g7r24_temp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, g7r24_temp | 0x10);
	cnt = 1000;
	do {
		udelay(delay);
		cnt--;
		all_ana_cal_status =
		    ((tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) >> 1) & 0x1);

	} while ((all_ana_cal_status == 0) && (cnt != 0));

	tc_phy_write_l_reg(FE_CAL_COMMON, 4, 23, (0x0000));
	tc_phy_write_l_reg(port_num, 4, 23, (0x0000));
	g7r24_temp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, g7r24_temp & (~0x10));

	return all_ana_cal_status;
}

static void fe_cal_tx_amp(u8 port_num, u32 delay)
{
	u8 all_ana_cal_status;
	int ad_cal_comp_out_init;
	u16 l3r25_temp, l0r26_temp, l2r20_temp;
	u16 l2r23_temp = 0;
	int calibration_polarity;
	u8 tx_amp_reg_shift = 0;
	int tx_amp_temp = 0, cnt = 0, phyaddr, tx_amp_cnt = 0;
	u16 tx_amp_final;
	//struct END_DEVICE *ei_local = netdev_priv(dev_raether);

	phyaddr = port_num + ephy_addr_base;
	tx_amp_temp = 0x20;
	/* *** Tx Amp Cal start ********************** */

/*Set device in 100M mode*/
	tc_phy_write_l_reg(port_num, 0, 0, 0x2100);
/*TXG output DC differential 1V*/
	tc_phy_write_g_reg(port_num, 2, 25, 0x10c0);

	tc_phy_write_g_reg(port_num, 1, 26, (0x8000 | DAC_IN_2V));
	tc_phy_write_g_reg(port_num, 4, 21, (0x0800));	/* set default */
	tc_phy_write_l_reg(port_num, 0, 30, (0x02c0));
	tc_phy_write_l_reg(port_num, 4, 21, (0x0000));

	tc_phy_write_l_reg(FE_CAL_COMMON, 3, 25, (0xc800));
	tc_phy_write_l_reg(port_num, 3, 25, (0xc800));

	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, 0x7000);

	l3r25_temp = tc_phy_read_l_reg(port_num, 3, 25);
	tc_phy_write_l_reg(port_num, 3, 25, (l3r25_temp | 0x400));

	/*decide which port calibration RG_ZCALEN by port_num*/
	l3r25_temp = tc_phy_read_l_reg(port_num, 3, 25);
	l3r25_temp = l3r25_temp | 0x1000;
	l3r25_temp = l3r25_temp & ~(0x200);
	tc_phy_write_l_reg(port_num, 3, 25, l3r25_temp);

	/*DA_PGA_MDIX_STASTUS_P0=0(L0R26[15:14] = 0x01*/
	l0r26_temp = tc_phy_read_l_reg(port_num, 0, 26);
	l0r26_temp = l0r26_temp & (~0xc000);
	tc_phy_write_l_reg(port_num, 0, 26, 0x5203);/* Kant */

	/*RG_RX2TX_EN_P0=0(L2R20[10] =0),*/
	l2r20_temp = tc_phy_read_l_reg(port_num, 2, 20);
	l2r20_temp = l2r20_temp & (~0x400);
	tc_phy_write_l_reg(port_num, 2, 20, l2r20_temp);
	tc_phy_write_l_reg(port_num, 2, 23, (tx_amp_temp));

	all_ana_cal_status = all_fe_ana_cal_wait_txamp(delay, port_num);

	if (all_ana_cal_status == 0) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info(" FE Tx amp AnaCal ERROR! (init)  \r\n");
	}

	tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	ad_cal_comp_out_init = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1;

	if (ad_cal_comp_out_init == 1)
		calibration_polarity = -1;
	else
		calibration_polarity = 1;

	tx_amp_temp += calibration_polarity;
	cnt = 0;
	tx_amp_cnt = 0;
	while (all_ana_cal_status < ANACAL_ERROR) {
		tc_phy_write_l_reg(port_num, 2, 23, (tx_amp_temp));
		l2r23_temp = tc_phy_read_l_reg(port_num, 2, 23);
		cnt++;
		tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
		tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
		all_ana_cal_status = all_fe_ana_cal_wait_txamp(delay, port_num);

		if (((tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24)) & 0x1) !=
		    ad_cal_comp_out_init) {
			all_ana_cal_status = ANACAL_FINISH;
			fe_cal_flag = 1;
		}
		if (all_ana_cal_status == 0) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info(" FE Tx amp AnaCal ERROR! (%d)  \r\n", cnt);
		} else if ((tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1) !=
			   ad_cal_comp_out_init) {
			tx_amp_cnt++;
			all_ana_cal_status = ANACAL_FINISH;
			tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
			tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
			ad_cal_comp_out_init =
			    tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1;
		} else {
			if ((l2r23_temp == 0x3f) || (l2r23_temp == 0x00)) {
				all_ana_cal_status = ANACAL_SATURATION;
				pr_info
				    (" Tx amp Cal Saturation(%d)(%x)(%x)\r\n",
				     cnt, tc_phy_read_l_reg(0, 3, 25),
				     tc_phy_read_l_reg(1, 3, 25));
				pr_info
				    (" Tx amp Cal Saturation(%x)(%x)(%x)\r\n",
				     tc_phy_read_l_reg(2, 3, 25),
				     tc_phy_read_l_reg(3, 3, 25),
				     tc_phy_read_l_reg(0, 2, 30));
				/* tx_amp_temp += calibration_polarity; */
			} else {
				tx_amp_temp += calibration_polarity;
			}
		}
	}

	if ((all_ana_cal_status == ANACAL_ERROR) ||
	    (all_ana_cal_status == ANACAL_SATURATION)) {
		l2r23_temp = tc_phy_read_l_reg(port_num, 2, 23);
		tc_phy_write_l_reg(port_num, 2, 23,
				   ((tx_amp_temp << tx_amp_reg_shift)));
		l2r23_temp = tc_phy_read_l_reg(port_num, 2, 23);
		pr_info("[%d] %s, ANACAL_SATURATION\n", port_num, __func__);
	} else {
#if 0	
		if (ei_local->chip_name == MT7622_FE) {
			if (port_num == 0)
				l2r23_temp = l2r23_temp + 10;
			else if (port_num == 1)
				l2r23_temp = l2r23_temp + 11;
			else if (port_num == 2)
				l2r23_temp = l2r23_temp + 10;
			else if (port_num == 3)
				l2r23_temp = l2r23_temp + 9;
			else if (port_num == 4)
				l2r23_temp = l2r23_temp + 10;
		} else if (ei_local->chip_name == LEOPARD_FE) {
#endif		
			if (port_num == 1)
				l2r23_temp = l2r23_temp + 3;
			else if (port_num == 2)
				l2r23_temp = l2r23_temp + 3;
			else if (port_num == 3)
				l2r23_temp = l2r23_temp + 3 - 2;
			else if (port_num == 4)
				l2r23_temp = l2r23_temp + 2 - 1 + 2;
		//}

		tc_phy_write_l_reg(port_num, 2, 23, ((l2r23_temp) << tx_amp_reg_shift));
		fe_cal_flag = 1;
	}

	tx_amp_final = tc_phy_read_l_reg(port_num, 2, 23) & 0x3f;
	tc_phy_write_l_reg(port_num, 2, 24, ((tx_amp_final + 15)  << 8) | 0x20);

	//if (ei_local->chip_name == LEOPARD_FE) {
		if (port_num == 1)
			tc_phy_write_l_reg(port_num, 2, 24, ((tx_amp_final + 15 - 4)  << 8) | 0x20);
		else if (port_num == 2)
			tc_phy_write_l_reg(port_num, 2, 24, ((tx_amp_final + 15 + 2)  << 8) | 0x20);
		else if (port_num == 3)
			tc_phy_write_l_reg(port_num, 2, 24, ((tx_amp_final + 15 + 4)  << 8) | 0x20);
		else if (port_num == 4)
			tc_phy_write_l_reg(port_num, 2, 24, ((tx_amp_final + 15 + 4)  << 8) | 0x20);
	//}

	pr_info("[%d] - tx_amp_final = 0x%x\n", port_num, tx_amp_final);

	/*clear RG_CAL_CKINV/RG_ANA_CALEN/RG_TXVOS_CALEN*/
	clear_ckinv_ana_txvos();

	tc_phy_write_l_reg(port_num, 3, 25, 0x0000);
	tc_phy_write_l_reg(FE_CAL_COMMON, 3, 25, 0x0000);
	tc_phy_write_g_reg(port_num, 1, 26, 0);
	/* *** Tx Amp Cal end *** */
}

static void fe_cal_tx_amp_mdix(u8 port_num, u32 delay)
{
	u8 all_ana_cal_status;
	int ad_cal_comp_out_init;
	u16 l3r25_temp, l4r26_temp, l0r26_temp;
	u16 l2r20_temp, l4r26_temp_amp;
	int calibration_polarity;
	int tx_amp_temp = 0, cnt = 0, phyaddr, tx_amp_cnt = 0;
	u16 tx_amp_mdix_final;
	//struct END_DEVICE *ei_local = netdev_priv(dev_raether);

	phyaddr = port_num + ephy_addr_base;
	tx_amp_temp = 0x20;
/*Set device in 100M mode*/
	tc_phy_write_l_reg(port_num, 0, 0, 0x2100);
/*TXG output DC differential 0V*/
	tc_phy_write_g_reg(port_num, 2, 25, 0x10c0);

	tc_phy_write_g_reg(port_num, 1, 26, (0x8000 | DAC_IN_2V));
	tc_phy_write_g_reg(port_num, 4, 21, (0x0800));	/* set default */
	tc_phy_write_l_reg(port_num, 0, 30, (0x02c0));/*0x3f80  // l0r30[9], [7], [6], [1]*/
	tc_phy_write_l_reg(port_num, 4, 21, (0x0000));
	tc_phy_write_l_reg(FE_CAL_COMMON, 3, 25, (0xc800));
	tc_phy_write_l_reg(port_num, 3, 25, (0xc800));	/* 0xca00 */
	/* *** Tx Amp Cal start ********************** */
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, 0x7000);
	/* pr_info(" g7r24[%d] = %x\n", port_num, tc_phy_read_g_reg(port_num, 7, 24)); */

	/*RG_TXG_CALEN =1 l3r25[10]by port number*/
	l3r25_temp = tc_phy_read_l_reg(port_num, 3, 25);
	tc_phy_write_l_reg(port_num, 3, 25, (l3r25_temp | 0x400));
	/*decide which port calibration RG_ZCALEN l3r25[12] by port_num*/
	l3r25_temp = tc_phy_read_l_reg(port_num, 3, 25);
	l3r25_temp = l3r25_temp | 0x1000;
	l3r25_temp = l3r25_temp & ~(0x200);
	tc_phy_write_l_reg(port_num, 3, 25, l3r25_temp);

	/*DA_PGA_MDIX_STASTUS_P0=0(L0R26[15:14] = 0x10) & RG_RX2TX_EN_P0=0(L2R20[10] =1),*/
	l0r26_temp = tc_phy_read_l_reg(port_num, 0, 26);
	l0r26_temp = l0r26_temp & (~0xc000);
	tc_phy_write_l_reg(port_num, 0, 26, 0x9203); /* Kant */
	l2r20_temp = tc_phy_read_l_reg(port_num, 2, 20);
	l2r20_temp = l2r20_temp | 0x400;
	tc_phy_write_l_reg(port_num, 2, 20, l2r20_temp);

	l3r25_temp = tc_phy_read_l_reg(port_num, 3, 25);
	tc_phy_write_l_reg(port_num, 3, 25, (l3r25_temp | 0x0400));
/*DA_TX_I2MPB_MDIX L4R26[5:0]*/
	l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
	/* pr_info("111l4r26 =%x\n", tc_phy_read_l_reg(port_num, 4, 26)); */
	l4r26_temp = l4r26_temp & (~0x3f);
	tc_phy_write_l_reg(port_num, 4, 26, (l4r26_temp | tx_amp_temp));
	/* pr_info("222l4r26 =%x\n", tc_phy_read_l_reg(port_num, 4, 26)); */
	all_ana_cal_status = all_fe_ana_cal_wait_txamp(delay, port_num);

	if (all_ana_cal_status == 0) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info(" FE Tx amp mdix AnaCal ERROR! (init)  \r\n");
	}

	tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	/*ad_cal_comp_out_init = (tc_phy_read_l_reg(FE_CAL_COMMON, 4, 23) >> 4) & 0x1;*/
	ad_cal_comp_out_init = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1;
	/* pr_info("mdix ad_cal_comp_out_init = %d\n", ad_cal_comp_out_init); */
	if (ad_cal_comp_out_init == 1) {
		calibration_polarity = -1;
		/* tx_amp_temp = 0x10; */
	} else {
		calibration_polarity = 1;
	}
	tx_amp_temp += calibration_polarity;
	cnt = 0;
	tx_amp_cnt = 0;
	while (all_ana_cal_status < ANACAL_ERROR) {
		l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
		l4r26_temp = l4r26_temp & (~0x3f);
		tc_phy_write_l_reg(port_num, 4, 26, (l4r26_temp | tx_amp_temp));
		l4r26_temp = (tc_phy_read_l_reg(port_num, 4, 26));
		l4r26_temp_amp = (tc_phy_read_l_reg(port_num, 4, 26)) & 0x3f;
		cnt++;

		tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
		tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
		all_ana_cal_status = all_fe_ana_cal_wait_txamp(delay, port_num);

		if (((tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24)) & 0x1) !=
		    ad_cal_comp_out_init) {
			all_ana_cal_status = ANACAL_FINISH;
			fe_cal_flag_mdix = 1;
		}
		if (all_ana_cal_status == 0) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info(" FE Tx amp mdix AnaCal ERROR! (%d)  \r\n", cnt);
		} else if (((tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24)) & 0x1) !=
			   ad_cal_comp_out_init) {
			all_ana_cal_status = ANACAL_FINISH;
			tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
			tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
			ad_cal_comp_out_init =
			    (tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24)) & 0x1;
		} else {
			if ((l4r26_temp_amp == 0x3f) || (l4r26_temp_amp == 0x00)) {
				all_ana_cal_status = ANACAL_SATURATION;
				pr_info
				    (" Tx amp Cal mdix Saturation(%d)(%x)(%x)\r\n",
				     cnt, tc_phy_read_l_reg(0, 3, 25),
				     tc_phy_read_l_reg(1, 3, 25));
				pr_info
				    (" Tx amp Cal mdix Saturation(%x)(%x)(%x)\r\n",
				     tc_phy_read_l_reg(2, 3, 25),
				     tc_phy_read_l_reg(3, 3, 25),
				     tc_phy_read_l_reg(0, 2, 30));
				/* tx_amp_temp += calibration_polarity; */
			} else {
				tx_amp_temp += calibration_polarity;
			}
		}
	}

	if ((all_ana_cal_status == ANACAL_ERROR) ||
	    (all_ana_cal_status == ANACAL_SATURATION)) {
		l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
		pr_info(" FE-%d Tx amp AnaCal mdix Saturation! (%d)(l4r26=0x%x)  \r\n",
			phyaddr, cnt, l4r26_temp);
		tc_phy_write_l_reg(port_num, 4, 26,
				   ((l4r26_temp & (~0x3f)) | tx_amp_temp));
		l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
		pr_info(" FE-%d Tx amp AnaCal mdix Saturation! (%d)(l4r26=0x%x)  \r\n",
			phyaddr, cnt, l4r26_temp);
		pr_info("[%d] %s, ANACAL_SATURATION\n", port_num, __func__);
	} else {
	#if 0
		if (ei_local->chip_name == MT7622_FE) {
			if (port_num == 0) {
				l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
				l4r26_temp = l4r26_temp + 10;
			} else if (port_num == 1) {
				l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
				l4r26_temp = l4r26_temp + 11;
			} else if (port_num == 2) {
				l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
				l4r26_temp = l4r26_temp + 9;
			} else if (port_num == 3) {
				l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
				l4r26_temp = l4r26_temp + 9;
			} else if (port_num == 4) {
				l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
				l4r26_temp = l4r26_temp + 9;
			}
		} else if (ei_local->chip_name == LEOPARD_FE) {
	#endif	
			if (port_num == 1) {
				l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
				l4r26_temp = l4r26_temp + 4 - 2;
			} else if (port_num == 2) {
				l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
				l4r26_temp = l4r26_temp + 3 - 1;
			} else if (port_num == 3) {
				l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
				l4r26_temp = l4r26_temp + 4 - 3;
			} else if (port_num == 4) {
				l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
				l4r26_temp = l4r26_temp + 4 - 2 + 1;
			}
		//}
		tc_phy_write_l_reg(port_num, 4, 26, l4r26_temp);
		fe_cal_flag_mdix = 1;
	}

	tx_amp_mdix_final = tc_phy_read_l_reg(port_num, 4, 26) & 0x3f;
	tc_phy_write_l_reg(port_num, 4, 27, ((tx_amp_mdix_final + 15) << 8) | 0x20);
	//if (ei_local->chip_name == LEOPARD_FE) {
		if (port_num == 2)
			tc_phy_write_l_reg(port_num, 4, 27,
					   ((tx_amp_mdix_final + 15 + 1)  << 8) | 0x20);
		else if (port_num == 3)
			tc_phy_write_l_reg(port_num, 4, 27,
					   ((tx_amp_mdix_final + 15 + 4)  << 8) | 0x20);
		else if (port_num == 4)
			tc_phy_write_l_reg(port_num, 4, 27,
					   ((tx_amp_mdix_final + 15 + 4)  << 8) | 0x20);
	//}
	pr_info("[%d] - tx_amp_mdix_final = 0x%x\n", port_num, tx_amp_mdix_final);

	clear_ckinv_ana_txvos();
	tc_phy_write_l_reg(port_num, 3, 25, 0x0000);
	tc_phy_write_l_reg(FE_CAL_COMMON, 3, 25, 0x0000);
	tc_phy_write_g_reg(port_num, 1, 26, 0);
	/* *** Tx Amp Cal end *** */
}

static void fe_cal_tx_offset(u8 port_num, u32 delay)
{
	u8 all_ana_cal_status;
	int ad_cal_comp_out_init;
	u16 l3r25_temp, l2r20_temp;
	u16 g4r21_temp, l0r30_temp, l4r17_temp, l0r26_temp;
	int calibration_polarity, tx_offset_temp;
	int cal_temp = 0;
	u8 tx_offset_reg_shift;
	u8 cnt = 0, phyaddr, tx_amp_cnt = 0;
	u16 tx_offset_final;

	phyaddr = port_num + ephy_addr_base;
/*Set device in 100M mode*/
	tc_phy_write_l_reg(port_num, 0, 0, 0x2100);

	/*// g4r21[11]:Hw bypass tx offset cal, Fw cal*/
	g4r21_temp = tc_phy_read_g_reg(port_num, 4, 21);
	tc_phy_write_g_reg(port_num, 4, 21, (g4r21_temp | 0x0800));

	/*l0r30[9], [7], [6], [1]*/
	l0r30_temp = tc_phy_read_l_reg(port_num, 0, 30);
	tc_phy_write_l_reg(port_num, 0, 30, (l0r30_temp | 0x02c0));

	/* tx_offset_temp = TX_AMP_OFFSET_0MV; */
	tx_offset_temp = 0x20;
	tx_offset_reg_shift = 8;
	tc_phy_write_g_reg(port_num, 1, 26, (0x8000 | DAC_IN_0V));

	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, 0x3000);
	/* pr_info(" g7r24[%d] = %x\n", port_num, tc_phy_read_g_reg(port_num, 7, 24)); */
	/*RG_TXG_CALEN =1 by port number*/
	l3r25_temp = tc_phy_read_l_reg(port_num, 3, 25);
	tc_phy_write_l_reg(port_num, 3, 25, (l3r25_temp | 0x400));
	/*decide which port calibration RG_ZCALEN by port_num*/
	l3r25_temp = tc_phy_read_l_reg(port_num, 3, 25);
	tc_phy_write_l_reg(port_num, 3, 25, (l3r25_temp | 0x1000));

	/*DA_PGA_MDIX_STASTUS_P0=0(L0R26[15:14] = 0x01) & RG_RX2TX_EN_P0=0(L2R20[10] =0),*/
	l0r26_temp = tc_phy_read_l_reg(port_num, 0, 26);
	l0r26_temp = l0r26_temp & (~0xc000);
	/* tc_phy_write_l_reg(port_num, 0, 26, (l0r26_temp | 0x4000)); */
	tc_phy_write_l_reg(port_num, 0, 26, 0x5203);/* Kant */
	/* pr_info("l0r26[%d] = %x\n", port_num, tc_phy_read_l_reg(port_num, 0, 26)); */
	l2r20_temp = tc_phy_read_l_reg(port_num, 2, 20);
	l2r20_temp = l2r20_temp & (~0x400);
	tc_phy_write_l_reg(port_num, 2, 20, l2r20_temp);
	/* pr_info("l2r20[%d] = %x\n", port_num, tc_phy_read_l_reg(port_num, 2, 20)); */

	tc_phy_write_l_reg(port_num, 4, 17, (0x0000));
	l4r17_temp = tc_phy_read_l_reg(port_num, 4, 17);
	tc_phy_write_l_reg(port_num, 4, 17,
			   l4r17_temp |
			   (tx_offset_temp << tx_offset_reg_shift));
/*wat AD_CAL_CLK = 1*/
	all_ana_cal_status = all_fe_ana_cal_wait(delay, port_num);
	if (all_ana_cal_status == 0) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info(" FE Tx offset AnaCal ERROR! (init)  \r\n");
	}

	tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
/*GET AD_CAL_COMP_OUT g724[0]*/
	/*ad_cal_comp_out_init = (tc_phy_read_l_reg(FE_CAL_COMMON, 4, 23) >> 4) & 0x1;*/
	ad_cal_comp_out_init = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1;

	if (ad_cal_comp_out_init == 1)
		calibration_polarity = -1;
	else
		calibration_polarity = 1;
	cnt = 0;
	tx_amp_cnt = 0;
	tx_offset_temp += calibration_polarity;

	while ((all_ana_cal_status < ANACAL_ERROR) && (cnt < 254)) {
		cnt++;
		cal_temp = tx_offset_temp;
		tc_phy_write_l_reg(port_num, 4, 17,
				   (cal_temp << tx_offset_reg_shift));

		tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
		tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
		all_ana_cal_status = all_fe_ana_cal_wait(delay, port_num);

		if (all_ana_cal_status == 0) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info(" FE Tx offset AnaCal ERROR! (%d)  \r\n", cnt);
		} else if ((tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1) !=
			   ad_cal_comp_out_init) {
			all_ana_cal_status = ANACAL_FINISH;
			tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
			tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);

			ad_cal_comp_out_init =
			    tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1;
		} else {
			l4r17_temp = tc_phy_read_l_reg(port_num, 4, 17);

			if ((tx_offset_temp == 0x3f) || (tx_offset_temp == 0x00)) {
				all_ana_cal_status = ANACAL_SATURATION;
				pr_info("tx offset ANACAL_SATURATION\n");
			} else {
				tx_offset_temp += calibration_polarity;
			}
		}
	}

	if ((all_ana_cal_status == ANACAL_ERROR) ||
	    (all_ana_cal_status == ANACAL_SATURATION)) {
		tx_offset_temp = TX_AMP_OFFSET_0MV;
		l4r17_temp = tc_phy_read_l_reg(port_num, 4, 17);
		tc_phy_write_l_reg(port_num, 4, 17,
				   (l4r17_temp |
				    (tx_offset_temp << tx_offset_reg_shift)));
		pr_info("[%d] %s, ANACAL_SATURATION\n", port_num, __func__);
	} else {
		fe_cal_tx_offset_flag = 1;
	}
	tx_offset_final = (tc_phy_read_l_reg(port_num, 4, 17) & 0x3f00) >> 8;
	pr_info("[%d] - tx_offset_final = 0x%x\n", port_num, tx_offset_final);

	clear_ckinv_ana_txvos();
	tc_phy_write_l_reg(port_num, 3, 25, 0x0000);
	tc_phy_write_l_reg(FE_CAL_COMMON, 3, 25, 0x0000);
	tc_phy_write_g_reg(port_num, 1, 26, 0);
}

static void fe_cal_tx_offset_mdix(u8 port_num, u32 delay)
{				/* for MT7622 */
	u8 all_ana_cal_status;
	int ad_cal_comp_out_init;
	u16 l3r25_temp, l2r20_temp, l4r26_temp;
	u16 g4r21_temp, l0r30_temp, l0r26_temp;
	int calibration_polarity, tx_offset_temp;
	int cal_temp = 0;
	u8 tx_offset_reg_shift;
	u8 cnt = 0, phyaddr;
	u16 tx_offset_final_mdix;

	phyaddr = port_num + ephy_addr_base;
/*Set device in 100M mode*/
	tc_phy_write_l_reg(port_num, 0, 0, 0x2100);

	/*// g4r21[11]:Hw bypass tx offset cal, Fw cal*/
	g4r21_temp = tc_phy_read_g_reg(port_num, 4, 21);
	tc_phy_write_g_reg(port_num, 4, 21, (g4r21_temp | 0x0800));

	/*l0r30[9], [7], [6], [1]*/
	l0r30_temp = tc_phy_read_l_reg(port_num, 0, 30);
	tc_phy_write_l_reg(port_num, 0, 30, (l0r30_temp | 0x02c0));

	tx_offset_temp = 0x20;
	tx_offset_reg_shift = 8;
	tc_phy_write_g_reg(port_num, 1, 26, (0x8000 | DAC_IN_0V));

	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, 0x3000);

	/*RG_TXG_CALEN =1 by port number*/
	l3r25_temp = tc_phy_read_l_reg(port_num, 3, 25);
	tc_phy_write_l_reg(port_num, 3, 25, (l3r25_temp | 0x400));

	/*decide which port calibration RG_ZCALEN by port_num*/
	l3r25_temp = tc_phy_read_l_reg(port_num, 3, 25);
	tc_phy_write_l_reg(port_num, 3, 25, (l3r25_temp | 0x1000));

	/*DA_PGA_MDIX_STASTUS_P0=0(L0R26[15:14] = 0x10) & RG_RX2TX_EN_P0=1(L2R20[10] =1),*/
	l0r26_temp = tc_phy_read_l_reg(port_num, 0, 26);
	l0r26_temp = l0r26_temp & (~0xc000);
	tc_phy_write_l_reg(port_num, 0, 26, 0x9203); /* Kant */
	l2r20_temp = tc_phy_read_l_reg(port_num, 2, 20);
	l2r20_temp = l2r20_temp | 0x400;
	tc_phy_write_l_reg(port_num, 2, 20, l2r20_temp);

	l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
	tc_phy_write_l_reg(port_num, 4, 26, l4r26_temp & (~0x3f00));
	tc_phy_write_l_reg(port_num, 4, 26,
			   (l4r26_temp & ~0x3f00) | (cal_temp << tx_offset_reg_shift));

	all_ana_cal_status = all_fe_ana_cal_wait(delay, port_num);
	if (all_ana_cal_status == 0) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info(" FE Tx offset mdix AnaCal ERROR! (init)  \r\n");
	}

	tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);

	ad_cal_comp_out_init = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1;

	if (ad_cal_comp_out_init == 1)
		calibration_polarity = -1;
	else
		calibration_polarity = 1;

	cnt = 0;
	tx_offset_temp += calibration_polarity;
	while ((all_ana_cal_status < ANACAL_ERROR) && (cnt < 254)) {
		cnt++;
		cal_temp = tx_offset_temp;
		l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
		tc_phy_write_l_reg(port_num, 4, 26,
				   (l4r26_temp & ~0x3f00) | (cal_temp << tx_offset_reg_shift));

		tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
		tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
		all_ana_cal_status = all_fe_ana_cal_wait(delay, port_num);

		if (all_ana_cal_status == 0) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info(" FE Tx offset mdix AnaCal ERROR! (%d)  \r\n", cnt);
		} else if ((tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1) !=
			   ad_cal_comp_out_init) {
			all_ana_cal_status = ANACAL_FINISH;
			tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
			tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
			ad_cal_comp_out_init =
			    tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1;
		} else {
			l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);

			if ((tx_offset_temp == 0x3f) || (tx_offset_temp == 0x00)) {
				all_ana_cal_status = ANACAL_SATURATION;
				pr_info("tx offset ANACAL_SATURATION\n");
			} else {
				tx_offset_temp += calibration_polarity;
			}
		}
	}

	if ((all_ana_cal_status == ANACAL_ERROR) ||
	    (all_ana_cal_status == ANACAL_SATURATION)) {
		tx_offset_temp = TX_AMP_OFFSET_0MV;
		l4r26_temp = tc_phy_read_l_reg(port_num, 4, 26);
		tc_phy_write_l_reg(port_num, 4, 26,
				   (l4r26_temp & (~0x3f00)) | (cal_temp << tx_offset_reg_shift));
		pr_info("[%d] %s, ANACAL_SATURATION\n", port_num, __func__);
	} else {
		fe_cal_tx_offset_flag_mdix = 1;
	}
	tx_offset_final_mdix = (tc_phy_read_l_reg(port_num, 4, 26) & 0x3f00) >> 8;
	pr_info("[%d] - tx_offset_final_mdix = 0x%x\n", port_num, tx_offset_final_mdix);

	clear_ckinv_ana_txvos();
	tc_phy_write_l_reg(port_num, 3, 25, 0x0000);
	tc_phy_write_l_reg(FE_CAL_COMMON, 3, 25, 0x0000);
	tc_phy_write_g_reg(port_num, 1, 26, 0);
}

static void set_r50_leopard(u8 port_num, u32 r50_cal_result)
{
	int rg_zcal_ctrl_tx, rg_zcal_ctrl_rx;
	u16 l4r22_temp;

	rg_zcal_ctrl_rx = 0;
	rg_zcal_ctrl_tx = 0;
	pr_info("r50_cal_result  = 0x%x\n", r50_cal_result);
	if (port_num == 0) {
		rg_zcal_ctrl_tx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result)];
		rg_zcal_ctrl_rx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result)];
	}
	if (port_num == 1) {
		rg_zcal_ctrl_tx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result)] + 4;
		rg_zcal_ctrl_rx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result)] + 4;
	}
	if (port_num == 2) {
		rg_zcal_ctrl_tx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result)] + 4;
		rg_zcal_ctrl_rx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result)] + 6;
	}
	if (port_num == 3) {
		rg_zcal_ctrl_tx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result)] + 5;
		rg_zcal_ctrl_rx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result)] + 6;
	}
	if (port_num == 4) {
		rg_zcal_ctrl_tx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result)] + 4;
		rg_zcal_ctrl_rx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result)] + 4;
	}
	if (rg_zcal_ctrl_tx > 0x7f)
		rg_zcal_ctrl_tx = 0x7f;
	if (rg_zcal_ctrl_rx > 0x7f)
		rg_zcal_ctrl_rx = 0x7f;
/*R50OHM_RSEL_TX= LP4R22[14:8]*/
	tc_phy_write_l_reg(port_num, 4, 22, ((rg_zcal_ctrl_tx << 8)));
	l4r22_temp = tc_phy_read_l_reg(port_num, 4, 22);
/*R50OHM_RSEL_RX= LP4R22[6:0]*/
	tc_phy_write_l_reg(port_num, 4, 22,
			   (l4r22_temp | (rg_zcal_ctrl_rx << 0)));
	fe_cal_r50_flag = 1;
	pr_info("[%d] - r50 final result l4r22[%d] = %x\n", port_num,
		port_num, tc_phy_read_l_reg(port_num, 4, 22));
}
#if 0
static void set_r50_mt7622(u8 port_num, u32 r50_cal_result)
{
	int rg_zcal_ctrl_tx, rg_zcal_ctrl_rx;
	u16 l4r22_temp;

	rg_zcal_ctrl_rx = 0;
	rg_zcal_ctrl_tx = 0;
	pr_info("r50_cal_result  = 0x%x\n", r50_cal_result);

	if (port_num == 0) {
		rg_zcal_ctrl_tx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result - 5)];
		rg_zcal_ctrl_rx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result - 5)];
	}
	if (port_num == 1) {
		rg_zcal_ctrl_tx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result - 3)];
		rg_zcal_ctrl_rx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result - 3)];
	}
	if (port_num == 2) {
		rg_zcal_ctrl_tx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result - 4)];
		rg_zcal_ctrl_rx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result - 5)];
	}
	if (port_num == 3) {
		rg_zcal_ctrl_tx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result - 4)];
		rg_zcal_ctrl_rx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result - 3)];
	}
	if (port_num == 4) {
		rg_zcal_ctrl_tx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result - 4)];
		rg_zcal_ctrl_rx = ZCAL_TO_R50OHM_TBL_100[(r50_cal_result - 5)];
	}
/*R50OHM_RSEL_TX= LP4R22[14:8]*/
	tc_phy_write_l_reg(port_num, 4, 22, ((rg_zcal_ctrl_tx << 8)));
	l4r22_temp = tc_phy_read_l_reg(port_num, 4, 22);
/*R50OHM_RSEL_RX= LP4R22[6:0]*/
	tc_phy_write_l_reg(port_num, 4, 22,
			   (l4r22_temp | (rg_zcal_ctrl_rx << 0)));
	fe_cal_r50_flag = 1;
	pr_info("[%d] - r50 final result l4r22[%d] = %x\n", port_num,
		port_num, tc_phy_read_l_reg(port_num, 4, 22));
}
#endif
static void fe_ge_r50_common(u8 port_num)
{
	u16 l3r25_temp, g7r24_tmp, l4r23_temp;
	u8 phyaddr;

	phyaddr = port_num;
	tc_phy_write_l_reg(port_num, 0, 0, 0x2100);
	/*g2r25[7:5]:0x110, BG voltage output*/
	tc_phy_write_g_reg(FE_CAL_COMMON, 2, 25, 0xf0c0);

	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, 0x0000);
	/*g7r24[13]:0x01, RG_ANA_CALEN_P0=1*/
	g7r24_tmp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, (g7r24_tmp | 0x2000));
	/*g7r24[14]:0x01, RG_CAL_CKINV_P0=1*/
	g7r24_tmp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, (g7r24_tmp | 0x4000));

	/*g7r24[12]:0x01, DA_TXVOS_CALEN_P0=0*/
	g7r24_tmp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, (g7r24_tmp & (~0x1000)));

	/*DA_R50OHM_CAL_EN l4r23[0] = 0*/
	l4r23_temp = tc_phy_read_l_reg(port_num, 4, 23);
	l4r23_temp = l4r23_temp & ~(0x01);
	tc_phy_write_l_reg(port_num, 4, 23, l4r23_temp);

	/*RG_REXT_CALEN l2r25[13] = 0*/
	l3r25_temp = tc_phy_read_l_reg(FE_CAL_COMMON, 3, 25);
	tc_phy_write_l_reg(FE_CAL_COMMON, 3, 25, (l3r25_temp & (~0x2000)));
}

static void fe_cal_r50(u8 port_num, u32 delay)
{
	int rg_zcal_ctrl, all_ana_cal_status, rg_zcal_ctrl_tx, rg_zcal_ctrl_rx;
	int ad_cal_comp_out_init;
	u16 l3r25_temp, l0r4, g7r24_tmp, l4r23_temp;
	int calibration_polarity;
	u8 cnt = 0, phyaddr;
	//struct END_DEVICE *ei_local = netdev_priv(dev_raether);

	phyaddr = port_num + ephy_addr_base;
	tc_phy_write_l_reg(port_num, 0, 0, 0x2100);
	/*g2r25[7:5]:0x110, BG voltage output*/
	tc_phy_write_g_reg(FE_CAL_COMMON, 2, 25, 0xf0c0);

	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, 0x0000);
	/*g7r24[13]:0x01, RG_ANA_CALEN_P0=1*/
	g7r24_tmp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, (g7r24_tmp | 0x2000));
	/*g7r24[14]:0x01, RG_CAL_CKINV_P0=1*/
	g7r24_tmp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, (g7r24_tmp | 0x4000));

	/*g7r24[12]:0x01, DA_TXVOS_CALEN_P0=0*/
	g7r24_tmp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, (g7r24_tmp & (~0x1000)));

	/* pr_info("g7r24 = %x\n", g7r24_tmp); */

	/*DA_R50OHM_CAL_EN l4r23[0] = 1*/
	l4r23_temp = tc_phy_read_l_reg(port_num, 4, 23);
	tc_phy_write_l_reg(port_num, 4, 23, (l4r23_temp | (0x01)));

	/*RG_REXT_CALEN l2r25[13] = 0*/
	l3r25_temp = tc_phy_read_l_reg(FE_CAL_COMMON, 3, 25);
	tc_phy_write_l_reg(FE_CAL_COMMON, 3, 25, (l3r25_temp & (~0x2000)));

	/*decide which port calibration RG_ZCALEN by port_num*/
	l3r25_temp = tc_phy_read_l_reg(port_num, 3, 25);
	tc_phy_write_l_reg(port_num, 3, 25, (l3r25_temp | 0x1000));

	rg_zcal_ctrl = 0x20;	/* start with 0 dB */
	g7r24_tmp = (tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & (~0xfc0));
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, g7r24_tmp | ((rg_zcal_ctrl & 0x3f) << 6));

	/*wait AD_CAL_COMP_OUT = 1*/
	all_ana_cal_status = all_fe_ana_cal_wait(delay, port_num);
	if (all_ana_cal_status == 0) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info(" FE R50 AnaCal ERROR! (init)   \r\n");
	}

	ad_cal_comp_out_init = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1;

	if (ad_cal_comp_out_init == 1)
		calibration_polarity = -1;
	else
		calibration_polarity = 1;

	cnt = 0;
	while ((all_ana_cal_status < ANACAL_ERROR) && (cnt < 254)) {
		cnt++;

		rg_zcal_ctrl += calibration_polarity;
		g7r24_tmp = (tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & (~0xfc0));
		tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, g7r24_tmp | ((rg_zcal_ctrl & 0x3f) << 6));
		all_ana_cal_status = all_fe_ana_cal_wait(delay, port_num);

		if (all_ana_cal_status == 0) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info(" FE R50 AnaCal ERROR! (%d)  \r\n", cnt);
		} else if ((tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1) !=
			ad_cal_comp_out_init) {
			all_ana_cal_status = ANACAL_FINISH;
		} else {
			if ((rg_zcal_ctrl == 0x3F) || (rg_zcal_ctrl == 0x00)) {
				all_ana_cal_status = ANACAL_SATURATION;
				pr_info(" FE R50 AnaCal Saturation! (%d)  \r\n",
					cnt);
			} else {
				l0r4 = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
				l0r4 = l0r4 & 0x1;
			}
		}
	}
	if (port_num == 0)
		r50_p0_cal_result = rg_zcal_ctrl;

	if ((all_ana_cal_status == ANACAL_ERROR) ||
	    (all_ana_cal_status == ANACAL_SATURATION)) {
		rg_zcal_ctrl = 0x20;	/* 0 dB */
		rg_zcal_ctrl_tx = 0x7f;
		rg_zcal_ctrl_rx = 0x7f;
		pr_info("[%d] %s, ANACAL_SATURATION\n", port_num, __func__);
	} else {
		fe_cal_r50_flag = 1;
	}
	//if (ei_local->chip_name == MT7622_FE)
		//set_r50_mt7622(port_num, rg_zcal_ctrl);
    //	else if (ei_local->chip_name == LEOPARD_FE)
		set_r50_leopard(port_num, rg_zcal_ctrl);

	clear_ckinv_ana_txvos();
	tc_phy_write_l_reg(port_num, 3, 25, 0x0000);
	tc_phy_write_l_reg(FE_CAL_COMMON, 3, 25, 0x0000);
}

static void fe_cal_vbg(u8 port_num, u32 delay)
{
	int rg_zcal_ctrl, all_ana_cal_status;
	int ad_cal_comp_out_init, port_no;
	u16 l3r25_temp, l0r4, g7r24_tmp, l3r26_temp;
	int calibration_polarity;
	//struct END_DEVICE *ei_local = netdev_priv(dev_raether);
	u16 g2r22_temp, rg_bg_rasel;
	u8 cnt = 0, phyaddr;

	ephy_addr_base = 0;
	phyaddr = port_num + ephy_addr_base;

	tc_phy_write_g_reg(FE_CAL_COMMON, 2, 25, 0x30c0);
	tc_phy_write_g_reg(FE_CAL_COMMON, 0, 25, 0x0030);
	g7r24_tmp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, (g7r24_tmp | 0x2000));
	g7r24_tmp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, (g7r24_tmp | 0x4000));

	g7r24_tmp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, (g7r24_tmp & (~0x1000)));

	l3r25_temp = tc_phy_read_l_reg(FE_CAL_COMMON, 3, 25);
	tc_phy_write_l_reg(FE_CAL_COMMON, 3, 25, (l3r25_temp | 0x2000));

	for (port_no = port_num; port_no < 5; port_no++) {
		l3r25_temp = tc_phy_read_l_reg(port_no, 3, 25);
		tc_phy_write_l_reg(port_no, 3, 25, (l3r25_temp & (~0x1000)));
	}
	rg_zcal_ctrl = 0x0;	/* start with 0 dB */

	g7r24_tmp = (tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & (~0xfc0));
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, g7r24_tmp | ((rg_zcal_ctrl & 0x3f) << 6));

	all_ana_cal_status = all_fe_ana_cal_wait(delay, port_num);
	if (all_ana_cal_status == 0) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info(" fe_cal_vbg ERROR! (init)   \r\n");
	}
	ad_cal_comp_out_init = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1;

	if (ad_cal_comp_out_init == 1)
		calibration_polarity = -1;
	else
		calibration_polarity = 1;

	cnt = 0;
	while ((all_ana_cal_status < ANACAL_ERROR) && (cnt < 254)) {
		cnt++;
		rg_zcal_ctrl += calibration_polarity;
		g7r24_tmp = (tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & (~0xfc0));
		tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, g7r24_tmp | ((rg_zcal_ctrl & 0x3f) << 6));
		all_ana_cal_status = all_fe_ana_cal_wait(delay, port_num);

		if (all_ana_cal_status == 0) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info("VBG ERROR(%d)status=%d\n", cnt, all_ana_cal_status);
		} else if ((tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1) !=
			ad_cal_comp_out_init) {
			all_ana_cal_status = ANACAL_FINISH;
		} else {
			if ((rg_zcal_ctrl == 0x3F) || (rg_zcal_ctrl == 0x00)) {
				all_ana_cal_status = ANACAL_SATURATION;
				pr_info(" VBG0 AnaCal Saturation! (%d)  \r\n",
					cnt);
			} else {
				l0r4 = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
				l0r4 = l0r4 & 0x1;
			}
		}
	}
	if ((all_ana_cal_status == ANACAL_ERROR) ||
	    (all_ana_cal_status == ANACAL_SATURATION)) {
		rg_zcal_ctrl = 0x20;	/* 0 dB */
	} else {
		fe_cal_vbg_flag = 1;
	}

	rg_zcal_ctrl = (tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & (0xfc0)) >> 6;
	iext_cal_result = rg_zcal_ctrl;
	pr_info("iext_cal_result = 0x%x\n", iext_cal_result);
	//if (ei_local->chip_name == LEOPARD_FE)
		rg_bg_rasel =  ZCAL_TO_REXT_TBL[rg_zcal_ctrl];

	l3r26_temp = tc_phy_read_l_reg(FE_CAL_COMMON, 3, 26);
	l3r26_temp = l3r26_temp & (~0xfc0);
	tc_phy_write_l_reg(FE_CAL_COMMON, 3, 26, l3r26_temp | ((rg_zcal_ctrl & 0x3f) << 6));

	g2r22_temp = tc_phy_read_g_reg(FE_CAL_COMMON, 2, 22);
	g2r22_temp = g2r22_temp & (~0xe00);/*[11:9]*/

	//if (ei_local->chip_name == LEOPARD_FE) {
		rg_bg_rasel = rg_bg_rasel & 0x7;
		tc_phy_write_g_reg(FE_CAL_COMMON, 2, 22,
				   g2r22_temp | (rg_bg_rasel << 9));
#if 0		
	} else if (ei_local->chip_name == MT7622_FE) {
		rg_zcal_ctrl = rg_zcal_ctrl & 0x38;
		tc_phy_write_g_reg(FE_CAL_COMMON, 2, 22,
				   g2r22_temp | (((rg_zcal_ctrl & 0x38) >> 3) << 9));
	}
#endif
	clear_ckinv_ana_txvos();

	tc_phy_write_l_reg(port_num, 3, 25, 0x0000);
	tc_phy_write_l_reg(FE_CAL_COMMON, 3, 25, 0x0000);
}



static void do_fe_phy_all_analog_cal(u8 port_num, bool fe_cal)
{
	u16 l0r26_temp, l0r30_temp, l3r25_tmp;
	u8 cnt = 0, phyaddr, i, iext_port;
	u32 iext_s, iext_e, r50_s, r50_e, txo_s, txo_e, txa_s, txa_e;
	int max_port = (fe_cal)? 4: 1;
	//struct END_DEVICE *ei_local = netdev_priv(dev_raether);

	ephy_addr_base = 0;
	phyaddr = port_num + ephy_addr_base;
	l0r26_temp = tc_phy_read_l_reg(port_num, 0, 26);
	tc_phy_write_l_reg(port_num, 0, 26, 0x5600);
	tc_phy_write_l_reg(port_num, 4, 21, 0x0000);
	tc_phy_write_l_reg(port_num, 0, 0, 0x2100);

	l0r30_temp = tc_phy_read_l_reg(port_num, 0, 30);

/*eye pic.*/
	tc_phy_write_g_reg(port_num, 5, 20, 0x0170);
	tc_phy_write_g_reg(port_num, 5, 23, 0x0220);
	tc_phy_write_g_reg(port_num, 5, 24, 0x0206);
	tc_phy_write_g_reg(port_num, 5, 26, 0x0370);
	tc_phy_write_g_reg(port_num, 5, 27, 0x02f2);
	tc_phy_write_g_reg(port_num, 5, 29, 0x001b);
	tc_phy_write_g_reg(port_num, 5, 30, 0x0002);
/*Yiron default setting*/
	for (i = port_num; i <= max_port; i++) {
		tc_phy_write_g_reg(i, 3, 23, 0x0);
		tc_phy_write_l_reg(i, 3, 23, 0x2004);
		tc_phy_write_l_reg(i, 2, 21, 0x8551);
		tc_phy_write_l_reg(i, 4, 17, 0x2000);
		tc_phy_write_g_reg(i, 7, 20, 0x7c62);
		tc_phy_write_l_reg(i, 4, 20, 0x4444);
		tc_phy_write_l_reg(i, 2, 22, 0x1011);
		tc_phy_write_l_reg(i, 4, 28, 0x1011);
		tc_phy_write_l_reg(i, 4, 19, 0x2222);
		tc_phy_write_l_reg(i, 4, 29, 0x2222);
		tc_phy_write_l_reg(i, 2, 28, 0x3444);
		tc_phy_write_l_reg(i, 2, 29, 0x04c6);
		tc_phy_write_l_reg(i, 4, 30, 0x0006);
		tc_phy_write_l_reg(i, 5, 16, 0x04c6);
	}
	//if (ei_local->chip_name == LEOPARD_FE) {
		tc_phy_write_l_reg(port_num, 0, 20, 0x0c0c);
		tc_phy_write_dev_reg(0, 0x1e, 0x017d, 0x0000);
		tc_phy_write_dev_reg(0, 0x1e, 0x017e, 0x0000);
		tc_phy_write_dev_reg(0, 0x1e, 0x017f, 0x0000);
		tc_phy_write_dev_reg(0, 0x1e, 0x0180, 0x0000);
		tc_phy_write_dev_reg(0, 0x1e, 0x0181, 0x0000);
		tc_phy_write_dev_reg(0, 0x1e, 0x0182, 0x0000);
		tc_phy_write_dev_reg(0, 0x1e, 0x0183, 0x0000);
		tc_phy_write_dev_reg(0, 0x1e, 0x0184, 0x0000);
		tc_phy_write_dev_reg(0, 0x1e, 0x00db, 0x0000);
		tc_phy_write_dev_reg(0, 0x1e, 0x00dc, 0x0000);
		tc_phy_write_dev_reg(0, 0x1e, 0x003e, 0x0000);
		tc_phy_write_dev_reg(0, 0x1e, 0x00dd, 0x0000);

		/*eye pic.*/
		tc_phy_write_g_reg(1, 5, 19, 0x0100);
		tc_phy_write_g_reg(1, 5, 20, 0x0161);
		tc_phy_write_g_reg(1, 5, 21, 0x00f0);
		tc_phy_write_g_reg(1, 5, 22, 0x0046);
		tc_phy_write_g_reg(1, 5, 23, 0x0210);
		tc_phy_write_g_reg(1, 5, 24, 0x0206);
		tc_phy_write_g_reg(1, 5, 25, 0x0238);
		tc_phy_write_g_reg(1, 5, 26, 0x0360);
		tc_phy_write_g_reg(1, 5, 27, 0x02f2);
		tc_phy_write_g_reg(1, 5, 28, 0x0240);
		tc_phy_write_g_reg(1, 5, 29, 0x0010);
		tc_phy_write_g_reg(1, 5, 30, 0x0002);
	//}
	//if (ei_local->chip_name == MT7622_FE)
	//	iext_port = 0;
	//else if (ei_local->chip_name == LEOPARD_FE)
		iext_port = 1;

	if (port_num == iext_port) {
			/*****VBG & IEXT Calibration*****/
		cnt = 0;
		while ((fe_cal_vbg_flag == 0) && (cnt < 0x03)) {
			iext_s = jiffies;
			fe_cal_vbg(port_num, 1);	/* allen_20160608 */
			iext_e = jiffies;
			if (show_time)
				pr_info("port[%d] fe_cal_vbg time = %u\n",
					port_num, (iext_e - iext_s) * 4);
			cnt++;
			if (fe_cal_vbg_flag == 0)
				pr_info(" FE-%d VBG wait! (%d)  \r\n", phyaddr, cnt);
		}
		fe_cal_vbg_flag = 0;
		/**** VBG & IEXT Calibration end ****/
	}

	if (!fe_cal)
		goto restore_default;

	/* *** R50 Cal start *************************************** */
	cnt = 0;
	while ((fe_cal_r50_flag == 0) && (cnt < 0x03)) {
		r50_s = jiffies;

		fe_cal_r50(port_num, 1);

		r50_e = jiffies;
		if (show_time)
			pr_info("port[%d] fe_r50 time = %u\n",
				port_num, (r50_e - r50_s) * 4);
		cnt++;
		if (fe_cal_r50_flag == 0)
			pr_info(" FE-%d R50 wait! (%d)  \r\n", phyaddr, cnt);
	}
	fe_cal_r50_flag = 0;
	cnt = 0;
	/* *** R50 Cal end *** */
	/* *** Tx offset Cal start ********************************* */

	cnt = 0;
	while ((fe_cal_tx_offset_flag == 0) && (cnt < 0x03)) {
		txo_s = jiffies;
		fe_cal_tx_offset(port_num, CALDLY);
		txo_e = jiffies;
		if (show_time)
			pr_info("port[%d] fe_cal_tx_offset time = %u\n",
				port_num, (txo_e - txo_s) * 4);
		cnt++;
	}
	fe_cal_tx_offset_flag = 0;
	cnt = 0;

	while ((fe_cal_tx_offset_flag_mdix == 0) && (cnt < 0x03)) {
		txo_s = jiffies;
		fe_cal_tx_offset_mdix(port_num, CALDLY);
		txo_e = jiffies;
		if (show_time)
			pr_info("port[%d] fe_cal_tx_offset_mdix time = %u\n",
				port_num, (txo_e - txo_s) * 4);
		cnt++;
	}
	fe_cal_tx_offset_flag_mdix = 0;
	cnt = 0;
	/* *** Tx offset Cal end *** */

	/* *** Tx Amp Cal start ************************************** */
	cnt = 0;
	while ((fe_cal_flag == 0) && (cnt < 0x3)) {
		txa_s = jiffies;
		fe_cal_tx_amp(port_num, CALDLY);	/* allen_20160608 */
		txa_e = jiffies;
		if (show_time)
			pr_info("port[%d] fe_cal_tx_amp time = %u\n",
				port_num, (txa_e - txa_s) * 4);
		cnt++;
	}
	fe_cal_flag = 0;
	cnt = 0;
	while ((fe_cal_flag_mdix == 0) && (cnt < 0x3)) {
		txa_s = jiffies;
		fe_cal_tx_amp_mdix(port_num, CALDLY);
		txa_e = jiffies;
		if (show_time)
			pr_info("port[%d] fe_cal_tx_amp_mdix time = %u\n",
				port_num, (txa_e - txa_s) * 4);
		cnt++;
	}
	fe_cal_flag_mdix = 0;
	cnt = 0;

restore_default:
	l3r25_tmp = tc_phy_read_l_reg(port_num, 3, 25);
	l3r25_tmp = l3r25_tmp & ~(0x1000);/*[12] RG_ZCALEN = 0*/
	tc_phy_write_l_reg(port_num, 3, 25, l3r25_tmp);
	tc_phy_write_g_reg(port_num, 1, 26, 0x0000);
	tc_phy_write_l_reg(port_num, 0, 26, l0r26_temp);
	tc_phy_write_l_reg(port_num, 0, 30, l0r30_temp);
	tc_phy_write_g_reg(port_num, 1, 26, 0x0000);
	tc_phy_write_l_reg(port_num, 0, 0, 0x3100);
	/*enable flow control*/
	tc_phy_write_g_reg(port_num, 0, 4, 0x5e1);
}

static u8 all_ge_ana_cal_wait(unsigned int delay, u8 port_num) /* for EN7512 */
{
	u8 all_ana_cal_status;
	u16 cnt, g7r24_temp;

	g7r24_temp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, g7r24_temp & (~0x10));
	g7r24_temp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, g7r24_temp | 0x10);

	cnt = 1000;
	do {
		udelay(delay);
		cnt--;
		all_ana_cal_status =
		    ((tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) >> 1) & 0x1);

	} while ((all_ana_cal_status == 0) && (cnt != 0));
	g7r24_temp = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, g7r24_temp & (~0x10));

	return all_ana_cal_status;
}

static void ge_cal_rext(u8 phyaddr, unsigned int delay)
{
	u8	rg_zcal_ctrl, all_ana_cal_status;
	u16	ad_cal_comp_out_init;
	u16	dev1e_e0_ana_cal_r5;
	int	calibration_polarity;
	u8	cnt = 0;
	u16	dev1e_17a_tmp, dev1e_e0_tmp;

	/* *** Iext/Rext Cal start ************ */
	all_ana_cal_status = ANACAL_INIT;
	/* analog calibration enable, Rext calibration enable */
	/* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a */
	/* 1e_dc[0]:rg_txvos_calen */
	/* 1e_e1[4]:rg_cal_refsel(0:1.2V) */
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00db, 0x1110);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dc, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00e1, 0x0000);

	rg_zcal_ctrl = 0x20;/* start with 0 dB */
	dev1e_e0_ana_cal_r5 = tc_phy_read_dev_reg(phyaddr, 0x1e, 0x00e0);
	/* 1e_e0[5:0]:rg_zcal_ctrl */
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00e0, (rg_zcal_ctrl));
	all_ana_cal_status = all_ge_ana_cal_wait(delay, phyaddr);/* delay 20 usec */
	if (all_ana_cal_status == 0) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info(" GE Rext AnaCal ERROR!   \r\n");
	}
	/* 1e_17a[8]:ad_cal_comp_out */
	ad_cal_comp_out_init = (tc_phy_read_dev_reg(phyaddr, 0x1e, 0x017a) >> 8) & 0x1;
	if (ad_cal_comp_out_init == 1)
		calibration_polarity = -1;
	else /* ad_cal_comp_out_init == 0 */
		calibration_polarity = 1;

	cnt = 0;
	while (all_ana_cal_status < ANACAL_ERROR) {
		cnt++;
		rg_zcal_ctrl += calibration_polarity;
		tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00e0, (rg_zcal_ctrl));
		all_ana_cal_status = all_ge_ana_cal_wait(delay, phyaddr); /* delay 20 usec */
		dev1e_17a_tmp = tc_phy_read_dev_reg(phyaddr, 0x1e, 0x017a);
		if (all_ana_cal_status == 0) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info("  GE Rext AnaCal ERROR!   \r\n");
		} else if (((dev1e_17a_tmp >> 8) & 0x1) != ad_cal_comp_out_init) {
			all_ana_cal_status = ANACAL_FINISH;
			pr_info("  GE Rext AnaCal Done! (%d)(0x%x)  \r\n", cnt, rg_zcal_ctrl);
		} else {
			dev1e_17a_tmp = tc_phy_read_dev_reg(phyaddr, 0x1e, 0x017a);
			dev1e_e0_tmp =	tc_phy_read_dev_reg(phyaddr, 0x1e, 0xe0);
			if ((rg_zcal_ctrl == 0x3F) || (rg_zcal_ctrl == 0x00)) {
				all_ana_cal_status = ANACAL_SATURATION;  /* need to FT(IC fail?) */
				pr_info(" GE Rext AnaCal Saturation!  \r\n");
				rg_zcal_ctrl = 0x20;  /* 0 dB */
			} else {
				pr_info(" GE Rxet cal (%d)(%d)(%d)(0x%x)  \r\n",
					cnt, ad_cal_comp_out_init,
				((dev1e_17a_tmp >> 8) & 0x1), dev1e_e0_tmp);
			}
		}
	}

	if (all_ana_cal_status == ANACAL_ERROR) {
		rg_zcal_ctrl = 0x20;  /* 0 dB */
		tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00e0, (dev1e_e0_ana_cal_r5 | rg_zcal_ctrl));
	} else {
		tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00e0, (dev1e_e0_ana_cal_r5 | rg_zcal_ctrl));
		tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00e0, ((rg_zcal_ctrl << 8) | rg_zcal_ctrl));
		/* ****  1f_115[2:0] = rg_zcal_ctrl[5:3]  // Mog review */
		tc_phy_write_dev_reg(phyaddr, 0x1f, 0x0115, ((rg_zcal_ctrl & 0x3f) >> 3));
		pr_info("  GE Rext AnaCal Done! (%d)(0x%x)  \r\n", cnt, rg_zcal_ctrl);
		ge_cal_flag = 1;
	}
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00db, 0x0000);
	/* *** Iext/Rext Cal end *** */
}

static void ge_cal_r50(u8 phyaddr, unsigned int delay)
{
	u8	rg_zcal_ctrl, all_ana_cal_status, i;
	u16	ad_cal_comp_out_init;
	u16	dev1e_e0_ana_cal_r5;
	int	calibration_polarity;
	u16	cal_pair, val_tmp, g7r24_tmp;
	u16	dev1e_174_tmp, dev1e_175_tmp, l3r25_temp;
	u8	rg_zcal_ctrl_filter, cnt = 0;

	/* *** R50 Cal start***************** */
	fe_ge_r50_common(phyaddr);
	/* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a */
	/* 1e_dc[0]:rg_txvos_calen */
	/*disable RG_ZCALEN*/
	/*decide which port calibration RG_ZCALEN by port_num*/
	for (i = 1; i <= 4; i++) {
		l3r25_temp = tc_phy_read_l_reg(i, 3, 25);
		l3r25_temp = l3r25_temp & ~(0x1000);
		tc_phy_write_l_reg(i, 3, 25, l3r25_temp);
	}
	for (cal_pair = ANACAL_PAIR_A; cal_pair <= ANACAL_PAIR_D; cal_pair++) {
		rg_zcal_ctrl = 0x20;/* start with 0 dB */
		dev1e_e0_ana_cal_r5 = (tc_phy_read_dev_reg(phyaddr, 0x1e, 0x00e0) & (~0x003f));
		/* 1e_e0[5:0]:rg_zcal_ctrl */
		if (cal_pair == ANACAL_PAIR_A) {
	/* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a */
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dd, 0x1000);
		} else if (cal_pair == ANACAL_PAIR_B) {
	/* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a */
	/* 1e_dc[12]:rg_zcalen_b */
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dd, 0x0100);
		} else if (cal_pair == ANACAL_PAIR_C) {
	/* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a */
	/* 1e_dc[8]:rg_zcalen_c */
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dd, 0x0010);

		} else {/* if(cal_pair == ANACAL_PAIR_D) */

	/* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a */
	/* 1e_dc[4]:rg_zcalen_d */
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dd, 0x0001);
		}
		rg_zcal_ctrl = 0x20;	/* start with 0 dB */
		g7r24_tmp = (tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & (~0xfc0));
		tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, g7r24_tmp | ((rg_zcal_ctrl & 0x3f) << 6));

		/*wait AD_CAL_COMP_OUT = 1*/
		all_ana_cal_status = all_ge_ana_cal_wait(delay, phyaddr);
		if (all_ana_cal_status == 0) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info(" GE R50 AnaCal ERROR! (init)   \r\n");
		}
		ad_cal_comp_out_init = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1;
		if (ad_cal_comp_out_init == 1)
			calibration_polarity = -1;
		else
			calibration_polarity = 1;

		cnt = 0;
		while ((all_ana_cal_status < ANACAL_ERROR) && (cnt < 254)) {
			cnt++;

			rg_zcal_ctrl += calibration_polarity;
			g7r24_tmp = (tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & (~0xfc0));
			tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24,
					   g7r24_tmp | ((rg_zcal_ctrl & 0x3f) << 6));
			all_ana_cal_status = all_ge_ana_cal_wait(delay, phyaddr);

			if (all_ana_cal_status == 0) {
				all_ana_cal_status = ANACAL_ERROR;
				pr_info(" GE R50 AnaCal ERROR! (%d)  \r\n", cnt);
			} else if ((tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1) !=
				ad_cal_comp_out_init) {
				all_ana_cal_status = ANACAL_FINISH;
			} else {
				if ((rg_zcal_ctrl == 0x3F) || (rg_zcal_ctrl == 0x00)) {
					all_ana_cal_status = ANACAL_SATURATION;
					pr_info(" GE R50 Cal Sat! rg_zcal_ctrl = 0x%x(%d)\n",
						cnt, rg_zcal_ctrl);
				}
			}
		}

		if ((all_ana_cal_status == ANACAL_ERROR) ||
		    (all_ana_cal_status == ANACAL_SATURATION)) {
			rg_zcal_ctrl = 0x20;  /* 0 dB */
			rg_zcal_ctrl_filter = 8; /*default value*/
		} else {
			/*DA_TX_R50*/
			rg_zcal_ctrl_filter = rg_zcal_ctrl;
			rg_zcal_ctrl = ZCAL_TO_R50ohm_GE_TBL[rg_zcal_ctrl];
			/*DA_TX_FILTER*/
			rg_zcal_ctrl_filter = ZCAL_TO_FILTER_TBL[rg_zcal_ctrl_filter];
			rg_zcal_ctrl_filter = rg_zcal_ctrl_filter & 0xf;
			rg_zcal_ctrl_filter = rg_zcal_ctrl_filter << 8 | rg_zcal_ctrl_filter;
		}
		if (all_ana_cal_status == ANACAL_FINISH) {
			if (cal_pair == ANACAL_PAIR_A) {
				dev1e_174_tmp = tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0174);
				dev1e_174_tmp = dev1e_174_tmp & ~(0xff00);
				if (rg_zcal_ctrl > 4) {
					val_tmp = (((rg_zcal_ctrl - 4) << 8) & 0xff00) |
						dev1e_174_tmp;
				} else {
					val_tmp = (((0) << 8) & 0xff00) | dev1e_174_tmp;
				}

				tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0174, val_tmp);
				tc_phy_write_dev_reg(phyaddr, 0x1e, 0x03a0, rg_zcal_ctrl_filter);

				pr_info("R50_PAIR_A : 1e_174 = 0x%x\n",
					tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0174));
				pr_info("R50_PAIR_A : 1e_3a0 = 0x%x\n",
					tc_phy_read_dev_reg(phyaddr, 0x1e, 0x03a0));

			} else if (cal_pair == ANACAL_PAIR_B) {
				dev1e_174_tmp = tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0174);
				dev1e_174_tmp = dev1e_174_tmp & (~0x007f);
				if (rg_zcal_ctrl > 2) {
					val_tmp = (((rg_zcal_ctrl - 2) << 0) & 0xff) |
						dev1e_174_tmp;
				} else {
					val_tmp = (((0) << 0) & 0xff) |
						dev1e_174_tmp;
				}
				tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0174, val_tmp);
				tc_phy_write_dev_reg(phyaddr, 0x1e, 0x03a1, rg_zcal_ctrl_filter);
				pr_info("R50_PAIR_B : 1e_174 = 0x%x\n",
					tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0174));
				pr_info("R50_PAIR_B : 1e_3a1 = 0x%x\n",
					tc_phy_read_dev_reg(phyaddr, 0x1e, 0x03a1));
			} else if (cal_pair == ANACAL_PAIR_C) {
				dev1e_175_tmp = tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0175);
				dev1e_175_tmp =  dev1e_175_tmp & (~0x7f00);
				if (rg_zcal_ctrl > 4) {
					val_tmp = dev1e_175_tmp |
						(((rg_zcal_ctrl - 4) << 8) & 0xff00);
				} else {
					val_tmp = dev1e_175_tmp | (((0) << 8) & 0xff00);
				}
				tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0175, val_tmp);
				tc_phy_write_dev_reg(phyaddr, 0x1e, 0x03a2, rg_zcal_ctrl_filter);
				pr_info("R50_PAIR_C : 1e_175 = 0x%x\n",
					tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0175));
				pr_info("R50_PAIR_C : 1e_3a2 = 0x%x\n",
					tc_phy_read_dev_reg(phyaddr, 0x1e, 0x03a2));

			} else {/* if(cal_pair == ANACAL_PAIR_D) */
				dev1e_175_tmp = tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0175);
				dev1e_175_tmp = dev1e_175_tmp & (~0x007f);
				if (rg_zcal_ctrl > 6) {
					val_tmp = dev1e_175_tmp |
						(((rg_zcal_ctrl - 6)  << 0) & 0xff);
				} else {
					val_tmp = dev1e_175_tmp |
						(((0)  << 0) & 0xff);
				}

				tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0175, val_tmp);
				tc_phy_write_dev_reg(phyaddr, 0x1e, 0x03a3, rg_zcal_ctrl_filter);
				pr_info("R50_PAIR_D : 1e_175 = 0x%x\n",
					tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0175));
				pr_info("R50_PAIR_D : 1e_3a3 = 0x%x\n",
					tc_phy_read_dev_reg(phyaddr, 0x1e, 0x03a3));
			}
		}
	}
	clear_ckinv_ana_txvos();
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00db, 0x0000);
	ge_cal_r50_flag = 1;
	/* *** R50 Cal end *** */
}

static void ge_cal_tx_amp(u8 phyaddr, unsigned int delay)
{
	u8	all_ana_cal_status;
	u16	ad_cal_comp_out_init;
	int	calibration_polarity;
	u16	cal_pair;
	u8	tx_amp_reg_shift;
	u16	reg_temp, val_tmp, l3r25_temp, val_tmp_100;
	u8	tx_amp_temp, tx_amp_reg, cnt = 0, tx_amp_reg_100;

	u16	tx_amp_temp_L, tx_amp_temp_M;
	u16	tx_amp_L_100, tx_amp_M_100;
	/* *** Tx Amp Cal start ***/
	tc_phy_write_l_reg(0, 0, 0, 0x0140);

	tc_phy_write_dev_reg(0, 0x1e, 0x3e, 0xf808);
	tc_phy_write_dev_reg(0, 0x1e, 0x145, 0x5010);
	tc_phy_write_dev_reg(0, 0x1e, 0x17d, 0x80f0);
	tc_phy_write_dev_reg(0, 0x1e, 0x17e, 0x80f0);
	tc_phy_write_dev_reg(0, 0x1e, 0x17f, 0x80f0);
	tc_phy_write_dev_reg(0, 0x1e, 0x180, 0x80f0);
	tc_phy_write_dev_reg(0, 0x1e, 0x181, 0x80f0);
	tc_phy_write_dev_reg(0, 0x1e, 0x182, 0x80f0);
	tc_phy_write_dev_reg(0, 0x1e, 0x183, 0x80f0);
	tc_phy_write_dev_reg(0, 0x1e, 0x184, 0x80f0);
	tc_phy_write_dev_reg(0, 0x1e, 0x00db, 0x1000);
	tc_phy_write_dev_reg(0, 0x1e, 0x00dc, 0x0001);
	tc_phy_write_dev_reg(0, 0x1f, 0x300, 0x4);
	tc_phy_write_dev_reg(0, 0x1f, 0x27a, 0x33);
	tc_phy_write_g_reg(1, 2, 25, 0xf020);
	tc_phy_write_dev_reg(0, 0x1f, 0x300, 0x14);
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, 0x7000);
	l3r25_temp = tc_phy_read_l_reg(FE_CAL_COMMON, 3, 25);
	l3r25_temp = l3r25_temp | 0x200;
	tc_phy_write_l_reg(FE_CAL_COMMON, 3, 25, l3r25_temp);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x11, 0xff00);
	tc_phy_write_dev_reg(phyaddr, 0x1f, 0x273, 0);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0xc9, 0xffff);
	tc_phy_write_g_reg(1, 2, 25, 0xb020);

	for (cal_pair = ANACAL_PAIR_A; cal_pair <= ANACAL_PAIR_D; cal_pair++) {
		tx_amp_temp = 0x20;	/* start with 0 dB */
		tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, 0x7000);
		if (cal_pair == ANACAL_PAIR_A) {
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dd, 0x1000);
			reg_temp = (tc_phy_read_dev_reg(phyaddr, 0x1e, 0x012) & (~0xfc00));
			tx_amp_reg_shift = 10;
			tx_amp_reg = 0x12;
			tx_amp_reg_100 = 0x16;
		} else if (cal_pair == ANACAL_PAIR_B) {
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dd, 0x0100);
			reg_temp = (tc_phy_read_dev_reg(phyaddr, 0x1e, 0x017) & (~0x3f00));
			tx_amp_reg_shift = 8;
			tx_amp_reg = 0x17;
			tx_amp_reg_100 = 0x18;
		} else if (cal_pair == ANACAL_PAIR_C) {
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dd, 0x0010);
			reg_temp = (tc_phy_read_dev_reg(phyaddr, 0x1e, 0x019) & (~0x3f00));
			tx_amp_reg_shift = 8;
			tx_amp_reg = 0x19;
			tx_amp_reg_100 = 0x20;
		} else {/* if(cal_pair == ANACAL_PAIR_D) */
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dd, 0x0001);
			reg_temp = (tc_phy_read_dev_reg(phyaddr, 0x1e, 0x021) & (~0x3f00));
			tx_amp_reg_shift = 8;
			tx_amp_reg = 0x21;
			tx_amp_reg_100 = 0x22;
		}
		/* 1e_12, 1e_17, 1e_19, 1e_21 */
		val_tmp = tx_amp_temp | (tx_amp_temp << tx_amp_reg_shift);
		tc_phy_write_dev_reg(phyaddr, 0x1e, tx_amp_reg, val_tmp);
		tc_phy_write_dev_reg(phyaddr, 0x1e, tx_amp_reg_100, val_tmp);
		all_ana_cal_status = all_ge_ana_cal_wait(delay, phyaddr);
		if (all_ana_cal_status == 0) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info(" GE Tx amp AnaCal ERROR!   \r\n");
		}
/* 1e_17a[8]:ad_cal_comp_out */
		ad_cal_comp_out_init = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1;
		if (ad_cal_comp_out_init == 1)
			calibration_polarity = -1;
		else
			calibration_polarity = 1;

		cnt = 0;
		while (all_ana_cal_status < ANACAL_ERROR) {
			cnt++;
			tx_amp_temp += calibration_polarity;

			val_tmp = (tx_amp_temp | (tx_amp_temp << tx_amp_reg_shift));
			tc_phy_write_dev_reg(phyaddr, 0x1e, tx_amp_reg, val_tmp);
			tc_phy_write_dev_reg(phyaddr, 0x1e, tx_amp_reg_100, val_tmp);
			all_ana_cal_status = all_ge_ana_cal_wait(delay, phyaddr);
			if (all_ana_cal_status == 0) {
				all_ana_cal_status = ANACAL_ERROR;
				pr_info(" GE Tx amp AnaCal ERROR!\n");
			} else if ((tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1) !=
				    ad_cal_comp_out_init) {
				all_ana_cal_status = ANACAL_FINISH;
			} else {
				if ((tx_amp_temp == 0x3f) || (tx_amp_temp == 0x00)) {
					all_ana_cal_status = ANACAL_SATURATION;
					pr_info(" GE Tx amp AnaCal Saturation!  \r\n");
				}
			}
		}
		if (all_ana_cal_status == ANACAL_ERROR) {
			pr_info("ANACAL_ERROR\n");
			tx_amp_temp = 0x20;
			val_tmp = (reg_temp | (tx_amp_temp << tx_amp_reg_shift));
			tc_phy_write_dev_reg(phyaddr, 0x1e, tx_amp_reg, val_tmp);
		}

		if (all_ana_cal_status == ANACAL_FINISH) {
			if (cal_pair == ANACAL_PAIR_A) {
				tx_amp_temp_M = tx_amp_temp + 9;
				tx_amp_temp_L = tx_amp_temp + 18;
			} else if (cal_pair == ANACAL_PAIR_B) {
				tx_amp_temp_M = tx_amp_temp + 8;
				tx_amp_temp_L = tx_amp_temp + 22;
			} else if (cal_pair == ANACAL_PAIR_C) {
				tx_amp_temp_M = tx_amp_temp + 9;
				tx_amp_temp_L = tx_amp_temp + 9;
			} else if (cal_pair == ANACAL_PAIR_D) {
				tx_amp_temp_M = tx_amp_temp + 9;
				tx_amp_temp_L = tx_amp_temp + 9;
			}
			if (tx_amp_temp_L >= 0x3f)
				tx_amp_temp_L = 0x3f;
			if (tx_amp_temp_M >= 0x3f)
				tx_amp_temp_M = 0x3f;
			val_tmp = ((tx_amp_temp_L) |
				((tx_amp_temp_M) << tx_amp_reg_shift));
			if (cal_pair == ANACAL_PAIR_A) {
				if (tx_amp_temp < 6)
					tx_amp_M_100 = 0;
				else
					tx_amp_M_100 = tx_amp_temp - 6;

				if ((tx_amp_temp + 9) >= 0x3f)
					tx_amp_L_100 = 0x3f;
				else
					tx_amp_L_100 = tx_amp_temp + 9;
				val_tmp_100 = ((tx_amp_L_100) |
					((tx_amp_M_100) << tx_amp_reg_shift));
			} else if (cal_pair == ANACAL_PAIR_B) {
				if (tx_amp_temp < 7)
					tx_amp_M_100 = 0;
				else
					tx_amp_M_100 = tx_amp_temp - 7;

				if ((tx_amp_temp + 8) >= 0x3f)
					tx_amp_L_100 = 0x3f;
				else
					tx_amp_L_100 = tx_amp_temp + 8;
				val_tmp_100 = ((tx_amp_L_100) |
					((tx_amp_M_100) << tx_amp_reg_shift));
			} else if (cal_pair == ANACAL_PAIR_C) {
				if ((tx_amp_temp + 9) >= 0x3f)
					tx_amp_L_100 = 0x3f;
				else
					tx_amp_L_100 = tx_amp_temp + 9;
				tx_amp_M_100 = tx_amp_L_100;
				val_tmp_100 = ((tx_amp_L_100) |
					((tx_amp_M_100) << tx_amp_reg_shift));
			} else if (cal_pair == ANACAL_PAIR_D) {
				if ((tx_amp_temp + 9) >= 0x3f)
					tx_amp_L_100 = 0x3f;
				else
					tx_amp_L_100 = tx_amp_temp + 9;

				tx_amp_M_100 = tx_amp_L_100;
				val_tmp_100 = ((tx_amp_L_100) |
					((tx_amp_M_100) << tx_amp_reg_shift));
			}

			tc_phy_write_dev_reg(phyaddr, 0x1e, tx_amp_reg, val_tmp);
			tc_phy_write_dev_reg(phyaddr, 0x1e, tx_amp_reg_100, val_tmp_100);

			if (cal_pair == ANACAL_PAIR_A) {
				pr_info("TX_AMP_PAIR_A : 1e_%x = 0x%x\n",
					tx_amp_reg,
					tc_phy_read_dev_reg(phyaddr, 0x1e, tx_amp_reg));
				pr_info("TX_AMP_PAIR_A : 1e_%x = 0x%x\n",
					tx_amp_reg_100,
					tc_phy_read_dev_reg(phyaddr, 0x1e, tx_amp_reg_100));
			} else if (cal_pair == ANACAL_PAIR_B) {
				pr_info("TX_AMP_PAIR_B : 1e_%x = 0x%x\n",
					tx_amp_reg,
					tc_phy_read_dev_reg(phyaddr, 0x1e, tx_amp_reg));
				pr_info("TX_AMP_PAIR_B : 1e_%x = 0x%x\n",
					tx_amp_reg_100,
					tc_phy_read_dev_reg(phyaddr, 0x1e, tx_amp_reg_100));
			} else if (cal_pair == ANACAL_PAIR_C) {
				pr_info("TX_AMP_PAIR_C : 1e_%x = 0x%x\n",
					tx_amp_reg,
					tc_phy_read_dev_reg(phyaddr, 0x1e, tx_amp_reg));
				pr_info("TX_AMP_PAIR_C : 1e_%x = 0x%x\n",
					tx_amp_reg_100,
					tc_phy_read_dev_reg(phyaddr, 0x1e, tx_amp_reg_100));

			} else {/* if(cal_pair == ANACAL_PAIR_D) */
				pr_info("TX_AMP_PAIR_D : 1e_%x = 0x%x\n",
					tx_amp_reg,
					tc_phy_read_dev_reg(phyaddr, 0x1e, tx_amp_reg));
				pr_info("TX_AMP_PAIR_D : 1e_%x = 0x%x\n",
					tx_amp_reg_100,
					tc_phy_read_dev_reg(phyaddr, 0x1e, tx_amp_reg_100));
			}
		}
	}

	ge_cal_flag = 1;
	pr_info("GE_TX_AMP END\n");
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x017d, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x017e, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x017f, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0180, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0181, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0182, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0183, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0184, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1f, 0x273, 0x2000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0xc9, 0x0fff);
	tc_phy_write_g_reg(1, 2, 25, 0xb020);
	tc_phy_write_dev_reg(0, 0x1e, 0x145, 0x1000);

/* disable analog calibration circuit */
/* disable Tx offset calibration circuit */
/* disable Tx VLD force mode */
/* disable Tx offset/amplitude calibration circuit */

	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00db, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dc, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x003e, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dd, 0x0000);
	/* *** Tx Amp Cal end *** */
}

static void ge_cal_tx_offset(u8 phyaddr, unsigned int delay)
{
	u8	all_ana_cal_status;
	u16	ad_cal_comp_out_init;
	int	calibration_polarity, tx_offset_temp;
	u16	cal_pair, cal_temp;
	u8	tx_offset_reg_shift;
	u16	tx_offset_reg, reg_temp, val_tmp;
	u8	cnt = 0;

	tc_phy_write_l_reg(0, 0, 0, 0x2100);

	/* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a */
	/* 1e_dc[0]:rg_txvos_calen */
	/* 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal */
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00db, 0x0100);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dc, 0x0001);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0096, 0x8000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x003e, 0xf808);/* 1e_3e */
	tc_phy_write_g_reg(FE_CAL_COMMON, 7, 24, 0x3000);

	for (cal_pair = ANACAL_PAIR_A; cal_pair <= ANACAL_PAIR_D; cal_pair++) {
		tx_offset_temp = 0x20;

		if (cal_pair == ANACAL_PAIR_A) {
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x145, 0x5010);
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dd, 0x1000);
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x017d, (0x8000 | DAC_IN_0V));
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0181, (0x8000 | DAC_IN_0V));
			reg_temp = (tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0172) & (~0x3f00));
			tx_offset_reg_shift = 8;/* 1e_172[13:8] */
			tx_offset_reg = 0x0172;

		} else if (cal_pair == ANACAL_PAIR_B) {
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x145, 0x5018);
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dd, 0x0100);
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x017e, (0x8000 | DAC_IN_0V));
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0182, (0x8000 | DAC_IN_0V));
			reg_temp = (tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0172) & (~0x003f));
			tx_offset_reg_shift = 0;
			tx_offset_reg = 0x0172;/* 1e_172[5:0] */
		} else if (cal_pair == ANACAL_PAIR_C) {
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dd, 0x0010);
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x017f, (0x8000 | DAC_IN_0V));
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0183, (0x8000 | DAC_IN_0V));
			reg_temp = (tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0173) & (~0x3f00));
			tx_offset_reg_shift = 8;
			tx_offset_reg = 0x0173;/* 1e_173[13:8] */
		} else {/* if(cal_pair == ANACAL_PAIR_D) */
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dd, 0x0001);
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0180, (0x8000 | DAC_IN_0V));
			tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0184, (0x8000 | DAC_IN_0V));
			reg_temp = (tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0173) & (~0x003f));
			tx_offset_reg_shift = 0;
			tx_offset_reg = 0x0173;/* 1e_173[5:0] */
		}
		/* 1e_172, 1e_173 */
		val_tmp =  (reg_temp | (tx_offset_temp << tx_offset_reg_shift));
		tc_phy_write_dev_reg(phyaddr, 0x1e, tx_offset_reg, val_tmp);

		all_ana_cal_status = all_ge_ana_cal_wait(delay, phyaddr); /* delay 20 usec */
		if (all_ana_cal_status == 0) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info(" GE Tx offset AnaCal ERROR!   \r\n");
		}
		ad_cal_comp_out_init = tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1;
		if (ad_cal_comp_out_init == 1)
			calibration_polarity = -1;
		else
			calibration_polarity = 1;

		cnt = 0;
		tx_offset_temp += calibration_polarity;
		while (all_ana_cal_status < ANACAL_ERROR) {
			cnt++;
			cal_temp = tx_offset_temp;
			val_tmp = (reg_temp | (cal_temp << tx_offset_reg_shift));
			tc_phy_write_dev_reg(phyaddr, 0x1e, tx_offset_reg, val_tmp);

			all_ana_cal_status = all_ge_ana_cal_wait(delay, phyaddr);
			if (all_ana_cal_status == 0) {
				all_ana_cal_status = ANACAL_ERROR;
				pr_info(" GE Tx offset AnaCal ERROR!   \r\n");
			} else if ((tc_phy_read_g_reg(FE_CAL_COMMON, 7, 24) & 0x1) !=
				    ad_cal_comp_out_init) {
				all_ana_cal_status = ANACAL_FINISH;
			} else {
				if ((tx_offset_temp == 0x3f) || (tx_offset_temp == 0x00)) {
					all_ana_cal_status = ANACAL_SATURATION;
					pr_info("GE tx offset ANACAL_SATURATION\n");
					/* tx_amp_temp += calibration_polarity; */
				} else {
					tx_offset_temp += calibration_polarity;
				}
			}
		}
		if (all_ana_cal_status == ANACAL_ERROR) {
			tx_offset_temp = 0x20;
			val_tmp = (reg_temp | (tx_offset_temp << tx_offset_reg_shift));
			tc_phy_write_dev_reg(phyaddr, 0x1e, tx_offset_reg, val_tmp);
		}

		if (all_ana_cal_status == ANACAL_FINISH) {
			if (cal_pair == ANACAL_PAIR_A) {
				pr_info("TX_OFFSET_PAIR_A : 1e_%x = 0x%x\n",
					tx_offset_reg,
				tc_phy_read_dev_reg(phyaddr, 0x1e, tx_offset_reg));
			} else if (cal_pair == ANACAL_PAIR_B) {
				pr_info("TX_OFFSET_PAIR_B : 1e_%x = 0x%x\n",
					tx_offset_reg,
				tc_phy_read_dev_reg(phyaddr, 0x1e, tx_offset_reg));
			} else if (cal_pair == ANACAL_PAIR_C) {
				pr_info("TX_OFFSET_PAIR_C : 1e_%x = 0x%x\n",
					tx_offset_reg,
				tc_phy_read_dev_reg(phyaddr, 0x1e, tx_offset_reg));

			} else {/* if(cal_pair == ANACAL_PAIR_D) */
				pr_info("TX_OFFSET_PAIR_D : 1e_%x = 0x%x\n",
					tx_offset_reg,
				tc_phy_read_dev_reg(phyaddr, 0x1e, tx_offset_reg));
			}
		}
	}
	ge_cal_tx_offset_flag = 1;
	clear_ckinv_ana_txvos();
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x017d, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x017e, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x017f, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0180, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0181, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0182, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0183, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0184, 0x0000);
/* disable analog calibration circuit */
/* disable Tx offset calibration circuit */
/* disable Tx VLD force mode */
/* disable Tx offset/amplitude calibration circuit */

	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00db, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dc, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x003e, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x00dd, 0x0000);
}

static void do_ge_phy_all_analog_cal(u8 phyaddr)
{
	u16	reg0_temp, dev1e_145_temp, reg_temp;
	u16	reg_tmp;

	tc_mii_write(phyaddr, 0x1f, 0x0000);/* g0 */
	reg0_temp = tc_mii_read(phyaddr, 0x0);/* keep the default value */
/* set [12]AN disable, [8]full duplex, [13/6]1000Mbps */
	tc_mii_write(phyaddr, 0x0,  0x0140);

	tc_phy_write_dev_reg(phyaddr, 0x1f, 0x0100, 0xc000);/* BG voltage output */
	dev1e_145_temp = tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0145);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0145, 0x1010);/* fix mdi */
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0185, 0x0000);/* disable tx slew control */

	tc_phy_write_dev_reg(phyaddr, 0x1f, 0x27c, 0x1f1f);
	tc_phy_write_dev_reg(phyaddr, 0x1f, 0x27c, 0x3300);
	tc_phy_write_dev_reg(phyaddr, 0x1f, 0x273, 0);

	reg_tmp = tc_phy_read_dev_reg(phyaddr, 0x1e, 0x11);
	reg_tmp = reg_tmp | (0xf << 12);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x11, reg_tmp);

	/* calibration start ============ */
	ge_cal_flag = 1; /*GE calibration not calibration*/
	while (ge_cal_flag == 0)
		ge_cal_rext(phyaddr, 100);

	/* *** R50 Cal start ***************************** */
	/*phyaddress = 0*/
	ge_cal_r50(phyaddr, CALDLY);
	/* *** R50 Cal end *** */

	/* *** Tx offset Cal start *********************** */
	ge_cal_tx_offset(phyaddr, CALDLY);
	/* *** Tx offset Cal end *** */

	/* *** Tx Amp Cal start *** */
	ge_cal_tx_amp(phyaddr, CALDLY);
	/* *** Tx Amp Cal end *** */

	/* *** Rx offset Cal start *************** */
	/* 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal */
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0096, 0x8000);
	/* tx/rx_cal_criteria_value */
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0037, 0x0033);
	/* [14]: bypass all calibration, [11]: bypass adc offset cal analog */
	reg_temp = (tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0039) & (~0x4800));
	/* rx offset cal by Hw setup */
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0039, reg_temp);
	/* [12]: enable rtune calibration */
	reg_temp = (tc_phy_read_dev_reg(phyaddr, 0x1f, 0x0107) & (~0x1000));
	/* disable rtune calibration */
	tc_phy_write_dev_reg(phyaddr, 0x1f, 0x0107, reg_temp);
	/* 1e_171[8:7]: bypass tx/rx dc offset cancellation process */
	reg_temp = (tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0171) & (~0x0180));
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0171, (reg_temp | 0x0180));
	reg_temp = tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0039);
	/* rx offset calibration start */
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0039, (reg_temp | 0x2000));
	/* rx offset calibration end */
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0039, (reg_temp & (~0x2000)));
	mdelay(10);	/* mdelay for Hw calibration finish */
	reg_temp = (tc_phy_read_dev_reg(phyaddr, 0x1e, 0x0171) & (~0x0180));
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0171, reg_temp);

	tc_mii_write(phyaddr, 0x0,  reg0_temp);
	tc_phy_write_dev_reg(phyaddr, 0x1f, 0x0100, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0145, dev1e_145_temp);
	tc_phy_write_dev_reg(phyaddr, 0x1f, 0x273, 0x2000);
	/* *** Rx offset Cal end *** */
	/*eye pic*/
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x0, 0x018d);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x1, 0x01c7);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x2, 0x01c0);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x3, 0x003a);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x4, 0x0206);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x5, 0x0000);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x6, 0x038a);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x7, 0x03c8);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x8, 0x03c0);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x9, 0x0235);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0xa, 0x0008);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0xb, 0x0000);

	/*tmp maybe changed*/
	tc_phy_write_dev_reg(phyaddr, 0x1f, 0x27c, 0x1111);
	tc_phy_write_dev_reg(phyaddr, 0x1f, 0x27b, 0x47);
	tc_phy_write_dev_reg(phyaddr, 0x1f, 0x273, 0x2200);

	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x3a8, 0x0810);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x3aa, 0x0008);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x3ab, 0x0810);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x3ad, 0x0008);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x3ae, 0x0106);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x3b0, 0x0001);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x3b1, 0x0106);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x3b3, 0x0001);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x18c, 0x0001);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x18d, 0x0001);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x18e, 0x0001);
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x18f, 0x0001);

	/*da_tx_bias1_b_tx_standby = 5'b10 (dev1eh_reg3aah[12:8])*/
	reg_tmp = tc_phy_read_dev_reg(phyaddr, 0x1e, 0x3aa);
	reg_tmp = reg_tmp & ~(0x1f00);
	reg_tmp = reg_tmp | 0x2 << 8;
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x3aa, reg_tmp);

	/*da_tx_bias1_a_tx_standby = 5'b10 (dev1eh_reg3a9h[4:0])*/
	reg_tmp = tc_phy_read_dev_reg(phyaddr, 0x1e, 0x3a9);
	reg_tmp = reg_tmp & ~(0x1f);
	reg_tmp = reg_tmp | 0x2;
	tc_phy_write_dev_reg(phyaddr, 0x1e, 0x3a9, reg_tmp);

	/* SpaceX/Mtk: Enable Auto downshift from 1G to 100M */
	mii_mgr_write(phyaddr, 31, 1);
	mii_mgr_read(phyaddr, 20, &reg_tmp);
	mii_mgr_write(phyaddr, 20, reg_tmp | BIT(4));
	mii_mgr_write(phyaddr, 31, 0);

       //restart AN
	mii_mgr_write(phyaddr, 9, 0x200);
	mii_mgr_write(phyaddr, 0, 0x1340);
}

void leopard_ephy_cal(bool fe_cal)
{
	int i, dbg;
	int max_port = (fe_cal)? 4: 1;
	unsigned long t_s, t_e;

	dbg = 1;
	if (dbg) {
		t_s = jiffies;
		/* IEXT result of fe port 1 is always required. */
		for (i = 1; i <= max_port; i++)
			do_fe_phy_all_analog_cal(i, fe_cal);

		do_ge_phy_all_analog_cal(0);

		t_e = jiffies;
	}
	if (show_time)
		pr_info("cal time = %lu\n", (t_e - t_s) * 4);
}


