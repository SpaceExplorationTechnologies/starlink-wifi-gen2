#!/bin/sh
#
# There are 2 env-variables set for you, you can use it in your script.
# ${BUILD_DIR} , working dir of this script, eg: openwrt/lede/
# ${INSTALL_DIR}, where to install your build result, including: image, build log.
#

#Global variable
BUILD_TIME=`date +%Y%m%d%H%M%S`
build_flag=0
recover_list=

if [ -z ${BUILD_DIR} ]; then
	LOCAL=1
	BUILD_DIR=`pwd`
fi


if [ -z ${INSTALL_DIR} ]; then
	INSTALL_DIR=autobuild_release
	mkdir -p ${INSTALL_DIR}
	if [ ! -d target/linux ]; then
		echo "You should call this scripts from lede's root directory."
	fi
fi

clean() {
	echo "clean start!"
	echo "It will take some time ......"
	mv dl dl_tmp
	make distclean
	mv dl_tmp dl
	rm -rf ${INSTALL_DIR}
	echo "clean done!"
}

copy_main_Config() {
	echo cp -rfa autobuild/$1/.config ./.config
	cp -rfa autobuild/$1/.config ./.config
}


copy_additional_Config() {
	if [ -f autobuild/$1/$4 ]; then
		
		echo cp -rfa autobuild/$1/$4 ./target/linux/$3/mt${2}/$4
		cp -rfa autobuild/$1/$4 ./target/linux/$3/mt${2}/$4
		recover_list=./target/linux/$3/mt${2}/$4
	fi
}

copy_target_makefile() {
	if [ -f autobuild/$1/target.mk ]; then
		echo cp -rfa autobuild/$1/target.mk ./target/linux/mediatek/mt${2:0:4}/target.mk
		cp -rfa autobuild/$1/target.mk ./target/linux/mediatek/mt${2:0:4}/target.mk
	fi

	if [ -f autobuild/$1/mt${2:0:4}.mk ]; then
		echo cp -rfa autobuild/$1/mt${2:0:4}.mk ./target/linux/mediatek/image/mt${2:0:4}.mk
		cp -rfa autobuild/$1/mt${2:0:4}.mk ./target/linux/mediatek/image/mt${2:0:4}.mk
	fi
}

copy_generic_patch() {
	if [ -d autobuild/$1/target/linux/generic/patches-4.4 ]; then
		cp autobuild/$1/target/linux/generic/patches-4.4/* ./target/linux/generic/patches-4.4/
	fi
	if [ -d autobuild/$1/include ]; then
		cp autobuild/$1/include/* ./include/
	fi
	if [ -d autobuild/$1/target/linux/$2/patches-4.4 ]; then
		cp autobuild/$1/target/linux/$2/patches-4.4/* ./target/linux/$2/patches-4.4/
	fi
	if [ -d autobuild/$1/wifi-profile ]; then
		cp -r autobuild/$1/wifi-profile/* ./package/mtk/drivers/wifi-profile
	fi
	cp -fpR autobuild/$1/target/ ./
}

install_output_Image() {
	mkdir -p ${INSTALL_DIR}/$1

	bin_files=`find bin/targets/$3/*${2}* -name "*.bin"`
	file_count=0

	for file in $bin_files
	do
		tmp=${file%.*}
		cp -rf $file ${INSTALL_DIR}/$1/${tmp##*/}-${BUILD_TIME}.bin
		((file_count++))
        done

	if [ ${file_count} = 0 ]; then
		if [ ${build_flag} -eq 0 ]; then
			let  build_flag+=1
			echo " Restart to debug-build with "make V=s -j1", starting......"
			build $1 -j1 || [ "$LOCAL" != "1" ]
		else
			echo " **********Failed to build $1, bin missing.**********"
		fi
	else
		echo "Install image OK!!!"
		echo "Build $1 successfully!"
	fi
}

install_output_Config() {
	echo cp -rfa autobuild/$1/.config ${INSTALL_DIR}/$1/lede.config
	cp -rfa autobuild/$1/.config ${INSTALL_DIR}/$1/lede.config
	[ -f tmp/kernel.config ] && cp tmp/kernel.config ${INSTALL_DIR}/$1/kernel.config
}

install_output_KernelDebugFile() {
	KernelDebugFile=bin/targets/$3/mt${2}*/kernel-debug.tar.bz2
	if [ -f ${KernelDebugFile} ]; then
		echo cp -rfa ${KernelDebugFile} ${INSTALL_DIR}/$1/kernel-debug.tar.bz2
		cp -rfa ${KernelDebugFile} ${INSTALL_DIR}/$1/kernel-debug.tar.bz2
	fi
}

install_output_RootfsDebugFile() {
	STAGING_DIR_ROOT=$(make -f "autobuild/get_stagingdir_root.mk" get-staging-dir-root)
	if [ -d ${STAGING_DIR_ROOT} ]; then
		STAGING_DIR_ROOT_PREFIX=$(dirname ${STAGING_DIR_ROOT})
		STAGING_DIR_ROOT_NAME=$(basename ${STAGING_DIR_ROOT})
		echo "tar -jcf ${INSTALL_DIR}/$1/rootfs-debug.tar.bz2 -C \"$STAGING_DIR_ROOT_PREFIX\" \"$STAGING_DIR_ROOT_NAME\""
		tar -jcf ${INSTALL_DIR}/$1/rootfs-debug.tar.bz2 -C "$STAGING_DIR_ROOT_PREFIX" "$STAGING_DIR_ROOT_NAME"
	fi
}

recover_Files() {
	git checkout $1
}


build() {
	echo "###############################################################################"
	echo "# $1"
	echo "###############################################################################"
	echo "build $1"

	echo ln -s ../dl dl
	ln -s ../dl dl

	[ -f autobuild/$1/.config ] || {
		echo "unable to locate autobuild/$1/.config !"
		return
	}

	temp=${1#*mt}
	chip_name=${temp:0:4}

	temp1=`grep "CONFIG_TARGET_ramips=y" autobuild/$1/.config`
	if [ "${temp1}" == "CONFIG_TARGET_ramips=y" ]; then
		arch_name="ramips"
	else
		arch_name="mediatek"
	fi
	kernel_config_file="config-4.4"

	#copy main test config(.config)
	copy_main_Config $1

	#copy additional test config "if exit", and add it to "recover_list"
	copy_additional_Config $1 ${chip_name} ${arch_name} ${kernel_config_file}

	#copy additional target Makefile "if exist"
	copy_target_makefile $1 ${temp}

	copy_generic_patch $1 ${arch_name}

	echo make defconfig
	make defconfig

    	#make
	echo make V=s $2
	make V=s $2

	#install output image
	install_output_Image $1 ${chip_name} ${arch_name}

	#install output config
	install_output_Config $1

	#install output Kernel-Debug-File
	install_output_KernelDebugFile $1 ${chip_name} ${arch_name}

	#tar unstripped rootfs for debug symbols
	install_output_RootfsDebugFile $1

	#recover files
	#if [ -n "${recover_list}" ]; then
	#	recover_Files ${recover_list}
	#fi
}
