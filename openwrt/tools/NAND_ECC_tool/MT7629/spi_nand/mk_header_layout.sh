#!/bin/sh

. ./flash.info

# Modify input/output image name

./sbch h "$FLASH_NAME" 0 snfi_block0.binary 1 64 0
