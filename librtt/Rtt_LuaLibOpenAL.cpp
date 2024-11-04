//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_LuaLibOpenAL.h"

#include "Rtt_LuaAux.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSystem.h"
//#include "Rtt_LuaProxy.h"
#include "Rtt_MPlatform.h"
//#include "Rtt_MPlatformDevice.h"

#ifdef Rtt_USE_ALMIXER
#include "Rtt_PlatformOpenALPlayer.h"
#include "luaal.h"

	#ifdef Rtt_IPHONE_ENV
		#include "Rtt_IPhoneAudioSessionManager.h"
	#endif

#endif

#include "Rtt_Runtime.h"
#include "Core/Rtt_String.h"
#include "Core/Rtt_Assert.h"
#include "CoronaLua.h"

// #include <string.h>

#if defined(Rtt_WIN_ENV)
#	include <crtdbg.h> // _CrtIsValidHeapPointer().
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{
	
// ----------------------------------------------------------------------------

//static const char kEventSound[] = "media.EventSound";
	
#ifdef Rtt_USE_ALMIXER

#if defined(Rtt_WIN_ENV ) || defined(Rtt_ANDROID_ENV) || defined(Rtt_NXS_ENV ) || defined(Rtt_LINUX_ENV )
#include <malloc.h>
#else
	#ifndef EMSCRIPTEN
		#include <malloc/malloc.h>
	#endif
#endif

static bool
isValidSoundData(lua_State *L, void *sound_data, const char *caller, size_t expectedSize = 0)
{
#if !defined(Rtt_ANDROID_ENV) && !defined(Rtt_LINUX_ENV) && !defined(Rtt_NXS_ENV)	// Can't do this on Android

	// Check to see that we have a valid pointer
	size_t allocSize = 0;
	
	if (sound_data != NULL)
	{
#if defined(Rtt_WIN_ENV)
		allocSize = _CrtIsValidHeapPointer(sound_data);
#elif defined(Rtt_APPLE_ENV)
		allocSize = malloc_size(sound_data);
#endif
	}

	if (allocSize == 0 /*|| (expectedSize != 0 && allocSize != expectedSize)*/)
	{
		luaL_error( L , "Warning: %s: invalid sound object %p (previously disposed?)\n", caller, sound_data);
		
		return false;
	}

	// printf("++++++: %s: valid sound object %p (%ld)\n", caller, sound_data, allocSize);

#endif // ! defined(Rtt_ANDROID_ENV)

	return true;
}

static int
loadSoundAll( lua_State *L )
{
	/**
		@fn loadSound(lua_String file_name, [baseDir], [{table_params})
		@brief Loads an entire file completely into memory.
		@details Loads an entire file completely into memory and returns a reference to the audio data. Files that are loaded completely into memory may be reused/played/shared simulataneously on multiple channels so you only need to load one instance of the file. You should use this to load all your short sounds, especially ones you may play frequently. For best results, load all the sounds at the launch of your app or the start of a new level.
		@param file_name The name of the file you want to load.
		@return Returns a handle to the loaded file.
		@code
		laserSound = audio.loadSound("laserBlast.wav")
		@endcode
	*/
		
	/* TODO: We might want to cache loadAll files in a map so if a user loads it multiple times,
	 * we don't waste memory with duplicates. This will not work with loadStream files though.
	 */
	ALmixer_Data* sound_data = NULL;
	const MPlatform & platform = LuaContext::GetRuntime( L )->Platform();
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	if ( lua_isstring( L, 1 ) )
	{
		const char* filename = lua_tostring( L, 1 );
		MPlatform::Directory baseDir = MPlatform::kResourceDir;
		String filePath( & platform.GetAllocator() );
		int current_stack_argument = 2;
		if ( lua_islightuserdata( L, current_stack_argument ) )
		{
			void* p = lua_touserdata( L, -1 );
				baseDir = (MPlatform::Directory)EnumForUserdata(
					LuaLibSystem::Directories(),
					p,
					MPlatform::kNumDirs,
					MPlatform::kResourceDir );
			current_stack_argument++;
		}
		
		if ( lua_istable( L, current_stack_argument ) )
		{
			/* Currently no supported parameters, but we may use this in the future */
/*
			lua_getfield( L, current_stack_argument, "baseDir");
			if ( lua_islightuserdata( L, -1 ) )
			{
				void* p = lua_touserdata( L, -1 );
				baseDir = (MPlatform::Directory)EnumForUserdata(
					LuaLibSystem::Directories(),
					p,
					MPlatform::kNumDirs,
					MPlatform::kResourceDir );
			}
			lua_pop( L, 1 );
*/
		}
		
		// Get the full path after we parse for the baseDir parameter
		platform.PathForFile( filename, baseDir, MPlatform::kDefaultPathFlags, filePath );

		if ( filePath.GetString()
			&& (sound_data = openal_player->LoadAll( filePath.GetString() )) )
		{
			// Store callback *prior* to pushing result
			//			soundData->SetListenerRef( nextArg );
			
			//			LuaContext::PushUserdata( L, soundID, kEventSound );
			lua_pushlightuserdata( L, sound_data);
		}
		else
		{
			CoronaLuaWarning(L, "audio.loadSound() failed to create sound '%s'", lua_tostring( L, 1 ) );
			lua_pushnil(L);
		}
	}
	
	return 1;
}
	
static int
loadSoundStream( lua_State *L )
{
	/**
	 @fn loadStream(lua_String file_name, lua_Table optional_or_hidden_parameters)
	 @brief Loads (opens) a file to be read as a stream.
	 @details Streamed files are read in little chunks at a time to minimize memory use. These 
	 are intended for large/long files like background music and speech.
	 Unlike load "all" files, these cannot be shared simultaneously across multiple channels. If you 
	 need to play multiple simulataneous instances, you must load multiple instances of the file.
	 @param file_name The name of the file you want to load.
	 @param optional_or_hidden_parameters Currently not public, but we may want to allow buffer sizes 
	 and number of buffers to be specified in case end developers need to fine-tune performance and memory.
	 @tableparam bufferSize size in bytes of how much data to fetch for each read. Must be a evenly divisble frame size. 
	 @tableparam maxQueueBuffers Maximum number of buffers that can be queued at any time.
	 @tableparam numberOfStartupBuffers Number of buffers to prefill before starting playback. Must be less than or equaltto maxQueueBuffers and at least 1.
	 @return Returns a handle to the loaded file.
	 @code
	 backgroundMusic = audio.loadStream("backgroundMusic.m4a")
	 @endcode
	 */
	 
	ALmixer_Data* sound_data = NULL;
	const MPlatform & platform = LuaContext::GetRuntime( L )->Platform();
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	if ( lua_isstring( L, 1 ) )
	{
		const char* filename = lua_tostring( L, 1 );
		MPlatform::Directory baseDir = MPlatform::kResourceDir;
		String filePath( & platform.GetAllocator() );
		
		/* Hidden parameters for bufferSize, startupBuffers, maxBuffers */
		unsigned int buffer_size = 0;
		unsigned int max_queue_buffers = 0;
		unsigned int num_startup_buffers = 0;
		unsigned int num_buffers_to_queue_per_update_pass = 0;

		int current_stack_argument = 2;
		if ( lua_islightuserdata( L, current_stack_argument ) )
		{
			void* p = lua_touserdata( L, -1 );
				baseDir = (MPlatform::Directory)EnumForUserdata(
					LuaLibSystem::Directories(),
					p,
					MPlatform::kNumDirs,
					MPlatform::kResourceDir );
			current_stack_argument++;
		}
		
		if ( lua_istable( L, current_stack_argument ) )
		{
			lua_getfield( L, current_stack_argument, "bufferSize");
			if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
			{
				buffer_size = (int) lua_tointeger( L, -1 );
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, current_stack_argument, "maxQueueBuffers");
			if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
			{
				max_queue_buffers = (int) lua_tointeger( L, -1 );
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, current_stack_argument, "startupBuffers");
			if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
			{
				num_startup_buffers = (int) lua_tointeger( L, -1 );
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, current_stack_argument, "buffersQueuedPerUpdate");
			if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
			{
				num_buffers_to_queue_per_update_pass = (int) lua_tointeger( L, -1 );
			}
			lua_pop( L, 1 );
			/*
			lua_getfield( L, current_stack_argument, "baseDir");
			if ( lua_islightuserdata( L, -1 ) )
			{
				void* p = lua_touserdata( L, -1 );
				baseDir = (MPlatform::Directory)EnumForUserdata(
					LuaLibSystem::Directories(),
					p,
					MPlatform::kNumDirs,
					MPlatform::kResourceDir );
			}
			lua_pop( L, 1 );
			 */
		}
		
		// Get the full path after we parse for the baseDir parameter
		platform.PathForFile( filename, baseDir, MPlatform::kDefaultPathFlags, filePath );

		
		if ( filePath.GetString()
			&& (sound_data = openal_player->LoadStream( filePath.GetString(), buffer_size, max_queue_buffers, num_startup_buffers, num_buffers_to_queue_per_update_pass )) )
		{
			// Store callback *prior* to pushing result
			//			soundData->SetListenerRef( nextArg );
			
			//			LuaContext::PushUserdata( L, soundID, kEventSound );
			lua_pushlightuserdata( L, sound_data);
		}
		else
		{
			CoronaLuaWarning(L, "audio.loadStream() failed to create stream '%s'", lua_tostring( L, 1 ) );
			lua_pushnil(L);
		}
	}
	else
	{
		// bad user input
		return 0;
	}
	
	return 1;
}

static int
freeData( lua_State *L )
{
	/**
	 @fn freeData(ALmixer_Data* almixer_data)
	 @brief Releases audio memory.
	 @details Audio memory associated with the handle is freed. You must not use the handle once the memory is freed.
	 @param almixer_data The handle of the data you want to release.
	 @code
	 backgroundMusic = audio.loadStream("backgroundMusic.m4a")
	 audio.dispose(backgroundMusic)
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	if( lua_islightuserdata( L, 1 ) )
	{
		ALmixer_Data * sound_data = (ALmixer_Data*)lua_touserdata( L, 1 );
		
		// Check to see that we have a valid pointer
		if (! isValidSoundData(L, sound_data, "audio.dispose"))
		{
			return 0;
		}
		
		openal_player->FreeData( sound_data );
	}
	return 0;
}

static int
playChannelTimed( lua_State *L )
{
	/**
	 @fn play(ALmixer_Data* file_handle, lua_Table optional_parameters)
	 @brief Plays audio on a channel.
	 @details Plays the audio specified by the file handle on a channel. If a channel is not explicitly
	 specified, an available channel will be automatically chosen for you if available.
	 @param file_handle The handle to the file you want to play.
	 
	 @param optional_parameters A table containing optional named parameters for finer control.

		@tableparam channel You may specify a channel you want to play on. Valid numbers are from 1 to the max
		number of channels available which is currently 32. If you specify a channel that is already in use,
		play will fail. Specifying 0 will ask the system to automatically provide an available channel which
		is the default behavior.

		@tableparam source You may specify the source you want to play on. If you specify a source that is already in use,
		play will fail. Specifying 0 will ask the system to automatically provide an available source which
		is the default behavior. Specifying a source overrides a channel if it conflicts.
		
		@tableparam loops The number of times you want the audio to loop. Notice that 0 means the audio will loop
		0 times which means that the sound will play once and not loop. Continuing that thought, 1 means the audio
		will play once and loop once which means you will hear the sound a total of 2 times.
		Passing -1 will tell the system to infinitely loop the sample.

		@tableparam duration In milliseconds, this will cause the system to play the audio for the specified amount
		of time and then auto-stop the playback regardless of whether the audio has finished or not.

		@tableparam fadein In milliseconds, this will cause the system to start playing a sound at the minimum channel volume and linearly ramp up to the normal channel volume over the specified number of milliseconds.	

		@tableparam onComplete A lua function to be called when playback ends.

	 @return Returns 2 parameters: the channel number and source the audio is played on or 0 if playback was not possible.

	 @code
	 backgroundMusic = audio.loadStream("backgroundMusic.m4a")
	 laserSound = audio.loadSound("laserBlast.wav")
	 local background_music_channel = audio.play(backgroundMusic, { loops=-1 } )	 
	 local laser_channel = audio.play(laserSound)
	 @endcode
	 */
	 
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	ALmixer_Data* sound_data = NULL;
	int which_channel = -1; // by default, auto pick a channel
	unsigned int al_source = 0; // by default, auto pick a channel
	int number_of_loops = 0;
	int expire_ticks = -1;
	int fade_ticks = 0;
	int user_function_callback_index = 0;
	bool found_user_function_callback = false;

	if( lua_islightuserdata( L, 1 ) )
	{
		sound_data = (ALmixer_Data*)lua_touserdata( L, 1 );
		
		// Check to see that we have a valid pointer
		if (! isValidSoundData(L, sound_data, "audio.playChannelTimed"))
		{
			return 2;
		}
	}
	
	if ( lua_istable( L, 2 ) )
	{
		lua_getfield( L, 2, "channel");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 2, "source");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			al_source = (int) lua_tointeger( L, -1 );
			// Source will override channel if there is an inconsistency
			which_channel = openal_player->GetChannelFromSource(al_source);
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 2, "loops");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			number_of_loops = (int) lua_tointeger( L, -1 );
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 2, "duration");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			expire_ticks = (int) lua_tointeger( L, -1 );
		}
		lua_pop( L, 1 );

		lua_getfield( L, 2, "fadein");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			fade_ticks = (int) lua_tointeger( L, -1 );
		}
		lua_pop( L, 1 );
		
		// Backup because people are going to make this mistake
		lua_getfield( L, 2, "fadeIn");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			fade_ticks = (int) lua_tointeger( L, -1 );
		}
		lua_pop( L, 1 );
		
		// Special handling for the listener callback function
		lua_getfield( L, 2, "onComplete");
		if ( lua_isfunction( L, -1 ) )
		{
			user_function_callback_index = -1;
			found_user_function_callback = true;	
		}
		// Don't pop the function yet. Leave it on the stack so the implementation can easily access the function.
	}
	
	int play_channel, callback = LUA_REFNIL;
	unsigned int play_source;
	LuaContext *context = LuaContext::GetContext( L );
//	PlatformALmixerPlaybackFinishedCallback *callback = Rtt_NEW( LuaContext::GetAllocator( L ), PlatformALmixerPlaybackFinishedCallback( context->LuaState() ) );

	if ( 0 != user_function_callback_index )
	{
	//	callback->SetListenerRef( user_function_callback_index );
		int t = lua_type( L, user_function_callback_index );
		if ( LUA_TFUNCTION == t || LUA_TTABLE == t )
		{
			lua_pushvalue( L, user_function_callback_index );
			callback = luaL_ref( L, LUA_REGISTRYINDEX );
		}
	}

	if(fade_ticks > 0)
	{
		play_channel = openal_player->FadeInChannelTimed( which_channel, sound_data, number_of_loops, fade_ticks, expire_ticks );//, callback );
	}
	else
	{
		play_channel = openal_player->PlayChannelTimed( which_channel, sound_data, number_of_loops, expire_ticks );//, callback );
	}
	openal_player->SetChannelCallback( L, play_channel, callback );

	if(found_user_function_callback)
	{
		// Now that PlayChannelTimed has done what it needs to do with the user function, we can pop it.
		lua_pop( L, 1 );
	}
	
	if(-1 == play_channel)
	{
		play_source = 0;
	}
	else
	{
		play_source = openal_player->GetSourceFromChannel(play_channel);
	}

	play_channel += 1; // add 1 to convert from ALmixer/C-array starts at 0 to Lua array starts at 1
	lua_pushinteger( L, play_channel ); // return the channel that the audio is played on
	lua_pushinteger( L, play_source ); // return the source that the audio is played on
	
	return 2;
}

static int
pauseChannel( lua_State *L )
{
	/**
	 @fn pauseChannel(lua_Integer which_channel)
	 @brief Pauses audio playback on a channel.
	 @details Pauses playback on a channel. Should have no effect on channels that aren't playing.
	 @param which_channel The channel to pause or 0 to pause all channels.
	 If table, supports channel= or source=.
	 @return The actual number of channels paused on success or -1 on error.
	 @code
	 local background_music_channel = audio.playChannel(backgroundMusic, { loops=-1 } ) 
	 audio.pause(background_music_channel)
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L ); 
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int which_channel = -1;
	unsigned int al_source = 0;

	if(number_of_arguments > 0)
	{
		if( lua_istable( L, 1 ) )
		{
			lua_getfield( L, 1, "channel");
			if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
			{
				which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, 1, "source");
			if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
			{
				al_source = (int) lua_tointeger( L, -1 );
				// Source will override channel if there is an inconsistency
				which_channel = openal_player->GetChannelFromSource(al_source);
			}
			lua_pop( L, 1 );
		}
		else
		{
			which_channel = (int) lua_tointeger( L, 1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0;
		}
		
	}
	
	int number_of_channels = openal_player->PauseChannel( which_channel );
	// -1 is an error condition. Since the Lua API starts with channel 1 instead of 0, we can return 0.
	if(number_of_channels < 0)
	{
		number_of_channels = 0; // add 1 to convert from ALmixer/C-array starts at 0 to Lua arrays start at 1
	}
	lua_pushinteger( L, number_of_channels);
	
	return 1;
}

static int
resumeChannel( lua_State *L )
{
	/**
	 @fn resume(lua_Integer which_channel)
	 @brief Resumes playback on a channel that is paused.
	 @details Resumes playback on a channel that is paused. Should have no effect on channels that aren't paused.
	 @param The channel to resume or -1 to resume all channels.
	 If table, supports channel= or source=.
	 @return The actual number of channels resumed on success or -1 on error.
	 @code
	 local background_music_channel = audio.play(backgroundMusic, { loops=-1 } ) 
	 audio.pause(background_music_channel)
	 audio.resume(background_music_channel)
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L ); 
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int which_channel = -1;
	unsigned int al_source = 0;

	if(number_of_arguments > 0)
	{
		if( lua_istable( L, 1 ) )
		{
			lua_getfield( L, 1, "channel");
			if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
			{
				which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, 1, "source");
			if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
			{
				al_source = (int) lua_tointeger( L, -1 );
				// Source will override channel if there is an inconsistency
				which_channel = openal_player->GetChannelFromSource(al_source);
			}
			lua_pop( L, 1 );
		}
		else
		{
			which_channel = (int) lua_tointeger( L, 1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0;
		}
		
	}
	
	int number_of_channels = openal_player->ResumeChannel( which_channel );
	// -1 is an error condition. Since the Lua API starts with channel 1 instead of 0, we can return 0.
	if(number_of_channels < 0)
	{
		number_of_channels = 0; // add 1 to convert from ALmixer/C-array starts at 0 to Lua arrays start at 1
	}
	lua_pushinteger( L, number_of_channels);
	
	return 1;
}

static int
stopChannel( lua_State *L )
{
	/**
	 @fn stop(lua_Integer which_channel)
	 @brief Stops playback on a channel.
	 @details Stops playback on a channel and clears the channel so it can be played on again.
	 Callbacks will still be invoked, but the finished_naturally flag will be set to false.
	 @param which_channel The channel to stop or -1 to stop all channels.
	 If table, supports channel= or source=.
	 @return The actual number of channels halted on success or -1 on error.
	 @code
	 local background_music_channel = audio.play(backgroundMusic, { loops=-1 } ) 
	 audio.stop(background_music_channel)
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L ); 
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int which_channel = -1;
	unsigned int al_source = 0;
	
	if(number_of_arguments > 0)
	{
		if( lua_istable( L, 1 ) )
		{
			lua_getfield( L, 1, "channel");
			if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
			{
				which_channel = (int) lua_tointeger( L, -1 ); // In Lua channel ranges
				which_channel = which_channel - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0;
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, 1, "source");
			if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
			{
				al_source = (int) lua_tointeger( L, -1 );
				// Source will override channel if there is an inconsistency
				which_channel = openal_player->GetChannelFromSource(al_source); // In ALmixer/C channel ranges
			}
			lua_pop( L, 1 );
			
			// Bug 8911: Based on user feedback, passing an explicit 0 to stop all channels causes too many unexpected problems when the load/play fails
			// because people not checking values inadvertently stop all the things that are playing. So we are changing this behavior and printing a warning.
			// Note we compare using '<' and not '<=' because 1 is already subtracted from the above and we are already in ALmixer channel ranges.
			if( which_channel < 0 )
			{
				CoronaLuaWarning(L, "audio.stop() called with 0, negative, or no channel/source. To stop all channels, call audio.stop() with no parameters");
				lua_pushinteger( L, 0 );
				return 1;
			}
		}
		else if( LUA_TNUMBER == lua_type( L, 1) )
		{
			which_channel = (int) lua_tointeger( L, 1 );
			
			// Bug 8911: Based on user feedback, passing an explicit 0 to stop all channels causes too many unexpected problems when the load/play fails
			// because people not checking values inadvertently stop all the things that are playing. So we are changing this behavior and printing a warning.
			if( which_channel <= 0 )
			{
				CoronaLuaWarning(L, "audio.stop() called with 0, negative, or no channel/source. To stop all channels, call audio.stop() with no parameters");
				lua_pushinteger( L, 0 );
				return 1;
			}
			which_channel = which_channel - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0;
		}
		else if( LUA_TNIL == lua_type( L, 1) )
		{
			// Bug 8911: Based on user feedback, passing an explicit nil to stop all channels causes too many unexpected problems when the load/play fails
			// because people not checking values inadvertently stop all the things that are playing. So we are changing this behavior and printing a warning.
            CoronaLuaWarning(L, "audio.stop() called with nil. To stop all channels, call audio.stop() with no parameters");
			lua_pushinteger( L, 0 );
			return 1;
		}
		else
		{
            CoronaLuaError(L, "audio.stop() called with unexpected parameter type");
			return 0;
		}
	}
	
	int number_of_channels = openal_player->StopChannel( which_channel );
	// -1 is an error condition. Since the Lua API starts with channel 1 instead of 0, we can return 0.
	if(number_of_channels < 0)
	{
		number_of_channels = 0; // add 1 to convert from ALmixer/C-array starts at 0 to Lua arrays start at 1
	}
	lua_pushinteger( L, number_of_channels);
	
	return 1;
}

static int
stopWithDelay( lua_State *L )
{
	/**
	 @fn stopWithDelay(lua_Integer number_of_milliseconds, optional_channel_or_table)
	 @brief Stops the playing sound in the specified amount of time.
	 @details Optional parameter will assume channel, but I will also take a table that specifies channel or source.
	 @param number_of_milliseconds When to stop the playing
	 @param optional_channel_or_table A number specifying the channel or
	 if table, supports channel= or source=.
	 @return The number of channels actually applied to.
	 @code
	 audio.stopWithDelay(5000, 1) -- channel 1
	 audio.stopWithDelay(5000, 0) -- all channels 
	 audio.stopWithDelay(5000, { channel=1 } )
	 audio.stopWithDelay(5000) 
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L );
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int number_of_milliseconds = (int) luaL_checkinteger( L, 1 );
	int which_channel = -1;
	unsigned int al_source = 0;


	if(2 == number_of_arguments)
	{
		if( lua_istable( L, 2 ) )
		{
			lua_getfield( L, 2, "channel");
			if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
			{
				which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, 2, "source");
			if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
			{
				al_source = (int) lua_tointeger( L, -1 );
				// Source will override channel if there is an inconsistency
				which_channel = openal_player->GetChannelFromSource(al_source);
			}
			lua_pop( L, 1 );
			
			// Bug 8911: Based on user feedback, passing an explicit 0 to stop all channels causes too many unexpected problems when the load/play fails
			// because people not checking values inadvertently stop all the things that are playing. So we are changing this behavior and printing a warning.
			// Note we compare using '<' and not '<=' because 1 is already subtracted from the above and we are already in ALmixer channel ranges.
			if( which_channel < 0 )
			{
				CoronaLuaWarning(L, "audio.stopWithDelay() called with 0, negative, or no channel/source. To stop all channels, call audio.stopWithDelay() with no parameters");
				lua_pushinteger( L, 0 );
				return 1;
			}
		}
		else if( LUA_TNUMBER == lua_type( L, 2) )
		{
			which_channel = (int) lua_tointeger( L, 2 );
			// Bug 8911: Based on user feedback, passing an explicit 0 to stop all channels causes too many unexpected problems when the load/play fails
			// because people not checking values inadvertently stop all the things that are playing. So we are changing this behavior and printing a warning.
			if( which_channel <= 0 )
			{
				CoronaLuaWarning(L, "audio.stopWithDelay() called with 0, negative, or no channel/source. To stop all channels, call audio.stopWithDelay() with no parameters");
				lua_pushinteger( L, 0 );
				return 1;
			}
			which_channel = which_channel - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0;
		}
		else if( LUA_TNIL == lua_type( L, 2) )
		{
			// Bug 8911: Based on user feedback, passing an explicit nil to stop all channels causes too many unexpected problems when the load/play fails
			// because people not checking values inadvertently stop all the things that are playing. So we are changing this behavior and printing a warning.
			CoronaLuaWarning(L, "audio.stopWithDelay() called with nil. To stop all channels, call audio.stopWithDelay() with no second parameter");
			lua_pushinteger( L, 0 );
			return 1;
		}
		else
		{
            CoronaLuaError(L, "audio.stopWithDelay() called with unexpected second parameter type");
            return 0;
		}
	}
		
	int number_of_channels = openal_player->ExpireChannel( which_channel, number_of_milliseconds );
	// -1 is an error condition. Since the Lua API starts with channel 1 instead of 0, we can return 0.
	if(number_of_channels < 0)
	{
		number_of_channels = 0; // add 1 to convert from ALmixer/C-array starts at 0 to Lua arrays start at 1
	}
	lua_pushinteger( L, number_of_channels );

	return 1;
}

static int
setVolume( lua_State *L )
{
	/**
	 @fn setVolume(lua_Number new_volume, lua_Table channel_source_table)
	 @brief Changes the OpenAL volume.
	 @details If passed with no table,
	 This changes the alListener's AL_GAIN which can be considered to be the master OpenAL volume.
	 This is not necessarily the hardware device's master ringer volume.
	 If passed with a table specifying channel or source, sets the alSource volume.
	 0 channel or 0 source will apply to all sources.
	 @param new_volume Ranges from 0.0 to 1.0 where 1.0 is max.
	 @return True on success, false on failure.
	 @code
	 audio.setVolume(1.0) 
	 audio.setVolume(0.3, { channel=1 } ) 
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L );
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	lua_Number new_volume = luaL_checknumber( L, 1 );
	int which_channel = -1;
	unsigned int al_source = 0;
	bool ret_flag;

	if ( (2 == number_of_arguments) && lua_istable( L, 2 ) )
	{
		lua_getfield( L, 2, "channel");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 2, "source");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			al_source = (int) lua_tointeger( L, -1 );
			// Source will override channel if there is an inconsistency
			which_channel = openal_player->GetChannelFromSource(al_source);
		}
		lua_pop( L, 1 );
		
		
		ret_flag = openal_player->SetVolumeChannel(which_channel, (float)new_volume);
	}
	else
	{
		ret_flag = openal_player->SetMasterVolume( (float)new_volume );

	}

	lua_pushboolean( L, ret_flag);	
	return 1;
}

static int
getVolume( lua_State *L )
{
	/**
	 @fn getVolume(lua_Number new_volume)
	 @brief Gets the OpenAL volume.
	 @details If passed with no table,
	 This gets the alListener's AL_GAIN which can be considered to be the master OpenAL volume.
	 This is not necessarily the hardware device's master ringer volume.
	 If passed with a table specifying channel or source, gets the alSource volume.
	 0 channel or 0 source will get an average across all sources.
	 @param new_volume Ranges from 0.0 to 1.0 where 1.0 is max.
	 @return True on success, false on failure.
	 @code
	 local master_volume = audio.getVolume() 
	 local channel1_volume = audio.getVolume( { channel = 1 } ) 
	 @endcode
	 */
	
	int number_of_arguments = lua_gettop( L );
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	lua_Number current_volume;
	int which_channel = -1;
	unsigned int al_source = 0;
	
	if ( (1 == number_of_arguments) && lua_istable( L, 1 ) )
	{
		lua_getfield( L, 1, "channel");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 1, "source");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			al_source = (int) lua_tointeger( L, -1 );
			// Source will override channel if there is an inconsistency
			which_channel = openal_player->GetChannelFromSource(al_source);
		}
		lua_pop( L, 1 );
		
		
		current_volume = openal_player->GetVolumeChannel(which_channel);
	}
	else
	{
		current_volume = openal_player->GetMasterVolume();
		
	}
	
	lua_pushnumber( L, (lua_Number)current_volume );
	return 1;
}

static int
setMaxVolume( lua_State *L )
{
	/**
	 @fn setMaxVolume(lua_Number new_volume, lua_Table channel_source_table)
	 @brief Changes the channel/source OpenAL AL_MAX_GAIN property.
	 @details If passed with no table, will apply to all channels/sources.
	 If passed with a table specifying channel or source, sets the alSource volume.
	 0 channel or 0 source will also apply to all sources.
	 @param new_volume Ranges from 0.0 to 1.0 where 1.0 is max.
	 @return True on success, false on failure.
	 @code
	 audio.setMaxVolume(0.3, { channel=1 } ) 
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L );
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	lua_Number new_volume = luaL_checknumber( L, 1 );
	int which_channel = -1;
	unsigned int al_source = 0;
	bool ret_flag;

	if ( (2 == number_of_arguments) && lua_istable( L, 2 ) )
	{
		lua_getfield( L, 2, "channel");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 2, "source");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			al_source = (int) lua_tointeger( L, -1 );
			// Source will override channel if there is an inconsistency
			which_channel = openal_player->GetChannelFromSource(al_source);
		}
		lua_pop( L, 1 );
		
	}

	ret_flag = openal_player->SetMaxVolumeChannel(which_channel, (float)new_volume);

	lua_pushboolean( L, ret_flag);	
	return 1;
}

static int
getMaxVolume( lua_State *L )
{
	/**
	 @fn getMaxVolume(lua_Number new_volume)
	 @brief Gets the master OpenAL volume.
	 @details If passed with no table,
	 This gets the alListener's AL_GAIN which can be considered to be the master OpenAL volume.
	 This is not necessarily the hardware device's master ringer volume.
	 If passed with a table specifying channel or source, gets the alSource volume.
	 0 channel or 0 source will get an average across all sources.
	 @return The channel/source volume or the average across all channels
	 @code
	 local channel1_volume = audio.getMaxVolume( { channel = 1 } ) 
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L );
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	lua_Number current_volume;
	int which_channel = -1;
	unsigned int al_source = 0;
	
	if ( (1 == number_of_arguments) && lua_istable( L, 1 ) )
	{
		lua_getfield( L, 1, "channel");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 1, "source");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			al_source = (int) lua_tointeger( L, -1 );
			// Source will override channel if there is an inconsistency
			which_channel = openal_player->GetChannelFromSource(al_source);
		}
		lua_pop( L, 1 );
	}

	current_volume = openal_player->GetMaxVolumeChannel(which_channel);

	lua_pushnumber( L, (lua_Number)current_volume );
	return 1;
}


static int
setMinVolume( lua_State *L )
{
	/**
	 @fn setMinVolume(lua_Number new_volume, lua_Table channel_source_table)
	 @brief Changes the channel/source OpenAL AL_MIN_GAIN property.
	 @details If passed with no table, will apply to all channels/sources.
	 If passed with a table specifying channel or source, sets the alSource volume.
	 0 channel or 0 source will also apply to all sources.
	 @param new_volume Ranges from 0.0 to 1.0 where 1.0 is max.
	 @return True on success, false on failure.
	 @code
	 audio.setMinVolume(0.3, { channel=1 } ) 
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L );
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	lua_Number new_volume = luaL_checknumber( L, 1 );
	int which_channel = -1;
	unsigned int al_source = 0;
	bool ret_flag;

	if ( (2 == number_of_arguments) && lua_istable( L, 2 ) )
	{
		lua_getfield( L, 2, "channel");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 2, "source");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			al_source = (int) lua_tointeger( L, -1 );
			// Source will override channel if there is an inconsistency
			which_channel = openal_player->GetChannelFromSource(al_source);
		}
		lua_pop( L, 1 );
		
	}

	ret_flag = openal_player->SetMinVolumeChannel(which_channel, (float)new_volume);

	lua_pushboolean( L, ret_flag);	
	return 1;
}

static int
getMinVolume( lua_State *L )
{
	/**
	 @fn getMinVolume(lua_Number new_volume)
	 @brief Gets the channel/source OpenAL AL_MIN_GAIN property.
	 @details If passed with no table,
	 will get average of all channels/sources
	 If passed with a table specifying channel or source, gets the alSource volume.
	 0 channel or 0 source will get an average across all sources.
	 @return The channel/source volume or the average across all channels
	 @code
	 local channel1_volume = audio.getMinVolume( { channel = 1 } ) 
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L );
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	lua_Number current_volume;
	int which_channel = -1;
	unsigned int al_source = 0;
	
	if ( (1 == number_of_arguments) && lua_istable( L, 1 ) )
	{
		lua_getfield( L, 1, "channel");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 1, "source");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			al_source = (int) lua_tointeger( L, -1 );
			// Source will override channel if there is an inconsistency
			which_channel = openal_player->GetChannelFromSource(al_source);
		}
		lua_pop( L, 1 );
	}

	current_volume = openal_player->GetMinVolumeChannel(which_channel);

	lua_pushnumber( L, (lua_Number)current_volume );
	return 1;
}


static int
fadeOut( lua_State *L )
{
	/**
	 @fn fadeOut(lua_Table table_of_parameters)
	 @brief Fades out a playing sound and stops.
	 @details Stops the playing sound in the specified amount of time and fades to AL_MIN_GAIN while doing it.
	 @tableparam channel The channel to fade out. 0 for all channels.
	 @tableparam time Time in milliseconds the fade out duration should occur over.
	 @return The number of channels actually applied to.
	 @code
	 audio.fadeOut( { channel=1, time=1000 } )
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int number_of_milliseconds = 1000; // default
	int which_channel = -1;
	unsigned int al_source = 0;

	if( lua_istable( L, 1 ) )
	{
		lua_getfield( L, 1, "channel");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 1, "source");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			al_source = (int) lua_tointeger( L, -1 );
			// Source will override channel if there is an inconsistency
			which_channel = openal_player->GetChannelFromSource(al_source);
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 1, "time");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			number_of_milliseconds = (int) lua_tointeger( L, -1 );
		}
		lua_pop( L, 1 );
	}

	int number_of_channels = openal_player->FadeOutChannel( which_channel, number_of_milliseconds );
	// -1 is an error condition. Since the Lua API starts with channel 1 instead of 0, we can return 0.
	if(number_of_channels < 0)
	{
		number_of_channels = 0; // add 1 to convert from ALmixer/C-array starts at 0 to Lua arrays start at 1
	}
	lua_pushinteger( L, number_of_channels );

	return 1;
}

static int
fade( lua_State *L )
{
	/**
	 @fn fade(lua_Table table_of_parameters)
	 @brief Fades a playing sound.
	 @details Fades the playing sound in the specified amount of time to a specified volume.
	 @tableparam channel The channel to fade out. 0 for all channels.
	 @tableparam time Time in milliseconds the fade out duration should occur over.
	 @tableparam volume The target volume. Valid ranges are 0.0 to 1.0 where 1.0 is max. 
	 @return The number of channels actually applied to.
	 @code
	 audio.fade( { channel=1, time=1000, volume=0.0 } )
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int number_of_milliseconds = 1000; // default
	int which_channel = -1;
	unsigned int al_source = 0;
	lua_Number new_volume = 0.0; // default
	
	if( lua_istable( L, 1 ) )
	{
		lua_getfield( L, 1, "channel");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 1, "source");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			al_source = (int) lua_tointeger( L, -1 );
			// Source will override channel if there is an inconsistency
			which_channel = openal_player->GetChannelFromSource(al_source);
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 1, "time");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			number_of_milliseconds = (int) lua_tointeger( L, -1 );
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 1, "volume");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			new_volume = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );
	}

	int number_of_channels = openal_player->FadeChannel( which_channel, number_of_milliseconds, (float)new_volume );
	// -1 is an error condition. Since the Lua API starts with channel 1 instead of 0, we can return 0.
	if(number_of_channels < 0)
	{
		number_of_channels = 0; // add 1 to convert from ALmixer/C-array starts at 0 to Lua arrays start at 1
	}
	lua_pushinteger( L, number_of_channels );

	return 1;
}

static int
reserveChannels( lua_State *L )
{
	/**
	 @fn reserveChannels(lua_Integer number_of_reserve_channels)
	 @brief Blocks off (reserves) the bottom number of channels.
	 @details Allows you to reserve a certain number of channels so they won't be automatically allocated to play on. This function will effectively block off a certain number of channels so they won't be automatically assigned to be played on when you call various play functions (applies to both play-channel and play-source functions since they are the same under the hood).
	 The lowest number channels will always be blocked off first.
	 For example, if there are 32 channels available, and you pass 2 into this function,
	 channels 1 and 2 will be reserved so they won't be played on automatically when you specify
	 you want to play a sound on any available channel/source. You can 
	 still play on channels 1 and 2 if you explicitly designate you want to play on their channel
	 number or source id.
	 Setting back to 0 will clear all the reserved channels so all will be available again for 
	 auto-assignment.
	 As an example, this feature can be useful if you always want your music to be on channel 1 and
	 speech on channel 2 and you don't want sound effects to ever occupy those channels. This allows
	 you to build in certain assumptions about your code, perhaps for deciding which data you want
	 to analyze in a data callback.
	 Specifying the number of reserve channels to the maximum number of channels will effectively
	 disable auto-assignment.	 
	 @param number_of_reserve_channels The number of channels/sources to reserve.
	 Or pass -1 or no parameter to find out how many channels are currently reserved. 
	 @return Returns the number of currently reserved channels.	 
	 @code
	 audio.reserveChannels( 2 ) 
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L );
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();

    if (openal_player != NULL)
    {
        int number_of_reserve_channels = -1;

        if( number_of_arguments > 0 )
        {
            number_of_reserve_channels = (int) luaL_checkinteger( L, 1 );
        }

        int return_number_of_channels = openal_player->ReserveChannels( number_of_reserve_channels );
        lua_pushinteger( L, return_number_of_channels );

        return 1;
    }
    else
    {
        CoronaLuaWarning(L , "audio.reserveChannels() called with no valid audio player");
        
        return 0;
    }
}

static int
findFreeChannel( lua_State *L )
{
	/**
	 @fn findFreeChannel(lua_Integer start_channel)
	 @brief Will look for a channel available for playback.
	 @details Given a start channel number, the search will increase to the highest channel 
	 until it finds one available. Does not include reserved channels.
	 @param start_channel The channel number you want to start looking at. 
	 0 or no parameter start at the lowest possible value.
	 @return A channel available or 0 if none could be found. Also return source id or 0.
	 @code
	 local available_channel = audio.findFreeChannel() 
	 local available_channel_from_3 = audio.findFreeChannel(3) 
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L ); 
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int start_channel = -1;
	
	if( number_of_arguments > 0 )
	{
		start_channel = (int) luaL_checkinteger( L, 1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
	}

	int free_channel = openal_player->FindFreeChannel( start_channel );
	unsigned int free_source = 0;
	if(-1 == free_channel)
	{
		free_source = 0;
	}
	else
	{
		free_source = openal_player->GetSourceFromChannel(free_channel);
	}
	
	free_channel++; // add 1 to convert from ALmixer/C-array starts at 0 to Lua array starts at 1
	lua_pushinteger( L, free_channel );
	lua_pushinteger( L, free_source ); // return the source that the audio is played on
	
	
	return 2;
}

static int
getSourceFromChannel( lua_State *L )
{
	/**
	 @fn getSourceFromChannel(lua_Integer which_channel)
	 @brief OpenAL source id for the corresponding channel number.
	 @details This function will look up the OpenAL source id for the corresponding channel number.
	 @param param which_channel The channel which you want to find the corresponding OpenAL source id for.
	 If 0 was specified, an available source for playback will be returned. No parameter means 0.
	 @return  The OpenAL source id corresponding to the channel. 0 if you specified an illegal channel value.
	 Or 0 if you specified 0 and no sources were currently available.
	 @code
	 local al_source = audio.getSourceFromChannel(1) 
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L ); 
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int which_channel = -1;
	
	if( number_of_arguments > 0 )
	{
		which_channel = (int) luaL_checkinteger( L, 1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
	}

	unsigned int al_source = openal_player->GetSourceFromChannel( which_channel );
	lua_pushinteger( L, al_source );

	return 1;
}

static int
getChannelFromSource( lua_State *L )
{
	/**
	 @fn getSourceFromChannel(lua_Integer al_source)
	 @brief This function will look up the channel for the corresponding source.
	 @details This function will look up the channel for the corresponding source.
	 @return  The OpenAL source id corresponding to the channel. 0 if you specified an illegal channel value.
	 @param al_source The source id you want to find the corresponding channel number for.
	 Or 0 if you specified 0 and no sources were currently available.
	 If 0 or no parameter is supplied, it will try to return the first channel not in use. 
	 @code
	 local channel = audio.getSourceFromChannel(al_source) 
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L ); 
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	unsigned int al_source = 0;
	
	if( number_of_arguments > 0 )
	{
		al_source = (int) luaL_checkinteger( L, 1 );
	}

	int which_channel = openal_player->GetChannelFromSource( al_source );
	which_channel++; // add 1 to convert from ALmixer/C-array starts at 0 to Lua array starts at 1
	lua_pushinteger( L, which_channel );

	return 1;
}

static int
getDuration( lua_State *L )
{
	/**
	 @fn getDuration(ALmixer_Data* sound_handle)
	 @brief Returns the total time in milliseconds of the audio resource.
	 @details Returns the total time in milliseconds of the audio resource. If the total length cannot be determined, -1 will be returned.
	 @return  Returns the total time in milliseconds of the audio resource.
	 @param sound_handle The handle of the audio you want to know the duration of.
	 @code
	 local total_time = audio.getDuration( backgroundMusic )
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L ); 
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	ALmixer_Data* sound_data = NULL; 
	if( 1 == number_of_arguments && lua_islightuserdata( L , 1 ) )
	{
		sound_data = (ALmixer_Data*)lua_touserdata( L, 1 );
	}

	if (! isValidSoundData(L, sound_data, "audio.getDuration"))
	{
		return 1;
	}
	
	int total_time = openal_player->GetTotalTime( sound_data );
	lua_pushinteger( L, total_time );

	return 1;
}


static int
seek( lua_State *L )
{
	/**
	 @fn rewind()
	 @brief Seeks channel/source or data.
	 @details Seeks channel/source or data.
	 @return True or false
	 @param al_source The source id you want to find the corresponding channel number for.
	 @code
	 audio.seek()
	 audio.seek(3000, channel)
	 audio.seek(3000, sound_handle)
	 audio.seek(3000,  {channel=1} )
	 audio.seek(3000,  {handle=sound_handle} )	 
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L ); 
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	ALmixer_Data* sound_data = NULL;
	int which_channel = -1;
	unsigned int al_source = 0;
	int ret_val;
	unsigned int number_of_milliseconds = 0;
	if( 0 == number_of_arguments )
	{
		/*
		ret_val = openal_player->RewindChannel(-1);
		if( ret_val <= 0 )
		{
			lua_pushboolean( L, false );
		}
		else
		{
			lua_pushboolean( L, true );
		}
		 return 1;
		 */
        CoronaLuaError(L , "audio.seek() requires parameters");
		return 0;
	}

	number_of_milliseconds = (int) luaL_checkinteger( L, 1 );
			
	
	if ( (2 == number_of_arguments) && lua_istable( L, 2 ) )
	{
		lua_getfield( L, 2, "channel");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 2, "source");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			al_source = (int) lua_tointeger( L, -1 );
			// Source will override channel if there is an inconsistency
			which_channel = openal_player->GetChannelFromSource(al_source);
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 2, "handle");
		if ( lua_islightuserdata( L , -1 ) )
		{
			sound_data = (ALmixer_Data*)lua_touserdata( L, -1 );
		}
		lua_pop( L, 1 );
		
	}
	else if ( (2 == number_of_arguments) && lua_isnumber( L, 2 ) )
	{
		which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
	}
	else if ( (2 == number_of_arguments) && lua_islightuserdata( L, 2 ) )
	{
		sound_data = (ALmixer_Data*)lua_touserdata( L, -1 );
	}
		
	if( NULL != sound_data )
	{
		// Check to see that we have a valid pointer
		if (! isValidSoundData(L, sound_data, "audio.seek"))
		{
			return 1;
		}

		ret_val = openal_player->SeekData( sound_data, number_of_milliseconds );
		if( ret_val == 0 )
		{
			lua_pushboolean( L, false );
		}
		else
		{
			lua_pushboolean( L, true );
		}
		
	}
	else
	{
		ret_val = openal_player->SeekChannel( which_channel, number_of_milliseconds );
		if( ret_val <= 0 )
		{
			lua_pushboolean( L, false );
		}
		else
		{
			lua_pushboolean( L, true );
		}
	}

	return 1;
}

static int
rewindAudio( lua_State *L )
{
	/**
	 @fn rewind()
	 @brief Rewind channel/source or data.
	 @details Rewind channel/source or data.
	 @return True or false
	 @param al_source The source id you want to find the corresponding channel number for.
	 @code
	 audio.rewind()
	 audio.rewind(channel)
	 audio.rewind(sound_handle)
	 audio.rewind( {channel=1} )
	 audio.rewind( {handle=sound_handle} )	 
	 @endcode
	 */
	int number_of_arguments = lua_gettop( L ); 
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	ALmixer_Data* sound_data = NULL;
	int which_channel = -1;
	unsigned int al_source = 0;
	int ret_val;
	if( 0 == number_of_arguments )
	{
		ret_val = openal_player->RewindChannel(-1);
		if( ret_val <= 0 )
		{
			lua_pushboolean( L, false );
		}
		else
		{
			lua_pushboolean( L, true );
		}

		return 1;
	}

	if ( (1 == number_of_arguments) && lua_istable( L, 1 ) )
	{
		lua_getfield( L, 1, "channel");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 1, "source");
		if ( ( ! lua_isnil( L, -1 ) ) && ( lua_isnumber( L, -1 ) ) )
		{
			al_source = (int) lua_tointeger( L, -1 );
			// Source will override channel if there is an inconsistency
			which_channel = openal_player->GetChannelFromSource(al_source);
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, 1, "handle");
		if ( lua_islightuserdata( L , -1 ) )
		{
			sound_data = (ALmixer_Data*)lua_touserdata( L, -1 );
		}
		lua_pop( L, 1 );
		
	}
	else if ( (1 == number_of_arguments) && lua_isnumber( L, 1 ) )
	{
		which_channel = (int) lua_tointeger( L, -1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0
	}
	else if ( (1 == number_of_arguments) && lua_islightuserdata( L, 1 ) )
	{
		sound_data = (ALmixer_Data*)lua_touserdata( L, -1 );
	}
	else
	{
        CoronaLuaError(L , "audio.rewind() called with unexpected parameter type");
		return 0;
	}
	
	
	if( NULL != sound_data )
	{
		// Check to see that we have a valid pointer
		if (! isValidSoundData(L, sound_data, "audio.rewind"))
		{
			return 1;
		}

		ret_val = openal_player->RewindData( sound_data );
		if( ret_val == 0 )
		{
			lua_pushboolean( L, false );
		}
		else
		{
			lua_pushboolean( L, true );
		}
		
	}
	else
	{
		ret_val = openal_player->RewindChannel( which_channel );
		if( ret_val <= 0 )
		{
			lua_pushboolean( L, false );
		}
		else
		{
			lua_pushboolean( L, true );
		}
		
	}

	return 1;
}

static int
isChannelActive( lua_State *L )
{
	/**
	 @fn isChannelActive(lua_Integer which_channel)
	 @brief Returns true if the specified channel is currently playing or paused.
	 @details Returns true if the specified channel is currently playing or paused.
	 @param which_channel The channel you want to know about.
	 @return True for active, false otherwise
	 @code
	 local result = audio.isChannelActive(which_channel) 
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int which_channel = (int) luaL_checkinteger( L , 1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0;
	if( which_channel < 0 )
	{
		lua_pushboolean( L, false );
		return 1;
	}

	int ret_val = openal_player->IsActiveChannel( which_channel );
	if(ret_val < 0)
	{
		lua_pushboolean( L, false );
	}
	lua_pushboolean( L, ret_val );

	return 1;
}

static int
isSourceActive( lua_State *L )
{
	/**
	 @fn isSourceActive(lua_Integer al_source)
	 @brief Returns true if the specified source is currently playing or paused.
	 @details Returns true if the specified source is currently playing or paused.
	 @param al_source The source you want to know about.
	 @return True for active, false otherwise
	 @code
	 local result = audio.isSourceActive(al_source) 
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	unsigned int al_source = (int) luaL_checkinteger( L , 1 );
	if( al_source == 0 )
	{
		lua_pushboolean( L, false );
		return 1;
	}

	int ret_val = openal_player->IsActiveSource( al_source );
	if(ret_val < 0)
	{
		lua_pushboolean( L, false );
	}
	lua_pushboolean( L, ret_val );

	return 1;
}

static int
isChannelPlaying( lua_State *L )
{
	/**
	 @fn isChannelPlaying(lua_Integer which_channel)
	 @brief Returns true if the specified channel is currently playing.
	 @details Returns true if the specified channel is currently playing.
	 @param which_channel The channel you want to know about.
	 @return True for playing, false otherwise
	 @code
	 local result = audio.isChannelPlaying(which_channel) 
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int which_channel = (int) luaL_checkinteger( L , 1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0;
	if( which_channel < 0 )
	{
		lua_pushboolean( L, false );
		return 1;
	}

	int ret_val = openal_player->IsPlayingChannel( which_channel );
	if(ret_val < 0)
	{
		lua_pushboolean( L, false );
	}
	lua_pushboolean( L, ret_val );

	return 1;
}

static int
isSourcePlaying( lua_State *L )
{
	/**
	 @fn isSourceActive(lua_Integer al_source)
	 @brief Returns true if the specified source is currently playing.
	 @details Returns true if the specified source is currently playing.
	 @param al_source The source you want to know about.
	 @return True for playing, false otherwise
	 @code
	 local result = audio.isSourcePlaying(al_source) 
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	unsigned int al_source = (int) luaL_checkinteger( L , 1 );
	if( al_source == 0 )
	{
		lua_pushboolean( L, false );
		return 1;
	}

	int ret_val = openal_player->IsPlayingSource( al_source );
	if(ret_val < 0)
	{
		lua_pushboolean( L, false );
	}
	lua_pushboolean( L, ret_val );

	return 1;
}

static int
isChannelPaused( lua_State *L )
{
	/**
	 @fn isChannelPaused(lua_Integer which_channel)
	 @brief Returns true if the specified channel is currently paused.
	 @details Returns true if the specified channel is currently paused.
	 @param which_channel The channel you want to know about.
	 @return True for paused, false otherwise
	 @code
	 local result = audio.isChannelPaused(which_channel) 
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int which_channel = (int) luaL_checkinteger( L , 1 ) - 1; // subtract 1 to convert from Lua arrays start at 1 to ALmixer/C-array starts at 0;
	if( which_channel < 0 )
	{
		lua_pushboolean( L, false );
		return 1;
	}

	int ret_val = openal_player->IsPausedChannel( which_channel );
	if(ret_val < 0)
	{
		lua_pushboolean( L, false );
	}
	lua_pushboolean( L, ret_val );

	return 1;
}

static int
isSourcePaused( lua_State *L )
{
	/**
	 @fn isSourceActive(lua_Integer al_source)
	 @brief Returns true if the specified source is currentlypaused.
	 @details Returns true if the specified source is currently paused.
	 @param al_source The source you want to know about.
	 @return True for paused, false otherwise
	 @code
	 local result = audio.isSourcePaused(al_source) 
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	unsigned int al_source = (int) luaL_checkinteger( L , 1 );
	if( al_source == 0 )
	{
		lua_pushboolean( L, false );
		return 1;
	}

	int ret_val = openal_player->IsPausedSource( al_source );
	if(ret_val < 0)
	{
		lua_pushboolean( L, false );
	}
	lua_pushboolean( L, ret_val );

	return 1;
}

static int
freeChannels( lua_State *L )
{
	/**
	 @fn freeChannels()
	 @brief Returns the number of channels that are currently available for playback (not playing, not paused).
	 @details Returns the number of channels that are currently available for playback (not playing, not paused).
	 @return Returns the count
	 @code
	 local result = audio.freeChannels
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int ret_val = openal_player->CountAllFreeChannels();
	lua_pushinteger( L, ret_val );

	return 1;
}

static int
unreservedFreeChannels( lua_State *L )
{
	/**
	 @fn unreservedFreeChannels()
	 @brief Returns the number of channels that are currently available for playback (not playing, not paused),
	 excluding the channels that have been reserved.
	 @details Returns the number of channels that are currently available for playback (not playing, not paused),
	 excluding the channels that have been reserved.
	 @return Returns the count
	 @code
	 local result = audio.unreservedFreeChannels
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int ret_val = openal_player->CountUnreservedFreeChannels();
	lua_pushinteger( L, ret_val );

	return 1;
}

static int
usedChannels( lua_State *L )
{
	/**
	 @fn usedChannels()
	 @brief Returns the number of channels that are currently in use (playing/paused).
	 @details Returns the number of channels that are currently in use (playing/paused).
	 @return Returns the count
	 @code
	 local result = audio.usedChannels 
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int ret_val = openal_player->CountAllUsedChannels();
	lua_pushinteger( L, ret_val );

	return 1;
}

static int
unreservedUsedChannels( lua_State *L )
{
	/**
	 @fn usedChannels()
	 @brief Returns the number of channels that are currently in use (playing/paused), excluding the channels that have been reserved.
	 @details Returns the number of channels that are currently in use (playing/paused), excluding the channels that have been reserved.
	 @return Returns the count
	 @code
	 local result = audio.unreservedUsedChannels
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int ret_val = openal_player->CountUnreservedUsedChannels();
	lua_pushinteger( L, ret_val );

	return 1;
}

static int
totalChannels( lua_State *L )
{
	/**
	 @fn countTotalChannels()
	 @brief Returns the number of channels. Should be 32 for now, but may be subject to change in the future.
	 @details Returns the number of channels. Should be 32 for now, but may be subject to change in the future.
	 @return Returns the count
	 @code
	 local result = audio.totalChannels
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int ret_val = openal_player->CountTotalChannels();
	lua_pushinteger( L, ret_val );

	return 1;
}

static int
reservedChannels( lua_State *L )
{
	/**
	 @fn reservedChannels()
	 @brief Returns the number of reserved channels set by audio.reserveChannels.
	 @details Returns the number of reserved channels set by audio.reserveChannels.
	 @return Returns the count
	 @code
	 local result = audio.reservedChannels
	 @endcode
	 */
	PlatformOpenALPlayer* openal_player = PlatformOpenALPlayer::SharedInstance();
	int ret_val = openal_player->CountReservedChannels();
	lua_pushinteger( L, ret_val );

	return 1;
}
	
static int
supportsSessionProperty( lua_State *L )
{
#ifdef Rtt_IPHONE_ENV
	lua_pushboolean( L, true);
#else
	lua_pushboolean( L, false);
#endif
	return 1;
}
	

static int
setSessionProperty( lua_State *L )
{
	
	// audio.setSessionProperty( audio.kSomeCategory, some_value, optional_type )
#ifdef Rtt_IPHONE_ENV
	int number_of_args = lua_gettop( L );
	if ( (number_of_args < 2) || (number_of_args > 3) )
	{
        CoronaLuaError(L , "audio.setSessionProperty() called with incorrect number of arguments");
		return 0;
	}
	
	if ( number_of_args == 3 ) 
	{
		const char* param_type = luaL_checkstring( L, 3 );
		AudioSessionPropertyID the_property = (AudioSessionPropertyID)luaL_checkinteger( L, 1 );
		if ( 0 == Rtt_StringCompareNoCase( "Float32", param_type ) )
		{
			IPhoneAudioSessionManager::GetInstance()->SetProperty( the_property, (Float32)luaL_checknumber( L, 2) );
		}
		else if ( 0 == Rtt_StringCompareNoCase( "Float64", param_type ) )
		{
			IPhoneAudioSessionManager::GetInstance()->SetProperty( the_property, (Float64)luaL_checknumber( L, 2) );
		}
		else if ( 0 == Rtt_StringCompareNoCase( "bool", param_type ) )
		{
			IPhoneAudioSessionManager::GetInstance()->SetProperty( the_property, (bool)luaL_checkinteger( L, 2) );
		}
		else
		{
			// Assuming UInt32. I could try AudioSessionGetPropertySize(), but it doesn't tell me if the type is int or float.
			IPhoneAudioSessionManager::GetInstance()->SetProperty( the_property, (UInt32)luaL_checkinteger( L, 2) );
		}
			
	}
	else
	{
		// Ugh, the type can be UInt32, Float64, Float32, and possibly even boolean or even other things.
		// I've attempted to deal with different size types for different architectures, but the the solution may be imperfect. I'm trying to not bring the entire Apple #include stack and Obj-C runtime into this file.
		AudioSessionPropertyID the_property = (AudioSessionPropertyID)luaL_checkinteger( L, 1 );
		switch ( IPhoneAudioSessionManager::GetInstance()->GetPropertyType( the_property ) )
		{
			// Assuming FLT maps to Float32
			case RTT_OBJCTYPE_UINT_FLT:
			{
				IPhoneAudioSessionManager::GetInstance()->SetProperty( the_property, (Float32)luaL_checknumber( L, 2) );
				break;
			}
				// Assuming DBL maps to Float64
			case RTT_OBJCTYPE_UINT_DBL:
			{
				IPhoneAudioSessionManager::GetInstance()->SetProperty( the_property, (Float64)luaL_checknumber( L, 2) );
				break;
			}
			case RTT_OBJCTYPE_UINT_BOOL:
			{
				IPhoneAudioSessionManager::GetInstance()->SetProperty( the_property, (bool)luaL_checkinteger( L, 2) );
				break;
			}
			default:
			{
				IPhoneAudioSessionManager::GetInstance()->SetProperty( the_property, (UInt32)luaL_checkinteger( L, 2) );
				break;
			}
		}
	}
	
#else
	
#endif
	return 0;
}
	
static int
getSessionProperty( lua_State *L )
{
#ifdef Rtt_IPHONE_ENV
	// Ugh, the type can be UInt32, Float64, Float32, and possibly even boolean or even other things.
	// I've attempted to deal with different size types for different architectures, but the the solution may be imperfect. I'm trying to not bring the entire Apple #include stack and Obj-C runtime into this file.
	AudioSessionPropertyID the_property = (AudioSessionPropertyID)luaL_checkinteger( L, 1 );
	switch ( IPhoneAudioSessionManager::GetInstance()->GetPropertyType( the_property ) )
	{
		
		// Assuming FLT maps to Float32
		case RTT_OBJCTYPE_UINT_FLT:
		{
			lua_pushnumber( L, (lua_Number)IPhoneAudioSessionManager::GetInstance()->GetPropertyFloat32( the_property ) );
			return 1;
		}
		// Assuming DBL maps to Float64
		case RTT_OBJCTYPE_UINT_DBL:
		{
			lua_pushnumber( L, (lua_Number)IPhoneAudioSessionManager::GetInstance()->GetPropertyFloat64( the_property ) );
			return 1;
		}
		case RTT_OBJCTYPE_UINT_BOOL:
		{
			lua_pushboolean( L, IPhoneAudioSessionManager::GetInstance()->GetPropertyBool( the_property ) );
			return 1;
		}
		default:
		{
			lua_pushinteger( L, (lua_Integer)IPhoneAudioSessionManager::GetInstance()->GetPropertyUInt32( the_property ) );
			return 1;
		}
	}

	return 1;
#else
	// Always returns nil
	return 0;
#endif
}

static int
valueForKey( lua_State *L )
{
	const char* table_key = luaL_checkstring( L, 2 );		

	if ( 0 == strcmp( "freeChannels", table_key ) )  // read-only
	{
		return freeChannels( L );
	}
	else if ( 0 == strcmp( "unreservedFreeChannels", table_key ) )  // read-only
	{
		return unreservedFreeChannels( L );
	}
	else if ( 0 == strcmp( "usedChannels", table_key ) )  // read-only
	{
		return usedChannels( L );
	}
	else if ( 0 == strcmp( "unreservedUsedChannels", table_key ) )  // read-only
	{
		return unreservedUsedChannels( L );
	}
	else if ( 0 == strcmp( "totalChannels", table_key ) )  // read-only
	{
		return totalChannels( L );
	}
	else if ( 0 == strcmp( "reservedChannels", table_key ) )  // read-only
	{
		return reservedChannels( L );
	}
	else if ( 0 == strcmp( "supportsSessionProperty", table_key ) )  // read-only
	{
		return supportsSessionProperty( L );
	}
	
	else
	{
		// Error started getting annoying with Audio Session keys because I want automatic nil.
//		return luaL_error( L , "Unsupported key: %s in audio library", table_key);
		CoronaLuaError(L, "Unsupported key: %s in audio library", table_key);
		return 0;
	}

	return 0;
}


#ifdef Rtt_IPHONE_ENV

typedef struct audio_Enum  
{
  const char* stringName;
  int value;
} audio_Enum;


static void audio_initenum(lua_State* lua_state, const audio_Enum* audio_enums)
{
	for( ; audio_enums->stringName; audio_enums++) 
	{
		lua_pushstring(lua_state, audio_enums->stringName);
		lua_pushinteger(lua_state, audio_enums->value);
		lua_settable(lua_state, -3);
	}
}


static const audio_Enum s_audioEnum[] = {
	{ "ActiveMode",					kRtt_Fake_AudioSessionProperty_SetActiveMode },
	{ "MixMode",						kAudioSessionProperty_AudioCategory },
	{ "OverrideMixWithOthers",			kAudioSessionProperty_OverrideCategoryMixWithOthers },
	{ "OtherAudioIsPlaying",			kAudioSessionProperty_OtherAudioIsPlaying },
	{ "OtherMixableAudioShouldDuck",	kAudioSessionProperty_OtherMixableAudioShouldDuck },

	{ "AmbientMixMode",		kAudioSessionCategory_AmbientSound },
	{ "SoloAmbientMixMode",	kAudioSessionCategory_SoloAmbientSound },
	{ "MediaPlaybackMixMode",		kAudioSessionCategory_MediaPlayback },
	{ "RecordAudioMixMode",		kAudioSessionCategory_RecordAudio },
	{ "PlayAndRecordMixMode",		kAudioSessionCategory_PlayAndRecord },
	{ "AudioProcessingMixMode",	kAudioSessionCategory_AudioProcessing },

	{ 0, 0 }
};

#endif // Rtt_IPHONE_ENV
	
#endif // Rtt_USE_ALMIXER


void
LuaLibOpenAL::Initialize( lua_State *L )
{
	const luaL_Reg kVTable[] =
	{
#ifdef Rtt_USE_ALMIXER
		{ "loadSound", loadSoundAll },
		{ "loadStream", loadSoundStream },
		{ "dispose", freeData },
		{ "play", playChannelTimed },
		{ "pause", pauseChannel },
		{ "resume", resumeChannel },
		{ "stop", stopChannel },
		{ "stopWithDelay", stopWithDelay },
		{ "setVolume", setVolume },
		{ "getVolume", getVolume },
		{ "setMaxVolume", setMaxVolume },
		{ "getMaxVolume", getMaxVolume },
		{ "setMinVolume", setMinVolume },
		{ "getMinVolume", getMinVolume },
		{ "fadeOut", fadeOut },
		{ "fade", fade },
		{ "reserveChannels", reserveChannels },
		{ "findFreeChannel", findFreeChannel },
		{ "getSourceFromChannel", getSourceFromChannel },
		{ "getChannelFromSource", getChannelFromSource },
		{ "getDuration", getDuration },
		{ "seek", seek },
		{ "rewind", rewindAudio },

		{ "isChannelActive", isChannelActive },
		{ "isSourceActive", isSourceActive },
		{ "isChannelPlaying", isChannelPlaying },
		{ "isSourcePlaying", isSourcePlaying },
		{ "isChannelPaused", isChannelPaused },
		{ "isSourcePaused", isSourcePaused },

		// Note: All these count functions are not documented.
		// The public documentation suggests using read-only dot-properties.
		// However, these were trivial to implement also as functions and are exposed
		// in case power users need to store functions for callbacks 
		// or other functional-style programming needs.
		{ "countFreeChannels", freeChannels },
		{ "countUnreservedFreeChannels", unreservedFreeChannels },
		{ "countUsedChannels", usedChannels },
		{ "countUnreservedUsedChannels", unreservedUsedChannels },
		{ "countTotalChannels", totalChannels },
		{ "countReservedChannels", reservedChannels },
		
		// Not OpenAL specific APIs, but putting under audio domain.
		{ "setSessionProperty", setSessionProperty },
		{ "getSessionProperty", getSessionProperty },
			
		
#endif // Rtt_USE_ALMIXER
		{ NULL, NULL }
	};

	const luaL_Reg kVTable_m[] = 
	{
#ifdef Rtt_USE_ALMIXER
		{ "__index", valueForKey },
#endif // Rtt_USE_ALMIXER
		{ NULL, NULL }
	};
	
	luaL_register( L, "audio", kVTable );
	// To provide access to queries via dot-notation (like audio.freeChannels)
	// I need a metatable on the main audio table.
	// And that metatable needs to have the __index metamethod defined.
	luaL_register( L, "metatable.audio", kVTable_m );
	lua_setmetatable( L, -2 );

#ifdef Rtt_USE_ALMIXER
	#ifdef Rtt_IPHONE_ENV
		audio_initenum( L, s_audioEnum );
	#endif
#endif	
	
	lua_pop( L, 1 ); // pop "almixer" table
	
	
#ifdef Rtt_USE_ALMIXER
	// Open basic OpenAL bindings
	lua_pushcfunction( L, luaopen_luaal );
	lua_pushstring( L, "al" );
	lua_call( L, 1, 0 );
#endif
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


