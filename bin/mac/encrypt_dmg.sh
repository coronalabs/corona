#!/bin/bash

# ./build_sdk.sh path/to/src.dmg path/to/dst.dmg password

path=`dirname $0`

if [ -z "$1" ] || [ -z "$2" ]
then
	echo "USAGE: $0 src.dmg dst.dmg password"
	exit 0
fi

if [ -n "$3" ]
then
	printf "$3" | hdiutil convert "$1" -format UDCO -encryption -stdinpass -o "$2"
else
	hdiutil convert "$1" -format UDCO -encryption -o "$2"
fi
