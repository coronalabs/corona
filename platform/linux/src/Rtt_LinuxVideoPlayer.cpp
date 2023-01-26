//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_LinuxVideoPlayer.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_LuaLibDisplay.h"
#include "Rtt_Runtime.h"

#if (wxUSE_MEDIACTRL == 1)

namespace Rtt
{
	LinuxVideoPlayer::LinuxVideoPlayer(const Rtt::ResourceHandle<lua_State> &handle, Rtt_Allocator &allocator, int w, int h)
		: PlatformVideoPlayer(handle)
		, fVideoObject(NULL)
	{
		Rect bounds;
		bounds.xMin = 0;
		bounds.xMax = w;
		bounds.yMin = 0;
		bounds.yMax = h;
		fVideoObject = new LinuxVideoObject(bounds);

		lua_State *L = handle.Dereference();
		Runtime &runtime = *LuaContext::GetRuntime(L);
		Display &display = runtime.GetDisplay();
		int result = LuaLibDisplay::AssignParentAndPushResult(L, display, fVideoObject, NULL);
		lua_pop(L, result);

		fVideoObject->Initialize();
	}

	LinuxVideoPlayer::~LinuxVideoPlayer()
	{
		// fixme add smart-ptr delete fVideoObject;
	}

	bool LinuxVideoPlayer::Load(const char *path, bool isRemote)
	{
		fVideoObject->setBackgroundColor(0, 0, 0, 255); // black
		fVideoObject->load(path, isRemote);
		return true;
	}

	void LinuxVideoPlayer::Play()
	{
		// fixme
		// not needs ?,fAutoPlay = true;
	}

	void LinuxVideoPlayer::SetProperty(U32 mask, bool newValue)
	{
		PlatformVideoPlayer::SetProperty(mask, newValue);

		if (mask & kShowControls)
		{
			fVideoObject->showControls(newValue);
		}
	}

	void LinuxVideoPlayer::NotificationCallback()
	{
		DidDismiss(NULL, NULL);
	}
}; // namespace Rtt

#endif
