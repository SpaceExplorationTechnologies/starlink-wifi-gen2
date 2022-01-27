/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2019
 *
 * Richard van Schagen <vschagen@cs.com>
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/interrupt.h>
#include <linux/scatterlist.h>
#include <linux/types.h>
#include <crypto/aead.h>
#include <crypto/aes.h>
#include <crypto/authenc.h>
#include <crypto/des.h>
#include <crypto/internal/aead.h>
#include <crypto/internal/skcipher.h>
#include <crypto/scatterwalk.h>

#include "eip93-common.h"
#include "eip93-core.h"
#include "eip93-process.h"
#include "eip93-regs.h"
#include "eip93-ring.h"

static void mtk_free_sg_cpy(int cryptlen, struct scatterlist **sg)
{
	int len;

	if (!*sg)
		return;

	len = ALIGN(cryptlen, AES_BLOCK_SIZE);
	free_pages((unsigned long)sg_virt(*sg), get_order(len));

	kfree(*sg);
	*sg = NULL;
}

static int mtk_make_sg_cpy(struct scatterlist *src, struct scatterlist **dst,
			int len, struct mtk_cipher_reqctx *rctx)
{
	void *pages;
	int totallen;

	*dst = kmalloc(sizeof(**dst), GFP_ATOMIC);
	if (!*dst)
		return -ENOMEM;

	/* allocate enough memory for full scatterlist */
	totallen = rctx->assoclen + rctx->textsize + rctx->authsize;

	pages = (void *)__get_free_pages(GFP_ATOMIC, get_order(totallen));
	if (!pages) {
		kfree(*dst);
		*dst = NULL;
		return -ENOMEM;
	}

	sg_init_table(*dst, 1);
	sg_set_buf(*dst, pages, totallen);
	/* copy only as requested */
	sg_copy_to_buffer(src, sg_nents_for_len(src, totallen), pages, len);

	return 0;

}

static bool mtk_is_sg_aligned(struct scatterlist *sg, int len, int blocksize)
{
	int nents;

	if (!IS_ALIGNED(len, blocksize))
		return false;

	for (nents = 0; sg; sg = sg_next(sg), ++nents) {
		if (!IS_ALIGNED(sg->offset, sizeof(u32)))
			return false;

		if (sg->offset + sg->length > PAGE_SIZE) {
			printk("Cross Page Boundery!"); // for debug!
			return false;
		}

		if (len <= sg->length) {
			if (!IS_ALIGNED(len, blocksize))
				return false;

			sg->length = len;
			return true;
		}

		if (!IS_ALIGNED(sg->length, blocksize))
			return false;

		len -= sg->length;
	}

	return false;
}

int mtk_skcipher_setkey(struct crypto_ablkcipher *ctfm, const u8 *key,
				 unsigned int keylen)
{
	struct crypto_tfm *tfm = crypto_ablkcipher_tfm(ctfm);
	struct mtk_cipher_ctx *ctx = crypto_tfm_ctx(tfm);
	struct mtk_alg_template *tmpl = container_of(tfm->__crt_alg,
				struct mtk_alg_template, alg.skcipher);
	unsigned long flags = tmpl->flags;
	u32 tmp[DES_EXPKEY_WORDS];
	int ret = 0;

	if (!key || !keylen)
		return -EINVAL;

	if (IS_AES(flags)) {
		switch (keylen) {
		case AES_KEYSIZE_128:
		case AES_KEYSIZE_192:
		case AES_KEYSIZE_256:
			break;
		default:
			return -EINVAL;
		}
	}

	if (IS_DES(flags)) {
		if (keylen != DES_KEY_SIZE) {
			crypto_ablkcipher_set_flags(ctfm,
						CRYPTO_TFM_RES_BAD_KEY_LEN);
			return -EINVAL;
		}
		/* test for weak key */
		ret = des_ekey(tmp, key);
		if (!ret && (tfm->crt_flags & CRYPTO_TFM_REQ_WEAK_KEY)) {
			tfm->crt_flags |= CRYPTO_TFM_RES_WEAK_KEY;
			return -EINVAL;
		}
	}

	if (IS_3DES(flags)) {
		if (keylen != DES3_EDE_KEY_SIZE) {
			crypto_ablkcipher_set_flags(ctfm,
						CRYPTO_TFM_RES_BAD_KEY_LEN);
			return -EINVAL;
		}
	}

	ctx->keylen = keylen;
	memcpy(ctx->key, key, keylen);

	return 0;
}

static int mtk_aead_setkey(struct crypto_aead *ctfm, const u8 *key,
			unsigned int len)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(ctfm);
	struct mtk_cipher_ctx *ctx = crypto_tfm_ctx(tfm);
	struct crypto_authenc_keys keys;
	int bs = crypto_shash_blocksize(ctx->shash);
	int ds = crypto_shash_digestsize(ctx->shash);
	int ss = crypto_shash_statesize(ctx->shash);
	char *ipad = crypto_shash_ctx_aligned(ctx->shash);
	char *opad = ipad + ss;
	struct crypto_shash *hash = ctx->shash;
	SHASH_DESC_ON_STACK(shash, hash);
	unsigned int i, err;

	if (crypto_authenc_extractkeys(&keys, key, len) != 0)
		goto badkey;

	if (keys.enckeylen > sizeof(ctx->key))
		goto badkey;

	/* Encryption key */
	ctx->keylen = keys.enckeylen;
	memcpy(ctx->key, keys.enckey, keys.enckeylen);

	shash->tfm = hash;
	shash->flags = 0;

	if (keys.authkeylen > bs) {
		int err;

		err = crypto_shash_digest(shash, keys.authkey,
					keys.authkeylen, ipad);

		if (err)
			return err;

		keys.authkeylen = ds;
	} else
		memcpy(ipad, keys.authkey, keys.authkeylen);

	memset(ipad + keys.authkeylen, 0, bs - keys.authkeylen);
	memcpy(opad, ipad, bs);

	for (i = 0; i < bs; i++) {
		ipad[i] ^= HMAC_IPAD_VALUE;
		opad[i] ^= HMAC_OPAD_VALUE;
	}

	err = crypto_shash_init(shash) ?:
	       crypto_shash_update(shash, ipad, bs) ?:
	       crypto_shash_export(shash, ipad) ?:
	       crypto_shash_init(shash) ?:
	       crypto_shash_update(shash, opad, bs) ?:
	       crypto_shash_export(shash, opad);

	memcpy(ctx->ipad, ipad, SHA256_DIGEST_SIZE);
	memcpy(ctx->opad, opad, SHA256_DIGEST_SIZE);

	return 0;

badkey:
	crypto_aead_set_flags(ctfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return -EINVAL;
}

static int mtk_aead_setauthsize(struct crypto_aead *ctfm,
				unsigned int authsize)
{
	struct mtk_cipher_ctx *ctx = crypto_aead_ctx(ctfm);

	ctx->authsize = authsize;

	return 0;
}

static void mtk_set_saRecord(struct mtk_device *mtk, int wptr,
			struct mtk_cipher_ctx *ctx,
			struct mtk_cipher_reqctx *rctx)
{
	struct saRecord_s *saRecord;
	unsigned int flags = rctx->flags;

	saRecord = &mtk->saRecord[wptr];

	if IS_ENCRYPT(flags)
		saRecord->saCmd0.bits.direction = 0x0;
	else
		saRecord->saCmd0.bits.direction = 0x1;

	saRecord->saCmd0.bits.ivSource = 0x2;//0x2;Load IV from saState
	saRecord->saCmd0.bits.saveIv = 0x1 ;//0x1;Save IV to saState
	saRecord->saCmd0.bits.opGroup = 0x0; // 0 - basic operation

	if (ctx->aead)
		saRecord->saCmd0.bits.opCode = 0x1;
	else
		saRecord->saCmd0.bits.opCode = 0x0;

	if IS_DES(flags)
		saRecord->saCmd0.bits.cipher = 0x0;

	else if IS_3DES(flags)
		saRecord->saCmd0.bits.cipher = 0x1;

	else if IS_AES(flags)
		saRecord->saCmd0.bits.cipher = 0x3;

	if IS_HASH(flags)
		saRecord->saCmd0.bits.saveHash = 1;

	saRecord->saCmd0.bits.hash = 15;

	if IS_HASH_MD5(flags)
		saRecord->saCmd0.bits.hash = 0;

	else if IS_HASH_SHA1(flags)
		saRecord->saCmd0.bits.hash = 1;

	else if IS_HASH_SHA224(flags)
		saRecord->saCmd0.bits.hash = 2;

	else if IS_HASH_SHA256(flags)
		saRecord->saCmd0.bits.hash = 3;

	saRecord->saCmd0.bits.hdrProc = 0x0; // no header processing
	saRecord->saCmd0.bits.padType = 3; // Zero padding
	saRecord->saCmd0.bits.extPad = 0;
	saRecord->saCmd0.bits.scPad = 0; //no padding

	if IS_ECB(flags)
		saRecord->saCmd1.bits.cipherMode = 0;

	else if IS_CBC(flags)
		saRecord->saCmd1.bits.cipherMode = 1;

	else if IS_CTR(flags)
		saRecord->saCmd1.bits.cipherMode = 2;

	if (IS_HMAC(flags)) {
		saRecord->saCmd1.bits.hmac = 1;
		memcpy(saRecord->saIDigest, ctx->ipad, rctx->authsize);
		memcpy(saRecord->saODigest, ctx->opad, rctx->authsize);
		saRecord->saCmd1.bits.byteOffset = 0;
		saRecord->saCmd1.bits.hashCryptOffset =
						(rctx->assoclen / sizeof(u32));
		saRecord->saCmd0.bits.digestLength =
						(rctx->authsize / sizeof(u32));
		saRecord->saCmd1.bits.copyDigest = 1;
		saRecord->saCmd1.bits.copyPayload = 0;
		saRecord->saCmd1.bits.copyHeader = 1;
	}

	if (IS_AES(flags)) {
		if (ctx->keylen == AES_KEYSIZE_256)
			saRecord->saCmd1.bits.aesKeyLen = 4;
		else if (ctx->keylen == AES_KEYSIZE_192)
			saRecord->saCmd1.bits.aesKeyLen = 3;
		else
			saRecord->saCmd1.bits.aesKeyLen = 2;
	}

	saRecord->saCmd1.bits.seqNumCheck = 0;

	memcpy(saRecord->saKey, ctx->key, ctx->keylen);

	saRecord->saSpi = 0x0; //WORDSWAP(spi); //esp spi

	saRecord->saSeqNumMask[0] = 0xFFFFFFFF;
	saRecord->saSeqNumMask[1] = 0x0;
}

int mtk_scatter_combine(struct mtk_device *mtk, dma_addr_t saRecord_base,
			dma_addr_t saState_base, struct scatterlist *sgsrc,
			struct scatterlist *sgdst, int datalen,  bool complete,
			struct crypto_async_request *areq, int *commands,
			int *results)
{
	struct aead_request *req = aead_request_cast(areq);
	struct mtk_cipher_reqctx *rctx = aead_request_ctx(req);
	struct mtk_desc_buf *buf;
	unsigned int remainin, remainout;
	int offsetin = 0, offsetout = 0;
	unsigned int n, len;
	dma_addr_t saddr, daddr;
	bool nextin = false;
	bool nextout = false;
	struct eip93_descriptor_s *cdesc;
	struct eip93_descriptor_s *rdesc;
	int ndesc_cdr = 0, ndesc_rdr = 0;
	int wptr;

	n = datalen;

	remainin = min(sgsrc->length, n);
	remainout = min(sgdst->length, n);

	saddr = dma_map_page(mtk->dev, sg_page(sgsrc), sgsrc->offset,
			     remainin, DMA_TO_DEVICE);

	daddr = dma_map_page(mtk->dev, sg_page(sgdst), sgdst->offset,
			     remainout, DMA_FROM_DEVICE);

	do {
		if (nextin) {
			sgsrc++;
			remainin = min(sgsrc->length, n);
			if (remainin == 0)
				continue;

			saddr = dma_map_page(mtk->dev, sg_page(sgsrc),
				sgsrc->offset, remainin, DMA_TO_DEVICE);
			offsetin = 0;
			nextin = false;
		}

		if (nextout) {
			sgdst++;
			remainout = min(sgdst->length, n);
			if (remainout == 0)
				continue;

			daddr = dma_map_page(mtk->dev, sg_page(sgdst),
				sgdst->offset, remainout, DMA_FROM_DEVICE);
			offsetout = 0;
			nextout = false;
		}
		if (remainin == remainout) {
			len = remainin;
				nextin = true;
				nextout = true;
		} else if (remainin < remainout) {
			len = remainin;
				offsetout += len;
				remainout -= len;
				nextin = true;
		} else {
			len = remainout;
				offsetin += len;
				remainin -= len;
				nextout = true;
		}

		cdesc = mtk_add_cdesc(mtk, saddr + offsetin, daddr + offsetout,
				      saRecord_base, saState_base, len, 0, 1);
		rdesc = mtk_add_rdesc(mtk);

		wptr = mtk_ring_cdr_index(mtk, cdesc);

		buf = &mtk->ring[0].dma_buf[wptr];
		buf->flags = 0;
		buf->req = areq;
		ndesc_cdr++;
		ndesc_rdr++;
		n -= len;

		dev_dbg(mtk->dev, "[%d]Src:%08x,Len:%d wptr:%d\n",
			ndesc_cdr, saddr + offsetin, len, wptr);
	} while (n);

	if (rctx->authsize > 0) {
		daddr = dma_map_page(mtk->dev, sg_page(sgdst),
			(sgdst->offset + len), rctx->authsize, DMA_FROM_DEVICE);
		dma_unmap_len_set(buf, auth_len, rctx->authsize);
	}

	if (complete == true)
		buf->flags |= BIT(0); // Indicate last

	*commands = ndesc_cdr;
	*results = ndesc_rdr;

	return 0;
}

static int mtk_send_req(struct crypto_async_request *base,
			struct mtk_cipher_ctx *ctx,
			struct scatterlist *reqsrc, struct scatterlist *reqdst,
			u8 *reqiv, struct mtk_cipher_reqctx *rctx,
			int *commands, int *results)
{
	struct mtk_device *mtk = ctx->mtk;
	int ndesc_cdr = 0, ndesc_rdr = 0, ctr_cdr = 0, ctr_rdr = 0;
	int offset = 0, wptr, err;
	u32 datalen;
	struct scatterlist *src, *src_ctr;
	struct scatterlist *dst, *dst_ctr;
	struct saState_s *saState;
	struct saRecord_s *saRecord;
	dma_addr_t saState_base, saRecord_base;
	u32 start, end, ctr, blocks;
	unsigned long flags;
	bool complete;
	u32 iv[AES_BLOCK_SIZE / sizeof(u32)];

	datalen = rctx->assoclen + rctx->textsize;
	complete = true;
	flags = rctx->flags;

	wptr = mtk_ring_curr_wptr_index(mtk);

	saState = &mtk->saState[wptr];
	saState_base = mtk->saState_base + wptr * sizeof(saState_t);
	memset(saState, 0x00, sizeof(saState_t));

	saRecord = &mtk->saRecord[wptr];
	saRecord_base = mtk->saRecord_base + wptr * sizeof(saRecord_t);
	memset(saRecord, 0x00, sizeof(saRecord_t));
	mtk_set_saRecord(mtk, wptr, ctx, rctx);

	if (IS_CBC(flags) || IS_CTR(flags)) {
		if (!reqiv)
			memset(saState->stateIv, 0xFF, AES_BLOCK_SIZE);
		else
			memcpy(saState->stateIv, reqiv, AES_BLOCK_SIZE);
			memcpy(iv, reqiv, AES_BLOCK_SIZE);
	}

	if (ctx->aead)
		err = false;
	else
		err = mtk_is_sg_aligned(reqdst, datalen + rctx->authsize,
				AES_BLOCK_SIZE);

	if (!err) {
		rctx->sg_dst = reqdst;
		rctx->dst_nents = sg_nents_for_len(reqdst, datalen);
		mtk_make_sg_cpy(rctx->sg_dst, &rctx->sg_dst, datalen, rctx);
		dst = rctx->sg_dst;
	} else {
		rctx->sg_dst = NULL;
		dst = reqdst;
	}

	if (ctx->aead)
		err = false;
	else
		err = mtk_is_sg_aligned(reqsrc, datalen, AES_BLOCK_SIZE);

	if (!err) {
		rctx->sg_src = reqsrc;
		mtk_make_sg_cpy(rctx->sg_src, &rctx->sg_src, datalen, rctx);
		src = rctx->sg_src;
	} else {
		rctx->sg_src = NULL;
		src = reqsrc;
	}

	if (IS_CTR(rctx->flags)) {
		/* Compute data length. */
		blocks = DIV_ROUND_UP(datalen, AES_BLOCK_SIZE);
		ctr = be32_to_cpu(iv[3]);
		/* Check 32bit counter overflow. */
		start = ctr;
		end = start + blocks - 1;
		if (end < start) {
			offset = AES_BLOCK_SIZE * -start;
			/*
			 * Increment the counter manually to cope with
			 * the hardware counter overflow.
			 */
			if (offset < datalen) {
				memcpy(iv, reqiv, AES_BLOCK_SIZE);
				ctr |= 0xffffffff;
				iv[3] = cpu_to_be32(ctr);
				crypto_inc((u8 *)iv, AES_BLOCK_SIZE);
				complete = false;
			}
		}
	}

	if (unlikely(complete == false)) {
		src_ctr = src;
		dst_ctr = dst;
		err = mtk_scatter_combine(mtk, saRecord_base,
				saState_base, src, dst,
				offset, complete, (void *)base,
				&ctr_cdr, &ctr_rdr);
		/* Jump to offset. */
		src = scatterwalk_ffwd(rctx->ctr_src, src_ctr, offset);
		dst = ((src_ctr == dst_ctr) ? src :
		       scatterwalk_ffwd(rctx->ctr_dst, dst_ctr, offset));
		/* Set new State */
		wptr = mtk_ring_curr_wptr_index(mtk);
		saState = &mtk->saState[wptr];
		saState_base = mtk->saState_base + wptr * sizeof(saState_t);
		memcpy(saState->stateIv, iv, AES_BLOCK_SIZE);
		datalen -= offset;
		complete = true;
	}

	err = mtk_scatter_combine(mtk, saRecord_base,
			saState_base, src, dst,
			datalen, complete, (void *)base,
			&ndesc_cdr, &ndesc_rdr);

	*commands = ndesc_cdr + ctr_cdr;
	*results = ndesc_rdr + ctr_rdr;

	return 0;
}

int mtk_queue_req(struct crypto_async_request *base,
			struct mtk_cipher_reqctx *rctx)
{
	struct mtk_cipher_ctx *ctx = crypto_tfm_ctx(base->tfm);
	struct mtk_device *mtk = ctx->mtk;
	int ret;
	unsigned long flags;

	spin_lock_irqsave(&mtk->ring[0].queue_lock, flags);
	ret = crypto_enqueue_request(&mtk->ring[0].queue, base);
	spin_unlock_irqrestore(&mtk->ring[0].queue_lock, flags);

	queue_work(mtk->ring[0].workqueue, &mtk->ring[0].work_data.work);

	return ret;
}

int mtk_skcipher_send(struct crypto_async_request *async,
			int *commands, int *results)
{
	struct ablkcipher_request *req = ablkcipher_request_cast(async);
	struct mtk_cipher_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	struct mtk_cipher_reqctx *rctx = ablkcipher_request_ctx(req);
	int err;

	err = mtk_send_req(async, ctx, req->src, req->dst, req->info,
				rctx, commands, results);
	return err;
}

int mtk_aead_send(struct crypto_async_request *async,
			int *commands, int *results)
{
	struct aead_request *req = aead_request_cast(async);
	struct mtk_cipher_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	struct mtk_cipher_reqctx *rctx = aead_request_ctx(req);
	int err;

	err = mtk_send_req(async, ctx, req->src, req->dst, req->iv,
				rctx, commands, results);

	return err;
}

int mtk_req_result(struct mtk_device *mtk, struct mtk_cipher_reqctx *rctx,
		struct scatterlist *reqsrc, struct scatterlist *reqdst,
		unsigned int len, bool *should_complete, int *ret)
{
	struct eip93_descriptor_s *cdesc;
	struct eip93_descriptor_s *rdesc;
	struct mtk_desc_buf *buf;
	int ndesc = 0, rptr, nreq;

	*ret = 0;
	*should_complete = false;


	nreq = readl(mtk->base + EIP93_REG_PE_RD_COUNT) & GENMASK(10, 0);

	while (ndesc < nreq) {
		rdesc = mtk_ring_next_rptr(mtk, &mtk->ring[0].rdr);

		if (IS_ERR(rdesc)) {
			dev_err(mtk->dev, "cipher: result: error!\n");
			*ret = PTR_ERR(rdesc);
			break;
		}

		if (rdesc->peCrtlStat.bits.errStatus) {
			dev_err(mtk->dev, "Err: %02x\n",
				rdesc->peCrtlStat.bits.errStatus);
			*ret = -EINVAL;
		}

		cdesc = mtk_ring_next_rptr(mtk, &mtk->ring[0].cdr);
		rptr = mtk_ring_cdr_index(mtk, cdesc);

		buf = &mtk->ring[0].dma_buf[rptr];

		if (dma_unmap_len(buf, src_len))
			dma_unmap_page(mtk->dev,
				       dma_unmap_addr(buf, src_addr),
				       dma_unmap_len(buf, src_len),
				       DMA_FROM_DEVICE);

		if (dma_unmap_len(buf, dst_len)) {
			dma_unmap_page(mtk->dev,
				       dma_unmap_addr(buf, dst_addr),
				       dma_unmap_len(buf, dst_len),
				       DMA_TO_DEVICE);
			if (dma_unmap_len(buf, auth_len)) {
				dma_unmap_page(mtk->dev,
					       dma_unmap_addr(buf, dst_addr) +
					       dma_unmap_len(buf, dst_len),
					       dma_unmap_len(buf, auth_len),
					       DMA_TO_DEVICE);
			}
		}
		dma_unmap_len_set(buf, src_addr, 0);
		dma_unmap_len_set(buf, dst_addr, 0);
		dma_unmap_len_set(buf, auth_len, 0);

		ndesc++;
		if (buf->flags & BIT(0)) {
			*should_complete = true;
			break;
		}
	}

	if (rctx->authsize > 0)
		memcpy(rctx->odigest, sg_virt(rctx->sg_dst) + len,
		       rctx->authsize);

	if (*should_complete) {
		if (rctx->sg_src)
			mtk_free_sg_cpy(len, &rctx->sg_src);
		if (rctx->sg_dst) {
			sg_copy_from_buffer(reqdst, rctx->dst_nents,
					    sg_virt(rctx->sg_dst),
					    len + rctx->authsize);
			mtk_free_sg_cpy(len + rctx->authsize, &rctx->sg_dst);
		}
	}

	return ndesc;
}

int mtk_skcipher_handle_result(struct mtk_device *mtk,
				struct crypto_async_request *async,
				bool *should_complete,  int *ret)
{
	struct ablkcipher_request *req = ablkcipher_request_cast(async);
	struct mtk_cipher_reqctx *rctx = ablkcipher_request_ctx(req);
	int ndesc;

	ndesc = mtk_req_result(mtk, rctx, req->src, req->dst, rctx->textsize,
			       should_complete, ret);

	return ndesc;
}

int mtk_aead_handle_result(struct mtk_device *mtk,
				struct crypto_async_request *async,
				bool *should_complete,  int *ret)
{
	struct aead_request *req = aead_request_cast(async);
	struct mtk_cipher_reqctx *rctx = aead_request_ctx(req);
	int ndesc, datalen, i;
	void *odigest = rctx->odigest;

	datalen = rctx->assoclen + rctx->textsize;

	ndesc = mtk_req_result(mtk, rctx, req->src, req->dst,
			datalen, should_complete, ret);

	/* EIP93 Auth tag is network formatted (BE32) */
	for (i = 0; i < 8; i++)
		rctx->odigest[i] = ntohl(rctx->odigest[i]);

	if IS_ENCRYPT(rctx->flags)
		scatterwalk_map_and_copy(odigest, req->dst, datalen,
					 rctx->authsize, 1);
	return ndesc;
}

int mtk_skcipher_encrypt(struct ablkcipher_request *req)
{
	struct mtk_cipher_reqctx *rctx = ablkcipher_request_ctx(req);
	struct crypto_async_request *base = &req->base;
	struct mtk_alg_template *tmpl = container_of(base->tfm->__crt_alg,
					struct mtk_alg_template, alg.skcipher);

	rctx->flags = tmpl->flags;
	rctx->flags |= MTK_ENCRYPT;
	rctx->textsize = req->nbytes;
	rctx->authsize = 0;
	rctx->assoclen = 0;

	return mtk_queue_req(base, rctx);
}

int mtk_skcipher_decrypt(struct ablkcipher_request *req)
{
	struct mtk_cipher_reqctx *rctx = ablkcipher_request_ctx(req);
	struct crypto_async_request *base = &req->base;
	struct mtk_alg_template *tmpl = container_of(base->tfm->__crt_alg,
				struct mtk_alg_template, alg.skcipher);

	rctx->flags = tmpl->flags;
	rctx->flags |= MTK_DECRYPT;
	rctx->textsize = req->nbytes;
	rctx->authsize = 0;
	rctx->assoclen = 0;

	return mtk_queue_req(base, rctx);
}

int mtk_skcipher_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_cipher_ctx *ctx = crypto_tfm_ctx(tfm);
	struct mtk_alg_template *tmpl = container_of(tfm->__crt_alg,
				struct mtk_alg_template, alg.skcipher);

	memset(ctx, 0, sizeof(struct mtk_cipher_ctx));

	tfm->crt_ablkcipher.reqsize = sizeof(struct mtk_cipher_reqctx);

	ctx->mtk = tmpl->mtk;
	ctx->base.send = mtk_skcipher_send;
	ctx->base.handle_result = mtk_skcipher_handle_result;
	ctx->aead = false;

	return 0;
}

static unsigned int mtk_crypto_extsize(struct crypto_alg *alg)
{
	return sizeof(struct mtk_cipher_ctx);
}

static int mtk_crypto_init_tfm(struct crypto_tfm *tfm)
{
	struct crypto_shash *hash = __crypto_shash_cast(tfm);

	hash->descsize = crypto_shash_alg(hash)->descsize;
	return 0;
}


static const struct crypto_type crypto_shash_type = {
	.extsize = mtk_crypto_extsize,
	.init_tfm = mtk_crypto_init_tfm,
	.maskclear = ~CRYPTO_ALG_TYPE_MASK,
	.maskset = CRYPTO_ALG_TYPE_MASK,
	.type = CRYPTO_ALG_TYPE_SHASH,
	.tfmsize = offsetof(struct crypto_shash, base),
};

static int mtk_aead_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_cipher_ctx *ctx = crypto_tfm_ctx(tfm);
	struct mtk_alg_template *tmpl = container_of(tfm->__crt_alg,
				struct mtk_alg_template, alg.aead.base);
	struct mtk_device *mtk = tmpl->mtk;
	unsigned long flags = tmpl->flags;

	ctx->mtk = mtk;

	crypto_aead_set_reqsize(__crypto_aead_cast(tfm),
				sizeof(struct mtk_cipher_reqctx));

	ctx->aead = true;
	ctx->base.send = mtk_aead_send;
	ctx->base.handle_result = mtk_aead_handle_result;

	if (IS_HMAC(flags)) {
		if IS_HASH_SHA1(flags)
			ctx->shash = crypto_alloc_tfm("sha1",
				     &crypto_shash_type, 0, 0);

		else if IS_HASH_SHA224(flags)
			ctx->shash = crypto_alloc_tfm("sha224",
				     &crypto_shash_type, 0, 0);

		else if IS_HASH_SHA256(flags)
			ctx->shash = crypto_alloc_tfm("sha256",
				     &crypto_shash_type, 0, 0);

		if (IS_ERR(ctx->shash))
			return PTR_ERR(ctx->shash);
	}

	return 0;
}

static void mtk_aead_cra_exit(struct crypto_tfm *tfm)
{
	struct mtk_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	if (ctx->shash)
		crypto_free_shash(ctx->shash);
}

static int mtk_aead_encrypt(struct aead_request *req)
{
	struct mtk_cipher_reqctx *rctx = aead_request_ctx(req);
	struct crypto_async_request *base = &req->base;
	struct mtk_alg_template *tmpl = container_of(base->tfm->__crt_alg,
				struct mtk_alg_template, alg.aead.base);
	struct crypto_aead *tfm = crypto_aead_reqtfm(req);
	u32 authsize = crypto_aead_authsize(tfm);

	rctx->flags = tmpl->flags;
	rctx->flags |= MTK_ENCRYPT;
	rctx->textsize = req->cryptlen;
	rctx->assoclen = req->assoclen;
	rctx->authsize = authsize;


	return mtk_queue_req(base, rctx);
}

static int mtk_aead_decrypt(struct aead_request *req)
{
	struct mtk_cipher_reqctx *rctx = aead_request_ctx(req);
	struct crypto_async_request *base = &req->base;
	struct crypto_aead *tfm = crypto_aead_reqtfm(req);
	u32 authsize = crypto_aead_authsize(tfm);
	struct mtk_alg_template *tmpl = container_of(base->tfm->__crt_alg,
				struct mtk_alg_template, alg.aead.base);

	rctx->flags = tmpl->flags;
	rctx->flags |= MTK_DECRYPT;

	rctx->textsize = req->cryptlen - authsize;
	rctx->assoclen = req->assoclen;
	rctx->authsize = authsize;

	return mtk_queue_req(base, rctx);
}


/* Available algorithms in this module */

struct mtk_alg_template mtk_alg_ecb_aes = {
	.type = MTK_ALG_TYPE_SKCIPHER,
	.flags = MTK_MODE_ECB | MTK_ALG_AES,
	.alg.skcipher = {
		.cra_name = "ecb(aes)",
		.cra_driver_name = "eip93-ecb-aes",
		.cra_priority = 300,
		.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER |
				CRYPTO_ALG_KERN_DRIVER_ONLY | CRYPTO_ALG_ASYNC,
		.cra_blocksize = AES_BLOCK_SIZE,
		.cra_ctxsize = sizeof(struct mtk_cipher_ctx),
		.cra_alignmask = 0xf,
		.cra_type = &crypto_ablkcipher_type,
		.cra_init = mtk_skcipher_cra_init,
		.cra_module = THIS_MODULE,
		.cra_u.ablkcipher = {
			.setkey = mtk_skcipher_setkey,
			.encrypt = mtk_skcipher_encrypt,
			.decrypt = mtk_skcipher_decrypt,
			.min_keysize = AES_MIN_KEY_SIZE,
			.max_keysize = AES_MAX_KEY_SIZE,
			.ivsize	= 0,
		}
	},
};

struct mtk_alg_template mtk_alg_cbc_aes = {
	.type = MTK_ALG_TYPE_SKCIPHER,
	.flags = MTK_MODE_CBC | MTK_ALG_AES,
	.alg.skcipher = {
		.cra_name = "cbc(aes)",
		.cra_driver_name = "eip93-cbc-aes",
		.cra_priority = 300,
		.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER |
				CRYPTO_ALG_KERN_DRIVER_ONLY | CRYPTO_ALG_ASYNC,
		.cra_blocksize = AES_BLOCK_SIZE,
		.cra_ctxsize = sizeof(struct mtk_cipher_ctx),
		.cra_alignmask = 0xf,
		.cra_type = &crypto_ablkcipher_type,
		.cra_init = mtk_skcipher_cra_init,
		.cra_module = THIS_MODULE,
		.cra_u = {
			.ablkcipher = {
				.min_keysize = AES_MIN_KEY_SIZE,
				.max_keysize = AES_MAX_KEY_SIZE,
				.ivsize	= AES_BLOCK_SIZE,
				.setkey = mtk_skcipher_setkey,
				.encrypt = mtk_skcipher_encrypt,
				.decrypt = mtk_skcipher_decrypt,
			},
		},
	},
};

struct mtk_alg_template mtk_alg_ctr_aes = {
	.type = MTK_ALG_TYPE_SKCIPHER,
	.flags = MTK_MODE_CTR | MTK_ALG_AES,
	.alg.skcipher = {
		.cra_name = "ctr(aes)",
		.cra_driver_name = "eip93-ctr-aes",
		.cra_priority = 300,
		.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER |
				CRYPTO_ALG_KERN_DRIVER_ONLY | CRYPTO_ALG_ASYNC,
		.cra_blocksize = AES_BLOCK_SIZE,
		.cra_ctxsize = sizeof(struct mtk_cipher_ctx),
		.cra_alignmask = 0xf,
		.cra_type = &crypto_ablkcipher_type,
		.cra_init = mtk_skcipher_cra_init,
		.cra_module = THIS_MODULE,
		.cra_u.ablkcipher = {
			.setkey = mtk_skcipher_setkey,
			.encrypt = mtk_skcipher_encrypt,
			.decrypt = mtk_skcipher_decrypt,
			.min_keysize = AES_MIN_KEY_SIZE,
			.max_keysize = AES_MAX_KEY_SIZE,
			.ivsize	= AES_BLOCK_SIZE,
		}
	},
};

struct mtk_alg_template mtk_alg_authenc_hmac_sha1_cbc_aes = {
	.type = MTK_ALG_TYPE_AEAD,
	.flags = MTK_HASH_HMAC | MTK_HASH_SHA1 | MTK_MODE_CBC | MTK_ALG_AES,
	.alg.aead = {
		.setkey = mtk_aead_setkey,
		.encrypt = mtk_aead_encrypt,
		.decrypt = mtk_aead_decrypt,
		.ivsize	= AES_BLOCK_SIZE,
		.setauthsize = mtk_aead_setauthsize,
		.maxauthsize = SHA1_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha1),cbc(aes))",
			.cra_driver_name = "eip93-authenc-hmac-sha1-cbc-aes",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_ASYNC |
					CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_cipher_ctx),
			.cra_alignmask = 0xf,
			.cra_init = mtk_aead_cra_init,
			.cra_exit = mtk_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

struct mtk_alg_template mtk_alg_authenc_hmac_sha224_cbc_aes = {
	.type = MTK_ALG_TYPE_AEAD,
	.flags = MTK_HASH_HMAC | MTK_HASH_SHA224 | MTK_MODE_CBC | MTK_ALG_AES,
	.alg.aead = {
		.setkey = mtk_aead_setkey,
		.encrypt = mtk_aead_encrypt,
		.decrypt = mtk_aead_decrypt,
		.ivsize	= AES_BLOCK_SIZE,
		.setauthsize = mtk_aead_setauthsize,
		.maxauthsize = SHA224_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha224),cbc(aes))",
			.cra_driver_name = "eip93-authenc-hmac-sha224-cbc-aes",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_ASYNC |
					CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_cipher_ctx),
			.cra_alignmask = 0xf,
			.cra_init = mtk_aead_cra_init,
			.cra_exit = mtk_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

struct mtk_alg_template mtk_alg_authenc_hmac_sha256_cbc_aes = {
	.type = MTK_ALG_TYPE_AEAD,
	.flags = MTK_HASH_HMAC | MTK_HASH_SHA256 | MTK_MODE_CBC | MTK_ALG_AES,
	.alg.aead = {
		.setkey = mtk_aead_setkey,
		.encrypt = mtk_aead_encrypt,
		.decrypt = mtk_aead_decrypt,
		.ivsize	= AES_BLOCK_SIZE,
		.setauthsize = mtk_aead_setauthsize,
		.maxauthsize = SHA256_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha256),cbc(aes))",
			.cra_driver_name = "eip93-authenc-hmac-sha256-cbc-aes",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_ASYNC |
					CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_cipher_ctx),
			.cra_alignmask = 0xf,
			.cra_init = mtk_aead_cra_init,
			.cra_exit = mtk_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

