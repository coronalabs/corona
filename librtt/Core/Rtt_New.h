//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_New_H__
#define _Rtt_New_H__

#include "Core/Rtt_Macros.h"
#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

// Placement new

#ifdef __cplusplus

	#ifdef Rtt_PLACEMENT_NEW
		// Some platforms don't define placement new
		// Or we want to define a version with the no-throw specification:
		inline void* operator new( size_t, void* p ) throw() { return p; }
		inline void* operator new[]( size_t, void* p ) throw() { return p; }
	#else
		#include <new>
	#endif

#endif

// ----------------------------------------------------------------------------

#endif // _Rtt_New_H__
