//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_TextureVolatile.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

TextureVolatile::TextureVolatile(
	Rtt_Allocator* allocator,
	U32 width, U32 height, Format format, Filter filter, Wrap wrapX, Wrap wrapY )
:	Super( allocator ),
	fWidth( width ),
	fHeight( height ),
	fFormat( format ),
	fFilter( filter ),
	fWrapX( wrapX ),
	fWrapY( wrapY )
{
}

U32 
TextureVolatile::GetWidth() const
{
	return fWidth;
}

U32 
TextureVolatile::GetHeight() const
{
	return fHeight;
}

Texture::Format
TextureVolatile::GetFormat() const
{
	return fFormat;
}

Texture::Filter
TextureVolatile::GetFilter() const
{
	return fFilter;
}

Texture::Wrap
TextureVolatile::GetWrapX() const
{
	return fWrapX;
}

Texture::Wrap
TextureVolatile::GetWrapY() const
{
	return fWrapY;
}

void
TextureVolatile::SetFilter( Filter newValue )
{
	fFilter = newValue;
	Invalidate();
}

void
TextureVolatile::SetWrapX( Wrap newValue )
{
	fWrapX = newValue;
	Invalidate();
}

void
TextureVolatile::SetWrapY( Wrap newValue )
{
	fWrapY = newValue;
	Invalidate();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
