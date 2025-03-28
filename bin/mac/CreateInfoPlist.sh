#!/bin/bash

PLATFORM=""

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
    case "$1" in
        --tvOS)
            PLATFORM="tvOS"
            ;;
        *)
            echo "Unknown argument: $1"
            exit 1
            ;;
    esac
    shift
done

BIN_DIR=$(dirname "$0")
ASSETS_DIR="$PROJECT_DIR"/../Corona

if [[ -n "$PLATFORM" ]]; then
    echo "$BIN_DIR/lua" "$BIN_DIR"/buildSettingsToPlist.lua "$ASSETS_DIR" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/" "$PLATFORM"
    "$BIN_DIR/lua" "$BIN_DIR"/buildSettingsToPlist.lua "$ASSETS_DIR" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/" "$PLATFORM"
else
    echo "$BIN_DIR/lua" "$BIN_DIR"/buildSettingsToPlist.lua "$ASSETS_DIR" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"
    "$BIN_DIR/lua" "$BIN_DIR"/buildSettingsToPlist.lua "$ASSETS_DIR" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"
fi
