//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_AutoResource.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

namespace Private
{

#if defined( Rtt_MINIMIZE_CODE_SIZE )

bool
RefCountRelease( RefCount* pRefCount )
{
	bool shouldCollect = false;

	if ( pRefCount )
	{
		--(*pRefCount);
		
		shouldCollect = ( 0 == *pRefCount );

		if ( shouldCollect )
		{
			Rtt_AllocatorFreeRefCount( pRefCount );
		}
	}

	return shouldCollect;
};

#endif // Rtt_MINIMIZE_CODE_SIZE

} // Private

#ifdef Rtt_DEBUG
RefCount* FreedRefCount()
{
	static RefCount sCount = -1;
	Rtt_ASSERT( sCount < 0 );
	return & sCount;
}
#endif

// ----------------------------------------------------------------------------

} // Rtt

