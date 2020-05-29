//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ShaderDataAdapter.h"

#include "Display/Rtt_Display.h"
#include "Display/Rtt_ShaderData.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Display/Rtt_ShaderResource.h"
#include "Renderer/Rtt_Uniform.h"
#include "Rtt_LuaContext.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const ShaderDataAdapter&
ShaderDataAdapter::Constant()
{
	static const ShaderDataAdapter sAdapter;
	return sAdapter;
}

// ----------------------------------------------------------------------------

ShaderDataAdapter::ShaderDataAdapter()
{
}

StringHash *
ShaderDataAdapter::GetHash( lua_State *L ) const
{
    static const char *keys[] =
    {
        "",
    };
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 0, 0, 0, __FILE__, __LINE__ );
	return &sHash;
}

int
ShaderDataAdapter::ValueForKey(
	const LuaUserdataProxy& sender,
	lua_State *L,
	const char *key ) const
{
	int result = 0;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	const ShaderData *object = (const ShaderData *)sender.GetUserdata();

	if ( ! object ) { return result; }

	SharedPtr< ShaderResource > shaderResource( object->GetShaderResource() );
	if ( shaderResource.IsNull() ) { return result; }

	bool usesUniforms = shaderResource->UsesUniforms();

	int index = shaderResource->GetDataIndex( key );
	if ( index >= 0 )
	{
		ShaderData::DataIndex dataIndex = (ShaderData::DataIndex)index;
		if ( usesUniforms )
		{
			result = object->PushUniform( L, dataIndex );
		}
		else
		{
			Real value = object->GetVertexData( dataIndex );
			lua_pushnumber( L, Rtt_RealToFloat( value ) );
			result = 1;
		}
	}

	return result;
}

bool
ShaderDataAdapter::SetValueForKey(
	LuaUserdataProxy& sender,
	lua_State *L,
	const char *key,
	int valueIndex ) const
{
	bool result = false;

	Rtt_ASSERT( key ); // Caller should check at the top-most level

	ShaderData *object = (ShaderData *)sender.GetUserdata();
	if ( ! object ) { return result; }

	SharedPtr< ShaderResource > shaderResource( object->GetShaderResource() );
	if ( shaderResource.IsNull() ) { return result; }

	bool usesUniforms = shaderResource->UsesUniforms();

	int index = (ShaderData::DataIndex)shaderResource->GetDataIndex( key );
	if ( index >= 0 )
	{
		ShaderData::DataIndex dataIndex = (ShaderData::DataIndex)index;
		if ( usesUniforms )
		{
			Uniform *uniform = object->GetUniform( dataIndex );
			if ( ! uniform )
			{
				// Create uniform
				ShaderResource::UniformData uniformData = shaderResource->GetUniformData( key );
				uniform = object->InitializeUniform(
					LuaContext::GetAllocator( L ), dataIndex, uniformData.dataType );
				
			}

			object->SetUniform( L, valueIndex, dataIndex );
		}
		else
		{
			Real value = Rtt_FloatToReal( (float) lua_tonumber( L, valueIndex ) );
			object->SetVertexData( dataIndex, value );
		}

		result = true;
	}

	return result;
}

void
ShaderDataAdapter::WillFinalize( LuaUserdataProxy& sender ) const
{
	ShaderData *data = (ShaderData *)sender.GetUserdata();
	if ( ! data ) { return; }
	
	data->DetachProxy();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

