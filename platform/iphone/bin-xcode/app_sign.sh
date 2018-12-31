#!/bin/bash

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

# Location of toolchain
TOOLCHAIN_DIR="$PROJECT_DIR/../../bin/mac"

if [[ $CONFIGURATION =~ .*template.* ]]
then
	echo "Skipping app_sign for template builds for $CONFIGURATION"
else
	echo "Running app_sign for $CONFIGURATION"

	"$TOOLCHAIN_DIR/app_sign" sign "$PROJECT_DIR/../resources/developerkey.cert" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/resource.car" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_PATH" little
    checkError

	# If more than one architecture, repeat app_sign for each *additional* architecture (hence $count > 1)
	count=$(echo "$ARCHS" | wc -w | tr -d ' ')
	while (( "$count" > 1 ))
	do
		"$TOOLCHAIN_DIR/app_sign" sign "$PROJECT_DIR/../resources/developerkey.cert" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/resource.car" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_PATH" little
		((count--))
	done

	code=$?
	if [ "$code" -ne "0" ]
	then
		echo "*** app_sign cannot resign $BUILT_PRODUCTS_DIR/$EXECUTABLE_PATH. Please remove and then build. ***"
		exit $code 
	fi
fi
