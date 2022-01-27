#!/bin/bash -ex

cd "$(dirname "$0")/../storage"

mkdir -p bin

# Make UBI image
ubinize -o bin/ubi.img -p 128KiB -m 2048 -s 2048 cfg.ini 

cp bin/ubi.img ../bin/$VERSION/v2-$VERSION-storage.img