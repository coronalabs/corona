//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

