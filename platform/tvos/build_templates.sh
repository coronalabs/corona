#!/bin/bash #-x

#
# Checks exit value for error
#
checkError() {
    if [ $? -ne 0 ]
    then
        echo "Exiting due to errors (above)"
        exit -1
    fi
}

PLATFORM_BASE=appletv

: "${TEMPLATE_TARGET:=template}"
TEMPLATE_TARGET_SUFFIX="${TEMPLATE_TARGET#template}"

# Passed in arguments
# $1 SDK_VERSION
if [ -z "$1" ]
then	
	SDK_VERSION=$(xcrun --sdk ${PLATFORM_BASE}os --show-sdk-version)
else
	SDK_VERSION=$1

	if [ ! -z "$2" ]
	then
		CORONA_BUILD_ID=$2
	else
		CORONA_BUILD_ID="DEV"
	fi
	export CORONA_BUILD_ID
fi


path="$(cd "$(dirname "$0")"; pwd)"

# Summarize xcodebuild output to stdout but save full output in separate file
XCODE_LOG_FILTERS="^    export |clang -x |libtool -static |^    cd $path"

FULL_LOG_FILE="tvos-build_templates-xcodebuild.log"
if [ "$WORKSPACE" != "" ]
then
	FULL_LOG_FILE="$WORKSPACE/$FULL_LOG_FILE"
fi

BUILD_DIR=${path}/build

SDK_DEVICE=${PLATFORM_BASE}os
SDK_SIMULATOR=${PLATFORM_BASE}simulator

# Clean
# -----------------------------------------------------------------------------

rm -rf "${BUILD_DIR}"
xcodebuild SYMROOT="$path/build" -project "${path}"/ratatouille.xcodeproj -configuration Release clean 2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"


# Directories
# -----------------------------------------------------------------------------

echo "Creating directories:"
mkdir -pv "${BUILD_DIR}/template"
mkdir -pv "${BUILD_DIR}/template-dSYM"

mkdir -pv "${BUILD_DIR}/template/${SDK_DEVICE}/${SDK_VERSION}"
mkdir -pv "${BUILD_DIR}/template/${SDK_SIMULATOR}/${SDK_VERSION}"

mkdir -pv "${BUILD_DIR}/template-dSYM/${SDK_DEVICE}/${SDK_VERSION}"
mkdir -pv "${BUILD_DIR}/template-dSYM/${SDK_SIMULATOR}/${SDK_VERSION}"


# Build
# -----------------------------------------------------------------------------

# Environment vars used by internal scripts called within Xcode
export SUPPRESS_APP_SIGN=1
export SUPPRESS_GUI=1

# template device

xcodebuild SYMROOT="$path/build" OTHER_CFLAGS="-fembed-bitcode" -project "${path}"/ratatouille.xcodeproj -target ${TEMPLATE_TARGET} -configuration Release -sdk ${SDK_DEVICE} 2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"
checkError

mv -v "${BUILD_DIR}/Release-${SDK_DEVICE}/template.app" "${BUILD_DIR}/template/${SDK_DEVICE}/${SDK_VERSION}/template.app"
checkError

mv -v "${BUILD_DIR}/Release-${SDK_DEVICE}/template.app.dSYM" "${BUILD_DIR}/template-dSYM/${SDK_DEVICE}/${SDK_VERSION}/template.app.dSYM"
checkError

# template simulator

xcodebuild SYMROOT="$path/build" -project "${path}"/ratatouille.xcodeproj -target ${TEMPLATE_TARGET} -configuration Release -sdk ${SDK_SIMULATOR} 2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"
checkError

mv -v "${BUILD_DIR}/Release-${SDK_SIMULATOR}/template.app" "${BUILD_DIR}/template/${SDK_SIMULATOR}/${SDK_VERSION}/template.app"
checkError

mv -v "${BUILD_DIR}/Release-${SDK_SIMULATOR}/template.app.dSYM" "${BUILD_DIR}/template-dSYM/${SDK_SIMULATOR}/${SDK_VERSION}/template.app.dSYM"
checkError

# ${JOB_NAME} is a Jenkins environment var
if [[ "${JOB_NAME}" =~ .*Enterprise.* ]]
then
	# CoronaCards.framework
	# NOTE: No need to do clean, since we already did a clean build in the above xcodebuild 
	# invocations. This xcodebuild will finish nearly instantaneously.
	xcodebuild SYMROOT="$path/build" OTHER_CFLAGS="-fembed-bitcode" -project "${path}"/ratatouille.xcodeproj -target CoronaCards.framework -configuration Release 2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"
	checkError
fi

