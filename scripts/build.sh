#!/bin/bash -ex

if [[ $UID != 0 ]]; then
    sudo -E $0 $*
    exit 0
fi

print_usage() {
    set +x
    echo
    echo "####################################################################################################"
    echo "Usage: [OVERLAY=all] [PAYLOAD=...] [VERSION=...] [NO_CLEAN=1] build.sh"
    echo "####################################################################################################"
    exit 1
}

cd "$(dirname "$0")/.."

export OVERLAY="${OVERLAY:=}"
export PAYLOAD="${PAYLOAD:=payload}"
export VERSION="${VERSION:=$(date +%Y-%m-%d)-$(git rev-parse --short HEAD)}"

# The anti-rollback version a FIP or BL2 image must be GREATER THAN OR EQUAL TO
# in order to be allowed to boot.
#
# IF YOU ARE CHANGING THIS NUMBER, PROCEED WITH EXTREME CAUTION.
# THIS WILL BE THE STSAFE VALUE SET BY WIFI_CONTROL.
# IF THIS DOES NOT CORRESPOND WITH THE BL2 ANTI-ROLLBACK VALUE,
# YOU COULD PERMANENTLY BRICK ANY BOARD THIS GETS ON.
#
# See https://confluence.spacex.corp/display/STARPROD/Starlink+UX+-+V2+Anti-Rollback
export ANTI_ROLLBACK_REQUIRED_VERSION=1
echo -n $ANTI_ROLLBACK_REQUIRED_VERSION > openwrt/package/base-files/files/etc/required-rollback-version

mkdir -p "bin/$VERSION"

scripts/inject_payload.sh

scripts/run_in_docker.sh scripts/build_uboot.sh
scripts/run_in_docker.sh scripts/build_fip.sh
scripts/run_in_docker.sh scripts/build_openwrt.sh
scripts/run_in_docker.sh scripts/build_storage.sh
scripts/run_in_docker.sh scripts/build_upgrade.sh
scripts/run_in_docker.sh scripts/build_single.sh
