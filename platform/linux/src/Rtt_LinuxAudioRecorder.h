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
#include <al.h>
#include <alc.h>
#include <cmath>
#include <fstream>
#include <iostream>

namespace Rtt
{
	class LinuxAudioRecorder : public PlatformAudioRecorder
	{
	public:
		class AudioTask : public Task
		{
		public:
			AudioTask(LinuxAudioRecorder *thiz)
				: Task(true), fLinuxAudioRecorder(thiz)
			{
			}

			virtual void operator()(Scheduler &sender)
			{
				fLinuxAudioRecorder->onEnterFrame();
			}

			LinuxAudioRecorder *fLinuxAudioRecorder;
		};

		typedef LinuxAudioRecorder Self;
		enum LinuxAudioRecorderEvent
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

		LinuxAudioRecorder(const ResourceHandle<lua_State> &handle, Rtt_Allocator &allocator, const char *file);
		virtual ~LinuxAudioRecorder();
		virtual void Start();
		virtual void Stop();
		void onEnterFrame();
		AudioTask *fAudioTask;

	private:
		String fFile;
		ALCdevice *fDevice;
		std::ofstream f;
		//FILE* fOut;
		size_t fWavDataChunkPos;
		int fChannels;
	};
} // namespace Rtt
