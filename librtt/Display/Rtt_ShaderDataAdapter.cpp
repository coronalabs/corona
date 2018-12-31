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

