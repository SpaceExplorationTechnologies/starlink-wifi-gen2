/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2019, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/

#include "rt_config.h"

#ifdef FW_LOG_DUMP
INT set_fw_log_dest_dir(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 index;
	CHAR last;
	UINT32 max_len = sizeof(pAd->fw_log_ctrl.fw_log_dest_dir) - 1;

	for (index = 0; index < max_len; index++)
		if (*(arg + index + 1) == '\0')
			break;
	last = *(arg + index);

	if (last == '/')
		snprintf(pAd->fw_log_ctrl.fw_log_dest_dir, max_len, "%sfw_log.bin", arg);
	else
		snprintf(pAd->fw_log_ctrl.fw_log_dest_dir, max_len, "%s/fw_log.bin", arg);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("FW Binary log destination directory: %s\n", pAd->fw_log_ctrl.fw_log_dest_dir));

	return TRUE;
}


NTSTATUS fw_log_to_file(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	RTMP_OS_FD_EXT srcf;
	INT8 Ret;

	srcf = os_file_open(pAd->fw_log_ctrl.fw_log_dest_dir, O_WRONLY|O_CREAT|O_APPEND, 0);
	if (srcf.Status) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Open file \"%s\" failed!\n", pAd->fw_log_ctrl.fw_log_dest_dir));
		return NDIS_STATUS_FAILURE;
	}

	os_file_write(srcf, (INT8 *)CMDQelmt->buffer, (UINT32)CMDQelmt->bufferlength);

	Ret = os_file_close(srcf);

	if (Ret)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("File Close Error ! Ret = %d\n", Ret));

	return NDIS_STATUS_SUCCESS;
}


VOID fw_log_to_ethernet(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *fw_log,
	IN UINT32 log_len)
{
	UCHAR s_addr[MAC_ADDR_LEN];
	UCHAR d_addr[MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	UINT32 source_ip = 0x00000000, dest_ip = 0xFFFFFFFF;
	UCHAR ETH_P_AIR_MONITOR[LENGTH_802_3_TYPE] = {0x08, 0x00};
	struct sk_buff *skb = NULL;
	UINT8 isPadding = 0;
	UINT8 *data, *header;
	UINT8 *ip_header, *ip_checksum;
	UINT8 *udp_header, *udp_checksum, *pseudo_header;
	UINT16 data_len, header_len;
	IP_V4_HDR *ipv4_hdr_ptr;
	UINT16 checksum;

	header_len = LENGTH_802_3 + 20 + 8; /* 802.3 + IP + UDP */
	if ((log_len % 2) == 0)
		data_len = log_len;
	else {
		data_len = log_len + 1;
		isPadding = 1;
	}

	skb = dev_alloc_skb(log_len + header_len + 2);

#ifdef CONFIG_AP_SUPPORT
	SET_OS_PKT_NETDEV(skb, pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.if_dev);
#else
	SET_OS_PKT_NETDEV(skb, pAd->StaCfg[0].wdev.if_dev);
#endif /* CONFIG_AP_SUPPORT */

	OS_PKT_RESERVE(skb, header_len);

	/* Prepare payload*/
	data = OS_PKT_TAIL_BUF_EXTEND(skb, data_len);
	NdisCopyMemory(data, fw_log, log_len);
	if (isPadding)
		*(data + log_len) = 0;

	/* Prepare UDP header */
	header = OS_PKT_HEAD_BUF_EXTEND(skb, 8);
	udp_header = header;
	*(UINT16 *)header = htons(54321);           /* source port */
	header += sizeof(UINT16);
	*(UINT16 *)header = htons(55688);           /* destination port */
	header += sizeof(UINT16);
	*(UINT16 *)header = htons(data_len + 8);     /* Length */
	header += sizeof(UINT16);
	udp_checksum = header;
	*(UINT16 *)header = htons(0);               /* UDP Checksum */
	pseudo_header = udp_header - 12;
	header = pseudo_header;
	*(UINT32 *)header = htonl(source_ip);       /* Source IP */
	header += sizeof(UINT32);
	*(UINT32 *)header = htonl(dest_ip);         /* Destination IP */
	header += sizeof(UINT32);
	*(UINT16 *)header = htons(data_len + 8);    /* Length */
	header += sizeof(UINT16);
	*(UINT16 *)header = htons(17);              /* Length */
	checksum = Checksum16(pseudo_header, data_len + 8 + 12);
	*(UINT16 *)udp_checksum = checksum;

	/* Prepare IP header */
	header = OS_PKT_HEAD_BUF_EXTEND(skb, 20);
	ip_header = header;
	ipv4_hdr_ptr = (IP_V4_HDR *)header;
	ipv4_hdr_ptr->version = 4;
	ipv4_hdr_ptr->ihl = 5;
	ipv4_hdr_ptr->tos = 0;
	ipv4_hdr_ptr->tot_len = htons(data_len + 20 + 8);
	ipv4_hdr_ptr->identifier = 0;
	header += sizeof(IP_V4_HDR);
	*(UINT16 *)header = htons(0x4000);          /* Fragmentation flags and offset */
	header += sizeof(UINT16);
	*header = 7;                                /* Time to live */
	header++;
	*header = 17;                               /* Protocol UDP */
	header++;
	ip_checksum = header;
	*(UINT16 *)header = htons(0);               /* IP Checksum */
	header += sizeof(UINT16);
	*(UINT32 *)header = htonl(source_ip);      /* Source IP */
	header += sizeof(UINT32);
	*(UINT32 *)header = htonl(dest_ip);      /* Destination IP */
	checksum = Checksum16(ip_header, 20);
	*(UINT16 *)ip_checksum = checksum;

	/* Prepare 802.3 header */
	header = OS_PKT_HEAD_BUF_EXTEND(skb, LENGTH_802_3);
	/* Fake a Source Address for transmission */
#ifdef CONFIG_AP_SUPPORT
	COPY_MAC_ADDR(s_addr, pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.if_addr);
#else
	COPY_MAC_ADDR(s_addr, pAd->StaCfg[0].wdev.if_addr);
#endif /* CONFIG_AP_SUPPORT */
	if (s_addr[1] == 0xff)
		s_addr[1] = 0;
	else
		s_addr[1]++;
	MAKE_802_3_HEADER(header, d_addr, s_addr, ETH_P_AIR_MONITOR);

	/* Report to upper layer */
	RtmpOsPktProtocolAssign(skb);
	RtmpOsPktRcvHandle(skb);
}
#endif /* FW_LOG_DUMP */

