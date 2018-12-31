#!/bin/sh

# ------------------------------------------------------------------------------------------
# Builds the sample project from the command line.
#
# You must provide the path to the root Android SDK directory by doing one of the following:
# 1) Provide the path as a comman line argument. For example:  build.sh <MyAndroidSdkPath>
# 2) Set the path to an environment variable named "ANDROID_SDK".
# ------------------------------------------------------------------------------------------

#
# Checks exit value for error
# 
checkError() {
    if [ $? -ne 0 ]
    then
        echo "Exiting due to errors (above)"
        exit -1
    fi
}

script=`basename $0`
path=`dirname $0`

# 
# Canonicalize relative paths to absolute paths
# 
pushd $path > /dev/null
dir=`pwd`
path=$dir
popd > /dev/null

# Fetch the Android SDK path from the first command line argument.
# If not provided from the command line, then attempt to fetch it from environment variable ANDROID_SDK.
SDK_PATH=
if [ ! -z "$1" ]
then
	SDK_PATH=$1
else
	SDK_PATH=$ANDROID_SDK
fi

if [ -z "$CORONA_NATIVE_DIR" ]; then
	CORONA_NATIVE_DIR="$HOME/Library/Application Support/Corona/Native"
fi

if [ ! -z "$2" ]; then
	CORONA_PATH=$2
else
	CORONA_PATH=$CORONA_NATIVE_DIR
fi

RELATIVE_PATH_TOOL=$CORONA_PATH/Corona/mac/bin/relativePath.sh

CORONA_PATH=`"$RELATIVE_PATH_TOOL" "$path" "$CORONA_PATH"`
echo CORONA_PATH: $CORONA_PATH

# Do not continue if we do not have the path to the Android SDK.
if [ -z "$SDK_PATH" ]
then

	echo ""
	echo "USAGE:  $script"
	echo "USAGE:  $script android_sdk_path"
	echo "USAGE:  $script android_sdk_path corona_enterprise_path"
	echo "\tandroid_sdk_path: Path to the root Android SDK directory."
	echo "\tcorona_enterprise_path: Path to the CoronaEnterprise directory."
	exit -1
fi

# Before we can do a build, we must update all Android project directories to use the given Android SDK.
# We do this by running the "android" command line tool. This will add a "local.properties" file to all
# project directories that is required by the Ant build system to compile these projects for Android.
"$SDK_PATH/tools/android" update project -p .
checkError

#
# Builds the ".jar" file.
#

ANDROID_MANIFEST=$path/AndroidManifest.plugin.xml

PLUGIN_NAME=`find src -name LuaLoader.java`
# Remove 'src/' prefix and 'LuaLoader.java' suffix. Replace '/' with '.'
PLUGIN_NAME=`echo $PLUGIN_NAME | sed -E "s#src/##" | sed -E "s#/LuaLoader.java##" | sed -E "s#/#.#g"`


## Clean
ant -buildfile build.plugin.xml -D"manifest.file"="$ANDROID_MANIFEST" -DCoronaEnterpriseDir="$CORONA_PATH" clean
checkError

## Compile Jar (outputs to bin/$PLUGIN_NAME.jar)
ant -buildfile build.plugin.xml				\
	-D"manifest.file"="$ANDROID_MANIFEST"	\
	-DCoronaEnterpriseDir="$CORONA_PATH"	\
	-DCoronaPluginName=$PLUGIN_NAME			\
	release
checkError

if [ -f "$path/bin/${PLUGIN_NAME}.jar" ]
then
	# Remove non-plugin related code
	zip -d "$path/bin/${PLUGIN_NAME}.jar" "com/*"

	echo "SUCCESS: Plugin binary is located at: '$path/bin/${PLUGIN_NAME}.jar'"
else
	echo "ERROR: Could not create '$path/bin/${PLUGIN_NAME}.jar'"
	exit -1	
fi
