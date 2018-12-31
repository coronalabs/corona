#!/bin/bash -x

#
# Checks exit value for error
# 
checkError() {
	# shellcheck disable=SC2181
    if [ $? -ne 0 ]
    then
        echo "Exiting due to errors (above)"
        exit -1
    fi
}

path=$(dirname "$0")

# Canonicalize paths
pushd "$path" > /dev/null
dir=$(pwd)
path=$dir
popd > /dev/null


UNAME=$(uname)
if [ "Darwin" != "$UNAME" ]
then
	LUA=lua
else

#	if [ ! -e $path/../../bin/mac/lua ] ; then
#		xcodebuild -project $path/../../platform/mac/lua.xcodeproj -target lua
#	fi

	# This script is at a fixed location relative to the corona repo root
	CORONA_ROOT=$path/../..	

	# Ugh, symlinks are really making this stupid.
	# Make sure shell tools are available
	if [ ! -e "${CORONA_ROOT}/bin/mac/luac" ] ; then
		#xcodebuild -project ${CORONA_ROOT}/platform/mac/lua.xcodeproj -target luac DSTROOT=${CORONA_ROOT}/platform/mac/build/Release install
		xcodebuild -project "${CORONA_ROOT}/platform/mac/lua.xcodeproj" -target luac
	fi

	if [ ! -e "${CORONA_ROOT}/bin/mac/lua" ] ; then
		#xcodebuild -project ${CORONA_ROOT}/platform/mac/lua.xcodeproj -target luac DSTROOT=${CORONA_ROOT}/platform/mac/build/Release install
		xcodebuild -project "${CORONA_ROOT}/platform/mac/lua.xcodeproj" -target lua
	fi

	LUA=${CORONA_ROOT}/bin/mac/lua
fi

usage() {
	echo "USAGE: $0 [product_type] [build_config] [build_type] [device_type]"
	echo "   product_type:   trial, basic, automation, coronacards, all (default)"
	echo "   build_config:   debug (default) or release"
	echo "   build_type:     incremental (default) or clean"
	echo "   device_type:    generic, kindle, nook_only, all (default)"
	exit 0
}

if [ "-h" = "$1" ] || [ "--help" = "$1" ]
then
	usage
fi

PRODUCT_TYPE="coronacards"

if [ "release" = "$2" ]
then
	echo "Building RELEASE"
	BUILD_CONFIG="release"
else
	echo "Defaulting to DEBUG"
	BUILD_CONFIG="debug"
fi

BUILD_TYPE=$3

DEVICE_TYPE=$4
if [ "generic" = "$4" ]
then
	echo "Building for generic Android devices"
elif [ "kindle" = "$4" ]
then
	echo "Building for Kindle Fire"
elif [ "nook_only" = "$4" ]
then
	echo "Building for Nook devices only"
else
	echo "Building for all Android devices"
	DEVICE_TYPE="all"
fi

# Run ant script to do actual build
pushd "$path/sdk" > /dev/null

	# Update the Corona project directory to use the selected Android SDK.
	# This will generate a "local.properties" file needed to build this project.
	"$ANDROID_SDK/tools/android" update project -p .

	# Use Lua script to parse the project.properties file to do the following:
	# 1) Updates 3rd party library project directories to use the selected Android SDK.
	#    - This will run "android update lib-project" to generate needed "local.properties" files.
	# 2) Copy 3rd party resource files to the template directory.
	# 3) Generate an "external_libs_paths" file to be used by the build server.
	$LUA -e "parameterArgTable={inputFile='$path/sdk/project.properties', absolutePathToInputFile='$path/sdk', templateDir='$path/template', externalLibsPathsFile='$path/template/external-libs-paths', absolutePathToAndroidSdk='$ANDROID_SDK'}" "$path/ParseAndroidProjectProperties.lua"
	touch "$path/template/external-libs-paths"


	# --------------------------------------------------------------------------------
	# Copy widget library's image files to the resource directory.
	# --------------------------------------------------------------------------------
	
	# Remove last files that were copied to the "res/raw" directory.
	find "$path/sdk/res/raw" -type f -name "corona_asset_*" -delete

	# ----------------------------------------
	# Splash Screen Beaconing
	# ----------------------------------------
	# Burn size of default splash screen into template so we can tell at runtime
	# whether the default splash screen was shown

	# The value is used to determine whether the app is displaying the default (unpaid) splash screen or one
	# of their own and this controls whether the "corona splash screen shown" analytics beacon is sent.

	mkdir -p "$path/ndk/generated"
	SPLASH_SCREEN_TEMPLATE="$path/sdk/SplashScreenBeacon.java.template"
	SPLASH_SCREEN_JAVA="$path/sdk/src/com/ansca/corona/SplashScreenBeacon.java"
	SPLASH_IMAGE_FILE="$path/sdk/res/drawable/_corona_splash_screen.png"

	if [ -r "$SPLASH_IMAGE_FILE" ]
	then
		DEFAULT_SPLASH_IMAGE_FILE_SIZE_ANDROID="$("${CORONA_ROOT}/tools/get_splash_screen_size.sh" android "${CORONA_ROOT}")"
		if [[ "$DEFAULT_SPLASH_IMAGE_FILE_SIZE_ANDROID" =~ ^ERROR: ]]
		then
			# We got an error, bail
			echo "$DEFAULT_SPLASH_IMAGE_FILE_SIZE_ANDROID"
			exit 1
		fi
	else
		DEFAULT_SPLASH_IMAGE_FILE_SIZE_ANDROID=0
	fi

	echo "Updating $SPLASH_SCREEN_JAVA ..."
	rm -f "$SPLASH_SCREEN_JAVA"
	sed -e "s/999/$DEFAULT_SPLASH_IMAGE_FILE_SIZE_ANDROID/" "$SPLASH_SCREEN_TEMPLATE" > "$SPLASH_SCREEN_JAVA"

	# ----------------------------------------
	# Prepare plugins
	# ----------------------------------------
	# preparePlugin "ads-inmobi"
	# preparePlugin "ads-inneractive"
	

	# ----------------------------------------
	# Build Corona Template
	# ----------------------------------------

	# Delete intermediate files if a clean build was requested.
	if [ "clean" = "$BUILD_TYPE" ]
	then
		echo "CLEANING java files..."
		ant clean; checkError
	fi

	# Build a "release" version so that we can get an unsigned apk!
	ant -v release -Dcorona.device.type="$DEVICE_TYPE" -Dcorona.product.type="$PRODUCT_TYPE" -Dcorona.build.config="$BUILD_CONFIG" -Dcorona.build.type="$BUILD_TYPE"
	checkError

	# ----------------------------------------
	# Build Corona JAR Library
	# ----------------------------------------

	# Clean the last built "Corona Cards" directory.
	CARDS_OUTPUT_DIR="$path/bin/Corona/Libraries/CoronaCards/"
	rm -Rf "$CARDS_OUTPUT_DIR"
	echo "$CARDS_OUTPUT_DIR"
	mkdir -p "$CARDS_OUTPUT_DIR"

	# Delete the Android generated "R" resource classes.
	# JAR files are not supposed to contain these files because new R classes will be generated for
	# the customer's Android application and this JAR library when resources are merged.
	find "$path/sdk/bin" -type f -name "R.class" -delete
	find "$path/sdk/bin" -type f -name "R\$*.class" -delete
	rm -Rf gen

	# find "$path/sdk/bin" -type f -name "CoronaActivity.class" -delete
	# find "$path/sdk/bin" -type f -name "CoronaActivity\$.class" -delete
	

	# Create the Corona JAR file.
	# It should only contain compiled files belonging to Corona and not 3rd party libraries.
	JAVA_CLASSES_PATH="$path/sdk/bin/classes"
	jar cvf "$path/sdk/bin/Corona.jar" -C "$JAVA_CLASSES_PATH" "android/opengl" -C "$JAVA_CLASSES_PATH" "com/ansca" -C "$JAVA_CLASSES_PATH" "com/android"
	mkdir "$CARDS_OUTPUT_DIR/libs"
	cp -Rf "$path/sdk/bin/Corona.jar" "$CARDS_OUTPUT_DIR/libs/Corona.jar"

	# Copy required library dependencies to the Corona library's directory.
	cp -Rf "$path/sdk/libs/armeabi-v7a/" "$CARDS_OUTPUT_DIR/libs/armeabi-v7a"
	rm -Rf "$CARDS_OUTPUT_DIR/libs/armeabi-v7a/libmpg123.so"
	rm -Rf "$CARDS_OUTPUT_DIR/libs/armeabi-v7a/libads.so"
	rm -Rf "$CARDS_OUTPUT_DIR/libs/armeabi-v7a/libanalytics.so"
	rm -Rf "$CARDS_OUTPUT_DIR/libs/armeabi-v7a/libCoronaProvider.gameNetwork.corona.so"
	rm -Rf "$CARDS_OUTPUT_DIR/libs/armeabi-v7a/libgameNetwork.so"
	rm -Rf "$CARDS_OUTPUT_DIR/libs/armeabi-v7a/liblicensing.so"
	cp -Rf "$path/sdk/libs/JNLua.jar" "$CARDS_OUTPUT_DIR/libs/JNLua.jar"
	cp -Rf "$path/sdk/libs/org.apache.http.legacy.jar" "$CARDS_OUTPUT_DIR/libs/org.apache.http.legacy.jar"

	cp -vf "$path"/sdk/libs/network.jar "$CARDS_OUTPUT_DIR/libs"
	checkError

	#"$path"/build_cards_docs.sh
	# TODO: GET THIS WORKING WITH ANDROID DOCUMENTATION LINKED IN VIA SOMETHING LIKE:
	# javadoc ... -linkoffline http://d.android.com/reference file://"$ANDROID_SDK"/docs/reference
	# Generate JavaDoc API documentation for public Corona classes.
	JAVA_DOC_OUTPUT_PATH="$CARDS_OUTPUT_DIR/API Documentation"
	JAVA_SOURCE_PATH="$path/sdk/src/com/ansca/corona"
	JAVA_DOC_LOGFILE="${WORKSPACE:-.}/android-javadoc-build_template_cards.log"

	CORONA_JAVA_FILES="$JAVA_SOURCE_PATH/package-info.java
		$JAVA_SOURCE_PATH/ApplicationContextProvider.java
		$JAVA_SOURCE_PATH/CoronaView.java
		$JAVA_SOURCE_PATH/CoronaEnvironment.java
		$JAVA_SOURCE_PATH/CoronaLua.java
		$JAVA_SOURCE_PATH/CoronaLuaEvent.java
		$JAVA_SOURCE_PATH/CoronaRuntime.java
		$JAVA_SOURCE_PATH/CoronaRuntimeListener.java
		$JAVA_SOURCE_PATH/CoronaRuntimeTask.java
		$JAVA_SOURCE_PATH/CoronaRuntimeTaskDispatcher.java
		$JAVA_SOURCE_PATH/permissions/package-info.java
		$JAVA_SOURCE_PATH/permissions/PermissionsServices.java
		$JAVA_SOURCE_PATH/permissions/PermissionsSettings.java
		$JAVA_SOURCE_PATH/permissions/PermissionState.java
		$JAVA_SOURCE_PATH/storage/package-info.java
		$JAVA_SOURCE_PATH/storage/FileContentProvider.java
		$JAVA_SOURCE_PATH/storage/FileServices.java
		$JAVA_SOURCE_PATH/storage/PackageServices.java
		$JAVA_SOURCE_PATH/storage/PackageState.java
		$JAVA_SOURCE_PATH/storage/UniqueFileNameBuilder.java
		$JAVA_SOURCE_PATH/storage/ZipFileEntryInputStream.java"

	# shellcheck disable=SC2086
	javadoc -d "$JAVA_DOC_OUTPUT_PATH" -link http://docs.oracle.com/javase/7/docs/api/ -sourcepath "$path/../../external/JNLua/src/main/java" com.naef.jnlua $CORONA_JAVA_FILES > "$JAVA_DOC_LOGFILE" 2>&1

popd > /dev/null
