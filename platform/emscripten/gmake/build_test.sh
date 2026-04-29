#!/bin/sh

path=$(dirname "$0")

CORONA_PROJECT_DIR="$path"/../../test/assets2
OUTPUT_HTML=build/corona_test.html

"$path"/build_app.sh "$CORONA_PROJECT_DIR" "$OUTPUT_HTML" "$1"
