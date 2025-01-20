//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformEventSound_H__
#define _Rtt_PlatformEventSound_H__

#include "Rtt_PlatformAudioPlayer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class CompletionEvent;

// ----------------------------------------------------------------------------

class PlatformEventSound : public PlatformAudioPlayer
{
	public:
		typedef PlatformEventSound Self;
		typedef PlatformAudioPlayer Super;

	public:
		// WARNING: CompletionEvent owns the sound
		static CompletionEvent* CreateCompletionEvent( Rtt_Allocator *a, PlatformEventSound *sound );
		
	public:
		PlatformEventSound( const ResourceHandle<lua_State> & handle );
		virtual ~PlatformEventSound();
		
		virtual void ReleaseOnComplete() = 0;

		// TODO: Rename
		bool WantsReleaseOnComplete() const { return fReleaseOnComplete; }
		void SetWantsReleaseOnComplete( bool newValue ) { fReleaseOnComplete = newValue; }

	private:
		bool fReleaseOnComplete;
};

// ----------------------------------------------------------------------------
	
} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformEventSound_H__
