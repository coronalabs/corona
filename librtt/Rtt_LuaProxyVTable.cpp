//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Solar2D game engine.
// With contributions from Dianchu Technology
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_LuaProxyVTable.h"

#include "Rtt_FilePath.h"
#include "Display/Rtt_BitmapMask.h"
#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_ClosedPath.h"
#include "Display/Rtt_ContainerObject.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_EmbossedTextObject.h"
#include "Display/Rtt_GradientPaint.h"
#include "Display/Rtt_LineObject.h"
#include "Display/Rtt_LuaLibDisplay.h"
#include "Display/Rtt_Paint.h"
#include "Display/Rtt_RectPath.h"
#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Display/Rtt_ShapeObject.h"
#include "Display/Rtt_SnapshotObject.h"
#include "Display/Rtt_SpriteObject.h"
#include "Display/Rtt_StageObject.h"
#include "Display/Rtt_TextObject.h"
#include "Display/Rtt_TextureFactory.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_PlatformDisplayObject.h"
#include "Rtt_Runtime.h"
#include "Rtt_PhysicsWorld.h"
#include "CoronaLua.h"

#include "Rtt_ParticleSystemObject.h"
#include "Display/Rtt_EmitterObject.h"

#include "Core/Rtt_StringHash.h"

#include <string.h>

#include "Rtt_Lua.h"
#include "Rtt_Profiling.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// LuaProxyVTable
//
// ----------------------------------------------------------------------------
    
// Not sure of benefit of using lua c closures...
// Seems like it'd use more memory
// #define USE_C_CLOSURE

#ifdef USE_C_CLOSURE
const char LuaProxyVTable::kDelegateKey[] = "Delegate";
#endif
/*
LuaProxyVTable::Self*
LuaProxyVTable::GetSelf( lua_State *L, int index )
{
	Self** p = (Self**)luaL_checkudata( L, index, kDelegateKey );
	return ( p ? *p : NULL );
}

int
LuaProxyVTable::__index( lua_State *L )
{
	int result = 0;

	Self* self = GetSelf( L, 1 );
	if ( self )
	{
		const char* key = lua_tostring( L, 2 );
		result = self->ValueForKey( L, key );
	}

	return result;
}

int
LuaProxyVTable::__newindex( lua_State *L )
{
	Self* self = GetSelf( L, 1 );
	if ( self )
	{
		const char* key = lua_tostring( L, 2 );
		self->SetValueForKey( L, key, 3 );
	}

	return 0;
}

int
LuaProxyVTable::__gcMeta( lua_State *L )
{
	Self* self = GetSelf( L, 1 );
	if ( self )
	{
		Self** userdata = & self;
		Rtt_ASSERT( (Self**)luaL_checkudata( L, index, kDelegateKey ) == userdata );

		*userdata = NULL;
		Rtt_DELETE( self );
	}
}
*/

bool
LuaProxyVTable::SetValueForKey( lua_State *, MLuaProxyable&, const char [], int ) const
{
	return false;
}

/*
int
LuaProxyVTable::Length( lua_State * ) const
{
	return 0;
}
*/

const LuaProxyVTable&
LuaProxyVTable::Parent() const
{
	return * this;
}

// ----------------------------------------------------------------------------

#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
// Proxy's delegate or an ancestor must match expected
bool
LuaProxyVTable::IsProxyUsingCompatibleDelegate( const LuaProxy* proxy, const Self& expected )
{
	// if proxy is NULL, skip the check
	bool result = ( NULL == proxy );

	if ( ! result )
	{
		for( const LuaProxyVTable *child = & proxy->Delegate(), *parent = & child->Parent();
			 ! result;
			 child = parent, parent = & child->Parent() )
		{
			result = ( child == & expected );
			if ( child == parent ) { break; }
		}
	}

	return result;
}
#endif // Rtt_DEBUG

// This implements introspection
bool
LuaProxyVTable::DumpObjectProperties( lua_State *L, const MLuaProxyable& object, const char **keys, const int numKeys, String& result ) const
{
    Rtt_LUA_STACK_GUARD( L );
    const int bufLen = 10240;
	char buf[bufLen];

    // JSON encode the value of each key
    for (int k = 0; k < numKeys; k++)
    {
        Rtt_LUA_STACK_GUARD( L );

        if (strchr(keys[k], '#'))
        {
            // Deprecated property, skip it
            continue;
        }

        // Note that the "overrideRestriction" parameter is set to true so that we don't
        // restrict access to certain properties based on license tier (this means that
        // Starter users can see some properties they can't use but makes debugger logic
        // much easier)
		int res = ValueForKey(L, object, keys[k], true);

        if (res > 0)
        {
			buf[0] = '\0';

			CoronaLuaPropertyToJSON(L, -1, keys[k], buf, bufLen, 0);

			if (! result.IsEmpty() && strlen(buf) > 0)
			{
				result.Append(", ");
			}

			result.Append(buf);

			lua_pop( L, res );
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
//
// LuaDisplayObjectProxyVTable
//
// ----------------------------------------------------------------------------

/*
#define Rtt_ASSERT_PROXY_TYPE( L, index, T )	\
	Rtt_ASSERT(									\
		IsProxyUsingCompatibleDelegate(			\
			LuaProxy::GetProxy((L),(index)),	\
			Lua ## T ## ProxyVTable::Constant() ) )


#define Rtt_WARN_SIM_PROXY_TYPE2( L, index, T, API_T )		\
	Rtt_WARN_SIM(											\
		Rtt_VERIFY( IsProxyUsingCompatibleDelegate(			\
			LuaProxy::GetProxy((L),(index)),				\
			Lua ## T ## ProxyVTable::Constant() ) ),		\
		( "ERROR: Argument(%d) is not of a %s\n", index, #API_T ) )

#define Rtt_WARN_SIM_PROXY_TYPE( L, index, T )	Rtt_WARN_SIM_PROXY_TYPE2( L, index, T, T )
*/

const LuaDisplayObjectProxyVTable&
LuaDisplayObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

int
LuaDisplayObjectProxyVTable::translate( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	if ( o )
	{
		Real x = luaL_checkreal( L, 2 );
		Real y = luaL_checkreal( L, 3 );

		o->Translate( x, y );
	}

	return 0;
}

int
LuaDisplayObjectProxyVTable::scale( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		Real sx = luaL_checkreal( L, 2 );
		Real sy = luaL_checkreal( L, 3 );

		o->Scale( sx, sy, false );
	}

	return 0;
}

int
LuaDisplayObjectProxyVTable::rotate( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		Real deltaTheta = luaL_checkreal( L, 2 );

		o->Rotate( deltaTheta );
	}

	return 0;
}

static int
getParent( lua_State *L )
{
#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
	CoronaLuaWarning(L, "[Deprecated display object API] Replace object:getParent() with object.parent");
#endif

	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	int result = o ? 1 : 0;
	if ( o )
	{
		// Orphans do not have parents
		GroupObject* parent = ( ! o->IsOrphan() ? o->GetParent() : NULL );
		if ( parent )
		{
			parent->GetProxy()->PushTable( L );
		}
		else
		{
			Rtt_ASSERT( o->GetStage() == o || o->IsOrphan() );
			lua_pushnil( L );
		}
	}

	return result;
}

static int
setReferencePoint( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	int result = 0;
	if ( o )
	{
		if ( ! o->IsV1Compatibility() )
		{
			luaL_error( L, "ERROR: object:setReferencePoint() is only available in graphicsCompatibility 1.0 mode. Use anchor points instead." );
		}
		else
		{
			bool anchorChildren = true;
		
			if ( lua_isnil( L, 2 ) )
			{
				Rtt_TRACE_SIM( ( "WARNING: object:setReferencePoint() was given a 'nil' value. The behavior is not defined.\n" ) );
				o->ResetReferencePoint();

				anchorChildren = false; // Restore to base case.
			}
			else
			{
				Rtt_WARN_SIM( lua_islightuserdata( L, 2 ), ( "WARNING: Invalid reference point constant passed to object:setReferencePoint()\n" ) );

				DisplayObject::ReferencePoint location = (DisplayObject::ReferencePoint)EnumForUserdata(
					LuaLibDisplay::ReferencePoints(),
					lua_touserdata( L, 2 ),
					DisplayObject::kNumReferencePoints,
					DisplayObject::kReferenceCenter );
				o->SetReferencePoint( LuaContext::GetRuntime( L )->Allocator(), location );
			}
			
			GroupObject *g = o->AsGroupObject();
			if ( g )
			{
				g->SetAnchorChildren( anchorChildren );
			}
		}
	}

	return result;
}

// object:removeSelf()
static int
removeSelf( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	int result = 0;

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		if ( ! o->IsRenderedOffScreen() )
		{
			GroupObject* parent = o->GetParent();

			if (parent != NULL)
			{
				S32 index = parent->Find( *o );

				LuaDisplayObjectProxyVTable::PushAndRemove( L, parent, index );

				result = 1;
			}
			else
			{
#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
				CoronaLuaWarning(L, "object:removeSelf() cannot be called on objects with no parent" );
#endif
			}
		}
		else
		{
#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
			CoronaLuaWarning(L, "object:removeSelf() can only be called on objects in the scene graph. Objects that are not directly in the scene, such as a snapshot's group, cannot be removed directly" );
#endif
		}
	}
	else
	{
		lua_pushnil( L );
		result = 1;
	}

	return result;
}

static int
localToContent( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	int result = 0;
	if ( o )
	{
		Real x = luaL_checkreal( L, 2 );
		Real y = luaL_checkreal( L, 3 );

		Vertex2 v = { x, y };
		o->LocalToContent( v );

		lua_pushnumber( L, v.x );
		lua_pushnumber( L, v.y );
		result = 2;
	}

	return result;
}

static int
contentToLocal( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( Rtt_VERIFY( o ) )
	{
		Vertex2 v = { luaL_toreal( L, 2 ), luaL_toreal( L, 3 ) };
		o->ContentToLocal( v );
		lua_pushnumber( L, Rtt_RealToFloat( v.x ) );
		lua_pushnumber( L, Rtt_RealToFloat( v.y ) );
	}

	return 2;
}

// object:toFront()
static int
toFront( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( Rtt_VERIFY( o ) )
	{
		GroupObject *parent = o->GetParent();

		if (parent != NULL)
		{
			parent->Insert( -1, o, false );
		}
#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
		else
		{
			CoronaLuaWarning(L, "DisplayObject:toFront() cannot be used on a snapshot group or texture canvas cache");
		}
#endif
	}
	return 0;
}

// object:toBack()
static int
toBack( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( Rtt_VERIFY( o ) )
	{
		GroupObject *parent = o->GetParent();

		if (parent != NULL)
		{
			parent->Insert( 0, o, false );
		}
#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
		else
		{
			CoronaLuaWarning(L, "DisplayObject:toBack() cannot be used on a snapshot group or texture canvas cache");
		}
#endif
	}
	return 0;
}

// object:setMask( mask )
static int
setMask( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( Rtt_VERIFY( o ) )
	{
		Runtime *runtime = LuaContext::GetRuntime( L ); Rtt_ASSERT( runtime );

		BitmapMask *mask = NULL;

		if ( lua_isuserdata( L, 2 ) )
		{
			FilePath **ud = (FilePath **)luaL_checkudata( L, 2, FilePath::kMetatableName );
			if ( ud )
			{
				FilePath *maskData = *ud;
				if ( maskData )
				{
					mask = BitmapMask::Create( * runtime, * maskData );
				}
			}
		}

		o->SetMask( runtime->Allocator(), mask );
	}
	return 0;
}

// object:_setHasListener( name, value )
static int
setHasListener( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( Rtt_VERIFY( o ) )
	{
		const char *name = lua_tostring( L, 2 );
		DisplayObject::ListenerMask mask = DisplayObject::MaskForString( name );
		if ( DisplayObject::kUnknownListener != mask )
		{
			bool value = lua_toboolean( L, 3 );
			o->SetHasListener( mask, value );
		}
	}
	return 0;
}

/*
int
LuaDisplayObjectProxyVTable::length( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	int len = 0;

	if ( o )
	{
		GroupObject* c = o->AsGroupObject();
		if ( c ) { len = c->NumChildren(); }
	}

	lua_pushinteger( L, len );

	return 1;
}
*/
/*
// TODO: too complicated; should break apart into smaller functions???
static int
moveAbove( lua_State *L )
{
	DisplayObject* childToMove = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	DisplayObject* dstLocation = (DisplayObject*)LuaProxy::GetProxyableObject( L, 2 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );
	Rtt_WARN_SIM_PROXY_TYPE( L, 2, DisplayObject );

	// Only move if dstLocation is above the child to be moved
	if ( dstLocation
		 && childToMove
		 && dstLocation->IsAbove( * childToMove ) )
	{
		const StageObject* dstLocationStage = dstLocation->GetStage();
		const StageObject* childToMoveStage = childToMove->GetStage();

		// Only move if neither objects are the canvas
		// And the dstLocation is in the canvas object tree
		if ( dstLocationStage && dstLocationStage != dstLocation
			 && childToMoveStage != childToMove )
		{
			GroupObject* parent = dstLocation->GetParent();

			S32 index = parent->Find( *dstLocation );

			Rtt_ASSERT( index >= 0 );
			parent->Insert( index, childToMove );
		}
	}

	return 0;
}
*/

/*
int
LuaDisplayObjectProxyVTable::stageBounds( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		const Rect& r = o->StageBounds();

		lua_createtable( L, 0, 4 );

		const char xMin[] = "xMin";
		const char yMin[] = "yMin";
		const char xMax[] = "xMax";
		const char yMax[] = "yMax";
		const size_t kLen = sizeof( xMin ) - 1;

		Rtt_STATIC_ASSERT( sizeof(char) == 1 );
		Rtt_STATIC_ASSERT( sizeof(xMin) == sizeof(yMin) );
		Rtt_STATIC_ASSERT( sizeof(xMin) == sizeof(xMax) );
		Rtt_STATIC_ASSERT( sizeof(xMin) == sizeof(yMax) );

		setProperty( L, xMin, kLen, r.xMin );
		setProperty( L, yMin, kLen, r.yMin );
		setProperty( L, xMax, kLen, r.xMax );
		setProperty( L, yMax, kLen, r.yMax );

		return 1;
	}

	return 0;
}

int
LuaDisplayObjectProxyVTable::stageWidth( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		const Rect& r = o->StageBounds();
		lua_pushinteger( L, Rtt_RealToInt( r.xMax - r.xMin ) );
		return 1;
	}

	return 0;
}

int
LuaDisplayObjectProxyVTable::stageHeight( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		const Rect& r = o->StageBounds();
		lua_pushinteger( L, Rtt_RealToInt( r.yMax - r.yMin ) );
		return 1;
	}

	return 0;
}
*/

/*
int
LuaDisplayObjectProxyVTable::canvas( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxy( L, 1 )->GetProxyableObject();

	LuaStageObject::PushOrCreateProxy( L, o->GetStage() );

	return 1;
}
*/

int
LuaDisplayObjectProxyVTable::ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction /* = false */ ) const
{
	if ( ! key ) { return 0; }
	
	int result = 1;

    // deprecated properties have a trailing '#'
	static const char * keys[] = 
	{
		"translate",			// 0
		"scale",				// 1
		"rotate",				// 2
		"getParent",			// 3
		"setReferencePoint",	// 4
		"removeSelf",			// 5
		"localToContent",		// 6
		"contentToLocal",		// 7
		"stageBounds#",         // 8 - DEPRECATED
		"stageWidth#",			// 9 - DEPRECATED
		"stageHeight#",         // 10 - DEPRECATED
        "numChildren#",			// 11 - DEPRECATED
		"length#",				// 12 - DEPRECATED
		"isVisible",			// 13
		"isHitTestable",		// 14
		"alpha",				// 15
		"parent",				// 16
		"stage",				// 17
		"x",					// 18
		"y",					// 19
		"anchorX",				// 20
		"anchorY",				// 21
		"contentBounds",		// 22
		"contentWidth",         // 23
		"contentHeight",		// 24
		"toFront",				// 25
		"toBack",				// 26
		"setMask",				// 27
		"maskX",				// 28
		"maskY",				// 29
		"maskScaleX",			// 30
		"maskScaleY",			// 31
		"maskRotation",         // 32
		"isHitTestMasked",		// 33
		"_setHasListener",		// 34
	};
    const int numKeys = sizeof( keys ) / sizeof( const char * );
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 35, 33, 15, __FILE__, __LINE__ );
	StringHash *hash = &sHash;

	int index = hash->Lookup( key );
	switch ( index )
	{
	case 0:
		{
			Lua::PushCachedFunction( L, Self::translate );
		}
		break;
	case 1:
		{
			Lua::PushCachedFunction( L, Self::scale );
		}
		break;
	case 2:
		{
			Lua::PushCachedFunction( L, Self::rotate );
		}
		break;
	case 3:
		{
			Lua::PushCachedFunction( L, getParent );
		}
		break;
	case 4:
		{
			Lua::PushCachedFunction( L, setReferencePoint );
		}
		break;
	case 5:
		{
			Lua::PushCachedFunction( L, removeSelf );
		}
		break;
	case 6:
		{
			Lua::PushCachedFunction( L, localToContent );
		}
		break;
	case 7:
		{
			Lua::PushCachedFunction( L, contentToLocal );
		}
		break;
	case 25:
		{
			Lua::PushCachedFunction( L, toFront );
		}
		break;
	case 26:
		{
			Lua::PushCachedFunction( L, toBack );
		}
		break;
	case 27:
		{
			Lua::PushCachedFunction( L, setMask );
		}
		break;
	case 34:
		{
			Lua::PushCachedFunction( L, setHasListener );
		}
		break;
	default:
		{
			// DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
			const DisplayObject& o = static_cast< const DisplayObject& >( object );
			Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

			switch ( index )
			{
			case 8:
			case 22:
				{
//					Rtt_WARN_SIM( strcmp( "stageBounds", key ) != 0, ( "WARNING: object.stageBounds has been deprecated. Use object.contentBounds instead\n" ) );

					const Rect& r = o.StageBounds();

	// Good way to catch autorotate bugs for bouncebehavior:
	// Rtt_ASSERT( r.xMin >= 0 );

					lua_createtable( L, 0, 4 );

					const char xMin[] = "xMin";
					const char yMin[] = "yMin";
					const char xMax[] = "xMax";
					const char yMax[] = "yMax";
					const size_t kLen = sizeof( xMin ) - 1;

					Rtt_STATIC_ASSERT( sizeof(char) == 1 );
					Rtt_STATIC_ASSERT( sizeof(xMin) == sizeof(yMin) );
					Rtt_STATIC_ASSERT( sizeof(xMin) == sizeof(xMax) );
					Rtt_STATIC_ASSERT( sizeof(xMin) == sizeof(yMax) );

					Real xMinRect = r.xMin;
					Real yMinRect = r.yMin;
					Real xMaxRect = r.xMax;
					Real yMaxRect = r.yMax;

					if ( r.IsEmpty() )
					{
						xMinRect = yMinRect = xMaxRect = yMaxRect = Rtt_REAL_0;
					}

					setProperty( L, xMin, kLen, xMinRect );
					setProperty( L, yMin, kLen, yMinRect );
					setProperty( L, xMax, kLen, xMaxRect );
					setProperty( L, yMax, kLen, yMaxRect );
				}
				break;
			case 9:
			case 23:
				{
					Rtt_WARN_SIM( strcmp( "stageWidth", key ) != 0, ( "WARNING: object.stageWidth has been deprecated. Use object.contentWidth instead\n" ) );

					const Rect& r = o.StageBounds();
					lua_pushinteger( L, Rtt_RealToInt( r.xMax - r.xMin ) );
				}
				break;
			case 10:
			case 24:
				{
					Rtt_WARN_SIM( strcmp( "stageHeight", key ) != 0, ( "WARNING: object.stageHeight has been deprecated. Use object.contentHeight instead\n" ) );

					const Rect& r = o.StageBounds();
					lua_pushinteger( L, Rtt_RealToInt( r.yMax - r.yMin ) );
				}
				break;
			case 11:
				{
					lua_pushnil( L );
				}
				break;
			case 12:
				{
#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
					CoronaLuaWarning(L, "[Deprecated display object property] Replace object.length with group.numChildren");
#endif
					int len = 0;
					const GroupObject* c = const_cast< DisplayObject& >( o ).AsGroupObject();
					if ( c ) { len = c->NumChildren(); }

					lua_pushinteger( L, len );
				}
				break;
			case 13:
				{
					lua_pushboolean( L, o.IsVisible() );
				}
				break;
			case 14:
				{
					lua_pushboolean( L, o.IsHitTestable() );
				}
				break;
			case 15:
				{
					lua_Number alpha = (float)o.Alpha() / 255.0;
					lua_pushnumber( L, alpha );
				}
				break;
			case 16:
				{
					const StageObject *stage = o.GetStage();

					// Only onscreen objects have a parent
					if ( stage
						 && ( stage->IsOnscreen() || stage->IsRenderedOffScreen() ) )
					{
						GroupObject* parent = o.GetParent();
						if ( parent )
						{
							parent->GetProxy()->PushTable( L );
						}
						else
						{
							// Stage objects and objects rendered offscreen have no parent,
							// so push nil
							Rtt_ASSERT( o.IsRenderedOffScreen() || o.GetStage() == & o );
							lua_pushnil( L );
						}
					}
					else
					{
						// Objects that have been removed effectively have no parent,
						// so push nil. Do NOT push the offscreen parent.
						lua_pushnil( L );
					}
				}
				break;
			case 17:
				{
					const StageObject* stage = o.GetStage();
					if ( stage && stage->IsOnscreen() )
					{
						stage->GetProxy()->PushTable( L );
					}
					else
					{
						lua_pushnil( L );
					}
				}
				break;
			case 18:
				{
					Rtt_Real value = o.GetGeometricProperty( kOriginX );

					if ( o.IsV1Compatibility() && o.IsV1ReferencePointUsed() )
					{
						Vertex2 p = o.GetAnchorOffset();
						value -= p.x;
					}
					lua_pushnumber( L, value );
				}
				break;
			case 19:
				{
					Rtt_Real value = o.GetGeometricProperty( kOriginY );

					if ( o.IsV1Compatibility() && o.IsV1ReferencePointUsed() )
					{
						Vertex2 p = o.GetAnchorOffset();
						value -= p.y;
					}
					lua_pushnumber( L, value );
				}
				break;
			case 20:
				{
					Real anchorX = o.GetAnchorX();
					lua_pushnumber( L, anchorX );
				}
				break;
			case 21:
				{
					Real anchorY = o.GetAnchorY();
					lua_pushnumber( L, anchorY );
				}
				break;
			case 28:
				{
					Rtt_Real value = o.GetMaskGeometricProperty( kOriginX );
					lua_pushnumber( L, value );
				}
				break;
			case 29:
				{
					Rtt_Real value = o.GetMaskGeometricProperty( kOriginY );
					lua_pushnumber( L, value );
				}
				break;
			case 30:
				{
					Rtt_Real value = o.GetMaskGeometricProperty( kScaleX );
					lua_pushnumber( L, value );
				}
				break;
			case 31:
				{
					Rtt_Real value = o.GetMaskGeometricProperty( kScaleY );
					lua_pushnumber( L, value );
				}
				break;
			case 32:
				{
					Rtt_Real value = o.GetMaskGeometricProperty( kRotation );
					lua_pushnumber( L, value );
				}
				break;
            case 33:
                {
                    lua_pushboolean( L, o.IsHitTestMasked() );
                }
                break;

			default:
				{
					GeometricProperty p = DisplayObject::PropertyForKey( LuaContext::GetAllocator( L ), key );
					if ( p < kNumGeometricProperties )
					{
						lua_pushnumber( L, Rtt_RealToFloat( o.GetGeometricProperty( p ) ) );
					}
					else
					{
						result = 0;
					}
				}
				break;
			}
		}
		break;
	}

    // We handle this outside the switch statement (and thus keys[]) so we can enumerate all the keys[] and not include it
    if (result == 0 && strncmp(key, "_properties", strlen(key)) == 0)
    {
        String displayProperties(LuaContext::GetRuntime( L )->Allocator());
        String geometricProperties(LuaContext::GetRuntime( L )->Allocator());
        const char **geometricKeys = NULL;
        const int geometricNumKeys = DisplayObject::KeysForProperties(geometricKeys);

        // "GeometricProperties" are derived from the object's geometry and thus handled separately from other properties
        const DisplayObject& o = static_cast< const DisplayObject& >( object );
        for ( int i = 0; i < geometricNumKeys; i++ )
        {
            const int bufLen = 10240;
            char buf[bufLen];

            GeometricProperty p = DisplayObject::PropertyForKey( LuaContext::GetAllocator( L ), geometricKeys[i] );

            if (strchr(geometricKeys[i], '#'))
            {
                // Deprecated property, skip it
                continue;
            }

            if ( p < kNumGeometricProperties )
            {
                snprintf(buf, bufLen, "\"%s\": %g", geometricKeys[i], Rtt_RealToFloat( o.GetGeometricProperty( p ) ) );

                if (! geometricProperties.IsEmpty() && strlen(buf) > 0)
                {
                    geometricProperties.Append(", ");
                }
                
                geometricProperties.Append(buf);
            }
        }

        DumpObjectProperties( L, object, keys, numKeys, displayProperties );

        const LuaProxyVTable *extensions = LuaProxy::GetProxy(L, 1)->GetExtensionsDelegate();
        if ( extensions )
        {
            result = extensions->ValueForKey( L, object, key );

            if (result == 1)
            {
                displayProperties.Append( ", " );
                displayProperties.Append( lua_tostring( L, -1 ) );
            }
        }

        lua_pushfstring( L, "%s, %s", geometricProperties.GetString(), displayProperties.GetString() );

        result = 1;
    }
    else if ( result == 0 && strcmp( key, "_type" ) == 0 )
    {
        const DisplayObject& o = static_cast< const DisplayObject& >( object );

        lua_pushstring( L, o.GetObjectDesc() );

        result = 1;
    }
    else if ( result == 0 && strcmp( key, "_defined" ) == 0 )
    {
        const DisplayObject& o = static_cast< const DisplayObject& >( object );

        lua_pushstring( L, o.fWhereDefined );

        result = 1;
    }
    else if ( result == 0 && strcmp( key, "_lastChange" ) == 0 )
    {
        const DisplayObject& o = static_cast< const DisplayObject& >( object );

        lua_pushstring( L, o.fWhereChanged );

        result = 1;
    }

	return result;
}

bool
LuaDisplayObjectProxyVTable::SetValueForKey( lua_State *L, MLuaProxyable& object, const char key[], int valueIndex ) const
{
	if ( ! key ) { return false; }

	// DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	DisplayObject& o = static_cast< DisplayObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	bool result = true;

	static const char * keys[] =
	{
		"isVisible",			// 0
		"isHitTestable",		// 1
		"alpha",				// 2
		"parent",				// 3
		"stage",				// 4
		"x",					// 5
		"y",					// 6
		"anchorX",				// 7
		"anchorY",				// 8
		"stageBounds",			// 9
		"maskX",				// 10
		"maskY",				// 11
		"maskScaleX",			// 12
		"maskScaleY",			// 13
		"maskRotation",         // 14
		"isHitTestMasked",		// 15
	};
    const int numKeys = sizeof( keys ) / sizeof( const char * );
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 16, 12, 6, __FILE__, __LINE__ );
	StringHash *hash = &sHash;

	int index = hash->Lookup( key );
	switch ( index )
	{
	case 0:
		{
			o.SetVisible( lua_toboolean( L, valueIndex ) != 0 );
		}
		break;
	case 1:
		{
			o.SetHitTestable( lua_toboolean( L, valueIndex ) != 0 );
		}
		break;
	case 2:
		{
            /* too verbose:
			Rtt_WARN_SIM(
				lua_tonumber( L, valueIndex ) >= 0. && lua_tonumber( L, valueIndex ) <= 1.0,
				( "WARNING: Attempt to set object.alpha to %g which is outside valid range. It will be clamped to the range [0,1]\n", lua_tonumber( L, valueIndex ) ) );
             */

			// Explicitly declare T b/c of crappy gcc compiler used by Symbian
			lua_Integer alpha = (lua_Integer)(lua_tonumber( L, valueIndex ) * 255.0f);
			lua_Integer value = Min( (lua_Integer)255, alpha );
			U8 newValue = Max( (lua_Integer)0, value );

			o.SetAlpha( newValue );
		}
		break;
	case 3:
		{
			// No-op for read-only property
		}
		break;
	case 4:
		{
			// No-op for read-only property
		}
		break;
	case 5:
		{
			Real newValue = luaL_toreal( L, valueIndex );

			if ( o.IsV1Compatibility() && o.IsV1ReferencePointUsed() )
			{
				Vertex2 p = o.GetAnchorOffset();
				newValue += p.x;
			}

			o.SetGeometricProperty( kOriginX, newValue );
		}
		break;
	case 6:
		{
			Real newValue = luaL_toreal( L, valueIndex );

			if ( o.IsV1Compatibility() && o.IsV1ReferencePointUsed() )
			{
				Vertex2 p = o.GetAnchorOffset();
				newValue += p.y;
			}

			o.SetGeometricProperty( kOriginY, newValue );
		}
		break;
	case 7:
		{
			if ( lua_type( L, valueIndex ) == LUA_TNUMBER )
			{
				Real newValue = luaL_toreal( L, valueIndex );
				if ( o.GetStage()->GetDisplay().GetDefaults().IsAnchorClamped() )
				{
					newValue = Clamp( newValue, Rtt_REAL_0, Rtt_REAL_1 );
				}
				o.SetAnchorX( newValue );
				
#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
				if ( o.IsV1Compatibility() )
                {
                    CoronaLuaWarning(L, "o.anchorX is only supported in graphics 2.0. Your mileage may vary in graphicsCompatibility 1.0 mode");
                }
#endif
			}
			else
			{
				luaL_error( L, "ERROR: o.anchorX can only be set to a number.\n" );
			}
			
		}
		break;
	case 8:
		{
			if ( lua_type( L, valueIndex) == LUA_TNUMBER )
			{
				Real newValue = luaL_toreal( L, valueIndex );
				if ( o.GetStage()->GetDisplay().GetDefaults().IsAnchorClamped() )
				{
					newValue = Clamp( newValue, Rtt_REAL_0, Rtt_REAL_1 );
				}
				o.SetAnchorY( newValue );
				
#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
				if ( o.IsV1Compatibility() )
                {
                    CoronaLuaWarning(L, "o.anchorY is only supported in graphics 2.0. Your mileage may vary in graphicsCompatibility 1.0 mode");
                }
#endif
			}
			else
			{
				luaL_error( L, "ERROR: o.anchorY can only be set to a number.\n" );
			}
			
		}
		break;
	case 9:
		{
			// No-op for read-only keys
		}
		break;
	case 10:
		{
			Real newValue = luaL_toreal( L, valueIndex );
			o.SetMaskGeometricProperty( kOriginX, newValue );
		}
		break;
	case 11:
		{
			Real newValue = luaL_toreal( L, valueIndex );
			o.SetMaskGeometricProperty( kOriginY, newValue );
		}
		break;
	case 12:
		{
			Real newValue = luaL_toreal( L, valueIndex );
			o.SetMaskGeometricProperty( kScaleX, newValue );
		}
		break;
	case 13:
		{
			Real newValue = luaL_toreal( L, valueIndex );
			o.SetMaskGeometricProperty( kScaleY, newValue );
		}
		break;
	case 14:
		{
			Real newValue = luaL_toreal( L, valueIndex );
			o.SetMaskGeometricProperty( kRotation, newValue );
		}
		break;
	case 15:
		{
			o.SetHitTestMasked( lua_toboolean( L, valueIndex ) != 0 );
		}
		break;
	default:
		{
			GeometricProperty p = DisplayObject::PropertyForKey( LuaContext::GetAllocator( L ), key );
			if ( p < kNumGeometricProperties )
			{
				Real newValue = luaL_toreal( L, valueIndex );
				o.SetGeometricProperty( p, newValue );
			}
			else if ( ! lua_isnumber( L, 2 ) )
			{
				result = false;
			}
		}
		break;
	}

    // We changed a property so record where we are so that "_lastChange" will be available later to say where it happened
    // (this is a noop on non-debug builds because lua_where returns an empty string)
    if (result)
    {
        luaL_where(L, 1);
        const char *where = lua_tostring( L, -1 );

        if (where[0] != 0)
        {
            if (o.fWhereChanged != NULL)
            {
                free((void *) o.fWhereChanged);
            }

            // If this fails, the pointer will be NULL and that's handled gracefully
            o.fWhereChanged = strdup(where);
        }

        lua_pop(L, 1);
    }

    return result;
}

// ----------------------------------------------------------------------------

const LuaLineObjectProxyVTable&
LuaLineObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

int
LuaLineObjectProxyVTable::setStrokeColor( lua_State *L )
{
	LineObject* o = (LineObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, LineObject );

	if ( o )
	{
		if ( ! o->GetPath().GetStroke() )
		{
			Paint* p = LuaLibDisplay::LuaNewColor( L, 2, o->IsByteColorRange() );
			o->SetStroke( p );
		}
		else
		{
			Color c = LuaLibDisplay::toColor( L, 2, o->IsByteColorRange() );
			o->SetStrokeColor( c );
		}
	}

	return 0;
}

static Paint*
DefaultPaint( lua_State *L, bool isBytes )
{
	lua_pushnumber( L, 1.0 ); // gray value, i.e. white

	Paint* p = LuaLibDisplay::LuaNewColor( L, lua_gettop( L ), isBytes );

	lua_pop( L, 1 );

	return p;
}

int
LuaLineObjectProxyVTable::setStrokeVertexColor( lua_State *L )
{
	LineObject* o = (LineObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, LineObject );

	if ( o )
	{
		OpenPath& path = o->GetPath();
		if ( ! path.GetStroke() )
		{
			o->SetStroke( DefaultPaint( L, o->IsByteColorRange() ) );
		}

		U32 index = lua_tointeger( L, 2 ) - 1U;
		Color c = LuaLibDisplay::toColor( L, 3, o->IsByteColorRange() );

		if (path.SetStrokeVertexColor( index, c ))
		{
			path.GetObserver()->Invalidate( DisplayObject::kGeometryFlag | DisplayObject::kColorFlag );
		}
	}

	return 0;
}

// object.stroke
int
LuaLineObjectProxyVTable::setStroke( lua_State *L )
{
	// This thin wrapper is necessary for Lua::PushCachedFunction().
	return setStroke( L, 2 );
}

// object.stroke
int
LuaLineObjectProxyVTable::setStroke( lua_State *L, int valueIndex )
{
	LineObject* o = (LineObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, LineObject );

	if ( Rtt_VERIFY( o ) )
	{
		if ( ! o->IsRestricted()
			 || ! o->GetStage()->GetDisplay().ShouldRestrict( Display::kLineStroke ) )
		{
			// Use factory method to create paint
			Paint *paint = LuaLibDisplay::LuaNewPaint( L, valueIndex );

			o->SetStroke( paint );
		}
	}
	return 0;
}

int
LuaLineObjectProxyVTable::append( lua_State *L )
{
	LineObject* o = (LineObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, LineObject );

	if ( o )
	{
		// number of parameters (excluding self)
		int numArgs = lua_gettop( L ) - 1;

		// iMax must be even
		for ( int i = 2, iMax = (numArgs & ~0x1); i <= iMax; i+=2 )
		{
			Vertex2 v = { luaL_checkreal( L, i ), luaL_checkreal( L, i + 1 ) };
			o->Append( v );
		}
	}

	return 0;
}

int
LuaLineObjectProxyVTable::ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction /* = false */ ) const
{
	if ( ! key ) { return 0; }
	
	int result = 1;

    // deprecated properties have a trailing '#'
	static const char * keys[] = 
	{
		"setColor#",		// 0 - DEPRECATED
		"setStrokeColor",	// 1
		"setStroke",		// 2
		"append",			// 3
		"blendMode",		// 4
		"width#",			// 5 - DEPRECATED
		"strokeWidth",		// 6
		"stroke",			// 7
		"anchorSegments",	// 8
		"setStrokeVertexColor", // 9
		"strokeVertexCount",	// 10
	};
    const int numKeys = sizeof( keys ) / sizeof( const char * );
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 11, 2, 2, __FILE__, __LINE__ );
	StringHash *hash = &sHash;

	int index = hash->Lookup( key );
	switch ( index )
	{
	case 0:
#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
		if (! static_cast< const LineObject& >( object ).IsV1Compatibility())
        {
            CoronaLuaWarning(L, "line:setColor() is deprecated. Use line:setStrokeColor() instead");
        }
#endif
		// Fall through
	case 1:
		{
			Lua::PushCachedFunction( L, Self::setStrokeColor );
		}
		break;
	case 2:
		{
			Lua::PushCachedFunction( L, Self::setStroke );
		}
		break;
	case 3:
		{
			Lua::PushCachedFunction( L, Self::append );
		}
		break;
	case 4:
		{
			const LineObject& o = static_cast< const LineObject& >( object );
			RenderTypes::BlendType blend = o.GetBlend();
			lua_pushstring( L, RenderTypes::StringForBlendType( blend ) );
		}
        break;
	case 5:
#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
            if (! static_cast< const LineObject& >( object ).IsV1Compatibility())
            {
                CoronaLuaWarning(L, "line.width is deprecated. Use line.strokeWidth");
            }
#endif
		// fall through
	case 6:
		{
			const LineObject& o = static_cast< const LineObject& >( object );
			Rtt_WARN_SIM_PROXY_TYPE( L, 1, LineObject );
			lua_pushnumber( L, Rtt_RealToFloat( o.GetStrokeWidth() ) );
		}
        break;
	case 7:
		{
			const LineObject& o = static_cast< const LineObject& >( object );
			Rtt_WARN_SIM_PROXY_TYPE( L, 1, LineObject );
			const Paint *paint = o.GetPath().GetStroke();
			if ( paint )
			{
				paint->PushProxy( L );
			}
            else
            {
                lua_pushnil( L );
            }
		}
        break;
	case 8:
		{
			const LineObject& o = static_cast< const LineObject& >( object );
			Rtt_WARN_SIM_PROXY_TYPE( L, 1, LineObject );
			lua_pushboolean( L, o.ShouldOffsetWithAnchor() );
			result = 1;
		}
        break;
	case 9:
		{
			Lua::PushCachedFunction( L, Self::setStrokeVertexColor );
		}
		break;
	case 10:
		{
			const LineObject& o = static_cast< const LineObject& >( object );
			lua_pushinteger( L, o.GetPath().GetStrokeVertexCount() );
		}
		break;

	default:
		{
			result = Super::ValueForKey( L, object, key, overrideRestriction );
		}
		break;
	}

    // If we retrieved the "_properties" key from the super, merge it with the local properties
    if ( result == 1 && strcmp( key, "_properties" ) == 0 )
    {
        String lineProperties(LuaContext::GetRuntime( L )->Allocator());

        DumpObjectProperties( L, object, keys, numKeys, lineProperties );

        lua_pushfstring( L, "{ %s, %s }", lineProperties.GetString(), lua_tostring( L, -1 ) );
        lua_remove( L, -2 ); // pop super properties
        result = 1;
    }

    return result;
}

bool
LuaLineObjectProxyVTable::SetValueForKey( lua_State *L, MLuaProxyable& object, const char key[], int valueIndex ) const
{
	if ( ! key ) { return false; }

	bool result = true;

	// LineObject* o = (LineObject*)LuaProxy::GetProxyableObject( L, 1 );
	LineObject& o = static_cast< LineObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, LineObject );

	static const char * keys[] = 
	{
		"setColor",			// 0
		"setStrokeColor",	// 1
		"setStroke",		// 2
		"append",			// 3
		"blendMode",		// 4
		"width",			// 5
		"strokeWidth",		// 6
		"stroke",			// 7
		"anchorSegments",	// 8
		"strokeVertexCount",	// 9
		"setStrokeVertexColor",	// 10
	};
    const int numKeys = sizeof( keys ) / sizeof( const char * );
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 11, 2, 2, __FILE__, __LINE__ );
	StringHash *hash = &sHash;

	int index = hash->Lookup( key );
	switch ( index )
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 10:
		// No-op: cannot set property for method
		break;
	case 4:
		{
			const char *v = lua_tostring( L, valueIndex );
			RenderTypes::BlendType blend = RenderTypes::BlendTypeForString( v );
			if ( RenderTypes::IsRestrictedBlendType( blend ) )
			{
				if ( o.IsRestricted()
					 && o.GetStage()->GetDisplay().ShouldRestrict( Display::kLineBlendMode ) )
				{
					CoronaLuaWarning(L, "using 'normal' blend because '%s' is a premium feature",
						RenderTypes::StringForBlendType( blend ) );
					blend = RenderTypes::kNormal;
				}
			}
			o.SetBlend( blend );
		}
        break;
	case 5:
#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
            if (! o.IsV1Compatibility())
            {
                CoronaLuaWarning(L, "line.width is deprecated. Use line.strokeWidth");
            }
#endif
		// fall through
	case 6:
		{
			o.SetStrokeWidth( luaL_toreal( L, valueIndex ) );
		}
        break;
	case 7:
		{
			setStroke( L, valueIndex );
		}
       break;
	case 8:
		{
			setAnchorSegments( L, valueIndex );
		}
       break;
	case 9:
		// No-op: cannot set vertex count
		break;
	default:
		{
			result = Super::SetValueForKey( L, object, key, valueIndex );
		}
		break;
	}

	return result;
}

const LuaProxyVTable&
LuaLineObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

int
LuaLineObjectProxyVTable::setAnchorSegments( lua_State *L, int valueIndex )
{
	LineObject* o = (LineObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, LineObject );

	if( Rtt_VERIFY( o ) )
	{
		o->SetAnchorSegments( lua_toboolean( L, valueIndex ) );
	}
	return 0;
}

// ----------------------------------------------------------------------------

const LuaShapeObjectProxyVTable&
LuaShapeObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

int
LuaShapeObjectProxyVTable::setFillColor( lua_State *L )
{
	ShapeObject* o = (ShapeObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ShapeObject );

	if ( o )
	{
		if ( lua_istable( L, 2 ) )
		{
			GradientPaint *gradient = LuaLibDisplay::LuaNewGradientPaint( L, 2 );
			if ( gradient )
			{
				o->SetFill( gradient );

				// Early return
				return 0;
			}
		}

		if ( ! o->GetPath().GetFill() )
		{
			Paint* p = LuaLibDisplay::LuaNewColor( L, 2, o->IsByteColorRange() );
			o->SetFill( p );
		}
		else
		{
			Color c = LuaLibDisplay::toColor( L, 2, o->IsByteColorRange() );
			o->SetFillColor( c );
		}
	}

	return 0;
}

int
LuaShapeObjectProxyVTable::setStrokeColor( lua_State *L )
{
	ShapeObject* o = (ShapeObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ShapeObject );

	if ( o )
	{
		if ( ! o->GetPath().GetStroke() )
		{
			Paint* p = LuaLibDisplay::LuaNewColor( L, 2, o->IsByteColorRange() );
			o->SetStroke( p );
		}
		else
		{
			Color c = LuaLibDisplay::toColor( L, 2, o->IsByteColorRange() );
			o->SetStrokeColor( c );
		}
	}

	return 0;
}

// object.fill
int
LuaShapeObjectProxyVTable::setFill( lua_State *L, int valueIndex )
{
	ShapeObject* o = (ShapeObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ShapeObject );

	if ( Rtt_VERIFY( o ) )
	{
		if ( ! o->IsRestricted()
			 || ! o->GetStage()->GetDisplay().ShouldRestrict( Display::kObjectFill ) )
		{
			// Use factory method to create paint
			Paint *paint = LuaLibDisplay::LuaNewPaint( L, valueIndex );

			o->SetFill( paint );
		}
	}
	return 0;
}

// object.stroke
int
LuaShapeObjectProxyVTable::setStroke( lua_State *L, int valueIndex )
{
	ShapeObject* o = (ShapeObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ShapeObject );

	if ( Rtt_VERIFY( o ) )
	{
		if ( ! o->IsRestricted()
			 || ! o->GetStage()->GetDisplay().ShouldRestrict( Display::kObjectStroke ) )
		{
			// Use factory method to create paint
			Paint *paint = LuaLibDisplay::LuaNewPaint( L, valueIndex );

			o->SetStroke( paint );
		}
	}
	return 0;
}

int
LuaShapeObjectProxyVTable::setFillVertexColor( lua_State *L )
{
	ShapeObject* o = (ShapeObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ShapeObject );

	if ( o )
	{
		ShapePath& path = static_cast< ShapePath& >( o->GetPath() );
		if ( ! path.GetFill() )
		{
			o->SetFill( DefaultPaint( L, o->IsByteColorRange() ) );
		}

		U32 index = lua_tointeger( L, 2 ) - 1U;
		Color c = LuaLibDisplay::toColor( L, 3, o->IsByteColorRange() );

		if (path.SetFillVertexColor( index, c ))
		{
			path.GetObserver()->Invalidate( DisplayObject::kGeometryFlag | DisplayObject::kColorFlag );
		}
	}

	return 0;
}

int
LuaShapeObjectProxyVTable::setStrokeVertexColor( lua_State *L )
{
	ShapeObject* o = (ShapeObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ShapeObject );

	if ( o )
	{
		ShapePath& path = static_cast< ShapePath& >( o->GetPath() );
		if ( ! path.GetStroke() )
		{
			o->SetStroke( DefaultPaint( L, o->IsByteColorRange() ) );
		}

		U32 index = lua_tointeger( L, 2 ) - 1U;
		Color c = LuaLibDisplay::toColor( L, 3, o->IsByteColorRange() );

		if (path.SetStrokeVertexColor( index, c ))
		{
			path.GetObserver()->Invalidate( DisplayObject::kGeometryFlag | DisplayObject::kColorFlag );
		}
	}

	return 0;
}

int
LuaShapeObjectProxyVTable::ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction /* = false */ ) const
{
	if ( ! key ) { return 0; }
	
	int result = 1;

	static const char * keys[] =
	{
		"path",				// 0
		"fill",				// 1
		"stroke",			// 2
		"blendMode",		// 3
		"setFillColor",		// 4
		"setStrokeColor",	// 5
		"strokeWidth",		// 6
		"innerstrokeWidth",	// 7
		"setFillVertexColor",	// 8
		"fillVertexCount",		// 9
		"setStrokeVertexColor", // 10
		"strokeVertexCount",	// 11
	};
    const int numKeys = sizeof( keys ) / sizeof( const char * );
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 12, 11, 2, __FILE__, __LINE__ );
	StringHash *hash = &sHash;
	int index = hash->Lookup( key );

	// ShapeObject* o = (ShapeObject*)LuaProxy::GetProxyableObject( L, 1 );
	const ShapeObject& o = static_cast< const ShapeObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ShapeObject );

	switch ( index )
	{
	case 0:
		{
			if ( overrideRestriction
                 || ! o.IsRestricted()
				 || ! o.GetStage()->GetDisplay().ShouldRestrict( Display::kObjectPath ) )
			{
                o.GetPath().PushProxy( L );
			}
			else
			{
				lua_pushnil( L );
			}
		}
		break;
	case 1:
		{
            if ( overrideRestriction
                 || ! o.IsRestricted()
				 || ! o.GetStage()->GetDisplay().ShouldRestrict( Display::kObjectFill ) )
			{
				const Paint *paint = o.GetPath().GetFill();
				if ( paint )
				{
					paint->PushProxy( L );
				}
                else
                {
                    lua_pushnil( L );
                }
			}
			else
			{
				lua_pushnil( L );
			}
		}
		break;
	case 2:
		{
            if ( overrideRestriction
                 || ! o.IsRestricted()
				 || ! o.GetStage()->GetDisplay().ShouldRestrict( Display::kObjectStroke ) )
			{
				const Paint *paint = o.GetPath().GetStroke();
				if ( paint )
				{
					paint->PushProxy( L );
				}
                else
                {
                    lua_pushnil( L );
                }
			}
			else
			{
				lua_pushnil( L );
			}
		}
		break;
	case 3:
		{
			RenderTypes::BlendType blend = o.GetBlend();
			lua_pushstring( L, RenderTypes::StringForBlendType( blend ) );
		}
		break;
	case 4:
		{
			Lua::PushCachedFunction( L, Self::setFillColor );
		}
		break;
	case 5:
		{
			Lua::PushCachedFunction( L, Self::setStrokeColor );
		}
		break;
	case 6:
		{
			lua_pushinteger( L, o.GetStrokeWidth() );			
		}
		break;
	case 7:
		{
			lua_pushinteger( L, o.GetInnerStrokeWidth() );		
		}
		break;
	case 8:
		{
			Lua::PushCachedFunction( L, Self::setFillVertexColor );
		}
		break;
	case 9:
		{
			lua_pushinteger( L, static_cast< const ShapePath& >( o.GetPath() ).GetFillVertexCount() );
		}
		break;
	case 10:
		{
			Lua::PushCachedFunction( L, Self::setStrokeVertexColor );
		}
		break;
	case 11:
		{
			lua_pushinteger( L, static_cast< const ShapePath& >( o.GetPath() ).GetStrokeVertexCount() );
		}
		break;
	default:
		{
             result = Super::ValueForKey( L, object, key, overrideRestriction );
		}
		break;
	}

    // Because this is effectively a derived class, we will have successfully gotten a value
    // for the "_properties" key from the parent and we now need to combine that with the
    // properties of the child
    if (result == 1 && strcmp( key, "_properties" ) == 0 )
    {
        String properties(LuaContext::GetRuntime( L )->Allocator());
        const char *prefix = "";
        const char *postfix = "";

        DumpObjectProperties( L, object, keys, numKeys, properties );

        // Some objects are derived from "ShapeObjects" but some are just "ShapeObjects and
        // we need to emit complete JSON in those cases so we add the enclosing braces if
        // this is a "ShapeObject"
        if (strcmp(o.GetObjectDesc(), "ShapeObject") == 0 || strcmp(o.GetObjectDesc(), "ImageObject") == 0)
        {
            prefix = "{ ";
            postfix = " }";
        }

        // Combine this object's properties with those of the super that were pushed above
        lua_pushfstring( L, "%s%s, %s%s", prefix, properties.GetString(), lua_tostring( L, -1 ), postfix );

        lua_remove( L, -2 ); // pop super properties
        result = 1;
    }

	return result;
}

static void
AssignDefaultStrokeColor( lua_State *L, ShapeObject& o )
{
	if ( ! o.GetPath().GetStroke() )
	{
		const Runtime* runtime = LuaContext::GetRuntime( L );
		
		SharedPtr< TextureResource > resource = runtime->GetDisplay().GetTextureFactory().GetDefault();
		Paint *p = Paint::NewColor(
							runtime->Allocator(),
							resource, runtime->GetDisplay().GetDefaults().GetStrokeColor() );
		o.SetStroke( p );
	}
}

bool
LuaShapeObjectProxyVTable::SetValueForKey( lua_State *L, MLuaProxyable& object, const char key[], int valueIndex ) const
{
	if ( ! key ) { return false; }

	// ShapeObject* o = (ShapeObject*)LuaProxy::GetProxyableObject( L, 1 );
	ShapeObject& o = static_cast< ShapeObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ShapeObject );

	bool result = true;

	static const char * keys[] = 
	{
		"fill",				// 0
		"stroke",			// 1
		"blendMode",		// 2
		"strokeWidth",		// 3
		"innerStrokeWidth", // 4
	};
    const int numKeys = sizeof( keys ) / sizeof( const char * );
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 5, 0, 1, __FILE__, __LINE__ );
	StringHash *hash = &sHash;

	int index = hash->Lookup( key );
	switch ( index )
	{
	case 0:
		{
			setFill( L, valueIndex );
		}
		break;
	case 1:
		{
			setStroke( L, valueIndex );
		}
		break;
	case 2:
		{
			const char *v = lua_tostring( L, valueIndex );
			RenderTypes::BlendType blend = RenderTypes::BlendTypeForString( v );
			if ( RenderTypes::IsRestrictedBlendType( blend ) )
			{
				if ( o.IsRestricted()
					 && o.GetStage()->GetDisplay().ShouldRestrict( Display::kObjectBlendMode ) )
				{
					CoronaLuaWarning(L, "using 'normal' blend because '%s' is a premium feature",
						RenderTypes::StringForBlendType( blend ) );
					blend = RenderTypes::kNormal;
				}
			}
			o.SetBlend( blend );
		}
		break;
	case 3:
		{
			U8 width = lua_tointeger( L, valueIndex );

			U8 innerWidth = width >> 1;
			o.SetInnerStrokeWidth( innerWidth );

			U8 outerWidth = width - innerWidth;
			o.SetOuterStrokeWidth( outerWidth );

			AssignDefaultStrokeColor( L, o );
		}
		break;
	case 4:
		{
			o.SetInnerStrokeWidth( lua_tointeger( L, valueIndex ) );

			AssignDefaultStrokeColor( L, o );
		}
		break;
	default:
		{
			result = Super::SetValueForKey( L, object, key, valueIndex );
		}
		break;
	}

	return result;
}

const LuaProxyVTable&
LuaShapeObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

const LuaEmitterObjectProxyVTable&
LuaEmitterObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

int
LuaEmitterObjectProxyVTable::start( lua_State *L )
{
	EmitterObject* o = (EmitterObject*)LuaProxy::GetProxyableObject( L, 1 );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, EmitterObject );

	if ( o )
	{
#if 0
		if ( lua_istable( L, 2 ) )
		{
			GradientPaint *gradient = LuaLibDisplay::LuaNewGradientPaint( L, 2 );
			if ( gradient )
			{
				o->SetFill( gradient );

				// Early return
				return 0;
			}
		}
#else
		o->Start();
#endif
	}

	return 0;
}

int
LuaEmitterObjectProxyVTable::stop( lua_State *L )
{
	EmitterObject* o = (EmitterObject*)LuaProxy::GetProxyableObject( L, 1 );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, EmitterObject );

	if ( o )
	{
		o->Stop();
	}

	return 0;
}

int
LuaEmitterObjectProxyVTable::pause( lua_State *L )
{
	EmitterObject* o = (EmitterObject*)LuaProxy::GetProxyableObject( L, 1 );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, EmitterObject );

	if ( o )
	{
		o->Pause();
	}

	return 0;
}

// IMPORTANT: This list MUST be kept in sync with the "EmitterObject_keys".
enum
{
	// Read-write properties.
	kEmitterObject_AbsolutePosition,
	kEmitterObject_GravityX,
	kEmitterObject_GravityY,
	kEmitterObject_StartColorR,
	kEmitterObject_StartColorG,
	kEmitterObject_StartColorB,
	kEmitterObject_StartColorA,
	kEmitterObject_StartColorVarianceR,
	kEmitterObject_StartColorVarianceG,
	kEmitterObject_StartColorVarianceB,
	kEmitterObject_StartColorVarianceA,
	kEmitterObject_FinishColorR,
	kEmitterObject_FinishColorG,
	kEmitterObject_FinishColorB,
	kEmitterObject_FinishColorA,
	kEmitterObject_FinishColorVarianceR,
	kEmitterObject_FinishColorVarianceG,
	kEmitterObject_FinishColorVarianceB,
	kEmitterObject_FinishColorVarianceA,
	kEmitterObject_StartParticleSize,
	kEmitterObject_StartParticleSizeVariance,
	kEmitterObject_FinishParticleSize,
	kEmitterObject_FinishParticleSizeVariance,
	kEmitterObject_MaxRadius,
	kEmitterObject_MaxRadiusVariance,
	kEmitterObject_MinRadius,
	kEmitterObject_MinRadiusVariance,
	kEmitterObject_RotateDegreesPerSecond,
	kEmitterObject_RotateDegreesPerSecondVariance,
	kEmitterObject_RotationStart,
	kEmitterObject_RotationStartVariance,
	kEmitterObject_RotationEnd,
	kEmitterObject_RotationEndVariance,
	kEmitterObject_Speed,
	kEmitterObject_SpeedVariance,
	kEmitterObject_EmissionRateInParticlesPerSeconds,
	kEmitterObject_RadialAcceleration,
	kEmitterObject_RadialAccelerationVariance,
	kEmitterObject_TangentialAcceleration,
	kEmitterObject_TangentialAccelerationVariance,
	kEmitterObject_SourcePositionVarianceX,
	kEmitterObject_SourcePositionVarianceY,
	kEmitterObject_RotationInDegrees,
	kEmitterObject_RotationInDegreesVariance,
	kEmitterObject_ParticleLifespanInSeconds,
	kEmitterObject_ParticleLifespanInSecondsVariance,
	kEmitterObject_Duration,

	// Read Only Property
	kEmitterObject_MaxParticles,

	// Methods.
	kEmitterObject_Start,
	kEmitterObject_Stop,
	kEmitterObject_Pause,

	// Read-only properties.
    kEmitterObject_State,
};

static const char * EmitterObject_keys[] = 
{
	// Read-write properties.
	"absolutePosition",
	"gravityx",
	"gravityy",
	"startColorRed",
	"startColorGreen",
	"startColorBlue",
	"startColorAlpha",
	"startColorVarianceRed",
	"startColorVarianceGreen",
	"startColorVarianceBlue",
	"startColorVarianceAlpha",
	"finishColorRed",
	"finishColorGreen",
	"finishColorBlue",
	"finishColorAlpha",
	"finishColorVarianceRed",
	"finishColorVarianceGreen",
	"finishColorVarianceBlue",
	"finishColorVarianceAlpha",
	"startParticleSize",
	"startParticleSizeVariance",
	"finishParticleSize",
	"finishParticleSizeVariance",
	"maxRadius",
	"maxRadiusVariance",
	"minRadius",
	"minRadiusVariance",
	"rotatePerSecond",
	"rotatePerSecondVariance",
	"rotationStart",
	"rotationStartVariance",
	"rotationEnd",
	"rotationEndVariance",
	"speed",
	"speedVariance",
	"emissionRateInParticlesPerSeconds",
	"radialAcceleration",
	"radialAccelVariance",
	"tangentialAcceleration",
	"tangentialAccelVariance",
	"sourcePositionVariancex",
	"sourcePositionVariancey",
	"angle",
	"angleVariance",
	"particleLifespan",
	"particleLifespanVariance",
	"duration",

	// read only properties
	"maxParticles",

	// Methods.
	"start",
	"stop",
	"pause",

	// Read-only properties.
    "state",
};

static StringHash*
GetEmitterObjectHash( lua_State *L )
{
	static StringHash sHash( *LuaContext::GetAllocator( L ), EmitterObject_keys, sizeof( EmitterObject_keys ) / sizeof(const char *), 52, 12, 14, __FILE__, __LINE__ );
	return &sHash;
}

int
LuaEmitterObjectProxyVTable::ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction /* = false */ ) const
{
	if ( ! key ) { return 0; }
	
	int result = 1;

	StringHash *hash = GetEmitterObjectHash( L );
	int index = hash->Lookup( key );

	// EmitterObject* o = (EmitterObject*)LuaProxy::GetProxyableObject( L, 1 );
	const EmitterObject& o = static_cast< const EmitterObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, EmitterObject );

	switch ( index )
	{
	case kEmitterObject_AbsolutePosition:
		{
			GroupObject *p = o.GetAbsolutePosition();
			if(p == NULL)
			{
				lua_pushboolean( L, false );
			}
			else if(p == EMITTER_ABSOLUTE_PARENT)
			{
				lua_pushboolean( L, false );
			}
			else
			{
				p->GetProxy()->PushTable( L );
			}
		}
		break;
	case kEmitterObject_GravityX:
		{
			lua_pushnumber( L, o.GetGravity().x );
		}
		break;
	case kEmitterObject_GravityY:
		{
			lua_pushnumber( L, o.GetGravity().y );
		}
		break;
	case kEmitterObject_StartColorR:
		{
			lua_pushnumber( L, o.GetStartColor().r );
		}
		break;
	case kEmitterObject_StartColorG:
		{
			lua_pushnumber( L, o.GetStartColor().g );
		}
		break;
	case kEmitterObject_StartColorB:
		{
			lua_pushnumber( L, o.GetStartColor().b );
		}
		break;
	case kEmitterObject_StartColorA:
		{
			lua_pushnumber( L, o.GetStartColor().a );
		}
		break;
	case kEmitterObject_StartColorVarianceR:
		{
			lua_pushnumber( L, o.GetStartColorVariance().r );
		}
		break;
	case kEmitterObject_StartColorVarianceG:
		{
			lua_pushnumber( L, o.GetStartColorVariance().g );
		}
		break;
	case kEmitterObject_StartColorVarianceB:
		{
			lua_pushnumber( L, o.GetStartColorVariance().b );
		}
		break;
	case kEmitterObject_StartColorVarianceA:
		{
			lua_pushnumber( L, o.GetStartColorVariance().a );
		}
		break;
	case kEmitterObject_FinishColorR:
		{
			lua_pushnumber( L, o.GetFinishColor().r );
		}
		break;
	case kEmitterObject_FinishColorG:
		{
			lua_pushnumber( L, o.GetFinishColor().g );
		}
		break;
	case kEmitterObject_FinishColorB:
		{
			lua_pushnumber( L, o.GetFinishColor().b );
		}
		break;
	case kEmitterObject_FinishColorA:
		{
			lua_pushnumber( L, o.GetFinishColor().a );
		}
		break;
	case kEmitterObject_FinishColorVarianceR:
		{
			lua_pushnumber( L, o.GetFinishColorVariance().r );
		}
		break;
	case kEmitterObject_FinishColorVarianceG:
		{
			lua_pushnumber( L, o.GetFinishColorVariance().g );
		}
		break;
	case kEmitterObject_FinishColorVarianceB:
		{
			lua_pushnumber( L, o.GetFinishColorVariance().b );
		}
		break;
	case kEmitterObject_FinishColorVarianceA:
		{
			lua_pushnumber( L, o.GetFinishColorVariance().a );
		}
		break;
	case kEmitterObject_StartParticleSize:
		{
			lua_pushnumber( L, o.GetStartParticleSize() );
		}
		break;
	case kEmitterObject_StartParticleSizeVariance:
		{
			lua_pushnumber( L, o.GetStartParticleSizeVariance() );
		}
		break;
	case kEmitterObject_FinishParticleSize:
		{
			lua_pushnumber( L, o.GetFinishParticleSize() );
		}
		break;
	case kEmitterObject_FinishParticleSizeVariance:
		{
			lua_pushnumber( L, o.GetFinishParticleSizeVariance() );
		}
		break;
	case kEmitterObject_MaxRadius:
		{
			lua_pushnumber( L, o.GetMaxRadius() );
		}
		break;
	case kEmitterObject_MaxRadiusVariance:
		{
			lua_pushnumber( L, o.GetMaxRadiusVariance() );
		}
		break;
	case kEmitterObject_MinRadius:
		{
			lua_pushnumber( L, o.GetMinRadius() );
		}
		break;
	case kEmitterObject_MinRadiusVariance:
		{
			lua_pushnumber( L, o.GetMinRadiusVariance() );
		}
		break;
	case kEmitterObject_RotateDegreesPerSecond:
		{
			lua_pushnumber( L, o.GetRotateDegreesPerSecond() );
		}
		break;
	case kEmitterObject_RotateDegreesPerSecondVariance:
		{
			lua_pushnumber( L, o.GetRotateDegreesPerSecondVariance() );
		}
		break;
	case kEmitterObject_RotationStart:
		{
			lua_pushnumber( L, o.GetRotationStart() );
		}
		break;
	case kEmitterObject_RotationStartVariance:
		{
			lua_pushnumber( L, o.GetRotationStartVariance() );
		}
		break;
	case kEmitterObject_RotationEnd:
		{
			lua_pushnumber( L, o.GetRotationEnd() );
		}
		break;
	case kEmitterObject_RotationEndVariance:
		{
			lua_pushnumber( L, o.GetRotationEndVariance() );
		}
		break;
	case kEmitterObject_Speed:
		{
			lua_pushnumber( L, o.GetSpeed() );
		}
		break;
	case kEmitterObject_SpeedVariance:
		{
			lua_pushnumber( L, o.GetSpeedVariance() );
		}
		break;
	case kEmitterObject_EmissionRateInParticlesPerSeconds:
		{
			lua_pushnumber( L, o.GetEmissionRateInParticlesPerSeconds() );
		}
		break;
	case kEmitterObject_RadialAcceleration:
		{
			lua_pushnumber( L, o.GetRadialAcceleration() );
		}
		break;
	case kEmitterObject_RadialAccelerationVariance:
		{
			lua_pushnumber( L, o.GetRadialAccelerationVariance() );
		}
		break;
	case kEmitterObject_TangentialAcceleration:
		{
			lua_pushnumber( L, o.GetTangentialAcceleration() );
		}
		break;
	case kEmitterObject_TangentialAccelerationVariance:
		{
			lua_pushnumber( L, o.GetTangentialAccelerationVariance() );
		}
		break;
	case kEmitterObject_SourcePositionVarianceX:
		{
			lua_pushnumber( L, o.GetSourcePositionVariance().x );
		}
		break;
	case kEmitterObject_SourcePositionVarianceY:
		{
			lua_pushnumber( L, o.GetSourcePositionVariance().y );
		}
		break;
	case kEmitterObject_RotationInDegrees:
		{
			lua_pushnumber( L, o.GetRotationInDegrees() );
		}
		break;
	case kEmitterObject_RotationInDegreesVariance:
		{
			lua_pushnumber( L, o.GetRotationInDegreesVariance() );
		}
		break;
	case kEmitterObject_ParticleLifespanInSeconds:
		{
			lua_pushnumber( L, o.GetParticleLifespanInSeconds() );
		}
		break;
	case kEmitterObject_ParticleLifespanInSecondsVariance:
		{
			lua_pushnumber( L, o.GetParticleLifespanInSecondsVariance() );
		}
		break;
	case kEmitterObject_Duration:
		{
			lua_pushnumber( L, o.GetDuration() );
		}
		break;
	case kEmitterObject_MaxParticles:
		{
			lua_pushinteger( L, o.GetMaxParticles() );
		}
		break;
	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	case kEmitterObject_Start:
		{
			Lua::PushCachedFunction( L, Self::start );
		}
		break;
	case kEmitterObject_Stop:
		{
			Lua::PushCachedFunction( L, Self::stop );
		}
		break;
	case kEmitterObject_Pause:
		{
			Lua::PushCachedFunction( L, Self::pause );
		}
		break;

	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	case kEmitterObject_State:
		{
			lua_pushstring( L, EmitterObject::GetStringForState( o.GetState() ) );
		}
		break;

	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

    default:
		{
            result = Super::ValueForKey( L, object, key, overrideRestriction );
		}
		break;
	}

    if ( result == 1 && strcmp( key, "_properties" ) == 0 )
    {
        String emitterProperties(LuaContext::GetRuntime( L )->Allocator());
        const char **keys = NULL;
        const int numKeys = hash->GetKeys(keys);

        DumpObjectProperties( L, object, keys, numKeys, emitterProperties );

        lua_pushfstring( L, "{ %s, %s }", emitterProperties.GetString(), lua_tostring( L, -1 ) );
        lua_remove( L, -2 ); // pop super properties
        result = 1;
    }

	return result;
}

bool
LuaEmitterObjectProxyVTable::SetValueForKey( lua_State *L, MLuaProxyable& object, const char key[], int valueIndex ) const
{
	if ( ! key ) { return false; }

	// EmitterObject* o = (EmitterObject*)LuaProxy::GetProxyableObject( L, 1 );
	EmitterObject& o = static_cast< EmitterObject& >( object );
    //const int numKeys = sizeof( EmitterObject_keys ) / sizeof( const char * );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, EmitterObject );

	bool result = true;

	StringHash *hash = GetEmitterObjectHash( L );
	int index = hash->Lookup( key );

	switch ( index )
	{
	case kEmitterObject_AbsolutePosition:
		{
			GroupObject *parentGroup = NULL;
			if(lua_istable(L, valueIndex))
			{
				DisplayObject* parent = (DisplayObject*)LuaProxy::GetProxyableObject( L, valueIndex );
				if(parent)
				{
					parentGroup = parent->AsGroupObject();
				}
			}

			if(parentGroup == NULL)
			{
				o.SetAbsolutePosition( lua_toboolean( L, valueIndex )?EMITTER_ABSOLUTE_PARENT:NULL );
			}
			else
			{
				o.SetAbsolutePosition(parentGroup);
				if(!o.ValidateEmitterParent())
				{
					CoronaLuaWarning(L, "if '%s' of Emitter Object is set to group object, it has to be one of it's parents", key);
				}
			}
		}
		break;
	case kEmitterObject_GravityX:
		{
			o.GetGravity().x = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_GravityY:
		{
			o.GetGravity().y = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_StartColorR:
		{
			o.GetStartColor().r = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_StartColorG:
		{
			o.GetStartColor().g = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_StartColorB:
		{
			o.GetStartColor().b = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_StartColorA:
		{
			o.GetStartColor().a = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_StartColorVarianceR:
		{
			o.GetStartColorVariance().r = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_StartColorVarianceG:
		{
			o.GetStartColorVariance().g = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_StartColorVarianceB:
		{
			o.GetStartColorVariance().b = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_StartColorVarianceA:
		{
			o.GetStartColorVariance().a = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_FinishColorR:
		{
			o.GetFinishColor().r = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_FinishColorG:
		{
			o.GetFinishColor().g = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_FinishColorB:
		{
			o.GetFinishColor().b = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_FinishColorA:
		{
			o.GetFinishColor().a = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_FinishColorVarianceR:
		{
			o.GetFinishColorVariance().r = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_FinishColorVarianceG:
		{
			o.GetFinishColorVariance().g = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_FinishColorVarianceB:
		{
			o.GetFinishColorVariance().b = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_FinishColorVarianceA:
		{
			o.GetFinishColorVariance().a = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_StartParticleSize:
		{
			o.SetStartParticleSize( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_StartParticleSizeVariance:
		{
			o.SetStartParticleSizeVariance( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_FinishParticleSize:
		{
			o.SetFinishParticleSize( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_FinishParticleSizeVariance:
		{
			o.SetFinishParticleSizeVariance( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_MaxRadius:
		{
			o.SetMaxRadius( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_MaxRadiusVariance:
		{
			o.SetMaxRadiusVariance( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_MinRadius:
		{
			o.SetMinRadius( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_MinRadiusVariance:
		{
			o.SetMinRadiusVariance( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_RotateDegreesPerSecond:
		{
			o.SetRotateDegreesPerSecond( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_RotateDegreesPerSecondVariance:
		{
			o.SetRotateDegreesPerSecondVariance( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_RotationStart:
		{
			o.SetRotationStart( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_RotationStartVariance:
		{
			o.SetRotationStartVariance( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_RotationEnd:
		{
			o.SetRotationEnd( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_RotationEndVariance:
		{
			o.SetRotationEndVariance( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_Speed:
		{
			o.SetSpeed( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_SpeedVariance:
		{
			o.SetSpeedVariance( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_EmissionRateInParticlesPerSeconds:
		{
			o.SetEmissionRateInParticlesPerSeconds( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_RadialAcceleration:
		{
			o.SetRadialAcceleration( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_RadialAccelerationVariance:
		{
			o.SetRadialAccelerationVariance( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_TangentialAcceleration:
		{
			o.SetTangentialAcceleration( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_TangentialAccelerationVariance:
		{
			o.SetTangentialAccelerationVariance( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_SourcePositionVarianceX:
		{
			o.GetSourcePositionVariance().x = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_SourcePositionVarianceY:
		{
			o.GetSourcePositionVariance().y = luaL_toreal( L, valueIndex );
		}
		break;
	case kEmitterObject_RotationInDegrees:
		{
			o.SetRotationInDegrees( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_RotationInDegreesVariance:
		{
			o.SetRotationInDegreesVariance( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_ParticleLifespanInSeconds:
		{
			o.SetParticleLifespanInSeconds( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_ParticleLifespanInSecondsVariance:
		{
			o.SetParticleLifespanInSecondsVariance( luaL_toreal( L, valueIndex ) );
		}
		break;
	case kEmitterObject_Duration:
		{
			o.SetDuration( luaL_toreal( L, valueIndex ) );
		}
		break;

	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
	case kEmitterObject_MaxParticles:
	case kEmitterObject_Start:
	case kEmitterObject_Stop:
	case kEmitterObject_Pause:
	case kEmitterObject_State:
		{
			// Read-only properties
			// no-op
		}
		break;

	default:
		{
			result = Super::SetValueForKey( L, object, key, valueIndex );
		}
		break;
	}

	return result;
}

const LuaProxyVTable&
LuaEmitterObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

const LuaParticleSystemObjectProxyVTable&
LuaParticleSystemObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

int
LuaParticleSystemObjectProxyVTable::CreateGroup( lua_State *L )
{
	ParticleSystemObject* o = (ParticleSystemObject*)LuaProxy::GetProxyableObject( L, 1 );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ParticleSystemObject );

	if ( o )
	{
		o->CreateGroup( L );
	}

	return 0;
}

int
LuaParticleSystemObjectProxyVTable::CreateParticle( lua_State *L )
{
	ParticleSystemObject* o = (ParticleSystemObject*)LuaProxy::GetProxyableObject( L, 1 );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ParticleSystemObject );

	if ( o )
	{
		o->CreateParticle( L );
	}

	return 0;
}

int
LuaParticleSystemObjectProxyVTable::DestroyParticlesInShape( lua_State *L )
{
	ParticleSystemObject* o = (ParticleSystemObject*)LuaProxy::GetProxyableObject( L, 1 );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ParticleSystemObject );

	if ( o )
	{
		return o->DestroyParticlesInShape( L );
	}

	return 0;
}

int
LuaParticleSystemObjectProxyVTable::RayCast( lua_State *L )
{
	ParticleSystemObject* o = (ParticleSystemObject*)LuaProxy::GetProxyableObject( L, 1 );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ParticleSystemObject );

	if ( o )
	{
		return o->RayCast( L );
	}

	return 0;
}

int
LuaParticleSystemObjectProxyVTable::QueryRegion( lua_State *L )
{
	ParticleSystemObject* o = (ParticleSystemObject*)LuaProxy::GetProxyableObject( L, 1 );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ParticleSystemObject );

	if ( o )
	{
		return o->QueryRegion( L );
	}

	return 0;
}

int
LuaParticleSystemObjectProxyVTable::ApplyForce( lua_State *L )
{
	ParticleSystemObject* o = (ParticleSystemObject*)LuaProxy::GetProxyableObject( L, 1 );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ParticleSystemObject );

	if ( o )
	{
		o->ApplyForce( L );
	}

	return 0;
}

int
LuaParticleSystemObjectProxyVTable::ApplyLinearImpulse( lua_State *L )
{
	ParticleSystemObject* o = (ParticleSystemObject*)LuaProxy::GetProxyableObject( L, 1 );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ParticleSystemObject );

	if ( o )
	{
		o->ApplyLinearImpulse( L );
	}

	return 0;
}

// IMPORTANT: This list MUST be kept in sync with the "ParticleSystemObject_keys".
enum
{
	// Read-write properties.
	kParticleSystemObject_particleDensity,
	kParticleSystemObject_particleRadius,
	kParticleSystemObject_particleDamping,
	kParticleSystemObject_particleStrictContactCheck,
	kParticleSystemObject_particleMaxCount,
	kParticleSystemObject_particleGravityScale,
	kParticleSystemObject_particleDestructionByAge,
	kParticleSystemObject_particlePaused,
	kParticleSystemObject_imageRadius,

	// Read-only property.
	kParticleSystemObject_particleMass,
	kParticleSystemObject_particleCount,

	// Methods.
	kParticleSystemObject_ApplyForce,
	kParticleSystemObject_ApplyLinearImpulse,
	kParticleSystemObject_CreateGroup,
	kParticleSystemObject_CreateParticle,
	kParticleSystemObject_DestroyParticlesInShape,
	kParticleSystemObject_QueryRegion,
	kParticleSystemObject_RayCast,
};

static const char * ParticleSystemObject_keys[] = 
{
	// Read-write properties.
	"particleDensity",
	"particleRadius",
	"particleDamping",
	"particleStrictContactCheck",
	"particleMaxCount",
	"particleGravityScale",
	"particleDestructionByAge",
	"particlePaused",
	"imageRadius",

	// Read-only property.
	"particleMass",
	"particleCount",

	// Methods.
	"applyForce",
	"applyLinearImpulse",
	"createGroup",
	"createParticle",
	"destroyParticles",
	"queryRegion",
	"rayCast",
};

static StringHash*
GetParticleSystemObjectHash( lua_State *L )
{
	static StringHash sHash( *LuaContext::GetAllocator( L ), ParticleSystemObject_keys, sizeof( ParticleSystemObject_keys ) / sizeof(const char *), 19, 28, 2, __FILE__, __LINE__ );
	return &sHash;
}

int
LuaParticleSystemObjectProxyVTable::ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction /* = false */ ) const
{
	if ( ! key ) { return 0; }
	
	int result = 1;
	StringHash *hash = GetParticleSystemObjectHash( L );
	int index = hash->Lookup( key );

	// ParticleSystemObject* o = (ParticleSystemObject*)LuaProxy::GetProxyableObject( L, 1 );
	const ParticleSystemObject& o = static_cast< const ParticleSystemObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ParticleSystemObject );
	const b2ParticleSystem *b2ps = o.GetB2ParticleSystem();

	switch ( index )
	{
	case kParticleSystemObject_particleDensity:
		{
			lua_pushnumber( L, b2ps->GetDensity() );
		}
		break;

	case kParticleSystemObject_particleRadius:
		{
			PhysicsWorld &physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();

			float world_scale_in_pixels_per_meter = physics.GetPixelsPerMeter();

			lua_pushnumber( L, ( b2ps->GetRadius() * world_scale_in_pixels_per_meter ) );
		}
		break;

	case kParticleSystemObject_particleDamping:
		{
			lua_pushnumber( L, b2ps->GetDamping() );
		}
		break;

	case kParticleSystemObject_particleStrictContactCheck:
		{
			lua_pushboolean( L, b2ps->GetStrictContactCheck() );
		}
		break;

	case kParticleSystemObject_particleMaxCount:
		{
			lua_pushnumber( L, b2ps->GetMaxParticleCount() );
		}
		break;

	case kParticleSystemObject_particleGravityScale:
		{
			lua_pushnumber( L, b2ps->GetGravityScale() );
		}
		break;

	case kParticleSystemObject_particleDestructionByAge:
		{
			lua_pushboolean( L, b2ps->GetDestructionByAge() );
		}
		break;

	case kParticleSystemObject_particlePaused:
		{
			lua_pushboolean( L, b2ps->GetPaused() );
		}
		break;

	case kParticleSystemObject_imageRadius:
		{
			lua_pushnumber( L, o.GetParticleRenderRadiusInContentUnits() );
		}
		break;

	case kParticleSystemObject_particleMass:
		{
			lua_pushnumber( L, b2ps->GetParticleMass() );
		}
		break;

	case kParticleSystemObject_particleCount:
		{
			lua_pushnumber( L, b2ps->GetParticleCount() );
		}
		break;

	case kParticleSystemObject_ApplyForce:
		{
			Lua::PushCachedFunction( L, Self::ApplyForce );
		}
		break;

	case kParticleSystemObject_ApplyLinearImpulse:
		{
			Lua::PushCachedFunction( L, Self::ApplyLinearImpulse );
		}
		break;

	case kParticleSystemObject_CreateGroup:
		{
			Lua::PushCachedFunction( L, Self::CreateGroup );
		}
		break;

	case kParticleSystemObject_CreateParticle:
		{
			Lua::PushCachedFunction( L, Self::CreateParticle );
		}
		break;

	case kParticleSystemObject_DestroyParticlesInShape:
		{
			Lua::PushCachedFunction( L, Self::DestroyParticlesInShape );
		}
		break;

	case kParticleSystemObject_QueryRegion:
		{
			Lua::PushCachedFunction( L, Self::QueryRegion );
		}
		break;

	case kParticleSystemObject_RayCast:
		{
			Lua::PushCachedFunction( L, Self::RayCast );
		}
		break;

	default:
		{
            result = Super::ValueForKey( L, object, key, overrideRestriction );
		}
		break;
	}

    // Because this is effectively a derived class, we will have successfully gotten a value
    // for the "_properties" key from the parent and we now need to combine that with the
    // properties of the child
    if ( result == 1 && strcmp( key, "_properties" ) == 0 )
    {
        String psProperties(LuaContext::GetRuntime( L )->Allocator());
        const char **keys = NULL;
        const int numKeys = hash->GetKeys(keys);

        DumpObjectProperties( L, object, keys, numKeys, psProperties );

        lua_pushfstring( L, "{ %s, %s }", psProperties.GetString(), lua_tostring( L, -1 ) );
        lua_remove( L, -2 ); // pop super properties
        result = 1;
    }

	return result;
}

bool
LuaParticleSystemObjectProxyVTable::SetValueForKey( lua_State *L, MLuaProxyable& object, const char key[], int valueIndex ) const
{
	if ( ! key ) { return false; }

	// ParticleSystemObject* o = (ParticleSystemObject*)LuaProxy::GetProxyableObject( L, 1 );
	ParticleSystemObject& o = static_cast< ParticleSystemObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, ParticleSystemObject );
	b2ParticleSystem *b2ps = o.GetB2ParticleSystem();

	bool result = true;

	StringHash *hash = GetParticleSystemObjectHash( L );
	int index = hash->Lookup( key );

	switch ( index )
	{
	case kParticleSystemObject_particleDensity:
		{
			b2ps->SetDensity( luaL_toreal( L, valueIndex ) );
		}
		break;

	case kParticleSystemObject_particleRadius:
		{
			PhysicsWorld &physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();

			float world_scale_in_meters_per_pixel = physics.GetMetersPerPixel();

			b2ps->SetRadius( luaL_toreal( L, valueIndex ) * world_scale_in_meters_per_pixel );
		}
		break;

	case kParticleSystemObject_particleDamping:
		{
			b2ps->SetDamping( luaL_toreal( L, valueIndex ) );
		}
		break;

	case kParticleSystemObject_particleStrictContactCheck:
		{
			b2ps->SetStrictContactCheck( !! lua_toboolean( L, valueIndex ) );
		}
		break;

	case kParticleSystemObject_particleMaxCount:
		{
			b2ps->SetMaxParticleCount( luaL_toreal( L, valueIndex ) );
		}
		break;

	case kParticleSystemObject_particleGravityScale:
		{
			b2ps->SetGravityScale( luaL_toreal( L, valueIndex ) );
		}
		break;

	case kParticleSystemObject_particleDestructionByAge:
		{
			b2ps->SetDestructionByAge( !! lua_toboolean( L, valueIndex ) );
		}
		break;

	case kParticleSystemObject_particlePaused:
		{
			b2ps->SetPaused( !! lua_toboolean( L, valueIndex ) );
		}
		break;

	case kParticleSystemObject_imageRadius:
		{
			o.SetParticleRenderRadiusInContentUnits( luaL_toreal( L, valueIndex ) );
		}
		break;

	case kParticleSystemObject_particleMass:
	case kParticleSystemObject_particleCount:
	case kParticleSystemObject_ApplyForce:
	case kParticleSystemObject_ApplyLinearImpulse:
	case kParticleSystemObject_CreateGroup:
	case kParticleSystemObject_CreateParticle:
	case kParticleSystemObject_DestroyParticlesInShape:
	case kParticleSystemObject_QueryRegion:
	case kParticleSystemObject_RayCast:
		{
			// Read-only properties
			// no-op
		}
		break;

	default:
		{
			result = Super::SetValueForKey( L, object, key, valueIndex );
		}
		break;
	}

	return result;
}

const LuaProxyVTable&
LuaParticleSystemObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

const LuaSnapshotObjectProxyVTable&
LuaSnapshotObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

int
LuaSnapshotObjectProxyVTable::Invalidate( lua_State *L )
{
	SnapshotObject* o = (SnapshotObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, SnapshotObject );

	if ( o )
	{
		const char *value = lua_tostring( L, 2 );
		SnapshotObject::RenderFlag flag = SnapshotObject::RenderFlagForString( value );
		o->SetDirty( flag );
	}

	return 0;
}

static StringHash *
GetSnapshotHash( lua_State *L )
{
	static const char *keys[] = 
	{
		"group",			// 0 (read-only)
		"invalidate",		// 1 (read-only)
		"textureFilter",	// 2
		"textureWrapX",		// 3
		"textureWrapY",		// 4
		"clearColor",		// 5
		"canvas",			// 6 (read-only)
        "canvasMode",		// 7
	};
    const int numKeys = sizeof( keys ) / sizeof( const char * );
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 8, 6, 1, __FILE__, __LINE__ );
	return &sHash;
}

int
LuaSnapshotObjectProxyVTable::ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction /* = false */ ) const
{
	if ( ! key )
    {
        return 0;
    }
	
	int result = 1;

	StringHash *sHash = GetSnapshotHash( L );

	const SnapshotObject& o = static_cast< const SnapshotObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, SnapshotObject );

	int index = sHash->Lookup( key );

	switch ( index )
	{
	case 0:
		{
			o.GetGroup().GetProxy()->PushTable( L );
            result = 1;
		}
		break;
	case 1:
		{
			Lua::PushCachedFunction( L, Self::Invalidate );
		}
		break;
	case 2:
		{
			const char *str = RenderTypes::StringForTextureFilter( o.GetTextureFilter() );
			lua_pushstring( L, str );
 		}
		break;
	case 3:
		{
			const char *str = RenderTypes::StringForTextureWrap( o.GetTextureWrapX() );
			lua_pushstring( L, str );
		}
		break;
	case 4:
		{
			const char *str = RenderTypes::StringForTextureWrap( o.GetTextureWrapY() );
			lua_pushstring( L, str );
		}
		break;
	case 5:
		{
			result = LuaLibDisplay::PushColorChannels( L, o.GetClearColor(), false );
		}
		break;
	case 6:
		{
			o.GetCanvas().GetProxy()->PushTable( L );
		}
		break;
	case 7:
		{
			const char *str = SnapshotObject::StringForCanvasMode( o.GetCanvasMode() );
			lua_pushstring( L, str );
		}
		break;
	default:
		{
			result = Super::ValueForKey( L, object, key, overrideRestriction );
		}
		break;
	}

    // Because this is effectively a derived class, we will have successfully gotten a value
    // for the "_properties" key from the parent and we now need to combine that with the
    // properties of the child
    if ( result == 1 && strcmp( key, "_properties" ) == 0 )
    {
        String snapshotProperties(LuaContext::GetRuntime( L )->Allocator());
        const char **keys = NULL;
        const int numKeys = sHash->GetKeys(keys);

        DumpObjectProperties( L, object, keys, numKeys, snapshotProperties );

        lua_pushfstring( L, "{ %s, %s }", snapshotProperties.GetString(), lua_tostring( L, -1 ) );
        lua_remove( L, -2 ); // pop super properties
        result = 1;
    }

    return result;
}

bool
LuaSnapshotObjectProxyVTable::SetValueForKey( lua_State *L, MLuaProxyable& object, const char key[], int valueIndex ) const
{
	if ( ! key ) { return false; }

	bool result = true;

	StringHash *sHash = GetSnapshotHash( L );

	SnapshotObject& o = static_cast< SnapshotObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, SnapshotObject );

	int index = sHash->Lookup( key );

	switch ( index )
	{
	case 0:
	case 1:
	case 6:
		{
			// No-op for read-only property
			CoronaLuaWarning(L, "the '%s' property of snapshot objects is read-only", key);
		}
		break;
	case 2:
		{
			const char *str = lua_tostring( L, valueIndex );
			o.SetTextureFilter( RenderTypes::TextureFilterForString( str ) );
		}
		break;
	case 3:
		{
			const char *str = lua_tostring( L, valueIndex );
			o.SetTextureWrapX( RenderTypes::TextureWrapForString( str ) );
		}
		break;
	case 4:
		{
			const char *str = lua_tostring( L, valueIndex );
			o.SetTextureWrapY( RenderTypes::TextureWrapForString( str ) );
		}
		break;
	case 5:
		{
			Color c = ColorZero();
			LuaLibDisplay::ArrayToColor( L, valueIndex, c, false );
			o.SetClearColor( c );
		}
		break;
	case 7:
		{
			const char *str = lua_tostring( L, valueIndex );
			o.SetCanvasMode( SnapshotObject::CanvasModeForString( str ) );
		}
		break;
	default:
		{
			result = Super::SetValueForKey( L, object, key, valueIndex );
		}
		break;
	}

	return result;
}

const LuaProxyVTable&
LuaSnapshotObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

const LuaCompositeObjectProxyVTable&
LuaCompositeObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

int
LuaCompositeObjectProxyVTable::ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction /* = false */ ) const
{
	return 0;
}

const LuaProxyVTable&
LuaCompositeObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

const LuaGroupObjectProxyVTable&
LuaGroupObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

int
LuaGroupObjectProxyVTable::Insert( lua_State *L, GroupObject *parent )
{
	int index = (int) lua_tointeger( L, 2 );

	ENABLE_SUMMED_TIMING( true );

	int childIndex = 3; // index of child object (table owned by proxy)
	if ( 0 == index )
	{
		// Optional index arg missing, so insert at end
		--childIndex;
		index = parent->NumChildren();
	}
	else
	{
		// Map Lua indices to C++ indices
		--index;
	}
	Rtt_ASSERT( index >= 0 );
	Rtt_ASSERT( lua_istable( L, childIndex ) );

	// Default to false if no arg specified
	bool resetTransform = lua_toboolean( L, childIndex + 1 ) != 0;

	DisplayObject* child = (DisplayObject*)LuaProxy::GetProxyableObject( L, childIndex );
	if ( child != parent )
	{
		if ( ! child->IsRenderedOffScreen() )
		{
			GroupObject* oldParent = child->GetParent();

			// Display an error if they're indexing beyond the array (bug http://bugs.coronalabs.com/?18838 )
			const S32 maxIndex = parent->NumChildren();
			if ( index > maxIndex || index < 0 )
			{
				CoronaLuaWarning(L, "group index %d out of range (should be 1 to %d)", (index+1), maxIndex );
			}

			SUMMED_TIMING( pi, "Group: Insert (into parent)" );
			
			parent->Insert( index, child, resetTransform );

			SUMMED_TIMING( ai, "Group: Insert (post-parent insert)" );

			// Detect re-insertion of a child back onto the display --- when a
			// child is placed into a new parent that has a canvas and the oldParent 
			// was the Orphanage(), then re-acquire a lua ref for the proxy
			if ( oldParent != parent )
			{
				StageObject* canvas = parent->GetStage();
				if ( canvas && oldParent == canvas->GetDisplay().Orphanage() )
				{
					lua_pushvalue( L, childIndex ); // push table representing child
					child->GetProxy()->AcquireTableRef( L ); // reacquire a ref for table
					lua_pop( L, 1 );

					child->WillMoveOnscreen();
				}
			}
		}
		else
		{
			CoronaLuaWarning( L, "Insertion failed: display objects that are owned by offscreen resources cannot be inserted into groups" );
		}
	}
	else
	{
		luaL_error( L, "ERROR: attempt to insert display object into itself" );
	}

	ENABLE_SUMMED_TIMING( false );

	return 0;
}

int
LuaGroupObjectProxyVTable::insert( lua_State *L )
{
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, GroupObject );
	GroupObject *parent = (GroupObject*)LuaProxy::GetProxyableObject( L, 1 );
	return Insert( L, parent );
}

// Removes child at index from parent and pushes onto the stack. Pushes nil
// if index is invalid.
void
LuaDisplayObjectProxyVTable::PushAndRemove( lua_State *L, GroupObject* parent, S32 index )
{
	if ( index >= 0 )
	{
		Rtt_ASSERT( parent );

		// Offscreen objects (i.e. ones in the orphanage) do have a stage
		StageObject *stage = parent->GetStage();
		if ( stage )
		{
			Rtt_ASSERT( LuaContext::GetRuntime( L )->GetDisplay().HitTestOrphanage() != parent
						&& LuaContext::GetRuntime( L )->GetDisplay().Orphanage() != parent );

			SUMMED_TIMING( par1, "Object: PushAndRemove (release)" );

			DisplayObject* child = parent->Release( index );

			if (child != NULL)
			{
				SUMMED_TIMING( par2, "Object: PushAndRemove (rest)" );

				// If child is the same as global focus, clear global focus
				DisplayObject *globalFocus = stage->GetFocus();
				if ( globalFocus == child )
				{
					stage->SetFocus( NULL );
				}

				// Always the per-object focus
				stage->SetFocus( child, NULL );
				child->SetFocusId( NULL ); // Defer removal from the focus object array

				child->RemovedFromParent( L, parent );

				// We need to return table, so push it on stack
				Rtt_ASSERT( child->IsReachable() );
				LuaProxy* proxy = child->GetProxy();
				proxy->PushTable( L );

				// Rtt_TRACE( ( "release table ref(%x)\n", lua_topointer( L, -1 ) ) );

				// Anytime we add to the Orphanage, it means the DisplayObject is no
				// longer on the display. Therefore, we should luaL_unref the
				// DisplayObject's table. If it's later re-inserted, then we simply
				// luaL_ref the incoming table.
				Display& display = LuaContext::GetRuntime( L )->GetDisplay();


				// NOTE: Snapshot renamed to HitTest orphanage to clarify usage
				// TODO: Remove snapshot orphanage --- or verify that we still need it?
				// Note on the snapshot orphanage. We use this list to determine
				// which proxy table refs need to be released the table ref once
				// we're done with the snapshot. If the object is reinserted in
				// LuaGroupObjectProxyVTable::Insert(), then it is implicitly
				// removed from the snapshot orphanage --- thus, in that method,
				// nothing special needs to be done, b/c the proxy table wasn't
				// released yet.
				GroupObject& offscreenGroup =
				* ( child->IsUsedByHitTest() ? display.HitTestOrphanage() : display.Orphanage() );
				offscreenGroup.Insert( -1, child, false );

#ifdef Rtt_PHYSICS
				child->RemoveExtensions();
#endif
				
				child->DidMoveOffscreen();
			}
		}
		else
		{
			luaL_error( L, "ERROR: attempt to remove an object that's already been removed from the stage or whose parent/ancestor group has already been removed" );

			// Rtt_ASSERT( LuaContext::GetRuntime( L )->GetDisplay().HitTestOrphanage() == parent
			// 			|| LuaContext::GetRuntime( L )->GetDisplay().Orphanage() == parent );
		}
	}
	else
	{
		lua_pushnil( L );
	}
}

int
LuaGroupObjectProxyVTable::Remove( lua_State *L, GroupObject *parent )
{
	Rtt_ASSERT( ! lua_isnil( L, 1 ) );

	S32 index = -1;
	if ( lua_istable( L, 2 ) )
	{
		DisplayObject* child = (DisplayObject*)LuaProxy::GetProxyableObject( L, 2 );
		if ( child )
		{
			index = parent->Find( * child );

#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
			if (index < 0)
			{
				CoronaLuaWarning(L, "objectGroup:remove(): invalid object reference (most likely object is not in group)");
			}
#endif
		}
	}
	else
	{
		// Lua indices start at 1
		index = (int) lua_tointeger( L, 2 ) - 1;

#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
		if (index < 0 || index > parent->NumChildren())
		{
			CoronaLuaWarning(L, "objectGroup:remove(): index of %ld out of range (should be 1 to %d)", lua_tointeger( L, 2 ), parent->NumChildren());
		}
#endif
	}

	PushAndRemove( L, parent, index );

	return 1;
}

// group:remove( indexOrChild )
int
LuaGroupObjectProxyVTable::Remove( lua_State *L )
{
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, GroupObject );
	GroupObject *parent = (GroupObject*)LuaProxy::GetProxyableObject( L, 1 );
	return Remove( L, parent );
}

int
LuaGroupObjectProxyVTable::PushChild( lua_State *L, const GroupObject& o )
{
	int result = 0;

	int index = (int) lua_tointeger( L, 2 ) - 1; // Map Lua index to C index
	if ( index >= 0 )
	{
		// GroupObject* o = (GroupObject*)LuaProxy::GetProxyableObject( L, 1 );

		if ( index < o.NumChildren() )
		{
			const DisplayObject& child = o.ChildAt( index );
			LuaProxy* childProxy = child.GetProxy();

			if (childProxy != NULL)
			{
				result = childProxy->PushTable( L );
			}
		}
	}

	return result;
}

int
LuaGroupObjectProxyVTable::PushMethod( lua_State *L, const GroupObject& o, const char *key ) const
{
	int result = 1;

	static const char * keys[] =
	{
		"insert",			// 0
		"remove",			// 1
		"numChildren",		// 2
		"anchorChildren"	// 3
	};
    static const int numKeys = sizeof( keys ) / sizeof( const char * );
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 4, 0, 1, __FILE__, __LINE__ );
	StringHash *hash = &sHash;

	int index = hash->Lookup( key );
	switch ( index )
	{
	case 0:
		{
			Lua::PushCachedFunction( L, Self::insert );
			result = 1;
		}
		break;
	case 1:
		{
			Lua::PushCachedFunction( L, Self::Remove );
			result = 1;
		}
		break;
	case 2:
		{
			// GroupObject* o = (GroupObject*)LuaProxy::GetProxyableObject( L, 1 );
			lua_pushinteger( L, o.NumChildren() );
			result = 1;
		}
		break;
	case 3:
		{
			lua_pushboolean( L, o.IsAnchorChildren() );
			result = 1;
		}
		break;
	default:
		{
            result = 0;
		}
		break;
	}

    if ( result == 0 && strcmp( key, "_properties" ) == 0 )
    {
        String snapshotProperties(LuaContext::GetRuntime( L )->Allocator());
        const char **keys = NULL;
        const int numKeys = hash->GetKeys(keys);

        DumpObjectProperties( L, o, keys, numKeys, snapshotProperties );
        Super::ValueForKey( L, o, "_properties", true );

        lua_pushfstring( L, "{ %s, %s }", snapshotProperties.GetString(), lua_tostring( L, -1 ) );
        lua_remove( L, -2 ); // pop super properties
        result = 1;
    }

    return result;
}

int
LuaGroupObjectProxyVTable::ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction /* = false */ ) const
{
	int result = 0;

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, GroupObject );
	const GroupObject& o = static_cast< const GroupObject& >( object );

	if ( lua_type( L, 2 ) == LUA_TNUMBER )
	{
		result = PushChild( L, o );
	}
	else if ( key )
	{
		result = PushMethod( L, o, key );

		if ( 0 == result )
		{
			result = Super::ValueForKey( L, object, key, overrideRestriction );
		}
	}

	return result;
}

bool
LuaGroupObjectProxyVTable::SetValueForKey( lua_State *L, MLuaProxyable& object, const char key[], int valueIndex ) const
{
	if ( ! key ) { return false; }

	bool result = true;

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, GroupObject );

	if ( 0 == strcmp( key, "anchorChildren" ) )
	{
		GroupObject& o = static_cast< GroupObject& >( object );

		o.SetAnchorChildren( !! lua_toboolean( L, valueIndex ) );
		
#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
        if ( o.IsV1Compatibility() )
        {
            CoronaLuaWarning(L, "group.anchorChildren is only supported in graphics 2.0. Your mileage may vary in graphicsCompatibility 1.0 mode");
        }
#endif
	}
	else
	{
		result = Super::SetValueForKey( L, object, key, valueIndex );
	}

	return result;
}

const LuaProxyVTable&
LuaGroupObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

// [OLD] stage:setFocus( object )
// [NEW] stage:setFocus( object [, touchId] )
int
LuaStageObjectProxyVTable::setFocus( lua_State *L )
{
	StageObject* o = (StageObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, StageObject );

	if ( o )
	{
		// By default, assume this is a call to set global focus (i.e. old behavior)
		bool isGlobal = true;
		DisplayObject* focus = NULL;

		if ( lua_istable( L, 2 ) )
		{
			focus = (DisplayObject*)LuaProxy::GetProxyableObject( L, 2 );
			Rtt_WARN_SIM_PROXY_TYPE( L, 2, DisplayObject );

			// If the optional touchId arg exists, then we are using the new behavior
			// If it doesn't, then isGlobal remains true and we use the old behavior
			if ( lua_type( L, 3 ) != LUA_TNONE )
			{
				const void *touchId = lua_touserdata( L, 3 );

				const MPlatformDevice& device = LuaContext::GetRuntime( L )->Platform().GetDevice();
				if ( device.DoesNotify( MPlatformDevice::kMultitouchEvent ) )
				{
					// If optional parameter supplied, set per-object focus instead of global
					isGlobal = false;
					o->SetFocus( focus, touchId );
				}
				else
				{
					// The new API maps to old behavior when we are *not* multitouch
					if ( ! touchId )
					{
						focus = NULL;
					}
				}
			}
		}

		if ( isGlobal )
		{
			o->SetFocus( focus );
		}
	}

	return 0;
}

const LuaStageObjectProxyVTable&
LuaStageObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

int
LuaStageObjectProxyVTable::ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction /* = false */ ) const
{
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, StageObject );

    if ( ! key )
    {
        // If there's no key, we'll may have a table index to look up which is handled
        // by LuaGroupObjectProxyVTable::ValueForKey()
        return Super::ValueForKey( L, object, key );
    }

    int result = 1;

    static const char * keys[] =
    {
        "setFocus",			// 0
    };
	static const int numKeys = sizeof( keys ) / sizeof( const char * );
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 1, 0, 1, __FILE__, __LINE__ );
	StringHash *hash = &sHash;

    int index = hash->Lookup( key );
    switch ( index )
    {
        case 0:
            {
                Lua::PushCachedFunction( L, Self::setFocus );
            }
            break;
        default:
            {
                result = Super::ValueForKey( L, object, key, overrideRestriction );
            }
            break;
    }
    
    // If we retrieved the "_properties" key from the super, merge it with the local properties
    if ( result == 1 && strcmp( key, "_properties" ) == 0 )
    {
        String stageProperties(LuaContext::GetRuntime( L )->Allocator());

        DumpObjectProperties( L, object, keys, numKeys, stageProperties );

        lua_pushfstring( L, "{ %s, %s }", stageProperties.GetString(), lua_tostring( L, -1 ) );
        lua_remove( L, -2 ); // pop super properties
        result = 1;
    }

    return result;
}

const LuaProxyVTable&
LuaStageObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

const LuaTextObjectProxyVTable&
LuaTextObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

int
LuaTextObjectProxyVTable::ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction /* = false */ ) const
{
	if ( ! key ) { return 0; }
	
	int result = 1;

	static const char * keys[] =
	{
		"text",              // 0
		"size",              // 1
		"setMask",           // 2
		"setTextColor#",     // 3 - DEPRECATED
		"baselineOffset",    // 4
	};

	static const int numKeys = sizeof( keys ) / sizeof( const char * );
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 5, 2, 2, __FILE__, __LINE__ );
	StringHash *hash = &sHash;

	// TextObject* o = (TextObject*)LuaProxy::GetProxyableObject( L, 1 );
	const TextObject& o = static_cast< const TextObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, TextObject );

	int index = hash->Lookup( key );

	switch ( index )
	{
        case 0: // "text"
			{
				lua_pushstring( L, o.GetText() );
			}
			break;
        case 1: // "size"
			{
				lua_pushnumber( L, Rtt_RealToFloat( o.GetSize() ) );
			}
			break;
        case 2: // setMask
			{
				// Disable o:setMask() calls for TextObjects. The mask is already used up by the text itself.
				result = 0;
			}
			break;
        case 3: // setTextColor
			{
#if defined( Rtt_DEBUG ) || defined( Rtt_AUTHORING_SIMULATOR )
				if ( o.IsV1Compatibility() )
                {
                    CoronaLuaWarning(L, "o:setTextColor() is deprecated. Use o:setFillColor() instead");
                }
#endif
				Lua::PushCachedFunction( L, LuaShapeObjectProxyVTable::setFillColor );
            }
            break;
		case 4:
			{
				lua_pushnumber(L, o.GetBaselineOffset() );
			}
            break;
		default:
			{
				result = Super::ValueForKey( L, object, key, overrideRestriction );
			}
			break;
	}

    // If we retrieved the "_properties" key from the super, merge it with the local properties
    if (result == 1 && strcmp( key, "_properties" ) == 0 )
    {
        String properties(LuaContext::GetRuntime( L )->Allocator());
        const char *prefix = "";
        const char *postfix = "";

        DumpObjectProperties( L, object, keys, numKeys, properties );

        // "EmbossedTextObjects" are derived from "TextObjects" so
        // we need to emit complete JSON in those cases so we add the enclosing braces if
        // this is a "TextObject" but not if it's forming part of of a larger object
        if (strcmp(o.GetObjectDesc(), "TextObject") == 0)
        {
            prefix = "{ ";
            postfix = " }";
        }

        // Combine this object's properties with those of the super that were pushed above
        lua_pushfstring( L, "%s%s, %s%s", prefix, properties.GetString(), lua_tostring( L, -1 ), postfix );

        lua_remove( L, -2 ); // pop super properties
        result = 1;
    }

    return result;
}

bool
LuaTextObjectProxyVTable::SetValueForKey( lua_State *L, MLuaProxyable& object, const char key[], int valueIndex ) const
{
	if ( ! key ) { return false; }

	// TextObject* o = (TextObject*)LuaProxy::GetProxyableObject( L, 1 );
	TextObject& o = static_cast< TextObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, TextObject );

	bool result = true;

	static const char * keys[] = 
	{
		"text",			// 0
		"size"			// 1
	};
	static const int numKeys = sizeof( keys ) / sizeof( const char * );
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 2, 0, 1, __FILE__, __LINE__ );
	StringHash *hash = &sHash;

	int index = hash->Lookup( key );

	switch ( index )
	{
	case 0:
		{
			o.SetText( lua_tostring( L, valueIndex ) );
		}
		break;
	case 1:
		{
			o.SetSize( luaL_toreal( L, valueIndex ) );
		}
		break;
	default:
		{
			result = Super::SetValueForKey( L, object, key, valueIndex );
		}
		break;
	}

	return result;
}

const LuaProxyVTable&
LuaTextObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

int
LuaPlatformDisplayObjectProxyVTable::ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction /* = false */ ) const
{
	if ( ! key ) { return 0; }

	// PlatformDisplayObject* o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	const PlatformDisplayObject& o = static_cast< const PlatformDisplayObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, PlatformDisplayObject );

	int result = o.ValueForKey( L, key ) || Super::ValueForKey( L, object, key, overrideRestriction );

	if ( 0 == result )
	{
		if ( strcmp( "getNativeProperty", key ) == 0 )
		{
			lua_pushlightuserdata( L, const_cast< PlatformDisplayObject * >( & o ) );
			lua_pushcclosure( L, PlatformDisplayObject::getNativeProperty, 1 );
			result = 1;
		}
		else if ( strcmp( "setNativeProperty", key ) == 0 )
		{
			lua_pushlightuserdata( L, const_cast< PlatformDisplayObject * >( & o ) );
			lua_pushcclosure( L, PlatformDisplayObject::setNativeProperty, 1 );
			result = 1;
		}
	}

    // If we retrieved the "_properties" key from the super, merge it with the local properties
    if ( result == 1 && strcmp( key, "_properties" ) == 0 )
    {
        String textProperties(LuaContext::GetRuntime( L )->Allocator());

        // TODO: Implement DumpObjectProperties for PlatformDisplayObjects
        // DumpObjectProperties( L, object, keys, numKeys, textProperties );

        // lua_pushfstring( L, "{ %s, %s }", textProperties.GetString(), lua_tostring( L, -1 ) );
        lua_pushfstring( L, "{ %s }", lua_tostring( L, -1 ) );
        lua_remove( L, -2 ); // pop super properties
        result = 1;
    }

	return result;
}

bool
LuaPlatformDisplayObjectProxyVTable::SetValueForKey( lua_State *L, MLuaProxyable& object, const char key[], int valueIndex ) const
{
	if ( ! key ) { return false; }

	// PlatformDisplayObject* o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	PlatformDisplayObject& o = static_cast< PlatformDisplayObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, PlatformDisplayObject );

	bool result =
		o.SetValueForKey( L, key, valueIndex )
		|| Super::SetValueForKey( L, object, key, valueIndex );

	return result;
}

// ----------------------------------------------------------------------------

const LuaEmbossedTextObjectProxyVTable&
LuaEmbossedTextObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

int
LuaEmbossedTextObjectProxyVTable::ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction /* = false */ ) const
{
	if ( ! key ) { return 0; }
	
	int result = 1;

	static const char * keys[] = 
	{
		"setText",			// 0
		"setSize",			// 1
		"setEmbossColor",	// 2
		"setTextColor",		// 3
		"setFillColor",		// 4
	};
	static const int numKeys = sizeof( keys ) / sizeof( const char * );
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 5, 4, 9, __FILE__, __LINE__ );
	StringHash *hash = &sHash;

	EmbossedTextObject& textObject = (EmbossedTextObject&)const_cast<MLuaProxyable&>(object);

	int index = hash->Lookup( key );
	switch ( index )
	{
		case 0:
			lua_pushcfunction( L, OnSetText );
			break;
		case 1:
			lua_pushcfunction( L, OnSetSize );
			break;
		case 2:
			lua_pushcfunction( L, OnSetEmbossColor );
			break;
		case 3:			// setTextColor
		case 4:			// setFillColor
			// Resetting the foreground text color will revert the embossed colors back to their defaults.
			textObject.UseDefaultHighlightColor();
			textObject.UseDefaultShadowColor();
			result = Super::ValueForKey( L, object, key, overrideRestriction );
			break;
		default:
			result = Super::ValueForKey( L, object, key, overrideRestriction );
			break;
	}

    // If we retrieved the "_properties" key from the super, merge it with the local properties
    if ( result == 1 && strcmp( key, "_properties" ) == 0 )
    {
        String embossedTextProperties(LuaContext::GetRuntime( L )->Allocator());

        DumpObjectProperties( L, object, keys, numKeys, embossedTextProperties );

        // Currently embossedTextProperties is empty but this might change in the future, make the JSON right
        const char *comma = "";
        if (embossedTextProperties.IsEmpty())
        {
            comma = "";
        }
        else
        {
            comma = ", ";
        }

        lua_pushfstring( L, "{ %s%s%s }", embossedTextProperties.GetString(), comma, lua_tostring( L, -1 ) );
        lua_remove( L, -2 ); // pop super properties
        result = 1;
    }

	return result;
}

bool
LuaEmbossedTextObjectProxyVTable::SetValueForKey( lua_State *L, MLuaProxyable& object, const char key[], int valueIndex ) const
{
	return Super::SetValueForKey( L, object, key, valueIndex );
}

int
LuaEmbossedTextObjectProxyVTable::OnSetText( lua_State *L )
{
	TextObject *textObjectPointer = (TextObject*)LuaProxy::GetProxyableObject( L, 1 );
	if (textObjectPointer)
	{
		textObjectPointer->SetText( lua_tostring( L, 2 ) );
	}
	return 0;
}

int
LuaEmbossedTextObjectProxyVTable::OnSetSize( lua_State *L )
{
	TextObject *textObjectPointer = (TextObject*)LuaProxy::GetProxyableObject( L, 1 );
	if (textObjectPointer)
	{
		Runtime& runtime = * LuaContext::GetRuntime( L );
		const Display& display = runtime.GetDisplay();
		Real fontSize = Rtt_RealDiv( luaL_toreal( L, 2 ), display.GetSx() );
		textObjectPointer->SetSize( fontSize );
	}
	return 0;
}

static U8
GetEmbossedColorValueFromField(
	lua_State *L, int tableIndex, const char *fieldName, U8 defaultValue, bool isByteValue)
{
	U8 value = defaultValue;
	if (L && tableIndex && fieldName)
	{
		lua_getfield(L, tableIndex, fieldName);
		if (lua_type(L, -1) == LUA_TNUMBER)
		{
			if (isByteValue)
			{

				value = (U8)Clamp((int)lua_tointeger(L, -1), 0, 255);
			}
			else
			{
				double decimalValue = Clamp(lua_tonumber(L, -1), 0.0, 1.0) * 255.0;
				value = (U8)(decimalValue + 0.5);
			}
		}
		lua_pop(L, 1);
	}
	return value;
}

int
LuaEmbossedTextObjectProxyVTable::OnSetEmbossColor( lua_State *L )
{
	// Validate.
	if (NULL == L)
	{
		return 0;
	}
	
	// Fetch the text object.
	EmbossedTextObject *textObjectPointer = (EmbossedTextObject*)LuaProxy::GetProxyableObject(L, 1);
	if (NULL == textObjectPointer)
	{
		return 0;
	}

	// Default the highlight and shadow colors to white.
	// Note: It doesn't make sense to make the shadow color default to white, but that was the old behavior.
	RGBA highlightColor = { 255, 255, 255, 255 };
	RGBA shadowColor = highlightColor;
	
	// Fetch the emboss colors from the table argument.
	if (lua_istable(L, 2))
	{
		// Fetch the highlight colors.
		lua_getfield(L, 2, "highlight");
		if (lua_istable(L, -1))
		{
			highlightColor.r = GetEmbossedColorValueFromField(
									L, -1, "r", highlightColor.r, textObjectPointer->IsByteColorRange());
			highlightColor.g = GetEmbossedColorValueFromField(
									L, -1, "g", highlightColor.g, textObjectPointer->IsByteColorRange());
			highlightColor.b = GetEmbossedColorValueFromField(
									L, -1, "b", highlightColor.b, textObjectPointer->IsByteColorRange());
			highlightColor.a = GetEmbossedColorValueFromField(
									L, -1, "a", highlightColor.a, textObjectPointer->IsByteColorRange());
		}
		lua_pop(L, 1);

		// Fetch the shadow colors.
		lua_getfield(L, 2, "shadow");
		if (lua_istable(L, -1))
		{
			shadowColor.r = GetEmbossedColorValueFromField(
									L, -1, "r", shadowColor.r, textObjectPointer->IsByteColorRange());
			shadowColor.g = GetEmbossedColorValueFromField(
									L, -1, "g", shadowColor.g, textObjectPointer->IsByteColorRange());
			shadowColor.b = GetEmbossedColorValueFromField(
									L, -1, "b", shadowColor.b, textObjectPointer->IsByteColorRange());
			shadowColor.a = GetEmbossedColorValueFromField(
									L, -1, "a", shadowColor.a, textObjectPointer->IsByteColorRange());
		}
		lua_pop(L, 1);
	}
	
	// Apply the emboss colors to the text object.
	textObjectPointer->SetHighlightColor(highlightColor);
	textObjectPointer->SetShadowColor(shadowColor);
	return 0;
}

const LuaProxyVTable&
LuaEmbossedTextObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

// Need explicit default constructor for const use by C++ spec
LuaPlatformTextFieldObjectProxyVTable::LuaPlatformTextFieldObjectProxyVTable()
	: LuaPlatformDisplayObjectProxyVTable()
{
}

const LuaPlatformTextFieldObjectProxyVTable&
LuaPlatformTextFieldObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

const LuaProxyVTable&
LuaPlatformTextFieldObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

// Need explicit default constructor for const use by C++ spec
LuaPlatformTextBoxObjectProxyVTable::LuaPlatformTextBoxObjectProxyVTable()
	: LuaPlatformDisplayObjectProxyVTable()
{
}

const LuaPlatformTextBoxObjectProxyVTable&
LuaPlatformTextBoxObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

const LuaProxyVTable&
LuaPlatformTextBoxObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

// Need explicit default constructor for const use by C++ spec
LuaPlatformMapViewObjectProxyVTable::LuaPlatformMapViewObjectProxyVTable()
	: LuaPlatformDisplayObjectProxyVTable()
{
}

const LuaPlatformMapViewObjectProxyVTable&
LuaPlatformMapViewObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

const LuaProxyVTable&
LuaPlatformMapViewObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

// Need explicit default constructor for const use by C++ spec
LuaPlatformWebViewObjectProxyVTable::LuaPlatformWebViewObjectProxyVTable()
	: LuaPlatformDisplayObjectProxyVTable()
{
}

const LuaPlatformWebViewObjectProxyVTable&
LuaPlatformWebViewObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

const LuaProxyVTable&
LuaPlatformWebViewObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

// Need explicit default constructor for const use by C++ spec
LuaPlatformVideoObjectProxyVTable::LuaPlatformVideoObjectProxyVTable()
	: LuaPlatformDisplayObjectProxyVTable()
{
}

const LuaPlatformVideoObjectProxyVTable&
LuaPlatformVideoObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

const LuaProxyVTable&
LuaPlatformVideoObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

const LuaSpriteObjectProxyVTable&
LuaSpriteObjectProxyVTable::Constant()
{
	static const Self kVTable;
	return kVTable;
}

int
LuaSpriteObjectProxyVTable::play( lua_State *L )
{
	SpriteObject *o = (SpriteObject*)LuaProxy::GetProxyableObject( L, 1 );
	
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, SpriteObject );
	
	if ( o )
	{
		o->Play( L );
	}

	return 0;
}

int
LuaSpriteObjectProxyVTable::pause( lua_State *L )
{
	SpriteObject *o = (SpriteObject*)LuaProxy::GetProxyableObject( L, 1 );
	
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, SpriteObject );
	
	if ( o )
	{
		o->Pause();
	}

	return 0;
}

int
LuaSpriteObjectProxyVTable::setSequence( lua_State *L )
{
	SpriteObject *o = (SpriteObject*)LuaProxy::GetProxyableObject( L, 1 );
	
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, SpriteObject );
	
	if ( o )
	{
		const char *name = lua_tostring( L, 2 );
		o->SetSequence( name );
	}

	return 0;
}

int
LuaSpriteObjectProxyVTable::setFrame( lua_State *L )
{
	SpriteObject *o = (SpriteObject*)LuaProxy::GetProxyableObject( L, 1 );
	
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, SpriteObject );
	
	if ( o )
	{
		int index = (int) lua_tointeger( L, 2 );
		if ( index < 1 )
		{
			CoronaLuaWarning(L, "sprite:setFrame() given invalid index (%d). Using index of 1 instead", index);
			index = 1;
		}
		else if ( index > o->GetNumFrames() )
		{
			CoronaLuaWarning(L, "sprite:setFrame() given invalid index (%d). Using index of %d instead", index, o->GetNumFrames() );
			index = o->GetNumFrames();
		}
		o->SetFrame( index - 1 ); // Lua is 1-based
	}

	return 0;
}

int
LuaSpriteObjectProxyVTable::useFrameForAnchors( lua_State *L )
{
	SpriteObject *o = (SpriteObject*)LuaProxy::GetProxyableObject( L, 1 );
	
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, SpriteObject );
	
	if ( o )
	{
		int index;
		if ( lua_isnoneornil( L, 2 ) )
		{
			index = o->GetFrame();
		}		
		else
		{
			index = (int) lua_tointeger( L, 2 );
			if ( index < 1 )
			{
				CoronaLuaWarning(L, "sprite:useFrameForAnchors() given invalid index (%d). Using index of 1 instead", index);
				index = 1;
			}
			else if ( index > o->GetNumFrames() )
			{
				CoronaLuaWarning(L, "sprite:useFrameForAnchors() given invalid index (%d). Using index of %d instead", index, o->GetNumFrames() );
				index = o->GetNumFrames();
			}
			--index; // Lua is 1-based
		}
		o->UseFrameForAnchors( index ); // Lua is 1-based
	}

	return 0;
}

int
LuaSpriteObjectProxyVTable::ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction /* = false */ ) const
{
	if ( ! key ) { return 0; }
	
	int result = 1;

	static const char * keys[] =
	{
		// Read-write properties
		"timeScale",	// 0
	
		// Read-only properties
		"frame",		// 1
		"numFrames",	// 2
		"isPlaying",	// 3
		"sequence",		// 4

		// Methods
		"play",			// 5
		"pause",		// 6
		"setSequence",	// 7
		"setFrame",		// 8
		"useFrameForAnchors"	// 9
	};
	static const int numKeys = sizeof( keys ) / sizeof( const char * );
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 10, 25, 7, __FILE__, __LINE__ );
	StringHash *hash = &sHash;

	int index = hash->Lookup( key );

	const SpriteObject& o = static_cast< const SpriteObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, SpriteObject );

	switch ( index )
	{
	case 0:
		{
			Real timeScale = o.GetTimeScale();
			lua_pushnumber( L, Rtt_RealToFloat( timeScale ) );
		}
		break;
	case 1:
		{
			int currentFrame = o.GetFrame() + 1; // Lua is 1-based
			lua_pushinteger( L, currentFrame );
		}
		break;
	case 2:
		{
			lua_pushinteger( L, o.GetNumFrames() );
		}
		break;
	case 3:
		{
			lua_pushboolean( L, o.IsPlaying() );
		}
		break;
	case 4:
		{
			const char *sequenceName = o.GetSequence();
			if ( sequenceName )
			{
				lua_pushstring( L, sequenceName );
			}
			else
			{
				lua_pushnil( L );
			}
		}
		break;
	case 5:
		{
			Lua::PushCachedFunction( L, Self::play );
		}
		break;
	case 6:
		{
			Lua::PushCachedFunction( L, Self::pause );
		}
		break;
	case 7:
		{
			Lua::PushCachedFunction( L, Self::setSequence );
		}
		break;
	case 8:
		{
			Lua::PushCachedFunction( L, Self::setFrame );
		}
		break;
	case 9:
		{
			Lua::PushCachedFunction( L, Self::useFrameForAnchors );
		}
		break;
	default:
		{
			result = Super::ValueForKey( L, object, key, overrideRestriction );
		}
		break;
	}

    // If we retrieved the "_properties" key from the super, merge it with the local properties
    if ( result == 1 && strcmp( key, "_properties" ) == 0 )
    {
        String spriteProperties(LuaContext::GetRuntime( L )->Allocator());

        DumpObjectProperties( L, object, keys, numKeys, spriteProperties );

        lua_pushfstring( L, "{ %s, %s }", spriteProperties.GetString(), lua_tostring( L, -1 ) );
        lua_remove( L, -2 ); // pop super properties
        result = 1;
    }

    return result;
}

bool
LuaSpriteObjectProxyVTable::SetValueForKey( lua_State *L, MLuaProxyable& object, const char key[], int valueIndex ) const
{
	if ( ! key ) { return false; }

	SpriteObject& o = static_cast< SpriteObject& >( object );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, SpriteObject );

	bool result = true;

	static const char * keys[] =
	{
		// Read-write properties
		"timeScale",	// 0
	
		// Read-only properties
		"frame",		// 1
		"numFrames",	// 2
		"isPlaying",	// 3
		"sequence",		// 4
	};
	static const int numKeys = sizeof( keys ) / sizeof( const char * );
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 5, 1, 1, __FILE__, __LINE__ );
	StringHash *hash = &sHash;

	int index = hash->Lookup( key );

	switch ( index )
	{
	case 0:
		{
			Real timeScale = Rtt_FloatToReal( (float)lua_tonumber( L, valueIndex ) );
			Real min = Rtt_FloatToReal( 0.05f );
			Real max = Rtt_FloatToReal( 20.0f );
			if ( timeScale < min )
			{
				CoronaLuaWarning(L, "sprite.timeScale must be >= %g. Using %g", min, min);
				timeScale = min;
			}
			else if ( timeScale < min )
			{
				CoronaLuaWarning(L, "sprite.timeScale must be <= %g. Using %g", max, max);
				timeScale = max;
			}
			o.SetTimeScale( timeScale );
		}
		break;

	case 1:
	case 2:
	case 3:
	case 4:
		{
			// Read-only properties
			// no-op
		}
		break;

	default:
		{
			result = Super::SetValueForKey( L, object, key, valueIndex );
		}
		break;
	}

    return result;
}

const LuaProxyVTable&
LuaSpriteObjectProxyVTable::Parent() const
{
	return Super::Constant();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
