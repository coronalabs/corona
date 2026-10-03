//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Renderer/Rtt_GL.h"
#include "Rtt_RenderTypes.h"

#ifdef Rtt_DEBUG
	#include "Corona/CoronaGraphics.h" /* validate some bit counts */
#endif

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

void
RGBA::ModulateAlpha( U8 alpha )
{
	// Modulate the receiver's alpha channel by 'alpha'
	if ( alpha < 0xFF )
	{
		this->a = (((U16)this->a) * alpha) >> 8;
	}
}

void
RGBA::PremultiplyAlpha()
{
	U8 alpha = this->a;

	if ( alpha < 0xFF )
	{
		// r,g,b is pre-multiplied by alpha
		this->r = (((U16)this->r) * alpha) >> 8;
		this->g = (((U16)this->g) * alpha) >> 8;
		this->b = (((U16)this->b) * alpha) >> 8;
	}
}

// ----------------------------------------------------------------------------

Color kColorZero = 0;
Color kColorWhite = ~kColorZero;

Color ColorZero()
{
	return kColorZero;
}

Color ColorWhite()
{
	return kColorWhite;
}

// ----------------------------------------------------------------------------

static const char kNormalBlendType[] = "normal";
static const char kAdditiveBlendType[] = "add";
static const char kScreenBlendType[] = "screen";
static const char kMultiplyBlendType[] = "multiply";
static const char kCustomBlendType[] = "custom";

// PorterDuff
static const char kClearType[] = "clear";
static const char kSrcType[] = "src";
static const char kDstType[] = "dst";
static const char kSrcOverType[] = "srcOver";
static const char kDstOverType[] = "dstOver";
static const char kSrcInType[] = "srcIn";
static const char kDstInType[] = "dstIn";
static const char kSrcOutType[] = "srcOut";
static const char kDstOutType[] = "dstOut";
static const char kSrcAtopType[] = "srcAtop";
static const char kDstAtopType[] = "dstAtop";
static const char kXorType[] = "xor";

const char*
RenderTypes::StringForBlendType( BlendType value )
{
	const char *result = kNormalBlendType;

	switch ( value )
	{
		case kAdditive:
			result = kAdditiveBlendType;
			break;
		case kScreen:
			result = kScreenBlendType;
			break;
		case kMultiply:
			result = kMultiplyBlendType;
			break;
		case kCustom:
			result = kCustomBlendType;
			break;
		case kClear:
			result = kClearType;
			break;
		case kSrc:
			result = kSrcType;
			break;
		case kDst:
			result = kDstType;
			break;
		case kSrcOver:
			result = kSrcOverType;
			break;
		case kDstOver:
			result = kDstOverType;
			break;
		case kSrcIn:
			result = kSrcInType;
			break;
		case kDstIn:
			result = kDstInType;
			break;
		case kSrcOut:
			result = kSrcOutType;
			break;
		case kDstOut:
			result = kDstOutType;
			break;
		case kSrcAtop:
			result = kSrcAtopType;
			break;
		case kDstAtop:
			result = kDstAtopType;
			break;
		case kXor:
			result = kXorType;
			break;
		default:
			break;
	}

	return result;
}

RenderTypes::BlendType
RenderTypes::BlendTypeForString( const char *str )
{
	BlendType result = kNormal;

	if ( str )
	{
		if ( 0 == strcmp( str, kAdditiveBlendType ) )
		{
			result = kAdditive;
		}
		else if ( 0 == strcmp( str, kScreenBlendType ) )
		{
			result = kScreen;
		}
		else if ( 0 == strcmp( str, kMultiplyBlendType ) )
		{
			result = kMultiply;
		}
		else if ( 0 == strcmp( str, kCustomBlendType ) )
		{
			result = kCustom;
		}
		else if ( 0 == strcmp( str, kClearType ) )
		{
			result = kClear;
		}
		else if ( 0 == strcmp( str, kSrcType ) )
		{
			result = kSrc;
		}
		else if ( 0 == strcmp( str, kDstType ) )
		{
			result = kDst;
		}
		else if ( 0 == strcmp( str, kSrcOverType ) )
		{
			result = kSrcOver;
		}
		else if ( 0 == strcmp( str, kDstOverType ) )
		{
			result = kDstOver;
		}
		else if ( 0 == strcmp( str, kSrcInType ) )
		{
			result = kSrcIn;
		}
		else if ( 0 == strcmp( str, kDstInType ) )
		{
			result = kDstIn;
		}
		else if ( 0 == strcmp( str, kSrcOutType ) )
		{
			result = kSrcOut;
		}
		else if ( 0 == strcmp( str, kDstOutType ) )
		{
			result = kDstOut;
		}
		else if ( 0 == strcmp( str, kSrcAtopType ) )
		{
			result = kSrcAtop;
		}
		else if ( 0 == strcmp( str, kDstAtopType ) )
		{
			result = kDstAtop;
		}
		else if ( 0 == strcmp( str, kXorType ) )
		{
			result = kXor;
		}
	}

	return result;
}

bool
RenderTypes::IsRestrictedBlendType( BlendType value )
{
	bool result = true;


	switch ( value )
	{
		case kNormal:
		case kAdditive:
		case kScreen:
		case kMultiply:
			result = false;
			break;
		default:
			break;
	}
	
	return result;
}

// PorterDuff only operates in premultiplied.
// NOTE: For non-premultiplied, we use the premultiplied values
RenderTypes::BlendType
RenderTypes::PorterDuffForBlendMode( const BlendMode& mode )
{
	RenderTypes::BlendType result = kCustom;

	if ( mode.fSrcColor == mode.fSrcAlpha
		 && mode.fDstColor == mode.fDstAlpha )
	{
		static const BlendMode kClearMode( kClear, true );
		static const BlendMode kSrcMode( kSrc, true );
		static const BlendMode kDstMode( kDst, true );
		static const BlendMode kSrcOverMode( kSrcOver, true );
		static const BlendMode kDstOverMode( kDstOver, true );
		static const BlendMode kSrcInMode( kSrcIn, true );
		static const BlendMode kDstInMode( kDstIn, true );
		static const BlendMode kSrcOutMode( kSrcOut, true );
		static const BlendMode kDstOutMode( kDstOut, true );
		static const BlendMode kSrcAtopMode( kSrcAtop, true );
		static const BlendMode kDstAtopMode( kDstAtop, true );
		static const BlendMode kXorMode( kXor, true );

		if ( kClearMode == mode )
		{
			result = kClear;
		}
		else if ( kSrcMode == mode )
		{
			result = kSrc;
		}
		else if ( kDstMode == mode )
		{
			result = kDst;
		}
		else if ( kSrcOverMode == mode )
		{
			result = kSrcOver;
		}
		else if ( kDstOverMode == mode )
		{
			result = kDstOver;
		}
		else if ( kSrcInMode == mode )
		{
			result = kSrcIn;
		}
		else if ( kDstInMode == mode )
		{
			result = kDstIn;
		}
		else if ( kSrcOutMode == mode )
		{
			result = kSrcOut;
		}
		else if ( kDstOutMode == mode )
		{
			result = kDstOut;
		}
		else if ( kSrcAtopMode == mode )
		{
			result = kSrcAtop;
		}
		else if ( kDstAtopMode == mode )
		{
			result = kDstAtop;
		}
		else if ( kXorMode == mode )
		{
			result = kXor;
		}
	}

	return kCustom;
}

RenderTypes::BlendType
RenderTypes::BlendTypeForBlendMode( const BlendMode& mode, bool isPremultiplied )
{
	BlendType result = kCustom;

	if ( isPremultiplied )
	{
		static const BlendMode kNormalMode( kNormal, true );
		static const BlendMode kAddMode( kAdditive, true );
		static const BlendMode kScreenMode( kScreen, true );
		static const BlendMode kMultiplyMode( kMultiply, true );

		if ( kNormalMode == mode )
		{
			result = kNormal;
		}
		else if ( kAddMode == mode )
		{
			result = kAdditive;
		}
		else if ( kScreenMode == mode )
		{
			result = kScreen;
		}
		else if ( kMultiplyMode == mode )
		{
			result = kMultiply;
		}
		else
		{
			result = PorterDuffForBlendMode( mode );
		}
	}
	else
	{
		static const BlendMode kNormalMode( kNormal, false );
		static const BlendMode kAddMode( kAdditive, false );
		static const BlendMode kScreenMode( kScreen, false );
		static const BlendMode kMultiplyMode( kMultiply, false );

		if ( kNormalMode == mode )
		{
			result = kNormal;
		}
		else if ( kAddMode == mode )
		{
			result = kAdditive;
		}
		else if ( kScreenMode == mode )
		{
			result = kScreen;
		}
		else if ( kMultiplyMode == mode )
		{
			result = kMultiply;
		}
		else
		{
			result = PorterDuffForBlendMode( mode );
		}
	}

	return result;
}

// ----------------------------------------------------------------------------
/*
RenderTypes::BlendMode
RenderTypes::BlendModeForBlendType( BlendType value, bool isPremultiplied )
{
	RenderTypes::BlendMode result = kDisabled;

	if ( isPremultiplied )
	{
		switch ( value )
		{
			case kAdditive:
				result = kAdditivePremultiplied;
				break;
			case kScreen:
				result = kScreenPremultiplied;
				break;
			case kMultiply:
				result = kMultiplyPremultiplied;
				break;
			default:
				result = kNormalPremultiplied;
				break;
		}
	}
	else
	{
		switch ( value )
		{
			case kAdditive:
				result = kAdditiveNonPremultiplied;
				break;
			case kScreen:
				result = kScreenNonPremultiplied;
				break;
			case kMultiply:
				result = kMultiplyNonPremultiplied;
				break;
			default:
				result = kNormalNonPremultiplied;
				break;
		}
	}

	return result;
}

static const char kDisabledString[] = "Disabled";
static const char kNormalNonPremultipliedString[] = "Normal (Non-Premultiplied)";
static const char kNormalPremultipliedString[] = "Normal (Premultiplied)";
static const char kAdditiveNonPremultipliedString[] = "Additive (Non-Premultiplied)";
static const char kAdditivePremultipliedString[] = "Additive (Premultiplied)";
static const char kScreenNonPremultipliedString[] = "Screen (Non-Premultiplied)";
static const char kScreenPremultipliedString[] = "Screen (Premultiplied)";
static const char kMultiplyNonPremultipliedString[] = "Multiply (Non-Premultiplied)";
static const char kMultiplyPremultipliedString[] = "Multiply (Premultiplied)";


const char*
RenderTypes::StringForBlendMode( BlendMode mode )
{
	const char *result = NULL;

	switch ( mode )
	{
		case kDisabled:
			result = kDisabledString;
			break;
		case kNormalNonPremultiplied:
			result = kNormalNonPremultipliedString;
			break;
		case kNormalPremultiplied:
			result = kNormalNonPremultipliedString;
			break;
		case kAdditiveNonPremultiplied:
			result = kAdditiveNonPremultipliedString;
			break;
		case kAdditivePremultiplied:
			result = kAdditivePremultipliedString;
			break;
		case kScreenNonPremultiplied:
			result = kScreenNonPremultipliedString;
			break;
		case kScreenPremultiplied:
			result = kScreenPremultipliedString;
			break;
		case kMultiplyNonPremultiplied:
			result = kMultiplyNonPremultipliedString;
			break;
		case kMultiplyPremultiplied:
			result = kMultiplyPremultipliedString;
			break;
		default:
			break;
	}

	return result;
}

bool
RenderTypes::BlendModeIsPremultiplied( BlendMode mode )
{
	bool result = false;

	switch ( mode )
	{
		case kNormalPremultiplied:
		case kAdditivePremultiplied:
		case kScreenPremultiplied:
		case kMultiplyPremultiplied:
			result = true;
			break;

		default:
			break;
	}

	return result;
}
*/

static const char kDisabledEquationKey[] = "disabled";
static const char kAddEquationKey[] = "add";
static const char kSubtractEquationKey[] = "subtract";
static const char kReverseSubtractEquationKey[] = "reverseSubtract";

const char*
RenderTypes::StringForBlendEquation( BlendEquation eq )
{
	const char *result = kAddEquationKey;

	switch ( eq )
	{
		case kDisabledEquation:
			result = kDisabledEquationKey;
			break;
		case kSubtractEquation:
			result = kSubtractEquationKey;
			break;
		case kReverseSubtractEquation:
			result = kReverseSubtractEquationKey;
			break;
		default:
			break;
	}

	return result;
}

RenderTypes::BlendEquation
RenderTypes::BlendEquationForString( const char *str )
{
	BlendEquation result = kAddEquation;

	if ( str )
	{
		if ( 0 == strcmp( str, kSubtractEquationKey ) )
		{
			result = kSubtractEquation;
		}
		else if ( 0 == strcmp( str, kReverseSubtractEquationKey ) )
		{
			result = kReverseSubtractEquation;
		}
		else if ( 0 == strcmp( str, kDisabledEquationKey ) )
		{
			result = kDisabledEquation;
		}
	}

	return result;
}

// ----------------------------------------------------------------------------

static const char kLinearTextureFilterKey[] = "linear";
static const char kNearestTextureFilterKey[] = "nearest";

RenderTypes::TextureFilter
RenderTypes::TextureFilterForString( const char *str )
{
	TextureFilter result = kLinearTextureFilter;

	if ( str )
	{
		if ( 0 == strcmp( str, kNearestTextureFilterKey ) )
		{
			result = kNearestTextureFilter;
		}
	}

	return result;
}

const char*
RenderTypes::StringForTextureFilter( TextureFilter filter )
{
	const char *result = kLinearTextureFilterKey;

	switch ( filter )
	{
		case kNearestTextureFilter:
			result = kNearestTextureFilterKey;
			break;
		default:
			break;
	}

	return result;
}

// ----------------------------------------------------------------------------

static const char kClampToEdgeWrapKey[] = "clampToEdge";
static const char kRepeatWrapKey[] = "repeat";
static const char kMirroredRepeatWrapKey[] = "mirroredRepeat";

RenderTypes::TextureWrap
RenderTypes::TextureWrapForString( const char *str )
{
	TextureWrap result = kClampToEdgeWrap;

	if ( str )
	{
		if ( 0 == strcmp( str, kRepeatWrapKey ) )
		{
			result = kRepeatWrap;
		}
		else if ( 0 == strcmp( str, kMirroredRepeatWrapKey ) )
		{
			result = kMirroredRepeatWrap;
		}
	}

	return result;
}

const char *
RenderTypes::StringForTextureWrap( TextureWrap wrap )
{
	const char *result = kClampToEdgeWrapKey;

	switch ( wrap )
	{
		case kRepeatWrap:
			result = kRepeatWrapKey;
			break;
		case kMirroredRepeatWrap:
			result = kMirroredRepeatWrapKey;
			break;
		default:
			break;
	}

	return result;
}

// ----------------------------------------------------------------------------

RenderTypes::TextureFilter
RenderTypes::Convert( Texture::Filter filter )
{
	RenderTypes::TextureFilter result = RenderTypes::kLinearTextureFilter;

	switch ( filter )
	{
		case Texture::kNearest:
			result = RenderTypes::kNearestTextureFilter;
			break;
		case Texture::kLinear:
			result = RenderTypes::kLinearTextureFilter;
			break;
		default:
			Rtt_ASSERT_NOT_IMPLEMENTED();
			break;
	}

	return result;
}

Texture::Filter
RenderTypes::Convert( RenderTypes::TextureFilter filter )
{
	Texture::Filter result = Texture::kLinear;

	switch ( filter )
	{
		case RenderTypes::kNearestTextureFilter:
			result = Texture::kNearest;
			break;
		case RenderTypes::kLinearTextureFilter:
			result = Texture::kLinear;
			break;
		default:
			Rtt_ASSERT_NOT_IMPLEMENTED();
			break;
	}

	return result;
}

// ----------------------------------------------------------------------------

RenderTypes::TextureWrap
RenderTypes::Convert( Texture::Wrap wrap )
{
	RenderTypes::TextureWrap result = RenderTypes::kClampToEdgeWrap;

	switch ( wrap )
	{
		case Texture::kClampToEdge:
			result = RenderTypes::kClampToEdgeWrap;
			break;
		case Texture::kRepeat:
			result = RenderTypes::kRepeatWrap;
			break;
		case Texture::kMirroredRepeat:
			result = RenderTypes::kMirroredRepeatWrap;
			break;
		default:
			Rtt_ASSERT_NOT_IMPLEMENTED();
			break;
	}

	return result;
}

Texture::Wrap
RenderTypes::Convert( RenderTypes::TextureWrap wrap )
{
	Texture::Wrap result = Texture::kClampToEdge;

	switch ( wrap )
	{
		case RenderTypes::kClampToEdgeWrap:
			result = Texture::kClampToEdge;
			break;
		case RenderTypes::kRepeatWrap:
			result = Texture::kRepeat;
			break;
		case RenderTypes::kMirroredRepeatWrap:
			result = Texture::kMirroredRepeat;
			break;
		default:
			Rtt_ASSERT_NOT_IMPLEMENTED();
			break;
	}

	return result;
}

// ----------------------------------------------------------------------------

static void GetBlendParamsPremultipliedPorterDuff(
	RenderTypes::BlendType value,
	BlendMode::Param& outSrc, BlendMode::Param& outDst,
	BlendMode::Param& outSrcAlpha, BlendMode::Param& outDstAlpha )
{
	switch ( value )
	{
		case RenderTypes::kClear:
			outSrc = outSrcAlpha = BlendMode::kZero;
			outDst = outDstAlpha = BlendMode::kZero;
			break;

		case RenderTypes::kSrc:
			outSrc = outSrcAlpha = BlendMode::kOne;
			outDst = outDstAlpha = BlendMode::kZero;
			break;
		case RenderTypes::kDst:
			outSrc = outSrcAlpha = BlendMode::kZero;
			outDst = outDstAlpha = BlendMode::kOne;
			break;
		case RenderTypes::kSrcOver:
			outSrc = outSrcAlpha = BlendMode::kOne;
			outDst = outDstAlpha = BlendMode::kOneMinusSrcAlpha;
			break;
		case RenderTypes::kDstOver:
			outSrc = outSrcAlpha = BlendMode::kOneMinusDstAlpha;
			outDst = outDstAlpha = BlendMode::kOne;
			break;

		case RenderTypes::kSrcIn:
			outSrc = outSrcAlpha = BlendMode::kDstAlpha;
			outDst = outDstAlpha = BlendMode::kZero;
			break;
		case RenderTypes::kDstIn:
			outSrc = outSrcAlpha = BlendMode::kZero;
			outDst = outDstAlpha = BlendMode::kSrcAlpha;
			break;

		case RenderTypes::kSrcOut:
			outSrc = outSrcAlpha = BlendMode::kOneMinusDstAlpha;
			outDst = outDstAlpha = BlendMode::kZero;
			break;
		case RenderTypes::kDstOut:
			outSrc = outSrcAlpha = BlendMode::kZero;
			outDst = outDstAlpha = BlendMode::kOneMinusSrcAlpha;
			break;

		case RenderTypes::kSrcAtop:
			outSrc = outSrcAlpha = BlendMode::kDstAlpha;
			outDst = outDstAlpha = BlendMode::kOneMinusSrcAlpha;
			break;
		case RenderTypes::kDstAtop:
			outSrc = outSrcAlpha = BlendMode::kOneMinusDstAlpha;
			outDst = outDstAlpha = BlendMode::kSrcAlpha;
			break;

		case RenderTypes::kXor:
			outSrc = outSrcAlpha = BlendMode::kOneMinusDstAlpha;
			outDst = outDstAlpha = BlendMode::kOneMinusSrcAlpha;
			break;
			
		default:
			// Caller should only call if they know its a PorterDuff blend type
			Rtt_ASSERT_NOT_REACHED();
			break;
	}
}

// ----------------------------------------------------------------------------

static void GetBlendParamsPremultiplied(
	RenderTypes::BlendType value,
	BlendMode::Param& outSrc, BlendMode::Param& outDst,
	BlendMode::Param& outSrcAlpha, BlendMode::Param& outDstAlpha )
{
	switch ( value )
	{
		case RenderTypes::kAdditive:
			// Additive: pixel = src + dst
			// rgb = (aSrc*[rSrc,gSrc,bSrc])*kOne + (rDst,gDst,bDst,aDst)*kOne
			// a   = (aSrc)                 *kOne + (aDst)               *kOne
			outSrc = BlendMode::kOne;
			outDst = BlendMode::kOne;
			outSrcAlpha = BlendMode::kOne;
			outDstAlpha = BlendMode::kOne;
			break;
		case RenderTypes::kScreen:
			// Screen: pixel = 1 - (1-src)*(1-dst) = src + dst*(1-src)
			// rgb = (aSrc*[rSrc,gSrc,bSrc])*kOne + (rDst,gDst,bDst)*(kOneMinusSrcColor)
			// a   = (aSrc)                 *kOne + (aDst)          *(kOneMinusSrcColor)
			outSrc = BlendMode::kOne;
			outDst = BlendMode::kOneMinusSrcColor;
			outSrcAlpha = BlendMode::kOne;
			outDstAlpha = BlendMode::kOneMinusSrcColor;
			break;
		case RenderTypes::kMultiply:
			// Multiply: pixel = src * dst
			// rgb = (aSrc*[rSrc,gSrc,bSrc])*kDstColor + (rDst,gDst,bDst,aDst)*(1-aSrc)
			outSrc = BlendMode::kDstColor;
			outDst = BlendMode::kOneMinusSrcAlpha;
			outSrcAlpha = BlendMode::kDstColor;
			outDstAlpha = BlendMode::kOneMinusSrcAlpha;
			break;
		case RenderTypes::kClear:
			outSrc = outSrcAlpha = BlendMode::kZero;
			outDst = outDstAlpha = BlendMode::kZero;
			break;
		case RenderTypes::kSrc:
			outSrc = outSrcAlpha = BlendMode::kOne;
			outDst = outDstAlpha = BlendMode::kZero;
			break;
		case RenderTypes::kDst:
			outSrc = outSrcAlpha = BlendMode::kZero;
			outDst = outDstAlpha = BlendMode::kOne;
			break;
		case RenderTypes::kDstOver:
			outSrc = outSrcAlpha = BlendMode::kOneMinusDstAlpha;
			outDst = outDstAlpha = BlendMode::kOne;
			break;
		case RenderTypes::kSrcIn:
			outSrc = outSrcAlpha = BlendMode::kDstAlpha;
			outDst = outDstAlpha = BlendMode::kZero;
			break;
		case RenderTypes::kDstIn:
			outSrc = outSrcAlpha = BlendMode::kZero;
			outDst = outDstAlpha = BlendMode::kSrcAlpha;
			break;
		case RenderTypes::kSrcOut:
			outSrc = outSrcAlpha = BlendMode::kOneMinusDstAlpha;
			outDst = outDstAlpha = BlendMode::kZero;
			break;
		case RenderTypes::kDstOut:
			outSrc = outSrcAlpha = BlendMode::kZero;
			outDst = outDstAlpha = BlendMode::kOneMinusSrcAlpha;
			break;
		case RenderTypes::kSrcAtop:
			outSrc = outSrcAlpha = BlendMode::kDstAlpha;
			outDst = outDstAlpha = BlendMode::kOneMinusSrcAlpha;
			break;
		case RenderTypes::kDstAtop:
			outSrc = outSrcAlpha = BlendMode::kOneMinusDstAlpha;
			outDst = outDstAlpha = BlendMode::kSrcAlpha;
			break;
		case RenderTypes::kXor:
			outSrc = outSrcAlpha = BlendMode::kOneMinusDstAlpha;
			outDst = outDstAlpha = BlendMode::kOneMinusSrcAlpha;
			break;
		case RenderTypes::kSrcOver:
		case RenderTypes::kCustom: // Default for kCustom is same as kNormal
		case RenderTypes::kNormal:
		default:
			// Normal: pixel = src*srcAlpha + dst*(1-srcAlpha)
			// rgb = (aSrc*[rSrc,gSrc,bSrc])*kOne + (rDst,gDst,bDst)*kOneMinusSrcAlpha
			// a   = (aSrc)                 *kOne + (aDst)          *kOneMinusSrcAlpha
			outSrc = BlendMode::kOne;
			outDst = BlendMode::kOneMinusSrcAlpha;
			outSrcAlpha = BlendMode::kOne;
			outDstAlpha = BlendMode::kOneMinusSrcAlpha;
			break;
	}
}

static void GetBlendParamsNonPremultiplied(
	RenderTypes::BlendType value,
	BlendMode::Param& outSrc, BlendMode::Param& outDst,
	BlendMode::Param& outSrcAlpha, BlendMode::Param& outDstAlpha )
{
	switch ( value )
	{
		case RenderTypes::kAdditive:
			// rgb = (rSrc,gSrc,bSrc)*kSrcAlpha + (rDst,gDst,bDst)*kOne
			// a   = (aSrc)          *kOne      + (aDst)          *kOne
			outSrc = BlendMode::kSrcAlpha;
			outDst = BlendMode::kOne;
			outSrcAlpha = BlendMode::kSrcAlpha; // kOne ???
			outDstAlpha = BlendMode::kOne;
			break;
		case RenderTypes::kScreen:
			// (R,G,B,A) = {src}*kSrcAlpha + {dst}*((1 - Src)*aSrc)
			// NOTE: This won't work with transparency
			outSrc = BlendMode::kSrcAlpha;
			outDst = BlendMode::kOneMinusSrcColor;
			outSrcAlpha = BlendMode::kSrcAlpha;
			outDstAlpha = BlendMode::kOneMinusSrcColor;
			break;
		case RenderTypes::kMultiply:
			// (R,G,B,A) = {src}*kSrcAlpha*kDstColor + {dst}*kOneMinusSrcAlpha
			// NOTE: This won't work with transparency
			outSrc = BlendMode::kDstColor;
			outDst = BlendMode::kOneMinusSrcAlpha;
			outSrcAlpha = BlendMode::kDstColor;
			outDstAlpha = BlendMode::kOneMinusSrcAlpha;
			break;
		case RenderTypes::kClear:
		case RenderTypes::kSrc:
		case RenderTypes::kDst:
		case RenderTypes::kDstOver:
		case RenderTypes::kSrcIn:
		case RenderTypes::kDstIn:
		case RenderTypes::kSrcOut:
		case RenderTypes::kDstOut:
		case RenderTypes::kSrcAtop:
		case RenderTypes::kDstAtop:
		case RenderTypes::kXor:
			Rtt_TRACE_SIM( ( "WARNING: PorterDuff blend modes cannot operate in straight color (non-premultiplied alpha). Making best attempt but your mileage will vary.\n" ) );
			GetBlendParamsPremultiplied( value, outSrc, outDst, outSrcAlpha, outDstAlpha );
			break;
		case RenderTypes::kSrcOver:
		case RenderTypes::kCustom: // Default for kCustom is same as kNormal
		case RenderTypes::kNormal:
		default:
			// rgb = (rSrc,gSrc,bSrc)*kSrcAlpha + (rDst,gDst,bDst)*kOneMinusSrcAlpha
			// a   = (aSrc)          *kOne      + (aDst)          *kOneMinusSrcAlpha
			outSrc = BlendMode::kSrcAlpha;
			outDst = BlendMode::kOneMinusSrcAlpha;
			outSrcAlpha = BlendMode::kOne;
			outDstAlpha = BlendMode::kOneMinusSrcAlpha;
			break;
	}
}

// ----------------------------------------------------------------------------

static const char kZeroKey[] = "zero";
static const char kOneKey[] = "one";
static const char kSrcColorKey[] = "srcColor";
static const char kOneMinusSrcColorKey[] = "oneMinusSrcColor";
static const char kDstColorKey[] = "dstColor";
static const char kOneMinusDstColorKey[] = "oneMinusDstColor";
static const char kSrcAlphaKey[] = "srcAlpha";
static const char kOneMinusSrcAlphaKey[] = "oneMinusSrcAlpha";
static const char kDstAlphaKey[] = "dstAlpha";
static const char kOneMinusDstAlphaKey[] = "oneMinusDstAlpha";
static const char kSrcAlphaSaturateKey[] = "srcAlphaSaturate";

const char*
BlendMode::StringForParam( Param param )
{
	const char *result = NULL;

	switch ( param )
	{
		case kZero:
			result = kZeroKey;
			break;
		case kOne:
			result = kOneKey;
			break;
		case kSrcColor:
			result = kSrcColorKey;
			break;
		case kOneMinusSrcColor:
			result = kOneMinusSrcColorKey;
			break;
		case kDstColor:
			result = kDstColorKey;
			break;
		case kOneMinusDstColor:
			result = kOneMinusDstColorKey;
			break;
		case kSrcAlpha:
			result = kSrcAlphaKey;
			break;
		case kOneMinusSrcAlpha:
			result = kOneMinusSrcAlphaKey;
			break;
		case kDstAlpha:
			result = kDstAlphaKey;
			break;
		case kOneMinusDstAlpha:
			result = kOneMinusDstAlphaKey;
			break;
		case kSrcAlphaSaturate:
			result = kSrcAlphaSaturateKey;
			break;
		default:
			// Bad param enum passed in if this assert is hit.
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

BlendMode::Param
BlendMode::ParamForString( const char *str )
{
	Param result = kUnknown;

	if ( ! str )
	{
		return result;
	}

	if ( 0 == strcmp( kZeroKey, str ) )
	{
		result = kZero;
	}
	else if ( 0 == strcmp( kOneKey, str ) )
	{
		result = kOne;
	}
	else if ( 0 == strcmp( kSrcColorKey, str ) )
	{
		result = kSrcColor;
	}
	else if ( 0 == strcmp( kOneMinusSrcColorKey, str ) )
	{
		result = kOneMinusSrcColor;
	}
	else if ( 0 == strcmp( kDstColorKey, str ) )
	{
		result = kDstColor;
	}
	else if ( 0 == strcmp( kOneMinusDstColorKey, str ) )
	{
		result = kOneMinusDstColor;
	}
	else if ( 0 == strcmp( kSrcAlphaKey, str ) )
	{
		result = kSrcAlpha;
	}
	else if ( 0 == strcmp( kOneMinusSrcAlphaKey, str ) )
	{
		result = kOneMinusSrcAlpha;
	}
	else if ( 0 == strcmp( kDstAlphaKey, str ) )
	{
		result = kDstAlpha;
	}
	else if ( 0 == strcmp( kOneMinusDstAlphaKey, str ) )
	{
		result = kOneMinusDstAlpha;
	}
	else if ( 0 == strcmp( kSrcAlphaSaturateKey, str ) )
	{
		result = kSrcAlphaSaturate;
	}

	return result;
}

BlendMode::Param
BlendMode::ParamForGLenum( int gl_enum )
{
	if( gl_enum == GL_ZERO )
	{
		return BlendMode::kZero;
	}
	else if( gl_enum == GL_ONE )
	{
		return BlendMode::kOne;
	}
	else if( gl_enum == GL_DST_COLOR )
	{
		return BlendMode::kDstColor;
	}
	else if( gl_enum == GL_ONE_MINUS_DST_COLOR )
	{
		return BlendMode::kOneMinusDstColor;
	}
	else if( gl_enum == GL_SRC_ALPHA )
	{
		return BlendMode::kSrcAlpha;
	}
	else if( gl_enum == GL_ONE_MINUS_SRC_ALPHA )
	{
		return BlendMode::kOneMinusSrcAlpha;
	}
	else if( gl_enum == GL_DST_ALPHA )
	{
		return BlendMode::kDstAlpha;
	}
	else if( gl_enum == GL_ONE_MINUS_DST_ALPHA )
	{
		return BlendMode::kOneMinusDstAlpha;
	}
	else if( gl_enum == GL_SRC_ALPHA_SATURATE )
	{
		return BlendMode::kSrcAlphaSaturate;
	}
	else if( gl_enum == GL_SRC_COLOR )
	{
		return BlendMode::kSrcColor;
	}
	else if( gl_enum == GL_ONE_MINUS_SRC_COLOR )
	{
		return BlendMode::kOneMinusSrcColor;
	}
	else
	{
		Rtt_ASSERT( !"Unsupported blend mode." );
	}

	return (BlendMode::Param)0;
}

// ----------------------------------------------------------------------------

BlendMode::BlendMode()
{
	GetBlendParamsPremultiplied(
		RenderTypes::kNormal, fSrcColor, fDstColor, fSrcAlpha, fDstAlpha );
}

BlendMode::BlendMode( RenderTypes::BlendType value, bool isPremultiplied )
{
	if ( isPremultiplied )
	{
		GetBlendParamsPremultiplied(
			value, fSrcColor, fDstColor, fSrcAlpha, fDstAlpha );
	}
	else
	{
		GetBlendParamsNonPremultiplied(
			value, fSrcColor, fDstColor, fSrcAlpha, fDstAlpha );
	}
}

BlendMode::BlendMode( Param srcColor, Param dstColor, Param srcAlpha, Param dstAlpha )
:	fSrcColor( srcColor ),
	fDstColor( dstColor ),
	fSrcAlpha( srcAlpha ),
	fDstAlpha( dstAlpha )
{
}

bool
BlendMode::operator==( const BlendMode& rhs ) const
{
	return 0 == memcmp( this, & rhs, sizeof( BlendMode ) );
}

// ----------------------------------------------------------------------------

// These details are used to lug around some information for non-built-in or
// non-2D-target texture and / or bitmap resources, and avoid the inconvenience
// and / or need to pester the texture factory.

enum {
	// Various single-bit flags.
	kFlagBits = 1 /* special flag */

		// This is used for a few special cases: the format is word-packed, or compressed
		// in an ASTC-ish (16-bit block size) way.

			+ 1 /* array flag */
			
		// The texture is structured as an array.

			+ 1 /* component #1 flag */
			
		// In the case of a word-packed format, component #2 has 1 more bit than average.
			
			+ 1 /* component #2 flag */

		// ...likewise, but for component #2...
		
			+ 1 /* component #3 flag */

		// ...and component #3.

			+ 1, /* is-mask flag */
		
		// In the case of a one-component format, indicate it has mask semantics.

	// These bits describe whether a texture and sampler can pair up, so that paints
	// know whether they can plug into a given shader input. Some bitmap operations
	// can also take this into consideration, e.g. only considering vanilla 2D.
	kFamilyBits = 2,
	kTargetBits = 3,

	// Allow 511 (0 being "no custom format") possible texture formats and aliases.
	// This is a bit arbitrary: some digging suggests a Vulkan backend, seemingly the
	// most prolific case, might allow 300-some options, although only a few of these
	// are likely to matter in real situations.
	// When one of these is in use, the detail bits also come into play. Furthermore,
	// the stock bits, i.e. those that would have contained a built-in format's value,
	// lose their meaning and so may be repurposed.
	kFormatIndexBits = 9,

	// These are some details used to interpret the format as a bitmap, e.g. finding
	// the size or component layout. The different sorts of formats (currently "normal",
	// word-packed, or compressed) each interpret these in a particular way.
	kBytesPerComponentBits = 2,
	kComponentCountBits = 2,
	kDataBits = 4,

	// These describe in broad strokes how to interpret the input data, e.g. as the
	// common 0-255, normalized to 0-1 on the GPU, versus actual floating-point. A
	// few hybrid formats can also distinguish themselves in this way.
	kInputKindBits = 3,

	// Sums of some related bits...
	kTargetPartBits = kFamilyBits + kTargetBits,
	kDetailBits = kBytesPerComponentBits + kComponentCountBits + kDataBits,

	// ...and # of bits maximally alloted (with stock format bits repurposed).
	kAllBits = kFlagBits + kFormatIndexBits + kTargetPartBits + kDetailBits + kInputKindBits

	// Some validation of these is done in corresponding C++ files.
};

Rtt_STATIC_ASSERT( kTextureInputKind_NumKinds <= ( 1U << kInputKindBits ) );
Rtt_STATIC_ASSERT( kAllBits <= 32 );

// ----------------------------------------------------------------------------

struct MaskInfo {
	int fFirstBit;
	int fNumBits;
	U32 fIncludeMask;
	U32 fExcludeMask;
};

#define INCLUDE_MASK( numBits ) ( ( 1U << ( numBits ) ) - 1 )
#define MAKE_MASK_INFO( first, numBits, next ) { \
	first, numBits, INCLUDE_MASK( numBits ), ~( INCLUDE_MASK( numBits ) << ( first ) ) \
}; const int next = first + numBits
#define MAKE_FLAG_MASK( offset, next ) 1U << ( offset ); const int next = ( offset ) + 1

static const MaskInfo StockFormatMask = MAKE_MASK_INFO( 0, FormatDetails::kStockFormatBits, kFormatIndexOffset );
static const MaskInfo FormatIndexMask = MAKE_MASK_INFO( kFormatIndexOffset, kFormatIndexBits, kFamilyOffset );

static const MaskInfo FamilyMask = MAKE_MASK_INFO( kFamilyOffset, kFamilyBits, kTargetOffset );
static const MaskInfo TargetMask = MAKE_MASK_INFO( kTargetOffset, kTargetBits, kIsArrayFlagOffset );
static const U32 IsArrayMask = MAKE_FLAG_MASK( kIsArrayFlagOffset, kBytesPerComponentOffset );

// n.b. repurposes stock format bits when using non-core format
static const MaskInfo ComponentCountMask = MAKE_MASK_INFO( 0, kComponentCountBits, kSpecialFlagOffset );
static const U32 SpecialFlagMask = MAKE_FLAG_MASK( kSpecialFlagOffset, kComponentsPlusSpecialMaskSize );
Rtt_STATIC_ASSERT( FormatDetails::kStockFormatBits >= kComponentsPlusSpecialMaskSize );

static const MaskInfo BytesPerComponentMask = MAKE_MASK_INFO( kBytesPerComponentOffset, kBytesPerComponentBits, kDataOffset );
static const MaskInfo DataMask = MAKE_MASK_INFO( kDataOffset, kDataBits, kDiff1FlagOffset );
static const U32 Diff1FlagMask = MAKE_FLAG_MASK( kDiff1FlagOffset, kDiff2FlagOffset );
static const U32 Diff2FlagMask = MAKE_FLAG_MASK( kDiff2FlagOffset, kDiff3FlagOffset );
static const U32 Diff3FlagMask = MAKE_FLAG_MASK( kDiff3FlagOffset, kInputKindOffset );

// TODO: if "word-packed" && #components < 3, further cases
	// can hijack Diff*Flag if #components = 1 (e.g. bit 1 says "is depth or stencil", bit 2 decides which; else = "other" with data)
	// if we ARE using depth-stencil, we can potentially forgo some stuff, e.g. "has alpha" or "color byte indices"

static const MaskInfo InputKindMask = MAKE_MASK_INFO( kInputKindOffset, kInputKindBits, kIsMaskFlagOffset );
static const U32 kIsMaskFlagMask = MAKE_FLAG_MASK( kIsMaskFlagOffset, kDoneOffset );

Rtt_STATIC_ASSERT( kDoneOffset == kAllBits );

// TODO: can supply 32 - kAllBits and getter / setter...

#undef INCLUDE_MASK
#undef MAKE_MASK_INFO
#undef MAKE_FLAG_MASK

static U32
GetBits( U32 v, const MaskInfo& info )
{
	return ( v >> info.fFirstBit ) & info.fIncludeMask;
}

static void
SetBits( U32* v, U32 bits, const MaskInfo& info )
{
	Rtt_ASSERT( ( bits & info.fIncludeMask ) == bits );
	
	*v &= info.fExcludeMask;
	*v |= bits << info.fFirstBit;
}

// ----------------------------------------------------------------------------

int
FormatDetails::GetFormatIndexBitCount()
{
	return kFormatIndexBits;
}

U32
FormatDetails::GetStockFormat( U32 v )
{
	return GetBits( v, StockFormatMask );
}

U32
FormatDetails::GetFormatIndex( U32 v )
{
	return GetBits( v, FormatIndexMask );
}

U32
FormatDetails::GetFamily( U32 v )
{
	return GetBits( v, FamilyMask );
}

U32
FormatDetails::GetTarget( U32 v )
{
	return GetBits( v, TargetMask );
}

bool
FormatDetails::HasArrayFlag( U32 v )
{
	return 0 != ( v & IsArrayMask );
}

bool
FormatDetails::IsCore( U32 v )
{
	return 0 == GetFormatIndex( v );
}

void
FormatDetails::AddMaskBit( U32& backingValue )
{
	backingValue |= kIsMaskFlagMask;
}

struct Bits {
	U32 Get( const MaskInfo& mask ) const { return GetBits( fValue, mask ); }
	U32 GetCount( const MaskInfo& mask ) const { return Get( mask ) + 1; }

	void Set( U32 bits, const MaskInfo& mask )
	{
		Rtt_ASSERT( ( bits & mask.fIncludeMask ) == bits );
		SetBits( &fValue, bits, mask );
	}
	
	void SetCount( U32 bits, const MaskInfo& mask )
	{
		Rtt_ASSERT( bits > 0 );
		Set( bits - 1, mask );
	}
	
	bool HasFlag( U32 flag ) const { return 0 != ( fValue & flag ); }
	void SetFlag( U32 flag ) { fValue |= flag; }

	U32 fValue;
};

// n.b. an alternative to this would be a struct with a bunch of U32-based
// bitfields, that ensures the final size == sizeof(U32); this would mildly
// affect repurposing the stock bits, and also not give an obvious way to
// access any "extra" bits that remain, so the scheme used here seems best

// ----------------------------------------------------------------------------

U32
FormatDetails::BuildFromDescription( const TextureFormatDescription* desc, U32 formatIndex )
{
	if ( desc->IsWordPacked() )
	{
		Rtt_ASSERT( ( 0 != desc->fSizes[0] ) && ( 0 != desc->fSizes[1] ) );
		Rtt_ASSERT( ( 0 != desc->fSizes[2] ) || ( 0 == desc->fSizes[3] ) );
		Rtt_ASSERT( ( desc->fSizes[0] + desc->fSizes[1] + desc->fSizes[2] + desc->fSizes[3] ) % 8 == 0 );
	}
	else
	{
		if ( desc->IsCompressed() )
		{
			Rtt_ASSERT( desc->fBlockWidth != 0 && desc->fBlockHeight != 0 && desc->fBlockSize != 0 );
		}
		else
		{
			Rtt_ASSERT( desc->fNumComponents >= 1 && desc->fNumComponents <= 4 );
			Rtt_ASSERT( desc->fBytesPerComponent == 1 || desc->fBytesPerComponent == 2 || desc->fBytesPerComponent == 4 );
		}
	}
	
	Bits bits = {};
	if ( desc->IsWordPacked() )
	{
		int componentCount = 2 + ( 0 != desc->fSizes[2] ) + ( 0 != desc->fSizes[3] );
// TODO: can use 1 to sneak in exceptions? (also, 2 == depth, if anything)
		bits.SetCount( componentCount, ComponentCountMask );
	}
	else if ( !desc->IsCompressed() )
	{
		bits.SetCount( desc->fNumComponents, ComponentCountMask );
	}

	if ( desc->IsCompressed() )
	{
		bits.SetCount( 3, BytesPerComponentMask ); // 3 != valid size, so encodes compression

		if ( 16 == desc->fBlockSize ) // ASTC or certain BC format?
		{
			U32 data = Texture::Format::BlockDimsID( desc->fBlockWidth, desc->fBlockHeight );

			Rtt_ASSERT( data >= 0 && data < ( 1 << kDataBits ) );
			
			bits.Set( data, DataMask );
			bits.SetFlag( SpecialFlagMask );
		}
	}
	else if ( desc->IsWordPacked() )
	{
		U32 sumOfSizes = desc->fSizes[0] + desc->fSizes[1] + desc->fSizes[2] + desc->fSizes[3];

		bits.SetCount( sumOfSizes / 8, BytesPerComponentMask );
		bits.SetFlag( SpecialFlagMask );

		bool hasThreeOrMoreComponents = ( 0 != desc->fSizes[2] );
		if ( hasThreeOrMoreComponents ) // "normal" word-packed format?
		{
			int firstIndex = 0;
			U32 dataBits = 0, isReversed = 0 != ( desc->fFlags & TextureFormatDescription::kIsReversed );
			if ( 0 != desc->fSizes[3] ) // has four components?
			{
				// In the case of word-packed formats with four components, at least three of
				// them have either their (floored) average value, or are one greater. There
				// are some formats where the fourth value is an outlier: these always appear
				// to be small, in particular <= 3 bits.
				
				// Whether the outlier differs or not, put it in the data and deduct it from
				// the sum-of-sizes, giving us an average honoring the above description.
				U8 outlier;
				if ( isReversed )
				{
					firstIndex++;
					outlier = desc->fSizes[0];
				}
				else
				{
					outlier = desc->fSizes[3];
				}
				
				Rtt_ASSERT( ( ( outlier << 1 ) & DataMask.fIncludeMask ) == ( outlier << 1 ) );
				
				dataBits = outlier;
				sumOfSizes -= outlier;
			}
			
			bits.Set( ( dataBits << 1 ) | isReversed, DataMask );
			
			U32 averagePlusOne = sumOfSizes / 3 + 1;
			if ( desc->fSizes[firstIndex] == averagePlusOne )
			{
				bits.SetFlag( Diff1FlagMask );
			}
			if ( desc->fSizes[firstIndex + 1] == averagePlusOne )
			{
				bits.SetFlag( Diff2FlagMask );
			}
			if ( desc->fSizes[firstIndex + 2] == averagePlusOne )
			{
				bits.SetFlag( Diff3FlagMask );
			}
		}
		else
		{
			// TODO: depth, planar, etc.
				// redundant, given family == kOtherFamily?
		}
	}
	else
	{
		bits.SetCount( desc->fBytesPerComponent, BytesPerComponentMask );
		// TODO: miscellaneous bits in 
	}

	bits.Set( formatIndex, FormatIndexMask );

	return bits.fValue;
}

U32
FormatDetails::GatherFamilyInfo( U32 family, U32 target, bool isArray )
{
	U32 value = 0;
	SetBits( &value, family, FamilyMask );
	SetBits( &value, target, TargetMask );
	
	if ( isArray )
	{
		value |= IsArrayMask;
	}
	
	return value;
}

size_t
FormatDetails::GetSize( U16 w, U16 h, U32 backingValue )
{
	Bits bits = { backingValue };
	U32 bytesPerComponent = bits.GetCount( BytesPerComponentMask );
	if ( 3 == bytesPerComponent ) // compressed? cf. BuildFromDescription()
	{
		U8 blockWidth, blockHeight, blockSize;
		bool has16Bytes = bits.HasFlag( SpecialFlagMask );
		if ( has16Bytes )
		{
			Texture::Format::GetBlockDims( bits.Get( DataMask ), blockWidth, blockHeight );
			
			blockSize = 16;
		}
		else
		{
			blockWidth = 4;
			blockHeight = 4;
			blockSize = 8;
		}
		
		return Texture::Format::GetCompressedSize( w, h, blockWidth, blockHeight, blockSize );
	}
	else if ( bits.HasFlag( SpecialFlagMask ) ) // word-packed?
	{
		return ( w * h ) * bytesPerComponent; // TODO! (1, 2, or 4, #components irrelevant)
				// assuming we don't care about non-color stuff (depth, stencils, etc.) probably covers it?
				// planar, etc. but might fall into same rubric (maybe suss out a bit combination that rules these out)
	}
	else
	{
		U32 componentCount = bits.GetCount( ComponentCountMask );
		return ( w * h ) * ( componentCount * bytesPerComponent );
	}
}

bool
FormatDetails::IsCompressed( U32 backingValue )
{
	return 3 == Bits{ backingValue }.GetCount( BytesPerComponentMask );
}

bool
FormatDetails::HasAlphaChannel( U32 backingValue )
{
	return 4 == Bits{ backingValue }.GetCount( ComponentCountMask );
	// TODO: RA, etc.
}

void
FormatDetails::GetComponentIndices( U32 backingValue, int& redIndex, int& greenIndex, int& blueIndex, int& alphaIndex )
{
	U32 componentCount = Bits{ backingValue }.GetCount( ComponentCountMask );
	
	redIndex = 0;
	greenIndex = ( componentCount > 1 ) ? 1 : -1;
	blueIndex = ( componentCount > 2 ) ? 2 : -1;
	alphaIndex = ( componentCount > 3 ) ? 3 : -1;
	// TODO: BGRA, etc.
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
