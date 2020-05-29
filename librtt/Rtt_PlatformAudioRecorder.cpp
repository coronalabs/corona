//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_PlatformAudioRecorder.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"
#include "Core/Rtt_Array.h"

#include <math.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

PlatformAudioRecorder::PlatformAudioRecorder( 
	const ResourceHandle<lua_State>& handle,
	Rtt_Allocator & allocator,
	const char * file )
:	PlatformNotifier( handle ), 
	fFile( & allocator ), 
	fAllocator( allocator ),
	fIsRunning( false ),
	fSampleRate( 44100 )
{
	if ( file )
	{
		fFile.Set( file );
	}
}

PlatformAudioRecorder::~PlatformAudioRecorder()
{
}

void
PlatformAudioRecorder::NotificationCallback( int status )
{
	// This is only used on Android
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

