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

