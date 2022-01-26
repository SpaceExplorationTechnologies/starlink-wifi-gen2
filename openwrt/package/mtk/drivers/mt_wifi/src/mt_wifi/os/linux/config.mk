# Support Multiple Interface within single driver
HAS_MULTI_INF=n

# Support ATE function
HAS_ATE=y

# Support WPA3
HAS_WPA3_SUPPORT=n

# Support OWE
HAS_OWE_SUPPORT=n

# Support QA ATE function
HAS_QA_SUPPORT=y

#Support Internal-Capture function
HAS_ICAP_SUPPORT=y

#Support Wifi-Spectrum function
HAS_WIFI_SPECTRUM_SUPPORT=y

# Support MTK 2.4G 256QAM
HAS_G_BAND_256QAM=y

# Support BRCM 2.4G 256QAM
HAS_BRCM_256QAM=y

# Support VHT TXBF 2G with BRCM STA
HAS_VHT_TXBF_2G_EPIGRAM_IE=n

HAS_RSSI_FEEDBACK=n

# Support XLINK mode
HAS_XLINK=n

# Support WSC function
HAS_WSC=y
HAS_WSC_V2=y
HAS_WSC_LED=n
HAS_WSC_NFC=n


HAS_MEM_ALLOC_INFO_SUPPORT=n

# Support LLTD function
HAS_LLTD=y

# Support WDS function
HAS_WDS=y

# Support AP-Client function
HAS_APCLI=y

# Roaming Enhance Support
HAS_ROAMING_ENHANCE_SUPPORT=n


# Support whole home customized event notify
HAS_WH_EVENT_NOTIFIER=n

# Support Air Monitor
HAS_AIR_MONITOR=n




#Support Net interface block while Tx-Sw queue full
HAS_BLOCK_NET_IF=y

#Support IGMP-Snooping function.
HAS_IGMP_SNOOP_SUPPORT=y

#Support DFS function
HAS_DFS_SUPPORT=n

#Support DFS function
HAS_MT_DFS_SUPPORT=n

#Support Background Scan function
HAS_BGND_SCAN_SUPPORT=n

#Support Smart Carrier Sense function
HAS_SMART_CARRIER_SENSE_SUPPORT=n

#Support Smart Carrier Sense FW offload function
HAS_SCS_FW_OFFLOAD=n

#Support Carrier-Sense function
HAS_CS_SUPPORT=n

#Support TCP_RACK function
HAS_TCP_RACK_SUPPORT=n

#ifeq ($(PLATFORM),MT85XX)
#Support HE_SUPPORT
HAS_HE_SUPPORT=n
HAS_HE_BD_SUPPORT=n

ifeq ($(PLATFORM),MT53XX)
#Support HE_SUPPORT
HAS_HE_SUPPORT=y
HAS_HE_TV_SUPPORT=y
endif

#Support WEPAUTO mode try Open first then shared
HAS_WEPAUTO_OPEN_FIRST_SUPPORT=y


# Support user specific transmit rate of Multicast packet.
HAS_MCAST_RATE_SPECIFIC_SUPPORT=n

# Support for Multiple Cards
HAS_MC_SUPPORT=n

#Support for PCI-MSI
HAS_MSI_SUPPORT=n



#Support for EXT_CHANNEL
HAS_EXT_BUILD_CHANNEL_LIST=n

#Support for IDS
HAS_IDS_SUPPORT=y


#Support for Net-SNMP
HAS_SNMP_SUPPORT=n

#Support features of 802.11n Draft3
HAS_DOT11N_DRAFT3_SUPPORT=y

#Support features of Single SKU.
HAS_SINGLE_SKU_SUPPORT=n

#Support features of Single SKU.
HAS_SINGLE_SKU_V2_SUPPORT=n

#Support features of Link Test support.
HAS_LINK_TEST_SUPPORT=n

#Support features of 802.11n
HAS_DOT11_N_SUPPORT=y

#Support for 802.11ac VHT
HAS_DOT11_VHT_SUPPORT=n

HAS_DOT11_HE_SUPPORT=n

#Support for dot11r FT
HAS_DOT11R_FT_SUPPORT=n

HAS_KTHREAD_SUPPORT=n

#Support for dot11k RRM
HAS_DOT11K_RRM_SUPPORT=n

#Support for dot 11v WNM
HAS_DOT11V_WNM_SUPPORT=n



#Support for WiFi Display
HAS_WFD_SUPPORT=n

#Support for Auto channel select enhance
HAS_AUTO_CH_SELECT_ENHANCE=n

#Support statistics count
HAS_STATS_COUNT=y

#Support TSSI Antenna Variation
HAS_TSSI_ANTENNA_VARIATION=n

#Support USB_BULK_BUF_ALIGMENT
HAS_USB_BULK_BUF_ALIGMENT=n

#Support for USB_SUPPORT_SELECTIVE_SUSPEND
HAS_USB_SUPPORT_SELECTIVE_SUSPEND=n

#Support USB load firmware by multibyte
HAS_USB_FIRMWARE_MULTIBYTE_WRITE=n


#Support ANDROID_SUPPORT (2.X with WEXT-based)
HAS_ANDROID_SUPPORT=n

#HAS_IFUP_IN_PROBE_SUPPORT
HAS_IFUP_IN_PROBE_SUPPORT=n


#Support for dot11w Protected Management Frame
HAS_DOT11W_PMF_SUPPORT=y


#Support TXRX SW Antenna Diversity
HAS_TXRX_SW_ANTDIV_SUPPORT=n

#Client support WDS function
HAS_CLIENT_WDS_SUPPORT=n

#Support for Bridge Fast Path & Bridge Fast Path function open to other module
HAS_BGFP_SUPPORT=n
HAS_BGFP_OPEN_SUPPORT=n

# Support HOSTAPD function
HAS_HOSTAPD_SUPPORT=n

#Support GreenAP function
HAS_GREENAP_SUPPORT=n

#Support PCIE ASPM dynamic control
HAS_PCIE_ASPM_DYM_CTRL_SUPPORT=n

#Support cfg80211 function with Linux Only.
#Please make sure insmod the cfg80211.ko before our driver,
#our driver references to its symbol.
HAS_CFG80211_SUPPORT=n
#smooth the scan signal for cfg80211 based driver
HAS_CFG80211_SCAN_SIGNAL_AVG_SUPPORT=n
#control two STA using wpa_supplicant
HAS_CFG80211_MULTI_STA_SUPPORT=n
#Cfg80211-based P2P Support
HAS_CFG80211_P2P_SUPPORT=n
#Cfg80211-based P2P Mode Selection (must one be chosen)
HAS_CFG80211_P2P_CONCURRENT_DEVICE=n
HAS_CFG80211_P2P_SINGLE_DEVICE=n
HAS_CFG80211_P2P_STATIC_CONCURRENT_DEVICE=n

HAS_CFG80211_P2P_MULTI_CHAN_SUPPORT=n

#Cfg80211-based TDLS support
HAS_CFG80211_TDLS_SUPPORT=n
#For android wifi priv-lib (cfg80211_based wpa_supplicant cmd expansion)
HAS_CFG80211_ANDROID_PRIV_LIB_SUPPORT=n

#Support RFKILL hardware block/unblock LINUX-only function
HAS_RFKILL_HW_SUPPORT=n


HAS_MT76XX_BT_COEXISTENCE_SUPPORT=n

HAS_MT_MAC_BT_COEXISTENCE_SUPPORT=n

HAS_APCLI_WPA_SUPPLICANT=n

HAS_RTMP_FLASH_SUPPORT=n

ifeq ($(OSABL),YES)
HAS_OSABL_FUNC_SUPPORT=y
HAS_OSABL_OS_PCI_SUPPORT=y
HAS_OSABL_OS_USB_SUPPORT=y
HAS_OSABL_OS_RBUS_SUPPORT=n
HAS_OSABL_OS_AP_SUPPORT=y
HAS_OSABL_OS_STA_SUPPORT=y
endif

HAS_LED_CONTROL_SUPPORT=n

#Support WIDI feature
#Must enable HAS_WSC at the same time.

HAS_TXBF_SUPPORT=y
HAS_VHT_TXBF_SUPPORT=y
HAS_STA_ITXBF_SUPPORT=y

HAS_STREAM_MODE_SUPPORT=n

HAS_NEW_RATE_ADAPT_SUPPORT=n

HAS_RATE_ADAPT_AGS_SUPPORT=n

HAS_RACTRL_FW_OFFLOAD_SUPPORT=n





HAS_FW_DUMP_SUPPORT=n

HAS_THERMAL_PROTECT_SUPPORT=y

HAS_ANDES_FIRMWARE_SUPPORT=n

HAS_MULTI_CHANNEL=n

HAS_MICROWAVE_OVEN_SUPPORT=n

HAS_MAC_REPEATER_SUPPORT=y

HAS_SWITCH_CHANNEL_OFFLOAD=n

HAS_RESOURCE_PRE_ALLOC=y
HAS_RESOURCE_BOOT_ALLOC=n


HAS_DVT_MODE=n

HAS_KEEP_ALIVE_OFFLOAD=y

HAS_WTBL_TLV_MODE=n

HAS_HW_HAL_OFFLOAD=y

HAS_SNIFFER_SUPPORT=n

HAS_HOTSPOT_SUPPORT=n

HAS_CALIBRATION_COLLECTION_SUPPORT=y

HAS_CAL_FREE_IC_SUPPORT=y

HAS_FW_DEBUG_SUPPORT=y

HAS_FWOWN_SUPPORT=n

HAS_TRACE_SUPPORT=n

HAS_WIFI_REGION32_HIDDEN_SSID_SUPPORT=n

HAS_UAPSD_SUPPORT=y


HAS_MLME_BY_CMDTHREAD=n

# Assign specified profile path when insmod
HAS_PROFILE_DYN_SUPPORT=n

# MU-MIMO AP
HAS_MU_MIMO_SUPPORT=n

# Falcon MURU
HAS_FALCON_MURU_SUPPORT=n

# This workaround can fix IOT issue with some specific USB host controllers.
# On those host controllers, a USB packet is unexpectedly divided into 2 smaller
# packets.
HAS_USB_IOT_WORKAROUND2=n

# VOW
HAS_VOW_SUPPORT=n

#DSCP PRI SUPPORT
HAS_DSCP_PRI_SUPPORT=n

# Per-clent tail drop(RED)
HAS_RED_SUPPORT=n

HAS_DELAY_INT=n

#Wifi System FW cmd support version
HAS_WIFI_SYS_FW_V1=y
HAS_WIFI_SYS_FW_V2=y

#Wifi MTD related access
HAS_WIFI_MTD=n
#Multi Profile Support
HAS_MULTI_PROFILE_SUPPORT=y


HAS_FW_LOG_DUMP_SUPPORT=n

HAS_WIFI_EAP_FEATURE=y

HAS_WIFI_GPIO_CTRL=y

HAS_VLAN_SUPPORT=y

HAS_MIN_PHY_RATE_SUPPORT=y

HAS_CONFIG_RA_PHY_RATE_SUPPORT=n

HAS_AMPDU_CONF_SUPPORT=n

HAS_ACK_CTS_TIMEOUT_SUPPORT=n

#################################################

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld

WFLAGS := -Wall -Wstrict-prototypes -Wno-trigraphs -Werror -Wno-date-time
WFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT  -DLINUX -DENHANCED_STAT_DISPLAY
WFLAGS += -DSYSTEM_LOG_SUPPORT -DRT28xx_MODE=$(RT28xx_MODE) -DCHIPSET=$(MODULE)
#WFLAGS += -DDBG_DIAGNOSE -DDBG_RX_MCS -DDBG_TX_MCS
WFLAGS += -DOLDSEC
#APsoc Specific
WFLAGS += -DCONFIG_RA_NAT_NONE
#end of /* APsoc Specific */

#For linux kernel > 3.5 to select SLAB or PAGE memory alloc method
#WFLAGS += -DCONFIG_WIFI_PAGE_ALLOC_SKB
#WFLAGS += -DCONFIG_WIFI_SLAB_ALLOC_SKB


WFLAGS += -I$(RT28xx_DIR)/include -I$(RT28xx_EMBEDDED_DIR)/include

ifeq ($(HAS_RESOURCE_PRE_ALLOC),y)
WFLAGS += -DRESOURCE_PRE_ALLOC
endif



ifeq ($(HAS_MULTI_PROFILE_SUPPORT),y)
WFLAGS += -DMULTI_PROFILE
endif

ifeq ($(HAS_WPA3_SUPPORT),y)
WFLAGS += -DDOT11_SAE_SUPPORT -DDOT11_SUITEB_SUPPORT -DDOT11W_PMF_SUPPORT
WFLAGS += -DDOT11_SAE_OPENSSL_BN

ifeq ($(HAS_APCLI),y)
WFLAGS += -DAPCLI_SAE_SUPPORT
endif

ifeq ($(HAS_OWE_SUPPORT),y)
WFLAGS += -DCONFIG_OWE_SUPPORT
endif
endif


ifeq ($(HAS_HE_SUPPORT),y)
WFLAGS += -DHE_SUPPORT
WFLAGS += -DIP_ASSEMBLY
endif

ifeq ($(HAS_G_BAND_256QAM),y)
WFLAGS += -DG_BAND_256QAM
ifeq ($(HAS_BRCM_256QAM),y)
WFLAGS += -DBRCM_256QAM
endif
endif

ifeq ($(HAS_VHT_TXBF_2G_EPIGRAM_IE),y)
WFLAGS += -DVHT_TXBF_2G_EPIGRAM_IE
endif

ifeq ($(HAS_MULTI_INF),y)
WFLAGS += -DMULTI_INF_SUPPORT
endif

ifeq ($(HAS_KTHREAD_SUPPORT),y)
WFLAGS += -DKTHREAD_SUPPORT
endif

ifeq ($(HAS_RTMP_FLASH_SUPPORT),y)
WFLAGS += -DRTMP_FLASH_SUPPORT
endif

ifeq ($(HAS_STREAM_MODE_SUPPORT),y)
WFLAGS += -DSTREAM_MODE_SUPPORT
endif

ifeq ($(HAS_SINGLE_SKU_SUPPORT),y)
WFLAGS += -DSINGLE_SKU
endif

ifeq ($(HAS_SINGLE_SKU_V2_SUPPORT),y)
WFLAGS += -DSINGLE_SKU_V2
endif

ifeq ($(HAS_LINK_TEST_SUPPORT),y)
WFLAGS += -DLINK_TEST_SUPPORT
endif

ifeq ($(HAS_DOT11_VHT_SUPPORT),y)
WFLAGS += -DDOT11_VHT_AC
endif

ifeq ($(HAS_ANDES_FIRMWARE_SUPPORT),y)
WFLAGS += -DANDES_FIRMWARE_SUPPORT
endif

ifeq ($(HAS_THERMAL_PROTECT_SUPPORT),y)
WFLAGS += -DTHERMAL_PROTECT_SUPPORT
endif

ifeq ($(HAS_ATE),y)
WFLAGS += -DCONFIG_ATE
WFLAGS += -I$(RT28xx_DIR)/ate/include

ifeq ($(HAS_QA_SUPPORT),y)
WFLAGS += -DCONFIG_QA
endif

endif

ifeq ($(HAS_CAL_FREE_IC_SUPPORT),y)
WFLAGS += -DCAL_FREE_IC_SUPPORT
endif

ifeq ($(HAS_UAPSD_SUPPORT),y)
WFLAGS += -DUAPSD_SUPPORT
endif

ifeq ($(HAS_MT_PS_SUPPORT),y)
WFLAGS += -DMT_PS
endif

ifeq ($(HAS_FW_DUMP_SUPPORT),y)
WFLAGS += -DFW_DUMP_SUPPORT
endif

ifeq ($(HAS_USB_IOT_WORKAROUND2),y)
WFLAGS += -DUSB_IOT_WORKAROUND2
endif

ifeq ($(HAS_MLME_BY_CMDTHREAD),y)
WFLAGS += -DMLME_BY_CMDTHREAD
endif

ifeq ($(HAS_DELAY_INT),y)
WFLAGS += -DCONFIG_DELAY_INT
endif

ifeq ($(HAS_WPA_SUPPLICANT),n)
ifeq ($(HAS_P2P_SUPPORT),n)
WFLAGS += -DNEWSEC
ifeq ($(HAS_DOT11W_PMF_SUPPORT),y)
WFLAGS += -DDOT11W_PMF_SUPPORT
endif
endif
endif

ifeq ($(HAS_WIFI_SYS_FW_V1),y)
WFLAGS += -DWIFI_SYS_FW_V1
endif

ifeq ($(HAS_WIFI_SYS_FW_V2),y)
WFLAGS += -DWIFI_SYS_FW_V2
endif

ifeq ($(HAS_WIFI_MTD),y)
WFLAGS += -DCONFIG_WIFI_MTD
endif


ifeq ($(HAS_FW_LOG_DUMP_SUPPORT),y)
WFLAGS += -DFW_LOG_DUMP
endif

#-Wframe-larger-than=4096

###############################################################################
#
# config for AP mode
#
###############################################################################


ifeq ($(RT28xx_MODE),AP)
WFLAGS += -DCONFIG_AP_SUPPORT -DMBSS_SUPPORT -DIAPP_SUPPORT  -DDOT1X_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT
WFLAGS += -DDBG
#WFLAGS += -DACCOUNTING_SUPPORT
ifeq ($(HAS_APCLI_WPA_SUPPLICANT),y)
WFLAGS += -DApCli_WPA_SUPPLICANT_SUPPORT
endif

ifeq ($(HAS_HOSTAPD_SUPPORT),y)
WFLAGS += -DHOSTAPD_SUPPORT
endif

ifeq ($(HAS_RSSI_FEEDBACK),y)
WFLAGS += -DRSSI_FEEDBACK
endif



ifeq ($(HAS_ICAP_SUPPORT),y)
WFLAGS += -DINTERNAL_CAPTURE_SUPPORT
endif

ifeq ($(HAS_WIFI_SPECTRUM_SUPPORT),y)
WFLAGS += -DWIFI_SPECTRUM_SUPPORT
endif

ifeq ($(HAS_WSC),y)
WFLAGS += -DWSC_AP_SUPPORT

ifeq ($(HAS_WSC_V2),y)
WFLAGS += -DWSC_V2_SUPPORT
endif
ifeq ($(HAS_WSC_LED),y)
WFLAGS += -DWSC_LED_SUPPORT
endif
ifeq ($(HAS_WSC_NFC),y)
WFLAGS += -DWSC_NFC_SUPPORT
endif
endif


ifeq ($(HAS_WDS),y)
WFLAGS += -DWDS_SUPPORT
endif


ifeq ($(HAS_WH_EVENT_NOTIFIER),y)
WFLAGS += -DWH_EVENT_NOTIFIER
endif

ifeq ($(HAS_AIR_MONITOR),y)
WFLAGS += -DAIR_MONITOR
endif

ifeq ($(HAS_ROAMING_ENHANCE_SUPPORT),y)
HAS_APCLI=y
WFLAGS += -DROAMING_ENHANCE_SUPPORT
endif

ifeq ($(HAS_APCLI),y)
WFLAGS += -DAPCLI_SUPPORT -DMAT_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT -DAPCLI_AUTO_CONNECT_SUPPORT  -DAPCLI_AUTO_BW_TMP
WFLAGS += -DCONFIG_APSTA_MIXED_SUPPORT -DETH_CONVERT_SUPPORT -DCONFIG_STA_SUPPORT
#ifeq ($(HAS_ETH_CONVERT_SUPPORT), y)
#WFLAGS += -DETH_CONVERT_SUPPORT
endif

ifeq ($(HAS_MAC_REPEATER_SUPPORT),y)
WFLAGS += -DMAC_REPEATER_SUPPORT -DAPCLI_SUPPORT -DMAT_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT
endif


ifeq ($(HAS_IGMP_SNOOP_SUPPORT),y)
WFLAGS += -DIGMP_SNOOP_SUPPORT
endif

ifeq ($(HAS_CS_SUPPORT),y)
WFLAGS += -DCARRIER_DETECTION_SUPPORT
endif

ifeq ($(HAS_MCAST_RATE_SPECIFIC_SUPPORT), y)
WFLAGS += -DMCAST_RATE_SPECIFIC
endif

ifneq ($(findstring 2860,$(CHIPSET)),)
ifeq ($(HAS_MSI_SUPPORT),y)
WFLAGS += -DPCI_MSI_SUPPORT
endif
endif


ifeq ($(HAS_SNMP_SUPPORT),y)
WFLAGS += -DSNMP_SUPPORT
endif

ifeq ($(HAS_DOT11_N_SUPPORT),y)
WFLAGS += -DDOT11_N_SUPPORT

ifeq ($(HAS_DOT11N_DRAFT3_SUPPORT),y)
WFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(HAS_TXBF_SUPPORT),y)
WFLAGS += -DTXBF_SUPPORT
endif

ifeq ($(HAS_VHT_TXBF_SUPPORT),y)
WFLAGS += -DVHT_TXBF_SUPPORT
endif

ifeq ($(HAS_STA_ITXBF_SUPPORT), y)
WFLAGS += -DSTA_ITXBF_SUPPORT
endif

ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
WFLAGS += -DNEW_RATE_ADAPT_SUPPORT
endif

ifeq ($(HAS_RATE_ADAPT_AGS_SUPPORT),y)
WFLAGS += -DAGS_SUPPORT
endif

ifeq ($(HAS_GREENAP_SUPPORT),y)
WFLAGS += -DGREENAP_SUPPORT
endif

ifeq ($(HAS_PCIE_ASPM_DYM_CTRL_SUPPORT),y)
WFLAGS += -DPCIE_ASPM_DYM_CTRL_SUPPORT
endif

endif

ifeq ($(HAS_AUTO_CH_SELECT_ENHANCE),y)
WFLAGS += -DAUTO_CH_SELECT_ENHANCE
endif

ifeq ($(HAS_STATS_COUNT),y)
WFLAGS += -DSTATS_COUNT_SUPPORT
endif

ifeq ($(HAS_TSSI_ANTENNA_VARIATION),y)
WFLAGS += -DTSSI_ANTENNA_VARIATION
endif

ifeq ($(HAS_USB_BULK_BUF_ALIGMENT),y)
WFLAGS += -DUSB_BULK_BUF_ALIGMENT
endif

ifeq ($(HAS_CFG80211_SUPPORT),y)
WFLAGS += -DRT_CFG80211_SUPPORT -DWPA_SUPPLICANT_SUPPORT
#WFLAGS += -DEXT_BUILD_CHANNEL_LIST
ifeq ($(HAS_RFKILL_HW_SUPPORT),y)
WFLAGS += -DRFKILL_HW_SUPPORT
endif
ifeq ($(HAS_CFG80211_SCAN_SIGNAL_AVG_SUPPORT),y)
WFLAGS += -DCFG80211_SCAN_SIGNAL_AVG
endif
ifeq ($(HAS_CFG80211_P2P_SUPPORT),y)
WFLAGS += -DRT_CFG80211_P2P_SUPPORT
WFLAGS += -DCONFIG_AP_SUPPORT -DUAPSD_SUPPORT -DMBSS_SUPPORT -DAP_SCAN_SUPPORT
WFLAGS += -DAPCLI_SUPPORT
ifeq ($(HAS_CFG80211_P2P_SINGLE_DEVICE),y)
WFLAGS += -DRT_CFG80211_P2P_SINGLE_DEVICE
else
ifeq ($(HAS_CFG80211_P2P_STATIC_CONCURRENT_DEVICE),y)
WFLAGS += -DRT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
else
WFLAGS += -DRT_CFG80211_P2P_CONCURRENT_DEVICE
ifeq ($(HAS_CFG80211_P2P_MULTI_CHAN_SUPPORT),y)
WFLAGS += -DMULTI_WMM_SUPPORT -DCONFIG_MULTI_CHANNEL -DRT_CFG80211_P2P_MULTI_CHAN_SUPPORT
endif
endif
endif
endif
ifeq ($(HAS_CFG80211_ANDROID_PRIV_LIB_SUPPORT),y)
WFLAGS += -DRT_RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT
endif
ifeq ($(HAS_CFG80211_TDLS_SUPPORT),y)
WFLAGS += -DCFG_TDLS_SUPPORT -DUAPSD_SUPPORT
endif
endif

ifeq ($(OSABL),YES)
WFLAGS += -DOS_ABL_SUPPORT
ifeq ($(HAS_OSABL_FUNC_SUPPORT),y)
WFLAGS += -DOS_ABL_FUNC_SUPPORT
endif
ifeq ($(HAS_OSABL_OS_PCI_SUPPORT),y)
WFLAGS += -DOS_ABL_OS_PCI_SUPPORT
endif
ifeq ($(HAS_OSABL_OS_USB_SUPPORT),y)
WFLAGS += -DOS_ABL_OS_USB_SUPPORT
endif
ifeq ($(HAS_OSABL_OS_RBUS_SUPPORT),y)
WFLAGS += -DOS_ABL_OS_RBUS_SUPPORT
endif
ifeq ($(HAS_OSABL_OS_AP_SUPPORT),y)
WFLAGS += -DOS_ABL_OS_AP_SUPPORT
endif
ifeq ($(HAS_OSABL_OS_STA_SUPPORT),y)
WFLAGS += -DOS_ABL_OS_STA_SUPPORT
endif
endif


ifeq ($(HAS_TXRX_SW_ANTDIV_SUPPORT),y)
WFLAGS += -DTXRX_SW_ANTDIV_SUPPORT
endif

ifeq ($(HAS_BGND_SCAN_SUPPORT),y)
ifneq ($(findstring mt7615,$(CHIPSET)),)
WFLAGS += -DBACKGROUND_SCAN_SUPPORT
endif
endif

ifeq ($(HAS_SMART_CARRIER_SENSE_SUPPORT),y)
WFLAGS += -DSMART_CARRIER_SENSE_SUPPORT
endif

ifeq ($(HAS_WIFI_EAP_FEATURE),y)
WFLAGS += -DWIFI_EAP_FEATURE
endif

ifeq ($(HAS_WIFI_GPIO_CTRL),y)
WFLAGS += -DWIFI_GPIO_CTRL
endif

ifeq ($(HAS_VLAN_SUPPORT),y)
WFLAGS += -DVLAN_SUPPORT
endif

ifeq ($(HAS_SCS_FW_OFFLOAD),y)
WFLAGS += -DSCS_FW_OFFLOAD
endif

ifeq ($(HAS_MIN_PHY_RATE_SUPPORT),y)
WFLAGS += -DMIN_PHY_RATE_SUPPORT
endif

ifeq ($(HAS_CONFIG_RA_PHY_RATE_SUPPORT),y)
WFLAGS += -DCONFIG_RA_PHY_RATE_SUPPORT
endif


endif #// endif of RT2860_MODE == AP //

########################################################
#
# config for STA mode
#
########################################################


ifeq ($(RT28xx_MODE),STA)
WFLAGS += -DCONFIG_STA_SUPPORT -DSCAN_SUPPORT -DDBG
ifeq ($(HAS_MT76XX_BT_COEXISTENCE_SUPPORT),y)
WFLAGS += -DMT76XX_BTCOEX_SUPPORT
endif

ifeq ($(HAS_MT_MAC_BT_COEXISTENCE_SUPPORT),y)
WFLAGS += -DMT_MAC_BTCOEX
endif

ifeq ($(HAS_XLINK),y)
WFLAGS += -DXLINK_SUPPORT
endif



ifeq ($(HAS_WSC),y)
WFLAGS += -DWSC_STA_SUPPORT
ifeq ($(HAS_WSC_V2),y)
WFLAGS += -DWSC_V2_SUPPORT
endif
ifeq ($(HAS_WSC_LED),y)
WFLAGS += -DWSC_LED_SUPPORT
endif
ifeq ($(HAS_IWSC_SUPPORT),y)
WFLAGS += -DIWSC_SUPPORT
endif
endif




ifeq ($(HAS_SNMP_SUPPORT),y)
WFLAGS += -DSNMP_SUPPORT
endif

ifeq ($(HAS_DOT11_N_SUPPORT),y)
WFLAGS += -DDOT11_N_SUPPORT

ifeq ($(HAS_DOT11N_DRAFT3_SUPPORT),y)
WFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(HAS_TXBF_SUPPORT),y)
WFLAGS += -DTXBF_SUPPORT
endif

ifeq ($(HAS_VHT_TXBF_SUPPORT),y)
WFLAGS += -DVHT_TXBF_SUPPORT
endif

ifeq ($(HAS_STA_ITXBF_SUPPORT), y)
WFLAGS += -DSTA_ITXBF_SUPPORT
endif

ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
WFLAGS += -DNEW_RATE_ADAPT_SUPPORT
endif

endif


ifeq ($(HAS_WFD_SUPPORT),y)
HAS_P2P_SUPPORT=y
HAS_P2P_ODD_MAC_ADJUST=y
HAS_P2P_SPECIFIC_WIRELESS_EVENT=y
WFLAGS += -DWFD_SUPPORT
endif

ifeq ($(HAS_PROFILE_DYN_SUPPORT),y)
WFLAGS += -DPROFILE_PATH_DYNAMIC
endif


ifeq ($(HAS_CS_SUPPORT),y)
WFLAGS += -DCARRIER_DETECTION_SUPPORT
endif

ifeq ($(HAS_STATS_COUNT),y)
WFLAGS += -DSTATS_COUNT_SUPPORT
endif

ifeq ($(HAS_TSSI_ANTENNA_VARIATION),y)
WFLAGS += -DTSSI_ANTENNA_VARIATION
endif

ifeq ($(HAS_ANDROID_SUPPORT),y)
WFLAGS += -DANDROID_SUPPORT
endif

ifeq ($(HAS_IFUP_IN_PROBE_SUPPORT),y)
WFLAGS += -DIFUP_IN_PROBE
endif

ifeq ($(HAS_USB_SUPPORT_SELECTIVE_SUSPEND),y)
WFLAGS += -DUSB_SUPPORT_SELECTIVE_SUSPEND
endif

ifeq ($(HAS_USB_FIRMWARE_MULTIBYTE_WRITE),y)
WFLAGS += -DUSB_FIRMWARE_MULTIBYTE_WRITE -DMULTIWRITE_BYTES=4
endif

ifeq ($(HAS_CFG80211_SUPPORT),y)
WFLAGS += -DRT_CFG80211_SUPPORT -DWPA_SUPPLICANT_SUPPORT
#WFLAGS += -DEXT_BUILD_CHANNEL_LIST
ifeq ($(HAS_RFKILL_HW_SUPPORT),y)
WFLAGS += -DRFKILL_HW_SUPPORT
endif
ifeq ($(HAS_CFG80211_SCAN_SIGNAL_AVG_SUPPORT),y)
WFLAGS += -DCFG80211_SCAN_SIGNAL_AVG
endif
ifeq ($(HAS_CFG80211_MULTI_STA_SUPPORT),y)
WFLAGS += -DCFG80211_MULTI_STA -DCONFIG_AP_SUPPORT -DAPCLI_SUPPORT
WFLAGS += -DMBSS_SUPPORT -DAP_SCAN_SUPPORT -DMULTI_WMM_SUPPORT
endif

ifeq ($(HAS_CFG80211_MULTI_STA_SUPPORT),y)
WFLAGS += -DCFG80211_MULTI_STA -DCONFIG_AP_SUPPORT -DAPCLI_SUPPORT
WFLAGS += -DMBSS_SUPPORT -DAP_SCAN_SUPPORT -DMULTI_WMM_SUPPORT
endif

ifeq ($(HAS_CFG80211_P2P_SUPPORT),y)
WFLAGS += -DRT_CFG80211_P2P_SUPPORT -DUAPSD_SUPPORT -DMBSS_SUPPORT -DAP_SCAN_SUPPORT
WFLAGS += -DCONFIG_AP_SUPPORT -DAPCLI_SUPPORT
ifeq ($(HAS_CFG80211_P2P_SINGLE_DEVICE),y)
WFLAGS += -DRT_CFG80211_P2P_SINGLE_DEVICE
else
ifeq ($(HAS_CFG80211_P2P_STATIC_CONCURRENT_DEVICE),y)
WFLAGS += -DRT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
else
WFLAGS += -DRT_CFG80211_P2P_CONCURRENT_DEVICE
ifeq ($(HAS_CFG80211_P2P_MULTI_CHAN_SUPPORT),y)
WFLAGS +=  -DMULTI_WMM_SUPPORT -DCONFIG_MULTI_CHANNEL -DRT_CFG80211_P2P_MULTI_CHAN_SUPPORT
endif #HAS_CFG80211_P2P_MULTI_CHAN_SUPPORT
endif #HAS_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
endif #HAS_CFG80211_P2P_SINGLE_DEVICE
endif #HAS_CFG80211_P2P_SUPPORT
ifeq ($(HAS_CFG80211_ANDROID_PRIV_LIB_SUPPORT),y)
WFLAGS += -DRT_RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT
endif #HAS_CFG80211_ANDROID_PRIV_LIB_SUPPORT
ifeq ($(HAS_CFG80211_TDLS_SUPPORT),y)
WFLAGS += -DCFG_TDLS_SUPPORT -DUAPSD_SUPPORT
endif
endif

ifeq ($(OSABL),YES)
WFLAGS += -DOS_ABL_SUPPORT
ifeq ($(HAS_OSABL_FUNC_SUPPORT),y)
WFLAGS += -DOS_ABL_FUNC_SUPPORT
endif
ifeq ($(HAS_OSABL_OS_PCI_SUPPORT),y)
WFLAGS += -DOS_ABL_OS_PCI_SUPPORT
endif
ifeq ($(HAS_OSABL_OS_USB_SUPPORT),y)
WFLAGS += -DOS_ABL_OS_USB_SUPPORT
endif
ifeq ($(HAS_OSABL_OS_RBUS_SUPPORT),y)
WFLAGS += -DOS_ABL_OS_RBUS_SUPPORT
endif
ifeq ($(HAS_OSABL_OS_AP_SUPPORT),y)
WFLAGS += -DOS_ABL_OS_AP_SUPPORT
endif
ifeq ($(HAS_OSABL_OS_STA_SUPPORT),y)
WFLAGS += -DOS_ABL_OS_STA_SUPPORT
endif
endif


ifeq ($(HAS_TXRX_SW_ANTDIV_SUPPORT),y)
WFLAGS += -DTXRX_SW_ANTDIV_SUPPORT
endif

ifeq ($(HAS_WIDI_SUPPORT),y)
WFLAGS += -DWIDI_SUPPORT

ifeq ($(HAS_INTEL_L2SD_TOGGLE_SCAN_SUPPORT),y)
WFLAGS += -DINTEL_L2SD_TOGGLE_SCAN_SUPPORT
endif

ifeq ($(HAS_P2P_SUPPORT),y)
ifeq ($(HAS_INTEL_WFD_SUPPORT),y)
WFLAGS += -DINTEL_WFD_SUPPORT
endif

ifeq ($(HAS_WFA_WFD_SUPPORT),y)
WFLAGS += -DWFA_WFD_SUPPORT
endif
endif

endif


ifeq ($(HAS_WOW_IFDOWN_SUPPORT),y)
WFLAGS += -DWOW_IFDOWN_SUPPORT
endif

ifeq ($(HAS_NEW_WOW_SUPPORT),y)
WFLAGS += -DNEW_WOW_SUPPORT
endif

ifeq ($(HAS_MT_WOW_SUPPORT),y)
WFLAGS += -DMT_WOW_SUPPORT
endif

endif
# endif of ifeq ($(RT28xx_MODE),STA)

###########################################################
#
# config for APSTA
#
###########################################################



##########################################################
#
# Common compiler flag
#
##########################################################



ifeq ($(HAS_DOT11R_FT_SUPPORT),y)
WFLAGS += -DDOT11R_FT_SUPPORT
endif

ifeq ($(HAS_DOT11K_RRM_SUPPORT),y)
WFLAGS += -DDOT11K_RRM_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT -DAPPLE_11K_IOT
endif

ifeq ($(HAS_DOT11V_WNM_SUPPORT),y)
WFLAGS += -DCONFIG_DOT11V_WNM
# -DWNM_NEW_API
#-DDOT11V_WNM_SUPPORT
endif

ifeq ($(HAS_EXT_BUILD_CHANNEL_LIST),y)
WFLAGS += -DEXT_BUILD_CHANNEL_LIST
endif

ifeq ($(HAS_IDS_SUPPORT),y)
WFLAGS += -DIDS_SUPPORT
endif


ifeq ($(OSABL),YES)
WFLAGS += -DEXPORT_SYMTAB
endif

ifeq ($(HAS_CLIENT_WDS_SUPPORT),y)
WFLAGS += -DCLIENT_WDS
endif

ifeq ($(HAS_BGFP_SUPPORT),y)
WFLAGS += -DBG_FT_SUPPORT
endif

ifeq ($(HAS_BGFP_OPEN_SUPPORT),y)
WFLAGS += -DBG_FT_OPEN_SUPPORT
endif

ifeq ($(HAS_LED_CONTROL_SUPPORT),y)
WFLAGS += -DLED_CONTROL_SUPPORT
endif

ifeq ($(HAS_HOTSPOT_SUPPORT),y)
WFLAGS += -DCONFIG_DOT11U_INTERWORKING -DCONFIG_DOT11V_WNM -DCONFIG_HOTSPOT -DCONFIG_HOTSPOT_R2
endif

ifeq ($(HAS_DSCP_PRI_SUPPORT),y)
WFLAGS += -DDSCP_PRI_SUPPORT
endif

ifeq ($(HAS_MULTI_CHANNEL),y)
WFLAGS += -DCONFIG_MULTI_CHANNEL
endif

ifeq ($(HAS_MICROWAVE_OVEN_SUPPORT),y)
WFLAGS += -DMICROWAVE_OVEN_SUPPORT
endif


ifeq ($(HAS_DVT_MODE),y)
WFLAGS += -DCONFIG_DVT_MODE
endif

ifeq ($(HAS_KEEP_ALIVE_OFFLOAD),y)
WFLAGS += -DCONFIG_KEEP_ALIVE_OFFLOAD
endif

ifeq ($(HAS_WTBL_TLV_MODE),y)
ifeq ($(CHIPSET),$(filter $(CHIPSET),mt7636u mt7636s mt7615 mt7637u mt7637e mt7637s))
WFLAGS += -DCONFIG_WTBL_TLV_MODE
endif
endif

ifeq ($(HAS_HW_HAL_OFFLOAD),y)
WFLAGS += -DCONFIG_HW_HAL_OFFLOAD
endif

ifeq ($(HAS_SNIFFER_SUPPORT),y)
WFLAGS += -DCONFIG_SNIFFER_SUPPORT -DMONITOR_FLAG_11N_SNIFFER_SUPPORT
ifeq ($(CHIPSET),$(filter $(CHIPSET),mt7615))
# Include -DWIRESHARK_1_12_2_1 for Wireshark-1.12.2.1
WFLAGS += -DSNIFFER_MT7615
#WFLAGS += -DSNIFFER_MT7615 -DSNIFFER_MT7615_RMAC_INC
endif
endif

ifeq ($(HAS_CALIBRATION_COLLECTION_SUPPORT),y)
WFLAGS += -DCONFIG_CALIBRATION_COLLECTION
endif

ifeq ($(HAS_TRACE_SUPPORT),y)
WFLAGS += -DCONFIG_TRACE_SUPPORT
endif

ifeq ($(HAS_FW_DEBUG_SUPPORT),y)
WFLAGS += -DCONFIG_FW_DEBUG
endif

ifeq ($(HAS_WIFI_REGION32_HIDDEN_SSID_SUPPORT),y)
WFLAGS += -DWIFI_REGION32_HIDDEN_SSID_SUPPORT
endif

ifeq ($(HAS_VOW_SUPPORT),y)
WFLAGS += -DVOW_SUPPORT
endif

# Per-clent tail drop(RED)
ifeq ($(HAS_RED_SUPPORT),y)
WFLAGS += -DRED_SUPPORT
endif

#################################################
# ChipSet specific definitions  - MT Series
#
#################################################










ifneq ($(findstring mt7626,$(CHIPSET)),)
WFLAGS += -DMT7626 -DMT_BBP -DMT_RF -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI
#PCI is for debug purpose
WFLAGS += -DRTMP_PCI_SUPPORT
#RBUS is for mt7626
#WFLAGS += -DRTMP_RBUS_SUPPORT
WFLAGS += -DDBDC_MODE
WFLAGS += -DMT_DMAC
WFLAGS += -DMCS_LUT_SUPPORT
WFLAGS += -DRTMP_EFUSE_SUPPORT
WFLAGS += -DBCN_OFFLOAD_SUPPORT
#WFLAGS += -DNEW_SET_RX_STREAM
WFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
WFLAGS += -DUNIFY_FW_CMD
WFLAGS += -DTX_POWER_CONTROL_SUPPORT
#WFLAGS += -DTMR_VERIFY
#WFLAGS += -DNEED_ROM_PATCH
WFLAGS += -DMAC_INIT_OFFLOAD
WFLAGS += -DENHANCED_STAT_DISPLAY
#WFLAGS += -DCONFIG_FWOWN_SUPPORT
WFLAGS += -DNEWSEC
WFLAGS += -DRESOURCE_PRE_ALLOC
WFLAGS += -DCONFIG_HW_HAL_OFFLOAD
WFLAGS += -DCONFIG_FPGA_MODE
#WFLAGS += -DCONFIG_CSO_SUPPORT
WFLAGS += -DPKT_BUDGET_CTRL_SUPPORT
#WFLAGS += -DMSP_UTILITY
WFLAGS += -DHW_TX_AMSDU_SUPPORT
WFLAGS += -DIP_ASSEMBLY

CONFIG_PLATFORM_DRV_SUPPORT=y
ifeq ($(CONFIG_PLATFORM_DRV_SUPPROT),y)
WFLAGS += -DPLATFORM_DRIVER_SUPPORT
endif

WFLAGS += -Werror
ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
        WFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif

ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
        WFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
        WFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
endif

ifeq ($(HAS_TXBF_SUPPORT),y)
WFLAGS += -DTXBF_SUPPORT
ifeq ($(HAS_VHT_TXBF_SUPPORT),y)
WFLAGS += -DVHT_TXBF_SUPPORT
endif
ifeq ($(HAS_STA_ITXBF_SUPPORT), y)
WFLAGS += -DSTA_ITXBF_SUPPORT
endif
endif

HAS_RATE_ADAPT_AGBS_SUPPORT=y
HAS_NEW_RATE_ADAPT_SUPPORT=n
HAS_RACTRL_FW_OFFLOAD_SUPPORT=y
ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
WFLAGS += -DNEW_RATE_ADAPT_SUPPORT
endif

ifeq ($(HAS_RATE_ADAPT_AGBS_SUPPORT),y)
WFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
WFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
endif

WFLAGS += -DSTA_LP_PHASE_2_SUPPORT -DCONFIG_PM_BIT_HW_MODE -DCONFIG_PM

ifeq ($(HAS_CSO_SUPPORT), y)
WFLAGS += -DCONFIG_CSO_SUPPORT
endif

ifeq ($(HAS_FPGA_MODE),y)
WFLAGS += -DP18_FPGA
WFLAGS += -DMT7663_FPGA
WFLAGS += -DMT7626_FPGA
HAS_NEED_ROM_PATCH=n
else
HAS_NEED_ROM_PATCH=y
endif

WFLAGS += -DRELEASE_EXCLUDE

ifeq ($(HAS_FWOWN_SUPPORT),y)
WFLAGS += -DCONFIG_FWOWN_SUPPORT
endif

WFLAGS += -DRTMP_WLAN_HOOK_SUPPORT

HAS_MT_BBP=y
HAS_MT_MAC=y
HAS_INF_PCI=y
HAS_MAC_PCI=y
HAS_MT_MAC=y
HAS_MT_RF=y
HAS_HDR_TRANS_TX_SUPPORT=y
HAS_HDR_TRANS_RX_SUPPORT=y
HAS_CUT_THROUGH=y
HAS_RX_CUT_THROUGH=n
HAS_CUT_THROUGH_DBG=n
HAS_PALLADIUM=n
HAS_FW_OFFLOAD=n
HAS_FW_OFFLOAD_DEV_BSS_STAREC=n
HAS_ERR_RECOVERY=y
HAS_RX_SCATTER=n
HAS_NEW_TLV=n
HAS_DOT11_VHT_SUPPORT=y
HAS_TCP_RACK_SUPPORT=n
HAS_RLM_CAL_CACHE=n
HAS_RF_LOCKDOWN=n

ifeq ($(HAS_RF_LOCKDOWN),y)
WFLAGS += -DRF_LOCKDOWN
endif

ifeq ($(HAS_RLM_CAL_CACHE),y)
WFLAGS += -DRLM_CAL_CACHE_SUPPORT
endif

ifeq ($(HAS_DOT11_VHT_SUPPORT),y)
WFLAGS += -DDOT11_VHT_AC
endif

WFLAGS += -DCONFIG_FWOWN_SUPPORT

ifeq ($(HAS_TCP_RACK_SUPPORT),y)
WFLAGS += -DREDUCE_TCP_ACK_SUPPORT
endif

ifeq ($(HAS_NEED_ROM_PATCH), y)
WFLAGS += -DNEED_ROM_PATCH
endif

CHIPSET_DAT = 2860
endif
# MT7626 end






#################################################
# Platform Related definitions
#
#################################################

ifeq ($(PLATFORM),5VT)
#WFLAGS += -DCONFIG_5VT_ENHANCE
endif

ifeq ($(HAS_MU_MIMO_SUPPORT),y)
WFLAGS += -DCFG_SUPPORT_MU_MIMO
endif

ifeq ($(HAS_FALCON_MURU_SUPPORT),y)
WFLAGS += -DCFG_SUPPORT_FALCON_MURU
endif

ifeq ($(HAS_BLOCK_NET_IF),y)
WFLAGS += -DBLOCK_NET_IF
endif

ifeq ($(HAS_DFS_SUPPORT),y)
WFLAGS += -DDFS_SUPPORT
endif

ifeq ($(HAS_MT_DFS_SUPPORT),y)
WFLAGS += -DMT_DFS_SUPPORT
endif

ifeq ($(HAS_ERR_RECOVERY),y)
WFLAGS += -DERR_RECOVERY
endif

ifeq ($(HAS_RX_SCATTER),y)
WFLAGS += -DRX_SCATTER
endif

ifeq ($(HAS_CUT_THROUGH),y)
WFLAGS += -DCUT_THROUGH -DCUT_THROGH_DBG
#-DFW_OFFLOAD
endif

ifeq ($(HAS_RX_CUT_THROUGH),y)
WFLAGS += -DRX_CUT_THROUGH
endif

ifeq ($(HAS_CUT_THROUGH_DBG),y)
WFLAGS += -DCUT_THROUGH_DBG
endif

ifeq ($(HAS_HDR_TRANS_TX_SUPPORT),y)
WFLAGS += -DHDR_TRANS_TX_SUPPORT
endif

ifeq ($(HAS_HDR_TRANS_RX_SUPPORT),y)
WFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

ifeq ($(HAS_MC_SUPPORT),y)
WFLAGS += -DMULTIPLE_CARD_SUPPORT
endif

ifeq ($(HAS_MEM_ALLOC_INFO_SUPPORT),y)
WFLAGS += -DMEM_ALLOC_INFO_SUPPORT
endif

ifeq ($(HAS_LLTD),y)
WFLAGS += -DLLTD_SUPPORT
endif

ifeq ($(PLATFORM),RMI)
WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),UBICOM_IPX8)
WFLAGS += -DRT_BIG_ENDIAN -DUNALIGNMENT_SUPPORT -DPLATFORM_UBM_IPX8 -DNO_CONSISTENT_MEM_SUPPORT -DCACHE_LINE_32B
endif

ifeq ($(PLATFORM),BL2348)
WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),BL23570)
WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),BLUBB)
WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),BLPMP)
WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),RMI_64)
WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),IXP)
WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),IKANOS_V160)
WFLAGS += -DRT_BIG_ENDIAN -DIKANOS_VX_1X0
endif

ifeq ($(PLATFORM),IKANOS_V180)
WFLAGS += -DRT_BIG_ENDIAN -DIKANOS_VX_1X0
endif

ifeq ($(PLATFORM),INF_TWINPASS)
WFLAGS += -DRT_BIG_ENDIAN -DINF_TWINPASS
endif

ifeq ($(PLATFORM),INF_DANUBE)
ifneq (,$(findstring 2.4,$(LINUX_SRC)))
# Linux 2.4
WFLAGS += -DINF_DANUBE -DRT_BIG_ENDIAN
else
# Linux 2.6
WFLAGS += -DRT_BIG_ENDIAN
endif
endif

ifeq ($(PLATFORM),INF_AR9)
WFLAGS += -DRT_BIG_ENDIAN -DINF_AR9
# support MAPI function for AR9.
#WFLAGS += -DAR9_MAPI_SUPPORT
endif

ifeq ($(PLATFORM),INF_VR9)
WFLAGS += -DRT_BIG_ENDIAN -DINF_AR9 -DINF_VR9
endif

ifeq ($(PLATFORM),CAVM_OCTEON)
WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),BRCM_6358)
WFLAGS += -DRT_BIG_ENDIAN -DBRCM_6358
endif

ifeq ($(PLATFORM),FREESCALE8377)
#EXTRA_CFLAGS := -v -I$(RT28xx_DIR)/include -I$(LINUX_SRC)/include $(WFLAGS)-O2 -Wall -Wstrict-prototypes -Wno-trigraphs
#export EXTRA_CFLAGS
WFLAGS += -DRT_BIG_ENDIAN
EXTRA_CFLAGS := $(WFLAGS) -I$(RT28xx_DIR)/include
endif

ifeq ($(PLATFORM),ST)
WFLAGS += -DST
endif

#kernel build options for 2.4
# move to Makefile outside LINUX_SRC := /opt/star/kernel/linux-2.4.27-star

ifeq ($(PLATFORM),STAR)
CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common -Uarm -fno-common -pipe -mapcs-32 -D__LINUX_ARM_ARCH__=4 -march=armv4  -mshort-load-bytes -msoft-float -Uarm -DMODULE -DMODVERSIONS -include $(LINUX_SRC)/include/linux/modversions.h $(WFLAGS)

export CFLAGS
endif

ifeq ($(PLATFORM),UBICOM_IPX8)
EXTRA_CFLAGS += $(WFLAGS)
export EXTRA_CFLAGS
endif

ifeq ($(PLATFORM),SIGMA)
CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm/gcc -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -DEM86XX_CHIP=EM86XX_CHIPID_TANGO2 -DEM86XX_REVISION=6 -I$(LINUX_SRC)/include/asm-mips/mach-generic -I$(RT2860_DIR)/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2     -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe  -mabi=32 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -DMODULE $(WFLAGS) -DSIGMA863X_PLATFORM

export CFLAGS
endif

ifeq ($(PLATFORM),SIGMA_8622)
CFLAGS := -D__KERNEL__ -I$(CROSS_COMPILE_INCLUDE)/include -I$(LINUX_SRC)/include -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common -fno-common -pipe -fno-builtin -D__linux__ -DNO_MM -mapcs-32 -march=armv4 -mtune=arm7tdmi -msoft-float -DMODULE -mshort-load-bytes -nostdinc -iwithprefix -DMODULE $(WFLAGS)
export CFLAGS
endif

ifeq ($(PLATFORM),5VT)
CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -O3 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=apcs-gnu -mno-thumb-interwork -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=arm926ej-s --param max-inline-insns-single=40000  -Uarm -Wdeclaration-after-statement -Wno-pointer-sign -DMODULE $(WFLAGS)

export CFLAGS
endif

ifeq ($(PLATFORM),IKANOS_V160)
CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm/gcc -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-generic -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2 -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe -march=lx4189 -Wa, -DMODULE $(WFLAGS)
export CFLAGS
endif

ifeq ($(PLATFORM),IKANOS_V180)
CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm/gcc -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-generic -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2 -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe -mips32r2 -Wa, -DMODULE $(WFLAGS)
export CFLAGS
endif

ifeq ($(PLATFORM),INF_TWINPASS)
CFLAGS := -D__KERNEL__ -DMODULE -I$(LINUX_SRC)/include -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fomit-frame-pointer -fno-strict-aliasing -fno-common -G 0 -mno-abicalls -fno-pic -march=4kc -mips32 -Wa,--trap -pipe -mlong-calls $(WFLAGS)
export CFLAGS
endif

ifeq ($(PLATFORM),INF_DANUBE)
	ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	CFLAGS := $(WFLAGS) -Wundef -fno-strict-aliasing -fno-common -ffreestanding -Os -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe -msoft-float  -mabi=32 -march=mips32 -Wa,-32 -Wa,-march=mips32 -Wa,-mips32 -Wa,--trap -I$(LINUX_SRC)/include/asm-mips/mach-generic
	else
	CFLAGS := $(WFLAGS) -Wundef -fno-strict-aliasing -fno-common -ffreestanding -Os -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe -msoft-float  -mabi=32 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -I$(LINUX_SRC)/include/asm-mips/mach-generic
	endif
export CFLAGS
endif

ifeq ($(PLATFORM),INF_AR9)
CFLAGS := $(WFLAGS) -Wundef -fno-strict-aliasing -fno-common -fno-pic -ffreestanding -Os -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe -msoft-float  -mabi=32 -mlong-calls -march=mips32r2 -mtune=34kc -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -I$(LINUX_SRC)/include/asm-mips/mach-generic
export CFLAGS
endif

ifeq ($(PLATFORM),INF_VR9)
CFLAGS := $(WFLAGS) -Wundef -fno-strict-aliasing -fno-common -fno-pic -ffreestanding -Os -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe -msoft-float  -mabi=32 -mlong-calls -march=mips32r2 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -I$(LINUX_SRC)/include/asm-mips/mach-generic
export CFLAGS
endif

ifeq ($(PLATFORM),BRCM_6358)
CFLAGS := $(WFLAGS) -nostdinc -iwithprefix include -D__KERNEL__ -Wall -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -I $(LINUX_SRC)/include/asm/gcc -G 0 -mno-abicalls -fno-pic -pipe  -finline-limit=100000 -mabi=32 -march=mips32 -Wa,-32 -Wa,-march=mips32 -Wa,-mips32 -Wa,--trap -I$(LINUX_SRC)/include/asm-mips/mach-bcm963xx -I$(LINUX_SRC)/include/asm-mips/mach-generic  -Os -fomit-frame-pointer -Wdeclaration-after-statement  -DMODULE -mlong-calls
export CFLAGS
endif

ifeq ($(PLATFORM),ST)
CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -Wall -O2 -Wundef -Wstrict-prototypes -Wno-trigraphs -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-aliasing -fno-common -fomit-frame-pointer -ffreestanding -m4-nofpu -o $(WFLAGS)
export CFLAGS
endif

ifeq ($(PLATFORM),PC)
    ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	# Linux 2.4
	CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -O2 -fomit-frame-pointer -fno-strict-aliasing -fno-common -pipe -mpreferred-stack-boundary=2 -march=i686 -DMODULE -DMODVERSIONS -include $(LINUX_SRC)/include/linux/modversions.h $(WFLAGS)
	export CFLAGS
    else
	# Linux 2.6
	EXTRA_CFLAGS := $(WFLAGS)
    endif
endif

ifeq ($(PLATFORM),INTELP6)
	EXTRA_CFLAGS := $(WFLAGS)
endif

#If the kernel version of RMI is newer than 2.6.27, please change "CFLAGS" to "EXTRA_FLAGS"
ifeq ($(PLATFORM),RMI)
EXTRA_CFLAGS := -D__KERNEL__ -DMODULE=1 -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm-mips/mach-generic -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common -DCONFIG_IFX_ALG_QOS -DCONFIG_WAN_VLAN_SUPPORT -fomit-frame-pointer -DIFX_PPPOE_FRAME -G 0 -fno-pic -mno-abicalls -mlong-calls -pipe -finline-limit=100000 -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -march=xlr -ffreestanding  -march=xlr -Wa,--trap, -nostdinc -iwithprefix include $(WFLAGS)
export EXTRA_CFLAGS
endif

ifeq ($(PLATFORM),RMI_64)
EXTRA_CFLAGS := -D__KERNEL__ -DMODULE=1 -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm-mips/mach-generic -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common -DCONFIG_IFX_ALG_QOS -DCONFIG_WAN_VLAN_SUPPORT -fomit-frame-pointer -DIFX_PPPOE_FRAME -G 0 -fno-pic -mno-abicalls -mlong-calls -pipe -finline-limit=100000 -mabi=64 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -march=xlr -ffreestanding  -march=xlr -Wa,--trap, -nostdinc -iwithprefix include $(WFLAGS)
export EXTRA_CFLAGS
endif

ifeq ($(PLATFORM),IXP)
	CFLAGS := -v -D__KERNEL__ -DMODULE -I$(LINUX_SRC)/include -mbig-endian -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common -Uarm -fno-common -pipe -mapcs-32 -D__LINUX_ARM_ARCH__=5 -mcpu=xscale -mtune=xscale -malignment-traps -msoft-float $(WFLAGS)
        EXTRA_CFLAGS := -v $(WFLAGS) -mbig-endian
	export CFLAGS
endif

ifeq ($(PLATFORM),SMDK)
        EXTRA_CFLAGS := $(WFLAGS)
endif

ifeq ($(PLATFORM),CAVM_OCTEON)
	EXTRA_CFLAGS := $(WFLAGS) -mabi=64 $(WFLAGS)
export CFLAGS
endif

ifeq ($(PLATFORM),BB_SOC)
WFLAGS += -DRT_BIG_ENDIAN -DBB_SOC -DWIFI_MODULE -DCONFIG_RA_NAT_NONE
ifeq ($(CONFIG_MIPS_TC3262),y)
WFLAGS += -DMULTI_CORE_SUPPORT
endif
ifeq ($(RALINK_PCIE_ADDR_SWAP),1)
WFLAGS += -DBB_PCIE_ADDR_SWAP
endif
ifeq ($(TCSUPPORT_HWNAT_WIFI),1)
WFLAGS += -DBB_RA_HWNAT_WIFI
endif
EXTRA_CFLAGS += -I$(RT28xx_DIR)/include $(WFLAGS)
export EXTRA_CFLAGS
endif

ifeq ($(PLATFORM),DM6446)
	CFLAGS := -nostdinc -iwithprefix include -D__KERNEL__ -I$(LINUX_SRC)/include  -Wall -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Os -fno-omit-frame-pointer -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mlittle-endian -mabi=apcs-gnu -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=arm9tdmi -msoft-float -Uarm -Wdeclaration-after-statement -c -o $(WFLAGS)
export CFLAGS
endif

ifeq ($(PLATFORM),BL2348)
CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm/gcc -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -DEM86XX_CHIP=EM86XX_CHIPID_TANGO2 -DEM86XX_REVISION=6 -I$(LINUX_SRC)/include/asm-mips/mach-generic -I$(RT2860_DIR)/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2     -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe  -mabi=32 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -DMODULE $(WFLAGS) -DSIGMA863X_PLATFORM -DEXPORT_SYMTAB -DPLATFORM_BL2348
export CFLAGS
endif

ifeq ($(PLATFORM),BL23570)
EXTRA_CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm/gcc -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -DEM86XX_CHIP=EM86XX_CHIPID_TANGO2 -DEM86XX_REVISION=6 -I$(LINUX_SRC)/include/asm-mips/mach-generic -I$(RT2860_DIR)/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2     -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe  -mabi=32 -march=74kc -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -DMODULE $(WFLAGS) -DSIGMA863X_PLATFORM -DEXPORT_SYMTAB -DPLATFORM_BL23570
export EXTRA_CFLAGS
endif

ifeq ($(PLATFORM),BLUBB)
CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm/gcc -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -DEM86XX_CHIP=EM86XX_CHIPID_TANGO2 -DEM86XX_REVISION=6 -I$(LINUX_SRC)/include/asm-mips/mach-generic -I$(RT2860_DIR)/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2     -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe  -mabi=32 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -DMODULE $(WFLAGS) -DSIGMA863X_PLATFORM -DEXPORT_SYMTAB -DPLATFORM_BL2348
export CFLAGS
endif

ifeq ($(PLATFORM),BLPMP)
CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm/gcc -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -DEM86XX_CHIP=EM86XX_CHIPID_TANGO2 -DEM86XX_REVISION=6 -I$(LINUX_SRC)/include/asm-mips/mach-generic -I$(RT2860_DIR)/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2     -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe  -mabi=32 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -DMODULE $(WFLAGS) -DSIGMA863X_PLATFORM -DEXPORT_SYMTAB
export CFLAGS
endif

ifeq ($(PLATFORM),MT85XX)
    ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	# Linux 2.4
	CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -O2 -fomit-frame-pointer -fno-strict-aliasing -fno-common -pipe -mpreferred-stack-boundary=2 -march=i686 -DMODULE -DMODVERSIONS -include $(LINUX_SRC)/include/linux/modversions.h $(WFLAGS)
	export CFLAGS
    else
	# Linux 2.6
	EXTRA_CFLAGS += $(WFLAGS)
	EXTRA_CFLAGS += -D _NO_TYPEDEF_BOOL_ \
	                -D _NO_TYPEDEF_UCHAR_ \
	                -D _NO_TYPEDEF_UINT8_ \
	                -D _NO_TYPEDEF_UINT16_ \
	                -D _NO_TYPEDEF_UINT32_ \
	                -D _NO_TYPEDEF_UINT64_ \
	                -D _NO_TYPEDEF_CHAR_ \
	                -D _NO_TYPEDEF_INT32_ \
	                -D _NO_TYPEDEF_INT64_ \

    endif
endif

ifeq ($(PLATFORM),NXP_TV550)
    ifneq (,$(findstring 2.4,$(LINUX_SRC)))
        # Linux 2.4
        CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -O2 -fomit-frame-pointer -fno-strict-aliasing -fno-common -pipe -mpreferred-stack-boundary=2 -march=mips -DMODULE -DMODVERSIONS -include $(LINUX_SRC)/include/linux/modversions.h $(WFLAGS)
        export CFLAGS
    else
        # Linux 2.6
        EXTRA_CFLAGS := $(WFLAGS)
    endif
endif

ifeq ($(PLATFORM),MVL5)
CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -O3 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mno-thumb-interwork -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=arm926ej-s --param max-inline-insns-single=40000  -Uarm -Wdeclaration-after-statement -Wno-pointer-sign -DMODULE $(WFLAGS)
export CFLAGS
endif

ifeq ($(PLATFORM),MT7620)
        EXTRA_CFLAGS := $(WFLAGS)
endif

ifeq ($(PLATFORM),MT7621)
        EXTRA_CFLAGS := $(WFLAGS)
endif

ifeq ($(PLATFORM),MT7628)
        EXTRA_CFLAGS := $(WFLAGS)
endif

ifeq ($(PLATFORM),MT7622)
        EXTRA_CFLAGS := $(WFLAGS)
endif
