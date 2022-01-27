FROM ubuntu:18.04

COPY buildroot-gcc492_arm.tar.bz2 /toolchain/
COPY setup.sh /

RUN /setup.sh

WORKDIR /starlink-wifi-mtk

CMD /bin/bash
