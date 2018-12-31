#!/bin/bash
#
# build_dmg.sh
#
# Build the OS X DMG for the CoronaSDK product
#

# 1st arg is dst path
# 2nd arg is path to docs root
# 3rd arg is Corona Folder name
# 4th arg custom id
# ./build_sdk.sh ~/Desktop /Volumes/rtt/docs

set -e # exit on any error

TOOLSPATH=$(dirname "$0")
PRODUCT_DIR=Corona
DSTDIR=$(date "+Corona.%Y.%m.%d")
CUSTOM_ID=""
S3_BUCKET=""
FULL_BUILD_NUM=""
DAILY_BUILD='false'
ENTERPRISE=""

RESOURCE_DIR="Resource Library"
SAMPLECODE_DIR="SampleCode"
NATIVE_DIR="Native"

while getopts 'dfb:c:s:e:' flag; do
  case "${flag}" in
    b) FULL_BUILD_NUM="${OPTARG}" ;;
    c) CUSTOM_ID="${OPTARG}" ;;
    d) DAILY_BUILD='true' ;;
    s) S3_BUCKET="${OPTARG}" ;;
    e) ENTERPRISE="${OPTARG}" ;;
	f) FORCE_MOUNT='true' ;;
    *) error "Unexpected option ${flag}" ;;
  esac
done

shift $((OPTIND - 1))

DSTBASE="$1"
TMPPATH="$1/$DSTDIR"
DOCSRC="$2/SDK"

if [ ! -d "$1" ] || [ ! -d "$2" ]
then
        echo "USAGE: $0 [-d] -b FULL_BUILD_NUM [-c CUSTOM_ID] [-s S3_BUCKET] [-e ENTERPRISE_TARBALL] destdir docsroot"
        exit 1
fi

if [ "${FULL_BUILD_NUM}" != "" ]
then
	# Get the base buildnum with a dash at the front of it suitable for concatenating with "${PRODUCT_DIR}"
	# (the code that follows is ok with this not being set)
	# shellcheck disable=2001
	BUILD_NUM=$(echo "$FULL_BUILD_NUM" | sed -e 's/.*\.\(.*\)/-\1/')
else
	echo "Error: FULL_BUILD_NUM is a required parameter"
	exit 1
fi

if [ -d "/Volumes/${PRODUCT_DIR}${BUILD_NUM}" ]
then
	if [ "$FORCE_MOUNT" == true ]
	then
		hdiutil detach "/Volumes/${PRODUCT_DIR}${BUILD_NUM}"
	else
		echo "Error: '${PRODUCT_DIR}${BUILD_NUM}' volume already mounted.  Unmount and try again."
		exit 1
	fi
fi

set -x 

if [[ -e "$TMPPATH" ]]
then
	echo "Cleaning $TMPPATH ..."
	rm -rf "$TMPPATH"
fi

# Clean build of Corona Simulator.app
logfileprefix=$(date +"%Y-%m-%d-%H%M")
logfilepath=$(pwd)/$TOOLSPATH/logs/$logfileprefix
mkdir -pv "$logfilepath"

(
cd "$TOOLSPATH"/../../platform/mac
./build.sh  "$CUSTOM_ID" "$S3_BUCKET" | tee  "$logfilepath"/simulator.txt

if [ "${PIPESTATUS[0]}" -ne 0 ] #exit if the build script failed. $? gives us the output from tee not build.sh
then
	echo "BUILD ERROR: Could not build 'Corona Simulator.app'"
	exit -1
fi
)

mkdir "$TMPPATH"

#codesign Simulator
codesign --deep -f -s "Developer ID Application: Corona Labs Inc" "$TOOLSPATH/Corona Simulator.app" --timestamp=none

#codesign Debugger and Terminal
codesign --deep -f -s "Developer ID Application: Corona Labs Inc" "$TOOLSPATH/debugger" --timestamp=none
codesign --deep -f -s "Developer ID Application: Corona Labs Inc" "$TOOLSPATH/Corona Terminal" --timestamp=none

mkdir "$TMPPATH/${PRODUCT_DIR}"
ditto "$TOOLSPATH/../../platform/resources/icons/CoronaSDK-DMG-DS_Store" "$TMPPATH"/.DS_Store
bin/mac/seticon "$TMPPATH/${PRODUCT_DIR}" "$TOOLSPATH/../../platform/resources/icons/CoronaIcon-Folder.png"

ditto -v -X "$TOOLSPATH/Corona Simulator.app" "$TMPPATH/${PRODUCT_DIR}/Corona Simulator.app"
mkdir "${TMPPATH}/${PRODUCT_DIR}/${RESOURCE_DIR}"
cp -v "$TOOLSPATH"/{debugger,"Corona Terminal"} "${TMPPATH}/${PRODUCT_DIR}/${RESOURCE_DIR}"
cp -v "$TOOLSPATH"/../../platform/resources/icons/Documentation.html "$TMPPATH"/${PRODUCT_DIR}/
ditto -v -X "$2"/${SAMPLECODE_DIR}/ "$TMPPATH/${PRODUCT_DIR}/${SAMPLECODE_DIR}/"

if [ -d "$TMPPATH/${PRODUCT_DIR}/${SAMPLECODE_DIR}/.hg" ] # helpful for test builds
then
	rm -r "$TMPPATH/${PRODUCT_DIR}/${SAMPLECODE_DIR}/.hg"
fi

ditto -v -X "$2/${RESOURCE_DIR}" "$TMPPATH/${PRODUCT_DIR}/${RESOURCE_DIR}"

if [ "$ENTERPRISE" != "" ]
then
	(tar -C "$TMPPATH/${PRODUCT_DIR}/" -xf "$ENTERPRISE" && mv "$TMPPATH/${PRODUCT_DIR}/CoronaEnterprise" "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}" && ls "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}") || (echo "ERROR: failed to extract Enterprise" && exit 1)
	ditto -v -X "$TOOLSPATH/../../sdk/dmg/Setup Corona Native.app" "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}/Setup Corona Native.app"
	ditto -v -X "$TOOLSPATH/../../sdk/dmg/Setup Corona Enterprise.app" "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}/Setup Corona Enterprise.app"
	cp -v "$TOOLSPATH/../../sdk/dmg/Setup Corona.icns" "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}/Setup Corona Native.app/Contents/Resources/applet.icns"
	cp -v "$TOOLSPATH/../../sdk/dmg/Setup Corona.icns" "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}/Setup Corona Enterprise.app/Contents/Resources/applet.icns"
	xattr -cr "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}/Setup Corona Native.app"
	codesign -s "Developer ID Application: Corona Labs Inc" --timestamp=none "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}/Setup Corona Native.app"
	xattr -cr "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}/Setup Corona Enterprise.app"
	codesign -s "Developer ID Application: Corona Labs Inc" --timestamp=none "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}/Setup Corona Enterprise.app"
fi

# unfortunately, since macOS 10.12 resource forks can not be signed, so removing some icons
#bin/mac/seticon "$TMPPATH/${PRODUCT_DIR}/${RESOURCE_DIR}/debugger" "$TOOLSPATH/../../platform/resources/icons/CoronaIcon-Debugger.png"
# bin/mac/seticon "$TMPPATH/${PRODUCT_DIR}/${SAMPLECODE_DIR}" "$TOOLSPATH/../../platform/resources/icons/CoronaIcon-Folder.png"
# bin/mac/seticon "$TMPPATH/${PRODUCT_DIR}/${RESOURCE_DIR}" "$TOOLSPATH/../../platform/resources/icons/CoronaIcon-Folder.png"
bin/mac/seticon "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}" "$TOOLSPATH/../../platform/resources/icons/CoronaIcon-Folder.png"
bin/mac/seticon "$TMPPATH/${PRODUCT_DIR}/Documentation.html" "$TOOLSPATH/../../platform/resources/icons/CoronaIcon-Docs.png"
#bin/mac/seticon "$TMPPATH/${PRODUCT_DIR}/${RESOURCE_DIR}/Corona Terminal" "$TOOLSPATH/../../platform/resources/icons/CoronaIcon-Terminal.png"
xcrun SetFile -a E "$TMPPATH/${PRODUCT_DIR}/Documentation.html" # hide extension

# this causes hdiutil create to generate corrupt filesystems for some reason
# ditto "$TOOLSPATH/../../platform/resources/icons/Applications-CoronaSDK-DS_Store" "$TMPPATH"/${PRODUCT_DIR}/.DS_Store

if [[ -d "$DOCSRC/Tools" ]]
then
    ditto -v -X "$DOCSRC"/Tools "$TMPPATH"/${PRODUCT_DIR}/Tools
fi

if [ "${BUILD_NUM}" != "" ] && [ "$DAILY_BUILD" == true ]
then
	mv "$TMPPATH"/${PRODUCT_DIR} "$TMPPATH/${PRODUCT_DIR}${BUILD_NUM}"
	VOLUME_NAME=${PRODUCT_DIR}${BUILD_NUM}
	ICON_NAME=${PRODUCT_DIR}${BUILD_NUM}
	DMG_FILE=${PRODUCT_DIR}-${FULL_BUILD_NUM}.dmg
else
	VOLUME_NAME=${PRODUCT_DIR}
	ICON_NAME=${PRODUCT_DIR}
	DMG_FILE=${PRODUCT_DIR}-${FULL_BUILD_NUM}-release.dmg
fi

BACKGROUND_PATH=$TOOLSPATH/../../sdk/dmg/CoronaBackground.png
WINDOW_WIDTH=620
WINDOW_HEIGHT=420
ICON_SIZE=128
APP_X=475
APP_Y=125
ICON_X=155
ICON_Y=$APP_Y

rm -f "$DMG_FILE"

# If we have a build number and ImageMagick's convert command is available, brand the DMG background
if [ "$FULL_BUILD_NUM" != "" ] && [ -x "$(which convert)" ]
then
	TMPBACKGROUND=/tmp/CoronaBackground$$.png
	convert sdk/dmg/CoronaBackground.png -pointsize 13 -stroke DarkGrey -fill DarkGrey -draw "text 39,387 '$FULL_BUILD_NUM'" "$TMPBACKGROUND"
	BACKGROUND_PATH="$TMPBACKGROUND"
fi

"$TOOLSPATH"/create-dmg/create-dmg --volname "$VOLUME_NAME" --background "$BACKGROUND_PATH" --window-size $WINDOW_WIDTH $WINDOW_HEIGHT --app-drop-link $APP_X $APP_Y --icon "$ICON_NAME" $ICON_X $ICON_Y --icon-size $ICON_SIZE "$DMG_FILE" "$TMPPATH"

mv -f "$DMG_FILE" "$DSTBASE"/"$DMG_FILE"
rm -f "$TMPBACKGROUND"

#if on the build server generate a manifest and save it to the workspace dir
if [ -n "$HUDSON_URL" ];
then
  find "$TMPPATH"  -type fl -exec "${WORKSPACE}/bin/mac/manifest-builder.sh" {} \; >  "${WORKSPACE}/manifest.txt"
#  if [ ${PIPESTATUS[0]} -ne 0 ] #exit if the build script failed. $? gives us the output from tee not build.sh
#  then
#    echo "BUILD ERROR: Could not generate manifest."
#    exit -1
#  fi
fi

rm -rf "$TMPPATH"		# Remove tmp folder

# Convert tmp dmg into read-only dmg and add license
hdiutil unflatten "$DSTBASE/$DMG_FILE"
"$TOOLSPATH/AddLicense" "$DSTBASE/$DMG_FILE" English "$TOOLSPATH/../../sdk/dmg/Corona_License.rtf"
hdiutil flatten "$DSTBASE/$DMG_FILE"

codesign -s "Developer ID Application: Corona Labs Inc" "$DSTBASE/$DMG_FILE" --timestamp=none
