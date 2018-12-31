#!/bin/bash -x
set -x
# This script replaces place holders in Lua templates with the current size of the default splash screen

TEMPLATE_LUA_ANDROID="$PROJECT_DIR/../../tools/CoronaBuilder/_CoronaSetup_android.lua.template"
OUTPUT_LUA_ANDROID="$(echo "$PROJECT_DIR/../../tools/CoronaBuilder/_CoronaSetup_android.lua.template" | sed -e 's/.template$//')"
TEMPLATE_LUA_IOS="$PROJECT_DIR/../../tools/CoronaBuilder/_CoronaSetup_ios.lua.template"
OUTPUT_LUA_IOS="$(echo "$PROJECT_DIR/../../tools/CoronaBuilder/_CoronaSetup_ios.lua.template" | sed -e 's/.template$//')"
PNG_FILE_ANDROID="$PROJECT_DIR/../../platform/android/sdk/res/drawable/_corona_splash_screen.png"
PNG_FILE_IOS="$PROJECT_DIR/../../platform/iphone/_CoronaSplashScreen.png"
TMP_PNG_FILE_IOS="/tmp/_CoronaSplashScreen.png"

echo "Generating $OUTPUT_LUA_ANDROID ..."
DEFAULT_SPLASH_IMAGE_FILE_SIZE_ANDROID=$(stat -f "%z" "$PNG_FILE_ANDROID")
sed -e "s/DEFAULT_ANDROID_SPLASH_IMAGE_FILE_SIZE/$DEFAULT_SPLASH_IMAGE_FILE_SIZE_ANDROID/" "$TEMPLATE_LUA_ANDROID" > "$OUTPUT_LUA_ANDROID"

echo "Generating $OUTPUT_LUA_IOS ..."
# building compresses the PNG file so we need to use the processed size
"$DEVELOPER_BIN_DIR"/copypng -compress -strip-PNG-text "$PNG_FILE_IOS" $TMP_PNG_FILE_IOS
DEFAULT_SPLASH_IMAGE_FILE_SIZE_IOS=$(stat -f "%z" $TMP_PNG_FILE_IOS)
sed -e "s/DEFAULT_IOS_SPLASH_IMAGE_FILE_SIZE/$DEFAULT_SPLASH_IMAGE_FILE_SIZE_IOS/" "$TEMPLATE_LUA_IOS" > "$OUTPUT_LUA_IOS"
