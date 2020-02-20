#!/bin/bash
#----------------------------------------------------------------------------
#
# This file is part of the Corona game engine.
# For overview and more information on licensing please refer to README.md 
# Home page: https://github.com/coronalabs/corona
# Contact: support@coronalabs.com
#
#----------------------------------------------------------------------------

# Tail the syslog of a connected iOS device filtering for Corona

DEBUG_BUILD_PROCESS=$(defaults read com.coronalabs.Corona_Simulator debugBuildProcess 2>/dev/null)

# Add "noise" logs to exclude here (egrep pattern)
EXCLUDE_PATTERNS='nw_connection_|nw_endpoint_|__nw_socket_service_|subsystem: com\.apple\.|\[Device\] TIC |HTTPCookieStorage singleton|Creating default cookie storage'

if [ "${DEBUG_BUILD_PROCESS:=0}" -gt 2 ]
then
	set -x
	exec 2>&1
fi

PROGRAM_NAME="ERROR" # $(basename "$0")
SEND_TO_ALL=$(defaults read com.coronalabs.Corona_Simulator sendToAllDevices 2>/dev/null)
CORONA_RES_DIR=$(dirname "$0")
RUN_UTIL_PATH="${CORONA_RES_DIR}/device-support/run-idevice-util"
APP="$1"
TARGETOS="$2"

if [ ! -x "${RUN_UTIL_PATH}" ]
then
	echo "${PROGRAM_NAME}: cannot run ${RUN_UTIL_PATH}" >&2
	exit 1
fi

# If a target OS is not provided, assume we're trying to target iOS.
if [ -z "$TARGETOS" ]
then
    TARGETOS="iPhone OS"
fi

BUNDLE_NAME=$(plutil -convert json "$APP"/Info.plist -o - -r |grep CFBundleName | sed -e 's/.* : "\([^"]*\)",/\1/')

# echo "DEBUG: $0 $@" >&2

# exec > >(tee /dev/stderr | /usr/bin/logger -t "Corona Simulator($PPID)") # duplicate output to syslog
# exec 2>&1  # copy stderr to stdout

IOS_SYSLOG_HELPER="${CORONA_RES_DIR}/ios-syslog-helper"

if [ ! -x "$RUN_UTIL_PATH" ]
then
	echo "$PROGRAM_NAME: can't find '$(basename "${RUN_UTIL_PATH}")'" >&2
	exit 1
fi

# Enumerate iOS device connections.  Some of these will be USB which we can use and some
# will be WiFi which we can't (generally there's one of each for each device though this
# isn't always the case)

# Get currently connected iOS device UDIDs and dedupe (USB and WiFi show up as dupes)
ENUM_IOS_DEVICES=( $("$RUN_UTIL_PATH" idevice_id --list | sort -u) )

if [ "${#ENUM_IOS_DEVICES[@]}" == 0 ]
then
	echo "$PROGRAM_NAME: no iOS devices detected (if one is attached, try reconnecting it)"
	exit 1
fi

IOS_DEVICES=()
COUNT=0
for DEVICE in "${ENUM_IOS_DEVICES[@]}"
do
    # Filter out devices that do not match our target (iPhone OS or Apple TVOS)
    DEVICEOS=$("$RUN_UTIL_PATH" ideviceinfo --udid "$DEVICE" | grep -i 'ProductName:' | cut -d ' ' -f 2-)
	if [ "$DEVICEOS" == "" ] && [ "$TARGETOS" == "Apple TVOS" ]
    then
		# older devices don't report "ProductName" but they also can't be tvOS
        continue
    elif [ "$DEVICEOS" != "" ] &&[ "$DEVICEOS" != "$TARGETOS" ]
	then
		continue
    fi

	# Devices may appear in the list because they are connected via WiFi but installing apps wont work
	# so make sure that they are really attached
	ATTACHED=$("$RUN_UTIL_PATH" ideviceinfo --udid "$DEVICE" | grep -ci 'HostAttached: true')

	# DEBUG: echo "$DEVICE: ATTACHED: $ATTACHED"
	# DEBUG: "$RUN_UTIL_PATH" ideviceinfo --udid "$DEVICE" | sed -e "s/^/$DEVICE: /"

	if [ "$ATTACHED" -gt 0 ]
	then
		IOS_DEVICES[$COUNT]="$DEVICE"

		COUNT=$((COUNT + 1))
	fi
done

# DEBUG: echo "IOS_DEVICES: echo ${IOS_DEVICES[*]}"

if [ "${#IOS_DEVICES[@]}" == 0 ]
then
	echo "$PROGRAM_NAME: no USB connected iOS devices detected (if one is attached, try reconnecting it)"
	exit 1
fi

if [ "$SEND_TO_ALL" == 1 ] || [ "$SEND_TO_ALL" == "YES" ]
then
	# Get currently connected iOS devices and dedupe (USB and WiFi show up as dupes)
	IOS_DEVICES=(${IOS_DEVICES[@]})
else
	# Pick the first as that's the most recently connected
	IOS_DEVICES=( ${IOS_DEVICES[0]} )
fi

trap 'kill -TERM 0' EXIT

for DEVICE_ID in "${IOS_DEVICES[@]}"
do
	{
		# Tail the device's syslog (looking just for the app in question)

		"$RUN_UTIL_PATH" idevicesyslog --udid "$DEVICE_ID" | "$IOS_SYSLOG_HELPER" now "$BUNDLE_NAME" '[Device] ' --level | egrep --line-buffer -v "$EXCLUDE_PATTERNS"
	} &
done

wait
