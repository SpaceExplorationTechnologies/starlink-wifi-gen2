#include <os/rt_linux_txrx_hook.h>

struct mt_wlan_hook_ctrl wlan_hook;

/*
*
*/
void mt_wlan_hook_init(void)
{
	os_zero_mem(&wlan_hook, sizeof(struct mt_wlan_hook_ctrl));
	DlListInit(&wlan_hook.hook_head);
}


/*
*
*/
int mt_wlan_hook_call(unsigned short hook, void *ad, void *priv)
{
	struct mt_wlan_hook_ops *cur;

	DlListForEach(cur, &wlan_hook.hook_head, struct mt_wlan_hook_ops, list)

	if (cur->hooks & 1 << hook)
		cur->fun(hook, ad, priv);

	return 0;
}


/*
*
*/
int mt_wlan_hook_register(struct mt_wlan_hook_ops *ops)
{
	struct mt_wlan_hook_ops *next = NULL;
	struct mt_wlan_hook_ops *cur = NULL;

	if (!ops)
		return -1;

	DlListForEachSafe(cur, next, &wlan_hook.hook_head,
					  struct mt_wlan_hook_ops, list)

	if (cur && ops->priority > cur->priority) {
		(ops->list).Next = &next->list;
		cur->list.Next = &ops->list;
		return 0;
	}

	/*add first entry*/
	DlListAdd(&wlan_hook.hook_head, &ops->list);
	return 0;
}
#ifndef MT76XX_COMBO_DUAL_DRIVER_SUPPORT
EXPORT_SYMBOL(mt_wlan_hook_register);
#endif

/*
*
*/
int mt_wlan_hook_unregister(struct mt_wlan_hook_ops *ops)
{
	struct mt_wlan_hook_ops *next, *cur;

	if (!ops)
		return -1;

	DlListForEachSafe(cur, next, &wlan_hook.hook_head,
					  struct mt_wlan_hook_ops, list)

	if (ops == cur) {
		DlListDel(&ops->list);
		return 0;
	}

	return -1;
}
#ifndef MT76XX_COMBO_DUAL_DRIVER_SUPPORT
EXPORT_SYMBOL(mt_wlan_hook_unregister);
#endif