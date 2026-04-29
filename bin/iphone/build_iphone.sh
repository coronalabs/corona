#!/bin/bash

# The first argument to this script should be the directory, e.g.
# The second arg is the name of the app
# The third arg is the version number
# The fourth arg is optional (path to a provision file)
# If a fourth arg is specified, then there needs to be a fifth arg specifying the 
# signing identity that corresponds to the provision file
# NOTE: The bundle and app id are extracted from the provision file
# 
# ./build_mac.sh fishies fishies [path/to/provision/file]

path=`dirname $0`

# If a name was specified
if test "$2"
then
	appName="$2"
else
	appName="ratatouille"
fi

version="$3"

# If a provision file was specified, ensure it exists
# and ensure a fifth identity arg was specified
if test "$4"
then
	if test -e "$4"
	then
		if test "$5"
		then
			provisionFile=$4
			identity=$5
		else
			echo "ERROR: Provision file specified but no corresponding identity was specified"
			exit 0
		fi
	fi
fi

# Default to user's AdHoc provision if none specified
if test ! "$provisionFile"
then
	filePath="$HOME/Library/MobileDevice/Provisioning Profiles/AdHoc.mobileprovision"
	echo "$filePath"

	if test -e "$filePath"
	then
		provisionFile="$filePath"
		identity="iPhone Distribution: ansca, inc"
	else
		echo "ERROR: Could not find a valid mobile provision file"
		exit 0
	fi
fi

# $path/lua $path/PackageApp.lua -a $path -d 64X8VS64A3.com.anscamobile.$2 /Users/wluh/Library/MobileDevice/Provisioning\ Profiles/AdHoc.mobileprovision -i $1/Icon.png $1/assets $1
$path/lua $path/PackageApp.lua -a "$path" -s "$identity" -v $version -o "$appName" "$provisionFile" -i "$1/Icon.png" "$1/assets" "$1"
