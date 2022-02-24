//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_PlatformEventSound.h"
#include "Core/Rtt_String.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class EmscriptenEventSound : public PlatformEventSound
{
	public:
		EmscriptenEventSound(const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator);
		virtual ~EmscriptenEventSound();

		virtual bool Load(const char* filePath);
		virtual void Play();
		virtual void Stop();
		virtual void Pause();
		virtual void Resume();
		virtual void SetVolume(Rtt_Real volume);
		virtual Rtt_Real GetVolume() const;
		virtual void ReleaseOnComplete();
};

}  // namespace Rtt
