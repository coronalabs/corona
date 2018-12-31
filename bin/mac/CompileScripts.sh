#!/bin/bash

TOOL_DIR=$(defaults read com.anscamobile.CoronaBuilder CoronaBuilderPath 2> /dev/null)
if [ -z "$TOOL_DIR" ]
then
    TOOL_DIR=$(dirname "$0")/CoronaBuilder.app
fi
TOOL_PATH="$TOOL_DIR/Contents/MacOS/CoronaBuilder"
if [ ! -e "$TOOL_PATH" ]
then
    echo "ERROR: Required CoronaBuilder tool not found at ($TOOL_PATH)"
    exit -1
fi

BIN_DIR=$TOOL_DIR/../

# 
# Canonicalize relative paths to absolute paths
# 
pushd "$BIN_DIR" > /dev/null
dir=`pwd`
BIN_DIR="$dir"
popd > /dev/null


if [ -z "$CORONA_ASSETS_DIR" ]
then
	CORONA_ASSETS_DIR=$PROJECT_DIR/assets
fi

if [ -z "$CORONA_TARGET_BUILD_DIR" ]
then
    CORONA_TARGET_BUILD_DIR=$TARGET_BUILD_DIR/$UNLOCALIZED_RESOURCES_FOLDER_PATH
fi

"$BIN_DIR/CopyResources.sh" $CONFIGURATION "$CORONA_ASSETS_DIR" "CORONA_TARGET_BUILD_DIR" "$BIN_DIR"

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

echo "$CONFIGURATION"

echo "EXECUTABLE_FOLDER_PATH $EXECUTABLE_FOLDER_PATH"

CONFIG_META="$BUILT_PRODUCTS_DIR/config.metadata.lua"
PROJECT_NAME=`basename "$PROJECT_DIR"`

if [ -z "$BUNDLE_ID" ]
then
    BUNDLE_ID=$(/usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" "$BUILT_PRODUCTS_DIR/$INFOPLIST_PATH")
fi

# Default iOS
if [ -z "$APP_ID" ]
then
    APP_ID=$(/usr/libexec/PlistBuddy -c "Print :CoronaAppId" "$BUILT_PRODUCTS_DIR/$INFOPLIST_PATH" 2>/dev/null)
fi

# Assign if all else fails
if [[ -z $APP_ID ]]
then
	APP_ID=$(md5 -q -s "$BUNDLE_ID")
fi

echo "Using Bundle Id: ($BUNDLE_ID)"
echo "Using AppId: ($APP_ID)"

MODE="developer"
if [[ $CONFIGURATION =~ .*Release.* ]]
then
	MODE="distribution"
fi

echo "Creating $CONFIG_META..."
cat <<EOF > "$CONFIG_META"
-- Ensure existence of "application" table
if not application or type( application ) ~= "table" then
    application = {}
end

application.metadata = {
    appName = "$PROJECT_NAME",
    appVersion = "1.0",
    appPackageId = "$BUNDLE_ID",
    appId = "$APP_ID",
    userId = "",
    subscription = "enterprise",
    mode = "$MODE",
}
EOF

# Concatenate config.metadata.lua into config.lu overwriting original config.lu
LUAC=$BIN_DIR/luac
CONFIG_DST="$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/config.lu"
CONFIG_SRC="$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/config.orig.lu"
if [ ! -e "$CONFIG_DST" ];
then
	touch "$CONFIG_SRC"
else
	mv "$CONFIG_DST" "$CONFIG_SRC"
fi

echo COMMAND: "$LUAC" -s -o "$CONFIG_DST" "$CONFIG_SRC" "$CONFIG_META"

"$LUAC" -s -o "$CONFIG_DST" "$CONFIG_SRC" "$CONFIG_META"
rm "$CONFIG_SRC"
rm "$CONFIG_META"

# Create resource.car from *.lu
echo Archiving "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/*.lu" into "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/resource.car"
"$TOOL_PATH" car "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/resource.car" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"*.lu
checkError

echo Removing: "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/*.lu"
rm -f "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"*.lu

