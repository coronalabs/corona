#!/bin/bash
#----------------------------------------------------------------------------
#
# This file is part of the Corona game engine.
# For overview and more information on licensing please refer to README.md 
# Home page: https://github.com/coronalabs/corona
# Contact: support@coronalabs.com
#
#----------------------------------------------------------------------------

# Send an APP to an iOS device connected to the computer

DEBUG_BUILD_PROCESS=$(defaults read com.coronalabs.Corona_Simulator debugBuildProcess 2>/dev/null)

exec 2>&1

if [ "${DEBUG_BUILD_PROCESS:=0}" -gt 1 ]
then
	set -x
fi

PROGRAM_NAME="ERROR" # "$(basename "$0"): "
SEND_TO_ALL=$(defaults read com.coronalabs.Corona_Simulator sendToAllDevices 2>/dev/null)
CORONA_RES_DIR=$(dirname "$0")
RUN_UTIL_PATH="${CORONA_RES_DIR}/device-support/run-idevice-util"
INSTALLATION_TIMEOUT=30
APP="$1"
APPNAME=$(basename "$1")
TARGETOS="$2"

if [ ! -x "${RUN_UTIL_PATH}" ]
then
	echo "${PROGRAM_NAME}: cannot run ${RUN_UTIL_PATH}" >&2
	exit 1
fi

if [ ! -d "$APP" ]
then
	echo "${PROGRAM_NAME}: cannot open '$APP'" >&2
	exit 1
fi

# If a target OS is not provided, assume we're trying to target iOS.
if [ -z "$TARGETOS" ]
then
    TARGETOS="iPhone OS"
fi

if [ ! -x "$RUN_UTIL_PATH" ]
then
	# NOTE: the Simulator checks for this error so change it in both places
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
    echo "$PROGRAM_NAME: no devices detected (if one is attached, try reconnecting it)" >&2
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

	# echo "DEBUG: $DEVICE: ATTACHED: $ATTACHED"
	# DEBUG: "$RUN_UTIL_PATH" ideviceinfo --udid "$DEVICE" | sed -e "s/^/$DEVICE: /"

	if [ "$ATTACHED" -gt 0 ]
	then
		IOS_DEVICES[$COUNT]="$DEVICE"

		COUNT=$((COUNT + 1))
	fi
done

if [ "$DEBUG_BUILD_PROCESS" -gt 1 ]
then
	echo "DEBUG: IOS_DEVICES: ${IOS_DEVICES[*]}"
fi

if [ "${#IOS_DEVICES[@]}" == 0 ]
then
    echo "$PROGRAM_NAME: no USB connected devices detected (if one is attached, try reconnecting it)" >&2
	exit 1
fi


if [ "$SEND_TO_ALL" == 1 ] || [ "$SEND_TO_ALL" == "YES" ]
then
	# All found devices
	IOS_DEVICES=(${IOS_DEVICES[@]})
else
	# Pick the first as that's the most recently connected
	IOS_DEVICES=(${IOS_DEVICES[0]})
fi

trap 'kill -TERM 0' EXIT

PIDS=()
for DEVICE in "${IOS_DEVICES[@]}"
do
		DEVICENAME=$("$RUN_UTIL_PATH" ideviceinfo --udid "$DEVICE" | sed -ne '/^DeviceName:/s/.*: \(.*\)/\1/p' | tr ' .!@#$%^&*()[]{}' '_')
		echo "Installing '$APPNAME' on $DEVICENAME ..."

		("$RUN_UTIL_PATH" ideviceinstaller --udid "$DEVICE" --install "$APP" | tr -d '\r') &

		PIDS=(${PIDS[@]} ${!})
		DEVICES[${!}]="$DEVICENAME ($DEVICE)"
done

# Implement a simple timeout (sometimes 'ideviceinstaller' hangs for unknown reasons)
TIMEOUT_COUNT=0
while [ "${#PIDS[@]}" -gt 0 ] && [ "$TIMEOUT_COUNT" -lt "$INSTALLATION_TIMEOUT" ]
do
	sleep 1

	NEWPIDS=()
	for PID in "${PIDS[@]}"
	do
		# Test whether the background process still exists
		kill -0 "$PID" 2>/dev/null

		if [ $? == 0 ]
		then
			NEWPIDS=( "${NEWPIDS[@]}" $PID )
		fi
	done

	PIDS=( "${NEWPIDS[@]}" )
	TIMEOUT_COUNT=$((TIMEOUT_COUNT + 1))
done

ERROR_CODE=0

# Kill off any stragglers
for PID in "${PIDS[@]}"
do
	kill -TERM "$PID" 2>/dev/null

	if [ "$?" == 0 ]
	then
        echo "$PROGRAM_NAME: timed out copying to device: ${DEVICES[$PID]}.  Try reconnecting it." >&2
		ERROR_CODE=1
	fi
done

exit $ERROR_CODE
