#!/bin/bash -ex
#
# Install build dependencies. Requires Ubuntu 18.04 or greater.

# If not run as root, rerun with sudo.
if [[ $UID != 0 ]]; then
    sudo $0 $*
    exit $?
fi

cd "$(dirname "$0")/.."

tar jxvf toolchain/buildroot-gcc492_arm.tar.bz2 -C /opt/

apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y \
    asciidoc \
    autoconf \
    bc \
    binutils \
    binutils-gold \
    bison \
    bsdmainutils \
    bzip2 \
    curl \
    device-tree-compiler \
    flex \
    g++ \
    gawk \
    gcc \
    gcc-arm-linux-gnueabihf \
    gettext \
    git \
    libc6-dev \
    libc6-i386 \
    libncurses5-dev \
    libncurses-dev \
    libpcre3-dev \
    libssl-dev \
    libxml-parser-perl \
    lzma \
    make \
    mtd-utils \
    ocaml \
    ocaml-findlib \
    ocaml-nox \
    patch \
    perl \
    pkg-config \
    python \
    python2.7 \
    python-yaml \
    sharutils \
    subversion \
    u-boot-tools \
    unzip \
    wget \
    xz-utils \
    zlib1g-dev
