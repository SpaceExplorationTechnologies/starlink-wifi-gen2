/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/cci.h>
#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <plat_imx8.h>
#include <sci/sci.h>

#include "../../common/sci/imx8_mu.h"

#define CORE_PWR_STATE(state) \
	((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) \
	((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) \
	((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

const static int ap_core_index[PLATFORM_CORE_COUNT] = {
	SC_R_A53_0, SC_R_A53_1, SC_R_A53_2,
	SC_R_A53_3, SC_R_A72_0, SC_R_A72_1,
};

/* save gic dist/redist context when GIC is poewr down */
static struct plat_gic_ctx imx_gicv3_ctx;
static unsigned int gpt_lpcg, gpt_reg[2];

static void imx_enable_irqstr_wakeup(void)
{
	uint32_t irq_mask;
	gicv3_dist_ctx_t *dist_ctx = &imx_gicv3_ctx.dist_ctx;

	/* put IRQSTR into ON mode */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_IRQSTR_SCU2, SC_PM_PW_MODE_ON);

	/* enable the irqsteer to handle wakeup irq */
	mmio_write_32(IMX_WUP_IRQSTR_BASE, 0x1);
	for (int i = 0; i < 15; i++) {
		irq_mask = dist_ctx->gicd_isenabler[i];
		mmio_write_32(IMX_WUP_IRQSTR_BASE + 0x3c - 0x4 * i, irq_mask);
	}

	/* set IRQSTR low power mode */
	if (imx_is_wakeup_src_irqsteer())
		sc_pm_set_resource_power_mode(ipc_handle, SC_R_IRQSTR_SCU2, SC_PM_PW_MODE_STBY);
	else
		sc_pm_set_resource_power_mode(ipc_handle, SC_R_IRQSTR_SCU2, SC_PM_PW_MODE_OFF);
}

static void imx_disable_irqstr_wakeup(void)
{
	/* put IRQSTR into ON from STBY mode */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_IRQSTR_SCU2, SC_PM_PW_MODE_ON);

	/* disable the irqsteer */
	mmio_write_32(IMX_WUP_IRQSTR_BASE, 0x0);
	for (int i = 0; i < 16; i++)
		mmio_write_32(IMX_WUP_IRQSTR_BASE + 0x4 + 0x4 * i, 0x0);

	/* put IRQSTR into OFF mode */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_IRQSTR_SCU2, SC_PM_PW_MODE_OFF);
}

int imx_pwr_domain_on(u_register_t mpidr)
{
	int ret = PSCI_E_SUCCESS;
	unsigned int cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	sc_pm_set_resource_power_mode(ipc_handle, cluster_id == 0 ?
		SC_R_A53 : SC_R_A72, SC_PM_PW_MODE_ON);

	if (cluster_id == 1)
		sc_pm_req_low_power_mode(ipc_handle, SC_R_A72, SC_PM_PW_MODE_ON);

	if (sc_pm_set_resource_power_mode(ipc_handle,
		ap_core_index[cpu_id + PLATFORM_CLUSTER0_CORE_COUNT * cluster_id],
		SC_PM_PW_MODE_ON) != SC_ERR_NONE) {
		ERROR("core %d power on failed!\n", cpu_id + PLATFORM_CLUSTER0_CORE_COUNT * cluster_id);
		ret = PSCI_E_INTERN_FAIL;
	}

	if (sc_pm_cpu_start(ipc_handle,
		ap_core_index[cpu_id + PLATFORM_CLUSTER0_CORE_COUNT * cluster_id],
		true, BL31_BASE) != SC_ERR_NONE) {
		ERROR("boot core %d failed!\n", cpu_id + PLATFORM_CLUSTER0_CORE_COUNT * cluster_id);
		ret = PSCI_E_INTERN_FAIL;
	}

	return ret;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();

	if (CLUSTER_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE)
		cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));

	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	plat_gic_cpuif_disable();
	sc_pm_req_cpu_low_power_mode(ipc_handle,
		ap_core_index[cpu_id + PLATFORM_CLUSTER0_CORE_COUNT * cluster_id],
		SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_NONE);

	if (is_local_state_off(CLUSTER_PWR_STATE(target_state))) {
		cci_disable_snoop_dvm_reqs(cluster_id);
		if (cluster_id == 1)
			sc_pm_req_low_power_mode(ipc_handle, SC_R_A72, SC_PM_PW_MODE_OFF);
	}
	printf("turn off cluster:%d core:%d\n", cluster_id, cpu_id);
}

void imx_domain_suspend(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		plat_gic_cpuif_disable();
		sc_pm_set_cpu_resume(ipc_handle,
			ap_core_index[cpu_id + PLATFORM_CLUSTER0_CORE_COUNT * cluster_id],
			true, BL31_BASE);
		sc_pm_req_cpu_low_power_mode(ipc_handle,
			ap_core_index[cpu_id + PLATFORM_CLUSTER0_CORE_COUNT * cluster_id],
			SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_GIC);
	} else {
		dsb();
		write_scr_el3(read_scr_el3() | SCR_FIQ_BIT);
		isb();
	}

	if (is_local_state_off(CLUSTER_PWR_STATE(target_state))) {
		cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));
		if (cluster_id == 1)
			sc_pm_req_low_power_mode(ipc_handle, SC_R_A72, SC_PM_PW_MODE_OFF);
	}

	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		plat_gic_cpuif_disable();

		/* save gic context */
		plat_gic_save(cpu_id, &imx_gicv3_ctx);
		/* enable the irqsteer for wakeup */
		imx_enable_irqstr_wakeup();

		cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));

		/* Put GIC in LP mode. */
		sc_pm_set_resource_power_mode(ipc_handle, SC_R_GIC, SC_PM_PW_MODE_OFF);
		/* Save GPT clock and registers, then turn off its power */
		gpt_lpcg = mmio_read_32(IMX_GPT_LPCG_BASE);
		gpt_reg[0] = mmio_read_32(IMX_GPT_BASE);
		gpt_reg[1] = mmio_read_32(IMX_GPT_BASE + 0x4);
		sc_pm_set_resource_power_mode(ipc_handle, SC_R_GPT_0, SC_PM_PW_MODE_OFF);

		sc_pm_req_low_power_mode(ipc_handle, SC_R_A53, SC_PM_PW_MODE_OFF);
		sc_pm_req_low_power_mode(ipc_handle, SC_R_A72, SC_PM_PW_MODE_OFF);
		sc_pm_req_low_power_mode(ipc_handle, SC_R_CCI, SC_PM_PW_MODE_OFF);

		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53, SC_PM_SYS_IF_DDR,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72, SC_PM_SYS_IF_DDR,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53, SC_PM_SYS_IF_MU,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72, SC_PM_SYS_IF_MU,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53, SC_PM_SYS_IF_INTERCONNECT,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72, SC_PM_SYS_IF_INTERCONNECT,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF);
		sc_pm_req_low_power_mode(ipc_handle, SC_R_CCI, SC_PM_PW_MODE_OFF);

		sc_pm_set_cpu_resume(ipc_handle,
			ap_core_index[cpu_id + PLATFORM_CLUSTER0_CORE_COUNT * cluster_id],
			true, BL31_BASE);
		if (imx_is_wakeup_src_irqsteer())
			sc_pm_req_cpu_low_power_mode(ipc_handle,
				ap_core_index[cpu_id + PLATFORM_CLUSTER0_CORE_COUNT * cluster_id],
				SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_IRQSTEER);
		else
			sc_pm_req_cpu_low_power_mode(ipc_handle,
				ap_core_index[cpu_id + PLATFORM_CLUSTER0_CORE_COUNT * cluster_id],
				SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_SCU);
	}
}

void imx_domain_suspend_finish(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	/* check the system level status */
	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		MU_Resume(SC_IPC_BASE);

		sc_pm_req_cpu_low_power_mode(ipc_handle,
			ap_core_index[cpu_id + PLATFORM_CLUSTER0_CORE_COUNT * cluster_id],
			SC_PM_PW_MODE_ON, SC_PM_WAKE_SRC_GIC);

		/* Put GIC/IRQSTR back to high power mode. */
		sc_pm_set_resource_power_mode(ipc_handle, SC_R_GIC, SC_PM_PW_MODE_ON);

		/* Turn GPT power and restore its clock and registers */
		sc_pm_set_resource_power_mode(ipc_handle, SC_R_GPT_0, SC_PM_PW_MODE_ON);
		sc_pm_clock_enable(ipc_handle, SC_R_GPT_0, SC_PM_CLK_PER, true, 0);
		mmio_write_32(IMX_GPT_BASE, gpt_reg[0]);
		mmio_write_32(IMX_GPT_BASE + 0x4, gpt_reg[1]);
		mmio_write_32(IMX_GPT_LPCG_BASE, gpt_lpcg);

		sc_pm_req_low_power_mode(ipc_handle, SC_R_A53, SC_PM_PW_MODE_ON);
		sc_pm_req_low_power_mode(ipc_handle, SC_R_A72, SC_PM_PW_MODE_ON);
		sc_pm_req_low_power_mode(ipc_handle, SC_R_CCI, SC_PM_PW_MODE_ON);

		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53, SC_PM_SYS_IF_DDR,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72, SC_PM_SYS_IF_DDR,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53, SC_PM_SYS_IF_MU,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72, SC_PM_SYS_IF_MU,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53, SC_PM_SYS_IF_INTERCONNECT,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72, SC_PM_SYS_IF_INTERCONNECT,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
		sc_pm_req_low_power_mode(ipc_handle, SC_R_CCI, SC_PM_PW_MODE_ON);

		cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));

		/* restore gic context */
		plat_gic_restore(cpu_id, &imx_gicv3_ctx);
		/* disable the irqsteer wakeup */
		imx_disable_irqstr_wakeup();

		plat_gic_cpuif_enable();
	}

	/* check the cluster level power status */
	if (is_local_state_off(CLUSTER_PWR_STATE(target_state))) {
		cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));
		if (cluster_id == 1)
			sc_pm_req_low_power_mode(ipc_handle, SC_R_A72, SC_PM_PW_MODE_ON);
	}

	/* check the core level power status */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		sc_pm_set_cpu_resume(ipc_handle,
			ap_core_index[cpu_id + PLATFORM_CLUSTER0_CORE_COUNT * cluster_id],
			false, BL31_BASE);
		sc_pm_req_cpu_low_power_mode(ipc_handle,
			ap_core_index[cpu_id + PLATFORM_CLUSTER0_CORE_COUNT * cluster_id],
			SC_PM_PW_MODE_ON, SC_PM_WAKE_SRC_GIC);
		plat_gic_cpuif_enable();
	} else {
		write_scr_el3(read_scr_el3() & (~SCR_FIQ_BIT));
		isb();
	}
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	return PSCI_E_SUCCESS;
}

static const plat_psci_ops_t imx_plat_psci_ops = {
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.pwr_domain_off = imx_pwr_domain_off,
	.pwr_domain_suspend = imx_domain_suspend,
	.pwr_domain_suspend_finish = imx_domain_suspend_finish,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.validate_power_state = imx_validate_power_state,
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.system_off = imx_system_off,
	.system_reset = imx_system_reset,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	imx_mailbox_init(sec_entrypoint);
	*psci_ops = &imx_plat_psci_ops;

	/* make sure system sources power ON in low power mode by default */
	sc_pm_req_low_power_mode(ipc_handle, SC_R_A53, SC_PM_PW_MODE_ON);
	sc_pm_req_low_power_mode(ipc_handle, SC_R_A72, SC_PM_PW_MODE_ON);
	sc_pm_req_low_power_mode(ipc_handle, SC_R_CCI, SC_PM_PW_MODE_ON);

	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53, SC_PM_SYS_IF_DDR,
		SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72, SC_PM_SYS_IF_DDR,
		SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53, SC_PM_SYS_IF_MU,
		SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72, SC_PM_SYS_IF_MU,
		SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53, SC_PM_SYS_IF_INTERCONNECT,
		SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72, SC_PM_SYS_IF_INTERCONNECT,
		SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);

	return 0;
}
