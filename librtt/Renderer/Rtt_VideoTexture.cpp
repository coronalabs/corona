//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_VideoTexture.h"

#include "Core/Rtt_Assert.h"

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

VideoTexture::VideoTexture( Rtt_Allocator* allocator )
:	Super( allocator ),
	fSource( kCamera )
{
}

VideoSource
VideoTexture::GetSource() const
{
	return fSource;
}

void
VideoTexture::SetSource( VideoSource source )
{
	fSource = source;
	Invalidate();
}
		
CPUResource::ResourceType 
VideoTexture::GetType() const
{
	return CPUResource::kVideoTexture;
}
		
U32 
VideoTexture::GetWidth() const
{
	return 100;
}

U32 
VideoTexture::GetHeight() const
{
	return 100;
}

Texture::Format
VideoTexture::GetFormat() const
{
	return Super::kLuminance; //Super::kRGBA;
}

Texture::Filter
VideoTexture::GetFilter() const
{
	return Super::kLinear;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
