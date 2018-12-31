#!/bin/bash

path=`dirname $0`

# -----------------------------------------------------------------------------

# Test parameters
export BUILD_NUMBER=55
export CHANGESET=tip
export YEAR=`date -j -f "%a %b %d %T %Z %Y" "\`date\`" "+%Y"`
export MONTH=`date -j -f "%a %b %d %T %Z %Y" "\`date\`" "+%m" | sed  's/^0//'`
export DAY=`date -j -f "%a %b %d %T %Z %Y" "\`date\`" "+%d" | sed  's/^0//'`
export DOCS_CHANGESET=tip
export REPO=ssh://hg@192.168.192.25/clones/walter-graphics/
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
popd > /dev/null

export WORKSPACE=$WORKSPACE

# -----------------------------------------------------------------------------

"$path"/build_jenkins.sh
