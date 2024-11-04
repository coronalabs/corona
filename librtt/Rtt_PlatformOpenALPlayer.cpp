//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#ifdef Rtt_USE_ALMIXER

#ifdef Rtt_ANDROID_ENV
	#define USE_STATIC_MIXER
#endif

#include "Rtt_PlatformOpenALPlayer.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"
#include "ALmixer.h"
#include "LuaHashMap.h"
#include "Rtt_PlatformAudioSessionManager.h"
#ifdef Rtt_ANDROID_ENV
	#include "NativeToJavaBridge.h"
#endif

static unsigned int s_playerFrequency = 0;

// Not supported yet because I need to change the callback array to be dynamic
// static unsigned int s_maxSources = kOpenALPlayerMaxNumberOfSources;

#ifdef __APPLE__
#include <unistd.h>
#include <TargetConditionals.h>
	#if (TARGET_OS_IOS == 1)
	#include "Rtt_IPhoneAudioSessionManager.h"
	#endif // #if (TARGET_OS_IOS == 1)
#endif /* __APPLE__ */

// Warning: ALmixer callbacks may occur on a background thread.
// Rule: Never call ALmixer functions from within a callback background thread.
// Generally it is a better idea to schedule the Lua callback to happen on the main thread.
static void Internal_SoundFinishedCallback(ALint which_channel, ALuint al_source, ALmixer_Data* almixer_data, ALboolean finished_naturally, void* user_data)
{
	Rtt::PlatformOpenALPlayer* openal_player = (Rtt::PlatformOpenALPlayer*)user_data;
	openal_player->NotificationCallback((int)which_channel, (unsigned int)al_source, almixer_data, (bool)finished_naturally);
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------


const char ALmixerSoundCompletionEvent::kName[] = "audio";

// local event

ALmixerSoundCompletionEvent::ALmixerSoundCompletionEvent()
	:
	whichChannel( -1 ), // add 1 to convert from ALmixer starts arrays at 0 to Lua starts arrays at 1
	alSource( 0 ),
	almixerData( NULL ),
	finishedNaturally( false )/*,
	platformALmixerPlaybackFinishedCallback( NULL )*/
{		
}

// Bug:5724. There seems to be a race condition bug with the callback system.
// Our workaround is to take ownership of the platform notifier and delete when the event is destroyed.
ALmixerSoundCompletionEvent::~ALmixerSoundCompletionEvent()
{
//	Rtt_DELETE( platformALmixerPlaybackFinishedCallback );
}

ALmixerSoundCompletionEvent::ALmixerSoundCompletionEvent(int which_channel, unsigned int al_source, ALmixer_Data* almixer_data, bool finished_naturally )//, PlatformALmixerPlaybackFinishedCallback* callback_notifier)
	:	
	whichChannel( which_channel + 1 ), // add 1 to convert from ALmixer starts arrays at 0 to Lua starts arrays at 1
	alSource( al_source ),
	almixerData( almixer_data ),
	finishedNaturally( finished_naturally )/*,
	platformALmixerPlaybackFinishedCallback( callback_notifier )*/
{

}

void ALmixerSoundCompletionEvent::SetProperties(int which_channel, unsigned int al_source, ALmixer_Data* almixer_data, bool finished_naturally )//, PlatformALmixerPlaybackFinishedCallback* callback_notifier)
{
	whichChannel = which_channel + 1; // add 1 to convert from ALmixer starts arrays at 0 to Lua starts arrays at 1
	alSource = al_source;
	almixerData = almixer_data;
	finishedNaturally = finished_naturally;
//	platformALmixerPlaybackFinishedCallback = callback_notifier;
}

const char*
ALmixerSoundCompletionEvent::Name() const
{
	return Self::kName;
}

int
ALmixerSoundCompletionEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		lua_pushinteger( L, whichChannel );
		lua_setfield( L, -2, "channel" );

		lua_pushinteger( L, alSource );
		lua_setfield( L, -2, "source" );

		lua_pushlightuserdata( L, almixerData );
		lua_setfield( L, -2, "handle" );

		lua_pushboolean( L, finishedNaturally );
		lua_setfield( L, -2, "completed" );

		// Future phases might relate to "fade"
		lua_pushstring( L, "stopped" );
		lua_setfield( L, -2, "phase" );
	}
	return 1;
}

// ----------------------------------------------------------------------------

PlatformALmixerPlaybackFinishedCallback::PlatformALmixerPlaybackFinishedCallback( const ResourceHandle<lua_State> & handle )
:	PlatformNotifier( handle )
{
}

PlatformALmixerPlaybackFinishedCallback::~PlatformALmixerPlaybackFinishedCallback()
{
}

// ----------------------------------------------------------------------------

PlatformOpenALPlayer::PlatformOpenALPlayer()
:	isInitialized( false ),
	isSuspended( false ),
	mapOfLoadedFileNamesToData(NULL),
	mapOfLoadedDataToFileNames(NULL),
	mapOfLoadedDataToReferenceCountNumber(NULL),
	useAudioSessionInitializationFailureToAbortEndInterruption(true), // iOS 4/5 need this on
	notifier( NULL )
{
	// It's possible that InitializeOpenALPlayer() is never called 
	// by the time RuntimeWillTerminate() is called in which case we have
	// uninitialized variables being accessed, so we make sure to init them
	// to NULL.
	InitializeCallbacks();
}

PlatformOpenALPlayer::~PlatformOpenALPlayer()
{
	QuitOpenALPlayer();
}


static PlatformOpenALPlayer *sOpenALPlayer = NULL;
static int sRefCount = 0;

void
PlatformOpenALPlayer::RuntimeWillTerminate( const Runtime& sender )
{
//	lua_State *L = sender.VMContext().L();

	// Find all channels used by this Runtime instance
	for ( unsigned int i = 0; i < kOpenALPlayerMaxNumberOfSources; i++ )
	{
		//PlatformALmixerPlaybackFinishedCallback *callback = arrayOfChannelToLuaCallbacks[i];
		int callback = SwapChannelCallback( (int)i, LUA_NOREF );
		if ( LUA_NOREF != callback )
		{
		//	lua_State *callbackL = callback->GetLuaState();

			// Channel is used by Runtime if the L states match
		//	if ( callbackL == L )
			{
				// Remove callback *before* calling StopChannel,
				// b/c the latter triggers notification to the callback
			//	arrayOfChannelToLuaCallbacks[i] = NULL;
			//	Rtt_DELETE( callback );
				
				// Ensure audio does not continue to play for this Runtime.
				StopChannel( i );
			}
		}
	}
}

bool
PlatformOpenALPlayer::IsInitialized() const
{
	return isInitialized;
}

void
PlatformOpenALPlayer::InitializeCallbacks()
{
	for(unsigned int i=0; i<kOpenALPlayerMaxNumberOfSources; i++)
	{
		arrayOfChannelToLuaCallbacks[i] = LUA_NOREF;//NULL;

		channelBusy[i].clear();
	}
}

bool
PlatformOpenALPlayer::InitializeOpenALPlayer()
{
	// Don't reinitialize
	if(true == isInitialized)
	{
		return true;
	}
	
	/* AudioSession stuff is only for iOS.
	 */
	PlatformAudioSessionManager* audioSessionMananger = PlatformAudioSessionManager::SharedInstance();
#if (TARGET_OS_IOS == 1)
	// On iOS, the OpenAL way of setting the frequency has no effect and we must set a special setting via AudioSession Services
	((IPhoneAudioSessionManager*)(audioSessionMananger))->SetProperty(kAudioSessionProperty_PreferredHardwareSampleRate, (Float64)s_playerFrequency);

#endif
#ifdef Rtt_ANDROID_ENV
	NativeToJavaBridge::OnAudioEnabled();
#endif
	audioSessionMananger->SetAudioSessionActive(true);
	
	ALboolean did_init = ALmixer_Init(s_playerFrequency, kOpenALPlayerMaxNumberOfSources, 0);
	// Windows users may not have sound cards	
//	Rtt_ASSERT( AL_TRUE == did_init );

	isInitialized = did_init;
	
	if(false == isInitialized)
	{
		return isInitialized;
	}
	
	ALmixer_SetPlaybackFinishedCallback(Internal_SoundFinishedCallback, this);
	InitializeCallbacks();
		
	mapOfLoadedFileNamesToData = LuaHashMap_Create();
	mapOfLoadedDataToFileNames = LuaHashMap_CreateShare(mapOfLoadedFileNamesToData);
	mapOfLoadedDataToReferenceCountNumber = LuaHashMap_CreateShare(mapOfLoadedFileNamesToData);

	// Initialize the interruption array just for good measure.
	for( unsigned int i=0; i<kOpenALPlayerMaxNumberOfSources; i++ )
	{
		arrayOfChannelStatesForInterruption[i] = 0;
	}
	
	return isInitialized;
}
	
bool
PlatformOpenALPlayer::InitializeOpenALPlayer() const
{
	PlatformOpenALPlayer* player = const_cast<PlatformOpenALPlayer*>(this);
	return player->InitializeOpenALPlayer();
}

void
PlatformOpenALPlayer::QuitOpenALPlayer()
{
	if(false == isInitialized)
	{
		return;
	}
	
	PlatformAudioSessionManager::Destroy();

	// When ALmixer_Quits, Halt is called on all channels and callbacks are still invoked for final cleanups.
	// However, this is a problem for us because we have hooks into the Lua scripts.
	// It is possible that the lua state is already destroyed in which case a callback into Lua will cause problems.
	// So we need to do something to make sure Lua isn't called back into.
	// I could disable ALmixer callbacks entirely here, but thinking about long term planning for automatically 
	// managing memory for ALmixer_Data, we need the callback system to monitor data exit points.
	// So I am hesistant to disable callbacks.
	// Alternatively, I can delete the callback mappings and check for NULL.
	// But I need to state that this problem may be worse than that. The problem is I need to really shut down 
	// before the Lua state is destroyed because callbacks happen on a background thread. There is a potential
	// race condition that will allow a sound to finish playing and trigger a callback while the similator is in
	// relaunching transition.
/*
	for(unsigned int i=0; i<kOpenALPlayerMaxNumberOfSources; i++)
	{
		Rtt_DELETE( arrayOfChannelToLuaCallbacks[i] );
		arrayOfChannelToLuaCallbacks[i] = NULL; // Set to NULL to try to avoid asynchronus issues on bad pointers.
	}
*/
	ALmixer_Quit();
	
	// ALmixer cleans up all the audio data so we can just free the map
	LuaHashMap_FreeShare(mapOfLoadedFileNamesToData);
	mapOfLoadedFileNamesToData = NULL;
	LuaHashMap_FreeShare(mapOfLoadedDataToFileNames);
	mapOfLoadedDataToFileNames = NULL;
	LuaHashMap_Free(mapOfLoadedDataToReferenceCountNumber);
	mapOfLoadedDataToReferenceCountNumber = NULL;

	isInitialized = false;

	AttachNotifier( NULL );
}

PlatformOpenALPlayer *
PlatformOpenALPlayer::RetainInstance()
{
	if ( NULL == sOpenALPlayer )
	{
		sOpenALPlayer = new PlatformOpenALPlayer;
	}
	else
	{
#ifdef USE_STATIC_MIXER
		ALmixer_EndInterruption();
#endif
	}
	++sRefCount;

	return sOpenALPlayer;
}

void 
PlatformOpenALPlayer::ReleaseInstance()
{
	if ( NULL != sOpenALPlayer )
	{
		if ( --sRefCount <= 0 )
		{
#ifndef USE_STATIC_MIXER
			delete sOpenALPlayer;
			sOpenALPlayer = NULL;
#endif
			sRefCount = 0;
		}
	}
}

PlatformOpenALPlayer *
PlatformOpenALPlayer::SharedInstance()
{
	return sOpenALPlayer;
}

void 
PlatformOpenALPlayer::SetFrequency( unsigned int play_frequency )
{
	// Note: For performance reasons, I do not re-init ALmixer.
	// Instead, the assumption is ALmixer is not already init because the user specifies it in a config file.
	// That means setting will have no effect until the next time ALmixer is reinitialized.
	s_playerFrequency = play_frequency;
}


void 
PlatformOpenALPlayer::SetMaxSources( unsigned int max_sources )
{
	// Not supported yet because I need to change the callback array to be dynamic
	// static unsigned int s_maxSources = kOpenALPlayerMaxNumberOfSources;
	
	
	// Note: For performance reasons, I do not re-init ALmixer.
	// Instead, the assumption is ALmixer is not already init because the user specifies it in a config file.
	// That means setting will have no effect until the next time ALmixer is reinitialized.
//	s_maxSources = max_sources;
}

void
PlatformOpenALPlayer::AttachNotifier( PlatformNotifier* pnotifier )
{
	if ( notifier != pnotifier )
	{
		Rtt_DELETE( notifier );
	}

	notifier = pnotifier;
}

void
PlatformOpenALPlayer::SetChannelCallback( lua_State* L, int channel, int userCallback )
{
	Rtt_ASSERT( notifier );

	if ( channel >= 0 )
	{
		userCallback = SwapChannelCallback( channel, userCallback );
	}

	lua_unref( L, userCallback );
}

ALmixer_Data* 
PlatformOpenALPlayer::LoadAll( const char* file_path )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	// If the user has already loaded the data, don't load it again, but return the cached pointer.
	LuaHashMapIterator filename_iterator = LuaHashMap_GetIteratorForKeyString(mapOfLoadedFileNamesToData, file_path);
	if(false == LuaHashMap_IteratorIsNotFound(&filename_iterator))
	{
		ALmixer_Data* ret_data = (ALmixer_Data*)LuaHashMap_GetCachedValuePointerAtIterator(&filename_iterator);
		lua_Integer refcount = LuaHashMap_GetValueIntegerForKeyPointer(mapOfLoadedDataToReferenceCountNumber, ret_data);
		LuaHashMap_SetValueIntegerForKeyPointer(mapOfLoadedDataToReferenceCountNumber, refcount+1, ret_data);
		return ret_data;
	}
	ALmixer_Data* ret_data = ALmixer_LoadAll(file_path, false);
	if(NULL != ret_data)
	{
		LuaHashMap_SetValuePointerForKeyString(mapOfLoadedFileNamesToData, ret_data, file_path);
		LuaHashMap_SetValueStringForKeyPointer(mapOfLoadedDataToFileNames, file_path, ret_data);
		lua_Integer refcount = LuaHashMap_GetValueIntegerForKeyPointer(mapOfLoadedDataToReferenceCountNumber, ret_data);
		LuaHashMap_SetValueIntegerForKeyPointer(mapOfLoadedDataToReferenceCountNumber, refcount+1, ret_data);
	}
	return ret_data;
}

// Note: Don't cache files in mapOfLoadedFiles for LoadStream because we permit multiple unique instances for streams.
ALmixer_Data* 
PlatformOpenALPlayer::LoadStream( const char* file_path, unsigned int buffer_size, unsigned int max_queue_buffers, unsigned int number_of_startup_buffers, unsigned int number_of_buffers_to_queue_per_update_pass )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	if(0 == buffer_size)
	{
		buffer_size = ALMIXER_DEFAULT_BUFFERSIZE;
	}
	if(0 == max_queue_buffers)
	{
		max_queue_buffers = ALMIXER_DEFAULT_QUEUE_BUFFERS;
	}
	if(0 == number_of_startup_buffers)
	{
		number_of_startup_buffers = ALMIXER_DEFAULT_STARTUP_BUFFERS;
	}
	if(0 == number_of_buffers_to_queue_per_update_pass)
	{
		number_of_buffers_to_queue_per_update_pass = ALMIXER_DEFAULT_BUFFERS_TO_QUEUE_PER_UPDATE_PASS;
	}
	
	ALmixer_Data* ret_data = ALmixer_LoadStream(file_path, buffer_size, max_queue_buffers, number_of_startup_buffers, number_of_buffers_to_queue_per_update_pass, false);
	// If the audio data is predecoded, we can treat it like a LoadSound and exploit some additional optimizations that come with it.
	if(ALmixer_IsPredecoded(ret_data))
	{
		// Put the data in the hash maps so we can do quick look ups and caching
		LuaHashMap_SetValuePointerForKeyString(mapOfLoadedFileNamesToData, ret_data, file_path);
		LuaHashMap_SetValueStringForKeyPointer(mapOfLoadedDataToFileNames, file_path, ret_data);
		lua_Integer refcount = LuaHashMap_GetValueIntegerForKeyPointer(mapOfLoadedDataToReferenceCountNumber, ret_data);
		LuaHashMap_SetValueIntegerForKeyPointer(mapOfLoadedDataToReferenceCountNumber, refcount+1, ret_data);
	}
	return ret_data;
}

void 
PlatformOpenALPlayer::FreeData( ALmixer_Data* almixer_data )
{
	if(NULL == almixer_data)
	{
		return;
	}
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	// If this was LoadAll, then we need to remove it from the map if the ref count is 0.
	if(ALmixer_IsPredecoded(almixer_data))
	{
		if(LuaHashMap_ExistsKeyPointer(mapOfLoadedDataToFileNames, almixer_data))
		{
			lua_Integer refcount = LuaHashMap_GetValueIntegerForKeyPointer(mapOfLoadedDataToReferenceCountNumber, almixer_data);
			Rtt_ASSERT(refcount > 0);
			if(1 == refcount)
			{
				// Remember that the life of file_name is controlled by mapOfLoadedDataToFileNames so only remove that entry from mapOfLoadedDataToFileNames after we are done using that pointer.
				const char* file_name = LuaHashMap_GetValueStringForKeyPointer(mapOfLoadedDataToFileNames, almixer_data);
				LuaHashMap_RemoveKeyString(mapOfLoadedFileNamesToData, file_name);
				LuaHashMap_RemoveKeyPointer(mapOfLoadedDataToFileNames, almixer_data);
				LuaHashMap_RemoveKeyPointer(mapOfLoadedDataToReferenceCountNumber, almixer_data);
				ALmixer_FreeData(almixer_data);
			}
			else
			{
				LuaHashMap_SetValueIntegerForKeyPointer(mapOfLoadedDataToReferenceCountNumber, refcount-1, almixer_data);
			}
			
		}
		else
		{
			// User has probably double freed the data.
			Rtt_TRACE_SIM(("Error: Over-disposing of the same audio data"));
		}
	}
	else
	{
		// LoadStream always needs to be freed
		ALmixer_FreeData(almixer_data);
	}
}

int
PlatformOpenALPlayer::GetTotalTime( ALmixer_Data* almixer_data ) const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_GetTotalTime(almixer_data);
}
	
	
int 
PlatformOpenALPlayer::ReserveChannels( int number_of_reserve_channels )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_ReserveChannels(number_of_reserve_channels);
}

int 
PlatformOpenALPlayer::PlayChannelTimed( int which_channel, ALmixer_Data* almixer_data, int number_of_loops, int number_of_milliseconds )//, PlatformALmixerPlaybackFinishedCallback *callback )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	int selected_channel = ALmixer_PlayChannelTimed(which_channel, almixer_data, number_of_loops, number_of_milliseconds);/*
	if(selected_channel >= 0)
	{
		// Cleanup any old callback
		PlatformALmixerPlaybackFinishedCallback *oldCallback = arrayOfChannelToLuaCallbacks[selected_channel];
		if ( oldCallback )
		{
			Rtt_DELETE( oldCallback );
		}

		arrayOfChannelToLuaCallbacks[selected_channel] = callback;
	}
	else
	{
		Rtt_DELETE( callback );
	}*/

	return selected_channel;
}

int 
PlatformOpenALPlayer::PauseChannel( int which_channel )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_PauseChannel(which_channel);
}

int 
PlatformOpenALPlayer::ResumeChannel( int which_channel )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_ResumeChannel(which_channel);
}

int 
PlatformOpenALPlayer::StopChannel( int which_channel )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_HaltChannel(which_channel);
}
	
unsigned int 
PlatformOpenALPlayer::GetSourceFromChannel( int which_channel ) const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	// ALmixer will return an available source if you pass less than 0. In our case, we just want a failure value.
	if( which_channel < 0 )
	{
		return 0;
	}
	else
	{
		return ALmixer_GetSource( which_channel );
	}
}

int 
PlatformOpenALPlayer::GetChannelFromSource( unsigned int al_source ) const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	// ALmixer will return an available channel if you pass 0. In our case, we just want a failure value.
	if( 0 == al_source )
	{
		return -1;
	}
	else
	{
		return ALmixer_GetChannel( al_source );
	}
}
	
int 
PlatformOpenALPlayer::FindFreeChannel( int start_channel ) const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_FindFreeChannel( start_channel );
}

bool
PlatformOpenALPlayer::RewindData( ALmixer_Data* almixer_data )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return (bool)ALmixer_RewindData( almixer_data );
}
	
bool
PlatformOpenALPlayer::SeekData( ALmixer_Data* almixer_data, unsigned int number_of_milliseconds )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return (bool)ALmixer_SeekData( almixer_data, number_of_milliseconds );
}

int
PlatformOpenALPlayer::SeekChannel( int which_channel, unsigned int number_of_milliseconds )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_SeekChannel( which_channel, number_of_milliseconds );
}

int
PlatformOpenALPlayer::RewindChannel( int which_channel )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_RewindChannel( which_channel );
}

int
PlatformOpenALPlayer::IsActiveChannel( int which_channel ) const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_IsActiveChannel( which_channel );
}

int
PlatformOpenALPlayer::IsActiveSource( unsigned int al_source ) const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_IsActiveSource( al_source );
}
	
int
PlatformOpenALPlayer::IsPlayingChannel( int which_channel ) const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_IsPlayingChannel( which_channel );
}

int
PlatformOpenALPlayer::IsPlayingSource( unsigned int al_source ) const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_IsPlayingSource( al_source );
}

int
PlatformOpenALPlayer::IsPausedChannel( int which_channel ) const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_IsPausedChannel( which_channel );
}

int
PlatformOpenALPlayer::IsPausedSource( unsigned int al_source ) const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_IsPausedSource( al_source );
}

unsigned int
PlatformOpenALPlayer::CountAllFreeChannels() const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_CountAllFreeChannels();
}

unsigned int
PlatformOpenALPlayer::CountUnreservedFreeChannels() const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_CountUnreservedFreeChannels();
}
	
unsigned int
PlatformOpenALPlayer::CountAllUsedChannels() const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_CountAllUsedChannels();
}
	
unsigned int
PlatformOpenALPlayer::CountUnreservedUsedChannels() const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_CountUnreservedUsedChannels();
}
	
unsigned int
PlatformOpenALPlayer::CountTotalChannels() const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_CountTotalChannels();
}

unsigned int
PlatformOpenALPlayer::CountReservedChannels() const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_CountReservedChannels();
}

int 
PlatformOpenALPlayer::ExpireChannel( int which_channel, int number_of_milliseconds )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_ExpireChannel(which_channel, number_of_milliseconds);
}

int 
PlatformOpenALPlayer::FadeInChannelTimed( int which_channel, ALmixer_Data* almixer_data, int number_of_loops, unsigned int fade_ticks, unsigned int expire_ticks )//, PlatformALmixerPlaybackFinishedCallback *callback )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	int selected_channel = ALmixer_FadeInChannelTimed(which_channel, almixer_data, number_of_loops, fade_ticks, expire_ticks);
/*
	if(selected_channel >= 0)
	{
		// Cleanup any old callback
		PlatformALmixerPlaybackFinishedCallback *oldCallback = arrayOfChannelToLuaCallbacks[selected_channel];
		if ( oldCallback )
		{
			Rtt_DELETE( oldCallback );
		}

		arrayOfChannelToLuaCallbacks[selected_channel] = callback;
	}
	else
	{
		Rtt_DELETE( callback );
	}*/

	return selected_channel;
}
	
int 
PlatformOpenALPlayer::FadeOutChannel( int which_channel, unsigned int fade_ticks )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_FadeOutChannel(which_channel, fade_ticks);
}
	
int 
PlatformOpenALPlayer::FadeChannel( int which_channel, unsigned int fade_ticks, float to_volume )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_FadeChannel(which_channel, fade_ticks, to_volume);
}
	
bool 
PlatformOpenALPlayer::SetVolumeChannel( int which_channel, float new_volume )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_SetVolumeChannel( which_channel, new_volume );
}

float 
PlatformOpenALPlayer::GetVolumeChannel(  int which_channel ) const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_GetVolumeChannel( which_channel );
}
	
bool 
PlatformOpenALPlayer::SetMaxVolumeChannel( int which_channel, float new_volume )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_SetMaxVolumeChannel( which_channel, new_volume );
}

float 
PlatformOpenALPlayer::GetMaxVolumeChannel(  int which_channel ) const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_GetMaxVolumeChannel( which_channel );
}
	
bool 
PlatformOpenALPlayer::SetMinVolumeChannel( int which_channel, float new_volume )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_SetMinVolumeChannel( which_channel, new_volume );
}

float 
PlatformOpenALPlayer::GetMinVolumeChannel(  int which_channel ) const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
	return ALmixer_GetMinVolumeChannel( which_channel );
}

bool 
PlatformOpenALPlayer::SetMasterVolume( float new_volume )
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
#ifdef __APPLE__ // Ugly hack due to Apple's OpenAL bug. Setting 0 gain on listener is ignored
	if(new_volume == 0) new_volume = __FLT_MIN__;
#endif
	return ALmixer_SetMasterVolume(new_volume);
}

float 
PlatformOpenALPlayer::GetMasterVolume() const
{
	if( ! IsInitialized() )
	{
		InitializeOpenALPlayer();
	}
#ifdef __APPLE__ // Ugly hack due to Apple's OpenAL bug. Setting 0 gain on listener is ignored
	float ret = ALmixer_GetMasterVolume();
	if(ret == __FLT_MIN__) ret = 0;
	return ret;
#else
	return ALmixer_GetMasterVolume();
#endif
}

void 
PlatformOpenALPlayer::SuspendPlayer()
{
	// Ugh. In iOS 5.0, bug rdar://10929452 appeared for suspending/resuming.
	// This causes mediaserverd to crash when suspending/resuming between two different OpenAL apps.
	// Apple tells me they expect me to set the OpenAL context to NULL when backgrounding which may be part of the problem.
	// But we made a differentiation intentionally between Interruptions and Suspend because of bug rdar://10081775
	// In this latter bug, there is a race condition when you set the OpenAL context and try using it too soon resulting
	// in audio not playing.
	// But now we are stuck and must do a full interruption for backgrounding.
	BeginInterruption();
	isSuspended = true;
}
	
void 
PlatformOpenALPlayer::ResumePlayer()
{
	// Don't check for interruption because we may have manually started a fake interruption for SuspendPlayer.
	// Just call EndInterruption() and let it do the right thing.
	if( ! isSuspended )
	{
		return;
	}
	EndInterruption();
	isSuspended = false;
}

bool 
PlatformOpenALPlayer::IsPlayerSuspended() const
{
	return isSuspended;
}

void
PlatformOpenALPlayer::PauseChannelsForInterruption()
{
	for( unsigned int i=0; i<kOpenALPlayerMaxNumberOfSources; i++ )
	{
		// This will return 1 for playing, or 0 for other.
		// We currently don't need to distinguish between paused or stopped/inactive because we only need to resume playing channels later.
		arrayOfChannelStatesForInterruption[i] = ALmixer_IsPlayingChannel(i);
		if( 1 == arrayOfChannelStatesForInterruption[i] )
		{
			ALmixer_PauseChannel(i);
		}
	}
}
void
PlatformOpenALPlayer::ResumeChannelsForInterruption()
{
	// Assertion: PauseChannelsForInterruption was called and we are now resuming from it.
	
	for( unsigned int i=0; i<kOpenALPlayerMaxNumberOfSources; i++ )
	{
		// Resume only those channels we marked as playing.
		if( 1 == arrayOfChannelStatesForInterruption[i] )
		{
			ALmixer_ResumeChannel(i);
		}
	}
}
void
PlatformOpenALPlayer::BeginInterruption()
{
	if( ! IsInitialized() )
	{
		return;
	}
	// If we are already in an interruption, it is too late to do anything as ALmixer is already disabled
	if ( 1 == ALmixer_IsInInterruption() )
	{
		return;
	}

	PauseChannelsForInterruption();
#if defined( Rtt_IPHONE_ENV ) || defined ( Rtt_MAC_ENV )
	// Workaround for Apple bug rdar://10081775
	// There is a race condition in iOS and Mac with setting the OpenAL context.
	// This causes our attempt to resume audio to sometimes later get clobbered and somehow the audio repauses.
	// Mac will probably never call this code because we decoupled Suspend/Resume from Interruptions, 
	// but just in case, Mac is included in this workaround too.
	// Update: These functions are no longer decoupled because of bug rdar://10929452 and are now run for both.
	// Mac suspend/resume is typically limited to builds and the menu so it probably isn't a huge problem for now.
	// MacApp might complicate this but we have no reason to suspend in MacApp at the moment.
	// Originally, we had this in the  EndInterruption, but my contact tells me the problem is actually in the setting
	// the context to NULL in the BeginInterruption and we are seeing the ripple effects on the other-side.
	// I was told it is better to put the sleep here. Also, it seems that we can use a smaller sleep value.
	// We used 60ms on the end and it failed 1/20 times, but here 15ms seems to work 21/21. I made it 20ms for superstitious reasons.
	usleep(20000); // Tom tested multiple values and found 10ms to fail but 15ms to work on an iPad 2 (iOS 5.1).
#endif

	ALmixer_BeginInterruption();

	// In a real interruption, the audio session is automatically set inactive,
	// but we use this for non-real interruptions and our manager needs to get explicitly set
	// to shadow the state correctly.
	PlatformAudioSessionManager::SharedInstance()->SetAudioSessionActive(false);
}

void 
PlatformOpenALPlayer::EndInterruption()
{
	if( ! IsInitialized() )
	{
		return;
	}
	
	bool is_success = PlatformAudioSessionManager::SharedInstance()->SetAudioSessionActive(true);
	
	// iOS 4/5 bug: A user leaving an app via notification center triggers a unnecessary resume/suspend event in Cocoa.
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
	if( ! is_success && GetUseAudioSessionInitializationFailureToAbortEndInterruption() )
	{
		PlatformAudioSessionManager::SharedInstance()->SetAudioSessionActive(false);
		return;
	}

	ALmixer_EndInterruption();
	ResumeChannelsForInterruption();
}

bool
PlatformOpenALPlayer::IsInInterruption() const
{
	if( ! IsInitialized() )
	{
		return false;
	}
	return (bool)ALmixer_IsInInterruption();
}
	

		
void 
PlatformOpenALPlayer::NotificationCallback( int which_channel, unsigned int al_source, ALmixer_Data* almixer_data, bool finished_naturally)
{
	// Bug:5724. There seems to be a race condition bug with the callback system.
	// Ultimately, the problem is that the callback system needs to hold on to the PlatformNotifer until it does the actual callback,
	// but during this period, there is a possibility that a new play() call is invoked which will fight over the platform notifier.
	// This may result in one of two conflicts: 
	// (1) The old notifier gets overwritten with a new callback and the callback calls the wrong function.
	// (2) The new play saves the callback (but happened to point to the same function), but when the old notifier finally gets invoked, it clears the callback
	// so when the new sound ends, no callback happens. (This is what we are reproducing with 5724.)
	// The workaround/solution seems to be to transfer ownership of the callback to the completion event and separate the notifier from the main array.
	// This way, we keep track of two separate instances which won't clobber each other in the race condition.
	// Since ALmixer locks during this callback, it should be safe to move all the book keeping stuff around in this function.
	// One more note on the callback system: It is possible for a channel to be reclaimed by ALmixer to be played on before the user
	// receives the actual callback notification in Lua. I suspect this is due to the order we process the scripts and when we process events.
	// Internally, this should work correctly and our audio engine will work fine, but Lua users might be a little surprised by this.
	// Anybody that builds a partial resource management system on top of our resource system may need to understand this.
	// If they use auto-channel-assignment in play, but use callbacks to record channel availability, their system may get out-of-sync and break.
	// So any user that implements a resource management system should also directly manage channel assignment instead of relying on auto-assignment.
//	PlatformALmixerPlaybackFinishedCallback *notifier = arrayOfChannelToLuaCallbacks[which_channel];
	int callback = SwapChannelCallback( which_channel, LUA_NOREF );

	if ( LUA_NOREF != callback && LUA_REFNIL != callback )//notifier )
	{
		// There's always a notifier attached, but we only dispatch if a listener is attached
		// NOTE: if we need to dispatch, we transfer ownership so that Lua-related data can
		// be freed on the Lua thread.
//		if ( notifier->HasListener() )
		{
			lua_State *L = notifier->GetLuaState();
			Rtt_ASSERT( L );
			if ( L )
			{
				// Ensure that the event is dispatched on the main thread
				// TODO: ensure that ScheduleDispatch() is thread-safe.
				ALmixerSoundCompletionEvent *e = Rtt_NEW( LuaContext::GetRuntime( L )->GetAllocator(), ALmixerSoundCompletionEvent );

				// Bug 5724: In addition to setting all the callback properties, we pass the notifier in to transfer ownership. 
				// We expect when the event is fired and deleted, it will also delete the notifier with it.
				e->SetProperties( which_channel, al_source, almixer_data, finished_naturally );//notifier );
				notifier->ScheduleDispatch( e, callback );
			}
		}/*
		else
		{
			// No Lua data attached (e.g. listener), so it's okay to delete immediately.
			Rtt_DELETE( notifier );
		}

		// Channel is free, so NULL out callback reference.
		arrayOfChannelToLuaCallbacks[which_channel] = NULL;*/
	}

	// I would really like to remove the callback function from our list here, but it appears that it is not safe
	// to remove it yet because nothing else in the callback system holds it.
	// It seems that because the callback happens on the main thread, if I remove it here,
	// there is a random chance the callback fires or it doesn't.
	// Careful with Bug:5724: Now that we transfer ownership, this approach needs to be re-evaluated.
//	arrayOfChannelToLuaCallbacks[which_channel]->CleanupNotifier();
}

void 
PlatformOpenALPlayer::SetUseAudioSessionInitializationFailureToAbortEndInterruption( bool enabled )
{
	useAudioSessionInitializationFailureToAbortEndInterruption = enabled;
}

bool
PlatformOpenALPlayer::GetUseAudioSessionInitializationFailureToAbortEndInterruption() const
{
	return useAudioSessionInitializationFailureToAbortEndInterruption;
}
	
int
PlatformOpenALPlayer::SwapChannelCallback( int channel, int newRef )
{
	std::atomic_flag& flag = channelBusy[channel];

	// see https://en.cppreference.com/w/cpp/atomic/atomic_flag (Aug 15, 2023)

	while ( !flag.test_and_set( std::memory_order_acquire ) )
		; // empty

    std::atomic_thread_fence( std::memory_order_acquire );

	int ref = arrayOfChannelToLuaCallbacks[channel];

	arrayOfChannelToLuaCallbacks[channel] = newRef;

    flag.clear( std::memory_order_release );

	return ref;
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_USE_ALMIXER

