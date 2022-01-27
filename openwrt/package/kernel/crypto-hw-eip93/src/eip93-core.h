/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2019
 *
 * Richard van Schagen <vschagen@cs.com>
 */
#ifndef _CORE_H_
#define _CORE_H_

#include <linux/dma-mapping.h>
#include <crypto/aead.h>
#include <crypto/algapi.h>
#include <crypto/internal/hash.h>
#include <crypto/sha.h>
#include <crypto/skcipher.h>


struct mtk_work_data {
	struct work_struct	work;
	struct mtk_device	*mtk;
};

/**
 * struct mtk_device - crypto engine device structure
 */
struct mtk_device {
	void __iomem		*base;
	struct device		*dev;
	int			irq;

	struct mtk_ring		*ring;
	struct saRecord_s	*saRecord;
	struct saState_s	*saState;
	dma_addr_t		saState_base;
	dma_addr_t		saRecord_base;
};

/**
 * struct mtk_desc_buf - holds the records associated with the ring buffer
 * @src_addr: Dma address of the source packet
 * @dst_addr: Dma address of the destination
 * @src_len : Dma length of the source
 * @dst_len : Dma length of the destination
 * @auth_len: Dma length of the authentication
 * @flags: Flags to indicate e.g. last block.
 * @req: crypto_async_request
 */
struct mtk_desc_buf {
	DEFINE_DMA_UNMAP_ADDR(src_addr);
	DEFINE_DMA_UNMAP_ADDR(dst_addr);
	unsigned int		src_len;
	unsigned int		dst_len;
	unsigned int		auth_len;
	unsigned int		flags;
	unsigned int		*req;
};

struct mtk_desc_ring {
	void			*base;
	void			*base_end;
	dma_addr_t		base_dma;

	/* write and read pointers */
	void			*read;
	void			*write;

	/* descriptor element offset */
	unsigned int		offset;
};

struct mtk_ring {
	spinlock_t			lock;

	struct workqueue_struct		*workqueue;
	struct mtk_work_data		work_data;

	/* command/result rings */
	struct mtk_desc_ring		cdr;
	struct mtk_desc_ring		rdr;

	/* descriptor scatter/gather record */
	struct mtk_desc_buf		*dma_buf;

	/* queue */
	struct crypto_queue		queue;
	spinlock_t			queue_lock;

	/* Number of request in the engine. */
	int				requests;

	/* The rings is handling at least one request */
	bool				busy;

	/* Store for current request when not
	 * enough resources avialable.
	 */
	struct crypto_async_request	*req;
	struct crypto_async_request	*backlog;
};

struct mtk_context {
	int (*send)(struct crypto_async_request *req, int *commands,
				int *results);
	int (*handle_result)(struct mtk_device *mtk,
				struct crypto_async_request *req,
				bool *complete,  int *ret);
};

enum mtk_alg_type {
	MTK_ALG_TYPE_SKCIPHER,
	MTK_ALG_TYPE_AEAD,
	MTK_ALG_TYPE_AHASH,
	MTK_ALG_TYPE_PRNG,
};

struct mtk_alg_template {
	struct mtk_device	*mtk;
	enum mtk_alg_type	type;
	unsigned long		flags;
	union {
		struct crypto_alg	skcipher;
		struct aead_alg		aead;
	} alg;
};

void mtk_push_request(struct mtk_device *mtk);

#endif /* _CORE_H_ */
