//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ShaderBuiltin.h"

#include "Corona/CoronaLua.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// Filter.

static const char kFilterBloomString[] = "bloom";
static const char kFilterBlurString[] = "blur";
static const char kFilterBlurGaussianString[] = "blurGaussian";
static const char kFilterBlurHorizontalString[] = "blurHorizontal";
static const char kFilterBlurVerticalString[] = "blurVertical";
static const char kFilterBlurGaussianLinearString[] = "blurGaussianLinear";
static const char kFilterBlurLinearHorizontalString[] = "blurLinearHorizontal";
static const char kFilterBlurLinearVerticalString[] = "blurLinearVertical";
static const char kFilterBrightnessString[] = "brightness";
static const char kFilterBulgeString[] = "bulge";
static const char kFilterChromaKeyString[] = "chromaKey";
static const char kFilterColorString[] = "color";
static const char kFilterColorChannelOffsetString[] = "colorChannelOffset";
static const char kFilterColorMatrixString[] = "colorMatrix";
static const char kFilterColorPolynomialString[] = "colorPolynomial";
static const char kFilterContrastString[] = "contrast";
static const char kFilterCrosshatchString[] = "crosshatch";
static const char kFilterCrystallizeString[] = "crystallize";
static const char kFilterDesaturateString[] = "desaturate";
static const char kFilterDissolveString[] = "dissolve";
static const char kFilterDuotoneString[] = "duotone";
static const char kFilterEmbossString[] = "emboss";
static const char kFilterExposureString[] = "exposure";
static const char kFilterFrostedGlassString[] = "frostedGlass";
static const char kFilterGrayscaleString[] = "grayscale";
static const char kFilterHueString[] = "hue";
static const char kFilterInvertString[] = "invert";
static const char kFilterIrisString[] = "iris";
static const char kFilterLevelsString[] = "levels";
static const char kFilterLinearWipeString[] = "linearWipe";
static const char kFilterMedianString[] = "median";
static const char kFilterMonotoneString[] = "monotone";
static const char kFilterOpTileString[] = "opTile";
static const char kFilterPixelateString[] = "pixelate";
static const char kFilterPolkaDotsString[] = "polkaDots";
static const char kFilterPosterizeString[] = "posterize";
static const char kFilterRadialWipeString[] = "radialWipe";
static const char kFilterSaturateString[] = "saturate";
static const char kFilterScatterString[] = "scatter";
static const char kFilterSepiaString[] = "sepia";
static const char kFilterSharpenLuminanceString[] = "sharpenLuminance";
static const char kFilterSobelString[] = "sobel";
static const char kFilterStepString[] = "step";
static const char kFilterStraightenString[] = "straighten";
static const char kFilterSwirlString[] = "swirl";
static const char kFilterVignetteString[] = "vignette";
static const char kFilterVignetteMaskString[] = "vignetteMask";
static const char kFilterWaterString[] = "water";
static const char kFilterWobbleString[] = "wobble";
static const char kFilterWoodCutString[] = "woodCut";
static const char kFilterZoomBlurString[] = "zoomBlur";

// Composite.

static const char kCompositeColorString[] = "color";
static const char kCompositeGlowString[] = "glow";
static const char kCompositeLinearLightString[] = "linearLight";
static const char kCompositeNormalMapWith1DirLightString[] = "normalMapWith1DirLight";
static const char kCompositeNormalMapWith1PointLightString[] = "normalMapWith1PointLight";
static const char kCompositePhoenixString[] = "phoenix";
static const char kCompositeSaturationString[] = "saturation";

// Generator.

static const char kGeneratorCheckerboardString[] = "checkerboard";
static const char kGeneratorLenticularHaloString[] = "lenticularHalo";
static const char kGeneratorLinearGradientString[] = "linearGradient";
static const char kGeneratorMarchingAntsString[] = "marchingAnts";
static const char kGeneratorPerlinNoiseString[] = "perlinNoise";
static const char kGeneratorRadialGradientString[] = "radialGradient";
static const char kGeneratorRandomString[] = "random";
static const char kGeneratorStripesString[] = "stripes";
static const char kGeneratorSunbeamsString[] = "sunbeams";

const char *
ShaderBuiltin::StringForFilter( Filter filter )
{
	const char *result = NULL;

	switch ( filter )
	{
		// Filter.

		case kFilterBloom:
			result = kFilterBloomString;
			break;
		case kFilterBlur:
			result = kFilterBlurString;
			break;
		case kFilterBlurGaussian:
			result = kFilterBlurGaussianString;
			break;
		case kFilterBlurHorizontal:
			result = kFilterBlurHorizontalString;
			break;
		case kFilterBlurVertical:
			result = kFilterBlurVerticalString;
			break;
		case kFilterBlurGaussianLinear:
			result = kFilterBlurGaussianLinearString;
			break;
		case kFilterBlurLinearHorizontal:
			result = kFilterBlurLinearHorizontalString;
			break;
		case kFilterBlurLinearVertical:
			result = kFilterBlurLinearVerticalString;
			break;
		case kFilterBrightness:
			result = kFilterBrightnessString;
			break;
		case kFilterBulge:
			result = kFilterBulgeString;
			break;
		case kFilterChromaKey:
			result = kFilterChromaKeyString;
			break;
		case kFilterColor:
			result = kFilterColorString;
			break;
		case kFilterColorChannelOffset:
			result = kFilterColorChannelOffsetString;
			break;
		case kFilterColorMatrix:
			result = kFilterColorMatrixString;
			break;
		case kFilterColorPolynomial:
			result = kFilterColorPolynomialString;
			break;
		case kFilterContrast:
			result = kFilterContrastString;
			break;
		case kFilterCrosshatch:
			result = kFilterCrosshatchString;
			break;
		case kFilterCrystallize:
			result = kFilterCrystallizeString;
			break;
		case kFilterDesaturate:
			result = kFilterDesaturateString;
			break;
		case kFilterDissolve:
			result = kFilterDissolveString;
			break;
		case kFilterDuotone:
			result = kFilterDuotoneString;
			break;
		case kFilterEmboss:
			result = kFilterEmbossString;
			break;
		case kFilterExposure:
			result = kFilterExposureString;
			break;
		case kFilterFrostedGlass:
			result = kFilterFrostedGlassString;
			break;
		case kFilterGrayscale:
			result = kFilterGrayscaleString;
			break;
		case kFilterHue:
			result = kFilterHueString;
			break;
		case kFilterInvert:
			result = kFilterInvertString;
			break;
		case kFilterIris:
			result = kFilterIrisString;
			break;
		case kFilterLevels:
			result = kFilterLevelsString;
			break;
		case kFilterLinearWipe:
			result = kFilterLinearWipeString;
			break;
		case kFilterMedian:
			result = kFilterMedianString;
			break;
		case kFilterMonotone:
			result = kFilterMonotoneString;
			break;
		case kFilterOpTile:
			result = kFilterOpTileString;
			break;
		case kFilterPixelate:
			result = kFilterPixelateString;
			break;
		case kFilterPolkaDots:
			result = kFilterPolkaDotsString;
			break;
		case kFilterPosterize:
			result = kFilterPosterizeString;
			break;
		case kFilterRadialWipe:
			result = kFilterRadialWipeString;
			break;
		case kFilterSaturate:
			result = kFilterSaturateString;
			break;
		case kFilterScatter:
			result = kFilterScatterString;
			break;
		case kFilterSepia:
			result = kFilterSepiaString;
			break;
		case kFilterSharpenLuminance:
			result = kFilterSharpenLuminanceString;
			break;
		case kFilterSobel:
			result = kFilterSobelString;
			break;
		case kFilterStep:
			result = kFilterStepString;
			break;
		case kFilterStraighten:
			result = kFilterStraightenString;
			break;
		case kFilterSwirl:
			result = kFilterSwirlString;
			break;
		case kFilterVignette:
			result = kFilterVignetteString;
			break;
		case kFilterVignetteMask:
			result = kFilterVignetteMaskString;
			break;
		case kFilterWater:
			result = kFilterWaterString;
			break;
		case kFilterWobble:
			result = kFilterWobbleString;
			break;
		case kFilterWoodCut:
			result = kFilterWoodCutString;
			break;
		case kFilterZoomBlur:
			result = kFilterZoomBlurString;
			break;

		// Composite.

		case kCompositeColor:
			result = kCompositeColorString;
			break;
		case kCompositeGlow:
			result = kCompositeGlowString;
			break;
		case kCompositeLinearLight:
			result = kCompositeLinearLightString;
			break;
		case kCompositeNormalMapWith1DirLight:
			result = kCompositeNormalMapWith1DirLightString;
			break;
		case kCompositeNormalMapWith1PointLight:
			result = kCompositeNormalMapWith1PointLightString;
			break;
		case kCompositePhoenix:
			result = kCompositePhoenixString;
			break;
		case kCompositeSaturation:
			result = kCompositeSaturationString;
			break;

		// Generator.

		case kGeneratorCheckerboard:
			result = kGeneratorCheckerboardString;
			break;
		case kGeneratorLenticularHalo:
			result = kGeneratorLenticularHaloString;
			break;
		case kGeneratorLinearGradient:
			result = kGeneratorLinearGradientString;
			break;
		case kGeneratorMarchingAnts:
			result = kGeneratorMarchingAntsString;
			break;
		case kGeneratorPerlinNoise:
			result = kGeneratorPerlinNoiseString;
			break;
		case kGeneratorRadialGradient:
			result = kGeneratorRadialGradientString;
			break;
		case kGeneratorRandom:
			result = kGeneratorRandomString;
			break;
		case kGeneratorStripes:
			result = kGeneratorStripesString;
			break;
		case kGeneratorSunbeams:
			result = kGeneratorSunbeamsString;
			break;

		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}
	
	return result;
}

int
ShaderBuiltin::AppendFilters( lua_State *L, int index )
{
	int base = (int) lua_objlen( L, index ) + 1;

	int i = 0;

	for( ;
			i < kNumFilters;
			++i )
	{
		lua_pushstring( L, StringForFilter( (Filter)i ) );
		lua_rawseti( L, index, base + i );
	}

	return i;
}

int
ShaderBuiltin::Append( lua_State *L, int index, ShaderTypes::Category category )
{
	int result = 0;

	switch ( category )
	{
		case ShaderTypes::kCategoryFilter:
			result = AppendFilters( L, index );
			break;
		case ShaderTypes::kCategoryComposite:
			result = AppendFilters( L, index );
			break;
		case ShaderTypes::kCategoryGenerator:
			result = AppendFilters( L, index );
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

bool
ShaderBuiltin::FilterExists( const char *name )
{
	Rtt_ASSERT( name );

	for( int i = 0;
			i < kNumFilters;
			i++ )
	{
		if( ! strcmp( name, StringForFilter( (Filter)i ) ) )
		{
			// "name" WAS found.
			return true;
		}
	}

	// "name" WASN'T found.
	return false;
}

bool
ShaderBuiltin::Exists( ShaderTypes::Category category, const char *name )
{
	bool result = false;

	if ( name )
	{
		switch ( category )
		{
			case ShaderTypes::kCategoryFilter:
				result = FilterExists( name );
				break;
			case ShaderTypes::kCategoryComposite:
				result = FilterExists( name );
				break;
			case ShaderTypes::kCategoryGenerator:
				result = FilterExists( name );
				break;
			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
		}
	}

	return result;
}

// ----------------------------------------------------------------------------

// Default
int luaload_shell_default_gl(lua_State* L);
int luaload_kernel_default_gl(lua_State* L);
int luaload_shell_default_vulkan(lua_State * L);

bool
ShaderBuiltin::PushDefaultShell( lua_State *L, const char * backend )
{
#if defined( Rtt_WIN_ENV )
	if (strcmp( backend, "vulkanBackend" ) == 0)
	{
		lua_pushcfunction( L, Corona::Lua::Open< luaload_shell_default_vulkan > );
	}
	else
	{
		lua_pushcfunction( L, Corona::Lua::Open< luaload_shell_default_gl > );
	}
#else
	lua_pushcfunction( L, Corona::Lua::Open< luaload_shell_default_gl > );
#endif

	return ( !! Rtt_VERIFY( 0 == Corona::Lua::DoCall( L, 0, 1 ) ) );
}

bool
ShaderBuiltin::PushDefaultKernel( lua_State *L )
{
	lua_pushcfunction( L, Corona::Lua::Open< luaload_kernel_default_gl > );

	return ( !! Rtt_VERIFY( 0 == Corona::Lua::DoCall( L, 0, 1 ) ) );
}

// ----------------------------------------------------------------------------

const char *
ShaderBuiltin::KeyForCategory( ShaderTypes::Category category )
{
	static const char kFilterKey[] = "corona.kernel.filter";
	static const char kCompositeKey[] = "corona.kernel.composite";
	static const char kGeneratorKey[] = "corona.kernel.generator";

	const char *result = NULL;

	switch ( category )
	{
		case ShaderTypes::kCategoryFilter:
			result = kFilterKey;
			break;
		case ShaderTypes::kCategoryComposite:
			result = kCompositeKey;
			break;
		case ShaderTypes::kCategoryGenerator:
			result = kGeneratorKey;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

// ----------------------------------------------------------------------------

// Filter.

int luaload_kernel_filter_bloom_gl( lua_State *L );
int luaload_kernel_filter_blur_gl( lua_State *L );
int luaload_kernel_filter_blurGaussian_gl( lua_State *L );
int luaload_kernel_filter_blurHorizontal_gl( lua_State *L );
int luaload_kernel_filter_blurVertical_gl( lua_State *L );
int luaload_kernel_filter_blurGaussianLinear_gl( lua_State *L );
int luaload_kernel_filter_blurLinearHorizontal_gl( lua_State *L );
int luaload_kernel_filter_blurLinearVertical_gl( lua_State *L );
int luaload_kernel_filter_brightness_gl( lua_State *L );
int luaload_kernel_filter_bulge_gl( lua_State *L );
int luaload_kernel_filter_chromaKey_gl( lua_State *L );
int luaload_kernel_filter_color_gl( lua_State *L );
int luaload_kernel_filter_colorChannelOffset_gl( lua_State *L );
int luaload_kernel_filter_colorMatrix_gl( lua_State *L );
int luaload_kernel_filter_colorPolynomial_gl( lua_State *L );
int luaload_kernel_filter_contrast_gl( lua_State *L );
int luaload_kernel_filter_crosshatch_gl( lua_State *L );
int luaload_kernel_filter_crystallize_gl( lua_State *L );
int luaload_kernel_filter_desaturate_gl( lua_State *L );
int luaload_kernel_filter_dissolve_gl( lua_State *L );
int luaload_kernel_filter_duotone_gl( lua_State *L );
int luaload_kernel_filter_emboss_gl( lua_State *L );
int luaload_kernel_filter_exposure_gl( lua_State *L );
int luaload_kernel_filter_frostedGlass_gl( lua_State *L );
int luaload_kernel_filter_grayscale_gl( lua_State *L );
int luaload_kernel_filter_hue_gl( lua_State *L );
int luaload_kernel_filter_invert_gl( lua_State *L );
int luaload_kernel_filter_iris_gl( lua_State *L );
int luaload_kernel_filter_levels_gl( lua_State *L );
int luaload_kernel_filter_linearWipe_gl( lua_State *L );
int luaload_kernel_filter_median_gl( lua_State *L );
int luaload_kernel_filter_monotone_gl( lua_State *L );
int luaload_kernel_filter_opTile_gl( lua_State *L );
int luaload_kernel_filter_pixelate_gl( lua_State *L );
int luaload_kernel_filter_polkaDots_gl( lua_State *L );
int luaload_kernel_filter_posterize_gl( lua_State *L );
int luaload_kernel_filter_radialWipe_gl( lua_State *L );
int luaload_kernel_filter_saturate_gl( lua_State *L );
int luaload_kernel_filter_scatter_gl( lua_State *L );
int luaload_kernel_filter_sepia_gl( lua_State *L );
int luaload_kernel_filter_sharpenLuminance_gl( lua_State *L );
int luaload_kernel_filter_sobel_gl( lua_State *L );
int luaload_kernel_filter_step_gl( lua_State *L );
int luaload_kernel_filter_straighten_gl( lua_State *L );
int luaload_kernel_filter_swirl_gl( lua_State *L );
int luaload_kernel_filter_vignette_gl( lua_State *L );
int luaload_kernel_filter_vignetteMask_gl( lua_State *L );
int luaload_kernel_filter_water_gl( lua_State *L );
int luaload_kernel_filter_wobble_gl( lua_State *L );
int luaload_kernel_filter_woodCut_gl( lua_State *L );
int luaload_kernel_filter_zoomBlur_gl( lua_State *L );

static const luaL_Reg kBuiltInFilterFuncs[] =
{
	{ kFilterBloomString,						Corona::Lua::Open< luaload_kernel_filter_bloom_gl > },
	{ kFilterBlurString, 						Corona::Lua::Open< luaload_kernel_filter_blur_gl > },
	{ kFilterBlurGaussianString, 				Corona::Lua::Open< luaload_kernel_filter_blurGaussian_gl > },
	{ kFilterBlurHorizontalString, 				Corona::Lua::Open< luaload_kernel_filter_blurHorizontal_gl > },
	{ kFilterBlurVerticalString, 				Corona::Lua::Open< luaload_kernel_filter_blurVertical_gl > },
	{ kFilterBlurGaussianLinearString, 			Corona::Lua::Open< luaload_kernel_filter_blurGaussianLinear_gl > },
	{ kFilterBlurLinearHorizontalString, 		Corona::Lua::Open< luaload_kernel_filter_blurLinearHorizontal_gl > },
	{ kFilterBlurLinearVerticalString, 			Corona::Lua::Open< luaload_kernel_filter_blurLinearVertical_gl > },
	{ kFilterBrightnessString, 					Corona::Lua::Open< luaload_kernel_filter_brightness_gl > },
	{ kFilterBulgeString, 						Corona::Lua::Open< luaload_kernel_filter_bulge_gl > },
	{ kFilterChromaKeyString, 					Corona::Lua::Open< luaload_kernel_filter_chromaKey_gl > },
	{ kFilterColorString, 						Corona::Lua::Open< luaload_kernel_filter_color_gl > },
	{ kFilterColorChannelOffsetString,			Corona::Lua::Open< luaload_kernel_filter_colorChannelOffset_gl > },
	{ kFilterColorMatrixString,					Corona::Lua::Open< luaload_kernel_filter_colorMatrix_gl > },
	{ kFilterColorPolynomialString,				Corona::Lua::Open< luaload_kernel_filter_colorPolynomial_gl > },
	{ kFilterContrastString, 					Corona::Lua::Open< luaload_kernel_filter_contrast_gl > },
	{ kFilterCrosshatchString, 					Corona::Lua::Open< luaload_kernel_filter_crosshatch_gl > },
	{ kFilterCrystallizeString, 				Corona::Lua::Open< luaload_kernel_filter_crystallize_gl > },
	{ kFilterDesaturateString,					Corona::Lua::Open< luaload_kernel_filter_desaturate_gl > },
	{ kFilterDissolveString, 					Corona::Lua::Open< luaload_kernel_filter_dissolve_gl > },
	{ kFilterDuotoneString, 					Corona::Lua::Open< luaload_kernel_filter_duotone_gl > },
	{ kFilterEmbossString, 						Corona::Lua::Open< luaload_kernel_filter_emboss_gl > },
	{ kFilterExposureString, 					Corona::Lua::Open< luaload_kernel_filter_exposure_gl > },
	{ kFilterFrostedGlassString,				Corona::Lua::Open< luaload_kernel_filter_frostedGlass_gl > },
	{ kFilterGrayscaleString, 					Corona::Lua::Open< luaload_kernel_filter_grayscale_gl > },
	{ kFilterHueString, 						Corona::Lua::Open< luaload_kernel_filter_hue_gl > },
	{ kFilterInvertString, 						Corona::Lua::Open< luaload_kernel_filter_invert_gl > },
	{ kFilterIrisString, 						Corona::Lua::Open< luaload_kernel_filter_iris_gl > },
	{ kFilterLevelsString,						Corona::Lua::Open< luaload_kernel_filter_levels_gl > },
	{ kFilterLinearWipeString, 					Corona::Lua::Open< luaload_kernel_filter_linearWipe_gl > },
	{ kFilterMedianString, 						Corona::Lua::Open< luaload_kernel_filter_median_gl > },
	{ kFilterMonotoneString, 					Corona::Lua::Open< luaload_kernel_filter_monotone_gl > },
	{ kFilterOpTileString,						Corona::Lua::Open< luaload_kernel_filter_opTile_gl > },
	{ kFilterPixelateString, 					Corona::Lua::Open< luaload_kernel_filter_pixelate_gl > },
	{ kFilterPolkaDotsString, 					Corona::Lua::Open< luaload_kernel_filter_polkaDots_gl > },
	{ kFilterPosterizeString, 					Corona::Lua::Open< luaload_kernel_filter_posterize_gl > },
	{ kFilterRadialWipeString, 					Corona::Lua::Open< luaload_kernel_filter_radialWipe_gl > },
	{ kFilterSaturateString, 					Corona::Lua::Open< luaload_kernel_filter_saturate_gl > },
	{ kFilterScatterString, 					Corona::Lua::Open< luaload_kernel_filter_scatter_gl > },
	{ kFilterSepiaString, 						Corona::Lua::Open< luaload_kernel_filter_sepia_gl > },
	{ kFilterSharpenLuminanceString, 			Corona::Lua::Open< luaload_kernel_filter_sharpenLuminance_gl > },
	{ kFilterSobelString,			 			Corona::Lua::Open< luaload_kernel_filter_sobel_gl > },
	{ kFilterStepString,			 			Corona::Lua::Open< luaload_kernel_filter_step_gl > },
	{ kFilterStraightenString,			 		Corona::Lua::Open< luaload_kernel_filter_straighten_gl > },
	{ kFilterSwirlString, 						Corona::Lua::Open< luaload_kernel_filter_swirl_gl > },
	{ kFilterVignetteString, 					Corona::Lua::Open< luaload_kernel_filter_vignette_gl > },
	{ kFilterVignetteMaskString,				Corona::Lua::Open< luaload_kernel_filter_vignetteMask_gl > },
	{ kFilterWaterString, 						Corona::Lua::Open< luaload_kernel_filter_water_gl > },
	{ kFilterWobbleString, 						Corona::Lua::Open< luaload_kernel_filter_wobble_gl > },
	{ kFilterWoodCutString, 					Corona::Lua::Open< luaload_kernel_filter_woodCut_gl > },
	{ kFilterZoomBlurString, 					Corona::Lua::Open< luaload_kernel_filter_zoomBlur_gl > },

	{ NULL, NULL }
};

// ----------------------------------------------------------------------------

// Composite.

static const char kCompositeAddString[] = "add";
static const char kCompositeAverageString[] = "average";
static const char kCompositeColorBurnString[] = "colorBurn";
static const char kCompositeColorDodgeString[] = "colorDodge";
static const char kCompositeDarkenString[] = "darken";
static const char kCompositeDifferenceString[] = "difference";
static const char kCompositeExclusionString[] = "exclusion";
static const char kCompositeHardLightString[] = "hardLight";
static const char kCompositeHardMixString[] = "hardMix";
static const char kCompositeHueString[] = "hue";
static const char kCompositeLightenString[] = "lighten";
static const char kCompositeLuminosityString[] = "luminosity";
static const char kCompositeMultiplyString[] = "multiply";
static const char kCompositeNegationString[] = "negation";
static const char kCompositeOverlayString[] = "overlay";
static const char kCompositePinLightString[] = "pinLight";
static const char kCompositeReflectString[] = "reflect";
static const char kCompositeScreenString[] = "screen";
static const char kCompositeSoftLightString[] = "softLight";
static const char kCompositeSubtractString[] = "subtract";
static const char kCompositeVividLightString[] = "vividLight";
static const char kCompositeYUV420fString[] = "yuv420f";
static const char kCompositeYUV420VString[] = "yuv420v";
/*
			kCompositeColor,
			kCompositeGlow,
			kCompositeLinearLight,
			kCompositeNormalMapWith1DirLight,
			kCompositeNormalMapWith1PointLight,
			kCompositePhoenix,
			kCompositeSaturation,
*/
// Built-in Composites
int luaload_kernel_composite_add_gl( lua_State *L );
int luaload_kernel_composite_average_gl( lua_State *L );
int luaload_kernel_composite_color_gl( lua_State *L );
int luaload_kernel_composite_colorBurn_gl( lua_State *L );
int luaload_kernel_composite_colorDodge_gl( lua_State *L );
int luaload_kernel_composite_darken_gl( lua_State *L );
int luaload_kernel_composite_difference_gl( lua_State *L );
int luaload_kernel_composite_exclusion_gl( lua_State *L );
int luaload_kernel_composite_glow_gl( lua_State *L );
int luaload_kernel_composite_hardLight_gl( lua_State *L );
int luaload_kernel_composite_hardMix_gl( lua_State *L );
int luaload_kernel_composite_hue_gl( lua_State *L );
int luaload_kernel_composite_lighten_gl( lua_State *L );
int luaload_kernel_composite_linearLight_gl( lua_State *L );
int luaload_kernel_composite_luminosity_gl( lua_State *L );
int luaload_kernel_composite_multiply_gl( lua_State *L );
int luaload_kernel_composite_negation_gl( lua_State *L );
int luaload_kernel_composite_overlay_gl( lua_State *L );
int luaload_kernel_composite_normalMapWith1DirLight_gl( lua_State *L );
int luaload_kernel_composite_normalMapWith1PointLight_gl( lua_State *L );
int luaload_kernel_composite_phoenix_gl( lua_State *L );
int luaload_kernel_composite_pinLight_gl( lua_State *L );
int luaload_kernel_composite_reflect_gl( lua_State *L );
int luaload_kernel_composite_saturation_gl( lua_State *L );
int luaload_kernel_composite_screen_gl( lua_State *L );
int luaload_kernel_composite_softLight_gl( lua_State *L );
int luaload_kernel_composite_subtract_gl( lua_State *L );
int luaload_kernel_composite_vividLight_gl( lua_State *L );
int luaload_kernel_composite_yuv420f_gl(lua_State* L);
int luaload_kernel_composite_yuv420v_gl(lua_State* L);

static const luaL_Reg kBuiltInCompositeFuncs[] =
{
	{ kCompositeAddString,							Corona::Lua::Open< luaload_kernel_composite_add_gl > },
	{ kCompositeAverageString,						Corona::Lua::Open< luaload_kernel_composite_average_gl > },
	{ kCompositeColorBurnString,					Corona::Lua::Open< luaload_kernel_composite_colorBurn_gl > },
	{ kCompositeColorDodgeString,					Corona::Lua::Open< luaload_kernel_composite_colorDodge_gl > },
	{ kCompositeColorString,						Corona::Lua::Open< luaload_kernel_composite_color_gl > },
	{ kCompositeDarkenString,						Corona::Lua::Open< luaload_kernel_composite_darken_gl > },
	{ kCompositeDifferenceString,					Corona::Lua::Open< luaload_kernel_composite_difference_gl > },
	{ kCompositeExclusionString,					Corona::Lua::Open< luaload_kernel_composite_exclusion_gl > },
	{ kCompositeGlowString,							Corona::Lua::Open< luaload_kernel_composite_glow_gl > },
	{ kCompositeHardLightString,					Corona::Lua::Open< luaload_kernel_composite_hardLight_gl > },
	{ kCompositeHardMixString,						Corona::Lua::Open< luaload_kernel_composite_hardMix_gl > },
	{ kCompositeHueString,							Corona::Lua::Open< luaload_kernel_composite_hue_gl > },
	{ kCompositeLightenString,						Corona::Lua::Open< luaload_kernel_composite_lighten_gl > },
	{ kCompositeLinearLightString,					Corona::Lua::Open< luaload_kernel_composite_linearLight_gl > },
	{ kCompositeLuminosityString,					Corona::Lua::Open< luaload_kernel_composite_luminosity_gl > },
	{ kCompositeMultiplyString,						Corona::Lua::Open< luaload_kernel_composite_multiply_gl > },
	{ kCompositeNegationString,						Corona::Lua::Open< luaload_kernel_composite_negation_gl > },
	{ kCompositeOverlayString,						Corona::Lua::Open< luaload_kernel_composite_overlay_gl > },
	{ kCompositeNormalMapWith1DirLightString,		Corona::Lua::Open< luaload_kernel_composite_normalMapWith1DirLight_gl > },
	{ kCompositeNormalMapWith1PointLightString,		Corona::Lua::Open< luaload_kernel_composite_normalMapWith1PointLight_gl > },
	{ kCompositePhoenixString,						Corona::Lua::Open< luaload_kernel_composite_phoenix_gl > },
	{ kCompositePinLightString,						Corona::Lua::Open< luaload_kernel_composite_pinLight_gl > },
	{ kCompositeReflectString,						Corona::Lua::Open< luaload_kernel_composite_reflect_gl > },
	{ kCompositeSaturationString,					Corona::Lua::Open< luaload_kernel_composite_saturation_gl > },
	{ kCompositeScreenString,						Corona::Lua::Open< luaload_kernel_composite_screen_gl > },
	{ kCompositeSoftLightString,					Corona::Lua::Open< luaload_kernel_composite_softLight_gl > },
	{ kCompositeSubtractString,						Corona::Lua::Open< luaload_kernel_composite_subtract_gl > },
	{ kCompositeVividLightString,					Corona::Lua::Open< luaload_kernel_composite_vividLight_gl > },
	{ kCompositeYUV420fString,						Corona::Lua::Open< luaload_kernel_composite_yuv420f_gl > },
#if defined( Rtt_NXS_ENV )
	{ kCompositeYUV420VString,						Corona::Lua::Open< luaload_kernel_composite_yuv420v_gl > },
#endif

	{ NULL, NULL }
};

// ----------------------------------------------------------------------------

// Generator.

int luaload_kernel_generator_checkerboard_gl( lua_State *L );
int luaload_kernel_generator_lenticularHalo_gl( lua_State *L );
int luaload_kernel_generator_linearGradient_gl( lua_State *L );
int luaload_kernel_generator_marchingAnts_gl( lua_State *L );
int luaload_kernel_generator_perlinNoise_gl( lua_State *L );
int luaload_kernel_generator_radialGradient_gl( lua_State *L );
int luaload_kernel_generator_random_gl( lua_State *L );
int luaload_kernel_generator_stripes_gl( lua_State *L );
int luaload_kernel_generator_sunbeams_gl( lua_State *L );

static const luaL_Reg kBuiltInGeneratorFuncs[] =
{
	{ kGeneratorCheckerboardString,		Corona::Lua::Open< luaload_kernel_generator_checkerboard_gl > },
	{ kGeneratorLenticularHaloString,	Corona::Lua::Open< luaload_kernel_generator_lenticularHalo_gl > },
	{ kGeneratorLinearGradientString, 	Corona::Lua::Open< luaload_kernel_generator_linearGradient_gl > },
	{ kGeneratorMarchingAntsString, 	Corona::Lua::Open< luaload_kernel_generator_marchingAnts_gl > },
	{ kGeneratorPerlinNoiseString, 		Corona::Lua::Open< luaload_kernel_generator_perlinNoise_gl > },
	{ kGeneratorRadialGradientString,	Corona::Lua::Open< luaload_kernel_generator_radialGradient_gl > },
	{ kGeneratorRandomString, 			Corona::Lua::Open< luaload_kernel_generator_random_gl > },
	{ kGeneratorStripesString, 			Corona::Lua::Open< luaload_kernel_generator_stripes_gl > },
	{ kGeneratorSunbeamsString, 		Corona::Lua::Open< luaload_kernel_generator_sunbeams_gl > },

	{ NULL, NULL }
};

// ----------------------------------------------------------------------------

void
ShaderBuiltin::Register( lua_State *L, ShaderTypes::Category category )
{
	const luaL_Reg *funcs = NULL;

	switch ( category )
	{
		case ShaderTypes::kCategoryFilter:
			funcs = kBuiltInFilterFuncs;
			break;
		case ShaderTypes::kCategoryComposite:
			funcs = kBuiltInCompositeFuncs;
			break;
		case ShaderTypes::kCategoryGenerator:
			funcs = kBuiltInGeneratorFuncs;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	if ( funcs )
	{
		luaL_register( L, NULL, funcs );
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
