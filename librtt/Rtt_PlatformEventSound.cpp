//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_PlatformEventSound.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class EventSoundCompletionEvent : public CompletionEvent
{
	public:
		EventSoundCompletionEvent( PlatformEventSound *sound );
		~EventSoundCompletionEvent();

	private:
		PlatformEventSound *fSound;
};

EventSoundCompletionEvent::EventSoundCompletionEvent( PlatformEventSound *sound )
:	fSound( sound )
{
}

EventSoundCompletionEvent::~EventSoundCompletionEvent()
{
	if ( fSound->WantsReleaseOnComplete() )
	{
		Rtt_DELETE( fSound );
	}
}

// ----------------------------------------------------------------------------

CompletionEvent*
PlatformEventSound::CreateCompletionEvent( Rtt_Allocator *a, PlatformEventSound *sound )
{
	CompletionEvent *result = NULL;
	
	if ( a )
	{
		result = Rtt_NEW( a, EventSoundCompletionEvent( sound ) );
	}
	
	return result;
}

PlatformEventSound::PlatformEventSound( const ResourceHandle<lua_State> & handle )
:	Super( handle ),
	fReleaseOnComplete( false )
{
}

PlatformEventSound::~PlatformEventSound()
{
}

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------
