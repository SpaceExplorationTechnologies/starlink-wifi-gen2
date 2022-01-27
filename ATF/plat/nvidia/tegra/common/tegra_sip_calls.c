/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>

#include <memctrl.h>
#include <tegra_platform.h>
#include <tegra_private.h>

/*******************************************************************************
 * Common Tegra SiP SMCs
 ******************************************************************************/
#define TEGRA_SIP_NEW_VIDEOMEM_REGION		0x82000003
#define TEGRA_SIP_FIQ_NS_ENTRYPOINT		0x82000005
#define TEGRA_SIP_FIQ_NS_GET_CONTEXT		0x82000006

/*******************************************************************************
 * This function is responsible for handling all SiP calls
 ******************************************************************************/
uintptr_t tegra_sip_handler(uint32_t smc_fid,
			    u_register_t x1,
			    u_register_t x2,
			    u_register_t x3,
			    u_register_t x4,
			    void *cookie,
			    void *handle,
			    u_register_t flags)
{
	uint32_t regval, local_x2_32 = (uint32_t)x2;
	int32_t err;

	/* Check if this is a SoC specific SiP */
	err = plat_sip_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags);
	if (err == 0) {

		SMC_RET1(handle, (uint64_t)err);

	} else {

		switch (smc_fid) {

		case TEGRA_SIP_NEW_VIDEOMEM_REGION:
			/* Check whether Video memory resize is enabled */
			if (mmio_read_32(TEGRA_MC_BASE + MC_VIDEO_PROTECT_REG_CTRL)
				!= MC_VIDEO_PROTECT_WRITE_ACCESS_ENABLED) {
				ERROR("Video Memory Resize isn't enabled! \n");
				SMC_RET1(handle, (uint64_t)-ENOTSUP);
			}

			/*
			 * Check if Video Memory overlaps TZDRAM (contains bl31/bl32)
			 * or falls outside of the valid DRAM range
			*/
			err = bl31_check_ns_address(x1, local_x2_32);
			if (err != 0) {
				SMC_RET1(handle, (uint64_t)err);
			}

			/*
			 * Check if Video Memory is aligned to 1MB.
			 */
			if (((x1 & 0xFFFFFU) != 0U) || ((local_x2_32 & 0xFFFFFU) != 0U)) {
				ERROR("Unaligned Video Memory base address!\n");
				SMC_RET1(handle, (uint64_t)-ENOTSUP);
			}

			/*
			 * The GPU is the user of the Video Memory region. In order to
			 * transition to the new memory region smoothly, we program the
			 * new base/size ONLY if the GPU is in reset mode.
			 */
			regval = mmio_read_32(TEGRA_CAR_RESET_BASE +
					      TEGRA_GPU_RESET_REG_OFFSET);
			if ((regval & GPU_RESET_BIT) == 0U) {
				ERROR("GPU not in reset! Video Memory setup failed\n");
				SMC_RET1(handle, (uint64_t)-ENOTSUP);
			}

			/* new video memory carveout settings */
			tegra_memctrl_videomem_setup(x1, local_x2_32);

			/*
			 * Ensure again that GPU is still in reset after VPR resize
			 */
			regval = mmio_read_32(TEGRA_CAR_RESET_BASE +
					      TEGRA_GPU_RESET_REG_OFFSET);
			if ((regval & GPU_RESET_BIT) == 0U) {
				mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_GPU_RESET_GPU_SET_OFFSET,
									GPU_SET_BIT);
			}

			SMC_RET1(handle, 0);

		/*
		 * The NS world registers the address of its handler to be
		 * used for processing the FIQ. This is normally used by the
		 * NS FIQ debugger driver to detect system hangs by programming
		 * a watchdog timer to fire a FIQ interrupt.
		 */
		case TEGRA_SIP_FIQ_NS_ENTRYPOINT:

			if (x1 == 0U) {
				SMC_RET1(handle, SMC_UNK);
			}

			/*
			 * TODO: Check if x1 contains a valid DRAM address
			 */

			/* store the NS world's entrypoint */
			tegra_fiq_set_ns_entrypoint(x1);

			SMC_RET1(handle, 0);

		/*
		 * The NS world's FIQ handler issues this SMC to get the NS EL1/EL0
		 * CPU context when the FIQ interrupt was triggered. This allows the
		 * NS world to understand the CPU state when the watchdog interrupt
		 * triggered.
		 */
		case TEGRA_SIP_FIQ_NS_GET_CONTEXT:

			/* retrieve context registers when FIQ triggered */
			(void)tegra_fiq_get_intr_context();

			SMC_RET0(handle);

		default:
			ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
			break;
		}
	}

	SMC_RET1(handle, SMC_UNK);
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	tegra_sip_fast,

	(OEN_SIP_START),
	(OEN_SIP_END),
	(SMC_TYPE_FAST),
	(NULL),
	(tegra_sip_handler)
);
