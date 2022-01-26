// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#include <common.h>
#include <autoboot.h>
#include <bootstage.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <env.h>
#include <init.h>
#include <net.h>
#include <version.h>
#include <efi_loader.h>
#include <spacex_v2.icc.c>

static void run_preboot_environment_command(void)
{
	char *p;

	p = env_get("preboot");
	if (p != NULL) {
		int prev = 0;

		if (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(p, -1, 0);

		if (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}
}

/* We come here after U-Boot is initialised and ready to process commands */
void main_loop(void)
{
	const char *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

	if (IS_ENABLED(CONFIG_VERSION_VARIABLE))
		env_set("ver", version_string);  /* set version variable */

	cli_init();

	if (IS_ENABLED(CONFIG_USE_PREBOOT))
		run_preboot_environment_command();

	if (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, NULL, NULL);

	if (IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK_EARLY))
		efi_launch_capsules();

	// SpaceX: Reset peripherals. Find a better home for this some day.
	spacex_v2_reset();

	// SpaceX: Moved the if (secureboot check) before bootdelay_process
	// so the bootmenu doesn't show up when the board is fused.
	if (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);
	s = bootdelay_process();

	autoboot_command(s);

	cli_loop();
	panic("No CLI available");
}
