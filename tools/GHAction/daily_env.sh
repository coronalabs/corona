#!/usr/bin/env bash
set -e

: "${YEAR:=$(date +"%Y")}"
if [[ "$GITHUB_REF" == refs/tags/* ]]
then
    : "${BUILD_NUMBER:=${GITHUB_REF#refs/tags/}}"
else
    : "${BUILD_NUMBER:=$GITHUB_RUN_NUMBER}"
fi

{
    echo "BUILD_NUMBER=$BUILD_NUMBER"
    echo "BUILD=$BUILD_NUMBER"
    echo "YEAR=$YEAR"
    echo "MONTH=$(date +"%-m")"
    echo "DAY=$(date +"%-d")"
 } >> "$GITHUB_ENV"
