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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#ifdef Rtt_WIN_ENV
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

#include "Core/Rtt_MD4.h"
#include "Core/Rtt_MessageDigest.h"

#include "Rtt_Message.h"
#include "Rtt_VerifierData.h"

#include "Rtt_AuthorizationTicket.h"

#include "rsa.hpp"
#include "vlong_io.h"

#include "Rtt_AppSign.h"
#include "Rtt_FileSystem.h"

#include <errno.h>

// #define Rtt_TEST_LARGE_REPLACE_STRING 1

// ----------------------------------------------------------------------------

using namespace Rtt;

// ----------------------------------------------------------------------------

typedef enum _Command
{
	kNone = 0,
	kDigest,	// Calculate digest
	kCreate,	// Create cert
	kDecrypt,	// Decrypt ciphertext (signature) into plaintext
	kEncrypt,	// Encrypt plaintext into signature
	kDecryptPrivate,	// Decrypt ciphertext using private key
	kEncryptPublic,		// Encrypt plaintext using public key
	kSign,		// Sign binary
	kVerify,	// Verify binary
	kSignJavaScript, // Sign Javascript

	kNumCommands
}
Command;

typedef struct _Options
{
	Command cmd;
	U32 param1;
	const char* file1;
	const char* file2;
	const char* file3;
	U32* pattern;
	size_t patternLen;
	bool littleEndian;
	bool isJavaScript;
}
Options;

// ----------------------------------------------------------------------------

static const size_t gPatternDataLen = 34;
static U32 gPatternData[] =
{
	// Modulus: n = p*q
	0xBAADF00D, 0xBAADF01D, 0xBAADF02D, 0xBAADF03D, 
	0xBAADF04D, 0xBAADF05D, 0xBAADF06D, 0xBAADF07D, 
	0xBAADF08D, 0xBAADF09D, 0xBAADF10D, 0xBAADF11D, 
	0xBAADF12D, 0xBAADF13D, 0xBAADF14D, 0xBAADF15D,

	// Signature (expected)
	0xBAADF16D, 0xBAADF17D, 0xBAADF18D, 0xBAADF19D, 
	0xBAADF20D, 0xBAADF21D, 0xBAADF22D, 0xBAADF23D, 
	0xBAADF24D, 0xBAADF25D, 0xBAADF26D, 0xBAADF27D, 
	0xBAADF28D, 0xBAADF29D, 0xBAADF30D, 0xBAADF31D, 

	// Exponent: coprime/relative prime to (p-1)*(q-1)
	0xBAADF32D, 0xBAADF33D,
	
	// Padding
	0xBAADF3FD, 0xBAADF3FD, 0xBAADF3FD, 0xBAADF3FD
};

static const size_t gPatternDataLenPadded = sizeof( gPatternData ) / sizeof( gPatternData[0] );

// ----------------------------------------------------------------------------

//#ifndef Rtt_WIN_ENV

/*
static U32
RotateLeft( U32 x, int shift )
{
	return ( x << shift ) | ( x >> (32 - shift) );
} 

static U32
RotateRight( U32 x, int shift )
{
	return ( x >> shift ) | ( x << (32 - shift) );
} 
*/

// ----------------------------------------------------------------------------

static char
RandomChar()
{
	U32 r = rand();
//	int shift = rand() % 32;
//	r = RotateLeft( r, shift );

	return (char)r; // truncate
}

static void
RandomString( char s[], size_t numBytes )
{
	for ( size_t i = 0, iMax = numBytes - 1; i < iMax; i++ )
	{
		// Find a char that is *not* the null-termination character
		char ch;
		do
		{
			ch = RandomChar();
		}
		while( '\0' == ch );

		s[i] = ch;
	}

	s[numBytes - 1] = '\0';
}

static void
FilePrintString( FILE* f, const char* prefix, const char* str )
{
	if ( prefix ) { fprintf( f, "%s ", prefix ); }
	fprintf( f, "(%lu chars)\n", strlen( str ) );

	for ( ; *str++ != '\0'; )
	{
		fprintf( f, "%03u ", (unsigned char)*str );
	}
	fprintf( f, "\n" );
}



static void
PrintString( const char* prefix, const char* str )
{
	FilePrintString( stdout, prefix, str );
}

// ----------------------------------------------------------------------------

struct KeyData
{
	vlong p;
	vlong q;
	vlong m;
	vlong e;
	vlong d;
};

static size_t
ReadKeyfile( const char *keyfile, KeyData& outKey )
{
	// Deserialize key file
	FILE* f = Rtt_FileOpen( keyfile, "r" );

	if (f == NULL)
	{
		fprintf(stderr, "app_sign: cannot open keyfile '%s'\n", keyfile);
	}
	else
	{
		// vlong p, q, m, e, d;
		vlong_io::FileReadVlong( f, "[p]", outKey.p );
		vlong_io::FileReadVlong( f, "[q]", outKey.q );
		vlong_io::FileReadVlong( f, "[m]", outKey.m );
		vlong_io::FileReadVlong( f, "[e]", outKey.e );
		vlong_io::FileReadVlong( f, "[d]", outKey.d );
		Rtt_FileClose( f );
	}

	return outKey.m.numWords();
}

// ----------------------------------------------------------------------------

static void
CalculateDigest( const Options& options )
{
	MessageDigest digest;
	digest.Initialize( NULL, options.file1, MessageDigest::kMD4 );
	digest.Print( options.file1 );
}

static void
CreateCertificate( const Options& options )
{
	// Generate two random strings p and q
	const size_t kNumBytes = (options.param1 >> 1) + 1;

	char *r1 = (char *)calloc(sizeof(char), kNumBytes);
	char *r2 = (char *)calloc(sizeof(char), kNumBytes);

	RandomString( r1, kNumBytes );
	RandomString( r2, kNumBytes );

	PrintString( "[r1]", r1 );
	PrintString( "[r2]", r2 );

	private_key k( r1, r2 );
	vlong p = k.get_p(); vlong_io::PrintVlong( "[p]", p );
	vlong q = k.get_q(); vlong_io::PrintVlong( "[q]", q );
	vlong m = k.get_m(); vlong_io::PrintVlong( "[m]", m );
	vlong e = k.get_e(); vlong_io::PrintVlong( "[e]", e );
	vlong d = k.get_d(); vlong_io::PrintVlong( "[d]", d );

	FILE* f = Rtt_FileOpen( options.file1, "w+" );

	if (f == NULL)
	{
		fprintf(stderr, "app_sign: cannot open certificate file '%s' for writing\n", options.file1);
	}
	else
	{
		vlong_io::FilePrintVlong( f, "[p]", p );
		vlong_io::FilePrintVlong( f, "[q]", q );
		vlong_io::FilePrintVlong( f, "[m]", m );
		vlong_io::FilePrintVlong( f, "[e]", e );
		vlong_io::FilePrintVlong( f, "[d]", d );
		Rtt_FileClose( f );
	}

	free(r1);
	free(r2);

#ifdef NOT_USED
	// Test code
	while (0)
	{
		private_key keyPrivate( p, q );

		Options o = options;
		o.file1 = __FILE__;
		MessageDigest digest;
		digest.Initialize( NULL, __FILE__, MessageDigest::kMD4 );
		digest.Print( o.file1 );
		U32 digestU32[MD4::kNumBytesDigest >> 2];
		const size_t kDigestLen = sizeof( digestU32 ) / sizeof( digestU32[0] );
		MD4Decode( digestU32, *digest, MD4::kNumBytesDigest );
		vlong_io::PrintU32( "md4  ", digestU32, kDigestLen );

		vlong plain;
		plain.load( (unsigned*)digestU32, kDigestLen ); Rtt_STATIC_ASSERT( sizeof( U32 ) == sizeof( unsigned ) );
		plain.pad( 15 );
		vlong_io::PrintVlong( "plain", plain );

		public_key keyPublic( keyPrivate.get_m(), keyPrivate.get_e() );
		vlong encrypted = keyPublic.encrypt( plain );
		vlong_io::PrintVlong( "encrypted", encrypted );

		vlong decrypted = keyPrivate.decrypt( encrypted );
		vlong_io::PrintVlong(	"decrypted", decrypted );

		if ( ! Rtt_VERIFY( plain == decrypted ) )
		{
			Rtt_TRACE( ( "[*** ERROR ***] plaintext != decrypted\n" ) );
		}
	}
#endif // NOT_USED
}

static void
DecryptCiphertext( const char *keyfile, const char *ciphertext )
{
	KeyData key;
	ReadKeyfile( keyfile, key );

	public_key keyPublic( key.m, key.e );
	size_t numBytes = 0;
	U8 *plaintext = AuthorizationTicket::CipherToPlain( keyPublic, ciphertext, numBytes );

	if ( Rtt_VERIFY( plaintext ) )
	{
		Rtt_ASSERT( strlen( (char*)plaintext ) <= numBytes );
		printf( "%s", plaintext );
		/*
		size_t j =0;
		for ( size_t i = 0; i < numBytes ; i++ )
		{
			j++;
			printf( "%c", plaintext[i] );
		}
		
		printf( "\nnumChars: %d %d\n", j, strlen( (char*)plaintext )  );
		*/
		free( plaintext );
	}

#if 0
	size_t numBytes = strlen( ciphertext );
	// len is a multiple of 8 because we emit little-endian hex during encryption
	if ( Rtt_VERIFY( (numBytes & 0x7) == 0 ) )
	{
		size_t numWords = numBytes >> 2;
		U32 *buf32 = (U32*)malloc( numWords*sizeof(U32) );
		for ( size_t i = 0, j = 0; i < numBytes; i += 8, j++ )
		{
			const char *bytes = ciphertext + i;
			// ciphertext was little-endian
			buf32[j] =
				HexToByte( bytes )
				| ( HexToByte( bytes+2 ) << 8 )
				| ( HexToByte( bytes+4 ) << 16 )
				| ( HexToByte( bytes+6 ) << 24 );
		}

		vlong c;
		c.load( (unsigned*)buf32, numWords ); Rtt_STATIC_ASSERT( sizeof( U32 ) == sizeof( unsigned ) );

		public_key keyPublic( key.m, key.e );
		vlong cPlain = keyPublic.encrypt( c );

		numWords = cPlain.numWords();
		cPlain.store( (unsigned*)buf32, numWords );

		#ifdef Rtt_DEBUG
			// Print as hex (little-endian)
			for ( size_t i = 0; i < numWords; i++ )
			{
				U32 value = buf32[i];
				printf( "%02x", value & 0xFF );
				printf( "%02x", (value>>8) & 0xFF );
				printf( "%02x", (value>>16) & 0xFF );
				printf( "%02x", (value>>24) & 0xFF );
			}

			PrintVlong( "\n[cPlain]",  cPlain );
		#endif

		numBytes = numWords*sizeof(*buf32);
		U8* buf8 = (U8*)malloc( numBytes + 1 );
		ToBytes( buf8, buf32, numWords, true );
		printf( "%s\n", buf8 );
/*
		for ( size_t i = 0; i < numBytes && '\0' != buf8[i]; i++ )
		{
			printf( "%c", buf8[i] );
		}
		printf( "\n" );
*/
		free( buf32 );
	}
	
#endif
}

static void
EncryptPlaintext( const char *keyfile, const char *plaintext )
{
	KeyData key;
	const size_t numWordsInKey = ReadKeyfile( keyfile, key ); Rtt_ASSERT( strlen( plaintext ) < (numWordsInKey << 2) );
	#pragma unused(numWordsInKey)

	private_key keyPrivate( key.p, key.q );

	size_t numBytes = 0;
	U8 *ciphertext = AuthorizationTicket::PlainToCipher( keyPrivate, plaintext, numBytes );

	if ( Rtt_VERIFY( ciphertext ) )
	{
		for ( size_t i = 0; i < numBytes; i++ )
		{
			printf( "%02x", ciphertext[i] );
		}

		free( ciphertext );
	}

#if 0
	const size_t numBytes = strlen( plaintext ); Rtt_ASSERT( numBytes < (numWordsInKey << 2) );
	const size_t numBytes4 = (numBytes + 3) & (~3);
	const size_t numWords4 = numBytes4 >> 2;

	U32 *buf32 = (U32*)malloc( sizeof(U32)*numWordsInKey );
	ToWords( buf32, (const U8*)plaintext, numBytes );

	vlong cPlain;
	cPlain.load( (unsigned*)buf32, numWords4 ); Rtt_STATIC_ASSERT( sizeof( U32 ) == sizeof( unsigned ) );

	// Pad remaining plain msg with random bits.  Subtract 1 word since msg len < key len
	cPlain.pad( (numWordsInKey - 1) - numWords4 );

#ifdef Rtt_DEBUG
printf( "[plaintext] %s\n", plaintext );
vlong_io::PrintVlong( "[cPlain]", cPlain );
#endif

	private_key keyPrivate( key.p, key.q );
	vlong c = keyPrivate.decrypt( cPlain );

#ifdef Rtt_DEBUG
vlong_io::PrintVlong( "[c     ]", c );
#endif

/*
	for ( size_t i = 0; i < numWordsInKey; i++ )
	{
		U32 value = buf32[i];
		printf( "%02x", value & 0xFF );
		printf( "%02x", (value>>8) & 0xFF );
		printf( "%02x", (value>>16) & 0xFF );
		printf( "%02x", (value>>24) & 0xFF );
	}
*/	

	Rtt_ASSERT( numWordsInKey == c.numWords() );
	c.store( (unsigned*)buf32, numWordsInKey );

	// Print as hex (little-endian)
	for ( size_t i = 0; i < numWordsInKey; i++ )
	{
		U32 value = buf32[i];
		printf( "%02x", value & 0xFF );
		printf( "%02x", (value>>8) & 0xFF );
		printf( "%02x", (value>>16) & 0xFF );
		printf( "%02x", (value>>24) & 0xFF );
	}

#if 0
	public_key keyPublic( key.m, key.e );
	vlong cDecrypted = keyPublic.encrypt( c );

	vlong_io::PrintVlong( "[cPlain]", cPlain );
	vlong_io::PrintVlong( "[c     ]", c );
	PrintVlong( "[cDecrypted]", cDecrypted );

	cDecrypted.store( (unsigned*)buf32, cDecrypted.numWords() );
	// Print as hex (little-endian)
	for ( size_t i = 0; i < cDecrypted.numWords(); i++ )
	{
		U32 value = buf32[i];
		printf( "%02x", value & 0xFF );
		printf( "%02x", (value>>8) & 0xFF );
		printf( "%02x", (value>>16) & 0xFF );
		printf( "%02x", (value>>24) & 0xFF );
	}
#endif

	free( buf32 );
#endif
}

static void
DecryptCiphertextPrivate( const char *keyfile, const char *ciphertext )
{
	KeyData key;
	ReadKeyfile( keyfile, key );

	private_key k( key.p, key.q );

#if 0
public_key kPublic( key.p, key.q );

U32 buf32[2] = { 0xdeadbeef, 0xbaadf00d };
vlong cPlain;
cPlain.load( (unsigned*)buf32, 2 ); Rtt_STATIC_ASSERT( sizeof( U32 ) == sizeof( unsigned ) );
cPlain.pad( k.get_m().numWords() - 2 -1 );
vlong c = k.decrypt( cPlain );
vlong cDecrypted = kPublic.encrypt( c );
//vlong c = kPublic.encrypt( cPlain );
//vlong cDecrypted = k.decrypt( c );

vlong_io::PrintVlong( "[c       ]", c );
vlong_io::PrintVlong( "[cPlain  ]", cPlain );
vlong_io::PrintVlong( "[cDecrypt]", cDecrypted );
Rtt_ASSERT( cPlain == cDecrypted );
#endif
	size_t numBytes = 0;
	U8 *plaintext = AuthorizationTicket::CipherToPlainPrivate( k, ciphertext, numBytes );

	if ( Rtt_VERIFY( plaintext ) )
	{
		Rtt_ASSERT( strlen( (char*)plaintext ) <= numBytes );
		printf( "%s", plaintext );
		free( plaintext );
	}
}

static void
EncryptPlaintextPublic( const char *keyfile, const char *plaintext )
{
	KeyData key;
	const size_t numWordsInKey = ReadKeyfile( keyfile, key ); Rtt_ASSERT( strlen( plaintext ) < (numWordsInKey << 2) );
	#pragma unused(numWordsInKey)

	public_key k( key.m, key.e );

	size_t numBytes = 0;
	U8 *ciphertext = AuthorizationTicket::PlainToCipherPublic( k, plaintext, numBytes );

	if ( Rtt_VERIFY( ciphertext ) )
	{
		for ( size_t i = 0; i < numBytes; i++ )
		{
			printf( "%02x", ciphertext[i] );
		}

		free( ciphertext );
	}
}

static bool
ReplaceSignature( const char* filepath, const U32 findBuf[], const U32 replaceBuf[], size_t bufLen, bool littleEndian )
{
	bool success = false;

	Rtt_ASSERT( filepath );

	int fd = Rtt_FileDescriptorOpen(filepath, O_RDWR, 0);

	if (fd == -1)
	{
		fprintf(stderr, "app_sign: cannot open file '%s' for reading\n", filepath);
		
		return false;
	}

	if ( Rtt_VERIFY( fd >= 0 ) )
	{
		size_t numBytes = sizeof(findBuf[0]) * bufLen;

		U8* findBytes = (U8*)malloc( numBytes );
		vlong_io::ToBytes( findBytes, findBuf, numBytes, littleEndian );

		U8* replaceBytes = (U8*)malloc( numBytes );
		vlong_io::ToBytes( replaceBytes, replaceBuf, numBytes, littleEndian );

		struct stat statbuf;
		int result = fstat( fd, & statbuf );

		if (result == -1)
		{
			fprintf(stderr, "app_sign: failed to stat '%s' (%s)\n", filepath, strerror(errno));

			return false;
		}

		//void* dst = mmap( NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
		void *dst = Rtt_FileMemoryMap(fd, 0, statbuf.st_size, true);

		void* findLoc = NULL;

		if (dst == NULL)
		{
#ifdef Rtt_WIN_ENV
			fprintf(stderr, "app_sign: failed to memory map %ld bytes of '%s' (%ld)\n", statbuf.st_size, filepath, GetLastError());
#else
			fprintf(stderr, "app_sign: failed to memory map %lld bytes of '%s' (%s)\n", statbuf.st_size, filepath, strerror(errno));
#endif

			return false;
		}

//		strnstr( dstBytes, (char*)findBytes, statbuf.st_size );
		void* p = dst;
		size_t pNumBytes = statbuf.st_size;
		char firstChar = findBytes[0];
		while ( p )
		{
			void* pNew = memchr( p, firstChar, pNumBytes );
			if ( pNew )
			{
				pNumBytes -= ((U8*)pNew) - ((U8*)p);
				if ( pNumBytes < numBytes )
				{
					break;
				}
				else if ( 0 == memcmp( pNew, findBytes, numBytes ) )
				{
					findLoc = pNew;
					break;
				}
				else
				{
					// Advance by 1 byte
					pNew = ((U8*)pNew) + 1;
					--pNumBytes;
				}
			}
			p = pNew;
		}
		
		if ( findLoc )
		{
			printf( "Replacing signature... " );
			memcpy( findLoc, replaceBytes, numBytes );
			Rtt_FileMemoryFlush( dst, statbuf.st_size );
			printf( "done.\n" );
			success = true;
		}
		else
		{
			fprintf(stderr, "app_sign: failed to replace signature *** (ReplaceSignature)\n" );
		}

		free( replaceBytes );
		free( findBytes );

		//munmap( dst, statbuf.st_size );
		Rtt_FileMemoryUnmap(dst, statbuf.st_size);

		Rtt_FileDescriptorClose(fd);
	}

	return success;
}

static bool
ReplaceBytes( const char* filepath, const U8 *findBytes, const U8 *replaceBytes, size_t numBytes )
{
	bool success = false;

	Rtt_ASSERT( filepath );

	int fd  = Rtt_FileDescriptorOpen(filepath, O_RDWR, 0);

	if (fd == -1)
	{
		fprintf(stderr, "app_sign: cannot open '%s' for reading\n", filepath);

		return false;
	}
	else
	{
		struct stat statbuf;
		int result = fstat( fd, & statbuf ); Rtt_ASSERT( result >= 0 );
		#pragma unused(result)


		// void* dst = mmap( NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
		void *dst = Rtt_FileMemoryMap(fd, 0, statbuf.st_size, true);

		if (dst == NULL)
		{
			fprintf(stderr, "app_sign: cannot map '%s' for writing\n", filepath);

			Rtt_FileDescriptorClose(fd);

			return false;
		}

		void* findLoc = NULL;

//		strnstr( dstBytes, (char*)findBytes, statbuf.st_size );
		void* p = dst;
		size_t pNumBytes = statbuf.st_size;
		char firstChar = findBytes[0];
		while ( p )
		{
			void* pNew = memchr( p, firstChar, pNumBytes );
			if ( pNew )
			{
				pNumBytes -= ((U8*)pNew) - ((U8*)p);
				if ( pNumBytes < numBytes )
				{
					break;
				}
				else if ( 0 == memcmp( pNew, findBytes, numBytes ) )
				{
					findLoc = pNew;
					break;
				}
				else
				{
					// Advance by 1 byte
					pNew = ((U8*)pNew) + 1;
					--pNumBytes;
				}
			}
			p = pNew;
		}
		
		if ( findLoc )
		{
			printf( "Replacing signature... " );
			memcpy( findLoc, replaceBytes, numBytes );
			Rtt_FileMemoryFlush( dst, statbuf.st_size );
			printf( "done.\n" );
			success = true;
		}
		else
		{
			printf( "*** Failed to replace signature *** (ReplaceBytes)\n" );
		}

		//munmap( dst, statbuf.st_size );
		Rtt_FileMemoryUnmap(dst, statbuf.st_size);

		Rtt_FileDescriptorClose(fd);
	}

	return success;
}

// Converts U32 to comma-delimited buffer in little-endian order
static U8 *
CreateBufferJS( const U32 *buf, size_t bufLen, size_t *outNumBytes, size_t numBytesAllocatedWithPadding )
{
	size_t bufNumBytes = bufLen * sizeof(buf[0]);

	// In worst case, each byte in 'buf' takes 4 characters
	// (3 actual characters, 0-255, plus 1 more for the comma)
	const int kCharactersPerByte = 4;
	size_t numBytesRequired = bufNumBytes * kCharactersPerByte;

	// The actual allocation can be larger than numBytesRequired
	// if we want extra space for padding
	// Add one for null-termination
	size_t numBytesAllocated = 1 + Max( numBytesRequired, numBytesAllocatedWithPadding );
	U8 *dst = (U8 *)malloc( numBytesAllocated );

	// Write out bytes in little endian order
	char *output = (char *)dst;
	const U32 *input = buf;
	for ( size_t i = 0; i < bufLen; i++ )
	{
		U32 value = input[i];
		
		U8 byte= (U8)(value & 0xff); // LSB is (value & 0x1)
		output += sprintf( output, "%d,", byte );
		
		byte = (U8)((value >> 8) & 0xff);
		output += sprintf( output, "%d,", byte );

		byte = (U8)((value >> 16) & 0xff);
		output += sprintf( output, "%d,", byte );

		byte = (U8)((value >> 24) & 0xff);
		output += sprintf( output, "%d,", byte );
		
		Rtt_ASSERT( ( (U8*)output - dst ) < (long)numBytesAllocated );
	}

	if ( outNumBytes )
	{
		* outNumBytes = (U8*)output - dst;

		// TEST CODE:
		// This tests case for when we write into the padded region
		// which can occur if the "replace" string is larger than the base "find" string
		// In that case, we have additional padding after the find string,
		// but we want to make sure the replacement preserves the number of
		// JS array entries.
		#if Rtt_TEST_LARGE_REPLACE_STRING
			const int kNumPaddedEntries = 1+VerifierData::kPaddingLen*sizeof(buf[0]);
			const int kBytesPerEntry = strlen( "0," );
			const int kNumPaddedBytes = kNumPaddedEntries * kBytesPerEntry;
			int n = (numBytesAllocatedWithPadding - kNumPaddedBytes);
			if ( * outNumBytes < n )
			{
				for ( ; ((U8*)output - dst) < n; )
				{
					output += sprintf( output, "0" ); // print leading 0 as a dummy character
				}
				* outNumBytes = (U8*)output - dst;
			}
		#endif
	}

	return dst;
}

static int
CalculateLengthPadded( const U32 buf[], size_t bufLen )
{
	int result = 0;
	char str[8];
	for ( size_t i = 0; i < bufLen; i++ )
	{
		U32 value = buf[i];
		U8 byte= (U8)(value & 0xff); // LSB is (value & 0x1)
		result += sprintf( str, "%d,", byte );
		
		byte = (U8)((value >> 8) & 0xff);
		result += sprintf( str, "%d,", byte );

		byte = (U8)((value >> 16) & 0xff);
		result += sprintf( str, "%d,", byte );

		byte = (U8)((value >> 24) & 0xff);
		result += sprintf( str, "%d,", byte );
	}
	
	return result;
}

static bool
ReplaceSignatureJS( const char* filepath, const U32 findBuf[], const U32 replaceBuf[], size_t findLen, size_t findLenPadded )
{
	size_t findLenJS = 0;
	U8 *findBufJS = CreateBufferJS( findBuf, findLen, & findLenJS, 0 );
	printf( "find: [%s]\n", findBufJS );

	#if Rtt_TEST_LARGE_REPLACE_STRING
		{
			int x = 0;
			U8 *findBufJSPadded = CreateBufferJS( findBuf, findLenPadded, & x, 0 );
			printf( "findPadded: [%s]\n", findBufJSPadded );
			free( findBufJSPadded );
		}
	#endif

	size_t replaceLenJSPadded = CalculateLengthPadded( findBuf, findLenPadded );
	size_t replaceLenJS = 0;
	U8 *replaceBufJS = CreateBufferJS( replaceBuf, findLen, & replaceLenJS, replaceLenJSPadded );
	char *replaceBufJSString = (char *)replaceBufJS;

	size_t iStart = replaceLenJS;
	size_t iMax = findLenJS;

	if ( replaceLenJS > findLenJS )
	{
		// We overwrote into the buffer space.
		// This buffer space contains the equivalent of 'kNumEntries' JavaScript array entries.
		//
		// We need to ensure there are that many entries, and then fill in remaining
		// characters with spaces ' '
		const int kNumEntries = VerifierData::kPaddingLen*sizeof(findBuf[0]);
		const char kDummyEntry[] = "0,";
		char *dst = replaceBufJSString + replaceLenJS;
		for ( int i = 0; i < kNumEntries; i++ )
		{
			dst += sprintf( dst, "%s", kDummyEntry );
			Rtt_ASSERT( (dst - replaceBufJSString) < (long)replaceLenJSPadded );
		}
		
		iStart = (dst - replaceBufJSString);
		iMax = replaceLenJSPadded;
	}

	// Fill remaining characters with spaces: ' '
	for ( size_t i = iStart; i < iMax; i++ )
	{
		replaceBufJSString[i] = ' ';
	}

	// When Rtt_TEST_LARGE_REPLACE_STRING is active, the number of commas
	// in the "repl" string should be the same as the num commas in "findPadded".
	// Otherwise, compare num commas in "find".
	printf( "repl: [%s]\n", replaceBufJS );
	Rtt_ASSERT( findLenJS >= replaceLenJS );

	bool success = ReplaceBytes( filepath, findBufJS, replaceBufJS, iMax );
	free( replaceBufJS );
	free( findBufJS );
	
	return success;
}

static bool
SignBinary( const Options& options )
{
	// TODO: Replace with ReadKeyfile( key )
	// Deserialize key file
	vlong p, q, m, e, d;
	FILE* f = Rtt_FileOpen( options.file1, "r" );

	if (f == NULL)
	{
		fprintf(stderr, "app_sign: cannot open key file '%s' for reading\n", options.file1);

		return false;
	}
	else
	{
		vlong_io::FileReadVlong( f, "[p]", p );
		vlong_io::FileReadVlong( f, "[q]", q );
		vlong_io::FileReadVlong( f, "[m]", m );
		vlong_io::FileReadVlong( f, "[e]", e );
		vlong_io::FileReadVlong( f, "[d]", d );
		Rtt_FileClose( f );
	}

	// Let d be the private key
	// To sign a data M, represent the data as an integer in the range 0 < M < n
	//    where n is the key length
	// Signature C = M^d mod n [DECRYPT using private key]

	if (! Rtt_FileExists(options.file2))
	{
		fprintf(stderr, "app_sign: file '%s' does not exist\n", options.file2);

		return false;
	}

	Message msg( options.file2 );
	const size_t kDigestLen = (MD4::kNumBytesDigest >> 2);
	const size_t kMsgBufferLen = kDigestLen + 1;
	U32 msgBuffer[kMsgBufferLen];
	MD4Decode( msgBuffer, msg.Data().digest, MD4::kNumBytesDigest );
	msgBuffer[kDigestLen] = msg.Data().timestamp;

	for ( size_t i = 0; i < MD4::kNumBytesDigest; i++ )
	{
		printf( "%02x", msg.Data().digest[i] );
	}
	printf( "   %s\n", options.file2 );

	vlong cPlain;
	cPlain.load( (unsigned*)msgBuffer, kMsgBufferLen ); Rtt_STATIC_ASSERT( sizeof( U32 ) == sizeof( unsigned ) );

	// Pad remaining plain msg with random bits.  Subtract 1 word since msg len < key len
	cPlain.pad( (Message::kApplicationLen - 1) - kMsgBufferLen );

	private_key keyPrivate( p, q );
	vlong c = keyPrivate.decrypt( cPlain );

#ifdef Rtt_DEBUG
	{
		vlong_io::PrintVlong( "[p]", p );
		vlong_io::PrintVlong( "[q]", q );
		vlong_io::PrintVlong( "[m]", m );
		vlong_io::PrintVlong( "[e]", e );
		vlong_io::PrintVlong( "[d]", d );

		public_key keyPublic( m, e );
		vlong cDecrypted = keyPublic.encrypt( c );
		Rtt_ASSERT( cDecrypted == cPlain );

		vlong_io::PrintVlong( "[cPlain]", cPlain );
		vlong_io::PrintVlong( "[cDecrypted]", cDecrypted );
		vlong_io::PrintVlong( "[c]", c );
	}
#endif

//printf( "length****** %d %d\n", options.patternLen, VerifierData::kVerifierDataLen );
	Rtt_ASSERT( options.patternLen == VerifierData::kVerifierDataLen );

	U32* data = (U32*)malloc( VerifierData::kVerifierDataLen*sizeof(unsigned) );
	m.store( (unsigned*)VerifierData::ModulusBase( data ), VerifierData::kModulusLen );
	c.store( (unsigned*)VerifierData::SignatureBase( data ), VerifierData::kSignatureLen );

	U32* exponent = VerifierData::ExponentBase( data );
	int numWords = e.numWords();
	e.store( (unsigned*)exponent, numWords );
	exponent += numWords;
	int numZeroWords = VerifierData::kExponentLen - numWords;
	for( int i = 0; i < numZeroWords; i++ )
	{
		*exponent++ = 0;
	}

	bool result = false;
	if ( options.isJavaScript )
	{
		Rtt_ASSERT( VerifierData::kVerifierDataLen == gPatternDataLen );
		result = ReplaceSignatureJS( options.file3, gPatternData, (U32*)data, gPatternDataLen, gPatternDataLenPadded );
	}
	else
	{
		result = ReplaceSignature( options.file3, options.pattern, (U32*)data, options.patternLen, options.littleEndian );
	}
	free( data );

	if ( /* DISABLES CODE */ (0) )
	{
		MessageDigest digest;
	//	digest.Initialize( NULL, __FILE__, MessageDigest::kMD4 );
		digest.Initialize( NULL, options.file2, MessageDigest::kMD4 );
		digest.Print( options.file2 );
		U32 digestU32[MD4::kNumBytesDigest >> 2];
		const size_t kDigestLen = sizeof( digestU32 ) / sizeof( digestU32[0] );
		MD4Decode( digestU32, *digest, MD4::kNumBytesDigest );
		vlong_io::PrintU32( "md4", digestU32, kDigestLen );

		vlong plain;
		plain.load( (unsigned*)digestU32, kDigestLen ); Rtt_STATIC_ASSERT( sizeof( U32 ) == sizeof( unsigned ) );
		plain.pad( 15 );

		private_key keyPrivate( p, q );
		vlong c = keyPrivate.decrypt( plain );

		public_key keyPublic( m, e );
		vlong mm = keyPublic.encrypt( c );
		Rtt_ASSERT( mm == plain );
		vlong_io::PrintVlong( "[c]    ", c );
		vlong_io::PrintVlong( "[plain]", plain );
		vlong_io::PrintVlong( "[mm]   ", mm );
	}

	return result;
}

static void
VerifyBinary( const Options& options )
{
	// Let M be the msg and C be the signature
	// Calculate MM = C^e mod n, where e is the public key [ENCRYPT using public key]
	// if MM == M, then the signature is verified, else failed 
}

// #endif // Rtt_WIN_ENV

// ----------------------------------------------------------------------------

static Command
ParseCmd( const char* cmd )
{
	Command result = kNone;

	if ( 0 == strcmp( cmd, "digest" ) )
	{
		result = kDigest;
	}
	else if ( 0 == strcmp( cmd, "create" ) )
	{
		result = kCreate;
	}
	else if ( 0 == strcmp( cmd, "decrypt" ) )
	{
		result = kDecrypt;
	}
	else if ( 0 == strcmp( cmd, "encrypt" ) )
	{
		result = kEncrypt;
	}
	else if ( 0 == strcmp( cmd, "decrypt-private" ) )
	{
		result = kDecryptPrivate;
	}
	else if ( 0 == strcmp( cmd, "encrypt-public" ) )
	{
		result = kEncryptPublic;
	}
	else if ( 0 == strcmp( cmd, "sign" ) )
	{
		result = kSign;
	}
	else if ( 0 == strcmp( cmd, "sign-js" ) )
	{
		result = kSignJavaScript;
	}
	else if ( 0 == strcmp( cmd, "verify" ) )
	{
		result = kVerify;
	}

	return result;
}

static bool
Parse( int argc, const char* argv[], Options& options )
{
	bool result = false;

	options.isJavaScript = false;

	if ( argc > 2 )
	{
		Command cmd = ParseCmd( argv[1] );
		if ( kNone == cmd ) { goto exit_gracefully; }

		switch( cmd )
		{
			case kDigest:
			case kVerify:
				options.file1 = argv[2];
				break;
			case kCreate:
				{
					options.file1 = argv[2];

					const U32 kDefaultKeyBytes = 64; // Default is 64 byte key
					U32 keyBytes = ( argc > 3 ? atoi( argv[3] ) : kDefaultKeyBytes );
					if ( 0 == keyBytes )
					{
						keyBytes = kDefaultKeyBytes;
					}

					// Key length in bytes must be a multiple of 2.
					// Promote keyBytes to next multiple of 2.
					if ( keyBytes & 0x1 )
					{
						fprintf( stderr, "app_sign: Key length (%d bytes) is not a multiple of 2.\n", keyBytes );
						++keyBytes;
						fprintf( stderr, "app_sign: Using length (%d) instead\n", keyBytes );
					}
					options.param1 = keyBytes;
				}
				break;
			case kDecrypt:
			case kEncrypt:
			case kDecryptPrivate:
			case kEncryptPublic:
				{
					if ( argc > 3 )
					{
						options.file1 = argv[2];
						options.file2 = argv[3]; // treat this as a string
					}
					else
					{
						fprintf(stderr, "app_sign: incorrect number of arguments for '%s'\n", argv[1]);

						goto exit_gracefully;
					}
				}
				break;
			case kSignJavaScript:
				options.isJavaScript = true;
				// Fall through to kSign

			case kSign:
				if ( argc > 5 )
				{
					options.file1 = argv[2]; // key file
					options.file2 = argv[3]; // src file used to generate msg digest
					options.file3 = argv[4]; // file to sign (will be modified)
					options.littleEndian = 0 != strcmp( argv[5], "big" ); // default to little unless "big" is specified
#ifdef NOT_USED
					// TODO: figure what this code does and trigger it by some sane mechanism (not one that is
					// subject to the random extra parameters macOS throws at commands depending on how they are
					// started)
					if ( argc > 6 )
					{
						vlong p;
						FILE* f = Rtt_FileOpen( options.file1, "r" );

						if (f == NULL)
						{
							fprintf(stderr, "app_sign: cannot open '%s' for reading\n", options.file1);

							goto exit_gracefully;
						}
						else
						{
						vlong_io::FileReadVlong( f, "[pattern]", p );  vlong_io::PrintVlong( "[pattern]", p );
						Rtt_FileClose( f );
						}

						p.store( (unsigned*)gPatternData, gPatternDataLen );
					}
#endif // NOT_USED
					options.pattern = gPatternData;
					options.patternLen = gPatternDataLen;
				}
				else
				{
					goto exit_gracefully;
				}
				break;
			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
		}

		options.cmd = cmd;
		result = true;
	}

exit_gracefully:
	return result;
}

static void
Usage( const char* arg0 )
{
	fprintf( stderr, "%s command args\n", arg0 );
	fprintf( stderr, "command types and their arguments:\n" );
	fprintf( stderr, "\tdigest srcfile\n" );
	fprintf( stderr, "\tcreate dstfile [numBytesInKey=64]\n" );
	fprintf( stderr, "\tdecrypt keyfile ciphertext\n" );
	fprintf( stderr, "\tencrypt keyfile plaintext\n" );
	fprintf( stderr, "\tdecrypt-private keyfile ciphertext\n" );
	fprintf( stderr, "\tencrypt-public keyfile plaintext\n" );
	fprintf( stderr, "\tsign keyfile srcMsgFile fileToSign {little,big} [fileWithSearchPattern]\n" );
	fprintf( stderr, "\tverify srcfile\n" );
	fprintf( stderr, "\tsign-js keyfile srcMsgFile fileToSign {little,big} [fileWithSearchPattern]\n" );
}

// ----------------------------------------------------------------------------

Rtt_EXPORT int
Rtt_AppSignMain( int argc, const char *argv[] )
{
	srand( (unsigned) time( NULL ) );

	int result = 0;

	Options options;

	if ( Parse( argc, argv, options ) )
	{
		switch( options.cmd )
		{
			case kDigest:
				CalculateDigest( options );
				break;
			case kCreate:
				CreateCertificate( options );
				break;
			case kDecrypt:
				DecryptCiphertext( options.file1, options.file2 );
				break;
			case kEncrypt:
				EncryptPlaintext( options.file1, options.file2 );
				break;
			case kDecryptPrivate:
				DecryptCiphertextPrivate( options.file1, options.file2 );
				break;
			case kEncryptPublic:
				EncryptPlaintextPublic( options.file1, options.file2 );
				break;
			case kSignJavaScript:
			case kSign:
				if ( ! SignBinary( options ) )
				{
					result = -2;
				}
				break;
			case kVerify:
				VerifyBinary( options );
				break;
			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
		}
	}
	else
	{
		Usage( argv[0] );
		result = -1;
	}

    return result;
}
