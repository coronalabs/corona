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

# Create XCFramework instead of fat binary (supports M1 simulators where both device and simulator use arm64)
XCFRAMEWORK_PATH="${BUILT_PRODUCTS_DIR}/${SF_TARGET_NAME}.xcframework"
rm -rf "${XCFRAMEWORK_PATH}"

xcodebuild -create-xcframework \
    -framework "${BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}" \
    -framework "${SF_OTHER_BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}" \
    -output "${XCFRAMEWORK_PATH}"

# Also create XCFramework in the other build products directory
cp -a "${XCFRAMEWORK_PATH}" "${SF_OTHER_BUILT_PRODUCTS_DIR}/${SF_TARGET_NAME}.xcframework"
