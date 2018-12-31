#!/bin/bash

BIN_DIR=$(dirname "$0")
ASSETS_DIR="$PROJECT_DIR"/../Corona

echo "$BIN_DIR/lua" "$BIN_DIR"/buildSettingsToPlist.lua "$ASSETS_DIR" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"
"$BIN_DIR/lua" "$BIN_DIR"/buildSettingsToPlist.lua "$ASSETS_DIR" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"
