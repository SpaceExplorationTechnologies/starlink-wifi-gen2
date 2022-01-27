#!/usr/bin/env bash
export LANG=C
export LC_ALL=C
[ -n "$TOPDIR" ] && cd $TOPDIR

# SpaceX: Our compilation script, at build time, injects the version file
# into base-files, use this file to set openwrt_version.
try_spacex_version() {
	REV="$(cat package/base-files/files/etc/version)"
	[ -n "$REV" ]
}

try_spacex_version || REV="unknown"
echo "$REV"
