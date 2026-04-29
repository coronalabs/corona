#!/bin/bash -ex

CUSTOM_ID=$1
S3_BUCKET=$2
CWD=$(dirname "$0")

# Summarize xcodebuild output to stdout but save full output in separate file
XCODE_LOG_FILTERS="^    export |clang -x |libtool -static |^    cd $CWD"

FULL_LOG_FILE="mac-build-xcodebuild.log"
if [ "$WORKSPACE" != "" ]
then
	FULL_LOG_FILE="$WORKSPACE/$FULL_LOG_FILE"
fi

echo "### Full xcodebuild output can be found in $FULL_LOG_FILE"

# Note build machine details and Xcode version
uname -a >> "$FULL_LOG_FILE"
xcodebuild -version >> "$FULL_LOG_FILE"

# Corona Simulator

if [ "$CUSTOM_ID" ]
then
	xcodebuild -project "$CWD"/ratatouille.xcodeproj -target rttplayer -configuration Release CUSTOM_BUILD_ID="$CUSTOM_ID" BUILD_BUCKET="$S3_BUCKET" 2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"
else
    # xcodebuild -project "$CWD"/ratatouille.xcodeproj -target rttplayer -configuration Release clean
	xcodebuild -project "$CWD"/ratatouille.xcodeproj -target rttplayer -configuration Release CUSTOM_BUILD_ID="$CUSTOM_ID" 2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"
fi

if [ $? -ne 0 ]
then
	exit -1
fi
