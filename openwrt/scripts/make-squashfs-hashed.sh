#!/bin/bash
#
# 1. Using veritysetup to append hash image into squashfs
# 2. Parsing output of veritysetup to generate uboot script
#
SQUASHFS_FILE_PATH=$1
STAGING_DIR_HOST=$2
TOPDIR=$3
SUMMARY_FILE=$4

FILE_SIZE=`stat -c "%s" ${SQUASHFS_FILE_PATH}`
BLOCK_SIZE=4096

DATA_BLOCKS=$((${FILE_SIZE} / ${BLOCK_SIZE}))
[ $((${FILE_SIZE} % ${BLOCK_SIZE})) -ne 0 ] && DATA_BLOCKS=$((${DATA_BLOCKS} + 1))

HASH_OFFSET=$((${DATA_BLOCKS} * ${BLOCK_SIZE}))

${STAGING_DIR_HOST}/bin/veritysetup format \
	--data-blocks=${DATA_BLOCKS} \
	--hash-offset=${HASH_OFFSET} \
	${SQUASHFS_FILE_PATH} ${SQUASHFS_FILE_PATH} \
	> ${SUMMARY_FILE}
