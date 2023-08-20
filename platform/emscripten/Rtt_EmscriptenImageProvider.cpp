//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_EmscriptenImageProvider.h"
#include "Rtt_EmscriptenBitmap.h"
#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_LuaLibDisplay.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Display/Rtt_Display.h"

#if defined(EMSCRIPTEN)
#include "emscripten/emscripten.h"		// for native alert and etc

extern "C"
{
	// Java ==> Lua callback
	void EMSCRIPTEN_KEEPALIVE jsVideoRecorderCallback(Rtt::EmscriptenImageProvider* obj,	Rtt::EmscriptenImageProvider::EmscriptenVideoEventTypes eventID, int w, int h, uint8_t* buf)
	{
		obj->dispatch(eventID, w, h, buf);
	}

	extern void jsImageProviderCreate(void* thiz, int w, int h);
	extern void jsDeleteObject(int id);
	extern bool jsImageProviderShow(int id);
	extern void jsImageProviderHide(int id);
}
#else
	void jsImageProviderCreate(void* thiz, int w, int h) {} 
	void jsDeleteObject(int id) {}
	bool jsImageProviderShow(int id) { return true; }
	void jsImageProviderHide(int id) {}
#endif

namespace Rtt
{ 

#pragma region Constructors/Destructors
	/// Creates a new image provider.
	/// @param handle Reference to Lua state used to send image selection notifications to a Lua listener function.
	EmscriptenImageProvider::EmscriptenImageProvider(const ResourceHandle<lua_State> & handle, int w, int h)
		: PlatformImageProvider(handle)
		, fObjID(0)
	{
		jsImageProviderCreate(this, w, h); 
	} 

	EmscriptenImageProvider::~EmscriptenImageProvider()
	{
		jsDeleteObject(fObjID); 
	}

#pragma endregion


#pragma region Public Member Functions
	/// Determines if the given image source (Camera, Photo Library, etc.) is supported on this platform.
	/// @param source Unique integer ID of the image source. IDs are defined in PlatformImageProvider class.
	/// @return Returns true if given image source is supported on this platform. Returns false if not.
	bool EmscriptenImageProvider::Supports(int source) const
	{
		// no ways to get webcam list synchronously, so just say 'true'
		// �� webcam exists will be checked later in media.capturePhoto()
		return true;
	}

	/// Displays a window for selecting an image.
	/// @param source Unique integer ID indicating what kind of window to display such as the Camera or Photo Library.
	/// @param filePath The path\file name to save the selected image file to.
	///                 Set to NULL to not save to file and display the selected photo as a display object instead.
	/// @return Returns true if the window was shown.
	///         Returns false if given source is not provided or if an image provider window is currently shown.
	// PlatformImageProvider::kCamera = 1
	bool EmscriptenImageProvider::Show(int source, const char* filePath, lua_State* L)
	{
		// Do not continue if:
		// 1) Given image source type is not supported on this platform.
		// 2) Image providing window is already shown.

		PlatformImageProvider::Source src = (PlatformImageProvider::Source) source;

		// If given file path is an empty string, then change it to NULL.
		if (filePath && (strlen(filePath) <= 0))
		{
			filePath = NULL;
		}

		if (fObjID > 0)		// streamer created ?
		{
			return jsImageProviderShow(fObjID); 
		}
		return false;
	}

	void EmscriptenImageProvider::dispatch(EmscriptenVideoEventTypes eventID, int w, int h, uint8_t* buf)
	{
		//printf("event: id=%d, w=%d, h=%d, buf=%p,fObjID=%d\n", eventID, w, h, buf, fObjID);

		if (eventID == onCreated)
		{
			fObjID = w;
			return;
		}

		lua_State *L = GetLuaState();
		EmscriptenImageProviderCompletionEvent e(w, h, buf);
		Runtime* runtime = LuaContext::GetRuntime(L);
		e.Dispatch(L, *runtime);

		if (fObjID > 0)
		{
			jsImageProviderHide(fObjID);
		}
	}

	int EmscriptenImageProvider::EmscriptenImageProviderCompletionEvent::Push(lua_State *L) const
	{
		if (Rtt_VERIFY(Super::Push(L)) && fData && fWidth > 0 && fHeight > 0)
		{
			lua_pushboolean(L, true);
			lua_setfield(L, -2, "completed");

			Runtime* runtime = LuaContext::GetRuntime(L);
			EmscriptenBaseBitmap* bitmap = new EmscriptenBaseBitmap(runtime->GetAllocator(), fWidth, fHeight, fData);
			BitmapPaint* paint = BitmapPaint::NewBitmap(runtime->GetDisplay().GetTextureFactory(), bitmap, false);
			LuaLibDisplay::PushImage(L, NULL, paint, runtime->GetDisplay(), NULL);
			lua_setfield(L, -2, "target");
		}
		return 1;
	}


#pragma endregion

} // namespace Rtt
