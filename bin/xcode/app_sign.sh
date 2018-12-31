#!/bin/bash
set -e

# Xcode Build Phase

# Script: app_sign
# -----------------------------------------------------------------------------

# Process args

BINARY_PATH=$1

if [ -z "${BINARY_PATH}" ]
then
    BINARY_PATH=$BUILT_PRODUCTS_DIR/$EXECUTABLE_PATH
fi

# app_sign 1 (or more) slices

TOOLCHAIN_DIR="$PROJECT_DIR/../../bin/mac"

if [ $SUPPRESS_APP_SIGN ]
then
    echo "Skipping app_sign for template builds for $CONFIGURATION"
else
    echo "Running app_sign for $CONFIGURATION"

    echo "$TOOLCHAIN_DIR/app_sign" sign "$PROJECT_DIR/../resources/developerkey.cert" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/resource.car" "${BINARY_PATH}" little
    "$TOOLCHAIN_DIR/app_sign" sign "$PROJECT_DIR/../resources/developerkey.cert" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/resource.car" "${BINARY_PATH}" little

    # If more than one architecture, repeat app_sign for each *additional* architecture (hence $count > 1)
    count=`echo $ARCHS | wc -w | tr -d ' '`
    while (( $count > 1 ))
    do
        "$TOOLCHAIN_DIR/app_sign" sign "$PROJECT_DIR/../resources/developerkey.cert" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/resource.car" "${BINARY_PATH}" little
        ((count--))
    done

    code=$?
    if [ "$code" -ne "0" ]
    then
        echo "*** app_sign cannot resign ${BINARY_PATH}. Please remove and then build. ***"
        exit $code 
    fi
fi