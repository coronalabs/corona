#!/bin/bash

export CODESIGN_ALLOCATE=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/codesign_allocate
export PATH="/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin:/Developer/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin"

echo "Code signing identity($3) path($2) entitlements($1)"
/usr/bin/codesign --deep --verbose -f -s "$3" --entitlements "$1" "$2"
