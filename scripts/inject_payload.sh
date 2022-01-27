#!/bin/bash -ex

# This script fetches the relevant wifi binaries from the payload repository and
# injects them into the wifi v2 platform filesystem. It takes a path to a built
# payload repsitory.

if [[ "$PAYLOAD" == "" ]]; then
    set +x
    echo
    echo "####################################################################################################"
    echo "Usage: PAYLOAD=... [COMMIT=1] inject_payload.sh"
    echo "####################################################################################################"
    exit 1
fi

if [[ ! -d "$PAYLOAD" ]]; then
    set +x
    echo "Error: $PAYLOAD path to payload repo doesn't exist!"
    exit 1
fi

PAYLOAD="$(realpath "$PAYLOAD")"
SPACEX_INJECT=openwrt/package/base-files/spacex_inject

cd "$(dirname "$0")/.."

if [[ "$COMMIT" == "1" ]]; then
    rm -rf payload
fi
rm -rf $SPACEX_INJECT

# Install file from payload into wifi filesystem.
# inject <src> <dst>
function inject {
    SRC="$PAYLOAD/$1"
    DST="$SPACEX_INJECT/$2"
    if [[ "$COMMIT" == "1" ]]; then
        COMMIT_DST="payload/$1"
    fi

    if [[ ! -e "$SRC" ]]; then
        set +x
        echo "Error: $SRC doesn't exist"
        echo "Have you built payload for ARMv7?"
        exit 1
    fi

    if [[ -f "$SRC" ]]; then
        install -D -m +rwx "$SRC" "$DST"
        if [[ $COMMIT_DST ]]; then
            install -D -m +rwx "$SRC" "$COMMIT_DST"
        fi
    else
        mkdir -p "$(dirname "$DST")"
        cp -rf "$SRC" "$DST"
        if [[ $COMMIT_DST ]]; then
            mkdir -p "$(dirname "$COMMIT_DST")"
            cp -rf "$SRC" "$COMMIT_DST"
        fi
    fi
}

# Install binary from payload into wifi usr/sbin
# Assumes binaries are in armv7 bazel out bin
# inject_sbin <src>
function inject_sbin {
    inject "bazel-out/armv7l-opt-clang-12/bin/$1.stripped" "/usr/sbin/$(basename "$1")"
}

inject_sbin spacex/ux/wifi/wifi_control/wifi_control
inject spacex/ux/wifi/wifi_control/static /etc/www/static
inject spacex/ux/wifi/wifi_control/templates /etc/www/templates
inject spacex/ux/wifi/wifi_control/regulatory_labels/v2_label.png /etc/www/static/images/regulatory_label.png +r
