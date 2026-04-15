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

# iOSSupport framework path — needed for MetalANGLE macCatalyst prebuild
MACOS_SDK="$(xcrun --sdk macosx --show-sdk-path)"
IOSSUPP="${MACOS_SDK}/System/iOSSupport/System/Library/Frameworks"

# MetalANGLE include dir (provides GLES2/gl2.h, GLES3/gl3.h, EGL/egl.h, …)
METALANGLE_INCLUDE="$SCRIPT_DIR/../../external/MetalANGLE/include"
METALANGLE_INCLUDE="$(cd "$METALANGLE_INCLUDE" && pwd)"

# Where prebuild products land (same BUILD_DIR used for angle library builds)
METALANGLE_PRODUCTS="$BUILD_ROOT/maccatalyst/Release"

# ── macCatalyst build-setting groups ──────────────────────────────────────────
#
# CATALYST_PREBUILD — used only when building OpenGLES.xcodeproj directly.
#   Includes LLVM_TARGET_TRIPLE_SUFFIX so that EVERY target in OpenGLES.xcodeproj
#   (static libs AND the framework) is compiled with the arm64-apple-ios16.0-macabi
#   triple.  Without this, the per-target static libs (libangle_base.a, …) get the
#   plain macOS triple and the linker rejects them with
#   "building for macCatalyst but linking object built for macOS".
#
# CATALYST_BUILD — used when building libplayer-angle / CoronaCards-angle from
#   ratatouille.xcodeproj.
#   • LLVM_TARGET_TRIPLE_SUFFIX="-macabi" is included here (same as PREBUILD) so
#     that Xcode properly sets up the macCatalyst SDK environment — in particular
#     adding iOSSupport to SYSTEM_FRAMEWORK_SEARCH_PATHS so <UIKit/UIKit.h> resolves.
#     Without it Xcode does not recognise the sdk=macosx build as macCatalyst and
#     UIKit (and other iOS frameworks) are not found.
#   • MetalANGLE and MetalANGLE_static have been removed from the Xcode target-
#     dependency lists in ratatouille.xcodeproj, so Xcode will NOT attempt to
#     rebuild them as sub-projects (the propagation issue that originally required
#     the split no longer applies).  Xcode uses the pre-built products already in
#     the shared BUILD_DIR.
#   • FRAMEWORK_SEARCH_PATHS puts the prebuild output dir first (MetalANGLE.framework),
#     then iOSSupport (UIKit, Foundation, …), then the macOS frameworks.
#   • HEADER_SEARCH_PATHS includes opengl-compat (fake <OpenGLES/…> wrappers)
#     and the MetalANGLE include tree (real GLES2/gl2.h, …).
# ─────────────────────────────────────────────────────────────────────────────

CATALYST_PREBUILD=(
    SUPPORTS_MACCATALYST=YES
    IS_MACCATALYST=YES
    TARGETED_DEVICE_FAMILY="1,2"
    IPHONEOS_DEPLOYMENT_TARGET=16.0
    MACCATALYST_DEPLOYMENT_TARGET=16.0
    LLVM_TARGET_TRIPLE_SUFFIX="-macabi"
    LLVM_TARGET_TRIPLE_OS_VERSION="ios16.0"
    ARCHS="arm64 x86_64"
    "FRAMEWORK_SEARCH_PATHS=${IOSSUPP} \$(SDKROOT)/System/Library/Frameworks"
)

# opengl-compat dir is created later (after BUILD_ROOT exists)
OPENGL_COMPAT="$BUILD_ROOT/opengl-compat"

CATALYST_BUILD=(
    SUPPORTS_MACCATALYST=YES
    TARGETED_DEVICE_FAMILY="1,2"
    IPHONEOS_DEPLOYMENT_TARGET=16.0
    MACCATALYST_DEPLOYMENT_TARGET=16.0
    LLVM_TARGET_TRIPLE_SUFFIX="-macabi"
    LLVM_TARGET_TRIPLE_OS_VERSION="ios16.0"
    ARCHS="arm64 x86_64"
    "FRAMEWORK_SEARCH_PATHS=$METALANGLE_PRODUCTS ${IOSSUPP} \$(SDKROOT)/System/Library/Frameworks"
    "HEADER_SEARCH_PATHS=$OPENGL_COMPAT $METALANGLE_INCLUDE \$(inherited)"
    # Rtt_EGL: allows #include <EGL/egl.h> in Corona source (Rtt_GLGeometry,
    #   Rtt_GLFrameBufferObject) so eglGetProcAddress is declared.  MetalANGLE
    #   provides a full EGL implementation so the calls succeed at runtime.
    #   Defining Rtt_EGL also causes the anonymous namespace in Rtt_GLGeometry.cpp
    #   to declare glBindVertexArrayOES / glGenVertexArraysOES as file-scope function
    #   pointer variables (PFNGL*PROC type), which serve as the sole in-TU declaration
    #   of those names and are initialised via eglGetProcAddress at runtime.
    #
    # NOTE: do NOT add GL_GLEXT_PROTOTYPES here.  MetalANGLE's gl2ext.h guards the
    #   OES vertex-array prototypes behind #ifdef GL_GLEXT_PROTOTYPES; if that macro
    #   is defined while Rtt_EGL is also defined the compiler sees both a function-
    #   pointer variable (anonymous namespace) and a function prototype for the same
    #   name → "reference to 'glBindVertexArrayOES' is ambiguous".
    "GCC_PREPROCESSOR_DEFINITIONS=\$(inherited) Rtt_EGL"
)

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

# ── Create <OpenGLES/ES2/gl.h> compatibility shim ────────────────────────────
#
# On macOS 15+ iOSSupport no longer ships OpenGLES.framework, so
# #include <OpenGLES/ES2/gl.h> (used in Rtt_GL.h) would fail to resolve.
# MetalANGLE provides the equivalent headers under GLES2/*, EGL/*, etc.
# We create a tiny shim tree that redirects the iOS-style paths to the
# MetalANGLE ANGLE include tree, then add both dirs to HEADER_SEARCH_PATHS.
# ─────────────────────────────────────────────────────────────────────────────
log "Creating <OpenGLES/ES2/gl.h> shim headers"
mkdir -p \
    "$OPENGL_COMPAT/OpenGLES/ES2" \
    "$OPENGL_COMPAT/OpenGLES/ES3"

cat > "$OPENGL_COMPAT/OpenGLES/ES2/gl.h"    <<'SHIM'
/* OpenGLES shim for macCatalyst — redirects to MetalANGLE GLES2 headers */
#pragma once
#include <GLES2/gl2.h>
SHIM

cat > "$OPENGL_COMPAT/OpenGLES/ES2/glext.h" <<'SHIM'
/* OpenGLES shim for macCatalyst — redirects to MetalANGLE GLES2 headers */
#pragma once
#include <GLES2/gl2ext.h>
/* iOS SDK defines GL_BGRA (0x80E1) unconditionally; MetalANGLE only has GL_BGRA_EXT */
#ifndef GL_BGRA
#  define GL_BGRA GL_BGRA_EXT
#endif
SHIM

cat > "$OPENGL_COMPAT/OpenGLES/ES3/gl.h"    <<'SHIM'
/* OpenGLES shim for macCatalyst — redirects to MetalANGLE GLES3 headers */
#pragma once
#include <GLES3/gl3.h>
SHIM

cat > "$OPENGL_COMPAT/OpenGLES/ES3/gl2.h"   <<'SHIM'
/* OpenGLES shim for macCatalyst — redirects to MetalANGLE GLES3 headers */
#pragma once
#include <GLES3/gl3.h>
SHIM

mkdir -p "$OPENGL_COMPAT/OpenGLES/ES1"

cat > "$OPENGL_COMPAT/OpenGLES/ES1/gl.h"    <<'SHIM'
/* OpenGLES ES1 shim for macCatalyst — MetalANGLE does not support ES1.
   Corona only includes this header for its macro side-effects; none of the
   actual ES1 symbols are used in the Mac Catalyst code path. */
#pragma once
SHIM

cat > "$OPENGL_COMPAT/OpenGLES/ES1/glext.h" <<'SHIM'
/* OpenGLES ES1 glext shim for macCatalyst — empty intentionally.
   MetalANGLE's GLES/glext.h uses GLclampx (an ES1 fixed-point type) which
   is not defined in the ES2+ MetalANGLE headers.  Corona's only use of this
   header in Rtt_PlatformSurface.cpp is a bare #include with no symbols from
   it referenced in the Mac Catalyst code path, so an empty shim is correct. */
#pragma once
SHIM

ok "Shim headers written to $OPENGL_COMPAT"

# ── METALANGLE_PROJECT ────────────────────────────────────────────────────────
METALANGLE_PROJECT="$SCRIPT_DIR/../../external/MetalANGLE/ios/xcode/OpenGLES.xcodeproj"
METALANGLE_PROJECT="$(cd "$(dirname "$METALANGLE_PROJECT")" && pwd)/$(basename "$METALANGLE_PROJECT")"

# ── prebuild_metalangle_catalyst ──────────────────────────────────────────────
#
# Builds MetalANGLE.framework (dynamic) AND libMetalANGLE_static.a from
# OpenGLES.xcodeproj with full macabi settings.
#
# Why direct invocation instead of letting Xcode handle the sub-project?
#   When ratatouille.xcodeproj builds libplayer-angle, it triggers a sub-project
#   build of OpenGLES.xcodeproj.  Command-line overrides passed to the parent
#   (ratatouille) do NOT propagate to the sub-project's own xcodebuild invocation.
#   This means the per-target static libs (libangle_base.a, libangle_common.a, …)
#   get compiled with the plain macOS triple instead of the macabi triple, and the
#   linker fails with "building for macCatalyst but linking object built for macOS".
#
#   By pre-building directly from OpenGLES.xcodeproj with LLVM_TARGET_TRIPLE_SUFFIX
#   as a command-line override, ALL targets in OpenGLES.xcodeproj (including the
#   angle static libs) receive the macabi triple.  The same BUILD_DIR is then used
#   for the ratatouille-based builds so Xcode finds the products already present.
#
#   MetalANGLE and MetalANGLE_static have been removed from the Xcode
#   target-dependency lists in ratatouille.xcodeproj so Xcode does not attempt a
#   sub-project rebuild during the main build.
# ─────────────────────────────────────────────────────────────────────────────
prebuild_metalangle_catalyst() {
    local DIR="$BUILD_ROOT/maccatalyst"
    mkdir -p "$DIR"

    local BUILD_ARGS=(
        -configuration Release
        -sdk           macosx
        "${CATALYST_PREBUILD[@]}"
        BUILD_DIR="$DIR"
        OBJROOT="$DIR/obj"
        BUILD_ROOT="$DIR"
        SYMROOT="$DIR"
        SKIP_INSTALL=YES
        DEPLOYMENT_POSTPROCESSING=NO
    )

    log "Pre-building MetalANGLE.framework for Mac Catalyst"
    xcodebuild build \
        -project "$METALANGLE_PROJECT" \
        -target   MetalANGLE \
        "${BUILD_ARGS[@]}" 2>&1 \
    | grep --line-buffered -E \
        "(^error:| error:|BUILD SUCCEEDED|BUILD FAILED)" \
    | grep -v "note:" \
    || true

    log "Pre-building libMetalANGLE_static.a for Mac Catalyst"
    xcodebuild build \
        -project "$METALANGLE_PROJECT" \
        -target   MetalANGLE_static \
        "${BUILD_ARGS[@]}" 2>&1 \
    | grep --line-buffered -E \
        "(^error:| error:|BUILD SUCCEEDED|BUILD FAILED)" \
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
        "(^error:| error:|BUILD SUCCEEDED|BUILD FAILED)" \
    | grep -v "note:" \
    || true
}

# ── find_lib ──────────────────────────────────────────────────────────────────
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

log "——— libplayer ———"
build_static_lib libplayer       iphoneos        ios-arm64
build_static_lib libplayer       iphonesimulator ios-sim

# Mac Catalyst: pre-build MetalANGLE framework + static lib with macabi triple,
# then build libplayer-angle using the pre-built products.
prebuild_metalangle_catalyst

build_static_lib libplayer-angle macosx          maccatalyst \
    "${CATALYST_BUILD[@]}"

LIBPLAYER_IOS="$(find_lib player       ios-arm64)"
LIBPLAYER_SIM="$(find_lib player       ios-sim)"
_LIBPLAYER_CAT_ANGLE="$(find_lib player-angle maccatalyst)"
LIBPLAYER_CAT_STAGED="$BUILD_ROOT/staged/libplayer.a"
mkdir -p "$BUILD_ROOT/staged"
cp "$_LIBPLAYER_CAT_ANGLE" "$LIBPLAYER_CAT_STAGED"
ok "iOS device    : $LIBPLAYER_IOS"
ok "iOS simulator : $LIBPLAYER_SIM"
ok "Mac Catalyst  : $_LIBPLAYER_CAT_ANGLE  →  $LIBPLAYER_CAT_STAGED"

# ── Build: CoronaCards ────────────────────────────────────────────────────────

log "——— CoronaCards ———"
build_static_lib CoronaCards       iphoneos        ios-arm64
build_static_lib CoronaCards       iphonesimulator ios-sim
build_static_lib CoronaCards-angle macosx          maccatalyst \
    "${CATALYST_BUILD[@]}"

LIBCORONA_IOS="$(find_lib CoronaCards       ios-arm64)"
LIBCORONA_SIM="$(find_lib CoronaCards       ios-sim)"
LIBCORONA_CAT="$(find_lib CoronaCards-angle maccatalyst)"
ok "iOS device    : $LIBCORONA_IOS"
ok "iOS simulator : $LIBCORONA_SIM"
ok "Mac Catalyst  : $LIBCORONA_CAT"

# ── Package: libplayer.xcframework ───────────────────────────────────────────

log "Packaging libplayer.xcframework"

LIBPLAYER_XCF="$OUTPUT_DIR/libplayer.xcframework"
xcodebuild -create-xcframework \
    -library "$LIBPLAYER_IOS"        -headers "$LIBPLAYER_HEADERS" \
    -library "$LIBPLAYER_SIM"        -headers "$LIBPLAYER_HEADERS" \
    -library "$LIBPLAYER_CAT_STAGED" -headers "$LIBPLAYER_HEADERS" \
    -output  "$LIBPLAYER_XCF"

ok "libplayer.xcframework  →  $LIBPLAYER_XCF"

# ── Package: CoronaCards.xcframework ─────────────────────────────────────────

log "Creating CoronaCards.framework slice bundles"

make_static_framework() {
    local LIB="$1" DEST="$2"
    local FW="$DEST/CoronaCards.framework"
    rm -rf "$FW"
    mkdir -p "$FW/Headers" "$FW/Modules"

    cp "$LIB" "$FW/CoronaCards"
    cp "$CORONACARDS_HEADERS"/*.h "$FW/Headers/" 2>/dev/null || true

    cat > "$FW/Modules/module.modulemap" <<'MODULEMAP'
framework module CoronaCards {
  umbrella header "CoronaCards.h"
  export *
  module * { export * }
}
MODULEMAP

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
