//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_GLProgram.h"
#include "Renderer/Rtt_GLGeometry.h"

#include "Renderer/Rtt_CommandBuffer.h"
#include "Renderer/Rtt_FormatExtensionList.h"
//#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_Texture.h"
#ifdef Rtt_USE_PRECOMPILED_SHADERS
    #include "Renderer/Rtt_ShaderBinary.h"
    #include "Renderer/Rtt_ShaderBinaryVersions.h"
#endif
#include "Core/Rtt_Assert.h"
#include "Core/Rtt_Traits.h"
#include <cstdio>
#include <string.h> // memset.
#ifdef Rtt_WIN_PHONE_ENV
    #include <GLES2/gl2ext.h>
#endif

#include "Display/Rtt_ShaderResource.h"
#include "Corona/CoronaLog.h"
#include "Corona/CoronaGraphics.h"

#include <string>
#include <vector>
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
		"}";}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

struct GLProgramUniformInfo {
    GLProgramUniformInfo()
    {
        for (int i = 0; i < Program::kNumVersions; ++i)
        {
            fLocations[i] = -1;
        }
    }
    
    GLint fLocations[Program::kNumVersions];
    GLint size;
    GLenum type;
    std::string fName;
};

struct GLProgramUniformsCache {
    std::vector< GLProgramUniformInfo > fInfo;
};

GLProgram::GLProgram()
:   fCleanupShellTransform( NULL ),
    fUniformsCache( NULL )
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

    Rtt_STATIC_ASSERT( ( Traits::IsSame< decltype(fCleanupShellTransform),  CoronaShellTransformStateCleanup >::Value ) );
    
    Program* program = static_cast<Program*>( fResource );
    ShaderResource* shaderResource = program->GetShaderResource();
    const CoronaShellTransform * transform = shaderResource->GetShellTransform();

    if (transform && transform->cleanup)
    {
        fCleanupShellTransform = transform->cleanup;
    }
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
    
    if (fCleanupShellTransform)
    {
        fCleanupShellTransform( &fCleanupShellTransform ); // n.b. used as own key
    }

    Rtt_DELETE( fUniformsCache );
    
    fUniformsCache = NULL;
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

static void
SetShaderSource( GLuint shader, CoronaShellTransformParams & params, const CoronaShellTransform * xform, void * userData, void * key )
{
    const char ** strings = params.sources, ** old = strings;

    if (xform)
    {
        Rtt_ASSERT( xform->begin );
        
        strings = xform->begin( &params, userData, key );

        if (!strings)
        {
            strings = old;
        }
    }

    glShaderSource( shader, params.nsources, strings, NULL );

    if (xform && xform->finish)
    {
        xform->finish( userData, key );
    }

    GL_CHECK_ERROR();
}

static bool
IsDoubleType( CoronaVertexExtensionAttributeType )
{
    return false; // NYI
}

static void
AppendMacroName( const char* name, std::string& extensionAttributes )
{
    char buf[BUFSIZ];
    const char * rest = name + 1;
    
    sprintf( buf, "#define Corona%c%s a_%s\n", toupper( *name ), *rest ? rest : "", name );

    extensionAttributes += buf;
}

static void
GatherAttributeExtensions( const FormatExtensionList* extensionList, std::string& extensionAttributes )
{
    extensionList->SortNames();
    
    for (int i = 0; i < extensionList->GetAttributeCount(); ++i)
    {
        const FormatExtensionList::Attribute& attribute = extensionList->GetAttributes()[i];
        char buf[64], count[2] = {};
        
        if (attribute.components > 1)
        {
            count[0] = '0' + attribute.components;
        }
        
        const char * prim = "float", * vec = "vec";

        CoronaVertexExtensionAttributeType type = (CoronaVertexExtensionAttributeType)attribute.type;

        if (IsDoubleType( type ))
        {
            prim = "double";
            vec = "dvec";
        }
 
        else if (!attribute.IsFloat())
        {
            prim = "int";
            vec = "ivec";
        }
            
        sprintf( buf, "attribute %s%s a_%s;\n", *count ? vec : prim, count, extensionList->FindNameByAttribute( i ) );
        
        extensionAttributes += buf;
    }
    
    extensionAttributes += "\n";
    
    for (int i = 0; i < extensionList->GetAttributeCount(); ++i)
    {
        AppendMacroName( extensionList->FindNameByAttribute( i ), extensionAttributes );
    }
}

void
GLProgram::UpdateShaderSource( Program* program, Program::Version version, VersionData& data )
{
#ifndef Rtt_USE_PRECOMPILED_SHADERS
    char maskBuffer[] = "#define MASK_COUNT 0\n";
    switch( version )
    {
        case Program::kMaskCount1:    maskBuffer[sizeof( maskBuffer ) - 3] = '1'; break;
        case Program::kMaskCount2:    maskBuffer[sizeof( maskBuffer ) - 3] = '2'; break;
        case Program::kMaskCount3:    maskBuffer[sizeof( maskBuffer ) - 3] = '3'; break;
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
    
    ShaderResource * shaderResource = program->GetShaderResource();
    const CoronaShellTransform * shellTransform = shaderResource->GetShellTransform();
    CoronaShellTransformParams params = {};
    const char * hints[] = { "header", "highpSupport", "mask", "texCoordZ", NULL };
    void * shellTransformKey = &fCleanupShellTransform; // n.b. done to make cleanup robust

    std::vector< CoronaEffectDetail > details;
    CoronaEffectDetail detail;

    for (int i = 0; shaderResource->GetEffectDetail( i, detail ); ++i)
    {
        details.push_back( detail );
    }

    params.details = details.data();
    params.ndetails = details.size();
    params.userData = shellTransform ? shellTransform->userData : NULL;

    std::vector< U8 > space;
    U8 * spaceData = NULL;

    if (shellTransform && shellTransform->workSpace)
    {
        space.resize( shellTransform->workSpace );

        spaceData = space.data();
    }

    // Vertex shader.
    {
        const char * extendedSources[7] = {}, * extendedHints[8] = {};
        std::string extensionAttributes, suffixStr, versionStr;
        
        params.hints = hints;
        params.sources = shader_source;
        params.nsources = sizeof(shader_source) / sizeof(shader_source[0]);
        params.type = "vertex";
        
        shader_source[4] = program->GetVertexShaderSource();
        hints[4] = "vertexSource";

        // add any boilerplate for extended vertices and / or instancing
        const FormatExtensionList* extensionList = shaderResource->GetExtensionList();
        
        if (extensionList)
        {
            for (int i = 0; i < 4; ++i)
            {
                extendedSources[i] = shader_source[i];
                extendedHints[i] = hints[i];
            }
                        
            GatherAttributeExtensions( extensionList, extensionAttributes );
            
            const char * originalSource = shader_source[4], * originalHint = hints[4];
            U32 nsources = params.nsources + 1;
            
            extendedSources[4] = extensionAttributes.c_str();
            extendedHints[4] = "extensionAttributes";
            
            // enable instances and / or provide IDs for the same
            if (extensionList->IsInstanced())
            {
                const char * idSuffix = GLGeometry::InstanceIDSuffix();
                
                if (idSuffix)
                {
                    char buf[BUFSIZ];
            
                    if ('*' == *idSuffix)
                    {
                        ++idSuffix;
                        
                        U32 offset = 0;
                        
                        char version[64] = {};
                        
                        while ('\n' != shader_source[0][offset])
                        {
                            Rtt_ASSERT( offset < 63 );
                            Rtt_ASSERT( shader_source[0][offset] );
                            
                            version[offset++] = shader_source[0][offset];
                        }
                        
                        sprintf( buf,
                                "%s\n\n#extension GL_%s_draw_instanced : enable%s",
                                version, idSuffix, shader_source[0] + offset );
                        
                        versionStr = buf;
                        
                        extendedSources[0] = versionStr.c_str();
                    }
                    
					sprintf( buf,
							"\n#define CoronaInstanceID int(gl_InstanceID%s)\n"
							"\n#define CoronaInstanceFloat float(gl_InstanceID%s)\n\n",
							idSuffix, idSuffix );
                    
                    suffixStr = buf;
                    
                    extendedSources[nsources - 1] = suffixStr.c_str();
                }
                
                else
                {
					extendedSources[nsources - 1] = "\n#define CoronaInstanceID 0\n"
												"\n#define CoronaInstanceFloat 0.\n\n";
                }
                
                extendedHints[nsources - 1] = "instanceID";
                
                ++nsources;
            }

            extendedSources[nsources - 1] = originalSource;
            extendedHints[nsources - 1] = originalHint;

            params.hints = extendedHints;
            params.sources = extendedSources;
            params.nsources = nsources;
        }
        
        SetShaderSource( data.fVertexShader, params, shellTransform, spaceData, shellTransformKey );
    }

    // Fragment shader.
    {
        shader_source[4] = ( version == Program::kWireframe ) ? kWireframeSource : program->GetFragmentShaderSource();

        hints[4] = "fragmentSource";
        params.type = "fragment";
        params.hints = hints;
        params.sources = shader_source;
        params.nsources = sizeof(shader_source) / sizeof(shader_source[0]);
        
        SetShaderSource( data.fFragmentShader, params, shellTransform, spaceData, shellTransformKey );
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

    const FormatExtensionList* extensionList = program->GetShaderResource()->GetExtensionList();

    if (extensionList)
    {
        GLuint first = Geometry::FirstExtraAttribute();

        for (U32 i = 0; i < extensionList->GetAttributeCount(); ++i)
        {
            S32 index;
            char buf[BUFSIZ];
            
            sprintf( buf, "a_%s", extensionList->FindNameByAttribute( i, &index ) );
            
            glBindAttribLocation( data.fProgram, first + index, buf );
        }

        GL_CHECK_ERROR();
    }
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

GLExtraUniforms::GLExtraUniforms()
:   fVersion( Program::kNumVersions ),
    fVersionData( NULL ),
    fCache( NULL )
{
}

GLExtraUniforms::GLExtraUniforms( Program::Version version, const GLProgram::VersionData * versionData, GLProgramUniformsCache ** cache )
:   fVersion( version ),
    fVersionData( versionData ),
    fCache( cache )
{
}

GLint
GLExtraUniforms::Find( const char * name, GLint & size, GLenum & type )
{
    if (!fCache)
    {
        Rtt_LogException( "Extra uniforms cache not yet initialized" );
        
        return -1;
    }
    
    // Has this name ever been found?
    int entryIndex = -1;
    
    if (*fCache)
    {
        for (int i = 0; i < (*fCache)->fInfo.size(); ++i)
        {
            const auto & pos = (*fCache)->fInfo[i];
            
            if (0 == strcmp( pos.fName.c_str(), name ))
            {
                entryIndex = i;
                
                if (pos.fLocations[fVersion] >= 0) // version as well?
                {
                    size = pos.size;
                    type = pos.type;
                    
                    return pos.fLocations[fVersion];
                }
                
                break;
            }
        }
    }

    // Does the uniform even exist?
    const GLProgram::VersionData & versionData = fVersionData[fVersion];
    GLint location = glGetUniformLocation( versionData.fProgram, reinterpret_cast< const GLchar * >( name ) );

    if (-1 == location)
    {
        Rtt_LogException( "WARNING: uniform `%s` not found in effect", name );
        
        return -1;
    }
    
    // No entry yet?
    if (-1 == entryIndex)
    {
        // Not a built-in?
        if (name[0] && name[1] && 'u' == name[0] && '_' == name[1])
        {
            for (int i = 0; i < Uniform::kNumBuiltInVariables; ++i)
            {
                if (versionData.fUniformLocations[i] == location)
                {
                    Rtt_LogException( "WARNING: `%s` is a built-in uniform", name );
                    
                    return -1;
                }
            }
        }
        
        // Gather details.
        GLint count;
        
        glGetProgramiv( versionData.fProgram, GL_ACTIVE_UNIFORMS, &count );
        
        GLchar nameBuf[GLProgram::kUniformNameBufferSize];
        GLsizei length;
        GLint uniformIndex;
        
        for (uniformIndex = 0; uniformIndex < count; ++uniformIndex)
        {
            glGetActiveUniform( versionData.fProgram, (GLuint)uniformIndex, GLProgram::kUniformNameBufferSize - 1, &length, &size, &type, nameBuf );

            const char * bracket = strchr( nameBuf, '[' );
            
            if (bracket)
            {
                length = bracket - nameBuf;
            }
            
            if (0 == strncmp( name, nameBuf, length ))
            {
                break;
            }
        }
        
        if (uniformIndex == count)
        {
            Rtt_LogException( "Location of uniform `%s` found, but no active info: name too long?", name );
            
            return -1;
        }
        
        switch (type)
        {
        case GL_FLOAT:
        case GL_FLOAT_VEC2:
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
        case GL_FLOAT_MAT2:
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT4:
            break;
        default:
            Rtt_LogException( "Location of uniform `%s` found, but type unsupported", name );
                
            return -1;
        }
          
        // No cache yet?
        if (!*fCache)
        {
            *fCache = Rtt_NEW( NULL, GLProgramUniformsCache );
        }
    
        // Install the details.
        entryIndex = (int)(*fCache)->fInfo.size();
        
        (*fCache)->fInfo.push_back( GLProgramUniformInfo{} );
        
        GLProgramUniformInfo & newInfo = (*fCache)->fInfo.back();
        
        newInfo.size = size;
        newInfo.type = type;
        newInfo.fName = name;
    }
    
    else
    {
        size = (*fCache)->fInfo[entryIndex].size;
        type = (*fCache)->fInfo[entryIndex].type;
    }
    
    // Register the location and return it.
    (*fCache)->fInfo[entryIndex].fLocations[fVersion] = location;
    
    return location;
}

void
GLProgram::GetExtraUniformsInfo( Program::Version version, GLExtraUniforms& extraUniforms )
{
    extraUniforms = GLExtraUniforms( version, fData, &fUniformsCache );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
