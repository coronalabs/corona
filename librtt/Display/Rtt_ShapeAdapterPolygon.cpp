//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
	lua_State *L, int index, TesselatorPolygon& tesselator, bool hasZ )
{
	bool result = false;

	index = Lua::Normalize( L, index );
	if ( lua_istable( L, index ) )
	{
		ArrayVertex2& contour = tesselator.GetContour();
		Rtt_ASSERT( contour.Length() == 0 );

		// This is used to find the center of the body.
		Rect bounds;

        int componentCount = hasZ ? 3 : 2;
        int numVertices = (int) lua_objlen( L, index ) / componentCount;
        
		for ( int i = 0; i < numVertices; i++ )
		{
			// Lua is one-based, so the first element must be at index 1.
            lua_rawgeti( L, index, ( ( i * componentCount ) + 1 ) );

			// Lua is one-based, so the second element must be at index 2.
            lua_rawgeti( L, index, ( ( i * componentCount ) + 2 ) );

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

