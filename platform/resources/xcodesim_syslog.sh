#!/bin/bash
#----------------------------------------------------------------------------
#
# This file is part of the Corona game engine.
# For overview and more information on licensing please refer to README.md 
# Home page: https://github.com/coronalabs/corona
# Contact: support@coronalabs.com
#
#----------------------------------------------------------------------------

#
# ios_syslog.sh <app-bundle> [device_udid]
#
# Tail the syslog the specified iOS device filtering for Corona
#

DEBUG_BUILD_PROCESS=$(defaults read com.coronalabs.Corona_Simulator debugBuildProcess 2>/dev/null)

if [ "${DEBUG_BUILD_PROCESS:=0}" -gt 2 ]
then
	set -x
	exec 2>&1
fi

# Add "noise" logs to exclude here (egrep pattern)
EXCLUDE_PATTERNS='nw_connection_|nw_endpoint_|__nw_socket_service_|subsystem: com\.apple\.|\[Device\] TIC |HTTPCookieStorage singleton|Creating default cookie storage|policyStatus:|BKSHIDEventDeliveryPolicyObserver|Setting default evaluation strategy|Not push traits update to screen|BKSHIDEventObserver|UIScene.*lifecycle|sceneOfRecord:|Ignoring already applied deactivation|Deactivation reason|startConnection|activating connection:|Scene updated orientation|Key window|Window.*become.*key|Begin local event deferring|BKSHIDEventDeliveryManager|Setting client identifier|Starting server.*AXIPCServer|Scene target of|Stack\[KeyWindow\]|Realizing settings extension|Send setDeactivating|Creating hang event|Updating.*timestamp|Updating configuration of monitor|Creating side-channel connection|Skip setting user action|invalidated because|Hit the server for a process handle|Received state update|Smart invert app status|Alloc com\.apple\.|Cache loaded|Creating new.*assertion|Created new background|Initializing connection|Removing all cached|Creating connection to com\.apple|Sending handshake|activating monitor|Handshake succeeded|Identity resolved|FBSWorkspace|Incrementing reference|Created background task|attempting immediate handshake|sent handshake|Added observer for process|Evaluated capturing state|Read CategoryName|setting counterpart class|Registering for test daemon|notify_get_state|Selected display:|Should send trait collection|Initializing:.*Notifier|setDelegate:|Initialized with scene:|Could not load asset catalog|CoreUI:|Warning: no visual style|Note: no default visual style|Creating the shared game controller|Received configuration|establishing connection|Session created|Session activated|Create activity from XPC|Set activity.*global parent|AggregateDictionary is deprecated|Ending background task|Ending task with identifier|Decrementing reference count|Will invalidate assertion|Event Timing Profile|Target list changed|Scene became target of keyboard|Connected devices changed|handleKeyboardChange|forceReloadInputViews|Reloading input views|isWritingToolsHandlingKeyboardTracking|nw_activity.*complete|Unsetting the global parent|Unset the global parent|got GAX status|TX focusApplication|Evaluating dispatch of UIEvent|Sending UIEvent'

PROGRAM_NAME="ERROR" # $(basename "$0")

if [ "$1" == "" ]
then
	echo "$PROGRAM_NAME: Usage: $(basename "$0") <app-bundle> [device_udid]"
	exit 1
fi

APP="$1"
UDID="$2"

RESOURCE_DIR=$(dirname "$0")
IOS_SYSLOG_HELPER="$RESOURCE_DIR/ios-syslog-helper"

SYSLOG_PATH=~/"Library/Logs/CoreSimulator/$UDID/system.log"

BUNDLE_NAME=$(plutil -convert json "$APP"/Info.plist -o - -r |grep CFBundleName | sed -e 's/.* : "\([^"]*\)",/\1/')

tail -1000f "$SYSLOG_PATH" | "$IOS_SYSLOG_HELPER" now "$BUNDLE_NAME" '[iOS Simulator] ' | egrep --line-buffer -v "$EXCLUDE_PATTERNS" 
# tail -1000f "$SYSLOG_PATH" | "$IOS_SYSLOG_HELPER" now "$BUNDLE_NAME" '[iOS Simulator] '
