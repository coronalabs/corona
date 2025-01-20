//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_Uniform.h"

#include "Core/Rtt_Assert.h"
#include <cstring>
#include <stddef.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

Uniform::DataType
Uniform::DataTypeForString( const char *value )
{
	static const char kScalarString[] = "scalar";
	static const char kVec2String[] = "vec2";
	static const char kVec3String[] = "vec3";
	static const char kVec4String[] = "vec4";
	static const char kMat3String[] = "mat3";
	static const char kMat4String[] = "mat4";

	Uniform::DataType result = kScalar; // Default
	if ( ! value ) { return result; }

	if ( 0 == strcmp( kScalarString, value ) )
	{
		result = kScalar;
	}
	else if ( 0 == strcmp( kVec2String, value ) )
	{
		result = kVec2;
	}
	else if ( 0 == strcmp( kVec3String, value ) )
	{
		result = kVec3;
	}
	else if ( 0 == strcmp( kVec4String, value ) )
	{
		result = kVec4;
	}
	else if ( 0 == strcmp( kMat3String, value ) )
	{
		result = kMat3;
	}
	else if ( 0 == strcmp( kMat4String, value ) )
	{
		result = kMat4;
	}

	return result;
}

// ----------------------------------------------------------------------------

Uniform::Uniform( Rtt_Allocator* allocator, DataType type )
:	CPUResource( allocator ),
	fType( type )
{
	Allocate();
}

Uniform::Uniform( DataType type )
:	CPUResource( NULL ),
	fType( type ),
	fData( NULL )
{
}

Uniform::~Uniform()
{
	Deallocate();
}

CPUResource::ResourceType Uniform::GetType() const
{
	return CPUResource::kUniform;
}

void
Uniform::Allocate()
{
	fData = new U8[GetSizeInBytes()];	
}

void
Uniform::Deallocate()
{
	delete [] fData;
	fData = NULL;
}

U32
Uniform::GetNumValues() const
{
	switch( fType )
	{
		case kScalar:	return 1;
		case kVec2:		return 2;
		case kVec3:		return 3;
		case kVec4:		return 4;
		case kMat3:		return 9;
		case kMat4:		return 16;
		default:		Rtt_ASSERT_NOT_REACHED();
	}
	return 0;
}

U32
Uniform::GetSizeInBytes() const
{
	switch( fType )
	{
		case kScalar:	return sizeof( Real );
		case kVec2:		return sizeof( Real ) * 2;
		case kVec3:		return sizeof( Real ) * 3;
		case kVec4:		return sizeof( Real ) * 4;
		case kMat3:		return sizeof( Real ) * 9;
		case kMat4:		return sizeof( Real ) * 16;
		default:		Rtt_ASSERT_NOT_REACHED();
	}
	return 0;
}

Uniform::DataType
Uniform::GetDataType() const
{
	return fType;
}

void
Uniform::GetValue( Real& x )
{
	Rtt_ASSERT( fType == kScalar );
	x = reinterpret_cast<Real*>( fData )[0];
}

void
Uniform::GetValue( Real& x, Real& y )
{
	Rtt_ASSERT( fType == kVec2 );
	Real* realData = reinterpret_cast<Real*>( fData );
	x = realData[0];
	y = realData[1];
}

void
Uniform::GetValue( Real& x, Real& y, Real& z )
{
	Rtt_ASSERT( fType == kVec3 );
	Real* realData = reinterpret_cast<Real*>( fData );
	x = realData[0];
	y = realData[1];
	z = realData[2];
}

void
Uniform::GetValue( Real& x, Real& y, Real& z, Real& w )
{
	Rtt_ASSERT( fType == kVec4 );
	Real* realData = reinterpret_cast<Real*>( fData );
	x = realData[0];
	y = realData[1];
	z = realData[2];
	w = realData[3];
}

void
Uniform::SetValue( Real x )
{
	Rtt_ASSERT( fType == kScalar );
	reinterpret_cast<Real*>( fData )[0] = x;
}

void
Uniform::SetValue( Real x, Real y )
{
	Rtt_ASSERT( fType == kVec2 );
	Real* realData = reinterpret_cast<Real*>( fData );
	realData[0] = x;
	realData[1] = y;
}

void
Uniform::SetValue( Real x, Real y, Real z )
{
	Rtt_ASSERT( fType == kVec3 );
	Real* realData = reinterpret_cast<Real*>( fData );
	realData[0] = x;
	realData[1] = y;
	realData[2] = z;
}

void
Uniform::SetValue( Real x, Real y, Real z, Real w )
{
	Rtt_ASSERT( fType == kVec4 );
	Real* realData = reinterpret_cast<Real*>( fData );
	realData[0] = x;
	realData[1] = y;
	realData[2] = z;
	realData[3] = w;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
