#!/bin/bash -e

# -----------------------------------------------------------------------------
# NOTES:
# * Forked from: https://gist.github.com/cromandini/1a9c4aeab27ca84f5d79
# * Designed to be invoked from inside an Xcode build phase
# -----------------------------------------------------------------------------

# Specifies the base SDK. Default is "iphone". Alternatives include: "appletv"
PLATFORM_BASE=$1

# Optional params to specify where output goes
OUTPUT_DIR=$2

# Optional params to override Xcode environment variables
ARG_CONFIGURATION=$3
ARG_TARGET_NAME=$4

if [ -z "${PLATFORM_BASE}" ]
then
	PLATFORM_BASE=iphone
fi

if [ -z "${OUTPUT_DIR}" ]
then
	OUTPUT_DIR=${PROJECT_DIR}/build
fi

if [ -z "${CONFIGURATION}" ]
then
	CONFIGURATION=${ARG_CONFIGURATION}

	if [ -z "${CONFIGURATION}" ]
	then
		# Default
		CONFIGURATION=Release
	fi
fi

if [ -z "${TARGET_NAME}" ]
then
	TARGET_NAME=${ARG_TARGET_NAME}

	if [ -z "${TARGET_NAME}" ]
	then
		echo "Error: Missing 'TARGET_NAME' environment var specifying the target in the Xcode project"
		exit -1
	fi
fi

if [ ! -d "${OUTPUT_DIR}" ]
then
	mkdir -p ${OUTPUT_DIR}
fi

echo "---------------------------------"
echo "Target:        ${TARGET_NAME}"
echo "Configuration: ${CONFIGURATION}"
echo "Platform:      ${PLATFORM_BASE}"
echo "Output dir:    ${OUTPUT_DIR}"
echo "---------------------------------"

SDK_DEVICE=${PLATFORM_BASE}os
SDK_SIMULATOR=${PLATFORM_BASE}simulator

# -----------------------------------------------------------------------------

TARGET_XCODE=`basename -s .framework $TARGET_NAME`

UNIVERSAL_OUTPUTFOLDER=${OUTPUT_DIR}/${CONFIGURATION}-universal

# make sure the output directory exists
mkdir -p "${UNIVERSAL_OUTPUTFOLDER}"

# passing through this setting will break bitcode generation
unset TOOLCHAINS

# STEP 1. Build Device and Simulator versions
xcodebuild -project "${PROJECT_FILE_PATH}" -target "${TARGET_XCODE}" -configuration ${CONFIGURATION} -sdk $SDK_DEVICE ONLY_ACTIVE_ARCH=NO BUILD_DIR="${OUTPUT_DIR}" BUILD_ROOT="${OUTPUT_DIR}" OBJROOT="${OUTPUT_DIR}/DependentBuilds" build
xcodebuild -project "${PROJECT_FILE_PATH}" -target "${TARGET_XCODE}" -configuration ${CONFIGURATION} -sdk $SDK_SIMULATOR ONLY_ACTIVE_ARCH=NO BUILD_DIR="${OUTPUT_DIR}" BUILD_ROOT="${OUTPUT_DIR}" OBJROOT="${OUTPUT_DIR}/DependentBuilds" build

# STEP 2. Copy the framework structure (from $SDK_DEVICE build) to the universal folder
# cp -R "${OUTPUT_DIR}/${CONFIGURATION}-$SDK_DEVICE/${TARGET_XCODE}.framework" "${UNIVERSAL_OUTPUTFOLDER}/"

# # dSYM on release builds
# if [ "${CONFIGURATION}" == "Release" ]
# then
# 	cp -R "${OUTPUT_DIR}/${CONFIGURATION}-$SDK_DEVICE/${TARGET_XCODE}.framework.dSYM" "${UNIVERSAL_OUTPUTFOLDER}/"
# fi


# # STEP 3. Copy Swift modules from $SDK_SIMULATOR build (if it exists) to the copied framework directory
# SIMULATOR_SWIFT_MODULES_DIR="${OUTPUT_DIR}/${CONFIGURATION}-$SDK_SIMULATOR/${TARGET_XCODE}.framework/Modules/${TARGET_XCODE}.swiftmodule/."
# if [ -d "${SIMULATOR_SWIFT_MODULES_DIR}" ]; then
# cp -R "${SIMULATOR_SWIFT_MODULES_DIR}" "${UNIVERSAL_OUTPUTFOLDER}/${TARGET_XCODE}.framework/Modules/${TARGET_XCODE}.swiftmodule"
# fi


# # STEP 4. Create universal binary file using lipo and place the combined executable in the copied framework directory
# lipo -create -output "${UNIVERSAL_OUTPUTFOLDER}/${TARGET_XCODE}.framework/${TARGET_XCODE}" \
# 	"${OUTPUT_DIR}/${CONFIGURATION}-$SDK_SIMULATOR/${TARGET_XCODE}.framework/${TARGET_XCODE}" \
# 	"${OUTPUT_DIR}/${CONFIGURATION}-$SDK_DEVICE/${TARGET_XCODE}.framework/${TARGET_XCODE}"

# # dSYM on release builds
# if [ "${CONFIGURATION}" == "Release" ]
# then
# 	lipo -create -output "${UNIVERSAL_OUTPUTFOLDER}/${TARGET_XCODE}.framework.dSYM/Contents/Resources/DWARF/${TARGET_XCODE}" \
# 		"${OUTPUT_DIR}/${CONFIGURATION}-$SDK_SIMULATOR/${TARGET_XCODE}.framework.dSYM/Contents/Resources/DWARF/${TARGET_XCODE}" \
# 		"${OUTPUT_DIR}/${CONFIGURATION}-$SDK_DEVICE/${TARGET_XCODE}.framework.dSYM/Contents/Resources/DWARF/${TARGET_XCODE}"
# fi


# STEP 5. Convenience step to copy the framework to the project's directory
# cp -R "${UNIVERSAL_OUTPUTFOLDER}/${TARGET_XCODE}.framework" "${OUTPUT_DIR}"


# STEP 6. Make xcframework
DSYM_PATH="${OUTPUT_DIR}/${CONFIGURATION}-$SDK_SIMULATOR/${TARGET_XCODE}.framework.dSYM"
if [ -d "$DSYM_PATH" ]
then
	DSYM_COMMAND=(-debug-symbols "$DSYM_PATH")
fi
rm -rf "${UNIVERSAL_OUTPUTFOLDER}/${TARGET_XCODE}.xcframework"
xcodebuild -create-xcframework -framework "${OUTPUT_DIR}/${CONFIGURATION}-$SDK_SIMULATOR/${TARGET_XCODE}.framework" \
							   -framework "${OUTPUT_DIR}/${CONFIGURATION}-$SDK_DEVICE/${TARGET_XCODE}.framework" \
							   "${DSYM_COMMAND[@]}" \
							   -output "${UNIVERSAL_OUTPUTFOLDER}/${TARGET_XCODE}.xcframework"


# STEP 7. Convenience step to open the project's directory in Finder
if [ -z "${SUPPRESS_GUI}" ]
then
	open "${UNIVERSAL_OUTPUTFOLDER}"
fi
