#!/bin/bash

path=`dirname "$0"`

SCRIPT_ROOT=$path
SRC_LUA=$1
DST_DIR=$2
BUILD_STYLE=$3
LUA_DIR=$4

#
# Prints usage
# 
usage() {
	echo ""
	echo "USAGE: $0 lua_srcfile dst_dir [DEBUG|RELEASE]"
	exit -1
}

if [ -z "$SRC_LUA" ]
then
	usage
fi

if [ -z "$DST_DIR" ]
then
	DST_DIR=`dirname $SRC_LUA`
fi

if [ -z "$BUILD_STYLE" ]
then
	BUILD_STYLE=RELEASE
fi

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

# 
# Canonicalize relative paths to absolute paths
# 
pushd "$SCRIPT_ROOT" > /dev/null
dir=`pwd`
SCRIPT_ROOT=$dir
popd > /dev/null

pushd "$DST_DIR" > /dev/null
dir=`pwd`
DST_DIR=$dir
popd > /dev/null

if [ -z "$LUA_DIR" ]
then
	LUA_DIR=$SCRIPT_ROOT
	echo "WARNING: No LUA_DIR specified. Looking for it here: $LUA_DIR"
fi

TARGET_TEMP_DIR=$DST_DIR
INPUT_FILE_PATH=$SRC_LUA
INPUT_FILE_BASE=`basename -s ".lua" "$SRC_LUA"`

echo "Compiling: $SRC_LUA => $TARGET_TEMP_DIR/$INPUT_FILE_BASE.c"
if [ -f "$SCRIPT_ROOT"/../rcc.lua ]
then
	# rttmain repo tool path
	echo $SCRIPT_ROOT
	"$LUA_DIR"/lua "$SCRIPT_ROOT"/../rcc.lua -c "$LUA_DIR" -O$BUILD_STYLE -o "$TARGET_TEMP_DIR"/"$INPUT_FILE_BASE".lu "$INPUT_FILE_PATH"
	checkError

	#"$LUA_DIR"/lua -epackage.path=[[$SCRIPT_ROOT/../../external/loop-2.3-beta/lua/?.lua]] "$SCRIPT_ROOT"/../../external/loop-2.3-beta/lua/precompiler.constant.lua -d "$TARGET_TEMP_DIR" -o "$INPUT_FILE_BASE" -l "$TARGET_TEMP_DIR"/?.lu -n "$INPUT_FILE_BASE"
	"$LUA_DIR"/lua "$SCRIPT_ROOT"/../lua2c.lua "$TARGET_TEMP_DIR"/"$INPUT_FILE_BASE".lu "$INPUT_FILE_BASE" "$TARGET_TEMP_DIR"/"$INPUT_FILE_BASE".c
	checkError
else
	# Enterprise tool path
	LUA_SCRIPT_ROOT=$SCRIPT_ROOT/../../shared/bin

	"$LUA_DIR"/lua "$LUA_SCRIPT_ROOT"/lua2c.lua "$INPUT_FILE_PATH" "$INPUT_FILE_BASE" "$TARGET_TEMP_DIR"/"$INPUT_FILE_BASE".c
	checkError
fi
