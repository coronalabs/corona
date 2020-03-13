#!/bin/bash -x

# If stdout is the terminal, save a copy of the output to a log file
if [ -t 1 ]
then
	# Redirect stdout ( > ) into a named pipe ( >() ) running "tee"
	exec > >(tee "build_template.log")
	# Copy stderr to stdout
	exec 2>&1
fi

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

if [ -z "$1" ] || ([ "all" != "$1" ] && [ "basic" != "$1" ] && [ "trial" != "$1" ] && [ "automation" != "$1" ] && [ "coronacards" != "$1" ] )
then
	echo 'Defaulting to "all" product template'
	PRODUCT_TYPE="all"
else
	PRODUCT_TYPE=$1
fi


if [ "release" = "$2" ]
then
	echo "Building RELEASE"
	export BUILD_CONFIG="release"
else
	echo "Defaulting to DEBUG"
	export BUILD_CONFIG="debug"
fi

BUILD_TYPE=$3

DEVICE_TYPE=$4
if [ "generic" = "$4" ]
then
	echo "Building for generic Android devices"
elif [ "kindle" = "$4" ]
then
	echo "Building for Kindle Fire"
	cp -Rf "$path/sdk/amazon_game_circle_res/" "$path/sdk/res/"
elif [ "nook_only" = "$4" ]
then
	echo "Building for Nook devices only"
else
	echo "Building for all Android devices"
	DEVICE_TYPE="all"
fi

# preparePlugin()
# {
# 	PLUGIN_NAME=$1
# 	PLUGIN_PATH=${CORONA_ROOT}/plugins/${PLUGIN_NAME}/android
# 	pushd ${PLUGIN_PATH} > /dev/null
# 		# Update the Corona project directory to use the selected Android SDK.
# 		# This will generate a "local.properties" file needed to build this project.
# 		"${ANDROID_SDK}/tools/android" update project -p .
# 
# 		# Delete intermediate files if a clean build was requested.
# 		if [ "clean" = "$BUILD_TYPE" ]
# 		then
# 			echo "CLEANING java files..."
# 			ant clean
#           checkError
# 		fi
# 	popd > /dev/null
# }


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

	# Copy all Corona widget image files to the "res/raw" directory.
	# The copied files must be renamed for the following reasons:
	# 1) Replace illegal resource characters with underscores. This is an Android requirement.
	# 2) Lower case all characters in the file name. This is an Android requirement.
	# 3) Prefix the resource file wth "corona_asset_" so that Corona can identify them.
	# shellcheck disable=SC2044
	for SOURCE_FILE_NAME in $(find "$path/../../subrepos/widget" -maxdepth 1 -type f -iname "*.png")
	do
		TARGET_FILE_NAME=$(basename "$SOURCE_FILE_NAME")
		TARGET_FILE_NAME=$(echo "$TARGET_FILE_NAME" | tr "[:upper:]" "[:lower:]")
		TARGET_FILE_NAME=$(echo "$TARGET_FILE_NAME" | tr '@' '_')
		TARGET_FILE_NAME=corona_asset_$TARGET_FILE_NAME
		cp -Rf "$SOURCE_FILE_NAME" "$path/sdk/res/raw/$TARGET_FILE_NAME"
	done


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
		ant clean
		checkError
	fi

	# Build a "release" version so that we can get an unsigned apk!
	ant -v release -Dcorona.device.type="$DEVICE_TYPE" -Dcorona.product.type="$PRODUCT_TYPE" -Dcorona.build.config="$BUILD_CONFIG" -Dcorona.build.type="$BUILD_TYPE"
	checkError

	# Something in the ant process causes files with .orig extensions to be added to the apk
	# this seems like the only way to be sure they are removed
	zip -q "$path/sdk/bin/template-release-unsigned.apk" -d '*.orig' >/dev/null

	# Rename output to template.apk
	echo "SUCCESS: $path/sdk/bin/template.apk"
	APK_FILE=$path/sdk/bin/template-release-unsigned.apk
	mv "$APK_FILE" "$path/template/template.apk"
	checkError

	# Copy Corona's resource directory to the template directory.
	rm -rf "$path/template/res"
	cp -Rf "$path/sdk/res" "$path/template/res"


	# ----------------------------------------
	# Build Corona JAR Library
	# ----------------------------------------

	# Clean the last built "Corona Enterprise" directory.
	rm -Rf "$path/bin/enterprise"
	mkdir -p "$path/bin/enterprise"
	cp -Rf "$path/enterprise_template/" "$path/bin/enterprise"

	# Delete the Android generated "R" resource classes.
	# JAR files are not supposed to contain these files because new R classes will be generated for
	# the customer's Android application and this JAR library when resources are merged.
	find "$path/sdk/bin" -type f -name "R.class" -delete
	find "$path/sdk/bin" -type f -name "R\$*.class" -delete
	rm -Rf gen

	find "$path/sdk/bin" -type f -name "CoronaView.class" -delete
	find "$path/sdk/bin" -type f -name "CoronaView\$*.class" -delete

	# Create the Corona JAR file.
	# It should only contain compiled files belonging to Corona and not 3rd party libraries.
	JAVA_CLASSES_PATH="$path/sdk/bin/classes"
	jar cvf "$path/sdk/bin/Corona.jar" -C "$JAVA_CLASSES_PATH" "com/ansca"
	mkdir "$path/bin/enterprise/Libraries/Corona/libs"
	cp -Rf "$path/sdk/bin/Corona.jar" "$path/bin/enterprise/Libraries/Corona/libs/Corona.jar"
	# Copy the Corona JAR file to the build template for multi-dex builds.
	cp -Rf "$path/sdk/bin/Corona.jar" "$path/template/coreJars/Corona.jar"

	# Copy required library dependencies to the Corona library's directory.
	cp -Rf "$path/sdk/libs/armeabi-v7a/" "$path/bin/enterprise/Libraries/Corona/libs/armeabi-v7a"
	cp -Rf "$path/sdk/libs/JNLua.jar" "$path/bin/enterprise/Libraries/Corona/libs/JNLua.jar"
	cp -Rf "$path/sdk/libs/org.apache.http.legacy.jar" "$path/bin/enterprise/Libraries/Corona/libs/org.apache.http.legacy.jar"
	cp -Rf "$path/sdk/libs/licensing-google.jar" "$path/bin/enterprise/Libraries/Corona/libs/licensing-google.jar"

	cp -vf "$path"/sdk/libs/network.jar "$path"/bin/enterprise/Libraries/Corona/libs/
	checkError

	# Copy Corona's resource directory to the Corona library's directory.
	cp -Rf "$path/sdk/res" "$path/bin/enterprise/Libraries/Corona/res"
	find "$path/bin/enterprise/Libraries/Corona/res" -type f -name "icon.png" -delete
	find "$path/bin/enterprise/Libraries/Corona/res" -type f -name "ouya_icon.png" -delete
	find "$path/bin/enterprise/Libraries/Corona/res" -type f -name "ouya_xiaomi_icon.png" -delete
	find "$path/bin/enterprise/Libraries/Corona/res" -type f -name "banner.png" -delete
	
	# Delete the above libraries' "bin" and "gen" directories that the compiler generated.
	find "$path/bin/enterprise/Libraries" -type d -name "bin" -exec rm -rf {} \;
	find "$path/bin/enterprise/Libraries" -type d -name "gen" -exec rm -rf {} \; 

	# Delete all Android generated "local.properties" files from the Libraries directory.
	# These files contain an absolute path to the Android SDK on the local machine and are not to be distributed.
	find "$path/bin/enterprise/Libraries" -type f -name "local.properties" -delete

	#"$path"/build_enterprise_docs.sh
	# TODO: GET THIS WORKING WITH ANDROID DOCUMENTATION LINKED IN VIA SOMETHING LIKE:
	# javadoc ... -linkoffline http://d.android.com/reference file://"$ANDROID_SDK"/docs/reference
	# Generate JavaDoc API documentation for public Corona classes.
	JAVA_DOC_OUTPUT_PATH="$path/bin/enterprise/API Documentation"
	JAVA_SOURCE_PATH="$path/sdk/src/com/ansca/corona"
	JAVA_DOC_LOGFILE="${WORKSPACE:-.}/android-javadoc-build_template_cards.log"

	javadoc -d "$JAVA_DOC_OUTPUT_PATH" -link http://docs.oracle.com/javase/7/docs/api/ -sourcepath "$path/../../external/JNLua/src/main/java" com.naef.jnlua \
		"$JAVA_SOURCE_PATH/package-info.java" \
		"$JAVA_SOURCE_PATH/ApplicationContextProvider.java" \
		"$JAVA_SOURCE_PATH/CoronaActivity.java" \
		"$JAVA_SOURCE_PATH/CoronaEnvironment.java" \
		"$JAVA_SOURCE_PATH/CoronaLua.java" \
		"$JAVA_SOURCE_PATH/CoronaLuaEvent.java" \
		"$JAVA_SOURCE_PATH/CoronaRuntime.java" \
		"$JAVA_SOURCE_PATH/CoronaRuntimeListener.java" \
		"$JAVA_SOURCE_PATH/CoronaRuntimeTask.java" \
		"$JAVA_SOURCE_PATH/CoronaRuntimeTaskDispatcher.java" \
		"$JAVA_SOURCE_PATH/permissions/package-info.java" \
		"$JAVA_SOURCE_PATH/permissions/PermissionsServices.java" \
		"$JAVA_SOURCE_PATH/permissions/PermissionsSettings.java" \
		"$JAVA_SOURCE_PATH/permissions/PermissionState.java" \
		"$JAVA_SOURCE_PATH/storage/package-info.java" \
		"$JAVA_SOURCE_PATH/storage/FileContentProvider.java" \
		"$JAVA_SOURCE_PATH/storage/FileServices.java" \
		"$JAVA_SOURCE_PATH/storage/PackageServices.java" \
		"$JAVA_SOURCE_PATH/storage/PackageState.java" \
		"$JAVA_SOURCE_PATH/storage/UniqueFileNameBuilder.java" \
		"$JAVA_SOURCE_PATH/storage/ZipFileEntryInputStream.java" > "$JAVA_DOC_LOGFILE" 2>&1

	# Copy files needed by the Android "Tools" directory.
	cp -Rf "$path/template/AndroidManifest.xml" "$path/bin/enterprise/Tools/AndroidManifestTemplate.xml"
	cp -Rf "$path/template/update_manifest.lua" "$path/bin/enterprise/Tools/update_manifest.lua"
	cp -Rf "$path/create_build_properties.lua" "$path/bin/enterprise/Tools/create_build_properties.lua"

popd > /dev/null
