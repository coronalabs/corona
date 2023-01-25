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
	
bool
ShapeAdapterMesh::InitializeMesh(lua_State *L, int index, TesselatorMesh& tesselator )
{
	if ( !lua_istable( L, index ) )
	{
		return false;
	}
	index = Lua::Normalize( L, index );

	ArrayVertex2& mesh = tesselator.GetMesh();
	Rtt_ASSERT( mesh.Length() == 0 );
	lua_getfield( L, index, "vertices" );
	if (lua_istable( L, -1))
	{
		Rtt_ASSERT (lua_objlen( L, -1 ) % 2 == 0);
		U32 numVertices = (U32)lua_objlen( L, -1 )/2;
		mesh.Reserve( numVertices );
		for(U32 i=0; i<numVertices; i++)
		{
			lua_rawgeti( L, -1, 2*i+1 );
			lua_rawgeti( L, -2, 2*i+2 );
			if ( lua_type( L, -2 ) == LUA_TNUMBER &&
			     lua_type( L, -1 ) == LUA_TNUMBER )
			{
				Vertex2 v = { Rtt_FloatToReal(lua_tonumber( L, -2)), Rtt_FloatToReal(lua_tonumber( L, -1))};
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
	
	if (mesh.Length() < 3)
	{
		CoronaLuaError( L, "display.newMesh() at least 3 pairs of (x;y) coordinates must be provided in 'vertices' parameter" );
		return false;
	}
	
	
	ArrayVertex2& UVs = tesselator.GetUV();
	lua_getfield( L, index, "uvs" );
	if (lua_istable( L, -1))
	{
		U32 numUVs = (U32)lua_objlen( L, -1 )/2;
		if ( numUVs == (U32)mesh.Length() )
		{
			UVs.Reserve( numUVs );
			for(U32 i=0; i<numUVs; i++)
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
	lua_pop( L, 1);
	
	int indicesStart = 1;
	lua_getfield( L, index, "zeroBasedIndices" );
	if (lua_type( L, -1) == LUA_TBOOLEAN && lua_toboolean( L, -1)) // TODO: add parsing
	{
		indicesStart = 0;
	}
	lua_pop( L, 1);
	
	TesselatorMesh::ArrayIndex& indices = tesselator.GetIndices();
	lua_getfield( L, index, "indices" );
	if (lua_istable( L, -1))
	{
		U32 numIndices = (U32)lua_objlen( L, -1 );
		indices.Reserve( numIndices );
		for(U32 i=0; i<numIndices; i++)
		{
			lua_rawgeti( L, -1, i+1 );
			if ( lua_type( L, -1 ) == LUA_TNUMBER )
			{
				U16 index = lua_tointeger(L, -1) - indicesStart;
				indices.Append(index);
			}
			lua_pop( L, 1 );
		}
	}
	lua_pop( L, 1);

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

			lua_getfield(L, -1, "vertices");
			if (lua_istable(L, -1))
			{
				U32 numVertices = (U32)lua_objlen(L, -1) / 2;
				Vertex2 *mesh = tesselator->GetMesh().WriteAccess();
				if(numVertices == (U32)tesselator->GetMesh().Length())
				{
					updated = true;
					for (U32 i = 0; i < numVertices; i++)
					{
						lua_rawgeti(L, -1, 2 * i +1);
						lua_rawgeti(L, -2, 2 * i +2);

						mesh[i].x = luaL_checkreal(L, -2);
						mesh[i].y = luaL_checkreal(L, -1);

						lua_pop(L, 2);
					}

					pathInvalidated = pathInvalidated | ClosedPath::kFillSource |
									  ClosedPath::kStrokeSource;

					observerInvalidated = observerInvalidated | DisplayObject::kGeometryFlag |
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
				}
				else
				{
					CoronaLuaWarning(L, "Vertices not updated: the amount of vertices in the mesh is not equal to the amount of vertices in the table");
				}

			}

			lua_pop(L, 1);

			lua_getfield(L, -1, "uvs");
			if (lua_istable(L, -1))
			{
				U32 numUVs = (U32)lua_objlen(L, -1) / 2;
				Vertex2 *uvs = tesselator->GetUV().WriteAccess();
				if (numUVs == (U32)tesselator->GetUV().Length())
				{
					updated = true;
					for (U32 i = 0; i < numUVs; i++)
					{
						lua_rawgeti(L, -1, 2 * i + 1);
						lua_rawgeti(L, -2, 2 * i + 2);

						uvs[i].x = luaL_checkreal(L, -2);
						uvs[i].y = luaL_checkreal(L, -1);

						lua_pop(L, 2);
					}
					pathInvalidated = pathInvalidated | ClosedPath::kFillSourceTexture;
					observerInvalidated = observerInvalidated | DisplayObject::kGeometryFlag;
				}
				else
				{
					CoronaLuaWarning(L, "UVS not updated: the amount of UVS in the mesh is not equal to the amount of UVS in the table");
				}
			}
			lua_pop(L, 1);

			int indicesStart = 1;
			lua_getfield(L, -1, "zeroBasedIndices");
			if (lua_type(L, -1) == LUA_TBOOLEAN && lua_toboolean(L, -1)) // TODO: add parsing
			{
				indicesStart = 0;
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "indices");
			if (lua_istable(L, -1))
			{
				U16 *indices = tesselator->GetIndices().WriteAccess();
				U32 numIndices = (U32)lua_objlen(L, -1);
				if (numIndices == (U32)tesselator->GetIndices().Length())
				{
					bool changed = false;
					for (U32 i = 0; i < numIndices; i++)
					{
						lua_rawgeti(L, -1, i + 1);
						U16 index = (U16)luaL_checkinteger(L, -1) - indicesStart;
						if (indices[i] != index)
						{
							changed = true;
							indices[i] = index;
						}
						lua_pop(L, 1);
					}

					if (changed)
					{
						updated = true;
						pathInvalidated = pathInvalidated | ClosedPath::kFillSourceIndices;
						observerInvalidated = observerInvalidated | DisplayObject::kGeometryFlag |
												DisplayObject::kStageBoundsFlag |
												DisplayObject::kTransformFlag;
					}
				}
				else
				{
					CoronaLuaWarning(L, "Indices not updated: the amount of Indices in the mesh is not equal to the amount of UVS in the table");
				}
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


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

