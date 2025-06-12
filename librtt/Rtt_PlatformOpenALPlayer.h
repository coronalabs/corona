//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformOpenALPlayer_H__
#define _Rtt_PlatformOpenALPlayer_H__

#include "Core/Rtt_Config.h"

#ifdef Rtt_USE_ALMIXER

#include "Core/Rtt_Types.h"
#include "Core/Rtt_ResourceHandle.h"
#include "Rtt_Scheduler.h"
#include "Rtt_PlatformNotifier.h"
#include "Rtt_Event.h"

#include <atomic>

// ----------------------------------------------------------------------------

struct lua_State;
struct ALmixer_Data;
struct LuaHashMap;

namespace Rtt
{

class PlatformALmixerPlaybackFinishedCallback;
class PlatformNotifier;

// Local event
class ALmixerSoundCompletionEvent : public VirtualEvent
{
	public:
		typedef VirtualEvent Super;
		typedef ALmixerSoundCompletionEvent Self;
		
	public:
		// Used when error occurs
		ALmixerSoundCompletionEvent();

		// We use doubles here b/c lua_Numbers are double and the source data was typically double
		ALmixerSoundCompletionEvent(
			int which_channel,
			unsigned int al_source,
			ALmixer_Data* almixer_data,
			bool finished_naturally/*,
			PlatformALmixerPlaybackFinishedCallback* callback_notifier*/
		);
	
		virtual ~ALmixerSoundCompletionEvent();


		void SetProperties(
			int which_channel,
			unsigned int al_source,
			ALmixer_Data* almixer_data,
			bool finished_naturally/*,
			PlatformALmixerPlaybackFinishedCallback* callback_notifier*/
		);	
	public:
		static const char kName[];
		virtual const char* Name() const;
		virtual int Push( lua_State *L ) const;

	private:
		// We use doubles here b/c lua_Numbers are double and the source data was typically double
		int whichChannel;
		unsigned int alSource;
		ALmixer_Data* almixerData;
		bool finishedNaturally;
		//PlatformALmixerPlaybackFinishedCallback* platformALmixerPlaybackFinishedCallback;
};


// ----------------------------------------------------------------------------

class PlatformALmixerPlaybackFinishedCallback : public PlatformNotifier
{
	public:
		PlatformALmixerPlaybackFinishedCallback( const ResourceHandle<lua_State> & handle );
		virtual ~PlatformALmixerPlaybackFinishedCallback();
		
		
	protected:
		friend class PlatformOpenALPlayerCallListenerTask;
};

// OpenAL player is a singleton for convenience.
class PlatformOpenALPlayer
{
	public:
		virtual ~PlatformOpenALPlayer();
	
		static PlatformOpenALPlayer* RetainInstance();
		static void ReleaseInstance();
		static PlatformOpenALPlayer* SharedInstance();
		static void SetFrequency( unsigned int play_frequency );
		// Currently not implemented or supported. Reserved for future use.
		static void SetMaxSources( unsigned int max_sources );

		bool IsInitialized() const;
		void RuntimeWillTerminate( const Runtime& sender );

	protected:
		// protected because this is a singleton
		explicit PlatformOpenALPlayer();

	private:
		void InitializeCallbacks();

	protected:
		// Initialization function helper
		bool InitializeOpenALPlayer();
		// I need to cheat: I need a const version of this function even though it really isn't const
		bool InitializeOpenALPlayer() const;
		void QuitOpenALPlayer();

	public:
		void AttachNotifier( PlatformNotifier* pnotifier );

		void SetChannelCallback( lua_State* L, int channel, int userCallback );

	public:
		virtual ALmixer_Data* LoadAll( const char* file_path );
		virtual ALmixer_Data* LoadStream( const char* file_path,  unsigned int buffer_size, unsigned int max_queue_buffers, unsigned int number_of_startup_buffers, unsigned int number_of_buffers_to_queue_per_update_pass );
		virtual void FreeData( ALmixer_Data* almixer_data );

		virtual int PlayChannelTimed( int channel, ALmixer_Data* almixer_data, int loops, int ticks );//, PlatformALmixerPlaybackFinishedCallback *callback );
		virtual int PauseChannel( int channel );
		virtual int ResumeChannel( int channel );
		virtual int StopChannel( int channel );

		virtual int ReserveChannels( int number_of_reserve_channels );
		virtual unsigned int GetSourceFromChannel( int which_channel ) const;
		virtual int GetChannelFromSource( unsigned int al_source ) const;
		virtual int FindFreeChannel( int start_channel ) const;
		
		virtual int GetTotalTime( ALmixer_Data* almixer_data ) const;
		virtual bool RewindData( ALmixer_Data* almixer_data );
		virtual bool SeekData( ALmixer_Data* almixer_data, unsigned int number_of_milliseconds );
		virtual int SeekChannel( int which_channel, unsigned int number_of_milliseconds );
		virtual int RewindChannel( int which_channel );

		virtual int IsActiveChannel( int which_channel ) const;
		virtual int IsActiveSource( unsigned int al_source ) const;
		virtual int IsPlayingChannel( int which_channel ) const;
		virtual int IsPlayingSource( unsigned int al_source ) const;
		virtual int IsPausedChannel( int which_channel ) const;
		virtual int IsPausedSource( unsigned int al_source ) const;

		virtual unsigned int CountAllFreeChannels() const;
		virtual unsigned int CountUnreservedFreeChannels() const;
		virtual unsigned int CountAllUsedChannels() const;
		virtual unsigned int CountUnreservedUsedChannels() const;
		virtual unsigned int CountTotalChannels() const;
		virtual unsigned int CountReservedChannels() const;

		virtual int ExpireChannel( int which_channel, int number_of_milliseconds );

		virtual int FadeInChannelTimed( int which_channel, ALmixer_Data* almixer_data, int number_of_loops, unsigned int fade_ticks, unsigned int expire_ticks );//, PlatformALmixerPlaybackFinishedCallback *callback );
		virtual int FadeOutChannel( int which_channel, unsigned int fade_ticks );
		virtual int FadeChannel( int which_channel, unsigned int fade_ticks, float to_volume );
		virtual bool SetVolumeChannel( int which_channel, float new_volume );
		virtual float GetVolumeChannel( int which_channel ) const;
		virtual bool SetMaxVolumeChannel( int which_channel, float new_volume );
		virtual float GetMaxVolumeChannel( int which_channel ) const;
		virtual bool SetMinVolumeChannel( int which_channel, float new_volume );
		virtual float GetMinVolumeChannel( int which_channel ) const;

		virtual bool SetMasterVolume( float new_volume );
		virtual float GetMasterVolume() const;

		// This is for suspending the OpenAL player. Though the current implementation calls Interruption, this is an implementation detail that could change.
		// Call this for when you want to suspend the OpenAL player, such as wanting to background/suspend.
		virtual void SuspendPlayer();
		virtual void ResumePlayer();
		virtual bool IsPlayerSuspended() const;

		// These should only be called for a real Interruption event (as defined by Apple's definition of an audio interruption on iOS).
		// For suspending the OpenAL player, call the SuspendPlayer API.
		virtual void BeginInterruption();
		virtual void EndInterruption();
		virtual bool IsInInterruption() const;
	
	protected:
		// Helper functions to save/restore playing state for interruptions
		void PauseChannelsForInterruption();
		void ResumeChannelsForInterruption();
	
	public:	
		virtual void NotificationCallback( int which_channel, unsigned int al_source, ALmixer_Data* almixer_data, bool finished_naturally);

	protected:
		friend class PlatformOpenALPlayerCallListenerTask;
		
		static const unsigned int kOpenALPlayerMaxNumberOfSources = 32;
		int/*PlatformALmixerPlaybackFinishedCallback* */arrayOfChannelToLuaCallbacks[kOpenALPlayerMaxNumberOfSources];
		//const ResourceHandle<lua_State>& resourceHandle;
		bool isInitialized;
		bool isSuspended;
		LuaHashMap* mapOfLoadedFileNamesToData;
		LuaHashMap* mapOfLoadedDataToFileNames;
		LuaHashMap* mapOfLoadedDataToReferenceCountNumber;
		// Currently using true/false to denote was playing, but might want to change to hold all OpenAL states
		int arrayOfChannelStatesForInterruption[kOpenALPlayerMaxNumberOfSources];

	
	
		// Workaround for iOS 4/5 bug: A user leaving an app via notification center triggers a unnecessary resume/suspend event in Cocoa.
		// This event forces us to re-initialize the audio system on resume which seems to lead to breaking audio completely.
		// My theory is that Core Audio is trying to shutdown and reactivating OpenAL messes things up.
		// It turns out setting the Audio Session to active seems to always fail here so we might be able to use that as a workaround clue.
		// The problem is that setting the audio session to the currently set state (e.g. on when already on, off when already off) also triggers an error.
		// And since there is no GetActive() API, we can't know the state since Apple may change it for us on audio interruptions or 3rd party API calls may change the state behind our back. So relying on this flag too much will also cause problems. But in a single isolated case of an EndInterruption, this might just work.
		// iOS 6 seems to work with or without this workaround. I'm indecisive on whether to use this on iOS 6 or not because Apple does not really define clearly why AudioSessionSetActive would fail. For simplicity, I'm thinking we will keep the code base the same, but this API is available so we can toggle it as needed (e.g. in AppDelegate after testing for iOS version).
		/*
		 radar://12022630 Core Audio breaks when leaving an app via Notification Center and then returning
		 radar://12022774 Redundant WillResignActive/BecomeActive when leaving app via Notification Center
		 radar://12022812 AudioSessionSetActive should not return errors if setting to false when already false
		 radar://12022855 Need API: AudioSessionGetActive() to balance AudioSessionSetActive
		 radar://12022872 Want Audio Session Interruptions to trigger for all events that require it
		 radar://12022894 Want automatic handling for OpenAL/Core Audio interruptions like AVFoundation
		 */
	public:
		virtual void SetUseAudioSessionInitializationFailureToAbortEndInterruption(bool enabled);
		virtual bool GetUseAudioSessionInitializationFailureToAbortEndInterruption() const;
	
	protected:
		int SwapChannelCallback( int channel, int newRef );

	protected:
		bool useAudioSessionInitializationFailureToAbortEndInterruption;

		PlatformNotifier* notifier;
		std::atomic_flag channelBusy[kOpenALPlayerMaxNumberOfSources];

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_USE_ALMIXER


#endif // _Rtt_PlatformOpenALPlayer_H__
