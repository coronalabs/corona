//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_ResourceHandle.h"

#include "Core/Rtt_RefCount.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

BaseResourceHandle::BaseResourceHandle()
:	fCount( NULL )
{
}

BaseResourceHandle::BaseResourceHandle( const Self& rhs )
{
	Retain( rhs.fCount );
}
		   
BaseResourceHandle::~BaseResourceHandle()
{
	Release();
}

BaseResourceHandle&
BaseResourceHandle::operator=( const Self& rhs )
{
	if ( & rhs != this )
	{
		Release();
		Retain( rhs.fCount );
	}

	return * this;
}

void
BaseResourceHandle::Initialize( Rtt_Allocator* pAllocator )
{
	Rtt_ASSERT( pAllocator );

	Retain( Rtt_AllocatorAllocRefCount( pAllocator ) );
}

// Bottom (least significant) 8 bits are reserved. Top 24 bits used for ref count.
static const int kNumReservedBits = 8;
static const RefCount kIncrement = 1 << kNumReservedBits;

bool
BaseResourceHandle::IsValid() const
{
	Rtt_ASSERT( fCount );

	// Valid only if bottom 8 bits must be zero
	return (fCount && (0 == ((*fCount) & 0xFF)));
}

void
BaseResourceHandle::Invalidate()
{
	// Ensure fCount is valid and > 0
	Rtt_ASSERT( IsValid() && (*fCount >> kNumReservedBits) > 0 );

	// Turn on bottom 8 bits 
	*fCount = (*fCount) | 0xFF;
}

void
BaseResourceHandle::Retain( RefCount *pCount )
{
	fCount = pCount;
	if ( pCount )
	{
		// ++(*pCount);
		*pCount = *pCount + kIncrement;
	}
}

void
BaseResourceHandle::Release()
{
	RefCount *pCount = fCount;
	if ( pCount )
	{
		// --(*pCount);
		RefCount oldCount = *pCount;
		RefCount c = (oldCount >> kNumReservedBits) - 1;
		if ( 0 >= c )
		{
			Rtt_AllocatorFreeRefCount( pCount );
		}
		else
		{
			*pCount = oldCount - kIncrement;
		}
	}
}

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

