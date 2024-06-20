//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_TextureResourceCapture.h"
#include "Rtt_TextureResourceCaptureAdapter.h"

#include "Rtt_TextureFactory.h"
#include "Rtt_Display.h"
#include "Rtt_DisplayDefaults.h"
#include "Rtt_StageObject.h"

#include "Renderer/Rtt_TextureVolatile.h"
#include "Renderer/Rtt_FrameBufferObject.h"

namespace Rtt {
	
TextureResourceCapture::TextureResourceCapture(
					  TextureFactory &factory,
					  Texture *texture,
					  FrameBufferObject* fbo,
					  Real width,
					  Real height,
					  int texWidth,
					  int texHeight)
: TextureResource(factory, texture, NULL, kTextureResourceCapture)
, fDstFBO(fbo)
, fContentWidth(width)
, fContentHeight(height)
, fTexWidth(texWidth)
, fTexHeight(texHeight)
{
	
}

TextureResourceCapture *
TextureResourceCapture::Create(
					TextureFactory& factory,
					Real w, Real h,
					int texW, int texH)
{
	Display &display = factory.GetDisplay();
	
	Rtt_Allocator* pAllocator = display.GetAllocator();
	
	Texture::Filter filter = RenderTypes::Convert( display.GetDefaults().GetMagTextureFilter() );
	// ^^ TODO: does this filter need to match the BlitFrameBuffer version?
	Texture::Wrap wrap = RenderTypes::Convert( display.GetDefaults().GetTextureWrapX() );

	Texture *texture = Rtt_NEW( pAllocator,
							   TextureVolatile( display.GetAllocator(), texW, texH, Texture::kRGB, filter, wrap, wrap ) );

	FrameBufferObject * fbo = NULL;
	
	if (display.HasFramebufferBlit( NULL ))
	{
		// TODO: extra options
		fbo = Rtt_NEW( pAllocator, FrameBufferObject( pAllocator, texture ) );
	}
	
	TextureResourceCapture *ret = new TextureResourceCapture(factory, texture, fbo, w, h, texW, texH);
	
	return ret;
}

TextureResourceCapture::~TextureResourceCapture()
{
	if (fDstFBO)
	{
		GetTextureFactory().GetDisplay().GetStage()->QueueRelease(fDstFBO);
	}
}

const MLuaUserdataAdapter&
TextureResourceCapture::GetAdapter() const
{
	return TextureResourceCaptureAdapter::Constant();
}

} // namespace Rtt



