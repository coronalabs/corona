#!/bin/bash

# $1 is the build config (if it ends in template, all .lu files are removed from dst dir)
# $2 is the src folder
# $3 is the dst folder
# $4 if "--preserve" preserves png's; default is to compress png's using pngcrush, or see $5
# $5 explicit directory where luac is. If not set, will use `dirname $0` 
shopt -s extglob

BUILD_CONFIG=$1
SRC_DIR="$2"
DST_DIR="$3"

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

# Initialize luac directory
# NOTE: luac must be in same dir as this file
LUAC_DIR=`dirname "$0"`
if [[ $4 == "--preserve" ]]; then
	CORONA_COPY_PNG_PRESERVE="--preserve"
	if [[ $5 != "" ]]; then
		# Take the user's luac directory
		LUAC_DIR=$5
	fi
elif [[ $4 != "" ]]; then
	# Take the user's luac directory
	LUAC_DIR=$4
fi

if [[ $BUILD_CONFIG == *-template* ]]; then
	for file in $DST_DIR/*.lu
	do
		filebase=`basename "$file"`
		case "$filebase" in
			'main.lu') echo "Removing $file"; rm $file ;;
		esac
	done
else
	DEVELOPER_BASE=$(xcode-select -print-path)
	PATH="$DEVELOPER_BASE/Platforms/iPhoneOS.platform/Developer/usr/bin:/Developer/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin"
	export PATH

	COPYPNG=$(xcrun --sdk iphoneos --find copypng)
	if [ ! -x "$COPYPNG" ]
	then
		echo "ERROR: cannot locate 'copypng' in Xcode"
		exit 1
	fi

	# This part injects resources from plugins and additional directories specified by CORONA_PLUGIN_RESOURCES
	# magic inspired by https://stackoverflow.com/a/15988793/236378
	SRC_DIRS="$SRC_DIR:$CORONA_PLUGIN_RESOURCES"
	SRC_DIR_OLD=$SRC_DIR
	while [ "$SRC_DIRS" ] ;do
		iter=${SRC_DIRS%%:*}
		SRC_DIR=$iter


		SRC_DIR_ESCAPED=$(echo "$SRC_DIR" | sed -E 's/[$\/\.*()+?^]/\\&/g')

		# for file in "$SRC_DIR"/*
		# 
		# Exclude hidden directories (like .hg or .git)
		# Usually -prune is used with -o because it returns true, 
		# so when combined with short-circuit OR, it has the effect of skipping 
		# the -print on hidden files / directories, which is exactly what you want.
		find -H "$SRC_DIR" -name '.*' -prune -o -print | while read file;
		do
			if [ "$file" != "$SRC_DIR" ]
			then
				# Create paths relative to $SRC_DIR
				filebase=$(echo "$file" | sed -E "s/$SRC_DIR_ESCAPED\/(.*)/\1/")

				if ( [ -d "$file" ] )
				then
					mkdir -p "$DST_DIR/$filebase"
				else
					case "$filebase" in
						*.lua)
							if [[ $BUILD_CONFIG == @(Debug|DEBUG|debug)* ]]; then
								OPTIONS=
							else
								OPTIONS=-s
							fi

							# Convert directory separator '/' to '.' in $filebase
							filebase=$(echo "$filebase" | sed -E "s/\//\./g")

							file_bytecode=`basename -s lua "$filebase"`lu
							echo "Compiling $file ===> $file_bytecode";
							# Run luac stripping off the first element of any error messages
							# (which is the path to luac and which prevents errors from being
							# clickable in Xcode)
							"$LUAC_DIR/luac" $OPTIONS -o "$DST_DIR"/"$file_bytecode" "$file" 2> >(sed -e 's/^[^:]*: //' 1>&2)
							checkError
							;;
						build.settings) ;; #echo "Ignoring $file";;
						*.png)
							if [[ $CORONA_COPY_PNG_PRESERVE == "--preserve" ]]; then
								echo "Copying $file to $DST_DIR/$filebase"; 

								# Copy file to dst
								cp -v "$file" "$DST_DIR/$filebase"
								checkError
							else
								echo "Compressing/copying PNG $file to $DST_DIR/$filebase"

								# Compress PNG and copy to dst
								"$COPYPNG" -compress "$file" "$DST_DIR/$filebase"
								checkError
							fi
							;;
						*)
							echo "Copying $file to $DST_DIR/$filebase"

							# Copy file to dst
							cp -v "$file" "$DST_DIR/$filebase"
							checkError
							;;
					esac
				fi
			fi
		done

		# The loop above is a subshell so we need to check its exit status to detect errors
		# (particularly luac errors when compiling a project's Lua code)
		if [ $? != 0 ]
		then
			exit -1
		fi

		# see stack overflow link above for details. This, basically, does SRC_DIRS = tail(SRC_DIRS)
		[ "$SRC_DIRS" = "$iter" ] && \
		SRC_DIRS='' || \
		SRC_DIRS="${SRC_DIRS#*:}"
	done
	SRC_DIR=$SRC_DIR_OLD
fi
