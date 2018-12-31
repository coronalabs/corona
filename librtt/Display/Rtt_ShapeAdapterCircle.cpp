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

#include "Display/Rtt_ShapeAdapterCircle.h"

#include "Core/Rtt_StringHash.h"
#include "Display/Rtt_ShapeObject.h"
#include "Display/Rtt_ShapePath.h"
#include "Display/Rtt_TesselatorCircle.h"
#include "Rtt_LuaContext.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const ShapeAdapterCircle&
ShapeAdapterCircle::Constant()
{
	static const ShapeAdapterCircle sAdapter;
	return sAdapter;
}

// ----------------------------------------------------------------------------

ShapeAdapterCircle::ShapeAdapterCircle()
:	Super( kCircleType )
{
}

StringHash *
ShapeAdapterCircle::GetHash( lua_State *L ) const
{
    static const char *keys[] =
    {
        "radius",				// 0
    };
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 1, 0, 0, __FILE__, __LINE__ );
	return &sHash;
}

int
ShapeAdapterCircle::ValueForKey(
	const LuaUserdataProxy& sender,
	lua_State *L,
	const char *key ) const
{
	int result = 0;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	const ShapePath *path = (const ShapePath *)sender.GetUserdata();
	if ( ! path ) { return result; }

    int index = GetHash( L )->Lookup( key );
    switch ( index )
    {
        case 0:
            {
                const TesselatorCircle *tesselator = static_cast< const TesselatorCircle * >( path->GetTesselator() );
                
                if ( tesselator )
                {
                    lua_pushnumber( L, tesselator->GetRadius() );
                    result = 1;
                }
            }
            break;

        default:
			result = Super::ValueForKey( sender, L, key );
            break;
    }

	return result;
}

bool
ShapeAdapterCircle::SetValueForKey(
	LuaUserdataProxy& sender,
	lua_State *L,
	const char *key,
	int valueIndex ) const
{
	bool result = false;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	ShapePath *path = (ShapePath *)sender.GetUserdata();
	if ( ! path ) { return result; }

    int index = GetHash( L )->Lookup( key );
    switch ( index )
    {
        case 0:
            {
                TesselatorCircle *tesselator = static_cast< TesselatorCircle * >( path->GetTesselator() );

                if ( tesselator )
                {
                    Real radius = luaL_toreal( L, valueIndex );
                    tesselator->SetRadius( radius );
                    path->Invalidate( ClosedPath::kFillSource | ClosedPath::kStrokeSource );
                    path->GetObserver()->Invalidate( DisplayObject::kGeometryFlag | DisplayObject::kStageBoundsFlag );
                    result = true;
                }
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

