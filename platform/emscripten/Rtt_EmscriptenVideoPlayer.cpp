//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_EmscriptenVideoPlayer.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_LuaLibDisplay.h"
#include "Rtt_Runtime.h"

#if defined(EMSCRIPTEN)
#include "emscripten/emscripten.h"		// for native alert and etc
#endif

namespace Rtt
{

#pragma region Constructors/Destructors

	EmscriptenVideoPlayer::EmscriptenVideoPlayer(const Rtt::ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator, int w, int h)
		: PlatformVideoPlayer(handle)
		, fVideoObject(NULL)
	{
		Rect bounds;
		bounds.xMin = 0;		
		bounds.xMax = w;
		bounds.yMin = 0;
		bounds.yMax = h;
		fVideoObject = new EmscriptenVideoObject(bounds);

		lua_State *L = handle.Dereference();
		Runtime& runtime = *LuaContext::GetRuntime(L);
		Display& display = runtime.GetDisplay();
		int result = LuaLibDisplay::AssignParentAndPushResult(L, display, fVideoObject, NULL);
		lua_pop(L, result);

		fVideoObject->Initialize();
	}

	EmscriptenVideoPlayer::~EmscriptenVideoPlayer()
	{
		delete fVideoObject;
	}

#pragma endregion


#pragma region Public Member Functions
	bool EmscriptenVideoPlayer::Load(const char * path, bool isRemote)
	{
		fVideoObject->setBackgroundColor(0, 0, 0, 255);		// black
		fVideoObject->load(path);
		return true;
	}

	void EmscriptenVideoPlayer::Play()
	{
		// fixme
		// not needs ?, 	fAutoPlay = true;
	}

	void EmscriptenVideoPlayer::SetProperty(U32 mask, bool newValue)
	{
		PlatformVideoPlayer::SetProperty(mask, newValue);

		if (mask & kShowControls)
		{
			fVideoObject->showControls(newValue);
		}
	}

	void EmscriptenVideoPlayer::NotificationCallback()
	{
		DidDismiss(NULL, NULL);
	}

#pragma endregion

} // namespace Rtt
