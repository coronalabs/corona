//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_GLCommandBuffer.h"

#include "Renderer/Rtt_FrameBufferObject.h"
#include "Renderer/Rtt_FormatExtensionList.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_GL.h"
#include "Renderer/Rtt_GLFrameBufferObject.h"
#include "Renderer/Rtt_GLGeometry.h"
#include "Renderer/Rtt_GLProgram.h"
#include "Renderer/Rtt_GLTexture.h"
#include "Renderer/Rtt_Program.h"
#include "Renderer/Rtt_Texture.h"
#include "Renderer/Rtt_Uniform.h"
#include "Display/Rtt_ShaderData.h"
#include "Display/Rtt_ShaderResource.h"
#include "Display/Rtt_ObjectHandle.h"
#include "Core/Rtt_Config.h"
#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_Assert.h"
#include "Core/Rtt_Math.h"
#include <cstdio>
#include <string.h>
#include "Core/Rtt_String.h"

#include "Corona/CoronaGraphics.h"

#define ENABLE_DEBUG_PRINT    0

// TODO: Temporary hack
#ifdef Rtt_IPHONE_ENV
#include "../platform/iphone/Rtt_IPhoneGLVideoTexture.h"
#endif

// ----------------------------------------------------------------------------

namespace /*anonymous*/
{
    enum Command
    {
        kCommandBindFrameBufferObject,
        kCommandUnBindFrameBufferObject,
		kCommandCaptureRect,
        kCommandBindGeometry,
        kCommandBindTexture,
        kCommandBindProgram,
        kCommandBindInstancing,
        kCommandResolveVertexFormat,
        kCommandApplyUniformScalar,
        kCommandApplyUniformVec2,
        kCommandApplyUniformVec3,
        kCommandApplyUniformVec4,
        kCommandApplyUniformMat3,
        kCommandApplyUniformMat4,
        kCommandApplyUniformFromPointerScalar,
        kCommandApplyUniformFromPointerVec2,
        kCommandApplyUniformFromPointerVec3,
        kCommandApplyUniformFromPointerVec4,
        kCommandApplyUniformFromPointerMat3,
        kCommandApplyUniformFromPointerMat4,
        kCommandEnableBlend,
        kCommandDisableBlend,
        kCommandSetBlendFunction,
        kCommandSetBlendEquation,
        kCommandSetViewport,
        kCommandEnableScissor,
        kCommandDisableScissor,
        kCommandSetScissorRegion,
        kCommandEnableMultisample,
        kCommandDisableMultisample,
        kCommandClearDepth,
        kCommandClearStencil,
        kCommandClear,
        kCommandDraw,
        kCommandDrawIndexed,
        kNumCommands
    };

    // To ease reading/writing of arrays
    struct Vec2 { Rtt::Real data[2]; };
    struct Vec3 { Rtt::Real data[3]; };
    struct Vec4 { Rtt::Real data[4]; };
    struct Mat3 { Rtt::Real data[9]; };
    struct Mat4 { Rtt::Real data[16]; };

#ifdef ENABLE_GPU_TIMER_QUERIES
    const Rtt::Real kNanosecondsToMilliseconds = 1.0f / 1000000.0f;
#endif
    const U32 kTimerQueryCount = 3;
    
    // The Uniform timestamp counter must be the same for both the
    // front and back CommandBuffers, though only one CommandBuffer
    // will ever write the timestamp on any given frame. If it were
    // ever the case that more than two CommandBuffers were used,
    // this would need to be made a shared member variable.
    static U32 gUniformTimestamp = 0;

    // Extract location and data from buffer
    #define READ_UNIFORM_DATA( Type ) \
        GLint location = Read<GLint>(); \
        Type value = Read<Type>();

    // Extract data but query for location
    #define READ_UNIFORM_DATA_WITH_PROGRAM( Type ) \
                Rtt::GLProgram* program = Read<Rtt::GLProgram*>(); \
                U32 index = Read<U32>(); \
                GLint location = program->GetUniformLocation( index, fCurrentDrawVersion ); \
                Type value = Read<Type>();
    
    #define CHECK_ERROR_AND_BREAK GL_CHECK_ERROR(); break;

    // Ensure command count is incremented
    #define WRITE_COMMAND( command ) Write<Command>( command ); ++fNumCommands;
    
    // Used to validate that the appropriate OpenGL commands
    // are being generated and that their arguments are correct
    #if ENABLE_DEBUG_PRINT
        #define DEBUG_PRINT( ... ) Rtt_LogException( __VA_ARGS__ ); Rtt_LogException("\n");
        #define DEBUG_PRINT_MATRIX( message, data, count ) \
            Rtt_LogException( "%s\n", message ); \
            Rtt_LogException( "[ %.3f", data[0] ); \
            for( U32 i = 1; i < count; ++i ) \
                Rtt_LogException( ", %.3f", data[i] ); \
            Rtt_LogException ("]\n" );
    #else
        #define DEBUG_PRINT( ... )
        #define DEBUG_PRINT_MATRIX( message, data, count )
    #endif
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

size_t
CommandBuffer::GetMaxUniformVectorsCount()
{
    GLint count;

#ifndef Rtt_OPENGLES
    glGetIntegerv( GL_MAX_VERTEX_UNIFORM_COMPONENTS, &count );
#else
    glGetIntegerv( GL_MAX_VERTEX_UNIFORM_VECTORS, &count );
#endif
    GL_CHECK_ERROR();
    
#ifndef Rtt_OPENGLES
    count /= 4; // as vectors
#endif

    count -=    4 // kViewProjectionMatrix
                + 3 * 4 // kMaskMatrix*, assuming 3 vectors each
                + 2 // kTotalTime, kDeltaTime, again 1 vector each
                + 1 // kTexelSize, ditto
                + 1;// kContentScale, the same

    return count;
}

size_t
CommandBuffer::GetMaxVertexTextureUnits()
{
    static size_t sMaxUnits = ~0; // 0 would be valid result
    
    if ( ~0 == sMaxUnits )
    {
        GLint maxUnits = 0;

        glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &maxUnits ); // TODO: check if this is same on Android, etc.
        sMaxUnits = maxUnits;
        GL_CHECK_ERROR();
    }

    return sMaxUnits;
}

size_t
CommandBuffer::GetMaxTextureSize()
{
    static size_t sMaxSize = 0;
    
    if ( 0 == sMaxSize )
    {
        GLint maxSize = 1024;
        glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxSize );
        sMaxSize = maxSize;
        GL_CHECK_ERROR();
    }
    return sMaxSize;
}

const char *
CommandBuffer::GetGlString( const char *s )
{
    if( Rtt_StringCompare( s, "GL_VENDOR" ) == 0 )
    {
        return (const char *)glGetString( GL_VENDOR );
    }
    else if( Rtt_StringCompare( s, "GL_RENDERER" ) == 0 )
    {
        return (const char *)glGetString( GL_RENDERER );
    }
    else if( Rtt_StringCompare( s, "GL_VERSION" ) == 0 )
    {
        return (const char *)glGetString( GL_VERSION );
    }
    else if( Rtt_StringCompare( s, "GL_SHADING_LANGUAGE_VERSION" ) == 0 )
    {
        return (const char *)glGetString( GL_SHADING_LANGUAGE_VERSION );
    }
    else if( Rtt_StringCompare( s, "GL_EXTENSIONS" ) == 0 )
    {
        return (const char *)glGetString( GL_EXTENSIONS );
    }
    else
    {
        return "";
    }
}

bool
CommandBuffer::GetGpuSupportsHighPrecisionFragmentShaders()
{
#if defined( Rtt_MAC_ENV ) || defined( Rtt_WIN_DESKTOP_ENV ) || defined( Rtt_EMSCRIPTEN_ENV )|| defined( Rtt_LINUX_ENV )

    // Assume desktops support HIGHP.
    return true;

#else

    static bool hasObtained_GpuSupportsHighPrecisionFragmentShaders = false;
    static bool gpuSupportsHighPrecisionFragmentShaders = false;

    if( hasObtained_GpuSupportsHighPrecisionFragmentShaders )
    {
        return gpuSupportsHighPrecisionFragmentShaders;
    }

    int range_in_bits[ 2 ];
    int precision_in_bits;

    memset( range_in_bits, 0, sizeof( range_in_bits ) );
    memset( &precision_in_bits, 0, sizeof( precision_in_bits ) );

    // lowp.
    {
        // This function is available everywhere EXCEPT OSX.
        glGetShaderPrecisionFormat( GL_FRAGMENT_SHADER,
                                    GL_LOW_FLOAT,
                                    range_in_bits,
                                    &precision_in_bits );

        DEBUG_PRINT( "%s GL_LOW_FLOAT min range_in_bits[ 0 ] : %d\n",
                        Rtt_FUNCTION,
                        range_in_bits[ 0 ] );
        DEBUG_PRINT( "%s GL_LOW_FLOAT max range_in_bits[ 1 ] : %d\n",
                        Rtt_FUNCTION,
                        range_in_bits[ 1 ] );
        DEBUG_PRINT( "%s GL_LOW_FLOAT precision_in_bits : %d\n",
                        Rtt_FUNCTION,
                        precision_in_bits );
    }

    // mediump.
    {
        // This function is available everywhere EXCEPT OSX.
        glGetShaderPrecisionFormat( GL_FRAGMENT_SHADER,
                                    GL_MEDIUM_FLOAT,
                                    range_in_bits,
                                    &precision_in_bits );

        DEBUG_PRINT( "%s GL_MEDIUM_FLOAT min range_in_bits[ 0 ] : %d\n",
                        Rtt_FUNCTION,
                        range_in_bits[ 0 ] );
        DEBUG_PRINT( "%s GL_MEDIUM_FLOAT max range_in_bits[ 1 ] : %d\n",
                        Rtt_FUNCTION,
                        range_in_bits[ 1 ] );
        DEBUG_PRINT( "%s GL_MEDIUM_FLOAT precision_in_bits : %d\n",
                        Rtt_FUNCTION,
                        precision_in_bits );
    }

    // highp.
    {
        // This function is available everywhere EXCEPT OSX.
        glGetShaderPrecisionFormat( GL_FRAGMENT_SHADER,
                                    GL_HIGH_FLOAT,
                                    range_in_bits,
                                    &precision_in_bits );

        DEBUG_PRINT( "%s GL_HIGH_FLOAT min range_in_bits[ 0 ] : %d\n",
                        Rtt_FUNCTION,
                        range_in_bits[ 0 ] );
        DEBUG_PRINT( "%s GL_HIGH_FLOAT max range_in_bits[ 1 ] : %d\n",
                        Rtt_FUNCTION,
                        range_in_bits[ 1 ] );
        DEBUG_PRINT( "%s GL_HIGH_FLOAT precision_in_bits : %d\n",
                        Rtt_FUNCTION,
                        precision_in_bits );

        // If the returned values from glGetShaderPrecisionFormat()
        // are ALL zeros, then HIGHP ISN'T supported.
        gpuSupportsHighPrecisionFragmentShaders = ( ! ( ( range_in_bits[ 0 ] == 0 ) &&
                                                        ( range_in_bits[ 1 ] == 0 ) &&
                                                        ( precision_in_bits == 0 ) ) );

        hasObtained_GpuSupportsHighPrecisionFragmentShaders = true;

        return gpuSupportsHighPrecisionFragmentShaders;
    }

#endif
}

bool
GLCommandBuffer::HasFramebufferBlit( bool * canScale ) const
{
	return GLFrameBufferObject::HasFramebufferBlit( canScale );
}

void
GLCommandBuffer::GetVertexAttributes( VertexAttributeSupport & support ) const
{
    static GLint sMaxVertexAttribs = -1;
    
    if (sMaxVertexAttribs < 0)
    {
        glGetIntegerv( GL_MAX_VERTEX_ATTRIBS, &sMaxVertexAttribs );
        
        sMaxVertexAttribs -= 4; // ignore built-ins
    }
    
    support.maxCount = (U32)sMaxVertexAttribs;
    support.hasInstancing = GLGeometry::SupportsInstancing();
    support.hasDivisors = GLGeometry::SupportsDivisors();
    support.hasPerInstance = support.hasDivisors; // divisor == 1
    support.suffix = GLGeometry::InstanceIDSuffix();
}

GLCommandBuffer::GLCommandBuffer( Rtt_Allocator* allocator )
:    CommandBuffer( allocator ),
	 fCurrentPrepVersion( Program::kMaskCount0 ),
	 fCurrentDrawVersion( Program::kMaskCount0 ),
	 fProgram( NULL ),
     fDefaultFBO( 0 ),
	 fTimerQueries( new U32[kTimerQueryCount] ),
	 fTimerQueryIndex( 0 ),
	 fElapsedTimeGPU( 0.0f ),
     fCustomCommands( allocator ),
     fExtraUniforms( NULL )
{
    for(U32 i = 0; i < Uniform::kNumBuiltInVariables; ++i)
    {
        fUniformUpdates[i].uniform = NULL;
        fUniformUpdates[i].timestamp = 0;
    }
}

GLCommandBuffer::~GLCommandBuffer()
{
    delete [] fTimerQueries;
}

void
GLCommandBuffer::Initialize()
{
#ifdef ENABLE_GPU_TIMER_QUERIES
    // Used to measure GPU execution time
    glGenQueries( kTimerQueryCount, fTimerQueries );
    for( U32 i = 0; i < kTimerQueryCount; ++i)
    {
        glBeginQuery( GL_TIME_ELAPSED, fTimerQueries[i] );
        glEndQuery( GL_TIME_ELAPSED );
    }
    GL_CHECK_ERROR();
#endif

    // Initialize OpenGL state
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_STENCIL_TEST );
    glDisable( GL_SCISSOR_TEST );
    glDisable( GL_CULL_FACE );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnableVertexAttribArray( Geometry::kVertexPositionAttribute );
    glEnableVertexAttribArray( Geometry::kVertexTexCoordAttribute );
    glEnableVertexAttribArray( Geometry::kVertexColorScaleAttribute );
    glEnableVertexAttribArray( Geometry::kVertexUserDataAttribute );

    InitializeFBO();
    InitializeCachedParams();
    //CacheQueryParam(kMaxTextureSize);
    
    GetMaxTextureSize();

}
void
GLCommandBuffer::InitializeFBO()
{
   // Some platforms render to an FBO by default
    //glGetIntegerv( GL_FRAMEBUFFER_BINDING, &fDefaultFBO );

    GLint curFBO = 0;
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, & curFBO );
    fDefaultFBO = curFBO;
    Rtt_STATIC_ASSERT( sizeof( curFBO ) == sizeof( fDefaultFBO ) );
}

void
GLCommandBuffer::InitializeCachedParams()
{
    for (int i = 0; i < kNumQueryableParams; i++)
    {
        fCachedQuery[i] = -1;
    }
}

void
GLCommandBuffer::CacheQueryParam( CommandBuffer::QueryableParams param )
{
    GLenum glQueryParam = GL_MAX_TEXTURE_SIZE;
    switch (param)
    {
        case CommandBuffer::kMaxTextureSize:
            glQueryParam = GL_MAX_TEXTURE_SIZE;
            break;
        default:
            break;
    }
    
    GLint retVal = -1;
    glGetIntegerv( glQueryParam, &retVal );
    fCachedQuery[param] = retVal;
    
    GL_CHECK_ERROR();
}

void
GLCommandBuffer::Denitialize()
{
#ifdef ENABLE_GPU_TIMER_QUERIES
    glDeleteQueries( kTimerQueryCount, fTimerQueries );
#endif
}

void
GLCommandBuffer::ClearUserUniforms()
{
    fUniformUpdates[Uniform::kMaskMatrix0].uniform = NULL;
    fUniformUpdates[Uniform::kMaskMatrix1].uniform = NULL;
    fUniformUpdates[Uniform::kMaskMatrix2].uniform = NULL;
    fUniformUpdates[Uniform::kUserData0].uniform = NULL;
    fUniformUpdates[Uniform::kUserData1].uniform = NULL;
    fUniformUpdates[Uniform::kUserData2].uniform = NULL;
    fUniformUpdates[Uniform::kUserData3].uniform = NULL;
}

void
GLCommandBuffer::BindFrameBufferObject(FrameBufferObject* fbo, bool asDrawBuffer)
{
	if( fbo )
	{
		WRITE_COMMAND( kCommandBindFrameBufferObject );
		Write<GPUResource*>( fbo->GetGPUResource() );
		Write<bool>( asDrawBuffer );
	}
	else
	{
		WRITE_COMMAND( kCommandUnBindFrameBufferObject );
	}
}

void
GLCommandBuffer::CaptureRect( FrameBufferObject* fbo, Texture& texture, const Rect& rect, const Rect& unclipped )
{
	WRITE_COMMAND( kCommandCaptureRect );
	
	if (!fbo)
	{
		Write<GPUResource*>( texture.GetGPUResource() );
	}
	
	else
	{
		Write<GPUResource*>( NULL );
	}
	
	Write<Rect>( rect );
	Write<Rect>( unclipped );
	Write<U32>( texture.GetWidth() );
	Write<U32>( texture.GetHeight() );
}

void
GLCommandBuffer::BindGeometry( Geometry* geometry )
{
    WRITE_COMMAND( kCommandBindGeometry );
    Write<GPUResource*>( geometry->GetGPUResource() );
}

void
GLCommandBuffer::BindTexture( Texture* texture, U32 unit )
{
    WRITE_COMMAND( kCommandBindTexture );
    Write<U32>( unit );
    Write<GPUResource*>( texture->GetGPUResource() );
}

void
GLCommandBuffer::BindProgram( Program* program, Program::Version version )
{
    WRITE_COMMAND( kCommandBindProgram );
    Write<Program::Version>( version );
    Write<GPUResource*>( program->GetGPUResource() );

    fCurrentPrepVersion = version;
    fProgram = program;

    AcquireTimeTransform( program->GetShaderResource() );
}

void
GLCommandBuffer::BindInstancing( U32 count, Geometry::Vertex* instanceData )
{
    WRITE_COMMAND( kCommandBindInstancing );
    Write( count );
    Write( instanceData );
}

void
GLCommandBuffer::BindVertexFormat( FormatExtensionList* list, U16 fullCount, U16 vertexSize, U32 offset )
{
    WRITE_COMMAND( kCommandResolveVertexFormat );
    Write( fullCount );
    Write( vertexSize );
	Write( offset );
    Write<U16>( list->GetAttributeCount() );
    
    for (U32 i = 0; i < list->GetAttributeCount(); ++i)
    {
        Write(list->GetAttributes()[i]);
    }
        
    Write<U16>( list->GetGroupCount() );

    for (U32 i = 0; i < list->GetGroupCount(); ++i)
    {
        Write(list->GetGroups()[i]);
    }
}

void
GLCommandBuffer::BindUniform( Uniform* uniform, U32 unit )
{
    Rtt_ASSERT( unit < Uniform::kNumBuiltInVariables );
    UniformUpdate& update = fUniformUpdates[ unit ];
    update.uniform = uniform;
    update.timestamp = gUniformTimestamp++;
}

void
GLCommandBuffer::SetBlendEnabled( bool enabled )
{
    WRITE_COMMAND( enabled ? kCommandEnableBlend : kCommandDisableBlend );
}

static GLenum
GLenumForBlendParam( BlendMode::Param param )
{
    GLenum result = GL_SRC_ALPHA;

    switch ( param )
    {
        case BlendMode::kZero:
            result = GL_ZERO;
            break;
        case BlendMode::kOne:
            result = GL_ONE;
            break;
        case BlendMode::kSrcColor:
            result = GL_SRC_COLOR;
            break;
        case BlendMode::kOneMinusSrcColor:
            result = GL_ONE_MINUS_SRC_COLOR;
            break;
        case BlendMode::kDstColor:
            result = GL_DST_COLOR;
            break;
        case BlendMode::kOneMinusDstColor:
            result = GL_ONE_MINUS_DST_COLOR;
            break;
        case BlendMode::kSrcAlpha:
            result = GL_SRC_ALPHA;
            break;
        case BlendMode::kOneMinusSrcAlpha:
            result = GL_ONE_MINUS_SRC_ALPHA;
            break;
        case BlendMode::kDstAlpha:
            result = GL_DST_ALPHA;
            break;
        case BlendMode::kOneMinusDstAlpha:
            result = GL_ONE_MINUS_DST_ALPHA;
            break;
        case BlendMode::kSrcAlphaSaturate:
            result = GL_SRC_ALPHA_SATURATE;
            break;
        default:
            Rtt_ASSERT_NOT_REACHED();
            break;
    }

    return result;
}

void
GLCommandBuffer::SetBlendFunction( const BlendMode& mode )
{
    WRITE_COMMAND( kCommandSetBlendFunction );

    GLenum srcColor = GLenumForBlendParam( mode.fSrcColor );
    GLenum dstColor = GLenumForBlendParam( mode.fDstColor );

    GLenum srcAlpha = GLenumForBlendParam( mode.fSrcAlpha );
    GLenum dstAlpha = GLenumForBlendParam( mode.fDstAlpha );

    Write<GLenum>( srcColor );
    Write<GLenum>( dstColor );
    Write<GLenum>( srcAlpha );
    Write<GLenum>( dstAlpha );

/*
    GLenum source = GL_SRC_ALPHA;
    GLenum dest = GL_ONE_MINUS_SRC_ALPHA;

    switch( mode )
    {
        case BlendMode::kDisabled:                source = GL_SRC_ALPHA;    dest = GL_ONE_MINUS_SRC_ALPHA;    break;
        case BlendMode::kNormalNonPremultiplied:    source = GL_SRC_ALPHA;    dest = GL_ONE_MINUS_SRC_ALPHA;    break;
        case BlendMode::kNormalPremultiplied:        source = GL_ONE;        dest = GL_ONE_MINUS_SRC_ALPHA;    break;
        case BlendMode::kAdditiveNonPremultiplied:source = GL_SRC_ALPHA;    dest = GL_ONE;                    break;
        case BlendMode::kAdditivePremultiplied:    source = GL_ONE;        dest = GL_ONE;                    break;
        case BlendMode::kScreenNonPremultiplied:    source = GL_SRC_ALPHA;    dest = GL_ONE_MINUS_SRC_COLOR;    break;
        case BlendMode::kScreenPremultiplied:        source = GL_ONE;        dest = GL_ONE_MINUS_SRC_COLOR;    break;
        case BlendMode::kMultiplyNonPremultiplied:source = GL_DST_COLOR;    dest = GL_ONE_MINUS_SRC_ALPHA;    break;
        case BlendMode::kMultiplyPremultiplied:    source = GL_DST_COLOR;    dest = GL_ONE_MINUS_SRC_ALPHA;    break;
        default:                                    Rtt_ASSERT_NOT_REACHED();                                break;
    }
    Write<GLenum>( source );
    Write<GLenum>( dest );
*/
}

void
GLCommandBuffer::SetBlendEquation( RenderTypes::BlendEquation mode )
{
    WRITE_COMMAND( kCommandSetBlendEquation );

    GLenum equation = GL_FUNC_ADD;

    switch( mode )
    {
        case RenderTypes::kSubtractEquation:
            equation = GL_FUNC_SUBTRACT;
            break;
        case RenderTypes::kReverseSubtractEquation:
            equation = GL_FUNC_REVERSE_SUBTRACT;
            break;
        default:
            break;
    }

    Write<GLenum>( equation );
}

void
GLCommandBuffer::SetViewport( int x, int y, int width, int height )
{
    WRITE_COMMAND( kCommandSetViewport );
    Write<GLint>(x);
    Write<GLint>(y);
    Write<GLsizei>(width);
    Write<GLsizei>(height);
}

void
GLCommandBuffer::SetScissorEnabled( bool enabled )
{
    WRITE_COMMAND( enabled ? kCommandEnableScissor : kCommandDisableScissor );
}

void
GLCommandBuffer::SetScissorRegion(int x, int y, int width, int height)
{
    WRITE_COMMAND( kCommandSetScissorRegion );
    Write<GLint>(x);
    Write<GLint>(y);
    Write<GLsizei>(width);
    Write<GLsizei>(height);
}

void
GLCommandBuffer::SetMultisampleEnabled( bool enabled )
{
    WRITE_COMMAND( enabled ? kCommandEnableMultisample : kCommandDisableMultisample );
}

void
GLCommandBuffer::ClearDepth( Real depth )
{
    WRITE_COMMAND( kCommandClearDepth );
    Write<GLfloat>(depth);
}

void
GLCommandBuffer::ClearStencil( U32 stencil )
{
    WRITE_COMMAND( kCommandClearStencil );
    Write<GLuint>(stencil);
}

void
GLCommandBuffer::Clear(Real r, Real g, Real b, Real a)
{
    WRITE_COMMAND( kCommandClear );
    Write<GLfloat>(r);
    Write<GLfloat>(g);
    Write<GLfloat>(b);
    Write<GLfloat>(a);
}

void
GLCommandBuffer::Draw( U32 offset, U32 count, Geometry::PrimitiveType type )
{
    Rtt_ASSERT( fProgram && fProgram->GetGPUResource() );
    ApplyUniforms( fProgram->GetGPUResource() );
    
    WRITE_COMMAND( kCommandDraw );
    switch( type )
    {
        case Geometry::kTriangleStrip:    Write<GLenum>(GL_TRIANGLE_STRIP);    break;
        case Geometry::kTriangleFan:    Write<GLenum>(GL_TRIANGLE_FAN);        break;
        case Geometry::kTriangles:        Write<GLenum>(GL_TRIANGLES);        break;
        case Geometry::kLines:            Write<GLenum>(GL_LINES);            break;
        case Geometry::kLineLoop:        Write<GLenum>(GL_LINE_LOOP);        break;
        default: Rtt_ASSERT_NOT_REACHED(); break;
    }
    Write<GLint>(offset);
    Write<GLsizei>(count);
}

void
GLCommandBuffer::DrawIndexed( U32, U32 count, Geometry::PrimitiveType type )
{
    // The first argument, offset, is currently unused. If support for non-
    // VBO based indexed rendering is added later, an offset may be needed.

    Rtt_ASSERT( fProgram && fProgram->GetGPUResource() );
    ApplyUniforms( fProgram->GetGPUResource() );
    
    WRITE_COMMAND( kCommandDrawIndexed );
    switch( type )
    {
        case Geometry::kIndexedTriangles:    Write<GLenum>(GL_TRIANGLES);    break;
        default: Rtt_ASSERT_NOT_REACHED(); break;
    }
    Write<GLsizei>(count);
}

S32
GLCommandBuffer::GetCachedParam( CommandBuffer::QueryableParams param )
{
    S32 result = -1;
    if (param < kNumQueryableParams)
    {
        result = fCachedQuery[param];
    }
    
    Rtt_ASSERT_MSG(result != -1, "Parameter not cached");
    
    return result;
}

void
GLCommandBuffer::AddCommand( const CoronaCommand* command )
{
    fCustomCommands.Append( command );
}

void
GLCommandBuffer::IssueCommand( U16 id, const void * data, U32 size )
{
    OBJECT_HANDLE_SCOPE();

    Command custom = Command( kNumCommands + id );

    WRITE_COMMAND( custom );
    Write< U32 >( size );

    U8 * buffer = Reserve( size );

    OBJECT_HANDLE_STORE( CommandBuffer, commandBuffer, this );
    
    fCustomCommands[id]->writer( commandBuffer, buffer, data, size );
}

static GLsizei
WriteCount( GLsizei size, GLsizei count, GLsizei valueCount )
{
    GLsizei n = size / (valueCount * sizeof( float ) );
    
    return count < n ? count : n;
}

bool
GLCommandBuffer::WriteNamedUniform( const char * uniformName, const void * data, unsigned int size )
{
    if (!fExtraUniforms)
    {
        Rtt_Log( "No shader bound yet" );
        
        return false;
    }
    
    U32 nameLength = (U32)strlen( uniformName );
    
    if (nameLength >= GLProgram::kUniformNameBufferSize)
    {
        Rtt_Log( "ERROR: Uniform name is %i characters long, maximum length is %i", nameLength, GLProgram::kUniformNameBufferSize - 1 );
        
        return false;
    }
    
    // TODO: validate offset? (probably in CoronaGraphics call)
    GLenum type;
    GLint count, location = fExtraUniforms->Find( uniformName, count, type );
    
    if (location >= 0)
    {
        const float * floatData = static_cast< const float * >( data );
        
        switch (type)
        {
        case GL_FLOAT:
            glUniform1fv( location, WriteCount( size, count, 1 ), floatData );
            break;
        case GL_FLOAT_VEC2:
            glUniform2fv( location, WriteCount( size, count, 2 ), floatData );
            break;
        case GL_FLOAT_VEC3:
            glUniform3fv( location, WriteCount( size, count, 3 ), floatData );
            break;
        case GL_FLOAT_VEC4:
            glUniform4fv( location, WriteCount( size, count, 4 ), floatData );
            break;
        case GL_FLOAT_MAT2:
            glUniformMatrix2fv( location, WriteCount( size, count, 4 ), GL_FALSE, floatData );
            break;
        case GL_FLOAT_MAT3:
            glUniformMatrix3fv( location, WriteCount( size, count, 9 ), GL_FALSE, floatData );
            break;
        case GL_FLOAT_MAT4:
            glUniformMatrix4fv( location, WriteCount( size, count, 16 ), GL_FALSE, floatData );
            break;
        default:
            Rtt_ASSERT_NOT_REACHED();
        }
    }
    
    else
    {
        Rtt_Log( "WARNING: Unable to write to uniform `%s`'s range", uniformName );
    }
    
    return true;
}

Real 
GLCommandBuffer::Execute( bool measureGPU )
{
    DEBUG_PRINT( "--Begin Rendering: GLCommandBuffer --" );

//TODO - make this a property that can be invalidated for specific platforms
//The Mac platform needs to set the default FBO for resize, sleep, zoom functionality
#ifdef Rtt_MAC_ENV
    InitializeFBO();
    //printf("DEFAULTFBO: %d", fDefaultFBO);
#endif

#ifdef ENABLE_GPU_TIMER_QUERIES
    if( measureGPU )
    {
        GLint available = 0;
        GLint id = fTimerQueries[fTimerQueryIndex];
        while( !available )
        {
            glGetQueryObjectiv( id, GL_QUERY_RESULT_AVAILABLE, &available);
        }

        GLuint64 result = 0;
        glGetQueryObjectui64vEXT( id, GL_QUERY_RESULT, &result );
        fElapsedTimeGPU = result * kNanosecondsToMilliseconds;

        glBeginQuery( GL_TIME_ELAPSED, id);
        fTimerQueryIndex = ( fTimerQueryIndex + 1 ) % kTimerQueryCount;
    }
#endif

    OBJECT_HANDLE_SCOPE();

    OBJECT_HANDLE_STORE( CommandBuffer, commandBuffer, this );

    GLExtraUniforms extraUniforms;

    fExtraUniforms = &extraUniforms;

	// Reset the offset pointer to the start of the buffer.
	// This is safe to do here, as preparation work is done
	// on another CommandBuffer while this one is executing.
	fOffset = fBuffer;
    
	S32 windowHeight;
	//GL_CHECK_ERROR();

    GLGeometry* geometry = NULL;
    Geometry::Vertex* instancingData = NULL;
    U32 currentAttributeCount = 0, instanceCount = 0;
    bool clearingDepth = false, clearingStencil = false;

    for( U32 i = 0; i < fNumCommands; ++i )
    {
        Command command = Read<Command>();

// printf( "GLCommandBuffer::Execute [%d/%d] %d\n", i, fNumCommands, command );

        Rtt_ASSERT( command < kNumCommands + fCustomCommands.Length() );
		switch( command )
		{
			case kCommandBindFrameBufferObject:
			{
				GLFrameBufferObject* fbo = Read<GLFrameBufferObject*>();
				bool asDrawBuffer = Read<bool>();
				fbo->Bind( asDrawBuffer );
				DEBUG_PRINT( "Bind FrameBufferObject (as draw buffer = %s): OpenGL name: %i, OpenGL Texture name, if any: %d",
								asDrawBuffer ? "true" : "false",
								fbo->GetName(),
                                fbo->GetTextureName() );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandUnBindFrameBufferObject:
            {
                glBindFramebuffer( GL_FRAMEBUFFER, fDefaultFBO );
                DEBUG_PRINT( "Unbind FrameBufferObject: OpenGL name: %i (fDefaultFBO)", fDefaultFBO );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandCaptureRect:
		      {
			      GLTexture* texture = Read<GLTexture*>();
			      Rect rect = Read<Rect>();
			      Rect unclipped = Read<Rect>();
			      U32 texW = Read<U32>();
			      U32 texH = Read<U32>();
			      U32 x = 0, w = rect.xMax - rect.xMin;
			      U32 y = 0, h = rect.yMax - rect.yMin;
			  
			      if (unclipped.xMin < 0)
			      {
				      x = -unclipped.xMin;
			      }
			  
			      if (unclipped.yMax > rect.yMax)
			      {
				      y = unclipped.yMax - rect.yMax;
			      }
			  
			      if (!texture)
			      {
				      S32 w1 = texW, w2 = unclipped.xMax - unclipped.xMin;
				      S32 h1 = texH, h2 = unclipped.yMax - unclipped.yMin;
				  
				      if (( abs( w1 - w2 ) > 5 || abs( h1 - h2 ) > 5 )) // more than a rounding difference?
				      {
					      x = x * w1 / w2;
					      y = y * h1 / h2;
					      w = w * w1 / w2;
					      h = h * h1 / h2;
				      }
				  
				      GLFrameBufferObject::Blit( rect.xMin, windowHeight - rect.yMax, rect.xMax, windowHeight - rect.yMin, x, y, x + w, y + h, GL_COLOR_BUFFER_BIT, GL_NEAREST );
			      }
			      else
			      {
				      texture->Bind( 0 );

				      glCopyTexSubImage2D( GL_TEXTURE_2D, 0, x, y, rect.xMin, (windowHeight - h) - rect.yMin, w, h );
				  
				      GL_CHECK_ERROR();
			      }
			  
			      DEBUG_PRINT( "Capture Rect: (%f, %f, %f, %f), using FBO = %s", rect.xMin, rect.yMin, rect.xMax, rect.yMax, !texture ? "true" : "false" );
			      CHECK_ERROR_AND_BREAK;
		      }
            case kCommandBindGeometry:
            {
                geometry = Read<GLGeometry*>();
                geometry->Bind();
                DEBUG_PRINT( "Bind Geometry %p (stored on GPU = %s)", geometry, geometry->StoredOnGPU() ? "true" : "false" );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandBindTexture:
            {
                U32 unit = Read<U32>();
                GLTexture* texture = Read<GLTexture*>();
                texture->Bind( unit );
                DEBUG_PRINT( "Bind Texture: texture=%p unit=%i OpenGL name=%d",
                                texture,
                                unit,
                                texture->GetName() );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandBindProgram:
            {
                fCurrentDrawVersion = Read<Program::Version>();
                GLProgram* program = Read<GLProgram*>();
                program->Bind( fCurrentDrawVersion );
 
                program->GetExtraUniformsInfo( fCurrentDrawVersion, extraUniforms );

                DEBUG_PRINT( "Bind Program: program=%p version=%i", program, fCurrentDrawVersion );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandBindInstancing:
            {
                instanceCount = Read<U32>();
                instancingData = Read<Geometry::Vertex*>();
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandResolveVertexFormat:
            {
                Rtt_ASSERT( geometry );
                
                U16 fullCount = Read<U16>();
                U16 vertexSize = Read<U16>();
				U32 offset = Read<U32>();
                
                // Reconstitute any attribute attached to the geometry.
                U16 attributeCount = Read<U16>();
                
                Array< FormatExtensionList::Attribute > attributeArr( fAllocator );

                std::vector< FormatExtensionList::Attribute > attributes;
                std::vector< FormatExtensionList::Group > groups;
                
                for (U32 i = 0; i < attributeCount; ++i)
                {
                    FormatExtensionList::Attribute attribute = Read<FormatExtensionList::Attribute>();
                    
                    attributeArr.Append( attribute );
                }
                
                U16 groupCount = Read<U16>();
                
                Array< FormatExtensionList::Group > groupArr( fAllocator );
                
                for (U32 i = 0; i < groupCount; ++i)
                {
                    FormatExtensionList::Group group = Read<FormatExtensionList::Group>();
                    
                    groupArr.Append( group );
                }

                FormatExtensionList list = FormatExtensionList::FromArrays( groupArr, attributeArr );

                // Bring the enabled arrays into agreement.
                if (!geometry->StoredOnGPU())
                {
                    if (currentAttributeCount > fullCount)
                    {
                        fullCount = currentAttributeCount;
                    }
                    
                    currentAttributeCount = list.GetAttributeCount();
                }
                
                bool hasDivisors = GLGeometry::SupportsDivisors();
                
                for (auto iter = FormatExtensionList::AllAttributes( &list ); !iter.IsDone(); iter.Advance())
                {
                    GLuint index = Geometry::FirstExtraAttribute() + iter.GetAttributeIndex();
                    
                    glEnableVertexAttribArray( index );
                    
                    if (hasDivisors)
                    {
                        GLGeometry::VertexAttribDivisor( index, iter.GetGroup()->divisor );
                    }
                }
                
                for (U32 i = list.GetAttributeCount(); i < fullCount; ++i)
                {
                    glDisableVertexAttribArray( Geometry::FirstExtraAttribute() + i );
                }
                
                // Commit the format.
                geometry->ResolveVertexFormat( &list, vertexSize, offset, instancingData, instanceCount );

                DEBUG_PRINT( "Resolved geometry vertex format" );
                
                instancingData = NULL;
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandApplyUniformScalar:
            {
                READ_UNIFORM_DATA( Real );
                glUniform1f( location, value );
                DEBUG_PRINT( "Set Uniform: value=%f location=%i", value, location );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandApplyUniformVec2:
            {
                READ_UNIFORM_DATA( Vec2 );
                glUniform2fv( location, 1, &value.data[0] );
                DEBUG_PRINT( "Set Uniform: value=(%f, %f) location=%i", value.data[0], value.data[1], location);
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandApplyUniformVec3:
            {
                READ_UNIFORM_DATA( Vec3 );
                glUniform3fv( location, 1, &value.data[0] );
                DEBUG_PRINT( "Set Uniform: value=(%f, %f, %f) location=%i", value.data[0], value.data[1], value.data[2], location);
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandApplyUniformVec4:
            {
                READ_UNIFORM_DATA( Vec4 );
                glUniform4fv( location, 1, &value.data[0] );
                DEBUG_PRINT( "Set Uniform: value=(%f, %f, %f, %f) location=%i", value.data[0], value.data[1], value.data[2], value.data[3], location);
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandApplyUniformMat3:
            {
                READ_UNIFORM_DATA( Mat3 );
                glUniformMatrix3fv( location, 1, GL_FALSE, &value.data[0] );
                DEBUG_PRINT_MATRIX( "Set Uniform: value=", value.data, 9 );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandApplyUniformMat4:
            {
                READ_UNIFORM_DATA( Mat4 );
                glUniformMatrix4fv( location, 1, GL_FALSE, &value.data[0] );
                DEBUG_PRINT_MATRIX( "Set Uniform: value=", value.data, 16 );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandApplyUniformFromPointerScalar:
            {
                READ_UNIFORM_DATA_WITH_PROGRAM( Real );
                glUniform1f( location, value );
                DEBUG_PRINT( "Set Uniform: value=%f location=%i", value, location );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandApplyUniformFromPointerVec2:
            {
                READ_UNIFORM_DATA_WITH_PROGRAM( Vec2 );
                glUniform2fv( location, 1, &value.data[0] );
                DEBUG_PRINT( "Set Uniform: value=(%f, %f) location=%i", value.data[0], value.data[1], location);
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandApplyUniformFromPointerVec3:
            {
                READ_UNIFORM_DATA_WITH_PROGRAM( Vec3 );
                glUniform3fv( location, 1, &value.data[0] );
                DEBUG_PRINT( "Set Uniform: value=(%f, %f, %f) location=%i", value.data[0], value.data[1], value.data[2], location);
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandApplyUniformFromPointerVec4:
            {
                READ_UNIFORM_DATA_WITH_PROGRAM( Vec4 );
                glUniform4fv( location, 1, &value.data[0] );
                DEBUG_PRINT( "Set Uniform: value=(%f, %f, %f, %f) location=%i", value.data[0], value.data[1], value.data[2], value.data[3], location);
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandApplyUniformFromPointerMat3:
            {
                READ_UNIFORM_DATA_WITH_PROGRAM( Mat3 );
                glUniformMatrix3fv( location, 1, GL_FALSE, &value.data[0] );
                DEBUG_PRINT_MATRIX( "Set Uniform: value=", value.data, 9 );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandApplyUniformFromPointerMat4:
            {
                READ_UNIFORM_DATA_WITH_PROGRAM( Mat4 );
                glUniformMatrix4fv( location, 1, GL_FALSE, &value.data[0] );
                DEBUG_PRINT_MATRIX( "Set Uniform: value=", value.data, 16 );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandEnableBlend:
            {
                glEnable( GL_BLEND );
                DEBUG_PRINT( "Enable blend" );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandDisableBlend:
            {
                glDisable( GL_BLEND );
                DEBUG_PRINT( "Disable blend" );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandSetBlendFunction:
            {
                GLenum srcColor = Read<GLenum>();
                GLenum dstColor = Read<GLenum>();

                GLenum srcAlpha = Read<GLenum>();
                GLenum dstAlpha = Read<GLenum>();

                glBlendFuncSeparate( srcColor, dstColor, srcAlpha, dstAlpha );
                DEBUG_PRINT(
                    "Set blend function: srcColor=%i, dstColor=%i, srcAlpha=%i, dstAlpha=%i",
                    srcColor, dstColor, srcAlpha, dstAlpha );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandSetBlendEquation:
            {
                GLenum mode = Read<GLenum>();
                glBlendEquation( mode );
                DEBUG_PRINT( "Set blend equation: mode=%i", mode );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandSetViewport:
            {
                GLint x = Read<GLint>();
                GLint y = Read<GLint>();
                GLsizei width = Read<GLsizei>();
                GLsizei height = Read<GLsizei>();
				windowHeight = height;
                glViewport( x, y, width, height );
                DEBUG_PRINT( "Set viewport: x=%i, y=%i, width=%i, height=%i", x, y, width, height );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandEnableScissor:
            {
                glEnable( GL_SCISSOR_TEST );
                DEBUG_PRINT( "Enable scissor test" );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandDisableScissor:
            {
                glDisable( GL_SCISSOR_TEST );
                DEBUG_PRINT( "Disable scissor test" );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandSetScissorRegion:
            {
                GLint x = Read<GLint>();
                GLint y = Read<GLint>();
                GLsizei width = Read<GLsizei>();
                GLsizei height = Read<GLsizei>();
                glScissor( x, y, width, height );
                DEBUG_PRINT( "Set scissor window x=%i, y=%i, width=%i, height=%i", x, y, width, height );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandEnableMultisample:
            {
                Rtt_glEnableMultisample();
                DEBUG_PRINT( "Enable multisample test" );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandDisableMultisample:
            {
                Rtt_glDisableMultisample();
                DEBUG_PRINT( "Disable multisample test" );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandClearDepth:
            {
                GLfloat depth = Read<GLfloat>();
                Rtt_glClearDepth( depth );
                clearingDepth = true;
                DEBUG_PRINT( "Clear Depth: value=%f", depth );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandClearStencil:
            {
                GLuint stencil = Read<GLuint>();
                glClearStencil( *reinterpret_cast<GLint *>(&stencil) );
                clearingStencil = true;
                DEBUG_PRINT( "Clear Stencil: value=%i", stencil );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandClear:
            {
                GLfloat r = Read<GLfloat>();
                GLfloat g = Read<GLfloat>();
                GLfloat b = Read<GLfloat>();
                GLfloat a = Read<GLfloat>();
                glClearColor( r, g, b, a );
                GLbitfield bits = GL_COLOR_BUFFER_BIT;
                if (clearingDepth)
                {
                    bits |= GL_DEPTH_BUFFER_BIT;
                }
                if (clearingStencil)
                {
                    bits |= GL_STENCIL_BUFFER_BIT;
                }
                clearingDepth = clearingStencil = false;
                glClear( GL_COLOR_BUFFER_BIT | bits );
                DEBUG_PRINT( "Clear: r=%f, g=%f, b=%f, a=%f", r, g, b, a );
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandDraw:
            {
                GLenum mode = Read<GLenum>();
                GLint offset = Read<GLint>();
                GLsizei count = Read<GLsizei>();
                if (0 == instanceCount)
                {
                    glDrawArrays( mode, offset, count );
                    DEBUG_PRINT( "Draw: mode=%i, offset=%i, count=%i", mode, offset, count );
                }
                else
                {
                    GLGeometry::DrawArraysInstanced( mode, offset, count, instanceCount );
                    DEBUG_PRINT( "Draw (instanced): mode=%i, offset=%i, count=%i, instance count=%i", mode, offset, count, instanceCount );
                    instanceCount = 0;
                }
                CHECK_ERROR_AND_BREAK;
            }
            case kCommandDrawIndexed:
            {
                GLenum mode = Read<GLenum>();
                GLsizei count = Read<GLsizei>();
                if (0 == instanceCount)
                {
                    glDrawElements( mode, count, GL_UNSIGNED_SHORT, NULL );
                    DEBUG_PRINT( "Draw indexed: mode=%i, count=%i", mode, count );
                }
                else
                {
                    GLGeometry::DrawElementsInstanced( mode, count, GL_UNSIGNED_SHORT, NULL, instanceCount );
                    DEBUG_PRINT( "Draw indexed (instanced): mode=%i, count=%i, instance count=%i", mode, count, instanceCount );
                    instanceCount = 0;
                }
                CHECK_ERROR_AND_BREAK;
            }
            default:
            {
                U16 id = command - kNumCommands;

                if (id < fCustomCommands.Length())
                {
                    U32 size = Read< U32 >();

                    fCustomCommands[id]->reader( commandBuffer, fOffset, size );

                    fOffset += size;
                }

                else
                {
                    DEBUG_PRINT( "Unknown command(%d)", command );
                    Rtt_ASSERT_NOT_REACHED();
                }

                break;
            }
        }
    }

    fBytesUsed = 0;
    fNumCommands = 0;
    fExtraUniforms = NULL;
    
    for (U32 i = 0; i < currentAttributeCount; ++i)
    {
        glDisableVertexAttribArray( Geometry::FirstExtraAttribute() + i );
    }
    
#ifdef ENABLE_GPU_TIMER_QUERIES
    if( measureGPU )
    {
        glEndQuery( GL_TIME_ELAPSED );
    }
#endif
    
    DEBUG_PRINT( "--End Rendering: GLCommandBuffer --\n" );

    GL_CHECK_ERROR();

    return fElapsedTimeGPU;
}

template <typename T>
T
GLCommandBuffer::Read()
{
    Rtt_ASSERT( fOffset < fBuffer + fBytesAllocated );
    T result;
    memcpy( &result, fOffset, sizeof( T ) );
    fOffset += sizeof( T );
    return result;
}

template <typename T>
void
GLCommandBuffer::Write( T value )
{
	U32 size = sizeof(T);

    /*
    U32 bytesNeeded = fBytesUsed + size;
    if( bytesNeeded > fBytesAllocated )
    {
        U32 doubleSize = fBytesUsed ? 2 * fBytesUsed : 4;
        U32 newSize = Max( bytesNeeded, doubleSize );
        U8* newBuffer = new U8[newSize];

        memcpy( newBuffer, fBuffer, fBytesUsed );
        delete [] fBuffer;

        fBuffer = newBuffer;
        fBytesAllocated = newSize;
    }*/
    U8 * writePos = Reserve( size );

    memcpy( /*fBuffer + fBytesUsed*/writePos, &value, size );
    //fBytesUsed += size;
}

void GLCommandBuffer::ApplyUniforms( GPUResource* resource )
{
    GLProgram* glProgram = static_cast<GLProgram*>(resource);

    Real rawTotalTime;
    bool transformed = false;

    if (fUsesTime)
    {
        const UniformUpdate& time = fUniformUpdates[Uniform::kTotalTime];
        if (fTimeTransform)
        {
            transformed = fTimeTransform->Apply( time.uniform, &rawTotalTime, time.timestamp );
        }
        if (transformed || !TimeTransform::Matches( fTimeTransform, fLastTimeTransform ))
        {
            fUniformUpdates[Uniform::kTotalTime].timestamp = glProgram->GetUniformTimestamp( Uniform::kTotalTime, fCurrentPrepVersion ) - 1; // force a refresh
        }
    }

    for( U32 i = 0; i < Uniform::kNumBuiltInVariables; ++i)
    {
        const UniformUpdate& update = fUniformUpdates[i];
        if( update.uniform && update.timestamp != glProgram->GetUniformTimestamp( i, fCurrentPrepVersion ) )
        {
            ApplyUniform( resource, i );
        }
    }

    if (transformed) // restore raw value (lets us avoid a redundant variable; will also be in place for un-transformed time dependencies)
    {
        fUniformUpdates[Uniform::kTotalTime].uniform->SetValue(rawTotalTime);
    }
}

void GLCommandBuffer::ApplyUniform( GPUResource* resource, U32 index )
{
    const UniformUpdate& update = fUniformUpdates[index];
    GLProgram* glProgram = static_cast<GLProgram*>( resource );
    glProgram->SetUniformTimestamp( index, fCurrentPrepVersion, update.timestamp );

    GLint location = glProgram->GetUniformLocation( Uniform::kViewProjectionMatrix + index, fCurrentPrepVersion );
    if( glProgram->GetHandle() && location >= 0 )
    {
        // The OpenGL program already exists and actually uses the specified uniform
        Uniform* uniform = update.uniform;
        switch( uniform->GetDataType() )
        {
            case Uniform::kScalar:    WRITE_COMMAND( kCommandApplyUniformScalar );    break;
            case Uniform::kVec2:    WRITE_COMMAND( kCommandApplyUniformVec2 );        break;
            case Uniform::kVec3:    WRITE_COMMAND( kCommandApplyUniformVec3 );        break;
            case Uniform::kVec4:    WRITE_COMMAND( kCommandApplyUniformVec4 );        break;
            case Uniform::kMat3:    WRITE_COMMAND( kCommandApplyUniformMat3 );        break;
            case Uniform::kMat4:    WRITE_COMMAND( kCommandApplyUniformMat4 );        break;
            default:                Rtt_ASSERT_NOT_REACHED();                        break;
        }
        Write<GLint>( location );
        WriteUniform( uniform );
    }
    else if( !glProgram->GetHandle() )
    {
        // The OpenGL program has not yet been created. Assume it uses the uniform
        Uniform* uniform = update.uniform;
        switch( uniform->GetDataType() )
        {
            case Uniform::kScalar:    WRITE_COMMAND( kCommandApplyUniformFromPointerScalar );    break;
            case Uniform::kVec2:    WRITE_COMMAND( kCommandApplyUniformFromPointerVec2 );    break;
            case Uniform::kVec3:    WRITE_COMMAND( kCommandApplyUniformFromPointerVec3 );    break;
            case Uniform::kVec4:    WRITE_COMMAND( kCommandApplyUniformFromPointerVec4 );    break;
            case Uniform::kMat3:    WRITE_COMMAND( kCommandApplyUniformFromPointerMat3 );    break;
            case Uniform::kMat4:    WRITE_COMMAND( kCommandApplyUniformFromPointerMat4 );    break;
            default:                Rtt_ASSERT_NOT_REACHED();                                break;
        }
        Write<GLProgram*>( glProgram );
        Write<U32>( index );
        WriteUniform( uniform );
    }
}

void GLCommandBuffer::WriteUniform( Uniform* uniform )
{
	switch( uniform->GetDataType() )
	{
		case Uniform::kScalar:	Write<Real>(*reinterpret_cast<Real*>(uniform->GetData()));	break;
		case Uniform::kVec2:	Write<Vec2>(*reinterpret_cast<Vec2*>(uniform->GetData()));	break;
		case Uniform::kVec3:	Write<Vec3>(*reinterpret_cast<Vec3*>(uniform->GetData()));	break;
		case Uniform::kVec4:	Write<Vec4>(*reinterpret_cast<Vec4*>(uniform->GetData()));	break;
		case Uniform::kMat3:	Write<Mat3>(*reinterpret_cast<Mat3*>(uniform->GetData()));	break;
		case Uniform::kMat4:	Write<Mat4>(*reinterpret_cast<Mat4*>(uniform->GetData()));	break;
		default:				Rtt_ASSERT_NOT_REACHED();									break;
	}
}

U8 *
GLCommandBuffer::Reserve( U32 size )
{
    U32 bytesNeeded = fBytesUsed + size;
    if( bytesNeeded > fBytesAllocated )
    {
        U32 doubleSize = fBytesUsed ? 2 * fBytesUsed : 4;
        U32 newSize = Max( bytesNeeded, doubleSize );
        U8* newBuffer = new U8[newSize];

        memcpy( newBuffer, fBuffer, fBytesUsed );
        delete [] fBuffer;

        fBuffer = newBuffer;
        fBytesAllocated = newSize;
    }

    U8 * buffer = fBuffer + fBytesUsed;

    fBytesUsed += size;

    return buffer;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

#undef READ_UNIFORM_DATA
#undef READ_UNIFORM_DATA_WITH_PROGRAM
#undef CHECK_ERROR_AND_BREAK
#undef WRITE_COMMAND
#undef DEBUG_PRINT
#undef DEBUG_PRINT_MATRIX

// ----------------------------------------------------------------------------
