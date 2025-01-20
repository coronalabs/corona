//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Solar2D game engine.
// With contributions from Dianchu Technology
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Rtt_Data_H
#define Rtt_Data_H

#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_New.h"
#include "Core/Rtt_Types.h"
#include "Core/Rtt_Assert.h"

#include <string.h>

#if defined(Rtt_NXS_ENV)
#define Rtt_FREE( p )						free( (p) )
#define Rtt_MALLOC( null, size )	malloc( (size) )
#endif

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

		// We own this anyway
		fOwnsStorage = true;
		fStorage = NULL;
		fLength = 0;
		
		if ( length != 0 )
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
	if ( fStorage )
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

