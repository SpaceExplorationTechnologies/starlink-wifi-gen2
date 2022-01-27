#!/bin/bash -ex

cd "$(dirname "$0")/.."

if [[ $SUDO_USER != "" ]]; then
   trap 'chown -R $SUDO_USER:UnixUsers .' EXIT
fi

ARGS=""
if [[ "$INTERACTIVE" == 1 ]]; then
   ARGS="$ARGS -it"
fi

DOCKER_IMAGE=$(cat docker/image.txt)
docker run \
    $ARGS --rm \
    --mount type=bind,source="$PWD",target="/starlink-wifi-mtk" \
    --env NO_CLEAN="$NO_CLEAN" \
    --env OVERLAY="$OVERLAY" \
    --env PAYLOAD="$PAYLOAD" \
    --env VERSION="$VERSION" \
    --env ANTI_ROLLBACK_REQUIRED_VERSION="$ANTI_ROLLBACK_REQUIRED_VERSION" \
    --entrypoint /bin/bash \
    "$DOCKER_IMAGE" \
    -c "$*"
