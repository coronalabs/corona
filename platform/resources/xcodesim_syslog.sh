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
EXCLUDE_PATTERNS="IOHIDLib|MobileContainerManager|com.apple.containermanagerd.internal|systemgroup.com.apple.configurationprofiles|Overriding MCM with the one true path|assertion failed: .*: libxpc.dylib"

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
