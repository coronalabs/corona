//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_PlatformAudioPlayer.h"
#include "Rtt_WinEventSound.h"
#include "WinSound.h"


#pragma region Forward Declarations
namespace Interop
{
	class RuntimeEnvironment;
}
namespace Rtt
{
	class BaseCallback;
}

#pragma endregion


namespace Rtt
{

class WinAudioPlayer : public PlatformAudioPlayer
{
	public:
		WinAudioPlayer(Interop::RuntimeEnvironment& environment, const ResourceHandle<lua_State> & handle);
		virtual ~WinAudioPlayer();

		void SetCompletionHandler(Rtt::BaseCallback* callbackPointer) { fCompletionCallbackPointer = callbackPointer; }
		virtual bool Load( const char* filePath );
		virtual void Play();
		virtual void Stop();
		virtual void Pause();
		virtual void Resume();
		virtual void SetVolume( Rtt_Real volume );
		virtual Rtt_Real GetVolume() const;
		virtual void NotificationCallback();

	private:
		WinSound fSound;
		Rtt::BaseCallback* fCompletionCallbackPointer;
};

}	// namespace Rtt
