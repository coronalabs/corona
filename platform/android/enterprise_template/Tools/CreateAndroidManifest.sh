#!/bin/bash

# ------------------------------------------------------------------------------------------
# Generates a new "AndroidManifest.xml" file from a Corona "build.settings" file.
#
# This utility will overwrite the last AndroidManifest.xml file. So, if you have edited the
# previous AndroidManifest.xml file, then those changes will be lost.
#
# Execute this utility at the command line without any arguments for an explanation
# on how to use it.
# ------------------------------------------------------------------------------------------


# If no arguments have been received, then explain how to use this command line utility.
if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ] || [ -z "$4" ]
then
	echo ""
	echo "Summary:"
	echo "Generates a new \"AndroidManifest.xml\" file based on a Corona project's \"build.settings\" file."
	echo ""
	echo "Usage:"
	echo "CreateAndroidManifest.sh android_manifest corona_project package_name [app_name [version_code [version_name]]]"
	echo "  android_manifest: The path of the \"AndroidManifest.xml\" file to generate."
	echo "                    This must include the file name."
	echo "                    If the file already exists, then it will be overwritten."
	echo "  corona_project:   The path to the Corona project directory that contains the"
	echo "                    \"build.settings\" file."
	echo "  package_name:     The unique package name of your app."
	echo "  app_name:         Optional argument. The human readable name of your application."
	echo "                    Will be set to the Corona project directory name if not assigned."
	echo "  version_code:     Optional argument. Version of your app in integer form."
	echo "                    Will be set to 1 if not assigned."
	echo "  version_name:     Optional argument. The app's human readable version string."
	echo "                    Will be set to \"1.0\" if not assigned."
	echo ""
	echo "Example:"
	echo "CreateAndroidManifest.sh \"~/MyAndroidProject/AndroidManifest.xml\" \"~/MyAndroidProject/corona\" \"com.coronalabs.myapp\" \"MyApp\" 102 \"1.02\""
	echo ""
	exit -1
fi

# Fetch command line arguments.
ANDROID_MANIFEST_FILE_PATH=$1
CORONA_PROJECT_DIRECTORY_PATH=$2
PACKAGE_NAME=$3
APP_NAME=$4
VERSION_CODE=$5
VERSION_NAME=$6

# Check if the given Corona project directory exists.
if [ ! -d "$CORONA_PROJECT_DIRECTORY_PATH" ]
then
	echo "Directory not found: $CORONA_PROJECT_DIRECTORY_PATH"
	exit -1
fi

# Strip the trailing slash from the Corona project directory if it exists.
CORONA_PROJECT_DIRECTORY_PATH=${CORONA_PROJECT_DIRECTORY_PATH%/}

# Check if a "build.settings" file exists in the Corona project directory.
# Warn the user if it is missing, but do not error out since this file is not required.
BUILD_SETTINGS_FILE_PATH=$CORONA_PROJECT_DIRECTORY_PATH/build.settings
if [ ! -f "$BUILD_SETTINGS_FILE_PATH" ]
then
	echo "Warning! File not found: $BUILD_SETTINGS_FILE_PATH"
elif [ ! -r "$BUILD_SETTINGS_FILE_PATH" ]
then
	echo "Warning! Read permission not set for file: $BUILD_SETTINGS_FILE_PATH"
fi

# If an app name has not been provided, then use the Corona project's directory name.
if [ -z APP_NAME ]
then
	APP_NAME=`basename "$CORONA_PROJECT_DIRECTORY_PATH"`
fi

# Fetch an absolute path to this script's directory.
CURRENT_DIRECTORY=`dirname $0`

# Generate a unique directory under the Temp directory.
# Will be used to write a temporary "build.properties" file from the "build.settings" file.
TEMP_DIRECTORY=$TMPDIR
if [ -z "TEMP_DIRECTORY" ] || [ ! -d "$TEMP_DIRECTORY" ]
then
	TEMP_DIRECTORY="/tmp/"
	if [ ! -d $TEMP_DIRECTORY ]
	then
		echo "Failed to access the machine's temporary directory."
		exit -1
	fi
fi
TEMP_DIRECTORY=${TEMP_DIRECTORY%/}
TEMP_DIRECTORY=`mktemp -d $TEMP_DIRECTORY/Corona.CreateAndroidManifest.XXXXXXXXXXXXXXXX`

# Create a "build.properties" file from the given "build.settings" file.
"$CURRENT_DIRECTORY/lua" "$CURRENT_DIRECTORY/create_build_properties.lua" "$TEMP_DIRECTORY" $PACKAGE_NAME "$CORONA_PROJECT_DIRECTORY_PATH" $VERSION_CODE $VERSION_NAME
if [ $? -ne 0 ]
then
	echo "Error creating \"build.properties\" file."
	exit -1
fi

# Create the "AndroidManifest.xml" file from the above "build.properties" file.
"$CURRENT_DIRECTORY/lua" "$CURRENT_DIRECTORY/update_manifest.lua" "$CURRENT_DIRECTORY/AndroidManifestTemplate.xml" "$TEMP_DIRECTORY/build.properties" "$APP_NAME" "$ANDROID_MANIFEST_FILE_PATH"
if [ $? -ne 0 ]
then
	echo "Error creating file: $ANDROID_MANIFEST_FILE_PATH"
	exit -1
fi

# The "AndroidManifest.xml" file was successfully created. Display a success message.
echo "Successfully created file: $ANDROID_MANIFEST_FILE_PATH"
