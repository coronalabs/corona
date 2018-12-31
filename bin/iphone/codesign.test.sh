#!/bin/bash

export CODESIGN_ALLOCATE=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/codesign_allocate
export PATH="/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin:/Developer/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin"

# Modify xcent

/usr/bin/codesign --deep -f -s "$3" --keychain ~/iphone.keychain --entitlements "$1" "$2"
