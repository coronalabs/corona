#!/bin/bash #-x
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

# Passed in arguments
# $1 SDK_VERSION
if [ -z "$1" ]
then	
	SDK_VERSION=$(xcrun --sdk iphoneos --show-sdk-version)
else
	SDK_VERSION=$1

	if [ ! -z "$2" ]
	then
		CORONA_BUILD_ID=$2
	else
		CORONA_BUILD_ID="DEV"
	fi
	export CORONA_BUILD_ID
fi


path=$(dirname "$0")

# Summarize xcodebuild output to stdout but save full output in separate file
XCODE_LOG_FILTERS="^    export |clang -x |libtool -static |^    cd $path"

FULL_LOG_FILE="iphone-build_templates-xcodebuild.log"
if [ "$WORKSPACE" != "" ]
then
	FULL_LOG_FILE="$WORKSPACE/$FULL_LOG_FILE"
fi

echo "### Full xcodebuild output can be found in $FULL_LOG_FILE"

# Prerequisites
# -----------------------------------------------------------------------------

# setup
# -----------------------------------------------------------------------------
echo "Creating directories:"
mkdir -pv "$path/template"
mkdir -pv "$path/template-dSYM"



# Utility method
# -----------------------------------------------------------------------------

# $1 device (e.g. ipad or iphone)
# $2 architecture (e.g. device or simulator)
# $3 version
# $4 configuration (e.g. trial, basic)
# $5 target (e.g. template)
# $6 extension (e.g. 'app' or 'a')
build_target() {
	DEVICE=$1
	ARCHITECTURE=$2
	VERSION=$3
	CONFIGURATION=$4
	TARGET=$5
	EXTENSION=$6

	export SUPPRESS_APP_SIGN=1

	# PRODUCT_NAME=template
	PRODUCT_NAME=$TARGET

	if [ -z "$EXTENSION" ]
	then
		EXTENSION=app
	fi

	PRODUCT_DST=${PRODUCT_NAME}.${EXTENSION}

	DST_ROOT=$DEVICE
	SDK_BASE=iphoneos
	if [ "$ARCHITECTURE" = "simulator" ]
	then
		SDK_BASE=iphonesimulator
		DST_ROOT=$DEVICE-sim
	fi
	SDK=$SDK_BASE$VERSION

	echo ""
	echo "$DEVICE $VERSION $TARGET $CONFIGURATION ($ARCHITECTURE)"
	echo "-------------------------------------------------------"

	
	mkdir -pv "$path/template/$DST_ROOT/$VERSION/$CONFIGURATION"
	mkdir -pv "$path/template-dSYM/$DST_ROOT/$VERSION/$CONFIGURATION"
	rm -vrf "$path/template/$DST_ROOT/$VERSION/$CONFIGURATION/${PRODUCT_DST}" 
	rm -vrf "$path/template-dSYM/$DST_ROOT/$VERSION/$CONFIGURATION/${PRODUCT_DST}.dSYM" 

	echo "Running: xcodebuild -project '$path'/ratatouille.xcodeproj -target '$TARGET' -configuration Release -sdk '$SDK'"
	xcodebuild -project "$path"/ratatouille.xcodeproj -target "$TARGET" -configuration Release -sdk "$SDK" 2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"
    checkError

	SUFFIX=$SDK_BASE

	# Move build product to final location
	mv -v "$path/build/Release-$SUFFIX/${PRODUCT_DST}" "$path/template/$DST_ROOT/$VERSION/$CONFIGURATION/${PRODUCT_DST}"

	if [ "app" = $EXTENSION ]
	then
		# Move dsym for .app executables
		mv -v "$path/build/Release-$SUFFIX/${PRODUCT_DST}.dSYM" "$path/template-dSYM/$DST_ROOT/$VERSION/$CONFIGURATION/${PRODUCT_DST}.dSYM"

		# Ensure that the binary is called "template"
		mv -v "$path/template/$DST_ROOT/$VERSION/$CONFIGURATION/${PRODUCT_DST}/$PRODUCT_NAME" "$path/template/$DST_ROOT/$VERSION/$CONFIGURATION/${PRODUCT_DST}/template"
	fi
}


# iOS TEMPLATES
# --------------------------------------------------------------------------------------------------

# iPhone basic (i.e. for subscribers)
build_target "iphone" "device" "$SDK_VERSION" "basic" "template"
checkError

# iPhone trial
ln -s basic "$path/template/iphone/$SDK_VERSION/trial"

# iPhone all is now the same as basic
ln -s basic "$path/template/iphone/$SDK_VERSION/all"

# Templates are universal now so iPad versions are just mirrors of the iPhone
ln -s iphone "$path/template/ipad"

# Plugins: libtemplate.a
TEMPLATE_DIR=$path/template/iphone/$SDK_VERSION/basic
cp -rv "$path"/../../tools/buildsys-ios/libtemplate "$TEMPLATE_DIR"

build_target "iphone" "device" "$SDK_VERSION" "basic" "libtemplate" "a"
checkError
LIBTEMPLATE_DIR=$TEMPLATE_DIR/libtemplate
mv -v "$TEMPLATE_DIR"/libtemplate.a "$LIBTEMPLATE_DIR"/.

# XCODE SIMULATOR TEMPLATES
# --------------------------------------------------------------------------------------------------

# iPhone basic (i.e. for subscribers)
build_target "iphone" "simulator" "$SDK_VERSION" "basic" "template"

# iPhone trial
ln -s basic "$path/template/iphone-sim/$SDK_VERSION/trial"

# iPhone all is now the same as basic
ln -s basic "$path/template/iphone-sim/$SDK_VERSION/all"

# Templates are universal now so iPad versions are just mirrors of the iPhone
ln -s iphone-sim "$path/template/ipad-sim"

# Plugins: libtemplate.a
TEMPLATE_DIR=$path/template/iphone-sim/$SDK_VERSION/basic
cp -rv "$path"/../../tools/buildsys-ios/libtemplate "$TEMPLATE_DIR"

build_target "iphone" "simulator" "$SDK_VERSION" "basic" "libtemplate" "a"
LIBTEMPLATE_DIR=$TEMPLATE_DIR/libtemplate
mv -v "$TEMPLATE_DIR"/libtemplate.a "$LIBTEMPLATE_DIR"/.



templateArchive=$(date "+template.%Y.%m.%d.zip")
zip -ry "$path/$templateArchive" "$path/template"
checkError

dsymArchive=$(date "+template-dSYM.%Y.%m.%d.zip")
zip -ry "$path/$dsymArchive" "$path/template-dSYM"
checkError
