#!/bin/bash
#----------------------------------------------------------------------------
#
# This file is part of the Corona game engine.
# For overview and more information on licensing please refer to README.md 
# Home page: https://github.com/coronalabs/corona
# Contact: support@coronalabs.com
#
#----------------------------------------------------------------------------

# Tail the syslogs of all connected Android devices filtering for Corona

DEBUG_BUILD_PROCESS=$(defaults read com.coronalabs.Corona_Simulator debugBuildProcess 2>/dev/null)

if [ "${DEBUG_BUILD_PROCESS:=0}" -gt 2 ]
then
	set -x
	exec 2>&1
fi

PROGRAM_NAME="ERROR" # $(basename "$0")
CORONA_RES_DIR=$(dirname "$0")
ADB="${CORONA_RES_DIR}/device-support/adb"

if [ ! -x "${ADB}" ]
then
	echo "${PROGRAM_NAME}: cannot run ${ADB}" >&2
	exit 1
fi
# echo "DEBUG: $0 $@" >&2

# exec > >(tee /dev/stderr | /usr/bin/logger -t "Corona Simulator($PPID)") # duplicate output to syslog
# exec 2>&1  # copy stderr to stdout

# Get the ids the connected device(s)
DEVICE_IDS=($("$ADB" devices | sed -n -e '/device$/ s/	device//p'))

if [ "${#DEVICE_IDS[@]}" == 0 ]
then
	echo "$PROGRAM_NAME: no device detected (if one is attached, try reconnecting it)"
	exit 1
fi

trap 'kill -TERM 0' EXIT

for DEVICE_ID in "${DEVICE_IDS[@]}"
do
	DEVICE_NAME=$("$ADB" -s "$DEVICE_ID" shell getprop ro.product.model | tr -d '\r')

	# Tail the device's syslog
	"$ADB" -s "$DEVICE_ID" logcat -v raw Corona:D 'ActivityManager:W' 'AndroidRuntime:E' '*:S' | sed -l -e '/--------- beginning of /d' -e "s/^/$DEVICE_NAME: /" | tr -u -d '\r' &
done

wait
