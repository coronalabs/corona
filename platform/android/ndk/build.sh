#!/bin/sh

# USAGE: ./build.sh device_type product_type [debug/release [clean]]
# device_type:   generic, kindle, nook_only, all
# product_type:  basic, all, trial, automation
# build_config:  debug, release
# build_type:    clean, incremental
# Default is debug/incremental

# Set this to exit the script on error (but things like 'javadoc' error all the time)
# set -e

path=`dirname $0`

if [ -z "$ANDROID_NDK" ]
then
	echo "ERROR: ANDROID_NDK environment variable must be defined"
	exit 0
fi

# Canonicalize paths
pushd $path > /dev/null
dir=`pwd`
path=$dir
popd > /dev/null

NDK_MODULE_PATH=$path/../../../plugins/
pushd $NDK_MODULE_PATH > /dev/null
dir=`pwd`
NDK_MODULE_PATH=$dir
popd > /dev/null

BIN_DIR=$path/../../../bin/mac
pushd $BIN_DIR > /dev/null
dir=`pwd`
BIN_DIR=$dir
popd > /dev/null


DEVICE_TYPE=$1
PRODUCT_TYPE=$2

echo "Building $PRODUCT_TYPE"

if [ "release" == "$3" ]
then
	echo "Building RELEASE"
	OPTIM_FLAGS="release"
	LUA2C_CONFIGURATION="RELEASE"
else
	echo "Building DEBUG"
	OPTIM_FLAGS="debug"
	LUA2C_CONFIGURATION="DEBUG"
fi

if [ "clean" == "$4" ]
then
	echo "== Clean build =="
	rm -rf $path/obj/ $path/libs/ $path/generated/
	FLAGS="-B"
else
	echo "== Incremental build =="
	FLAGS=""
fi

#####################
# Pre-compile Steps #
#####################

generated_files_path="$path/generated/"
mkdir -p "$generated_files_path"

# Compile lua files into .cpp files
$path/lua_to_native.sh $path/../../resources/init.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../external/lpeg/re.lua $generated_files_path/
$path/lua_to_native.sh $path/../../resources/json.lua $generated_files_path/
$path/lua_to_native.sh $path/../../resources/dkjson.lua $generated_files_path/
$path/lua_to_native.sh $path/../../resources/runtime.lua $generated_files_path/
$path/lua_to_native.sh $path/../../resources/loader_ccdata.lua $generated_files_path/
$path/lua_to_native.sh $path/../../resources/loader_callback.lua $generated_files_path/
$path/lua_to_native.sh $path/../../resources/launchpad.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/easing/easing.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/composer/composer.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/composer/composer_scene.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/timer/timer.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/transition/transition.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/transition/transition_v1.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widgetLibrary/widget.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widgetLibrary/widget_button.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widgetLibrary/widget_momentumScrolling.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widgetLibrary/widget_pickerWheel.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widgetLibrary/widget_scrollview.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widgetLibrary/widget_slider.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widgetLibrary/widget_tabbar.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widgetLibrary/widget_tableview.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widgetLibrary/widget_progressView.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widgetLibrary/widget_searchField.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widgetLibrary/widget_segmentedControl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widgetLibrary/widget_spinner.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widgetLibrary/widget_stepper.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widgetLibrary/widget_switch.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widget_theme_android_sheet.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widget_theme_android.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widget_theme_android_holo_dark.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widget_theme_android_holo_dark_sheet.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widget_theme_android_holo_light.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widget_theme_android_holo_light_sheet.lua $generated_files_path/
# These are for iOS pre 7
$path/lua_to_native.sh $path/../../../subrepos/widget/widget_theme_ios_sheet.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widget_theme_ios.lua $generated_files_path/
# These are for iOS 7+
$path/lua_to_native.sh $path/../../../subrepos/widget/widget_theme_ios7_sheet.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../subrepos/widget/widget_theme_ios7.lua $generated_files_path/
$path/lua_to_native.sh $path/../ndk/shell.lua $generated_files_path/

# Corona API
$path/lua_to_native.sh $path/../../../librtt/Corona/CoronaLibrary.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Corona/CoronaPrototype.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Corona/CoronaProvider.lua $generated_files_path/

# ads plugin
$BIN_DIR/lua2c.sh $path/../../../plugins/ads/shared/ads.lua $generated_files_path/. $LUA2C_CONFIGURATION $LUA_DIR
$BIN_DIR/lua2c.sh $path/../../../plugins/ads/shared/CoronaProvider.ads.lua $generated_files_path/. $LUA2C_CONFIGURATION $LUA_DIR

# analytics plugin
$BIN_DIR/lua2c.sh $path/../../../plugins/analytics/shared/analytics.lua $generated_files_path/. $LUA2C_CONFIGURATION $LUA_DIR
$BIN_DIR/lua2c.sh $path/../../../plugins/analytics/shared/CoronaProvider.analytics.lua $generated_files_path/. $LUA2C_CONFIGURATION $LUA_DIR

# Shaders
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_add_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_average_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_color_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_colorBurn_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_colorDodge_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_darken_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_difference_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_exclusion_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_glow_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_hardLight_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_hardMix_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_hue_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_lighten_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_linearLight_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_luminosity_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_multiply_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_negation_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_normalMapWith1DirLight_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_normalMapWith1PointLight_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_overlay_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_phoenix_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_pinLight_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_reflect_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_saturation_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_screen_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_softLight_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_subtract_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_vividLight_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_composite_yuv420f_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_default_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_bloom_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_blur_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_blurGaussian_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_blurHorizontal_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_blurVertical_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_brightness_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_bulge_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_chromaKey_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_color_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_colorChannelOffset_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_colorMatrix_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_colorPolynomial_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_contrast_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_crosshatch_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_crystallize_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_desaturate_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_dissolve_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_duotone_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_emboss_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_exposure_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_frostedGlass_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_grayscale_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_hue_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_invert_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_iris_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_levels_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_linearWipe_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_median_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_monotone_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_opTile_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_pixelate_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_polkaDots_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_posterize_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_radialWipe_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_saturate_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_scatter_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_sepia_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_sharpenLuminance_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_sobel_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_step_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_straighten_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_swirl_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_vignette_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_vignetteMask_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_water_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_wobble_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_woodCut_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_filter_zoomBlur_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_generator_checkerboard_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_generator_lenticularHalo_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_generator_linearGradient_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_generator_marchingAnts_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_generator_perlinNoise_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_generator_radialGradient_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_generator_random_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_generator_stripes_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/kernel_generator_sunbeams_gl.lua $generated_files_path/
$path/lua_to_native.sh $path/../../../librtt/Display/Shader/shell_default_gl.lua $generated_files_path/

# luasocket
$path/lua_to_native.sh -m luasocket $path/../../../external/luasocket/src/ftp.lua $generated_files_path/
$path/lua_to_native.sh -m luasocket $path/../../../external/luasocket/src/headers.lua $generated_files_path/
$path/lua_to_native.sh -m luasocket $path/../../../external/luasocket/src/http.lua $generated_files_path/
$path/lua_to_native.sh -m luasocket $path/../../../external/luasocket/src/ltn12.lua $generated_files_path/
$path/lua_to_native.sh -m luasocket $path/../../../external/luasocket/src/mbox.lua $generated_files_path/
$path/lua_to_native.sh -m luasocket $path/../../../external/luasocket/src/mime.lua $generated_files_path/
$path/lua_to_native.sh -m luasocket $path/../../../external/luasocket/src/smtp.lua $generated_files_path/
$path/lua_to_native.sh -m luasocket $path/../../../external/luasocket/src/socket.lua $generated_files_path/
$path/lua_to_native.sh -m luasocket $path/../../../external/luasocket/src/ssl.lua $generated_files_path/
$path/lua_to_native.sh -m luasocket $path/../../../external/luasocket/src/tp.lua $generated_files_path/
$path/lua_to_native.sh -m luasocket $path/../../../external/luasocket/src/url.lua $generated_files_path/

######################
# Build libcorona.so #
######################

pushd $path/jni > /dev/null

CFLAGS=
ALMIXER_CFLAGS=
ALMIXER_CFLAGS_LINKED_LIBRARIES=
if [ "$DEVICE_TYPE" = "generic" ]
then
	CFLAGS=""
elif [ "$DEVICE_TYPE" = "kindle" ]
then
	CFLAGS="-DRtt_SUPPORTS_KINDLE"
elif [ "$DEVICE_TYPE" = "nook" ]
then
	CFLAGS="-DRtt_SUPPORTS_NOOK"
elif [ "$DEVICE_TYPE" = "nook_only" ]
then
	CFLAGS="-DRtt_NOOK_ENV -DRtt_SUPPORTS_NOOK"
else
	CFLAGS="-DRtt_SUPPORTS_KINDLE -DRtt_SUPPORTS_NOOK"
fi
if [ "$PRODUCT_TYPE" = "automation" ]
then
	CFLAGS="${CFLAGS} -DRtt_OSEXIT_ON_LUAERROR"
else
	if [ "$PRODUCT_TYPE" = "trial" ]
	then
		CFLAGS="${CFLAGS} -DRtt_TRIAL"
	fi

	if [ "$OPTIM_FLAGS" = "debug" ]
	then
		CFLAGS="${CFLAGS} -DRtt_DEBUG -g"
		FLAGS="$FLAGS NDK_DEBUG=1"
	fi
fi

if [ "$PRODUCT_TYPE" != "coronacards" ]
then
	ALMIXER_CFLAGS="-DMPG123_NO_CONFIGURE -DSOUND_SUPPORTS_MPG123"
	LINK_TO_LIBMPG="libmpg123"
else
	ALMIXER_CFLAGS=""
	LINK_TO_LIBMPG=""
fi

# Suppress warnings about "note: the mangling of 'va_list' has changed in GCC 4.4"
CFLAGS="$CFLAGS -Wno-psabi"

export NDK_MODULE_PATH=$NDK_MODULE_PATH
echo $NDK_MODULE_PATH

if [ -z "$CFLAGS" ]
then
	echo "----------------------------------------------------------------------------"
	echo "$ANDROID_NDK/ndk-build $FLAGS V=1 APP_OPTIM=$OPTIM_FLAGS"
	echo "----------------------------------------------------------------------------"

	$ANDROID_NDK/ndk-build -j9 $FLAGS V=1 MY_ALMIXER_CFLAGS="$ALMIXER_CFLAGS" MY_LINK_TO_LIBMPG="$LINK_TO_LIBMPG" APP_OPTIM=$OPTIM_FLAGS
else
	echo "----------------------------------------------------------------------------"
	echo "$ANDROID_NDK/ndk-build $FLAGS V=1 MY_CFLAGS="$CFLAGS" APP_OPTIM=$OPTIM_FLAGS"
	echo "----------------------------------------------------------------------------"

	$ANDROID_NDK/ndk-build -j9 $FLAGS V=1 MY_CFLAGS="$CFLAGS" MY_ALMIXER_CFLAGS="$ALMIXER_CFLAGS" MY_LINK_TO_LIBMPG="$LINK_TO_LIBMPG" APP_OPTIM=$OPTIM_FLAGS
fi

if [ $? -ne 0 ]
then
	exit -1
fi

popd > /dev/null

######################
# Post-compile Steps #
######################

# Copy .so files over to the Android SDK (Java) side of things
cp -rv $path/libs $path/../sdk

