/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rt_main_dev.c

    Abstract:
    Create and register network interface.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/


#define RTMP_MODULE_OS

#include "rt_config.h"
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"

#if (KERNEL_VERSION(2, 6, 24) <= LINUX_VERSION_CODE)
#ifndef SA_SHIRQ
#define SA_SHIRQ IRQF_SHARED
#endif
#endif

/*---------------------------------------------------------------------*/
/* Private Variables Used                                              */
/*---------------------------------------------------------------------*/

RTMP_STRING *mac = "";		/* default 00:00:00:00:00:00 */
RTMP_STRING *mode = "";		/* supported mode: normal/ate/monitor;  default: normal */
RTMP_STRING *hostname = "";	/* default CMPC */

#if (KERNEL_VERSION(2, 6, 12) >= LINUX_VERSION_CODE)
MODULE_PARM(mac, "s");
MODULE_PARM(mode, "s");
#else
module_param(mac, charp, 0);
module_param(mode, charp, 0);
#endif
MODULE_PARM_DESC(mac, "rt_wifi: wireless mac addr");
MODULE_PARM_DESC(mode, "rt_wifi: wireless operation mode");

#if !defined(CONFIG_PROPRIETARY_DRIVER) || defined(CONFIG_DBG_OOM)
MODULE_LICENSE("GPL");
#else
MODULE_LICENSE("Proprietary");
#endif

#ifdef OS_ABL_SUPPORT
RTMP_DRV_ABL_OPS RtmpDrvOps, *pRtmpDrvOps = &RtmpDrvOps;
RTMP_NET_ABL_OPS RtmpDrvNetOps, *pRtmpDrvNetOps = &RtmpDrvNetOps;
#endif /* OS_ABL_SUPPORT */


/*---------------------------------------------------------------------*/
/* Prototypes of Functions Used                                        */
/*---------------------------------------------------------------------*/

/* public function prototype */
int mt_wifi_close(VOID *net_dev);
int mt_wifi_open(VOID *net_dev);
int virtual_if_up_handler(VOID *dev);
int virtual_if_down_handler(VOID *dev);

/* private function prototype */
INT rt28xx_send_packets(IN struct sk_buff *skb_p, IN struct net_device *net_dev);

struct net_device_stats *RT28xx_get_ether_stats(struct net_device *net_dev);


/*
 * ========================================================================
 * Routine Description:
 *    Close raxx interface.
 *
 * Arguments:
 *	*net_dev			the raxx interface pointer
 *
 * Return Value:
 *    0					Open OK
 *	otherwise			Open Fail
 *
 * Note:
 *	1. if open fail, kernel will not call the close function.
 *	2. Free memory for
 *		(1) Mlme Memory Handler:		MlmeHalt()
 *		(2) TX & RX:					RTMPFreeTxRxRingMemory()
 *		(3) BA Reordering:				ba_reordering_resource_release()
 * ========================================================================
 */
int main_virtual_if_close(IN struct net_device *net_dev)
{
	VOID *pAd = NULL;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(net_dev), __func__));

	pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);

	if (pAd == NULL)
		return 0;

	RTMP_OS_NETDEV_CARRIER_OFF(net_dev);
	RTMP_OS_NETDEV_STOP_QUEUE(net_dev);

#ifdef IFUP_IN_PROBE
#else
	VIRTUAL_IF_DOWN(pAd, net_dev);
#endif /* IFUP_IN_PROBE */

	VIRTUAL_IF_DEINIT(pAd, net_dev);

	RT_MOD_HNAT_DEREG(net_dev);
	RT_MOD_DEC_USE_COUNT();
	return 0; /* close ok */
}

/*
 * ========================================================================
 * Routine Description:
 *   Open raxx interface.
 *
 * Arguments:
 *	*net_dev			the raxx interface pointer
 *
 * Return Value:
 *   0					Open OK
 *	otherwise			Open Fail
 *
 * Note:
 *	1. if open fail, kernel will not call the close function.
 *	2. Free memory for
 *		(1) Mlme Memory Handler:		MlmeHalt()
 *		(2) TX & RX:					RTMPFreeTxRxRingMemory()
 *		(3) BA Reordering:				ba_reordering_resource_release()
 * ========================================================================
 */
int main_virtual_if_open(struct net_device *net_dev)
{
	VOID *pAd = NULL;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(net_dev), __func__));

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL)
		return 0;

#ifdef IFUP_IN_PROBE

	while (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) != NDIS_STATUS_SUCCESS) {
		OS_WAIT(10);
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Card not ready, NDIS_STATUS_SUCCESS!\n"));
	}

#else
	if (VIRTUAL_IF_INIT(pAd, net_dev) != 0)
		return -1;

	if (VIRTUAL_IF_UP(pAd, net_dev) != 0)
		return -1;

#endif /* IFUP_IN_PROBE */
	RT_MOD_INC_USE_COUNT();
	RT_MOD_HNAT_REG(net_dev);
	netif_start_queue(net_dev);
	netif_carrier_on(net_dev);
	netif_wake_queue(net_dev);
	return 0;
}


/*
 * ========================================================================
 * Routine Description:
 *   Close raxx interface.
 *
 * Arguments:
 *	*net_dev			the raxx interface pointer
 *
 * Return Value:
 *    0					Open OK
 *	otherwise			Open Fail
 *
 * Note:
 *	1. if open fail, kernel will not call the close function.
 *	2. Free memory for
 *		(1) Mlme Memory Handler:		MlmeHalt()
 *		(2) TX & RX:					RTMPFreeTxRxRingMemory()
 *		(3) BA Reordering:				ba_reordering_resource_release()
 * ========================================================================
 */
int mt_wifi_close(VOID *dev)
{
	struct net_device *net_dev = (struct net_device *)dev;
	VOID	*pAd = NULL;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> mt_wifi_close\n"));
	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL)
		return 0;

#ifdef WIFI_DIAG
	DiagProcExit(pAd);
#endif
	RTMPDrvClose(pAd, net_dev);
	/*system down hook point*/
	WLAN_HOOK_CALL(WLAN_HOOK_SYS_DOWN, pAd, NULL);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<=== mt_wifi_close\n"));
	return 0;
}

#ifdef BB_SOC
#ifdef TCSUPPORT_WLAN_SW_RPS
extern int (*ecnt_wifi_rx_rps_hook)(struct sk_buff *skb);
extern int ecnt_wifi_rx_rps(struct sk_buff *skb);
#endif
#endif


/*
 * ========================================================================
 * Routine Description:
 *   Open raxx interface.
 *
 * Arguments:
 *	*net_dev			the raxx interface pointer
 *
 * Return Value:
 *    0					Open OK
 *	otherwise			Open Fail
 * ========================================================================
 */
int mt_wifi_open(VOID *dev)
{
	struct net_device *net_dev = (struct net_device *)dev;
	VOID *pAd = NULL;
	int retval = 0;
	ULONG OpMode;

	if (sizeof(ra_dma_addr_t) < sizeof(dma_addr_t))
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Fatal error for DMA address size!!!\n"));

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL) {
		/* if 1st open fail, pAd will be free; */
		/*   So the net_dev->priv will be NULL in 2rd open */
		return -1;
	}

	RTMP_DRIVER_MCU_SLEEP_CLEAR(pAd);
	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);
#if WIRELESS_EXT >= 12

	/*	if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_MAIN) */
	if (RTMP_DRIVER_MAIN_INF_CHECK(pAd, RT_DEV_PRIV_FLAGS_GET(net_dev)) == NDIS_STATUS_SUCCESS) {
#ifdef CONFIG_APSTA_MIXED_SUPPORT
#ifdef CONFIG_WIRELESS_EXT
		if (OpMode == OPMODE_AP)
			net_dev->wireless_handlers = (struct iw_handler_def *) &rt28xx_ap_iw_handler_def;
#endif /* CONFIG_WIRELESS_EXT */

#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_WIRELESS_EXT

		if (OpMode == OPMODE_STA)
			net_dev->wireless_handlers = (struct iw_handler_def *) &rt28xx_iw_handler_def;
#endif /* CONFIG_WIRELESS_EXT */
#endif /* CONFIG_STA_SUPPORT */
	}

#endif /* WIRELESS_EXT >= 12 */
	/* load_dev_l1profile should prior to RTMPPreReadProfile, for get_l2_profile() access data updated */
	if (load_dev_l1profile(pAd) == NDIS_STATUS_SUCCESS)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("load l1profile succeed!\n"));
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("load l1profile failed!\n"));
	/*system up hook point should before interrupt register*/
	WLAN_HOOK_CALL(WLAN_HOOK_SYS_UP, pAd, NULL);
	/*
	 *	Request interrupt service routine for PCI device
	 *	register the interrupt routine with the os
	 *
	 *	AP Channel auto-selection will be run in mt_wifi_init(),
	 *	so we must reqister IRQ hander here.
	 */
	RtmpOSIRQRequest(net_dev);
	/* Init IRQ parameters stored in pAd */
	/*	rtmp_irq_init(pAd); */
	RTMP_DRIVER_IRQ_INIT(pAd);

	/* Chip & other init */
	if (mt_wifi_init(pAd, mac, hostname) == FALSE)
		goto err;

#ifdef MBSS_SUPPORT
	/*
	 *	the function can not be moved to RT2860_probe() even register_netdev()
	 *	is changed as register_netdevice().
	 *	Or in some PC, kernel will panic (Fedora 4)
	 */
#if defined(P2P_APCLI_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
#else
	RT_CONFIG_IF_OPMODE_ON_AP(GET_OPMODE_FROM_PAD(pAd))
	{
	RT28xx_MBSS_Init(pAd, net_dev);
	}
#endif /* P2P_APCLI_SUPPORT || RT_CFG80211_P2P_SUPPORT || CFG80211_MULTI_STA */
#endif /* MBSS_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	{
		RT28xx_MSTA_Init(pAd, net_dev);
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef WDS_SUPPORT
	RT28xx_WDS_Init(pAd, net_dev);
#endif /* WDS_SUPPORT */
#ifdef SNIFFER_SUPPORT
	RT28xx_Monitor_Init(pAd, net_dev);
#endif /* SNIFFER_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
#ifdef CFG80211_MULTI_STA
	RTMP_CFG80211_MutliStaIf_Init(pAd);
#endif /* CFG80211_MULTI_STA */
#else
#endif /* RT_CFG80211_SUPPORT */
#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
	RTMP_DRIVER_CFG80211_START(pAd);
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */
	RTMPDrvOpen(pAd);
#ifdef VENDOR_FEATURE2_SUPPORT
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Number of Packet Allocated in open = %lu\n", OS_NumOfPktAlloc));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Number of Packet Freed in open = %lu\n", OS_NumOfPktFree));
#endif /* VENDOR_FEATURE2_SUPPORT */

#ifdef WIFI_DIAG
	DiagProcInit(pAd);
#endif

#ifdef BB_SOC
#ifdef TCSUPPORT_WLAN_SW_RPS
	if (sizeof(RX_BLK) > 150) {
		printk("!!!rx_blk size = %d ,is larger than 150 in skb!!!\n", sizeof(RX_BLK));
				goto err;
		}

	printk("\n==ecnt_wifi_rx_rps_hook===\n");
	rcu_assign_pointer(ecnt_wifi_rx_rps_hook, ecnt_wifi_rx_rps);
#endif
#endif

	return retval;
err:
	RTMP_DRIVER_IRQ_RELEASE(pAd);
	return -1;
}

int virtual_if_up_handler(VOID *dev)
{
	struct net_device *net_dev = (struct net_device *)dev;
	struct _RTMP_ADAPTER *pAd = NULL;
	int retval = 0;
	struct wifi_dev *wdev = NULL;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL) {
		retval = -1;
		return retval;
	}

	wdev = wdev_search_by_netdev(pAd, net_dev);
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() wdev fail!!!\n", __func__));
		retval = -1;
		return retval;
	}

	wdev_if_up_down(pAd, wdev, TRUE);
#ifdef GREENAP_SUPPORT
	/* This function will check and update allow status */
	if (greenap_check_when_if_down_up(pAd) == FALSE)
		return retval;
#endif /* GREENAP_SUPPORT */

	if (VIRTUAL_IF_NUM(pAd) != 0) {
		if (wdev_do_open(wdev) != TRUE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() inf_up (idx %d) fail!!!\n",
				__func__, wdev->wdev_idx));
		}
	}

	return retval;
}

int virtual_if_down_handler(VOID *dev)
{
	struct net_device *net_dev = (struct net_device *)dev;
	struct _RTMP_ADAPTER *pAd = NULL;
	int retval = 0;
	struct wifi_dev *wdev = NULL;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL) {
		retval = -1;
		return retval;
	}

	wdev = wdev_search_by_netdev(pAd, net_dev);
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() wdev fail!!!\n", __func__));
		retval = -1;
		return retval;
	}

/*wdev->if_up_down_state should be mark false after wdev_do_close*/
	if (wdev_do_close(wdev) != TRUE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() inf_down (idx %d) fail!!!\n",
			__func__, wdev->wdev_idx));
	}
	wdev_if_up_down(pAd, wdev, FALSE);
#ifdef GREENAP_SUPPORT
	greenap_check_when_if_down_up(pAd);
#endif /* GREENAP_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
		pAd->cfg80211_ctrl.beaconIsSetFromHostapd = FALSE;
#endif

#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_RADIO_OFF, HcGetBandByWdev(wdev));
#endif /* LED_CONTROL_SUPPORT */

	return retval;
}


INT virtual_if_init_handler(VOID *dev)
{
	struct net_device *net_dev = (struct net_device *)dev;
	struct _RTMP_ADAPTER *pAd = NULL;
	int retval = 0;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL) {
		retval = -1;
		return retval;
	}

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		VIRTUAL_IF_INC(pAd);

#ifdef MT_DFS_SUPPORT
		/* Update bInitMbssZeroWait  */
		UPDATE_MT_INIT_ZEROWAIT_MBSS(pAd, TRUE);
#endif /* MT_DFS_SUPPORT */

		/* must use main net_dev */
		if (mt_wifi_open(pAd->net_dev) != 0) {
			VIRTUAL_IF_DEC(pAd);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("mt_wifi_open return fail!\n"));
			return NDIS_STATUS_FAILURE;
		}
	} else
		VIRTUAL_IF_INC(pAd);

	return retval;
}

INT virtual_if_deinit_handler(VOID *dev)
{
	struct net_device *net_dev = (struct net_device *)dev;
	struct _RTMP_ADAPTER *pAd = NULL;
	int retval = 0;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL) {
		retval = -1;
		return retval;
	}

	if (VIRTUAL_IF_NUM(pAd) == 0)
		mt_wifi_close(pAd->net_dev);

	return retval;
}

PNET_DEV RtmpPhyNetDevInit(VOID *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevHook)
{
	struct net_device *net_dev = NULL;
	ULONG InfId, OpMode;
	RTMP_DRIVER_MAIN_INF_GET(pAd, &InfId);
	/*	net_dev = RtmpOSNetDevCreate(pAd, INT_MAIN, 0, sizeof(struct mt_dev_priv), INF_MAIN_DEV_NAME); */
	RTMP_DRIVER_MAIN_INF_CREATE(pAd, &net_dev);

	if (net_dev == NULL) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): main physical net device creation failed!\n", __func__));
		return NULL;
	}

	os_zero_mem((unsigned char *)pNetDevHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	pNetDevHook->open = main_virtual_if_open;
	pNetDevHook->stop = main_virtual_if_close;
	pNetDevHook->xmit = rt28xx_send_packets;
#ifdef IKANOS_VX_1X0
	pNetDevHook->xmit = IKANOS_DataFramesTx;
#endif /* IKANOS_VX_1X0 */
	pNetDevHook->ioctl = rt28xx_ioctl;
	pNetDevHook->priv_flags = InfId; /*INT_MAIN; */
	pNetDevHook->get_stats = RT28xx_get_ether_stats;
	pNetDevHook->needProtcted = FALSE;
#if (WIRELESS_EXT < 21) && (WIRELESS_EXT >= 12)
	pNetDevHook->get_wstats = rt28xx_get_wireless_stats;
#endif
	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);
	/* put private data structure */
	RTMP_OS_NETDEV_SET_PRIV(net_dev, pAd);

	/* double-check if pAd is associated with the net_dev */
	if (RTMP_OS_NETDEV_GET_PRIV(net_dev) == NULL) {
		RtmpOSNetDevFree(net_dev);
		return NULL;
	}

	RTMP_DRIVER_NET_DEV_SET(pAd, net_dev);
#if (KERNEL_VERSION(2, 6, 24) > LINUX_VERSION_CODE)
	SET_MODULE_OWNER(net_dev);
#endif
	return net_dev;
}


VOID *RtmpNetEthConvertDevSearch(VOID *net_dev_, UCHAR *pData)
{
	struct net_device *pNetDev;
#if (KERNEL_VERSION(2, 6, 24) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 26) <= LINUX_VERSION_CODE)
	struct net_device *net_dev = (struct net_device *)net_dev_;
	struct net *net;

	net = dev_net(net_dev);
	BUG_ON(!net);
	for_each_netdev(net, pNetDev)
#else
	struct net *net;
	struct net_device *net_dev = (struct net_device *)net_dev_;

	BUG_ON(!net_dev->nd_net);
	net = net_dev->nd_net;
	for_each_netdev(net, pNetDev)
#endif
#else
#if (KERNEL_VERSION(2, 6, 22) <= LINUX_VERSION_CODE)
	for_each_netdev(pNetDev)
#else

	for (pNetDev = dev_base; pNetDev; pNetDev = pNetDev->next)
#endif
#endif
	{
		if ((pNetDev->type == ARPHRD_ETHER)
			&& os_equal_mem(pNetDev->dev_addr, &pData[6], pNetDev->addr_len))
			break;
	}

	return (VOID *)pNetDev;
}



/*
 * ========================================================================
 * Routine Description:
 *   The entry point for Linux kernel sent packet to our driver.
 *
 * Arguments:
 *   sk_buff *skb	the pointer refer to a sk_buffer.
 *
 * Return Value:
 *    0
 *
 * Note:
 *	This function is the entry point of Tx Path for OS delivery packet to
 *	our driver. You only can put OS-depened & STA/AP common handle procedures
 *	in here.
 * ========================================================================
 */
int rt28xx_packet_xmit(void *pkt)
{
	struct sk_buff *skb = (struct sk_buff *)pkt;
	struct net_device *net_dev = skb->dev;
	struct wifi_dev *wdev;
	PNDIS_PACKET pPacket = (PNDIS_PACKET)skb;
#ifdef DYNAMIC_STEERING_LOADING
	RTMP_ADAPTER *pAd;
#endif
	int status = 0;

	wdev = RTMP_OS_NETDEV_GET_WDEV(net_dev);
	ASSERT(wdev);

#ifdef DYNAMIC_STEERING_LOADING
	if (!wdev->sys_handle) {
		ASSERT(wdev->sys_handle);
		return status;
	}
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	if (!pAd->disable_auto_rps)
	{
		RTMP_ARCH_OP *arch_ops;

		arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
		if ((arch_ops->arch_combo_tx_rps_check) && (arch_ops->arch_combo_tx_rps_check(pAd, pPacket, wdev) == 1))
			return status;
	}
#endif /* DYNAMIC_STEERING_LOADING */
	status = RTMPSendPackets((NDIS_HANDLE)wdev, (PPNDIS_PACKET) & pPacket, 1,
						   skb->len, RtmpNetEthConvertDevSearch);
	return status;
}


/*
 * ========================================================================
 * Routine Description:
 *    Send a packet to WLAN.
 *
 * Arguments:
 *    skb_p           points to our adapter
 *    dev_p           which WLAN network interface
 *
 * Return Value:
 *   0: transmit successfully
 *   otherwise: transmit fail
 *
 * ========================================================================
 */
int rt28xx_send_packets(struct sk_buff *skb, struct net_device *ndev)
{
	if (!(RTMP_OS_NETDEV_STATE_RUNNING(ndev))) {
		RELEASE_NDIS_PACKET(NULL, (PNDIS_PACKET)skb, NDIS_STATUS_FAILURE);
		return 0;
	}

	os_zero_mem((PUCHAR)&skb->cb[CB_OFF], 26);
	MEM_DBG_PKT_ALLOC_INC(skb);
	return rt28xx_packet_xmit(skb);
}


#if WIRELESS_EXT >= 12
/* This function will be called when query /proc */
struct iw_statistics *rt28xx_get_wireless_stats(struct net_device *net_dev)
{
	VOID *pAd = NULL;
	struct iw_statistics *pStats;
	RT_CMD_IW_STATS DrvIwStats, *pDrvIwStats = &DrvIwStats;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("rt28xx_get_wireless_stats --->\n"));
	pDrvIwStats->priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pDrvIwStats->dev_addr = (PUCHAR)net_dev->dev_addr;

	if (RTMP_DRIVER_IW_STATS_GET(pAd, pDrvIwStats) != NDIS_STATUS_SUCCESS)
		return NULL;

	pStats = (struct iw_statistics *)(pDrvIwStats->pStats);
	pStats->status = 0; /* Status - device dependent for now */
	pStats->qual.updated = 1;     /* Flags to know if updated */
#ifdef IW_QUAL_DBM
	pStats->qual.updated |= IW_QUAL_DBM;	/* Level + Noise are dBm */
#endif /* IW_QUAL_DBM */
	pStats->qual.qual = pDrvIwStats->qual;
	pStats->qual.level = pDrvIwStats->level;
	pStats->qual.noise = pDrvIwStats->noise;
	pStats->discard.nwid = 0;     /* Rx : Wrong nwid/essid */
	pStats->miss.beacon = 0;      /* Missed beacons/superframe */
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--- rt28xx_get_wireless_stats\n"));
	return pStats;
}
#endif /* WIRELESS_EXT */


INT rt28xx_ioctl(PNET_DEV net_dev, struct ifreq *rq, INT cmd)
{
	INT ret = 0;
	struct wifi_dev *wdev;
	struct wifi_dev_ops *ops;
	VOID *pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);
	if (pAd == NULL)
		return -ENETDOWN;

	GET_WDEV_FROM_NET_DEV(wdev, net_dev);
	if (wdev == NULL)
		return -ENETDOWN;
	ops = wdev->wdev_ops;

	if (!ops)
		return -ENETDOWN;

	ASSERT(ops->ioctl);

	if (ops->ioctl)
		ops->ioctl(net_dev, rq, cmd);
	else
		return -ENETDOWN;
	return ret;
}


/*
 * ========================================================================
 *
 *  Routine Description:
 *	return ethernet statistics counter
 *
 *  Arguments:
 *	net_dev                     Pointer to net_device
 *
 *  Return Value:
 *	net_device_stats*
 * ========================================================================
 */
struct net_device_stats *RT28xx_get_ether_stats(struct net_device *net_dev)
{
	VOID *pAd = NULL;
	struct net_device_stats *pStats;

	if (net_dev)
		GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd) {
		RT_CMD_STATS DrvStats, *pDrvStats = &DrvStats;
		/* assign net device for RTMP_DRIVER_INF_STATS_GET() */
		pDrvStats->pNetDev = net_dev;
		RTMP_DRIVER_INF_STATS_GET(pAd, pDrvStats);
		pStats = (struct net_device_stats *)(pDrvStats->pStats);
		pStats->rx_packets = pDrvStats->rx_packets;
		pStats->tx_packets = pDrvStats->tx_packets;
		pStats->rx_bytes = pDrvStats->rx_bytes;
		pStats->tx_bytes = pDrvStats->tx_bytes;
		pStats->rx_errors = pDrvStats->rx_errors;
		pStats->tx_errors = pDrvStats->tx_errors;
		pStats->rx_dropped = 0;
		pStats->tx_dropped = 0;
		pStats->multicast = pDrvStats->multicast;
		pStats->collisions = pDrvStats->collisions;
		pStats->rx_length_errors = 0;
		pStats->rx_over_errors = pDrvStats->rx_over_errors;
		pStats->rx_crc_errors = 0;/*pAd->WlanCounters[0].FCSErrorCount;     // recved pkt with crc error */
		pStats->rx_frame_errors = pDrvStats->rx_frame_errors;
		pStats->rx_fifo_errors = pDrvStats->rx_fifo_errors;
		pStats->rx_missed_errors = 0;                                            /* receiver missed packet */
		/* detailed tx_errors */
		pStats->tx_aborted_errors = 0;
		pStats->tx_carrier_errors = 0;
		pStats->tx_fifo_errors = 0;
		pStats->tx_heartbeat_errors = 0;
		pStats->tx_window_errors = 0;
		/* for cslip etc */
		pStats->rx_compressed = 0;
		pStats->tx_compressed = 0;
		return pStats;
	} else
		return NULL;
}


BOOLEAN RtmpPhyNetDevExit(VOID *pAd, PNET_DEV net_dev)
{
	/*remove cfg */
	wpf_exit(pAd);
#ifdef CONFIG_AP_SUPPORT
#ifdef WDS_SUPPORT
	/* remove all WDS virtual interfaces. */
	RT28xx_WDS_Remove(pAd);
#endif /* WDS_SUPPORT */
#ifdef SNIFFER_SUPPORT
	RT28xx_Monitor_Remove(pAd);
#endif	/* SNIFFER_SUPPORT */
#ifdef MBSS_SUPPORT
#if defined(P2P_APCLI_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
#else
	RT28xx_MBSS_Remove(pAd);
#endif /* P2P_APCLI_SUPPORT */
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	RT28xx_MSTA_Remove(pAd);
#endif
#ifdef RT_CFG80211_SUPPORT
#ifdef CFG80211_MULTI_STA
	RTMP_CFG80211_MutliStaIf_Remove(pAd);
#endif /* CFG80211_MULTI_STA */
#else
#endif /* RT_CFG80211_SUPPORT */
#ifdef INF_PPA_SUPPORT
	RTMP_DRIVER_INF_PPA_EXIT(pAd);
#endif /* INF_PPA_SUPPORT */

	/* Unregister network device */
	if (net_dev != NULL) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("RtmpOSNetDevDetach(): RtmpOSNetDeviceDetach(), dev->name=%s!\n", net_dev->name));
		RtmpOSNetDevProtect(1);
		RtmpOSNetDevDetach(net_dev);
		RtmpOSNetDevProtect(0);
#ifdef RT_CFG80211_SUPPORT
		RTMP_DRIVER_80211_UNREGISTER(pAd, net_dev);
#endif /* RT_CFG80211_SUPPORT */
	}

	return TRUE;
}


/* Device IRQ related functions. */
int RtmpOSIRQRequest(IN PNET_DEV pNetDev)
{
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
	struct net_device *net_dev = pNetDev;
#endif
	ULONG infType;
	VOID *pAd = NULL;
	int retval = 0;

	GET_PAD_FROM_NET_DEV(pAd, pNetDev);
	ASSERT(pAd);
	RTMP_DRIVER_INF_TYPE_GET(pAd, &infType);
#ifdef RTMP_PCI_SUPPORT

	if (infType == RTMP_DEV_INF_PCI || infType == RTMP_DEV_INF_PCIE) {
#if defined(CONFIG_ARCH_MT7623) || defined(CONFIG_ARCH_MT7622)
		retval = request_irq(net_dev->irq,  rt2860_interrupt, SA_SHIRQ | IRQF_TRIGGER_LOW, (net_dev)->name, (net_dev));
#else
		retval = request_irq(net_dev->irq,  rt2860_interrupt, SA_SHIRQ, (net_dev)->name, (net_dev));
#endif

		if (retval != 0)
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("RT2860: request_irq  ERROR(%d)\n", retval));
	}

#endif /* RTMP_PCI_SUPPORT */
#ifdef RTMP_RBUS_SUPPORT

	if (infType == RTMP_DEV_INF_RBUS) {
#if (KERNEL_VERSION(2, 6, 22) <= LINUX_VERSION_CODE)
#if defined(MT7622) || defined(P18) || defined(MT7663) || defined(AXE) || defined(MT7626)
		retval = request_irq(net_dev->irq, rt2860_interrupt, IRQF_SHARED | IRQF_TRIGGER_LOW, net_dev->name, net_dev);
#else
		retval = request_irq(net_dev->irq, rt2860_interrupt, IRQF_SHARED, net_dev->name, net_dev);
#endif
#else
		retval = request_irq(net_dev->irq, rt2860_interrupt, SA_INTERRUPT, net_dev->name, net_dev);
#endif

		if (retval) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("RT2860: request_irq  ERROR(%d)\n", retval));
		}
#ifdef MT7626_E2_SUPPORT
		{
			PRTMP_ADAPTER pAd2 = (PRTMP_ADAPTER)pAd;
			if (IS_MT7626_FW_VER_E2(pAd2) && IS_ASIC_CAP(pAd2, fASIC_CAP_MD)) {
				retval = request_irq(pAd2->dev_irq_md, rt2860_interrupt_irq_md, IRQF_SHARED | IRQF_TRIGGER_LOW, "md"/*net_dev->name*/, net_dev);
				if (retval) {
					MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("RT2860: dev_irq_md request_irq  ERROR(%d)\n", retval));
				} else {
					MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("RT2860: 7626 E2 run MD\n"));
				}
			} else if (IS_MT7626_FW_VER_E2(pAd2)) {
				 MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("RT2860: 7626 E2 does not run MD\n"));
			} else {
				MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("RT2860: 7626 E1\n"));
			}
		}
#endif
	}

#endif /* RTMP_RBUS_SUPPORT */
	return retval;
}


#ifdef WDS_SUPPORT
/* ========================================================================
 *  Routine Description:
 *	return ethernet statistics counter
 *
 *  Arguments:
 *	net_dev                     Pointer to net_device
 *
 *  Return Value:
 *	net_device_stats*
 * ========================================================================
 */
struct net_device_stats *RT28xx_get_wds_ether_stats(
	IN PNET_DEV net_dev)
{
	VOID *pAd = NULL;
	/*	INT WDS_apidx = 0,index; */
	struct net_device_stats *pStats;
	RT_CMD_STATS WdsStats, *pWdsStats = &WdsStats;

	if (net_dev)
		GET_PAD_FROM_NET_DEV(pAd, net_dev);

	/*	if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_WDS) */
	{
		if (pAd) {
			pWdsStats->pNetDev = net_dev;

			if (RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_WDS_STATS_GET,
									 0, pWdsStats, RT_DEV_PRIV_FLAGS_GET(net_dev)) != NDIS_STATUS_SUCCESS)
				return NULL;

			pStats = (struct net_device_stats *)pWdsStats->pStats; /*pAd->stats; */
			pStats->rx_packets = pWdsStats->rx_packets; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.ReceivedFragmentCount.QuadPart; */
			pStats->tx_packets = pWdsStats->tx_packets; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.TransmittedFragmentCount.QuadPart; */
			pStats->rx_bytes = pWdsStats->rx_bytes; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.ReceivedByteCount; */
			pStats->tx_bytes = pWdsStats->tx_bytes; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.TransmittedByteCount; */
			pStats->rx_errors = pWdsStats->rx_errors; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.RxErrorCount; */
			pStats->tx_errors = pWdsStats->tx_errors; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.TxErrors; */
			pStats->rx_dropped = 0;
			pStats->tx_dropped = 0;
			pStats->multicast = pWdsStats->multicast; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.MulticastReceivedFrameCount.QuadPart;   // multicast packets received */
			pStats->collisions = pWdsStats->collisions; /* Collision packets */
			pStats->rx_length_errors = 0;
			pStats->rx_over_errors = pWdsStats->rx_over_errors; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.RxNoBuffer;                   // receiver ring buff overflow */
			pStats->rx_crc_errors = 0;/*pAd->WlanCounters[0].FCSErrorCount;     // recved pkt with crc error */
			pStats->rx_frame_errors = 0; /* recv'd frame alignment error */
			pStats->rx_fifo_errors = pWdsStats->rx_fifo_errors; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.RxNoBuffer;                   // recv'r fifo overrun */
			pStats->rx_missed_errors = 0;                                            /* receiver missed packet */
			/* detailed tx_errors */
			pStats->tx_aborted_errors = 0;
			pStats->tx_carrier_errors = 0;
			pStats->tx_fifo_errors = 0;
			pStats->tx_heartbeat_errors = 0;
			pStats->tx_window_errors = 0;
			/* for cslip etc */
			pStats->rx_compressed = 0;
			pStats->tx_compressed = 0;
			return pStats;
		} else
			return NULL;
	}
}
#endif /* WDS_SUPPORT */
