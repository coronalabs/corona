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

# ── MetalANGLE pre-build for the other platform ────────────────────────────
# If MetalANGLE.framework is present in the current BUILT_PRODUCTS_DIR it
# means this is an angle build.  Pre-build MetalANGLE for the other SDK into
# the SAME SYMROOT so the sub-build below finds it cached and does not attempt
# to re-link it in the sub-project context (which would fail without the right
# OTHER_LDFLAGS environment).
PROJECT_DIR="$(dirname "${PROJECT_FILE_PATH}")"
METALANGLE_XCODEPROJ="${PROJECT_DIR}/../../external/MetalANGLE/ios/xcode/OpenGLES.xcodeproj"
if [[ -d "${BUILT_PRODUCTS_DIR}/MetalANGLE.framework" && -f "${METALANGLE_XCODEPROJ}" ]]; then
    echo "Pre-building MetalANGLE.framework for ${SF_OTHER_PLATFORM}..."
    xcodebuild build \
        -project "${METALANGLE_XCODEPROJ}" \
        -target MetalANGLE \
        -configuration "${CONFIGURATION}" \
        -sdk "${SF_OTHER_PLATFORM}${SF_SDK_VERSION}" \
        SYMROOT="${SYMROOT}" \
        SKIP_INSTALL=YES \
        DEPLOYMENT_POSTPROCESSING=NO \
        "OTHER_LDFLAGS=-weak_framework OpenGLES \$(inherited)" \
        || echo "Warning: MetalANGLE pre-build for ${SF_OTHER_PLATFORM} failed — sub-build will attempt it"
fi
# ── end MetalANGLE pre-build ────────────────────────────────────────────────

# Build the other platform.
# Pass ALLOW_NON_MODULAR_INCLUDES_IN_FRAMEWORK_MODULES so that angle builds
# importing MetalANGLE (whose MGLContext.h includes non-modular EGL/egl.h)
# don't fail here — command-line settings from the outer xcodebuild are not
# inherited by sub-builds.
# Non-fatal: if MetalANGLE can't be linked for the simulator in this context
# (e.g. CI sub-project environment differences), we skip that slice and still
# produce an iphoneos-only xcframework.
SF_OTHER_BUILD_OK=1
xcodebuild -project "${PROJECT_FILE_PATH}" -target "${TARGET_NAME}" -configuration "${CONFIGURATION}" -sdk "${SF_OTHER_PLATFORM}${SF_SDK_VERSION}" BUILD_DIR="${BUILD_DIR}" OBJROOT="${OBJROOT}/DependantBuilds" BUILD_ROOT="${BUILD_ROOT}" SYMROOT="${SYMROOT}" ALLOW_NON_MODULAR_INCLUDES_IN_FRAMEWORK_MODULES=YES "$ACTION" \
    || { echo "Warning: ${SF_OTHER_PLATFORM} build failed — skipping ${SF_OTHER_PLATFORM} slice"; SF_OTHER_BUILD_OK=0; }

# Copy the static library into each platform's framework bundle
cp -a "${BUILT_PRODUCTS_DIR}/${SF_EXECUTABLE_PATH}" "${BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}/Versions/A/${SF_TARGET_NAME}"
if [[ "${SF_OTHER_BUILD_OK}" = "1" && -f "${SF_OTHER_BUILT_PRODUCTS_DIR}/${SF_EXECUTABLE_PATH}" ]]; then
    cp -a "${SF_OTHER_BUILT_PRODUCTS_DIR}/${SF_EXECUTABLE_PATH}" "${SF_OTHER_BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}/Versions/A/${SF_TARGET_NAME}"
fi

# ── Mac Catalyst slice ──────────────────────────────────────────────────────
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
    # Non-fatal: on macOS 26+ SDK the OpenGLES headers are gone, so non-angle targets
    # cannot build for maccatalyst. Capture the exit code and skip the Catalyst slice
    # rather than failing the whole build.
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
               "$ACTION" || { echo "Warning: Mac Catalyst build failed — skipping Catalyst slice"; SF_CATALYST_WRAPPER=""; }

    # The Catalyst build may not have created the .framework wrapper (because
    # build_framework.sh exited early due to SF_MASTER_SCRIPT_RUNNING, or because
    # the build was skipped due to failure above).
    # Seed the wrapper from the iOS device slice and replace the binary.
    if [[ -n "${SF_CATALYST_WRAPPER}" ]]; then
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
fi
# ── end Mac Catalyst ─────────────────────────────────────────────────────────

# Create XCFramework instead of fat binary (supports M1 simulators where both device and simulator use arm64)
XCFRAMEWORK_PATH="${BUILT_PRODUCTS_DIR}/${SF_TARGET_NAME}.xcframework"
rm -rf "${XCFRAMEWORK_PATH}"

# Build the argument list dynamically based on which slices were built successfully.
XCFRAMEWORK_ARGS=(-framework "${BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}")
if [[ "${SF_OTHER_BUILD_OK}" = "1" && -d "${SF_OTHER_BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}" ]]; then
    XCFRAMEWORK_ARGS+=(-framework "${SF_OTHER_BUILT_PRODUCTS_DIR}/${SF_WRAPPER_NAME}")
fi
if [[ -n "${SF_CATALYST_WRAPPER}" && -d "${SF_CATALYST_WRAPPER}" ]]; then
    XCFRAMEWORK_ARGS+=(-framework "${SF_CATALYST_WRAPPER}")
fi

xcodebuild -create-xcframework "${XCFRAMEWORK_ARGS[@]}" -output "${XCFRAMEWORK_PATH}"

# Also create XCFramework in the other build products directory
if [[ "${SF_OTHER_BUILD_OK}" = "1" && -d "${SF_OTHER_BUILT_PRODUCTS_DIR}" ]]; then
    cp -a "${XCFRAMEWORK_PATH}" "${SF_OTHER_BUILT_PRODUCTS_DIR}/${SF_TARGET_NAME}.xcframework"
fi
