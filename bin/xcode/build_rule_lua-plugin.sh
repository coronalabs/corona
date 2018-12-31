#!/bin/bash
set -e

# Plugins
# 
# Xcode Build Rule: *.lua
# -----------------------------------------------------------------------------

# Location of toolchain
if [ -z "$TOOLCHAIN_DIR" ]
then
	TOOLCHAIN_DIR="$PROJECT_DIR/../../bin/mac"
fi

echo "Using lua from $TOOLCHAIN_DIR ..."

if [ ! -e "$TOOLCHAIN_DIR/lua" ]; then
    echo ERROR: "$TOOLCHAIN_DIR"/lua does not exist.
    exit -1
fi

DST_DIR=$DERIVED_FILE_DIR
if [ ! -d "$DST_DIR" ]; then		
    mkdir "$DST_DIR"
fi

# "$TOOLCHAIN_DIR"/lua $SRCROOT/../../bin/rcc.lua -c "$TOOLCHAIN_DIR" -O$CONFIGURATION -o $DST_DIR/$INPUT_FILE_BASE.lu $INPUT_FILE_PATH
# "$TOOLCHAIN_DIR"/lua -epackage.path=[[$SRCROOT/../../external/loop-2.3-beta/lua/?.lua]] $SRCROOT/../../external/loop-2.3-beta/lua/precompiler.constant.lua -d $DST_DIR -o $INPUT_FILE_BASE -l $DST_DIR/?.lu -n $INPUT_FILE_BASE

"$TOOLCHAIN_DIR"/lua "$TOOLCHAIN_DIR"/../lua2c.lua "$INPUT_FILE_PATH" "$INPUT_FILE_BASE" "$DST_DIR"/"$INPUT_FILE_BASE".c
