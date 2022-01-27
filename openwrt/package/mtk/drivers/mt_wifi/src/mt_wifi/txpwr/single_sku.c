/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_single_sku.c
*/

/*******************************************************************************
 *    INCLUDED COMMON FILES
 ******************************************************************************/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Single_sku.tmh"
#endif
#else
#if defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif
#endif

/*******************************************************************************
 *    INCLUDED EXTERNAL FILES
 ******************************************************************************/


/*******************************************************************************
 *    INCLUDED INTERNAL FILES
 ******************************************************************************/

#ifdef RF_LOCKDOWN
#include	"txpwr/PowerLimit.h"
#endif /* RF_LOCKDOWN */

/*******************************************************************************
 *   PRIVATE DEFINITIONS
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE DATA
 ******************************************************************************/


/*******************************************************************************
 *    PUBLIC DATA
 ******************************************************************************/


/*******************************************************************************
 *    EXTERNAL DATA
 ******************************************************************************/

extern RTMP_STRING *__rstrtok;

/*******************************************************************************
 *    EXTERNAL FUNCTION PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/

NDIS_STATUS MtPwrLimitLoadParamHandle(RTMP_ADAPTER *pAd, UINT8 u1Type)
{
	PCHAR pi1Buffer;
	PDL_LIST pList = NULL;

	/* get pointer of link list address */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_LINK_LIST, (PVOID *)&pList);

	/* sanity check for null pointer */
	if (!pList)
		goto error4;

	/* Link list Init */
	DlListInit(pList);

	/* allocate memory for buffer power limit value */
	os_alloc_mem(pAd, (UINT8 **)&pi1Buffer, MAX_POWER_LIMIT_BUFFER_SIZE);

	if (!pi1Buffer)
		return NDIS_STATUS_FAILURE;

	/* update buffer with power limit table content */
	if (NDIS_STATUS_SUCCESS != MtReadPwrLimitTable(pAd, pi1Buffer, u1Type))
		goto error1;

	/* parsing sku table contents from buffer */
	if (NDIS_STATUS_SUCCESS != MtParsePwrLimitTable(pAd, pi1Buffer, u1Type))
		goto error2;

	/* enable flag for Read Power limit table pass */
		pAd->fgPwrLimitRead[u1Type] = TRUE;

	/* print out power limit table info */
	if (NDIS_STATUS_SUCCESS != MtShowPwrLimitTable(pAd, u1Type, DBG_LVL_INFO))
		goto error3;

	/* free allocated memory */
	os_free_mem(pi1Buffer);
	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Read Power Table Error!!\n", __func__));
	/* free allocated memory */
	os_free_mem(pi1Buffer);
	return NDIS_STATUS_FAILURE;

error2:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Parse Power Table Error!!\n", __func__));
	/* free allocated memory */
	os_free_mem(pi1Buffer);
	return NDIS_STATUS_FAILURE;

error3:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Show Power Table Error!!\n", __func__));
	/* free allocated memory */
	os_free_mem(pi1Buffer);
	return NDIS_STATUS_FAILURE;

error4:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for link list!!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPwrLimitUnloadParamHandle(RTMP_ADAPTER *pAd, UINT8 u1Type)
{
	P_CH_POWER prPwrLimitTbl, prTempPwrLimitTbl;
	PDL_LIST pList = NULL;

	/* get pointer of link list address */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_LINK_LIST, (PVOID *)&pList);

	/* sanity check for null pointer */
	if (!pList)
		goto error0;

	/* free allocated memory for power limit table */
	if (pAd->fgPwrLimitRead[u1Type]) {
		DlListForEachSafe(prPwrLimitTbl, prTempPwrLimitTbl, pList, CH_POWER, List) {

			/* delete this element link to next element */
			DlListDel(&prPwrLimitTbl->List);

			/* free memory for channel list with same table contents */
			os_free_mem(prPwrLimitTbl->pu1ChList);

			/* free memory for power limit parameters */
			os_free_mem(prPwrLimitTbl->pu1PwrLimit);

			/* free memory for table contents*/
			os_free_mem(prPwrLimitTbl);
		}

		/* disable flag for Read Power limit table pass */
		pAd->fgPwrLimitRead[u1Type] = FALSE;
	}

	return NDIS_STATUS_SUCCESS;

error0:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for link list!!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtParsePwrLimitTable(RTMP_ADAPTER *pAd, PCHAR pi1Buffer, UINT8 u1Type)
{
	UINT8 u1ChBand = CH_G_BAND;
	PCHAR pcReadline, pcToken, pcptr;
	UINT8 u1Channel = 0;
	UINT8 *prTempChList;
	UINT8 u1PwrLimitParamNum[TABLE_PARSE_TYPE_NUM] = {SINGLE_SKU_PARAM_PARSE_NUM, BACKOFF_PARAM_PARSE_NUM};
	P_CH_POWER prTbl = NULL, prStartCh = NULL;
#ifdef RF_LOCKDOWN
	UCHAR uDelimiter = '\t';
#else
	UCHAR uDelimiter = '\n';
#endif

	/* sanity check for null pointer */
	if (!pi1Buffer)
		goto error;

	for (pcReadline = pcptr = pi1Buffer; (pcptr = os_str_chr(pcReadline, uDelimiter)) != NULL; pcReadline = pcptr + 1) {
		*pcptr = '\0';

		/* Skip Phy mode notation cloumn line */
		if (pcReadline[0] == '#')
			continue;

		/* Channel Band Info Parsing */
		if (!strncmp(pcReadline, "Band: ", 6)) {

#ifdef RF_LOCKDOWN
			pcToken = rstrtok(pcReadline + 6, " ");
#else
			pcToken = rstrtok(pcReadline + 6, ",");
#endif /* RF_LOCKDOWN */

			/* sanity check for non-Null pointer */
			if (!pcToken)
				continue;

			u1ChBand = (UINT8)os_str_tol(pcToken, 0, 10);

			switch (u1ChBand) {
			case 2:
				u1ChBand = CH_G_BAND;
				break;
			case 5:
				u1ChBand = CH_A_BAND;
				break;
			default:
				break;
			}

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ChBand: %s\n", (CH_G_BAND == u1ChBand) ? "CH_G_BAND" : "CH_A_BAND"));
		}

		/* Rate Info Parsing for each u1Channel */
		if (!strncmp(pcReadline, "Ch", 2)) {
			/* Dynamic allocate memory for parsing structure */
			os_alloc_mem(pAd, (UINT8 **)&prTbl, sizeof(CH_POWER));
			/* set default value to 0 for parsing structure */
			os_zero_mem(prTbl, sizeof(CH_POWER));

			/* Dynamic allocate memory for parsing structure power limit paramters */
			os_alloc_mem(pAd, (UINT8 **)&prTbl->pu1PwrLimit, u1PwrLimitParamNum[u1Type]);
			/* set default value to 0 for parsing structure */
			os_zero_mem(prTbl->pu1PwrLimit, u1PwrLimitParamNum[u1Type]);

			/* pasrsing channel info */
#ifdef RF_LOCKDOWN
			pcToken = rstrtok(pcReadline + 2, " ");
#else
			pcToken = rstrtok(pcReadline + 2, ",");
#endif /* RF_LOCKDOWN */

			/* sanity check for null pointer */
			if (!pcToken) {
				/* free memory buffer of power limit parameters before escape this loop */
				os_free_mem(prTbl->pu1PwrLimit);
				/* free total memory buffer before escape this loop */
				os_free_mem(prTbl);
				/* escape this loop for Null pointer */
				continue;
			}

			u1Channel = (UINT8)os_str_tol(pcToken, 0, 10);
			prTbl->u1StartChannel = u1Channel;
			prTbl->u1ChBand = u1ChBand;

			/* Rate Info Parsing (CCK, OFDM, VHT20/40/80/160) */
			MtPwrLimitParse(pAd, prTbl->pu1PwrLimit, u1ChBand, u1Type);

			/* Create New Data Structure to simpilify the SKU table (Represent together for channels with same Rate Power Limit Info, Band Info) */
			if (!prStartCh) {
				/* (Begining) assign new pointer head to SKU table contents for this u1Channel */
				prStartCh = prTbl;
				/* add tail for Link list */
				if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type)
					DlListAddTail(&pAd->PwrLimitSkuList, &prTbl->List);
				else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type)
					DlListAddTail(&pAd->PwrLimitBackoffList, &prTbl->List);
			} else {
				BOOLEAN fgSameCont = TRUE;

				/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160) */
				MtPwrLimitSimilarCheck(pAd, prStartCh->pu1PwrLimit, prTbl->pu1PwrLimit, &fgSameCont, u1ChBand, u1Type);

				/* check if different info contents for different channel (channel band) */
				if (fgSameCont) {
					if (prStartCh->u1ChBand != prTbl->u1ChBand)
						fgSameCont = FALSE;
				}

				/* check similarity of SKU table content for different u1Channel */
				if (fgSameCont) {
					os_free_mem(prTbl->pu1PwrLimit);
					os_free_mem(prTbl);
				} else {
					/* Assign new pointer head to SKU table contents for this u1Channel */
					prStartCh = prTbl;
					/* add tail for Link list */
					if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type)
						DlListAddTail(&pAd->PwrLimitSkuList, &prStartCh->List);
					else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type)
						DlListAddTail(&pAd->PwrLimitBackoffList, &prStartCh->List);
				}
			}

			/* Increment total u1Channel counts for channels with same SKU table contents */
			prStartCh->u1ChNum++;
			/* allocate memory for u1Channel list with same SKU table contents */
			os_alloc_mem(pAd, (PUINT8 *)&prTempChList, prStartCh->u1ChNum);

			/* backup non-empty u1Channel list to prTempChList buffer */
			if (prStartCh->pu1ChList) {
				/* copy u1Channel list to prTempChList buffer */
				os_move_mem(prTempChList, prStartCh->pu1ChList, prStartCh->u1ChNum - 1);
				/* free memory for u1Channel list used before assign pointer of prTempChList memory buffer */
				os_free_mem(prStartCh->pu1ChList);
			}

			/* assign pointer of prTempChList memory buffer */
			prStartCh->pu1ChList = prTempChList;
			/* update latest u1Channel number to u1Channel list */
			prStartCh->pu1ChList[prStartCh->u1ChNum - 1] = u1Channel;
		}
	}

	return NDIS_STATUS_SUCCESS;

error:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer when parsing power limit table !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtReadPwrLimitTable(RTMP_ADAPTER *pAd, PCHAR pi1Buffer, UINT8 u1Type)
{
#ifdef RF_LOCKDOWN
#else
	RTMP_OS_FD_EXT srcfile;
	UCHAR *sku_path = NULL;
#endif /* RF_LOCKDOWN */

#ifdef RF_LOCKDOWN
	PUINT8 pcptrSkuTbl[TABLE_SIZE] = {Sku_01, Sku_02, Sku_03, Sku_04, Sku_05,
									  Sku_06, Sku_07, Sku_08, Sku_09, Sku_10,
									  Sku_11, Sku_12, Sku_13, Sku_14, Sku_15,
									  Sku_16, Sku_17, Sku_18, Sku_19, Sku_20};

	PUINT8 pcptrBackoffTbl[TABLE_SIZE] = {Backoff_01, Backoff_02, Backoff_03, Backoff_04, Backoff_05,
										  Backoff_06, Backoff_07, Backoff_08, Backoff_09, Backoff_10,
										  Backoff_11, Backoff_12, Backoff_13, Backoff_14, Backoff_15,
										  Backoff_16, Backoff_17, Backoff_18, Backoff_19, Backoff_20};
#endif /* RF_LOCKDOWN */

	/* sanity check for null pointer */
	if (!pi1Buffer)
		goto error1;

#ifdef RF_LOCKDOWN
	pAd->CommonCfg.SKUTableIdx = pAd->EEPROMImage[SINGLE_SKU_TABLE_EFFUSE_ADDRESS] & BITS(0, 6);
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, (KBLU "%s: RF_LOCKDOWN Feature ON !!!\n" KNRM, __func__));
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, (KBLU "%s: SKU Table index: %d\n" KNRM, __func__, pAd->CommonCfg.SKUTableIdx));

	/* init bufer for Sku table */
	os_zero_mem(pi1Buffer, MAX_POWER_LIMIT_BUFFER_SIZE);

	/* update buffer with sku table content */
	if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type)
		os_move_mem(pi1Buffer, pcptrSkuTbl[pAd->CommonCfg.SKUTableIdx], MAX_POWER_LIMIT_BUFFER_SIZE);
	else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type)
		os_move_mem(pi1Buffer, pcptrBackoffTbl[pAd->CommonCfg.SKUTableIdx], MAX_POWER_LIMIT_BUFFER_SIZE);

	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for buffer to read power limit table !!\n", __func__));
	return NDIS_STATUS_FAILURE;

#else

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: RF_LOCKDOWN Feature OFF !!!\n", __func__));

	/* open Sku table file */
	if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type)
		sku_path = get_single_sku_path(pAd);
	else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type)
		sku_path = get_bf_sku_path(pAd);

	if (sku_path && *sku_path)
		srcfile = os_file_open(sku_path, O_RDONLY, 0);
	else
		srcfile.Status = 1;

	if (srcfile.Status)
		goto error2;

	/* Read Sku table file */
	os_zero_mem(pi1Buffer, MAX_POWER_LIMIT_BUFFER_SIZE);

	if (os_file_read(srcfile, pi1Buffer, MAX_POWER_LIMIT_BUFFER_SIZE) < 0)
		goto error3;

	/* close Sku table file */
	if (os_file_close(srcfile) < 0)
		goto error4;

	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for buffer to read power limit table !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error2:
	/* file does not exist */
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--> Error opening file <%s>\n", sku_path));
	return NDIS_STATUS_FAILURE;

error3:
	/* file cannot not read */
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, (KRED "--> Error read <%s>\n" KNRM, sku_path));
	return NDIS_STATUS_FAILURE;

error4:
	/* file cannot not close */
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, (KRED "--> Error close <%s>\n" KNRM, sku_path));
	return NDIS_STATUS_FAILURE;

#endif
}

NDIS_STATUS MtPwrFillLimitParam(RTMP_ADAPTER *pAd, UINT8 ChBand, UINT8 u1ControlChannel,
				UINT8 u1CentralChannel, VOID *pi1PwrLimitParam, UINT8 u1Type)
{
	UINT8 u1RateIdx, u1FillParamType, u1ParamIdx, u1ParamIdx2, u1ChListIdx;
	PUINT8 pu1FillParamTypeLen = NULL;
	PUINT8 pu1RawDataIdxOffset = NULL;
	P_CH_POWER prPwrLimitTbl, prTempPwrLimitTbl;
	UINT8 u1TypeFillNum[TABLE_PARSE_TYPE_NUM] = {SINGLE_SKU_TYPE_NUM, BACKOFF_TYPE_NUM};
	UINT8 u1PwrLimitChannel;
	PDL_LIST pList = NULL;

	/* sanity check for null pointer */
	if (!pi1PwrLimitParam)
		goto error0;

	/* update power limit value data length */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_DATA_LENGTH, (PVOID *)&pu1FillParamTypeLen);

	/* sanity check for null pointer */
	if (!pu1FillParamTypeLen)
		goto error1;

	/* update power limit value raw data offset */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_RAW_DATA_OFFSET, (PVOID *)&pu1RawDataIdxOffset);

	/* update power limit link list */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_LINK_LIST, (PVOID *)&pList);

	/* sanity check for null pointer */
	if (!pu1RawDataIdxOffset)
		goto error1;

	if (u1ControlChannel >= 16) /* must be 5G */
		ChBand = 1;
	else if ((u1ControlChannel <= 14) && (u1ControlChannel >= 8)) /* depends on "channel_band" in MtCmdChannelSwitch */
		ChBand = ChBand;
	else if (u1ControlChannel <= 8) /* must be 2.4G */
		ChBand = 0;

	/* Check G-Band/A-Band and power limit channel */
	u1PwrLimitChannel = (ChBand) ? (u1ControlChannel) : (u1CentralChannel);

	DlListForEachSafe(prPwrLimitTbl, prTempPwrLimitTbl, pList, CH_POWER, List) {
		/* search for specific channel */
		for (u1ChListIdx = 0; u1ChListIdx < prPwrLimitTbl->u1ChNum; u1ChListIdx++) {
			/* check Channel Band and Channel */
			if ((ChBand == prPwrLimitTbl->u1ChBand) && (u1PwrLimitChannel == prPwrLimitTbl->pu1ChList[u1ChListIdx])) {

				/* update sku parameter for cck, ofdm, ht20/40, vht20/40/80/160 to buffer */
				for (u1FillParamType = 0, u1ParamIdx = 0, u1ParamIdx2 = 0; u1FillParamType < u1TypeFillNum[u1Type]; u1FillParamType++) {
					/* raw data index increment for different parameter type */
					u1ParamIdx2 = *(pu1RawDataIdxOffset + u1FillParamType);

					for (u1RateIdx = 0; u1RateIdx < *(pu1FillParamTypeLen + u1FillParamType); u1RateIdx++) {

						/* init power limit value */
						*((INT8 *)pi1PwrLimitParam + u1ParamIdx + u1RateIdx) = (0x3F);

						/* special case handler for ht40 mcs32 */
						if ((u1Type == POWER_LIMIT_TABLE_TYPE_SKU) &&
							(u1FillParamType == SINGLE_SKU_TABLE_HT40) &&
							(u1RateIdx == (SINGLE_SKU_FILL_TABLE_HT40_LENGTH - 1))) {
							if (prPwrLimitTbl->pu1PwrLimit + u1ParamIdx2 + (SINGLE_SKU_PARSE_TABLE_HTVHT40_LENGTH - 1))
								*((INT8 *)pi1PwrLimitParam + u1ParamIdx + u1RateIdx) = *(prPwrLimitTbl->pu1PwrLimit + u1ParamIdx2 + (SINGLE_SKU_PARSE_TABLE_HTVHT40_LENGTH - 1));
						} else {
							if (prPwrLimitTbl->pu1PwrLimit + u1ParamIdx2 + u1RateIdx)
								*((INT8 *)pi1PwrLimitParam + u1ParamIdx + u1RateIdx) = *(prPwrLimitTbl->pu1PwrLimit + u1ParamIdx2 + u1RateIdx);
						}
					}

					/* data index increment for different parameter type */
					u1ParamIdx += *(pu1FillParamTypeLen + u1FillParamType);
				}

				/* stop channel list search loop */
				break;
			}
		}
	}

	return NDIS_STATUS_SUCCESS;

error0:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for buffer to fill power limit table !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for parameter related to fill power limit table proc !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPwrLimitParse(RTMP_ADAPTER *pAd, PUINT8 pi1PwrLimitNewCh, UINT8 u1ChBand, UINT8 u1Type)
{
	UINT8 u1ColIdx, u1ParamType, u1ParamIdx;
	INT8  *pu1ParamTypeLen = NULL, *pu1ChBandNeedParse = NULL;
	PCHAR pcToken;
	UINT8 u1TypeParseNum[TABLE_PARSE_TYPE_NUM] = {SINGLE_SKU_TYPE_PARSE_NUM, BACKOFF_TYPE_PARSE_NUM};

	/* sanity check for null pointer */
	if (!pi1PwrLimitNewCh)
		goto error0;

	/* update power limit value raw data type number */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_RAW_DATA_LENGTH, (PVOID *)&pu1ParamTypeLen);

	/* sanity check for null pointer */
	if (!pu1ParamTypeLen)
		goto error1;

	/* update power limit value channel band need parsing bit-field */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_CH_BAND_NEED_PARSE_BITFIELD, (PVOID *)&pu1ChBandNeedParse);

	/* sanity check for null pointer */
	if (!pu1ChBandNeedParse)
		goto error1;

	/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160) */
	for (u1ParamType = 0, u1ParamIdx = 0; u1ParamType < u1TypeParseNum[u1Type]; u1ParamType++) {
		/* check if need to parse for specific channel band */
		if (*(pu1ChBandNeedParse + u1ParamType) & (u1ChBand + 1)) {
			for (u1ColIdx = 0; u1ColIdx < *(pu1ParamTypeLen + u1ParamType); u1ColIdx++) {
				/* toker update for next character parsing */
#ifdef RF_LOCKDOWN
				pcToken = rstrtok(NULL, " ");
#else
				pcToken = rstrtok(NULL, ",");
#endif /* RF_LOCKDOWN */
				if (!pcToken)
					break;
				/* config VHT20 Power Limit */
				MtPowerLimitFormatTrans(pAd, pi1PwrLimitNewCh + u1ColIdx + u1ParamIdx, pcToken);
			}
		}

		/* parameter index increment for different parameter type */
		u1ParamIdx += *(pu1ParamTypeLen + u1ParamType);
	}

	return NDIS_STATUS_SUCCESS;

error0:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for buffer to update power limit table after parsing !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for parameter related to parse power limit table proc !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPwrLimitSimilarCheck(RTMP_ADAPTER *pAd, PUINT8 pi1PwrLimitStartCh, PUINT8 pi1PwrLimitNewCh, BOOLEAN *pfgSameContent, UINT8 u1ChBand, UINT8 u1Type)
{
	UINT8 u1ColIdx, u1ParamType, u1ParamIdx;
	INT8  *pu1ParamTypeLen = NULL, *pu1ChBandNeedParse = NULL;
	UINT8 u1TypeParseNum[TABLE_PARSE_TYPE_NUM] = {SINGLE_SKU_TYPE_PARSE_NUM, BACKOFF_TYPE_PARSE_NUM};

	/* sanity check for null pointer */
	if (!pi1PwrLimitStartCh)
		goto error1;

	/* sanity check for null pointer */
	if (!pi1PwrLimitNewCh)
		goto error2;

	/* update power limit value raw data type number */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_RAW_DATA_LENGTH, (PVOID *)&pu1ParamTypeLen);

	/* sanity check for null pointer */
	if (!pu1ParamTypeLen)
		goto error3;

	/* update power limit value channel band need parsing bit-field */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_CH_BAND_NEED_PARSE_BITFIELD, (PVOID *)&pu1ChBandNeedParse);

	/* sanity check for null pointer */
	if (!pu1ChBandNeedParse)
		goto error3;

	/* same content flag init */
	*pfgSameContent = TRUE;

	/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160) */
	for (u1ParamType = 0, u1ParamIdx = 0; u1ParamType < u1TypeParseNum[u1Type]; u1ParamType++) {
		/* check if need to parse for specific channel band */
		if (*(pu1ChBandNeedParse + u1ParamType) & (u1ChBand + 1)) {
			for (u1ColIdx = 0; u1ColIdx < *(pu1ParamTypeLen + u1ParamType); u1ColIdx++) {
				if (*(pi1PwrLimitStartCh + u1ColIdx + u1ParamIdx) != *(pi1PwrLimitNewCh + u1ColIdx + u1ParamIdx)) {
					*pfgSameContent = FALSE;
					return NDIS_STATUS_SUCCESS;
				}
			}
		}

		/* parameter index increment for different parameter type */
		u1ParamIdx += *(pu1ParamTypeLen + u1ParamType);
	}

	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for pointer to power limit table start channel for check !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error2:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for pointer to power limit table current channel for check !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error3:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for parameter related to power limit table proc similar check !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtShowPwrLimitTable(RTMP_ADAPTER *pAd, UINT8 u1Type, UINT8 u1DebugLevel)
{
	PDL_LIST pList = NULL;
	UINT8 u1ColIdx, u1ParamType, u1ParamIdx;
	INT8 *pu1ParamTypeLen = NULL;
	P_CH_POWER prPwrLimitTbl, prTempPwrLimitTbl;
	UINT8 u1TypeParseNum[TABLE_PARSE_TYPE_NUM] = {SINGLE_SKU_TYPE_PARSE_NUM, BACKOFF_TYPE_PARSE_NUM};
	CHAR cSkuParseTypeName[SINGLE_SKU_TYPE_PARSE_NUM][7] = {"CCK", "OFDM", "VHT20", "VHT40", "VHT80", "VHT160"};
	CHAR cBackoffParseTypeName[BACKOFF_TYPE_PARSE_NUM][14] = {"BFOFF_CCK", "BF_OFF_OFDM", "BF_ON_OFDM", "BF_OFF_VHT20", "BF_ON_VHT20", "BF_OFF_VHT40", "BF_ON_VHT40", "BF_OFF_VHT80", "BF_ON_VHT80", "BF_OFF_VHT160", "BF_ON_VHT160"};

	/* update power limit value raw data type number */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_RAW_DATA_LENGTH, (PVOID *)&pu1ParamTypeLen);

	/* sanity check for null pointer */
	if (!pu1ParamTypeLen)
		goto error0;

	/* update pointer of link list address */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_LINK_LIST, (PVOID *)&pList);

	/* sanity check for null pointer */
	if (!pList)
		goto error1;

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("-----------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("SKU table index: %d \n", pAd->CommonCfg.SKUTableIdx));

	DlListForEachSafe(prPwrLimitTbl, prTempPwrLimitTbl, pList, CH_POWER, List) {
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("start channel: %d, ChListNum: %d\n", prPwrLimitTbl->u1StartChannel, prPwrLimitTbl->u1ChNum));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("Band: %d \n", prPwrLimitTbl->u1ChBand));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("Channel: "));
		for (u1ColIdx = 0; u1ColIdx < prPwrLimitTbl->u1ChNum; u1ColIdx++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->pu1ChList[u1ColIdx]));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

		/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160) */
		for (u1ParamType = 0, u1ParamIdx = 0; u1ParamType < u1TypeParseNum[u1Type]; u1ParamType++) {

			if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%s: ", cSkuParseTypeName[u1ParamType]));
			else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%s: ", cBackoffParseTypeName[u1ParamType]));

			for (u1ColIdx = 0; u1ColIdx < *(pu1ParamTypeLen + u1ParamType); u1ColIdx++)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", *(prPwrLimitTbl->pu1PwrLimit + u1ColIdx + u1ParamIdx)));
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

			/* parameter index increment for different parameter type */
			u1ParamIdx += *(pu1ParamTypeLen + u1ParamType);
		}
	}

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("-----------------------------------------------------------------\n"));

	return NDIS_STATUS_SUCCESS;

error0:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for parameter related to show power limit table !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for list of power limit table to show power limit info !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

VOID MtPwrLimitTblChProc(RTMP_ADAPTER *pAd, UINT8 u1BandIdx, UINT8 u1ChannelBand, UINT8 u1ControlChannel, UINT8 u1CentralChannel)
{
	UINT8 u1Type;

	for (u1Type = POWER_LIMIT_TABLE_TYPE_SKU; u1Type < POWER_LIMIT_TABLE_TYPE_NUM; u1Type++) {
		if (pAd->fgPwrLimitRead[u1Type])
			MtCmdPwrLimitTblUpdate(pAd, u1BandIdx, u1Type, u1ChannelBand, u1ControlChannel, u1CentralChannel);
	}
}

NDIS_STATUS MtPwrGetPwrLimitInstanceSku(RTMP_ADAPTER *pAd, ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx, PVOID *ppvBuffer)
{
	switch (eInstanceIdx) {
	case POWER_LIMIT_LINK_LIST:
		*ppvBuffer = &(pAd->PwrLimitSkuList);
		break;
	case POWER_LIMIT_RAW_DATA_LENGTH:
		*ppvBuffer = pAd->u1SkuParamLen;
		break;
	case POWER_LIMIT_RAW_DATA_OFFSET:
		*ppvBuffer = pAd->u1SkuParamTransOffset;
		break;
	case POWER_LIMIT_DATA_LENGTH:
		*ppvBuffer = pAd->u1SkuFillParamLen;
		break;
	case POWER_LIMIT_CH_BAND_NEED_PARSE_BITFIELD:
		*ppvBuffer = pAd->u1SkuChBandNeedParse;
		break;
	default:
		goto error;
	}

	return NDIS_STATUS_SUCCESS;

error:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: invalid instance for sku !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPwrGetPwrLimitInstanceBackoff(RTMP_ADAPTER *pAd, ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx, PVOID *ppvBuffer)
{
	switch (eInstanceIdx) {
	case POWER_LIMIT_LINK_LIST:
		*ppvBuffer = &(pAd->PwrLimitBackoffList);
		break;
	case POWER_LIMIT_RAW_DATA_LENGTH:
		*ppvBuffer = pAd->u1BackoffParamLen;
		break;
	case POWER_LIMIT_RAW_DATA_OFFSET:
		*ppvBuffer = pAd->u1BackoffParamTransOffset;
		break;
	case POWER_LIMIT_DATA_LENGTH:
		*ppvBuffer = pAd->u1BackoffFillParamLen;
		break;
	case POWER_LIMIT_CH_BAND_NEED_PARSE_BITFIELD:
		*ppvBuffer = pAd->u1BackoffChBandNeedParse;
		break;
	default:
		goto error;
	}

	return NDIS_STATUS_SUCCESS;

error:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: invalid instance for backoff !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPwrGetPwrLimitInstance(RTMP_ADAPTER *pAd, POWER_LIMIT_TABLE u1Type, ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx, PVOID *ppvBuffer)
{
	/* get pointer of link list address */
	switch (u1Type) {
	case POWER_LIMIT_TABLE_TYPE_SKU:
		MtPwrGetPwrLimitInstanceSku(pAd, eInstanceIdx, ppvBuffer);
		break;
	case POWER_LIMIT_TABLE_TYPE_BACKOFF:
		MtPwrGetPwrLimitInstanceBackoff(pAd, eInstanceIdx, ppvBuffer);
		break;
	default:
		goto error;
	}

	return NDIS_STATUS_SUCCESS;

error:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: invalid instance type !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}
NDIS_STATUS MtPowerLimitFormatTrans(RTMP_ADAPTER *pAd, PUINT8 pu1Value, PCHAR pcRawData)
{
	CHAR *cBuffer = NULL;
	CHAR *cToken = NULL;
	UINT8 u1NonInteValue = 0;

	/* sanity check for null pointer */
	if (!pu1Value)
		goto error1;

	/* sanity check for null poitner */
	if (!pcRawData)
		goto error2;

	/* neglect multiple spaces for content parsing */
	pcRawData += strspn(pcRawData, " ");

	/* decimal point existence check */
	if (!strchr(pcRawData, '.'))
		*pu1Value = (UINT8)os_str_tol(pcRawData, 0, 10) * 2;
	else {
		/* backup pointer to string of parser function */
		cBuffer = __rstrtok;

		/* parse integer part */
		cToken = rstrtok(pcRawData, ".");

		/* sanity check for null pointer */
		if (!cToken)
			goto error3;

		/* transform integer part unit to (0.5) */
		*pu1Value = (UINT8)os_str_tol(cToken, 0, 10) * 2;

		/* parse non-integer part */
		cToken = rstrtok(NULL, ".");

		/* sanity check for null pointer */
		if (!cToken)
			goto error4;

		/* get non-integer part */
		u1NonInteValue = (UINT8)os_str_tol(cToken, 0, 10);

		/* increment for non-zero non-integer part */
		if (u1NonInteValue >= 5)
			(*pu1Value) += 1;

		/* backup pointer to string of parser function */
		__rstrtok = cBuffer;
	}

	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: null pointer for buffer to update transform result !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error2:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: null pointer for raw data buffer !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error3:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: null pointer for integer value parsing !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error4:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: null pointer for decimal value parsing !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

CHAR SKUTxPwrOffsetGet(RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT8 ucBW, UINT8 ucPhymode, UINT8 ucMCS, UINT8 ucNss, BOOLEAN fgSE)
{
	CHAR   cPowerOffset = 0;
	UINT8  ucRateOffset = 0;
	UINT8  ucNSS = 1;
	UINT8  BW_OFFSET[4] = {VHT20_OFFSET, VHT40_OFFSET, VHT80_OFFSET, VHT160C_OFFSET};
#ifdef CONFIG_ATE
	struct	_ATE_CTRL	*ATECtrl = &(pAd->ATECtrl);
#endif

	/* Compute MCS rate and Nss for HT mode */
	if ((ucPhymode == MODE_HTMIX) || (ucPhymode == MODE_HTGREENFIELD)) {
		ucNss = (ucMCS >> 3) + 1;
		ucMCS &= 0x7;
	}

	switch (ucPhymode) {
	case MODE_CCK:
		ucRateOffset = SKU_CCK_OFFSET;

		switch (ucMCS) {
		case MCS_0:
		case MCS_1:
			ucRateOffset = SKU_CCK_RATE_M01;
			break;

		case MCS_2:
		case MCS_3:
			ucRateOffset = SKU_CCK_RATE_M23;
			break;

		default:
			break;
		}

		break;

	case MODE_OFDM:
		ucRateOffset = SKU_OFDM_OFFSET;

		switch (ucMCS) {
		case MCS_0:
		case MCS_1:
			ucRateOffset = SKU_OFDM_RATE_M01;
			break;

		case MCS_2:
		case MCS_3:
			ucRateOffset = SKU_OFDM_RATE_M23;
			break;

		case MCS_4:
		case MCS_5:
			ucRateOffset = SKU_OFDM_RATE_M45;
			break;

		case MCS_6:
			ucRateOffset = SKU_OFDM_RATE_M6;
			break;

		case MCS_7:
			ucRateOffset = SKU_OFDM_RATE_M7;
			break;

		default:
			break;
		}

		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		ucRateOffset = SKU_HT_OFFSET + BW_OFFSET[ucBW];

		switch (ucMCS) {
		case MCS_0:
			ucRateOffset += SKU_HT_RATE_M0;
			break;

		case MCS_1:
		case MCS_2:
			ucRateOffset += SKU_HT_RATE_M12;
			break;

		case MCS_3:
		case MCS_4:
			ucRateOffset += SKU_HT_RATE_M34;
			break;

		case MCS_5:
			ucRateOffset += SKU_HT_RATE_M5;
			break;

		case MCS_6:
			ucRateOffset += SKU_HT_RATE_M6;
			break;

		case MCS_7:
			ucRateOffset += SKU_HT_RATE_M7;
			break;
		}

		break;

	case MODE_VHT:
		ucRateOffset = SKU_VHT_OFFSET + BW_OFFSET[ucBW];

		switch (ucMCS) {
		case MCS_0:
			ucRateOffset += SKU_VHT_RATE_M0;
			break;

		case MCS_1:
		case MCS_2:
			ucRateOffset += SKU_VHT_RATE_M12;
			break;

		case MCS_3:
		case MCS_4:
			ucRateOffset += SKU_VHT_RATE_M34;
			break;

		case MCS_5:
		case MCS_6:
			ucRateOffset += SKU_VHT_RATE_M56;
			break;

		case MCS_7:
			ucRateOffset += SKU_VHT_RATE_M7;
			break;

		case MCS_8:
			ucRateOffset += SKU_VHT_RATE_M8;
			break;

		case MCS_9:
			ucRateOffset += SKU_VHT_RATE_M9;
			break;

		default:
			break;
		}

		break;
	}

	/* Update Power offset by look up Tx Power Compensation Table */
	cPowerOffset = (fgSE) ? (pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucRateOffset][ucNSS - 1]) : (pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucRateOffset][3]);

	/* Debug log for SKU Power offset to compensate */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 (KBLU "%s: ucBW: %d, ucPhymode: %d, ucMCS: %d, ucNss: %d, fgSPE: %d !!!\n" KNRM, __func__, ucBW, ucPhymode, ucMCS,
			  ucNss, fgSE));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (KBLU "%s: cPowerOffset: 0x%x (%d) !!!\n" KNRM, __func__,
			 cPowerOffset, cPowerOffset));

#ifdef CONFIG_ATE
	/* Check if Single SKU is disabled */
	if (!ATECtrl->fgTxPowerSKUEn)
		cPowerOffset = 0;
#endif
	return cPowerOffset;
}
