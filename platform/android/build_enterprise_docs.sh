#!/bin/bash

# Builds just the JavaDoc for Corona Enterprise APIs.
# Use this script to iterate on the JavaDocs without rebuilding an entire project.

# TODO: GET THIS WORKING WITH ANDROID DOCUMENTATION LINKED IN VIA SOMETHING LIKE:
# javadoc ... -linkoffline http://d.android.com/reference file://"$ANDROID_SDK"/docs/reference

path=`dirname $0`

# Generate JavaDoc API documentation for public Corona classes.
JAVA_DOC_OUTPUT_PATH="$path/bin/enterprise/API Documentation"
JAVA_SOURCE_PATH="$path/sdk/src/com/ansca/corona"
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
	"$JAVA_SOURCE_PATH/storage/ZipFileEntryInputStream.java"