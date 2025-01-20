//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_GradientPaintAdapter.h"

#include "Display/Rtt_BitmapPaintAdapter.h"
#include "Display/Rtt_GradientPaint.h"
#include "Display/Rtt_ShapeObject.h"
#include "Core/Rtt_StringHash.h"
#include "Rtt_LuaContext.h"
#include "CoronaLua.h"

#ifdef Rtt_DEBUG
	#include "Display/Rtt_ClosedPath.h"
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const GradientPaintAdapter&
GradientPaintAdapter::Constant()
{
	static const GradientPaintAdapter sAdapter;
	return sAdapter;
}

// ----------------------------------------------------------------------------

GradientPaintAdapter::GradientPaintAdapter()
{
}

StringHash *
GradientPaintAdapter::GetHash( lua_State *L ) const
{
	static const char *keys[] = 
	{
		"r1",			// 0
		"g1",			// 1
		"b1",			// 2
		"a1",			// 3
		"r2",			// 4
		"g2",			// 5
		"b2",			// 6
		"a2",			// 7
	};

	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 8, 0, 2, __FILE__, __LINE__ );
	return &sHash;
}

int
GradientPaintAdapter::ValueForKey(
	const LuaUserdataProxy& sender,
	lua_State *L,
	const char *key ) const
{
	int result = 0;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	const GradientPaint *paint = (const GradientPaint *)sender.GetUserdata();
	if ( ! paint ) { return result; }

	int index = GetHash( L )->Lookup( key );

	if ( index >= 0 )
	{
		ColorUnion c;
		switch ( index )
		{
			case 0:
			case 1:
			case 2:
			case 3:
				c.pixel = paint->GetStart();
				break;
			case 4:
			case 5:
			case 6:
			case 7:
				c.pixel = paint->GetEnd();
				index -= 4; // map to same range as 'start'
				break;
			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
		}

		U8 value = 0;
		switch ( index )
		{
			case 0:	value = c.rgba.r;	break;
			case 1:	value = c.rgba.g;	break;
			case 2:	value = c.rgba.b;	break;
			case 3:	value = c.rgba.a;	break;
			default:	break;
		}

		const float kInv255 = 1./255.;
		lua_pushnumber( L, kInv255 * value );

		result = 1;
	}
	else
	{
		result = Super::Constant().ValueForKey( sender, L, key );
	}

	return result;
}

bool
GradientPaintAdapter::SetValueForKey(
	LuaUserdataProxy& sender,
	lua_State *L,
	const char *key,
	int valueIndex ) const
{
	bool result = false;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	GradientPaint *paint = (GradientPaint *)sender.GetUserdata();
	if ( ! paint ) { return result; }

	int index = GetHash( L )->Lookup( key );

	if ( index >= 0 )
	{
		U8 value = lua_tonumber( L, valueIndex )*255;

		bool isStart = ( index < 4 );
		if ( ! isStart )
		{
			index -= 4; // map to same range as 'start'
		}

		ColorUnion c;
		c.pixel = ( isStart ? paint->GetStart() : paint->GetEnd() );

		switch ( index )
		{
			case 0:	c.rgba.r = value;	break;
			case 1:	c.rgba.g = value;	break;
			case 2:	c.rgba.b = value;	break;
			case 3:	c.rgba.a = value;	break;
			default:	Rtt_ASSERT_NOT_REACHED();	break;
		}

		if ( isStart )
		{
			paint->SetStart( c.pixel );
		}
		else
		{
			paint->SetEnd( c.pixel );
		}
		result = true;
	}
	
	if ( ! result )
	{
		result = Super::Constant().SetValueForKey( sender, L, key, valueIndex );
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

