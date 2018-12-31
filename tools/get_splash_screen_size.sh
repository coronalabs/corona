#!/bin/bash

# This script gets the current size of the default splash screen making sure that the file
# it's using has been crunched for the appropriate platform
#
# Usage: get_splash_screen_size.sh {ios|android} {repo_dir}


PLATFORM="$1"
REPO_DIR="$2"

if [ "$PLATFORM" == "" ] || [ ! -d "$REPO_DIR" ]
then
	echo "ERROR: Usage: $(basename "$0") {ios|android} {repo_dir}"
	exit 1
fi

set -x

if [ ! -d "$ANDROID_SDK" ]
then
	ANDROID_SDK="${HOME}/Library/Android/sdk"
fi

PNG_FILE_ANDROID="$REPO_DIR/platform/android/sdk/res/drawable/_corona_splash_screen.png"
PNG_FILE_IOS="$REPO_DIR/platform/iphone/_CoronaSplashScreen.png"
TMP_PNG_FILE_IOS="/tmp/_CoronaSplashScreen$$.png"
TMP_PNG_FILE_ANDROID="/tmp/_corona_splash_screen$$.png"

if [ "$PLATFORM" == "android" ]
then

	if [ ! -r "$PNG_FILE_ANDROID" ]
	then
		echo "ERROR: cannot find splash png file '$PNG_FILE_ANDROID'"
		exit 1
	fi

	# Get the most recent version of the build tools
	AAPT="$ANDROID_SDK/build-tools/$(ls -t "$ANDROID_SDK"/build-tools | head -1)/aapt"
	if [ -x "$AAPT" ]
	then
		# building compresses the PNG file so we need to use the processed size
		"$AAPT" singleCrunch -i "$PNG_FILE_ANDROID" -o "$TMP_PNG_FILE_ANDROID" >/dev/null
		CRUNCHED_SPLASH_IMAGE_FILE_SIZE="$(stat -f "%z" "$TMP_PNG_FILE_ANDROID")"
		CURRENT_SPLASH_IMAGE_FILE_SIZE="$(stat -f "%z" "$PNG_FILE_ANDROID")"
	else
		echo "ERROR: cannot find a usable version of 'aapt' in '$AAPT' (ANDROID_SDK: $ANDROID_SDK)"
		exit 1
	fi

elif [ "$PLATFORM" == "ios" ]
then

	if [ ! -r "$PNG_FILE_IOS" ]
	then
		echo "ERROR: cannot find splash png file '$PNG_FILE_IOS'"
		exit 1
	fi

	COPYPNG=$(xcrun -f copypng)
	if [ -x "$COPYPNG" ]
	then
		"$COPYPNG" -compress -strip-PNG-text "$PNG_FILE_IOS" "$TMP_PNG_FILE_IOS"
		CRUNCHED_SPLASH_IMAGE_FILE_SIZE=$(stat -f "%z" "$TMP_PNG_FILE_IOS")
		CURRENT_SPLASH_IMAGE_FILE_SIZE=$(stat -f "%z" "$PNG_FILE_IOS")
	else
		echo "ERROR: cannot find a usable version of 'copypng' in '$COPYPNG'"
		exit 1
	fi

else

	echo "ERROR: unsupported platform '$PLATFORM'"
	exit 1

fi

if [ "$CRUNCHED_SPLASH_IMAGE_FILE_SIZE" != "$CURRENT_SPLASH_IMAGE_FILE_SIZE" ]
then
	echo "ERROR: $PLATFORM splash image does not appear to be crunched (CRUNCHED_FILE_SIZE $CRUNCHED_SPLASH_IMAGE_FILE_SIZE != CURRENT_FILE_SIZE $CURRENT_SPLASH_IMAGE_FILE_SIZE)"
	exit 1
else
	echo "$CURRENT_SPLASH_IMAGE_FILE_SIZE"
fi
