#!/bin/bash

# Location of toolchain
TOOLCHAIN_DIR="$PROJECT_DIR/../../bin/mac"

ASSETS_DIR="$PROJECT_DIR"/../test/assets
if [ -d "$PROJECT_DIR"/../test/assets2 ]
then
    ASSETS_DIR="$PROJECT_DIR"/../test/assets2
	echo "$(basename "$0"): Using '$ASSETS_DIR' ..."
fi

echo "$(basename "$0"): Running \"$TOOLCHAIN_DIR\"/lua \"$PROJECT_DIR\"/../../bin/mac/buildSettingsToPlist.lua $ASSETS_DIR \"$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/\""
"$TOOLCHAIN_DIR"/lua "$TOOLCHAIN_DIR"/buildSettingsToPlist.lua "$ASSETS_DIR" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"
