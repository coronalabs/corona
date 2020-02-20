//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#if 0

#include "Core/Rtt_Build.h"

#include "Rtt_Rendering.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kNormalBlendType[] = "normal";
static const char kAdditiveBlendType[] = "add";
static const char kScreenBlendType[] = "screen";
static const char kMultiplyBlendType[] = "multiply";

const char*
Rendering::StringForBlendType( BlendType value )
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
		default:
			break;
	}

	return result;
}

Rendering::BlendType
Rendering::BlendTypeForString( const char *str )
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
	}

	return result;
}

Rendering::BlendMode
Rendering::BlendModeForBlendType( BlendType value, bool isPremultiplied )
{
	Rendering::BlendMode result = kUnknownBlendMode;

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

static const char kLinearTextureFilterKey[] = "linear";
static const char kNearestTextureFilterKey[] = "nearest";

Rendering::TextureFilter
Rendering::TextureFilterForString( const char *str )
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

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif