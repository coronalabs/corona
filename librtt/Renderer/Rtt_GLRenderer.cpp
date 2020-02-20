//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_GLRenderer.h"

#include "Renderer/Rtt_GLCommandBuffer.h"
#include "Renderer/Rtt_GLFrameBufferObject.h"
#include "Renderer/Rtt_GLGeometry.h"
#include "Renderer/Rtt_GLProgram.h"
#include "Renderer/Rtt_GLTexture.h"
#include "Renderer/Rtt_CPUResource.h"
#include "Core/Rtt_Assert.h"

// TODO: Temporary hack
#ifdef Rtt_IPHONE_ENV
#include "../platform/iphone/Rtt_IPhoneGLVideoTexture.h"
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

GLRenderer::GLRenderer( Rtt_Allocator* allocator )
:   Super( allocator )
{
	fFrontCommandBuffer = Rtt_NEW( allocator, GLCommandBuffer( allocator ) );
	fBackCommandBuffer = Rtt_NEW( allocator, GLCommandBuffer( allocator ) );
}

GPUResource* 
GLRenderer::Create( const CPUResource* resource )
{
	switch( resource->GetType() )
	{
		case CPUResource::kFrameBufferObject: return new GLFrameBufferObject;
		case CPUResource::kGeometry: return new GLGeometry;
		case CPUResource::kProgram: return new GLProgram;
		case CPUResource::kTexture: return new GLTexture;
		case CPUResource::kUniform: return NULL;
#ifdef Rtt_IPHONE_ENV
		case CPUResource::kVideoTexture: return new IPhoneGLVideoTexture;
#endif
		default: Rtt_ASSERT_NOT_REACHED(); return NULL;
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
