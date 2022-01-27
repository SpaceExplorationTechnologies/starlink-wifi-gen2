ifeq ("$(IS_LSDK)","y")
    #LSDK
    MAP_DAEMON_LIBRARY_PATH = $(ROOTDIR)/lib/libmapd
else
    #OpenWRT
    include $(TOPDIR)/rules.mk
    MAP_DAEMON_LIBRARY_PATH = $(BUILD_DIR)/libmapd
endif

OBJ_IOCTL = ioctl_helper.o
OBJ_MAP = map_helper.o
LDFLAGS_MAP = -L$(MAP_DAEMON_LIBRARY_PATH) -lmapd_interface_client
INCLUDES_MAP = -I$(MAP_DAEMON_LIBRARY_PATH)/include
CFLAGS += -I.
CFLAGS += -I$(ROOTDIR)/user/luci/lua-5.1.5/src
CFLAGS += -Wall -shared -fPIC
LUCI_APP_MTK_TARGET = ioctl_helper

ifeq ("$(IS_LSDK)","y")
    ifeq ("$(CONFIG_LIB_MAPD_INTERFACE)","y")
        ifneq ("$(wildcard $(MAP_DAEMON_LIBRARY_PATH))","")
            LUCI_APP_MTK_TARGET += map_helper
        endif
    endif
else
    ifeq ("$(CONFIG_PACKAGE_libmapd)","y")
        ifneq ("$(wildcard $(MAP_DAEMON_LIBRARY_PATH))","")
            LUCI_APP_MTK_TARGET += map_helper
        endif
    endif
endif

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(LUCI_APP_MTK_TARGET)

ioctl_helper: $(OBJ_IOCTL)
	$(CC) -o $@.so $^ $(CFLAGS)

map_helper: $(OBJ_MAP)
	$(CC) -o $@.so $^ $(CFLAGS) $(INCLUDES_MAP) $(LDFLAGS_MAP)

map_helper.o: map_helper.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES_MAP) $(LDFLAGS_MAP)

compile: $(LUCI_APP_MTK_TARGET)

install: compile
	mkdir -p $(DESTDIR)/usr/lib/lua
ifeq ("$(LUCI_APP_MTK_TARGET)","ioctl_helper")
	cp -pR ioctl_helper.so $(DESTDIR)/usr/lib/lua/
else
	cp -pR ioctl_helper.so map_helper.so $(DESTDIR)/usr/lib/lua/
endif

clean:
	rm -f *.o *.so

romfs:
