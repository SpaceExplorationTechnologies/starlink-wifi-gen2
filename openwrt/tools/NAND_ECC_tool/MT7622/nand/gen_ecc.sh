#!/bin/sh

. ./flash.info

# Modify input/output image name
INPUT_NAME=mt7622_img
OUTPUT_NAME=mt7622_img.ecc 

./bch e "$FLASH_NAME" "$INPUT_NAME" "$OUTPUT_NAME" 0 0 0


