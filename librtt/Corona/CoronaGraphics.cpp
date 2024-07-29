//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "CoronaGraphics.h"
#include "CoronaLua.h"

#include "Rtt_LuaContext.h"

#include "Display/Rtt_ObjectHandle.h"
#include "Display/Rtt_TextureResource.h"
#include "Core/Rtt_SharedPtr.h"
#include "Display/Rtt_TextureFactory.h"
#include "Rtt_Runtime.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderData.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Display/Rtt_ShaderResource.h"
#include "Renderer/Rtt_CommandBuffer.h"
#include "Renderer/Rtt_Matrix_Renderer.h"
#include "Renderer/Rtt_Renderer.h"

#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderData.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Display/Rtt_ShaderResource.h"
#include "Renderer/Rtt_CommandBuffer.h"
#include "Renderer/Rtt_Matrix_Renderer.h"

#include "Renderer/Rtt_Renderer.h"

#include "Display/Rtt_TextureResourceExternalAdapter.h"


CORONA_API
int CoronaExternalPushTexture( lua_State *L, const CoronaExternalTextureCallbacks *callbacks, void* context)
{
    if ( callbacks->size != sizeof(CoronaExternalTextureCallbacks) )
    {
        CoronaLuaError(L, "TextureResourceExternal - invalid binary version for callback structure; size value isn't valid");
        return 0;
    }
    
    if (callbacks == NULL || callbacks->onRequestBitmap == NULL || callbacks->getWidth == NULL || callbacks->getHeight == NULL )
    {
        CoronaLuaError(L, "TextureResourceExternal - bitmap, width and height callbacks are required");
        return 0;
    }
    
    static unsigned int sNextExternalTextureId = 1;
    char filename[30];
    snprintf(filename, 30, "corona://exTex_%u", sNextExternalTextureId++);
    
    Rtt::TextureFactory& factory = Rtt::LuaContext::GetRuntime( L )->GetDisplay().GetTextureFactory();
    
    Rtt::SharedPtr< Rtt::TextureResource > ret = factory.FindOrCreateExternal(filename, callbacks, context);
    factory.Retain(ret);
    
    if (ret.NotNull())
    {
        ret->PushProxy( L );
        return 1;
    }
    else
    {
        return 0;
    }
}


CORONA_API
void* CoronaExternalGetUserData( lua_State *L, int index )
{
    return Rtt::TextureResourceExternalAdapter::GetUserData( L, index );
}


CORONA_API
int CoronaExternalFormatBPP(CoronaExternalBitmapFormat format)
{
    switch (format)
    {
        case kExternalBitmapFormat_Mask:
            return 1;
        case kExternalBitmapFormat_RGB:
            return 3;
        default:
            return 4;
    }
}

// ----------------------------------------------------------------------------

OBJECT_HANDLE_DEFINE_TYPE( Renderer );
OBJECT_HANDLE_DEFINE_TYPE( RenderData );
OBJECT_HANDLE_DEFINE_TYPE( Shader );
OBJECT_HANDLE_DEFINE_TYPE( ShaderData );
OBJECT_HANDLE_DEFINE_TYPE( CommandBuffer );

// ----------------------------------------------------------------------------

static Rtt::Renderer &
GetRenderer( lua_State * L )
{
    return Rtt::LuaContext::GetRuntime( L )->GetDisplay().GetRenderer();
}

CORONA_API
void CoronaRendererInvalidate( lua_State * L )
{
    Rtt::LuaContext::GetRuntime( L )->GetDisplay().Invalidate();
}

CORONA_API
int CoronaRendererRegisterStateBlock( lua_State * L, const CoronaStateBlock * block, unsigned long * blockID )
{
    Rtt::Renderer & renderer = GetRenderer( L );
    
    if (block && block->stateDirty && block->blockSize)
    {
        U16 id = renderer.AddStateBlock( *block );

        if (id)
        {
            *blockID = id;
            
            return 1;
        }
    }
    
    return 0;
}

CORONA_API
int CoronaRendererReadStateBlock( const CoronaRenderer * renderer, unsigned long blockID, void * data, unsigned int * size )
{
    Rtt::Renderer * rendererObject = OBJECT_HANDLE_LOAD( Renderer, renderer );

    if (rendererObject && blockID && size)
    {
        U32 dataSize = *size;
        U8 * out;
    
        if (rendererObject->GetStateBlockInfo( blockID - 1U, out, *size, false ))
        {
            if (data)
            {
                if (*size < dataSize)
                {
                    dataSize = *size;
                }
                
                memcpy( data, out, dataSize );
            }
            
            return 1;
        }
    }
    
    return 0;
}

CORONA_API
int CoronaRendererWriteStateBlock( const CoronaRenderer * renderer, unsigned long blockID, const void * data, unsigned int size )
{
    Rtt::Renderer * rendererObject = OBJECT_HANDLE_LOAD( Renderer, renderer );

    if (rendererObject && blockID)
    {
        U8 * out;
        unsigned int outSize;
    
        if (rendererObject->GetStateBlockInfo( blockID - 1U, out, outSize, size > 0 ))
        {
            if (outSize < size)
            {
                size = outSize;
            }
            
            if (size > 0)
            {
                memcpy( out, data, size );
            }
            
            return 1;
        }
    }
    
    return 0;
}

// ----------------------------------------------------------------------------

static void
CommandCopyWriter( const CoronaCommandBuffer *, unsigned char * to, const void * data, unsigned int size )
{
    if (data)
    {
        memcpy( to, data, size );
    }
    
    else
    {
        Rtt_TRACE_SIM(( "WARNING: Built-in command copy writer has no data source" ));
    }
}

CORONA_API
int CoronaRendererRegisterCommand( lua_State * L, const CoronaCommand * command, unsigned long * commandID )
{
    Rtt::Renderer & renderer = GetRenderer( L );
    
    if (command && command->reader)
    {
        CoronaCommand localCommand = *command;
        
        if (!localCommand.writer)
        {
            localCommand.writer = CommandCopyWriter;
        }
        
        U16 id = renderer.AddCustomCommand( localCommand );

        if (id)
        {
            *commandID = id;
            
            return 1;
        }
    }
    
    return 0;
}

CORONA_API
int CoronaRendererIssueCommand( const CoronaRenderer * renderer, unsigned long commandID, void * data, unsigned int size )
{
    Rtt::Renderer * rendererObject = OBJECT_HANDLE_LOAD( Renderer, renderer );

    if (rendererObject && commandID && rendererObject->IssueCustomCommand( commandID - 1U, data, size ))
    {
        return 1;
    }
    
    return 0;
}

// ----------------------------------------------------------------------------

CORONA_API
const unsigned char * CoronaCommandBufferGetBaseAddress( const CoronaCommandBuffer * commandBuffer )
{
    const Rtt::CommandBuffer * commandBufferObject = OBJECT_HANDLE_LOAD( CommandBuffer, commandBuffer );
    
    if (commandBufferObject)
    {
        return commandBufferObject->GetBaseAddress();
    }
    
    return NULL;
}

CORONA_API
int CoronaCommandBufferWriteNamedUniform( const CoronaCommandBuffer * commandBuffer, const char * uniformName, const CoronaWriteUniformParams * params, unsigned int size )
{
    Rtt::CommandBuffer * commandBufferObject = OBJECT_HANDLE_LOAD( CommandBuffer, commandBuffer );
    
    if (commandBufferObject && params)
    {
        const void * data;
        
        if (params->useOffset)
        {
            data = commandBufferObject->GetBaseAddress() + params->u.offset;
        }
        
        else
        {
            data = params->u.data;
        }

        if (commandBufferObject->WriteNamedUniform( uniformName, data, size ))
        {
            return 1;
        }
    }
    
    return 0;
}

// ----------------------------------------------------------------------------

static bool GetLayout( const char * name, Rtt::GeometryWriter * writer )
{
    if (!name || !name[0])
    {
        return false;
    }
        
    U32 offset = 0U;

    if (name[1])
    {
        if (strcmp( name, "position" ) == 0)
        {
            writer->fComponents = 3U;
            writer->fOffset = offsetof( Rtt::Geometry::Vertex, x );
            writer->fType = kAttributeType_Float;
            writer->fMask = Rtt::GeometryWriter::kPosition;
        }

        else if (strcmp( name, "texCoord" ) == 0)
        {
            writer->fComponents = 3U;
            writer->fOffset = offsetof( Rtt::Geometry::Vertex, u );
            writer->fType = kAttributeType_Float;
            writer->fMask = Rtt::GeometryWriter::kTexcoord;
        }

        else if (strcmp( name, "color" ) == 0)
        {
            writer->fComponents = 4U;
            writer->fOffset = offsetof( Rtt::Geometry::Vertex, rs );
            writer->fType = kAttributeType_Byte;
            writer->fMask = Rtt::GeometryWriter::kColor;
        }

        else if (strcmp( name, "userData" ) == 0)
        {
            writer->fComponents = 4U;
            writer->fOffset = offsetof( Rtt::Geometry::Vertex, ux );
            writer->fType = kAttributeType_Float;
            writer->fMask = Rtt::GeometryWriter::kUserdata;
        }

        else if ('u' == name[0] && name[1] && !name[2])
        {
            switch (name[1])
            {
            case 'x':
            case 'y':
            case 'z':
                offset = name[1] - 'x';

                writer->fOffset = offsetof( Rtt::Geometry::Vertex, ux ) + offset * sizeof( float );
                writer->fMask = Rtt::GeometryWriter::kUX << offset;

                break;
            case 'w':
                writer->fOffset = offsetof( Rtt::Geometry::Vertex, uw );
                writer->fMask = Rtt::GeometryWriter::kUW;

                break;
            default:
                return false;
            }

            writer->fComponents = 1U;
            writer->fType = kAttributeType_Float;
        }

        else
        {
            return false;
        }
    }

    else
    {
        switch (*name)
        {
        case 'x':
        case 'y':
        case 'z':
            offset = *name - 'x';

            writer->fType = kAttributeType_Float;
            writer->fOffset = offsetof( Rtt::Geometry::Vertex, x ) + offset * sizeof( float );
            writer->fMask = Rtt::GeometryWriter::kX << offset;

            break;
        case 'u':
        case 'v':
        case 'q':
            offset = *name != 'q' ? (*name - 'u') : 2U;

            writer->fType = kAttributeType_Float;
            writer->fOffset = offsetof( Rtt::Geometry::Vertex, u ) + offset * sizeof( float );
            writer->fMask = Rtt::GeometryWriter::kU << offset;

            break;
        case 'a':
            ++offset; // n.b. fallthrough...
        case 'b':
            ++offset; // ...again...
        case 'g':
            ++offset; // ...again...
        case 'r':
            writer->fType = kAttributeType_Byte;
            writer->fOffset = offsetof( Rtt::Geometry::Vertex, rs ) + offset;
            writer->fMask = Rtt::GeometryWriter::kRS << offset;

            break;
        default:
            return false;
        }

        writer->fComponents = 1U;
    }

    return true;
}

CORONA_API
int CoronaGeometrySetComponentWriter ( const CoronaRenderer * renderer, const char * name, CoronaGeometryComponentWriter writer, const void * context, int update )
{
    Rtt::Renderer * rendererObject = OBJECT_HANDLE_LOAD( Renderer, renderer );

    if (rendererObject)
    {
        Rtt::GeometryWriter gw = {};
        
        gw.fContext = context;
        gw.fWriter = writer;

        if ( GetLayout( name, &gw ) && rendererObject->AddGeometryWriter( gw, update != 0 ) )
        {
            return 1;
        }
    }

    return 0;
}

CORONA_API
int CoronaGeometryRegisterVertexExtension( lua_State * L, const char * name, const CoronaVertexExtension * extension )
{
    if (extension && extension->count)
    {
        return Rtt::LuaContext::GetRuntime( L )->GetDisplay().GetShaderFactory().RegisterVertexExtension( name, *extension );
    }
    
    return 0;
}

CORONA_API
int CoronaGeometryUnregisterVertexExtension( lua_State * L, const char * name )
{
    return Rtt::LuaContext::GetRuntime( L )->GetDisplay().GetShaderFactory().UnregisterVertexExtension( name );
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaShaderGetEffectDetail( const CoronaShader * shader, int index, CoronaEffectDetail * detail )
{
    const Rtt::Shader * shaderObject = OBJECT_HANDLE_LOAD( Shader, shader );

    if (shaderObject)
    {
        const Rtt::ShaderData * data = shaderObject->GetData();

        Rtt::SharedPtr< Rtt::ShaderResource > resource( data->GetShaderResource() );

        if (resource.NotNull())
        {
            return resource->GetEffectDetail( index, *detail );
        }
    }

    return 0;
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaShaderRegisterShellTransform( lua_State * L, const char * name, const CoronaShellTransform * transform )
{
    if (transform && transform->begin)
    {
        return Rtt::LuaContext::GetRuntime( L )->GetDisplay().GetShaderFactory().RegisterShellTransform( name, *transform );
    }
    
    return 0;
}

CORONA_API
int CoronaShaderUnregisterShellTransform( lua_State * L, const char * name )
{
	Rtt::ShaderFactory & factory = Rtt::LuaContext::GetRuntime( L )->GetDisplay().GetShaderFactory();
	
	factory.RemoveExternalInfo( L, name, "shellTransform" );
	
    return factory.UnregisterShellTransform( name );
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaShaderRawDraw( const CoronaShader * shader, const CoronaRenderData * renderData, const CoronaRenderer * renderer )
{
    const Rtt::Shader * shaderObject = OBJECT_HANDLE_LOAD( Shader, shader );
    const Rtt::RenderData * renderDataObject = OBJECT_HANDLE_LOAD( RenderData, renderData );
    Rtt::Renderer * rendererObject = OBJECT_HANDLE_LOAD( Renderer, renderer );

    if (shaderObject && renderDataObject && rendererObject)
    {
        shaderObject->Draw( *rendererObject, *renderDataObject );

        return 1;
    }

    return 0;
}

CORONA_API
int CoronaShaderGetVersion( const CoronaRenderData * renderData, const CoronaRenderer * renderer )
{
    const Rtt::RenderData * renderDataObject = OBJECT_HANDLE_LOAD( RenderData, renderData );
    const Rtt::Renderer * rendererObject = OBJECT_HANDLE_LOAD( Renderer, renderer );

    if (renderDataObject && rendererObject)
    {
        return rendererObject->GetVersionCode( !!renderDataObject->fMaskTexture );
    }

    return -1;
}

CORONA_API
int CoronaShaderRegisterEffectDataType( lua_State * L, const char * name, const CoronaEffectCallbacks * callbacks )
{
    return Rtt::LuaContext::GetRuntime( L )->GetDisplay().GetShaderFactory().RegisterDataType( name, *callbacks );
}

CORONA_API
int CoronaShaderUnregisterEffectDataType( lua_State * L, const char * name )
{
    return Rtt::LuaContext::GetRuntime( L )->GetDisplay().GetShaderFactory().UnregisterDataType( name );
}

// ----------------------------------------------------------------------------

CORONA_API
void CoronaMultiplyMatrix4x4( const CoronaMatrix4x4 m1, const CoronaMatrix4x4 m2, CoronaMatrix4x4 result )
{
    Rtt::Multiply4x4( m1, m2, result );
}

CORONA_API
void CoronaCreateViewMatrix( const CoronaVector3 eye, const CoronaVector3 center, const CoronaVector3 up, CoronaMatrix4x4 result )
{
    Rtt::CreateViewMatrix( eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0], up[1], up[2], result );
}

CORONA_API
void CoronaCreateOrthoMatrix( float left, float right, float bottom, float top, float zNear, float zFar, CoronaMatrix4x4 result )
{
    Rtt::CreateOrthoMatrix( left, right, bottom, top, zNear, zFar, result );
}

CORONA_API
void CoronaCreatePerspectiveMatrix( float fovy, float aspectRatio, float zNear, float zFar, CoronaMatrix4x4 result )
{
    Rtt::CreatePerspectiveMatrix( fovy, aspectRatio, zNear, zFar, result );
}

// ----------------------------------------------------------------------------
