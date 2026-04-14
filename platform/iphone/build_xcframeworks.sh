#!/usr/bin/env bash
# =============================================================================
# build_xcframeworks.sh
#
# Builds libplayer.xcframework and CoronaCards.xcframework for iOS with:
#   • iOS device        (arm64)
#   • iOS Simulator     (arm64 + x86_64)
#   • Mac Catalyst      (arm64 + x86_64)
#
# Installs the result into App/Resources/ of the Patterned World Tour project
# if found at the expected path relative to this repo.
#
# Usage:
#   cd corona/platform/iphone
#   ./build_xcframeworks.sh
#
# Optional override:
#   ./build_xcframeworks.sh --output /absolute/path/to/output/dir
# =============================================================================

set -euo pipefail

# ── Paths ─────────────────────────────────────────────────────────────────────

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT="$SCRIPT_DIR/ratatouille.xcodeproj"
BUILD_ROOT="${TMPDIR:-/tmp}/corona-xcfw-$$"

OUTPUT_DIR="$SCRIPT_DIR/output-xcframeworks"
if [[ "${1:-}" == "--output" && -n "${2:-}" ]]; then
    OUTPUT_DIR="$2"
fi

# Public headers bundled with each slice
LIBPLAYER_HEADERS="$SCRIPT_DIR/Corona"
CORONACARDS_HEADERS="$SCRIPT_DIR/CoronaCards"

# Patterned World Tour game project resources (auto-detect)
GAME_RESOURCES=""
for _c in \
        "$SCRIPT_DIR/../../../patterned-worldtour/App/Resources" \
        "$HOME/Desktop/patterned-worldtour/App/Resources"; do
    if [[ -d "$_c" ]]; then
        GAME_RESOURCES="$(cd "$_c" && pwd)"; break
    fi
done

# ── Helpers ───────────────────────────────────────────────────────────────────

log()  { printf '\n\033[1;34m==> %s\033[0m\n' "$*"; }
ok()   { printf '\033[0;32m✓  %s\033[0m\n' "$*"; }
warn() { printf '\033[0;33m⚠  %s\033[0m\n' "$*"; }
fail() { printf '\033[0;31mERROR: %s\033[0m\n' "$*" >&2; exit 1; }

[[ -f "$PROJECT/project.pbxproj" ]] \
    || fail "ratatouille.xcodeproj not found at $SCRIPT_DIR"

# ── Prepare ───────────────────────────────────────────────────────────────────

log "Preparing build area"
rm -rf "$BUILD_ROOT" "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"
ok "Build root : $BUILD_ROOT"
ok "Output dir : $OUTPUT_DIR"

# ── METALANGLE_PROJECT ────────────────────────────────────────────────────────
# ratatouille.xcodeproj embeds OpenGLES.xcodeproj as a sub-project.
METALANGLE_PROJECT="$(dirname "$PROJECT")/../external/MetalANGLE/ios/xcode/OpenGLES.xcodeproj"
METALANGLE_PROJECT="$(cd "$(dirname "$METALANGLE_PROJECT")" && pwd)/$(basename "$METALANGLE_PROJECT")"

# ── prebuild_metalangle_catalyst ──────────────────────────────────────────────
# The libplayer-angle target depends on MetalANGLE (dynamic framework).
# When xcodebuild builds OpenGLES.xcodeproj as a sub-project for macOS, it does
# NOT inherit SUPPORTS_MACCATALYST from the parent — so UIKit is unavailable and
# the framework fails to link.
#
# Fix: pre-build MetalANGLE.framework from OpenGLES.xcodeproj directly into the
# same BUILD_DIR using the correct Mac Catalyst settings:
#   • LLVM_TARGET_TRIPLE_SUFFIX / LLVM_TARGET_TRIPLE_OS_VERSION — force macabi
#     target triple (x86_64-apple-ios16.0-macabi) rather than macos26
#   • SYSTEM_FRAMEWORK_SEARCH_PATHS — adds $(SDKROOT)/System/iOSSupport/… so
#     #include <UIKit/UIKit.h> resolves against the Catalyst UIKit stubs
#
# Xcode's incremental build then finds MetalANGLE.framework already present in
# BUILD_DIR/Release/ and skips re-building it when libplayer-angle is compiled.
# ─────────────────────────────────────────────────────────────────────────────
prebuild_metalangle_catalyst() {
    local DIR="$BUILD_ROOT/maccatalyst"
    mkdir -p "$DIR"

    log "Pre-building MetalANGLE.framework for Mac Catalyst"

    local MACOS_SDK; MACOS_SDK="$(xcrun --sdk macosx --show-sdk-path)"

    xcodebuild build \
        -project "$METALANGLE_PROJECT" \
        -target   MetalANGLE \
        -configuration Release \
        -sdk       macosx \
        SUPPORTS_MACCATALYST=YES \
        IS_MACCATALYST=YES \
        TARGETED_DEVICE_FAMILY="1,2" \
        IPHONEOS_DEPLOYMENT_TARGET=16.0 \
        MACCATALYST_DEPLOYMENT_TARGET=16.0 \
        LLVM_TARGET_TRIPLE_SUFFIX="-macabi" \
        LLVM_TARGET_TRIPLE_OS_VERSION="ios16.0" \
        ARCHS="arm64 x86_64" \
        "SYSTEM_FRAMEWORK_SEARCH_PATHS=${MACOS_SDK}/System/iOSSupport/System/Library/Frameworks \$(SDKROOT)/System/Library/Frameworks" \
        BUILD_DIR="$DIR" \
        OBJROOT="$DIR/obj" \
        BUILD_ROOT="$DIR" \
        SYMROOT="$DIR" \
        SKIP_INSTALL=YES \
        DEPLOYMENT_POSTPROCESSING=NO 2>&1 \
    | grep --line-buffered -E \
        "(^(error:|warning:)| error:| warning:|BUILD SUCCEEDED|BUILD FAILED)" \
    | grep -v "note:" \
    || true
}

# ── build_static_lib ──────────────────────────────────────────────────────────
# build_static_lib TARGET SDK SUBDIR [EXTRA_XCODEBUILD_SETTINGS...]
#
# Compiles a static-library (or aggregate) target for one platform slice.
# Sets SF_MASTER_SCRIPT_RUNNING=1 so that any build_framework.sh build phase
# on aggregate targets exits immediately — we handle xcframework packaging.
# ─────────────────────────────────────────────────────────────────────────────
build_static_lib() {
    local TARGET="$1" SDK="$2" SUBDIR="$3"
    shift 3
    local DIR="$BUILD_ROOT/$SUBDIR"
    mkdir -p "$DIR"

    log "Building $TARGET  [sdk=$SDK${*:+  | $*}]"

    SF_MASTER_SCRIPT_RUNNING=1 \
    xcodebuild build \
        -project "$PROJECT" \
        -target   "$TARGET" \
        -configuration Release \
        -sdk       "$SDK" \
        BUILD_DIR="$DIR" \
        OBJROOT="$DIR/obj" \
        BUILD_ROOT="$DIR" \
        SYMROOT="$DIR" \
        SKIP_INSTALL=YES \
        DEPLOYMENT_POSTPROCESSING=NO \
        "$@" 2>&1 \
    | grep --line-buffered -E \
        "(^(error:|warning:)| error:| warning:|BUILD SUCCEEDED|BUILD FAILED)" \
    | grep -v "note:" \
    || true
}

# ── find_lib ──────────────────────────────────────────────────────────────────
# Prints the path of the built .a — searches all Xcode output subdirs
# (Release/, Release-maccatalyst/, etc.) because Xcode names them differently
# depending on platform.
# ─────────────────────────────────────────────────────────────────────────────
find_lib() {
    local NAME="$1" SUBDIR="$2"
    local LIB
    LIB="$(find "$BUILD_ROOT/$SUBDIR" \
               -name "lib${NAME}.a" \
               -not -path "*/obj/*" \
           | head -1)"
    [[ -n "$LIB" ]] \
        || fail "lib${NAME}.a not found under $SUBDIR — check build output above"
    echo "$LIB"
}

# ── Build: libplayer ──────────────────────────────────────────────────────────
#
# iOS device + simulator use the standard OpenGL ES target.
# Mac Catalyst uses libplayer-angle (MetalANGLE → Metal) because macOS has no
# OpenGL ES headers.  Xcode builds the MetalANGLE _mac sub-project dependencies
# automatically when libplayer-angle is the target.
# ─────────────────────────────────────────────────────────────────────────────

log "——— libplayer ———"
build_static_lib libplayer       iphoneos        ios-arm64
build_static_lib libplayer       iphonesimulator ios-sim

# Pre-build MetalANGLE with Catalyst settings so sub-project dependency resolves
prebuild_metalangle_catalyst

build_static_lib libplayer-angle macosx          maccatalyst \
    SUPPORTS_MACCATALYST=YES \
    TARGETED_DEVICE_FAMILY="1,2" \
    IPHONEOS_DEPLOYMENT_TARGET=16.0 \
    ARCHS="arm64 x86_64"

LIBPLAYER_IOS="$(find_lib player       ios-arm64)"
LIBPLAYER_SIM="$(find_lib player       ios-sim)"
# Angle target produces libplayer-angle.a; rename to libplayer.a so all three
# slices have the same binary name inside the xcframework.
_LIBPLAYER_CAT_ANGLE="$(find_lib player-angle maccatalyst)"
LIBPLAYER_CAT_STAGED="$BUILD_ROOT/staged/libplayer.a"
mkdir -p "$BUILD_ROOT/staged"
cp "$_LIBPLAYER_CAT_ANGLE" "$LIBPLAYER_CAT_STAGED"
ok "iOS device    : $LIBPLAYER_IOS"
ok "iOS simulator : $LIBPLAYER_SIM"
ok "Mac Catalyst  : $_LIBPLAYER_CAT_ANGLE  →  $LIBPLAYER_CAT_STAGED"

# ── Build: CoronaCards ────────────────────────────────────────────────────────
#
# Same rationale: use CoronaCards-angle for Mac Catalyst.
# ─────────────────────────────────────────────────────────────────────────────

log "——— CoronaCards ———"
build_static_lib CoronaCards       iphoneos        ios-arm64
build_static_lib CoronaCards       iphonesimulator ios-sim
build_static_lib CoronaCards-angle macosx          maccatalyst \
    SUPPORTS_MACCATALYST=YES \
    TARGETED_DEVICE_FAMILY="1,2" \
    IPHONEOS_DEPLOYMENT_TARGET=16.0 \
    ARCHS="arm64 x86_64"

LIBCORONA_IOS="$(find_lib CoronaCards       ios-arm64)"
LIBCORONA_SIM="$(find_lib CoronaCards       ios-sim)"
LIBCORONA_CAT="$(find_lib CoronaCards-angle maccatalyst)"
ok "iOS device    : $LIBCORONA_IOS"
ok "iOS simulator : $LIBCORONA_SIM"
ok "Mac Catalyst  : $LIBCORONA_CAT"

# ── Package: libplayer.xcframework ───────────────────────────────────────────
#
# Uses the -library form (not -framework) because libplayer has always been
# distributed as a plain static-library xcframework — matches the structure
# already in App/Resources/libplayer.xcframework.
# ─────────────────────────────────────────────────────────────────────────────

log "Packaging libplayer.xcframework"

LIBPLAYER_XCF="$OUTPUT_DIR/libplayer.xcframework"
xcodebuild -create-xcframework \
    -library "$LIBPLAYER_IOS"        -headers "$LIBPLAYER_HEADERS" \
    -library "$LIBPLAYER_SIM"        -headers "$LIBPLAYER_HEADERS" \
    -library "$LIBPLAYER_CAT_STAGED" -headers "$LIBPLAYER_HEADERS" \
    -output  "$LIBPLAYER_XCF"

ok "libplayer.xcframework  →  $LIBPLAYER_XCF"

# ── Package: CoronaCards.xcframework ─────────────────────────────────────────
#
# The existing *tvOS* CoronaCards binary is a dynamic framework (mh_dylib).
# For iOS, the ratatouille.xcodeproj produces a static library (libCoronaCards.a).
#
# We wrap each static .a inside a minimal .framework bundle so the xcframework
# tool can identify the platform correctly from the Mach-O metadata.
# Mixing static (iOS) and dynamic (tvOS) slices in one xcframework is valid —
# each slice is consumed independently by the linker for its target platform.
# Since the user only needs iOS + Mac Catalyst right now, this xcframework
# contains only those slices.  The tvOS target keeps its own separate framework.
# ─────────────────────────────────────────────────────────────────────────────

log "Creating CoronaCards.framework slice bundles"

make_static_framework() {
    local LIB="$1" DEST="$2"
    local FW="$DEST/CoronaCards.framework"
    rm -rf "$FW"
    mkdir -p "$FW/Headers" "$FW/Modules"

    # Binary: static archive renamed to match framework naming convention
    cp "$LIB" "$FW/CoronaCards"

    # Public headers
    cp "$CORONACARDS_HEADERS"/*.h "$FW/Headers/" 2>/dev/null || true

    # Module map (identical to the tvOS version)
    cat > "$FW/Modules/module.modulemap" <<'MODULEMAP'
framework module CoronaCards {
  umbrella header "CoronaCards.h"
  export *
  module * { export * }
}
MODULEMAP

    # Minimal Info.plist
    cat > "$FW/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
    "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>        <string>CoronaCards</string>
    <key>CFBundleIdentifier</key>  <string>com.coronalabs.CoronaCards</string>
    <key>CFBundleVersion</key>     <string>1.0</string>
    <key>CFBundlePackageType</key> <string>FMWK</string>
    <key>MinimumOSVersion</key>    <string>16.0</string>
</dict>
</plist>
PLIST

    echo "$FW"
}

STAGES="$BUILD_ROOT/fw-stages"
mkdir -p "$STAGES"/{ios-arm64,ios-sim,maccatalyst}

FW_IOS="$(make_static_framework "$LIBCORONA_IOS" "$STAGES/ios-arm64")"
FW_SIM="$(make_static_framework "$LIBCORONA_SIM" "$STAGES/ios-sim")"
FW_CAT="$(make_static_framework "$LIBCORONA_CAT" "$STAGES/maccatalyst")"
ok "Framework bundle stubs created"

log "Packaging CoronaCards.xcframework (iOS + Catalyst)"
CORONACARDS_XCF="$OUTPUT_DIR/CoronaCards.xcframework"
xcodebuild -create-xcframework \
    -framework "$FW_IOS" \
    -framework "$FW_SIM" \
    -framework "$FW_CAT" \
    -output    "$CORONACARDS_XCF"

ok "CoronaCards.xcframework  →  $CORONACARDS_XCF"

# Summarise slices
printf '\nSlices built:\n'
plutil -p "$CORONACARDS_XCF/Info.plist" 2>/dev/null \
    | grep "LibraryIdentifier\|BinaryPath" \
    | sed 's/^/  /' || true

# ── Install to game project ───────────────────────────────────────────────────

if [[ -n "$GAME_RESOURCES" ]]; then
    log "Installing xcframeworks → $GAME_RESOURCES"
    cp -Rf "$LIBPLAYER_XCF"   "$GAME_RESOURCES/libplayer.xcframework"
    cp -Rf "$CORONACARDS_XCF" "$GAME_RESOURCES/CoronaCards.xcframework"
    ok "libplayer.xcframework installed"
    ok "CoronaCards.xcframework installed"
else
    warn "Game project not found — copy manually from $OUTPUT_DIR to App/Resources/"
fi

# ── Cleanup & summary ─────────────────────────────────────────────────────────

rm -rf "$BUILD_ROOT"

log "Build complete"
printf '\n  libplayer.xcframework   →  %s\n' "$LIBPLAYER_XCF"
printf '  CoronaCards.xcframework →  %s\n\n' "$CORONACARDS_XCF"
