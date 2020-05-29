//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_ArrayTuple.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

size_t
ArrayTuple::GetTypeSize( Type t )
{
	size_t result = 0;

	switch ( t )
	{
		case kUnsignedByte:
		{
			result = sizeof( char );
		}
		break;

		case kFixed:
		{
			result = sizeof( int );
		}
		break;

		case kFloat:
		{
			result = sizeof( float );
		}
		break;

		default:
		{
			Rtt_ASSERT_NOT_REACHED();
		}
		break;
	}

	return result;
}

ArrayTuple::ArrayTuple( Rtt_Allocator* pAllocator, Type t )
:	Super( pAllocator ),
	fTupleSize( 0 ),
	fType( t )
{
}

void
ArrayTuple::Initialize( size_t numTypesPerTuple )
{
	if ( Rtt_VERIFY( 0 == fTupleSize ) )
	{
		fTupleSize = numTypesPerTuple * GetTypeSize();
	}
}

bool
ArrayTuple::EnsureCapacity( int minLength )
{
	return Grow( fTupleSize, minLength );
}

void
ArrayTuple::SetLength( S32 newLength )
{
	fLength = Min( newLength, fLengthMax );
}

// Sets multiple
void
ArrayTuple::Set( int index, const void *src, int numSrcTuples )
{
	Rtt_ASSERT( numSrcTuples > 0 );

	Grow( fTupleSize, index + numSrcTuples );
	void *p = WriteAccess();

	const void *pSrc = src;
	size_t numBytes = numSrcTuples * fTupleSize;

	size_t byteOffset = index * fTupleSize;
	void *pDst = ((U8*)p) + byteOffset;

	if ( pDst )
	{
		memcpy( pDst, pSrc, numBytes );
	}
}

// Sets single
void
ArrayTuple::Set( int index, const void *src )
{
	Grow( fTupleSize, index + 1 );
	void *p = WriteAccess();

	const void *pSrc = src;
	size_t numBytes = fTupleSize;

	size_t byteOffset = index * fTupleSize;
	void *pDst = ((U8*)p) + byteOffset;

	if ( pDst )
	{
		memcpy( pDst, pSrc, numBytes );
	}
}

void
ArrayTuple::Append( const void *src, int numSrcTuples )
{
	Set( Length(), src, numSrcTuples );
}

void
ArrayTuple::Append( const void *srcTuple )
{
	Set( Length(), srcTuple );
}

void
ArrayTuple::DuplicateLast()
{
	int length = Length();
	if ( Rtt_VERIFY( length > 0 ) )
	{
		Grow( fTupleSize, length + 1 );

		const void *src = (U8*)ReadAccess() + (length-1)*fTupleSize;
		void *dst = (U8*)WriteAccess() + length*fTupleSize;
		memcpy( dst, src, fTupleSize );
	}
}

void
ArrayTuple::Print( int numTabIndents ) const
{
	Type t = GetType();

	// Get base address of tuple at 'index'
	const U8 *p = (const U8*)ReadAccess();

	int numComponentsPerTuple = GetNumComponentsPerTuple();

	for ( int i = 0, numTuples = Length(); i < numTuples; i++ )
	{
		const void *ptr = p + i*GetTupleSize();

		for ( int k = 0; k < numTabIndents; k++ )
		{
			Rtt_TRACE( ( "\t" ) );
		}
		Rtt_TRACE( ( "[%p]( ", ptr ) );
		for ( int j = 0; j < numComponentsPerTuple; j++ )
		{
			switch ( t )
			{
				case kUnsignedByte:
					Rtt_TRACE( ( "%d ", *((U8*)ptr + j) ) );
					break;
				case kFloat:
					Rtt_TRACE( ( "%g ", *((float*)ptr + j) ) );
					break;
				default:
					Rtt_ASSERT_NOT_IMPLEMENTED();
					break;
			}
		}
		Rtt_TRACE( ( ")\n" ) );
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

