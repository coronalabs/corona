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

static Real
Modulo( Real x, Real range )
{
    return fmod( x, range );
}

static Real
PingPong( Real x, Real range )
{
    Real pos = fmod( x, Rtt_REAL_2 * range );

    if (pos > range)
    {
        pos = Rtt_REAL_2 * range - pos;
    }

    return pos;
}

static Real
Sine( Real x, Real amplitude, Real speed, Real phase )
{
    return amplitude * Rtt_RealSin( speed * x + phase );
}

static Real
SmootherStep( Real y1, Real y2, Real t )
{
	Real t3 = t * t * t;
	Real factor = t3 * ( 10. - t * ( 15. - t * 6. ) );
	
	return y1 + (y2 - y1) * factor;
}

static Real
SmoothedModulo( Real x, Real range, Real extra )
{
	Real rem = fmod( x, range + extra );
	
	if ( rem < range )
	{
		return rem;
	}
	else
	{
		return SmootherStep( range, 0., ( rem - range ) / extra );
	}
}

static Real
Noise_Lissajous( Real x )
{
	return sin( M_PI * x ) + sin( 2. * x );
}

static Real
Noise_R2( Real x, Real range )
{
	// See from Martin Roberts "Extreme Learning", in particular "Jittering quasirandom point sets":
	// https://web.archive.org/web/20260228140726/https://extremelearning.com.au/a-simple-method-to-construct-isotropic-quasirandom-blue-noise-point-sequences/

	const double K = 1.0 / 1.324717957244746; /* 1 / plastic constant */

	Real ip, fp = modf( x / range, &ip );
	Real raw = ip * K;
	Real y1 = raw - floor( raw );
	Real y2 = ( raw + K ) - floor( raw + K );
	
	return SmootherStep( y1, y2, fp );
}

Real
TimeTransform::Apply( Real value ) const
{
	switch ( fMethod )
	{
	case kNone:
		return value;
	case kModulo:
		return Modulo( value, fArg1 );
	case kPingPong:
		return PingPong( value, fArg1 );
	case kSine:
		return Sine( value, fArg1, fArg2, fArg3 );
	case kSmoothedModulo:
		return SmoothedModulo( value, fArg1, fArg2 );
	case kNoise_Lissajous:
		return Noise_Lissajous( value );
	case kNoise_R2:
		return Noise_R2( value, fArg1 );
	default:
		Rtt_ASSERT_NOT_REACHED();
		return -1;
	}
}

int
TimeTransform::Push( lua_State *L ) const
{
	const char *name = StringForMethod( fMethod );
	
	Rtt_ASSERT( name );

	lua_newtable( L );
	lua_pushstring( L, name );
	lua_setfield( L, -2, "func" );

	switch ( fMethod )
	{
	case kNoise_Lissajous:
		break;
	case kModulo:
	case kPingPong:
	case kNoise_R2:
	case kSmoothedModulo:
		lua_pushnumber( L, fArg1 );
		lua_setfield( L, -2, "range" );
	   
		if ( kSmoothedModulo == fMethod )
		{
			lua_pushnumber( L, fArg2 );
			lua_setfield( L, -2, "smooth" );
		}
		
		break;
	case kSine:
		lua_pushnumber( L, fArg1 );
		lua_setfield( L, -2, "amplitude" );
		lua_pushnumber( L, (Rtt_REAL_2 * M_PI) / fArg2 );
		lua_setfield( L, -2, "period" );
		lua_pushnumber( L, fArg3 );
		lua_setfield( L, -2, "phase" );
		
		break;
	default:
		Rtt_ASSERT_NOT_REACHED();

		return 0;
	}
        
    return 1;
}

static bool
GetNumberArg( lua_State * L, int arg, Real * value, TimeTransform::Method method, const char * name, const char * what )
{
	bool ok = true;

    lua_getfield( L, arg, name ); // ..., xform, ..., value?
        
	if ( lua_isnumber( L, -1 ) )
	{
		*value = (Real)lua_tonumber( L, -1 );
	}

	else if ( !lua_isnil( L, -1 ) )
	{
		CoronaLuaWarning( L, "%s ignoring invalid '%s' parameter for %s time transform (expected number but got %s)",
					what, name, TimeTransform::StringForMethod( method ), lua_typename( L, lua_type( L, -1 ) ) );
	
		ok = false;
	}

    lua_pop( L, 1 ); // ..., xform, ...
    
    return ok;
}

static bool
GetPositiveNumberArg( lua_State * L, int arg, Real * value, TimeTransform::Method method, const char * name, const char * what )
{
    if( GetNumberArg( L, arg, value, method, name, what ) && ( *value > Rtt_REAL_0 ) )
    {
		return true;
	}
	else
    {
        CoronaLuaWarning( L, "%s ignoring invalid '%s' parameter for %s time transform (must be positive number)",
            what, name, TimeTransform::StringForMethod( method ) );
            
		return false;
    }
}

bool
TimeTransform::Matches( const TimeTransform *other ) const
{
	if ( NULL != other )
	{
		return 0 == memcmp( this, other, sizeof(TimeTransform) );
	}
	else
	{	
		return kNone == fMethod;
	}
}

void
TimeTransform::SetMethod( lua_State *L, int arg, const char *what, Method method )
{
	*this = TimeTransform(); // reset to wipe args and account for errors

    switch ( method )
    {
    case kModulo:
    case kPingPong:
    case kNoise_R2:
        {
            Real range = Rtt_REAL_1;

            if( GetPositiveNumberArg( L, arg, &range, method, "range", what ) )
            {
				fMethod = method;
				fArg1 = range;
			}
        }
        break;
    case kSine:
        {
            Real amplitude = Rtt_REAL_1, period = Rtt_REAL_2 * M_PI, phase = Rtt_REAL_0;

            if( GetNumberArg( L, arg, &amplitude, method, "amplitude", what ) &&
				GetPositiveNumberArg( L, arg, &period, method, "period", what ) &&
				GetNumberArg( L, arg, &phase, method, "phase", what ) )
			{
				fMethod = method;
				fArg1 = amplitude;
				fArg2 = (Rtt_REAL_2 * M_PI) / period;
				fArg3 = phase;
			}
        }
        break;
	case kSmoothedModulo:
		{
            Real range = Rtt_REAL_1, smooth = Rtt_REAL_1;

            if( GetPositiveNumberArg( L, arg, &range, method, "range", what ) &&
				GetPositiveNumberArg( L, arg, &smooth, method, "smooth", what ) )
			{
				fMethod = method;
				fArg1 = range;
				fArg2 = smooth;
			}
        }
		break;

	case kNoise_Lissajous:
		fMethod = method;
		break;

    default:
        Rtt_ASSERT_NOT_REACHED();
    }
}

const static struct {
	TimeTransform::Method method;
	const char *name;
} kMethodPairs[] = {
	{ TimeTransform::kNone, "none" },
	{ TimeTransform::kModulo, "modulo" },
	{ TimeTransform::kPingPong, "pingpong" },
	{ TimeTransform::kSine, "sine" },
	{ TimeTransform::kSmoothedModulo, "smoothedModulo" },
	{ TimeTransform::kNoise_Lissajous, "lissajousNoise" },
	{ TimeTransform::kNoise_R2, "r2Noise" }
};

const char*
TimeTransform::StringForMethod( Method method )
{
	for ( auto && pair : kMethodPairs )
	{
		if ( pair.method == method )
		{
			return pair.name;
		}
	}
	
	Rtt_ASSERT_NOT_REACHED();
	
	return NULL;
}

TimeTransform::Method
TimeTransform::MethodForString( const char *name )
{
	for ( auto && pair : kMethodPairs )
	{
		if ( 0 == strcmp( name, pair.name ) )
		{
			return pair.method;
		}
	}
	
	return kNumMethodTypes;
}

TimeTransform::Method
TimeTransform::FindMethod( lua_State *L, int arg, const char *what )
{
	Method method = kNumMethodTypes;
	
	// n.b. "func" is documented, so keep in spite of method nomenclature

    lua_getfield( L, arg, "func" ); // ..., xform, ..., func

    if ( lua_isstring( L, -1 ) )
    {
		method = MethodForString( lua_tostring( L, -1 ) );
    }

    lua_pop( L, 1 ); // ..., xform, ...

	return method;
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
	fTimeTransform(),
	fUsesUniforms( false )
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
	fTimeTransform(),
	fUsesUniforms( false )
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

