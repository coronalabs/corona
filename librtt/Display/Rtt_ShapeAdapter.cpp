//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ShapeAdapter.h"

#include "Core/Rtt_StringHash.h"
#include "Display/Rtt_ShapePath.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------
/*
const ShapeAdapter&
ShapeAdapter::Constant()
{
	static const ShapeAdapter sAdapter;
	return sAdapter;
}
*/
// ----------------------------------------------------------------------------

static const char kCircleString[] = "circle";
static const char kPolygonString[] = "polygon";
static const char kRectString[] = "rect";
static const char kRoundedRectString[] = "roundedRect";
static const char kMeshString[] = "mesh";

const char *
ShapeAdapter::StringForType( Type t )
{
	const char *result = NULL;

	switch ( t )
	{
		case kCircleType:
			result = kCircleString;
			break;
		case kPolygonType:
			result = kPolygonString;
			break;
		case kRectType:
			result = kRectString;
			break;
		case kRoundedRectType:
			result = kRoundedRectString;
			break;
		case kMeshType:
			result = kMeshString;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

/*
ShapeAdapter::Type
ShapeAdapter::TypeForString( const char * s )
{
	Type result;

	Rtt_ASSERT_NOT_IMPLEMENTED();

	return result;
}
*/

ShapeAdapter::ShapeAdapter( Type t )
:	fTypeString( StringForType( t ) )
{
}

/*
static StringHash *
GetHash( lua_State *L )
{
	static const char *keys[] = 
	{
	};

	static StringHash *sHash = NULL;
	if ( ! sHash )
	{
		Rtt_Allocator *allocator = LuaContext::GetAllocator( L );
		sHash = Rtt_NEW( allocator, StringHash( *allocator, keys, sizeof( keys ) / sizeof( const char * ), 0, 0, 0, __FILE__, __LINE__ ) );
	}

	return sHash;
}
*/

int
ShapeAdapter::ValueForKey(
	const LuaUserdataProxy& sender,
	lua_State *L,
	const char *key ) const
{
	int result = 0;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	const ShapePath *path = (const ShapePath *)sender.GetUserdata();
	if ( ! path ) { return result; }

#if 0
	// TODO: Use hash when we have more than one property
	result = 1;

	int index = GetHash( L )->Lookup( key );
	switch ( index )
	{
		default:
			result = 0;
			break;
	}
#else
	if ( 0 == strcmp( key, "type" ) )
	{
		lua_pushstring( L, fTypeString );
		result = 1;
	}

	// TODO: is hash worth it? these are not likely to be called often
	else if (0 == strcmp( key, "textureBounds" ))
	{
		ArrayVertex2 texVertices( LuaContext::GetRuntime( L )->GetAllocator() );

		const_cast<ShapePath *>( path )->GetTextureVertices( texVertices ); // TODO: okay? tesselator updates scale factors...

		Rect r = path->GetTextureExtents( texVertices );

		lua_createtable( L, 0, 4 ); // as with contentBounds...

		const char uMin[] = "uMin";
		const char vMin[] = "vMin";
		const char uMax[] = "uMax";
		const char vMax[] = "vMax";
		const size_t kLen = sizeof( uMin ) - 1;

		Rtt_STATIC_ASSERT( sizeof(char) == 1 );
		Rtt_STATIC_ASSERT( sizeof(uMin) == sizeof(vMin) );
		Rtt_STATIC_ASSERT( sizeof(uMin) == sizeof(uMax) );
		Rtt_STATIC_ASSERT( sizeof(uMin) == sizeof(vMax) );

		Real uMinRect = r.xMin;
		Real vMinRect = r.yMin;
		Real uMaxRect = r.xMax;
		Real vMaxRect = r.yMax;

		if ( r.IsEmpty() )
		{
			uMinRect = vMinRect = uMaxRect = vMaxRect = Rtt_REAL_0;
		}

		setProperty( L, uMin, kLen, uMinRect );
		setProperty( L, vMin, kLen, vMinRect );
		setProperty( L, uMax, kLen, uMaxRect );
		setProperty( L, vMax, kLen, vMaxRect );

		result = 1;
	}

	else if (0 == strcmp( key, "textureVertices" ))
	{
		ArrayVertex2 texVertices( LuaContext::GetRuntime( L )->GetAllocator() );

		const_cast<ShapePath *>( path )->GetTextureVertices( texVertices ); // TODO: okay? tesselator updates scale factors...

		S32 iMax = texVertices.Length();

		lua_createtable( L, iMax * 2, 0 );

		for (S32 i = 0, j = 0; i < iMax; ++i, j += 2)
		{
			const Vertex2& v = texVertices[i];

			lua_pushnumber( L, v.x );
			lua_rawseti( L, -2, j + 1 );
			lua_pushnumber( L, v.y );
			lua_rawseti( L, -2, j + 2 );
		}

		result = 1;
	}
#endif

	return result;
}

bool
ShapeAdapter::SetValueForKey(
	LuaUserdataProxy& sender,
	lua_State *L,
	const char *key,
	int valueIndex ) const
{
	bool result = false;

/*
	Rtt_ASSERT( key ); // Caller should check at the top-most level

	ShapePath *path = (ShapePath *)sender.GetUserdata();
	if ( ! path ) { return result; }

	result = 1;

	int index = GetHash( L )->Lookup( key );
	switch ( index )
	{
		default:
			result = 0;
			break;
	}
*/

	return result;
}

void
ShapeAdapter::WillFinalize( LuaUserdataProxy& sender ) const
{
	DisplayPath *path = (DisplayPath *)sender.GetUserdata();
	if ( ! path ) { return; }
	
	path->DetachProxy();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

