# SPDX-License-Identifier:	GPL-2.0

# Empty file to make sure compilation pass
obj-y += empty.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_MEDIATEK_BOOTMENU) += load_data.o upgrade_helper.o spacex_mtk_efuse_check.o boot_helper.o
obj-$(CONFIG_MEDIATEK_BOOTMENU) += cmd_mtkupgrade.o cmd_mtkload.o \
				   cmd_mtkboardboot.o cmd_mtkautoboot.o
endif
