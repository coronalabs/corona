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
