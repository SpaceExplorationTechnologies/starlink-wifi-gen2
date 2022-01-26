#ifndef __MT7626_H__
#define __MT7626_H__

#include "mcu/andes_core.h"
#include "phy/mt_rf.h"

struct _RTMP_ADAPTER;
struct _RTMP_CHIP_DBG;

#define MAX_RF_ID	127
#define MAC_RF_BANK 7

#ifdef WIFI_EAP_FEATURE
#define MT7626_MT_WTBL_SIZE	253
#else /* WIFI_EAP_FEATURE */
#define MT7626_MT_WTBL_SIZE	144
#endif

#define MT7626_MT_WMM_SIZE	4
#define MT7626_PDA_PORT		0xf800

#define MT7626_CT_PARSE_LEN	0

#define MT7626_BIN_FILE_NAME "WIFI_RAM_CODE_MT7626.bin"
#define MT7626_ROM_PATCH_BIN_FILE_NAME_E1 "mt7626_patch_e1_hdr.bin"

#define MT7626_ROM_PATCH_START_ADDRESS	0x0001C000

#define MT7626_RO_BAND0_PHYCTRL_STS0_OFFSET 0x4
#define MT7626_RO_BAND0_PHYCTRL_STS5_OFFSET	0xFFFFFFF4 /*-12*/
#define MT7626_PHY_BAND0_PHYMUX_5_OFFSET	0xFFFFFE00 /*-512*/

#ifdef CAL_BIN_FILE_SUPPORT
#define PA_TRIM_OFFSET        0
#define PA_TRIM_SIZE          16
#endif /* CAL_BIN_FILE_SUPPORT */

/* wilsonl */
/* */
/* Device ID & Vendor ID, these values should match EEPROM value */
/* */

#define OF_WBSYS_NAME "mediatek,wbsys"
#define RTMP_MAC_CSR_ADDR       0x18000000
#define RTMP_IRQ_NUM			243
#define RTMP_MAC_CSR_LEN		0x100000
#define RTMP_FLASH_BASE_ADDR    0xbc000000 /* wilsonl */

#define IOC_WBSYS_BASE	0x10000320
#define IOC_WBSYS_VALUE	0x80000000

#define HIF_INTR_BASE	0x10000700
#define HIF_INTR_VALUE	0x2
#define HIF_REMAP_SIZE	0x10


#ifdef RF_LOCKDOWN
#define COUNTRY_CODE_BYTE0_EEPROME_OFFSET   0x20
#define COUNTRY_CODE_BYTE1_EEPROME_OFFSET   0x21

#define COUNTRY_REGION_2G_EEPROME_OFFSET    0x22
#define COUNTRY_REGION_5G_EEPROME_OFFSET    0x23
#define COUNTRY_REGION_VALIDATION_MASK      BIT(7)
#define COUNTRY_REGION_VALIDATION_OFFSET    7
#define COUNTRY_REGION_CONTENT_MASK         BITS(0, 6)

#define RF_LOCKDOWN_EEPROME_BLOCK_OFFSET    0x1f
#define RF_LOCKDOWN_EEPROME_COLUMN_OFFSET   0xC
#define RF_LOCKDOWN_EEPROME_BIT             BIT(7)
#define RF_LOCKDOWN_EEPROME_BIT_OFFSET      7
#define RF_VALIDATION_NUMBER                10
#endif /* RF_LOCKDOWN */

#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT
typedef struct _EFUSE_INFO_T {
	/* MT7626 as example */
	UINT8 bytes_0x0_0x2F[0x30]; /* 0x00 ~ 0x2F : other purpose */
	UINT8 bytes_0x30_0x31[0x2]; /* 0x30 ~ 0x31 : for MT7761 / MT7762 PA current reduction */
	UINT8 bytes_0x32_0x3F[0xE]; /* 0x32 ~ 0x3F : other purpose */
	/* 0x40 */
	EFUSE_ADIE_INFO_T arADieCal[EFUSE_ADIE_MAX_NUM]; /* 0x40 ~ 0x5F : A-die 7761 & 7762 conttent */
	/* 0x60 */
	EFUSE_INFO_MODULE_SYSTEM_T rSys; /* 0x60 ~ 0x6F */ /* 16 bytes */
	/* 0x70 */
	EFUSE_INFO_MODULE_TX_POWER_T rTxPower; /* 0x70 ~ 0xEF */ /* 128 bytes */ /* use extend */
	/* 0xF0 */
	EFUSE_INFO_MODULE_2G4_COMMON_T r2G4Cmm; /* 0xF0 ~ 0x11F */ /* 48 bytes */
	/* 0x120 */
	EFUSE_INFO_MODULE_2G4_WIFI_PATH_T ar2G4WFPath[MAX_ANTENNA_NUM]; /* 0x120 ~ 0x14F */ /* 16*3=48 bytes */
	/* 0x150 */
	EFUSE_INFO_MODULE_IBF_CAL_T rIBfCal; /* 0x150 ~ 0x1F1 */ /* 18*9 = 162 bytes */
	UINT8 bytes_0x1F2_0x1FF[0xE]; /* 0x1F2 ~ 0x1FF : padding for iBF */ /* 14 bytes */
	/* 0x200 */
	EFUSE_INFO_ELEMENT_ELNA_RX_GAIN_COMP_T rRXELNAGainComp; /* 0x200 : for RX ELNA Gain Compensation */
	EFUSE_INFO_ELEMENT_2G_RX_ELNA_GAIN_DELTA_T r2GRxELNAGainDelta;  /* 0x201 ~ 0x208 : for 2G RX ELNA Gain Delta */
	UINT8 bytes_0x209_0x22F[0x27]; /* 0x209 ~ 0x22F : reserved */ /* 39 bytes */
	/* 0x230 */
	EFUSE_INFO_MODULE_5G_COMMON_T r5GCmm; /* 0x230 ~ 0x27F */ /* 80 bytes include reserved */
	/* 0x280 */
	EFUSE_INFO_MODULE_5G_WIFI_PATH_T ar5GWFPath[MAX_ANTENNA_NUM]; /* 0x280 ~ 0x3CF */ /* 112*3=336 bytes */
	/* 0x3D0 */
       UINT8 bytes_0x3D0x3DF[0x10]; /* 0x3D0 ~ 0x3DF: other purpose */    /* 16 bytes */
       UINT8 u1EdccaAntGain[EFUSE_ADIE_MAX_NUM]; /* 0x3E0 : for 2G antenna gain, 0x3E1 : for 5G antenna gain  */
       UINT8 u1EPADCPdet; /* 0x3E2 : for EPA DC Pdet */
       UINT8 bytes_0x3E3x3FF[0x1D]; /* 0x3E3 ~ 0x3FF: other purpose */    /* 29 bytes */
} EFUSE_INFO_T, *P_EFUSE_INFO_T;
#endif /*#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT*/

void mt7626_init(struct _RTMP_ADAPTER *pAd);
void mt7626_get_tx_pwr_per_rate(struct _RTMP_ADAPTER *pAd);
void mt7626_get_tx_pwr_info(struct _RTMP_ADAPTER *pAd);
void mt7626_antenna_sel_ctl(struct _RTMP_ADAPTER *pAd);
int mt7626_read_chl_pwr(struct _RTMP_ADAPTER *pAd);
void mt7626_pwrOn(struct _RTMP_ADAPTER *pAd);
void mt7626_calibration(struct _RTMP_ADAPTER *pAd, UCHAR channel);
void mt7626_tssi_compensation(struct _RTMP_ADAPTER *pAd, UCHAR channel);
VOID mt7626_chip_dbg_init(struct _RTMP_CHIP_DBG *dbg_ops);

#ifdef MT7626_FPGA
INT mt7626_chk_top_default_cr_setting(struct _RTMP_ADAPTER *pAd);
INT mt7626_chk_hif_default_cr_setting(struct _RTMP_ADAPTER *pAd);
#endif /* MT7663_FPGA */

#ifdef PRE_CAL_MT7626_SUPPORT
void mt7626_apply_dpd_flatness_data(struct _RTMP_ADAPTER *pAd, struct _MT_SWITCH_CHANNEL_CFG SwChCfg);

enum {
    GBAND = 0,
    ABAND = 1,
};


#ifdef RTMP_FLASH_SUPPORT
#define CAL_PRE_CAL_SIZE_OFFSET		1536	/* DW0 : 0x600 ~ 0x603 Used for save total pre-cal size
											* DW1 : reserved
											* DW2 : reserved
											* DW3 : reserved
											*/
#define CAL_FLASH_OFFSET			1552	/* 0x610 ~ 0x0x73F0 Used for save Group calibration data */
#else
#define CAL_FLASH_OFFSET			0
#endif

/* Group Calibration item */
#define PRE_CAL_2_4G_CR_NUM      528
#define PRE_CAL_5G_CR_NUM        6504

/*
 * TOTAL CR NUM : # of total 4Bytes = 2.4G Group + 5G Group
 * Fullsize     : 528 + 6504 = 7032
 */
#define PRE_CAL_TOTAL_CR_NUM     (PRE_CAL_2_4G_CR_NUM + PRE_CAL_5G_CR_NUM)

/*
 * TOTAL SIZE   : 2.4G Group + 5G Group size
 * Fullsize     : 7032 * 4 = 28128 Bytes
 */
#define PRE_CAL_TOTAL_SIZE       (PRE_CAL_TOTAL_CR_NUM * 4)


/* DPD & Flatness item */
#define PER_CHAN_FLATNESS_CR_NUM ((5 + 1) * 3)
#define PER_CHAN_DPD_2G_CR_NUM   ((50 + 1) * 3)
#define PER_CHAN_DPD_5G_CR_NUM   ((34 + 1) * 3)
#define DPD_FLATNESS_2G_CAL_SIZE ((PER_CHAN_DPD_2G_CR_NUM + PER_CHAN_FLATNESS_CR_NUM) * 4)
#define DPD_FLATNESS_5G_CAL_SIZE ((PER_CHAN_DPD_5G_CR_NUM + PER_CHAN_FLATNESS_CR_NUM) * 4)

#define DPD_FLATNESS_5G_CHAN_NUM 24
#define DPD_FLATNESS_2G_CHAN_NUM 3
#define TOTAL_CHAN_FOR_DPD_CAL   (DPD_FLATNESS_5G_CHAN_NUM + DPD_FLATNESS_2G_CHAN_NUM)

#define DPD_CAL_5G_TOTAL_SIZE    (DPD_FLATNESS_5G_CAL_SIZE * DPD_FLATNESS_5G_CHAN_NUM)
#define DPD_CAL_2G_TOTAL_SIZE    (DPD_FLATNESS_2G_CAL_SIZE * DPD_FLATNESS_2G_CHAN_NUM)
#define DPD_CAL_TOTAL_SIZE       (DPD_CAL_5G_TOTAL_SIZE + DPD_CAL_2G_TOTAL_SIZE)


/* Flast offset */
#define PRE_CAL_FLASH_OFFSET     (CAL_FLASH_OFFSET)
#define DPD_FLASH_OFFSET         (CAL_FLASH_OFFSET + PRE_CAL_TOTAL_SIZE)

/* Length limitation from Host to Firmware */
#define PRE_CAL_SET_MAX_CR_NUM   800
#define PRE_CAL_SET_MAX_LENGTH   (PRE_CAL_SET_MAX_CR_NUM * 4)

extern UINT16 MT7626_DPD_FLATNESS_ABAND_BW20_FREQ[];
extern UINT16 MT7626_DPD_FLATNESS_GBAND_BW20_FREQ[];
extern UINT16 MT7626_DPD_FLATNESS_BW20_FREQ[];
extern UINT16 MT7626_DPD_FLATNESS_ABAND_BW20_SIZE;
extern UINT16 MT7626_DPD_FLATNESS_GBAND_BW20_SIZE;
extern UINT16 MT7626_DPD_FLATNESS_BW20_FREQ_SIZE;

extern UINT16 MT7626_DPD_FLATNESS_ABAND_BW20_CH[];
extern UINT16 MT7626_DPD_FLATNESS_GBAND_BW20_CH[];
extern UINT16 MT7626_DPD_FLATNESS_BW20_CH[];
extern UINT16 MT7626_DPD_FLATNESS_ABAND_BW20_CH_SIZE;
extern UINT16 MT7626_DPD_FLATNESS_GBAND_BW20_CH_SIZE;
extern UINT16 MT7626_DPD_FLATNESS_B20_CH_SIZE;
#endif /*PRE_CAL_MT7626_SUPPORT*/

#endif /* __MT7626_H__ */

