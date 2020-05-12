#!/bin/bash -x

# builds output.zip for Android

# $1 = app name
# $2 = path to template dir
# $3 = path to tmp build dir
# $4 = path to certificate file
# $5 = path to input zip file // main.lu file (or src file used to create message digest)
# $6 = "little" or "big" (endianness of file)
# $7 = app bundle identifier
# $8 = corona app id

#
# Checks exit value for error
# 
checkError() {
	# shellcheck disable=SC2181
	if [ $? -ne 0 ]
	then
		echo "Exiting due to errors (above)"
		exit -1
	fi
}

path=$(dirname "$0")

appName="$1"
TEMPLATE_DIR="$2"

# assign tmp dir to $dst
TMP_DIR="$3"

if [ -z "$ANDROID_SDK" ]
then
	ANDROID_SDK_SERVER="/home/ansca-web/util-versions/sdk/android/r11"
	if [ -e $ANDROID_SDK_SERVER ]
	then
		ANDROID_SDK=$ANDROID_SDK_SERVER
	else
		echo "ERROR: ANDROID_SDK environment variable must be defined"
		exit -1
	fi
fi

# Canonicalize paths
pushd "$path" > /dev/null
dir=$(pwd)
path=$dir
popd > /dev/null

pushd "$TEMPLATE_DIR" > /dev/null
dir=$(pwd)
TEMPLATE_DIR=$dir
popd > /dev/null

pushd "$TMP_DIR" > /dev/null
dir=$(pwd)
TMP_DIR=$dir
popd > /dev/null

# add a divider between builds
printf "\n---" >> /tmp/android_ant_output
date >> /tmp/android_ant_output

################
# resource.car #
################

# Unpack input zip file
INPUT_DIR=$TMP_DIR/input
mkdir "$INPUT_DIR"

unzip -o "$5" -d "$INPUT_DIR"
checkError

if [ -z "$CAR_PATH" ]
then
	CAR_PATH=$path/car
fi
$CAR_PATH "$TMP_DIR/resource.car" "$INPUT_DIR"/*.lu
checkError

rm -f "$INPUT_DIR"/*.lu




#################
# Create Output #
#################

OUTPUT_DIR=$TMP_DIR

# Copy template.apk into working dir
cp "$TEMPLATE_DIR/template.apk" "$OUTPUT_DIR"
checkError

# sign exe with signature of resource.car
LIB_NAME=libcorona.so
LIB_REL_PATH=lib/armeabi-v7a/$LIB_NAME
unzip -o "$OUTPUT_DIR/template.apk" "$LIB_REL_PATH" -d "$OUTPUT_DIR"
checkError

LUA=lua
UNAME=$(uname)
TIMEOUT=
TIMEOUT_SEC=
if [ "Darwin" != "$UNAME" ]
then
	TIMEOUT=timeout
	TIMEOUT_SEC=60
else
	LUA=$path/../../bin/mac/lua
fi

pushd "$OUTPUT_DIR" > /dev/null

# Create assets/resource.car
mkdir assets
cp "$TMP_DIR/resource.car" assets

# Create proper AndroidManifest.xml
mkdir -p "$OUTPUT_DIR/res/values/" # for strings.xml
$LUA -e "package.path=package.path..';$path/../resources/?.lua'" "$TEMPLATE_DIR/update_manifest.lua" "$TEMPLATE_DIR/AndroidManifest.xml" "$INPUT_DIR/build.properties" "$appName" "$OUTPUT_DIR/AndroidManifest.xml" "" "$TEMPLATE_DIR/strings.xml" "$OUTPUT_DIR/res/values/strings.xml" "$TMP_DIR/copy-files-to-apk.properties"
checkError

cp AndroidManifest.xml AndroidManifest.plain.xml

# Fetch Android API Level that Corona is built with from the AndroidManifest.xml file.
# This is needed to select the correct version of Android SDK to use with the AAPT command below.
SDK_API_LEVEL=$(xpath "$OUTPUT_DIR/AndroidManifest.xml" 'string(//manifest/uses-sdk/@android:targetSdkVersion)' 2> /dev/null)
if [ -z "$SDK_API_LEVEL" ]
then
	SDK_API_LEVEL=10
fi

# Select the right 'aapt' command line tool for the API Level we're building with.
# Note: The newest 'aapt' tool moves un-versioned "res/drawable" images to a "res/drawable-v4" folder.
#       To prevent this from breaking older builds, we must select the older tool for server builds.
if [ $SDK_API_LEVEL -gt 16 ]
then
	# Use the newest build-tools version.
	AAPT=$(find $ANDROID_SDK/build-tools -name aapt | sort -r | head -1)
else
	# Use version 19.1, which is backward compatible with older Corona builds using API Level 16 or lower.
	AAPT=$ANDROID_SDK/build-tools/19.1.0/aapt
fi
if [ ! -x "$AAPT" ]
then
	echo "$0: cannot find 'aapt' command in ANDROID_SDK"
	exit 1
fi

echo "Using AAPT: $AAPT"

# Update with new AndroidManifest.xml
if [ -s "$TEMPLATE_DIR/external-libs-paths" ]
then
	EXTERNAL_LIBS=$(cat "$TEMPLATE_DIR/external-libs-paths")
	PROCESSED_LINE=$(eval echo "$EXTERNAL_LIBS")
	echo "$AAPT p -v -M \"$OUTPUT_DIR/AndroidManifest.xml\" --auto-add-overlay -I \"$ANDROID_SDK/platforms/android-$SDK_API_LEVEL/android.jar\" -F \"$OUTPUT_DIR/tmp.apk\" -S \"$TEMPLATE_DIR/res\" $PROCESSED_LINE"
	# shell check complains that $PROCESSED_LINE should be quoted to prevent splitting but we want it to be split so ignore that
	# shellcheck disable=SC2086
	$AAPT p -v -M "$OUTPUT_DIR/AndroidManifest.xml" --auto-add-overlay -I "$ANDROID_SDK/platforms/android-$SDK_API_LEVEL/android.jar" -F "$OUTPUT_DIR/tmp.apk" -S "$TEMPLATE_DIR/res" $PROCESSED_LINE
	checkError
else
	echo "$AAPT p -v -M \"$OUTPUT_DIR/AndroidManifest.xml\" --auto-add-overlay -I \"$ANDROID_SDK/platforms/android-$SDK_API_LEVEL/android.jar\" -F \"$OUTPUT_DIR/tmp.apk\" -S \"$OUTPUT_DIR/res\" -S \"$TEMPLATE_DIR/res\""
	$AAPT p -v -M "$OUTPUT_DIR/AndroidManifest.xml" --auto-add-overlay -I "$ANDROID_SDK/platforms/android-$SDK_API_LEVEL/android.jar" -F "$OUTPUT_DIR/tmp.apk" -S "$TEMPLATE_DIR/res" -S "$OUTPUT_DIR/res"
	checkError
fi

# Replace old AndroidManifest.xml and resources.arsc (extract binary versions from tmp.apk)
unzip -o tmp.apk AndroidManifest.xml resources.arsc
checkError

$AAPT r -v "$OUTPUT_DIR/template.apk" AndroidManifest.xml
$AAPT a -v "$OUTPUT_DIR/template.apk" AndroidManifest.xml
$AAPT r -v "$OUTPUT_DIR/template.apk" resources.arsc # this gets us "res/values/strings.xml"
$AAPT a -v "$OUTPUT_DIR/template.apk" resources.arsc
checkError

# Add assets/resource.car
zip -v "$OUTPUT_DIR/template.apk" assets/*
checkError

# Update libcorona.so
zip -v "$OUTPUT_DIR/template.apk" $LIB_REL_PATH
checkError

# Create output.zip
zip -j -D -0 output.zip template.apk copy-files-to-apk.properties
checkError

popd > /dev/null
