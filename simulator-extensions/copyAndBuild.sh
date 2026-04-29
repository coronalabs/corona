#!/bin/bash

# $1 is the src folder

shopt -s extglob

if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]
then
    echo "USAGE: $0 src_dir dst_dir luac_dir"
    exit 0
fi

SRC_DIR="$1"
DST_DIR="$2"
LUAC_DIR="$3"

# Canonicalize paths
pushd "$SRC_DIR" > /dev/null
dir=$(pwd)
SRC_DIR=$dir
popd > /dev/null

if [ ! -d "$DST_DIR" ]
then
	mkdir -p "$DST_DIR"
fi
pushd "$DST_DIR" > /dev/null
dir=$(pwd)
DST_DIR=$dir
popd > /dev/null

pushd "$LUAC_DIR" > /dev/null
dir=$(pwd)
LUAC_DIR=$dir
popd > /dev/null

if [ "$CONFIGURATION" = "Debug" ]
then
	OPTIONS=
else
	OPTIONS="-s"
fi

echo "Sourcing files from $SRC_DIR"
echo "   and writing out files to $DST_DIR"
echo "   using luac compiler in $LUAC_DIR"
echo "   with configuration $CONFIGURATION"

# First copy the directory tree
echo "Copying $SRC_DIR/*  --> $DST_DIR/"
# This command assumes $DST_DIR exists and doesn't copy dotfiles (at the top level)
cp -r "$SRC_DIR"/* "$DST_DIR/"

# Compile Lua sources in top level directory

ls "$DST_DIR"/*.lua | while read FILE
do
	echo "Compiling $FILE...";
	"$LUAC_DIR/luac" $OPTIONS -o "$FILE" "$FILE"
done
