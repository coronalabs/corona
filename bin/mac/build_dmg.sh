#!/bin/bash -ex
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
SRCROOT="$TOOLSPATH/../.."
PLATFORM_MAC="$SRCROOT/platform/mac"
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
cd "$PLATFORM_MAC"
./build.sh  "$CUSTOM_ID" "$S3_BUCKET" | tee  "$logfilepath"/simulator.txt

if [ "${PIPESTATUS[0]}" -ne 0 ] #exit if the build script failed. $? gives us the output from tee not build.sh
then
	echo "BUILD ERROR: Could not build 'Corona Simulator.app'"
	exit 22
fi
)

mkdir "$TMPPATH"

mkdir "$TMPPATH/${PRODUCT_DIR}"
ditto "$SRCROOT/platform/resources/icons/CoronaSDK-DMG-DS_Store" "$TMPPATH"/.DS_Store
bin/mac/seticon "$TMPPATH/${PRODUCT_DIR}" "$SRCROOT/platform/resources/icons/CoronaIcon-Folder.png"

ditto -v -X "$TOOLSPATH/Corona Simulator.app" "$TMPPATH/${PRODUCT_DIR}/Corona Simulator.app"
mkdir "${TMPPATH}/${PRODUCT_DIR}/${RESOURCE_DIR}"
cp -v "$TOOLSPATH"/{debugger,"Corona Terminal"} "${TMPPATH}/${PRODUCT_DIR}/${RESOURCE_DIR}"
cp -v "$SRCROOT"/platform/resources/icons/Documentation.html "$TMPPATH"/${PRODUCT_DIR}/
ditto -v -X "$2"/${SAMPLECODE_DIR}/ "$TMPPATH/${PRODUCT_DIR}/${SAMPLECODE_DIR}/"

if [ -d "$TMPPATH/${PRODUCT_DIR}/${SAMPLECODE_DIR}/.hg" ] # helpful for test builds
then
	rm -r "$TMPPATH/${PRODUCT_DIR}/${SAMPLECODE_DIR}/.hg"
fi

if [ -d "$2/${RESOURCE_DIR}" ]
then
	ditto -v -X "$2/${RESOURCE_DIR}" "$TMPPATH/${PRODUCT_DIR}/${RESOURCE_DIR}"
fi

mkdir -p "$TMPPATH/${PRODUCT_DIR}/${RESOURCE_DIR}/Android"
cp -v -X "$SRCROOT"/platform/android/resources/debug.keystore "$TMPPATH/${PRODUCT_DIR}/${RESOURCE_DIR}/Android/"

mkdir -p "$TMPPATH/${PRODUCT_DIR}/${RESOURCE_DIR}/iOS"
ditto -v -X "$SRCROOT"/platform/iphone/Images.xcassets "$TMPPATH/${PRODUCT_DIR}/${RESOURCE_DIR}/iOS/Images.xcassets"

if [ "$ENTERPRISE" != "" ]
then
	(tar -C "$TMPPATH/${PRODUCT_DIR}/" -xf "$ENTERPRISE" && mv "$TMPPATH/${PRODUCT_DIR}/CoronaEnterprise" "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}" && ls "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}") || (echo "ERROR: failed to extract Enterprise" && exit 1)
	ditto -v -X "$SRCROOT/sdk/dmg/Setup Corona Native.app" "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}/Setup Corona Native.app"
	ditto -v -X "$SRCROOT/sdk/dmg/Setup Corona Enterprise.app" "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}/Setup Corona Enterprise.app"
	cp -v "$SRCROOT/sdk/dmg/Setup Corona.icns" "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}/Setup Corona Native.app/Contents/Resources/applet.icns"
	cp -v "$SRCROOT/sdk/dmg/Setup Corona.icns" "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}/Setup Corona Enterprise.app/Contents/Resources/applet.icns"
	xattr -cr "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}/Setup Corona Native.app"
	xattr -cr "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}/Setup Corona Enterprise.app"
fi

# unfortunately, since macOS 10.12 resource forks can not be signed, so removing some icons
# bin/mac/seticon "$TMPPATH/${PRODUCT_DIR}/${RESOURCE_DIR}/debugger" "$SRCROOT/platform/resources/icons/CoronaIcon-Debugger.png"
# bin/mac/seticon "$TMPPATH/${PRODUCT_DIR}/${SAMPLECODE_DIR}" "$SRCROOT/platform/resources/icons/CoronaIcon-Folder.png"
# bin/mac/seticon "$TMPPATH/${PRODUCT_DIR}/${RESOURCE_DIR}" "$SRCROOT/platform/resources/icons/CoronaIcon-Folder.png"
bin/mac/seticon "$TMPPATH/${PRODUCT_DIR}/${NATIVE_DIR}" "$SRCROOT/platform/resources/icons/CoronaIcon-Folder.png"
bin/mac/seticon "$TMPPATH/${PRODUCT_DIR}/Documentation.html" "$SRCROOT/platform/resources/icons/CoronaIcon-Docs.png"
#bin/mac/seticon "$TMPPATH/${PRODUCT_DIR}/${RESOURCE_DIR}/Corona Terminal" "$SRCROOT/platform/resources/icons/CoronaIcon-Terminal.png"
xcrun SetFile -a E "$TMPPATH/${PRODUCT_DIR}/Documentation.html" # hide extension


## Codesign everything
(
	ENTITLEMENTS="$(cd "$PLATFORM_MAC" ; pwd)/CoronaSimulator.entitlements"
	TMPPATH="$(cd "$TMPPATH" ; pwd)"

	function CodesignLocation {
	(
		cd "$1"

		local EXECUTABLES=()
		local LIBRARIES=()
		local ZIPS=()

		save_x=$-
		set +x

		while read f
		do
			file_result=$(file "$f" -b -h)

			if [[ $file_result == *"Mach-O"*"executable"* ]]
			then
				EXECUTABLES+=("$f")
			elif [[ $file_result == *"Zip archive data"* ]] && [[ $f == *OSXAppTemplate* ]]
			then
				ZIPS+=("$f")
			elif [[ $file_result == *"Mach-O"*"library"* ]]
			then
				LIBRARIES+=("$f")
			elif [[ $file_result == *"Mach-O"*"ar archive"* ]]
			then
				LIBRARIES+=("$f")
			elif [[ $file_result == *"Mach-O"*"bundle"* ]]
			then
				LIBRARIES+=("$f")
			fi
		done < <(find . -type f)

		while read f
		do
			EXECUTABLES+=("$f")
		done < <(find . -d -name '*.app')

		[[ $save_x =~ x ]] && set -x

		for z in "${ZIPS[@]}"
		do
			ZD="$(mktemp -d "${TMPPATH}/ziptmp-XXXXX")"
			ditto -x -k "$z" "$ZD"
			CodesignLocation "$ZD"
			ditto -c -k "$ZD" "$ZD.zip"
			mv "$ZD.zip" "$z"
			rm -r "$ZD"
		done

		
		codesign --timestamp --deep --force --options runtime --strict --sign 'Developer ID Application: Corona Labs Inc' "${LIBRARIES[@]}"
		codesign --timestamp --deep --force --options runtime --strict --sign 'Developer ID Application: Corona Labs Inc' --entitlements "$ENTITLEMENTS"  "${EXECUTABLES[@]}"
	)
	}

	CodesignLocation "$TMPPATH"

)


# this causes hdiutil create to generate corrupt filesystems for some reason
# ditto "$SRCROOT/platform/resources/icons/Applications-CoronaSDK-DS_Store" "$TMPPATH"/${PRODUCT_DIR}/.DS_Store

if [[ -d "$DOCSRC/Tools" ]]
then
    ditto -v -X "$DOCSRC"/Tools "$TMPPATH"/${PRODUCT_DIR}/Tools
fi

RESULT_DIR="$DSTDIR/${PRODUCT_DIR}"
if [ "${BUILD_NUM}" != "" ] && [ "$DAILY_BUILD" == true ]
then
	RESULT_DIR="$DSTDIR/${PRODUCT_DIR}${BUILD_NUM}"
	mv "$TMPPATH"/${PRODUCT_DIR} "$TMPPATH/${PRODUCT_DIR}${BUILD_NUM}"
	VOLUME_NAME=${PRODUCT_DIR}${BUILD_NUM}
	ICON_NAME=${PRODUCT_DIR}${BUILD_NUM}
	DMG_FILE=${PRODUCT_DIR}-${FULL_BUILD_NUM}.dmg
else
	VOLUME_NAME=${PRODUCT_DIR}
	ICON_NAME=${PRODUCT_DIR}
	DMG_FILE=${PRODUCT_DIR}-${FULL_BUILD_NUM}-release.dmg
fi

BACKGROUND_PATH=$SRCROOT/sdk/dmg/CoronaBackground.png
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

	convert sdk/dmg/BG.png    -pointsize 13 -stroke DarkGrey -fill DarkGrey -draw "text 39,387 '$FULL_BUILD_NUM'" sdk/dmg/bgp.png
	convert sdk/dmg/BG@2x.png -pointsize 26 -stroke DarkGrey -fill DarkGrey -draw "text 78,774 '$FULL_BUILD_NUM'" sdk/dmg/bgp@2x.png
else
	cp sdk/dmg/BG.png    sdk/dmg/bgp.png
	cp sdk/dmg/BG@2x.png sdk/dmg/bgp@2x.png
fi

if [ -x "$(command -v appdmg)" ]
then
	sed "s#XXXXXXXX#../../$RESULT_DIR#g ; s#YYYY#$FULL_BUILD_NUM#g" sdk/dmg/appdmg.json > sdk/dmg/processed_appdmg.json
	appdmg sdk/dmg/processed_appdmg.json "$DMG_FILE"
else
	"$TOOLSPATH"/create-dmg/create-dmg $JENKINS --volname "$VOLUME_NAME" --background "$BACKGROUND_PATH" --window-size $WINDOW_WIDTH $WINDOW_HEIGHT --app-drop-link $APP_X $APP_Y --icon "$ICON_NAME" $ICON_X $ICON_Y --icon-size $ICON_SIZE "$DMG_FILE" "$TMPPATH"
fi

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
# hdiutil unflatten "$DSTBASE/$DMG_FILE"
# "$TOOLSPATH/AddLicense" "$DSTBASE/$DMG_FILE" English "$SRCROOT/sdk/dmg/Corona_License.rtf"
# hdiutil flatten "$DSTBASE/$DMG_FILE"

codesign --timestamp --deep --force --options runtime --strict --sign "Developer ID Application: Corona Labs Inc" "$DSTBASE/$DMG_FILE"
mkdir -p "$SRCROOT/output"
cp "$DSTBASE/$DMG_FILE" "$SRCROOT/output"
