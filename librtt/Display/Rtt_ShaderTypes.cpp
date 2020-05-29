//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

