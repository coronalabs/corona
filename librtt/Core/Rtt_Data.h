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

#ifndef Rtt_Data_H
#define Rtt_Data_H

#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_New.h"
#include "Core/Rtt_Types.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/**
 * A generic array of memory. T should be a simple type, e.g. char.
 */
template < typename T >
class Data
{
	Rtt_CLASS_NO_COPIES( Data<T> );

	public:
		Data<T>( Rtt_Allocator* pAllocator ) : fStorage( NULL ), fLength( 0 ), fAllocator( pAllocator ), fOwnsStorage( true )
		{
		}
		
		Data<T>( T * data, int length ) : fStorage( data ), fLength( length ), fAllocator( NULL ), fOwnsStorage( false )
		{
		}

		~Data<T>()
		{
			Release();
		}

	public:
		Rtt_FORCE_INLINE S32 Length() const { return fLength; }
		Rtt_FORCE_INLINE S32 GetLength() const { return fLength; }
		Rtt_FORCE_INLINE Rtt_Allocator* Allocator() const { return fAllocator; }

		void SetLength( size_t length );
		void Set( const T * p, size_t length );
		T * Get() const;
		Rtt_FORCE_INLINE T * GetData() const { return Get(); }

		void Release() const
		{
			if ( fOwnsStorage )
				Rtt_FREE( (void *) fStorage );
			fStorage = NULL;
			fLength = 0;
			fAllocator = NULL;
			fOwnsStorage = true;
		}
		
	protected:
		mutable T * fStorage;
		mutable S32 fLength;
		mutable Rtt_Allocator * fAllocator;
		mutable bool fOwnsStorage;

};

template < typename T >
void
Data< T >::SetLength( size_t length )
{
	if ( fLength != (S32)length )
	{
		if ( fOwnsStorage )
			Rtt_FREE( (void *) fStorage );
		else
			fOwnsStorage = true;
		
		if ( length == 0 )
		{
			fStorage = NULL;
			fLength = 0;
		}
		else
		{
			fStorage = (T *) Rtt_MALLOC( fAllocator, length * sizeof( T ) );

			Rtt_ASSERT( fStorage );
			if ( fStorage )
				fLength = (S32) length;
		}
	}
}

template < typename T >
void
Data< T >::Set( const T * p, size_t length )
{
	SetLength( length );
	memcpy( (void *) fStorage, p, length * sizeof( T ) );
}

template < typename T >
T *
Data< T >::Get() const
{
	return fStorage;
}

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_Data_H

