#
# Copyright (C) 2019 MediaTek Inc.
#
# Author: Sam Shih <sam.shih@mediatek.com>
#
# SPDX-License-Identifier:     BSD-3-Clause
# https://spdx.org/licenses
#

TARGET := ar-table

.PHONY: all clean

all: ${TARGET}

%: %.py Makefile
	cp $< $@

clean:
	rm ${TARGET}
