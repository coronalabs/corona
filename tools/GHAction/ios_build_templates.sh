#!/usr/bin/env bash
set -ex

WORKSPACE=$(cd "$(dirname "$0")/../.." && pwd)
BUILD_NUMBER=${BUILD_NUMBER:-2020}
YEAR=${YEAR:-3575}
MONTH=${MONTH:-04}
DAY=${DAY:-04}
BUILD=${BUILD:-$YEAR.$BUILD_NUMBER}

: "${TEMPLATE_TARGET:=template}"
TEMPLATE_TARGET_SUFFIX="${TEMPLATE_TARGET#template}"

PLATFORM_DIR=$(basename "$(cd "$(dirname "$0")" && pwd)")
if [ "$PLATFORM_DIR" = "tvos" ]
then
    XCODE_SDK=${XCODE_SDK:-$(xcrun --sdk appletvos --show-sdk-version)}
    TEMPLATE_DIR=build/template
    BASIC=
    OUTPUT="$WORKSPACE/tvos_output.zip"
    PLATFORMS="appletvos appletvsimulator"
elif [ "$PLATFORM_DIR" = "iphone" ]
then
    XCODE_SDK=${XCODE_SDK:-$(xcrun --sdk iphoneos --show-sdk-version)}
    TEMPLATE_DIR=template
    BASIC=basic
    OUTPUT="$WORKSPACE/ios_output.zip"
    PLATFORMS="iphone iphone-sim"
else
    echo "ERROR: Don't know how to handle '$PLATFORM_DIR'"
    exit 1
fi

rm -f "$OUTPUT"

export BUILD_NUMBER
export YEAR
export MONTH
export DAY
export BUILD
export WORKSPACE
export TEMPLATE_TARGET

if [ -n "$CERT_PASSWORD" ]
then
    security delete-keychain build.keychain || true
    security create-keychain -p 'Password123' build.keychain
    security default-keychain -s build.keychain
    security import "$WORKSPACE/tools/GHAction/Certificates.p12" -A -P "$CERT_PASSWORD"
    security unlock-keychain -p 'Password123' build.keychain
    security set-keychain-settings build.keychain
    security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k 'Password123' build.keychain > /dev/null

    mkdir -p "$HOME/Library/MobileDevice/Provisioning Profiles"
    cp "$WORKSPACE/platform/$PLATFORM_DIR"/*.mobileprovision "$HOME/Library/MobileDevice/Provisioning Profiles/"
fi

if ! (cd "$WORKSPACE/platform/$PLATFORM_DIR/" && ./build_templates.sh "$XCODE_SDK" "$BUILD")
then
    BUILD_FAILED=YES
    echo "BUILD FAILED"
fi

if [ -n "$CERT_PASSWORD" ]
then
    security default-keychain -s login.keychain
    security delete-keychain build.keychain &> /dev/null || true
fi

if [ "$BUILD_FAILED" = "YES" ]
then
    exit 1
fi

(
    set -e
    cd "$WORKSPACE/platform/$PLATFORM_DIR/"
    for PLATFORM in $PLATFORMS
    do
        case "$PLATFORM" in
            "iphone-sim") SDK_PLATFORM=iphonesimulator ;;
            "iphone") SDK_PLATFORM=iphoneos ;;
            "tvos") SDK_PLATFORM=appletvos ;;
            "tvos-sim") SDK_PLATFORM=appletvsimulator ;;
            *) SDK_PLATFORM=$PLATFORM ;;
        esac
        echo "Creating $PLATFORM template for $SDK_PLATFORM"
        # remove symbolic link which confuses things
        rm -f "$TEMPLATE_DIR/$PLATFORM/$PLATFORM"

        for IOS_VER in "$TEMPLATE_DIR/$PLATFORM"/*
        do
            IOS_VER=$(basename "${IOS_VER}")
            ARCHIVE="$WORKSPACE/platform/$PLATFORM_DIR/${SDK_PLATFORM}_${IOS_VER}${TEMPLATE_TARGET_SUFFIX}.tar.bz"

            (
                set -e
                cd "$TEMPLATE_DIR/${PLATFORM}/${IOS_VER}/$BASIC"
                cp -X "$WORKSPACE/platform/resources/config_require.lua" ./
                mkdir -p libtemplate
                cp -X "$WORKSPACE/tools/buildsys-ios/libtemplate/build_output.sh" libtemplate/ || true
                rm -f "${ARCHIVE}"
                tar cvjf "${ARCHIVE}" --exclude='CoronaSimLogo-256.png' --exclude='world.jpg' --exclude='Icon*.png' ./{libtemplate,template.app}
                echo "Built ${ARCHIVE}"
            )
            mkdir -p "$WORKSPACE/output"
            cp -v "$ARCHIVE" "$WORKSPACE/output"
        done
    done
)