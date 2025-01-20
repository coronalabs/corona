//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_Array.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ArrayStorage::ArrayStorage( Rtt_Allocator* pAllocator )
:	fStorage( NULL ),
	fLength( 0 ),
	fLengthMax( 0 ),
	fAllocator( pAllocator )
{
}

ArrayStorage::~ArrayStorage()
{
	Rtt_FREE( fStorage );
}

size_t
ArrayStorage::Copy( const ArrayStorage& rhs, size_t elementSize )
{
	Rtt_Allocator* pAllocator = fAllocator; Rtt_ASSERT( pAllocator );
	const S32 length = rhs.fLength;
	size_t numBytes = length * elementSize;
	void* dstStorage = Rtt_MALLOC( pAllocator, numBytes );

	if ( dstStorage )
	{
		Rtt_ASSERT( fStorage == NULL );
		fStorage = dstStorage;
		fLength = length;
		fLengthMax = length;
		fAllocator = pAllocator;
	}
	else
	{
		numBytes = 0;
	}

	return numBytes;
}

void 
ArrayStorage::Preallocate( U32 length, size_t elementSize )
{
	Rtt_ASSERT( fStorage == NULL );
	
	void * dstStorage = Rtt_MALLOC( fAllocator, length * elementSize );

	fStorage = dstStorage;
	fLength = 0;
	fLengthMax = length;
}

void*
ArrayStorage::Expand( size_t elementSize, S32 minLength )
{
	Rtt_ASSERT( fLengthMax >= fLength );
	
	void* result = fStorage;

	S32 newLengthMax = fLengthMax;
	if ( newLengthMax < minLength )
	{
		newLengthMax = minLength;
	}

	const S32 kMinThreshold = 4;
	const S32 kMaxThreshold = 64;
	newLengthMax = Max( newLengthMax, kMinThreshold );
	newLengthMax += ( newLengthMax < kMaxThreshold ? newLengthMax : kMaxThreshold );

	fStorage = Rtt_MALLOC( fAllocator, newLengthMax * elementSize );
	fLengthMax = newLengthMax;

	Rtt_ASSERT( fStorage );

	return result;
}

void
ArrayStorage::ShiftMem( S32 index, S32 shiftAmount, size_t elementSize )
{
	Rtt_ASSERT( (Length() + shiftAmount) <= LengthMax() );
	Rtt_ASSERT( (index + shiftAmount) >= 0 );

	S32 newIndex = index + shiftAmount;
	void* dst = ((U8*)Storage()) + (elementSize*newIndex);
	void* src = ((U8*)Storage()) + (elementSize*index);
	memmove( dst, src, elementSize * ( Length() - index ) );
}

// ----------------------------------------------------------------------------

ArrayBuffer::ArrayBuffer( Rtt_Allocator *pAllocator )
:	Super( pAllocator )
{
}

bool
ArrayBuffer::Grow( size_t elementSize, S32 newLength )
{
	// Only grow when buffer is no longer large enough
	bool result = ( newLength > LengthMax() );

	S32 oldLength = Length();	
	if ( result )
	{
		void *oldStorage = Expand( elementSize, newLength );
		size_t numSrcBytes = oldLength*elementSize;

		Rtt_ASSERT( newLength <= LengthMax() ); // TODO: Ensure Expand() made a buffer large enough

		// Copy over src bytes
		void *newStorage = fStorage;
		memcpy( newStorage, oldStorage, numSrcBytes );
		Rtt_FREE( oldStorage );

		// Zero out the portion of the new buffer that's raw storage
		unsigned char *p = (unsigned char *)fStorage;
		memset( p + numSrcBytes, 0, ( newLength - oldLength )*elementSize );
	}

	if ( newLength > oldLength )
	{
		fLength = newLength;
	}

	return result;
}

// ----------------------------------------------------------------------------

} // Rtt

