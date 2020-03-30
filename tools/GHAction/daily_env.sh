#!/usr/bin/env bash
set -e

BUILD_NUMBER=$GITHUB_RUN_NUMBER
YEAR="$(date +"%Y")"
if [[ "$GITHUB_REF" == refs/tags/* ]]
then
    BUILD_NUMBER="${GITHUB_REF#refs/tags/}"
else
    echo ERROR: Tag was not pushed
    exit 1
fi

echo ::set-env name=BUILD_NUMBER::"$BUILD_NUMBER"
echo ::set-env name=BUILD::"$BUILD_NUMBER"
echo ::set-env name=YEAR::"$YEAR"
echo ::set-env name=MONTH::"$(date +"%-m")"
echo ::set-env name=DAY::"$(date +"%-d")"
