/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2019
 *
 * Richard van Schagen <vschagen@cs.com>
 */

#ifndef _CIPHER_H_
#define _CIPHER_H_

struct mtk_cipher_ctx {
	struct mtk_context	base;
	struct mtk_device	*mtk;
	u32			key[AES_MAX_KEY_SIZE / sizeof(u32)];
	u32			keylen;
	/* AEAD specific */
	bool			aead;
	u32			authsize;
	struct crypto_shash	*shash;
	u32			ipad[SHA256_DIGEST_SIZE / sizeof(u32)];
	u32			opad[SHA256_DIGEST_SIZE / sizeof(u32)];
};

struct mtk_cipher_reqctx {
	unsigned long	flags;
	int		textsize;
	/* AEAD */
	int		assoclen;
	int		authsize;
	u32		odigest[SHA256_DIGEST_SIZE];
	/* copy in case of mis-alignment or AEAD if no-consecutive blocks */
	struct scatterlist	*sg_src;
	int			src_nents;
	struct scatterlist	*sg_dst;
	int			dst_nents;
	/* AES-CTR in case of counter overflow */
	struct scatterlist	ctr_src[2];
	struct scatterlist	ctr_dst[2];
};

#endif /* _CIPHER_H_ */

#ifndef _CRYPTO_HMAC_H
#define _CRYPTO_HMAC_H

#define HMAC_IPAD_VALUE		0x36
#define HMAC_OPAD_VALUE		0x5c

#endif /* _CRYPTO_HMAC_H */

void *crypto_alloc_tfm(const char *alg_name,
		       const struct crypto_type *frontend, u32 type, u32 mask);
