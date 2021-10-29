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

