//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Build.h"
#include "Rtt_Callback.h"
#include "Rtt_PlatformEventSound.h"


#pragma region Forward Declarations
namespace Interop
{
	class RuntimeEnvironment;
}
namespace Rtt
{
	class WinAudioPlayer;
}

#pragma endregion


namespace Rtt
{

class WinEventSound : public PlatformEventSound
{
	public:
		typedef PlatformEventSound Super;

		WinEventSound(Interop::RuntimeEnvironment& environment, const ResourceHandle<lua_State> & handle);
		virtual ~WinEventSound();

		virtual bool Load(const char* filePath);
		virtual void Play();
		virtual void Stop();
		virtual void Pause();
		virtual void Resume();
		virtual void SetVolume(Rtt_Real volume);
		virtual Rtt_Real GetVolume() const;
		virtual void ReleaseOnComplete();

	private:
		void OnSoundCompleted();

		Rtt::MethodCallback<WinEventSound> fCompletionCallback;
		WinAudioPlayer *fSound;
		bool fPlaying;
};

}  // namespace Rtt
