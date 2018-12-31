#!/bin/bash

path=$(dirname "$0")

OUTPUT_DIR=$path
LIBRARIES_DIR=$OUTPUT_DIR/CoronaSampleApp/Libraries
CORONA_DIR=$LIBRARIES_DIR/Corona

cd "$path" || exit 1

# Create cert
CERT_FILE="$CORONA_DIR"/bin/developerkey.cert
"$CORONA_DIR"/bin/app_sign create "$CERT_FILE"

zip -u CoronaSampleApp.zip "$CERT_FILE"

# clean up
rm "$CERT_FILE"
cd - || exit 1
