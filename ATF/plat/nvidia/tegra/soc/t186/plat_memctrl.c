/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/bl_common.h>

#include <mce.h>
#include <memctrl_v2.h>
#include <tegra186_private.h>
#include <tegra_mc_def.h>
#include <tegra_platform.h>
#include <tegra_private.h>

extern uint64_t tegra_bl31_phys_base;

/*******************************************************************************
 * Array to hold stream_id override config register offsets
 ******************************************************************************/
const static uint32_t tegra186_streamid_override_regs[] = {
	MC_STREAMID_OVERRIDE_CFG_SDMMCRA,
	MC_STREAMID_OVERRIDE_CFG_SDMMCRAA,
	MC_STREAMID_OVERRIDE_CFG_SDMMCR,
	MC_STREAMID_OVERRIDE_CFG_SDMMCRAB,
	MC_STREAMID_OVERRIDE_CFG_SDMMCWA,
	MC_STREAMID_OVERRIDE_CFG_SDMMCWAA,
	MC_STREAMID_OVERRIDE_CFG_SDMMCW,
	MC_STREAMID_OVERRIDE_CFG_SDMMCWAB,
};

/*******************************************************************************
 * Array to hold the security configs for stream IDs
 ******************************************************************************/
const static mc_streamid_security_cfg_t tegra186_streamid_sec_cfgs[] = {
	mc_make_sec_cfg(SCEW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(AFIR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(AFIW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVDISPLAYR1, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(XUSB_DEVR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(VICSRD1, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVENCSWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(TSECSRDB, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(AXISW, SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCWAB, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(AONDMAW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(GPUSWR2, SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SATAW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(UFSHCW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(SCEDMAW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(UFSHCR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCWAA, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(SESWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(MPCORER, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(PTCR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(BPMPW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(ETRW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(GPUSRD, SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(VICSWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SCEDMAR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(HDAW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(ISPWA, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(EQOSW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(XUSB_HOSTW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(TSECSWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCRAA, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(VIW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(AXISR, SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(BPMPDMAW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(ISPRA, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(NVDECSWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(XUSB_DEVW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(NVDECSRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(MPCOREW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVDISPLAYR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(BPMPDMAR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVJPGSWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVDECSRD1, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(TSECSRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVJPGSRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCWA, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(SCER, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(XUSB_HOSTR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(VICSRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(AONDMAR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(AONW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCRA, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(HOST1XDMAR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(EQOSR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(SATAR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(BPMPR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(HDAR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCRAB, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(ETRR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(AONR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SESRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVENCSRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(GPUSWR, SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(TSECSWRB, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(ISPWB, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(GPUSRD2, SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(APEDMAW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(APER, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(APEW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(APEDMAR, NON_SECURE, NO_OVERRIDE, DISABLE),
};

/*******************************************************************************
 * Array to hold the transaction override configs
 ******************************************************************************/
const static mc_txn_override_cfg_t tegra186_txn_override_cfgs[] = {
	mc_make_txn_override_cfg(BPMPW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(EQOSW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(NVJPGSWR, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SDMMCWAA, CGID_TAG_ADR),
	mc_make_txn_override_cfg(MPCOREW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SCEDMAW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SDMMCW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(AXISW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(TSECSWR, CGID_TAG_ADR),
	mc_make_txn_override_cfg(GPUSWR, CGID_TAG_ADR),
	mc_make_txn_override_cfg(XUSB_HOSTW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(TSECSWRB, CGID_TAG_ADR),
	mc_make_txn_override_cfg(GPUSWR2, CGID_TAG_ADR),
	mc_make_txn_override_cfg(AONDMAW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(AONW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SESWR, CGID_TAG_ADR),
	mc_make_txn_override_cfg(BPMPDMAW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SDMMCWA, CGID_TAG_ADR),
	mc_make_txn_override_cfg(HDAW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(NVDECSWR, CGID_TAG_ADR),
	mc_make_txn_override_cfg(UFSHCW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SATAW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(ETRW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(VICSWR, CGID_TAG_ADR),
	mc_make_txn_override_cfg(NVENCSWR, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SDMMCWAB, CGID_TAG_ADR),
	mc_make_txn_override_cfg(ISPWB, CGID_TAG_ADR),
	mc_make_txn_override_cfg(APEW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(XUSB_DEVW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(AFIW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SCEW, CGID_TAG_ADR),
};

static void tegra186_memctrl_reconfig_mss_clients(void)
{
#if ENABLE_ROC_FOR_ORDERING_CLIENT_REQUESTS
	uint32_t val, wdata_0, wdata_1;

	/*
	 * Assert Memory Controller's HOTRESET_FLUSH_ENABLE signal for
	 * boot and strongly ordered MSS clients to flush existing memory
	 * traffic and stall future requests.
	 */
	val = tegra_mc_read_32(MC_CLIENT_HOTRESET_CTRL0);
	assert(val == MC_CLIENT_HOTRESET_CTRL0_RESET_VAL);

	wdata_0 = MC_CLIENT_HOTRESET_CTRL0_HDA_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL0_AFI_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL0_SATA_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL0_XUSB_HOST_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL0_XUSB_DEV_FLUSH_ENB;
	tegra_mc_write_32(MC_CLIENT_HOTRESET_CTRL0, wdata_0);

	/* Wait for HOTRESET STATUS to indicate FLUSH_DONE */
	do {
		val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS0);
	} while ((val & wdata_0) != wdata_0);

	/* Wait one more time due to SW WAR for known legacy issue */
	do {
		val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS0);
	} while ((val & wdata_0) != wdata_0);

	val = tegra_mc_read_32(MC_CLIENT_HOTRESET_CTRL1);
	assert(val == MC_CLIENT_HOTRESET_CTRL1_RESET_VAL);

	wdata_1 = MC_CLIENT_HOTRESET_CTRL1_SDMMC4A_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_APE_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_SE_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_ETR_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_AXIS_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_EQOS_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_UFSHC_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_BPMP_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_AON_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_SCE_FLUSH_ENB;
	tegra_mc_write_32(MC_CLIENT_HOTRESET_CTRL1, wdata_1);

	/* Wait for HOTRESET STATUS to indicate FLUSH_DONE */
	do {
		val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS1);
	} while ((val & wdata_1) != wdata_1);

	/* Wait one more time due to SW WAR for known legacy issue */
	do {
		val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS1);
	} while ((val & wdata_1) != wdata_1);

	/*
	 * Change MEMTYPE_OVERRIDE from SO_DEV -> PASSTHRU for boot and
	 * strongly ordered MSS clients. ROC needs to be single point
	 * of control on overriding the memory type. So, remove TSA's
	 * memtype override.
	 *
	 * MC clients with default SO_DEV override still enabled at TSA:
	 * AONW, BPMPW, SCEW, APEW
	 */
	mc_set_tsa_passthrough(AFIW);
	mc_set_tsa_passthrough(HDAW);
	mc_set_tsa_passthrough(SATAW);
	mc_set_tsa_passthrough(XUSB_HOSTW);
	mc_set_tsa_passthrough(XUSB_DEVW);
	mc_set_tsa_passthrough(SDMMCWAB);
	mc_set_tsa_passthrough(APEDMAW);
	mc_set_tsa_passthrough(SESWR);
	mc_set_tsa_passthrough(ETRW);
	mc_set_tsa_passthrough(AXISW);
	mc_set_tsa_passthrough(EQOSW);
	mc_set_tsa_passthrough(UFSHCW);
	mc_set_tsa_passthrough(BPMPDMAW);
	mc_set_tsa_passthrough(AONDMAW);
	mc_set_tsa_passthrough(SCEDMAW);

	/* Parker has no IO Coherency support and need the following:
	 * Ordered MC Clients on Parker are AFI, EQOS, SATA, XUSB.
	 * ISO clients(DISP, VI, EQOS) should never snoop caches and
	 *     don't need ROC/PCFIFO ordering.
	 * ISO clients(EQOS) that need ordering should use PCFIFO ordering
	 *     and bypass ROC ordering by using FORCE_NON_COHERENT path.
	 * FORCE_NON_COHERENT/FORCE_COHERENT config take precedence
	 *     over SMMU attributes.
	 * Force all Normal memory transactions from ISO and non-ISO to be
	 *     non-coherent(bypass ROC, avoid cache snoop to avoid perf hit).
	 * Force the SO_DEV transactions from ordered ISO clients(EQOS) to
	 *     non-coherent path and enable MC PCFIFO interlock for ordering.
	 * Force the SO_DEV transactions from ordered non-ISO clients (PCIe,
	 *     XUSB, SATA) to coherent so that the transactions are
	 *     ordered by ROC.
	 * PCFIFO ensure write ordering.
	 * Read after Write ordering is maintained/enforced by MC clients.
	 * Clients that need PCIe type write ordering must
	 *     go through ROC ordering.
	 * Ordering enable for Read clients is not necessary.
	 * R5's and A9 would get necessary ordering from AXI and
	 *     don't need ROC ordering enable:
	 *     - MMIO ordering is through dev mapping and MMIO
	 *       accesses bypass SMMU.
	 *     - Normal memory is accessed through SMMU and ordering is
	 *       ensured by client and AXI.
	 *     - Ack point for Normal memory is WCAM in MC.
	 *     - MMIO's can be early acked and AXI ensures dev memory ordering,
	 *       Client ensures read/write direction change ordering.
	 *     - See Bug 200312466 for more details.
	 *
	 * CGID_TAG_ADR is only present from T186 A02. As this code is common
	 *    between A01 and A02, tegra_memctrl_set_overrides() programs
	 *    CGID_TAG_ADR for the necessary clients on A02.
	 */
	mc_set_txn_override(HDAR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(BPMPW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(PTCR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(NVDISPLAYR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(EQOSW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(NVJPGSWR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(ISPRA, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(SDMMCWAA, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(VICSRD, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(MPCOREW, CGID_TAG_DEFAULT, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
	mc_set_txn_override(GPUSRD, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(AXISR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(SCEDMAW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(SDMMCW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(EQOSR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	/* See bug 200131110 comment #35*/
	mc_set_txn_override(APEDMAR, CGID_TAG_CLIENT_AXI_ID, SO_DEV_CLIENT_AXI_ID, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(NVENCSRD, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(SDMMCRAB, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(VICSRD1, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(BPMPDMAR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(VIW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(SDMMCRAA, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(AXISW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(XUSB_DEVR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(UFSHCR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(TSECSWR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(GPUSWR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(SATAR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(XUSB_HOSTW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(TSECSWRB, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(GPUSRD2, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(SCEDMAR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(GPUSWR2, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(AONDMAW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	/* See bug 200131110 comment #35*/
	mc_set_txn_override(APEDMAW, CGID_TAG_CLIENT_AXI_ID, SO_DEV_CLIENT_AXI_ID, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(AONW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(HOST1XDMAR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(ETRR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(SESWR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(NVJPGSRD, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(NVDECSRD, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(TSECSRDB, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(BPMPDMAW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(APER, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(NVDECSRD1, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(XUSB_HOSTR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(ISPWA, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(SESRD, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(SCER, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(AONR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(MPCORER, CGID_TAG_DEFAULT, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
	mc_set_txn_override(SDMMCWA, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(HDAW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(NVDECSWR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(UFSHCW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(AONDMAR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(SATAW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(ETRW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(VICSWR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(NVENCSWR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	/* See bug 200131110 comment #35 */
	mc_set_txn_override(AFIR, CGID_TAG_DEFAULT, SO_DEV_CLIENT_AXI_ID, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(SDMMCWAB, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(SDMMCRA, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(NVDISPLAYR1, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(ISPWB, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(BPMPR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(APEW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(SDMMCR, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(XUSB_DEVW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(TSECSRD, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	/*
	 * See bug 200131110 comment #35 - there are no normal requests
	 * and AWID for SO/DEV requests is hardcoded in RTL for a
	 * particular PCIE controller
	 */
	mc_set_txn_override(AFIW, CGID_TAG_DEFAULT, SO_DEV_CLIENT_AXI_ID, FORCE_NON_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(SCEW, CGID_TAG_DEFAULT, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);

	/*
	 * At this point, ordering can occur at ROC. So, remove PCFIFO's
	 * control over ordering requests.
	 *
	 * Change PCFIFO_*_ORDERED_CLIENT from ORDERED -> UNORDERED for
	 * boot and strongly ordered MSS clients
	 */
	val = MC_PCFIFO_CLIENT_CONFIG1_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(1, AFIW) &
		mc_set_pcfifo_unordered_boot_so_mss(1, HDAW) &
		mc_set_pcfifo_unordered_boot_so_mss(1, SATAW);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG1, val);

	val = MC_PCFIFO_CLIENT_CONFIG2_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(2, XUSB_HOSTW) &
		mc_set_pcfifo_unordered_boot_so_mss(2, XUSB_DEVW);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG2, val);

	val = MC_PCFIFO_CLIENT_CONFIG3_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(3, SDMMCWAB);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG3, val);

	val = MC_PCFIFO_CLIENT_CONFIG4_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(4, SESWR) &
		mc_set_pcfifo_unordered_boot_so_mss(4, ETRW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, AXISW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, UFSHCW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, BPMPDMAW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, AONDMAW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, SCEDMAW);
	/* EQOSW is the only client that has PCFIFO order enabled. */
	val |= mc_set_pcfifo_ordered_boot_so_mss(4, EQOSW);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG4, val);

	val = MC_PCFIFO_CLIENT_CONFIG5_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(5, APEDMAW);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG5, val);

	/*
	 * Deassert HOTRESET FLUSH_ENABLE for boot and strongly ordered MSS
	 * clients to allow memory traffic from all clients to start passing
	 * through ROC
	 */
	val = tegra_mc_read_32(MC_CLIENT_HOTRESET_CTRL0);
	assert(val == wdata_0);

	wdata_0 = MC_CLIENT_HOTRESET_CTRL0_RESET_VAL;
	tegra_mc_write_32(MC_CLIENT_HOTRESET_CTRL0, wdata_0);

	val = tegra_mc_read_32(MC_CLIENT_HOTRESET_CTRL1);
	assert(val == wdata_1);

	wdata_1 = MC_CLIENT_HOTRESET_CTRL1_RESET_VAL;
	tegra_mc_write_32(MC_CLIENT_HOTRESET_CTRL1, wdata_1);

#endif
}

static void tegra186_memctrl_set_overrides(void)
{
	uint32_t i, val;

	/*
	 * Set the MC_TXN_OVERRIDE registers for write clients.
	 */
	if ((tegra_chipid_is_t186()) &&
	    (!tegra_platform_is_silicon() ||
	    (tegra_platform_is_silicon() && (tegra_get_chipid_minor() == 1U)))) {

		/*
		 * GPU and NVENC settings for Tegra186 simulation and
		 * Silicon rev. A01
		 */
		val = tegra_mc_read_32(MC_TXN_OVERRIDE_CONFIG_GPUSWR);
		val &= (uint32_t)~MC_TXN_OVERRIDE_CGID_TAG_MASK;
		tegra_mc_write_32(MC_TXN_OVERRIDE_CONFIG_GPUSWR,
			val | MC_TXN_OVERRIDE_CGID_TAG_ZERO);

		val = tegra_mc_read_32(MC_TXN_OVERRIDE_CONFIG_GPUSWR2);
		val &= (uint32_t)~MC_TXN_OVERRIDE_CGID_TAG_MASK;
		tegra_mc_write_32(MC_TXN_OVERRIDE_CONFIG_GPUSWR2,
			val | MC_TXN_OVERRIDE_CGID_TAG_ZERO);

		val = tegra_mc_read_32(MC_TXN_OVERRIDE_CONFIG_NVENCSWR);
		val &= (uint32_t)~MC_TXN_OVERRIDE_CGID_TAG_MASK;
		tegra_mc_write_32(MC_TXN_OVERRIDE_CONFIG_NVENCSWR,
			val | MC_TXN_OVERRIDE_CGID_TAG_CLIENT_AXI_ID);

	} else {

		/*
		 * Settings for Tegra186 silicon rev. A02 and onwards.
		 */
		for (i = 0; i < ARRAY_SIZE(tegra186_txn_override_cfgs); i++) {
			val = tegra_mc_read_32(tegra186_txn_override_cfgs[i].offset);
			val &= (uint32_t)~MC_TXN_OVERRIDE_CGID_TAG_MASK;
			tegra_mc_write_32(tegra186_txn_override_cfgs[i].offset,
				val | tegra186_txn_override_cfgs[i].cgid_tag);
		}
	}
}


/*******************************************************************************
 * Array to hold MC context for Tegra186
 ******************************************************************************/
static __attribute__((aligned(16))) mc_regs_t tegra186_mc_context[] = {
	_START_OF_TABLE_,
	mc_make_sid_security_cfg(SCEW),
	mc_make_sid_security_cfg(AFIR),
	mc_make_sid_security_cfg(NVDISPLAYR1),
	mc_make_sid_security_cfg(XUSB_DEVR),
	mc_make_sid_security_cfg(VICSRD1),
	mc_make_sid_security_cfg(NVENCSWR),
	mc_make_sid_security_cfg(TSECSRDB),
	mc_make_sid_security_cfg(AXISW),
	mc_make_sid_security_cfg(SDMMCWAB),
	mc_make_sid_security_cfg(AONDMAW),
	mc_make_sid_security_cfg(GPUSWR2),
	mc_make_sid_security_cfg(SATAW),
	mc_make_sid_security_cfg(UFSHCW),
	mc_make_sid_security_cfg(AFIW),
	mc_make_sid_security_cfg(SDMMCR),
	mc_make_sid_security_cfg(SCEDMAW),
	mc_make_sid_security_cfg(UFSHCR),
	mc_make_sid_security_cfg(SDMMCWAA),
	mc_make_sid_security_cfg(APEDMAW),
	mc_make_sid_security_cfg(SESWR),
	mc_make_sid_security_cfg(MPCORER),
	mc_make_sid_security_cfg(PTCR),
	mc_make_sid_security_cfg(BPMPW),
	mc_make_sid_security_cfg(ETRW),
	mc_make_sid_security_cfg(GPUSRD),
	mc_make_sid_security_cfg(VICSWR),
	mc_make_sid_security_cfg(SCEDMAR),
	mc_make_sid_security_cfg(HDAW),
	mc_make_sid_security_cfg(ISPWA),
	mc_make_sid_security_cfg(EQOSW),
	mc_make_sid_security_cfg(XUSB_HOSTW),
	mc_make_sid_security_cfg(TSECSWR),
	mc_make_sid_security_cfg(SDMMCRAA),
	mc_make_sid_security_cfg(APER),
	mc_make_sid_security_cfg(VIW),
	mc_make_sid_security_cfg(APEW),
	mc_make_sid_security_cfg(AXISR),
	mc_make_sid_security_cfg(SDMMCW),
	mc_make_sid_security_cfg(BPMPDMAW),
	mc_make_sid_security_cfg(ISPRA),
	mc_make_sid_security_cfg(NVDECSWR),
	mc_make_sid_security_cfg(XUSB_DEVW),
	mc_make_sid_security_cfg(NVDECSRD),
	mc_make_sid_security_cfg(MPCOREW),
	mc_make_sid_security_cfg(NVDISPLAYR),
	mc_make_sid_security_cfg(BPMPDMAR),
	mc_make_sid_security_cfg(NVJPGSWR),
	mc_make_sid_security_cfg(NVDECSRD1),
	mc_make_sid_security_cfg(TSECSRD),
	mc_make_sid_security_cfg(NVJPGSRD),
	mc_make_sid_security_cfg(SDMMCWA),
	mc_make_sid_security_cfg(SCER),
	mc_make_sid_security_cfg(XUSB_HOSTR),
	mc_make_sid_security_cfg(VICSRD),
	mc_make_sid_security_cfg(AONDMAR),
	mc_make_sid_security_cfg(AONW),
	mc_make_sid_security_cfg(SDMMCRA),
	mc_make_sid_security_cfg(HOST1XDMAR),
	mc_make_sid_security_cfg(EQOSR),
	mc_make_sid_security_cfg(SATAR),
	mc_make_sid_security_cfg(BPMPR),
	mc_make_sid_security_cfg(HDAR),
	mc_make_sid_security_cfg(SDMMCRAB),
	mc_make_sid_security_cfg(ETRR),
	mc_make_sid_security_cfg(AONR),
	mc_make_sid_security_cfg(APEDMAR),
	mc_make_sid_security_cfg(SESRD),
	mc_make_sid_security_cfg(NVENCSRD),
	mc_make_sid_security_cfg(GPUSWR),
	mc_make_sid_security_cfg(TSECSWRB),
	mc_make_sid_security_cfg(ISPWB),
	mc_make_sid_security_cfg(GPUSRD2),
	mc_make_sid_override_cfg(APER),
	mc_make_sid_override_cfg(VICSRD),
	mc_make_sid_override_cfg(NVENCSRD),
	mc_make_sid_override_cfg(NVJPGSWR),
	mc_make_sid_override_cfg(AONW),
	mc_make_sid_override_cfg(BPMPR),
	mc_make_sid_override_cfg(BPMPW),
	mc_make_sid_override_cfg(HDAW),
	mc_make_sid_override_cfg(NVDISPLAYR1),
	mc_make_sid_override_cfg(APEDMAR),
	mc_make_sid_override_cfg(AFIR),
	mc_make_sid_override_cfg(AXISR),
	mc_make_sid_override_cfg(VICSRD1),
	mc_make_sid_override_cfg(TSECSRD),
	mc_make_sid_override_cfg(BPMPDMAW),
	mc_make_sid_override_cfg(MPCOREW),
	mc_make_sid_override_cfg(XUSB_HOSTR),
	mc_make_sid_override_cfg(GPUSWR),
	mc_make_sid_override_cfg(XUSB_DEVR),
	mc_make_sid_override_cfg(UFSHCW),
	mc_make_sid_override_cfg(XUSB_HOSTW),
	mc_make_sid_override_cfg(SDMMCWAB),
	mc_make_sid_override_cfg(SATAW),
	mc_make_sid_override_cfg(SCEDMAR),
	mc_make_sid_override_cfg(HOST1XDMAR),
	mc_make_sid_override_cfg(SDMMCWA),
	mc_make_sid_override_cfg(APEDMAW),
	mc_make_sid_override_cfg(SESWR),
	mc_make_sid_override_cfg(AXISW),
	mc_make_sid_override_cfg(AONDMAW),
	mc_make_sid_override_cfg(TSECSWRB),
	mc_make_sid_override_cfg(MPCORER),
	mc_make_sid_override_cfg(ISPWB),
	mc_make_sid_override_cfg(AONR),
	mc_make_sid_override_cfg(BPMPDMAR),
	mc_make_sid_override_cfg(HDAR),
	mc_make_sid_override_cfg(SDMMCRA),
	mc_make_sid_override_cfg(ETRW),
	mc_make_sid_override_cfg(GPUSWR2),
	mc_make_sid_override_cfg(EQOSR),
	mc_make_sid_override_cfg(TSECSWR),
	mc_make_sid_override_cfg(ETRR),
	mc_make_sid_override_cfg(NVDECSRD),
	mc_make_sid_override_cfg(TSECSRDB),
	mc_make_sid_override_cfg(SDMMCRAA),
	mc_make_sid_override_cfg(NVDECSRD1),
	mc_make_sid_override_cfg(SDMMCR),
	mc_make_sid_override_cfg(NVJPGSRD),
	mc_make_sid_override_cfg(SCEDMAW),
	mc_make_sid_override_cfg(SDMMCWAA),
	mc_make_sid_override_cfg(APEW),
	mc_make_sid_override_cfg(AONDMAR),
	mc_make_sid_override_cfg(PTCR),
	mc_make_sid_override_cfg(SCER),
	mc_make_sid_override_cfg(ISPRA),
	mc_make_sid_override_cfg(ISPWA),
	mc_make_sid_override_cfg(VICSWR),
	mc_make_sid_override_cfg(SESRD),
	mc_make_sid_override_cfg(SDMMCW),
	mc_make_sid_override_cfg(SDMMCRAB),
	mc_make_sid_override_cfg(EQOSW),
	mc_make_sid_override_cfg(GPUSRD2),
	mc_make_sid_override_cfg(SCEW),
	mc_make_sid_override_cfg(GPUSRD),
	mc_make_sid_override_cfg(NVDECSWR),
	mc_make_sid_override_cfg(XUSB_DEVW),
	mc_make_sid_override_cfg(SATAR),
	mc_make_sid_override_cfg(NVDISPLAYR),
	mc_make_sid_override_cfg(VIW),
	mc_make_sid_override_cfg(UFSHCR),
	mc_make_sid_override_cfg(NVENCSWR),
	mc_make_sid_override_cfg(AFIW),
	mc_smmu_bypass_cfg,	/* TBU settings */
	_END_OF_TABLE_,
};

/*******************************************************************************
 * Handler to return the pointer to the MC's context struct
 ******************************************************************************/
mc_regs_t *plat_memctrl_get_sys_suspend_ctx(void)
{
	/* index of _END_OF_TABLE_ */
	tegra186_mc_context[0].val = (uint32_t)(ARRAY_SIZE(tegra186_mc_context)) - 1U;

	return tegra186_mc_context;
}

void plat_memctrl_setup(void)
{
	uint32_t val;
	unsigned int i;

	/* Program all the Stream ID overrides */
	for (i = 0U; i < ARRAY_SIZE(tegra186_streamid_override_regs); i++) {
		tegra_mc_streamid_write_32(tegra186_streamid_override_regs[i],
			MC_STREAM_ID_MAX);
	}

	/* Program the security config settings for all Stream IDs */
	for (i = 0U; i < ARRAY_SIZE(tegra186_streamid_sec_cfgs); i++) {
		val = (tegra186_streamid_sec_cfgs[i].override_enable << 16) |
		      (tegra186_streamid_sec_cfgs[i].override_client_inputs << 8) |
		      (tegra186_streamid_sec_cfgs[i].override_client_ns_flag << 0);
		tegra_mc_streamid_write_32(tegra186_streamid_sec_cfgs[i].offset, val);
	}

	/*
	 * Re-configure MSS to allow ROC to deal with ordering of the
	 * Memory Controller traffic. This is needed as the Memory Controller
	 * boots with MSS having all control, but ROC provides a performance
	 * boost as compared to MSS.
	 */
	tegra186_memctrl_reconfig_mss_clients();

	/* Program overrides for MC transactions */
	tegra186_memctrl_set_overrides();
}

/*******************************************************************************
 * Handler to restore platform specific settings to the memory controller
 ******************************************************************************/
void plat_memctrl_restore(void)
{
	/*
	 * Re-configure MSS to allow ROC to deal with ordering of the
	 * Memory Controller traffic. This is needed as the Memory Controller
	 * boots with MSS having all control, but ROC provides a performance
	 * boost as compared to MSS.
	 */
	tegra186_memctrl_reconfig_mss_clients();

	/* Program overrides for MC transactions */
	tegra186_memctrl_set_overrides();
}

/*******************************************************************************
 * Handler to program the scratch registers with TZDRAM settings for the
 * resume firmware
 ******************************************************************************/
void plat_memctrl_tzdram_setup(uint64_t phys_base, uint64_t size_in_bytes)
{
	uint32_t val;

	/*
	 * Setup the Memory controller to allow only secure accesses to
	 * the TZDRAM carveout
	 */
	INFO("Configuring TrustZone DRAM Memory Carveout\n");

	tegra_mc_write_32(MC_SECURITY_CFG0_0, (uint32_t)phys_base);
	tegra_mc_write_32(MC_SECURITY_CFG3_0, (uint32_t)(phys_base >> 32));
	tegra_mc_write_32(MC_SECURITY_CFG1_0, size_in_bytes >> 20);

	/*
	 * When TZ encryption is enabled, we need to setup TZDRAM
	 * before CPU accesses TZ Carveout, else CPU will fetch
	 * non-decrypted data. So save TZDRAM setting for SC7 resume
	 * FW to restore.
	 *
	 * Scratch registers map:
	 *  RSV55_0 = CFG1[12:0] | CFG0[31:20]
	 *  RSV55_1 = CFG3[1:0]
	 */
	val = tegra_mc_read_32(MC_SECURITY_CFG1_0) & MC_SECURITY_SIZE_MB_MASK;
	val |= tegra_mc_read_32(MC_SECURITY_CFG0_0) & MC_SECURITY_BOM_MASK;
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_TZDRAM_ADDR_LO, val);

	val = tegra_mc_read_32(MC_SECURITY_CFG3_0) & MC_SECURITY_BOM_HI_MASK;
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_TZDRAM_ADDR_HI, val);

	/*
	 * MCE propagates the security configuration values across the
	 * CCPLEX.
	 */
	(void)mce_update_gsc_tzdram();
}
