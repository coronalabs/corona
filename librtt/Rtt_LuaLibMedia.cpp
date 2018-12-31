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

#include "Rtt_LuaLibMedia.h"

#include "Rtt_LuaContext.h"
#include "Display/Rtt_LuaLibDisplay.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_PlatformAudioPlayer.h"
#include "Rtt_PlatformAudioRecorder.h"
#include "Rtt_PlatformEventSound.h"
#include "Rtt_PlatformImageProvider.h"
#include "Rtt_PlatformMediaProviderBase.h"
#include "Rtt_PlatformVideoPlayer.h"
#include "Rtt_PlatformVideoProvider.h"

#include "Rtt_Runtime.h"
#include "Core/Rtt_String.h"

#include <string.h>

#include "CoronaLua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kEventSound[] = "media.EventSound";

static int
gcEventSound( lua_State *L )
{
	PlatformEventSound ** pSoundID = (PlatformEventSound **)luaL_checkudata( L, 1, kEventSound );
	if ( pSoundID )
	{
		const MPlatform& platform = LuaContext::GetRuntime( L )->Platform();
		platform.ReleaseEventSound( * pSoundID );
	}

	return 0;
}

static PlatformEventSound *
createEventSound( lua_State *L, const MPlatform& platform )
{
	PlatformEventSound * soundID = NULL;
	if ( lua_isstring( L, 1 ) )
	{
		String filePath( & platform.GetAllocator() );
		int nextArg = luaL_initpath( L, platform, 1, filePath );
		// platform.PathForFile( lua_tostring( L, 1 ), MPlatform::kResourceDir, MPlatform::kDefaultPathFlags, filePath );

		if ( filePath.GetString()
			&& (soundID = platform.CreateEventSound( LuaContext::GetRuntime( L )->VMContext().LuaState(), filePath.GetString() )) )
		{
			// Store callback *prior* to pushing result
			soundID->SetListenerRef( nextArg );

			Lua::PushUserdata( L, soundID, kEventSound );
		}
		else
		{
			CoronaLuaWarning( L, "failed to create event sound '%s'", lua_tostring( L, 1 ) );
		}
	}

	if ( ! soundID )
	{
		lua_pushnil( L );
	}

	return soundID;
}

static int
playEventSound( lua_State *L )
{
	PlatformEventSound * soundID;
	const MPlatform & platform = LuaContext::GetRuntime( L )->Platform();
	bool isUserData = lua_isuserdata( L, 1 );
	if ( isUserData )
	{
		soundID = * (PlatformEventSound **)lua_touserdata( L, 1 );
		if ( soundID )
		{
			soundID->SetListenerRef( 2 );
		}

		lua_pushvalue( L, 1 );
	}
	else
	{
		soundID = createEventSound( L, platform );
	}

	if ( soundID )
	{
		platform.PlayEventSound( soundID );
	}

	return 1;
}

static int
newEventSound( lua_State *L )
{
	const MPlatform& platform = LuaContext::GetRuntime( L )->Platform();
	createEventSound( L, platform );
	return 1;
}

// media.playSound( file [, baseDir] [, isLooping OR onComplete] )
static int
playSound( lua_State *L )
{
	const MPlatform& platform = LuaContext::GetRuntime( L )->Platform();
	PlatformAudioPlayer* player = platform.GetAudioPlayer( LuaContext::GetRuntime( L )->VMContext().LuaState() );

	int nextArg = 2;

	if ( lua_isstring( L, 1 ) )
	{
		String filePath( & platform.GetAllocator() );
		nextArg = luaL_initpath( L, platform, 1, filePath );
		if (filePath.IsEmpty())
		{
			CoronaLuaWarning(L, "media.playSound() could not load sound from path '%s'", lua_tostring( L, 1 ) );
			return 0;
		}
		player->Load( filePath.GetString() );
	}

	if ( lua_isboolean( L, nextArg ) )
	{
		// Bug 762 looks a lot like bug 299. So let's make sure the callback is cleared if the user doesn't specify one.
		player->CleanupNotifier();
		player->SetLooping( lua_toboolean( L, nextArg ) );
	}
	else 
	{
		// To prevent looping bug 299 (play with looping, stop, play without looping but still get looping) 
		// we assume if looping is not explicitly specified, we disable it.
		player->SetLooping( false );

		// Store callback if one was provided.
		// If a callback was not provided, then don't blow away the last assigned callback.
		if (lua_gettop(L) >= 2)
		{
			player->SetListenerRef( nextArg );
		}
	}


	player->Play();

	return 0;
}

static int
pauseSound( lua_State *L )
{
	const MPlatform& platform = LuaContext::GetRuntime( L )->Platform();
	PlatformAudioPlayer* player = platform.GetAudioPlayer( LuaContext::GetRuntime( L )->VMContext().LuaState() );
	player->Pause();

	return 0;
}

static int
stopSound( lua_State *L )
{
	const MPlatform& platform = LuaContext::GetRuntime( L )->Platform();
	PlatformAudioPlayer* player = platform.GetAudioPlayer( LuaContext::GetRuntime( L )->VMContext().LuaState() );
	player->Stop();

	return 0;
}

static int
getSoundVolume( lua_State *L )
{
	const MPlatform & platform = LuaContext::GetRuntime( L )->Platform();
	PlatformAudioPlayer * player = platform.GetAudioPlayer( LuaContext::GetRuntime( L )->VMContext().LuaState() );
	
	Rtt_Real volume = player->GetVolume();
	lua_pushnumber( L, volume );
	
	return 1;
}

static int
setSoundVolume( lua_State *L )
{
	Rtt_Real volume = luaL_toreal( L, 1 );
	
	const MPlatform & platform = LuaContext::GetRuntime( L )->Platform();
	PlatformAudioPlayer * player = platform.GetAudioPlayer( LuaContext::GetRuntime( L )->VMContext().LuaState() );
	if ( volume < 0.0f )
		volume = 0.0f;
	if ( volume > 1.0f )
		volume = 1.0f;
	
	player->SetVolume( volume );
	
	return 0;
}

static const char kGenericMediaSources[LuaLibMedia::kNumSources + 1] = "0";

const char *
LuaLibMedia::GetLocalOrRemotePath( lua_State *L, int& nextArg, String& filePath, bool& isRemote )
{
	const MPlatform& platform = LuaContext::GetPlatform( L );

	const char *path = lua_tostring( L, nextArg );

	++nextArg;

	isRemote = false;
	MPlatform::Directory baseDir = MPlatform::kResourceDir;
	if ( lua_islightuserdata( L, nextArg ) )
	{
		void *p = lua_touserdata( L, nextArg );
		++nextArg;

		if ( EnumExistsForUserdata( kGenericMediaSources, p, LuaLibMedia::kNumSources ) )
		{
			isRemote = true;
		}
		else
		{
			Rtt_ASSERT( EnumExistsForUserdata( LuaLibSystem::Directories(), p, MPlatform::kNumDirs ) );

			baseDir = (MPlatform::Directory)EnumForUserdata(
				 LuaLibSystem::Directories(),
				 p,
				 MPlatform::kNumDirs,
				 MPlatform::kResourceDir );
		}
	}

	if ( ! isRemote )
	{
		platform.PathForFile( path, baseDir, MPlatform::kDefaultPathFlags, filePath );
		path = filePath.GetString();
	}

	return path;
}


static int
playVideo( lua_State *L )
{
	if ( lua_isstring( L, 1 ) )
	{
		Runtime& runtime = * LuaContext::GetRuntime( L );
		const MPlatform& platform = runtime.Platform();
		PlatformVideoPlayer * player = platform.GetVideoPlayer( runtime.VMContext().LuaState() );

		bool isRemote = false;
		String filePath( runtime.GetAllocator() );
		int nextArg = 1;
		const char *path = LuaLibMedia::GetLocalOrRemotePath( L, nextArg, filePath, isRemote );

		if ( path == NULL ) 
		{
			CoronaLuaWarning(L, "media.playVideo() could not load video from path '%s'", lua_tostring( L, 1 ) );
		}
		else 
		{
			player->Load( path, isRemote );
			player->SetProperty( PlatformVideoPlayer::kShowControls, lua_toboolean(L, nextArg++) ? true : false );

			// Store callback
			player->SetListenerRef( nextArg );
			
			// BeginSession causes the runtime to suspend which puts the audio system into a hibernation mode.
			// We don't want the audio system to necessarily suspend when the movie is playing.
			player->SetProperty( PlatformModalInteraction::kDoNotSuspend, true );
			player->BeginSession( runtime );
			player->Play();
		}
	}

	return 0;
}

static const char kMediaSources[PlatformMediaProviderBase::kNumSources + 1] = "012";

static int
showImageUI(int source, const char *path, lua_State *L, int listenerRef)
{
	Runtime& runtime = * LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime.Platform();
	PlatformImageProvider* provider = platform.GetImageProvider( runtime.VMContext().LuaState() );
	if (listenerRef != LUA_NOREF)
	{
		provider->RawSetListenerRef( listenerRef );
	}

	// BeginSession causes the runtime to suspend which puts the audio system into a hibernation mode.
	// We don't want the audio system to suspend when the picker is displayed.
	// Note: This flag is never cleared.
	provider->SetProperty( PlatformModalInteraction::kDoNotSuspend, true );

	provider->BeginSession( runtime );
	provider->Show( source, path, L );

	return 0;
}

static int
showVideoUI(int source, lua_State *L, int listenerRef, int maxTime, int quality)
{
	Runtime& runtime = * LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime.Platform();
	PlatformVideoProvider* provider = platform.GetVideoProvider( runtime.VMContext().LuaState() );

	if (listenerRef != LUA_NOREF)
	{
		provider->RawSetListenerRef( listenerRef );
	}
	// BeginSession causes the runtime to suspend which puts the audio system into a hibernation mode.
	// We don't want the audio system to suspend when the picker is displayed.
	// Note: This flag is never cleared.
	provider->SetProperty( PlatformModalInteraction::kDoNotSuspend, true );

	provider->BeginSession( runtime );
	provider->Show( source, L, maxTime, quality );

	return 0;
}

// media.show( source, listener [, file] )
static int
showImagePicker( lua_State *L )
{
	int top = lua_gettop(L);

	int listener = LUA_NOREF;
	const char *path = NULL;
	
	U32 source = EnumForUserdata(
		kMediaSources,
		lua_touserdata( L, 1 ),
		PlatformImageProvider::kNumSources,
		PlatformImageProvider::kPhotoLibrary );

	LuaLibSystem::FileType fileType;
	int numResults = LuaLibSystem::PathForTable( L, 3, fileType );
	if ( numResults > 0 )
	{
		path = lua_tostring( L, -1 );
	}
	lua_pop( L, numResults );

	// Store callback
	if (  Lua::IsListener( L, 2, CompletionEvent::kName ) )
	{
		lua_pushvalue( L, 2 );
		//luaL_ref already pops the value off the stack
		listener = luaL_ref( L, LUA_REGISTRYINDEX );
	}
	else if ( lua_type( L, 2 ) == LUA_TTABLE )
	{
		lua_getfield( L, 2, "listener" );
		if ( Lua::IsListener( L, -1, CompletionEvent::kName ) ) 
		{
			// This creates the reference and pops it off the stack
			listener = luaL_ref( L, LUA_REGISTRYINDEX );
		}
		else
		{
			lua_pop( L, 1 );
		}
	}

	showImageUI( source, path, L, listener );
	Rtt_ASSERT( lua_type( L, 1 ) == LUA_TLIGHTUSERDATA );

	lua_settop( L, top );

	return 0;
}

/* 
media.capturePhoto
({
	destination = {basedir = ..., filename = ...} -- location of where to save the file
	listener = myListener -- callback to get the name of the saved file
})
*/
static int
capturePhoto( lua_State *L )
{
	int top = lua_gettop(L);

	const char *path = NULL;
	int listener = LUA_NOREF;

	if ( lua_type( L, -1 ) == LUA_TTABLE )
	{
		lua_getfield( L, -1, "destination" );
		if ( lua_type( L, -1 ) == LUA_TTABLE )
		{
			LuaLibSystem::FileType fileType;
			int numResults = LuaLibSystem::PathForTable( L, -1, fileType );
			if ( numResults > 0 )
			{
				path = lua_tostring( L, -1 );
			}
			lua_pop( L, numResults );
		}
		lua_pop( L, 1);

		lua_getfield( L, -1, "listener" );
		if ( Lua::IsListener( L, -1, CompletionEvent::kName ) ) 
		{
			//luaL_ref already pops the value off the stack
			listener = luaL_ref( L, LUA_REGISTRYINDEX );
		}
		else
		{
			lua_pop( L, 1 );
		}
	}

	showImageUI( PlatformImageProvider::kCamera, path, L, listener );

	lua_settop( L, top );

	return 0;
}

/* 
media.captureVideo
({
	destination = {basedir = ..., filename = ...} -- location of where to save the file
	listener = myListener -- callback to get the name of the saved file
})

We don't allow the destination because the file might be very large which will take a very long time to copy

*/
static int
captureVideo( lua_State *L )
{
	int top = lua_gettop(L);

	// The maximum time of the video in seconds
	int maxTime = -1;
	// Defaults to low so that low end devices aren't as slow
	U32 quality = PlatformMediaProviderBase::kLow;

	int listener = LUA_NOREF;

	if ( lua_type( L, 1 ) == LUA_TTABLE )
	{
		lua_getfield( L, 1, "listener" );
		if ( Lua::IsListener( L, -1, CompletionEvent::kName ) ) 
		{
			//luaL_ref already pops the value off the stack
			listener = luaL_ref( L, LUA_REGISTRYINDEX );
		}
		else
		{
			lua_pop( L, 1 );
		}

		lua_getfield( L, 1, "preferredMaxDuration");
		if ( lua_type( L, -1 ) == LUA_TNUMBER )
		{
			maxTime = (int) lua_tointeger( L, -1 );
		}
		lua_pop( L, 1 );

		lua_getfield( L, 1, "preferredQuality");
		if ( lua_type( L, -1 ) == LUA_TSTRING )
		{
			const char* videoQuality = lua_tostring( L, -1 );
			if ( Rtt_StringCompare( videoQuality, "high" ) == 0 )
			{
				quality = PlatformMediaProviderBase::kHigh;
			}
			else if ( Rtt_StringCompare( videoQuality, "medium" ) == 0 )
			{
				quality = PlatformMediaProviderBase::kMedium;
			}
			else if ( Rtt_StringCompare( videoQuality, "low" ) == 0 )
			{
				quality = PlatformMediaProviderBase::kLow;
			}
			else
			{
				CoronaLuaWarning(L, "media.captureVideo() quality '%s' is not recognized.  Defaulting to 'low'", videoQuality );
			}
		}
		lua_pop( L, 1 );
	}
	showVideoUI( PlatformVideoProvider::kCamera, L, listener, maxTime, quality );

	lua_settop( L, top );

	return 0;
}

/* 
media.selectPhoto
({
	destination = {basedir = ..., filename = ...} -- location of where to save the file
	listener = myListener -- callback to get the name of the saved file
	mediaSource = source -- only applicable to iOS because it can be from 2 sources, the PhotoLibrary or the SavedPhotosAlbum which don't exist on Android	
})

TODO:Currently it still accepts Camera for the source but it really shouldn't

*/
static int
selectPhoto( lua_State *L )
{
	int top = lua_gettop(L);

	U32 source = PlatformImageProvider::kPhotoLibrary;
	const char *path = NULL;
	int listener = LUA_NOREF;

	if ( lua_type( L, 1 ) == LUA_TTABLE )
	{
		lua_getfield( L, 1, "mediaSource" );
		if ( lua_islightuserdata(L, -1) )
		{
			source = EnumForUserdata(
			kMediaSources,
			lua_touserdata( L, -1 ),
			PlatformImageProvider::kNumSources,
			PlatformImageProvider::kPhotoLibrary );
		}
		lua_pop( L, 1);

		lua_getfield( L, 1, "destination" );
		if ( lua_type( L, -1 ) == LUA_TTABLE )
		{
			LuaLibSystem::FileType fileType;
			int numResults = LuaLibSystem::PathForTable( L, -1, fileType );
			if ( numResults > 0 )
			{
				path = lua_tostring( L, -1 );
			}
			lua_pop( L, numResults );
		}
		lua_pop( L, 1);

		lua_getfield( L, 1, "listener" );
		if ( Lua::IsListener( L, -1, CompletionEvent::kName ) ) 
		{
			//luaL_ref already pops the value off the stack
			listener = luaL_ref( L, LUA_REGISTRYINDEX );
		}
		else
		{
			lua_pop( L, 1 );
		}
	}

	showImageUI( source, path, L, listener );

	lua_settop( L, top );

	return 0;
}

/* 
media.selectVideo
({
	listener = myListener -- callback to get the name of the saved file
	mediaSource = source -- only applicable to iOS because it can be from 2 sources, the PhotoLibrary or the SavedPhotosAlbum which don't exist on Android	
})

TODO:Currently it still accepts Camera for the source but it really shouldn't
We don't allow the destination because the file might be very large which will take a very long time to copy
*/
static int
selectVideo( lua_State *L )
{
	int top = lua_gettop(L);

	U32 source = PlatformVideoProvider::kPhotoLibrary;
	int listener = LUA_NOREF;

	if ( lua_type( L, 1 ) == LUA_TTABLE )
	{
		lua_getfield( L, 1, "mediaSource" );
		if ( lua_islightuserdata(L, -1) )
		{
			source = EnumForUserdata(
			kMediaSources,
			lua_touserdata( L, -1 ),
			PlatformVideoProvider::kNumSources,
			PlatformVideoProvider::kPhotoLibrary );
		}
		lua_pop( L, 1);

		lua_getfield( L, 1, "listener" );
		if ( Lua::IsListener( L, -1, CompletionEvent::kName ) ) 
		{
			//luaL_ref already pops the value off the stack
			listener = luaL_ref( L, LUA_REGISTRYINDEX );
		}
		else
		{
			lua_pop( L, 1 );
		}
	}

	// The last 2 parameters are for the max time and the quality which shouldn't apply when selecting videos
	showVideoUI( source, L, listener, -1, 0 );

	lua_settop( L, top );

	return 0;
}

// media.save( filename [, baseDir] )
static int
save( lua_State *L )
{
	int nextArg = 1;

	MPlatform::Directory baseDir = MPlatform::kResourceDir;
	const char *filename = LuaLibSystem::GetFilename( L, nextArg, baseDir );
	const MPlatform& platform = LuaContext::GetPlatform( L );

	String path( & platform.GetAllocator() );
	platform.PathForFile( filename, baseDir, MPlatform::kTestFileExists, path );
	if ( path.GetString() )
	{
        platform.SaveImageToPhotoLibrary(path.GetString());
	}
	else
	{
		CoronaLuaError( L, "media.save() file '%s' does not exist", path.GetString() );
	}

	return 0;
}

static int
hasSource( lua_State *L )
{
	Runtime& runtime = * LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime.Platform();
	PlatformImageProvider* provider = platform.GetImageProvider( runtime.VMContext().LuaState() );
	if ( NULL == provider )
	{
		lua_pushboolean( L, 0 );
		lua_pushboolean( L, 0 );
		return 2;
	}

	U32 source = EnumForUserdata(
					 kMediaSources,
					 lua_touserdata( L, 1 ),
					 PlatformImageProvider::kNumSources,
					 PlatformImageProvider::kPhotoLibrary );
	bool hasSource = provider->Supports( source );
	bool hasAccessTo = provider->HasAccessTo( source );
	lua_pushboolean( L, hasAccessTo ? 1 : 0 );
	Rtt_ASSERT( lua_type( L, 1 ) == LUA_TLIGHTUSERDATA );
	lua_pushboolean( L, hasSource ? 1 : 0 );
	Rtt_ASSERT( lua_type( L, 1 ) == LUA_TLIGHTUSERDATA );
	return 2;
}

// ----------------------------------------------------------------------------

static const char kAudioRecorder[] = "media.AudioRecorder";

static int
gcAudioRecorder( lua_State * L )
{
	PlatformAudioRecorder ** pRecorder = (PlatformAudioRecorder **)luaL_checkudata( L, 1, kAudioRecorder );
	if ( pRecorder )
	{
		Rtt_DELETE( *pRecorder );
	}

	return 0;
}

static int
newAudioRecorder( lua_State * L )
{
	const MPlatform & platform = LuaContext::GetRuntime( L )->Platform();

	PlatformAudioRecorder * recorder = NULL;
	const char * path = NULL;

	if ( lua_isstring( L, 1 ) )
	{
		path = lua_tostring( L, 1 );
	}

	recorder = platform.CreateAudioRecorder( LuaContext::GetRuntime( L )->VMContext().LuaState(), path );

	if ( recorder ) 
	{
		Lua::PushUserdata( L, recorder, kAudioRecorder );
	}
	else
	{
		CoronaLuaWarning(L, "failed to create audio recorder '%s'", lua_tostring( L, 1 ) );
	}

//		int nextArg = 2;
		
		// Store callback
//		recorder->SetListenerRef( nextArg );

	if ( ! recorder )
	{
		lua_pushnil( L );
	}
	return 1;
}

static int
startRecording( lua_State *L )
{
	PlatformAudioRecorder ** pr = (PlatformAudioRecorder **) luaL_checkudata( L, 1, kAudioRecorder );
	PlatformAudioRecorder * recorder = *pr;

	recorder->Start();
	
	return 0;
}

static int
stopRecording( lua_State *L )
{
	PlatformAudioRecorder ** pr = (PlatformAudioRecorder **) luaL_checkudata( L, 1, kAudioRecorder );
	PlatformAudioRecorder * recorder = *pr;

	recorder->Stop();
	
	return 0;
}

static int
getSampleRate( lua_State *L )
{
	PlatformAudioRecorder ** pr = (PlatformAudioRecorder **) luaL_checkudata( L, 1, kAudioRecorder );
	PlatformAudioRecorder * recorder = *pr;

	lua_pushnumber( L, recorder->GetSampleRate() );

	return 1;
}

static int
setSampleRate( lua_State *L )
{
	PlatformAudioRecorder ** pr = (PlatformAudioRecorder **) luaL_checkudata( L, 1, kAudioRecorder );
	PlatformAudioRecorder * recorder = *pr;

	Rtt_Real rate = lua_tonumber( L, 2 );

	recorder->SetSampleRate( (U32) rate );

	return 0;
}

static int
isRecording( lua_State *L )
{
	PlatformAudioRecorder ** pr = (PlatformAudioRecorder **) luaL_checkudata( L, 1, kAudioRecorder );
	PlatformAudioRecorder * recorder = *pr;

	lua_pushboolean( L, recorder->IsRecording() );

	return 1;
}


static int
recordingValueForKey( lua_State *L )
{
	int result = 0;    // number of args pushed on the stack
	
	PlatformAudioRecorder ** pr = (PlatformAudioRecorder **) luaL_checkudata( L, 1, kAudioRecorder );
	PlatformAudioRecorder * recorder = *pr;
	
	if ( recorder )
	{
		const char *key = luaL_checkstring( L, 2 );		
		result = 1;

		if ( 0 == Rtt_StringCompare( "startRecording", key ) )
		{
			lua_pushcfunction( L, startRecording );
		}
		else if ( 0 == Rtt_StringCompare( "stopRecording", key ) )
		{
			lua_pushcfunction( L, stopRecording );
		}
		else if ( 0 == Rtt_StringCompare( "getSampleRate", key ) )
		{
			lua_pushcfunction( L, getSampleRate );
		}
		else if ( 0 == Rtt_StringCompare( "setSampleRate", key ) )
		{
			lua_pushcfunction( L, setSampleRate );
		}
		else if ( 0 == Rtt_StringCompare( "isRecording", key ) )
		{
			lua_pushcfunction( L, isRecording );
		}
		else
		{
			result = 0;
		}
	}
	
	return result;
}

// ----------------------------------------------------------------------------

void
LuaLibMedia::Initialize( lua_State *L )
{
	const luaL_Reg kVTable[] =
	{
		{ "playEventSound", playEventSound },
		{ "newEventSound", newEventSound },
		{ "playSound", playSound },
		{ "pauseSound", pauseSound },
		{ "stopSound", stopSound },
		{ "getSoundVolume", getSoundVolume },
		{ "setSoundVolume", setSoundVolume },
		{ "playVideo", playVideo },
		{ "show", showImagePicker },
		{ "save", save },
		{ "hasSource", hasSource },
		{ "capturePhoto", capturePhoto },
		{ "captureVideo", captureVideo },
		{ "selectPhoto", selectPhoto },
		{ "selectVideo", selectVideo },
		{ "newRecording", newAudioRecorder },

		{ NULL, NULL }
	};

	luaL_register( L, "media", kVTable );
	{
		lua_pushlightuserdata( L, UserdataForEnum( kMediaSources, PlatformMediaProviderBase::kPhotoLibrary ) );
		lua_setfield( L, -2, "PhotoLibrary" );
		lua_pushlightuserdata( L, UserdataForEnum( kMediaSources, PlatformMediaProviderBase::kCamera ) );
		lua_setfield( L, -2, "Camera" );
		lua_pushlightuserdata( L, UserdataForEnum( kMediaSources, PlatformMediaProviderBase::kSavedPhotosAlbum ) );
		lua_setfield( L, -2, "SavedPhotosAlbum" );
		lua_pushlightuserdata( L, UserdataForEnum( kGenericMediaSources, Self::kRemoteSource ) );
		lua_setfield( L, -2, "RemoteSource" );
	}
	lua_pop( L, 1 ); // pop "media" table

	const luaL_Reg kAudioRecorderMetaTable[] = 
	{
		{ "__gc", gcAudioRecorder },
		{ "__index", recordingValueForKey },
//		{ "__newindex", Self::SetValueForKey },
		{ NULL, NULL }
	};
	Lua::InitializeMetatable( L, kAudioRecorder, kAudioRecorderMetaTable );
	
	Lua::InitializeGCMetatable( L, kEventSound, gcEventSound );
}

#ifdef Rtt_ANDROID_ENV
void
LuaLibMedia::RecordCallback( int status, int id )
{
	PlatformAudioRecorder * recorder = (PlatformAudioRecorder *) id;
	
	recorder->NotificationCallback( status );
}

void
LuaLibMedia::SoundEndCallback( int id )
{
	PlatformAudioPlayer * player = (PlatformAudioPlayer *) id;
	
	player->NotificationCallback();
}

void
LuaLibMedia::VideoEndCallback( int id )
{
	PlatformVideoPlayer * player = (PlatformVideoPlayer *) id;
	
	player->NotificationCallback();
}
#endif // Rtt_ANDROID_ENV
    
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

