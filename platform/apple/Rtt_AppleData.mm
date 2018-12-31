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

