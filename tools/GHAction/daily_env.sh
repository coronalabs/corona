#!/usr/bin/env bash
set -e

: "${YEAR:=$(date +"%Y")}"
if [[ "$GITHUB_REF" == refs/tags/* ]]
then
    : "${BUILD_NUMBER:=${GITHUB_REF#refs/tags/}}"
else
    : "${BUILD_NUMBER:=$GITHUB_RUN_NUMBER}"
fi

echo ::set-env name=BUILD_NUMBER::"$BUILD_NUMBER"
echo ::set-env name=BUILD::"$BUILD_NUMBER"
echo ::set-env name=YEAR::"$YEAR"
echo ::set-env name=MONTH::"$(date +"%-m")"
echo ::set-env name=DAY::"$(date +"%-d")"
