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

#include "Rtt_AppleDictionaryWrapper.h"

#import <Foundation/NSDictionary.h>
#import <Foundation/NSKeyValueCoding.h>
//#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------
/*
AppleDictionaryWrapper::AppleDictionaryWrapper( MSimulatorPlatform& platform, va_list ap )
:	fPlatform( platform )
	fObject( nil )
{
	Initialize( ap );
}
*/

AppleDictionaryWrapper::AppleDictionaryWrapper( NSDictionary* dictionary )
:	fObject( [dictionary retain] )
{
}

AppleDictionaryWrapper::~AppleDictionaryWrapper()
{
	[fObject release];
}

/*
void
AppleDictionaryWrapper::Initialize()
{
	if ( ! fObject )
	{
		// Default to 4 key/value pairs
		fObject = [[NSMutableDictionary alloc] initWithCapacity:4];
	}
}

void
AppleDictionaryWrapper::SetValueForKey( const PlatformStringWrapper* key, const PlatformObjectWrapper* value )
{
	Initialize();

	[fObject setValue:(id)value->PlatformObject() forKey:(NSString*)key->PlatformObject()];
}
*/

const char*
AppleDictionaryWrapper::ValueForKey( const char* key ) const
{
	NSString* platformKey = [[NSString alloc] initWithUTF8String:key];

	NSString* v = ValueForKey( platformKey );
	const char* result = [v UTF8String];

	[platformKey release];

	return result;
}

const char*
AppleDictionaryWrapper::ValueForKeyPath( const char* keyPath ) const
{
	const char* result = NULL;

	NSString* platformKeyPath = [[NSString alloc] initWithUTF8String:keyPath];
	id value = [fObject valueForKeyPath:platformKeyPath];
	[platformKeyPath release];

	if ( ! [value isKindOfClass:[NSString class]] )
	{
		value = [value description];
	}

	NSString* v = (NSString*)value;
	result = [v UTF8String];

	return result;
}

NSString*
AppleDictionaryWrapper::ValueForKey( NSString* key ) const
{
	NSString* v = nil;

	id value = [fObject valueForKey:key];
	if ( ! [value isKindOfClass:[NSString class]] )
	{
		value = [value description];
	}

	v = (NSString*)value;

	return v;
}

void*
AppleDictionaryWrapper::PlatformObject() const
{
	return fObject;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

