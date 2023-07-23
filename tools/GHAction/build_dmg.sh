#!/usr/bin/env bash
set -ex

WORKSPACE=$(cd "$(dirname "$0")/../.." && pwd)
export WORKSPACE
cd "${WORKSPACE}"

if [ -n "$CERT_PASSWORD" ]
then
    security delete-keychain build.keychain || true
    security create-keychain -p 'Password123' build.keychain
    security default-keychain -s build.keychain
    security import "$WORKSPACE/tools/GHAction/Certificates.p12" -A -P "$CERT_PASSWORD"
    security import "$WORKSPACE/tools/GHAction/CertificatesMac.p12" -A -P "$CERT_PASSWORD"
    security unlock-keychain -p 'Password123' build.keychain
    security set-keychain-settings build.keychain
    security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k 'Password123' build.keychain > /dev/null

    mkdir -p "$HOME/Library/MobileDevice/Provisioning Profiles"
    for PLATFORM_DIR in iphone tvos
    do
        cp "$WORKSPACE/platform/$PLATFORM_DIR"/*.mobileprovision "$HOME/Library/MobileDevice/Provisioning Profiles/"
    done
fi


BUILD_NUMBER=${BUILD_NUMBER:-3575}
YEAR=${YEAR:-2020}

if ! bin/mac/build_dmg.sh -d -b "$YEAR.$BUILD_NUMBER" -e "${WORKSPACE}/Native/CoronaNative.tar.gz" "${WORKSPACE}" "${WORKSPACE}/docs"
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
echo $?
