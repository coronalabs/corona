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
#include "Rtt_LinuxAudioRecorder.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"
#include "Rtt_LinuxPlatform.h"
#include "Rtt_LinuxContext.h"

namespace Rtt
{
	LinuxAudioRecorder::LinuxAudioRecorder(const ResourceHandle<lua_State> &handle, Rtt_Allocator &allocator, const char * file)
		: PlatformAudioRecorder(handle, allocator, file)
		, fFile(file)
		, fDevice(NULL)
		, fAudioTask(NULL)
		  //	, fOut(NULL)
		, fWavDataChunkPos(0)
		, fChannels(2)		// stereo
	{
	}

	LinuxAudioRecorder::~LinuxAudioRecorder()
	{
		Stop();
	}

	template <typename Word>
	std::ostream& write_word( std::ostream &outs, Word value, unsigned size = sizeof(Word))
	{
		for (; size; --size, value >>= 8)
		{
			outs.put(static_cast<char>(value & 0xFF));
		}

		return outs;
	}

	void LinuxAudioRecorder::Start()
	{
		Stop();

		U32 rate = GetSampleRate();
		const int SSIZE = 1024;

		fDevice = alcCaptureOpenDevice(NULL, rate, AL_FORMAT_STEREO16, SSIZE);

		if (fDevice)
		{
			alcCaptureStart(fDevice);
			fIsRunning = true;

			// attach to onEnterFrame
			Rtt_ASSERT(fAudioTask == NULL);
			fAudioTask = Rtt_NEW(Allocator(), AudioTask(this));
			Runtime* runtime = wxGetApp().GetRuntime();
			Scheduler& scheduler = runtime->GetScheduler();
			scheduler.Append(fAudioTask);

			f.open (fFile.GetString(), std::ofstream::out | std::ofstream::binary);

			// Write the file headers

			// Write the file headers
			f << "RIFF----WAVEfmt ";     // (chunk size to be filled in later)
			write_word(f, 16, 4);  // no extension data
			write_word(f, 1, 2);  // PCM - integer samples
			write_word(f, 2, 2);  // two channels (stereo file)
			write_word(f, rate, 4);  // samples per second (Hz)
			write_word(f, rate * 2 * fChannels, 4);  // (Sample Rate * BitsPerSample * Channels) / 8
			write_word(f, 4, 2);  // data block size (size of two integer samples, one for each channel, in bytes)
			write_word(f, 16, 2);  // number of bits per sample (use a multiple of 8)

			// Write the data chunk header
			fWavDataChunkPos = f.tellp();		// save pos
			f << "data----";  // (chunk size to be filled in later)
		}
	}

	void LinuxAudioRecorder::Stop()
	{
		if (f.is_open())
		{
			// (We'll need the final file size to fix the chunk sizes above)
			size_t file_length = f.tellp();

			// Fix the data chunk header to contain the data size
			f.seekp(fWavDataChunkPos + 4);
			write_word(f, file_length - fWavDataChunkPos + 8);

			// Fix the file header to contain the proper RIFF chunk size, which is (file size - 8) bytes
			f.seekp(0 + 4);
			write_word(f, file_length - 8, 4);
			f.close();
		}

		fWavDataChunkPos = 0;

		if (fAudioTask)
		{
			fAudioTask->setKeepAlive(false);
			fAudioTask = NULL;
		}

		if (fDevice)
		{
			alcCaptureStop(fDevice);
			fDevice = NULL;
		}

		fIsRunning = false;
	}

	// onEnterFrame
	void LinuxAudioRecorder::onEnterFrame()
	{
		if (fDevice)
		{
			// Get the number of samples available
			ALint samplesAvailable;
			alcGetIntegerv(fDevice, ALC_CAPTURE_SAMPLES, (ALCsizei) sizeof(ALint), &samplesAvailable);

			if (samplesAvailable > 0)
			{
				int bufsize = samplesAvailable * fChannels * sizeof(int16_t);		// AL_FORMAT_STEREO16
				char* buffer = (char*) malloc(bufsize);

				// Copy the samples to our capture buffer
				alcCaptureSamples(fDevice, buffer, samplesAvailable);

				// LITTLE ENDIAN !!!
				f.write(buffer, bufsize);
				free(buffer);
			}
		}
	}
} // namespace Rtt
