#!/bin/bash

set -ex
path=$(dirname "$0")

PRODUCT=CoronaEnterprise
USER=$1
PASSWORD=$2
OUTPUT_DIR=$3
TARGET=$4

if [ -z "$OUTPUT_DIR" ]
then
	OUTPUT_DIR=$path/$PRODUCT/
	echo "OUTPUT_DIR not specified. Defaulting to: $OUTPUT_DIR" 
fi

# Summarize xcodebuild output to stdout but save full output in separate file
XCODE_LOG_FILTERS="^    export |clang -x |libtool -static |^    cd $path"

FULL_LOG_FILE="iphone-enterprise-xcodebuild.log"

if [ "$WORKSPACE" != "" ]
then
	FULL_LOG_FILE="$WORKSPACE/$FULL_LOG_FILE"
fi

echo "### Full xcodebuild output can be found in $FULL_LOG_FILE"


checkDir() {
	if [ ! -d "$1" ]
    then
    	echo "ERROR: Directory '$1' does not exist."
        exit 255
    fi
}

# 
# Canonicalize relative paths to absolute paths
# 
pushd "$OUTPUT_DIR" > /dev/null
dir=$(pwd)
OUTPUT_DIR=$dir
popd > /dev/null

ROOT_DIR=$path/../../..
PLATFORM_DIR=$ROOT_DIR/platform
# TOOLS_DIR="$ROOT_DIR/tools"

# Shared dirs
DST_CORONA_DIR=$OUTPUT_DIR/Corona
checkDir "$DST_CORONA_DIR"

#DST_SAMPLES_DIR=$OUTPUT_DIR/Samples
#checkDir "$DST_SAMPLES_DIR"


# iOS dirs
DST_CORONA_IOS_DIR=$DST_CORONA_DIR/ios
checkDir "$DST_CORONA_IOS_DIR"

DST_INCLUDE_IOS_DIR=$DST_CORONA_IOS_DIR/include
checkDir "$DST_INCLUDE_IOS_DIR"

DST_LIB_DIR=$DST_CORONA_IOS_DIR/lib
checkDir "$DST_LIB_DIR"

DST_RESOURCE_IOS_DIR=$DST_CORONA_IOS_DIR/resource
checkDir "$DST_RESOURCE_IOS_DIR"


#
# Disable GUI prompts (TODO: Does not work)
#
if [[ "$USER" ]] && [[ "$PASSWORD" ]]
then
	security unlock-keychain -p "$PASSWORD" "/Users/$USER/Library/Keychains/login.keychain"
elif [[ "$USER" ]]
then
	security unlock-keychain "/Users/$USER/Library/Keychains/login.keychain"
fi

#
# iOS Template Project
#

#cp -rv "$path"/CoronaSampleApp "$DST_SAMPLES_DIR"/template/ios


# 
# Corona
# 

SYMROOT="$(cd "$PLATFORM_DIR/iphone" && pwd)/build"

# libplayer
# xcodebuild SYMROOT="$SYMROOT" -project "$PLATFORM_DIR"/iphone/ratatouille.xcodeproj -target libplayer clean 2>&1 | tee -a "$FULL_LOG_FILE" | grep -E -v "$XCODE_LOG_FILTERS"

# Device
CONFIG=Release

XCODE_TARGET=libplayer
if [ "trial" = "$TARGET" ]
then
	XCODE_TARGET=libplayer-trial
fi

# Classic

xcodebuild SYMROOT="$SYMROOT" -project "$PLATFORM_DIR"/iphone/ratatouille.xcodeproj -target ${XCODE_TARGET} -configuration $CONFIG -sdk iphoneos 2>&1 | tee -a "$FULL_LOG_FILE" | grep -E -v "$XCODE_LOG_FILTERS"

# Simulator
xcodebuild SYMROOT="$SYMROOT" -project "$PLATFORM_DIR"/iphone/ratatouille.xcodeproj -target ${XCODE_TARGET} -configuration $CONFIG -sdk iphonesimulator 2>&1 | tee -a "$FULL_LOG_FILE" | grep -E -v "$XCODE_LOG_FILTERS"

# create universal binary
lipo -create "$SYMROOT"/$CONFIG-iphoneos/${XCODE_TARGET}.a "$SYMROOT"/$CONFIG-iphonesimulator/${XCODE_TARGET}.a -output "$DST_LIB_DIR"/libplayer.a

# Angle

xcodebuild SYMROOT="$SYMROOT" -project "$PLATFORM_DIR"/iphone/ratatouille.xcodeproj -target ${XCODE_TARGET}-angle -configuration $CONFIG -sdk iphoneos 2>&1 | tee -a "$FULL_LOG_FILE" | grep -E -v "$XCODE_LOG_FILTERS"

# Simulator
xcodebuild SYMROOT="$SYMROOT" EXCLUDED_ARCHS=arm64 -project "$PLATFORM_DIR"/iphone/ratatouille.xcodeproj -target ${XCODE_TARGET}-angle -configuration $CONFIG -sdk iphonesimulator 2>&1 | tee -a "$FULL_LOG_FILE" | grep -E -v "$XCODE_LOG_FILTERS"

# create universal binary
lipo -create "$SYMROOT"/$CONFIG-iphoneos/${XCODE_TARGET}-angle.a "$SYMROOT"/$CONFIG-iphonesimulator/${XCODE_TARGET}-angle.a -output "$DST_LIB_DIR"/libplayer-angle.a

# Create MetalANGLE universal framewok
cp -vR  "$SYMROOT"/$CONFIG-iphoneos/MetalANGLE.framework "$DST_LIB_DIR/MetalANGLE.framework"
lipo -create "$SYMROOT"/$CONFIG-iphoneos/MetalANGLE.framework/MetalANGLE "$SYMROOT"/$CONFIG-iphonesimulator/MetalANGLE.framework/MetalANGLE -output "$DST_LIB_DIR"/MetalANGLE.framework/MetalANGLE.combined
xcrun bitcode_strip -r "$DST_LIB_DIR"/MetalANGLE.framework/MetalANGLE.combined -o "$DST_LIB_DIR"/MetalANGLE.framework/MetalANGLE
rm "$DST_LIB_DIR"/MetalANGLE.framework/MetalANGLE.combined

# copy headers
cp -v "$PLATFORM_DIR/iphone/Corona/"*.h "$DST_INCLUDE_IOS_DIR/Corona"

# copy resources
cp -v "$PLATFORM_DIR/iphone/MainWindow.xib" "$DST_RESOURCE_IOS_DIR"

cp -v "$PLATFORM_DIR/iphone/_CoronaSplashScreen.png" "$DST_RESOURCE_IOS_DIR"

cp -v "$PLATFORM_DIR/iphone/Resources-iPad/MainWindow-iPad.xib" "$DST_RESOURCE_IOS_DIR"

cp -rv "$SYMROOT/$CONFIG-iphoneos"/CoronaResources.bundle "$DST_RESOURCE_IOS_DIR"
