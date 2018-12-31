#!/bin/bash

echo "Running app_sign for $CONFIGURATION"

path=`dirname "$0"`

CUSTOM_EXE_PATH="$1"

pushd "$path" > /dev/null
dir=`pwd`
path="$dir"
popd > /dev/null

# Assume CoronaBuilder.app is in the same dir
TOOL_DIR=$path/CoronaBuilder.app
if [ -z "$TOOL_DIR" ]
then
    # Assume default install location
    TOOL_DIR=$(dirname "$0")/CoronaBuilder.app
fi
TOOL_PATH="$TOOL_DIR/Contents/MacOS/CoronaBuilder"
if [ ! -e "$TOOL_PATH" ]
then
    echo "ERROR: Required CoronaBuilder tool not found at ($TOOL_PATH)"
    exit -1
fi

echo "USING TOOL_PATH: $TOOL_PATH"

BIN_DIR=$TOOL_DIR/../
SHARED_RESOURCE_DIR=$BIN_DIR/../../shared/resource

# 
# Canonicalize relative paths to absolute paths
# 
pushd "$BIN_DIR" > /dev/null
dir=`pwd`
BIN_DIR="$dir"
popd > /dev/null

pushd "$SHARED_RESOURCE_DIR" > /dev/null
dir=`pwd`
SHARED_RESOURCE_DIR="$dir"
popd > /dev/null


if [[ ! -e "$SHARED_RESOURCE_DIR/developerkey.cert" ]]
then
    echo "Error: Missing developer key"
    exit -1
fi

CORONA_EXECUTABLE_PATH=$TARGET_BUILD_DIR/$EXECUTABLE_PATH

if [ "$CORONA_APK_PATH" ]
then
    BUNDLE_EXE_PATH=$CORONA_APK_PATH
else
    BUNDLE_EXE_PATH=$CORONA_EXECUTABLE_PATH
fi

if [ -z "$CORONA_RESOURCE_CAR_PATH" ]
then
    CORONA_RESOURCE_CAR_PATH=$TARGET_BUILD_DIR/$EXECUTABLE_FOLDER_PATH
fi

#
# Checks exit value for error
# 
checkError() {
    if [ $? -ne 0 ]
    then
        echo "*** app_sign cannot resign $BUNDLE_EXE_PATH. Please remove and then build. ***"
        exit -1
    fi
}

if [ "$CORONA_APK_PATH" ]
then
    # Android
    OUTPUT_DIR=`dirname "$CORONA_APK_PATH"`

    pushd "$OUTPUT_DIR" > /dev/null

        # sign exe with signature of resource.car
        LIB_NAME=libcorona.so
        LIB_REL_PATH=lib/armeabi-v7a/$LIB_NAME
        unzip -o "$CORONA_APK_PATH" "$LIB_REL_PATH" -d "$OUTPUT_DIR"
        checkError

        "$TOOL_PATH" app_sign sign "$SHARED_RESOURCE_DIR/developerkey.cert" "$CORONA_RESOURCE_CAR_PATH/resource.car" "$OUTPUT_DIR/$LIB_REL_PATH" little
        checkError

        zip -v "$CORONA_APK_PATH" "$LIB_REL_PATH"

        rm -rf "$LIB_REL_PATH"

    popd > /dev/null
else
    # iOS
    if [ "${CUSTOM_EXE_PATH}" ]
    then
        EXE_PATH="${CUSTOM_EXE_PATH}"
    else
        EXE_PATH="${CORONA_EXECUTABLE_PATH}"
    fi

    echo "Signing $EXE_PATH"

    "$TOOL_PATH" app_sign sign "$SHARED_RESOURCE_DIR/developerkey.cert" "$CORONA_RESOURCE_CAR_PATH/resource.corona-archive" "$EXE_PATH" little
    checkError
fi
