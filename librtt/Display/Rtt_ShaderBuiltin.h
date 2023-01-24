//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ShaderBuiltin_H__
#define _Rtt_ShaderBuiltin_H__

#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderTypes.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

class ShaderBuiltin
{
	public:
		typedef enum _Filter
		{
			// Filter.

			kFilterBloom,
			kFilterBlur,
			kFilterBlurGaussian,
			kFilterBlurHorizontal,
			kFilterBlurVertical,
			kFilterBlurGaussianLinear,
			kFilterBlurLinearHorizontal,
			kFilterBlurLinearVertical,
			kFilterBrightness,
			kFilterBulge,
			kFilterChromaKey,
			kFilterColor,
			kFilterColorChannelOffset,
			kFilterColorMatrix,
			kFilterColorPolynomial,
			kFilterContrast,
			kFilterCrosshatch,
			kFilterCrystallize,
			kFilterDesaturate,
			kFilterDissolve,
			kFilterDuotone,
			kFilterEmboss,
			kFilterExposure,
			kFilterFrostedGlass,
			kFilterGrayscale,
			kFilterHue,
			kFilterInvert,
			kFilterIris,
			kFilterLevels,
			kFilterLinearWipe,
			kFilterMedian,
			kFilterMonotone,
			kFilterOpTile,
			kFilterPixelate,
			kFilterPolkaDots,
			kFilterPosterize,
			kFilterRadialWipe,
			kFilterSaturate,
			kFilterScatter,
			kFilterSepia,
			kFilterSharpenLuminance,
			kFilterSobel,
			kFilterStep,
			kFilterStraighten,
			kFilterSwirl,
			kFilterVignette,
			kFilterVignetteMask,
			kFilterWater,
			kFilterWobble,
			kFilterWoodCut,
			kFilterZoomBlur,

			//! TODO: FOR THE SAKE OF CONSISTENCY, FIX NAMING OF THESE BY
			//! RENAMING ALL INSTANCES OF "FILTER" IN THIS FILE TO "EFFECT"!!!

			// Composite.

			kCompositeColor,
			kCompositeGlow,
			kCompositeLinearLight,
			kCompositeNormalMapWith1DirLight,
			kCompositeNormalMapWith1PointLight,
			kCompositePhoenix,
			kCompositeSaturation,

			// Generator.

			kGeneratorCheckerboard,
			kGeneratorLenticularHalo,
			kGeneratorLinearGradient,
			kGeneratorMarchingAnts,
			kGeneratorPerlinNoise,
			kGeneratorRadialGradient,
			kGeneratorRandom,
			kGeneratorStripes,
			kGeneratorSunbeams,

			kNumFilters,
		}
		Filter;
		
		static const char *StringForFilter( Filter filter );

	public:
		static int AppendFilters( lua_State *L, int index );
		static int Append( lua_State *L, int index, ShaderTypes::Category category );

	public:
		static bool FilterExists( const char *name );
		static bool Exists( ShaderTypes::Category category, const char *name );

	public:
		static bool PushDefaultShell( lua_State *L );
		static bool PushDefaultKernel( lua_State *L );
	
		static const char *KeyForCategory( ShaderTypes::Category category );
	
		// Registers (built-in) Builtin loaders on the table at the top of the stack
		static void Register( lua_State *L, ShaderTypes::Category category );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShaderBuiltin_H__
