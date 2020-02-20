//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_LuaLibMedia_H__
#define _Rtt_LuaLibMedia_H__

// ----------------------------------------------------------------------------

#include "Rtt_Lua.h"

namespace Rtt
{

class String;

// ----------------------------------------------------------------------------

class LuaLibMedia
{
	public:
		typedef LuaLibMedia Self;

	public:
		enum Source
		{
			kRemoteSource = 0,

			kNumSources
		};

		// Returns true, if path is a remote url; false, if path is a local file
		static const char* GetLocalOrRemotePath( lua_State *L, int& nextArg, String& path, bool& isRemote );

	public:
		static void Initialize( lua_State *L );

#ifdef Rtt_ANDROID_ENV
		static void RecordCallback( uintptr_t id, int status );
		static void SoundEndCallback( uintptr_t id );
		static void VideoEndCallback( uintptr_t id );
#endif // Rtt_ANDROID_ENV
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_LuaLibMedia_H__
