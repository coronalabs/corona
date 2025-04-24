#!/bin/bash

PLATFORM=""

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
    case "$1" in
        --tvOS)
            PLATFORM="tvOS"
            ;;
        --macOS)
            PLATFORM="OSX"
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

if [[ "$PLATFORM" == "OSX" ]]; then
    STRIPPED_PATH="${EXECUTABLE_FOLDER_PATH//Contents\/MacOS}"
    DEST_PATH="$BUILT_PRODUCTS_DIR/$STRIPPED_PATH"
    echo "$BIN_DIR/lua" "$BIN_DIR/buildSettingsToPlist.lua" "$ASSETS_DIR" "$DEST_PATH" "$PLATFORM"
    "$BIN_DIR/lua" "$BIN_DIR/buildSettingsToPlist.lua" "$ASSETS_DIR" "$DEST_PATH" "$PLATFORM"

elif [[ -n "$PLATFORM" ]]; then
    DEST_PATH="$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"
    echo "$BIN_DIR/lua" "$BIN_DIR/buildSettingsToPlist.lua" "$ASSETS_DIR" "$DEST_PATH" "$PLATFORM"
    "$BIN_DIR/lua" "$BIN_DIR/buildSettingsToPlist.lua" "$ASSETS_DIR" "$DEST_PATH" "$PLATFORM"

else
    DEST_PATH="$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"
    echo "$BIN_DIR/lua" "$BIN_DIR/buildSettingsToPlist.lua" "$ASSETS_DIR" "$DEST_PATH"
    "$BIN_DIR/lua" "$BIN_DIR/buildSettingsToPlist.lua" "$ASSETS_DIR" "$DEST_PATH"
fi
