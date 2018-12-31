#!/bin/bash -x

# Builds input.zip that goes to server

path=$(dirname "$0")

if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]
then
	echo "USAGE: $0 build_config project_dir tmp_dir"
	echo "   build_config:   debug (default) or release"
	exit -1
fi

#
# Checks exit value for error
# 
checkError() {
	# shellcheck disable=SC2181
	if [ $? -ne 0 ]
	then
		echo "Exiting due to errors (above)"
		exit -1
	fi
}

BUILD_CONFIG=$1
PROJECT_DIR=$2
TMP_DIR=$3

# Canonicalize paths
pushd "$path" > /dev/null
dir=$(pwd)
path=$dir
popd > /dev/null

pushd "$PROJECT_DIR" > /dev/null
dir=$(pwd)
PROJECT_DIR=$dir
popd > /dev/null

pushd "$TMP_DIR" > /dev/null
dir=$(pwd)
TMP_DIR=$dir
popd > /dev/null


# -----------------------------------------------------------------------------
# Compile Lua files => .lu
# -----------------------------------------------------------------------------

# Create input dir
INPUT_DIR=$TMP_DIR/input
mkdir "$INPUT_DIR"

"$path/../../bin/mac/BuildSources.sh" "$BUILD_CONFIG" "$PROJECT_DIR" "$INPUT_DIR"
checkError


# -----------------------------------------------------------------------------
# config.lua
# -----------------------------------------------------------------------------
CONFIG_META="$TMP_DIR/config.metadata.lua"
PROJECT_NAME=$(basename "$PROJECT_DIR")
APP_ID=$(echo "$PROJECT_DIR" | md5)
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
    appId = "$APP_ID",
    userId = "345", -- drupal user id
    subscription = "pro", -- either "trial", "pro", "indie-ios", "indie-android"
    mode = "developer", -- either "developer" or nil (where nil means distribution/store)
	build = 42,
}
EOF

# Concatenate config.metadata.lua into config.lu overwriting original config.lu
LUAC=$path/../../bin/mac/luac
CONFIG_SRC="$INPUT_DIR/config.lu"
CONFIG_ORIG="$TMP_DIR/config.orig.lu"
if [ -f "$CONFIG_SRC" ]
then
	mv "$CONFIG_SRC" "$CONFIG_ORIG"
else
	touch "$CONFIG_ORIG"
fi
"$LUAC" -s -o "$INPUT_DIR/config.lu" "$CONFIG_ORIG" "$CONFIG_META"
rm "$CONFIG_META"
rm "$CONFIG_ORIG"

# -----------------------------------------------------------------------------
# Create build.properties
# -----------------------------------------------------------------------------

# Pass optional project dir only if build.settings exists
PROJ_DIR=
if [ -e "$PROJECT_DIR/build.settings" ]
then
	PROJ_DIR=$PROJECT_DIR
fi

"$path/../../bin/mac/lua" -e "package.path=package.path..';$path/../resources/?.lua'" -e "package.cpath=package.cpath..';$path/../../bin/mac/lfs.so'" "$path/create_build_properties.lua" "$TMP_DIR" com.ansca.test.Corona "$PROJ_DIR" 1 1.0
checkError

# -----------------------------------------------------------------------------
# input.zip
# -----------------------------------------------------------------------------

zip -rj "$INPUT_DIR" "$TMP_DIR"
checkError
