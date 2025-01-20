//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_Program.h"
#include "Core/Rtt_Config.h"
#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_Assert.h"
#if defined( Rtt_USE_PRECOMPILED_SHADERS )
	#include "Renderer/Rtt_ShaderBinaryVersions.h"
#endif

#include <cstring>
#include <stddef.h>
#include <stdio.h>

// ----------------------------------------------------------------------------

namespace /*anonymous*/
{
	void SetSource( char** destination, const char* source )
	{
		if ( ! source ) { return; }
	
		if( *destination )
		{
			delete [] *destination;
		}

		size_t length = strlen( source );
		*destination = new char[length + 1];
		strcpy( *destination, source );
	}
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const char *
Program::HeaderForLanguage( Language language, const ProgramHeader& headerData )
{
	static char header[ 256 ];
	headerData.CopyHeaderSource( language, header, sizeof( header ) );
	return header;
}

int
Program::CountLines( const char *str )
{
	int result = 0;

	if ( Rtt_VERIFY( str ) )
	{
		char ch;
		while ( (ch = *str++) != '\0' )
		{
			if ( '\n' == ch )
			{
				++result;
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------------

Program::Program( Rtt_Allocator* allocator )
:	CPUResource( allocator ),
	fVertexShaderSource( NULL ),
	fFragmentShaderSource( NULL ),
	fHeaderSource( NULL ),
	fVertexShellNumLines( 0 ),
	fFragmentShellNumLines( 0 ),
	fCompilerVerbose( false )
{
#if defined( Rtt_USE_PRECOMPILED_SHADERS )
	fCompiledShaders = NULL;
	if (allocator)
	{
		fCompiledShaders = Rtt_NEW(allocator, ShaderBinaryVersions(allocator));
	}
#endif
}

Program::~Program()
{
	Deallocate();

#if defined( Rtt_USE_PRECOMPILED_SHADERS )
	Rtt_DELETE(fCompiledShaders);
#endif
}

CPUResource::ResourceType 
Program::GetType() const
{
	return CPUResource::kProgram;
}

void 
Program::Allocate()
{
	// Intentionally empty. The number of bytes to allocate depends on the
	// shader source code, so allocation is done in Set(*)ShaderSource().
}

void 
Program::Deallocate()
{
	delete [] fHeaderSource;
	delete [] fVertexShaderSource;
	delete [] fFragmentShaderSource;

	fHeaderSource = NULL;
	fVertexShaderSource = NULL;
	fFragmentShaderSource = NULL;

#if defined( Rtt_USE_PRECOMPILED_SHADERS )
	if (fCompiledShaders)
	{
		fCompiledShaders->FreeAllShaders();
	}
#endif
}

void 
Program::SetVertexShaderSource( const char* source )
{
	SetSource( &fVertexShaderSource, source );
}

void 
Program::SetFragmentShaderSource( const char* source )
{
	SetSource( &fFragmentShaderSource, source );
}

const char * 
Program::GetVertexShaderSource() const
{
	return fVertexShaderSource;
}

const char * 
Program::GetFragmentShaderSource() const
{
	return fFragmentShaderSource;
}

void 
Program::SetHeaderSource( const char* source )
{
	SetSource( &fHeaderSource, source );
}

// ----------------------------------------------------------------------------

// Type strings
static const char kDefaultTypeString[] = "P_DEFAULT";
static const char kRandomTypeString[] = "P_RANDOM";
static const char kPositionTypeString[] = "P_POSITION";
static const char kNormalTypeString[] = "P_NORMAL";
static const char kUVTypeString[] = "P_UV";
static const char kColorTypeString[] = "P_COLOR";

// Precision strings
static const char kLowPrecisionString[] = "lowp";
static const char kMediumPrecisionString[] = "mediump";
static const char kHighPrecisionString[] = "highp";

// ----------------------------------------------------------------------------
/*
const char *
ProgramHeader::StringForType( Type value )
{
	const char *result = NULL;

	switch ( value )
	{
		case kDefaultType:
			result = kDefaultTypeString;
			break;
		case kRandomType:
			result = kRandomTypeString;
			break;
		case kPositionType:
			result = kPositionTypeString;
			break;
		case kNormalType:
			result = kNormalTypeString;
			break;
		case kUVType:
			result = kUVTypeString;
			break;
		case kColorType:
			result = kColorTypeString;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}
*/
ProgramHeader::Type
ProgramHeader::TypeForString( const char *value )
{
	Type result = kUnknownType;

	if ( Rtt_VERIFY( value ) )
	{
		if ( 0 == strcmp( kDefaultTypeString, value ) )
		{
			result = kDefaultType;
		}
		else if ( 0 == strcmp( kRandomTypeString, value ) )
		{
			result = kRandomType;
		}
		else if ( 0 == strcmp( kPositionTypeString, value ) )
		{
			result = kPositionType;
		}
		else if ( 0 == strcmp( kNormalTypeString, value ) )
		{
			result = kNormalType;
		}
		else if ( 0 == strcmp( kUVTypeString, value ) )
		{
			result = kUVType;
		}
		else if ( 0 == strcmp( kColorTypeString, value ) )
		{
			result = kColorType;
		}
	}

	return result;
}

const char *
ProgramHeader::StringForPrecision( Precision value )
{
	const char *result = NULL;

	switch ( value )
	{
		case kLowPrecision:
			result = kLowPrecisionString;
			break;
		case kMediumPrecision:
			result = kMediumPrecisionString;
			break;
		case kHighPrecision:
			result = kHighPrecisionString;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

ProgramHeader::Precision
ProgramHeader::PrecisionForString( const char *value )
{
	Precision result = kUnknownPrecision;

	if ( Rtt_VERIFY( value ) )
	{
		if ( 0 == strcmp( kLowPrecisionString, value ) )
		{
			result = kLowPrecision;
		}
		else if ( 0 == strcmp( kMediumPrecisionString, value ) )
		{
			result = kMediumPrecision;
		}
		else if ( 0 == strcmp( kHighPrecisionString, value ) )
		{
			result = kHighPrecision;
		}
	}

	return result;
}

// ----------------------------------------------------------------------------

ProgramHeader::ProgramHeader()
{
	// Defaults
	fPrecision[kDefaultType] = kHighPrecision;
	fPrecision[kRandomType] = kHighPrecision;
	fPrecision[kPositionType] = kMediumPrecision;
	fPrecision[kNormalType] = kMediumPrecision;
	fPrecision[kUVType] = kMediumPrecision;
	fPrecision[kColorType] = kLowPrecision;
}

static const char kOpenGL_2_1_Header[] =
	#ifdef Rtt_MAC_ENV
		"#version 120\n"
	#else
		// On Windows we allow GLSL 110, to allow users on older hardware to get
		// by with GL 2.0.  The rest of the header stays the same.  This isn't
		// technically "correct" since some shaders won't work, but it is better
		// than before (i.e. blank screens)
		"#version 110\n"
	#endif

		"\n"
		"#define lowp\n"
		"#define mediump\n"
		"#define highp\n"
		"\n"
		"#define P_DEFAULT\n"
		"#define P_RANDOM\n"
		"#define P_POSITION\n"
		"#define P_NORMAL\n"
		"#define P_UV\n"
		"#define P_COLOR\n"
		"\n";

static const char kVulkanGLSL_Header[] =
		"#version 450\n"
		"\n"
		"#define lowp\n"
		"#define mediump\n"
		"#define highp\n"
		"\n"
		"#define P_DEFAULT\n"
		"#define P_RANDOM\n"
		"#define P_POSITION\n"
		"#define P_NORMAL\n"
		"#define P_UV\n"
		"#define P_COLOR\n"
		"\n";

// Copy the header string to 'dst' up to (dstSize-1) chars.
// Always terminates '\0'
void
ProgramHeader::CopyHeaderSource( Program::Language language, char *dst, int dstSize ) const
{
	if ( dst )
	{
		if ( Program::kOpenGL_2_1 == language )
		{
			strncpy( dst, kOpenGL_2_1_Header, dstSize );
			dst[dstSize - 1] = '\0'; // strncpy does not '\0' terminate if src is larger than dst

			Rtt_ASSERT( sizeof( kOpenGL_2_1_Header ) < dstSize );
		}
		else if ( Program::kVulkanGLSL == language )
		{
			strncpy( dst, kVulkanGLSL_Header, dstSize );
			dst[dstSize - 1] = '\0'; // strncpy does not '\0' terminate if src is larger than dst

			Rtt_ASSERT( sizeof( kVulkanGLSL_Header ) < dstSize );
		}
		else
		{
			Rtt_ASSERT( Program::kOpenGL_ES_2 == language );

			int dstLen = snprintf( dst, dstSize,
				"\n"
				"#define P_DEFAULT	%s\n"
				"#define P_RANDOM	%s\n"
				"#define P_POSITION	%s\n"
				"#define P_NORMAL	%s\n"
				"#define P_UV		%s\n"
				"#define P_COLOR	%s\n"
				"\n",
				StringForPrecision( (Precision)fPrecision[kDefaultType] ),
				StringForPrecision( (Precision)fPrecision[kRandomType] ),
				StringForPrecision( (Precision)fPrecision[kPositionType] ),
				StringForPrecision( (Precision)fPrecision[kNormalType] ),
				StringForPrecision( (Precision)fPrecision[kUVType] ),
				StringForPrecision( (Precision)fPrecision[kColorType] ) );

			// Assume we should always have extra space, so no equals
			// snprintf doesn't include '\0'
			Rtt_ASSERT( (dstLen + 1) < dstSize ); Rtt_UNUSED( dstLen );
		}
	}
}

void
ProgramHeader::SetPrecision( Type t, Precision p )
{
	if ( Rtt_VERIFY( t > kUnknownType && t < kNumType )
		 && kUnknownPrecision != p )
	{
		fPrecision[t] = p;
	}
}

void
ProgramHeader::SetPrecision( Precision p )
{
	if ( kUnknownPrecision != p )
	{
		for ( int i = 0; i < kNumType; i++ )
		{
			fPrecision[i] = p;
		}
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
