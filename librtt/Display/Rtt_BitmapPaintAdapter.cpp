//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_BitmapPaintAdapter.h"

#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_DisplayPath.h"
#include "Display/Rtt_PaintAdapter.h"
#include "Display/Rtt_ShapeObject.h"
#include "Core/Rtt_StringHash.h"
#include "Rtt_LuaContext.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const BitmapPaintAdapter&
BitmapPaintAdapter::Constant()
{
	static const BitmapPaintAdapter sAdapter;
	return sAdapter;
}

// ----------------------------------------------------------------------------

BitmapPaintAdapter::BitmapPaintAdapter()
{
}

StringHash *
BitmapPaintAdapter::GetHash( lua_State *L ) const
{
	static const char *keys[] = 
	{
		"x",			// 0
		"y",			// 1
		"scaleX",		// 2
		"scaleY",		// 3
		"rotation",		// 4
	};

	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 5, 1, 1, __FILE__, __LINE__ );
	return &sHash;
}

int
BitmapPaintAdapter::ValueForKey(
	const LuaUserdataProxy& sender,
	lua_State *L,
	const char *key ) const
{
	int result = 1;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	const BitmapPaint *paint = (const BitmapPaint *)sender.GetUserdata();
	if ( ! paint ) { return result; }

	int index = GetHash( L )->Lookup( key );

	if ( index >= 0 )
	{
		const Transform& t = paint->GetTransform();

		result = 1;
		switch ( index )
		{
			case 0:
				lua_pushnumber( L, Rtt_RealToFloat( t.GetProperty( kOriginX ) ) );
				break;
			case 1:
				lua_pushnumber( L, Rtt_RealToFloat( t.GetProperty( kOriginY ) ) );
				break;
			case 2:
				lua_pushnumber( L, Rtt_RealToFloat( Rtt_RealDiv( Rtt_REAL_1,  t.GetProperty( kScaleX ) ) ) );
				break;
			case 3:
				lua_pushnumber( L, Rtt_RealToFloat( Rtt_RealDiv( Rtt_REAL_1,  t.GetProperty( kScaleY ) ) ) );
				break;
			case 4:
				lua_pushnumber( L, Rtt_RealToFloat( t.GetProperty( kRotation ) ) );
				break;
			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
		}
	}
	else
	{
		result = Super::Constant().ValueForKey( sender, L, key );
	}

	return result;
}

bool
BitmapPaintAdapter::SetValueForKey(
	LuaUserdataProxy& sender,
	lua_State *L,
	const char *key,
	int valueIndex ) const
{
	bool result = false;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	BitmapPaint *paint = (BitmapPaint *)sender.GetUserdata();
	if ( ! paint ) { return result; }

	int index = GetHash( L )->Lookup( key );
	
	if ( index >= 0 )
	{ 
		result = true;

		Transform& t = paint->GetTransform();
		Real value = luaL_toreal( L, valueIndex );
		switch ( index )
		{
			case 0:
				t.SetProperty( kOriginX, value );
				break;
			case 1:
				t.SetProperty( kOriginY, value );
				break;
			case 2:
				t.SetProperty( kScaleX, Rtt_RealDiv( Rtt_REAL_1, value ) );
				break;
			case 3:
				t.SetProperty( kScaleY, Rtt_RealDiv( Rtt_REAL_1, value ) );
				break;
			case 4:
				t.SetProperty( kRotation, value );
				break;
			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
		}

		paint->Invalidate( Paint::kTextureTransformFlag );

		result = true;
	}
	else
	{
		result = Super::Constant().SetValueForKey( sender, L, key, valueIndex );
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

