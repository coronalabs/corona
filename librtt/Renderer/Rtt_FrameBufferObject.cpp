//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_FrameBufferObject.h"
#include "Renderer/Rtt_Texture.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

FrameBufferObject::FrameBufferObject( Rtt_Allocator* allocator, Texture* texture, ExtraOptions * options )
:	CPUResource( allocator ),
	fTexture( texture ),
	fDepthBits( options ? options->depthBits : 0 ),
	fStencilBits( options ? options->stencilBits : 0 ),
	fMustClear( options ? options->mustClear : true )
{
	fTexture->SetTarget( true );
}

CPUResource::ResourceType FrameBufferObject::GetType() const
{
	return CPUResource::kFrameBufferObject;
}

Texture* FrameBufferObject::GetTexture() const
{
	return fTexture;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
