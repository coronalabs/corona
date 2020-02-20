//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AppleEventSound_H__
#define _Rtt_AppleEventSound_H__

#include "Rtt_PlatformEventSound.h"

#include <AudioToolbox/AudioServices.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AppleEventSound : public PlatformEventSound
{
	public:
		typedef PlatformEventSound Super;

	public:
		AppleEventSound( const ResourceHandle<lua_State> & handle );
		virtual ~AppleEventSound();
		
	public:
		virtual bool Load( const char* filePath );
		
	public:
		virtual void Play();
		virtual void Stop();
		virtual void Pause();
		virtual void Resume();
		virtual void SetVolume( Rtt_Real volume );
		virtual Rtt_Real GetVolume() const;
		
		virtual void ReleaseOnComplete();
		
	private:
		SystemSoundID fSoundID;
		bool fPlaying;
		
		static void OnCompleteEventSound( SystemSoundID soundID, void * userData );
};

// ----------------------------------------------------------------------------

}  // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AppleEventSound_H__