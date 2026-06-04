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

// These details are used to lug around some information for non-built-in or non-2D-target
// texture and / or bitmap resources. Although this can be looked up, there are several
// usual situations where doing so is inconvenient.

enum {
	// These bits describe whether a texture and sampler can pair up, so that paints
	// know whether they can plug into a given shader input. Some bitmap operations
	// can also take this into consideration, e.g. only considering vanilla 2D.
	kFamilyBits = 2,
	kTargetBits = 3,
	kTargetSubtypeBits = 2,
	
	// If this flag is set, we have a "non-core" format and the remaining bits come
	// into play; the stock bits themselves lose their meaning and may be repurposed.
	// N.B. this flag could be anywhere, but together with the stock bits we follow
	// the reasonable assumption and put them in the lowest few bits.
	kFormatFlagBit = 1 << kStockFormatBits,

	// Allow 512 possible texture formats and aliases.
	// This value is fairly arbitrary: some research suggests a Vulkan backend, which
	// seemed to be the most prolific case, might allow 300-some options. In practice
	// this is probably overly generous, with the majority of these being irrelevant.
	kFormatIndexBits = 9,

	// These are some details used to interpret the format as a bitmap, e.g. to find
	// the size or component layout. The format may be "normal", packed, or compressed,
	// with the information interpreted accordingly.
	kShiftBits = 2,
	kComponentBits = 3,
	kDataBits = 4,

	// Sums of some related bits...
	kTargetPartBits = kFamilyBits + kTargetBits + kTargetSubtypeBits,
	kDetailBits = kShiftBits + kComponentBits + kDataBits,

	// ...and # of bits maximally alloted, i.e. when the format flag bit is set and
	// the stock format bits have been fully repurposed.
	kAllBits = 1 + kFormatIndexBits + kTargetPartBits + kDetailBits

	// Some validation of these is done in corresponding C++ files.
};

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

static const MaskInfo FormatMask = MAKE_MASK_INFO( 0, kStockFormatBits, kFormatFlagOffset );
static const U32 FormatFlag = 1U << kFormatFlagOffset;
static const MaskInfo FormatAndFlagMask = MAKE_MASK_INFO( 0, kStockFormatBits + 1, kFormatIndexOffset );

static const MaskInfo FormatIndexMask = MAKE_MASK_INFO( kFormatIndexOffset, kFormatIndexBits, kFamilyOffset );

static const MaskInfo FamilyMask = MAKE_MASK_INFO( kFamilyOffset, kFamilyBits, kTargetOffset );
static const MaskInfo TargetMask = MAKE_MASK_INFO( kTargetOffset, kTargetBits, kTargetSubtypeOffset );
static const MaskInfo TargetSubtypeMask =  MAKE_MASK_INFO( kTargetSubtypeOffset, kTargetSubtypeBits, kShiftOffset );

// n.b. repurposes stock format bits when using non-core format
Rtt_STATIC_ASSERT(kStockFormatBits == kComponentBits);
static const MaskInfo ComponentMask = FormatMask;

static const MaskInfo ShiftMask = MAKE_MASK_INFO( kShiftOffset, kShiftBits, kDataOffset );
static const MaskInfo DataMask = MAKE_MASK_INFO( kDataOffset, kDataBits, kDoneOffset );

Rtt_STATIC_ASSERT( kDoneOffset == kAllBits );

#undef INCLUDE_MASK
#undef MAKE_MASK_INFO

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

bool
HasFormatFlag( U32 v )
{
	return 0 != ( v & kFormatFlagBit );
}

U32
GetStockFormatAndFlag( U32 v )
{
	return GetBits( v, FormatAndFlagMask );
}

U32
GetFormatIndex( U32 v )
{
	return GetBits( v, FormatIndexMask );
}

void
SetFormatIndex( U32* v, U32 index )
{
	SetBits( v, index, FormatIndexMask );
}

U32
GetFamily( U32 v )
{
	return GetBits( v, FamilyMask );
}

void
SetFamily( U32* v, U32 family )
{
	SetBits( v, family, FamilyMask );
}

U32
GetTarget( U32 v )
{
	return GetBits( v, TargetMask );
}

void
SetTarget( U32* v, U32 target )
{
	SetBits( v, target, TargetMask );
}

U32
GetTargetSubtype( U32 v )
{
	return GetBits( v, TargetSubtypeMask );
}

void
SetTargetSubtype( U32* v, U32 targetSubtype )
{
	SetBits( v, targetSubtype, TargetSubtypeMask );
}

U32
GetComponents( U32 v )
{
	return GetBits( v, ComponentMask );
}

void
SetComponents( U32* v, U32 components )
{
	SetBits( v, components, ComponentMask );
}

U32
GetShift( U32 v )
{
	return GetBits( v, ShiftMask );
}

void
SetShift( U32* v, U32 shift )
{
	SetBits( v, shift, ShiftMask );
}

U32
GetData( U32 v )
{
	return GetBits( v, DataMask );
}

void
SetData( U32* v, U32 data )
{
	SetBits( v, data, DataMask );
}

// Grouped:

// TODO?
	// format-and-flag sort of abuses that #formats == 1 << StockFormatBits (and then uses those bits + flag)
	// maybe should just make the enum have U32 class?

// TODO?
	// could save DetailMask::kNextBitIndex to use the remaining 6 or so bits

// ----------------------------------------------------------------------------

const U32 kASTCMask = 1U << ( kComponentBits - 1 ); // high bit, when compressed = has ASTC-style block

bool
FormatDetails::PackToValue( U32* v )
{
	// ensure power-of-2 in (1, 2, 4, 8)
	Rtt_ASSERT( fBytesPerComponent > 0 );
	Rtt_ASSERT( fBytesPerComponent < 16 );
	Rtt_ASSERT( 0 == ( fBytesPerComponent & ( fBytesPerComponent - 1 ) ) );

	// ensure power-of-2 in (1, 2, 4)
	bool isCompressed = 0 != fBlockSize;

	Rtt_ASSERT( !isCompressed || ( 0 != fBlockWidth && 0 != fBlockHeight ) );
	Rtt_ASSERT( isCompressed || fBytesPerComponent > 0 );
	Rtt_ASSERT( isCompressed || fBytesPerComponent < 8 );
	Rtt_ASSERT( isCompressed || 0 == ( fBytesPerComponent & ( fBytesPerComponent - 1 ) ) );

	// TODO: so far we only have simple schemes; fix up
	// if and when more sophisticated cases arise

	int componentCount = 1;
	int indices[] = { fRedIndex, fGreenIndex, fBlueIndex, fAlphaIndex };
	const char * names[] = { "red", "green", "blue", "index" };

	for (int i = 0; i < 4; i++)
	{
		if (indices[i] != i)
		{
			if (-1 == indices[i] && i > 0)
			{
				break;
			}
			else
			{
				// CORONA_LOG_ERROR( "Formats with non-%i %s index not yet supported", i, names[i] );

				return false;
			}
		}
		else if (i > 0)
		{
			componentCount++;
		}
	}

	U32 shift = 0, components = componentCount - 1, data = 0;
	
	if ( !isCompressed )
	{
		for ( ; shift < 3; shift++ )
		{
			if ( fBytesPerComponent == ( 1 << shift ) )
			{
				break;
			}
		}

		Rtt_ASSERT( shift < 3 );
	}
	else
	{
		// TODO: any reason to worry about non-Texture case?
		shift = 3; // cf. UnpackDetails()

		if ( 16 == fBlockSize ) // ASTC or BC7?
		{
			Rtt_ASSERT( 0 == ( components & kASTCMask ) );

			components |= kASTCMask;
			data = Texture::Format::BlockDimsID( fBlockWidth, fBlockHeight );

			Rtt_ASSERT( data >= 0 && data < ( 1 << kDataBits ) );
		}
	}

	SetShift( v, shift );
	SetComponents( v, components );
	SetData( v, data );

	return true;
}

FormatDetails
FormatDetails::UnpackFromValue( U32 v )
{
	FormatDetails details;

	details.fBytesPerComponent = 1U << GetShift( v );

	U32 components = GetComponents( v );
	if ( 8 == details.fBytesPerComponent ) // 8 not a valid bpc, encodes "compressed"
	{
		details.fBytesPerComponent = 1;

		bool isASTCish = 0 != ( components & kASTCMask );
		if ( isASTCish )
		{
			components &= ~kASTCMask;
			
			Texture::Format::GetBlockDims( GetData( v ), details.fBlockWidth, details.fBlockHeight );
		}
		else
		{
			details.fBlockWidth = 4;
			details.fBlockHeight = 4;
		}
		
		details.fBlockSize = isASTCish ? 16 : 8;
	}

	if ( components < 4 ) // 0-3: "normal" case, value = #components - 1
	{
		details.fNumComponents = components + 1;
		switch ( details.fNumComponents )
		{
			case 1:
				details.fAlphaIndex = -1;
				// ...and fall through
			case 2:
				details.fBlueIndex = -1;
				// ...and fall through
			case 3:
				details.fGreenIndex = -1;
				// ...and fall through
			default:
				break;
		}
	}
	else
	{
		// CORONA_LOG_ERROR( "NYI: unpacking non-count component formats" );
	}
	
	return details;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
