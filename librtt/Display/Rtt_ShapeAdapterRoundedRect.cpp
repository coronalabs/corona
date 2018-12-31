//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ShapeAdapterRoundedRect.h"

#include "Core/Rtt_StringHash.h"
#include "Display/Rtt_ShapeObject.h"
#include "Display/Rtt_ShapePath.h"
#include "Display/Rtt_TesselatorRoundedRect.h"
#include "Rtt_LuaContext.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const ShapeAdapterRoundedRect&
ShapeAdapterRoundedRect::Constant()
{
	static const ShapeAdapterRoundedRect sAdapter;
	return sAdapter;
}

// ----------------------------------------------------------------------------

ShapeAdapterRoundedRect::ShapeAdapterRoundedRect()
:	Super( kRoundedRectType )
{
}

StringHash *
ShapeAdapterRoundedRect::GetHash( lua_State *L ) const
{
	static const char *keys[] = 
	{
		"width",				// 0
		"height",				// 1
		"radius",				// 2
	};
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 3, 0, 1, __FILE__, __LINE__ );
	return &sHash;
}

int
ShapeAdapterRoundedRect::ValueForKey(
	const LuaUserdataProxy& sender,
	lua_State *L,
	const char *key ) const
{
	int result = 0;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	const ShapePath *path = (const ShapePath *)sender.GetUserdata();
	if ( ! path ) { return result; }

	const TesselatorRoundedRect *tesselator =
		static_cast< const TesselatorRoundedRect * >( path->GetTesselator() );
	if ( ! tesselator ) { return result; }

	result = 1; // Assume 1 Lua value will be pushed on the stack

	int index = GetHash( L )->Lookup( key );
	switch ( index )
	{
		case 0:
			lua_pushnumber( L, tesselator->GetWidth() );
			break;
		case 1:
			lua_pushnumber( L, tesselator->GetHeight() );
			break;
		case 2:
			lua_pushnumber( L, tesselator->GetRadius() );
			break;
		default:
			result = Super::ValueForKey( sender, L, key );
			break;
	}

	return result;
}

bool
ShapeAdapterRoundedRect::SetValueForKey(
	LuaUserdataProxy& sender,
	lua_State *L,
	const char *key,
	int valueIndex ) const
{
	bool result = false;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	ShapePath *path = (ShapePath *)sender.GetUserdata();
	if ( ! path ) { return result; }

	TesselatorRoundedRect *tesselator =
		static_cast< TesselatorRoundedRect * >( path->GetTesselator() );
	if ( ! tesselator ) { return result; }

	result = true; // Assume value will be set

	int index = GetHash( L )->Lookup( key );
	switch ( index )
	{
		case 0:
			{
				Real newValue = luaL_toreal( L, valueIndex );
				tesselator->SetWidth( newValue );
				path->Invalidate( ClosedPath::kFillSource | ClosedPath::kStrokeSource );
				path->GetObserver()->Invalidate( DisplayObject::kGeometryFlag | DisplayObject::kStageBoundsFlag );
			}
			break;
		case 1:
			{
				Real newValue = luaL_toreal( L, valueIndex );
				tesselator->SetHeight( newValue );
				path->Invalidate( ClosedPath::kFillSource | ClosedPath::kStrokeSource );
				path->GetObserver()->Invalidate( DisplayObject::kGeometryFlag | DisplayObject::kStageBoundsFlag );
			}
			break;
		case 2:
			{
				Real maxRadius = Min( tesselator->GetWidth(), tesselator->GetHeight() );
				maxRadius = Rtt_RealDiv2( maxRadius );

				Real radius = luaL_toreal( L, valueIndex );
				radius = Min( radius, maxRadius );

				tesselator->SetRadius( radius );
				path->Invalidate( ClosedPath::kFillSource | ClosedPath::kStrokeSource );
				path->GetObserver()->Invalidate( DisplayObject::kGeometryFlag | DisplayObject::kStageBoundsFlag );
			}
			break;
		default:
			result = Super::SetValueForKey( sender, L, key, valueIndex );
			break;
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

