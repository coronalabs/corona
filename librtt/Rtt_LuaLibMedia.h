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
