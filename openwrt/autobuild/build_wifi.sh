#!/bin/sh

if [ ! -n "$1" ]; then
	echo "Please input : module/built-in/both"
	echo "module - rebuild mt_wifi module only"
	echo "built-in - rebuild kernel only, it is for built-in kernel wifi driver"
	echo "both   - rebuild module and kernel"
	exit 1
fi

if [ "$1" == "module" ]; then
	echo "module - rebuild mt_wifi module only"
	make V=s package/mtk/drivers/mt_wifi/clean
	make V=s package/mtk/drivers/mt_wifi/compile
	make V=s package/mtk/drivers/mt_wifi/install
	make V=s package/install
	make V=s target/install-image
	exit 1
fi

if [ "$1" == "built-in" ]; then
	echo "built-in - rebuild kernel only, it is for built-in kernel wifi driver"
	make V=s target/clean-linux
	make V=s package/install
	make V=s target/install
	exit 1
fi

if [ "$1" == "both" ]; then
	echo "both   - rebuild module and kernel"
	make V=s package/mtk/drivers/mt_wifi/clean
	make V=s package/mtk/drivers/mt_wifi/compile
	make V=s package/mtk/drivers/mt_wifi/install
	make V=s target/clean-linux
	make V=s package/install
	make V=s target/install
	exit 1
fi

echo "Do nothing - invlaid command"
