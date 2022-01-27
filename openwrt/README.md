# SpaceX OpenWRT Source for MT7629 Platform

## Build

### Prequisites: Build Payload
First build `wifi_control` from within `payload`:
``` sh
bazel build --linkopt=-Wl,--strip-all --config=armv7l --compilation_mode=opt //spacex/ux/wifi/wifi_control:wifi_control.stripped
```
Then build an image:
```sh
PAYLOAD=path/to/payload NO_CLEAN=1 scripts/build.sh
```
For convenience, we also cache `payload` within this repository so people can quickly build images without needing `payload`. To update this cache run:
```sh
PAYLOAD=path/to/payload COMMIT=1 scripts/inject_payload.sh
```

### Docker: Preferred Method
To do a full build inside of docker. This takes awhile because
we always use a clean build.
``` sh
sudo ./scripts/build_openwrt_docker.sh -t dev -p <path/to/payload/repo>
```
Image will be located at head of repository.

To make subsequent builds, you can use the --noclean flag to do an incremental build
This won't be a reproducible, so it is not recomended if you are a robot.

``` sh
sudo ./scripts/build_openwrt_docker.sh -t dev -p <path/to/payload/repo> --noclean
```

### Manual
See docker/DOCKERFILE for a list of dependenices to install on your
linux machine. 

``` sh
scripts/inject_wifi_payload.sh <path/to/payload/repo>
cd starlink-wifi-mtk/openwrt/
cp spacex-mtk-openwrt.config .config
make V=s -j4
```

The image is located at:
`bin/targets/mediatek/mt7629/lede-mediatek-mt7629-xxx-squashfs-sysupgrade.bin`

## Menuconfig options
Orginally set from guidance from MediaTek, will be adjusted for SpaceX later.

Target System (MediaTek ARM SOC)
Subtarget (MT7629 based boards)
Target Profile (MTK7629 RFB)

## Make Tips
Builds will take the longest the first time, subsequent runs are faster
because OpenWrt will not recompile everything.

* `make` standard build
* `make V=s` standard build, verbose log
* `make V=s -j4` build but compile on 4 cores for moar speed.

### Partial build
This won't update the final firmware until you run a full make again.
* `make target/linux/compile` just rebuild the kernel
* `make package/<package path>/{prepare, compile, install}` build a package

### Purify
Start from scratch.
* `make clean` to get a clean build
* `make distclean` to get a _very_ clean build (Rebuilds toolchain too)

## Adding packages
All OpenWRT driver software source code is in `/openwrt/dl` in zipped files. The current build system unpacks these zipped files and copies them to the build directory. To modify source code for a given OpenWRT application, unpack the zipped files in `/openwrt/dl` and move the contents into a `/src` directory you create in `/openwrt/package/mtk/applications/[OpenWRT application name]/src`. Anything in this directory will be copied into the build directory on top of the whatever was first copied over from the unpacked zipped files in `/openwrt/dl`.

To fully remove the use of zipped files in `/openwrt/dl`, you can delete the zipped file and also modify the Makefile in `/openwrt/package/mtk/applications/[OpenWRT application name]` to remove the reference to the zipped file, e.g.:

```
include $(TOPDIR)/rules.mk

PKG_NAME:=mapd
PKG_REVISION:=7255d8b3
PKG_SOURCE:=$(PKG_NAME)-$(PKG_REVISION).tar.bz2
PKG_RELEASE:=1
```

### Flashing over JTAG
In order, install:
1. [libusb](https://libusb.info) with `sudo apt install libusb-1.0-0-dev`
2. [Segger J-Link Software](https://www.segger.com/downloads/jlink).
3. [OpenOCD](https://github.com/ntfreak/openocd) 0.11 or greater from source.

Connect a Segger J-Link and boot a minimal bootloader over JTAG:
```sh
cd openocd
openocd -f mt7629_boot.cfg
```

From there, flash the desired image as normal.