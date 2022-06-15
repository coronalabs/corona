//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_TextureResourceCanvas.h"
#include "Rtt_TextureResourceCanvasAdapter.h"

#include "Rtt_TextureFactory.h"
#include "Rtt_Display.h"
#include "Rtt_DisplayDefaults.h"
#include "Renderer/Rtt_Renderer.h"

#include "Renderer/Rtt_TextureVolatile.h"
#include "Renderer/Rtt_Matrix_Renderer.h"
#include "Renderer/Rtt_FrameBufferObject.h"
#include "Rtt_Scene.h"

namespace Rtt {
	
TextureResourceCanvas::TextureResourceCanvas(TextureFactory &factory,
											 Texture *texture,
											 FrameBufferObject* fbo,
											 GroupObject *cache,
											 GroupObject *queue,
											 Real width,
											 Real height,
											 int texWidth,
											 int texHeight)
: TextureResource(factory, texture, NULL, kTextureResourceCanvas)
, fDstFBO(fbo)
, fContentWidth(width)
, fContentHeight(height)
, fTexWidth(texWidth)
, fTexHeight(texHeight)
, fGroupQueue(queue)
, fGroupCache(cache)
, fInvalidateCache(false)
, fInvalidateClear(true)
, fClearColor( ColorZero() )
, fAnchorX(0)
, fAnchorY(0)
{

}

TextureResourceCanvas* TextureResourceCanvas::Create(Rtt::TextureFactory &factory, Real width, Real height, int texWidth, int texHeight, Texture::Format format)
{
	Display &display = factory.GetDisplay();
	
	Rtt_Allocator* pAllocator = display.GetAllocator();
	
	Texture::Filter filter = RenderTypes::Convert( display.GetDefaults().GetMagTextureFilter() );
	Texture::Wrap wrap = RenderTypes::Convert( display.GetDefaults().GetTextureWrapX() );

	if (Texture::kLuminance == format)
	{
		format = Texture::kRGBA;
	}

	Texture *texture = Rtt_NEW( pAllocator,
							   TextureVolatile( display.GetAllocator(), texWidth, texHeight, format, filter, wrap, wrap ) );
	
	FrameBufferObject * fbo = Rtt_NEW( pAllocator,
									  FrameBufferObject( pAllocator, texture ) );

	GroupObject *cache = Rtt_NEW( pAllocator,
								 GroupObject(display.GetAllocator(), display.GetStageOffscreen() ) );
	cache->SetRenderedOffScreen( true );
	cache->SetHitTestable( false );
	
	GroupObject *queue = Rtt_NEW( pAllocator,
								 GroupObject(display.GetAllocator(), display.GetStageOffscreen() ) );
	queue->SetRenderedOffScreen( true );
	queue->SetHitTestable( false );

	
	
	TextureResourceCanvas *ret = new TextureResourceCanvas(factory, texture, fbo, cache, queue, width, height, texWidth, texHeight);

	
	return ret;
}

TextureResourceCanvas::~TextureResourceCanvas()
{
	GetTextureFactory().GetDisplay().GetStage()->QueueRelease(fDstFBO);
	Rtt_DELETE( fGroupCache );
	Rtt_DELETE( fGroupQueue );
}

void TextureResourceCanvas::ReleaseLuaReferences( lua_State *L )
{
	// Since this 'fake' groups are never moved to Orphanage, we should release their children proxys
	fGroupCache->ReleaseChildrenLuaReferences(L);
	fGroupQueue->ReleaseChildrenLuaReferences(L);
}

const MLuaUserdataAdapter&
TextureResourceCanvas::GetAdapter() const
{
	return TextureResourceCanvasAdapter::Constant();
}


void TextureResourceCanvas::Invalidate(bool cache, bool clear)
{
	fInvalidateCache = fInvalidateCache || cache;
	fInvalidateClear = fInvalidateClear || clear;
	GetTextureFactory().AddTextureToUpdateList( GetCacheKey() );
	GetTextureFactory().GetDisplay().Invalidate();
}


void TextureResourceCanvas::MoveQueueToCache()
{
	GroupObject *src = GetQueueGroup();
	GroupObject *dest = GetCacheGroup();
	
	for ( int i = 0, iMax = src->NumChildren(); i < iMax; i++ )
	{
		// Always take first object
		DisplayObject *object = src->Release( 0 );
		
		// Append to group
		dest->Insert( -1, object, false );
	}

}

void TextureResourceCanvas::Render(Rtt::Renderer &renderer)
{
	if (fInvalidateCache)
	{// in case we're redrawing everything, first move everything from queue to cache, than draw cache with clear
		MoveQueueToCache();
		Render(renderer, GetCacheGroup(), fInvalidateClear);
	}
	else
	{// in case we do accumulative draw, first, render a queue without clearing, then move queue to cache
		Render(renderer, GetQueueGroup(), fInvalidateClear);
		MoveQueueToCache();
	}
	fInvalidateCache = false;
	fInvalidateClear = false;
}

void TextureResourceCanvas::Render(Rtt::Renderer &renderer, GroupObject *group, bool clear)
{
	Matrix identity;

	Rect contentBounds;
	contentBounds.Initialize(-GetAnchorX()*GetContentWidth(), -GetAnchorY()*GetContentHeight(), GetContentWidth(), GetContentHeight());
	
	group->GetStage()->SetSnapshotBounds( &contentBounds );
	group->UpdateTransform(identity);
	group->Prepare(GetTextureFactory().GetDisplay());
	group->GetStage()->SetSnapshotBounds( NULL );
	
	
	// Save current state so we can restore it later
	FrameBufferObject *fbo = renderer.GetFrameBufferObject();
	
	// Usually we would store viewport and frustrum matrices here (like in SnapshotObject),
	// but textures are rendered before those values are set in Renderer.

	// Render all children to the offscreen texture
	Rtt::Real offscreenViewMatrix[16];
	Rtt::CreateViewMatrix( 0.0f, 0.0f, 0.5f,
						  0.0f, 0.0f, 0.0f,
						  0.0f, 1.0f, 0.0f,
						  offscreenViewMatrix );
	
	Rtt::Real offscreenProjMatrix[16];
	Rtt::CreateOrthoMatrix(
						   contentBounds.xMin, contentBounds.xMax,
						   contentBounds.yMin, contentBounds.yMax,
						   0.0f, 1.0f, offscreenProjMatrix );
	
	renderer.SetFrameBufferObject( fDstFBO );
	renderer.PushMaskCount();
	{
		renderer.SetFrustum( offscreenViewMatrix, offscreenProjMatrix );
		renderer.SetViewport( 0, 0, GetTexWidth(), GetTexHeight() );
		if ( clear )
		{
			ColorUnion color;
			color.pixel = fClearColor;
			const Real inv255 = 1.f / 255.f;
			renderer.Clear( color.rgba.r * inv255, color.rgba.g * inv255, color.rgba.b * inv255, color.rgba.a * inv255 );
		}
		
		group->WillDraw( renderer );
		group->Draw( renderer );
		group->DidDraw( renderer );
	}
	renderer.PopMaskCount();
	
	renderer.SetFrameBufferObject( fbo );
}

} // namespace Rtt



