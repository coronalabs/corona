//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#ifdef Rtt_EMSCRIPTEN_ENV

#include "Rtt_PlatformWebAudioPlayer.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"
#include "Rtt_PlatformAudioSessionManager.h"

#if defined(EMSCRIPTEN)
	#include "emscripten/emscripten.h"	
#endif

#ifdef WIN32
	// this fixes #define remove from luaconf.h
	#undef remove
#endif

namespace Rtt
{

	const char SDLmixerSoundCompletionEvent::kName[] = "audio"; 

	// music finished function
	extern PlatformWebAudioPlayer sdlAudio;

#if defined(EMSCRIPTEN)
extern "C"
{
	// Java ==> C callback
	void EMSCRIPTEN_KEEPALIVE jsOnSoundEnded(int channel, PlatformSDLmixerPlaybackFinishedCallback *notifier, bool finished_naturally)
	{
		sdlAudio.NotificationCallback(channel, notifier, finished_naturally);
	}

	// JS API
	extern int jsAudioInit();
	extern int jsAudioDecode(const char* file_path, const void* data, int size);
	extern int jsAudioGet(const char* file_path);
	extern int jsAudioPlay(int channel, int soundID, int loops, int ticks, const void* callback, int fade_ticks);
	extern int jsAudioPause(int channel);
	extern int jsAudioResume(int channel);
	extern int jsAudioStop(int channel);
	extern int jsAudioSeek(int channel, int number_of_milliseconds);
	extern int jsAudioRewind(int channel);
	extern int jsAudioIsPlaying(int channel);
	extern int jsAudioCountAllFreeChannels();
	extern float jsAudioGetVolume(int channel);
	extern int jsAudioSetVolume(int channel, float vol);
	extern float jsAudioGetMasterVolume();
	extern int jsAudioSetMasterVolume(float vol);
	extern int jsAudioSuspendPlayer();
	extern int jsAudioResumePlayer(); 
	extern int jsAudioGetTotalTime(int soundID); 
	extern int jsAudioIsActiveChannel(int channel);
	extern int jsAudioCountUnreservedFreeChannels(); 
	extern int jsAudioCountTotalChannels();
	extern int jsAudioCountReservedChannels();
	extern int jsAudioReserveChannels(int number_of_reserve_channels);
	extern int jsFreeData(int soundID);
	extern int jsAudioFadeChannel(int which_channel, int fade_ticks, float to_volume);
	extern int jsAudioFindFreeChannel(int start_channel);
	extern bool jsAudioSetMaxVolumeChannel(int which_channel, float new_volume);
	extern float jsAudioGetMaxVolumeChannel(int which_channel);
	extern bool jsAudioSetMinVolumeChannel(int which_channel, float new_volume);
	extern float jsAudioGetMinVolumeChannel(int which_channel);
	extern int jsAudioStopWithDelay();
	extern int jsAudioSetPitch(int ch, float val);
}
#else
	int jsAudioInit() { return 0; };
	int jsAudioDecode(const char* file_path, const void* data, int size) { return 0; };
	int jsAudioGet(const char* file_path) { return 0; };
	int jsAudioPlay(int channel, int soundID, int loops, int ticks, const void* callback, int fade_ticks) { return 0; };
	int jsAudioPause(int channel) { return 0; };
	int jsAudioResume(int channel) { return 0; };
	int jsAudioStop(int channel) { return 0; };
	int jsAudioSeek(int channel, int number_of_milliseconds) { return 0; };
	int jsAudioRewind(int channel) { return 0; };
	int jsAudioIsPlaying(int channel) { return 0; };
	int jsAudioCountAllFreeChannels() { return 0; };
	float jsAudioGetVolume(int channel) { return 0; };
	int jsAudioSetVolume(int channel, float vol) { return 0; };
	float jsAudioGetMasterVolume() { return 0; };
	int jsAudioSetMasterVolume(float vol) { return 0; };
	int jsAudioSuspendPlayer() { return 0; };
	int jsAudioResumePlayer() { return 0; };
	int jsAudioGetTotalTime(int soundID) { return 0; };
	int jsAudioIsActiveChannel(int channel) { return 0; }
	int jsAudioCountUnreservedFreeChannels() { return 0; };
	int jsAudioCountTotalChannels() { return 0; };
	int jsAudioCountReservedChannels() { return 0; };
	int jsAudioReserveChannels(int number_of_reserve_channels) { return 0; };
	int jsFreeData(int soundID) { return 0; };
	int jsAudioFadeChannel(int which_channel, int fade_ticks, float to_volume) { return 0; };
	int jsAudioFindFreeChannel(int start_channel) { return 0; };
	bool jsAudioSetMaxVolumeChannel(int which_channel, float new_volume) { return 0; };
	float jsAudioGetMaxVolumeChannel(int which_channel) { return 0; };
	bool jsAudioSetMinVolumeChannel(int which_channel, float new_volume) { return 0; };
	float jsAudioGetMinVolumeChannel(int which_channel) { return 0; };
	int jsAudioStopWithDelay() { return 0; };
	int jsAudioSetPitch(int ch, float val) { return 0; }
#endif 

	// local event
	 
	SDLmixerSoundCompletionEvent::SDLmixerSoundCompletionEvent() :
		whichChannel(-1), // add 1 to convert from ALmixer starts arrays at 0 to Lua starts arrays at 1
		alSource(0),
		almixerData(NULL),
		finishedNaturally(false),
		platformALmixerPlaybackFinishedCallback(NULL)
	{
	}

	// Bug:5724. There seems to be a race condition bug with the callback system.
	// Our workaround is to take ownership of the platform notifier and delete when the event is destroyed.
	SDLmixerSoundCompletionEvent::~SDLmixerSoundCompletionEvent()
	{
	}

	SDLmixerSoundCompletionEvent::SDLmixerSoundCompletionEvent(int which_channel, unsigned int al_source, int almixer_data, bool finished_naturally, PlatformSDLmixerPlaybackFinishedCallback* callback_notifier) :
		whichChannel(which_channel + 1), // add 1 to convert from ALmixer starts arrays at 0 to Lua starts arrays at 1
		alSource(al_source),
		almixerData(almixer_data),
		finishedNaturally(finished_naturally),
		platformALmixerPlaybackFinishedCallback(callback_notifier)
	{
	}

	void SDLmixerSoundCompletionEvent::SetProperties(int which_channel, unsigned int al_source, int almixer_data, bool finished_naturally, PlatformSDLmixerPlaybackFinishedCallback* callback_notifier)
	{
		whichChannel = which_channel + 1; // add 1 to convert from ALmixer starts arrays at 0 to Lua starts arrays at 1
		alSource = al_source;
		almixerData = almixer_data;
		finishedNaturally = finished_naturally;
		platformALmixerPlaybackFinishedCallback = callback_notifier;
	}

	const char* 	SDLmixerSoundCompletionEvent::Name() const
	{
		return Self::kName;
	}

	int 	SDLmixerSoundCompletionEvent::Push(lua_State *L) const
	{
		if (Rtt_VERIFY(Super::Push(L)))
		{
			lua_pushinteger(L, whichChannel);
			lua_setfield(L, -2, "channel");

			lua_pushinteger(L, alSource);
			lua_setfield(L, -2, "source");

			lua_pushinteger(L, almixerData);
			lua_setfield(L, -2, "handle");

			lua_pushboolean(L, finishedNaturally);
			lua_setfield(L, -2, "completed");

			// Future phases might relate to "fade"  
			lua_pushstring(L, "stopped");  
			lua_setfield(L, -2, "phase");
		}
		return 1;
	}

	//
	//
	//

	PlatformSDLmixerPlaybackFinishedCallback::PlatformSDLmixerPlaybackFinishedCallback(const ResourceHandle<lua_State> & handle)
		: PlatformNotifier(handle)
	{
	}

	PlatformSDLmixerPlaybackFinishedCallback::~PlatformSDLmixerPlaybackFinishedCallback()
	{
	}

	//
	//	 PlatformWebAudioPlayer
	//

	PlatformWebAudioPlayer::PlatformWebAudioPlayer()
		: fInittedFlags(0)
		, fIsSuspended(false) 
	{
		jsAudioInit();
	}

	PlatformWebAudioPlayer::~PlatformWebAudioPlayer()
	{
	}

	void PlatformWebAudioPlayer::RuntimeWillTerminate(const Runtime& sender)
	{
		lua_State *L = sender.VMContext().L();
	}

	bool PlatformWebAudioPlayer::IsInitialized() const
	{
		return fInittedFlags != 0;
	}

	int PlatformWebAudioPlayer::LoadAll(const char* file_path)
	{
		int soundID = jsAudioGet(file_path);
		if (soundID >= 0)
		{
			return soundID;
		}

		FILE* fi = fopen(file_path, "rb");
		if (fi == NULL)
		{
			return -1;
		}

		fseek(fi, 0, SEEK_END); 
		int size = ftell(fi);
		fseek(fi, 0, SEEK_SET);
		void* audio = (char*) malloc(size);
		fread(audio, 1, size, fi);
		fclose(fi);

		soundID = jsAudioDecode(file_path, audio, size);

		free(audio);
		return soundID;
	} 

	int PlatformWebAudioPlayer::LoadStream(const char* file_path, unsigned int buffer_size, unsigned int max_queue_buffers, unsigned int number_of_startup_buffers, unsigned int number_of_buffers_to_queue_per_update_pass)
	{
		return LoadAll(file_path);
	}

	void PlatformWebAudioPlayer::FreeData(int id)
	{
		jsFreeData(id);
	}

	int PlatformWebAudioPlayer::PlayChannelTimed(int channel, int soundID, int loops, int ticks, PlatformSDLmixerPlaybackFinishedCallback *callback, int fade_ticks)
	{
		return jsAudioPlay(channel, soundID, loops, ticks, callback, fade_ticks);
	}

	int PlatformWebAudioPlayer::PauseChannel(int channel)
	{
		return jsAudioPause(channel);
	}

	int PlatformWebAudioPlayer::ResumeChannel(int channel)
	{
		return jsAudioResume(channel);
	}

	int PlatformWebAudioPlayer::StopChannel(int channel)
	{
		return jsAudioStop(channel);
	}

	int PlatformWebAudioPlayer::ReserveChannels(int number_of_reserve_channels)
	{
		return jsAudioReserveChannels(number_of_reserve_channels);
	}

	unsigned int PlatformWebAudioPlayer::GetSourceFromChannel(int which_channel) const
	{
		return which_channel;
	}

	int PlatformWebAudioPlayer::GetChannelFromSource(unsigned int al_source) const
	{
		return al_source;
	}

	int PlatformWebAudioPlayer::FindFreeChannel(int start_channel) const
	{
		return jsAudioFindFreeChannel(start_channel);
	}


	int PlatformWebAudioPlayer::GetTotalTime(int id) const
	{
		return jsAudioGetTotalTime(id);
	}

	bool PlatformWebAudioPlayer::RewindData(int SDLmixer_Data)
	{
		Rtt_Log("RewindData not implemented yet\n");
		return 0;
	}

	bool PlatformWebAudioPlayer::SeekData(int SDLmixer_Data, unsigned int number_of_milliseconds)
	{
		Rtt_Log("SeekData not implemented yet\n");
		return 0;
	}

	int PlatformWebAudioPlayer::SeekChannel(int channel, unsigned int number_of_milliseconds)
	{
		return jsAudioSeek(channel, number_of_milliseconds);
	}

	int PlatformWebAudioPlayer::RewindChannel(int channel)
	{
		return jsAudioRewind(channel);
	}


	int PlatformWebAudioPlayer::IsActiveChannel(int channel) const
	{
		return jsAudioIsActiveChannel(channel);
	}

	int PlatformWebAudioPlayer::IsPlayingChannel(int channel) const
	{
		return jsAudioIsPlaying(channel); 
	}

	int PlatformWebAudioPlayer::IsPausedChannel(int channel) const
	{
		return !IsPlayingChannel(channel);
	}

	unsigned int PlatformWebAudioPlayer::CountAllFreeChannels() const
	{
		return jsAudioCountAllFreeChannels();
	}

	unsigned int PlatformWebAudioPlayer::CountUnreservedFreeChannels() const
	{
		return jsAudioCountUnreservedFreeChannels();
	}

	unsigned int PlatformWebAudioPlayer::CountAllUsedChannels() const
	{
		Rtt_Log("CountAllUsedChannels not implemented yet\n");
		return 0;
	}

	unsigned int PlatformWebAudioPlayer::CountUnreservedUsedChannels() const
	{
		Rtt_Log("CountUnreservedUsedChannels not implemented yet\n");
		return 0;
	}

	unsigned int PlatformWebAudioPlayer::CountTotalChannels() const
	{
		return jsAudioCountTotalChannels();
	}

	unsigned int PlatformWebAudioPlayer::CountReservedChannels() const
	{
		return jsAudioCountReservedChannels();
	}


	int PlatformWebAudioPlayer::ExpireChannel(int which_channel, int number_of_milliseconds)
	{
		return jsAudioStopWithDelay();
	}


//	int PlatformWebAudioPlayer::FadeInChannelTimed(int channel, int snd, int loops, unsigned int fade_ticks, unsigned int expire_ticks, PlatformSDLmixerPlaybackFinishedCallback *callback)
//	{
		// hack, fade in does not work in emscripten
//		int ticks = static_cast<int>(expire_ticks);
//		return PlayChannelTimed(channel, snd, loops, ticks, callback, fade_ticks);
//	}

	int PlatformWebAudioPlayer::FadeOutChannel(int which_channel, unsigned int fade_ticks)
	{
		return jsAudioFadeChannel(which_channel, fade_ticks, 0);
	}

	int PlatformWebAudioPlayer::FadeChannel(int which_channel, unsigned int fade_ticks, float to_volume)
	{
		return jsAudioFadeChannel(which_channel, fade_ticks, to_volume);
	}

	// which_channel The channel to set the volume to or -1 to set on all channels.
	// volume The new volume to use. Valid values are 0.0 to 1.0.
	bool PlatformWebAudioPlayer::SetVolumeChannel(int channel, float vol)
	{
		vol = Clamp(vol, 0.f, 1.f);
		return jsAudioSetVolume(channel, vol);
	}

	float PlatformWebAudioPlayer::GetVolumeChannel(int channel) const
	{
		return jsAudioGetVolume(channel);
	}

	bool PlatformWebAudioPlayer::SetMaxVolumeChannel(int which_channel, float new_volume)
	{
		return jsAudioSetMaxVolumeChannel(which_channel, new_volume);
	}

	float PlatformWebAudioPlayer::GetMaxVolumeChannel(int which_channel) const
	{
		return jsAudioGetMaxVolumeChannel(which_channel);
	}

	bool PlatformWebAudioPlayer::SetMinVolumeChannel(int which_channel, float new_volume)
	{
		return jsAudioSetMinVolumeChannel(which_channel, new_volume);
	}

	float PlatformWebAudioPlayer::GetMinVolumeChannel(int which_channel) const
	{
		return jsAudioGetMinVolumeChannel(which_channel);
	}


	bool PlatformWebAudioPlayer::SetMasterVolume(float vol)
	{
		vol = Clamp(vol, 0.f, 1.f);
		return jsAudioSetMasterVolume(vol);
	}

	float PlatformWebAudioPlayer::GetMasterVolume() const
	{
		return jsAudioGetMasterVolume();
	}

	void PlatformWebAudioPlayer::NotificationCallback(int which_channel, PlatformSDLmixerPlaybackFinishedCallback *notifier, bool finished_naturally)
	{
		if (notifier)
		{
			// There's always a notifier attached, but we only dispatch if a listener is attached
			// NOTE: if we need to dispatch, we transfer ownership so that Lua-related data can
			// be freed on the Lua thread.
			if (notifier->HasListener())
			{
				lua_State *L = notifier->GetLuaState();
				if (L)
				{
					// Ensure that the event is dispatched on the main thread
					// TODO: ensure that ScheduleDispatch() is thread-safe.
					SDLmixerSoundCompletionEvent *e = Rtt_NEW(LuaContext::GetRuntime(L)->GetAllocator(), SDLmixerSoundCompletionEvent);

					// Bug 5724: In addition to setting all the callback properties, we pass the notifier in to transfer ownership.
					// We expect when the event is fired and deleted, it will also delete the notifier with it.
					e->SetProperties(which_channel, 0, 0, finished_naturally ? 1 : 0, notifier);
					notifier->ScheduleDispatch(e);
				}
			}
		}
	}

	void PlatformWebAudioPlayer::SuspendPlayer()
	{
		if (fIsSuspended == false)
		{
			jsAudioSuspendPlayer();
		}
		fIsSuspended = true;
	}

	void PlatformWebAudioPlayer::ResumePlayer()
	{
		if (fIsSuspended)
		{
			jsAudioResumePlayer();
		}
		fIsSuspended = false;
	}

	bool PlatformWebAudioPlayer::IsPlayerSuspended() const
	{
		return fIsSuspended;
	}

	void PlatformWebAudioPlayer::setProperty(int ch, const char* key, float val)
	{
		if (key && Rtt_StringCompare(key, "PITCH") == 0)
		{
			jsAudioSetPitch(ch, val);
		}
	}

} // namespace Rtt


#endif // Rtt_EMSCRIPTEN_ENV

 