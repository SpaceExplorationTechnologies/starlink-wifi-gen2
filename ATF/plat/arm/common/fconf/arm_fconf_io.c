/*
 * Copyright (c) 2019-2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/io/io_storage.h>
#include <lib/object_pool.h>
#include <libfdt.h>
#include <tools_share/firmware_image_package.h>

#include <plat/arm/common/arm_fconf_getter.h>
#include <plat/arm/common/arm_fconf_io_storage.h>
#include <platform_def.h>

const io_block_spec_t fip_block_spec = {
	.offset = PLAT_ARM_FIP_BASE,
	.length = PLAT_ARM_FIP_MAX_SIZE
};

const io_uuid_spec_t arm_uuid_spec[MAX_NUMBER_IDS] = {
	[BL2_IMAGE_ID] = {UUID_TRUSTED_BOOT_FIRMWARE_BL2},
	[TB_FW_CONFIG_ID] = {UUID_TB_FW_CONFIG},
	[FW_CONFIG_ID] = {UUID_FW_CONFIG},
#if !ARM_IO_IN_DTB
	[SCP_BL2_IMAGE_ID] = {UUID_SCP_FIRMWARE_SCP_BL2},
	[BL31_IMAGE_ID] = {UUID_EL3_RUNTIME_FIRMWARE_BL31},
	[BL32_IMAGE_ID] = {UUID_SECURE_PAYLOAD_BL32},
	[BL32_EXTRA1_IMAGE_ID] = {UUID_SECURE_PAYLOAD_BL32_EXTRA1},
	[BL32_EXTRA2_IMAGE_ID] = {UUID_SECURE_PAYLOAD_BL32_EXTRA2},
	[BL33_IMAGE_ID] = {UUID_NON_TRUSTED_FIRMWARE_BL33},
	[HW_CONFIG_ID] = {UUID_HW_CONFIG},
	[SOC_FW_CONFIG_ID] = {UUID_SOC_FW_CONFIG},
	[TOS_FW_CONFIG_ID] = {UUID_TOS_FW_CONFIG},
	[NT_FW_CONFIG_ID] = {UUID_NT_FW_CONFIG},
#endif /* ARM_IO_IN_DTB */
#if TRUSTED_BOARD_BOOT
	[TRUSTED_BOOT_FW_CERT_ID] = {UUID_TRUSTED_BOOT_FW_CERT},
#if !ARM_IO_IN_DTB
	[TRUSTED_KEY_CERT_ID] = {UUID_TRUSTED_KEY_CERT},
	[SCP_FW_KEY_CERT_ID] = {UUID_SCP_FW_KEY_CERT},
	[SOC_FW_KEY_CERT_ID] = {UUID_SOC_FW_KEY_CERT},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {UUID_TRUSTED_OS_FW_KEY_CERT},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {UUID_NON_TRUSTED_FW_KEY_CERT},
	[SCP_FW_CONTENT_CERT_ID] = {UUID_SCP_FW_CONTENT_CERT},
	[SOC_FW_CONTENT_CERT_ID] = {UUID_SOC_FW_CONTENT_CERT},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {UUID_TRUSTED_OS_FW_CONTENT_CERT},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {UUID_NON_TRUSTED_FW_CONTENT_CERT},
#if defined(SPD_spmd)
	[SIP_SP_CONTENT_CERT_ID] = {UUID_SIP_SECURE_PARTITION_CONTENT_CERT},
	[PLAT_SP_CONTENT_CERT_ID] = {UUID_PLAT_SECURE_PARTITION_CONTENT_CERT},
#endif
#endif /* ARM_IO_IN_DTB */
#endif /* TRUSTED_BOARD_BOOT */
};

/* By default, ARM platforms load images from the FIP */
struct plat_io_policy policies[MAX_NUMBER_IDS] = {
	[FIP_IMAGE_ID] = {
		&memmap_dev_handle,
		(uintptr_t)&fip_block_spec,
		open_memmap
	},
	[BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[BL2_IMAGE_ID],
		open_fip
	},
	[TB_FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[TB_FW_CONFIG_ID],
		open_fip
	},
	[FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[FW_CONFIG_ID],
		open_fip
	},
#if !ARM_IO_IN_DTB
	[SCP_BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[SCP_BL2_IMAGE_ID],
		open_fip
	},
	[BL31_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[BL31_IMAGE_ID],
		open_fip
	},
	[BL32_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[BL32_IMAGE_ID],
		open_fip
	},
	[BL32_EXTRA1_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[BL32_EXTRA1_IMAGE_ID],
		open_fip
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[BL32_EXTRA2_IMAGE_ID],
		open_fip
	},
	[BL33_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[BL33_IMAGE_ID],
		open_fip
	},
	[HW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[HW_CONFIG_ID],
		open_fip
	},
	[SOC_FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[SOC_FW_CONFIG_ID],
		open_fip
	},
	[TOS_FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[TOS_FW_CONFIG_ID],
		open_fip
	},
	[NT_FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[NT_FW_CONFIG_ID],
		open_fip
	},
#endif /* ARM_IO_IN_DTB */
#if TRUSTED_BOARD_BOOT
	[TRUSTED_BOOT_FW_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[TRUSTED_BOOT_FW_CERT_ID],
		open_fip
	},
#if !ARM_IO_IN_DTB
	[TRUSTED_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[TRUSTED_KEY_CERT_ID],
		open_fip
	},
	[SCP_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[SCP_FW_KEY_CERT_ID],
		open_fip
	},
	[SOC_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[SOC_FW_KEY_CERT_ID],
		open_fip
	},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[TRUSTED_OS_FW_KEY_CERT_ID],
		open_fip
	},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[NON_TRUSTED_FW_KEY_CERT_ID],
		open_fip
	},
	[SCP_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[SCP_FW_CONTENT_CERT_ID],
		open_fip
	},
	[SOC_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[SOC_FW_CONTENT_CERT_ID],
		open_fip
	},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[TRUSTED_OS_FW_CONTENT_CERT_ID],
		open_fip
	},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[NON_TRUSTED_FW_CONTENT_CERT_ID],
		open_fip
	},
#if defined(SPD_spmd)
	[SIP_SP_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[SIP_SP_CONTENT_CERT_ID],
		open_fip
	},
	[PLAT_SP_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&arm_uuid_spec[PLAT_SP_CONTENT_CERT_ID],
		open_fip
	},
#endif
#endif /* ARM_IO_IN_DTB */
#endif /* TRUSTED_BOARD_BOOT */
};

#ifdef IMAGE_BL2

#if TRUSTED_BOARD_BOOT
#define FCONF_ARM_IO_UUID_NUMBER	U(21)
#else
#define FCONF_ARM_IO_UUID_NUMBER	U(10)
#endif

static io_uuid_spec_t fconf_arm_uuids[FCONF_ARM_IO_UUID_NUMBER];
static OBJECT_POOL_ARRAY(fconf_arm_uuids_pool, fconf_arm_uuids);

struct policies_load_info {
	unsigned int image_id;
	const char *name;
};

/* image id to property name table */
static const struct policies_load_info load_info[FCONF_ARM_IO_UUID_NUMBER] = {
	{SCP_BL2_IMAGE_ID, "scp_bl2_uuid"},
	{BL31_IMAGE_ID, "bl31_uuid"},
	{BL32_IMAGE_ID, "bl32_uuid"},
	{BL32_EXTRA1_IMAGE_ID, "bl32_extra1_uuid"},
	{BL32_EXTRA2_IMAGE_ID, "bl32_extra2_uuid"},
	{BL33_IMAGE_ID, "bl33_uuid"},
	{HW_CONFIG_ID, "hw_cfg_uuid"},
	{SOC_FW_CONFIG_ID, "soc_fw_cfg_uuid"},
	{TOS_FW_CONFIG_ID, "tos_fw_cfg_uuid"},
	{NT_FW_CONFIG_ID, "nt_fw_cfg_uuid"},
#if TRUSTED_BOARD_BOOT
	{TRUSTED_KEY_CERT_ID, "t_key_cert_uuid"},
	{SCP_FW_KEY_CERT_ID, "scp_fw_key_uuid"},
	{SOC_FW_KEY_CERT_ID, "soc_fw_key_uuid"},
	{TRUSTED_OS_FW_KEY_CERT_ID, "tos_fw_key_cert_uuid"},
	{NON_TRUSTED_FW_KEY_CERT_ID, "nt_fw_key_cert_uuid"},
	{SCP_FW_CONTENT_CERT_ID, "scp_fw_content_cert_uuid"},
	{SOC_FW_CONTENT_CERT_ID, "soc_fw_content_cert_uuid"},
	{TRUSTED_OS_FW_CONTENT_CERT_ID, "tos_fw_content_cert_uuid"},
	{NON_TRUSTED_FW_CONTENT_CERT_ID, "nt_fw_content_cert_uuid"},
#if defined(SPD_spmd)
	{SIP_SP_CONTENT_CERT_ID, "sip_sp_content_cert_uuid"},
	{PLAT_SP_CONTENT_CERT_ID, "plat_sp_content_cert_uuid"},
#endif
#endif /* TRUSTED_BOARD_BOOT */
};

int fconf_populate_arm_io_policies(uintptr_t config)
{
	int err, node;
	unsigned int i;
	unsigned int j;

	union uuid_helper_t uuid_helper;
	io_uuid_spec_t *uuid_ptr;

	/* As libfdt uses void *, we can't avoid this cast */
	const void *dtb = (void *)config;

	/* Assert the node offset point to "arm,io-fip-handle" compatible property */
	const char *compatible_str = "arm,io-fip-handle";
	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find %s compatible in dtb\n", compatible_str);
		return node;
	}

	/* Locate the uuid cells and read the value for all the load info uuid */
	for (i = 0; i < FCONF_ARM_IO_UUID_NUMBER; i++) {
		uuid_ptr = pool_alloc(&fconf_arm_uuids_pool);
		err = fdt_read_uint32_array(dtb, node, load_info[i].name,
					    4, uuid_helper.word);
		if (err < 0) {
			WARN("FCONF: Read cell failed for %s\n", load_info[i].name);
			return err;
		}

		/* Convert uuid from big endian to little endian */
		for (j = 0U; j < 4U; j++) {
			uuid_helper.word[j] =
				((uuid_helper.word[j] >> 24U) & 0xff) |
				((uuid_helper.word[j] << 8U) & 0xff0000) |
				((uuid_helper.word[j] >> 8U) & 0xff00) |
				((uuid_helper.word[j] << 24U) & 0xff000000);
		}

		VERBOSE("FCONF: arm-io_policies.%s cell found with value = 0x%x 0x%x 0x%x 0x%x\n",
			load_info[i].name,
			uuid_helper.word[0], uuid_helper.word[1],
			uuid_helper.word[2], uuid_helper.word[3]);

		uuid_ptr->uuid = uuid_helper.uuid_struct;
		policies[load_info[i].image_id].image_spec = (uintptr_t)uuid_ptr;
		policies[load_info[i].image_id].dev_handle = &fip_dev_handle;
		policies[load_info[i].image_id].check = open_fip;
	}
	return 0;
}

#if ARM_IO_IN_DTB
FCONF_REGISTER_POPULATOR(TB_FW, arm_io, fconf_populate_arm_io_policies);
#endif /* ARM_IO_IN_DTB */

#endif /* IMAGE_BL2 */
