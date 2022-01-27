#!/bin/sh

. ./flash.info

# Modify input/output image name
PRELOADER_NAME=preloader_evb7622_64.bin
OUTPUT_NAME=preloader_header.bin


./bch i "$FLASH_NAME" "$PRELOADER_NAME" "$OUTPUT_NAME" 1 64 0

