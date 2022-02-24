//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformSDLAudioPlayer_H__
#define _Rtt_PlatformSDLAudioPlayer_H__

#include "Core/Rtt_Config.h"

#ifdef Rtt_EMSCRIPTEN_ENV

//#include <SDL2/SDL_mixer.h>
#include "Rtt_PlatformNotifier.h"
#include "Rtt_EmscriptenContainer.h"

struct Mix_Chunk;

namespace Rtt
{
	class PlatformSDLmixerPlaybackFinishedCallback : public PlatformNotifier
	{
	public:
		PlatformSDLmixerPlaybackFinishedCallback(const ResourceHandle<lua_State> & handle);
		virtual ~PlatformSDLmixerPlaybackFinishedCallback();


	protected:
		friend class PlatformOpenALPlayerCallListenerTask;
	};

	//
	//	 mixer data
	//
	struct sdlSound : public ref_counted
	{
		sdlSound(Mix_Chunk* snd);
		virtual ~sdlSound();

		Mix_Chunk* fChunk;
	};

	//
	//	 channel
	//
	struct sdlChannel : public ref_counted
	{
		sdlChannel()
			: fCallback(NULL)
			, fVolume(1)
			, fLoops(-1)
			, fTicks(-1)
		{
		}
		void setCallback(PlatformSDLmixerPlaybackFinishedCallback* func);

		PlatformSDLmixerPlaybackFinishedCallback* fCallback;
		float fVolume;
		smart_ptr<sdlSound> fSound;
		int fLoops;
		int fTicks;
	};


	//
	// Local event
	//

	class SDLmixerSoundCompletionEvent : public VirtualEvent
	{
	public:
		typedef VirtualEvent Super;
		typedef SDLmixerSoundCompletionEvent Self;

		// Used when error occurs
		SDLmixerSoundCompletionEvent();

		// We use doubles here b/c lua_Numbers are double and the source data was typically double
		SDLmixerSoundCompletionEvent(
			int which_channel,
			unsigned int al_source,
			int almixer_data,
			bool finished_naturally,
			PlatformSDLmixerPlaybackFinishedCallback* callback_notifier
			);

		virtual ~SDLmixerSoundCompletionEvent();


		void SetProperties(
			int which_channel,
			unsigned int al_source,
			int almixer_data,
			bool finished_naturally,
			PlatformSDLmixerPlaybackFinishedCallback* callback_notifier
			);

	public:
		static const char kName[];
		virtual const char* Name() const;
		virtual int Push(lua_State *L) const;

	private:
		// We use doubles here b/c lua_Numbers are double and the source data was typically double
		int whichChannel;
		unsigned int alSource;
		int almixerData;
		bool finishedNaturally;
		PlatformSDLmixerPlaybackFinishedCallback* platformALmixerPlaybackFinishedCallback;
	};

	//
	//
	//

	// PlatformSDLAudioPlayer player is a singleton for convenience.
	class PlatformSDLAudioPlayer : public ref_counted
	{
	public:
		PlatformSDLAudioPlayer();
		virtual ~PlatformSDLAudioPlayer();

		bool IsInitialized() const;

		int LoadAll(const char* file_path);
		int LoadStream(const char* file_path, unsigned int buffer_size, unsigned int max_queue_buffers, unsigned int number_of_startup_buffers, unsigned int number_of_buffers_to_queue_per_update_pass);
		void FreeData(int SDLmixer_Data);

		int PlayChannelTimed(int channel, int SDLmixer_Data, int loops, int ticks, PlatformSDLmixerPlaybackFinishedCallback *callback);
		int PauseChannel(int channel);
		int ResumeChannel(int channel);
		int StopChannel(int channel);

		int ReserveChannels(int number_of_reserve_channels);
		unsigned int GetSourceFromChannel(int which_channel) const;
		int GetChannelFromSource(unsigned int al_source) const;
		virtual int FindFreeChannel(int start_channel) const;

		int GetTotalTime(int SDLmixer_Data) const;
		bool RewindData(int SDLmixer_Data);
		bool SeekData(int SDLmixer_Data, unsigned int number_of_milliseconds);
		int SeekChannel(int which_channel, unsigned int number_of_milliseconds);
		int RewindChannel(int which_channel);

		int IsActiveChannel(int which_channel) const;
		int IsActiveSource(unsigned int al_source) const;
		int IsPlayingChannel(int which_channel) const;
		int IsPlayingSource(unsigned int al_source) const;
		int IsPausedChannel(int which_channel) const;
		int IsPausedSource(unsigned int al_source) const;

		unsigned int CountAllFreeChannels() const;
		unsigned int CountUnreservedFreeChannels() const;
		unsigned int CountAllUsedChannels() const;
		unsigned int CountUnreservedUsedChannels() const;
		unsigned int CountTotalChannels() const;
		unsigned int CountReservedChannels() const;

		int ExpireChannel(int which_channel, int number_of_milliseconds);

		int FadeInChannelTimed(int which_channel, int SDLmixer_Data, int number_of_loops, unsigned int fade_ticks, unsigned int expire_ticks, PlatformSDLmixerPlaybackFinishedCallback *callback);
		int FadeOutChannel(int which_channel, unsigned int fade_ticks);
		int FadeChannel(int which_channel, unsigned int fade_ticks, float to_volume);
		bool SetVolumeChannel(int which_channel, float new_volume);
		float GetVolumeChannel(int which_channel) const;
		bool SetMaxVolumeChannel(int which_channel, float new_volume);
		float GetMaxVolumeChannel(int which_channel) const;
		bool SetMinVolumeChannel(int which_channel, float new_volume);
		float GetMinVolumeChannel(int which_channel) const;

		bool SetMasterVolume(float new_volume);
		float GetMasterVolume() const;

		// This is for suspending the OpenAL player. Though the current implementation calls Interruption, this is an implementation detail that could change.
		// Call this for when you want to suspend the OpenAL player, such as wanting to background/suspend.
		void SuspendPlayer();
		void ResumePlayer();
		bool IsPlayerSuspended() const;

		// These should only be called for a real Interruption event (as defined by Apple's definition of an audio interruption on iOS).
		// For suspending the OpenAL player, call the SuspendPlayer API.
		void BeginInterruption();
		void EndInterruption();
		bool IsInInterruption() const;
		void NotificationCallback(int which_channel, unsigned int al_source, int data, bool finished_naturally);
		void RuntimeWillTerminate(const Runtime& sender);

		int fInittedFlags;
		//		bool isSuspended;

	private:

		static const unsigned int kSDLPlayerMaxNumberOfSources = 32;
		array< smart_ptr<sdlChannel> > fChannels;		// playing sounds
		array< smart_ptr<sdlSound> > fSounds;		// loaded sounds
		float fMasterVolume;
		int fReservedChannels;
		bool fIsSuspended;
	};

	PlatformSDLAudioPlayer* getSDLAudio();

} // namespace Rtt

#endif // Rtt_USE_ALMIXER

#endif // _Rtt_PlatformSDLAudioPlayer_H__
