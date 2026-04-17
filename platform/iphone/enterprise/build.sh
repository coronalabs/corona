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

# Simulator (includes arm64 for M1 simulator support)
xcodebuild SYMROOT="$SYMROOT" -project "$PLATFORM_DIR"/iphone/ratatouille.xcodeproj -target ${XCODE_TARGET} -configuration $CONFIG -sdk iphonesimulator 2>&1 | tee -a "$FULL_LOG_FILE" | grep -E -v "$XCODE_LOG_FILTERS"

# Mac Catalyst (non-fatal: OpenGLES headers removed from macOS 26 SDK)
echo "Building ${XCODE_TARGET} for Mac Catalyst (non-fatal)"
xcodebuild SYMROOT="$SYMROOT" -project "$PLATFORM_DIR"/iphone/ratatouille.xcodeproj -target ${XCODE_TARGET} -configuration $CONFIG \
    -sdk macosx SUPPORTS_MACCATALYST=YES IPHONEOS_DEPLOYMENT_TARGET=16.0 ARCHS="arm64 x86_64" \
    2>&1 | tee -a "$FULL_LOG_FILE" | grep -E -v "$XCODE_LOG_FILTERS" || echo "Warning: Mac Catalyst ${XCODE_TARGET} build failed — skipping Catalyst slice"

# create xcframework (device + simulator + catalyst if available)
rm -rf "$DST_LIB_DIR"/libplayer.xcframework
LIBPLAYER_XCF_ARGS=(
    -library "$SYMROOT"/$CONFIG-iphoneos/${XCODE_TARGET}.a         -headers "$PLATFORM_DIR/iphone/Corona"
    -library "$SYMROOT"/$CONFIG-iphonesimulator/${XCODE_TARGET}.a  -headers "$PLATFORM_DIR/iphone/Corona"
)
if [ -f "$SYMROOT"/$CONFIG-maccatalyst/${XCODE_TARGET}.a ]; then
    LIBPLAYER_XCF_ARGS+=(-library "$SYMROOT"/$CONFIG-maccatalyst/${XCODE_TARGET}.a -headers "$PLATFORM_DIR/iphone/Corona")
fi
xcodebuild -create-xcframework "${LIBPLAYER_XCF_ARGS[@]}" -output "$DST_LIB_DIR"/libplayer.xcframework

# Also copy device library as standard .a for backward compatibility
cp -v "$SYMROOT"/$CONFIG-iphoneos/${XCODE_TARGET}.a "$DST_LIB_DIR"/libplayer.a

# Angle

# Pre-build MetalANGLE.framework so that libplayer-angle and libplayer-core-angle
# can find <MetalANGLE/MGLKit.h> via FRAMEWORK_SEARCH_PATHS (BUILT_PRODUCTS_DIR).
# MetalANGLE is no longer a sub-project dependency in ratatouille.xcodeproj
# (removed to fix macCatalyst propagation issues), so it must be built here first.
METALANGLE_PROJECT="$ROOT_DIR/external/MetalANGLE/ios/xcode/OpenGLES.xcodeproj"
METALANGLE_INCLUDE="$ROOT_DIR/external/MetalANGLE/include"
GLSLANG_DIR="$ROOT_DIR/external/MetalANGLE/third_party/glslang/src"

echo "Pre-building MetalANGLE.framework for iphoneos (angle build)"
xcodebuild build \
    -project "$METALANGLE_PROJECT" \
    -target MetalANGLE \
    -configuration Release \
    -sdk iphoneos \
    SYMROOT="$SYMROOT" \
    SKIP_INSTALL=YES \
    DEPLOYMENT_POSTPROCESSING=NO \
    "OTHER_LDFLAGS=-weak_framework OpenGLES \$(inherited)" \
    2>&1 | tee -a "$FULL_LOG_FILE" | grep -E "(BUILD SUCCEEDED|BUILD FAILED|error:|Undefined symbol|ld:)" || true
if [ ${PIPESTATUS[0]} -ne 0 ]; then
    echo "=== MetalANGLE iphoneos build failed — last 150 lines of log ==="
    tail -150 "$FULL_LOG_FILE"
    echo "Exiting due to errors (above)"; exit 1
fi

echo "Pre-building MetalANGLE.framework for iphonesimulator (angle build)"
xcodebuild build \
    -project "$METALANGLE_PROJECT" \
    -target MetalANGLE \
    -configuration Release \
    -sdk iphonesimulator \
    SYMROOT="$SYMROOT" \
    SKIP_INSTALL=YES \
    DEPLOYMENT_POSTPROCESSING=NO \
    "OTHER_LDFLAGS=-weak_framework OpenGLES \$(inherited)" \
    2>&1 | tee -a "$FULL_LOG_FILE" | grep -E "(BUILD SUCCEEDED|BUILD FAILED|error:|Undefined symbol|ld:)" || true
if [ ${PIPESTATUS[0]} -ne 0 ]; then
    echo "=== MetalANGLE iphonesimulator build failed — last 150 lines of log ==="
    tail -150 "$FULL_LOG_FILE"
    echo "Exiting due to errors (above)"; exit 1
fi

# Mac Catalyst MetalANGLE (non-fatal: OpenGLES may be absent from macOS 26 SDK)
echo "Pre-building MetalANGLE.framework for Mac Catalyst (non-fatal)"
xcodebuild build \
    -project "$METALANGLE_PROJECT" \
    -target MetalANGLE \
    -configuration Release \
    -sdk macosx \
    SUPPORTS_MACCATALYST=YES \
    IPHONEOS_DEPLOYMENT_TARGET=16.0 \
    ARCHS="arm64 x86_64" \
    SYMROOT="$SYMROOT" \
    SKIP_INSTALL=YES \
    DEPLOYMENT_POSTPROCESSING=NO \
    "OTHER_LDFLAGS=-weak_framework OpenGLES \$(inherited)" \
    2>&1 | tee -a "$FULL_LOG_FILE" | grep -E "(BUILD SUCCEEDED|BUILD FAILED|error:|Undefined symbol|ld:)" || \
    echo "Warning: MetalANGLE Mac Catalyst build failed — skipping Catalyst slice"

ANGLE_BUILD_SETTINGS=(
    "HEADER_SEARCH_PATHS=$METALANGLE_INCLUDE $GLSLANG_DIR \$(inherited)"
    "GCC_PREPROCESSOR_DEFINITIONS=\$(inherited) Rtt_EGL"
    "ALLOW_NON_MODULAR_INCLUDES_IN_FRAMEWORK_MODULES=YES"
)

xcodebuild SYMROOT="$SYMROOT" -project "$PLATFORM_DIR"/iphone/ratatouille.xcodeproj -target ${XCODE_TARGET}-angle -configuration $CONFIG -sdk iphoneos "${ANGLE_BUILD_SETTINGS[@]}" 2>&1 | tee -a "$FULL_LOG_FILE" | grep -E -v "$XCODE_LOG_FILTERS"

# Simulator (includes arm64 for M1 simulator support)
xcodebuild SYMROOT="$SYMROOT" -project "$PLATFORM_DIR"/iphone/ratatouille.xcodeproj -target ${XCODE_TARGET}-angle -configuration $CONFIG -sdk iphonesimulator "${ANGLE_BUILD_SETTINGS[@]}" 2>&1 | tee -a "$FULL_LOG_FILE" | grep -E -v "$XCODE_LOG_FILTERS"

# Mac Catalyst libplayer-angle (non-fatal: OpenGLES headers removed from macOS 26 SDK)
echo "Building ${XCODE_TARGET}-angle for Mac Catalyst (non-fatal)"
xcodebuild SYMROOT="$SYMROOT" -project "$PLATFORM_DIR"/iphone/ratatouille.xcodeproj -target ${XCODE_TARGET}-angle -configuration $CONFIG \
    -sdk macosx SUPPORTS_MACCATALYST=YES IPHONEOS_DEPLOYMENT_TARGET=16.0 ARCHS="arm64 x86_64" "${ANGLE_BUILD_SETTINGS[@]}" \
    2>&1 | tee -a "$FULL_LOG_FILE" | grep -E -v "$XCODE_LOG_FILTERS" || echo "Warning: Mac Catalyst ${XCODE_TARGET}-angle build failed — skipping Catalyst slice"

# create xcframework (device + simulator + catalyst if available)
rm -rf "$DST_LIB_DIR"/libplayer-angle.xcframework
LIBPLAYER_ANGLE_XCF_ARGS=(
    -library "$SYMROOT"/$CONFIG-iphoneos/${XCODE_TARGET}-angle.a        -headers "$PLATFORM_DIR/iphone/Corona"
    -library "$SYMROOT"/$CONFIG-iphonesimulator/${XCODE_TARGET}-angle.a -headers "$PLATFORM_DIR/iphone/Corona"
)
if [ -f "$SYMROOT"/$CONFIG-maccatalyst/${XCODE_TARGET}-angle.a ]; then
    LIBPLAYER_ANGLE_XCF_ARGS+=(-library "$SYMROOT"/$CONFIG-maccatalyst/${XCODE_TARGET}-angle.a -headers "$PLATFORM_DIR/iphone/Corona")
fi
xcodebuild -create-xcframework "${LIBPLAYER_ANGLE_XCF_ARGS[@]}" -output "$DST_LIB_DIR"/libplayer-angle.xcframework

# Also copy device library as standard .a for backward compatibility
cp -v "$SYMROOT"/$CONFIG-iphoneos/${XCODE_TARGET}-angle.a "$DST_LIB_DIR"/libplayer-angle.a

# Create MetalANGLE xcframework (device + simulator + catalyst if available)
rm -rf "$DST_LIB_DIR"/MetalANGLE.xcframework
METALANGLE_XCF_ARGS=(
    -framework "$SYMROOT"/$CONFIG-iphoneos/MetalANGLE.framework
    -framework "$SYMROOT"/$CONFIG-iphonesimulator/MetalANGLE.framework
)
if [ -d "$SYMROOT"/$CONFIG-maccatalyst/MetalANGLE.framework ]; then
    METALANGLE_XCF_ARGS+=(-framework "$SYMROOT"/$CONFIG-maccatalyst/MetalANGLE.framework)
fi
xcodebuild -create-xcframework "${METALANGLE_XCF_ARGS[@]}" -output "$DST_LIB_DIR"/MetalANGLE.xcframework

# copy headers
cp -v "$PLATFORM_DIR/iphone/Corona/"*.h "$DST_INCLUDE_IOS_DIR/Corona"

# copy resources
cp -v "$PLATFORM_DIR/iphone/MainWindow.xib" "$DST_RESOURCE_IOS_DIR"

cp -v "$PLATFORM_DIR/iphone/_CoronaSplashScreen.png" "$DST_RESOURCE_IOS_DIR"

cp -v "$PLATFORM_DIR/iphone/Resources-iPad/MainWindow-iPad.xib" "$DST_RESOURCE_IOS_DIR"

cp -rv "$SYMROOT/$CONFIG-iphoneos"/CoronaResources.bundle "$DST_RESOURCE_IOS_DIR"
