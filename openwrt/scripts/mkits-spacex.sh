#!/usr/bin/env bash
# U-Boot firmware supports the booting of images in the Flattened Image
# Tree (FIT) format.  The FIT format uses a device tree structure to
# describe a kernel image, device tree blob, ramdisk, etc.  This script
# creates an Image Tree Source (.its file) which can be passed to the
# 'mkimage' utility to generate an Image Tree Blob (.itb file).  The .itb
# file can then be booted by U-Boot (or other bootloaders which support
# FIT images).  See doc/uImage.FIT/howto.txt in U-Boot source code for
# additional information on FIT images.
#
# SpaceX: Made a simpler version of the same script, mkits. We need to add
# addtional DTS's and scripts, so this was easier. Preserving the same interface
# though, with some light modifications for easier porting.

usage() {
	echo "Usage: `basename $0` -A arch -C comp -a addr -e entry" \
	     "-v version -k kernel [-D name -d dtb] -o its_file" \
	     "[-s script] [-S key_name_hint] [-r ar_ver]"
	echo -e "\t-A ==> set architecture to 'arch'"
	echo -e "\t-C ==> set compression type 'comp'"
	echo -e "\t-a ==> set load address to 'addr' (hex)"
	echo -e "\t-e ==> set entry point to 'entry' (hex)"
	echo -e "\t-v ==> set kernel version to 'version'"
	echo -e "\t-k ==> include kernel image 'kernel'"
	echo -e "\t-D ==> human friendly Device Tree Blob 'name'"
	echo -e "\t-d ==> include Device Tree Blob Dev 0'dtbdev0'"
	echo -e "\t-b ==> include Device Tree Blob Dev 1'dtbdev1'"
	echo -e "\t-t ==> include Device Tree Blob Prod 0'dtbprod0'"
	echo -e "\t-T ==> include Device Tree Blob Prod 1'dtbprod1'"
	echo -e "\t-o ==> create output file 'its_file'"
	echo -e "\t-s ==> include u-boot script 1'script1'"
	echo -e "\t-u ==> include u-boot script 2'script2'"
	echo -e "\t-S ==> add signature at configurations and assign its key_name_hint by 'key_name_hint'"
	echo -e "\t-r ==> set anti-rollback version to 'fit_ar_ver' (dec)"
	exit 1
}

while getopts ":A:a:C:D:d:b:t:T:e:k:o:v:s:S:r:b:u:" OPTION
do
	case $OPTION in
		A ) ARCH=$OPTARG;;
		a ) LOAD_ADDR=$OPTARG;;
		C ) COMPRESS=$OPTARG;;
		D ) DEVICE=$OPTARG;;
		d ) DTB_DEV_0=$OPTARG;;
		b ) DTB_DEV_1=$OPTARG;;
		t ) DTB_PROD_0=$OPTARG;;
		T ) DTB_PROD_1=$OPTARG;;
		e ) ENTRY_ADDR=$OPTARG;;
		k ) KERNEL=$OPTARG;;
		o ) OUTPUT=$OPTARG;;
		v ) VERSION=$OPTARG;;
		s ) UBOOT_SCRIPT_0=$OPTARG;;
		u ) UBOOT_SCRIPT_1=$OPTARG;;
		S ) KEY_NAME_HINT=$OPTARG;;
		r ) AR_VER=$OPTARG;;
		* ) echo "Invalid option passed to '$0' (options:$@)"
		usage;;
	esac
done

# Make sure user entered all required parameters
if [ -z "${ARCH}" ] || [ -z "${COMPRESS}" ] || [ -z "${LOAD_ADDR}" ] || \
   [ -z "${ENTRY_ADDR}" ] || [ -z "${VERSION}" ] || [ -z "${KERNEL}" ] || \
   [ -z "${OUTPUT}" ]; then
	usage
fi

ARCH_UPPER=`echo $ARCH | tr '[:lower:]' '[:upper:]'`

# Create a default, fully populated DTS file
DATA="/dts-v1/;

/ {
	description = \"${ARCH_UPPER} OpenWrt FIT (Flattened Image Tree)\";
	#address-cells = <1>;

	images {
		kernel@0 {
			description = \"${ARCH_UPPER} OpenWrt Linux-${VERSION}\";
			data = /incbin/(\"${KERNEL}\");
			type = \"kernel\";
			arch = \"${ARCH}\";
			os = \"linux\";
			compression = \"${COMPRESS}\";
			load = <${LOAD_ADDR}>;
			entry = <${ENTRY_ADDR}>;
			hash@1 {
				algo = \"sha256\";
			};
		};

		fdt@dev0 {
			description = \"${ARCH_UPPER} OpenWrt ${DEVICE} device tree blob dev0\";
			data = /incbin/(\"${DTB_DEV_0}\");
			type = \"flat_dt\";
			arch = \"${ARCH}\";
			compression = \"none\";
			hash@1 {
				algo = \"sha256\";
			};
		};

		fdt@prod0 {
			description = \"${ARCH_UPPER} OpenWrt ${DEVICE} device tree blob prod0\";
			data = /incbin/(\"${DTB_PROD_0}\");
			type = \"flat_dt\";
			arch = \"${ARCH}\";
			compression = \"none\";
			hash@1 {
				algo = \"sha256\";
			};
		};

		fdt@dev1 {
			description = \"${ARCH_UPPER} OpenWrt ${DEVICE} device tree blob dev1\";
			data = /incbin/(\"${DTB_DEV_1}\");
			type = \"flat_dt\";
			arch = \"${ARCH}\";
			compression = \"none\";
			hash@1 {
				algo = \"sha256\";
			};
		};

		fdt@prod1 {
			description = \"${ARCH_UPPER} OpenWrt ${DEVICE} device tree blob prod1\";
			data = /incbin/(\"${DTB_PROD_1}\");
			type = \"flat_dt\";
			arch = \"${ARCH}\";
			compression = \"none\";
			hash@1 {
				algo = \"sha256\";
			};
		};

		script@0 {
			description = \"U-Boot Script\";
			data = /incbin/(\"${UBOOT_SCRIPT_0}\");
			type = \"script\";
			arch = \"${ARCH}\";
			os = \"linux\";
			load = <0>;
			entry = <0>;
			compression = \"none\";
			hash@1 {
				algo = \"sha256\";
			};
		};

		script@1 {
			description = \"U-Boot Script\";
			data = /incbin/(\"${UBOOT_SCRIPT_1}\");
			type = \"script\";
			arch = \"${ARCH}\";
			os = \"linux\";
			load = <0>;
			entry = <0>;
			compression = \"none\";
			hash@1 {
				algo = \"sha256\";
			};
		};
	};

	configurations {
		default = \"config@dev0\";
		config@dev0 {
			description = \"OpenWrt running in chain 0dev\";
			loadables = \"script@0\";
			kernel = \"kernel@0\";
			fdt = \"fdt@dev0\";
			signature {
				algo = \"sha256,rsa2048\";
				key-name-hint = \"${KEY_NAME_HINT}\";
				sign-images = \"fdt\", \"kernel\", \"loadables\";\
			};
		};
		config@prod0 {
			description = \"OpenWrt running in chain 0prod\";
			loadables = \"script@0\";
			kernel = \"kernel@0\";
			fdt = \"fdt@prod0\";
			signature {
				algo = \"sha256,rsa2048\";
				key-name-hint = \"${KEY_NAME_HINT}\";
				sign-images = \"fdt\", \"kernel\", \"loadables\";\
			};
		};
		config@dev1 {
			description = \"OpenWrt running in chain 1dev\";
			loadables = \"script@1\";
			kernel = \"kernel@0\";
			fdt = \"fdt@dev1\";
			signature {
				algo = \"sha256,rsa2048\";
				key-name-hint = \"${KEY_NAME_HINT}\";
				sign-images = \"fdt\", \"kernel\", \"loadables\";\
			};
		};
		config@prod1 {
			description = \"OpenWrt running in chain 1prod\";
			loadables = \"script@1\";
			kernel = \"kernel@0\";
			fdt = \"fdt@prod1\";
			signature {
				algo = \"sha256,rsa2048\";
				key-name-hint = \"${KEY_NAME_HINT}\";
				sign-images = \"fdt\", \"kernel\", \"loadables\";\
			};
		};
	};
};"

# Write .its file to disk
echo "$DATA" > ${OUTPUT}
