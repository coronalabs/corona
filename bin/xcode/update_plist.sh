#!/bin/bash
set -e

# Xcode Build Phase

# Script: build.settings to Info.plist
# -----------------------------------------------------------------------------

TOOLCHAIN_DIR="$PROJECT_DIR/../../bin/mac"
LUA="$TOOLCHAIN_DIR/lua"

ASSETS_DIR="$PROJECT_DIR"/../test/assets
if [ -d "$PROJECT_DIR"/../test/assets2 ]
then
    ASSETS_DIR="$PROJECT_DIR"/../test/assets2
fi

echo Running "$LUA" "$PROJECT_DIR"/../../bin/mac/buildSettingsToPlist.lua $ASSETS_DIR "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"
LUA_PATH="$PROJECT_DIR/../../platform/resources/?.lua" export LUA_PATH
"$LUA" "$PROJECT_DIR"/../../bin/mac/buildSettingsToPlist.lua $ASSETS_DIR "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"
