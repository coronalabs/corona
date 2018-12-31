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