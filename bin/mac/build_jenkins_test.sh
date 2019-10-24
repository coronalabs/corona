#!/bin/bash

path=`dirname $0`

function cleanup()
{
	( # reverting version files
		cd "$WORKSPACE"
		hg revert --no-backup platform/resources/linuxtemplate.tar.gz platform/resources/raspbiantemplate.tar.gz platform/resources/webtemplate.zip
		hg revert --no-backup platform/mac/Info.plist
		hg revert --no-backup librtt/Core/Rtt_Version.h 
	) &>/dev/null
	exit
}


# Test parameters
export CHANGESET=tip
export MONTH=`date "+%m"`
export DAY=`date "+%d"`
export DOCS_CHANGESET=tip
export REPO=ssh://hg@bitbucket.org/coronalabs/main
export DOCS_REPO=ssh://hg@bitbucket.com/coronalabs/api-tachyon
export Release=daily
#export Release=release
export SAMPLE_APPS_URL="http://192.168.192.25/docs-tachyon/archive/tip.tar.gz"
export CUSTOM_ID=50000
export S3_BUCKET=sandbox-templates-tachyon

# Workspace path relative to script location
WORKSPACE=$path/../..

# 
# Canonicalize relative paths to absolute paths
# 
pushd $WORKSPACE > /dev/null
dir=`pwd`
WORKSPACE=$dir
export BUILD_NUMBER=$(find .. -name 'CoronaEnterprise-*.tgz' -maxdepth 1 | sort  | tail -n1 | cut -d '.' -f 4)
export YEAR=$(find .. -name 'CoronaEnterprise-*.tgz' -maxdepth 1 | sort  | tail -n1 | cut -d '.' -f 3 | cut -d '-' -f 2)
popd > /dev/null

export WORKSPACE

if [ -z "$BUILD_NUMBER" -o -z "$YEAR" ]; then
	echo 'Missing ../CoronaEnterprise-YYYY.BBBBB.tgz'
	cleanup
fi

if [ ! -f "$WORKSPACE/docs.zip" ]; then
	echo 'Missing docs.zip file'
	cleanup
fi


"$path"/build_jenkins.sh

cleanup
