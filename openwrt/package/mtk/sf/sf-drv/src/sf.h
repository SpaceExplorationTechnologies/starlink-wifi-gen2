// SPDX-License-Identifier:	GPL-2.0+
/*
 * Copyright (C) 2018 MediaTek Incorporation. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MT7622_SF_H_
#define _MT7622_SF_H_

#include <linux/types.h>

struct sf_op {
	unsigned int tx_num;
	unsigned int rx_num;
	unsigned char tx_buf[];
};

#endif /* _MT7622_SF_H_ */
