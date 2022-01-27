#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/debugfs.h>
#include <linux/arm-smccc.h>

static u64 a[5], r[4], issue;
static struct arm_smccc_res res;
static struct dentry *mtk_smc_dir, *parameters_dir, *results_dir;

static int mtk_smc_issue(void)
{
	arm_smccc_smc(a[0], a[1], a[2], a[3], a[4], 0x0, 0x0, 0x0, &res);

	r[0] = res.a0;
	r[1] = res.a1;
	r[2] = res.a2;
	r[3] = res.a3;

	return 0;
}

static int mtk_smc_set_issue(void *data, u64 val)
{
	*((u64 *)data) = 0;

	/* echo 1 > /sys/kernel/debug/mtk-smc/issue */
	if (val == 1)
		if (!mtk_smc_issue())
			*((u64 *)data) = 1;

	return 0;
}

static int mtk_smc_get_issue(void *data, u64 *val)
{
	*val = *((u64 *)data);

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(mtk_smc_issue_fops, mtk_smc_get_issue,
			mtk_smc_set_issue, "%llu\n");

static int __init mtk_smc_init(void)
{
	u8 i;
	char file_name[8];

	/* /sys/kernel/debug/mtk-smc/ */
	mtk_smc_dir = debugfs_create_dir("mtk-smc", NULL);
	if (!mtk_smc_dir)
		goto fail;

	/* /sys/kernel/debug/mtk-smc/parameters/ */
	parameters_dir = debugfs_create_dir("parameters", mtk_smc_dir);
	if (!parameters_dir)
		goto fail;

	/* /sys/kernel/debug/mtk-smc/parameters/aX */
	for (i = 0; i < ARRAY_SIZE(a); i++) {
		snprintf(file_name, sizeof(file_name), "a%d", i);
		if (!debugfs_create_u64(file_name, 0644,
					parameters_dir, (u64 *)&a[i]))
			goto fail;
	}

	/* /sys/kernel/debug/mtk-smc/results/ */
	results_dir = debugfs_create_dir("results", mtk_smc_dir);
	if (!results_dir)
		goto fail;

	/* /sys/kernel/debug/mtk-smc/results/rX */
	for (i = 0; i < ARRAY_SIZE(r); i++) {
		snprintf(file_name, sizeof(file_name), "r%d", i);
		if (!debugfs_create_u64(file_name, 0644,
					results_dir, (u64 *)&r[i]))
			goto fail;
	}

	/* /sys/kernel/debug/mtk-smc/issue */
	if (!debugfs_create_file("issue", 0644, mtk_smc_dir,
				 &issue, &mtk_smc_issue_fops))
		goto fail;

	return 0;

fail:
	debugfs_remove_recursive(mtk_smc_dir);
	mtk_smc_dir = NULL;
	parameters_dir = NULL;
	results_dir = NULL;
	return -ENOENT;
}

static void __exit mtk_smc_exit(void)
{
	debugfs_remove_recursive(mtk_smc_dir);
	mtk_smc_dir = NULL;
	parameters_dir = NULL;
	results_dir = NULL;
}

module_init(mtk_smc_init);
module_exit(mtk_smc_exit);

MODULE_DESCRIPTION("Simple SMC driver");
MODULE_LICENSE("GPL");
