#!/bin/bash

set -e
set +u
# Avoid recursively calling this script.
if [[ $SF_MASTER_SCRIPT_RUNNING ]]
then
    exit 0
fi
set -u
export SF_MASTER_SCRIPT_RUNNING=1

if [ -z "$SF_TARGET_NAME" ]
then
    SF_TARGET_NAME=${PROJECT_NAME}
    echo "Using ${SF_TARGET_NAME} +++++++++"
else
    echo "Using ${SF_TARGET_NAME} *********"
fi
SF_EXECUTABLE_PATH="lib${SF_TARGET_NAME}.a"
SF_WRAPPER_NAME="${SF_TARGET_NAME}.framework"

# The following conditionals come from
# https://github.com/kstenerud/iOS-Universal-Framework

if [[ "$SDK_NAME" =~ ([A-Za-z]+) ]]
then
    SF_SDK_PLATFORM=${BASH_REMATCH[1]}
else
    echo "Could not find platform name from SDK_NAME: $SDK_NAME"
    exit 1
fi

if [[ "$SDK_NAME" =~ ([0-9]+.*$) ]]
then
    SF_SDK_VERSION=${BASH_REMATCH[1]}
else
    echo "Could not find sdk version from SDK_NAME: $SDK_NAME"
    exit 1
fi

if [[ "$SF_SDK_PLATFORM" = "iphoneos" ]]
then
    SF_OTHER_PLATFORM=iphonesimulator
else
    SF_OTHER_PLATFORM=iphoneos
fi

if [[ "$BUILT_PRODUCTS_DIR" =~ (.*)$SF_SDK_PLATFORM$ ]]
then
    SF_OTHER_BUILT_PRODUCTS_DIR="${BASH_REMATCH[1]}${SF_OTHER_PLATFORM}"
else
    echo "Could not find platform name from build products directory: $BUILT_PRODUCTS_DIR"
    exit 1
fi

# Build the other platform.
xcodebuild -project "${PROJECT_FILE_PATH}" -target "${TARGET_NAME}" -configuration "${CONFIGURATION}" -sdk "${SF_OTHER_PLATFORM}${SF_SDK_VERSION}" BUILD_DIR="${BUILD_DIR}" OBJROOT="${OBJROOT}/DependantBuilds" BUILD_ROOT="${BUILD_ROOT}" SYMROOT="${SYMROOT}" "$ACTION"

# Copy the static library into each platform's framework bundle
cp -a "${BUILT_PRODUCTS_DIR}/${SF_EXECUTABLE_PATH}" "${BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}/Versions/A/${SF_TARGET_NAME}"
cp -a "${SF_OTHER_BUILT_PRODUCTS_DIR}/${SF_EXECUTABLE_PATH}" "${SF_OTHER_BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}/Versions/A/${SF_TARGET_NAME}"

# ── Mac Catalyst slice (added) ──────────────────────────────────────────────
# Build for Mac Catalyst and add it to the xcframework so the same
# xcframework works for iOS, iOS Simulator, and Mac Catalyst targets.
#
# Guard: only run when we are the *primary* invocation (i.e. building for an
# iOS SDK), not when we are a recursion triggered by the "other platform" sub-
# build above.  SF_MASTER_SCRIPT_RUNNING is already exported at line 11 above,
# so any child xcodebuild that runs build_framework.sh will exit at the top.
SF_CATALYST_WRAPPER=""
if [[ "$SF_SDK_PLATFORM" != "macosx" ]]; then
    # Derive the Catalyst build-products directory from the current one:
    # e.g.  .../Build/Products/Release-iphoneos  →  .../Release-maccatalyst
    if [[ "$BUILT_PRODUCTS_DIR" =~ (.*)$SF_SDK_PLATFORM ]]; then
        SF_CATALYST_BUILT_PRODUCTS_DIR="${BASH_REMATCH[1]}maccatalyst"
    else
        SF_CATALYST_BUILT_PRODUCTS_DIR="${BUILD_DIR}/Release-maccatalyst"
    fi
    SF_CATALYST_WRAPPER="${SF_CATALYST_BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}"

    # Compile for Mac Catalyst.  SF_MASTER_SCRIPT_RUNNING prevents build_framework.sh
    # from running again inside this xcodebuild invocation.
    xcodebuild -project "${PROJECT_FILE_PATH}" \
               -target "${TARGET_NAME}" \
               -configuration "${CONFIGURATION}" \
               -sdk macosx \
               SUPPORTS_MACCATALYST=YES \
               TARGETED_DEVICE_FAMILY="1,2" \
               IPHONEOS_DEPLOYMENT_TARGET=16.0 \
               ARCHS="arm64 x86_64" \
               BUILD_DIR="${BUILD_DIR}" \
               OBJROOT="${OBJROOT}/DependantBuilds" \
               BUILD_ROOT="${BUILD_ROOT}" \
               SYMROOT="${SYMROOT}" \
               "$ACTION"

    # The Catalyst build may not have created the .framework wrapper (because
    # build_framework.sh exited early due to SF_MASTER_SCRIPT_RUNNING).
    # Seed the wrapper from the iOS device slice and replace the binary.
    CATALYST_LIB="${SF_CATALYST_BUILT_PRODUCTS_DIR}/${SF_EXECUTABLE_PATH}"
    if [[ -f "${CATALYST_LIB}" ]]; then
        cp -a "${BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}" "${SF_CATALYST_WRAPPER}"
        cp -f "${CATALYST_LIB}" \
              "${SF_CATALYST_WRAPPER}/Versions/A/${SF_TARGET_NAME}"
    else
        echo "Warning: Catalyst lib not found at ${CATALYST_LIB} — skipping Catalyst slice"
        SF_CATALYST_WRAPPER=""
    fi
fi
# ── end Mac Catalyst ─────────────────────────────────────────────────────────

# Create XCFramework instead of fat binary (supports M1 simulators where both device and simulator use arm64)
XCFRAMEWORK_PATH="${BUILT_PRODUCTS_DIR}/${SF_TARGET_NAME}.xcframework"
rm -rf "${XCFRAMEWORK_PATH}"

if [[ -n "${SF_CATALYST_WRAPPER}" && -d "${SF_CATALYST_WRAPPER}" ]]; then
    xcodebuild -create-xcframework \
        -framework "${BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}" \
        -framework "${SF_OTHER_BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}" \
        -framework "${SF_CATALYST_WRAPPER}" \
        -output "${XCFRAMEWORK_PATH}"
else
    xcodebuild -create-xcframework \
        -framework "${BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}" \
        -framework "${SF_OTHER_BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}" \
        -output "${XCFRAMEWORK_PATH}"
fi

# Also create XCFramework in the other build products directory
cp -a "${XCFRAMEWORK_PATH}" "${SF_OTHER_BUILT_PRODUCTS_DIR}/${SF_TARGET_NAME}.xcframework"
