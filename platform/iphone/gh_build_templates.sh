#!/usr/bin/env bash
set -ex

WORKSPACE=$(cd "$(dirname "$0")/../.." && pwd)
BUILD_NUMBER=${BUILD_NUMBER:-2020}
YEAR=${YEAR:-3575}
MONTH=${MONTH:-04}
DAY=${DAY:-04}
BUILD=${BUILD:-$YEAR.$BUILD_NUMBER}

IOS_SDK=${IOS_SDK:-$(xcrun --sdk iphoneos --show-sdk-version)}

export BUILD_NUMBER
export YEAR
export MONTH
export DAY
export BUILD
export WORKSPACE


[ -z "$BUILD_NUMBER" ]  || sed -i .bak -E "s/^#define[[:space:]]*Rtt_BUILD_REVISION.*$/#define Rtt_BUILD_REVISION $BUILD_NUMBER/" "${WORKSPACE}/librtt/Core/Rtt_Version.h"
[ -z "$YEAR" ]          || sed -i .bak -E "s/^#define[[:space:]]*Rtt_BUILD_YEAR[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_YEAR $YEAR/" "${WORKSPACE}/librtt/Core/Rtt_Version.h"
[ -z "$MONTH" ]         || sed -i .bak -E "s/^#define[[:space:]]*Rtt_BUILD_MONTH[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_MONTH $MONTH/" "${WORKSPACE}/librtt/Core/Rtt_Version.h"
[ -z "$DAY" ]           || sed -i .bak -E "s/^#define[[:space:]]*Rtt_BUILD_DAY[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_DAY $DAY/" "${WORKSPACE}/librtt/Core/Rtt_Version.h"

# security create-keychain -p 'Password123' build.keychain
# security default-keychain -s build.keychain
# security unlock-keychain -p 'Password123' build.keychain
# security import Certificate.p12 
# security set-key-partition-list -S apple-tool:,apple: -s -k 'Password123' build.keychain

if ! (cd "$WORKSPACE/platform/iphone/" && ./build_templates.sh "$IOS_SDK" "$BUILD")
then
    BUILD_FAILED=YES
    echo "BUILD FAILED"
fi

git checkout "$WORKSPACE/librtt/Core/Rtt_Version.h"
rm -rf "$WORKSPACE/librtt/Core/Rtt_Version.h.bak"
# security default-keychain -s login.keychain
# security delete-keychain build.keychain &> /dev/null || true

if [ "$BUILD_FAILED" = "YES" ]
then
    exit 1
fi

(
    set -ex
    cd "$WORKSPACE/platform/iphone/"
    for PLATFORM in iphone iphone-sim
    do
        case "$PLATFORM" in
            "iphone-sim") SDK_PLATFORM=iphonesimulator ;;
            "iphone") SDK_PLATFORM=iphoneos ;;
            "tvos") SDK_PLATFORM=appletvos ;;
            "tvos-sim") SDK_PLATFORM=appletvsimulator ;;
        esac
        echo "Creating $PLATFORM template for $SDK_PLATFORM"
        # remove symbolic link which confuses things
        rm -f "template/$PLATFORM/$PLATFORM"

        for IOS_VER in template/"${PLATFORM}"/*
        do
            IOS_VER=$(basename "${IOS_VER}")
            ARCHIVE="$WORKSPACE/platform/iphone/${SDK_PLATFORM}_${IOS_VER}.tar.bz"

            (
                set -ex
                cd "template/${PLATFORM}/${IOS_VER}/basic/"
                rm -f "${ARCHIVE}"
                tar cvjf "${ARCHIVE}" --exclude='CoronaSimLogo-256.png' --exclude='world.jpg' --exclude='Icon*.png' ./{libtemplate,template.app}
                echo "Built ${ARCHIVE}"
            )

            UPLOADS="$UPLOADS $ARCHIVE"
        done
    done
    echo "$UPLOADS"
    rm -f "$WORKSPACE/ios_output.zip"
    zip -0 "$WORKSPACE/ios_output.zip" iphone*.tar.bz
)