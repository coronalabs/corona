#!/bin/bash
set -e

# Xcode Build Phase

# Script: CopyResources and resource.corona-archive
# -----------------------------------------------------------------------------

# Location of toolchain
TOOLCHAIN_DIR="$PROJECT_DIR/../../bin/mac"
LUAC="$TOOLCHAIN_DIR/luac"
CAR="$TOOLCHAIN_DIR/car"

# If a folder called assets2 exists, use that instead of assets because assets2 wasn't put accidentally under hg revision control.
echo "$PROJECT_DIR"/../test/assets2/main.lua 
if [ -f "$PROJECT_DIR"/../test/assets2/main.lua ]; then
    echo "$CONFIGURATION: Building resource.corona-archive"

    $TOOLCHAIN_DIR/CopyResources.sh $CONFIGURATION "$PROJECT_DIR"/../test/assets2 "$TARGET_BUILD_DIR"/"$UNLOCALIZED_RESOURCES_FOLDER_PATH" "$LUA_DIR"

    echo "EXECUTABLE_FOLDER_PATH $EXECUTABLE_FOLDER_PATH"

    CONFIG_META="$BUILT_PRODUCTS_DIR/config.metadata.lua"
    PROJECT_NAME=`basename "$PROJECT_DIR"`
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
    build = "42", -- either "developer" or nil (where nil means distribution/store)
}

-- application.launchPad = false -- not set is the same as true

EOF

    echo "********* user($USER_ID) sub($SUB_TYPE) mode($MODE_TYPE)"
    CONFIG_REQUIRE="$PROJECT_DIR"/../resources/config_require.lua

    # Write out new config.lu into input dir
    # 
    # Our library restriction code relies on metadata and hijacking of 'require'
    # This code is implemented by these 2 blocks: ("$CONFIG_META" "$CONFIG_REQUIRE").
    # 
    # IMPORTANT: We must _prepend_ this library restriction code at the BEGINNING!
    # That is, it must execute BEFORE the original config.lua ($CONFIG_SRC).
    # If we put it after the original config.lua, then the developer could bypass
    # our restriction code via a 'return' at the end of the original config.lua.
    #
    # NOTE: Historically, we just appended the original with "$CONFIG_META". We
    # continue to append $CONFIG_META at the end even though we also prepend at the
    # beginning. This is because in the pre-library-restriction world, we always
    # appended this, so to avoid any unforseen backward compatibility issues, we 
    # continue to append it.
    # 

    # Concatenate config.metadata.lua into config.lu overwriting original config.lu
    CONFIG_DST="$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/config.lu"
    CONFIG_SRC="$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/config.orig.lu"
    if [ ! -e "$CONFIG_DST" ];
    then
        touch "$CONFIG_SRC"
    else
        mv "$CONFIG_DST" "$CONFIG_SRC"
    fi
   
    if [ -e "$CONFIG_REQUIRE" ]
    then
        # Enforce library restriction
        echo "********** Enforce Library Restriction *******"
        "$LUAC" -s -o "$CONFIG_DST" "$CONFIG_META" "$CONFIG_REQUIRE" "$CONFIG_SRC" "$CONFIG_META"
    else
        # Backwards compatibility for pre-Colossus (pre-Free) era
        echo "********** Backwards compatibility  *******"
        "$LUAC" -s -o "$CONFIG_DST" "$CONFIG_SRC" "$CONFIG_META"
    fi

    rm "$CONFIG_SRC"
    rm "$CONFIG_META"

    # Create resource.corona-archive from *.lu
    echo Archiving "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/*.lu" into "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/resource.corona-archive"
    "$CAR" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/resource.corona-archive" "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"*.lu

    echo Removing: "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/*.lu"
    rm -f "$BUILT_PRODUCTS_DIR/$EXECUTABLE_FOLDER_PATH/"*.lu
else
    echo "Skipping creation of resource.corona-archive for template builds"
fi
