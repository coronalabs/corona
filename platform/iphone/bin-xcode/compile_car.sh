#!/bin/bash

# If a folder called assets2 exists, use that instead of assets because assets2 wasn't put accidentally under hg revision control.
echo "$PROJECT_DIR"/../test/assets2/main.lua 
if [ -f "$PROJECT_DIR"/../test/assets2/main.lua ]; then
	"$PROJECT_DIR/../../bin/mac/CopyResources.sh" "$CONFIGURATION" "$PROJECT_DIR"/../test/assets2 "$TARGET_BUILD_DIR"/"$UNLOCALIZED_RESOURCES_FOLDER_PATH" "$BUILT_PRODUCTS_DIR"
else
	"$PROJECT_DIR/../../bin/mac/CopyResources.sh" "$CONFIGURATION" "$PROJECT_DIR"/../test/assets "$TARGET_BUILD_DIR"/"$UNLOCALIZED_RESOURCES_FOLDER_PATH" "$BUILT_PRODUCTS_DIR"
fi

# Location of toolchain
TOOLCHAIN_DIR="$PROJECT_DIR/../../bin/mac"

echo "$CONFIGURATION"

if [[ $CONFIGURATION =~ .*template.* ]]
then
echo "Skipping"

	echo "Skipping creation of resource.corona-archive for template builds"
else
    echo "EXECUTABLE_FOLDER_PATH $EXECUTABLE_FOLDER_PATH"

    CONFIG_META="$BUILT_PRODUCTS_DIR/config.metadata.lua"
    PROJECT_NAME=$(basename "$PROJECT_DIR")
echo "Creating $CONFIG_META..."
cat <<EOF > "$CONFIG_META"
-- Ensure existence of "application" table
if not application or type( application ) ~= "table" then
    application = {}
end

application.metadata = {
    appName = "$PROJECT_NAME",
    appVersion = "1.0",
    appPackageId = "com.anscamobile.$PROJECT_NAME",
    appId = "0123456789",
    userId = "345", -- drupal user id
    subscription = "pro", -- either "trial", "pro", "indie-ios", "indie-android"
    mode = "developer", -- either "developer" or nil (where nil means distribution/store)
}
EOF

    # Concatenate config.metadata.lua into config.lu overwriting original config.lu
    LUAC=$TOOLCHAIN_DIR/luac
    CONFIG_DST="$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/config.lu"
    CONFIG_SRC="$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/config.orig.lu"
    if [ ! -e "$CONFIG_DST" ];
    then
        touch "$CONFIG_SRC"
    else
        mv "$CONFIG_DST" "$CONFIG_SRC"
    fi
   
    "$LUAC" -s -o "$CONFIG_DST" "$CONFIG_SRC" "$CONFIG_META"
    rm "$CONFIG_SRC"
    rm "$CONFIG_META"

    # Create resource.corona-archive from *.lu
	echo Archiving "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/*.lu" into "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/resource.corona-archive"
	"$TOOLCHAIN_DIR/car" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/resource.corona-archive" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"*.lu

	echo Removing: "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/*.lu"
	rm -f "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"*.lu
fi
