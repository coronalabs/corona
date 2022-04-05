#!/bin/bash

# See https://sites.google.com/a/anscamobile.com/dev/home/corona-specs/init-lua

path=$(dirname "$0")

# Save optional module name
if [ "$1" == "-m" ]
then
	MODULE_NAME="$2"
	shift 2
else
	MODULE_NAME=""
fi

if [ -z "$1" -o -z "$2" ]
then
        echo "USAGE: $0 [-m modulename] file.lua dstDir"
        exit 0
fi

SRC_DIR=$(dirname "$1")
SRCFILE=$(basename "$1")

# Remove extension
SRCNAME=$(basename -s .lua "$SRCFILE")

DST_DIR="$2"

case "$BUILD_CONFIG" in
	RELEASE)
		LUA_BUILD_TYPE=RELEASE
		;;
	*)
		LUA_BUILD_TYPE=DEBUG
		;;
esac

# Canonicalize paths
pushd "$path" > /dev/null
dir=$(pwd)
path=$dir
popd > /dev/null

pushd "$SRC_DIR" > /dev/null
dir=$(pwd)
SRC_DIR=$dir
popd > /dev/null

pushd "$DST_DIR" > /dev/null
dir=$(pwd)
DST_DIR=$dir
popd > /dev/null

SRC_PATH="$SRC_DIR/$SRCFILE"

#if [[ $(uname -s) == Darwin ]];then
#    BIN_DIR=mac
#else 
#    BIN_DIR=linux
#fi
BIN_DIR="$3"

# If the Lua is newer than the CPP, compile it
if [ "$SRC_PATH" -nt "$DST_DIR/$SRCNAME.cpp" ]
then
	TEMP_DIR=/tmp/$$
	mkdir $TEMP_DIR

	echo "Compiling $SRC_PATH to $DST_DIR/$SRCNAME.cpp"
	pushd "$path/../../bin" > /dev/null

	LU_PATH=$TEMP_DIR/$SRCNAME.lu

	${BIN_DIR}/lua rcc.lua -c ${BIN_DIR} -O$LUA_BUILD_TYPE -o "$LU_PATH" "$SRC_PATH"
	${BIN_DIR}/lua -epackage.path="[[../external/loop-2.3-beta/lua/?.lua]]" ../external/loop-2.3-beta/lua/precompiler.constant.lua -d "$DST_DIR" -o "$SRCNAME" -l "$LU_PATH" -n -m "$MODULE_NAME" "$SRCNAME"

	popd > /dev/null

	rm -r "$TEMP_DIR"
fi
