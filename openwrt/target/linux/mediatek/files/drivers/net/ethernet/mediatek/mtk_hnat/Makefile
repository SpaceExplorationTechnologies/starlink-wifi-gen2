ccflags-y=-Werror

obj-$(CONFIG_NET_MEDIATEK_HNAT)         += mtkhnat.o
mtkhnat-objs := hnat.o hnat_nf_hook.o hnat_debugfs.o hnat_mcast.o
mtkhnat-$(CONFIG_NET_DSA_MT7530)	+= hnat_stag.o
mtkhnat-$(CONFIG_NET_MEDIATEK_HW_QOS)	+= hnat_sfq.o
