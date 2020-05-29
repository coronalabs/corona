//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_AppleTextAlignment.h"


namespace Rtt
{

static const char kTextAlignmentLeft[] = "left";
static const char kTextAlignmentCenter[] = "center";
static const char kTextAlignmentRight[] = "right";


#ifdef Rtt_MAC_ENV
	NSTextAlignment AppleAlignment::AlignmentForString( const char *value )
	{
		NSTextAlignment result = NSLeftTextAlignment;
		
		if ( value )
		{
			if ( 0 == strcmp( value, kTextAlignmentCenter ) )
			{
				result = NSCenterTextAlignment;
			}
			else if ( 0 == strcmp( value, kTextAlignmentRight ) )
			{
				result = NSRightTextAlignment;
			}
		}
		
		return result;
	}
	
	const char* AppleAlignment::StringForAlignment( NSTextAlignment value )
	{
		const char *result = kTextAlignmentLeft;
		
		switch ( value )
		{
			case NSCenterTextAlignment:
				result = kTextAlignmentCenter;
				break;
			case NSRightTextAlignment:
				result = kTextAlignmentRight;
				break;
			default:
				break;
		}
		
		return result;
	}
#else
	
	NSTextAlignment AppleAlignment::AlignmentForString( const char *value )
	{
		NSTextAlignment result = NSTextAlignmentLeft;
		
		if ( value )
		{
			if ( 0 == strcmp( value, kTextAlignmentCenter ) )
			{
				result = NSTextAlignmentCenter;
			}
			else if ( 0 == strcmp( value, kTextAlignmentRight ) )
			{
				result = NSTextAlignmentRight;
			}
		}
		
		return result;
	}
	
	const char* AppleAlignment::StringForAlignment( NSTextAlignment value )
	{
		const char *result = kTextAlignmentLeft;
		
		switch ( value )
		{
			case NSTextAlignmentCenter:
				result = kTextAlignmentCenter;
				break;
			case NSTextAlignmentRight:
				result = kTextAlignmentRight;
				break;
			default:
				break;
		}
		
		return result;
	}
	
#endif
	
}



// ----------------------------------------------------------------------------

