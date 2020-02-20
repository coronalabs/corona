//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

