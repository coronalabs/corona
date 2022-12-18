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
static int MarkBuffer( lua_State *L )
{
	lua_settop(L, 1);
	lua_pushboolean(L, 1);
	lua_rawset( L, lua_upvalueindex(1) );

	return 1;
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
// STEVE HACK
lua_getfield( L, LUA_REGISTRYINDEX, "_BufferWT" );
bool exists = !lua_isnil( L, -1 );
lua_pop( L, 1 );
if (!exists)
{
	lua_newtable( L );
	lua_createtable( L, 0, 1 );
	lua_pushliteral( L, "k" );
	lua_setfield( L, -2, "__mode" );
	lua_setmetatable( L, -1 );
	lua_pushvalue( L, -1 );
	lua_pushcclosure( L, MarkBuffer, 1 );
	lua_setfield( L, LUA_REGISTRYINDEX, "_MarkBuffer" );
	lua_setfield( L, LUA_REGISTRYINDEX, "_BufferWT" );
}
// /STEVE HACK
	// STEVE CHANGE moved indices
	int indicesStart = 1;
	lua_getfield( L, index, "zeroBasedIndices" );
	if (lua_type( L, -1 ) == LUA_TBOOLEAN && lua_toboolean( L, -1 )) // TODO: add parsing
	{
		indicesStart = 0;
	}
	lua_pop( L, 1);
	// STEVE CHANGE
	lua_getfield( L, index, "rebaseIndices" );
	bool rebaseIndices = lua_toboolean( L, -1 );
	lua_pop( L, 1 );

	lua_getfield( L, index, "vertexCount" );
	U32 vertexCount = std::numeric_limits<U32>::max();
	if ( lua_type( L, -1 ) == LUA_TNUMBER )
	{
		vertexCount = lua_tointeger( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, index, "hasUVs" );
	bool hasUVs = lua_toboolean( L, -1 );
	lua_pop( L, 1 );
	// /STEVE CHANGE
	
	TesselatorMesh::ArrayIndex& indices = tesselator.GetIndices();
	U16 minIndex = std::numeric_limits<U16>::max(), maxIndex = 0; // <- STEVE CHANGE
	U32 baseOffset = 0; // <- STEVE CHANGE
	lua_getfield( L, index, "indices" );
	if (lua_istable( L, -1 ))
	{
		U32 numIndices = (U32)lua_objlen( L, -1 );
		indices.Reserve( numIndices );
		for(U32 i=0; i<numIndices; i++)
		{
			lua_rawgeti( L, -1, i+1 );
			if ( lua_type( L, -1 ) == LUA_TNUMBER )
			{
				U16 index = lua_tointeger( L, -1 ) - indicesStart;
				minIndex = index < minIndex ? index : minIndex; // <- STEVE CHANGE
				maxIndex = index > maxIndex ? index : maxIndex; // <- STEVE CHANGE
				indices.Append(index);
			}
			lua_pop( L, 1 );
		}
		// STEVE CHANGE
		if ( rebaseIndices && minIndex < std::numeric_limits<U16>::max() )
		{
			baseOffset += minIndex;
			for ( U32 i = 0; i < numIndices; i++ )
			{
				indices.WriteAccess()[i] -= minIndex;
			}
		}
		// /STEVE CHANGE
	}
	// STEVE CHANGE
	else
	{
		lua_getfield( L, index, "indexCount" );
		if ( lua_type( L, -1 ) == LUA_TNUMBER )
		{
			U32 numIndices = lua_tointeger( L, -1 );
			if ( numIndices > 0 )
			{
				indices.PadToSize( numIndices, minIndex );
			}
		}
		lua_pop( L, 1 );
	}
	// /STEVE CHANGE
	lua_pop( L, 1);
	// /STEVE CHANGE moved indices

	ArrayVertex2& mesh = tesselator.GetMesh();
	Rtt_ASSERT( mesh.Length() == 0 );
	lua_getfield( L, index, "vertices" );
	if (lua_istable( L, -1 ))
	{
		Rtt_ASSERT (lua_objlen( L, -1 ) % 2 == 0);
		U32 numVertices = (U32)lua_objlen( L, -1 )/2;
		// STEVE CHANGE
		if ( rebaseIndices && minIndex < std::numeric_limits<U16>::max() )
		{
			numVertices = maxIndex + 1;
		}
		// /STEVE CHANGE
		mesh.Reserve( numVertices - baseOffset ); // <- STEVE CHANGE
		for(U32 i=baseOffset; i<numVertices; i++) // <- STEVE CHANGE
		{
			lua_rawgeti( L, -1, 2*i+1 );
			lua_rawgeti( L, -2, 2*i+2 );
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
		numVertices -= baseOffset; // <- STEVE CHANGE
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
	// STEVE CHANGE
	else if (vertexCount < std::numeric_limits<U32>::max())
	{
		Vertex2 zero = {0, 0};
		mesh.PadToSize( vertexCount - baseOffset, zero );
		tesselator.SetVertexOffset( zero );
	}
	// /STEVE CHANGE
	lua_pop( L, 1);
	
	// STEVE CHANGE moved count check down
	
	
	ArrayVertex2& UVs = tesselator.GetUV();
	lua_getfield( L, index, "uvs" );
	if (lua_istable( L, -1))
	{
		U32 numUVs = (U32)lua_objlen( L, -1 )/2;
		// STEVE CHANGE
		if ( rebaseIndices && minIndex < std::numeric_limits<U16>::max() )
		{
			numUVs = maxIndex + 1;
		}
		// /STEVE CHANGE
		if ( numUVs - baseOffset == (U32)mesh.Length() ) // <- STEVE CHANGE
		{
			UVs.Reserve( numUVs - baseOffset ); // <- STEVE CHANGE
			for(U32 i=baseOffset; i<numUVs; i++) // <- STEVE CHANGE
			{
				lua_rawgeti( L, -1, 2*i+1 );
				lua_rawgeti( L, -2, 2*i+2 );
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
	// STEVE CHANGE
	else if (hasUVs)
	{
		Vertex2 zero = {0, 0};
		UVs.PadToSize( mesh.Length(), zero );
	}
	// /STEVE CHANGE
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
	};
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 6, 15, 7, __FILE__, __LINE__ );
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

// STEVE CHANGE


static const unsigned char* AuxGetBuffer( lua_State *L, int index, size_t valueSize, size_t &len, size_t &n )
{
	lua_getfield( L, LUA_REGISTRYINDEX, "_BufferWT" ); // ..., buffer, ..., wt
	lua_pushvalue( L, index ); // ..., buffer, ..., wt, buffer
	lua_rawget( L, -2 ); // ..., buffer, ..., wt, is_buffer

	unsigned char* buffer = NULL;

	if ( lua_toboolean( L, -1 ) )
	{
		buffer = static_cast< unsigned char* >( lua_touserdata( L, index ) );
		len = lua_objlen( L, index );
		n = len / valueSize;
	}

	lua_pop( L, 2 ); // ..., buffer, ...

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

static const unsigned char* GetBuffer( lua_State *L, int index, size_t valueSize, size_t outputLength, size_t element, size_t &n, size_t &stride )
{
	index = CoronaLuaNormalize( L, index );

	size_t len;

	const unsigned char* buffer = AuxGetBuffer( L, index, valueSize, len, n );

	if ( !buffer && lua_istable( L, index ) )
	{
		lua_getfield( L, index, "buffer" ); // ..., t, ..., buffer

		buffer = AuxGetBuffer( L, lua_gettop( L ), valueSize, len, n );

		lua_pop( L, 1 ); // ..., t, ...

		if (buffer)
		{
			lua_getfield( L, index, "count" ); // ..., t, ..., count
			lua_getfield( L, index, "stride" ); // ..., t, ..., count, stride
			lua_getfield( L, index, "offset" ); // ..., t, ..., count, stride, offset

			// Supplied own `count`? If a `stride` of 0 is not explicitly supplied, this is expected to be <= what the
			// buffer can hold (`size` / `stride`, where the latter defaults to value size).
			size_t count = 0;

			if ( lua_type( L, -3 ) == LUA_TNUMBER )
			{
				count = lua_tointeger( L, -3 );
			}

			// Can it also fit in the output?
			if ( count > outputLength )
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

			lua_pop( L, 3 ); // ..., t, ...
		}
	}

	return buffer;
}

template<typename T> void CopyToOutput (T * to, const unsigned char * from, size_t count, size_t stride)
{
	if ( sizeof( T ) == stride )
	{
		memcpy( to, from, count * sizeof( T ) );
	}
	else
	{
		for ( U32 i = 0; i < count; i++, from += stride )
		{
			memcpy( &to[i], from, sizeof( T ) );
		}
	}
}

// /STEVE CHANGE

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
			lua_getfield( L, -1, "rebaseIndices" );
			bool rebaseIndices = lua_toboolean(L, -1);
			lua_pop( L, 1 );
			// /STEVE CHANGE

			lua_getfield(L, -1, "indices");
			U16 minIndex = std::numeric_limits<U16>::max(), maxIndex = 0; // <- STEVE CHANGE
			// STEVE CHANGE
			size_t stride;
			U16 *indices = tesselator->GetIndices().WriteAccess();
			bool changed = false;
			U32 numIndices, indicesOutputLen = (U32)tesselator->GetIndices().Length();
			const unsigned char* fromIndices = GetBuffer(L, -1, sizeof(U16), indicesOutputLen, 0, numIndices, stride);

			if (fromIndices)
			{
				for (U32 i = 0; i < numIndices; i++)
				{
					U16 newIndex;
					memcpy(&newIndex, fromIndices, sizeof(U16));
					changed |= newIndex != indices[i];
					indices[i] = newIndex;
					minIndex = newIndex < minIndex ? newIndex : minIndex;
					maxIndex = newIndex > maxIndex ? newIndex : maxIndex;
					fromIndices += stride;
				}
			}
			// /STEVE CHANGE
			else if (lua_istable(L, -1)) //<- STEVE CHANGE
			{
				// STEVE CHANGED moved indices
				/*U32 */numIndices = (U32)lua_objlen(L, -1); // <- STEVE CHANGE
				if (numIndices <= indicesOutputLen) // <- STEVE CHANGE
				{
					// STEVE CHANGE moved changed
					for (U32 i = 0; i < numIndices; i++)
					{
						lua_rawgeti(L, -1, i + 1);
						U16 index = (U16)luaL_checkinteger(L, -1) - indicesStart;
						minIndex = index < minIndex ? index : minIndex; // <- STEVE CHANGE
						maxIndex = index > maxIndex ? index : maxIndex; // <- STEVE CHANGE
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
			U32 baseOffset = 0; // <- STEVE CHANGE
			if (changed)
			{
				// STEVE CHANGE
				if (rebaseIndices && minIndex < std::numeric_limits<U16>::max())
				{
					for (U32 i = 0; i < numIndices; ++i)
					{
						indices[i] -= minIndex;
					}
					baseOffset += minIndex;
				}

				// Pad any shortfall with degenerate triangles.
				if (numIndices < indicesOutputLen)
				{
					U16 last = indices[numIndices - 1]; // if anything changed, numIndices must be > 0
					for (U32 i = numIndices; i < indicesOutputLen; ++i)
					{
						indices[i] = last;
					}
				}
				// /STEVE CHANGE

				updated = true;
				pathInvalidated = pathInvalidated | ClosedPath::kFillSourceIndices;
				observerInvalidated = observerInvalidated | DisplayObject::kGeometryFlag |
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
			const unsigned char* fromVertices = GetBuffer(L, -1, sizeof(Vertex2), verticesOutputLen, baseOffset, numVertices, stride);

			if (fromVertices)
			{
				updatedVertices = true;
				if (rebaseIndices && minIndex < std::numeric_limits<U16>::max())
				{
					numVertices = maxIndex - minIndex + 1;
				}
				CopyToOutput(mesh, fromVertices, numVertices, stride);
			}
			// /STEVE CHANGE
			else if (lua_istable(L, -1)) // <- STEVE CHANGE
			{
				/*U32 */numVertices = (U32)lua_objlen(L, -1) / 2; // <- STEVE CHANGE
				// STEVE CHANGE moved mesh
				if(numVertices <= verticesOutputLen) // <- STEVE CHANGE
				{
					// STEVE CHANGE
					if (rebaseIndices && minIndex < std::numeric_limits<U16>::max())
					{
						numVertices = maxIndex - minIndex + 1;
					}
					// /STEVE CHANGE
					updatedVertices = true; // <- STEVE CHANGE
					for (U32 i = 0, j = baseOffset; i < numVertices; i++, j++) // <- STEVE CHANGE
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

				// Pad any shortfall with zero-valued vertices, unless there is an index buffer too.
				if (!indices && numVertices < verticesOutputLen)
				{
					memset(mesh + numVertices, 0, (verticesOutputLen - numVertices) * sizeof(Vertex2));
				}

				updated = true;
			}
			// /STEVE CHANGE moved vertex update
			lua_pop(L, 1);

			lua_getfield(L, -1, "uvs");
			bool updatedUVs = false; // <- STEVE CHANGE
			Vertex2 *uvs = tesselator->GetUV().WriteAccess(); // <- STEVE CHANGE
			U32 numUVs, uvsOutputLen = (U32)tesselator->GetUV().Length(); // <- STEVE CHANGE
			// STEVE CHANGE
			const unsigned char* fromUVs = GetBuffer(L, -1, sizeof(Vertex2), uvsOutputLen, baseOffset, numUVs, stride);

			if (fromUVs)
			{
				updatedUVs = true;
				if (rebaseIndices && minIndex < std::numeric_limits<U16>::max())
				{
					numUVs = maxIndex - minIndex + 1;
				}
				CopyToOutput(uvs, fromUVs, numUVs, stride);
			}
			// /STEVE CHANGE
			else if (lua_istable(L, -1)) // <- STEVE CHANGE
			{
				/*U32 */numUVs = (U32)lua_objlen(L, -1) / 2; // <- STEVE CHANGE
				// Vertex2 *uvs = tesselator->GetUV().WriteAccess(); <- STEVE CHANGE
				if (numUVs <= uvsOutputLen) // <- STEVE CHANGE
				{
					// STEVE CHANGE
					if (rebaseIndices && minIndex < std::numeric_limits<U16>::max())
					{
						numUVs = maxIndex - minIndex + 1;
					}
					// /STEVE CHANGE
					updatedUVs = true; // <- STEVE CHANGE
					for (U32 i = 0, j = baseOffset; i < numUVs; i++, j++) // <- STEVE CHANGE
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

				// Pad any shortfall with zero-valued uvs, unless there is an index buffer too.
				if (!indices && numUVs < uvsOutputLen)
				{
					memset(uvs + numUVs, 0, (uvsOutputLen - numUVs) * sizeof(Vertex2));
				}

				updated = true;
			}
			// /STEVE CHANGE moved uv update
			lua_pop(L, 1);
			
			// STEVE CHANGE moved indices up

			// STEVE CHANGE
			lua_getfield(L, -1, "fillVertexColors");
			U32 *fvcs = NULL; // <- STEVE CHANGE
			U32 numFVCs, fvcsOutputLen = path->GetFillVertexCount(); // <- STEVE CHANGE
			const unsigned char* fromFVCs = GetBuffer(L, -1, sizeof(U32), fvcsOutputLen, baseOffset, numFVCs, stride);

			if (fromFVCs)
			{
				// STEVE CHANGE
				if (rebaseIndices && minIndex < std::numeric_limits<U16>::max())
				{
					numFVCs = maxIndex - minIndex + 1;
				}
				// /STEVE CHANGE
				fvcs = path->GetFillVertexColors();
				CopyToOutput(fvcs, fromFVCs, numFVCs, stride);
			}
			else if (lua_istable(L, -1))
			{
				// TODO!
			}

			if (fvcs)
			{
				observerInvalidated |= DisplayObject::kGeometryFlag | DisplayObject::kColorFlag;

				// Pad any shortfall with white, unless there is an index buffer too.
				if (!indices && numFVCs < fvcsOutputLen)
				{
					Color white = ColorWhite();
					for (int i = 0, iMax = fvcsOutputLen - numFVCs; i < iMax; i++)
					{
						fvcs[numFVCs + i] = white;
					}
				}

				updated = true;
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


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

