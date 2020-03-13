#!/usr/bin/env bash
set -ex

WORKSPACE=$(cd "$(dirname "$0")/../.." && pwd)
cd "$WORKSPACE"

BUILD_NUMBER=${BUILD_NUMBER:-3575}
YEAR=${YEAR:-2020}
MONTH=${MONTH:-04}
DAY=${DAY:-04}
BUILD=${BUILD:-$YEAR.$BUILD_NUMBER}

sed -i .bak -E "s/^#define[[:space:]]*Rtt_BUILD_REVISION.*$/#define Rtt_BUILD_REVISION $BUILD_NUMBER/" librtt/Core/Rtt_Version.h
sed -i .bak -E "s/^#define[[:space:]]*Rtt_BUILD_YEAR[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_YEAR $YEAR/" librtt/Core/Rtt_Version.h
sed -i .bak -E "s/^#define[[:space:]]*Rtt_BUILD_MONTH[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_MONTH $MONTH/" librtt/Core/Rtt_Version.h
sed -i .bak -E "s/^#define[[:space:]]*Rtt_BUILD_DAY[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_DAY $DAY/" librtt/Core/Rtt_Version.h
rm -f librtt/Core/Rtt_Version.h.bak

defaults write platform/mac/Info CFBundleVersion "$YEAR"."$BUILD_NUMBER"
defaults write platform/mac/Info CFBundleShortVersionString "$YEAR"."$BUILD_NUMBER"
plutil -convert xml1 platform/mac/Info.plist


COPYFILE_DISABLE=1 tar -czf corona.tgz --exclude '.git' --exclude 'corona.tgz' ./
mkdir -p output
mv corona.tgz output/corona.tgz