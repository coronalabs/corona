//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#if 0

#ifndef _Rtt_Rendering_H__
#define _Rtt_Rendering_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

struct RGBA
{
	#if 0 //def Rtt_BIG_ENDIAN
		U8 a;
		U8 b;
		U8 g;
		U8 r;
	#else
		U8 r;
		U8 g;
		U8 b;
		U8 a;
	#endif
};

typedef U32 Color;

typedef union
{
	Color	pixel;
	RGBA	rgba;
	U8		channels[4];
}
ColorUnion;

Rtt_STATIC_ASSERT( sizeof( ColorUnion ) == sizeof( Color ) );

// ----------------------------------------------------------------------------

class Rendering
{
	public:
		typedef enum _Mode
		{
			kFan = 0,
			kStrip = 1,
			kTriangle = 2
		}
		Mode;

		typedef enum _BlendMode
		{
			kUnknownBlendMode = -1,
			kNormalNonPremultiplied = 0,
			kNormalPremultiplied,
			kAdditiveNonPremultiplied,
			kAdditivePremultiplied,
			kScreenNonPremultiplied, // This is not supported on OGL 1.x
			kScreenPremultiplied,
			kMultiplyNonPremultiplied,
			kMultiplyPremultiplied,

			// Destination alpha: use dst alpha channel as coverage mask for 
			// incoming (src) fragments. The following will modify the dst alpha
			// channel depending on the incoming triangles that are drawn.
			kDstAlphaZero, // zero's dst alpha based on incoming fragment
			kDstAlphaAccumulate, // accumulates samples in dst alpha channel
			kDstAlphaModulateSrc, // modulates src color by dst alpha
			kDstAlphaBlit, // blits incoming fragment using dst alpha as a coverage mask

			kNumBlendModes
		}
		BlendMode;

	public:
		typedef enum _BlendType
		{
			kNormal = 0,
			kAdditive,
			kScreen,
			kMultiply,

			kNumBlendTypes
		}
		BlendType;

	public:
		typedef enum _TextureFilter
		{
			kLinearTextureFilter = 0,
			kNearestTextureFilter,

			kNumTextureFilters
		}
		TextureFilter;

	public:
		static const char* StringForBlendType( BlendType value );
		static BlendType BlendTypeForString( const char *str );

		// By default, returns the premultiplied blend mode
		static BlendMode BlendModeForBlendType( BlendType value, bool isPremultiplied );

		static TextureFilter TextureFilterForString( const char *str );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Rendering_H__

#endif