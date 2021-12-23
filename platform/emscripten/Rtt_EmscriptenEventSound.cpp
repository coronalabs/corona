//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_EmscriptenEventSound.h"
#include "Rtt_LuaContext.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#pragma region Constructors/Destructors
EmscriptenEventSound::EmscriptenEventSound(const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator)
:	PlatformEventSound(handle)
{
}

EmscriptenEventSound::~EmscriptenEventSound()
{
}

#pragma endregion


#pragma region Public Member Functions
bool EmscriptenEventSound::Load(const char * filePath)
{
	return false;
}

void EmscriptenEventSound::Play()
{
}

void EmscriptenEventSound::ReleaseOnComplete()
{
}

void EmscriptenEventSound::Stop()
{
}

void EmscriptenEventSound::Pause()
{
}

void EmscriptenEventSound::Resume()
{
}

void EmscriptenEventSound::SetVolume(Rtt_Real volume)
{
}

Rtt_Real EmscriptenEventSound::GetVolume() const
{
	return 1.0f;
}

#pragma endregion

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
