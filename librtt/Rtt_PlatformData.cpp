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

#include "Rtt_PlatformData.h"

#include "Rtt_Lua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kPlatformDataKey[] = "PlatformData";

static int
gcPlatformData( lua_State *L )
{
	PlatformData** p = (PlatformData**)luaL_checkudata( L, 1, kPlatformDataKey );
	Rtt_DELETE( *p );

	return 0;
}

void
PlatformData::Initialize( lua_State* L )
{
	Lua::InitializeGCMetatable( L, kPlatformDataKey, gcPlatformData );
}

const char*
PlatformData::Name()
{
	return kPlatformDataKey;
}

// ----------------------------------------------------------------------------

PlatformData::~PlatformData()
{
}
/*
bool
PlatformData::Push( lua_State* L )
{
	PlatformData** p = (PlatformData**)lua_newuserdata( L, sizeof( PlatformData* ) );
	*p = this;

	luaL_getmetatable( L, kPlatformDataKey );
	lua_setmetatable( L, -2 );

	return true;
}
*/
static int
PlatformDataSource( lua_State* L )
{
	bool result = 0;

	int index = lua_upvalueindex( 1 );
	PlatformData** pp = (PlatformData**)lua_touserdata( L, index ); // luaL_checkudata( L, index, kPlatformDataKey );
	PlatformData* p = pp ? *pp : NULL;
	if ( p )
	{
		size_t numBytes = 2048; // Read in 2K chunks
		size_t numBytesRead = 0;
		const char* data = p->Read( numBytes, numBytesRead );
		if ( data )
		{
			Rtt_ASSERT( numBytesRead > 0 );
			lua_pushlstring( L, data, numBytesRead );
			result = 1;
		}
	}

	return result;
}

bool
PlatformData::PushIterator( lua_State* L )
{
	Lua::PushUserdata( L, this, kPlatformDataKey );
//	bool success = Push( L );
//	if ( success )
	{
		lua_pushcclosure( L, &PlatformDataSource, 1 );
	}
	return true;
//	return success;
}

/*
function source.data( data )
	if data then
		local 
	end
end

*/

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

