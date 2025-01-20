//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Rtt_WinRTEventSound.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Rtt_LuaContext.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Constructors/Destructors
WinRTEventSound::WinRTEventSound(const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator)
:	PlatformEventSound(handle)
{
}

WinRTEventSound::~WinRTEventSound()
{
}

#pragma endregion


#pragma region Public Member Functions
bool WinRTEventSound::Load(const char * filePath)
{
	return false;
}

void WinRTEventSound::Play()
{
}

void WinRTEventSound::ReleaseOnComplete()
{
}

void WinRTEventSound::Stop()
{
}

void WinRTEventSound::Pause()
{
}

void WinRTEventSound::Resume()
{
}

void WinRTEventSound::SetVolume(Rtt_Real volume)
{
}

Rtt_Real WinRTEventSound::GetVolume() const
{
	return 1.0f;
}

#pragma endregion

} // namespace Rtt
