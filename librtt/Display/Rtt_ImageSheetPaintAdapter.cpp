//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ImageSheetPaintAdapter.h"

#include "Display/Rtt_ImageSheetPaint.h"
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

const ImageSheetPaintAdapter&
ImageSheetPaintAdapter::Constant()
{
	static const ImageSheetPaintAdapter sAdapter;
	return sAdapter;
}

// ----------------------------------------------------------------------------

ImageSheetPaintAdapter::ImageSheetPaintAdapter()
{
}

StringHash *
ImageSheetPaintAdapter::GetHash( lua_State *L ) const
{
	static const char *keys[] = 
	{
		"frame",		// 0
	};

	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 1, 1, 1, __FILE__, __LINE__ );
	return &sHash;
}

int
ImageSheetPaintAdapter::ValueForKey(
	const LuaUserdataProxy& sender,
	lua_State *L,
	const char *key ) const
{
	int result = 0;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	const ImageSheetPaint *paint = (const ImageSheetPaint *)sender.GetUserdata();
	if ( ! paint ) { return result; }

	int index = GetHash( L )->Lookup( key );

	switch ( index )
	{
		case 0:
			result = 1;
			lua_pushinteger( L, paint->GetFrame() + 1 );
			break;
		default:
			result = Super::Constant().ValueForKey( sender, L, key );
			break;
	}
	
	return result;
}

bool
ImageSheetPaintAdapter::SetValueForKey(
	LuaUserdataProxy& sender,
	lua_State *L,
	const char *key,
	int valueIndex ) const
{
	bool result = false;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	ImageSheetPaint *paint = (ImageSheetPaint *)sender.GetUserdata();
	if ( ! paint ) { return result; }

	int index = GetHash( L )->Lookup( key );
	
	switch ( index )
	{
		case 0:
			result = true;
			if( lua_type( L, valueIndex ) == LUA_TNUMBER )
			{
				int frame = (int)lua_tointeger( L, valueIndex ) - 1;
				frame = Clamp( frame, 0, paint->GetNumFrames()-1 );
				paint->SetFrame(frame);
			}
			else
			{
				Rtt_TRACE_SIM( ( "ERROR: paint.frame should be an iteger.\n" ) );
			}
			break;
		default:
			result = Super::Constant().SetValueForKey( sender, L, key, valueIndex );
			break;
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

