//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Core/Rtt_String.h"
#	include "Rtt_PlatformEventSound.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

class WinRTEventSound : public PlatformEventSound
{
	public:
		WinRTEventSound(const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator);
		virtual ~WinRTEventSound();

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
