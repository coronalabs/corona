//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_PaintAdapter.h"

#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_DisplayPath.h"
#include "Display/Rtt_Paint.h"
#include "Core/Rtt_StringHash.h"
#include "Rtt_LuaContext.h"

#include "Display/Rtt_Display.h"
#include "Display/Rtt_ShaderData.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Display/Rtt_StageObject.h"

#include "Display/Rtt_ShaderName.h"

#ifdef Rtt_DEBUG
    #include "Display/Rtt_ClosedPath.h"
    #include "Display/Rtt_ShaderDataAdapter.h"
#endif

#include <cstring>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const PaintAdapter&
PaintAdapter::Constant()
{
    static const PaintAdapter sAdapter;
    return sAdapter;
}

// ----------------------------------------------------------------------------

PaintAdapter::PaintAdapter()
{
}

StringHash *
PaintAdapter::GetHash( lua_State *L ) const
{
    static const char *keys[] =
    {
        "r",                // 0
        "g",                // 1
        "b",                // 2
        "a",                // 3
        "effect",            // 4
        "blendMode",        // 5
        "blendEquation",    // 6
    };

    static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 7, 0, 1, __FILE__, __LINE__ );
    return &sHash;
}

int
PaintAdapter::ValueForKey(
    const LuaUserdataProxy& sender,
    lua_State *L,
    const char *key ) const
{
    int result = 0;

    Rtt_ASSERT( key ); // Caller should check at the top-most level

    const Paint *paint = (const Paint *)sender.GetUserdata();
    if ( ! paint ) { return result; }

    int index = GetHash( L )->Lookup( key );
    if ( index < 0 ) { return result; }

    if ( index < 4 )
    {
        const float kInv255 = 1.f / 255.f;
    
        ColorUnion c;
        c.pixel = paint->GetColor();
        switch ( index )
        {
            case 0:    lua_pushnumber( L, kInv255 * c.rgba.r );    break;
            case 1:    lua_pushnumber( L, kInv255 * c.rgba.g );    break;
            case 2:    lua_pushnumber( L, kInv255 * c.rgba.b );    break;
            case 3:    lua_pushnumber( L, kInv255 * c.rgba.a );    break;
            default:    break;
        }
        result = 1;
    }
    else
    {
        switch ( index )
        {
            case 4:
                // TODO: For extra protection, this should be protected by Display::ShouldRestrict()
                // We get it for free right now b/c access to paint properties are restricted
                paint->PushShader( L );
                result = 1;
                break;
            case 5:
                {
                    // TODO: If it's kCustom, then need to send back table
                    RenderTypes::BlendType blend = paint->GetBlend();
                    lua_pushstring( L, RenderTypes::StringForBlendType( blend ) );
                    result = 1;
                }
                break;
            case 6:
                // TODO: For extra protection, this should be protected by Display::ShouldRestrict()
                // We get it for free right now b/c access to paint properties are restricted
                {
                    RenderTypes::BlendEquation eq = paint->GetBlendEquation();
                    lua_pushstring( L, RenderTypes::StringForBlendEquation( eq ) );
                    result = 1;
                }
                break;
            default:
                Rtt_ASSERT_NOT_REACHED();
                break;
        }
    }

    return result;
}

bool
PaintAdapter::SetValueForKey(
    LuaUserdataProxy& sender,
    lua_State *L,
    const char *key,
    int valueIndex ) const
{
    bool result = false;

    Rtt_ASSERT( key ); // Caller should check at the top-most level

    Paint *paint = (Paint *)sender.GetUserdata();
    if ( ! paint ) { return result; }

    int index = GetHash( L )->Lookup( key );
    if ( index < 0 ) { return result; }

    if ( index < 4 )
    {
        float v = lua_tonumber( L, valueIndex );
        v = Clamp( v, 0.f, 1.f );
        U8 value = 255.f * v;

        ColorUnion c;
        c.pixel = ( paint->GetColor() );

        switch ( index )
        {
            case 0:    c.rgba.r = value;    break;
            case 1:    c.rgba.g = value;    break;
            case 2:    c.rgba.b = value;    break;
            case 3:    c.rgba.a = value;    break;
            default:    break;
        }

        paint->SetColor( c.pixel );
        result = true;
    }
    else
    {
        switch ( index )
        {
            case 4:
                // TODO: For extra protection, this should be protected by Display::ShouldRestrict()
                // We get it for free right now b/c access to paint properties are restricted
                {
                    Shader *shader = NULL;
                    Geometry *geometry = NULL;

                    // Ensure paint has a parent display object
                    DisplayObject *observer = paint->GetObserver();
                    if ( Rtt_VERIFY( observer ) )
                    {
                        const char *s = NULL;
                        
                        // Avoid conversions to string
                        if ( LUA_TSTRING == lua_type( L, valueIndex ) )
                        {
                            s = lua_tostring( L, valueIndex );
                        }
                        
                        if ( s )
                        {
                            ShaderName namedShader( s );

                            // Use NULL for default shader
                            if ( namedShader.GetCategory() != ShaderTypes::kCategoryDefault )
                            {
                                StageObject *stage = observer->GetStage();
                                Display& display = stage->GetDisplay();
                                ShaderFactory& factory = display.GetShaderFactory();
                                shader = factory.FindOrLoad( namedShader );

                                bool isFill = DisplayPath::ExtensionAdapter::IsFillPaint( observer, paint );
                                
                                geometry = DisplayPath::ExtensionAdapter::GetGeometry( observer, isFill );
                            }
                        }
                    }

                    if (shader && !shader->IsCompatible( geometry ))
                    {
                        result = false;
                    }
                    
                    else
                    {
                        paint->SetShader( shader );

                        result = true;
                    }
                }
                break;
            case 5:
                {
                    int valueType = lua_type( L, valueIndex );
                    if ( LUA_TSTRING == valueType )
                    {
                        const char *v = lua_tostring( L, valueIndex );
                        RenderTypes::BlendType blend = RenderTypes::BlendTypeForString( v );
                        paint->SetBlend( blend );
                    }
                    else if ( LUA_TTABLE == valueType )
                    {
                        const char *tmpStr;

                        // Required values
                        lua_getfield( L, valueIndex, "srcColor" );
                        tmpStr = lua_tostring( L, -1 );
                        BlendMode::Param srcColor = BlendMode::ParamForString( tmpStr );
                        lua_pop( L, 1 );

                        lua_getfield( L, valueIndex, "dstColor" );
                        tmpStr = lua_tostring( L, -1 );
                        BlendMode::Param dstColor = BlendMode::ParamForString( tmpStr );
                        lua_pop( L, 1 );

                        // Optional values
                        lua_getfield( L, valueIndex, "srcAlpha" );
                        tmpStr = lua_tostring( L, -1 );
                        BlendMode::Param srcAlpha = BlendMode::ParamForString( tmpStr );
                        if ( BlendMode::kUnknown == srcAlpha ) { srcAlpha = srcColor; }
                        lua_pop( L, 1 );

                        lua_getfield( L, valueIndex, "dstAlpha" );
                        tmpStr = lua_tostring( L, -1 );
                        BlendMode::Param dstAlpha = BlendMode::ParamForString( tmpStr );
                        if ( BlendMode::kUnknown == dstAlpha ) { dstAlpha = dstColor; }
                        lua_pop( L, 1 );

                        if ( BlendMode::kUnknown != srcColor
                             && BlendMode::kUnknown != dstColor )
                        {
                            BlendMode blendMode( srcColor, dstColor, srcAlpha, dstAlpha );
                            paint->SetBlend( blendMode );
                        }
                        else
                        {
                            Rtt_TRACE_SIM( ( "ERROR: paint.blendMode could not be set b/c the provided table had invalid values for the 'srcColor' and/or 'dstColor' keys.\n" ) );
                        }
                    }
                }
                break;
            case 6:
                // TODO: For extra protection, this should be protected by Display::ShouldRestrict()
                // We get it for free right now b/c access to paint properties are restricted
                {
                    const char *v = lua_tostring( L, valueIndex );
                    RenderTypes::BlendEquation blendEquation = RenderTypes::BlendEquationForString( v );
                    paint->SetBlendEquation( blendEquation );
                }
                break;
            default:
                Rtt_ASSERT_NOT_REACHED();
                break;
        }
    }

    return result;
}

void
PaintAdapter::WillFinalize( LuaUserdataProxy& sender ) const
{
    Paint *paint = (Paint *)sender.GetUserdata();
    if ( ! paint ) { return; }
    
    paint->DetachProxy();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

