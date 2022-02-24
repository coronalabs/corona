//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Assert.h"
#include "Core/Rtt_Build.h"
#include "Core/Rtt_Data.h"
#include "Rtt_EmscriptenAudioRecorder.h"
#include "Rtt_LuaContext.h"

#if defined(EMSCRIPTEN)
#include "emscripten/emscripten.h"		// for native alert and etc
#endif

#if defined(EMSCRIPTEN)
extern "C"
{
	extern void jsAudioRecorderInit(void* thiz, const char* file);
	extern void jsDeleteObject(int id);
	extern void jsAudioRecorderStart(int id, int rate);
	extern void jsAudioRecorderStop(int id);

	// Java ==> Lua callback
	void EMSCRIPTEN_KEEPALIVE jsAudioRecorderCallback(Rtt::EmscriptenAudioRecorder* obj,
		Rtt::EmscriptenAudioRecorder::EmscriptenAudioRecorderEvent eventID, int bufsize, uint8_t* buf)
	{
		obj->dispatch(eventID, bufsize, buf);
	}
}
#else
	void jsAudioRecorderInit(void* thiz, const char* file) {}
	extern void jsDeleteObject(int id);
	void jsAudioRecorderStart(int id, int rate) {}
	void jsAudioRecorderStop(int id) {}
#endif

namespace Rtt
{

#pragma region Constructors/Destructors
	EmscriptenAudioRecorder::EmscriptenAudioRecorder(
		const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator, const char * file)
		: PlatformAudioRecorder(handle, allocator, file)
		, fFile(file)
		, fObjID(0)
	{
		jsAudioRecorderInit(this, file);
	}

	EmscriptenAudioRecorder::~EmscriptenAudioRecorder()
	{
		jsDeleteObject(fObjID);
	}

#pragma endregion


#pragma region Public Member Functions
	void EmscriptenAudioRecorder::Start()
	{
		U32 rate = GetSampleRate();
		jsAudioRecorderStart(fObjID, rate);
	}

	void EmscriptenAudioRecorder::Stop()
	{
		jsAudioRecorderStop(fObjID);
	}

	void EmscriptenAudioRecorder::NotificationCallback(int bytesRead)
	{
	}

	void EmscriptenAudioRecorder::dispatch(EmscriptenAudioRecorderEvent eventID, int bufsize, uint8_t* buf)
	{
		//printf("event: id=%d, bufsize=%d, buf=%p\n", eventID, bufsize, buf);
		switch (eventID)
		{
		case onError:
			fIsRunning = false;
			break;
		case onStart:
			fIsRunning = true;
			break;
		case onStop:
			fIsRunning = false;
			break;
		case onResume:
			fIsRunning = true;
			break;
		case onPause:
			fIsRunning = false;
			break;
		case onDataAvailable:
			break;
		case onCreated:
			fObjID = bufsize;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
		}
	}

#pragma endregion

} // namespace Rtt

