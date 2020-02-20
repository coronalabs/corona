//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_MessageDigest.h"
#include "Core/Rtt_MD4.h"

#include <stdio.h>
#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

void
MD4::Hash( FILE* f, U8* dstBytes )
{
	unsigned char input[64];
	const size_t kNumBytesInput = sizeof( input ) / sizeof( input[0] );

	MD4_CTX context;
	MD4Init( & context );

	size_t numBytes = 0;
	while ( ( numBytes = fread( input, 1, kNumBytesInput, f ) ) != 0 )
	{
		MD4Update( & context, input, (unsigned int) numBytes );
	}

	MD4Final( dstBytes, & context );
}

void 
MD4::Hash( const void * data, U32 length, U8* dstBytes )
{
	unsigned char input[64];
	const size_t kNumBytesInput = sizeof( input ) / sizeof( input[0] );
	const unsigned char * p = (const unsigned char *) data;

	MD4_CTX context;
	MD4Init( & context );

	while ( length > 0 ) {
		size_t numBytes = (length > kNumBytesInput) ? kNumBytesInput : length;
		MD4Update( & context, p, (unsigned int) numBytes );
		p += numBytes;
		length -= numBytes;
	}

	MD4Final( dstBytes, & context );
}

void
MD4::Hash( const char* str, U8* dstBytes )
{
	// TODO: Remove this to save code size and replace with:
	// Hash( str, strlen( str ), dstBytes );

	MD4_CTX context;
	MD4Init( & context );

	size_t len = strlen( str );
	const U8* src = (const U8*)str;
	size_t i;
	for ( i = 0; (i+64) <= len; i = i + 64 )
	{
		MD4Update( & context, src + i, 64 );
	}
	MD4Update( & context, src + i, (unsigned int) (len - i) );

	MD4Final( dstBytes, & context );
}

// ----------------------------------------------------------------------------

#ifdef Rtt_DEBUG

static void MD4Print( const char* str, const U8 digest[MD4::kNumBytesDigest] )
{
	for ( size_t i = 0; i < MD4::kNumBytesDigest; i++ )
	{
		Rtt_TRACE( ( "%02x", digest[i] ) );
	}
	Rtt_TRACE( ( " \"%s\"\n", str ) );
}

static void MD4String( const char* str )
{
	U8 digest[MD4::kNumBytesDigest];
	MD4::Hash( str, digest );
	MD4Print( str, digest );
}

static void MD4File( const char* filePath )
{
	FILE *f = fopen( filePath, "rb" );
	U8 digest[MD4::kNumBytesDigest];
	MD4::Hash( f, digest );
	MD4Print( filePath, digest );
	fclose( f );
}

void
MessageDigest::Test()
{
	
	MD4String("");
	MD4String("a");
	MD4String("abc");
	MD4String("message digest");
	MD4String("abcdefghijklmnopqrstuvwxyz");
	MD4String("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
	MD4String("12345678901234567890123456789012345678901234567890123456789012345678901234567890");
	MD4File( __FILE__ );
}
#endif // Rtt_DEBUG

MessageDigest::MessageDigest()
:	fBytes( NULL ),
	fNumBytes( 0 )
{
}

MessageDigest::~MessageDigest()
{
	if ( fBytes )
	{
		Rtt_FREE( fBytes );
	}
}

void
MessageDigest::Initialize( Rtt_Allocator* allocator, const char* path, HashFunction h )
{
	FILE *f = fopen( path, "rb" );
	if ( Rtt_VERIFY( f ) )
	{
		switch( h )
		{
			case kMD4:
				{
					Alloc( allocator, MD4::kNumBytesDigest );
					MD4::Hash( f, fBytes );
				}
				break;
			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
		}

		fclose( f );
	}
}

void
MessageDigest::Alloc( Rtt_Allocator* allocator, size_t numBytes )
{
	if ( Rtt_VERIFY( ! fBytes ) )
	{
		fBytes = (U8*)Rtt_MALLOC( allocator, numBytes );
		fNumBytes = numBytes;
	}
}

void
MessageDigest::Print( const char* src )
{
	if ( fBytes )
	{
		for ( size_t i = 0; i < fNumBytes; i++ )
		{
			Rtt_PRINT( ( "%02x", fBytes[i] ) );
		}

		if ( src )
		{
			Rtt_PRINT( ( " %s\n", src ) );
		}
		else
		{
			Rtt_PRINT( ( "\n" ) );
		}
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

