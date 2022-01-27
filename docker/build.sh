#!/bin/bash -ex

IMAGE="${IMAGE:=harbor.spacex.corp/spacex/starlink-wifi-mtk-build}"

cd "$(dirname "$0")"

cp ../scripts/setup.sh .
cp ../toolchain/buildroot-gcc492_arm.tar.bz2 .

# Build image.
docker image rm --force $IMAGE
docker build --network host -t $IMAGE .

# Push image.
if [[ "$NO_PUSH" == "1" ]]; then
    VERSION=":latest"
else
    VERSION="@$(
        docker push $IMAGE \
            | tee /dev/stderr \
            | tail -n 1 \
            | sed -r 's/'latest': digest: (.*) size: .*/\1/'
    )"
fi

# Update pin.
echo "${IMAGE}${VERSION}" > image.txt
