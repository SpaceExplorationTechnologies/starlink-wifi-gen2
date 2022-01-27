// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include "mtk-snand-def.h"

/* ECC registers */
#define ECC_ENCCON			0x000
#define ENC_EN				BIT(0)

#define ECC_ENCCNFG			0x004
#define ENC_MS_S			16
#define ENC_BURST_EN			BIT(8)
#define ENC_TNUM_S			0

#define ECC_ENCIDLE			0x00c
#define ENC_IDLE			BIT(0)

#define ECC_DECCON			0x100
#define DEC_EN				BIT(0)

#define ECC_DECCNFG			0x104
#define DEC_EMPTY_EN			BIT(31)
#define DEC_CS_S			16
#define DEC_CON_S			12
#define   DEC_CON_CORRECT		3
#define DEC_BURST_EN			BIT(8)
#define DEC_TNUM_S			0

#define ECC_DECIDLE			0x10c
#define DEC_IDLE			BIT(0)

#define ECC_DECENUM0			0x114
#define ECC_DECENUM(n)			(ECC_DECENUM0 + (n) * 4)

/* ECC_ENCIDLE & ECC_DECIDLE */
#define ECC_IDLE			BIT(0)

/* ENC_MODE & DEC_MODE */
#define ECC_MODE_NFI			1

#define ECC_TIMEOUT			500000

static const uint8_t mt7622_ecc_caps[] = { 4, 6, 8, 10, 12 };

static const uint8_t mt7986_ecc_caps[] = {
	4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24
};

static const uint32_t mt7622_ecc_regs[] = {
	[ECC_DECDONE] = 0x11c,
};

static const uint32_t mt7986_ecc_regs[] = {
	[ECC_DECDONE] = 0x124,
};

static const struct mtk_ecc_soc_data mtk_ecc_socs[__SNAND_SOC_MAX] = {
	[SNAND_SOC_MT7622] = {
		.ecc_caps = mt7622_ecc_caps,
		.num_ecc_cap = ARRAY_SIZE(mt7622_ecc_caps),
		.regs = mt7622_ecc_regs,
		.mode_shift = 4,
		.errnum_bits = 5,
		.errnum_shift = 5,
	},
	[SNAND_SOC_MT7629] = {
		.ecc_caps = mt7622_ecc_caps,
		.num_ecc_cap = ARRAY_SIZE(mt7622_ecc_caps),
		.regs = mt7622_ecc_regs,
		.mode_shift = 4,
		.errnum_bits = 5,
		.errnum_shift = 5,
	},
	[SNAND_SOC_MT7986] = {
		.ecc_caps = mt7986_ecc_caps,
		.num_ecc_cap = ARRAY_SIZE(mt7986_ecc_caps),
		.regs = mt7986_ecc_regs,
		.mode_shift = 5,
		.errnum_bits = 5,
		.errnum_shift = 8,
	},
};

static inline uint32_t ecc_read32(struct mtk_snand *snf, uint32_t reg)
{
	return readl(snf->ecc_base + reg);
}

static inline void ecc_write32(struct mtk_snand *snf, uint32_t reg,
			       uint32_t val)
{
	writel(val, snf->ecc_base + reg);
}

static inline void ecc_write16(struct mtk_snand *snf, uint32_t reg,
			       uint16_t val)
{
	writew(val, snf->ecc_base + reg);
}

static int mtk_ecc_poll(struct mtk_snand *snf, uint32_t reg, uint32_t bits)
{
	uint32_t val;

	return read16_poll_timeout(snf->ecc_base + reg, val, (val & bits), 0,
				   ECC_TIMEOUT);
}

static int mtk_ecc_wait_idle(struct mtk_snand *snf, uint32_t reg)
{
	int ret;

	ret = mtk_ecc_poll(snf, reg, ECC_IDLE);
	if (ret) {
		snand_log_ecc(snf->pdev, "ECC engine is busy\n");
		return -EBUSY;
	}

	return 0;
}

int mtk_ecc_setup(struct mtk_snand *snf, void *fmdaddr, uint32_t max_ecc_bytes,
		  uint32_t msg_size)
{
	uint32_t i, val, ecc_msg_bits, ecc_strength;
	int ret;

	snf->ecc_soc = &mtk_ecc_socs[snf->soc];

	snf->ecc_parity_bits = fls(1 + 8 * msg_size);
	ecc_strength = max_ecc_bytes * 8 / snf->ecc_parity_bits;

	for (i = snf->ecc_soc->num_ecc_cap - 1; i >= 0; i--) {
		if (snf->ecc_soc->ecc_caps[i] <= ecc_strength)
			break;
	}

	if (unlikely(i < 0)) {
		snand_log_ecc(snf->pdev, "Page size %u+%u is not supported\n",
			      snf->writesize, snf->oobsize);
		return -ENOTSUPP;
	}

	snf->ecc_strength = snf->ecc_soc->ecc_caps[i];
	snf->ecc_bytes = DIV_ROUND_UP(snf->ecc_strength * snf->ecc_parity_bits,
				      8);

	/* Encoder config */
	ecc_write16(snf, ECC_ENCCON, 0);
	ret = mtk_ecc_wait_idle(snf, ECC_ENCIDLE);
	if (ret)
		return ret;

	ecc_msg_bits = msg_size * 8;
	val = (ecc_msg_bits << ENC_MS_S) |
	      (ECC_MODE_NFI << snf->ecc_soc->mode_shift) | i;
	ecc_write32(snf, ECC_ENCCNFG, val);

	/* Decoder config */
	ecc_write16(snf, ECC_DECCON, 0);
	ret = mtk_ecc_wait_idle(snf, ECC_DECIDLE);
	if (ret)
		return ret;

	ecc_msg_bits += snf->ecc_strength * snf->ecc_parity_bits;
	val = DEC_EMPTY_EN | (ecc_msg_bits << DEC_CS_S) |
	      (DEC_CON_CORRECT << DEC_CON_S) |
	      (ECC_MODE_NFI << snf->ecc_soc->mode_shift) | i;
	ecc_write32(snf, ECC_DECCNFG, val);

	return 0;
}

int mtk_snand_ecc_encoder_start(struct mtk_snand *snf)
{
	int ret;

	ret = mtk_ecc_wait_idle(snf, ECC_ENCIDLE);
	if (ret) {
		ecc_write16(snf, ECC_ENCCON, 0);
		mtk_ecc_wait_idle(snf, ECC_ENCIDLE);
	}

	ecc_write16(snf, ECC_ENCCON, ENC_EN);

	return 0;
}

void mtk_snand_ecc_encoder_stop(struct mtk_snand *snf)
{
	mtk_ecc_wait_idle(snf, ECC_ENCIDLE);
	ecc_write16(snf, ECC_ENCCON, 0);
}

int mtk_snand_ecc_decoder_start(struct mtk_snand *snf)
{
	int ret;

	ret = mtk_ecc_wait_idle(snf, ECC_DECIDLE);
	if (ret) {
		ecc_write16(snf, ECC_DECCON, 0);
		mtk_ecc_wait_idle(snf, ECC_DECIDLE);
	}

	ecc_write16(snf, ECC_DECCON, DEC_EN);

	return 0;
}

void mtk_snand_ecc_decoder_stop(struct mtk_snand *snf)
{
	mtk_ecc_wait_idle(snf, ECC_DECIDLE);
	ecc_write16(snf, ECC_DECCON, 0);
}

int mtk_ecc_wait_decoder_done(struct mtk_snand *snf)
{
	uint16_t val, step_mask = (1 << snf->ecc_steps) - 1;
	uint32_t reg = snf->ecc_soc->regs[ECC_DECDONE];
	int ret;

	ret = read16_poll_timeout(snf->ecc_base + reg, val,
				  (val & step_mask) == step_mask, 0,
				  ECC_TIMEOUT);
	if (ret)
		snand_log_ecc(snf->pdev, "ECC decoder is busy\n");

	return ret;
}

int mtk_ecc_check_decode_error(struct mtk_snand *snf, uint32_t page)
{
	uint32_t i, regi, fi, errnum;
	uint32_t errnum_shift = snf->ecc_soc->errnum_shift;
	uint32_t errnum_mask = (1 << snf->ecc_soc->errnum_bits) - 1;
	int ret = 0;

	for (i = 0; i < snf->ecc_steps; i++) {
		regi = i / 4;
		fi = i % 4;

		errnum = ecc_read32(snf, ECC_DECENUM(regi));
		errnum = (errnum >> (fi * errnum_shift)) & errnum_mask;
		if (!errnum)
			continue;

		if (errnum <= snf->ecc_strength) {
			if (ret >= 0)
				ret += errnum;
			continue;
		}

		snand_log_ecc(snf->pdev,
			      "Uncorrectable bitflips in page %u sect %u\n",
			      page, i);
		ret = -EBADMSG;
	}

	return ret;
}
