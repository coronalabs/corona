//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ShaderResource.h"

#include "Display/Rtt_ShaderData.h"
#include "Renderer/Rtt_Program.h"

#include "Display/Rtt_Display.h"
#include "CoronaLua.h"
#include "CoronaGraphics.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

bool
TimeTransform::Apply( Uniform *time, Real *old, U32 now )
{
	if (NULL != func && NULL != time)
	{
		if (timestamp != now)
		{
			timestamp = now;

			time->GetValue(cached);

			if (NULL != old)
			{
				*old = cached;
			}

			func( &cached, arg1, arg2, arg3 );
		}

		time->SetValue(cached);

		return true;
	}

	return false;
}

static void
Modulo( Real *x, Real range, Real, Real )
{
    *x = fmod( *x, range ); // TODO?: Rtt_RealFmod
}

static void
PingPong( Real *x, Real range, Real, Real )
{
    Real pos = fmod( *x, Rtt_REAL_2 * range ); // TODO?: Rtt_RealFmod

    if (pos > range)
    {
        pos = Rtt_REAL_2 * range - pos;
    }

    *x = pos;
}

static void
Sine( Real *x, Real amplitude, Real speed, Real shift )
{
    Real t = *x;

    *x = amplitude * Rtt_RealSin( speed * t + shift );
}

int
TimeTransform::Push( lua_State *L ) const
{
    if ( func )
    {
        lua_newtable( L );

        if ( &Modulo == func || &PingPong == func )
        {
            lua_pushstring( L, &Modulo == func ? "modulo" : "pingpong" );
            lua_setfield( L, -2, "func" );
            lua_pushnumber( L, arg1 );
            lua_setfield( L, -2, "range" );
        }

        else if ( &Sine == func )
        {
            lua_pushliteral( L, "sine" );
            lua_setfield( L, -2, "func" );
            lua_pushnumber( L, arg1 );
            lua_setfield( L, -2, "amplitude" );
            lua_pushnumber( L, (Rtt_REAL_2 * M_PI) / arg2 );
            lua_setfield( L, -2, "period" );
            lua_pushnumber( L, arg3 );
            lua_setfield( L, -2, "shift" );
        }

        else
        {
            Rtt_ASSERT_NOT_REACHED();

            return 0;
        }
    }

    else
    {
        lua_pushnil( L );
    }

    return 1;
}

static void
GetNumberArg( lua_State * L, int arg, Real * value, const char * func, const char * name, const char * what )
{
    lua_getfield( L, arg, name ); // ..., xform, ..., value?
        
    if (!lua_isnil( L, -1 ))
    {
        if (lua_isnumber( L, -1 ))
        {
            *value = (Real)lua_tonumber( L, -1 );
        }

        else
        {
            CoronaLuaWarning( L, "%s ignoring invalid '%s' parameter for %s time transform (expected number but got %s)",
                        what, name, func, lua_typename( L, lua_type( L, -1 ) ) );
        }
    }

    lua_pop( L, 1 ); // ..., xform, ...
}

static void
GetPositiveNumberArg( lua_State * L, int arg, Real * value, const char * func, const char * name, const char * what )
{
    Real old = *value;

    GetNumberArg( L, arg, value, func, name, what );

    if (*value <= Rtt_REAL_0)
    {
        *value = old;

        CoronaLuaWarning( L, "%s ignoring invalid '%s' parameter for %s time transform (must be positive number)",
            what, name, func );
    }
}

void
TimeTransform::SetDefault()
{
    func = &PingPong;
    arg1 = Display::GetGpuSupportsHighPrecisionFragmentShaders() ? 3600 * 5 : 50;
    arg2 = arg3 = 0;
}

void
TimeTransform::SetFunc( lua_State *L, int arg, const char *what, const char *fname )
{
    switch (*fname)
    {
    case 'm': // modulo
    case 'p': // pingpong
        {
            Real range = Rtt_REAL_1;
                
            GetPositiveNumberArg( L, arg, &range, fname, "range", what );

            bool isModulo = 'm' == *fname;

            func = isModulo ? &Modulo : &PingPong;
            arg1 = range;
        }
        break;

    case 's': // sine
        {
            Real amplitude = Rtt_REAL_1, period = Rtt_REAL_2 * M_PI, shift = Rtt_REAL_0;

            GetNumberArg( L, arg, &amplitude, fname, "amplitude", what );
            GetPositiveNumberArg( L, arg, &period, fname, "period", what );
            GetNumberArg( L, arg, &shift, fname, "shift", what );

            func = &Sine;
            arg1 = amplitude;
            arg2 = (Rtt_REAL_2 * M_PI) / period;
            arg3 = shift;
        }
        break;

    default:
        Rtt_ASSERT_NOT_REACHED();
    }
}

bool
TimeTransform::Matches( const TimeTransform *xform1, const TimeTransform *xform2 )
{
	if (xform1 == xform2)
	{
		return true;
	}

	else if (NULL == xform1 || NULL == xform2)
	{
		return false;
	}

	else
	{
		return xform1->func == xform2->func && xform1->arg1 == xform2->arg1 && xform1->arg2 == xform2->arg2 && xform1->arg3 == xform2->arg3;
	}
}

const char*
TimeTransform::FindFunc( lua_State *L, int arg, const char *what )
{
	const char *fname = NULL;

    lua_getfield( L, arg, "func" );    // ..., xform, ..., func

    if (lua_isstring( L, -1 ))
    {
        fname = lua_tostring( L, -1 );

        bool isValid = strcmp( fname, "modulo" ) == 0 ||
                        strcmp( fname, "pingpong" ) == 0 ||
                        strcmp( fname, "sine" ) == 0;
            
		if ( !isValid )
        {
            CoronaLuaWarning( L, "%s ignoring unknown %s time transform", what, fname );

			fname = NULL;
        }
    }

    lua_pop( L, 1 ); // ..., xform, ...

	return fname;
}

ShaderResource::ShaderResource( Program *program, ShaderTypes::Category category )
:	fCategory( category ),
	fName(),
	fVertexDataMap(),
	fUniformDataMap(),
	fDefaultData( NULL ),
    fEffectCallbacks( NULL ),
    fDetailNames( NULL ),
    fDetailValues( NULL ),
    fDetailsCount( 0U ),
    fShellTransform( NULL ),
	fTimeTransform( NULL ),
	fUsesUniforms( false ),
	fUsesTime( false )
{
	Init(program);
}

ShaderResource::ShaderResource( Program *program, ShaderTypes::Category category, const char *name )
:	fCategory( category ),
	fName( name ),
	fVertexDataMap(),
	fUniformDataMap(),
	fDefaultData( NULL ),
    fEffectCallbacks( NULL ),
    fDetailNames( NULL ),
    fDetailValues( NULL ),
    fDetailsCount( 0U ),
    fShellTransform( NULL ),
	fTimeTransform( NULL ),
	fUsesUniforms( false ),
	fUsesTime( false )
{
	Init(program);
}

void
ShaderResource::Init(Program *defaultProgram)
{
	for (int i = 0; i < kNumProgramMods; i++)
	{
		fPrograms[i] = NULL;
	}
	fPrograms[ShaderResource::kDefault] = defaultProgram;

	defaultProgram->SetShaderResource( this );
}

ShaderResource::~ShaderResource()
{
	// TODO: We will need to queuerelease of this once we move away from a prototype-based cloning in ShaderFactory
	for (int i = 0; i < kNumProgramMods; i++)
	{
		Rtt_DELETE(fPrograms[i]);
	}
    
    if ( NULL != fDefaultData )
	{
		Rtt_DELETE( fDefaultData );
    }

	if ( NULL != fTimeTransform )
	{
		Rtt_DELETE( fTimeTransform );
	}

    SetEffectCallbacks( NULL );
    SetShellTransform( NULL );
}

void
ShaderResource::AddEffectDetail( const char * name, const char * value )
{
    fDetailNames.push_back( name );
    fDetailValues.push_back( value );
}

int
ShaderResource::GetEffectDetail( int index, CoronaEffectDetail & detail ) const
{
    if (index >= 0 && index < fDetailNames.size() )
    {
        detail.name = fDetailNames[index].c_str();
        detail.value = fDetailValues[index].c_str();

        return 1;
    }

    return 0;
}

void
ShaderResource::SetProgramMod(ProgramMod mod, Program *program)
{
	
	if ( Rtt_VERIFY(NULL == fPrograms[mod]) )
	{
		fPrograms[mod] = program;

		program->SetShaderResource( this );
	}
}

Program *
ShaderResource::GetProgramMod(ProgramMod mod) const
{
	return fPrograms[mod];
}

void
ShaderResource::SetEffectCallbacks( CoronaEffectCallbacks * callbacks )
{
    if (NULL != fEffectCallbacks)
    {
        Rtt_DELETE( fEffectCallbacks );
        
        fEffectCallbacks = NULL;
    }
    
    if (NULL != callbacks) // clone to allow removal of original
    {
        fEffectCallbacks = Rtt_NEW( NULL, CoronaEffectCallbacks( *callbacks ) );
    }
}

void
ShaderResource::SetShellTransform( CoronaShellTransform * shellTransform )
{
    if (NULL != fShellTransform)
    {
        Rtt_DELETE( fShellTransform );
        
        fShellTransform = NULL;
    }
    
    if (NULL != shellTransform) // clone to allow removal of original
    {
        fShellTransform = Rtt_NEW( NULL, CoronaShellTransform( *shellTransform ) );
    }
}

int
ShaderResource::GetDataIndex( const char *key ) const
{
	int result = -1;

	if ( Rtt_VERIFY( key ) )
	{
		if ( UsesUniforms() )
		{
			std::string k( key );
			UniformDataMap::const_iterator element = fUniformDataMap.find( k );
			if ( element != fUniformDataMap.end() )
			{
				result = element->second.index;
			}
		}
		else
		{
			std::string k( key );
			VertexDataMap::const_iterator element = fVertexDataMap.find( k );
			if ( element != fVertexDataMap.end() )
			{
				result = element->second;
			}
		}
	}

    if (-1 == result && fEffectCallbacks && fEffectCallbacks->getDataIndex)
    {
        result = fEffectCallbacks->getDataIndex( key );

        if (result >= 0)
        {
            result += ShaderData::kNumData;
        }
    }

	return result;
}

ShaderResource::UniformData
ShaderResource::GetUniformData( const char *key ) const
{
	UniformData result = { -1, Uniform::kScalar };

	if ( Rtt_VERIFY( UsesUniforms() ) )
	{
		std::string k( key );
		UniformDataMap::const_iterator element = fUniformDataMap.find( k );
		if ( element != fUniformDataMap.end() )
		{
			result = element->second;
		}
	}

	return result;
}

void
ShaderResource::SetDefaultData( ShaderData *defaultData )
{
	if ( defaultData != fDefaultData )
	{
		Rtt_DELETE( fDefaultData );
		fDefaultData = defaultData;
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

