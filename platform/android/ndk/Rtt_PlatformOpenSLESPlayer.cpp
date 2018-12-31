//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"

#if defined(Rtt_USE_OPENSLES)

#include <dlfcn.h>
#include <unistd.h>
#include <vector>
#include <android/log.h>
//#include <sys/timeb.h>	// for ftime()
#include "Rtt_PlatformOpenSLESPlayer.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"
#include "Rtt_PlatformTimer.h"
#include "LuaHashMap.h"
#include "Rtt_PlatformAudioSessionManager.h"
#include "NativeToJavaBridge.h" 

namespace Rtt
{
	PlatformOpenSLESPlayer OpenSLES_player;
	PlatformOpenSLESPlayer* getOpenSLES_player()	{ return &OpenSLES_player; }

#define LOG_TAG "Corona"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define callSLES(cmd)	result = cmd;	if (slError(result)) {	LOGI("line %d", __LINE__);  }
#define callSLESBOOL(cmd)	result = cmd;	if (slError(result)) {	LOGI("line %d", __LINE__); return false; }
#define callSLESVOID(cmd)	result = cmd;	if (slError(result)) {	LOGI("line %d", __LINE__); return; }
#define callSLESZERO(cmd)	result = cmd;	if (slError(result)) {	LOGI("line %d", __LINE__); return 0; }

	const char* slResultToString(SLresult res)
	{
		switch (res)
		{
		case SL_RESULT_SUCCESS: return "Success";
		case SL_RESULT_BUFFER_INSUFFICIENT: return "Buffer insufficient";
		case SL_RESULT_CONTENT_CORRUPTED: return "Content corrupted";
		case SL_RESULT_CONTENT_NOT_FOUND: return "Content not found";
		case SL_RESULT_CONTENT_UNSUPPORTED: return "Content unsupported";
		case SL_RESULT_CONTROL_LOST: return "Control lost";
		case SL_RESULT_FEATURE_UNSUPPORTED: return "Feature unsupported";
		case SL_RESULT_INTERNAL_ERROR: return "Internal error";
		case SL_RESULT_IO_ERROR: return "IO error";
		case SL_RESULT_MEMORY_FAILURE: return "Memory failure";
		case SL_RESULT_OPERATION_ABORTED: return "Operation aborted";
		case SL_RESULT_PARAMETER_INVALID: return "Parameter invalid";
		case SL_RESULT_PERMISSION_DENIED: return "Permission denied";
		case SL_RESULT_PRECONDITIONS_VIOLATED: return "Preconditions violated";
		case SL_RESULT_RESOURCE_ERROR: return "Resource error";
		case SL_RESULT_RESOURCE_LOST: return "Resource lost";
		case SL_RESULT_UNKNOWN_ERROR: return "Unknown error";
		default: return "Undefined error";
		}
	}

	inline bool slError(SLresult res)
	{
		if (res != SL_RESULT_SUCCESS)
		{
			LOGI("[OpenSL Error]: %s", slResultToString(res));
		}
		return res != SL_RESULT_SUCCESS;
	}

	//
	// sound event
	//

	const char openSLESoundCompletionEvent::kName[] = "audio";
	openSLESoundCompletionEvent::openSLESoundCompletionEvent(int ch, const char* handle, bool completed)
		:	fChannel(ch) // add 1 to convert from ALmixer starts arrays at 0 to Lua starts arrays at 1
		, fHandle(handle)
		, fCompleted(completed)
	{
	}

	const char*	openSLESoundCompletionEvent::Name() const
	{
		return Self::kName;
	}

	int	openSLESoundCompletionEvent::Push(lua_State *L) const
	{
		if (Rtt_VERIFY(Super::Push(L)))
		{
			lua_pushinteger(L, fChannel);
			lua_setfield(L, -2, "channel");

			lua_pushinteger(L, fChannel);
			lua_setfield(L, -2, "source");

			lua_pushstring(L, fHandle);
			lua_setfield(L, -2, "handle");

			lua_pushboolean(L, fCompleted);
			lua_setfield(L, -2, "completed");

			// Future phases might relate to "fade"
			lua_pushstring(L, "stopped");
			lua_setfield(L, -2, "phase");
		}
		return 1;
	}

	void onPrefetchCallback(SLPrefetchStatusItf itf, void *ctx, SLuint32 event)
	{
		mycondition* condition = static_cast<mycondition*>(ctx);

		SLpermille level = 0;		// in range [0..1000] permille
		SLresult result = (*itf)->GetFillLevel(itf, &level);
		if (result == SL_RESULT_SUCCESS)
		{
			SLuint32 status = 0;
			result = (*itf)->GetPrefetchStatus(itf, &status);
			if (result == SL_RESULT_SUCCESS)
			{
				if (level == 1000)
				{
					// prefetch buffer is full
					condition->signal();
				}
				else
					if ((PREFETCH_ERROR_MASK == (event & PREFETCH_ERROR_MASK)) && (level == 0) && (status == SL_PREFETCHSTATUS_UNDERFLOW))
					{
						condition->setRetcode(1);
						condition->signal();
					}
				return;
			}
		}
		condition->setRetcode(2);
		condition->signal();
	}

	//
	// sound handler
	//

	// default settings
	static int sBufferSize(2048);
	static int sSampleRate(SL_SAMPLINGRATE_44_1);

	PlatformOpenSLESPlayer::PlatformOpenSLESPlayer()
		: fEngineObject(NULL)
		, fOutputMixObject(NULL)
		, fMasterVolume(1)
		, fReservedChannels(0)
	{
	}

	void PlatformOpenSLESPlayer::init(lua_State *L)
	{
		std::vector<int> settings;
		NativeToJavaBridge::getAudioOutputSettings(settings);
		if (settings.size() >= 2 && settings[0] > 0 && settings[1] > 0)
		{
			sSampleRate = settings[0] * 1000;		// 44100 ==> SL_SAMPLINGRATE_44_1
			sBufferSize = settings[1] * 2;		// stereo
		}

		clearEngine();

		// see if OpenSL library is available
		void* handle = dlopen("libOpenSLES.so", RTLD_LAZY);
		if (handle == NULL)
		{
			LOGI("libOpenSLES.so not available");
			return;
		}

		if (createEngine() == false)
		{
			LOGI("Failed to init OpenSLES");
			return;
		}

		// attach to onEnterFrame
		AudioTask* e = Rtt_NEW(Allocator(), AudioTask());
		Runtime* runtime = LuaContext::GetRuntime(L);
		Scheduler& scheduler = runtime->GetScheduler();
		scheduler.Append(e);

		fSources.clear();
		fPlayers.clear();

		for (int i = 0; i < 30; i++)
		{
			fPlayers.push_back(new aPlayer(i));
		}
	}

	PlatformOpenSLESPlayer::~PlatformOpenSLESPlayer()
	{
		clearEngine();
	}

	void PlatformOpenSLESPlayer::clearEngine()
	{
		fPlayers.clear();
		fSources.clear();

		// destroy output mix object
		if (fOutputMixObject != NULL)
		{
			(*fOutputMixObject)->Destroy(fOutputMixObject);
			fOutputMixObject = NULL;
		}

		// destroy engine object, and invalidate all associated interfaces
		if (fEngineObject != NULL)
		{
			(*fEngineObject)->Destroy(fEngineObject);
			fEngineObject = NULL;
			fEngine = NULL;
		}
	}

	bool PlatformOpenSLESPlayer::createEngine()
	{
		if (createEngineInternal() == false)
		{
			clearEngine();
			return false;
		}
		return true;
	}

	bool PlatformOpenSLESPlayer::createEngineInternal()
	{
		SLresult result;

		// realize the engine
		callSLESBOOL( slCreateEngine(&fEngineObject, 0, NULL, 0, NULL, NULL) );
		callSLESBOOL( (*fEngineObject)->Realize(fEngineObject, SL_BOOLEAN_FALSE) );

		// get the engine interface, which is needed in order to  other objects
		callSLESBOOL( (*fEngineObject)->GetInterface(fEngineObject, SL_IID_ENGINE, &fEngine) );

		// create output mix, with environmental reverb specified as a non-required interface
		const SLInterfaceID ids[1] = { SL_IID_VOLUME };
		const SLboolean req[1] = { SL_BOOLEAN_FALSE };
		int n = sizeof(ids) / sizeof(ids[0]);
		callSLESBOOL( (*fEngine)->CreateOutputMix(fEngine, &fOutputMixObject, n, ids, req) );

		// realize the output mix
		callSLESBOOL( (*fOutputMixObject)->Realize(fOutputMixObject, SL_BOOLEAN_FALSE) );

		return this;
	}

	// the SL_PLAYEVENT_HEADATEND callback handler is currently called with an internal mutex locked,
	// which means if that handler then tries to call any other OpenSL ES API on the same object or interface,
	// it will immediately deadlock.
	// A workaround for app developers is to not issue OpenSL ES APIs within this handler. 
	// Instead, for example, post a pthread condition signal to another app thread which can then issue any OpenSL ES API. 
	void aPlayer::onDecoderCompleted(SLPlayItf player, void* ctx, SLuint32 event)
	{
		if (event & SL_PLAYEVENT_HEADATEND)
		{
			aPlayer* thiz = (aPlayer*)ctx;
			thiz->fIsFinished = true;

			if (thiz->fData)
			{
				thiz->fData->fIsReady = true;
			}
//			LOGI("decoder ended\n");
		}
	}

	void aPlayer::onDecoderBufferDecoded(SLAndroidSimpleBufferQueueItf queueItf, void* ctx)
	{
		aPlayer* thiz = (aPlayer*)ctx;
		thiz->bufferDecoded(queueItf);
	}

	void aPlayer::bufferDecoded(SLAndroidSimpleBufferQueueItf queueItf)
	{
		autolock lock(fMutex);
		if (fData != NULL)
		{
			SLresult result;
			if (fData->fPCM.size() == 1)
			{
				determineDecodedFormat(fData->fFormat);
			}

			fData->fPCM.push_back((U8*) malloc(sBufferSize));
			callSLESVOID((*queueItf)->Enqueue(queueItf, fData->fPCM.back(), sBufferSize));
//			LOGI("decoded %d\n", fData->fPCM.size());
		}
	}

	bool PlatformOpenSLESPlayer::LoadAll(const char* filename)
	{
		if (filename)
		{
			FreeData(filename);

			audioData* adata = new audioData(filename, false);
			fSources[filename] = adata;

			int ch = FindChannelForDecoder();
			if (ch >= 0)
			{
				bool rc = fPlayers[ch]->startDecoder(fEngine, adata);
				//	LOGI("decoder started: %s on channel %d", filename, ch);
				return rc;
			}
		}
		return false;
	}

	bool PlatformOpenSLESPlayer::LoadStream(const char* filename, unsigned int buffer_size, unsigned int max_queue_buffers, unsigned int number_of_startup_buffers, unsigned int number_of_buffers_to_queue_per_update_pass)
	{
		if (filename)
		{
			FreeData(filename);
			fSources[filename] = new audioData(filename, true);
			return true;
		}
		return false;
	}

	int	PlatformOpenSLESPlayer::FadeInChannelTimed(int which_channel, const char* filename, int number_of_loops, unsigned int fade_ticks, unsigned int expire_ticks, PlatformNotifier* callback)
	{
		// LOGI("play %s on channel %d", filename, which_channel);

		//	which_channel	Allows you to specify the specific channel you want to play on.Channels range from 0 to the(Number of allocated channels - 1).
		// If you specify - 1, an available channel will be chosen automatically for you.
		// Returns the channel that was selected for playback or -1 if no channels were available.
		if (which_channel < 0)
		{
			which_channel = FindFreeChannel(fReservedChannels);
			if (which_channel < 0)
			{
				// no available channels
				return -1;
			}
		}

		// sanity check
		if (which_channel >= fPlayers.size() || filename == NULL)
		{
			// wrong channel
			return -1;
		}

		std::map<std::string, smart_ptr<audioData> >::iterator it = fSources.find(filename);
		if (it == fSources.end())
		{
			// no data
			return -1;
		}

		bool rc = fPlayers[which_channel]->play(fEngine, fOutputMixObject, it->second, number_of_loops, expire_ticks, callback);
		if (rc == true && fade_ticks > 0)
		{
			fPlayers[which_channel]->setVolume(0, fMasterVolume);
			FadeChannel(which_channel, fade_ticks, 1);
		}
		return which_channel;
	}

	unsigned int	PlatformOpenSLESPlayer::CountAllFreeChannels() const
	{
		// Returns the number of channels that are currently available for playback(not playing, not paused).
		// ALmixer_CountAllFreeChannels();
		int n = 0;
		for (int i = 0; i < fPlayers.size(); i++) if (fPlayers[i]->isFree())
		{
			n++;
		}
		return n;
	}

	unsigned int	PlatformOpenSLESPlayer::CountUnreservedFreeChannels() const
	{
		// Returns the number of channels that are currently available for playback (not playing, not paused),
		// excluding the channels that have been reserved.
		// ALmixer_CountUnreservedFreeChannels();
		int ret = CountAllFreeChannels() - fReservedChannels;
		return ret < 0 ? 0 : ret; 
	}

	unsigned int	PlatformOpenSLESPlayer::CountTotalChannels() const
	{
		// Returns the number of allocated channels.
		return fPlayers.size();
	}

	unsigned int	PlatformOpenSLESPlayer::CountAllUsedChannels() const
	{
		// Returns the number of channels that are currently in use (playing/paused).
		//ALmixer_CountAllUsedChannels();
		int n = 0;
		for (int i = 0; i < fPlayers.size(); i++) if (fPlayers[i]->isUsed())
		{
			n++;
		}
		return n;
	}

	unsigned int	PlatformOpenSLESPlayer::CountUnreservedUsedChannels() const
	{
		// Returns the number of channels that are currently in use(playing / paused), excluding the channels that have been reserved.
		// ALmixer_CountUnreservedUsedChannels();
		return CountAllUsedChannels() - fReservedChannels;
	}

	unsigned int	PlatformOpenSLESPlayer::CountReservedChannels() const
	{
		// Returns the number of reserved channels.
		// ALmixer_CountReservedChannels();
		return fReservedChannels; 
	}

	void	PlatformOpenSLESPlayer::FreeData(const char* filename)
	{
		if (filename)
		{
			std::map<std::string, smart_ptr<audioData> >::iterator it = fSources.find(filename);
			if (it != fSources.end())
			{
				//LOGI("FreeData %s\n", filename);
				// stop all related playing threads
				for (int i = 0; i < fPlayers.size(); i++)
				{
					if (it->second.get() == fPlayers[i]->getData())
					{
						fPlayers[i]->clear();
					}
				}
				fSources.erase(it);
			}
		}
	}

	float	PlatformOpenSLESPlayer::GetVolumeChannel(int ch) const
	{
		return (ch >= 0 && ch < fPlayers.size()) ? fPlayers[ch]->getVolume() : 0;
	}

	bool	PlatformOpenSLESPlayer::SetVolumeChannel(int ch, float new_volume)
	{
		if (ch >= 0 && ch < fPlayers.size())
		{
			return fPlayers[ch]->setVolume(new_volume, fMasterVolume);
		}
		return false;
	}

	bool	PlatformOpenSLESPlayer::SetMasterVolume(float masterVal)
	{
		fMasterVolume = fclamp(masterVal, 0, 1);
		for (int i = 0; i < fPlayers.size(); i++)
		{
			// reset 
			fPlayers[i]->setVolume(fPlayers[i]->getVolume(), fMasterVolume);
		}
		return true;
	}

	float	PlatformOpenSLESPlayer::GetMasterVolume() const
	{
		return fMasterVolume;
	}

	int	PlatformOpenSLESPlayer::FindFreeChannel(int start_channel) const
	{
		// start_channel	The channel number you want to start looking at.
		// Returns A channel available or -1 if none could be found.
		// ALmixer_FindFreeChannel(start_channel);
		for (int i = start_channel; i >= 0 && i < fPlayers.size(); i++)
		{
			if (fPlayers[i]->isFree())
			{
				return i;
			}
		}
		return -1;
	}

	int	PlatformOpenSLESPlayer::FindChannelForDecoder() const
	{
		for (int i = fPlayers.size() - 1; i >= fReservedChannels; i--)
		{
			if (fPlayers[i]->isFree())
			{
				return i;
			}
		}
		return -1;
	}

	// this must be high optimized
	void	PlatformOpenSLESPlayer::advance()
	{
		for (int i = 0; i < fPlayers.size(); i++)
		{
			aPlayer* p = fPlayers[i];

			// notify onComplete if need
			if (p->isFinished())
			{
				p->clear();
			}
			if (p->fPlayTimer > 0)
			{
				// stream sound looping 

				int currentPosition = p->getPosition();
				int delta = currentPosition - p->fOldPosition;
				if (delta < 0)
				{
					delta += p->fData->fDuration;
				}

//				U64 t = Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime());
//				LOGI("%lld %d %d %d %d", t, p->fPlayTimer, delta, currentPosition, p->fOldPosition);

				p->fOldPosition = currentPosition;
				p->fPlayTimer -= delta;
				if (p->fPlayTimer <= 0)
				{
					StopChannel(i);
				}
			}
			else
			{
				// fade if need
				p->fade();
			}
		}
	}

	int	PlatformOpenSLESPlayer::PauseChannel(int ch)
	{
		// which_channel:	The channel to pause or -1 to pause all channels.
		// Returns: the actual number of channels paused on success or -1 on error.
		if (ch >= 0 && ch < fPlayers.size())
		{
			return fPlayers[ch]->pause();
		}

		if (ch < 0)
		{
			int ret = 0;
			for (int i = 0; i < fPlayers.size(); i++)
			{
				ret += fPlayers[i]->pause();
			}
			return ret;
		}
		return -1;
	}

	int	PlatformOpenSLESPlayer::ResumeChannel(int ch)
	{
		//	which_channel:	The channel to resume or -1 to resume all channels.
		// Returns	The actual number of channels resumed on success or -1 on error.
		if (ch >= 0 && ch < fPlayers.size())
		{
			return fPlayers[ch]->resume();
		}

		if (ch < 0)
		{
			int ret = 0;
			for (int i = 0; i < fPlayers.size(); i++)
			{
				ret += fPlayers[i]->resume();
			}
			return ret;
		}
		return -1;
	}

	int	PlatformOpenSLESPlayer::StopChannel(int ch)
	{
		// 	which_channel:	The channel to halt or -1 to halt all channels.
		// Returns The actual number of channels halted on success or -1 on error.
		// ALmixer_HaltChannel(which_channel);
		if (ch >= 0 && ch < fPlayers.size())
		{
			return fPlayers[ch]->clear();
		}

		if (ch < 0)
		{
			int ret = 0;
			for (int i = 0; i < fPlayers.size(); i++)
			{
				ret += fPlayers[i]->clear();
			}
			return ret;
		}
		return -1;
	}

	int	PlatformOpenSLESPlayer::IsActiveChannel(int ch) const
	{
		// Returns true if the specified channel is currently playing or paused, 
		// or if - 1 is passed the number of channels that are currently playing or paused.
		if (ch >= 0 && ch < fPlayers.size())
		{
			return fPlayers[ch]->isUsed();
		}

		if (ch < 0)
		{
			int n = 0;
			for (int i = 0; i < fPlayers.size(); i++) if (fPlayers[i]->isUsed())
			{
				n++;
			}
			return n;
		}
		return 0;
	}

	int	PlatformOpenSLESPlayer::IsPlayingChannel(int ch) const
	{
		// Returns true if the specified channel is currently playing.
		// or if -1 is passed the number of channels that are currently playing.
		//ALmixer_IsPlayingChannel(which_channel);
		if (ch >= 0 && ch < fPlayers.size())
		{
			return (fPlayers[ch] && fPlayers[ch]->isPlaying()) ? 1 : 0;
		}

		if (ch < 0)
		{
			int n = 0;
			for (int i = 0; i < fPlayers.size(); i++) 
			if (fPlayers[i] && fPlayers[i]->isPlaying())
			{
				n++;
			}
			return n;
		}
		return -1;
	}

	int	PlatformOpenSLESPlayer::IsPausedChannel(int ch) const
	{
		// which_channel = The channel you want to know about or -1 to get the count of currently paused channels.
		// For a specific channel, 1 if the channel is paused, 0 if not.
		// Or returns the count of currently paused channels.
		// Or - 1 on an error.
		if (ch >= 0 && ch < fPlayers.size())
		{
			return (fPlayers[ch] && fPlayers[ch]->isPaused()) ? 1 : 0;
		}

		if (ch < 0)
		{
			int n = 0;
			for (int i = 0; i < fPlayers.size(); i++)
				if (fPlayers[i] && fPlayers[i]->isPaused())
				{
					n++;
				}
			return n;
		}
		return 0;
	}

	int	PlatformOpenSLESPlayer::RewindChannel(int which_channel)
	{
		// ALmixer_RewindChannel(which_channel);
		return SeekChannel(which_channel, 0);	
	}

	int	PlatformOpenSLESPlayer::SeekChannel(int ch, unsigned int number_of_milliseconds)
	{
		// Seeks the sound to the beginning that is playing on a specific channel.
		// which_channel =	The channel to seek or -1 to seek all channels.
		// Returns The actual number of channels rewound on success or -1 on error.
		if (ch >= 0 && ch < fPlayers.size())
		{
			return (fPlayers[ch]->seek(number_of_milliseconds)) ? 1 : 0;
		}

		if (ch < 0)
		{
			int n = 0;
			for (int i = 0; i < fPlayers.size(); i++)
			{
				n += fPlayers[i]->seek(number_of_milliseconds) ? 1 : 0;
			}
			return n;
		}
		return -1;
	}

	bool	PlatformOpenSLESPlayer::SeekData(const char* almixer_data, unsigned int number_of_milliseconds)
	{
		// (bool)ALmixer_SeekData(almixer_data, number_of_milliseconds);
		return false; 
	}

	int	PlatformOpenSLESPlayer::getDuration(SLObjectItf playerObject)
	{
		SLresult result;

		callSLESZERO((*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE));

		// get play interface:
		SLPlayItf slPlay;
		callSLESZERO((*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &slPlay));

		// setup prefetch (listen for errors in opening the compressed file):
		SLPrefetchStatusItf slPrefetch;
		callSLESZERO((*playerObject)->GetInterface(playerObject, SL_IID_PREFETCHSTATUS, &slPrefetch));
		callSLESZERO((*slPrefetch)->RegisterCallback(slPrefetch, onPrefetchCallback, this));
		callSLESZERO((*slPrefetch)->SetCallbackEventsMask(slPrefetch, PREFETCH_ERROR_MASK));

		// start prefetching:
		callSLESZERO((*slPlay)->SetPlayState(slPlay, SL_PLAYSTATE_PAUSED));

		fPrefetch.wait(3000);

		SLmillisecond durationInMsec = 0;
		if (fPrefetch.getRetcode() == 0)
		{
			callSLESZERO((*slPlay)->GetDuration(slPlay, &durationInMsec));
		}
		else
		{
			//LOGI("Failed to read: %s on channel=%d", adata->fPath.c_str(), fChannel);
		}
		return durationInMsec;
	}

	int	PlatformOpenSLESPlayer::GetTotalTime(const char* filename)
	{
		if (filename)
		{
			// The total time in milliseconds or -1 if some kind of failure.
			std::map<std::string, smart_ptr<audioData> >::const_iterator it = fSources.find(filename);
			if (it != fSources.end() && it->second != NULL)
			{
				audioData* adata = it->second;
				if (adata->getDuration() == 0)
				{
					SLresult result;

					// configure audio source
					SLDataLocator_URI sourceLocator = { SL_DATALOCATOR_URI, (SLchar*)adata->fPath.c_str() };
					SLDataFormat_MIME format = { SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED };
					SLDataSource audioSource = { &sourceLocator, &format };

					// configure audio sink
					SLDataLocator_AndroidSimpleBufferQueue sinkLocator = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, OPENSLES_BUFFERS };
					SLDataFormat_PCM fmt = { SL_DATAFORMAT_PCM, 2, sSampleRate, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN };
					SLDataSink audioSink = { &sinkLocator, &fmt };

					SLInterfaceID ids[1] = { SL_IID_PREFETCHSTATUS };
					SLboolean required[1] = { SL_BOOLEAN_TRUE };
					int n = sizeof(ids) / sizeof(ids[0]);

					SLObjectItf playerObject;
					callSLESZERO((*fEngine)->CreateAudioPlayer(fEngine, &playerObject, &audioSource, &audioSink, n, ids, required));

					adata->setDuration(getDuration(playerObject));

					(*playerObject)->Destroy(playerObject);
				}
				return adata->getDuration();
			}
		}
		return 0;
	}

	bool	PlatformOpenSLESPlayer::SetMaxVolumeChannel(int ch, float val)
	{
		// which_channel =	The channel to set the volume to or -1 to set on all channels.
		//	volume	The new volume to use.Valid values are 0.0 to 1.0.
		//	Returns	AL_TRUE on success or AL_FALSE on error.
		if (ch >= 0 && ch < fPlayers.size())
		{
			return (fPlayers[ch] && fPlayers[ch]->setMaxVolume(val)) ? true : false;
		}

		if (ch < 0)
		{
			for (int i = 0; i < fPlayers.size(); i++)
			{
				fPlayers[i]->setMaxVolume(val);
			}
			return true;
		}
		return false;
	}

	float	PlatformOpenSLESPlayer::GetMaxVolumeChannel(int ch) const
	{
		// which_channel	= The channel to get the volume from. - 1 will return the average volume set across all channels.
		// Returns the volume for the specified channel, or the average set volume for all channels, or -1.0 on error.
		// ALmixer_GetMaxVolumeChannel(which_channel);
		if (ch >= 0 && ch < fPlayers.size())
		{
			return fPlayers[ch]->getMaxVolume();
		}

		if (ch < 0)
		{
			float sum = 0;
			for (int i = 0; i < fPlayers.size(); i++)
			{
				sum += fPlayers[i]->getMaxVolume();
			}
			return sum / fPlayers.size();
		}
		return 0;
	}

	bool	PlatformOpenSLESPlayer::SetMinVolumeChannel(int ch, float val)
	{
		// which_channel =	The channel to set the volume to or -1 to set on all channels.
		//	volume	The new volume to use.Valid values are 0.0 to 1.0.
		//	Returns	AL_TRUE on success or AL_FALSE on error.
		if (ch >= 0 && ch < fPlayers.size())
		{
			return fPlayers[ch]->setMinVolume(val);
		}

		if (ch < 0)
		{
			for (int i = 0; i < fPlayers.size(); i++)
			{
				fPlayers[i]->setMinVolume(val);
			}
			return true;
		}
		return false;
	}

	float PlatformOpenSLESPlayer::GetMinVolumeChannel(int ch) const
	{
		// which_channel	= The channel to get the volume from. - 1 will return the average volume set across all channels.
		// Returns the volume for the specified channel, or the average set volume for all channels, or -1.0 on error.
		if (ch >= 0 && ch < fPlayers.size())
		{
			return fPlayers[ch]->getMinVolume();
		}

		if (ch < 0)
		{
			float sum = 0;
			for (int i = 0; i < fPlayers.size(); i++)
			{
				sum += fPlayers[i]->getMinVolume();
			}
			return sum / fPlayers.size();
		}
		return 0;
	}

	int	PlatformOpenSLESPlayer::FadeOutChannel(int which_channel, unsigned int fade_ticks)
	{
		// ALmixer_FadeOutChannel(which_channel, fade_ticks);
		return FadeChannel(which_channel, fade_ticks, 0);
	}

	int	PlatformOpenSLESPlayer::FadeChannel(int ch, unsigned int fade_ticks, float to_volume)
	{
		if (ch >= 0 && ch < fPlayers.size())
		{
			return fPlayers[ch]->startFading(fade_ticks, fPlayers[ch]->getVolume(), to_volume);
		}

		int countFadedChannels = 0;
		if (ch < 0)
		{
			for (int i = 0; i < fPlayers.size(); i++) if (fPlayers[i]->isUsed())
			{
				countFadedChannels += fPlayers[i]->startFading(fade_ticks, fPlayers[i]->getVolume(), to_volume);
			}
		}
		return countFadedChannels;
	}

	int PlatformOpenSLESPlayer::ExpireChannel(int which_channel, int number_of_milliseconds)
	{
		// ALmixer_ExpireChannel(which_channel, number_of_milliseconds);
		return -1; 
	}

	int	PlatformOpenSLESPlayer::ReserveChannels(int number_of_reserve_channels)
	{
		// Allows you to reserve a certain number of channels so they won't be automatically allocated to play on.
		// number_of_reserve_channels =	The number of channels / sources to reserve.
		// Or pass - 1 to find out how many channels are currently reserved.
		// ALmixer_ReserveChannels(number_of_reserve_channels);
		if (number_of_reserve_channels >= 0)
		{
			fReservedChannels = iclamp(number_of_reserve_channels, 0, fPlayers.size());
		}
		return fReservedChannels;
	}

	void PlatformOpenSLESPlayer::SuspendPlayer()
	{
		for (int i = 0; i < fPlayers.size(); i++)
		{
			if (fPlayers[i]->isPlaying())
			{
				fPlayers[i]->fPausedBySystem = true;
				fPlayers[i]->pause();
			}
		}
	}

	void PlatformOpenSLESPlayer::ResumePlayer()
	{
		for (int i = 0; i < fPlayers.size(); i++)
		{
			if (fPlayers[i]->fPausedBySystem)
			{
				fPlayers[i]->fPausedBySystem = false;
				fPlayers[i]->resume();
			}
		}
	}

	void PlatformOpenSLESPlayer::setProperty(int ch, const char* key, float val)
	{
		if (key && Rtt_StringCompare(key, "PITCH") == 0)
		{
			if (ch >= 0 && ch < fPlayers.size())
			{
				fPlayers[ch]->setPitch(val);
			}
		}
	}


	//
	// aPlayer
	//

	aPlayer::aPlayer(int channel)
		: fPlayerObject(NULL)
		, fNotifier(NULL)
		, fChannel(channel)
		, fVolume(1)
		, fMaxVolume(1)
		, fMinVolume(0)
		, fMaster(1)
		, fLoops(0)		// 0 means one time
		, fPausedBySystem(false)
		, fFade(fadeParams())
		, fCurrentBuffer(0)
		, fIsFinished(false)
		, fPitch(1)
		, fPlayTimer(0)
		, fOldPosition(0)
	{
		clear();
	}

	aPlayer::~aPlayer()
	{
		clear();
	}

	int aPlayer::clear()
	{
		SLresult result;
		if (fPlayerObject)
		{
			//LOGI("clear channel=%d, object=%p, data=%p, path=%s", fChannel, fPlayerObject, fData.get(), fData ? fData->fPath.c_str() : "");
			autolock lock(fMutex);

			// clear queue
			SLAndroidSimpleBufferQueueItf slQueue;
			result = (*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &slQueue);
			if (result == SL_RESULT_SUCCESS)
			{
				result = (*slQueue)->Clear(slQueue);
			}

			SLPlayItf player;
			result = (*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_PLAY, &player);
			if (result == SL_RESULT_SUCCESS)
			{
				result = (*player)->SetPlayState(player, SL_PLAYSTATE_STOPPED);
				if (result == SL_RESULT_SUCCESS)
				{
					SLuint32 state;
					result = (*player)->GetPlayState(player, &state);
					if (state != SL_PLAYSTATE_STOPPED || result != SL_RESULT_SUCCESS)
					{
						// something wrong in openSL
						LOGI("[OpenSLES Error]: Playing not stoped");
					}
				}
				else
				{
					// something wrong in openSL
					LOGI("[OpenSLES Error]: Failed to stop playing");
				}
			}
			else
			{
				// something wrong in openSL
				LOGI("[OpenSLES Error]: SL_IID_PLAY interface must be always if object is not ZERO");
			}
		}


		// now one can to destroy the player
		// this must be outside of mutex
		if (fPlayerObject)
		{
			(*fPlayerObject)->Destroy(fPlayerObject);
			fPlayerObject = NULL;
			//LOGI("fPlayerObject destryed");
		}

		// this must be outside of mutex because of possible nested calls
		if (fNotifier)
		{
			PlatformNotifier* notifier = getNotifier();
			if (notifier != NULL && notifier->HasListener() && notifier->GetLuaState())
			{
				openSLESoundCompletionEvent e(fChannel + 1, getPath(), fIsFinished);
				notifier->CallListener(openSLESoundCompletionEvent::kName, e);
			}
			Rtt_DELETE(fNotifier);
			fNotifier = NULL;
		}

		// async using of fData may be only in Player, Player is destroyed here, so no need mutex
		fData = NULL;

		fFade = fadeParams();		// clear
		fIsFinished = false;
		fCurrentBuffer = 0;
		fPlayTimer = 0;
		fOldPosition = 0;

		//LOGI("channel cleaned");
		return 1;
	}


	// stream player callback
	void aPlayer::onPlayerStreamCallback(SLPlayItf player, void *pContext, SLuint32 event)
	{
		SLresult result;
		aPlayer* p = (aPlayer*)pContext;
		p->fIsFinished = true;
	}

	// This callback handler is called every time a buffer finishes playing.
	// The documentation available is very unclear about how to best manage buffers.
	// I've chosen to this approach: Instantly enqueue a buffer that was rendered to the last time,
	// and then render the next. Hopefully it's okay to spend time in this callback after having enqueued. 
	void aPlayer::onPlayerQueueCallback(SLAndroidSimpleBufferQueueItf queueItf, void *ctx)
	{
		aPlayer* thiz = static_cast<aPlayer*>(ctx);
		thiz->playBuffer(queueItf);
	}

	void aPlayer::playBuffer(SLAndroidSimpleBufferQueueItf queueItf)
	{
		autolock lock(fMutex);

		if (fData != NULL)
		{
			if (++fCurrentBuffer >= fData->fPCM.size() - 1)
			{
				if (fLoops >= 0)
				{
					if (fLoops == 0)
					{
						// stop playing
						fIsFinished = true;
						return;
					}
					fLoops--;
				}

				// rewind
				fCurrentBuffer = 0;
			}
			SLresult result;
			callSLESVOID((*queueItf)->Enqueue(queueItf, fData->fPCM[fCurrentBuffer], sBufferSize));
		}
	}

	long long aPlayer::getTicks() const
	{
		struct timeval t;
		gettimeofday(&t, NULL);
		return (long long)t.tv_sec * 1000L + t.tv_usec / 1000; //get current timestamp in milliseconds
	}

	int aPlayer::startFading(int fade_ticks, float from, float to)
	{
		fFade.fStartTime = getTicks();
		fFade.fTime = fade_ticks;
		fFade.fFrom = from;
		fFade.fTo = to;
		return 1;
	}

	bool aPlayer::startDecoder(SLEngineItf fEngine, audioData* adata)
	{
		autolock lock(fMutex);

		fData = adata;

		SLresult result;

		// configure audio source
		SLDataLocator_URI sourceLocator = { SL_DATALOCATOR_URI, (SLchar*) adata->fPath.c_str() };
		SLDataFormat_MIME format = { SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED };
		SLDataSource audioSource = { &sourceLocator, &format };

		// configure audio sink
		SLDataLocator_AndroidSimpleBufferQueue sinkLocator = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, OPENSLES_BUFFERS };
		SLDataFormat_PCM fmt = { SL_DATAFORMAT_PCM, 2, sSampleRate, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN };
		SLDataSink audioSink = { &sinkLocator, &fmt };

		SLInterfaceID ids[2] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_METADATAEXTRACTION };
		SLboolean required[2] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
		int n = sizeof(ids) / sizeof(ids[0]);

		callSLESBOOL((*fEngine)->CreateAudioPlayer(fEngine, &fPlayerObject, &audioSource, &audioSink, n, ids, required));
		callSLESBOOL((*fPlayerObject)->Realize(fPlayerObject, SL_BOOLEAN_FALSE));

		// setup play interface:
		SLPlayItf slPlay;
		callSLESBOOL((*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_PLAY, &slPlay));
		SLuint32 mask = SL_PLAYEVENT_HEADATEND | SL_PLAYEVENT_HEADSTALLED | SL_PLAYEVENT_HEADATNEWPOS;
		callSLESBOOL((*slPlay)->SetCallbackEventsMask(slPlay, mask));
		callSLESBOOL((*slPlay)->RegisterCallback(slPlay, onDecoderCompleted, this));

		// setup buffer queue:
		SLAndroidSimpleBufferQueueItf slQueue;
		callSLESBOOL((*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &slQueue));
		callSLESBOOL((*slQueue)->RegisterCallback(slQueue, onDecoderBufferDecoded, this));

		fIsFinished = false;
		fData->fPCM.push_back((U8*)malloc(sBufferSize)); // data);
		callSLESBOOL((*slQueue)->Enqueue(slQueue, fData->fPCM.back(), sBufferSize));

		// start decoding:
		callSLESBOOL((*slPlay)->SetPlayState(slPlay, SL_PLAYSTATE_PLAYING));
		return true;
	}

	bool aPlayer::prefetch(audioData* adata)
	{
		//LOGI("prefetch %s, ch=%d", adata->fPath.c_str(), fChannel);
		if (fPlayerObject)
		{
			SLresult result;

			SLPlayItf player;
			callSLESBOOL((*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_PLAY, &player));

			// setup prefetch (listen for errors in opening the compressed file):
			SLPrefetchStatusItf slPrefetch;
			callSLESBOOL((*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_PREFETCHSTATUS, &slPrefetch));
			callSLESBOOL((*slPrefetch)->RegisterCallback(slPrefetch, onPrefetchCallback, &fPrefetch));
			callSLESBOOL((*slPrefetch)->SetCallbackEventsMask(slPrefetch, PREFETCH_ERROR_MASK));

			// start prefetching:
			callSLESBOOL((*player)->SetPlayState(player, SL_PLAYSTATE_PAUSED));
			
			fPrefetch.wait(3000);
			if (fPrefetch.getRetcode() == 0)
			{
				determineDecodedFormat(adata->fFormat);
			}
			else
			{
				LOGI("Failed to prefetch '%s' on channel=%d", adata->fPath.c_str(), fChannel);
			}
			return fPrefetch.getRetcode() == 0;
		}
		return false;
	}

	bool aPlayer::play(SLEngineItf fEngine, SLObjectItf fOutputMixObject, audioData* adata, int number_of_loops, int duration, PlatformNotifier* callback)
	{
		SLresult result;
		 
		// same sound ?
		if (fPlayerObject && fData.get() == adata)
		{
			// LOGI("re-play same sound on channel=%d, ignoring", fChannel);
			return true;
		}

		// destroy active player
		clear();

		// here no active player so no mutex lock needs

		fData = adata;
		fLoops = number_of_loops;
		fNotifier = callback;

		if (adata->fIsStream)
		{
			//LOGI("play stream sound: %s, channel=%d", adata->fPath.c_str(), fChannel);
			// set source
			SLDataLocator_URI loc_fd = { SL_DATALOCATOR_URI, (SLchar*) adata->fPath.c_str() };
			SLDataFormat_MIME format = { SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED };
			SLDataSource audioSrc = { &loc_fd, &format };

			SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX,  fOutputMixObject };
			SLDataSink audioSnk = { &loc_outmix, NULL }; 

			const SLInterfaceID ids[8] = { SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME, SL_IID_PREFETCHSTATUS, SL_IID_METADATAEXTRACTION, SL_IID_BASSBOOST, SL_IID_EFFECTSEND, SL_IID_PLAYBACKRATE };
			const SLboolean req[8] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
			int n = sizeof(ids) / sizeof(ids[0]);

			//Rtt_ASSERT(fPlayerObject == NULL);
			callSLESBOOL((*fEngine)->CreateAudioPlayer(fEngine, &fPlayerObject, &audioSrc, &audioSnk, n, ids, req));

			// realize
			callSLESBOOL((*fPlayerObject)->Realize(fPlayerObject, SL_BOOLEAN_FALSE));

			// register player callback 
			SLPlayItf player;
			callSLESBOOL((*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_PLAY, &player));

			callSLESBOOL((*player)->RegisterCallback(player, onPlayerStreamCallback, this));
			callSLESBOOL((*player)->SetCallbackEventsMask(player, SL_PLAYEVENT_HEADATEND | SL_PLAYEVENT_HEADATMARKER));

			// prefetch
			if (prefetch(adata) == false)
			{
				clear();
				return false;
			}

			callSLESZERO((*player)->GetDuration(player, &fData->fDuration));

			// get seek interface
			SLSeekItf seek;
			callSLESBOOL( (*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_SEEK, &seek));

			// start infinite looping in case of loops=-1 or loops > 0
			bool isLooping = number_of_loops == 0 ? false : true;
			callSLESBOOL( (*seek)->SetLoop(seek, isLooping ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE, 0, SL_TIME_UNKNOWN));

			// set looping timer
			fPlayTimer = (number_of_loops > 0) ? fData->fDuration * (number_of_loops + 1) : 0;		// for 2+ loops
			fOldPosition = 0;

			callSLESBOOL( (*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING));

			if (duration > 0)
			{
				callSLESBOOL( (*player)->SetMarkerPosition(player, duration));
			}

			// set pre-defined values
			setVolume(fVolume, fMaster);
			setPitch(fPitch);

			return true;
		}
		else
		{ 
			// wait for data, 1 sec
			bool dataDecoded = false;
			for (int i = 0; i < 10000; i++)
			{
				{
					autolock lock(fMutex);
					if (fData->fIsReady || fData->fPCM.size() >= 20)		// ensure that exist enough data
					{
						dataDecoded = true;
						break;
					}
				}
				usleep(100);
			}

			if (dataDecoded)
			{
				//LOGI("play buffered sound: %s, channel=%d", adata->fPath.c_str(), fChannel);
				SLresult result;
				SLDataLocator_AndroidSimpleBufferQueue loc_fd = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, OPENSLES_BUFFERS };

				//LOGI("play =%d, rate=%d, samples=%d", fData->fFormat.numChannels, fData->fFormat.bitsPerSample, fData->fFormat.samplesPerSec);
				SLDataSource audioSrc = { &loc_fd, &fData->fFormat };

				SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX,  OpenSLES_player.fOutputMixObject };
				SLDataSink audioSnk = { &loc_outmix, NULL };

				const SLInterfaceID ids[4] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_VOLUME, SL_IID_PLAYBACKRATE, SL_IID_EFFECTSEND };
				const SLboolean req[4] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
				int n = sizeof(ids) / sizeof(ids[0]);

				//				Rtt_ASSERT(fPlayerObject == NULL);
				callSLESBOOL((*OpenSLES_player.fEngine)->CreateAudioPlayer(OpenSLES_player.fEngine, &fPlayerObject, &audioSrc, &audioSnk, n, ids, req));

				// realize
				callSLESBOOL((*fPlayerObject)->Realize(fPlayerObject, SL_BOOLEAN_FALSE));

				// set pre-defined values
				setVolume(fVolume, fMaster);
				setPitch(fPitch);

				SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
				callSLESBOOL((*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &bqPlayerBufferQueue));
				callSLESBOOL((*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, onPlayerQueueCallback, this));

				// start playing
				fCurrentBuffer = 0;
				SLPlayItf player;
				callSLESBOOL((*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_PLAY, &player));
				callSLESBOOL((*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING));
				callSLESBOOL((*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, fData->fPCM[0], sBufferSize));

				return true;
			}
		}
		return false;
	}

	float aPlayer::getVolume() const
	{
		return fVolume;
	}

	bool aPlayer::setVolume(float val, float master)
	{
		fVolume = fclamp(val, fMinVolume, fMaxVolume);
		fMaster = fclamp(master, 0, 1);
		if (fPlayerObject)
		{
			SLresult result;

			SLVolumeItf volumeItf;
			result = (*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_VOLUME, &volumeItf);
			if (result == SL_RESULT_SUCCESS)
			{
				// linear volume to millibel
				val = fVolume * master;
				SLmillibel Volume = val < 0.001 ? SL_MILLIBEL_MIN : 20 * 100 * log10f(val);
				callSLESBOOL((*volumeItf)->SetVolumeLevel(volumeItf, Volume));
			}
		}
		return true;
	}

	// returns 1=paused, 0=not paused
	int aPlayer::pause()
	{
		if (fPlayerObject)
		{
			SLresult result;

			// get player interface
			SLPlayItf player;
			callSLESBOOL((*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_PLAY, &player));

			// get current state
			SLuint32 state; 
			callSLESBOOL((*player)->GetPlayState(player, &state));

			(*player)->SetPlayState(player, SL_PLAYSTATE_PAUSED);

			// clear queue
			SLAndroidSimpleBufferQueueItf slQueue;
			result = (*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &slQueue);
			if (result == SL_RESULT_SUCCESS)
			{
				result = (*slQueue)->Clear(slQueue);
			}

			return 1;
		}
		return 0;
	}

	int aPlayer::resume()
	{
		if (fPlayerObject)
		{
			SLresult result;

			// get player interface
			SLPlayItf player;
			callSLESZERO( (*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_PLAY, &player));

			// get current state
			SLuint32 state;
			callSLESZERO( (*player)->GetPlayState(player, &state));

			callSLESZERO( (*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING));

			// for buffered sound only
			SLAndroidSimpleBufferQueueItf slQueue;
			result = (*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &slQueue);
			if (result == SL_RESULT_SUCCESS)
			{
				autolock lock(fMutex);
				fCurrentBuffer = iclamp(fCurrentBuffer, 0, fData->fPCM.size());
				callSLESZERO((*slQueue)->Enqueue(slQueue, fData->fPCM[fCurrentBuffer], sBufferSize));
			}

			return 1;
		}
		return 0;
	}

	bool aPlayer::isPaused()
	{
		if (fPlayerObject)
		{
			SLresult result;

			// get player interface
			SLPlayItf player;
			callSLESBOOL( (*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_PLAY, &player));

			SLuint32 state;
			callSLESBOOL( (*player)->GetPlayState(player, &state));
			return state == SL_PLAYSTATE_PAUSED;
		}
		return false;
	}	
	
	bool aPlayer::isPlaying()
	{
		if (fPlayerObject)
		{
			SLresult result;

			// get player interface
			SLPlayItf player;
			callSLESBOOL( (*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_PLAY, &player));

			SLuint32 state;
			callSLESBOOL( (*player)->GetPlayState(player, &state));
			return state == SL_PLAYSTATE_PLAYING;
		}
		return false;
	}

	bool aPlayer::seek(int ms)
	{
		fCurrentBuffer = 0;
		if (fPlayerObject)
		{
			SLresult result;

			// get seek interface
			SLSeekItf seek;
			result = (*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_SEEK, &seek);
			if (result == SL_RESULT_SUCCESS)
			{
				(*seek)->SetPosition(seek, ms, SL_SEEKMODE_FAST);
			}
			return true;
		}
		return false;
	}

	void aPlayer::fade()
	{
		if (fFade.fTime > 0 && isPlaying())
		{
			int ticks = getTicks() - fFade.fStartTime;
			float k = (float)(ticks) / (float)fFade.fTime;
			k = fclamp(k, 0, 1);

			float vol = fFade.fFrom + (fFade.fTo - fFade.fFrom) * k;
			setVolume(vol, fMaster);
			if (k >= 1)
			{
				// stop fading
				fFade = fadeParams();		// clear
			}
		}
	}

	bool aPlayer::determineDecodedFormat(SLDataFormat_PCM& ret)
	{
		if (fPlayerObject == NULL)
		{
			return false;
		}

		SLresult result;

		SLMetadataExtractionItf extractor;
		callSLESBOOL((*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_METADATAEXTRACTION, &extractor));

		SLuint32 mdCount = 0;
		SLuint32 i;

		ret = SLDataFormat_PCM();
		ret.formatType = SL_DATAFORMAT_PCM;

		// scan through the metadata items
		callSLESBOOL( (*extractor)->GetItemCount(extractor, &mdCount));

		for (i = 0; i < mdCount; ++i)
		{
			SLMetadataInfo *key = NULL;
			SLMetadataInfo *value = NULL;
			SLuint32 itemSize = 0;

			// Get the size of and malloc memory for the metadata item
			callSLESBOOL( (*extractor)->GetKeySize(extractor, i, &itemSize));
			key = (SLMetadataInfo*)malloc(itemSize);

			// Extract the key into the memory
			callSLES( (*extractor)->GetKey(extractor, i, itemSize, key));
			if (result != SL_RESULT_SUCCESS)
			{
				free(key);
				return false;
			}

			callSLES( (*extractor)->GetValueSize(extractor, i, &itemSize));
			if (result != SL_RESULT_SUCCESS)
			{
				free(key);
				return false;
			}

			value = (SLMetadataInfo*)malloc(itemSize);

			// Extract the value into memory
			callSLES( (*extractor)->GetValue(extractor, i, itemSize, value));
			if (result != SL_RESULT_SUCCESS)
			{
				free(value);
				free(key);
				return false;
			}

			if (strcmp((const char *)key->data, ANDROID_KEY_PCMFORMAT_NUMCHANNELS) == 0)
			{
				ret.numChannels = *((SLuint32*)value->data);
			}
			else if (strcmp((const char *)key->data, ANDROID_KEY_PCMFORMAT_SAMPLERATE) == 0)
			{
				ret.samplesPerSec = *((SLuint32*)value->data) * 1000;
			}
			else if (strcmp((const char *)key->data, ANDROID_KEY_PCMFORMAT_BITSPERSAMPLE) == 0)
			{
				ret.bitsPerSample = *((SLuint32*)value->data);
			}
			else if (strcmp((const char *)key->data, ANDROID_KEY_PCMFORMAT_CONTAINERSIZE) == 0)
			{
				ret.containerSize = *((SLuint32*)value->data);
			}
			else if (strcmp((const char *)key->data, ANDROID_KEY_PCMFORMAT_CHANNELMASK) == 0)
			{
				ret.channelMask = *((SLuint32*)value->data);
			}
			else if (strcmp((const char *)key->data, ANDROID_KEY_PCMFORMAT_ENDIANNESS) == 0)
			{
				ret.endianness = *((SLuint32*)value->data);
			}
			free(value);
			free(key);
		}
		//LOGI("[slDecoder] input metadata. sr: %u, ch: %u, bits: %u", ret.samplesPerSec, ret.numChannels, ret.bitsPerSample);
		return true;
	}

	bool aPlayer::setPitch(float val)
	{
		fPitch = val;		// keep, it may be used later when the channel activated
		if (fPlayerObject)
		{
			SLresult result;

			SLPlaybackRateItf pbItf;
			callSLESBOOL((*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_PLAYBACKRATE, &pbItf));

			SLpermille stepSize;
			SLuint32 capabilities;
			SLpermille minRate;
			SLpermille maxRate;
			callSLESBOOL((*pbItf)->GetRateRange(pbItf, 0, &minRate, &maxRate, &stepSize, &capabilities));

			// result=(*pbItf)->SetPropertyConstraints(pbItf, SL_RATEPROP_NOPITCHCORAUDIO);
			// (void)result;

			// val = fclamp(val, 0.5f, 2.0f);
			SLpermille playbackRate = (SLpermille)iclamp(1000 * val, minRate, maxRate);
			callSLESBOOL((*pbItf)->SetRate(pbItf, playbackRate));

			return true;
		}
		return false;
	}

	bool aPlayer::setMaxVolume(float val)
	{
		fMaxVolume = val; 

		// reset 
		setVolume(getVolume(), fMaster);
	}

	bool aPlayer::setMinVolume(float val)
	{
		fMinVolume = val; 

		// reset 
		setVolume(getVolume(), fMaster);
	}

	int aPlayer::getPosition()
	{
		SLmillisecond msec = 0;
		if (fPlayerObject)
		{
			SLresult result;

			// get player interface
			SLPlayItf player;
			callSLESZERO((*fPlayerObject)->GetInterface(fPlayerObject, SL_IID_PLAY, &player));
			callSLESZERO((*player)->GetPosition(player, &msec));
		}
		return msec;
	}

	//
	// audioData
	//

	audioData::audioData(const char* filename, bool isStream)
		: fIsStream(isStream)
		, fFormat(SLDataFormat_PCM())
		, fPath(filename)
		, fDuration(0)
		, fIsReady(false)
	{
		Rtt_ASSERT(filename);
	}

	audioData::~audioData()
	{
		for (int i = 0; i < fPCM.size(); i++)
		{
			delete fPCM[i];
		}
	}

	// onEnterFrame
	void AudioTask::operator()(Scheduler& sender)
	{
		OpenSLES_player.advance();
	}

} // namespace Rtt
 
#endif // Rtt_USE_OPENSLES

