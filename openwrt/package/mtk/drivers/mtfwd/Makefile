# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mtfwd
PKG_REVISION:=2aab1d9b
PKG_SOURCE:=$(PKG_NAME)-$(PKG_REVISION).tar.bz2
#PKG_VERSION:=1.0

include $(INCLUDE_DIR)/package.mk


define KernelPackage/mtfwd
  CATEGORY:=MTK Properties
  TITLE:=MTK forward module
  FILES:=$(PKG_BUILD_DIR)/mtfwd.ko
  AUTOLOAD:=$(call AutoProbe,mtfwd)
  SUBMENU:=Drivers
  KCONFIG:=CONFIG_MTFWD_SUPPORT=y
endef


EXTRA_KCONFIG:= \
	CONFIG_MTFWD_SUPPORT=m


EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG))))

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		$(EXTRA_KCONFIG) \
		modules
endef

$(eval $(call KernelPackage,mtfwd))
