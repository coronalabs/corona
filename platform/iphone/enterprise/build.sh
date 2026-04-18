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
# Uses ios16.0-macabi triple + VFS overlay for AppKit/UIKit collision fix.
echo "Building ${XCODE_TARGET} for Mac Catalyst (non-fatal)"
xcodebuild SYMROOT="$SYMROOT" -project "$PLATFORM_DIR"/iphone/ratatouille.xcodeproj -target ${XCODE_TARGET} -configuration $CONFIG \
    -sdk macosx SUPPORTS_MACCATALYST=YES IPHONEOS_DEPLOYMENT_TARGET=16.0 ARCHS="arm64 x86_64" \
    "${CATALYST_COMMON_SETTINGS[@]}" \
    2>&1 | tee -a "$FULL_LOG_FILE" | grep -E -v "$XCODE_LOG_FILTERS" || echo "Warning: Mac Catalyst ${XCODE_TARGET} build failed — skipping Catalyst slice"

# create xcframework (device + simulator + catalyst if available)
# Mac Catalyst builds go to Release/ (not Release-maccatalyst/) when using -sdk macosx
rm -rf "$DST_LIB_DIR"/libplayer.xcframework
LIBPLAYER_XCF_ARGS=(
    -library "$SYMROOT"/$CONFIG-iphoneos/${XCODE_TARGET}.a         -headers "$PLATFORM_DIR/iphone/Corona"
    -library "$SYMROOT"/$CONFIG-iphonesimulator/${XCODE_TARGET}.a  -headers "$PLATFORM_DIR/iphone/Corona"
)
if [ -f "$SYMROOT"/$CONFIG/${XCODE_TARGET}.a ]; then
    LIBPLAYER_XCF_ARGS+=(-library "$SYMROOT"/$CONFIG/${XCODE_TARGET}.a -headers "$PLATFORM_DIR/iphone/Corona")
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

# macOS 26 SDK / Mac Catalyst build helpers
# ---------------------------------------------------------------------------
# The macOS 26 SDK has a bug: AppKit.h includes headers (NSCollectionViewCompositionalLayout,
# NSDiffableDataSource, NSLayoutConstraint, etc.) that redefine types already provided by
# UIKit's iOSSupport overlay, causing "duplicate interface definition" errors on Catalyst.
# We create a clang VFS overlay that stubs those conflicting headers with empty files,
# so the real types from UIKit remain in effect.
#
# Additionally, the ios16.0-macabi target triple must be used (not macos-macabi) so that
# TARGET_OS_IPHONE=1, which makes SDK headers (WKWebView.h, MapKit.h, etc.) take the
# UIKit path instead of the AppKit path.
MACOS_SDK=$(xcrun --sdk macosx --show-sdk-path)
IOSUPPORT_FW="$MACOS_SDK/System/iOSSupport/System/Library/Frameworks"
APPKIT_HEADERS="$MACOS_SDK/System/Library/Frameworks/AppKit.framework/Headers"

# Generate the VFS overlay file (stubs conflicting AppKit headers)
CATALYST_VFS="$SYMROOT/catalyst-appkit-vfs.yaml"
CATALYST_STUBS_DIR="$SYMROOT/catalyst-appkit-stubs"
mkdir -p "$CATALYST_STUBS_DIR"
CONFLICT_HEADERS=(
    "NSCollectionViewCompositionalLayout.h"
    "NSDiffableDataSource.h"
    "NSLayoutAnchor.h"
    "NSLayoutConstraint.h"
    "NSLayoutGuide.h"
    "NSStackView.h"
    "NSTextStorage.h"
    "NSLayoutManager.h"
)
for hdr in "${CONFLICT_HEADERS[@]}"; do
    cat > "$CATALYST_STUBS_DIR/$hdr" << EOF
/* Mac Catalyst shim: AppKit/${hdr}
 * Suppressed — UIKit iOSSupport already defines these types.
 * Redefining them from AppKit causes duplicate definition errors on Mac Catalyst. */
#pragma once
EOF
done

# Also stub the missing AVKit/AVCaptureView.h (macOS-only header absent from iOSSupport)
AVKIT_STUBS_DIR="$SYMROOT/catalyst-avkit-stubs"
mkdir -p "$AVKIT_STUBS_DIR"
cat > "$AVKIT_STUBS_DIR/AVCaptureView.h" << 'EOF'
/* Mac Catalyst shim: AVKit/AVCaptureView.h
 * AVCaptureView is a macOS-only NSView-based class, absent from iOSSupport.
 * This empty shim prevents a fatal "file not found" from iOSSupport AVKit.h. */
#pragma once
EOF

python3 - <<PYEOF
import json
appkit = "$APPKIT_HEADERS"
stubs  = "$CATALYST_STUBS_DIR"
avkit_sdk = "$MACOS_SDK/System/iOSSupport/System/Library/Frameworks/AVKit.framework/Headers"
avkit_stubs = "$AVKIT_STUBS_DIR"
conflict = [
    "NSCollectionViewCompositionalLayout.h","NSDiffableDataSource.h",
    "NSLayoutAnchor.h","NSLayoutConstraint.h","NSLayoutGuide.h",
    "NSStackView.h","NSTextStorage.h","NSLayoutManager.h",
]
roots = [
    {"name": appkit, "type": "directory",
     "contents": [{"name": h, "type": "file",
                   "external-contents": stubs+"/"+h} for h in conflict]},
    {"name": avkit_sdk, "type": "directory",
     "contents": [{"name": "AVCaptureView.h", "type": "file",
                   "external-contents": avkit_stubs+"/AVCaptureView.h"}]},
]
with open("$CATALYST_VFS","w") as f:
    json.dump({"version":0,"redirecting-with":"fallthrough","roots":roots},f,indent=2)
print("VFS overlay: $CATALYST_VFS")
PYEOF

# Also create a metalangle-headers symlink so <MetalANGLE/MGLKit.h> is found on Catalyst
METALANGLE_HEADERS_DIR="$SYMROOT/metalangle-headers"
mkdir -p "$METALANGLE_HEADERS_DIR"
# (symlink is set up after MetalANGLE iphoneos build below)

# Catalyst-specific build settings for libplayer and libplayer-angle
# - ios16.0-macabi triple: makes TARGET_OS_IPHONE=1 so SDK headers take UIKit path
# - VFS overlay: stubs conflicting AppKit headers
# - FRAMEWORK_SEARCH_PATHS: adds iOSSupport so UIKit/WebKit/MapKit are found
CATALYST_COMMON_SETTINGS=(
    "FRAMEWORK_SEARCH_PATHS=$IOSUPPORT_FW \$(inherited)"
    "LLVM_TARGET_TRIPLE_OS_VERSION=ios16.0"
    "LLVM_TARGET_TRIPLE_SUFFIX=-macabi"
    "OTHER_CFLAGS=-ivfsoverlay $CATALYST_VFS \$(inherited)"
    "OTHER_CPLUSPLUSFLAGS=-ivfsoverlay $CATALYST_VFS \$(inherited)"
)

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

# Create metalangle-headers symlink so <MetalANGLE/MGLKit.h> is found during Mac Catalyst
# libplayer-angle build (MetalANGLE.framework is iOS-only, but the headers are reusable).
rm -rf "$METALANGLE_HEADERS_DIR/MetalANGLE"
ln -sf "$SYMROOT/Release-iphoneos/MetalANGLE.framework/Headers" "$METALANGLE_HEADERS_DIR/MetalANGLE"
echo "MetalANGLE headers linked: $METALANGLE_HEADERS_DIR/MetalANGLE"

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

# Mac Catalyst MetalANGLE (non-fatal: OpenGLES absent from macOS 26 SDK)
# Do NOT pass -weak_framework OpenGLES here: macOS 26 SDK has no OpenGLES.framework
# at all, so the linker cannot resolve even a weak reference to it.
# MetalANGLE IS the OpenGLES implementation (Metal-backed), so it doesn't need
# to link against the system OpenGLES on Mac Catalyst.
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
    OTHER_LDFLAGS="\$(inherited)" \
    2>&1 | tee -a "$FULL_LOG_FILE" | grep -E "(BUILD SUCCEEDED|BUILD FAILED|error:|Undefined symbol|ld:)" || \
    echo "Warning: MetalANGLE Mac Catalyst build failed — skipping Catalyst slice"

# libplayer-angle build settings:
# - HEADER_SEARCH_PATHS includes metalangle-headers symlink so <MetalANGLE/MGLKit.h> is found
# - GCC_PREPROCESSOR_DEFINITIONS must include Rtt_MetalANGLE: command-line overrides target-level,
#   so we must carry it through to prevent GLKit→AppKit fallback in Rtt_MetalAngleTypes.h
ANGLE_BUILD_SETTINGS=(
    "HEADER_SEARCH_PATHS=$METALANGLE_HEADERS_DIR $METALANGLE_INCLUDE $GLSLANG_DIR \$(inherited)"
    "GCC_PREPROCESSOR_DEFINITIONS=\$(inherited) Rtt_EGL Rtt_MetalANGLE"
    "ALLOW_NON_MODULAR_INCLUDES_IN_FRAMEWORK_MODULES=YES"
    "EXCLUDED_SOURCE_FILE_NAMES=AutoLinkModules.m"
)

xcodebuild SYMROOT="$SYMROOT" -project "$PLATFORM_DIR"/iphone/ratatouille.xcodeproj -target ${XCODE_TARGET}-angle -configuration $CONFIG -sdk iphoneos "${ANGLE_BUILD_SETTINGS[@]}" 2>&1 | tee -a "$FULL_LOG_FILE" | grep -E -v "$XCODE_LOG_FILTERS"

# Simulator (includes arm64 for M1 simulator support)
xcodebuild SYMROOT="$SYMROOT" -project "$PLATFORM_DIR"/iphone/ratatouille.xcodeproj -target ${XCODE_TARGET}-angle -configuration $CONFIG -sdk iphonesimulator "${ANGLE_BUILD_SETTINGS[@]}" 2>&1 | tee -a "$FULL_LOG_FILE" | grep -E -v "$XCODE_LOG_FILTERS"

# Mac Catalyst libplayer-angle: uses ios16.0-macabi triple + VFS overlay for AppKit fix.
# - ios16.0-macabi makes TARGET_OS_IPHONE=1 so WKWebView.h/MapKit.h take the UIKit path
# - VFS overlay stubs conflicting AppKit headers that UIKit iOSSupport already defines
# - Rtt_MetalANGLE must be explicit (command-line GCC_PREPROCESSOR_DEFINITIONS overrides target)
echo "Building ${XCODE_TARGET}-angle for Mac Catalyst (non-fatal)"
CATALYST_ANGLE_SETTINGS=(
    "${ANGLE_BUILD_SETTINGS[@]}"
    "${CATALYST_COMMON_SETTINGS[@]}"
)
xcodebuild SYMROOT="$SYMROOT" -project "$PLATFORM_DIR"/iphone/ratatouille.xcodeproj -target ${XCODE_TARGET}-angle -configuration $CONFIG \
    -sdk macosx SUPPORTS_MACCATALYST=YES IPHONEOS_DEPLOYMENT_TARGET=16.0 ARCHS="arm64 x86_64" \
    "${CATALYST_ANGLE_SETTINGS[@]}" \
    2>&1 | tee -a "$FULL_LOG_FILE" | grep -E -v "$XCODE_LOG_FILTERS" || echo "Warning: Mac Catalyst ${XCODE_TARGET}-angle build failed — skipping Catalyst slice"

# create xcframework (device + simulator + catalyst if available)
# Mac Catalyst builds go to Release/ (not Release-maccatalyst/) when using -sdk macosx
rm -rf "$DST_LIB_DIR"/libplayer-angle.xcframework
LIBPLAYER_ANGLE_XCF_ARGS=(
    -library "$SYMROOT"/$CONFIG-iphoneos/${XCODE_TARGET}-angle.a        -headers "$PLATFORM_DIR/iphone/Corona"
    -library "$SYMROOT"/$CONFIG-iphonesimulator/${XCODE_TARGET}-angle.a -headers "$PLATFORM_DIR/iphone/Corona"
)
if [ -f "$SYMROOT"/$CONFIG/${XCODE_TARGET}-angle.a ]; then
    LIBPLAYER_ANGLE_XCF_ARGS+=(-library "$SYMROOT"/$CONFIG/${XCODE_TARGET}-angle.a -headers "$PLATFORM_DIR/iphone/Corona")
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
