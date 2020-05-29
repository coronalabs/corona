//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformAudioPlayer_H__
#define _Rtt_PlatformAudioPlayer_H__

#include "Core/Rtt_ResourceHandle.h"
#include "Rtt_Scheduler.h"
#include "Rtt_PlatformNotifier.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

// TODO: Rename PlatformAudioPlayer

class PlatformAudioPlayer : public PlatformNotifier
{
	public:
		PlatformAudioPlayer( const ResourceHandle<lua_State> & handle );
		virtual ~PlatformAudioPlayer();

	public:
		virtual bool Load( const char* filePath ) = 0;

	public:
		virtual void Play() = 0;
		virtual void Stop() = 0;
		virtual void Pause() = 0;
		virtual void Resume() = 0;
		virtual void SetVolume( Rtt_Real volume ) = 0;
		virtual Rtt_Real GetVolume() const = 0;
	
		virtual void NotificationCallback();

	public:
		void SetLooping( bool l )
		{
			fLooping = l;
		}
	
		bool GetLooping() const
		{
			return fLooping;
		}
	
	protected:
		Rtt_Real fVolume;
	
	protected:
		friend class PlatformAudioPlayerCallListenerTask;

		bool fLooping;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformAudioPlayer_H__
