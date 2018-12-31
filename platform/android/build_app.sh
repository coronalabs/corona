#!/bin/bash -x
#
# Sample build file for compiling a test project
#
# - In terminal, run "./build.sh"

# If stdout is the terminal, save a copy of the output to a log file
if [ -t 1 ]
then
	# Redirect stdout ( > ) into a named pipe ( >() ) running "tee"
	exec > >(tee "$(basename "$0" .sh).log")
	# Copy stderr to stdout
	exec 2>&1
fi

path=$(dirname "$0")

SCRIPT_ROOT=$path

# This script is at a fixed location relative to the corona repo root
CORONA_ROOT=$path/../..

#
# Prints usage
# 
usage() {
	echo ""
	echo "USAGE: $0 [--release|--debug] [--clean|--incremental] project_path [ndk_path sdk_path device_type product_type]"
	echo "   project_path: the path to the Corona project folder"
	echo "   ndk__path: optional override to env variable ANDROID_NDK"
	echo "   sdk__path: optional override to env variable ANDROID_SDK"
	echo "   device_type: optional override to specify generic, kindle, nook_only, or all"
	echo "   product_type: optional override to specify trial, basic, automation, coronacards, or all"
	echo "NOTE: if ndk_path is supplied, so must sdk_path"
	exit -1
}

#
# Checks exit value for error
# 
checkError() {
	# shellcheck disable=SC2181
	if [ $? -ne 0 ]
	then
		echo "Exiting due to ${1:-'errors (above)'}"
		exit -1
	fi
}


# handle optional arguments
if [ "$1" = "--release" ]
then
	BUILD_CONFIG=release
	shift
fi

if [ "$1" = "--debug" ]
then
	BUILD_CONFIG=debug
	shift
fi

if [ "$1" = "--clean" ]
then
	BUILD_TYPE=clean
	shift
fi

if [ "$1" = "--incremental" ]
then
	BUILD_TYPE=incremental
	shift
fi

# $1 path/to/projectDir
PROJECT_PATH=$1

# $2 path/to/ndk
# $3 path/to/sdk
if [ ! -z "$2" ]
then	
	NDK_PATH=$2
	SDK_PATH=$3

	if [ ! -z "$4" ]
	then
		DEVICE_TYPE=$4
	fi
	if [ ! -z "$5" ]
	then
		PRODUCT_TYPE=$5
	fi
else
	NDK_PATH=$ANDROID_NDK
	SDK_PATH=$ANDROID_SDK
	if [ -z "$NDK_PATH" ]
	then
		echo "ERROR: ANDROID_NDK env variable not found"
		usage
	elif [ -z "$SDK_PATH" ]
	then
		echo "ERROR: ANDROID_SDK env variable not found"
		usage
	fi
fi

#
# Verify parameters
#
if [ ! -d "PROJECT_PATH" ] && [ ! -e "$PROJECT_PATH/main.lua" ]
then
    echo "ERROR: Corona project folder ($PROJECT_PATH) is not valid or main.lua file does not exist"
    usage
fi

if [ ! -d "$NDK_PATH" ] && [ ! -e "$NDK_PATH/ndk-build" ]
then
    echo "ERROR: Android NDK path ($NDK_PATH) is not valid"
    usage
fi

if [ ! -d "$SDK_PATH" ]
then
    echo "ERROR: Android SDK path ($SDK_PATH) is not valid"
    usage
fi

if [ ! -d "$CORONA_ROOT" ] 
then
    echo "ERROR: Corona root path ($CORONA_ROOT) is not valid"
    usage
fi


# 
# Canonicalize relative paths to absolute paths
# 
pushd "$SCRIPT_ROOT" > /dev/null
dir=$(pwd)
SCRIPT_ROOT=$dir
popd > /dev/null

pushd "$PROJECT_PATH" > /dev/null
dir=$(pwd)
PROJECT_PATH=$dir
popd > /dev/null

pushd "$NDK_PATH" > /dev/null
dir=$(pwd)
NDK_PATH=$dir
popd > /dev/null

pushd "$SDK_PATH" > /dev/null
dir=$(pwd)
SDK_PATH=$dir
popd > /dev/null

pushd "$CORONA_ROOT" > /dev/null
dir=$(pwd)
CORONA_ROOT=$dir
popd > /dev/null

CERTIFICATE_PATH=${CORONA_ROOT}/platform/resources/developerkey.cert

if [ ! -e "$CERTIFICATE_PATH" ] 
then
    echo "Certificate file ($CERTIFICATE_PATH) used by app_sign could not be found"
    usage
fi


###############################################################################

# -----------------------------------------------------------------------------
# Defaults
# -----------------------------------------------------------------------------

if [ -z "$DEVICE_TYPE" ]
then
	DEVICE_TYPE=all
fi

if [ -z "$PRODUCT_TYPE" ]
then
	PRODUCT_TYPE=all
fi

if [ -z "$BUILD_CONFIG" ]
then
	BUILD_CONFIG=debug
fi

if [ -z "$BUILD_TYPE" ]
then
	BUILD_TYPE=incremental
fi

# -----------------------------------------------------------------------------
# Make sure shell tools are available
# -----------------------------------------------------------------------------

if [ "$BUILD_TYPE" = "clean" ]
then
	XCODE_CLEAN_ACTION="clean"
else
	XCODE_CLEAN_ACTION=
fi

if [ ! -e "${CORONA_ROOT}/bin/mac/app_sign" ] || [ "$BUILD_TYPE" = "clean" ] ; then
	echo "xcodebuild -project \"${CORONA_ROOT}/platform/mac/app_sign.xcodeproj\" $XCODE_CLEAN_ACTION build"
	xcodebuild -project "${CORONA_ROOT}/platform/mac/app_sign.xcodeproj" $XCODE_CLEAN_ACTION build
fi

if [ ! -e "${CORONA_ROOT}/bin/mac/car" ] || [ "$BUILD_TYPE" = "clean" ] ; then
	echo "xcodebuild -project \"${CORONA_ROOT}/platform/mac/car.xcodeproj\" $XCODE_CLEAN_ACTION build"
	xcodebuild -project "${CORONA_ROOT}/platform/mac/car.xcodeproj" $XCODE_CLEAN_ACTION build
fi

if [ ! -e "${CORONA_ROOT}/bin/mac/luac" ] || [ "$BUILD_TYPE" = "clean" ] ; then
	echo "xcodebuild -project \"${CORONA_ROOT}/platform/mac/lua.xcodeproj\" -target luac $XCODE_CLEAN_ACTION build"
	xcodebuild -project "${CORONA_ROOT}/platform/mac/lua.xcodeproj" -target luac $XCODE_CLEAN_ACTION build
fi

if [ ! -e "${CORONA_ROOT}/bin/mac/lua" ] || [ "$BUILD_TYPE" = "clean" ] ; then
	echo "xcodebuild -project \"${CORONA_ROOT}/platform/mac/lua.xcodeproj\" -target lua $XCODE_CLEAN_ACTION build"
	xcodebuild -project "${CORONA_ROOT}/platform/mac/lua.xcodeproj" -target lua $XCODE_CLEAN_ACTION build
fi

# Default LUA_DIR if it isn't already set
export LUA_DIR="${LUA_DIR:-${CORONA_ROOT}/bin/mac}"

# -----------------------------------------------------------------------------
# Create tmp dir
# -----------------------------------------------------------------------------

CLIENT_TMP_DIR=$(mktemp -d /tmp/temp.XXXXXXXXXXXXXXXX)
SERVER_TMP_DIR=$(mktemp -d /tmp/temp.XXXXXXXXXXXXXXXX)

#
# Display arguments this script will use
#
echo "Using the following:"
echo "	Corona PROJECT_PATH = $PROJECT_PATH"
echo "	Corona PROJECT_NAME = $PROJECT_NAME"
echo "	Android NDK_PATH = $NDK_PATH"
echo "	Android SDK_PATH = $SDK_PATH"
echo "	CORONA_ROOT = $CORONA_ROOT"
echo "	CLIENT_TMP_DIR = $CLIENT_TMP_DIR"
echo "	SERVER_TMP_DIR = $SERVER_TMP_DIR"
echo "	LUA_DIR = $LUA_DIR"
echo ""


# -----------------------------------------------------------------------------
# Build template
# -----------------------------------------------------------------------------

export ANDROID_NDK=${NDK_PATH}
export ANDROID_SDK=${SDK_PATH}

if [ "$PRODUCT_TYPE" = "coronacards" ]
then
	BUILD_SCRIPT="$SCRIPT_ROOT/build_template_cards.sh"
else
	BUILD_SCRIPT="$SCRIPT_ROOT/build_template.sh"
fi

"$BUILD_SCRIPT" $PRODUCT_TYPE $BUILD_CONFIG $BUILD_TYPE $DEVICE_TYPE
checkError "errors encountered building template.apk"

# -----------------------------------------------------------------------------
# Build input.zip
# -----------------------------------------------------------------------------

echo " "
echo "-------------------------------------------------"
echo "input.zip"
echo "-------------------------------------------------"

"$SCRIPT_ROOT/build_server_input.sh" $BUILD_CONFIG "$PROJECT_PATH" "$CLIENT_TMP_DIR"
checkError

# -----------------------------------------------------------------------------
# Build output.zip
# -----------------------------------------------------------------------------

echo " "
echo "-------------------------------------------------"
echo "output.zip"
echo "-------------------------------------------------"

# Create project name without spaces
PROJECT_NAME=$(basename "$PROJECT_PATH" | sed "s/ //g")

TEMPLATE_DIR=$SCRIPT_ROOT/template
export CAR_PATH=$SCRIPT_ROOT/../../bin/mac/car
export APP_SIGN_PATH=$SCRIPT_ROOT/../../bin/mac/app_sign
"$path/build_server_output.sh" "$PROJECT_NAME" "$TEMPLATE_DIR" "$SERVER_TMP_DIR" "$CERTIFICATE_PATH" "$CLIENT_TMP_DIR/input.zip" little
checkError

cp "$SERVER_TMP_DIR/output.zip" "$CLIENT_TMP_DIR"

# -----------------------------------------------------------------------------
# Post process
# -----------------------------------------------------------------------------

echo " "
echo "-------------------------------------------------"
echo "Post process"
echo "-------------------------------------------------"

BUNDLE_DIR=$SCRIPT_ROOT/resources

set -x
ant -buildfile "$BUNDLE_DIR/build.xml" build \
	-D'sdk.dir'="$SDK_PATH" \
	-DTEMP_DIR="$CLIENT_TMP_DIR" \
	-DSRC_DIR="$PROJECT_PATH" \
	-DDST_DIR="$PROJECT_PATH" \
	-DBUNDLE_DIR="$BUNDLE_DIR" \
	-DUSER_APP_NAME="$PROJECT_NAME" \
	-DLIVE_BUILD='NO' \
	-DLIVE_BUILD_ASSET_DIR='' \
	-DKS="$BUNDLE_DIR/debug.keystore" \
	-DKP="android" \
	-DKA="androiddebugkey" \
	-DAP="android"

