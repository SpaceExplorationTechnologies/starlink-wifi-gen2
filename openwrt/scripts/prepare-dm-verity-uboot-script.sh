#!/bin/bash

ROOT_DEVICE=$1
EXTRA_ARGS=$2

while read line; do
	key=$(echo ${line} | cut -f1 -d':')
	value=$(echo ${line} | cut -f2 -d':')

	case "${key}" in
	"UUID")
		UUID=${value}
		;;
	"Data blocks")
		DATA_BLOCKS=${value}
		;;
	"Data block size")
		DATA_BLOCK_SIZE=${value}
		;;
	"Hash block size")
		HASH_BLOCK_SIZE=${value}
		;;
	"Hash algorithm")
		HASH_ALG=${value}
		;;
	"Salt")
		SALT=${value}
		;;
	"Root hash")
		ROOT_HASH=${value}
		;;
	esac
done

#
# dm=<name>,<uuid>,<mode>,<num>:
#    <start>,<length>,<type>,
#    <version>,<data_dev>,<hash_dev>,<data_block_size>,<hash_block_size>,
#    <num_data_blocks><hash_start_block>,<algorithm>,<root_hash>,<root_salt>:
#
# <uuid>   ::= xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx | "none"
# <mode>   ::= "ro" | "rw"
# <num>    ::= number of target device, set as 1
#
# More detail in field you can ref.
# https://www.kernel.org/doc/html/latest/admin-guide/device-mapper/verity.html
# https://gitlab.com/cryptsetup/cryptsetup/wikis/DMVerity
#

BOOTARGS=$( printf '%s root=/dev/dm-0 dm="vroot none ro 1,0 %s verity 1 %s %s %s %s %s %s %s %s %s,"' \
                   "${EXTRA_ARGS}" $((${DATA_BLOCKS} * 8)) ${ROOT_DEVICE} ${ROOT_DEVICE} ${DATA_BLOCK_SIZE} ${HASH_BLOCK_SIZE} ${DATA_BLOCKS} $((${DATA_BLOCKS} + 1)) ${HASH_ALG} ${ROOT_HASH} ${SALT} )

echo setenv bootargs \'${BOOTARGS}\'
