#!/bin/bash
set -ex

# librtt
# 
# Xcode Build Rule: *.lua
# -----------------------------------------------------------------------------

# Location of toolchain
TOOLCHAIN_DIR="$PROJECT_DIR/../../bin/mac"

echo "Using lua from $TOOLCHAIN_DIR ..."

if [ ! -e "$TOOLCHAIN_DIR/lua" ]; then
    echo ERROR: "$TOOLCHAIN_DIR"/lua does not exist.
    exit -1
fi

if [ "$1" == "-m" ]
then
	MODULE_NAME="$2"
else
	MODULE_NAME=""
fi

DST_DIR="${TARGET_TEMP_DIR}/${CURRENT_ARCH}"
if [ ! -d "$DST_DIR" ]; then		
    mkdir "$DST_DIR"
fi

"$TOOLCHAIN_DIR"/lua $SRCROOT/../../bin/rcc.lua -c "$TOOLCHAIN_DIR" -O$CONFIGURATION -g -o $DST_DIR/$INPUT_FILE_BASE.lu $INPUT_FILE_PATH
"$TOOLCHAIN_DIR"/lua -epackage.path=[[$SRCROOT/../../external/loop-2.3-beta/lua/?.lua]] $SRCROOT/../../external/loop-2.3-beta/lua/precompiler.constant.lua -d $DST_DIR -o $INPUT_FILE_BASE -l $DST_DIR/?.lu -n -m "$MODULE_NAME" $INPUT_FILE_BASE
