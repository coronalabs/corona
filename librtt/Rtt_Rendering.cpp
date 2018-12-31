//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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