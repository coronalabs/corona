//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_PlatformAudioPlayer.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

PlatformAudioPlayer::PlatformAudioPlayer( const ResourceHandle<lua_State> & handle )
:	PlatformNotifier( handle ),
	fVolume( 1.0f ),
	fLooping( false )
{
}

PlatformAudioPlayer::~PlatformAudioPlayer()
{
}

void 
PlatformAudioPlayer::NotificationCallback()
{
	lua_State * L = GetLuaState();
	Rtt_ASSERT( L );
	if ( L )
	{
		CompletionEvent *e = Rtt_NEW( LuaContext::GetRuntime( L )->GetAllocator(), CompletionEvent );
		ScheduleDispatch( e );
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

