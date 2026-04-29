#!/bin/bash

# $1 is the build config
# $2 is the src folder
# $3 is the dst folder
# $4 explicit directory where luac is. If not set, will use `dirname $0` 

shopt -s extglob

BUILD_CONFIG="$1"
SRC_DIR="$2"
DST_DIR="$3"

# Initialize luac directory
# NOTE: luac must be in same dir as this file
LUAC_DIR=`dirname $0`
if [[ $4 != "" ]]; then
	# Take the user's luac directory
	LUAC_DIR=$4
fi


if [[ $BUILD_CONFIG == @(Debug|DEBUG|debug)* ]]; then
	OPTIONS=
else
	OPTIONS=-s
fi


SRC_DIR_ESCAPED=$(echo "$SRC_DIR" | sed -E 's/[$\/\.*()+?^]/\\&/g')

# Recursively go through each file to compile Lua sources
find -H "$SRC_DIR" -name '*.lua' -print | while read file;
do
	# Create paths relative to $SRC_DIR
	filebase=$(echo "$file" | sed -E "s/$SRC_DIR_ESCAPED\/(.*)/\1/")

	# Convert directory separator '/' to '.' in $filebase (and ".lua" to ".lu")
	filebase=$(echo "$filebase" | sed -E 's/.lua$/.lu/;s/\//\./g')

	echo "Compiling $file ===> $filebase";
	$LUAC_DIR/luac $OPTIONS -o "$DST_DIR"/`basename "$filebase"` "$file"
done
