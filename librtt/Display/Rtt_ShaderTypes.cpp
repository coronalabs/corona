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

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ShaderTypes.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kCategoryDefaultString[] = "default";
static const char kCategoryFilterString[] = "filter";
static const char kCategoryCompositeString[] = "composite";
static const char kCategoryGeneratorString[] = "generator";

const char *
ShaderTypes::StringForCategory( Category category )
{
	const char *result = NULL;

	switch ( category )
	{
		case kCategoryDefault:
			result = kCategoryDefaultString;
			break;

		case kCategoryFilter:
			result = kCategoryFilterString;
			break;

		case kCategoryComposite:
			result = kCategoryCompositeString;
			break;

		case kCategoryGenerator:
			result = kCategoryGeneratorString;
			break;

		default:
			Rtt_ASSERT_NOT_IMPLEMENTED();
			break;
	}

	Rtt_ASSERT( result );

	return result;
}

ShaderTypes::Category
ShaderTypes::CategoryForString( const char *category )
{
	ShaderTypes::Category result = kCategoryDefault;

	if ( category )
	{
		
		if ( 0 == strcmp( category, kCategoryFilterString ) )
		{
			result = kCategoryFilter;
		}
		else if ( 0 == strcmp( category, kCategoryCompositeString ) )
		{
			result = kCategoryComposite;
		}
		else if ( 0 == strcmp( category, kCategoryGeneratorString ) )
		{
			result = kCategoryGenerator;
		}
	}
	
	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

