//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_Time_H__
#define __Rtt_Time_H__

// ----------------------------------------------------------------------------

#include "Rtt_Macros.h"

#if defined( Rtt_APPLE_ENV ) || defined( Rtt_ANDROID_ENV ) || defined( Rtt_WEBOS_ENV ) || defined( Rtt_EMSCRIPTEN_ENV ) || defined( Rtt_WIN_ENV ) || defined( Rtt_POWERVR_ENV ) || defined( Rtt_NINTENDO_ENV ) || defined( Rtt_LINUX_ENV )
	#include <stdint.h>
	typedef uint64_t Rtt_AbsoluteTime;
#elif defined( Rtt_SYMBIAN_ENV )
	Rtt_STATIC_ASSERT( false );
#else
	Rtt_STATIC_ASSERT( false );
#endif

Rtt_EXPORT Rtt_AbsoluteTime Rtt_GetAbsoluteTime(void);
Rtt_EXPORT uint64_t Rtt_AbsoluteToMilliseconds( Rtt_AbsoluteTime absoluteTime );
Rtt_EXPORT uint64_t Rtt_AbsoluteToMicroseconds( Rtt_AbsoluteTime absoluteTime );

// ----------------------------------------------------------------------------

#endif // __Rtt_Time_H__
