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
#include "CoronaMemory.h"

#include <limits>

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

static bool IsValidBuffer( lua_State *L, const void* &buffer, size_t &len )
{
    CoronaMemoryAcquireState state;

    if ( CoronaMemoryAcquireInterface( L, -1, &state ) && CORONA_MEMORY_HAS( state, getReadableBytes ) )
    {
        buffer = CORONA_MEMORY_GET( state, ReadableBytes );
        len = CORONA_MEMORY_GET( state, ByteCount );
        
        return true;
    }
    
    else
    {
        return false;
    }
}

static const unsigned char* AuxGetBuffer( lua_State *L, size_t valueSize, size_t &len, U32 &n )
{
    const void* buffer = NULL;

    if ( IsValidBuffer( L, buffer, len ) )
    {
        n = (U32)(len / valueSize);
    }

    return static_cast< const unsigned char * >( buffer );
}

static const unsigned char* IssueWarning( lua_State *L, const unsigned char* buffer, const char * warning )
{
	if (buffer)
	{
		CoronaLuaWarning( L, "%s", warning );
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
				buffer = IssueWarning( L, buffer, "Too many values to fit in output" );
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
					buffer = IssueWarning( L, buffer, "`stride` is too low" );
				}

				else if ( 0 == stride )
				{
					if ( 0 == count )
					{
						buffer = IssueWarning( L, buffer, "Explicit zero `stride` expects `count`" );
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
					buffer = IssueWarning( L, buffer, "Buffer not large enough to supply any values" );
				}

				else if ( count > n )
				{
					buffer = IssueWarning( L, buffer, "Buffer not large enough to supply `count` values" );
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
					buffer = IssueWarning( L, buffer, "Buffer not large enough to supply (repeated) value" );
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

static bool HasIndices( U16 minIndex, U16 maxIndex )
{
	return maxIndex > 0 // i.e. has changed from default of 0?
		|| 0 == minIndex; // i.e. maxIndex == 0 but minIndex no longer max unsigned U16?
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

bool
ShapeAdapterMesh::InitializeMesh(lua_State *L, int index, TesselatorMesh& tesselator, bool hasZ )
{
	if ( !lua_istable( L, index ) )
	{
		return false;
	}
	index = Lua::Normalize( L, index );

	int indicesStart = 1;
	lua_getfield( L, index, "zeroBasedIndices" );
	if (lua_type( L, -1 ) == LUA_TBOOLEAN && lua_toboolean( L, -1 )) // TODO: add parsing
	{
		indicesStart = 0;
	}
	lua_pop( L, 1);

	lua_getfield( L, index, "narrowIndexedRanges" );
	bool narrowIndexedRanges = lua_toboolean( L, -1 );
	lua_pop( L, 1 );
	
	TesselatorMesh::ArrayIndex& indices = tesselator.GetIndices();
	U16 minIndex = std::numeric_limits<U16>::max(), maxIndex = 0;
	U32 baseVertex = 0, vertexCount = 0;
	lua_getfield( L, index, "indices" );

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

	else if (lua_istable( L, -1 ))
	{
		U32 numIndices = (U32)lua_objlen( L, -1 );
		indices.Reserve( numIndices );
		for(U32 i=0; i<numIndices; i++)
		{
			lua_rawgeti( L, -1, i+1 );
			if ( lua_type( L, -1 ) == LUA_TNUMBER )
			{
				U16 index = lua_tointeger( L, -1 ) - indicesStart;
				UpdateIndexRange( index, minIndex, maxIndex );
				indices.Append(index);
			}
			lua_pop( L, 1 );
		}
	}

	bool hasIndices = HasIndices( minIndex, maxIndex );
	if ( hasIndices )
	{
		tesselator.SetLowestIndex( minIndex );
		
		if ( narrowIndexedRanges )
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
	lua_pop( L, 1);

	ArrayVertex2& mesh = tesselator.GetMesh();
	U32 numVertices;
	Rtt_ASSERT( mesh.Length() == 0 );
	lua_getfield( L, index, "vertices" );

	const unsigned char* fromVertices = GetBuffer(L, sizeof(Vertex2), baseVertex, numVertices, stride);

	if (fromVertices)
	{
		if ( vertexCount )
		{
			numVertices = vertexCount;
		}
		mesh.Reserve( numVertices );
		for ( U32 i = 0; i < numVertices; i++ )
		{
			Vertex2 v = GetValueFromStream<Vertex2>( fromVertices, stride );
			mesh.Append( v );
		}
	}

	else if (lua_istable( L, -1 )) 
	{
    int componentCount = hasZ ? 3 : 2;

    Rtt_ASSERT (lua_objlen( L, -1 ) % componentCount == 0);
    U32 numVertices = (U32)lua_objlen( L, -1 )/componentCount;

		if ( vertexCount )
		{
			numVertices = vertexCount;
		}
		mesh.Reserve( numVertices );
		for(U32 i=0,j=baseVertex; i<numVertices; i++, j++)
		{
      lua_rawgeti( L, -1, componentCount*j+1 );
      lua_rawgeti( L, -2, componentCount*j+2 );

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
	
	
	ArrayVertex2& UVs = tesselator.GetUV();
	U32 numUVs;
	lua_getfield( L, index, "uvs" );
	const unsigned char* fromUVs = GetBuffer(L, sizeof(Vertex2), baseVertex, numUVs, stride);

	if (fromVertices)
	{
		if ( vertexCount )
		{
			numUVs = vertexCount;
		}
		UVs.Reserve( numUVs );
		for ( U32 i = 0; i < numUVs; i++ )
		{
			Vertex2 v = GetValueFromStream<Vertex2>( fromUVs, stride );
			UVs.Append( v );
		}
	}
	else if (lua_istable( L, -1))
	{
		numUVs = (U32)lua_objlen( L, -1 )/2;
		if ( vertexCount )
		{
			numUVs = vertexCount;
		}
		if ( numUVs == (U32)mesh.Length() )
		{
			UVs.Reserve( numUVs );
			for(U32 i=0, j=baseVertex; i<numUVs; i++, j++)
			{
				lua_rawgeti( L, -1, 2*j+1 );
				lua_rawgeti( L, -2, 2*j+2 );
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
	
	if (mesh.Length() < 3)
	{
		CoronaLuaError( L, "display.newMesh() at least 3 pairs of (x;y) coordinates must be provided in 'vertices' parameter" );
		return false;
	}

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
		"getLowestIndex"   // 6
	};
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 7, 3, 2, __FILE__, __LINE__ );
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
		case 6:
			Lua::PushCachedFunction( L, getLowestIndex );
			break;
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

    VertexCache & cache = path->GetFillSource();
    ArrayFloat * floatArray = cache.ExtraFloatArray( DisplayPath::ZKey() );

    bool zChanged = false;

    if (floatArray && lua_isnumber( L, nextArg ))
    {
        Real z = luaL_toreal( L, nextArg );
        Real & origZ = (*floatArray)[vertIndex];

        if ( !Rtt_RealEqual( z, origZ ) )
        {
            origZ = z;
            zChanged = true;
        }
    }

	Vertex2& orig = tesselator->GetMesh().WriteAccess()[vertIndex];
	
    if( !Rtt_RealEqual(x, orig.x) || !Rtt_RealEqual(y, orig.y) || zChanged)
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

        VertexCache & cache = path->GetFillSource();
        ArrayFloat * floatArray = cache.ExtraFloatArray( DisplayPath::ZKey() );

        if (floatArray)
        {
            lua_pushnumber( L, (*floatArray)[vertIndex] );

            result = 3;
        }
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
		int indicesStart = 1;
		lua_getfield(L, -1, "zeroBasedIndices");
		if (lua_type(L, -1) == LUA_TBOOLEAN && lua_toboolean(L, -1)) // TODO: add parsing
		{
			indicesStart = 0;
		}
		lua_pop(L, 1);

		lua_getfield( L, -1, "narrowIndexedRanges" );
		bool narrowIndexedRanges = lua_toboolean(L, -1);
		lua_pop( L, 1 );
		
		lua_getfield( L, -1, "lowestIndex" );
		U16 lowestIndex = 0;
		if (lua_type(L, -1) == LUA_TNUMBER)
		{
			lowestIndex = lua_tointeger( L, -1 );
		}
		lua_pop( L, 1 );

		lua_getfield(L, -1, "indices");
		size_t stride;
		bool changed = false;
		U16 minIndex = std::numeric_limits<U16>::max(), maxIndex = 0;
		U16 *indices = tesselator->GetIndices().WriteAccess();
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
		else if (lua_istable(L, -1)) 
		{
			numIndices = (U32)lua_objlen(L, -1);
			if (numIndices == indicesOutputLen)
			{
				for (U32 i = 0; i < numIndices; i++)
				{
					lua_rawgeti(L, -1, i + 1);
					U16 index = (U16)luaL_checkinteger(L, -1) - indicesStart;
					UpdateIndexRange( index, minIndex, maxIndex );
					if (indices[i] != index)
					{
						changed = true;
						indices[i] = index;
					}
					lua_pop(L, 1);
				}
			}
			else
			{
				CoronaLuaWarning(L, "Indices not updated: the amount of Indices in the mesh is not equal to the amount in the table");
			}
		}

		U32 baseVertex = 0;
		bool hasIndices = HasIndices( minIndex, maxIndex );
		if ( !hasIndices )
		{
			baseVertex = lowestIndex;
		}
		else if (changed)
		{
			if (hasIndices)
			{
				tesselator->SetLowestIndex(minIndex);
				if (narrowIndexedRanges)
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
		lua_pop(L, 1);

		lua_getfield(L, -1, "vertices");
		bool updatedVertices = false;
		Vertex2 *mesh = tesselator->GetMesh().WriteAccess();
		U32 numVertices, verticesOutputLen = (U32)tesselator->GetMesh().Length();
		const unsigned char* fromVertices = GetBuffer(L, sizeof(Vertex2), baseVertex, numVertices, stride, &verticesOutputLen);

		if (fromVertices)
		{
			updatedVertices = true;
			CopyToOutput(mesh, fromVertices, numVertices, stride);
		}

		else if (lua_istable(L, -1))
		{
			numVertices = (U32)lua_objlen(L, -1) / 2;
			if(numVertices == verticesOutputLen)
			{
				updatedVertices = true;
				for (U32 i = 0, j = baseVertex; i < numVertices; i++, j++)
				{
					lua_rawgeti(L, -1, 2 * j +1);
					lua_rawgeti(L, -2, 2 * j +2);

					mesh[i].x = luaL_checkreal(L, -2);
					mesh[i].y = luaL_checkreal(L, -1);

					lua_pop(L, 2);
				}
			}
			else
			{
				CoronaLuaWarning(L, "Vertices not updated: the amount of vertices in the mesh is not equal to the amount in the table");
			}

		}

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
		lua_pop(L, 1);

		lua_getfield(L, -1, "uvs");
		bool updatedUVs = false;
		Vertex2 *uvs = tesselator->GetUV().WriteAccess();
		U32 numUVs;
		const unsigned char* fromUVs = GetBuffer(L, sizeof(Vertex2), baseVertex, numUVs, stride, &verticesOutputLen);

		if (fromUVs)
		{
			updatedUVs = true;
			CopyToOutput(uvs, fromUVs, numUVs, stride);
		}

		else if (lua_istable(L, -1))
		{
			numUVs = (U32)lua_objlen(L, -1) / 2;
			if (numUVs <= (U32)tesselator->GetUV().Length())
			{
				updatedUVs = true;
				for (U32 i = 0, j = baseVertex; i < numUVs; i++, j++)
				{
					lua_rawgeti(L, -1, 2 * j + 1);
					lua_rawgeti(L, -2, 2 * j + 2);

					uvs[i].x = luaL_checkreal(L, -2);
					uvs[i].y = luaL_checkreal(L, -1);

					lua_pop(L, 2);
				}
			}
			else
			{
				CoronaLuaWarning(L, "UVS not updated: the amount of UVS in the mesh is less than the amount in the table");
			}
		}

		if (updatedUVs)
		{
			pathInvalidated |= ClosedPath::kFillSourceTexture;
			observerInvalidated |= DisplayObject::kGeometryFlag;

			updated = true;
		}
		lua_pop(L, 1);
			
		lua_getfield(L, -1, "fillVertexColors");
		U32 *fvcs = NULL;
		U32 numFVCs;
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

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

