#!/bin/bash

path=$(dirname "$0")

# Summarize xcodebuild output to stdout but save full output in separate file
XCODE_LOG_FILTERS="^    export |clang -x |libtool -static |^    cd $path"

FULL_LOG_FILE="iphone-build-xcodebuild.log"
if [ "$WORKSPACE" != "" ]
then
	FULL_LOG_FILE="$WORKSPACE/$FULL_LOG_FILE"
fi

echo "### Full xcodebuild output can be found in $FULL_LOG_FILE"

# lua, luac, debugger, etc.
xcodebuild -project "$path"/../mac/lua.xcodeproj -alltargets -configuration Release 2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"

# car
xcodebuild -project "$path"/../mac/car.xcodeproj -target car -configuration Release 2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"

# Corona Simulator
xcodebuild -project "$path"/ratatouille.xcodeproj -target rttplayer -configuration Release-template -sdk iphoneos 2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"
xcodebuild -project "$path"/ratatouille.xcodeproj -target rttplayer -configuration Release-template-trial -sdk iphoneos 2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"
