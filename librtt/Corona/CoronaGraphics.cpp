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
int CoronaRendererRegisterCommand( lua_State * L, const CoronaCommand * command, unsigned long * commandID )
{
    Rtt::Renderer & renderer = GetRenderer( L );
    
    if (command)
    {
        U16 id = renderer.AddCustomCommand( *command );
        
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

