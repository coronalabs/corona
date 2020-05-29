//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_SpriteSourceFrame.h"
#include "Rtt_Lua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// Assumes element of frames array is at index = -1
void
SpriteSourceFrame::Initialize( lua_State * L, SpriteSourceFrame& sourceFrame )
{
	Rtt_ASSERT( lua_istable( L, -1 ) );

	Rtt_Real f;

	lua_getfield( L, -1, "spriteColorRect" );
	if ( lua_istable( L, -1 ) )
	{
		lua_getfield( L, -1, "x" );
		if ( lua_isnumber( L, -1 ) )
		{
			f = (Rtt_Real) lua_tonumber( L, -1 );
			sourceFrame.fTrimmedOrigin.x = (int) f;
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, -1, "y" );
		if ( lua_isnumber( L, -1 ) )
		{
			f = (Rtt_Real) lua_tonumber( L, -1 );
			sourceFrame.fTrimmedOrigin.y = (int) f;
		}
		lua_pop( L, 1 );
	}
	lua_pop( L, 1 );

/*
	lua_getfield( L, -1, "spriteOffset" );
	if ( lua_istable( L, -1 ) )
	{
		lua_getfield( L, -1, "x" );
		if ( lua_isnumber( L, -1 ) )
		{
			f = (Rtt_Real) lua_tonumber( L, -1 );
			sourceFrame.fOffset.x = (int) f;
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, -1, "y" );
		if ( lua_isnumber( L, -1 ) )
		{
			f = (Rtt_Real) lua_tonumber( L, -1 );
			sourceFrame.fOffset.y = (int) f;
		}
		lua_pop( L, 1 );
	}
	lua_pop( L, 1 );
*/

	lua_getfield( L, -1, "textureRect" );
	if ( lua_istable( L, -1 ) )
	{
		lua_getfield( L, -1, "x" );
		if ( lua_isnumber( L, -1 ) )
		{
			f = (Rtt_Real) lua_tonumber( L, -1 );
			sourceFrame.fSheetOrigin.x = (int) f;
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, -1, "y" );
		if ( lua_isnumber( L, -1 ) )
		{
			f = (Rtt_Real) lua_tonumber( L, -1 );
			sourceFrame.fSheetOrigin.y = (int) f;
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "width" );
		if ( lua_isnumber( L, -1 ) )
		{
			f = (Rtt_Real) lua_tonumber( L, -1 );
			sourceFrame.fSheetSize.x = (int) f;
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, -1, "height" );
		if ( lua_isnumber( L, -1 ) )
		{
			f = (Rtt_Real) lua_tonumber( L, -1 );
			sourceFrame.fSheetSize.y = (int) f;
		}
		lua_pop( L, 1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "spriteSourceSize" );
	if ( lua_istable( L, -1 ) )
	{
		lua_getfield( L, -1, "width" );
		f = (Rtt_Real) lua_tonumber( L, -1 );
		sourceFrame.fSourceSize.x = (int) f;
		lua_pop( L, 1 );
		
		lua_getfield( L, -1, "height" );
		f = (Rtt_Real) lua_tonumber( L, -1 );
		sourceFrame.fSourceSize.y = (int) f;
		lua_pop( L, 1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "spriteTrimmed" );
	sourceFrame.fTrimmed = lua_toboolean( L, -1 );
	lua_pop( L, 1 );

	lua_getfield( L, -1, "textureRotated" );
	sourceFrame.fRotated = lua_toboolean( L, -1 );
	lua_pop( L, 1 );

}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
