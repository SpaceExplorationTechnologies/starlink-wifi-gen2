#!/bin/sh

. ./flash.info

# Modify input/output image name
INPUT_NAME=flashimage.bin
OUTPUT_NAME=flashimage.ecc

./sbch e "$FLASH_NAME" "$INPUT_NAME" "$OUTPUT_NAME" 0 0 0
