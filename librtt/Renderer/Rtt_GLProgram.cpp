//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_GLProgram.h"

#include "Renderer/Rtt_CommandBuffer.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_Texture.h"
#ifdef Rtt_USE_PRECOMPILED_SHADERS
	#include "Renderer/Rtt_ShaderBinary.h"
	#include "Renderer/Rtt_ShaderBinaryVersions.h"
#endif
#include "Core/Rtt_Assert.h"
#include <cstdio>
#include <string.h> // memset.
#ifdef Rtt_WIN_PHONE_ENV
	#include <GLES2/gl2ext.h>
#endif

#include "Rtt_Profiling.h"


// To reduce memory consumption and startup cost, defer the
// creation of GL shaders and programs until they're needed.
// Depending on usage, this could result in framerate dips.
#define DEFER_CREATION 1

// ----------------------------------------------------------------------------

namespace /*anonymous*/
{
	using namespace Rtt;

	// Check that the given shader compiled and log any errors
	void CheckShaderCompilationStatus( GLuint name, bool isVerbose, const char *label, int startLine )
	{
		GLint result;
		glGetShaderiv( name, GL_COMPILE_STATUS, &result );
		if( result == GL_FALSE )
		{
			GLint length;
			glGetShaderiv( name, GL_INFO_LOG_LENGTH, &length );

			GLchar* infoLog = new GLchar[length];
			glGetShaderInfoLog( name, length, NULL, infoLog );

			if ( isVerbose )
			{
				if ( label )
				{
					Rtt_LogException( "ERROR: An error occurred in the %s kernel.\n", label );
				}
				Rtt_LogException( "%s", infoLog );
				Rtt_LogException( "\tNOTE: Kernel starts at line number (%d), so subtract that from the line numbers above.\n", startLine );
			}
			delete[] infoLog;
		}
	}

	// Check that the given program linked and log any errors
	void CheckProgramLinkStatus( GLuint name, bool isVerbose )
	{
		GLint result;
		glGetProgramiv( name, GL_LINK_STATUS, &result );
		if( result == GL_FALSE )
		{
			GLint length;
			glGetProgramiv( name, GL_INFO_LOG_LENGTH, &length );

			GLchar* infoLog = new GLchar[length];
			glGetProgramInfoLog( name, length, NULL, infoLog );

			if ( isVerbose )
			{
				Rtt_LogException( "%s", infoLog );
			}
			else
			{
				Rtt_LogException(
					"ERROR: A shader failed to compile. To see errors, add the following to the top of your main.lua:\n"
					"\tdisplay.setDefault( 'isShaderCompilerVerbose', true )\n" );
			}
			delete[] infoLog;
		}
	}
	
	const char* kWireframeSource =
		"void main()" \
		"{" \
			"gl_FragColor = vec4(1.0);" \
		"}";
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

GLProgram::GLProgram()
{
	for( U32 i = 0; i < Program::kNumVersions; ++i )
	{
		Reset( fData[i] );
	}
}

void 
GLProgram::Create( CPUResource* resource )
{
	SUMMED_TIMING( glpc, "Program GPU Resource: Create" );

	Rtt_ASSERT( CPUResource::kProgram == resource->GetType() );
	fResource = resource;
	
	#if !DEFER_CREATION
		for( U32 i = 0; i < kMaximumMaskCount + 1; ++i )
		{
			Create( fData[i], i );
		}
	#endif
}

void
GLProgram::Update( CPUResource* resource )
{
	SUMMED_TIMING( glpu, "Program GPU Resource: Update" );

	Rtt_ASSERT( CPUResource::kProgram == resource->GetType() );	
	if( fData[Program::kMaskCount0].fProgram ) Update( Program::kMaskCount0, fData[Program::kMaskCount0] );
	if( fData[Program::kMaskCount1].fProgram ) Update( Program::kMaskCount1, fData[Program::kMaskCount1] );
	if( fData[Program::kMaskCount2].fProgram ) Update( Program::kMaskCount2, fData[Program::kMaskCount2] );
	if( fData[Program::kMaskCount3].fProgram ) Update( Program::kMaskCount3, fData[Program::kMaskCount3] );
	if( fData[Program::kWireframe].fProgram ) Update( Program::kWireframe, fData[Program::kWireframe]);
}

void 
GLProgram::Destroy()
{
	for( U32 i = 0; i < Program::kNumVersions; ++i )
	{
		VersionData& data = fData[i];
		if( data.fProgram )
		{
#ifndef Rtt_USE_PRECOMPILED_SHADERS
			glDeleteShader( data.fVertexShader );
			glDeleteShader( data.fFragmentShader );
#endif
			glDeleteProgram( data.fProgram );
			GL_CHECK_ERROR();
			Reset( data );
		}
	}
}

void
GLProgram::Bind( Program::Version version )
{
	VersionData& data = fData[version];
	
	#if DEFER_CREATION
		if( !data.fProgram )
		{
			Create( version, data );
		}
	#endif
	
	glUseProgram( data.fProgram );
	GL_CHECK_ERROR();
}

void
GLProgram::Create( Program::Version version, VersionData& data )
{
#ifndef Rtt_USE_PRECOMPILED_SHADERS
	data.fVertexShader = glCreateShader( GL_VERTEX_SHADER );
	data.fFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
	GL_CHECK_ERROR();
#endif

	data.fProgram = glCreateProgram();
	GL_CHECK_ERROR();

#ifndef Rtt_USE_PRECOMPILED_SHADERS
	glAttachShader( data.fProgram, data.fVertexShader );
	glAttachShader( data.fProgram, data.fFragmentShader );
	GL_CHECK_ERROR();
#endif
	
	Update( version, data );
}

static int
CountLines( const char **segments, int numSegments )
{
	int result = 0;

	for ( int i = 0; i < numSegments; i++ )
	{
		result += Program::CountLines( segments[i] );
	}

	return result;
}

void
GLProgram::UpdateShaderSource( Program* program, Program::Version version, VersionData& data )
{
#ifndef Rtt_USE_PRECOMPILED_SHADERS
	char maskBuffer[] = "#define MASK_COUNT 0\n";
	switch( version )
	{
		case Program::kMaskCount1:	maskBuffer[sizeof( maskBuffer ) - 3] = '1'; break;
		case Program::kMaskCount2:	maskBuffer[sizeof( maskBuffer ) - 3] = '2'; break;
		case Program::kMaskCount3:	maskBuffer[sizeof( maskBuffer ) - 3] = '3'; break;
		default: break;
	}

	char highp_support[] = "#define FRAGMENT_SHADER_SUPPORTS_HIGHP 0\n";
	highp_support[ sizeof( highp_support ) - 3 ] = ( CommandBuffer::GetGpuSupportsHighPrecisionFragmentShaders() ? '1' : '0' );

	//! \TODO Make the definition of "TEX_COORD_Z" conditional.
	char texCoordZBuffer[] = "";//#define TEX_COORD_Z 1\n";

	const char *program_header_source = program->GetHeaderSource();
	const char *header = ( program_header_source ? program_header_source : "" );

	const char* shader_source[5];
	memset( shader_source, 0, sizeof( shader_source ) );
	shader_source[0] = header;
	shader_source[1] = highp_support;
	shader_source[2] = maskBuffer;
	shader_source[3] = texCoordZBuffer;

	if ( program->IsCompilerVerbose() )
	{
		// All the segments except the last one
		int numSegments = sizeof( shader_source ) / sizeof( shader_source[0] ) - 1;
		data.fHeaderNumLines = CountLines( shader_source, numSegments );
	}

	// Vertex shader.
	{
		shader_source[4] = program->GetVertexShaderSource();

		glShaderSource( data.fVertexShader,
						( sizeof(shader_source) / sizeof(shader_source[0]) ),
						shader_source,
						NULL );
		GL_CHECK_ERROR();
	}

	// Fragment shader.
	{
		shader_source[4] = ( version == Program::kWireframe ) ? kWireframeSource : program->GetFragmentShaderSource();

		glShaderSource( data.fFragmentShader,
						( sizeof(shader_source) / sizeof(shader_source[0]) ),
						shader_source,
						NULL );
		GL_CHECK_ERROR();
	}
#endif
}

void
GLProgram::Update( Program::Version version, VersionData& data )
{
	Program* program = static_cast<Program*>( fResource );

#ifndef Rtt_USE_PRECOMPILED_SHADERS
	glBindAttribLocation( data.fProgram, Geometry::kVertexPositionAttribute, "a_Position" );
	glBindAttribLocation( data.fProgram, Geometry::kVertexTexCoordAttribute, "a_TexCoord" );
	glBindAttribLocation( data.fProgram, Geometry::kVertexColorScaleAttribute, "a_ColorScale" );
	glBindAttribLocation( data.fProgram, Geometry::kVertexUserDataAttribute, "a_UserData" );
	GL_CHECK_ERROR();
#endif

	UpdateShaderSource( program,
						version,
						data );

#ifdef Rtt_USE_PRECOMPILED_SHADERS
	ShaderBinary *shaderBinary = program->GetCompiledShaders()->Get(version);
	glProgramBinaryOES(data.fProgram, GL_PROGRAM_BINARY_ANGLE, shaderBinary->GetBytes(), shaderBinary->GetByteCount());
	GL_CHECK_ERROR();
	GLint linkResult = 0;
	glGetProgramiv(data.fProgram, GL_LINK_STATUS, &linkResult);
	if (!linkResult)
	{
		const int MAX_MESSAGE_LENGTH = 1024;
		char message[MAX_MESSAGE_LENGTH];
		GLint resultLength = 0;
		glGetProgramInfoLog(data.fProgram, MAX_MESSAGE_LENGTH, &resultLength, message);
		Rtt_LogException(message);
	}
	int locationIndex;
	locationIndex = glGetAttribLocation(data.fProgram, "a_Position");
	locationIndex = glGetAttribLocation(data.fProgram, "a_TexCoord");
	locationIndex = glGetAttribLocation(data.fProgram, "a_ColorScale");
	locationIndex = glGetAttribLocation(data.fProgram, "a_UserData");
#else
	bool isVerbose = program->IsCompilerVerbose();
	int kernelStartLine = 0;

	glCompileShader( data.fVertexShader );
	if ( isVerbose )
	{
		kernelStartLine = data.fHeaderNumLines + program->GetVertexShellNumLines();
	}
	CheckShaderCompilationStatus( data.fVertexShader, isVerbose, "vertex", kernelStartLine );
	GL_CHECK_ERROR();

	glCompileShader( data.fFragmentShader );
	if ( isVerbose )
	{
		kernelStartLine = data.fHeaderNumLines + program->GetFragmentShellNumLines();
	}
	CheckShaderCompilationStatus( data.fFragmentShader, isVerbose, "fragment", kernelStartLine );
	GL_CHECK_ERROR();

	glLinkProgram( data.fProgram );
	CheckProgramLinkStatus( data.fProgram, isVerbose );
	GL_CHECK_ERROR();
#endif

	data.fUniformLocations[Uniform::kViewProjectionMatrix] = glGetUniformLocation( data.fProgram, "u_ViewProjectionMatrix" );
	GL_CHECK_ERROR();
	data.fUniformLocations[Uniform::kMaskMatrix0] = glGetUniformLocation( data.fProgram, "u_MaskMatrix0" );
	GL_CHECK_ERROR();
	data.fUniformLocations[Uniform::kMaskMatrix1] = glGetUniformLocation( data.fProgram, "u_MaskMatrix1" );
	GL_CHECK_ERROR();
	data.fUniformLocations[Uniform::kMaskMatrix2] = glGetUniformLocation( data.fProgram, "u_MaskMatrix2" );
	GL_CHECK_ERROR();
	data.fUniformLocations[Uniform::kTotalTime] = glGetUniformLocation( data.fProgram, "u_TotalTime" );
	GL_CHECK_ERROR();
	data.fUniformLocations[Uniform::kDeltaTime] = glGetUniformLocation( data.fProgram, "u_DeltaTime" );
	GL_CHECK_ERROR();
	data.fUniformLocations[Uniform::kTexelSize] = glGetUniformLocation( data.fProgram, "u_TexelSize" );
	GL_CHECK_ERROR();
	data.fUniformLocations[Uniform::kContentScale] = glGetUniformLocation( data.fProgram, "u_ContentScale" );
	GL_CHECK_ERROR();
	data.fUniformLocations[Uniform::kUserData0] = glGetUniformLocation( data.fProgram, "u_UserData0" );
	GL_CHECK_ERROR();
	data.fUniformLocations[Uniform::kUserData1] = glGetUniformLocation( data.fProgram, "u_UserData1" );
	GL_CHECK_ERROR();
	data.fUniformLocations[Uniform::kUserData2] = glGetUniformLocation( data.fProgram, "u_UserData2" );
	GL_CHECK_ERROR();
	data.fUniformLocations[Uniform::kUserData3] = glGetUniformLocation( data.fProgram, "u_UserData3" );
	GL_CHECK_ERROR();

	glUseProgram( data.fProgram );
	glUniform1i( glGetUniformLocation( data.fProgram, "u_FillSampler0" ), Texture::kFill0 );
	glUniform1i( glGetUniformLocation( data.fProgram, "u_FillSampler1" ), Texture::kFill1 );
	glUniform1i( glGetUniformLocation( data.fProgram, "u_MaskSampler0" ), Texture::kMask0 );
	glUniform1i( glGetUniformLocation( data.fProgram, "u_MaskSampler1" ), Texture::kMask1 );
	glUniform1i( glGetUniformLocation( data.fProgram, "u_MaskSampler2" ), Texture::kMask2 );
	glUseProgram( 0 );
	GL_CHECK_ERROR();
}

void
GLProgram::Reset( VersionData& data )
{
	data.fProgram = 0;
	data.fVertexShader = 0;
	data.fFragmentShader = 0;

	for( U32 i = 0; i < Uniform::kNumBuiltInVariables; ++i )
	{
		// OpenGL uses the location -1 for inactive uniforms
		const GLint kInactiveLocation = -1;
		data.fUniformLocations[ i ] = kInactiveLocation;
		
		// CommandBuffer also initializes timestamp to zero
		const U32 kTimestamp = 0;
		data.fTimestamps[ i ] = kTimestamp;
	}
	
	data.fHeaderNumLines = 0;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
