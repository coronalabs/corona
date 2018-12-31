#!/bin/bash
#
# android_sendapp.sh
#
# Send an APK to an Android device connected to the computer and optionally launch it
#
# Copyright (c) 2015 Corona Labs Inc. All rights reserved.
#

DEBUG_BUILD_PROCESS=$(defaults read com.coronalabs.Corona_Simulator debugBuildProcess 2>/dev/null)

exec 2>&1

if [ "${DEBUG_BUILD_PROCESS:=0}" -gt 1 ]
then
	set -x
fi

CORONA_ACTIVITY_NAME="com.ansca.corona.CoronaActivity"
PROGRAM_NAME="ERROR" # $(basename "$0")
SEND_TO_ALL=$(defaults read com.coronalabs.Corona_Simulator sendToAllDevices 2>/dev/null)
CORONA_RES_DIR=$(dirname "$0")
ADB="${CORONA_RES_DIR}/device-support/adb"
AAPT="${CORONA_RES_DIR}/device-support/aapt"

if [ ! -x "${ADB}" ] || [ ! -x "${AAPT}" ]
then
	echo "${PROGRAM_NAME}: cannot run ${ADB} or ${AAPT}" >&2
	exit 1
fi

RUNAPP=
while getopts ":r" opt
do
	case $opt in
		r)
			RUNAPP=YES
			;;
		\?)
			echo "$PROGRAM_NAME: invalid option: -$OPTARG" >&2
			;;
	esac
done
shift $((OPTIND-1))

if [ "$RUNAPP" == "YES" ] && [ ! -x "$AAPT" ]
then
	# Not in the user's PATH, look in ANDROID_SDK
	echo "WARNING: cannot locate 'aapt' utility.  App will have to be launched manually"
fi

APK="$1"
if [ ! -r "$APK" ]
then
	echo "$PROGRAM_NAME: can't read APK in '$APK'" >&2
	exit 1
fi

# If we're sending to all devices get the full list otherwise find the last
# device listed by adb which is either the only one or usually the last one
# connected
if [ "$SEND_TO_ALL" == "YES" ] || [ "$SEND_TO_ALL" == "1" ]
then
	NUM_DEVICES=1000 # all of them
else
	NUM_DEVICES=1
fi

# Get the ids the connected device(s)
DEVICE_IDS=($("$ADB" devices | sed -n -e '/device$/ s/	device//p' |tail -${NUM_DEVICES}))
API_LEVEL=
if [ "${#DEVICE_IDS[@]}" == 0 ]
then
	echo "$PROGRAM_NAME: no device detected (if one is attached, try reconnecting it)" >&2
	exit 1
elif [ "${#DEVICE_IDS[@]}" == 1 ]
then
	# Verify the device is new enough to run Corona.
	API_LEVEL="$("$ADB" -s "${DEVICE_IDS[0]}" shell getprop ro.build.version.sdk | tr -d '\r')"
	if [ "$API_LEVEL" -lt 15 ]
	then
		ANDROID_OS_VERSION="$("$ADB" -s "${DEVICE_IDS[0]}" shell getprop ro.build.version.release | tr -d '\r')"
		echo "$PROGRAM_NAME: Currently connected device is running an unsupported Android OS version. To launch app on an Android $ANDROID_OS_VERSION device, please use a daily build which supports Android $ANDROID_OS_VERSION:" >&2
		echo "" >&2
		echo "* Android 2.3.3 - 4.0.2; use 2016.3007 or older" >&2
		echo "* Android 2.2 - 2.3.2; use 2014.2263 or older" >&2
		exit 1
	fi
fi

if [ -x "$AAPT" ]
then
	APPLICATION_NAME=$("$AAPT" dump badging "$APK" | sed -n -e "/^application-label:/s/.*label:'\([^']*\).*/\1/p")
else
	APPLICATION_NAME="the APK"
fi

for DEVICE_ID in "${DEVICE_IDS[@]}"
do
	# Verify this device is new enough to run Corona apps, skipping it if it isn't
	API_LEVEL="$("$ADB" -s "$DEVICE_ID" shell getprop ro.build.version.sdk | tr -d '\r')"
	if [ "$API_LEVEL" -lt 15 ]
	then
		echo "Skipping '$DEVICE_ID' because it runs an unsupported Android OS version."
		continue
	fi

	echo "Installing ${RUNAPP:+and running }${APPLICATION_NAME} on $("$ADB" -s "$DEVICE_ID" shell getprop ro.product.model | tr -d '\r')"

	# Turn the devices's screen on if it's off
	# Search terms can be added to the egrep; the intention is positive confirmation that the screen is off otherwise we
	# end up turning it off when it's on which is annoying; the search terms are very dependent on the version of Android
	# on the device but have been tested with 2.3.x, 4.x, 5.x and 6.x)
	# The screen is turned on by sending the device the keycode for the "Power" button (which is a toggle)
	"$ADB" -s "$DEVICE_ID" shell dumpsys power | grep -E -qs "mScreenOn=false|mLastScreenOnTime=0|Display Power: state=OFF" && "$ADB" -s "$DEVICE_ID" shell input keyevent 26

	# Install the APK
	# First copy it to a "safe" filename because adb is fussy about things like UTF-8 in names
	TMPNAME="/tmp/CoronaSDK$(echo "$APK" | md5).apk"
	/bin/cp "$APK" "$TMPNAME"
	# "$ADB" -s "$DEVICE_ID" install -r "$TMPNAME"
	ADB_INSTALL_OUTPUT="$("$ADB" -s "$DEVICE_ID" install -r "$TMPNAME")"
	
	# If the installation failed for some reason, report it back nicely
	# TODO: Implement this if later versions of adb will provide exit codes: https://code.google.com/p/android/issues/detail?id=63210
	# If installation returnes failure 
	if [[ "${ADB_INSTALL_OUTPUT}" == *Success* ]]; then
		echo "Installing "${RUNAPP:+and running }${APPLICATION_NAME}" to $("$ADB" -s "$DEVICE_ID" shell getprop ro.product.model | tr -d '\r') successfull!"
	else
		echo "$PROGRAM_NAME: "${ADB_INSTALL_OUTPUT}" Error found during installation!"
		exit 1
	fi

	"$ADB" -s "$DEVICE_ID" shell rm "/data/local/tmp/$(basename "$TMPNAME")" >/dev/null # remove tmp dir copy of apk in case Android version leaves it behind
	/bin/rm -f "$TMPNAME"

	# If requested (and we have the necessary tool), launch the app
	if [ "$RUNAPP" == "YES" ] && [ -x "$AAPT" ]
	then
		# Extract the package name from the apk
		PACKAGE_NAME=$("$AAPT" dump badging "$APK" | sed -n -e "/^package:/s/.*name='\([^']*\).*/\1/p")

		# Clear the device's syslog (makes android_syslog.sh usable)
		"$ADB" -s "$DEVICE_ID" logcat -c

		# Launch the package
		"$ADB" -s "$DEVICE_ID" shell am start -n "$PACKAGE_NAME"/"$CORONA_ACTIVITY_NAME"
	fi
done
