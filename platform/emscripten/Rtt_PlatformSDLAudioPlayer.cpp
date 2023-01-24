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

#include "Rtt_PlatformSDLAudioPlayer.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"
#include "Rtt_PlatformAudioSessionManager.h"
#include <SDL2/SDL_mixer.h>

#ifdef WIN32
	// this fixes #define remove from luaconf.h
	#undef remove
#endif

namespace Rtt
{

	const char SDLmixerSoundCompletionEvent::kName[] = "audio";

	// music finished function
	extern PlatformSDLAudioPlayer sdlAudio;

	// chunk finished function
	static void chunkFinished(int channel)
	{
		sdlAudio.NotificationCallback(channel, 0, 0, 0);
	}

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

	// Warning: ALmixer callbacks may occur on a background thread.
	// Rule: Never call ALmixer functions from within a callback background thread.
	// Generally it is a better idea to schedule the Lua callback to happen on the main thread.
	//	static void Internal_SoundFinishedCallback(ALint which_channel, ALuint al_source, ALmixer_Data* almixer_data, ALboolean finished_naturally, void* user_data)
	//	{
	//		Rtt::PlatformOpenALPlayer* openal_player = (Rtt::PlatformOpenALPlayer*)user_data;
	//		openal_player->NotificationCallback((int)which_channel, (unsigned int)al_source, almixer_data, (bool)finished_naturally);
	//	}



	//
	//	 PlatformSDLAudioPlayer
	//

	sdlSound::sdlSound(Mix_Chunk* snd) : 	fChunk(snd) 	{}
	sdlSound::~sdlSound()	{	Mix_FreeChunk(fChunk); }

	PlatformSDLAudioPlayer::PlatformSDLAudioPlayer()
		: fInittedFlags(0)
		, fMasterVolume(1.0f)		// multiplier
		, fReservedChannels(0)
		, fIsSuspended(false)
	{
		fInittedFlags = Mix_Init(MIX_INIT_OGG | MIX_INIT_MOD | MIX_INIT_FLAC | MIX_INIT_MP3);
		if (fInittedFlags == 0)
		{
			printf("Mix_Init: Failed to init, %s\n", Mix_GetError());
			return;
		}
		printf("SDL_Mixer init flags: %x\n", fInittedFlags);

		//Initialize SDL_mixer 
		if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
		{
			printf("Mix_OpenAudio: %s\n", Mix_GetError());
			Mix_Quit();
			return;
		}

		SDL_version compiled;
		SDL_MIXER_VERSION(&compiled);
		printf("SDL_mixer compiled version %d.%d.%d\n", compiled.major, compiled.minor, compiled.patch);
		//		const SDL_version* linked = Mix_Linked_Version();
		//		printf("linked version %d.%d.%d\n", linked->major, linked->minor, linked->patch);

		// Currently we support 32 channels. You may not play new audio on a channel that is currently active (paused or playing).
		int n = Mix_AllocateChannels(kSDLPlayerMaxNumberOfSources);
		Rtt_ASSERT(n == kSDLPlayerMaxNumberOfSources);

		fChannels.resize(kSDLPlayerMaxNumberOfSources);
		for (int i = 0; i < fChannels.size(); i++)
		{
			fChannels[i] = new sdlChannel();
		}

	}

	PlatformSDLAudioPlayer::~PlatformSDLAudioPlayer()
	{
		Mix_CloseAudio();
	}

	void PlatformSDLAudioPlayer::RuntimeWillTerminate(const Runtime& sender)
	{
		lua_State *L = sender.VMContext().L();
	}

	bool PlatformSDLAudioPlayer::IsInitialized() const
	{
		return fInittedFlags != 0;
	}

	int PlatformSDLAudioPlayer::LoadAll(const char* file_path)
	{
		SDL_ClearError();
		Mix_Chunk* chunk = Mix_LoadWAV(file_path);
		if (chunk)
		{
			fSounds.push_back(new sdlSound(chunk));
			return fSounds.size() - 1;
		}
		else
		{
			Rtt_LogException("SDL_Mixer: %s\n", Mix_GetError());
		}
		return -1;
	}

	int PlatformSDLAudioPlayer::LoadStream(const char* file_path, unsigned int buffer_size, unsigned int max_queue_buffers, unsigned int number_of_startup_buffers, unsigned int number_of_buffers_to_queue_per_update_pass)
	{
		if (0 == buffer_size)
		{
			//buffer_size = ALMIXER_DEFAULT_BUFFERSIZE;
		}
		if (0 == max_queue_buffers)
		{
			//max_queue_buffers = ALMIXER_DEFAULT_QUEUE_BUFFERS;
		}
		if (0 == number_of_startup_buffers)
		{
			//number_of_startup_buffers = ALMIXER_DEFAULT_STARTUP_BUFFERS;
		}
		if (0 == number_of_buffers_to_queue_per_update_pass)
		{
			//	number_of_buffers_to_queue_per_update_pass = ALMIXER_DEFAULT_BUFFERS_TO_QUEUE_PER_UPDATE_PASS;
		}

		return LoadAll(file_path);
	}

	void PlatformSDLAudioPlayer::FreeData(int id)
	{
		if (id >= 0 && id < fSounds.size())
		{
			fSounds.remove(id);
		}
	}

	int PlatformSDLAudioPlayer::PlayChannelTimed(int channel, int sndID, int loops, int ticks, PlatformSDLmixerPlaybackFinishedCallback *callback)
	{
		if (channel == -1)
		{
			// find available one
			for (int i = 0; i < fChannels.size(); i++)
			{
				if (fChannels[i]->fSound == NULL)
				{
					channel = i;
					break;
				}
			}
		}

		if (channel < 0 || channel >= fChannels.size())
		{
			return -1;
		}

		if (sndID >= 0 && sndID < fSounds.size() && fSounds[sndID] != NULL)
		{
			sdlChannel* ch = fChannels[channel];
			ch->fSound = fSounds[sndID];
			ch->fTicks = ticks;
			ch->fLoops = loops;
			ch->fCallback = callback;
			Mix_ChannelFinished(chunkFinished);
			Mix_PlayChannelTimed(channel, fSounds[sndID]->fChunk, loops, ticks);
			Mix_Volume(channel, ch->fVolume * fMasterVolume * 128);

			return channel;
		}
		return -1;
	}

	int PlatformSDLAudioPlayer::PauseChannel(int channel)
	{
		Mix_Pause(channel);
		return true;
	}

	int PlatformSDLAudioPlayer::ResumeChannel(int channel)
	{
		Mix_Resume(channel);
		return true;
	}

	int PlatformSDLAudioPlayer::StopChannel(int channel)
	{
		return Mix_HaltChannel(channel);
	}

	int PlatformSDLAudioPlayer::ReserveChannels(int number_of_reserve_channels)
	{
		fReservedChannels = number_of_reserve_channels;
		return 0;
	}

	unsigned int PlatformSDLAudioPlayer::GetSourceFromChannel(int which_channel) const
	{
		return which_channel;
	}

	int PlatformSDLAudioPlayer::GetChannelFromSource(unsigned int al_source) const
	{
		Rtt_Log("GetChannelFromSource not implemented yet\n");
		return 0;
	}

	int PlatformSDLAudioPlayer::FindFreeChannel(int start_channel) const
	{
		start_channel = start_channel < 0 ? 0 : start_channel;
		for (int i = start_channel; i < fChannels.size(); i++)
		{
			if (fChannels[i]->fSound == NULL)
			{
				return i;
			}
		}
		return 0;
	}


	int PlatformSDLAudioPlayer::GetTotalTime(int id) const
	{
		Rtt_Log("GetTotalTime not implemented yet\n");
		return 0;
	}

	bool PlatformSDLAudioPlayer::RewindData(int SDLmixer_Data)
	{
		Rtt_Log("RewindData not implemented yet\n");
		return 0;
	}

	bool PlatformSDLAudioPlayer::SeekData(int SDLmixer_Data, unsigned int number_of_milliseconds)
	{
		Rtt_Log("SeekData not implemented yet\n");
		return 0;
	}

	int PlatformSDLAudioPlayer::SeekChannel(int which_channel, unsigned int number_of_milliseconds)
	{
		Rtt_Log("SeekChannel not implemented yet\n");
		return 0;
	}

	int PlatformSDLAudioPlayer::RewindChannel(int channel)
	{
		sdlChannel* snd = fChannels[channel];
		if (snd->fSound)
		{
			// restart
			Mix_ChannelFinished(NULL);
			Mix_HaltChannel(channel);

			Mix_ChannelFinished(chunkFinished);
			Mix_PlayChannelTimed(channel, snd->fSound->fChunk, snd->fLoops, snd->fTicks);
		}
		return true;
	}


	int PlatformSDLAudioPlayer::IsActiveChannel(int which_channel) const
	{
		Rtt_Log("IsActiveChannel not implemented yet\n");
		return 0;
	}

	int PlatformSDLAudioPlayer::IsActiveSource(unsigned int al_source) const
	{
		Rtt_Log("IsActiveSource not implemented yet\n");
		return 0;
	}

	int PlatformSDLAudioPlayer::IsPlayingChannel(int channel) const
	{
		return Mix_Playing(channel);
	}

	int PlatformSDLAudioPlayer::IsPlayingSource(unsigned int al_source) const
	{
		Rtt_Log("IsPlayingSource not implemented yet\n");
		return 0;
	}

	int PlatformSDLAudioPlayer::IsPausedChannel(int channel) const
	{
		return Mix_Paused(channel);
	}

	int PlatformSDLAudioPlayer::IsPausedSource(unsigned int al_source) const
	{
		Rtt_Log("IsPausedSource not implemented yet\n");
		return 0;
	}


	unsigned int PlatformSDLAudioPlayer::CountAllFreeChannels() const
	{
		int k = 0;
		for (int i = 0; i < fChannels.size(); i++)
		{
			if (fChannels[i]->fSound == NULL)
			{
				k++;
			}
		}
		return k;
	}

	unsigned int PlatformSDLAudioPlayer::CountUnreservedFreeChannels() const
	{
		Rtt_Log("CountUnreservedFreeChannels not implemented yet\n");
		return 0;
	}

	unsigned int PlatformSDLAudioPlayer::CountAllUsedChannels() const
	{
		Rtt_Log("CountAllUsedChannels not implemented yet\n");
		return 0;
	}

	unsigned int PlatformSDLAudioPlayer::CountUnreservedUsedChannels() const
	{
		Rtt_Log("CountUnreservedUsedChannels not implemented yet\n");
		return 0;
	}

	unsigned int PlatformSDLAudioPlayer::CountTotalChannels() const
	{
		return kSDLPlayerMaxNumberOfSources;
	}

	unsigned int PlatformSDLAudioPlayer::CountReservedChannels() const
	{
		return fReservedChannels;
	}


	int PlatformSDLAudioPlayer::ExpireChannel(int which_channel, int number_of_milliseconds)
	{
		Rtt_Log("ExpireChannel not implemented yet\n");
		return 0;
	}


	int PlatformSDLAudioPlayer::FadeInChannelTimed(int channel, int snd, int loops, unsigned int fade_ticks, unsigned int expire_ticks, PlatformSDLmixerPlaybackFinishedCallback *callback)
	{
		// hack, fade in does not work in emscripten
		int ticks = static_cast<int>(expire_ticks);
		return PlayChannelTimed(channel, snd, loops, ticks, callback);
	}

	int PlatformSDLAudioPlayer::FadeOutChannel(int which_channel, unsigned int fade_ticks)
	{
		Rtt_Log("FadeOutChannel not implemented yet\n");
		return 0;
	}

	int PlatformSDLAudioPlayer::FadeChannel(int which_channel, unsigned int fade_ticks, float to_volume)
	{
		Rtt_Log("FadeChannel not implemented yet\n");
		return 0;
	}

	// which_channel The channel to set the volume to or -1 to set on all channels.
	// volume The new volume to use. Valid values are 0.0 to 1.0.
	bool PlatformSDLAudioPlayer::SetVolumeChannel(int which_channel, float vol)
	{
		vol = Clamp(vol, 0.f, 1.f);
		if (which_channel == -1)
		{
			for (int i = 0; i < fChannels.size(); i++)
			{
				sdlChannel* snd = fChannels[i];
				snd->fVolume = vol;
				if (snd->fSound)
				{
					Mix_Volume(i, vol * fMasterVolume * 128);
				}
			}
			return true;
		}
		else
		{
			sdlChannel* snd = fChannels[which_channel];
			snd->fVolume = vol;
			if (snd->fSound)
			{
				Mix_Volume(which_channel, vol * fMasterVolume * 128);
				return true;
			}
		}
		return false;
	}

	float PlatformSDLAudioPlayer::GetVolumeChannel(int which_channel) const
	{
		sdlChannel* snd = fChannels[which_channel];
		return snd->fVolume;
	}

	bool PlatformSDLAudioPlayer::SetMaxVolumeChannel(int which_channel, float new_volume)
	{
		Rtt_Log("SetMaxVolumeChannel not implemented yet\n");
		return 0;
	}

	float PlatformSDLAudioPlayer::GetMaxVolumeChannel(int which_channel) const
	{
		Rtt_Log("GetMaxVolumeChannel not implemented yet\n");
		return 0;
	}

	bool PlatformSDLAudioPlayer::SetMinVolumeChannel(int which_channel, float new_volume)
	{
		Rtt_Log("SetMinVolumeChannel not implemented yet\n");
		return 0;
	}

	float PlatformSDLAudioPlayer::GetMinVolumeChannel(int which_channel) const
	{
		Rtt_Log("GetMinVolumeChannel not implemented yet\n");
		return 0;
	}


	bool PlatformSDLAudioPlayer::SetMasterVolume(float new_volume)
	{
		fMasterVolume = Clamp(new_volume, 0.0f, 1.0f);
		for (int i = 0; i < fChannels.size(); i++)
		{
			sdlChannel* snd = fChannels[i];
			if (snd->fSound)
			{
				Mix_Volume(i, snd->fVolume * fMasterVolume * 128);
			}
		}
		return true;
	}

	float PlatformSDLAudioPlayer::GetMasterVolume() const
	{
		return fMasterVolume;
	}

	void PlatformSDLAudioPlayer::NotificationCallback(int which_channel, unsigned int al_source, int data, bool finished_naturally)
	{
		Rtt_ASSERT(which_channel >= 0 && which_channel < fChannels.size());
		sdlChannel* snd = fChannels[which_channel];

		PlatformSDLmixerPlaybackFinishedCallback *notifier = snd->fCallback;
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
					e->SetProperties(which_channel, al_source, data, finished_naturally, notifier);
					notifier->ScheduleDispatch(e);
				}
			}
		}

		// stopped
		snd->fSound = NULL;
	}

	void PlatformSDLAudioPlayer::SuspendPlayer()
	{
		if (fIsSuspended == false)
		{
			Mix_Pause(-1);
		}
		fIsSuspended = true;
	}

	void PlatformSDLAudioPlayer::ResumePlayer()
	{
		if (fIsSuspended)
		{
			Mix_Resume(-1);
		}
		fIsSuspended = false;
	}

	bool PlatformSDLAudioPlayer::IsPlayerSuspended() const
	{
		return fIsSuspended;
	}

} // namespace Rtt


#endif // Rtt_EMSCRIPTEN_ENV

