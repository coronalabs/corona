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
#include "Core/Rtt_StringHash.h"
#include "Core/Rtt_Math.h"
#include "Core/Rtt_String.h"
#include <string.h>
#include <ctype.h>

#if defined(Rtt_WIN_ENV) || defined(Rtt_NINTENDO_ENV)
// strndup() is POSIX so when we update VC we'll get it officially.  For now, define a helper on Win32
static char *
strndup (const char *src, size_t size)
{
    char *result = NULL;

    result = (char *) malloc(size+1);

    if (size > 0 && result != NULL)
    {
        memcpy(result, src, size);
        result[size] = '\0';
    }

    return result;
}
#endif // Rtt_WIN_ENV

// ----------------------------------------------------------------------------

/*
This module defines a fast, compact, self-initializing hash table.

Sample usage:

First, define a table of keys. This must be static so it stays resident.
	static const char * keys[] = 
	{
		"setColor",
		"append"
	};

Next, define a StringHash to lookup the keys. This calculates a perfect
hash function once.
This also must be static, to preserve the hash function across invocations.

	static StringHash * hash;
	
	if ( !hash )
	{
		hash = Rtt_NEW( allocator, StringHash( *allocator, keys, sizeof( keys ) / sizeof( const char * ), A, B, C, __FILE__, __LINE__ ) );
	}

A, B, C above are initialization parameters, Set them to 0 to start, you'll update them later.

Next, calculate an index for a particular key. This returns an integer
in the same order as the table of keys. It returns -1 if the key did not match.
	int index = hash->Lookup( key );

Next, you can use this index to perform whatever you want. Using switch
is pretty optimal, and you can use default to generate an error or handle
any other situation.

	switch ( index )
	{
	case 0:
		// handle setColor
		break;
	case 1:
		// handle append
		break;
	default:
		// handle none of the above
		break;
	}

Now run the code in debug. In the output, you'll see output something like this:

	StringHash [10 0 1] 3, 0, 1
	StringHash [62 5 52] 22, 17, 6
	StringHash: update needed at /Volumes/Ansca/code/main/platform/mac/../../librtt/Rtt_LuaProxyVTable.cpp line 2026
	StringHash [9 0 1] 2, 0, 1
	(etc)

The "update needed" means that StringHash had to do extra computation to find the right initialization parameters. These are given
on the next line. Copy the parameters into the initializer's parameters A, B, C. In this example, the parameters are 2, 0, 1.
So we'd update it as follows:
		hash = Rtt_NEW( allocator, StringHash( *allocator, keys, sizeof( keys ) / sizeof( const char * ), 2, 0, 1, __FILE__, __LINE__ ) );

Any trailing punctuation character is ignored (so 'stageWidth' is equal to 'stageWidth#') to allow callers to decorate key tables
to indicate things like deprecation without affecting the lookup of keys.
*/

namespace Rtt
{
	template class IndexArray<unsigned int>;
	template class IndexArray<unsigned short>;

	static const unsigned int sPrimes[] = { 
		17,
		5381,
		27541, 
		29191, 
		29611, 
		31321, 
		34429, 
		36739, 
		40099, 
		40591, 
		42589,
		46691, 
		50821, 
		54251, 
		56897, 
		57793, 
		65213, 
		65599,
		68111, 
		72073, 
		76147, 
		84631, 
		89041, 
		93563, 
		28813, 
		37633, 
		43201, 
		47629, 
		60493, 
		63949, 
		65713, 
		69313, 
		73009, 
		76801
		};
	static const unsigned int sNumPrimes = sizeof( sPrimes ) / sizeof( unsigned int );

	static unsigned int calculateHash( const char * key, unsigned int count, unsigned int tableStart, unsigned int firstChar )
	{
		unsigned int i, prime, keyIndex, hash;

		keyIndex = 0;
		hash = 1;
		prime = tableStart;

#if 1
		hash = sPrimes[ prime ];
#endif
		
		// Start at an interesting char
		for ( i = 0; i < firstChar; i++, keyIndex++ )
		{
			if ( key[ keyIndex ] == 0 )
				keyIndex = 0;
		}
		
		for ( i = 0; i < count; i++, keyIndex++ )
		{
			if ( key[ keyIndex ] == 0 )
			{
				keyIndex = 0;
			}

#if 0
			unsigned int f1 = hash * sPrimes[ prime ];
			unsigned int f2 = key[ keyIndex ];

			hash = (f1 + f2);
#endif
#if 1
			hash = (hash ^ key[ keyIndex ]) + ((hash<<26)+(hash>>6));
#endif
			
#if 0
			prime++;
			if ( prime >= sNumPrimes )
				prime = 0;
#endif
		}

		return hash;
	}

	static void calculateAllHashes( IndexArray<unsigned int> & hashes, char ** keys, unsigned int keyCount,
		unsigned int tableStart, unsigned int count, unsigned int tableSize, unsigned int firstChar )
	{
		for ( unsigned int i = 0; i < keyCount; i++ ) {
			hashes[ i ] = calculateHash( keys[ i ], count, tableStart, firstChar ) % tableSize;
		}
	}

	static size_t calculateMaxKeyLength( char ** keys, unsigned int keyCount )
	{
		size_t maxLength = 0;

		for ( unsigned int i = 0; i < keyCount; i++ ) {
			size_t length = strlen( keys[ i ] );
			if ( length > maxLength )
				maxLength = length;
		}

		return maxLength;
	}

	static unsigned int numCollisions( const IndexArray<unsigned int> & hashes )
	{
		unsigned int collisions = 0;
		unsigned int hashCount = hashes.Length();

		for ( unsigned int i = 0; i < hashCount; i++ )
		{
			for ( unsigned int j = i + 1; j < hashCount; j++ )
			{
				if ( hashes[i] == hashes[j] )
				{
					collisions++;

#ifndef Rtt_DEBUGGER
					return collisions; // we don't realy care how many, just that there are
#endif
				}
			}
		}

		return collisions;
	}

	static unsigned int minPrefixLength( char ** keys, unsigned int keyCount )
	{
		unsigned int prefix = 0;

		for ( unsigned int i = 0; i < keyCount; i++ )
		{
			for ( unsigned int j = i + 1; j < keyCount; j++ )
			{
				unsigned int count;

				for ( count = 0; keys[i][count] == keys[j][count]; count++ )
				{
					// If this happens, you have two identical keys
					Rtt_ASSERT( (keys[i][count] != 0) || (keys[j][count] != 0) );

					if ( keys[i][count] == 0 || keys[j][count] == 0 )
						break;
				}

				if ( count > prefix )
					prefix = count;
			}
		}
		
		return prefix;
	}

	unsigned int
	StringHash::tryTableSizeAndCharCount( IndexArray<unsigned int> &hashes, unsigned int tableSize, 
										 unsigned int hashCharCount, unsigned int tableStart )
	{
		unsigned int i;
		unsigned int collisions = 0;

		// Try different table starts
		for ( ; tableStart < sNumPrimes; tableStart++ )
		{
#ifdef Rtt_DEBUG
			fTryCount++;
#endif

			calculateAllHashes( hashes, fKeys, fKeyCount, tableStart, hashCharCount, tableSize, fFirstInterestingChar );

			collisions = numCollisions( hashes );

			if ( collisions == 0 )
			{
				// No collisions, so this one's good to go
				fIndices.Reserve( tableSize, -1 );

				for ( i = 0; i < hashes.Length(); i++ )
				{
					fIndices[ hashes[ i ] ] = i;
				}

				fTableStart = tableStart;
				fHashCharCount = hashCharCount;

#ifdef Rtt_DEBUG
				// these values can be used as parameters to the constructor to reduce init time
				// tableStart, hashCharCount, fFirstInterestingChar
				
				Rtt_TRACE( ( "StringHash [%d %d %d]   (A,B,C) = (%d, %d, %d)\n", tableSize, fFirstInterestingChar, fTryCount, fKeyCount, tableStart, hashCharCount ) );
#endif
				
				return collisions;
			}
		}

		// Couldn't find optimal hash; give up
		return collisions;
	}

	unsigned int
	StringHash::tryTable( Rtt_Allocator & allocator, unsigned int maxKeyLength )
	{
		unsigned int hashCharCount;
		IndexArray<unsigned int> hashes( allocator );
		unsigned int tableSize = fKeyCount;
		unsigned int collisions = 0;

		hashCharCount = fHashCharCount;
		hashes.Reserve( fKeyCount, -1 );
		
		// If this happens, there are two identical keys
		Rtt_ASSERT( fFirstInterestingChar < maxKeyLength );

		for ( ; hashCharCount <= maxKeyLength; hashCharCount++ )
		{
			for ( tableSize = fKeyCount + 7; tableSize < 255; tableSize += 11 )
			{
				collisions = tryTableSizeAndCharCount( hashes, tableSize, hashCharCount, fTableStart );
				if ( collisions == 0 )
				{
					return collisions;
				}
			}
		}

		// Couldn't find a good hash function. If this happens, probly need more primes in sPrimes.
		return collisions;
	}

	StringHash::StringHash( Rtt_Allocator & allocator, const char ** keys, unsigned int keyCount,
		unsigned int lastKeyCount, unsigned int tableStart, unsigned int hashCharCount,
	    const char * file, unsigned int line
		) : fIndices( allocator )
	{
		fKeyCount = keyCount;
        fOriginalKeys = keys;

        // Copy the keys omitting any trailing punctuation so we don't need to account
        // for that on every lookup
        fKeys = (char **) malloc( sizeof(char *) * keyCount );
        Rtt_ASSERT( fKeys != NULL );

        if ( fKeys != NULL )
        {
            for (unsigned int i = 0; i < keyCount; i++)
            {
                Rtt_ASSERT( keys[i] != NULL );

                size_t keyLen = strlen( keys[i] );

                if ( ispunct( keys[i][keyLen - 1]) )
                {
                    --keyLen;
                }

                fKeys[i] = strndup( keys[i], keyLen );

                Rtt_ASSERT( fKeys[i] != NULL );
            }
        }

        fTableStart = tableStart;
		if ( hashCharCount == 0 )
			hashCharCount = 1;
		fHashCharCount = hashCharCount;

#ifdef Rtt_DEBUG
		fTryCount = 0;
		if ( lastKeyCount != fKeyCount )
		{
			Rtt_TRACE( ( "StringHash: update needed at %s line %d. Use the triplet (A,B,C) in the following line:\n", file, line ) );
		}
#endif
		
		// optimization: determine min identical prefix length
		fFirstInterestingChar = minPrefixLength( fKeys, fKeyCount );

		// optimization: single key
		if ( keyCount == 1 )
		{
			fIndices.Reserve( 1, 0 );
			fTableStart = 0;
			fHashCharCount = 1;
			return;
		}

		unsigned int maxKeyLength = (unsigned int) calculateMaxKeyLength( fKeys, fKeyCount );

		unsigned int collisions = tryTable( allocator, maxKeyLength );

		Rtt_ASSERT( collisions == 0 );
	}

    StringHash::~StringHash()
    {
        if ( fKeys != NULL )
        {
            for (unsigned int i = 0; i < fKeyCount; i++)
            {
                free( fKeys[i] );
            }
            free( fKeys );
            
            fKeys = NULL;
            fKeyCount = 0;
        }
    }

	int 
	StringHash::Lookup( const char * value ) const
	{
		Rtt_ASSERT( value );
		
		if ( value == NULL )
			return -1;

		unsigned int hash = calculateHash( value, fHashCharCount, fTableStart, fFirstInterestingChar ) % fIndices.Length();

		short index = fIndices[hash];

        if ( index >= 0 && strcmp( fKeys[ index ], value ) == 0 )
        {
			return index;
        }

		return -1;
	}
}
