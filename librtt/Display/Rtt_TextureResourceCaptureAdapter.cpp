//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_TextureResourceCaptureAdapter.h"
#include "Rtt_TextureResourceCapture.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_LuaLibDisplay.h"
#include "Rtt_GroupObject.h"

#include "Rtt_RectObject.h"
#include "Rtt_RectPath.h"
#include "Rtt_StageObject.h"
#include "Rtt_Display.h"
#include "Rtt_DisplayDefaults.h"
#include "Rtt_TextureFactory.h"

#include "Renderer/Rtt_Renderer.h"

namespace Rtt {
	
const TextureResourceCaptureAdapter&
TextureResourceCaptureAdapter::Constant()
{
	static const TextureResourceCaptureAdapter sAdapter;
	return sAdapter;
}

StringHash *
TextureResourceCaptureAdapter::GetHash( lua_State *L ) const
{
	static const char *keys[] =
	{
		"width",            //0
		"height",			//1
		"pixelWidth",	    //2
		"pixelHeight",		//3
		"newCaptureEvent"	//4
	};
	
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 5, 2, 1, __FILE__, __LINE__ );
	return &sHash;
}

int
TextureResourceCaptureAdapter::ValueForKey(
						const LuaUserdataProxy& sender,
						lua_State *L,
						const char *key ) const
{
	int results = 0;
	
	Rtt_ASSERT( key ); // Caller should check at the top-most level
	
	const TextureResourceCapture *entry = (const TextureResourceCapture *)sender.GetUserdata();
	if ( ! entry ) { return results; }
	
	int index = GetHash( L )->Lookup( key );
	
	if ( index >= 0 )
	{
		switch ( index )
		{
			case 0:
				lua_pushnumber( L, entry->GetContentWidth() );
				results = 1;
				break;
				
			case 1:
				lua_pushnumber( L, entry->GetContentHeight() );
				results = 1;
				break;

			case 2:
				lua_pushinteger( L, entry->GetTexWidth() );
				results = 1;
				break;
				
			case 3:
				lua_pushinteger( L, entry->GetTexHeight() );
				results = 1;
				break;
				
			case 4:
				Lua::PushCachedFunction( L, Self::newCaptureEvent );
				results = 1;
				break;
				
			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
		}
	}
	else
	{
		results = Super::Constant().ValueForKey( sender, L, key );
	}
	
	return results;
}

bool
TextureResourceCaptureAdapter::SetValueForKey(LuaUserdataProxy& sender,
							lua_State *L,
							const char *key,
							int valueIndex ) const
{
	bool result = false;
	
	Rtt_ASSERT( key ); // Caller should check at the top-most level
	
	TextureResourceCapture *entry = (TextureResourceCapture *)sender.GetUserdata();
	if ( ! entry ) { return result; }
	
	int index = GetHash( L )->Lookup( key );
	
	switch ( index )
	{
		default:
			result = Super::Constant().SetValueForKey( sender, L, key, valueIndex );
			break;
	}
	return result;
}

class CaptureObject : public RectObject
{
	Rtt_CLASS_NO_COPIES( CaptureObject )

	public:
		typedef RectObject Super;
		typedef CaptureObject Self;

	public:
		static CaptureObject *NewCapture( Rtt_Allocator*, TextureResourceCapture* capture );
	
		virtual void Draw( Renderer& renderer ) const;
		virtual bool CanCull() const { return false; }
		virtual bool HitTest( Rtt_Real, Rtt_Real ) const { return false; }

	protected:
		CaptureObject( RectPath *path, TextureResourceCapture* capture );
	
	private:
		WeakPtr<TextureResource> mCaptureResource;
};

// ----------------------------------------------------------------------------

CaptureObject*
CaptureObject::NewCapture( Rtt_Allocator* pAllocator, TextureResourceCapture *capture )
{
	RectPath* path = RectPath::NewRect( pAllocator, 10, 10 ); // arbitrary dimension
	return Rtt_NEW( pAllocator, CaptureObject( path, capture ) );
}

// ----------------------------------------------------------------------------

CaptureObject::CaptureObject( RectPath *path, TextureResourceCapture *capture )
:	Super( path ),
	mCaptureResource( capture->GetWeakResource() )
{
	Rtt_ASSERT( path );
	Rtt_ASSERT( capture );
}

void
CaptureObject::Draw( Renderer& renderer ) const
{
	SharedPtr<TextureResource> tex( mCaptureResource );
	
	if (tex.IsNull())
	{				
		return;
	}
	
	Display& display = GetStage()->GetDisplay();
	TextureResourceCapture & capture = static_cast< TextureResourceCapture & >( *tex );
	
	S32 screenWidth = display.ScreenWidth();
	S32 screenHeight = display.ScreenHeight();
	S32 windowWidth = display.WindowWidth();
	S32 windowHeight = display.WindowHeight();
	
	float screenToWindowX = (float)windowWidth / screenWidth;
	float screenToWindowY = (float)windowHeight / screenHeight;
	
	Vertex2 center;
	
	StageBounds().GetCenter( center );
	
	display.ContentToScreenUnrounded( center.x, center.y );
	
	Rect texBounds;
	
	float width = capture.GetTexWidth();
	float height = capture.GetTexHeight();
/*
N.B. Currently "canScale" is always false, cf. GLFrameBufferObject::HasFramebufferBlit()
TODO: on Mac, in GLView.m, we have NSOpenGLPFASamples,(NSOpenGLPixelFormatAttribute)4,
so we get GL_INVALID_OPERATION due to the mismatch. (It will work if we comment out the
two sample-related lines.) Other platforms probably also have these issues, so for now
I'm just going to disable the feature.
*/
	bool canScale = false;

	if (display.HasFramebufferBlit( &canScale ) && canScale)
	{
		width = fabs( width * GetGeometricProperty( kScaleX ) );
		height = fabs( height * GetGeometricProperty( kScaleY ) );
	}

	texBounds.xMin = -width / 2;
	texBounds.yMin = -height / 2;
	texBounds.xMax = +width / 2;
	texBounds.yMax = +height / 2;
	
	texBounds.Translate( center.x * screenToWindowX, center.y * screenToWindowY );
					
	texBounds.xMin = floorf( texBounds.xMin );
	texBounds.yMin = floorf( texBounds.yMin );
	texBounds.xMax = ceilf( texBounds.xMax ) - 1;
	texBounds.yMax = ceilf( texBounds.yMax ) - 1;

	Rect rawRect = texBounds, bounds;
	
	bounds.xMin = bounds.yMin = 0;
	bounds.xMax = windowWidth;
	bounds.yMax = windowHeight;
	
	texBounds.Intersect( bounds );

	if (texBounds.NotEmpty())
	{
		display.GetRenderer().InsertCaptureRect( capture.GetFBO(), &capture.GetTexture(), texBounds, rawRect );
	}
}

// ----------------------------------------------------------------------------

int
TextureResourceCaptureAdapter::newCaptureEvent( lua_State *L )
{
	LuaUserdataProxy* sender = LuaUserdataProxy::ToProxy( L, 1 );
	
	if (sender)
	{
		TextureResourceCapture *entry = (TextureResourceCapture *)sender->GetUserdata();

		if (entry)
		{
			TextureFactory & factory = entry->GetTextureFactory();
			Display& display = factory.GetDisplay();
		
			int nextArg = 2;
			GroupObject *parent = LuaLibDisplay::GetParent( L, nextArg );

			Real x = luaL_checkreal( L, nextArg++ );
			Real y = luaL_checkreal( L, nextArg++ );

			ShapeObject* v = CaptureObject::NewCapture( display.GetAllocator(), entry );

			int result = LuaLibDisplay::AssignParentAndPushResult( L, display, v, parent );

			if ( display.GetDefaults().IsV1Compatibility() )
			{
				x += Rtt_RealDiv2( 10 );
				y += Rtt_RealDiv2( 10 );
			}
			v->Translate( x, y );
			
			return result;
		}
	}
	
	return 0;
}

} // namespace Rtt
