//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "Core/Rtt_New.h"

#include <string.h>
#include <ctype.h>

// ----------------------------------------------------------------------------

Rtt_EXPORT_BEGIN

// ----------------------------------------------------------------------------

static char *
Rtt_StringCopy( Rtt_Allocator *pAllocator, const char *key )
{
	size_t keyLen = strlen( key );
	char *result = (char *)Rtt_MALLOC( pAllocator, sizeof( char ) * (keyLen + 1) );
	strncpy( result, key, keyLen );
	result[keyLen] = '\0';
	return result;
}

int
Rtt_StringCompareNoCase( const char * s1, const char * s2 )
{
	if ( s1 == NULL || s2 == NULL ) {
		if ( s1 == NULL && s2 == NULL )
			return 0;
		if ( s1 == NULL )
			return -1;
		return 1;
	}
#if defined( Rtt_WIN_ENV )
	return _stricmp( s1, s2 );
#else
	return strcasecmp( s1, s2 );
#endif
}

// NULL safe string compare
int
Rtt_StringCompare( const char * s1, const char * s2 )
{
	if ( s1 == NULL || s2 == NULL ) {
		if ( s1 == NULL && s2 == NULL )
			return 0;
		if ( s1 == NULL )
			return -1;
		return 1;
	}

	return strcmp( s1, s2 );
}

// Returns true if the string ends with the specified characters
int
Rtt_StringEndsWith( const char * s, const char * last )
{
	if ( s == NULL || last == NULL )
		return false;
		
	size_t length = strlen( s );
	size_t lastLength = strlen( last );
	if ( lastLength > length )
		return false;
	
	const char * p = s + length - lastLength;
	return Rtt_StringCompare( p, last ) == 0;
}

// Returns true if the string ends with the specified characters, case insensitive
int
Rtt_StringEndsWithNoCase( const char * s, const char * last )
{
	if ( s == NULL || last == NULL )
		return false;
		
	size_t length = strlen( s );
	size_t lastLength = strlen( last );
	if ( lastLength > length )
		return false;
	
	const char * p = s + length - lastLength;
	return Rtt_StringCompareNoCase( p, last ) == 0;
}


// Returns true if the string starts with the specified string
int
Rtt_StringStartsWith( const char * s, const char * prefix )
{
	if ( s == NULL || prefix == NULL )
		return false;
	size_t prefixLength = strlen( prefix );

	return strncmp( s, prefix, prefixLength ) == 0;
}

// Returns true if the string is NULL or ""
bool
Rtt_StringIsEmpty( const char *s )
{
	return ( (NULL == s) || ('\0' == s[0]) );
}

// ----------------------------------------------------------------------------

Rtt_EXPORT_END

// ----------------------------------------------------------------------------

namespace Rtt 
{

// ----------------------------------------------------------------------------

String::String()
:	fBuffer( NULL ),
	fSrcBuffer( NULL ),
	fAllocator( NULL ),
#ifdef Rtt_WIN_ENV
	fPathSeparator("\\/")
#else
	fPathSeparator("/")
#endif
{
}

String::String( Rtt_Allocator * a )
:	fBuffer( NULL ),
	fSrcBuffer( NULL ),
	fAllocator(a),
#ifdef Rtt_WIN_ENV
	fPathSeparator("\\/")
#else
	fPathSeparator("/")
#endif
{
}

String::String(Rtt_Allocator * a, const char * str)
	: fBuffer(NULL),
	fSrcBuffer(NULL),
	fAllocator(a),
#ifdef Rtt_WIN_ENV
	fPathSeparator("\\/")
#else
	fPathSeparator("/")
#endif
{
	if (str != NULL)
		fBuffer = Rtt_StringCopy(fAllocator, str);
}

String::String(const char * str)
	: fBuffer(NULL),
	fSrcBuffer(NULL),
	fAllocator(NULL),
#ifdef Rtt_WIN_ENV
	fPathSeparator("\\/")
#else
	fPathSeparator("/")
#endif
{
	if (str != NULL)
	{
		fBuffer = Rtt_StringCopy(fAllocator, str);
	}
}

String::~String()
{
	if (fBuffer)
	{
		Rtt_FREE(fBuffer);
	}
}

void
String::Append(const char *str)
{
	if (fSrcBuffer)
	{
		// Copy srcbuffer (copy on write semantics)
		Set(fSrcBuffer);
		fSrcBuffer = NULL;
	}

	size_t l1 = (!fBuffer ? 0 : strlen(fBuffer));
	size_t l2 = strlen(str);

	char *result = (char*)Rtt_MALLOC(fAllocator, sizeof(char) * (l1 + l2 + 1));
	if (fBuffer == NULL)
		result[0] = 0;
	else
		strcpy(result, fBuffer);
	strcat(result + l1, str);

	Rtt_FREE(fBuffer);
	fBuffer = result;
}

template<typename T, T kPrime, T kOffsetBasis>
T
GetHash( const char* buffer )
{
	// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
    T hash = kOffsetBasis;

	if ( NULL != buffer ) // if absent, interpret as empty string
	{
		for ( int i = 0; buffer[i]; i++)
		{
			hash *= kPrime;
			hash ^= buffer[i];
		}
	}

    return hash;
}

U32
String::GetHash32() const
{
	return GetHash<U32, 0x01000193, 0x811C9DC5>( GetString() );
}

U64
String::GetHash64() const
{
	return GetHash<U64, 0x00000100000001B3, 0xCBF29CE484222325>( GetString() );
}

void
String::Trim(const char *trimChars)
{
	LTrim(trimChars);
	RTrim(trimChars);
}

void
String::LTrim(const char *trimChars)
{
	if (trimChars == NULL || trimChars[0] == 0)
	{
		return;
	}

	if (fSrcBuffer)
	{
		// Copy srcbuffer (copy on write semantics)
		Set(fSrcBuffer);
		fSrcBuffer = NULL;
	}

	size_t len = (fBuffer == NULL ? 0 : strlen(fBuffer));

	if (len > 0)
	{
		size_t trimAmount = strspn(fBuffer, trimChars);

		if (trimAmount == len)
		{
			// Everything was trimmed
			Set("");
		}
		else if (trimAmount > 0)
		{
			char *result = (char*)Rtt_MALLOC(fAllocator, sizeof(char) * (len - trimAmount));

			strcpy(result, &fBuffer[trimAmount]);

			SetSrc(result);
		}
	}
}

void
String::RTrim(const char *trimChars)
{
	if (trimChars == NULL || trimChars[0] == 0)
	{
		return;
	}

	if (fSrcBuffer)
	{
		// Copy srcbuffer (copy on write semantics)
		Set(fSrcBuffer);
		fSrcBuffer = NULL;
	}

	size_t len = (fBuffer == NULL ? 0 : strlen(fBuffer));
	size_t loc = len;

	if (loc > 0)
	{
		while (strchr(trimChars, fBuffer[--loc]) != NULL)
			;

		++loc; // back up one

		if (loc == 0)
		{
			Set("");
		}
		else if (loc != len)
		{
			fBuffer = (char*)Rtt_REALLOC(fAllocator, fBuffer, (sizeof(char) * loc) + 1);

			fBuffer[loc] = 0;
		}
	}
}

// Encoding bit layout, for bytes 0-3:
// 0 0 0 0 0 0 1 1 | 1 1 1 1 2 2 2 2 | 2 2 3 3 3 3 3 3

enum {

	#define COUNT_AND_OFFSET( NAME, COUNT, OFFSET ) kCount##NAME = COUNT, kOffset##NAME = OFFSET
	#define AFTER_PREV( PREV ) kOffset##PREV + kCount##PREV
	#define NEXT_COUNT_AND_OFFSET( NAME, OFFSET ) COUNT_AND_OFFSET( NAME, kMax##NAME - kMin##NAME + 1, OFFSET )

	kMinUpper = 'A', kMaxUpper = 'Z',
	kMinLower = 'a', kMaxLower = 'z',
	kMinDigit = '0', kMaxDigit = '9',

	NEXT_COUNT_AND_OFFSET( Upper, 0 ),
	NEXT_COUNT_AND_OFFSET( Lower, AFTER_PREV( Upper ) ),
	NEXT_COUNT_AND_OFFSET( Digit, AFTER_PREV( Lower ) ),
	kOffsetUnderscore = AFTER_PREV( Digit ),
	kOffsetNUL = kOffsetUnderscore + 1

	#undef COUNT_AND_OFFSET
	#undef AFTER_PREV
	#undef NEXT_COUNT_AND_OFFSET
	
};

Rtt_STATIC_ASSERT( kOffsetNUL + 1 == 64 );

int
String::EncodeIdentifier( U8* buf, const char* ident, int maxTriples )
{
	if ( !isalpha( *ident ) && *ident != '_' )
	{
		Rtt_LogException( "ERROR: Identifier must begin with a letter or underscore, got '%c'", *ident );
	
		return -1;
	}
	
	// TODO? empty string; maxTriples == 0
	
	#define PLUS_1_PRED( COND, RESULT ) ( ( COND ) ? 1 + ( RESULT ) : 0 )
	#define OFFSET_PLUS_1( CHAR, NAME ) PLUS_1_PRED( ( (U32)( CHAR - kMin##NAME ) ) <= ( (U32)( kMax##NAME - kMin##NAME ) ), kOffset##NAME + CHAR - kMin##NAME )

	int k = 0, bufSize = maxTriples * 3, bad = 0;
	
	do {
		U8 work[4] = { kOffsetNUL, kOffsetNUL, kOffsetNUL, kOffsetNUL };
	
		for (int j = 0; *ident && j < 4; ++ident, ++j)
		{
			int c = *ident;
			U8 code = PLUS_1_PRED( 0 == c, kOffsetNUL ) | PLUS_1_PRED( '_' == c, kOffsetUnderscore ) |
						OFFSET_PLUS_1( c, Upper ) | OFFSET_PLUS_1( c, Lower ) | OFFSET_PLUS_1( c, Digit );

			bad = ( 0 == code ) ? c : bad;
			work[j] = code - 1;
		}

		// This check prevents overflow, but will lead to bogus results. This is mainly
		// intended to detect encodability, in which case the results are thrown away.
		k = Min( k + 3, bufSize );

		buf[k - 3] = ( work[0] << 2 ) | ( work[1] >> 4 );
		buf[k - 2] = ( work[1] << 4 ) | ( work[2] >> 2 );
		buf[k - 1] = ( work[2] << 6 ) | ( work[3] >> 0 );
	} while ( *ident );

	#undef PLUS_1_PRED
	#undef OFFSET_PLUS_1

	if ( bad )
	{
		Rtt_LogException( "ERROR: Non-identifier character(s) found, including %c", bad );
		
		return -1;
	}

	return k;
}

bool 
String::IsIdentifier( const char* name )
{
	U8 junk[3];

	return EncodeIdentifier( junk, name, 1 ) > 0;
}

U32
String::DecodeIdentifier( char* name, const U8* buf, int numTriples )
{
	const char* start = name;
	for ( int i = 0; i < numTriples; i++, buf += 3 )
	{
		U32 b1 = buf[0], b2 = buf[1], b3 = buf[2];
		U32 work[] = {
			b1 >> 2,
			( ( b1 & 0x3 ) << 4 ) | ( b2 >> 4 ),
			( ( b2 & 0xF ) << 2 ) | ( b3 >> 6 ),
			b3 & 0x3F,
			kOffsetNUL
		};
			
		for ( int k = 0; work[k] < kOffsetNUL; ++k )
		{
			static const char kSymbols[] = 
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz"
				"0123456789_";
			
			*name++ = kSymbols[work[k]];
		}
	}
		
	*name = 0;

	return (U32)( name - start );
}

int
String::IdentifierLengthToTriples( int length )
{
	return ( length + 3 ) / 4;
}

void
String::AppendPathComponent(const char *pathComponent)
{
	if (pathComponent == NULL)
	{
		return;
	}

	if (fSrcBuffer)
	{
		// Copy srcbuffer (copy on write semantics)
		Set(fSrcBuffer);
		fSrcBuffer = NULL;
	}

	size_t len = (fBuffer == NULL ? 0 : strlen(fBuffer));

	if (len == 0)
	{
		Set(pathComponent);
	}
	else
	{
		if (!IsPathSeparator(fBuffer[len - 1]) && !IsPathSeparator(pathComponent[0]))
		{
			// No path separator on either string, add one
			char buf[2] = {};
			buf[0] = fPathSeparator[0];  // first is the default

			Append(buf);
		}
		else if (IsPathSeparator(fBuffer[len - 1]) && IsPathSeparator(pathComponent[0]))
		{
			// Path separators on both strings, remove one
			RTrim(fPathSeparator);
		}

		Append(pathComponent);
	}
}

void String::RemovePathComponent()
{
	RTrim(fPathSeparator); // normalize

	size_t len = (fBuffer == NULL ? 0 : strlen(fBuffer));

	if (len > 0)
	{
		char *lastPathSeparatorPos = NULL;

		// Handle case where there's more than one possible path separator
		for (size_t i = 0; i < strlen(fPathSeparator); i++)
		{
			char *tmpPtr = strrchr(fBuffer, fPathSeparator[i]);

			// We want the last matching separator
			lastPathSeparatorPos = (tmpPtr > lastPathSeparatorPos) ? tmpPtr : lastPathSeparatorPos;
		}

		if (lastPathSeparatorPos != NULL)
		{
			*lastPathSeparatorPos = 0;

			fBuffer = (char*)Rtt_REALLOC(fAllocator, fBuffer, (sizeof(char) * strlen(fBuffer)) + 1);
		}
	}
}

void
String::AppendPathSeparator()
{
	AppendPathComponent("");
}

void
String::RemovePathSeparator()
{
	RTrim(fPathSeparator);
}

const char *String::GetLastPathComponent()
{
	RTrim(fPathSeparator); // normalize

	size_t len = (fBuffer == NULL ? 0 : strlen(fBuffer));

	if (len > 0)
	{
		char *lastPathSeparatorPos = NULL;

		// Handle case where there's more than one possible path separator
		for (size_t i = 0; i < strlen(fPathSeparator); i++)
		{
			char *tmpPtr = strrchr(fBuffer, fPathSeparator[i]);

			// We want the last matching separator
			lastPathSeparatorPos = (tmpPtr > lastPathSeparatorPos) ? tmpPtr : lastPathSeparatorPos;
		}

		if (lastPathSeparatorPos != NULL)
		{
			return lastPathSeparatorPos + 1;
		}
		else
		{
			return fBuffer;
		}
	}
	else
	{
		return NULL;
	}
}

void
String::Reserve( size_t length )
{
	if ( fSrcBuffer )
		fSrcBuffer = NULL;

	Rtt_FREE( fBuffer );
	char * result = (char*)Rtt_MALLOC( fAllocator, length + 1 );
	fBuffer = result;
}

void
String::Set( const char *str )
{
	// We no longer need this b/c we're setting a new string
	fSrcBuffer = NULL;

	if ( str != fBuffer )
	{
		if ( fBuffer )
		{
			Rtt_FREE( fBuffer );
		}

		if ( str == NULL )
		{
			fBuffer = NULL;
		}
		else
		{
			fBuffer = Rtt_StringCopy( fAllocator, str );
		}
	}
}

StringArray::StringArray( Rtt_Allocator * a )
:	fLength( 0 ),
	fElements( NULL ),
	fAllocator( a )
{
}

StringArray::~StringArray()
{
	for ( int i = 0; i < fLength; i++ )
	{
		fElements[i].~String();
	}
	Rtt_FREE( fElements );
}

void
StringArray::Reserve( int length )
{
	Rtt_ASSERT( fElements == NULL );
	fElements = (String *) Rtt_MALLOC( fAllocator, length * sizeof( String ) );

	Rtt_ASSERT( fElements );

	if ( !fElements )
		return;

	fLength = length;

	String * nextElement = fElements;

	for ( int i = 0; i < length; i++ ) {
		new( nextElement++ ) String( fAllocator );
	}
}

void
StringArray::SetElement( int index, const char *str )
{
	Rtt_ASSERT( index < fLength );

	if ( index < fLength )
		fElements[index].Set( str );
}

const char *
StringArray::GetElement( int index ) const
{
	Rtt_ASSERT( index < fLength );

	if ( index >= fLength )
		return NULL;

	return fElements[index].GetString();
}

} // namespace Rtt
