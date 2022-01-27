#!/bin/sh
source ./autobuild/lede-build-sanity.sh

#step1 clean
clean

#get the brach_name
temp=${0%/*}
branch_name=${temp##*/}

#step2 build
build ${branch_name} -j4 || [ "$LOCAL" != "1" ]


