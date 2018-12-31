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

#include "Rtt_PlatformVideoProvider.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_PlatformData.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

void
PlatformVideoProvider::AddProperties( lua_State *L, void* userdata )
{
	VideoProviderResult* result = (VideoProviderResult*)userdata;

	if (result->SelectedVideoFileName)
	{
		lua_pushstring( L, result->SelectedVideoFileName);
		lua_setfield( L, -2, "url" );
	}
	else
	{
		// The file wasn't selected so the "selectedFile" should be nil.
	}

	if ( result->Duration > 0)
	{
		lua_pushnumber( L, result->Duration);
		lua_setfield( L, -2, "duration" );
	}
	else
	{
		// The duration is unknown so don't set it
	}

	if ( result->Size > 0)
	{
		lua_pushnumber( L, result->Size);
		lua_setfield( L, -2, "fileSize" );
	}
	else
	{
		// The size is unknown so don't set it
	}
	
	lua_pushboolean( L, result->SelectedVideoFileName ? 1 : 0 );
	lua_setfield( L, -2, "completed" );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

