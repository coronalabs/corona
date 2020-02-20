//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

