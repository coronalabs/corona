//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_RefCount.h"

// ----------------------------------------------------------------------------

Rtt::RefCount*
Rtt_AllocatorAllocRefCount( Rtt_Allocator* pAllocator )
{
	Rtt::RefCount *result = (Rtt::RefCount*)Rtt_MALLOC( pAllocator, sizeof( Rtt::RefCount ) );
	*result = 0;
	return result;
}

void
Rtt_AllocatorFreeRefCount( Rtt::RefCount* p )
{
	Rtt_FREE( p );
}

// ----------------------------------------------------------------------------


