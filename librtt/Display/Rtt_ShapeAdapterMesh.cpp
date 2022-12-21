//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ShapeAdapterMesh.h"

#include "Core/Rtt_StringHash.h"
#include "Display/Rtt_ShapePath.h"
#include "Display/Rtt_ShapeObject.h"
#include "Display/Rtt_TesselatorMesh.h"
#include "Rtt_LuaContext.h"
#include "CoronaLua.h"

#include <limits> // <- STEVE CHANGE

// ----------------------------------------------------------------------------

namespace Rtt
{

Geometry::PrimitiveType
ShapeAdapterMesh::GetMeshMode(lua_State *L, int index)
{
	Geometry::PrimitiveType ret = Geometry::kTriangles;
	
	if (lua_istable( L, index ) )
	{
		lua_getfield( L, index, "mode" );
		if( lua_type( L, -1 ) == LUA_TSTRING )
		{
			const char* type = lua_tostring( L, -1);
			if ( strcmp(type, "triangles" ) == 0 )
			{
				ret = Geometry::kTriangles;
			}
			else if ( strcmp(type, "strip" ) == 0 )
			{
				ret = Geometry::kTriangleStrip;
			}
			else if ( strcmp(type, "fan" ) == 0 )
			{
				ret = Geometry::kTriangleFan;
			}
			else if ( strcmp(type, "indexed" ) == 0 )
			{
				ret = Geometry::kIndexedTriangles;
			}
			else if ( strcmp(type, "lines" ) == 0 )
			{
				ret = Geometry::kLines;
			}
			else if ( strcmp(type, "lineLoop" ) == 0 )
			{
				ret = Geometry::kLineLoop;
			}
		}
		lua_pop( L, 1);
	}
	
	return ret;
}

// STEVE CHANGE
static bool IsValidBuffer( lua_State *L, unsigned char* &buffer, size_t &len )
{
	if ( LUA_TUSERDATA == lua_type( L, -1 ) ) // TODO: generalize with memory API
	{
		buffer = static_cast< unsigned char* >( lua_touserdata( L, -1 ) );
		len = lua_objlen( L, -1 );
		
		return true;
	}
	
	else
	{
		return false;
	}
}

static const unsigned char* AuxGetBuffer( lua_State *L, size_t valueSize, size_t &len, U32 &n )
{
	unsigned char* buffer = NULL;

	if ( IsValidBuffer( L, buffer, len ) )
	{
		n = (U32)(len / valueSize);
	}

	return buffer;
}

static const unsigned char* IssueWarning( const unsigned char* buffer, const char * warning )
{
	if (buffer)
	{
		CORONA_LOG_WARNING( "%s", warning );
	}

	return NULL;
}

static const unsigned char* GetBuffer( lua_State *L, size_t valueSize, size_t element, U32 &n, size_t &stride, const U32* outputLength = NULL )
{
	size_t len;

	const unsigned char* buffer = AuxGetBuffer( L, valueSize, len, n );

	if ( !buffer && lua_istable( L, -1 ) )
	{
		lua_getfield( L, -1, "buffer" ); // ..., t, buffer

		buffer = AuxGetBuffer( L, valueSize, len, n );

		lua_pop( L, 1 ); // ..., t

		if (buffer)
		{
			lua_getfield( L, -1, "count" ); // ..., t, count
			lua_getfield( L, -2, "stride" ); // ..., t, count, stride
			lua_getfield( L, -3, "offset" ); // ..., t, count, stride, offset

			// Supplied own `count`? If a `stride` of 0 is not explicitly supplied, this is expected to be <= what the
			// buffer can hold (`size` / `stride`, where the latter defaults to value size).
			size_t count = 0;

			if ( lua_type( L, -3 ) == LUA_TNUMBER )
			{
				count = lua_tointeger( L, -3 );
			}

			// Can it also fit in the output?
			if ( outputLength && count > *outputLength )
			{
				buffer = IssueWarning( buffer, "Too many values to fit in output" );
			}

			// Possible strides include 0 (repeat the first element `count` times) or >= the value size.
			// By default, the value size is used.
			stride = 0;

			bool repeatFirstValue = false;

			if ( lua_type( L, -2 ) == LUA_TNUMBER )
			{
				stride = lua_tointeger( L, -2 );

				if ( stride > 0 && stride <= valueSize )
				{
					buffer = IssueWarning( buffer, "`stride` is too low" );
				}

				else if ( 0 == stride )
				{
					if ( 0 == count )
					{
						buffer = IssueWarning( buffer, "Explicit zero `stride` expects `count`" );
					}

					else
					{
						repeatFirstValue = true;
					}
				}
			}

			if ( 0 == stride && !repeatFirstValue )
			{
				stride = valueSize;
			}

			// Get any custom `offset` and use it to find the final count, ensuring the data still fits.
			// This will be added to any offset into the stream, e.g. starting at some vertex #3 versus
			// right from the start. If all is well, update the buffer with respect to the offset.
			size_t offset = element * stride;

			if ( lua_type( L, -1 ) == LUA_TNUMBER )
			{
				offset += lua_tointeger( L, -1 );
			}
							
			if ( stride > 0 )
			{
				n = ( len - offset ) / stride;

				if ( 0 == n )
				{
					buffer = IssueWarning( buffer, "Buffer not large enough to supply any values" );
				}

				else if ( count > n )
				{
					buffer = IssueWarning( buffer, "Buffer not large enough to supply `count` values" );
				}

				else if ( count > 0 )
				{
					n = count;
				}
			}

			else
			{
				n = count;

				if ( len - offset < valueSize )
				{
					buffer = IssueWarning( buffer, "Buffer not large enough to supply (repeated) value" );
				}
			}

			if ( buffer )
			{
				buffer += offset;
			}

			lua_pop( L, 3 ); // ..., t
		}
	}

	return buffer;
}

static void UpdateIndexRange( U16 index, U16 &minIndex, U16 &maxIndex )
{
	minIndex = index < minIndex ? index : minIndex;
	maxIndex = index > maxIndex ? index : maxIndex;
}

template<typename T> T GetValueFromStream( const unsigned char* &from, size_t stride )
{
	T v;
	memcpy( &v, from, sizeof( T ) );
	from += stride;
	return v;
}

template<typename T> void CopyToOutput( T * to, const unsigned char * from, U32 count, size_t stride )
{
	if ( sizeof( T ) == stride )
	{
		memcpy( to, from, count * sizeof( T ) );
	}
	else
	{
		for ( U32 i = 0; i < count; i++)
		{
			to[i] = GetValueFromStream<T>( from, stride );
		}
	}
}
// /STEVE CHANGE

bool
ShapeAdapterMesh::InitializeMesh(lua_State *L, int index, TesselatorMesh& tesselator )
{
	if ( !lua_istable( L, index ) )
	{
		return false;
	}
	index = Lua::Normalize( L, index );

	// STEVE CHANGE moved indices
	int indicesStart = 1;
	lua_getfield( L, index, "zeroBasedIndices" );
	if (lua_type( L, -1 ) == LUA_TBOOLEAN && lua_toboolean( L, -1 )) // TODO: add parsing
	{
		indicesStart = 0;
	}
	lua_pop( L, 1);
	// STEVE CHANGE
	lua_getfield( L, index, "adjustFromIndices" );
	bool adjustFromIndices = lua_toboolean( L, -1 );
	lua_pop( L, 1 );
	// /STEVE CHANGE
	
	TesselatorMesh::ArrayIndex& indices = tesselator.GetIndices();
	U16 minIndex = std::numeric_limits<U16>::max(), maxIndex = 0; // <- STEVE CHANGE
	U32 baseVertex = 0, vertexCount = 0; // <- STEVE CHANGE
	lua_getfield( L, index, "indices" );
	// STEVE CHANGE
	size_t stride;
	U32 numIndices = (U32)indices.Length();
	const unsigned char* fromIndices = GetBuffer(L, sizeof(U16), 0, numIndices, stride);

	if (fromIndices)
	{
		indices.Reserve( numIndices );
		for (U32 i = 0; i < numIndices; i++)
		{
			U16 newIndex = GetValueFromStream<U16>( fromIndices, stride );
			indices.Append(newIndex);
			UpdateIndexRange( newIndex, minIndex, maxIndex );
		}
	}
	// /STEVE CHANGE
	else if (lua_istable( L, -1 )) // <- STEVE CHANGE
	{
		U32 numIndices = (U32)lua_objlen( L, -1 );
		indices.Reserve( numIndices );
		for(U32 i=0; i<numIndices; i++)
		{
			lua_rawgeti( L, -1, i+1 );
			if ( lua_type( L, -1 ) == LUA_TNUMBER )
			{
				U16 index = lua_tointeger( L, -1 ) - indicesStart;
				UpdateIndexRange( index, minIndex, maxIndex ); // <- STEVE CHANGE
				indices.Append(index);
			}
			lua_pop( L, 1 );
		}
	}
	// STEVE CHANGE
	bool hasIndices = minIndex < std::numeric_limits<U16>::max();
	if ( hasIndices )
	{
		tesselator.SetLowestIndex( minIndex );
		
		if (adjustFromIndices )
		{
			if ( minIndex > 0 )
			{
				baseVertex = minIndex;
				for ( U32 i = 0; i < numIndices; i++ )
				{
					indices.WriteAccess()[i] -= minIndex;
				}
			}
			
			vertexCount = maxIndex - minIndex + 1;
		}
	}
	// /STEVE CHANGE
	lua_pop( L, 1);
	// /STEVE CHANGE moved indices

	ArrayVertex2& mesh = tesselator.GetMesh();
	U32 numVertices; // <- STEVE CHANGE
	Rtt_ASSERT( mesh.Length() == 0 );
	lua_getfield( L, index, "vertices" );
	// STEVE CHANGE
	const unsigned char* fromVertices = GetBuffer(L, sizeof(Vertex2), baseVertex, numVertices, stride);

	if (fromVertices)
	{
		if ( vertexCount )
		{
			numVertices = vertexCount; // ^^ TODO: validate that we have this many...
		}
		mesh.Reserve( numVertices );
		for ( U32 i = 0; i < numVertices; i++ )
		{
			Vertex2 v = GetValueFromStream<Vertex2>( fromVertices, stride );
			mesh.Append( v );
		}
	}
	// /STEVE CHANGE
	else if (lua_istable( L, -1 )) // <- STEVE CHANGE
	{
		Rtt_ASSERT (lua_objlen( L, -1 ) % 2 == 0);
		/*U32*/ numVertices = (U32)lua_objlen( L, -1 )/2; // <- STEVE CHANGE
		// STEVE CHANGE
		if ( vertexCount )
		{
			numVertices = vertexCount;
		}
		// /STEVE CHANGE
		mesh.Reserve( numVertices );
		for(U32 i=0,j=baseVertex; i<numVertices; i++, j++) // <- STEVE CHANGE
		{
			lua_rawgeti( L, -1, 2*j+1 ); // <- STEVE CHANGE
			lua_rawgeti( L, -2, 2*j+2 ); // <- STEVE CHANGE
			if ( lua_type( L, -2 ) == LUA_TNUMBER &&
			     lua_type( L, -1 ) == LUA_TNUMBER )
			{
				Vertex2 v = { Rtt_FloatToReal(lua_tonumber( L, -2 )), Rtt_FloatToReal(lua_tonumber( L, -1 ))};
				mesh.Append(v);
			}
			lua_pop( L, 2 );
		}

		Rect r;
		numVertices = mesh.Length();
		for ( U32 i = 0; i < numVertices; i++ )
		{
			r.Union( mesh[i] );
		}

		Vertex2 vertexOffset = {0, 0};

		if (!r.IsEmpty())
		{
			r.GetCenter(vertexOffset);

			for ( U32 i = 0; i < numVertices; i++ )
			{
				mesh[i].x -= vertexOffset.x;
				mesh[i].y -= vertexOffset.y;
			}
		}

		tesselator.SetVertexOffset(vertexOffset);

	}
	lua_pop( L, 1);
	
	// STEVE CHANGE moved count check down
	
	
	ArrayVertex2& UVs = tesselator.GetUV();
	U32 numUVs; // <- STEVE CHANGE
	lua_getfield( L, index, "uvs" );
	// STEVE CHANGE
	const unsigned char* fromUVs = GetBuffer(L, sizeof(Vertex2), baseVertex, numUVs, stride);

	if (fromVertices)
	{
		if ( vertexCount )
		{
			numUVs = vertexCount; // ^^ TODO: validate that we have this many...
		}
		UVs.Reserve( numUVs );
		for ( U32 i = 0; i < numUVs; i++ )
		{
			Vertex2 v = GetValueFromStream<Vertex2>( fromUVs, stride );
			UVs.Append( v );
		}
	}
	// /STEVE CHANGE
	else if (lua_istable( L, -1)) // <- STEVE CHANGE
	{
		/*U32*/ numUVs = (U32)lua_objlen( L, -1 )/2; // <- STEVE CHANGE
		// STEVE CHANGE
		if ( vertexCount )
		{
			numUVs = vertexCount;
		}
		// /STEVE CHANGE
		if ( numUVs == (U32)mesh.Length() )
		{
			UVs.Reserve( numUVs );
			for(U32 i=0, j=baseVertex; i<numUVs; i++, j++) // <- STEVE CHANGE
			{
				lua_rawgeti( L, -1, 2*j+1 ); // <- STEVE CHANGE
				lua_rawgeti( L, -2, 2*j+2 ); // <- STEVE CHANGE
				if ( lua_type( L, -2 ) == LUA_TNUMBER &&
					 lua_type( L, -1 ) == LUA_TNUMBER )
				{
					Vertex2 v = { Rtt_FloatToReal(lua_tonumber( L, -2)), Rtt_FloatToReal(lua_tonumber( L, -1))};
					UVs.Append(v);
				}
				lua_pop( L, 2 );
			}
		}		
	}
	lua_pop( L, 1);
	
	// STEVE CHANGE moved indices up
	// STEVE CHANGE moved
	if (mesh.Length() < 3)
	{
		CoronaLuaError( L, "display.newMesh() at least 3 pairs of (x;y) coordinates must be provided in 'vertices' parameter" );
		return false;
	}
	// /STEVE CHANGE moved

	tesselator.Invalidate();
	tesselator.Update();
	
	return true;
}
// ----------------------------------------------------------------------------

const ShapeAdapterMesh&
ShapeAdapterMesh::Constant()
{
	static const ShapeAdapterMesh sAdapter;
	return sAdapter;
}

// ----------------------------------------------------------------------------

ShapeAdapterMesh::ShapeAdapterMesh()
:	Super( kMeshType )
{
}

StringHash *
ShapeAdapterMesh::GetHash( lua_State *L ) const
{
	static const char *keys[] =
	{
		"setVertex",       // 0
		"getVertex",       // 1
		"setUV",		   // 2
		"getUV",           // 3
		"getVertexOffset", // 4
		"update",		   // 5
		"getLowestIndex"   // 6 // <- STEVE CHANGE
	};
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 0, 0, 0, __FILE__, __LINE__ ); // <- STEVE CHANGE
	return &sHash;
}

int
ShapeAdapterMesh::ValueForKey(
	const LuaUserdataProxy& sender,
	lua_State *L,
	const char *key ) const
{
	int result = 0;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	const ShapePath *path = (const ShapePath *)sender.GetUserdata();
	if ( ! path ) { return result; }

	const TesselatorMesh *tesselator =
		static_cast< const TesselatorMesh * >( path->GetTesselator() );
	if ( ! tesselator ) { return result; }

	result = 1; // Assume 1 Lua value will be pushed on the stack

	int index = GetHash( L )->Lookup( key );
	switch ( index )
	{
		case 0:
			Lua::PushCachedFunction( L, setVertex );
			break;
		case 1:
			Lua::PushCachedFunction( L, getVertex );
			break;
		case 2:
			Lua::PushCachedFunction( L, setUV );
			break;
		case 3:
			Lua::PushCachedFunction( L, getUV );
			break;
		case 4:
			Lua::PushCachedFunction( L, getVertexOffset );
			break;
		case 5:
			Lua::PushCachedFunction(L, update);
			break;
		// STEVE CHANGE
		case 6:
			Lua::PushCachedFunction( L, getLowestIndex );
			break;
		// /STEVE CHANGE
		default:
			result = Super::ValueForKey( sender, L, key );
			break;
	}

	return result;
}

int ShapeAdapterMesh::setVertex( lua_State *L )
{
	int result = 0;
	int nextArg = 1;
	LuaUserdataProxy* sender = LuaUserdataProxy::ToProxy( L, nextArg++ );
	if(!sender) { return result; }
	
	ShapePath *path = (ShapePath *)sender->GetUserdata();
	if ( ! path ) { return result; }
	
	TesselatorMesh *tesselator =
	static_cast< TesselatorMesh * >( path->GetTesselator() );
	if ( ! tesselator ) { return result; }

	int vertIndex = luaL_checkint(L, nextArg++) - 1;
	Real x = luaL_checkreal( L, nextArg++ );
	Real y = luaL_checkreal( L, nextArg++ );

	if (vertIndex >= tesselator->GetMesh().Length() || vertIndex < 0)
	{
		luaL_argerror( L, 1, "index is out of bounds");
	}

	const Vertex2 &offset = tesselator->GetVertexOffset();
	x -= offset.x;
	y -= offset.y;

	Vertex2& orig = tesselator->GetMesh().WriteAccess()[vertIndex];
	
	if( !Rtt_RealEqual(x, orig.x) || !Rtt_RealEqual(y, orig.y))
	{
		orig.x = x;
		orig.y = y;

		path->Invalidate( ClosedPath::kFillSource |
						 ClosedPath::kStrokeSource );
		
		path->GetObserver()->Invalidate( DisplayObject::kGeometryFlag |
										DisplayObject::kStageBoundsFlag |
										DisplayObject::kTransformFlag );
	}

	return 0;
}
	
int ShapeAdapterMesh::getVertex( lua_State *L )
{
	int result = 0;
	int nextArg = 1;
	LuaUserdataProxy* sender = LuaUserdataProxy::ToProxy( L, nextArg++ );
	if(!sender) { return result; }
	
	ShapePath *path = (ShapePath *)sender->GetUserdata();
	if ( ! path ) { return result; }
	
	TesselatorMesh *tesselator =
	static_cast< TesselatorMesh * >( path->GetTesselator() );
	if ( ! tesselator ) { return result; }
	
	int vertIndex = luaL_checkint(L, nextArg++) - 1;
	if (vertIndex >= tesselator->GetMesh().Length() || vertIndex < 0)
	{
		CoronaLuaWarning( L, "mesh:getVertex() index is out of bounds");
	}
	else
	{
		const Vertex2 &vert = tesselator->GetMesh()[vertIndex];
		const Vertex2 &offset = tesselator->GetVertexOffset();
		lua_pushnumber( L, vert.x+offset.x );
		lua_pushnumber( L, vert.y+offset.y );
		result = 2;
	}
	
	return result;
}
	
int ShapeAdapterMesh::getUV( lua_State *L )
{
	int result = 0;
	int nextArg = 1;
	LuaUserdataProxy* sender = LuaUserdataProxy::ToProxy( L, nextArg++ );
	if(!sender) { return result; }
	
	ShapePath *path = (ShapePath *)sender->GetUserdata();
	if ( ! path ) { return result; }
	
	TesselatorMesh *tesselator =
	static_cast< TesselatorMesh * >( path->GetTesselator() );
	if ( ! tesselator ) { return result; }
	
	int vertIndex = luaL_checkint(L, nextArg++) - 1;
	if (vertIndex >= tesselator->GetUV().Length() || vertIndex < 0)
	{
		CoronaLuaWarning( L, "mesh:getVertex() index is out of bounds");
	}
	else
	{
		const Vertex2 &vert = tesselator->GetUV()[vertIndex];
		lua_pushnumber( L, vert.x );
		lua_pushnumber( L, vert.y );
		result = 2;
	}
	
	return result;
}

int ShapeAdapterMesh::setUV( lua_State *L )
{
	int result = 0;
	int nextArg = 1;
	LuaUserdataProxy* sender = LuaUserdataProxy::ToProxy( L, nextArg++ );
	if(!sender) { return result; }
	
	ShapePath *path = (ShapePath *)sender->GetUserdata();
	if ( ! path ) { return result; }
	
	TesselatorMesh *tesselator =
	static_cast< TesselatorMesh * >( path->GetTesselator() );
	if ( ! tesselator ) { return result; }
	
	int vertIndex = luaL_checkint(L, nextArg++) - 1;
	Real u = luaL_checkreal( L, nextArg++ );
	Real v = luaL_checkreal( L, nextArg++ );
	
	if (vertIndex >= tesselator->GetUV().Length() || vertIndex < 0)
	{
		luaL_argerror( L, 1, "index is out of bounds");
	}
	
	Vertex2& uv = tesselator->GetUV().WriteAccess()[vertIndex];
	if( !Rtt_RealEqual(u, uv.x) || !Rtt_RealEqual(v, uv.y))
	{
		uv.x = u;
		uv.y = v;
		
		path->Invalidate( ClosedPath::kFillSourceTexture );

		path->GetObserver()->Invalidate( DisplayObject::kGeometryFlag );
	}
	return 0;
}

int ShapeAdapterMesh::update(lua_State *L)
{
	int result = 0;
	int nextArg = 1;

	bool updated = false;
	int pathInvalidated = 0;
	int observerInvalidated = 0;

	LuaUserdataProxy *sender = LuaUserdataProxy::ToProxy(L, nextArg++);
	if(!sender) { return result; }
	
	ShapePath *path = (ShapePath *)sender->GetUserdata();
	if ( ! path ) { return result; }
	
	TesselatorMesh *tesselator =
	static_cast< TesselatorMesh * >( path->GetTesselator() );
	if ( ! tesselator ) { return result; }

	if (lua_istable(L, nextArg))
	{
		// STEVE CHANGE moved indices
		int indicesStart = 1;
		lua_getfield(L, -1, "zeroBasedIndices");
		if (lua_type(L, -1) == LUA_TBOOLEAN && lua_toboolean(L, -1)) // TODO: add parsing
		{
			indicesStart = 0;
		}
		lua_pop(L, 1);

		// STEVE CHANGE
		lua_getfield( L, -1, "adjustFromIndices" );
		bool adjustFromIndices = lua_toboolean(L, -1);
		lua_pop( L, 1 );
		
		lua_getfield( L, -1, "lowestIndex" );
		U16 lowestIndex = 0;
		if (lua_type(L, -1) == LUA_TNUMBER)
		{
			lowestIndex = lua_tointeger( L, -1 );
		}
		lua_pop( L, 1 );
		// /STEVE CHANGE

		lua_getfield(L, -1, "indices");
		U16 minIndex = std::numeric_limits<U16>::max(), maxIndex = 0; // <- STEVE CHANGE
		// STEVE CHANGE
		size_t stride;
		U16 *indices = tesselator->GetIndices().WriteAccess();
		bool changed = false;
		U32 numIndices, indicesOutputLen = (U32)tesselator->GetIndices().Length();
		const unsigned char* fromIndices = GetBuffer(L, sizeof(U16), 0, numIndices, stride, &indicesOutputLen);

		if (fromIndices)
		{
			for (U32 i = 0; i < numIndices; i++)
			{
				U16 newIndex = GetValueFromStream<U16>( fromIndices, stride );
				if (newIndex != indices[i])
				{
					changed = true;
				}
				indices[i] = newIndex;
				UpdateIndexRange( newIndex, minIndex, maxIndex );
			}
		}
		// /STEVE CHANGE
		else if (lua_istable(L, -1)) //<- STEVE CHANGE
		{
			// STEVE CHANGED moved indices
			/*U32 */numIndices = (U32)lua_objlen(L, -1); // <- STEVE CHANGE
			if (numIndices == indicesOutputLen) // <- STEVE CHANGE
			{
				// STEVE CHANGE moved changed
				for (U32 i = 0; i < numIndices; i++)
				{
					lua_rawgeti(L, -1, i + 1);
					U16 index = (U16)luaL_checkinteger(L, -1) - indicesStart;
					UpdateIndexRange( index, minIndex, maxIndex ); // <- STEVE CHANGE
					if (indices[i] != index)
					{
						changed = true;
						indices[i] = index;
					}
					lua_pop(L, 1);
				}
				// STEVE CHANGE moved if (changed)
			}
			else
			{
				CoronaLuaWarning(L, "Indices not updated: the amount of Indices in the mesh is less than the amount in the table"); // <- STEVE CHANGE
			}
		}
		// STEVE CHANGE moved if (changed)
		// STEVE CHANGE
		U32 baseVertex = 0;
		bool hasIndices = minIndex < std::numeric_limits<U16>::max();
		if ( !hasIndices )
		{
			baseVertex = lowestIndex;
		}
		// /STEVE CHANGE
		else if (changed) // <- STEVE CHANGE
		{
			// STEVE CHANGE
			if (hasIndices)
			{
				tesselator->SetLowestIndex(minIndex);
				if (adjustFromIndices)
				{
					for (U32 i = 0; i < numIndices; ++i)
					{
						indices[i] -= minIndex;
					}
					baseVertex = minIndex;
				}
			}

			updated = true;
			pathInvalidated |= ClosedPath::kFillSourceIndices;
			observerInvalidated |=	DisplayObject::kGeometryFlag |
									DisplayObject::kStageBoundsFlag |
									DisplayObject::kTransformFlag;
		}
		// /STEVE CHANGE moved if (changed)

		lua_pop(L, 1);
		// /STEVE CHANGE moved indices

		lua_getfield(L, -1, "vertices");
		bool updatedVertices = false; // <- STEVE CHANGE
		Vertex2 *mesh = tesselator->GetMesh().WriteAccess(); // <- STEVE CHANGE
		U32 numVertices, verticesOutputLen = (U32)tesselator->GetMesh().Length(); // <- STEVE CHANGE
		// STEVE CHANGE
		const unsigned char* fromVertices = GetBuffer(L, sizeof(Vertex2), baseVertex, numVertices, stride, &verticesOutputLen);

		if (fromVertices)
		{
			updatedVertices = true;
			CopyToOutput(mesh, fromVertices, numVertices, stride);
		}
		// /STEVE CHANGE
		else if (lua_istable(L, -1)) // <- STEVE CHANGE
		{
			/*U32 */numVertices = (U32)lua_objlen(L, -1) / 2; // <- STEVE CHANGE
			// STEVE CHANGE moved mesh
			if(numVertices == verticesOutputLen) // <- STEVE CHANGE
			{
				updatedVertices = true; // <- STEVE CHANGE
				for (U32 i = 0, j = baseVertex; i < numVertices; i++, j++) // <- STEVE CHANGE
				{
					lua_rawgeti(L, -1, 2 * j +1); // <- STEVE CHANGE
					lua_rawgeti(L, -2, 2 * j +2); // <- STEVE CHANGE

					mesh[i].x = luaL_checkreal(L, -2);
					mesh[i].y = luaL_checkreal(L, -1);

					lua_pop(L, 2);
				}

				// STEVE CHANGE moved vertex update
			}
			else
			{
				CoronaLuaWarning(L, "Vertices not updated: the amount of vertices in the mesh is less than the amount of vertices in the table"); // <- STEVE CHANGE
			}

		}
		// STEVE CHANGE moved vertex update
		if (updatedVertices)
		{
			pathInvalidated |=	ClosedPath::kFillSource |
								ClosedPath::kStrokeSource;

			observerInvalidated |=	DisplayObject::kGeometryFlag |
									DisplayObject::kStageBoundsFlag |
									DisplayObject::kTransformFlag;
			Rect r;
			for (U32 i = 0; i < numVertices; i++)
			{
				r.Union(mesh[i]);
			}

			Vertex2 vertexOffset = {0, 0};

			if (!r.IsEmpty())
			{
				r.GetCenter(vertexOffset);
				for (U32 i = 0; i < numVertices; i++)
				{
					mesh[i].x -= vertexOffset.x;
					mesh[i].y -= vertexOffset.y;
				}
			}

			tesselator->SetVertexOffset(vertexOffset);

			updated = true;
		}
		// /STEVE CHANGE moved vertex update
		lua_pop(L, 1);

		lua_getfield(L, -1, "uvs");
		bool updatedUVs = false; // <- STEVE CHANGE
		Vertex2 *uvs = tesselator->GetUV().WriteAccess(); // <- STEVE CHANGE
		U32 numUVs; // <- STEVE CHANGE
		// STEVE CHANGE
		const unsigned char* fromUVs = GetBuffer(L, sizeof(Vertex2), baseVertex, numUVs, stride, &verticesOutputLen);

		if (fromUVs)
		{
			updatedUVs = true;
			CopyToOutput(uvs, fromUVs, numUVs, stride);
		}
		// /STEVE CHANGE
		else if (lua_istable(L, -1)) // <- STEVE CHANGE
		{
			/*U32 */numUVs = (U32)lua_objlen(L, -1) / 2; // <- STEVE CHANGE
			// Vertex2 *uvs = tesselator->GetUV().WriteAccess(); <- STEVE CHANGE
			if (numUVs <= (U32)tesselator->GetUV().Length()) // <- STEVE CHANGE
			{
				updatedUVs = true; // <- STEVE CHANGE
				for (U32 i = 0, j = baseVertex; i < numUVs; i++, j++) // <- STEVE CHANGE
				{
					lua_rawgeti(L, -1, 2 * j + 1); // <- STEVE CHANGE
					lua_rawgeti(L, -2, 2 * j + 2); // <- STEVE CHANGE

					uvs[i].x = luaL_checkreal(L, -2);
					uvs[i].y = luaL_checkreal(L, -1);

					lua_pop(L, 2);
				}
				// STEVE CHANGE moved uv update
			}
			else
			{
				CoronaLuaWarning(L, "UVS not updated: the amount of UVS in the mesh is less than the amount of UVS in the table"); // <- STEVE CHANGE
			}
		}
		// STEVE CHANGE moved uv update
		if (updatedUVs)
		{
			pathInvalidated |= ClosedPath::kFillSourceTexture;
			observerInvalidated |= DisplayObject::kGeometryFlag;

			updated = true;
		}
		// /STEVE CHANGE moved uv update
		lua_pop(L, 1);
			
		// STEVE CHANGE moved indices up

		// STEVE CHANGE
		lua_getfield(L, -1, "fillVertexColors");
		U32 *fvcs = NULL; // <- STEVE CHANGE
		U32 numFVCs; // <- STEVE CHANGE
		const unsigned char* fromFVCs = GetBuffer(L, sizeof(U32), baseVertex, numFVCs, stride, &verticesOutputLen);

		if (fromFVCs)
		{
			fvcs = path->GetFillVertexColors();
			CopyToOutput(fvcs, fromFVCs, numFVCs, stride);
		}
		else if (lua_istable(L, -1))
		{
			// TODO!
		}
		lua_pop(L, 1);
		// /STEVE CHANGE

		if (updated)
		{
			path->Invalidate(pathInvalidated);
			path->GetObserver()->Invalidate(observerInvalidated);
		}
	}

	return 0;
}

int ShapeAdapterMesh::getVertexOffset( lua_State *L )
{
	int result = 0;
	int nextArg = 1;
	LuaUserdataProxy* sender = LuaUserdataProxy::ToProxy( L, nextArg++ );
	if(!sender) { return result; }

	ShapePath *path = (ShapePath *)sender->GetUserdata();
	if ( ! path ) { return result; }

	TesselatorMesh *tesselator =
	static_cast< TesselatorMesh * >( path->GetTesselator() );
	if ( ! tesselator ) { return result; }

	const Vertex2 &vert = tesselator->GetVertexOffset();
	lua_pushnumber( L, vert.x );
	lua_pushnumber( L, vert.y );
	result = 2;

	
	return result;
}

// STEVE CHANGE
int ShapeAdapterMesh::getLowestIndex( lua_State *L )
{
	int result = 0;
	int nextArg = 1;
	LuaUserdataProxy* sender = LuaUserdataProxy::ToProxy( L, nextArg++ );
	if(!sender) { return result; }

	ShapePath *path = (ShapePath *)sender->GetUserdata();
	if ( ! path ) { return result; }
	
	TesselatorMesh *tesselator =
	static_cast< TesselatorMesh * >( path->GetTesselator() );
	if ( ! tesselator ) { return result; }
	
	U16 lowestIndex = tesselator->GetLowestIndex();
	lua_pushinteger( L, lowestIndex );
	result = 1;
	
	
	return result;
}
// /STEVE CHANGE

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

