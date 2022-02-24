//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_PlatformAudioRecorder.h"
#include "Core/Rtt_Types.h"

namespace Rtt
{

	class EmscriptenAudioRecorder : public PlatformAudioRecorder
	{
	public:
		typedef EmscriptenAudioRecorder Self;


		enum EmscriptenAudioRecorderEvent
		{
			undefined = 0,
			onError = 1,
			onStart = 2,
			onStop = 3,
			onResume = 4,
			onDataAvailable = 5,
			onPause = 6,
			onCreated = 7,
		};


		EmscriptenAudioRecorder(const ResourceHandle<lua_State> &handle, Rtt_Allocator &allocator, const char *file);
		virtual ~EmscriptenAudioRecorder();

		virtual void Start();
		virtual void Stop();
		virtual void NotificationCallback(int status);

		void dispatch(EmscriptenAudioRecorderEvent eventID, int bufsize, uint8_t* buf);

		String fFile;
		int fObjID;		// unique JS obj ID
	};

	// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
