//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_AppleData.h"

#import <Foundation/NSData.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AppleData::AppleData( NSData* data )
:	fData( [data retain] ),
	fOffset( 0 )
{
}

AppleData::~AppleData()
{
	[fData release];
}

const char*
AppleData::Read( size_t numBytes, size_t& numBytesRead ) const
{
	const char* result = NULL;

	const size_t len = Length();
	if ( fOffset < len )
	{
		size_t bytesLeft = len - fOffset;
		numBytesRead = Min( bytesLeft, numBytes );

		const char* bytes = (const char*)[fData bytes];
		result = bytes + fOffset;
		fOffset += numBytesRead;
	}
	else
	{
		numBytesRead = 0;
	}

	return result;
}

void
AppleData::Seek( SeekOrigin origin, S32 offset ) const
{
	switch( origin )
	{
		case kSet:
			Rtt_ASSERT( offset >= 0 && (size_t)offset <= Length() );
			fOffset = offset;
			break;
		case kCurrent:
			fOffset += offset;
			break;
		case kEnd:
			fOffset = Length();
			break;
	}
}

size_t
AppleData::Length() const
{
	return [fData length];
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

