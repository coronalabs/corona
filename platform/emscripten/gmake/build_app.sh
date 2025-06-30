#!/bin/sh

path=$(dirname "$0")
CORONA_PROJECT_DIR=$1
OUTPUT_HTML=$2
CONFIG=$3
BIN_DIR=$path/../../../bin/mac

# -----------------------------------------------------------------------------

#
# Prints usage
# 
usage() {
	echo ""
	echo "USAGE: $0 CORONA_PROJECT_DIR [OUTPUT_HTML [CONFIG]]"
	exit 255
}

#
# Checks exit value for error
# 
checkError() {
	if [ $? -ne 0 ]
	then
		echo "Exiting due to errors (above)"
		exit 233
	fi
}

# -----------------------------------------------------------------------------

if [ ! -d "$CORONA_PROJECT_DIR" ]
then
	usage
fi

# -----------------------------------------------------------------------------

# 
# Canonicalize relative paths to absolute paths
# 
pushd $path > /dev/null
dir=`pwd`
path=$dir
popd > /dev/null

pushd $CORONA_PROJECT_DIR > /dev/null
dir=`pwd`
CORONA_PROJECT_DIR=$dir
popd > /dev/null

# -----------------------------------------------------------------------------

echo "Using following settings:"

if which emcc >/dev/null; then
    echo "\t Emscripten path = '$(dirname "$(which emcc)")'"
else
	echo "\t ERROR: Unable to find EMSCRIPTEN_ROOT."
	exit 200
fi

if [ -z "$CONFIG" ]
then
	CONFIG=Debug
fi
echo "\t Configuration = '$CONFIG'"

CC_FLAGS=-g
LUA2C_CONFIGURATION="DEBUG"
if [ "$CONFIG" = "Release" ]
then
	CC_FLAGS=-O2
	LUA2C_CONFIGURATION="RELEASE"
fi
echo "\t CC flags = '$CC_FLAGS'"

if [ -z "$OUTPUT_HTML" ]
then
	OUTPUT_HTML=a.html
fi
echo "\t Output = '$OUTPUT_HTML'"

# Create output directory
mkdir -p `dirname "$OUTPUT_HTML"`

if [ -z "$TMP_DIR" ]
then
	TMP_DIR=`mktemp -d /tmp/CoronaWeb-XXXXXXXX`
	checkError
fi
echo "\t Tmp dir = '$TMP_DIR'"

# -----------------------------------------------------------------------------

pushd $path > /dev/null

	echo " "
	echo "Building Corona libraries:"

	echo '\t' make AR=emar CC=emcc CXX=em++ verbose=1 config="$CONFIG" CXXFLAGS="-s LEGACY_VM_SUPPORT=1 -s USE_SDL=2"
	make AR=emar CC=emcc CXX=em++ verbose=1 config="$CONFIG" CXXFLAGS="-s LEGACY_VM_SUPPORT=1 -s USE_SDL=2 -I\"$path/hack_includes\""
	checkError

	echo " "
	echo "Copying project files:"
	BIN_DIR=$path/../../../bin/mac
	echo '\t' "$BIN_DIR/CopyResources.sh" $CONFIG "$CORONA_PROJECT_DIR" "$TMP_DIR" "$BIN_DIR"
	"$BIN_DIR/CopyResources.sh" $CONFIG "$CORONA_PROJECT_DIR" "$TMP_DIR" --preserve "$BIN_DIR"
	checkError

	if [[ -n $(find "$TMP_DIR" -mindepth 1 -maxdepth 1 -iname "*.lu") ]]
	then
		echo " "
		echo "Generate resource.car:"
		find "$TMP_DIR" -mindepth 1 -maxdepth 1 -iname "*.lu" | "$path"/../../../bin/mac/car -f - "$TMP_DIR/resource.car"
		checkError
		find "$TMP_DIR" -mindepth 1 -maxdepth 1 -iname "*.lu" -print0 | xargs -0 rm -f 
		checkError
	else
		echo "Skipping .car"
	fi

	echo " "
	echo "Building HTML:"
	echo '\t' emcc obj/"$CONFIG"/libratatouille.a obj/"$CONFIG"/librtt.a $CC_FLAGS obj/"$CONFIG"/libBox2D.a $CC_FLAGS obj/"$CONFIG"/liblua.a $CC_FLAGS obj/"$CONFIG"/libpng.a $CC_FLAGS obj/"$CONFIG"/libjpeg.a $CC_FLAGS obj/"$CONFIG"/libz.a $CC_FLAGS obj/"$CONFIG"/liblfs.a $CC_FLAGS obj/"$CONFIG"/liblpeg.a $CC_FLAGS obj/"$CONFIG"/libRenderer.a -s LEGACY_VM_SUPPORT=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' -O3 -s USE_SDL=2 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=128MB --js-library ../Rtt_PlatformWebAudioPlayer.js --js-library ../Rtt_EmscriptenPlatform.js --js-library ../Rtt_EmscriptenVideo.js --preload-file "$TMP_DIR"@/ -o "$OUTPUT_HTML"
	emcc obj/"$CONFIG"/libratatouille.a obj/"$CONFIG"/librtt.a $CC_FLAGS obj/"$CONFIG"/libBox2D.a $CC_FLAGS obj/"$CONFIG"/liblua.a $CC_FLAGS obj/"$CONFIG"/libpng.a $CC_FLAGS obj/"$CONFIG"/libjpeg.a $CC_FLAGS obj/"$CONFIG"/libz.a $CC_FLAGS obj/"$CONFIG"/liblfs.a $CC_FLAGS obj/"$CONFIG"/liblpeg.a $CC_FLAGS obj/"$CONFIG"/libRenderer.a -s LEGACY_VM_SUPPORT=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' -O3 -s USE_SDL=2 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=128MB --js-library ../Rtt_PlatformWebAudioPlayer.js --js-library ../Rtt_EmscriptenPlatform.js --js-library ../Rtt_EmscriptenVideo.js -lidbfs.js --preload-file "$TMP_DIR"@/ -o "$OUTPUT_HTML"
	checkError


	echo "SUCCESS! Run with command:"
	echo '\t' emrun $OUTPUT_HTML
	
	# pushd `dirname "$OUTPUT_HTML"` > /dev/null
	# OUT_PATH=`pwd`
	# OUT_FILE=`basename "$OUTPUT_HTML"`
	# FULL_OUTPUT_PATH="$OUT_PATH/$OUT_FILE"
	# popd > /dev/null
	# echo '\t' "\"$EMSCRIPTEN_ROOT/emrun\" \"$FULL_OUTPUT_PATH\""

popd $path > /dev/null

