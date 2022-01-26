################################################################
# STA Common Feature Selection
################################################################
#ifdef MT76XX_BT_COEXISTENCE_SUPPORT
HAS_MT76XX_BT_COEXISTENCE_SUPPORT=n
#endif /* MT76XX_BT_COEXISTENCE_SUPPORT */

#ifdef HAS_MT_MAC_BT_COEXISTENCE_SUPPORT
HAS_MT_MAC_BT_COEXISTENCE_SUPPORT=n
#endif /* HAS_MT_MAC_BT_COEXISTENCE_SUPPORT */







# Assign specified profile path when insmod
HAS_PROFILE_DYN_SUPPORT=n

#Support ANDROID_SUPPORT (2.X with WEXT-based)
HAS_ANDROID_SUPPORT=n

#HAS_IFUP_IN_PROBE_SUPPORT
HAS_IFUP_IN_PROBE_SUPPORT=n

#Support for USB_SUPPORT_SELECTIVE_SUSPEND
HAS_USB_SUPPORT_SELECTIVE_SUSPEND=n

#Support USB load firmware by multibyte
HAS_USB_FIRMWARE_MULTIBYTE_WRITE=n


################################################################
# STA Customized Feature Selection
################################################################
#ifdef XLINK_SUPPORT
# Support XLINK mode
HAS_XLINK=n
#endif /* XLINK_SUPPORT */


################################################################
# STA Feature Compiler Flag
################################################################
WFLAGS += -DCONFIG_STA_SUPPORT -DSCAN_SUPPORT

#ifdef WSC_INCLUDED
ifeq ($(HAS_WSC),y)
WFLAGS += -DWSC_STA_SUPPORT -DWSC_V2_SUPPORT
ifeq ($(HAS_WSC_LED),y)
WFLAGS += -DWSC_LED_SUPPORT
endif
ifeq ($(HAS_IWSC_SUPPORT),y)
WFLAGS += -DIWSC_SUPPORT
endif
endif
#endif /* WSC_INCLUDED */

#ifdef MT76XX_BT_COEXISTENCE_SUPPORT
ifeq ($(HAS_MT76XX_BT_COEXISTENCE_SUPPORT),y)
WFLAGS += -DMT76XX_BTCOEX_SUPPORT
endif
#endif /* MT76XX_BT_COEXISTENCE_SUPPORT */

#ifdef HAS_MT_MAC_BT_COEXISTENCE_SUPPORT
ifeq ($(HAS_MT_MAC_BT_COEXISTENCE_SUPPORT),y)
WFLAGS += -DMT_MAC_BTCOEX
endif
#endif /* HAS_MT_MAC_BT_COEXISTENCE_SUPPORT */







ifeq ($(HAS_PROFILE_DYN_SUPPORT),y)
WFLAGS += -DPROFILE_PATH_DYNAMIC
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

#ifdef XLINK_SUPPORT
ifeq ($(HAS_XLINK),y)
WFLAGS += -DXLINK_SUPPORT
endif
#endif /* XLINK_SUPPORT */

