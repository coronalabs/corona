//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_StringHash_H__
#define _Rtt_StringHash_H__

#include "Core/Rtt_Allocator.h"

// ----------------------------------------------------------------------------

namespace Rtt
{
	template < typename T >
	class IndexArray
	{
		public:
			typedef T Index;

			unsigned int fLength;
			Index * fElements;
			Rtt_Allocator & fAllocator;

			Rtt_CLASS_NO_COPIES( IndexArray )

		public:
			IndexArray( Rtt_Allocator & a ) : fAllocator( a ), fElements( NULL ), fLength( 0 )
			{
			}
			~IndexArray()
			{
				Rtt_DELETE( fElements );
			}

			unsigned int Length() const
			{
				return fLength;
			}

			void Reserve( unsigned int length, Index initValue )
			{
				Rtt_ASSERT( fElements == NULL );
				fElements = (Index *) Rtt_MALLOC( fAllocator, length * sizeof( Index ) );

				Rtt_ASSERT( fElements );

				if ( !fElements )
					return;

				fLength = length;

				for ( unsigned int i = 0; i < length; i++ )
					fElements[ i ] = initValue;
			}

			const T& operator[]( unsigned int index ) const
			{
				Rtt_ASSERT( VerifyIndex( index ) );
				return fElements[index];
			}
			Rtt_FORCE_INLINE T& operator[]( unsigned int index )
			{
				Rtt_ASSERT( VerifyIndex( index ) );
				return fElements[index];
			}

			Rtt_FORCE_INLINE bool VerifyIndex( unsigned int index ) const { return index < Length(); }
	};


	class StringHash
	{
	public:
		StringHash( Rtt_Allocator & allocator, const char ** keys, unsigned int keyCount,
			unsigned int lastKeyCount, unsigned int tableStart, unsigned int hashCharCount,
		    const char * file, unsigned int line );
        ~StringHash();
        
		int Lookup( const char * key ) const;

        int GetKeys(const char **&keys) { keys = fOriginalKeys; return fKeyCount; }

	private:
		unsigned int	fTableStart;
		unsigned int	fHashCharCount;
		unsigned int	fFirstInterestingChar;

		unsigned int	fKeyCount;
        const char **   fOriginalKeys;
        char **         fKeys;

		IndexArray<unsigned short>	fIndices;

#ifdef Rtt_DEBUG
		unsigned int	fTryCount;
#endif
		
		unsigned int	tryTableSizeAndCharCount( IndexArray<unsigned int> &hashes, unsigned int tableSize, unsigned int hashCharCount, unsigned int tableStart );

		unsigned int	tryTable( Rtt_Allocator & allocator, unsigned int maxKeyLength );
	};

}

#endif // _Rtt_StringHash_H__