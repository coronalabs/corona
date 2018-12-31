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
#include "Rtt_Message.h"
#include <string.h>
#include <sys/stat.h>
#include "Core/Rtt_MessageDigest.h"
#if defined( Rtt_ANDROID_ENV )
	#include "NativeToJavaBridge.h"
	#include "Rtt_AndroidPlatform.h"
	#include "Core/Rtt_Data.h"
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

bool operator==( const MessageData& lhs, const MessageData& rhs )
{
	bool result = ( & lhs == & rhs );

	if ( ! result )
	{
		result = ( 0 == memcmp( lhs.digest, rhs.digest, sizeof( lhs.digest ) ) )
			&& lhs.timestamp == rhs.timestamp;
	}

	return result;
}

Message::Message( const char* filename )
{
	memset( &fData, 0, sizeof( fData ) );

#if Rtt_ANDROID_ENV
	#if !defined(Rtt_ALLOCATOR_SYSTEM)
	#error "This assumes we don't need the allocator object" 
	#endif
	Rtt::Data<char> theBits( NULL );
	bool ok = NativeToJavaBridge::GetRawAsset( filename, theBits );
	if ( ok ) {
		MD4::Hash( theBits.Get(), theBits.Length(), fData.digest );

		// TODO: need a way to get the timestamp of a file...
		fData.timestamp = 0;
	}
#else

	struct stat info;
	lstat( filename, & info );

	Rtt_STATIC_ASSERT( MD4::kNumBytesDigest == sizeof( fData.digest ) );

	FILE *f = fopen( filename, "rb" );
	MD4::Hash( f, fData.digest );
	fData.timestamp = (unsigned int) info.st_mtime;
	// Rtt_ASSERT( info.st_ctime == info.st_mtime );

	fclose( f );
#endif
}

Message::~Message()
{
	memset( &fData, 0, sizeof( fData ) );
}

bool
Message::operator==( const Message& rhs ) const
{
	return Rtt::operator==( fData, rhs.fData );
}

bool
Message::operator==( const MessageData& rhs ) const
{
	return Rtt::operator==( fData, rhs );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

