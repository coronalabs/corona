// ----------------------------------------------------------------------------
// 
// vlong_io.cpp
// Copyright (c) 2009 Ansca, Inc. All rights reserved.
// 
// Reviewers:
// 		Walter
//
// ----------------------------------------------------------------------------

#include "Core/Rtt_Build.h"

#include "vlong_io.h"
#include "vlong.hpp"

#include <string.h>

#ifdef ANDROID_NDK
#include <stdlib.h>
#endif
#if defined( _WINDOWS )
#include <stdlib.h>
#endif

#ifdef Rtt_DEBUG
#include <errno.h>
#endif

void
vlong_io::ToWords( U32* output, const U8* input, size_t numBytes )
{
	size_t i, j;
	const size_t len = numBytes & (~0x3); // floor to nearest multiple of 4

	for ( i = 0, j = 0; j < len; i++, j += 4)
	{
		#ifdef Rtt_LITTLE_ENDIAN
			output[i] = *(U32*)(&input[j]);
		#else
			output[i] = ((U32)input[j]) | (((U32)input[j+1]) << 8) |
				(((U32)input[j+2]) << 16) | (((U32)input[j+3]) << 24);
		#endif
	}

	if ( len < numBytes )
	{
		char lastBytes[4] = { 'w', 'l', 'u', 'h' }; // Initialize to non-zero 
		size_t lastBytesLen = numBytes - len; Rtt_ASSERT( lastBytesLen < 4 );
		memcpy( lastBytes, input+len, lastBytesLen );

		Rtt_ASSERT( 0 != *(U32*)lastBytes );

		#ifdef Rtt_LITTLE_ENDIAN
			output[i] = *(U32*)(lastBytes);
		#else
			output[i] = ((U32)lastBytes[0]) | (((U32)lastBytes[1]) << 8) |
				(((U32)lastBytes[2]) << 16) | (((U32)lastBytes[3]) << 24);
		#endif
	}
}

// len is num bytes in output
// TODO: Shouldn't littleEndian be determined at compile-time???
// No, the littleEndian param is to enforce byte ordering of the output bytes
// for serialization purposes.
void
vlong_io::ToBytes( U8* output, const U32* input, size_t len, bool littleEndian )
{
	unsigned int i, j;

	if ( littleEndian )
	{
		// Write out bytes in little endian order
		for (i = 0, j = 0; j < len; i++, j += 4)
		{
			#ifdef Rtt_LITTLE_ENDIAN
				*(U32*)(&output[j]) = input[i];
			#else
				U32 value = input[i];
				output[j] = (U8)(value & 0xff); // LSB is (value & 0x1)
				output[j+1] = (U8)((value >> 8) & 0xff);
				output[j+2] = (U8)((value >> 16) & 0xff);
				output[j+3] = (U8)((value >> 24) & 0xff);
			#endif
		}
	}
	else
	{
		for (i = 0, j = 0; j < len; i++, j += 4)
		{
			// Write out bytes in big endian order
			#ifdef Rtt_LITTLE_ENDIAN
				U32 value = input[i];
				output[j] = (U8)(value & 0xff); // MSB is (value & 0x1)
				output[j+1] = (U8)((value >> 8) & 0xff);
				output[j+2] = (U8)((value >> 16) & 0xff);
				output[j+3] = (U8)((value >> 24) & 0xff);
			#else
				*(U32*)(&output[j]) = input[i];
			#endif
		}
	}
}

U8
vlong_io::HexToByte( const char *s )
{
	char hex[] = { s[0], s[1], '\0' };
#ifdef ANDROID_SDK
	Rtt_ASSERT_NOT_IMPLEMENTED();
#else
	U32 result = (U32) strtoul( hex, NULL, 16 );
#endif
	// If the result is zero and errno is EINVAL we have a problem
	// (it appears that strtoul() no longer clears errno so we get spurious asserts)
	// Rtt_ASSERT( ! (result == 0 && errno == EINVAL) );
	return (U8)result;
}

void
vlong_io::FilePrintU32( FILE* f, const char* prefix, const U32* a, size_t numWords )
{
	if ( prefix ) { fprintf( f, "%s ", prefix ); }

	fprintf( f, "(%ld words)\n", numWords );

	for ( size_t i = 0; i < numWords; i++ )
	{
		fprintf( f, "%08x ", a[i] );
	}
	fprintf( f, "\n" );
}

void
vlong_io::FilePrintVlong( FILE* f, const char* prefix, const vlong& n )
{
	unsigned numWords = n.numWords();
	unsigned* a = (unsigned*)malloc( numWords*sizeof(unsigned) );

	n.store( a, numWords );
	FilePrintU32( f, prefix, (U32*)a, numWords ); Rtt_STATIC_ASSERT( sizeof( U32 ) == sizeof( unsigned ) );
	free( a );

/*
	if ( prefix ) { Rtt_TRACE( ( "%s ", prefix ) ); }

	unsigned numWords = n.numWords(); Rtt_TRACE( ( "(%d words)\n", numWords ) );
	unsigned* a = (unsigned*)malloc( numWords*sizeof(unsigned) );

	n.store( a, numWords );
	for ( unsigned i = 0; i < numWords; i++ )
	{
		Rtt_TRACE( ( "%08x ", a[i] ) );
	}
	Rtt_TRACE( ( "\n" ) );

	free( a );
*/
}

void
vlong_io::FileReadU32( FILE* f, const U32* a, size_t numWords )
{
	for ( size_t i = 0; i < numWords; i++ )
	{
		fscanf( f, "%08x ", (U32 *)a + i );
	}
	fscanf( f, "\n" );
}

void
vlong_io::FileReadVlong( FILE* f, const char* prefix, vlong& n )
{
	char str[16];
	int numWords = 0;
	fscanf( f, "%s (%d words)\n", str, & numWords );

	// Rtt_ASSERT( strcmp( str, prefix ) == 0 );

	if ( numWords > 0 )
	{
		unsigned* a = (unsigned*)malloc( numWords*sizeof(unsigned) );
		FileReadU32( f, (U32*)a, numWords ); Rtt_STATIC_ASSERT( sizeof( U32 ) == sizeof( unsigned ) );
		
		n.load( a, numWords );
		free( a );
	}
}
void
vlong_io::PrintU32( const char* prefix, const U32* a, size_t numWords )
{
	FilePrintU32( stdout, prefix, a, numWords );
}

void
vlong_io::PrintVlong( const char* prefix, const vlong& n )
{
	FilePrintVlong( stdout, prefix, n );
}
