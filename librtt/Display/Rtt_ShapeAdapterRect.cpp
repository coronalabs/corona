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

#include "Display/Rtt_ShapeAdapterRect.h"

#include "Core/Rtt_StringHash.h"
#include "Display/Rtt_RectPath.h"
#include "Display/Rtt_ShapeObject.h"
#include "Display/Rtt_TesselatorRect.h"
#include "Rtt_LuaContext.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const ShapeAdapterRect&
ShapeAdapterRect::Constant()
{
	static const ShapeAdapterRect sAdapter;
	return sAdapter;
}

// ----------------------------------------------------------------------------

ShapeAdapterRect::ShapeAdapterRect()
:	Super( kRectType )
{
}

StringHash *
ShapeAdapterRect::GetHash( lua_State *L ) const
{
	static const char *keys[] = 
	{
		"x1",				// 0
		"y1",				// 1
		"x2",				// 2
		"y2",				// 3
		"x3",				// 4
		"y3",				// 5
		"x4",				// 6
		"y4",				// 7
		"width",			// 8
		"height",			// 9
	};
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 10, 0, 2, __FILE__, __LINE__ );
	return &sHash;
}

int
ShapeAdapterRect::ValueForKey(
	const LuaUserdataProxy& sender,
	lua_State *L,
	const char *key ) const
{
	int result = 0;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	const RectPath *path = (const RectPath *)sender.GetUserdata();
	if ( ! path ) { return result; }

	const TesselatorRect *tesselator =
		static_cast< const TesselatorRect * >( path->GetTesselator() );
	if ( ! tesselator ) { return result; }

	result = 1; // Assume 1 Lua value will be pushed on the stack

	int index = GetHash( L )->Lookup( key );
	switch ( index )
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			{
				const int kBase = 0; // index of "x1"

				TesselatorRect::RectOffset offset =
					(TesselatorRect::RectOffset)(index - kBase);
				Real value = tesselator->GetOffset( offset );
				lua_pushnumber( L, Rtt_RealToFloat( value ) );
			}
			break;
		case 8:
			lua_pushnumber( L, tesselator->GetWidth() );
			break;
		case 9:
			lua_pushnumber( L, tesselator->GetHeight() );
			break;
		default:
			result = Super::ValueForKey( sender, L, key );
			break;
	}

	return result;
}

bool
ShapeAdapterRect::SetValueForKey(
	LuaUserdataProxy& sender,
	lua_State *L,
	const char *key,
	int valueIndex ) const
{
	bool result = false;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	RectPath *path = (RectPath *)sender.GetUserdata();
	if ( ! path ) { return result; }

	TesselatorRect *tesselator =
		static_cast< TesselatorRect * >( path->GetTesselator() );
	if ( ! tesselator ) { return result; }

	result = true; // Assume value will be set

	int index = GetHash( L )->Lookup( key );
	switch ( index )
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			{
				const int kBase = 0; // index of "x1"

				TesselatorRect::RectOffset offset =
					(TesselatorRect::RectOffset)(index - kBase);
				Real value = luaL_toreal( L, valueIndex );
				tesselator->SetOffset( offset, value );
				path->Invalidate( ClosedPath::kStrokeSource |
									ClosedPath::kFillSource |
									ClosedPath::kFillSourceTexture );
				path->GetObserver()->Invalidate( DisplayObject::kGeometryFlag |
													DisplayObject::kStageBoundsFlag |
													DisplayObject::kProgramFlag );
			}
			break;
		case 8:
			{
				Real newValue = luaL_toreal( L, valueIndex );
				tesselator->SetWidth( newValue );
				path->Invalidate( ClosedPath::kFillSource |
									ClosedPath::kStrokeSource );
				path->GetObserver()->Invalidate( DisplayObject::kGeometryFlag |
													DisplayObject::kStageBoundsFlag |
													DisplayObject::kTransformFlag );
			}
			break;
		case 9:
			{
				Real newValue = luaL_toreal( L, valueIndex );
				tesselator->SetHeight( newValue );
				path->Invalidate( ClosedPath::kFillSource |
									ClosedPath::kStrokeSource );
				path->GetObserver()->Invalidate( DisplayObject::kGeometryFlag |
													DisplayObject::kStageBoundsFlag  |
													DisplayObject::kTransformFlag );
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

