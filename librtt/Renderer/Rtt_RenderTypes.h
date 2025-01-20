//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_RenderTypes_H__
#define _Rtt_RenderTypes_H__

#include "Core/Rtt_Assert.h"
#include "Core/Rtt_Types.h"

#include "Renderer/Rtt_Texture.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

struct BlendMode;

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

	void Clear()
	{
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	}
	
	void ModulateAlpha( U8 alpha );
	void PremultiplyAlpha();
};

typedef U32 Color;

Color ColorZero();
Color ColorWhite();

// ----------------------------------------------------------------------------

typedef union
{
	Color	pixel;
	RGBA	rgba;
	U8		channels[4];
}
ColorUnion;

Rtt_STATIC_ASSERT( sizeof( ColorUnion ) == sizeof( Color ) );

struct ColorOffset
{
	S8 r;
	S8 g;
	S8 b;
	S8 a;
};

// ----------------------------------------------------------------------------

class RenderTypes
{
	public:
		typedef enum _BlendEquation
		{
			kAddEquation = 0, // Default
			kSubtractEquation,
			kReverseSubtractEquation,

			kDisabledEquation, // Special case, so make this last

			kNumEquations
		}
		BlendEquation;

	public:
		// TODO: Move this into BlendMode and rename as BlendMode::Preset
		typedef enum _BlendType
		{
			// Standard Preset
			kNormal = 0,
			kAdditive,
			kScreen,
			kMultiply,
			
			// PorterDuff Presets
			kClear,
			kSrc,
			kDst,
			kSrcOver,
			kDstOver,
			kSrcIn,
			kDstIn,
			kSrcOut,
			kDstOut,
			kSrcAtop,
			kDstAtop,
			kXor,
			
			// Not using built-in preset
			kCustom,

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
		typedef enum _TextureWrap
		{
			kClampToEdgeWrap = 0,
			kRepeatWrap,
			kMirroredRepeatWrap,

			kNumWraps
		}
		TextureWrap;

	public:
		static const char* StringForBlendType( BlendType value );
		static BlendType BlendTypeForString( const char *str );
		static bool IsRestrictedBlendType( BlendType value );

		static BlendType PorterDuffForBlendMode( const BlendMode& mode );
		static BlendType BlendTypeForBlendMode( const BlendMode& mode, bool isPremultiplied );

		static const char* StringForBlendEquation( BlendEquation eq );
		static BlendEquation BlendEquationForString( const char *str );

		static TextureFilter TextureFilterForString( const char *str );
		static const char* StringForTextureFilter( TextureFilter filter );

		static TextureWrap TextureWrapForString( const char *str );
		static const char* StringForTextureWrap( TextureWrap wrap );

		static RenderTypes::TextureFilter Convert( Texture::Filter filter );
		static Texture::Filter Convert( RenderTypes::TextureFilter filter );

		static RenderTypes::TextureWrap Convert( Texture::Wrap wrap );
		static Texture::Wrap Convert( RenderTypes::TextureWrap wrap );
};

// ----------------------------------------------------------------------------

// TODO: Move to separate file
struct BlendMode
{
	typedef enum _Param
	{
		kZero = 0,
		kOne,
		kSrcColor,
		kOneMinusSrcColor,
		kDstColor,
		kOneMinusDstColor,
		kSrcAlpha,
		kOneMinusSrcAlpha,
		kDstAlpha,
		kOneMinusDstAlpha,
		kSrcAlphaSaturate,
		
		kNumParams,
		kUnknown
	}
	Param;

	static const char* StringForParam( Param param );
	static Param ParamForString( const char *str );
	static Param ParamForGLenum( int gl_enum );

	BlendMode();
	BlendMode( RenderTypes::BlendType value, bool isPremultiplied );
	BlendMode( Param srcColor, Param dstColor, Param srcAlpha, Param dstAlpha );

	bool operator==( const BlendMode& rhs ) const;
	bool operator!=( const BlendMode& rhs ) const { return ! operator==( rhs ); }

	Param fSrcColor;
	Param fDstColor;
	Param fSrcAlpha;
	Param fDstAlpha;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_RenderTypes_H__
