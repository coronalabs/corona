//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_PlatformVideoPlayer.h"
#include "Core/Rtt_String.h"
#include "Rtt_EmscriptenVideoObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class EmscriptenVideoPlayer : public PlatformVideoPlayer
{
	public:
		EmscriptenVideoPlayer(const Rtt::ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator, int w, int h);
		virtual ~EmscriptenVideoPlayer();

		virtual bool Load(const char* path, bool remote);
		virtual void NotificationCallback();
		virtual void Play();
		virtual void SetProperty(U32 mask, bool newValue);

		EmscriptenVideoObject* fVideoObject;

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
