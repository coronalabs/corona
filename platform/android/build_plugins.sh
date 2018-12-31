#!/bin/bash

path=`dirname $0`

# Canonicalize paths
pushd $path > /dev/null
dir=`pwd`
path=$dir
popd > /dev/null

# This script is at a fixed location relative to the corona repo root
CORONA_ROOT=$path/../..	

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

build()
{
	PLUGIN_NAME=$1
	PLUGIN_PATH=${CORONA_ROOT}/plugins/${PLUGIN_NAME}/android
	pushd ${PLUGIN_PATH} > /dev/null
		# Delete intermediate files if a clean build was requested.
		if [ "clean" = "$BUILD_TYPE" ]
		then
			echo "CLEANING java files..."
			ant clean; checkError
		fi

		ant release
	popd > /dev/null
}

build ads-inmobi
build ads-inneractive