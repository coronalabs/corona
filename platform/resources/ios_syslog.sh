#!/bin/bash
#----------------------------------------------------------------------------
#
# This file is part of the Corona game engine.
# For overview and more information on licensing please refer to README.md 
# Home page: https://github.com/coronalabs/corona
# Contact: support@coronalabs.com
#
#----------------------------------------------------------------------------

# Show the output of an app that's just been sent to an iOS or tvOS device (see ios_sendapp.sh)

DEBUG_BUILD_PROCESS=$(defaults read com.coronalabs.Corona_Simulator debugBuildProcess 2>/dev/null)

# Add "noise" logs to exclude here (egrep pattern)
EXCLUDE_PATTERNS='nw_connection_|nw_endpoint_|__nw_socket_service_|subsystem: com\.apple\.|\[Device\] TIC |HTTPCookieStorage singleton|Creating default cookie storage|policyStatus:|BKSHIDEventDeliveryPolicyObserver|Setting default evaluation strategy|Not push traits update to screen|BKSHIDEventObserver|UIScene.*lifecycle|sceneOfRecord:|Ignoring already applied deactivation|Deactivation reason|startConnection|activating connection:|Scene updated orientation|Key window|Window.*become.*key|Begin local event deferring|BKSHIDEventDeliveryManager|Setting client identifier|Starting server.*AXIPCServer|Scene target of|Stack\[KeyWindow\]|Realizing settings extension|Send setDeactivating|Creating hang event|Updating.*timestamp|Updating configuration of monitor|Creating side-channel connection|Skip setting user action|invalidated because|Hit the server for a process handle|Received state update|Smart invert app status|Alloc com\.apple\.|Cache loaded|Creating new.*assertion|Created new background|Initializing connection|Removing all cached|Creating connection to com\.apple|Sending handshake|activating monitor|Handshake succeeded|Identity resolved|FBSWorkspace|Incrementing reference|Created background task|attempting immediate handshake|sent handshake|Added observer for process|Evaluated capturing state|Read CategoryName|setting counterpart class|Registering for test daemon|notify_get_state|Selected display:|Should send trait collection|Initializing:.*Notifier|setDelegate:|Initialized with scene:|Could not load asset catalog|CoreUI:|Warning: no visual style|Note: no default visual style|Creating the shared game controller|Received configuration|establishing connection|Session created|Session activated|Create activity from XPC|Set activity.*global parent|AggregateDictionary is deprecated|Ending background task|Ending task with identifier|Decrementing reference count|Will invalidate assertion|Event Timing Profile|Target list changed|Scene became target of keyboard|Connected devices changed|handleKeyboardChange|forceReloadInputViews|Reloading input views|isWritingToolsHandlingKeyboardTracking|nw_activity.*complete|Unsetting the global parent|Unset the global parent|got GAX status|TX focusApplication|Evaluating dispatch of UIEvent|Sending UIEvent'

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

# If a target OS is not provided, assume we're trying to target iOS.
if [ -z "$TARGETOS" ]
then
    TARGETOS="iPhone OS"
fi

BUNDLE_NAME=$(plutil -convert json "$APP"/Info.plist -o - -r |grep CFBundleName | sed -e 's/.* : "\([^"]*\)",/\1/')

# With Xcode's devicectl we launch the app ourselves and show what it writes to stdout and stderr.  Launching it with
# OS_ACTIVITY_DT_MODE set copies its log messages there too, including NSLog() and so Corona's print().
. "${CORONA_RES_DIR}/ios_devicectl.sh"

# Filters the output of "devicectl device process launch --console", prefixing each line with LABEL and leaving out
# messages that match EXCLUDE_PATTERNS.  devicectl's own errors are held back and the exit status is 2 if the device
# was locked, 1 for any other error that stopped the app starting and 3 for an error after it started.
CONSOLE_FILTER_AWK='
	BEGIN { show = 1 }
	{ sub(/\r$/, "") }
	/^ERROR: .*\(com\.apple\.dt\.CoreDeviceError / { failed = 1 }
	/BSErrorCodeDescription = Locked/ { locked = 1 }
	failed { error = error ENVIRON["LABEL"] $0 "\n"; next }
	{ ran = 1 }
	# Log messages start with a timestamp and the process name and ids, lines without are part of the one before
	sub(/^[0-9-]+ [0-9:.+-]+ [^[]*\[[0-9]+:[0-9a-fx]+\] /, "") { show = (ENVIRON["LABEL"] $0 !~ ENVIRON["EXCLUDE_PATTERNS"]) }
	show { print ENVIRON["LABEL"] $0; fflush() }
	END { if (locked) exit 2; if (failed) { printf "%s", error; exit ran ? 3 : 1 } }
'

# Succeeds if the device is locked (apps can't be launched until it's unlocked)
device_is_locked()
{
	local JSON LOCKED

	JSON=$(mktemp -t corona-devicectl) || return 1

	"$DEVICECTL" device info lockState --quiet --timeout 30 --device "$1" --json-output "$JSON" >/dev/null 2>&1
	LOCKED=$(plutil -extract result.passcodeRequired raw -o - "$JSON" 2>/dev/null)

	rm -f "$JSON"

	[ "$LOCKED" == "true" ]
}

# Run the app on a device, prefixing each line of its output with a label.
#
# The app can't start while the device is locked so keep trying until it's unlocked (the lock state devicectl reports
# doesn't cover every case, the Lock Screen still showing after Touch ID say, so the launch error decides).  Other
# errors are shown and tried once more because they can be temporary, just after the device is unlocked for one.
run_app()
{
	local DEVICE_ID="$1" DEVICE_NAME="$2" LABEL="$3" STARTING= WAITING= RETRIED= STATUS

	while true
	do
		if device_is_locked "$DEVICE_ID"
		then
			STATUS=2
		else
			if [ -z "$STARTING" ]
			then
				echo "${LABEL}Starting $BUNDLE_NAME on $DEVICE_NAME"
				STARTING=YES
			fi

			# The app's output only comes to us if devicectl starts it so restart it if it's already running
			"$DEVICECTL" device process launch --console --terminate-existing --quiet --environment-variables '{"OS_ACTIVITY_DT_MODE": "YES"}' \
					--device "$DEVICE_ID" "$BUNDLE_ID" 2>&1 |
				LABEL="$LABEL" EXCLUDE_PATTERNS="$EXCLUDE_PATTERNS" awk "$CONSOLE_FILTER_AWK"
			STATUS=${PIPESTATUS[1]}
		fi

		if [ "$STATUS" == 2 ]
		then
			if [ -z "$WAITING" ]
			then
				echo "${LABEL}Unlock $DEVICE_NAME to start $BUNDLE_NAME"
				WAITING=YES
			fi

			sleep 1
		elif [ "$STATUS" == 1 ] && [ -z "$RETRIED" ]
		then
			RETRIED=YES
			sleep 2
		else
			break
		fi
	done
}

if devicectl_find_devices "$TARGETOS"
then
	BUNDLE_ID=$(plutil -extract CFBundleIdentifier raw -o - "$APP/Info.plist")

	# The Simulator terminates us when it stops showing the output.  devicectl passes on the signals it can catch
	# to the app so kill it outright to leave the app running.
	trap 'trap - TERM; pkill -KILL -g "$(ps -o pgid= -p $$ | tr -d " ")" -x devicectl; kill -TERM 0' TERM

	for DEVICE in "${DEVICECTL_DEVICES[@]}"
	do
		LABEL='[Device] '

		if [ "${#DEVICECTL_DEVICES[@]}" -gt 1 ]
		then
			LABEL="[$(devicectl_device_name "$DEVICE")] "
		fi

		run_app "$(devicectl_device_id "$DEVICE")" "$(devicectl_device_name "$DEVICE")" "$LABEL" &
	done

	wait
	exit
fi

# Otherwise use the libimobiledevice tools to tail the device's syslog (see ios_sendapp.sh)
if ! arch -x86_64 /usr/bin/true 2>/dev/null
then
	echo "$PROGRAM_NAME: no devices detected"
	exit 1
fi

echo "[Device] Launch $BUNDLE_NAME on the device to see its output"

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
