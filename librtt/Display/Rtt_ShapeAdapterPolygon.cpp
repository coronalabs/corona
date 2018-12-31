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

#include "Display/Rtt_ShapeAdapterPolygon.h"

#include "Core/Rtt_StringHash.h"
#include "Display/Rtt_DisplayTypes.h"
#include "Display/Rtt_ShapePath.h"
#include "Display/Rtt_TesselatorPolygon.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const ShapeAdapterPolygon&
ShapeAdapterPolygon::Constant()
{
	static const ShapeAdapterPolygon sAdapter;
	return sAdapter;
}

bool
ShapeAdapterPolygon::InitializeContour(
	lua_State *L, int index, TesselatorPolygon& tesselator )
{
	bool result = false;

	index = Lua::Normalize( L, index );
	if ( lua_istable( L, index ) )
	{
		ArrayVertex2& contour = tesselator.GetContour();
		Rtt_ASSERT( contour.Length() == 0 );

		// This is used to find the center of the body.
		Rect bounds;

		int numVertices = (int) lua_objlen( L, index ) >> 1;
		for ( int i = 0; i < numVertices; i++ )
		{
			// Lua is one-based, so the first element must be at index 1.
			lua_rawgeti( L, index, ( ( i * 2 ) + 1 ) );

			// Lua is one-based, so the second element must be at index 2.
			lua_rawgeti( L, index, ( ( i * 2 ) + 2 ) );

			Vertex2 v = { luaL_toreal( L, -2 ),
							luaL_toreal( L, -1 ) };
			lua_pop( L, 2 );

			contour.Append( v );
			bounds.Union( v );
		}

		Vertex2 center_offset;
		bounds.GetCenter( center_offset );

		// Offset the contour to center the body around its center of mass.
		for ( int i = 0; i < numVertices; i++ )
		{
			contour[ i ].x -= center_offset.x;
			contour[ i ].y -= center_offset.y;
		}

		tesselator.Invalidate();
		result = true;
	}

	return result;
}

// ----------------------------------------------------------------------------

ShapeAdapterPolygon::ShapeAdapterPolygon()
:	Super( kPolygonType )
{
}

StringHash *
ShapeAdapterPolygon::GetHash( lua_State *L ) const
{
	static const char *keys[] = 
	{
		"",
	};
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 1, 0, 0, __FILE__, __LINE__ );
	return &sHash;
}

// No properties (except inherited ones), so disabling for now.
#if 0
int
ShapeAdapterPolygon::ValueForKey(
	const LuaUserdataProxy& sender,
	lua_State *L,
	const char *key ) const
{
	int result = 0;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	const ShapePath *path = (const ShapePath *)sender.GetUserdata();
	if ( ! path ) { return result; }

	const TesselatorPolygon *tesselator =
		static_cast< const TesselatorPolygon * >( path->GetTesselator() );
	if ( ! tesselator ) { return result; }

	result = 1; // Assume 1 Lua value will be pushed on the stack

	int index = GetHash( L )->Lookup( key );
	switch ( index )
	{
		default:
			result = 0; // No Lua values pushed
			break;
	}

	return result;
}

bool
ShapeAdapterPolygon::SetValueForKey(
	LuaUserdataProxy& sender,
	lua_State *L,
	const char *key,
	int valueIndex ) const
{
	bool result = false;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	ShapePath *path = (ShapePath *)sender.GetUserdata();
	if ( ! path ) { return result; }

	TesselatorPolygon *tesselator =
		static_cast< TesselatorPolygon * >( path->GetTesselator() );
	if ( ! tesselator ) { return result; }

	result = true; // Assume value will be set

	int index = GetHash( L )->Lookup( key );
	switch ( index )
	{
		default:
			result = false; // No value set
			break;
	}

	return result;
}
#endif

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

