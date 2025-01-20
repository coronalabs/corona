//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Rtt_RefCount_H
#define Rtt_RefCount_H

#include "Core/Rtt_Types.h"
#include "Core/Rtt_Allocator.h"

// ----------------------------------------------------------------------------

Rtt_EXPORT Rtt::RefCount*
Rtt_AllocatorAllocRefCount( Rtt_Allocator* pAllocator );

Rtt_EXPORT void
Rtt_AllocatorFreeRefCount( Rtt::RefCount* p );

// ----------------------------------------------------------------------------

#endif // Rtt_RefCount_H

