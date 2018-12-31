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

#include "Rtt_FilePath.h"
#include "Rtt_Lua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const char FilePath::kMetatableName[] = "FilePath"; // unique identifier for this userdata type

int
FilePath::CreateAndPush(
	lua_State *L,
	Rtt_Allocator *allocator,
	const char *filename,
	MPlatform::Directory baseDir )
{
	int result = 0;
	FilePath *path = Rtt_NEW( allocator, FilePath( filename, baseDir ) );

	if ( Rtt_VERIFY( path ) )
	{
		Lua::PushUserdata( L, path, Self::kMetatableName );
		result = 1;
	}

	return result;
}

int
FilePath::Finalizer( lua_State *L )
{
	Self **ud = (Self **)luaL_checkudata( L, 1, Self::kMetatableName );
	if ( ud )
	{
		Rtt_DELETE( *ud );
	}

	return 0;
}

// Call this to init metatable
void
FilePath::Initialize( lua_State *L )
{
	Rtt_LUA_STACK_GUARD( L );

	const luaL_Reg kVTable[] =
	{
		{ "__gc", Self::Finalizer },
		{ NULL, NULL }
	};
		
	Lua::InitializeMetatable( L, Self::kMetatableName, kVTable );
}

// ----------------------------------------------------------------------------

FilePath::FilePath( const char *filename, MPlatform::Directory baseDir )
:	fFilename(),
	fBaseDir( baseDir )
{
	if ( filename )
	{
		fFilename = filename;
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

