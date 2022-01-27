/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SUNXI_SPC_H
#define SUNXI_SPC_H

#define SUNXI_SPC_NUM_PORTS		6

#define SUNXI_SPC_DECPORT_STA_REG(p)	(SUNXI_SPC_BASE + 0x0004 + 0x0c * (p))
#define SUNXI_SPC_DECPORT_SET_REG(p)	(SUNXI_SPC_BASE + 0x0008 + 0x0c * (p))
#define SUNXI_SPC_DECPORT_CLR_REG(p)	(SUNXI_SPC_BASE + 0x000c + 0x0c * (p))

#endif /* SUNXI_SPC_H */
