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

#include "Display/Rtt_ObjectBoxList.h"
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

static Rtt::Renderer &
GetRenderer( lua_State * L )
{
    return Rtt::LuaContext::GetRuntime( L )->GetDisplay().GetRenderer();
}

static Rtt::Renderer *
GetRenderer( CoronaRendererOpParams * params )
{
    Rtt::Renderer * renderer = NULL;
    
    if (params)
    {
        if (params->useLuaState)
        {
            renderer = &GetRenderer( params->u.luaState );
        }
        
        else
        {
            renderer = OBJECT_BOX_LOAD( Renderer, params->u.renderer );
        }
    }
    
    return renderer;
}

CORONA_API
int CoronaRendererScheduleEndFrameOp( CoronaRendererOpParams * params, CoronaRendererOp onEndFrame, void * userData, unsigned long * opID )
{
    Rtt::Renderer * renderer = GetRenderer( params );
    
    if (renderer)
    {
        U16 index = renderer->AddEndFrameOp( onEndFrame, userData );
        
        if (index)
        {
            if (opID)
            {
                *opID = index;
            }
            
            return 1;
        }
    }
    
    return 0;
}

CORONA_API
int CoronaRendererCancelEndFrameOp( CoronaRendererOpParams * params, unsigned long opID )
{
    Rtt::Renderer * renderer = GetRenderer( params );
    
    if (renderer && opID)
    {
        Rtt::Array< Rtt::Renderer::CustomOp > & endFrameOps = renderer->GetEndFrameOps();
        
        if (opID <= (unsigned long)endFrameOps.Length())
        {
            endFrameOps[opID - 1].fAction = NULL;
            
            return 1;
        }
    }

    return 0;
}

CORONA_API
int CoronaRendererInstallClearOp( CoronaRendererOpParams * params, CoronaRendererOp onClear, void * userData, unsigned long * opID )
{
    Rtt::Renderer * renderer = GetRenderer( params );
    
    if (renderer)
    {
        U16 index = renderer->AddClearOp( onClear, userData );
        
        if (index)
        {
            Rtt::Array< Rtt::Renderer::CustomOp > & clearOps = renderer->GetClearOps();
            
            static unsigned long id;
            
            clearOps[index - 1].fID = id;
            
            if (opID)
            {
                *opID = id;
            }
            
            id++;
            
            return 1;
        }
    }
    
    return 0;
}

static S32
FindClearOp( Rtt::Array< Rtt::Renderer::CustomOp > & clearOps, unsigned long opID )
{
    for (S32 i = 0; i < clearOps.Length(); ++i)
    {
        if (opID == clearOps[i].fID)
        {
            return i;
        }
    }
    
    return -1;
}

CORONA_API
int CoronaRendererRemoveClearOp( CoronaRendererOpParams * params, unsigned long opID )
{
    Rtt::Renderer * renderer = GetRenderer( params );

    if (renderer)
    {
        S32 index = FindClearOp( renderer->GetClearOps(), opID );
        
        if (index >= 0)
        {
            renderer->GetClearOps().Remove( index, 1 );
            
            return 1;
        }
    }
    
    return 0;
}

CORONA_API
int CoronaRendererDo( const CoronaRenderer * renderer, CoronaRendererOp action, void * userData )
{
    Rtt::Renderer * rendererObject = OBJECT_BOX_LOAD( Renderer, renderer );
    
    if (rendererObject && action)
    {
        rendererObject->Inject( renderer, action, userData );

        return 1;
    }

    return 0;
}

CORONA_API
void CoronaRendererInvalidate( lua_State * L )
{
    Rtt::LuaContext::GetRuntime( L )->GetDisplay().Invalidate();
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
        CoronaLog( "WARNING: Built-in command copy writer has no data source" );
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
    Rtt::Renderer * rendererObject = OBJECT_BOX_LOAD( Renderer, renderer );

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
    const Rtt::CommandBuffer * commandBufferObject = OBJECT_BOX_LOAD( CommandBuffer, commandBuffer );
    
    if (commandBufferObject)
    {
        return commandBufferObject->GetBaseAddress();
    }
    
    return NULL;
}

CORONA_API
int CoronaCommandBufferWriteNamedUniform( const CoronaCommandBuffer * commandBuffer, const char * uniformName, const CoronaWriteUniformParams * params, unsigned int size )
{
    Rtt::CommandBuffer * commandBufferObject = OBJECT_BOX_LOAD( CommandBuffer, commandBuffer );
    
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

CORONA_API
unsigned int CoronaGeometryCopyData( void * dst, const CoronaGeometryMappingLayout * dstLayout, const void * src, const CoronaGeometryMappingLayout * srcLayout )
{
    if (!dst || !dstLayout || !src || !srcLayout)
    {
        return 0U;
    }

    if (!dstLayout->stride || dstLayout->count != srcLayout->count || dstLayout->type != srcLayout->type)
    {
        return 0U;
    }

    U32 valuesSize = 0U;

    switch (dstLayout->type)
    {
    case kAttributeType_Byte:
        valuesSize = 1U;
        break;
    case kAttributeType_Float:
        valuesSize = 4U;
        break;
    default:
        return 0U;
    }

    valuesSize *= dstLayout->count;

    U32 srcDatumSize = srcLayout->stride ? srcLayout->stride : srcLayout->size;

    if (dstLayout->offset + valuesSize > dstLayout->stride || srcLayout->offset + valuesSize > srcDatumSize)
    {
        return 0U;
    }

    U32 n = dstLayout->size / dstLayout->stride;

    if (srcLayout->stride)
    {
        U32 n2 = srcLayout->size / srcLayout->stride;

        if (n2 < n)
        {
            n = n2;
        }
    }
    
    const U8 * srcData = reinterpret_cast< const U8 * >( src ) + srcLayout->offset;
    U8 * dstData = reinterpret_cast< U8 * >( dst ) + dstLayout->offset;

    for (size_t i = 0U; i < n; ++i)
    {
        memcpy( dstData + i * dstLayout->stride, srcData + i * srcLayout->stride, valuesSize );
    }

    return n;
}

static bool GetLayout( const Rtt::Geometry * geometry, const char * name, CoronaGeometryMappingLayout * layout )
{
    if (!name || !name[0])
    {
        return false;
    }

    if (name[1])
    {
        if (strcmp( name, "position" ) == 0)
        {
            layout->count = 3U;
            layout->offset = offsetof( Rtt::Geometry::Vertex, x );
            layout->type = kAttributeType_Float;
        }

        else if (strcmp( name, "texCoord" ) == 0)
        {
            layout->count = 3U;
            layout->offset = offsetof( Rtt::Geometry::Vertex, u );
            layout->type = kAttributeType_Float;
        }

        else if (strcmp( name, "color" ) == 0)
        {
            layout->count = 4U;
            layout->offset = offsetof( Rtt::Geometry::Vertex, rs );
            layout->type = kAttributeType_Byte;
        }

        else if (strcmp( name, "userData" ) == 0)
        {
            layout->count = 4U;
            layout->offset = offsetof( Rtt::Geometry::Vertex, ux );
            layout->type = kAttributeType_Float;
        }

        else if ('u' == name[0] && name[1] && !name[2])
        {
            switch (name[1])
            {
            case 'x':
            case 'y':
            case 'z':
                layout->offset = offsetof( Rtt::Geometry::Vertex, ux ) + (name[1] - 'x') * sizeof( float );

                break;
            case 'w':
                layout->offset = offsetof( Rtt::Geometry::Vertex, uw );

                break;
            default:
                return false;
            }

            layout->count = 1U;
            layout->type = kAttributeType_Float;
        }

        else
        {
            return false;
        }
    }

    else
    {
        U32 offset = 0U;

        switch (*name)
        {
        case 'x':
        case 'y':
        case 'z':
            layout->type = kAttributeType_Float;
            layout->offset = offsetof( Rtt::Geometry::Vertex, x ) + (*name - 'x') * sizeof( float );

            break;
        case 'u':
        case 'v':
        case 'q':
            layout->type = kAttributeType_Float;
            layout->offset = offsetof( Rtt::Geometry::Vertex, u ) + (*name != 'q' ? (*name - 'u') : 2U) * sizeof( float );

            break;
        case 'a':
            ++offset; // n.b. fallthrough...
        case 'b':
            ++offset; // ...again...
        case 'g':
            ++offset; // ...again...
        case 'r':
            layout->type = kAttributeType_Byte;
            layout->offset = offsetof( Rtt::Geometry::Vertex, rs ) + offset;

            break;
        default:
            return false;
        }

        layout->count = 1U;
    }

    layout->stride = sizeof( Rtt::Geometry::Vertex );
    layout->size = sizeof( Rtt::Geometry::Vertex ) * geometry->GetVerticesUsed();

    return true;
}

CORONA_API
void * CoronaGeometryGetMappingFromRenderData( const CoronaRenderData * renderData, const char * name, CoronaGeometryMappingLayout * layout )
{
    Rtt::RenderData * renderDataObject = OBJECT_BOX_LOAD( RenderData, renderData );

    if (renderDataObject && GetLayout( renderDataObject->fGeometry, name, layout ))
    {
        return renderDataObject->fGeometry->GetVertexData();
    }

    return NULL;
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaShaderGetEffectDetail( const CoronaShader * shader, int index, CoronaEffectDetail * detail )
{
    const Rtt::Shader * shaderObject = OBJECT_BOX_LOAD( Shader, shader );

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
    return Rtt::LuaContext::GetRuntime( L )->GetDisplay().GetShaderFactory().UnregisterShellTransform( name );
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaShaderRawDraw( const CoronaShader * shader, const CoronaRenderData * renderData, const CoronaRenderer * renderer )
{
    const Rtt::Shader * shaderObject = OBJECT_BOX_LOAD( Shader, shader );
    const Rtt::RenderData * renderDataObject = OBJECT_BOX_LOAD( RenderData, renderData );
    Rtt::Renderer * rendererObject = OBJECT_BOX_LOAD( Renderer, renderer );

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
    const Rtt::RenderData * renderDataObject = OBJECT_BOX_LOAD( RenderData, renderData );
    const Rtt::Renderer * rendererObject = OBJECT_BOX_LOAD( Renderer, renderer );

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
