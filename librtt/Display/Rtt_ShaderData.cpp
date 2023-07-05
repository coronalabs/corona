//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ShaderData.h"

#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_DisplayPath.h"
#include "Display/Rtt_Paint.h"
#include "Display/Rtt_ShaderDataAdapter.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Display/Rtt_ShaderResource.h"
#include "Rtt_LuaAux.h"
#include "Rtt_LuaUserdataProxy.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ShaderData::ShaderData( const WeakPtr< ShaderResource >& resource )
:	fProxy( NULL ),
	fShaderResource( resource ),
	fOwner( NULL )
{
	memset( fVertexData, 0, sizeof( fVertexData ) );
	memset( fUniformData, 0, sizeof( fUniformData ) );
}

ShaderData::~ShaderData()
{
	if ( fProxy )
	{
		fProxy->DetachUserdata();
	}

	for ( int i = 0; i < kNumData; i++ )
	{
		Rtt_DELETE( fUniformData[i] );
	}
}

void
ShaderData::SetOwner( const Shader *shader )
{
	// Should only be called once, under current assumptions
	if ( Rtt_VERIFY( NULL == fOwner ) )
	{
		fOwner = shader;
	}
}

void
ShaderData::QueueRelease( DisplayObject *observer )
{
	for ( int i = 0; i < kNumData; i++ )
	{
		observer->QueueRelease( fUniformData[i] );

		fUniformData[i] = NULL;
	}
	observer->QueueRelease( fProxy );
}

ShaderData *
ShaderData::Clone( Rtt_Allocator *allocator )
{
	ShaderData *result = Rtt_NEW( allocator, ShaderData( fShaderResource ) );

	SharedPtr< ShaderResource > resource( fShaderResource );
	if ( resource.NotNull() )
	{
		if ( resource->UsesUniforms() )
		{
			for ( int i = 0; i < kNumData; i++ )
			{
				Uniform *src = fUniformData[i];
				if ( src )
				{
					Uniform *dst = result->InitializeUniform( allocator, (DataIndex)i, src->GetDataType() );
					memcpy( dst->GetData(), src->GetData(), src->GetSizeInBytes() );
				}
			}
		}
		else
		{
			memcpy( result->fVertexData, fVertexData, sizeof( fVertexData ) );
		}
	}

	return result;
}

Uniform *
ShaderData::InitializeUniform(
	Rtt_Allocator *allocator,
	DataIndex index,
	Uniform::DataType dataType )
{
	Uniform *result = NULL;

	if ( Rtt_VERIFY( NULL == GetUniform( index ) )
		 && Rtt_VERIFY( index >= kData0 && index < kNumData ) )
	{
		result = Rtt_NEW( allocator, Uniform( allocator, dataType ) );
		fUniformData[index] = result;
	}

	Paint *paint = GetPaint();
	if ( paint )
	{
		paint->Invalidate( Paint::kShaderUniformDataFlag );
		DisplayObject *observer = paint->GetObserver();
		if ( observer )
		{
			observer->Invalidate( DisplayObject::kPaintFlag );
		}
	}

	return result;
}

Paint *
ShaderData::GetPaint() const
{
	return fOwner ? fOwner->GetPaint() : NULL;
}

void
ShaderData::PushProxy( lua_State *L )
{
	if ( ! fProxy )
	{
		fProxy = LuaUserdataProxy::New( L, const_cast< Self * >( this ) );
		fProxy->SetAdapter( & ShaderDataAdapter::Constant() );
	}

	fProxy->Push( L );
}

void
ShaderData::DetachProxy()
{
	fProxy = NULL;
}

Real
ShaderData::GetVertexData( DataIndex index ) const
{
	Rtt_ASSERT( index < kNumData ); // Verify bounds

	index = Min( Max( index, kDataMin ), kDataMax ); // Ensure bounds

	return fVertexData[index];
}

void
ShaderData::CopyVertexData( Real& a, Real& b, Real& c, Real& d ) const
{
	a = fVertexData[kData0];
	b = fVertexData[kData1];
	c = fVertexData[kData2];
	d = fVertexData[kData3];
}

void
ShaderData::SetVertexData( DataIndex index, Real newValue )
{
	Rtt_ASSERT( index < kNumData ); // Verify bounds

	fVertexData[index] = newValue;
	
	Paint *paint = GetPaint();
	if ( paint )
	{
		paint->Invalidate( Paint::kShaderVertexDataFlag );
		DisplayObject *observer = paint->GetObserver();
		if ( observer )
		{
			observer->Invalidate( DisplayObject::kColorFlag );
		}
	}
}

static void
PushArray( lua_State *L, U8 *data, int numValues )
{
	lua_createtable( L, numValues, 0 );

	Real *values = (Real*)data;
	for ( int i = 0; i < numValues; i++ )
	{
		float v = Rtt_RealToFloat( values[i] );
		lua_pushnumber( L, v );
		lua_rawseti( L, -2, i+1 );
	}
}

int
ShaderData::PushUniform( lua_State *L, DataIndex dataIndex ) const
{
	int result = 0;

	Uniform *uniform = GetUniform( dataIndex );
	if ( uniform )
	{
		U8 *data = uniform->GetData();
		int numValues = uniform->GetNumValues();
		if ( numValues > 1 )
		{
			PushArray( L, data, numValues );
		}
		else
		{
			Real *values = (Real*)data;
			float v = Rtt_RealToFloat( *values );
			lua_pushnumber( L, v );
		}
		result = 1;
	}

	return result;
}

static void
AssignArray( lua_State *L, U8 *data, int numValues, int valueIndex )
{
	Real *values = (Real*)data;

	valueIndex = Lua::Normalize( L, valueIndex );
	if ( lua_istable( L, valueIndex ) )
	{
		int tableLen = (int) lua_objlen( L, valueIndex );

		for ( int i = 0; i < numValues; i++ )
		{
			if ( i < tableLen )
			{
				lua_rawgeti( L, valueIndex, (i+1) ); // Lua is 1-based
				{
					Real v = luaL_toreal( L, -1 );
					values[i] = v;
				}
				lua_pop( L, 1 );
			}
			else
			{
				values[i] = Rtt_REAL_0;
			}
		}
	}
	else
	{
		Real v = luaL_toreal( L, valueIndex );
		for ( int i = 0; i < numValues; i++ )
		{
			values[i] = v;
		}
	}
}

void
ShaderData::SetUniform( lua_State *L, int valueIndex, DataIndex dataIndex )
{
	Uniform *uniform = GetUniform( dataIndex );

	// Caller responsible for init'ing Uniform via InitializeUniform()
	if ( Rtt_VERIFY( uniform ) )
	{
		U8 *data = uniform->GetData();
		int numValues = uniform->GetNumValues();
		if ( numValues > 1 )
		{
			AssignArray( L, data, numValues, valueIndex );
		}
		else
		{
			Real v = luaL_toreal( L, valueIndex );
			Real *values = (Real*)data;
			*values = v;
		}

		DidUpdateUniform( dataIndex );
	}
}

Uniform *
ShaderData::GetUniform( DataIndex index ) const
{
	Rtt_ASSERT( index < kNumData ); // Verify bounds

	return fUniformData[index];
}

void
ShaderData::DidUpdateUniform( DataIndex index )
{
	Uniform *uniform = GetUniform( index );
	uniform->Invalidate();

	Paint *paint = GetPaint();
	if ( paint )
	{
		DisplayObject *observer = paint->GetObserver();
		if ( observer )
		{
			observer->InvalidateDisplay(); // force reblit
		}
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

