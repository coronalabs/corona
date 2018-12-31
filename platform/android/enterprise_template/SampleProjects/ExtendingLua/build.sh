#!/bin/bash

# ------------------------------------------------------------------------------------------
# Builds the sample project from the command line.
#
# You must provide the path to the root Android SDK directory by doing one of the following:
# 1) Provide the path as a comman line argument. For example:  build.sh <MyAndroidSdkPath>
# 2) Set the path to an environment variable named "ANDROID_SDK".
# ------------------------------------------------------------------------------------------


# Fetch the Android SDK path from the first command line argument.
# If not provided from the command line, then attempt to fetch it from environment variable ANDROID_SDK.
SDK_PATH=
if [ ! -z "$1" ]
then
	SDK_PATH=$1
else
	SDK_PATH=$ANDROID_SDK
fi


# Do not continue if we do not have the path to the Android SDK.
if [ -z SDK_PATH ]
then
	echo ""
	echo "USAGE:  build.sh [android_sdk_path]"
	echo "\tandroid_sdk_path: Path to the root Android SDK directory."
	exit -1
fi


# Before we can do a build, we must update all Android project directories to use the given Android SDK.
# We do this by running the "android" command line tool. This will add a "local.properties" file to all
# project directories that is required by the Ant build system to compile these projects for Android.
"$SDK_PATH/tools/android" update project -p .
"$SDK_PATH/tools/android" update lib-project -p "../../Libraries/Corona"


# Build the Test project via the Ant build system.
ant release
