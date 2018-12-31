#!/bin/bash

path=$(dirname "$0")

# Summarize xcodebuild output to stdout but save full output in separate file
XCODE_LOG_FILTERS="^    export |clang -x |libtool -static |^    cd $path"

FULL_LOG_FILE="iphone-build_tools-xcodebuild.log"
if [ "$WORKSPACE" != "" ]
then
	FULL_LOG_FILE="$WORKSPACE/$FULL_LOG_FILE"
fi

echo "### Full xcodebuild output can be found in $FULL_LOG_FILE"

# lua, luac, debugger, etc.
xcodebuild -project "$path"/../mac/lua.xcodeproj -alltargets -configuration Release 2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"

# car
xcodebuild -project "$path"/../mac/car.xcodeproj -target car -configuration Release 2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"

# app_sign
xcodebuild -project "$path"/../mac/app_sign.xcodeproj -target app_sign -configuration Release 2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"

